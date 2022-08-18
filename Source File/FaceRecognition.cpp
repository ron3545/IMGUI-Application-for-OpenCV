#include "..\Header Files\FaceRecognition.h"
#include "opencv2/face/facerec.hpp"
#include "opencv2/face.hpp"
#include <vector>

using namespace cv;
using namespace face;

#pragma region LBP


bool LBP::FaceRecognition::Detected(cv::Mat& frames)
{
	cv::Mat Grayscale;
	std::vector<cv::Rect> faces;
	
	cv::cvtColor(frames, Grayscale, cv::COLOR_BGR2GRAY);
	cv::resize(Grayscale, Grayscale, cv::Size(Grayscale.size().width / scale, Grayscale.size().height/scale));
	
	cascade.detectMultiScale(Grayscale, faces, 1.2, 3, 0, cv::Size(30, 30));
	if (!faces.empty())
	{
		HighlightFace(frames, faces);
		return true;
	}
	return false;
}

inline std::string LBP::FaceRecognition::InttoStr(int i)
{
	std::ostringstream tmp;
	tmp << i;
	return tmp.str();
}

bool LBP::FaceRecognition::Recognize(cv::Mat frames, const std::vector<cv::Mat>& images, const std::vector<int>& labels)
{
	if (Detected(frames))
	{
		Ptr<FaceRecognizer> recognizer = LBPHFaceRecognizer::create();
		
	}
	return true;
}

LBP::FaceRecognition::~FaceRecognition()
{
	cv::destroyAllWindows();
}

//if database contains no data. This function sholud be called immediately before recognizing
bool LBP::FaceRecognition::register_face(cv::Mat& baseMat,
	std::vector<cv::Mat>& images,  const unsigned int image_number) noexcept
{
	cv::Mat derivedMat = { };
	std::stringstream ss = { };
	unsigned int img_counter = 0;
	
	images.clear(); //should be cleared first bago gamitin
	if (images.empty())
	{
		images.reserve(image_number);
		
		while (img_counter != image_number)
		{
			crop_image(baseMat, derivedMat);
			images.push_back(baseMat);

			++img_counter;
		}
		return true;
	}
	
	return false;
}

void LBP::FaceRecognition::crop_image(const cv::Mat& input, cv::Mat& out) 
{
	std::vector<cv::Rect> faces;
	cv::Rect ROI;

	input.copyTo(out);

	cv::cvtColor(out, out, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(out, out);

	cv::resize(out, out, cv::Size(out.size().width / scale, out.size().height / scale));
	cascade.detectMultiScale(out, faces, 1.1, 3, 0, cv::Size(30, 30));
	
	//process one image at a time
	//dont create copy every iteration
	for (const cv::Rect& face_rect : faces)
	{
		ROI.x = cvRound(face_rect.x * scale);
		ROI.y = cvRound(face_rect.y * scale);

		ROI.height = (out.size().height - (face_rect.y * 3));
		ROI.width = (out.size().width - (face_rect.x * 3));

		out = out(ROI); //croped image
	}
}

void LBP::FaceRecognition::HighlightFace(cv::Mat& frames, std::vector<cv::Rect>& face)
{
	cv::Point textpoint;
	cv::Scalar color = cv::Scalar(0, 255, 0);

	for (const cv::Rect& area : face)
	{
		textpoint.x = area.x;
		textpoint.y = area.y;

		rectangle(frames, cv::Point(cvRound(area.x * scale), cvRound(area.y * scale)), cv::Point((cvRound(area.x + area.width - 1) * scale), (cvRound(area.y + area.height - 1) * scale)),
			color, 2);

		predicted_name = "  ";
		cv::putText(frames, predicted_name, textpoint, 3, 0.5, BoundingBox_Color, 2);
	}
}
#pragma endregion
