#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>  // Include this header for inet_pton
#include <fstream>
#include <opencv2/opencv.hpp>

int process_frame(const std::vector<uchar>& frame) {
    // Decode the JPEG frame
    cv::Mat img = cv::imdecode(frame, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Failed to decode frame." << std::endl;
        return 0;
    }

    // Display the image
    cv::imshow("Received Frame", img);
    
    cv::waitKey(1); // Allow OpenCV to process the window events

    return 1;
}

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

int main(int argc, char* argv[]) {

    std::string hostname, port;
    int opt;
    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
        }
    }


    // Create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Define the server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(stoi(port));
    server_addr.sin_addr.s_addr = inet_addr(hostname.c_str()); // Change this to server's IP address
    socklen_t addr_len = sizeof(server_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // Now retrieve the local address and port used for tcp connection
    struct sockaddr_in local_addr;
    if (getsockname(sockfd, (struct sockaddr*)&local_addr, &addr_len) < 0) {
        perror("getsockname");
        close(sockfd);
        return 1;
    }

    // Bind the UDP socket to the specified local address and port
    if (bind(udp_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(udp_sock);
        return 1;
    }   

    uint32_t frames = 0;
    uint32_t bad_frames = 0;
    
    std::vector<uchar> packet(150000, 0);
    while (true) {

        ssize_t bytes_received = recvfrom(udp_sock, packet.data(), packet.size(), 0,
                                  (struct sockaddr*)&local_addr, &addr_len);
        if (bytes_received <= 0) {
            std::cerr << "Connection closed or error occurred." << std::endl;
            break;
        }

        uint32_t frame_size;
        std::memcpy(&frame_size, packet.data(), sizeof(frame_size)); // Copy the first 4 bytes
        frame_size = htonl(frame_size);

        uint16_t checksum = calculate_checksum(packet);

        std::vector<uchar> frame(packet.begin()+sizeof(frame_size), packet.begin()+sizeof(frame_size)+frame_size);

        frames++;
        bad_frames += !process_frame(frame);
    }

    // Close the socket
    close(sockfd);
    return 0;
}
