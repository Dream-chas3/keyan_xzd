#ifndef SAFINA_PCL_TEST_H
#define SAFINA_PCL_TEST_H

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>

#include <opencv2/opencv.hpp>
#include <iostream>

#include <unordered_map>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl_conversions/pcl_conversions.h>


class Safina_PointCloud
{
public:
    Safina_PointCloud(ros::NodeHandle node_pointcloud);
    ~Safina_PointCloud();


    int img_rows;
    int img_cols;


    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;

    float depthScale;
    float downPlane;
    float upPlane;
    int downSampleSacle;



private:
    ros::NodeHandle node_pointcloud;

    ros::Subscriber sub_depimg;
    std::string depimg_topic_;
    //发布pointcloud
    ros::Publisher pub_pointcloud_orin;
    ros::Publisher pub_pointcloud_rad;
    ros::Publisher pub_pointcloud_voxel;
    ros::Publisher pub_pointcloud_voxel3d;

    void initializeSubscribers();
    void initializePublishers();
    void readStream(const sensor_msgs::ImageConstPtr &img_msg);


};

#endif  