#include "../Header Files/PedestrianTracker.h"
#include "../Header Files/HunggarianAlgorithm.hpp"

#include <stdlib.h>
#include <deque>
#include <map>
#include <ostream>
#include <unordered_map>
#include <set>
#include <utility>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <limits>

#undef max //para matanggal yung #define max(a,b)

namespace
{
    cv::Point Center(const cv::Rect& rect) 
    {
        return cv::Point(static_cast<int>(rect.x + rect.width * 0.5), static_cast<int>(rect.y + rect.height * 0.5));
    }

    std::vector<cv::Point> Centers(const TrackedPersons& detections) 
    {
        std::vector<cv::Point> centers(detections.size());
        for (size_t i = 0; i < detections.size(); i++) {
            centers[i] = Center(detections[i].rect);
        }
        return centers;
    }

    DetectionLog ConvertTracksToDetectionLog(const PersonTracks& tracks)
    {
        DetectionLog logs;

        //combine detected persons by respective indices
        std::map<int64_t, TrackedPersons> objs;
        for (const auto& track : tracks)
        {
            for (const auto& obj : track.second)
            {
                auto iter = objs.find(obj.frame_idx);
                if (iter != objs.end())
                    iter->second.emplace_back(obj);
                else
                    objs.emplace(obj.frame_idx, TrackedPersons{ obj });
            }
        }

        for (const auto& frame_res : objs)
        {
            DetectionLogEntry entry;
            entry.frame_idx = frame_res.first;
            entry.objects = std::move(frame_res.second);
            logs.push_back(std::move(entry));
        }
        return logs;
    }

    inline bool IsInRange(float value, float min, float max)
    {
        return min <= value && value <= max;
    }

    inline bool IsInRange(float value, cv::Vec2f range)
    {
        return IsInRange(value, range[0], range[1]);
    }

    std::vector<cv::Scalar> GetRandomColors(int colors_num)
    {
        std::vector<cv::Scalar> colors(colors_num);
        for (int i = 0; i < colors_num; i++)
            colors[i] = cv::Scalar(static_cast<uchar>(  255. * rand() / RAND_MAX),  
                                                        static_cast<uchar>(255. * rand() / RAND_MAX),  
                                                        static_cast<uchar>(255. * rand() / RAND_MAX));  
        return colors;
    }
} // anonymous namespace

void ValidateParams(const HumanTrackerParams& params)
{
    CV_Assert(params.min_track_duration >= static_cast<size_t>(500));
    CV_Assert(params.min_track_duration <= static_cast<size_t>(10000));

    CV_Assert(params.forget_delay <= static_cast<size_t>(10000));
    CV_Assert(params.affinity_threshold_fast >= 0.0f);
    CV_Assert(params.affinity_threshold_fast <= 1.0f);

    CV_Assert(params.shape_affinity_weight >= 0.0f);
    CV_Assert(params.shape_affinity_weight <= 100.0f);

    CV_Assert(params.Motion_affinity_weight >= 0.0f);
    CV_Assert(params.Motion_affinity_weight <= 100.0f);

    CV_Assert(params.Time_affinity_weight >= 0.0f);
    CV_Assert(params.Time_affinity_weight <= 100.0f);

    CV_Assert(params.min_detection_confidence >= 0.0f);
    CV_Assert(params.min_detection_confidence <= 1.0f);


    CV_Assert(params.bbox_aspect_ratios_range[0] >= 0.0f);
    CV_Assert(params.bbox_aspect_ratios_range[1] <= 10.0f);
    CV_Assert(params.bbox_aspect_ratios_range[0] <= params.bbox_aspect_ratios_range[1]);

    CV_Assert(params.bbox_heights_range[0] >= 10.0f);
    CV_Assert(params.bbox_heights_range[1] <= 1080.0f);
    CV_Assert(params.bbox_heights_range[0] <= params.bbox_heights_range[1]);

    CV_Assert(params.prediction >= 0);
    CV_Assert(params.prediction <= 10000);

    CV_Assert(params.strong_affinity_threshold >= 0.0f);
    CV_Assert(params.strong_affinity_threshold <= 1.0f);

    CV_Assert(params.reidentification_threshold >= 0.0f);
    CV_Assert(params.reidentification_threshold <= 1.0f);

    if (params.max_number_of_persons_in_tracks > 0)
    {
        int min_required_track_length = static_cast<int>(params.forget_delay);
        CV_Assert(params.max_number_of_persons_in_tracks >= min_required_track_length);
        CV_Assert(params.max_number_of_persons_in_tracks <= 10000);
    }
}

HumanTracker::HumanTracker(const HumanTrackerParams& params)
    : params_(params), descriptor_strong_(nullptr), distance_strong_(nullptr), tracks_counter(0),
    frame_size(0, 0), prev_timestamp(std::numeric_limits<uint64_t>::max())
{
    ValidateParams(params);
}

const HumanTrackerParams& HumanTracker::params() const
{
    return params_;
}

const HumanTracker::descriptor& HumanTracker::descriptor_fast() const
{
    return descriptor_fast_;
}

void HumanTracker::set_descriptor_fast(const descriptor& val)
{
    descriptor_fast_ = val;
}

const HumanTracker::descriptor& HumanTracker::strong_descriptor() const 
{
    return descriptor_strong_;
}

void HumanTracker::set_strong_descriptor(const descriptor& val)
{
    descriptor_strong_ = val;
}

const HumanTracker::distance& HumanTracker::fast_distance() const
{
    return distance_fast_;
}

void HumanTracker::set_fast_distance(const distance& val)
{
    distance_fast_ = val;
}

const HumanTracker::distance& HumanTracker::distance_strong() const
{
    return distance_strong_;
}

void HumanTracker::set_distance_strong(const distance& val)
{
    distance_strong_ = val;
}

DetectionLog HumanTracker::GetDetectionLog(const bool valid) const
{
    return ConvertTracksToDetectionLog(all_tracks(valid));
}

const std::unordered_map<size_t, Track>& HumanTracker::tracks()const
{
    return tracks_;
}

const std::set<size_t>& HumanTracker::active_track_ids() const
{
    return active_track_ids_;
}

TrackedPersons HumanTracker::FilterDetections(const TrackedPersons& detections) const
{
    TrackedPersons filtered_detection;
    for (const auto& detection : detections)
    {
        float aspect_ratio = static_cast<float>(detection.rect.height) / detection.rect.width;
        if (detection.detection_confidence > params_.min_detection_confidence && IsInRange(aspect_ratio, params_.bbox_aspect_ratios_range) && IsInRange(static_cast<float>(detection.rect.height), params_.bbox_aspect_ratios_range))
            filtered_detection.emplace_back(detection);
    }
    return filtered_detection;
}

void HumanTracker::SolveAssignmentProblem(const std::set<size_t>& tracks_id, const TrackedPersons& detections, const std::vector<cv::Mat>& descriptors, float thr, std::set<size_t>* unmatched_tracks, std::set<size_t>* unmatched_detections, std::set<std::tuple<size_t, size_t, float>>* matches)
{
    CHECK(unmatched_tracks);
    CHECK(unmatched_detections);
    unmatched_tracks->clear();
    unmatched_detections->clear();

    CHECK(!tracks_id.empty());
    CHECK(!detections.empty());
    CHECK(descriptors.size() == detections.size());
    CHECK(matches);
    matches->clear();

    cv::Mat disimilarity;
    DissimilarityMatrix(tracks_id, detections,descriptors,&disimilarity);
    std::vector<size_t> res = KuhnMunkres().Solve(disimilarity);
}
