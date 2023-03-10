
#include "../Header Files/OvDetector.h"
#include "../Header Files/Log.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <openvino/openvino.hpp>

using namespace CPlusPlusLogging;
BaseModel::BaseModel(const Config& config, const ov::Core& core, const std::string& device_name)
    : config(config),
    core(core),
    device_name(device_name) {}

void BaseModel::Load() {
    auto model = core.read_model(config.path_to_model);

    if (model->inputs().size() != 1) {
        throw std::logic_error("Demo supports topologies with only 1 input");
    }

    ov::preprocess::PrePostProcessor ppp(model);
    input_layout = getLayoutFromShape(model->input().get_shape());
    ppp.input().tensor().set_element_type(ov::element::u8).set_layout({ "NCHW" });

    ppp.input().model().set_layout(input_layout);

    if (model->outputs().size() != 1) {
        throw std::runtime_error("Demo supports topologies with only 1 output");
    }
    ppp.output().tensor().set_element_type(ov::element::f32);

    model = ppp.build();

    input_shape = model->input().get_shape();
    input_shape[ov::layout::batch_idx(input_layout)] = config.max_batch_size;
    output_shape = model->output().get_shape();
    ov::set_batch(model, { 1, static_cast<int64>(config.max_batch_size) });

    compiled_model = core.compile_model(model, device_name);

    infer_request = compiled_model.create_infer_request();
    input_tensor = infer_request.get_input_tensor();
    output_tensor = infer_request.get_output_tensor();
}

void BaseModel::InferBatch(const std::vector<cv::Mat>& frames,
    const std::function<void(const ov::Tensor&, size_t)>& fetch_results) const {
    size_t num_imgs = frames.size();
    input_tensor.set_shape(input_shape);
    for (size_t batch_i = 0; batch_i < num_imgs;) {
        int x = num_imgs - batch_i;
        size_t batch_size = (x < (size_t)config.max_batch_size ? x : (size_t)config.max_batch_size);
        for (size_t b = 0; b < batch_size; ++b) {
            matToTensor(frames[batch_i + b], input_tensor, b);
        }
        infer_request.set_input_tensor(ov::Tensor(input_tensor,
            { 0, 0, 0, 0 },
            { batch_size,
             input_shape[ov::layout::channels_idx(input_layout)],
             input_shape[ov::layout::height_idx(input_layout)],
             input_shape[ov::layout::width_idx(input_layout)] }));
        infer_request.infer();
        fetch_results(infer_request.get_output_tensor(), batch_size);
        batch_i += batch_size;
    }
}

VectorCNN::VectorCNN(const Config& config, const ov::Core& core, const std::string& deviceName)
    : BaseModel(config, core, deviceName) {
    Load();
    result_size = std::accumulate(std::next(output_shape.begin(), 1), output_shape.end(), 1, std::multiplies<int>());
}

void VectorCNN::Compute(const cv::Mat& frame, cv::Mat* vector) const {
    std::vector<cv::Mat> output;
    Compute({ frame }, &output);
    *vector = output[0];
}

void VectorCNN::Compute(const std::vector<cv::Mat>& images, std::vector<cv::Mat>* vectors) const {
    if (images.empty()) {
        return;
    }
    vectors->clear();
    auto results_fetcher = [vectors](const ov::Tensor& tensor, size_t batch_size) {
        ov::Shape shape = tensor.get_shape();
        std::vector<int> tensor_sizes(shape.size(), 0);
        for (size_t i = 0; i < tensor_sizes.size(); ++i) {
            tensor_sizes[i] = shape[i];
        }
        cv::Mat out_tensor(tensor_sizes, CV_32F, tensor.data<float>());
        for (size_t b = 0; b < batch_size; b++) {
            cv::Mat tensor_wrapper(out_tensor.size[1],
                1,
                CV_32F,
                reinterpret_cast<void*>((out_tensor.ptr<float>(0) + b * out_tensor.size[1])));
            vectors->emplace_back();
            tensor_wrapper.copyTo(vectors->back());
        }
    };
    InferBatch(images, results_fetcher);
}

static inline  ov::Layout getLayoutFromShape(const ov::Shape& shape)
{
    if (shape.size() == 2) {
        return "NC";
    }
    else if (shape.size() == 3) {
        return (shape[0] >= 1 && shape[0] <= 4) ? "CHW" :
            "HWC";
    }
    else if (shape.size() == 4) {
        return (shape[1] >= 1 && shape[1] <= 4) ? "NCHW" :
            "NHWC";
    }
    else {
        throw std::runtime_error("Usupported " + std::to_string(shape.size()) + "D shape");
    }

}

template<typename T> const T getMatValue(const cv::Mat& mat, size_t height, size_t width, size_t channels)
{
    switch (mat.type())
    {
    case CV_8UC1:  return (T)mat.at<uchar>(height, width);
    case CV_8UC3:  return (T)mat.at<cv::Vec3b>(height, width)[channels];
    case CV_32FC1: return (T)mat.at<float>(height, width);
    case CV_32FC3: return (T)mat.at<cv::Vec3f>(height, width)[channels];
    }
    throw std::runtime_error("cv::Mat type is not recognized");
}

static void matToTensor(const cv::Mat& mat, const ov::Tensor& tensor, int batchIndex )
{
    ov::Shape tensorShape = tensor.get_shape();
    static const ov::Layout layout("NCHW");
    const size_t width = tensorShape[ov::layout::width_idx(layout)];
    const size_t height = tensorShape[ov::layout::height_idx(layout)];
    const size_t channels = tensorShape[ov::layout::channels_idx(layout)];

    if (channels != 1 && channels != 3)
        throw std::runtime_error("unsuported number of channels");

    int batchOffset = batchIndex * width * height * channels;
    cv::Mat resizedMat;
    if (static_cast<int>(width) != mat.size().width || static_cast<int>(height) != mat.size().height)
        cv::resize(mat, resizedMat, cv::Size(width, height));
    else
        resizedMat = mat;

    if (tensor.get_element_type() == ov::element::f32)
    {
        float_t* tensor_data = tensor.data<float_t>();
        for (size_t c = 0; c < channels; c++)
            for (size_t h = 0; h < height; h++)
                for (size_t w = 0; w < width; w++)
                    tensor_data[batchOffset + c * width * height + h * width + w] = getMatValue<float_t>(resizedMat, h, w, c);
    }
    else
    {
        uint8_t* tensor_data = tensor.data<uint8_t>();
        if (resizedMat.depth() == CV_32F)
            throw std::runtime_error("Mat conversion from float_t to uin8_t is forbidden");
        for(size_t c = 0; c < channels; c++)
            for(size_t h = 0; h < height; h++)
                for(size_t w = 0; w < width; w++)
                    tensor_data[batchOffset + c * width * height + h * width + w] = getMatValue<float_t>(resizedMat, h, w, c);
    }
}