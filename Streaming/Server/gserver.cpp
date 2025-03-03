#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

int main() {
    // Open the video file "FLIPRESET.mp4"
    cv::VideoCapture cap("FLIPRESET.mp4");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file FLIPRESET.mp4" << std::endl;
        return -1;
    }

    // Get video properties
    int width  = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    if(fps <= 0) {
        fps = 30; // fallback to 30 fps if unknown
    }

    // Build a GStreamer pipeline string to send the video as RTP
    // Pipeline explanation:
    //  - appsrc: receives frames from OpenCV.
    //  - videoconvert: converts frames to a format suitable for encoding.
    //  - x264enc: encodes frames to H.264 (tune=zerolatency minimizes latency).
    //  - rtph264pay: packetizes the H.264 stream into RTP packets.
    //  - udpsink: sends the RTP packets to the specified host and port.
    std::string pipeline = 
        "appsrc ! videoconvert ! "
        "x264enc tune=zerolatency bitrate=2500 speed-preset=superfast ! "
        "rtph264pay config-interval=1 pt=96 ! "
        "udpsink host=127.0.0.1 port=5000";

    // Create a VideoWriter with the GStreamer pipeline as its output
    cv::VideoWriter writer(pipeline, cv::CAP_GSTREAMER, 0, fps, cv::Size(width, height), true);
    if (!writer.isOpened()) {
        std::cerr << "Error: Could not open VideoWriter with pipeline." << std::endl;
        return -1;
    }

    std::cout << "Streaming FLIPRESET.mp4 via RTP on udp://127.0.0.1:5000" << std::endl;

    cv::Mat frame;
    while (true) {
        if (!cap.read(frame)) {
            std::cout << "End of video." << std::endl;
            break;  // Exit if no more frames
        }
        writer.write(frame);

        // Optionally display the frame locally (for debugging)
        cv::imshow("Streaming Video", frame);
        if (cv::waitKey(30) == 27) {  // Exit on ESC key
            break;
        }
    }

    cap.release();
    writer.release();
    return 0;
}
