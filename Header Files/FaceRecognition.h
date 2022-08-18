#pragma once
#include "libraries.h"
#include <sstream>

namespace LBP 
{
	class FaceRecognition 
	{
	private:
		const int line_thickness, scale;
		const cv::Scalar BoundingBox_Color;
		
		std::string predicted_name;
		cv::CascadeClassifier cascade;
	
	public:
		FaceRecognition() : line_thickness(2),
			BoundingBox_Color(cv::Scalar(0, 255, 0)),
			scale(4), predicted_name(" "), 
			cascade("$(SolutionDir)/Resources/trained data/lbpcascade/lbpcascade_frontalface_improved.xml")
		{}

		FaceRecognition( const std::string& filename) : cascade(filename), 
			line_thickness(2), BoundingBox_Color(cv::Scalar(0, 255, 0)),
			scale(4), predicted_name(" ")
		{}
		~FaceRecognition();
		bool register_face( cv::Mat& baseMat, 
			std::vector<cv::Mat>& images, const unsigned int image_number = 9) noexcept;
		bool Recognize(cv::Mat frames,const std::vector<cv::Mat>& images, const std::vector<int>& labels);
	private:
		void HighlightFace(cv::Mat& mat, std::vector<cv::Rect>& face);
		bool Detected(cv::Mat& frames);
		inline std::string InttoStr(int i);
		void crop_image(const cv::Mat& input, cv::Mat& outut);
	};
}

//the goal is to stream the video whle registering the face
/*reference link for data training :
	https://github.com/ni-chi-Tech
*/

/*
export cv::Mat to mysql:
	https://answers.opencv.org/question/228274/how-to-save-opencv-image-in-mysql-database/

 Convert blob images to Mat images:
	https://stackoverflow.com/questions/23006396/convert-blob-images-to-mat-images
 */