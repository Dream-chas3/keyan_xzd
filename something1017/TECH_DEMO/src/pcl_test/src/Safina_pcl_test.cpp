#include "pcl_test/Safina_pcl_test.h"
typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;
// 体素 key
struct VoxelKey3D {
    int x, y, z;
    bool operator==(const VoxelKey3D& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct VoxelKey3DHash {
    std::size_t operator()(const VoxelKey3D& k) const {
        // 简单哈希组合
        return ((std::size_t)k.x * 73856093) ^
                ((std::size_t)k.y * 19349663) ^
                ((std::size_t)k.z * 83492791);
    }
};
Safina_PointCloud::Safina_PointCloud(ros::NodeHandle node_pointcloud) : node_pointcloud(node_pointcloud)
{

    initializeSubscribers();
    initializePublishers();

    //相机相关 
    node_pointcloud.param<int>("camera_rows", img_rows, 480);
    node_pointcloud.param<int>("camera_cols", img_cols, 640);


    node_pointcloud.param<float>("downplane", downPlane, 1.5);
    node_pointcloud.param<float>("upplane", upPlane, 1.5);
    node_pointcloud.param<float>("depth_scale", depthScale, 1000.0);  
    node_pointcloud.param<int>("downSampleSacle", downSampleSacle, 3);

    camera_matrix_ = (cv::Mat_<double>(3,3) <<
        369.8394470214844, 0.0,                319.1000061035156,
        0.0,               369.8394470214844,  238.5,
        0.0,               0.0,                1.0);

    // 畸变系数（D，前 5 个，都是 0）
    dist_coeffs_ = (cv::Mat_<double>(1,5) << 0.0, 0.0, 0.0, 0.0, 0.0);



}
void Safina_PointCloud::initializeSubscribers()
{
    node_pointcloud.param<std::string>("depimg_topic_", depimg_topic_, "/camera/depth/image_raw");
    sub_depimg=node_pointcloud.subscribe(depimg_topic_,1, &Safina_PointCloud::readStream,this);
}

void Safina_PointCloud::initializePublishers()
{
    pub_pointcloud_orin = node_pointcloud.advertise<sensor_msgs::PointCloud2>("/Safina_pointcloud_orin", 10);

    pub_pointcloud_rad = node_pointcloud.advertise<sensor_msgs::PointCloud2>("/Safina_pointcloud_rad", 10);

    pub_pointcloud_voxel = node_pointcloud.advertise<sensor_msgs::PointCloud2>("/Safina_pointcloud_voxel", 10);

    pub_pointcloud_voxel3d = node_pointcloud.advertise<sensor_msgs::PointCloud2>("/Safina_pointcloud_voxel3d", 10);

}
Safina_PointCloud::~Safina_PointCloud(){}


// 回调，提交任务
void Safina_PointCloud::readStream(const sensor_msgs::ImageConstPtr &img_msg)
{

    unsigned short *depth_data = (unsigned short*)&img_msg->data[0];
    

    PointCloud::Ptr orin(new PointCloud);

    float x,y,z;
    for(int uy=0; uy<img_rows; uy=uy+downSampleSacle)
    {
        for(int ux=0; ux<img_cols; ux=ux+downSampleSacle+2)
        {   
            z = *(depth_data + uy*img_cols + ux) / depthScale;
            if(z!=0){
                x = z * (ux - camera_matrix_.at<double>(0,2)) / camera_matrix_.at<double>(0,0);
                y = z * (uy - camera_matrix_.at<double>(1,2)) / camera_matrix_.at<double>(1,1);
                if(y>downPlane||y<-upPlane)
                    continue;
                pcl::PointXYZ p(x, y, z);
                orin->push_back(p);
            }
        }
    }
    // ---------------- 原始点云 ----------------
    sensor_msgs::PointCloud2 pointcloud_orin;
    pcl::toROSMsg(*orin, pointcloud_orin);
    pointcloud_orin.header.frame_id = "camera_link";
    pub_pointcloud_orin.publish(pointcloud_orin);

    // ---------------- VoxelGrid：几何体素下采样 ----------------
    PointCloud::Ptr voxel_ds(new PointCloud);
    pcl::VoxelGrid<pcl::PointXYZ> voxel_filter;
    voxel_filter.setInputCloud(orin);
    voxel_filter.setLeafSize(0.01f, 0.01f, 0.01f);   
    voxel_filter.filter(*voxel_ds);

    sensor_msgs::PointCloud2 pointcloud_voxel;
    pcl::toROSMsg(*voxel_ds, pointcloud_voxel);
    pointcloud_voxel.header = pointcloud_orin.header;
    pub_pointcloud_voxel.publish(pointcloud_voxel);

	// ---------------- RadiusOutlierRemoval：半径离群点滤波 ----------------
    PointCloud::Ptr rad_ds(new PointCloud);
    pcl::RadiusOutlierRemoval<pcl::PointXYZ> outrem;
    outrem.setInputCloud(orin);
    outrem.setRadiusSearch(0.3);
    outrem.setMinNeighborsInRadius(50);
    outrem.filter(*rad_ds);

    sensor_msgs::PointCloud2 pointcloud_rad;
    pcl::toROSMsg(*rad_ds, pointcloud_rad);
    pointcloud_rad.header.frame_id = "camera_link";
    pub_pointcloud_rad.publish(pointcloud_rad);


    // ---------------- 3D 体素栅格 ----------------
    // 体素分辨率
    const float voxel_res = 0.1f; 
    // hash 表：key 为体素索引 (ix,iy,iz)，value 为该体素内点数
    std::unordered_map<VoxelKey3D, int, VoxelKey3DHash> voxel_3d_map;

    // 遍历原始点云，将点映射到体素索引
    for (const auto& p : orin->points) {
        int vx = static_cast<int>(std::floor(p.x / voxel_res));
        int vy = static_cast<int>(std::floor(p.y / voxel_res));
        int vz = static_cast<int>(std::floor(p.z / voxel_res));

        VoxelKey3D key{vx, vy, vz};
        voxel_3d_map[key] += 1;  
    }

    // 把每个占据体素的中心点输出为点云
    PointCloud::Ptr voxel3d_cloud(new PointCloud);
    voxel3d_cloud->reserve(voxel_3d_map.size());

    for (const auto& kv : voxel_3d_map) {
        const VoxelKey3D& k = kv.first;
        pcl::PointXYZ pt;
        pt.x = (k.x + 0.5f) * voxel_res;
        pt.y = (k.y + 0.5f) * voxel_res;
        pt.z = (k.z + 0.5f) * voxel_res;
        voxel3d_cloud->push_back(pt);
    }

    sensor_msgs::PointCloud2 pointcloud_voxel3d;
    pcl::toROSMsg(*voxel3d_cloud, pointcloud_voxel3d);
    pointcloud_voxel3d.header.frame_id = "camera_link";
    pub_pointcloud_voxel3d.publish(pointcloud_voxel3d);


}

int main(int argc, char **argv)
{

    ros::init(argc, argv, "pointcloud");
    ros::NodeHandle node_pointcloud;

    Safina_PointCloud safina_pointcloud(node_pointcloud) ;

    ROS_INFO("pointcloud_test runing ...");

    ros::spin();
    return 0;
}