#ifndef SAFINA_MAP_TEST_H
#define SAFINA_MAP_TEST_H
#include <ros/ros.h>
#include <Eigen/Eigen>
#include <Eigen/StdVector>
#include <queue>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <vector>
#include <cmath>
#include <iostream>


int signum(int x) {
    return x == 0 ? 0 : x < 0 ? -1 : 1;
}

double mod(double value, double modulus) {
    return fmod(fmod(value, modulus) + modulus, modulus);
}

double intbound(double s, double ds) {

    if (ds < 0) {
        return intbound(-s, -ds);
    } else {
        s = mod(s, 1);
        return (1 - s) / ds;
    }
}

class RayCaster {
private:
    /* data */
    Eigen::Vector3d start_;
    Eigen::Vector3d end_;
    Eigen::Vector3d direction_;
    Eigen::Vector3d min_;
    Eigen::Vector3d max_;
    int x_;
    int y_;
    int z_;
    int endX_;
    int endY_;
    int endZ_;
    double maxDist_;
    double dx_;
    double dy_;
    double dz_;
    int stepX_;
    int stepY_;
    int stepZ_;
    double tMaxX_;
    double tMaxY_;
    double tMaxZ_;
    double tDeltaX_;
    double tDeltaY_;
    double tDeltaZ_;
    double dist_;

    int step_num_;

public:
    RayCaster() {
    }
    ~RayCaster() {
    }

    bool setInput(const Eigen::Vector3d& start,
                const Eigen::Vector3d& end /* , const Eigen::Vector3d& min,
                const Eigen::Vector3d& max */)
    {
        start_ = start;
        end_ = end;


        x_ = (int)std::floor(start_.x());
        y_ = (int)std::floor(start_.y());
        z_ = (int)std::floor(start_.z());
        endX_ = (int)std::floor(end_.x());
        endY_ = (int)std::floor(end_.y());
        endZ_ = (int)std::floor(end_.z());
        direction_ = (end_ - start_);
        maxDist_ = direction_.squaredNorm();


        dx_ = endX_ - x_;
        dy_ = endY_ - y_;
        dz_ = endZ_ - z_;

        stepX_ = (int)signum((int)dx_);
        stepY_ = (int)signum((int)dy_);
        stepZ_ = (int)signum((int)dz_);

        tMaxX_ = intbound(start_.x(), dx_);
        tMaxY_ = intbound(start_.y(), dy_);
        tMaxZ_ = intbound(start_.z(), dz_);


        tDeltaX_ = ((double)stepX_) / dx_;
        tDeltaY_ = ((double)stepY_) / dy_;
        tDeltaZ_ = ((double)stepZ_) / dz_;

        dist_ = 0;
        step_num_ = 0;


        if (stepX_ == 0 && stepY_ == 0 && stepZ_ == 0) return false;
        else return true;
    }

    bool step(Eigen::Vector3d& ray_pt)
    {

        ray_pt = Eigen::Vector3d(x_, y_, z_);

        if (x_ == endX_ && y_ == endY_ && z_ == endZ_) {
            return false;
        }

        if (tMaxX_ < tMaxY_) {
            if (tMaxX_ < tMaxZ_) {
                x_ += stepX_;
                tMaxX_ += tDeltaX_;
            } else {
                z_ += stepZ_;
                tMaxZ_ += tDeltaZ_;
            }
        } 
        else {
            if (tMaxY_ < tMaxZ_) {
                y_ += stepY_;
                tMaxY_ += tDeltaY_;
            } else {
                z_ += stepZ_;
                tMaxZ_ += tDeltaZ_;
            }
        }
        return true;
    }
};


using std::cout; using std::endl;

class occMap{
	private:

	protected:
		std::string ns_;

		ros::NodeHandle node_land_;

