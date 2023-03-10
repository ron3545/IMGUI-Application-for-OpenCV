#pragma once

#include "libraries.h"
#include <vector>

class Distance
{
public:
	virtual float Compute(const cv::Mat& descr1, const cv::Mat& descr2) = 0;
	virtual std::vector<float> Compute(const std::vector<cv::Mat>& descrs1, const std::vector<cv::Mat>& descrs2) = 0;

	virtual ~Distance() {}
};

class CosineDistance : public Distance
{
	cv::Size m_descrSize;
public:
	explicit CosineDistance(const cv::Size& descr_size);

	float Compute(const cv::Mat& descr1, const cv::Mat& descr2) override;
	std::vector<float> Compute(const std::vector<cv::Mat>& descrs1, const std::vector<cv::Mat>& descrs2) override;
};

class ImageDistance : public Distance
{
	int m_type;
	float m_scale;
	float m_offset;
public:
	ImageDistance(int type = cv::TemplateMatchModes::TM_CCORR_NORMED, float scale = -1, float offset = 1)
		: m_type(type), m_scale(scale), m_offset(offset)
	{}
	float Compute(const cv::Mat& descr1, const cv::Mat& descr2) override;
	std::vector<float> Compute(const std::vector<cv::Mat>& descrs1, const std::vector<cv::Mat>& descrs2);
};