#include <opencv2/opencv.hpp>
#include <cstddef>
#include <iostream>
#include <Processing.NDI.Lib.h>
#include <thread>
#include <chrono>

int main() {
    // Initialize NDI library
    NDIlib_initialize();
    
    // Create NDI sender
    NDIlib_send_create_t sendDesc;
    sendDesc.p_ndi_name = "Simple Camera Stream";
    sendDesc.p_groups = nullptr; // No group
    sendDesc.clock_video = true;
    sendDesc.clock_audio = false;

    NDIlib_send_instance_t ndiSender = NDIlib_send_create(&sendDesc);
    if (!ndiSender) {
        std::cerr << "Failed to create NDI sender\n";
        return -1;
    }

    // Open camera
    cv::VideoCapture cap(0); // Open default camera
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera\n";
        return -1;
    }

    int width = 1280;
    int height = 720;
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS, 30);

    // Main loop: capture frames and send via NDI
    while (true) {
        cv::Mat frame;
        cap >> frame; // Capture a frame
        if (frame.empty()) continue;

        // Convert BGR to BGRA (NDI requires 32-bit BGRA format)
        cv::Mat frameBGRA;
        cv::cvtColor(frame, frameBGRA, cv::COLOR_BGR2BGRA);

        // Setup NDI video frame structure
        NDIlib_video_frame_v2_t videoFrame;
        videoFrame.xres = frameBGRA.cols;
        videoFrame.yres = frameBGRA.rows;
        videoFrame.FourCC = NDIlib_FourCC_type_BGRA;
        videoFrame.frame_rate_N = 30;
        videoFrame.frame_rate_D = 1;
        videoFrame.picture_aspect_ratio = (float)frameBGRA.cols / frameBGRA.rows;
        videoFrame.frame_format_type = NDIlib_frame_format_type_progressive;
        videoFrame.timecode = NDIlib_send_timecode_synthesize;
        videoFrame.p_data = frameBGRA.data;
        videoFrame.line_stride_in_bytes = frameBGRA.cols * 4;
        videoFrame.p_metadata = nullptr;

        // Send video frame
        NDIlib_send_send_video_v2(ndiSender, &videoFrame);

        // Simulate frame rate (~30fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // Cleanup
    cap.release();
    NDIlib_send_destroy(ndiSender);
    NDIlib_destroy();
    return 0;
}