        // 点云订阅器（message_filters 封装，支持与里程计同步）
		std::shared_ptr<message_filters::Subscriber<sensor_msgs::PointCloud2>> pointcloudSub_;
        // 里程计订阅器（与点云同步，用于得到姿态/位姿）
		std::shared_ptr<message_filters::Subscriber<nav_msgs::Odometry>> odomSub_;
        // 点云 + 里程计的近似时间同步策略
		typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::PointCloud2, nav_msgs::Odometry> pointcloudOdomSync;
        // 同步器对象（回调里同时拿到 pointcloud 和 odom）
		std::shared_ptr<message_filters::Synchronizer<pointcloudOdomSync>> pointcloudOdomSync_;	

        // 定时器：占据地图更新（raycast + 概率更新）
        ros::Timer occTimer_;
        // 定时器：地图膨胀（根据机器人尺寸做安全距离扩展）
        ros::Timer inflateTimer_;
        // 定时器：可视化发布（投影点云 / 地图 / 膨胀地图）
        ros::Timer visTimer_;

        // 发布投影点云（调试用）
        ros::Publisher depthCloudPub_;
        // 发布占据地图（占据体素点云）
        ros::Publisher mapVisPub_;
        // 发布膨胀地图
        ros::Publisher inflatedMapVisPub_;

        // ---------------- 参数区（从 rosparam 读取或用默认值） ----------------
		std::string pointcloudTopicName_= "/cloud_registered_body"; 
		std::string odomTopicName_= "/mavros/local_position/odom"; 

		Eigen::Vector3d robotSize_;

		Eigen::Matrix4d body2Cam_; 
		
        // 射线最大长度（更新占据时的最大感知距离）
		double raycastMaxLength_ = 5.0;
        // 占据 / 释放 / 上下限 / 判定占据 的 logit 概率
		double pHitLog_ = this->logit(0.70);
		double pMissLog_ = this->logit(0.35);
		double pMinLog_ = this->logit(0.12);
		double pMaxLog_ = this->logit(0.97);
		double pOccLog_ = this->logit(0.80);

		// ----地图相关 ----
        // 用于标记 unknown 初值时偏移的一个小量（让 unknown < pMinLog_）
		double UNKNOWN_FLAG_ = 0.01;
        // 地图分辨率（体素大小，单位 m）
		double mapRes_=0.1;
        // 地面高度（z 最低值，单位 m）
		double groundHeight_=-0.1;
        // 地图在世界坐标下的物理范围 [min, max]（单位 m）
		Eigen::Vector3d mapSizeMin_, mapSizeMax_; 
        // 地图在体素索引空间的范围 [min, max)（体素索引）
		Eigen::Vector3i mapVoxelMin_, mapVoxelMax_; 
        // 局部更新范围（以当前位置为中心的 x,y,z 范围，单位 m）
		Eigen::Vector3d localUpdateRange_; 
        // 在计算 localBound 时，在 x,y 平面上再额外膨胀一些（m）
		double localBoundInflate_=0.5; 


		// ----可视相关 ----
        // 可视化最高高度（z 超过该高度的体素不画出来）
		double maxVisHeight_=1.5;
        // 局部可视化地图的半尺寸（x,y,z 的一半长度，单位 m）
		Eigen::Vector3d localMapSize_;
		bool visGlobalMap_=true;
		bool verbose_=true;

        // ----运行时状态与数据缓存 ----
        // 原始点云（pcl 格式，单位 m，传感器坐标转为 map 坐标后用）
        pcl::PointCloud<pcl::PointXYZ> pointcloud_;
        // 当前相机/传感器在地图中的位置（map 坐标）
        Eigen::Vector3d position_; 
        // 当前相机/传感器朝向（R_map^cam，3x3 旋转矩阵）
        Eigen::Matrix3d orientation_; 
        // 最近一次 raycast 更新区域的局部边界（体素索引）
        Eigen::Vector3i localBoundMin_, localBoundMax_; 


        // 当前投影点数量（getPointcloud 后的点数）
        int projPointsNum_ = 0;
        // 投影到地图坐标系下的 3D 点集合（用于 raycast）
        std::vector<Eigen::Vector3d> projPoints_;

