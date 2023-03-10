#pragma once
#include "OvDetector.h"
#include "Distance_Calculator.h"
#include "Core.h"
#include "Descriptor.h"
#include "utils.hpp"
#include "Log.h"

#include <stdint.h>
#include <cstddef>
#include <memory>
#include <algorithm>
#include <cmath>
#include <map>
#include <ostream>
#include <set>
#include <unordered_map>
#include <utility>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

struct HumanTrackerParams
{
	size_t min_track_duration; // in millisec
	size_t forget_delay; 
	float affinity_threshold_fast; /*Used to determine if tracklet and detection should be combined*/
	float affinity_threshold_strong; /*Determine if tracklet and detection should be combined*/

	float shape_affinity_weight, Motion_affinity_weight, Time_affinity_weight, min_detection_confidence;
	
	cv::Vec2f bbox_aspect_ratios_range, bbox_heights_range;
	int prediction; /*Determine how many  frames used to predict bounding box incase we lost its track*/

	float strong_affinity_threshold; ///< If 'fast' confidence is greater than this
									/// threshold then 'strong' Re-ID approach is
									/// used.
	float reidentification_threshold;

	bool drop_forgotten_tracks; //enabling this will disable the ability to get detection logs

	int max_number_of_persons_in_tracks; //number of person will be restricted by this parameter. If it is negative or zero, the objecy in track is not restricted.
	
	HumanTrackerParams()  : min_track_duration(1000), forget_delay(150), affinity_threshold_fast(0.8f), 
							affinity_threshold_strong(0.75f), shape_affinity_weight(0.5f), Motion_affinity_weight(0.0f),
							Time_affinity_weight(0.0f), min_detection_confidence(0.65f), bbox_aspect_ratios_range(0.666f, 5.0f),
							bbox_heights_range(40, 1000), prediction(25), strong_affinity_threshold(0.2805f), reidentification_threshold(0.61f),
							drop_forgotten_tracks(true), max_number_of_persons_in_tracks(300)
	{}
};

struct Track
{
	Track(	const TrackedPersons& prsns,
			const cv::Mat& last_image,
			const cv::Mat& descriptor_fast,
			const cv::Mat& descriptor_strong	)
			:	predicted_rect(!prsns.empty() ? prsns.back().rect : cv::Rect()),
				persons( prsns ), m_last_image( last_image ), m_descriptor_fast( descriptor_fast ),
				m_descriptor_strong( descriptor_strong ), lost( 0 ), length( 1 )
	{
		CV_Assert(!prsns.empty());
		first_object_in_tracked = prsns[0];
	}
		
	bool empty() const  { return persons.empty(); }
	size_t size() const { return persons.size();  }
	
	//returns a const reference to detected object with  specified index. param i index of objects.
	const TrackedPerson& operator[] (size_t i) const{ return persons[i]; }

	//operator  [] return non-const reference to detected person with specified index.  Param i Index of objects.
	TrackedPerson& operator[](size_t i) { return persons[i]; }

	//return referece to const
	const TrackedPerson& back() const
	{
		CV_Assert(!empty());
		return persons.back();
	}
	//return reference to non-const
	TrackedPerson& back()
	{
		CV_Assert(!empty());
		return persons.back();
	}

	TrackedPersons persons;
	cv::Rect predicted_rect;
	cv::Mat m_last_image;
	cv::Mat m_descriptor_fast;
	cv::Mat m_descriptor_strong;
	size_t lost; /*frames ago track has been lost*/

	TrackedPerson first_object_in_tracked;
	size_t length;
};


class HumanTracker
{
public:
	using descriptor = std::shared_ptr<Image_Description>;
	using distance = std::shared_ptr< Distance >;

	explicit HumanTracker(const HumanTrackerParams& params = HumanTrackerParams());
	virtual ~HumanTracker(){}

	void Process(const cv::Mat& frame, const TrackedPerson& detection, uint64_t timestamp);

	const HumanTrackerParams& params() const;

	///
   /// \brief Fast descriptor getter.
   /// \return Fast descriptor used in pipeline.
   ///
	const descriptor& descriptor_fast() const;

	///
   /// \brief Fast descriptor setter.
   /// \param[in] val Fast descriptor used in pipeline.
   ///
	void set_descriptor_fast(const descriptor& val);

	///
   /// \brief Strong descriptor getter.
   /// \return Strong descriptor used in pipeline.
   ///
	const descriptor& strong_descriptor() const;

	///
   /// \brief Strong descriptor setter.
   /// \param[in] val Strong descriptor used in pipeline.
   ///
	void set_strong_descriptor(const descriptor& val);
	///
	/// \brief Fast distance getter.
	/// \return Fast distance used in pipeline.
	///
	const distance& fast_distance() const;
	///
	/// \brief Fast distance setter.
	/// \param[in] val Fast distance used in pipeline.
	///
	void set_fast_distance(const distance& val);

	///
   /// \brief Strong distance getter.
   /// \return Strong distance used in pipeline.
   ///
	const distance& distance_strong() const;

	///
	/// \brief Strong distance setter.
	/// \param[in] val Strong distance used in pipeline.
	///
	void set_distance_strong(const distance& val);


	/// \brief Returns a detection log which is used for tracks saving.
	/// \param[in] valid_only If it is true the method returns valid track only.
	/// \return a detection log which is used for tracks saving.
	DetectionLog GetDetectionLog(const bool valid) const;
	
	/// \brief Get active tracks to draw
	/// \return Active tracks.
	///
	std::unordered_map<size_t, std::vector<cv::Point>> GetActiveTracks() const;
	
	//returns tracked detections
	TrackedPersons TrackedDetection() const;

	cv::Mat DrawActiveTracks(const cv::Mat& frame);

