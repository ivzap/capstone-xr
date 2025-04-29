#include <opencv2/opencv.hpp>
#include <cstddef>
#include <iostream>
#include <Processing.NDI.Lib.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

void putTextWithWrap(cv::Mat &frame, const std::string &text, int x, int y, double fontScale, int thickness, const cv::Scalar &color, int maxWidth) {
    std::vector<std::string> words;
    std::string word;
    for (char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word.clear();
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    std::string currentLine;
    int lineHeight = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, nullptr).height + 5;

    int yOffset = y;

    for (const std::string &word : words) {
        std::string testLine = currentLine + " " + word;
        int testWidth = cv::getTextSize(testLine, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, nullptr).width;

        if (testWidth <= maxWidth) {
            currentLine = testLine;
        } else {
            cv::putText(frame, currentLine, cv::Point(x, yOffset), cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness, 8);
            yOffset += lineHeight;
            currentLine = word;
        }
    }

    if (!currentLine.empty()) {
        cv::putText(frame, currentLine, cv::Point(x, yOffset), cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness, 8);
    }
}

std::string readTextFromFile(const std::string &filename) {
    std::ifstream file(filename);
    std::stringstream buffer;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::srand(std::time(0));

    NDIlib_initialize();
    
    NDIlib_send_create_t sendDesc;
    sendDesc.p_ndi_name = "Simple Camera Stream";
    sendDesc.p_groups = nullptr;
    sendDesc.clock_video = true;
    sendDesc.clock_audio = false;

    NDIlib_send_instance_t ndiSender = NDIlib_send_create(&sendDesc);
    if (!ndiSender) {
        std::cerr << "Failed to create NDI sender\n";
        return -1;
    }

    std::string text = readTextFromFile("covid.txt");
    if (text.empty()) {
        return -1;
    }

    int width = 1280;
    int height = 720;

    cv::Mat frame = cv::Mat::zeros(height, width, CV_8UC3);

    int maxWidth = width - 200;

    double fontScale = 1;
    int thickness = 2.3;

    while (true) {
        int x = (width - maxWidth) / 2;
        int y = 100;
        putTextWithWrap(frame, text, x, y, fontScale, thickness, cv::Scalar(255, 255, 255), maxWidth);

        cv::Mat frameBGRA;
        cv::cvtColor(frame, frameBGRA, cv::COLOR_BGR2BGRA);

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

        NDIlib_send_send_video_v2(ndiSender, &videoFrame);

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    NDIlib_send_destroy(ndiSender);
    NDIlib_destroy();
    return 0;
}