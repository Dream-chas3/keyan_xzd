#include "yoloUltra_custom.h"
#include <random>
#include "utils/logging.h"
#include "process/preprocess.h"
#include "process/postprocess.h"

// define global classes
static std::vector<std::string> g_classes = {
    "person", "bicycle", "car", "motorbike ", "aeroplane ", "bus ", "train", "truck ", "boat", "traffic light",
    "fire hydrant", "stop sign ", "parking meter", "bench", "bird", "cat", "dog ", "horse ", "sheep", "cow", "elephant",
    "bear", "zebra ", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite",
    "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife ",
    "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza ", "donut", "cake", "chair", "sofa",
    "pottedplant", "bed", "diningtable", "toilet ", "tvmonitor", "laptop	", "mouse	", "remote ", "keyboard ", "cell phone", "microwave ",
    "oven ", "toaster", "sink", "refrigerator ", "book", "clock", "vase", "scissors ", "teddy bear ", "hair drier", "toothbrush "};
// static std::vector<std::string> g_classes = {
// "pedestrians", "riders", "partially-visible-person", "ignore-regions", "crowd"};
// static std::vector<std::string> g_classes = {"pedestrian", "pedestrian", "bicycle", "car", "van","truck", "tricycle", "awning-tricycle", "bus", "motor"};

// static std::vector<std::string> g_classes = {"fixed-wing-aircraft", "small-aircraft", "cargo-plane", "helicopter", "passenger-vehicle", "small-car", "bus", "pickup-truck", "utility-truck", "truck", "cargo-truck", "truck-w-box", "truck-tractor", "trailer", "truck-w-flatbed", "truck-w-liquid", "crane-truck", "railway-vehicle", "passenger-car", "cargo-car", "flat-car", "tank-car", "locomotive", "maritime-vessel", "motorboat", "sailboat", "tugboat", "barge", "fishing-vessel", "ferry", "yacht", "container-ship", "oil-tanker", "engineering-vehicle", "tower-crane", "container-crane", "reach-stacker", "straddle-carrier", "mobile-crane", "dump-truck", "haul-truck", "scraper-tractor", "front-loader-bulldozer", "excavator", "cement-mixer", "ground-grader", "hut-tent", "shed", "building", "aircraft-hangar", "damaged-building", "facility", "construction-site", "vehicle-lot", "helipad", "storage-tank", "shipping-container-lot", "shipping-container", "pylon", "tower"};


YoloUltraCustom::YoloUltraCustom()
{
    engine_ = CreateRKNNEngine();
    input_tensor_.data = nullptr;
    want_float_ = false; // 是否使用浮点数版本的后处理
    ready_ = false;
}

YoloUltraCustom::~YoloUltraCustom()
{
    // release input tensor and output tensor
    NN_LOG_DEBUG("release input tensor");
    if (input_tensor_.data != nullptr)
    {
        free(input_tensor_.data);
        input_tensor_.data = nullptr;
    }
    NN_LOG_DEBUG("release output tensor");
    for (auto &tensor : output_tensors_)
    {
        if (tensor.data != nullptr)
        {
            free(tensor.data);
            tensor.data = nullptr;
        }
    }
}

nn_error_e YoloUltraCustom::LoadModel(const char *model_path)
{
    auto ret = engine_->LoadModelFile(model_path);
    if (ret != NN_SUCCESS)
    {
        NN_LOG_ERROR("yoloUltra load model file failed");
        return ret;
    }
    // get input tensor
    auto input_shapes = engine_->GetInputShapes();

    // check number of input and n_dims
    if (input_shapes.size() != 1)
    {
        NN_LOG_ERROR("yoloUltra input tensor number is not 1, but %ld", input_shapes.size());
        return NN_RKNN_INPUT_ATTR_ERROR;
    }
    nn_tensor_attr_to_cvimg_input_data(input_shapes[0], input_tensor_);
    input_tensor_.data = malloc(input_tensor_.attr.size);

    auto output_shapes = engine_->GetOutputShapes();
    if (output_shapes.size() != 6)
    {
        NN_LOG_ERROR("yoloUltra output tensor number is not 6, but %ld", output_shapes.size());
        return NN_RKNN_OUTPUT_ATTR_ERROR;
    }
    if (output_shapes[0].type == NN_TENSOR_FLOAT16)
    {
        want_float_ = true;
        NN_LOG_WARNING("yoloUltra output tensor type is float16, want type set to float32");
    }
    for (int i = 0; i < output_shapes.size(); i++)
    {
        tensor_data_s tensor;
        tensor.attr.n_elems = output_shapes[i].n_elems;
        tensor.attr.n_dims = output_shapes[i].n_dims;
        for (int j = 0; j < output_shapes[i].n_dims; j++)
        {
            tensor.attr.dims[j] = output_shapes[i].dims[j];
        }
        // output tensor needs to be float32
        tensor.attr.type = want_float_ ? NN_TENSOR_FLOAT : output_shapes[i].type;
        tensor.attr.index = 0;
        tensor.attr.size = output_shapes[i].n_elems * nn_tensor_type_to_size(tensor.attr.type);
        tensor.data = malloc(tensor.attr.size);
        output_tensors_.push_back(tensor);
        out_zps_.push_back(output_shapes[i].zp);
        out_scales_.push_back(output_shapes[i].scale);
    }

    ready_ = true;
    return NN_SUCCESS;
}