        // 每个体素：本轮观测命中+空闲计数（hit+miss 总数）
        std::vector<int> countHitMiss_;
        // 每个体素：本轮观测命中计数（hit 次数）
        std::vector<int> countHit_;
        // 本轮需要更新的体素队列（只 push 第一次被观测到的体素索引）
        std::queue<Eigen::Vector3i> updateVoxelCache_;
        // 每个体素的占据 logit 值（用于概率更新和阈值判定）
        std::vector<double> occupancy_; 
        // 每个体素是否在「膨胀后的占据集合」中（true 表示被膨胀占据）
        std::vector<bool> occupancyInflated_; 
        // 射线遍历次数计数器（每次 raycastUpdate 自增，用于帧内标记）
        int raycastNum_ = 0; 
        // 对每个体素，记录它在当前 raycastNum_ 是否已被 ray 穿过（去重用）
        std::vector<int> flagTraverse_;
        // 对每个体素，记录它在当前 raycastNum_ 是否被当作 ray 终点（去重用）
        std::vector<int> flagRayend_;
        // 当前已知地图的最小/最大坐标范围（根据已更新非 unknown 体素动态扩展）
        Eigen::Vector3d currMapRangeMin_ = Eigen::Vector3d (0, 0, 0); 
        Eigen::Vector3d currMapRangeMax_ = Eigen::Vector3d (0, 0, 0);

        // 标记：是否需要进行一次占据更新（由同步回调设置）
        bool occNeedUpdate_ = false;
        // 标记：是否需要进行一次地图膨胀（由占据更新完成后设置）
        bool mapNeedInflate_ = false;

        // 射线步进工具（从起点到终点遍历经过的体素 / 栅格）
        RayCaster raycaster_;


	public:

        //构造函数
		occMap(); 
		occMap(const ros::NodeHandle& nh);
		virtual ~occMap() = default;
		// void initMap(const ros::NodeHandle& nh);
        // 从config读取参数
		void initParam();
        // 注册订阅与回调：
        // - 点云 + 里程计同步回调
        // - 占据更新定时器回调
        // - 膨胀定时器回调（判定是否需要膨胀，调用膨胀）
        // - 可视化三种地图发布

		void registerCallback();
        // 发布三种地图  1.投影点云 2.占据地图点云 3.膨胀占据地图点云

		void registerPub();

        // 点云 + 里程计同步回调：
        // - 将点云转为 PCL 格式
        // - 根据里程计 + body2Cam 计算相机在 map 坐标下的位姿
        // - 若相机在地图范围内，则标记需要一次占据更新
		void pointcloudOdomCB(const sensor_msgs::PointCloud2ConstPtr& pointcloud, const nav_msgs::OdometryConstPtr& odom);

        // 占据更新定时器回调：
        // - 若 occNeedUpdate_ ：
        //   - 处理点云（getPointcloud）
        //   - 进行射线更新（raycastUpdate）
        //   - 置 mapNeedInflate_ = true，膨胀回调
		void updateOccupancyCB(const ros::TimerEvent& );

        // 地图膨胀定时器回调：
        // - 若 mapNeedInflate_ ，执行 inflateLocalMap()膨胀
		void inflateMapCB(const ros::TimerEvent& );


        // ------------ 建图相关核心函数 ------------------

        // 将点云从相机坐标系变换到地图坐标系：
        // - 记录为 projPoints_（过滤掉离相机太近的点）
        void getPointcloud();

        // 对 projPoints_ 内的点进行射线更新：
        // - 将终点体素视为 hit / miss
        // - 对射线经过的体素视为 miss
        // - 统计每个体素的 hit / miss 次数，并在本轮结束时更新 occupancy_
        // - 更新当前局部边界 localBoundMin_/Max_ 与 currMapRangeMin_/Max_
        void raycastUpdate();

