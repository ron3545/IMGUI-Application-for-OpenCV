#include "..\Header Files\FaceRecognition.h"
#include "opencv2/face/facerec.hpp"
#include "opencv2/face.hpp"
#include <vector>
#include "../Header Files/Log.h"

using namespace cv;
using namespace std;

FaceRecognition::FaceRecognition(const char* fd_modelpath, const char* fr_modelpath) 
	:	score_threshold(0.9), nms_threshold(0.3), 
		topK(5000), cosine_similar_thresh(0.363), l2norm_similar_thresh(1.128), scale(1.0f)
{	
	//Initialize FaceDetectorYN
	FD = FaceDetectorYN::create(fd_modelpath, "",Size(320, 320), score_threshold, nms_threshold, topK);

	//initialize FaceRecognizerSF
	FR = FaceRecognizerSF::create(fr_modelpath, "");
}

FaceRecognition::FaceRecognition()
	: score_threshold(0.9), nms_threshold(0.3),
	topK(5000), cosine_similar_thresh(0.363), l2norm_similar_thresh(1.128), scale(1.0f)
{
	//Initialize FaceDetectorYN
	//FD = FaceDetectorYN::create("Resources/face_detection_yunet_2022mar.onnx", " " , Size(320, 320), score_threshold, nms_threshold, topK);

	//initialize FaceRecognizerSF
	FR = FaceRecognizerSF::create("Resources/face_recognition_sface_2021dec.onnx", "");
}

bool FaceRecognition::Face_Count(const cv::Mat& image, int required_face_num)
{
	FD->setInputSize(image.size());
	Mat faces;
	int face_count = 0;

	FD->detect(image, faces);
	if (faces.rows < 1)
		return false;

	for (int i = 0; i < faces.rows; i++)
		++face_count;

	if (face_count == required_face_num)
		return true;
	else
		return false; 
}

bool FaceRecognition::Recognize(std::string& label, cv::Mat& frame, const std::map <std::string, jdbc::RegistryImages>& person_images, int frame_width, int frame_height)
{
	FD->setInputSize(Size(frame_width * scale, frame_height * scale));
	
	//inference
	Mat faces, entry_face, features1, features2;
	tm.start();
	FD->detect(frame, faces);
	if (faces.rows < 1)
		return false;

	Mat aligned_face1, aligned_face2;
	for (std::pair<std::string, jdbc::RegistryImages> item : person_images) 
	{
		std::string current_name = item.first;
		for (const cv::Mat& reg_face : item.second.images) 
		{
			FD->detect(reg_face, entry_face);

			FR->alignCrop(reg_face, entry_face.row(0), aligned_face2);
			FR->alignCrop(frame, faces.row(0), aligned_face1);

			FR->feature(aligned_face1, features1);
			features1 = features1.clone();
			FR->feature(aligned_face2, features2);
			features2 = features2.clone();

			double cos_score = FR->match(features1, features2, FaceRecognizerSF::DisType::FR_COSINE);
			double L2_score = FR->match(features1, features2, FaceRecognizerSF::DisType::FR_NORM_L2);

			if (cos_score >= cosine_similar_thresh)//same identity
				label = (current_name);
			
			else
				label = "unknown";
		}
	}
	tm.stop();

	BoundingBox(label, frame, faces, tm.getFPS());
}

bool FaceRecognition::BoundingBox(const std::string& label, const cv::Mat& input, cv::Mat& faces, double fps, int thickness, bool show_fps)
{
	std::string fpsString = cv::format("FPS : %.2f", (float)fps);
	
	for (int i = 0; i < faces.rows; i++)
	{
		Rect bbox = Rect(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)), int(faces.at<float>(i, 2)), int(faces.at<float>(i, 3)));
		
		rectangle(input, bbox, Scalar(0, 255, 0), thickness);

		if(!label.empty())
			putText(input, label, Point(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)) + 18), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0));

		circle(input, Point2i(int(faces.at<float>(i, 4)), int(faces.at<float>(i, 5))), 2, Scalar(255, 0, 0), thickness);
		circle(input, Point2i(int(faces.at<float>(i, 6)), int(faces.at<float>(i, 7))), 2, Scalar(0, 0, 255), thickness);
		circle(input, Point2i(int(faces.at<float>(i, 8)), int(faces.at<float>(i, 9))), 2, Scalar(0, 255, 0), thickness);
		circle(input, Point2i(int(faces.at<float>(i, 10)), int(faces.at<float>(i, 11))), 2, Scalar(255, 0, 255), thickness);
		circle(input, Point2i(int(faces.at<float>(i, 12)), int(faces.at<float>(i, 13))), 2, Scalar(0, 255, 255), thickness);
	}
	if(show_fps)
		putText(input, fpsString, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2);
	
}