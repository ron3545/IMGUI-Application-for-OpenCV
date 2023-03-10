#pragma once
#include "libraries.h"
#include <sstream>
#include "DataBase.h"

class FaceRecognition
{
private:
	float score_threshold, nms_threshold, topK;
	double cosine_similar_thresh, l2norm_similar_thresh;
	float scale;

	cv::Ptr<cv::FaceDetectorYN> FD;
	cv::Ptr<cv::FaceRecognizerSF> FR;
	cv::TickMeter tm;
	
public:
	FaceRecognition(const char* fd_modelpath, const char* fr_modelpath);
	FaceRecognition();
	
	bool Recognize(std::string& label, cv::Mat& frame, const std::map <std::string, jdbc::RegistryImages>& person_images,
		int frame_width, int frame_height);

	//returns true if face counted is equal to the expected value set by the user
	bool Face_Count(const cv::Mat& image, int required_face_num = 1);  
private:
	FaceRecognition(const FaceRecognition&);				//same reason na nakalagay sa DataBase.h
	FaceRecognition& operator=(const FaceRecognition&);

	bool BoundingBox(const std::string& label ,const cv::Mat& input, cv::Mat& faces, double fps, int thickness = 2, bool show_fps = false);
};