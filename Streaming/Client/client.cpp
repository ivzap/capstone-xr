#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // GStreamer pipeline string for receiving an RTP H264 stream.
    std::string pipeline =
        "udpsrc port=5000 ! "
        "application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, payload=96 ! "
        "rtph264depay ! avdec_h264 ! videoconvert ! appsink";

    // Open the GStreamer pipeline using OpenCV's VideoCapture
    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open GStreamer pipeline." << std::endl;
        return -1;
    }

    cv::namedWindow("RTP Stream", cv::WINDOW_AUTOSIZE);

    cv::Mat frame;
    while (true) {
        if (!cap.read(frame)) {
            std::cerr << "Error: Could not read frame from pipeline." << std::endl;
            break;
        }
        if (frame.empty()) continue;

        cv::imshow("RTP Stream", frame);
        if (cv::waitKey(30) == 27) { // Exit on ESC key
            break;
        }
    }

    return 0;
}