        // 在局部边界内对占据体素进行空间膨胀：
        // - 按机器人尺寸在栅格中扩张占据区域
        // - 更新 occupancyInflated_ 标记
        void inflateLocalMap();


        // 根据（世界坐标、体素索引）查询该位置在占据地图中是否被占据
		bool isOccupied(const Eigen::Vector3d& pos);
		bool isOccupied(const Eigen::Vector3i& idx);
        // 根据（世界坐标、体素索引）查询该位置在膨胀地图中是否被占据
		bool isInflatedOccupied(const Eigen::Vector3d& pos);
		bool isInflatedOccupied(const Eigen::Vector3i& idx);
        // 根据（世界坐标、体素索引）查询该位置是否为未知区域
		bool isUnknown(const Eigen::Vector3d& pos);
		bool isUnknown(const Eigen::Vector3i& idx);

        //  --------可视化相关 --------
        void visCB(const ros::TimerEvent&);

        // 发布投影点云
        void publishProjPoints();

        // 发布占据地图
        void publishMap();

        // 发布膨胀地图
        void publishInflatedMap();


		double logit(double x);
        // 根据（世界坐标、体素索引）查询该位置在地图范围内
		bool isInMap(const Eigen::Vector3d& pos);
		bool isInMap(const Eigen::Vector3i& idx);
        // 世界坐标 → 体素索引（根据 mapSizeMin_ 和 mapRes_）
        void posToIndex(const Eigen::Vector3d& pos, Eigen::Vector3i& idx);
        // 体素索引 → 世界坐标（返回体素中心点坐标）
        void indexToPos(const Eigen::Vector3i& idx, Eigen::Vector3d& pos);
        // 世界坐标 → 一维数组地址（先转索引，再用 indexToAddress）
		int posToAddress(const Eigen::Vector3d& idx);
		int posToAddress(double x, double y, double z);
        // 体素索引 → 一维数组地址（x,y,z 线性映射到 occupancy_ 下标）
		int indexToAddress(const Eigen::Vector3i& idx);
		int indexToAddress(int x, int y, int z);
        // 将索引 idx 限制在合法范围 [mapVoxelMin_, mapVoxelMax_-1] 内
		void boundIndex(Eigen::Vector3i& idx);
        // 判断世界坐标 pos 是否在局部更新范围内（position_ ± localUpdateRange_）
		bool isInLocalUpdateRange(const Eigen::Vector3d& pos);
        // 判断体素索引 idx 是否在局部更新范围内
		bool isInLocalUpdateRange(const Eigen::Vector3i& idx);

        // 射线裁剪 / 占据缓存更新 
        // 当点超出地图范围时，将点沿着 sensor→point 方向裁剪回地图边界内
		Eigen::Vector3d adjustPointInMap(const Eigen::Vector3d& point);
        // 当射线长度超过最大感知距离时，将点沿着射线方向裁剪到最大长度处
		Eigen::Vector3d adjustPointRayLength(const Eigen::Vector3d& point);
        // 更新单个体素的 hit/miss 统计：
        // - 根据 point 得到其体素索引
        // - countHitMiss_[voxel]++，若 isOccupied 为真则 countHit_[voxel]++
        // - 首次访问的体素压入 updateVoxelCache_
        // 返回该体素的一维地址（voxelID）
		int updateOccupancyInfo(const Eigen::Vector3d& point, bool isOccupied);

        // 位姿转换
		void getCameraPose(const nav_msgs::OdometryConstPtr& odom, Eigen::Matrix4d& camPoseMatrix);
};
inline bool occMap::isOccupied(const Eigen::Vector3d& pos){
    Eigen::Vector3i idx;
    this->posToIndex(pos, idx);
    return this->isOccupied(idx);
}

inline bool occMap::isOccupied(const Eigen::Vector3i& idx){
    if (not this->isInMap(idx)){
        return true;
    }
    int address = this->indexToAddress(idx);
    return this->occupancy_[address] >= this->pOccLog_;
}