	bool IsTrackForgotten(size_t id) const;

	const std::unordered_map<size_t, Track>& tracks() const;

	///
	/// \brief IsTrackValid Checks whether track is valid (duration > threshold).
	/// \param track_id Index of checked track.
	/// \return True if track duration exceeds some predefined value.
	///
	bool IsTrackValid(int track_id) const;

	/// \brief DropForgottenTracks Removes tracks from memory that were lost too
	/// many frames ago.
	void DropForgottenTracks();

private:
	
	struct Match
	{
		int64_t frame_idx1, frame_idx2;
		cv::Rect rect1, rect2, pr_rect1;
		bool pr_label, gt_label;

		Match() {}

		Match(const TrackedPerson& trackedpersonA, const cv::Rect& a_pr_rect, const TrackedPerson& trackedpersonB, bool pr_label)
			:	frame_idx1(trackedpersonA.frame_idx), frame_idx2(trackedpersonB.frame_idx),
				rect1(trackedpersonA.rect), rect2(trackedpersonB.rect), pr_rect1(a_pr_rect),
				pr_label(pr_label), gt_label(trackedpersonA.person_id == trackedpersonB.person_id)
		{	
			CV_Assert(frame_idx1 != frame_idx2);
		}
	};

	const PersonTracks all_tracks(bool valid) const;
	static float ShapeAffinity(float w, const cv::Rect& trk, const cv::Rect& det);
	static float MotionAffinity(float w, const cv::Rect& trk, const cv::Rect& det);
	static float TimeAffinity(float w, const float& trk, const float& det);

	cv::Rect PredictRect(size_t id, size_t k, size_t s) const;
	cv::Rect PredictRectSmoothed(size_t id, size_t k, size_t s) const;
	cv::Rect PredictRectSimple(size_t id, size_t k, size_t s) const;

	void SolveAssignmentProblem(const std::set<size_t>& tracks_id, const TrackedPersons& detections,
								const std::vector<cv::Mat>& descriptors, float thr,
								std::set<size_t>* unmatched_tracks, std::set<size_t>* unmatched_detections,
								std::set<std::tuple<size_t, size_t, float>>* matches
								);

	void ComputeFastDescriptors(const cv::Mat& frame, const TrackedPersons& detection, std::vector<cv::Mat>* descriptors);

	void DissimilarityMatrix(	const std::set<size_t>& active_track_ids, const TrackedPersons& detections, 
								const std::vector<cv::Mat>& fast_descriptor, cv::Mat* dissimilarity_matrix);

	std::vector < double > ComputedDistances(const cv::Mat& frame, const TrackedPersons& detections, 
											 const std::vector<std::pair<size_t, size_t>>& track_and_det_ids, 
											 std::map<size_t, cv::Mat>* det_id_to_descriptor);

	std::map<size_t, std::pair<bool, cv::Mat>> StrongMatching(const cv::Mat& frame, const TrackedPersons& detections,
															  const std::vector<std::pair<size_t, size_t>>& track_and_det_ids);

	std::vector<std::pair<size_t, size_t>> GetTrackToDetectionIds(const std::set<std::tuple<size_t, size_t, float>>& matches);

	double AffinityFast(const cv::Mat& descriptor1, const TrackedPerson& prsn1, const cv::Mat& descriptor2, const TrackedPerson& prsn2);

	float Affinity(const TrackedPerson& prsn1, const TrackedPerson& prsn2);
	void AddNewTrack(const cv::Mat& frame, const TrackedPerson& detection, const cv::Mat& fast_descriptor);
	void AddNewTracks(const cv::Mat& frame, const TrackedPersons& detections, const std::vector<cv::Mat>& descriptor_fast);

	void AddNewTracks(	const cv::Mat& frame,
						const TrackedPersons& detections,
						const std::vector<cv::Mat>& descriptors_fast,
						const std::set<size_t>& ids);

	void AppendToTrack(	const cv::Mat& frame,
						size_t track_id,
						const TrackedPerson& detection,
						const cv::Mat& descriptor_fast,
						const cv::Mat& descriptor_strong);

	bool EraseTrackIfBBoxIsOutOfFrame(size_t track_id);

	bool EraseTrackIfItWasLostTooManyFramesAgo(size_t track_id);

	bool UpdateLostTrackAndEraseIfItsNeeded(size_t track_id);

	void UpdateLostTracks(const std::set<size_t>& track_ids);

	const std::set<size_t>& active_track_ids() const;

	TrackedPersons FilterDetections(const TrackedPersons& detections) const;
	bool IsTrackForgotten(const Track& track) const;

	// Parameters of the pipeline.
	HumanTrackerParams params_;

	// Indexes of active tracks.
	std::set<size_t> active_track_ids_;

	// Descriptor fast (base classifier).
	descriptor descriptor_fast_;

	// distance fast (base classifier).
	distance distance_fast_;

	// Descriptor strong (reid classifier).
	descriptor descriptor_strong_;

	// distance strong (reid classifier).
	distance distance_strong_;

	// All tracks.
	std::unordered_map<size_t, Track> tracks_;

	// Previous frame image.
	cv::Size prev_frame_size_;

	struct  pair_harsh
	{
		std::size_t operator()(const std::pair<size_t, size_t>& p) const
		{
			CV_Assert(p.first < 1e6 && p.second < 1e6);
			return static_cast<size_t>(p.first * 1e6 + p.second);
		}
	};

	//distance between current tracks
	std::unordered_map<std::pair<size_t, size_t>, float, pair_harsh> tracks_distances;

	//number of all current tracks
	size_t tracks_counter;
	cv::Size frame_size;
	std::vector<cv::Scalar> colors;
	uint64_t prev_timestamp;
};