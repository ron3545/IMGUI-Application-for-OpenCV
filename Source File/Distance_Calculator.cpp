#include "../Header Files/Distance_Calculator.h"
#include <cmath>

CosineDistance::CosineDistance(const cv::Size& descr_size)
{
	CV_Assert(descr_size.area() != 0);
}

float CosineDistance::Compute(const cv::Mat& descr1, const cv::Mat& descr2)
{
	CV_Assert(!descr1.empty());
	CV_Assert(!descr2.empty());
	CV_Assert(descr1.size() == m_descrSize);
	CV_Assert(descr2.size() == m_descrSize);

	double xy = descr1.dot(descr2);
	double xx = descr1.dot(descr1);
	double yy = descr2.dot(descr2);
	double norm = sqrt(xx * yy) + 1e-6;
	return 0.5f * static_cast<float>(1.0 - xy / norm);
}

std::vector<float> CosineDistance::Compute(const std::vector<cv::Mat>& descrs1, const std::vector<cv::Mat>& descrs2)
{
	CV_Assert(descrs1.size() != 0);
	CV_Assert(descrs1.size() == descrs2.size());

	std::vector < float > distances(descrs1.size(), 1.f);
	for (size_t i = 0; i < descrs1.size(); i++)
		distances.at(i) = Compute(descrs1.at(i), descrs2.at(i));
	return distances;
}

float ImageDistance::Compute(const cv::Mat& descr1, const cv::Mat& descr2)
{
	CV_Assert(!descr1.empty() && !descr2.empty());
	CV_Assert(descr1.size() == descr2.size());
	CV_Assert(descr1.type() == descr2.type());

	cv::Mat temp;
	cv::matchTemplate(descr1, descr2, temp, m_type);
	CV_Assert(temp.size() == cv::Size(1, 1));
	float distance = temp.at<float>(0, 0);
	return m_scale * distance + m_offset;
}

std::vector<float> ImageDistance::Compute(const std::vector<cv::Mat>& descrs1, const std::vector<cv::Mat>& descrs2)
{
	std::vector<float> result;
	for (size_t i = 0; i < descrs1.size(); i++)
		result.push_back(Compute(descrs1[i], descrs2[i]));
	return result;
}