inline bool occMap::isInflatedOccupied(const Eigen::Vector3d& pos){
    Eigen::Vector3i idx;
    this->posToIndex(pos, idx);
    return this->isInflatedOccupied(idx);
}

inline bool occMap::isInflatedOccupied(const Eigen::Vector3i& idx){
    if (not this->isInMap(idx)){
        return true;
    }
    int address = this->indexToAddress(idx);
    return this->occupancyInflated_[address] == true;
}



inline bool occMap::isUnknown(const Eigen::Vector3d& pos){
    Eigen::Vector3i idx;
    this->posToIndex(pos, idx);
    return this->isUnknown(idx);
}

inline bool occMap::isUnknown(const Eigen::Vector3i& idx){
    if (not this->isInMap(idx)){
        return true;
    }
    int address = this->indexToAddress(idx);
    return this->occupancy_[address] < this->pMinLog_;		
}




inline double occMap::logit(double x){
    return log(x/(1-x));
}

inline bool occMap::isInMap(const Eigen::Vector3d& pos){
    if ((pos(0) >= this->mapSizeMin_(0)) and (pos(0) <= this->mapSizeMax_(0)) and 
        (pos(1) >= this->mapSizeMin_(1)) and (pos(1) <= this->mapSizeMax_(1)) and 
        (pos(2) >= this->mapSizeMin_(2)) and (pos(2) <= this->mapSizeMax_(2))){
        return true;
    }
    else{
        return false;
    }
}

inline bool occMap::isInMap(const Eigen::Vector3i& idx){
    if ((idx(0) >= this->mapVoxelMin_(0)) and (idx(0) < this->mapVoxelMax_(0)) and
        (idx(1) >= this->mapVoxelMin_(1)) and (idx(1) < this->mapVoxelMax_(1)) and 
        (idx(2) >= this->mapVoxelMin_(2)) and (idx(2) < this->mapVoxelMax_(2))){
        return true;
    }
    else{
        return false;
    }
}

inline void occMap::posToIndex(const Eigen::Vector3d& pos, Eigen::Vector3i& idx){
    idx(0) = floor( (pos(0) - this->mapSizeMin_(0) ) / this->mapRes_ );
    idx(1) = floor( (pos(1) - this->mapSizeMin_(1) ) / this->mapRes_ );
    idx(2) = floor( (pos(2) - this->mapSizeMin_(2) ) / this->mapRes_ );
}

inline void occMap::indexToPos(const Eigen::Vector3i& idx, Eigen::Vector3d& pos){
    pos(0) = (idx(0) + 0.5) * this->mapRes_ + this->mapSizeMin_(0); 
    pos(1) = (idx(1) + 0.5) * this->mapRes_ + this->mapSizeMin_(1);
    pos(2) = (idx(2) + 0.5) * this->mapRes_ + this->mapSizeMin_(2);
}

inline int occMap::posToAddress(const Eigen::Vector3d& pos){
    Eigen::Vector3i idx;
    this->posToIndex(pos, idx);
    return this->indexToAddress(idx);
}

inline int occMap::posToAddress(double x, double y, double z){
    Eigen::Vector3d pos (x, y, z);
    return this->posToAddress(pos);
}

inline int occMap::indexToAddress(const Eigen::Vector3i& idx){
    return idx(0) * this->mapVoxelMax_(1) * this->mapVoxelMax_(2) + idx(1) * this->mapVoxelMax_(2) + idx(2);
}

inline int occMap::indexToAddress(int x, int y, int z){
    Eigen::Vector3i idx (x, y, z);
    return this->indexToAddress(idx);
}

inline void occMap::boundIndex(Eigen::Vector3i& idx){
    Eigen::Vector3i temp;
    temp(0) = std::max(std::min(idx(0), this->mapVoxelMax_(0)-1), this->mapVoxelMin_(0));
    temp(1) = std::max(std::min(idx(1), this->mapVoxelMax_(1)-1), this->mapVoxelMin_(1));
    temp(2) = std::max(std::min(idx(2), this->mapVoxelMax_(2)-1), this->mapVoxelMin_(2));
    idx = temp;
}

