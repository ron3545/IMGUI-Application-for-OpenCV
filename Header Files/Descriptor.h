#pragma once
#include <memory>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <openvino/openvino.hpp>

#include "OvDetector.h"
#include "Core.h"
#include "Log.h"


class Image_Description
{
public:
	virtual cv::Size size() const = 0;

	//computes image description
	// param[in] mat Color image.
	// param[out] descr Computed descriptor.
	virtual void Compute(const cv::Mat& mat, cv::Mat* descr) = 0;
	
	/// \brief Computes image descriptors in batches.
	/// \param[in] mats Images of interest.
	/// \param[out] descrs Matrices to store the computed descriptors.
	virtual void Compute(const std::vector<cv::Mat>& mats, std::vector<cv::Mat>* descrs) = 0;
	virtual ~Image_Description() {}
};

class ResizedImage : public Image_Description
{
private:
	cv::Size m_descr_size;
	cv::InterpolationFlags m_interpolation;
public:
	explicit ResizedImage(const cv::Size& descr_size, const cv::InterpolationFlags interpolation)
		: m_descr_size(descr_size), m_interpolation(interpolation)
	{
		PT_CHECK_GT(descr_size.width, 0);
		PT_CHECK_GT(descr_size.height, 0);
	}

	//Get the number of elements in the image descriptor
	cv::Size size() const override { return m_descr_size; }

	//[Param] mat = image of interest
	//[Param] descr = stores computed descriptor
	void Compute(const cv::Mat& mat, cv::Mat* descr) override
	{
		CHECK(descr != nullptr);
		CHECK(!mat.empty());
		cv::resize(mat, *descr, m_descr_size, 0, 0, m_interpolation);
	}
	//by batch
	void Compute(const std::vector<cv::Mat>& mats, std::vector<cv::Mat>* descrs) override
	{
		CHECK(descrs != nullptr);
		descrs->resize(mats.size());
		
		for (size_t i = 0; i < mats.size(); i++)
			Compute(mats[i], &(descrs[i]));
	}
};

class Descriptor : Image_Description
{
public:
	Descriptor(const Config& config, const ov::Core& core, const std::string& deviceName)
		: handler(config, core, deviceName){}

	cv::Size size() const override { return cv::Size(1, handler.size()); }
	void Compute(const cv::Mat& mat, cv::Mat* descr) override
	{ handler.Compute(mat, descr); }

	void Compute(const std::vector<cv::Mat>& mats, std::vector<cv::Mat>* descrs) override
	{ handler.Compute(mats, descrs); }
private:
	VectorCNN handler;
};