nn_error_e YoloUltraCustom::Preprocess(const cv::Mat &img, const std::string process_type, cv::Mat &image_letterbox)
{



    float wh_ratio = (float)input_tensor_.attr.dims[2] / (float)input_tensor_.attr.dims[1];

    // lettorbox

    // if (process_type == "opencv")
    // {
    // BGR2RGB，resize，再放入input_tensor_中
    letterbox_info_ = letterbox(img, image_letterbox, wh_ratio);
    cvimg2tensor(image_letterbox, input_tensor_.attr.dims[2], input_tensor_.attr.dims[1], input_tensor_);
    // }
    // else if (process_type == "rga")
    // {
    //     // rga resize
    //     letterbox_info_ = letterbox_rga(img, image_letterbox, wh_ratio);
    //     // save img
    //     // cv::imwrite("rga.jpg", image_letterbox);
    //     cvimg2tensor_rga(image_letterbox, input_tensor_.attr.dims[2], input_tensor_.attr.dims[1], input_tensor_);
    // }
   

    return NN_SUCCESS;
}

nn_error_e YoloUltraCustom::Inference()
{
    std::vector<tensor_data_s> inputs;
    inputs.push_back(input_tensor_);
    return engine_->Run(inputs, output_tensors_, want_float_);
}

nn_error_e YoloUltraCustom::Postprocess(const cv::Mat &img, std::vector<Detection> &objects)
{
    void *output_data[6];
    for (int i = 0; i < 6; i++)
    {
        output_data[i] = (void *)output_tensors_[i].data;
    }
    std::vector<float> DetectiontRects;
    if (want_float_)
    {
        // 使用浮点数版本的后处理，他也支持量化的模型
        yolo::GetConvDetectionResult((float **)output_data, DetectiontRects);
        // NN_LOG_INFO("use float version postprocess");
    }
    else
    {
        // 使用量化版本的后处理，只能处理量化的模型
        yolo::GetConvDetectionResultInt8((int8_t **)output_data, out_zps_, out_scales_, DetectiontRects);
        // NN_LOG_INFO("use int8 version postprocess");
    }

    int img_width = img.cols;
    int img_height = img.rows;
    for (int i = 0; i < DetectiontRects.size(); i += 6)
    {
        int classId = int(DetectiontRects[i + 0]);
        float conf = DetectiontRects[i + 1];
        int xmin = int(DetectiontRects[i + 2] * float(img_width) + 0.5);
        int ymin = int(DetectiontRects[i + 3] * float(img_height) + 0.5);
        int xmax = int(DetectiontRects[i + 4] * float(img_width) + 0.5);
        int ymax = int(DetectiontRects[i + 5] * float(img_height) + 0.5);
        Detection result;
        result.class_id = classId;
        result.confidence = conf;

        // std::random_device rd;
        // std::mt19937 gen(rd());
        // std::uniform_int_distribution<int> dis(100, 255);
        // result.color = cv::Scalar(dis(gen),
        //                           dis(gen),
        //                           dis(gen));

        result.className = g_classes[result.class_id];
        result.box = cv::Rect(xmin, ymin, xmax - xmin, ymax - ymin);
        if(result.class_id==0)
            objects.push_back(result);
    }


    return NN_SUCCESS;
}
void letterbox_decode(std::vector<Detection> &objects, bool hor, int pad)
{
    for (auto &obj : objects)
    {
        if (hor)
        {
            obj.box.x -= pad;
        }
        else
        {
            obj.box.y -= pad;
        }
    }
}

nn_error_e YoloUltraCustom::Run(const cv::Mat &img, std::vector<Detection> &objects)
{

    // letterbox后的图像
    cv::Mat image_letterbox;
    // 预处理，支持opencv或rga
    Preprocess(img, "opencv", image_letterbox);
    // Preprocess(img, "rga", image_letterbox);
    // 推理
    Inference();
    // 后处理
    Postprocess(image_letterbox, objects);

    letterbox_decode(objects, letterbox_info_.hor, letterbox_info_.pad);

    return NN_SUCCESS;
}