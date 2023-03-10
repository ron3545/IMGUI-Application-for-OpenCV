#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <opencv2/core.hpp>

struct TrackedPerson
{
	cv::Rect rect; // ROI of detected person
	double detection_confidence; //-1 if N/A
	int64_t frame_idx; //frame index where object was detected (-1 if N/A)
	int person_id; //unique person identifier(-1 if N/A)
	uint64_t timestamp; //in milliseconds

	TrackedPerson() : detection_confidence(-1), frame_idx(-1), person_id(-1), timestamp(0)
	{}

	TrackedPerson(const cv::Rect& rect, float confidence, int64_t frame_idx, int person_id) 
		: rect(rect), detection_confidence(confidence), frame_idx(frame_idx), person_id(person_id), timestamp(0)
	{}
};

using TrackedPersons = std::deque<TrackedPerson>;
using PersonTracks = std::unordered_map<int, TrackedPersons>;

