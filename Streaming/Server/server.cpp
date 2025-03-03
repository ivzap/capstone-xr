#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <sys/socket.h>
#include <semaphore.h>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>    
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <thread>

#include <cstring>  // for strerror
#include <cerrno>   // for errno
/**
 * 
 * tcp_connections = []
 * 
 * THREAD 1
 * func send_frames(tcp_connections)
 *     while(true)
 *         frame = read_frame(device)
 *         for conn in tcp_connections
 *              if conn.isAlive()
 *                  udp_send(frame)
 * 
 * THREAD 2
 *  func check_tcp_connections(tcp_connections)
 *      for conn in conn:
 *          if !conn.keep_alive_acked()
 *              conn.dead = true
 * 
 * func server_init()
 *  thread(send_frames, tcp_connections)
 * 
 * THREAD 3 (main)
 * while(true)
 *      new_conn = wait_and_connect(client)
 *      tcp_connections.add(new_conn)
 *      
 * 
 */

sem_t sem; // Declare semaphore

// Function to calculate 16-bit one's complement checksum
uint16_t calculate_checksum(const std::vector<uchar>& data) {
    uint32_t sum = 0;

    // Process every 16-bit chunk in the data
    for (size_t i = 0; i < data.size(); i += 2) {
        // Combine two bytes to form a 16-bit word
        uint16_t word = 0;
        word |= data[i];            // Lower byte
        if (i + 1 < data.size()) {
            word |= (data[i + 1] << 8);  // Upper byte
        }

        // Add the 16-bit word to the sum
        sum += word;

        // If sum overflows 16 bits, wrap the carry around to the lower 16 bits
        if (sum > 0xFFFF) {
            sum -= 0xFFFF;
        }
    }

    // Take one's complement of the sum
    return ~sum;
}

bool set_tcp_keep_alive_interval(int sockfd, int idle_time, int interval, int max_probes) {
    int optval = 1;

    // Enable TCP keep-alive
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
        perror("setsockopt SO_KEEPALIVE");
        return false;
    }

    // Set the keep-alive idle time
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &idle_time, sizeof(idle_time)) < 0) {
        perror("setsockopt TCP_KEEPIDLE");
        return false;
    }

    // Set the keep-alive interval
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
        perror("setsockopt TCP_KEEPINTVL");
        return false;
    }

    // Set the maximum number of keep-alive probes
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &max_probes, sizeof(max_probes)) < 0) {
        perror("setsockopt TCP_KEEPCNT");
        return false;
    }

    return true;
}

void send_frame(std::map<int, sockaddr_in>& client_conns, std::vector<uchar>& frame, int udp_sock){
    std::vector<int> invalid_socks;
    for(auto& [sock, addr]: client_conns){
        // send the data via UDP
        ssize_t sent_bytes = sendto(udp_sock, frame.data(), frame.size(), 0, 
        (struct sockaddr*)&addr, sizeof(addr));
        if (sent_bytes < 0) {
            std::cout << "recv failed with error: " << strerror(errno) << std::endl;
            perror("sendto");
        }
        
        // check if socket conn is still up
        char buf[1];
        ssize_t sock_status = recv(sock, buf, 1, 0);
        
        if(errno != EAGAIN && errno != EWOULDBLOCK){
            perror("client disconnected, updating connection list...");
            invalid_socks.push_back(sock);
        }
    }

    for(int sock: invalid_socks){
        close(sock);
        client_conns.erase(sock);
    }
}

void capture_and_send_frame(std::map<int, sockaddr_in>& client_conns, int width, int height, int udp_sock) {
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame; 

        if (frame.empty()) {
            std::cerr << "Error: Captured empty frame." << std::endl;
            continue;
        }

        std::vector<uchar> jpeg_buffer;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90}; // Set JPEG quality
        cv::imencode(".jpg", frame, jpeg_buffer, params); // Encode frame to JPEG

        // encode size of packet in first 4 bytes (network byte order)
        uint32_t size = htonl(static_cast<uint32_t>(jpeg_buffer.size()));
        std::vector<uchar> size_buffer(sizeof(size)); // Create a buffer to hold the size
        std::memcpy(size_buffer.data(), &size, sizeof(size));

        // insert 4 bytes at front of packet payload
        jpeg_buffer.insert(jpeg_buffer.begin(), size_buffer.begin(), size_buffer.end());

        sem_wait(&sem);
        send_frame(client_conns, jpeg_buffer, udp_sock);
        sem_post(&sem);

    }
}



int main(int argc, char* argv[]) {

    std::string hostname, port, width, height;
    int opt;
    while ((opt = getopt(argc, argv, "h:p:x:y:")) != -1) {
        switch (opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'x':
                width = optarg;
                break;
            case 'y':
                height = optarg;
                break;
        }
    }

    std::map<int, sockaddr_in> client_conns;

    sem_init(&sem, 0, 1); 
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname.c_str()); 
    server_addr.sin_port = htons(stoi(port));       

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, 5) == -1) { 
        perror("listen");
        close(sockfd);
        return 1;
    }
     
    std::thread worker_thread(capture_and_send_frame, std::ref(client_conns), stoi(width), stoi(height), udp_sock);

    while(true){
        // accept a client connection
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_sockfd == -1) {
            perror("accept");
            close(sockfd);
            return 1;
        }
        
        // set socket to nonblocking, if we cant send at the moment skip frame...
        int default_flags = fcntl(client_sockfd, F_GETFL, 0);
        fcntl(client_sockfd, F_SETFL, default_flags | O_NONBLOCK);
        
        set_tcp_keep_alive_interval(client_sockfd, 3, 5, 2);

        sem_wait(&sem);
        client_conns[client_sockfd] = client_addr;
        sem_post(&sem);
    }

    worker_thread.join();
    close(sockfd);
}
