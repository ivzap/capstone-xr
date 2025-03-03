#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    cv::VideoCapture cap(0);  // Open the default camera (0)
    if (!cap.isOpened()) {
        std::cerr << "Failed to open the camera!" << std::endl;
        return -1;
    }

    // Set the resolution (Optional)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame;
    
    // Measure the time it takes to capture and display frames
    auto start = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    while (true) {
        cap >> frame;  // Capture frame from the camera
        // convert_frame_to_mesh/obj()
        if (frame.empty()) {
            std::cerr << "Failed to capture frame!" << std::endl;
            break;
        }

        // Show the captured frame
        cv::imshow("Camera", frame);
        frame_count++;

        // Measure elapsed time for every 100 frames
        if (frame_count % 100 == 0) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            std::cout << "Time for 100 frames (non-DMA): " << duration.count() << " seconds." << std::endl;
            start = std::chrono::high_resolution_clock::now();  // Reset the start time
        }

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();  // Release the camera
    return 0;
}