inline bool occMap::isInLocalUpdateRange(const Eigen::Vector3d& pos){
    Eigen::Vector3i idx;
    this->posToIndex(pos, idx);
    return this->isInLocalUpdateRange(idx);
}

inline bool occMap::isInLocalUpdateRange(const Eigen::Vector3i& idx){
    Eigen::Vector3d rangeMin = this->position_ - this->localUpdateRange_;
    Eigen::Vector3d rangeMax = this->position_ + this->localUpdateRange_;
    
    Eigen::Vector3i rangeMinIdx, rangeMaxIdx;
    this->posToIndex(rangeMin, rangeMinIdx);
    this->posToIndex(rangeMax, rangeMaxIdx);

    this->boundIndex(rangeMinIdx);
    this->boundIndex(rangeMaxIdx);

    bool inRange = (idx(0) >= rangeMinIdx(0)) and (idx(0) <= rangeMaxIdx(0)) and
                    (idx(1) >= rangeMinIdx(1)) and (idx(1) <= rangeMaxIdx(1)) and
                    (idx(2) >= rangeMinIdx(2)) and (idx(2) <= rangeMaxIdx(2));
    return inRange;
}




inline Eigen::Vector3d occMap::adjustPointInMap(const Eigen::Vector3d& point){
    Eigen::Vector3d pos = this->position_;
    Eigen::Vector3d diff = point - pos;
    Eigen::Vector3d offsetMin = this->mapSizeMin_ - pos;
    Eigen::Vector3d offsetMax = this->mapSizeMax_ - pos;

    double minRatio = std::numeric_limits<double>::max();
    for (int i=0; i<3; ++i){ 
        if (diff[i] != 0){
            double ratio1 = offsetMin[i]/diff[i];
            double ratio2 = offsetMax[i]/diff[i];
            if ((ratio1 > 0) and (ratio1 < minRatio)){
                minRatio = ratio1;
            }

            if ((ratio2 > 0) and (ratio2 < minRatio)){
                minRatio = ratio2;
            }
        }
    }

    return pos + (minRatio - 1e-3) * diff;
}


inline Eigen::Vector3d occMap::adjustPointRayLength(const Eigen::Vector3d& point){
    double length = (point - this->position_).norm();
    return (point - this->position_) * (this->raycastMaxLength_/length) + this->position_;
}

inline int occMap::updateOccupancyInfo(const Eigen::Vector3d& point, bool isOccupied){
    Eigen::Vector3i idx;
    this->posToIndex(point, idx);
    int voxelID = this->indexToAddress(idx);
    this->countHitMiss_[voxelID] += 1;
    if (this->countHitMiss_[voxelID] == 1){
        this->updateVoxelCache_.push(idx);
    }
    if (isOccupied){ 
        this->countHit_[voxelID] += 1;
    }
    return voxelID;
}


inline void occMap::getCameraPose(const nav_msgs::OdometryConstPtr& odom, Eigen::Matrix4d& camPoseMatrix){
    Eigen::Quaterniond quat;
    quat = Eigen::Quaterniond(odom->pose.pose.orientation.w, odom->pose.pose.orientation.x, odom->pose.pose.orientation.y, odom->pose.pose.orientation.z);
    Eigen::Matrix3d rot = quat.toRotationMatrix();

    // convert body pose to camera pose
    Eigen::Matrix4d map2body; map2body.setZero();
    map2body.block<3, 3>(0, 0) = rot;
    map2body(0, 3) = odom->pose.pose.position.x; 
    map2body(1, 3) = odom->pose.pose.position.y;
    map2body(2, 3) = odom->pose.pose.position.z;
    map2body(3, 3) = 1.0;

    camPoseMatrix = map2body * this->body2Cam_;
}

#endif