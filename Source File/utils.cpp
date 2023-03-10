#include "../Header Files/utils.hpp"
#include <stddef.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include <opencv2/imgproc.hpp>
#include "../Header Files/Core.h"


namespace {
    template <typename StreamType, typename EndlType>
    void SaveDetectionLogToStream(StreamType& stream, const EndlType& endl, const DetectionLog& log) {
        for (const auto& entry : log) {
            std::vector<TrackedPerson> objects(entry.objects.begin(), entry.objects.end());
            std::sort(objects.begin(), objects.end(), [](const TrackedPerson& a, const TrackedPerson& b) {
                return a.person_id < b.person_id;
                });
            for (const auto& object : objects) {
                auto frame_idx_to_save = entry.frame_idx;
                stream << frame_idx_to_save << ',';
                stream << object.person_id << ',' << object.rect.x << ',' << object.rect.y << ',' << object.rect.width
                    << ',' << object.rect.height;
                stream << endl;
            }
        }
    }
}  // anonymous namespace

void DrawPolyline(const std::vector<cv::Point>& polyline, const cv::Scalar& color, cv::Mat* image, int lwd) {
    CV_Assert(image);
    CV_Assert(!image->empty());
    CV_Assert(image->type() == CV_8UC3);
    CV_Assert(lwd > 0);
    CV_Assert(lwd < 20);

    for (size_t i = 1; i < polyline.size(); i++) {
        cv::line(*image, polyline[i - 1], polyline[i], color, lwd);
    }
}

void SaveDetectionLogToTrajFile(const std::string& path, const DetectionLog& log) {
    std::ofstream file(path.c_str());
    CV_Assert(file.is_open());
    SaveDetectionLogToStream(file, '\n', log);
}
