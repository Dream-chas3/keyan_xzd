#include <ros/ros.h>
#include <message_filters/subscriber.h>

#include <std_msgs/String.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#include <opencv2/aruco.hpp>
#include <opencv2/calib3d/calib3d.hpp>

ros::Subscriber sub_img;
ros::Publisher pub_aruco_imgresult;

cv::Ptr<cv::aruco::Dictionary> dictionary_ori = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
cv::Ptr<cv::aruco::DetectorParameters> detector_params_ = cv::aruco::DetectorParameters::create();

// 相机内参矩阵赋值（来自 K）
cv::Mat camera_matrix_ = (cv::Mat_<double>(3,3) <<
    2035.5320457932803, 0.0,                 945.1582202655426,
    0.0,                2034.0254504998773, 515.2610762930509,
    0.0,                0.0,                1.0);

// 畸变系数赋值（来自 D）
cv::Mat dist_coeffs_ = (cv::Mat_<double>(1,5) <<
    -0.4780459609215665,
    0.3408621567942472,
    5.7553918012774816e-05,
    -0.0005717031580970413,
    0.0);

// 回调，提交任务
void readStream(const sensor_msgs::ImageConstPtr &img_msg)
{

    cv::Mat img= cv_bridge::toCvShare(img_msg, "bgr8")->image;


    tf2::Quaternion q;
    cv::Mat gray_img;
    cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

    std::vector<int> ids_ori;
    std::vector<std::vector<cv::Point2f>> corners_ori;
    cv::aruco::detectMarkers(gray_img, dictionary_ori, corners_ori, ids_ori, detector_params_);

    std::vector<cv::Vec3d> rvecs_ori, tvecs_ori;
    cv::aruco::estimatePoseSingleMarkers(corners_ori, 0.035, camera_matrix_, dist_coeffs_, rvecs_ori, tvecs_ori);

    if (!ids_ori.empty()) {


        for (size_t i = 0; i < corners_ori.size(); ++i) {

            cv::Point2f center(0, 0);
            for (const auto& c : corners_ori[i]) center += c;
            center *= 0.25f;
            cv::circle(img, center, 10, cv::Scalar(0, 255, 255), 10);

            cv::aruco::drawAxis(img, camera_matrix_, dist_coeffs_,rvecs_ori[i], tvecs_ori[i],0.03);  // axis length [m]
        }

        cv::aruco::drawDetectedMarkers(img, corners_ori, ids_ori);

    }
    pub_aruco_imgresult.publish(cv_bridge::CvImage(std_msgs::Header(), "bgr8", img).toImageMsg());

}

int main(int argc, char **argv)
{

    ros::init(argc, argv, "aruco");
    ros::NodeHandle node_ori_aruco;

    sub_img = node_ori_aruco.subscribe("/rtsp_cam/image_raw", 1, readStream);
    pub_aruco_imgresult = node_ori_aruco.advertise<sensor_msgs::Image>("/aruco_imgresult", 1);
    ROS_INFO("ori_aruco runing ...");

    ros::spin();
    return 0;
}
