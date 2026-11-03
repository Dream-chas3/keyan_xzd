#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

class DrawRectNode
{
public:
    DrawRectNode()
    {
        ros::NodeHandle nh("~");

        // 读取矩形参数（可配置）
        nh.param<int>("rect_x", rect_x_, 700);
        nh.param<int>("rect_y", rect_y_, 400);
        nh.param<int>("rect_width", rect_width_, 500);
        nh.param<int>("rect_height", rect_height_, 500);
        nh.param<int>("r", r_, 255);
        nh.param<int>("g", g_, 0);
        nh.param<int>("b", b_, 0);
        nh.param<int>("thickness", thickness_, 3);

        // 订阅图像话题
        sub_ = nh.subscribe("/rtsp_cam/image_raw", 1, &DrawRectNode::imageCallback, this);
        // 发布带框图像
        pub_ = nh.advertise<sensor_msgs::Image>("/rtsp_cam/rtsp_draw", 1);
    }

private:
    void imageCallback(const sensor_msgs::ImageConstPtr& msg)
    {
        // 将 ROS 消息转换为 OpenCV 图像
        cv_bridge::CvImagePtr cv_ptr;
        try {
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        } catch (cv_bridge::Exception& e) {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }

        // 绘制矩形
        cv::rectangle(cv_ptr->image,
                      cv::Point(rect_x_, rect_y_),
                      cv::Point(rect_x_ + rect_width_, rect_y_ + rect_height_),
                      cv::Scalar(b_, g_, r_),  // OpenCV 使用 BGR 顺序
                      thickness_);

        // 发布出去
        pub_.publish(cv_ptr->toImageMsg());
    }

    ros::Subscriber sub_;
    ros::Publisher pub_;
    int rect_x_, rect_y_, rect_width_, rect_height_;
    int r_, g_, b_, thickness_;
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "rtsp_draw");
    DrawRectNode node;
    ROS_INFO("rtsp_draw running ...");
    ros::spin();
    return 0;
}