#include "map_test/Safina_map_test.h"
#include <ros/ros.h>
occMap::occMap(){
	this->ns_ = "/occmap";

}

occMap::occMap(const ros::NodeHandle& nh) : node_land_(nh){
	this->ns_ = "/occmap";
	this->initParam();

	this->registerPub();
	this->registerCallback();
}

// void occMap::initMap(const ros::NodeHandle& nh){
// 	this->node_land_ = nh;
// 	this->initParam();

// 	this->registerPub();
// 	this->registerCallback();
// }

void occMap::initParam(){

	this->node_land_.getParam(this->ns_ + "/point_cloud_topic", this->pointcloudTopicName_);

	std::vector<double> robotSizeVec{0.5, 0.5, 0.3};
	this->node_land_.getParam(this->ns_ + "/robot_size", robotSizeVec);
	this->robotSize_(0) = robotSizeVec[0]; 
	this->robotSize_(1) = robotSizeVec[1]; 
	this->robotSize_(2) = robotSizeVec[2];

	std::vector<double> body2CamVec (16);
	this->node_land_.getParam(this->ns_ + "/body_to_camera", body2CamVec);
	for (int i=0; i<4; ++i){
		for (int j=0; j<4; ++j){
			this->body2Cam_(i, j) = body2CamVec[i * 4 + j];
		}
	}

	this->node_land_.getParam(this->ns_ + "/map_resolution", this->mapRes_);

	std::vector<double> mapSizeVec{20.0, 20.0, 3.0};
	this->node_land_.getParam(this->ns_ + "/map_size", mapSizeVec);

	// 物理尺寸：以原点为中心，z 从 groundHeight_ 开始
	mapSizeMin_(0) = -mapSizeVec[0] / 2.0;
	mapSizeMax_(0) =  mapSizeVec[0] / 2.0;
	mapSizeMin_(1) = -mapSizeVec[1] / 2.0;
	mapSizeMax_(1) =  mapSizeVec[1] / 2.0;
	mapSizeMin_(2) =  groundHeight_;
	mapSizeMax_(2) =  groundHeight_ + mapSizeVec[2];

	// 体素索引范围 [0, max)
	mapVoxelMin_(0) = 0;
	mapVoxelMax_(0) = static_cast<int>(std::ceil(mapSizeVec[0] / mapRes_));
	mapVoxelMin_(1) = 0;
	mapVoxelMax_(1) = static_cast<int>(std::ceil(mapSizeVec[1] / mapRes_));
	mapVoxelMin_(2) = 0;
	mapVoxelMax_(2) = static_cast<int>(std::ceil(mapSizeVec[2] / mapRes_));

	// 一维数组总大小
	const int reservedSize =mapVoxelMax_(0) * mapVoxelMax_(1) * mapVoxelMax_(2);

	// 分配 / 重置各数组
	countHitMiss_.assign(reservedSize, 0);
	countHit_.assign(reservedSize, 0);
	occupancy_.assign(reservedSize, pMinLog_ - UNKNOWN_FLAG_);
	occupancyInflated_.assign(reservedSize, false);
	flagTraverse_.assign(reservedSize, -1);
	flagRayend_.assign(reservedSize, -1);


	std::vector<double> localUpdateRangeVec{4.0, 4.0, 2.0};
	this->node_land_.getParam(this->ns_ + "/local_update_range", localUpdateRangeVec);
	this->localUpdateRange_(0) = localUpdateRangeVec[0]; 
	this->localUpdateRange_(1) = localUpdateRangeVec[1]; 
	this->localUpdateRange_(2) = localUpdateRangeVec[2];


	std::vector<double> localMapSizeVec{10.0, 10.0, 2.0};
	this->node_land_.getParam(this->ns_ + "/local_map_size", localMapSizeVec);
	this->localMapSize_(0) = localMapSizeVec[0]/2;
	this->localMapSize_(1) = localMapSizeVec[1]/2; 
	this->localMapSize_(2) = localMapSizeVec[2]/2;


	this->node_land_.getParam(this->ns_ + "/max_height_visualization", this->maxVisHeight_);

}
void occMap::registerCallback(){

	this->pointcloudSub_.reset(new message_filters::Subscriber<sensor_msgs::PointCloud2>(this->node_land_, this->pointcloudTopicName_, 50));
	this->odomSub_.reset(new message_filters::Subscriber<nav_msgs::Odometry>(this->node_land_, this->odomTopicName_, 25));
	this->pointcloudOdomSync_.reset(new message_filters::Synchronizer<pointcloudOdomSync>(pointcloudOdomSync(100), *this->pointcloudSub_, *this->odomSub_));
	this->pointcloudOdomSync_->registerCallback(boost::bind(&occMap::pointcloudOdomCB, this, _1, _2));


	this->occTimer_ = this->node_land_.createTimer(ros::Duration(0.05), &occMap::updateOccupancyCB, this);
	this->inflateTimer_ = this->node_land_.createTimer(ros::Duration(0.05), &occMap::inflateMapCB, this);


	this->visTimer_ = this->node_land_.createTimer(ros::Duration(0.1), &occMap::visCB, this);


}
void occMap::registerPub(){

	this->depthCloudPub_ = this->node_land_.advertise<sensor_msgs::PointCloud2>(this->ns_ + "/depth_cloud", 10);
	this->mapVisPub_ = this->node_land_.advertise<sensor_msgs::PointCloud2>(this->ns_ + "/voxel_map", 10);
	this->inflatedMapVisPub_ = this->node_land_.advertise<sensor_msgs::PointCloud2>(this->ns_ + "/inflated_voxel_map", 10);

}
void occMap::pointcloudOdomCB(const sensor_msgs::PointCloud2ConstPtr& pointcloud, const nav_msgs::OdometryConstPtr& odom){

	pcl::PCLPointCloud2 pclPC2;
	pcl_conversions::toPCL(*pointcloud, pclPC2); 
	pcl::fromPCLPointCloud2(pclPC2, this->pointcloud_);


	Eigen::Matrix4d camPoseMatrix;
	this->getCameraPose(odom, camPoseMatrix);

	this->position_(0) = camPoseMatrix(0, 3);
	this->position_(1) = camPoseMatrix(1, 3);
	this->position_(2) = camPoseMatrix(2, 3);
	this->orientation_ = camPoseMatrix.block<3, 3>(0, 0);

	if (this->isInMap(this->position_)){
		this->occNeedUpdate_ = true;
	}
	else{
		this->occNeedUpdate_ = false;
	}
}

void occMap::updateOccupancyCB(const ros::TimerEvent& ){
	if (not this->occNeedUpdate_){
		return;
	}

	ros::Time startTime, endTime;
	
	startTime = ros::Time::now();

	this->getPointcloud();

	this->raycastUpdate();


	// this->inflateLocalMap();
	endTime = ros::Time::now();
	if (this->verbose_){
		cout << ": Occupancy update time: " << (endTime - startTime).toSec() << " s." << endl;
	}
	this->occNeedUpdate_ = false;
	this->mapNeedInflate_ = true;
}

void occMap::inflateMapCB(const ros::TimerEvent& ){
	if (this->mapNeedInflate_){
		this->inflateLocalMap();
		this->mapNeedInflate_ = false;
	}
}
void occMap::getPointcloud(){
	this->projPointsNum_ = this->pointcloud_.size();
	this->projPoints_.resize(this->projPointsNum_);
	Eigen::Vector3d currPointCam, currPointMap;
	for (int i=0; i<this->projPointsNum_; ++i){
		currPointCam(0) = this->pointcloud_.points[i].x;
		currPointCam(1) = this->pointcloud_.points[i].y;
		currPointCam(2) = this->pointcloud_.points[i].z;
		currPointMap = this->orientation_ * currPointCam + this->position_; // transform to map coordinate
		if ((currPointMap-this->position_).norm()>=0.2){
			this->projPoints_[i] = currPointMap;
		}
	}
}

void occMap::raycastUpdate(){
	if (this->projPointsNum_ == 0){
		return;
	}
	this->raycastNum_ += 1;

	double xmin, xmax, ymin, ymax, zmin, zmax;
	xmin = xmax = this->position_(0);
	ymin = ymax = this->position_(1);
	zmin = zmax = this->position_(2);

	Eigen::Vector3d currPoint;
	bool pointAdjusted;
	int rayendVoxelID, raycastVoxelID;
	double length;
	for (int i=0; i<this->projPointsNum_; ++i){
		currPoint = this->projPoints_[i];
		if (std::isnan(currPoint(0)) or std::isnan(currPoint(1)) or std::isnan(currPoint(2))){
			continue; 
		}

		pointAdjusted = false;
		
		if (not this->isInMap(currPoint)){
			currPoint = this->adjustPointInMap(currPoint);
			pointAdjusted = true;
		}

		
		length = (currPoint - this->position_).norm();
		if (length > this->raycastMaxLength_){
			currPoint = this->adjustPointRayLength(currPoint);
			pointAdjusted = true;
		}


		
		if (currPoint(0) < xmin){xmin = currPoint(0);}
		if (currPoint(1) < ymin){ymin = currPoint(1);}
		if (currPoint(2) < zmin){zmin = currPoint(2);}
		if (currPoint(0) > xmax){xmax = currPoint(0);}
		if (currPoint(1) > ymax){ymax = currPoint(1);}
		if (currPoint(2) > zmax){zmax = currPoint(2);}

		
		rayendVoxelID = this->updateOccupancyInfo(currPoint, not pointAdjusted);

		
		// rayendVoxelID = this->posToAddress(currPoint);
		if (this->flagRayend_[rayendVoxelID] == this->raycastNum_){
			continue; // skip
		}
		else{
			this->flagRayend_[rayendVoxelID] = this->raycastNum_;
		}

		
		this->raycaster_.setInput(currPoint/this->mapRes_, this->position_/this->mapRes_);
		Eigen::Vector3d rayPoint, actualPoint;
		while (this->raycaster_.step(rayPoint)){
			actualPoint = rayPoint;
			actualPoint(0) += 0.5;
			actualPoint(1) += 0.5;
			actualPoint(2) += 0.5;
			actualPoint *= this->mapRes_;
			raycastVoxelID = this->updateOccupancyInfo(actualPoint, false);

			
			if (this->flagTraverse_[raycastVoxelID] == this->raycastNum_){
				break;
			}
			else{
				this->flagTraverse_[raycastVoxelID] = this->raycastNum_;
			}

		}
	}

	this->posToIndex(Eigen::Vector3d (xmin, ymin, zmin), this->localBoundMin_);
	this->posToIndex(Eigen::Vector3d (xmax, ymax, zmax), this->localBoundMax_);
	this->localBoundMin_ -= int(ceil(this->localBoundInflate_/this->mapRes_)) * Eigen::Vector3i(1, 1, 0); // inflate in x y direction
	this->localBoundMax_ += int(ceil(this->localBoundInflate_/this->mapRes_)) * Eigen::Vector3i(1, 1, 0); 
	this->boundIndex(this->localBoundMin_); 
	this->boundIndex(this->localBoundMax_);

	double logUpdateValue;
	int cacheAddress, hit, miss;
	while (not this->updateVoxelCache_.empty()){
		Eigen::Vector3i cacheIdx = this->updateVoxelCache_.front();
		this->updateVoxelCache_.pop();
		cacheAddress = this->indexToAddress(cacheIdx);

		hit = this->countHit_[cacheAddress];
		miss = this->countHitMiss_[cacheAddress] - hit;

		if (hit >= miss and hit != 0){
			logUpdateValue = this->pHitLog_;
		}
		else{
			logUpdateValue = this->pMissLog_;
		}
		this->countHit_[cacheAddress] = 0; 
		this->countHitMiss_[cacheAddress] = 0; 

		
		if (not this->isInLocalUpdateRange(cacheIdx)){
			continue; 
		}

		// update occupancy info
		if ((logUpdateValue >= 0) and (this->occupancy_[cacheAddress] >= this->pMaxLog_)){
			continue; 
		}
		else if ((logUpdateValue <= 0) and (this->occupancy_[cacheAddress] == this->pMinLog_)){
			continue; 
		}
		else if ((logUpdateValue <= 0) and (this->occupancy_[cacheAddress] < this->pMinLog_)){
			this->occupancy_[cacheAddress] = this->pMinLog_;  
			continue;
		}

		this->occupancy_[cacheAddress] = std::min(std::max(this->occupancy_[cacheAddress]+logUpdateValue, this->pMinLog_), this->pMaxLog_);

		
		if (not this->isUnknown(cacheIdx)){
			Eigen::Vector3d cachePos;
			this->indexToPos(cacheIdx, cachePos);
			if (cachePos(0) > this->currMapRangeMax_(0)){
				this->currMapRangeMax_(0) = cachePos(0);
			}
			else if (cachePos(0) < this->currMapRangeMin_(0)){
				this->currMapRangeMin_(0) = cachePos(0);
			}

			if (cachePos(1) > this->currMapRangeMax_(1)){
				this->currMapRangeMax_(1) = cachePos(1);
			}
			else if (cachePos(1) < this->currMapRangeMin_(1)){
				this->currMapRangeMin_(1) = cachePos(1);
			}

			if (cachePos(2) > this->currMapRangeMax_(2)){
				this->currMapRangeMax_(2) = cachePos(2);
			}
			else if (cachePos(2) < this->currMapRangeMin_(2)){
				this->currMapRangeMin_(2) = cachePos(2);
			}
		}
	}
}
void occMap::inflateLocalMap(){
	int xmin = this->localBoundMin_(0);
	int xmax = this->localBoundMax_(0);
	int ymin = this->localBoundMin_(1);
	int ymax = this->localBoundMax_(1);
	int zmin = this->localBoundMin_(2);
	int zmax = this->localBoundMax_(2);
	Eigen::Vector3i clearIndex;
	
	for (int x=xmin; x<=xmax; ++x){
		for (int y=ymin; y<=ymax; ++y){
			for (int z=zmin; z<=zmax; ++z){
				clearIndex(0) = x; clearIndex(1) = y; clearIndex(2) = z;
				this->occupancyInflated_[this->indexToAddress(clearIndex)] = false;
			}
		}
	}

	int xInflateSize = ceil(this->robotSize_(0)/(2*this->mapRes_));
	int yInflateSize = ceil(this->robotSize_(1)/(2*this->mapRes_));
	int zInflateSize = ceil(this->robotSize_(2)/(2*this->mapRes_));

	Eigen::Vector3i pointIndex, inflateIndex;
	int inflateAddress;
	const int  maxIndex = this->mapVoxelMax_(0) * this->mapVoxelMax_(1) * this->mapVoxelMax_(2);
	for (int x=xmin; x<=xmax; ++x){
		for (int y=ymin; y<=ymax; ++y){
			for (int z=zmin; z<=zmax; ++z){
				pointIndex(0) = x; pointIndex(1) = y; pointIndex(2) = z;
				if (this->isOccupied(pointIndex)){
					for (int ix=-xInflateSize; ix<=xInflateSize; ++ix){
						for (int iy=-yInflateSize; iy<=yInflateSize; ++iy){
							for (int iz=-zInflateSize; iz<=zInflateSize; ++iz){
								inflateIndex(0) = pointIndex(0) + ix;
								inflateIndex(1) = pointIndex(1) + iy;
								inflateIndex(2) = pointIndex(2) + iz;
								inflateAddress = this->indexToAddress(inflateIndex);
								if ((inflateAddress < 0) or (inflateAddress > maxIndex)){
									continue; 
								} 
								this->occupancyInflated_[inflateAddress] = true;
							}
						}
					}
				}
			}
		}
	}
}

void occMap::visCB(const ros::TimerEvent& ){
	this->publishProjPoints();
	this->publishMap();
	this->publishInflatedMap();

}

void occMap::publishProjPoints(){
	pcl::PointXYZ pt;
	pcl::PointCloud<pcl::PointXYZ> cloud;

	for (int i=0; i<this->projPointsNum_; ++i){
		pt.x = this->projPoints_[i](0);
		pt.y = this->projPoints_[i](1);
		pt.z = this->projPoints_[i](2);
		cloud.push_back(pt);
	}

	cloud.width = cloud.points.size();
	cloud.height = 1;
	cloud.is_dense = true;
	cloud.header.frame_id = "map";

	sensor_msgs::PointCloud2 cloudMsg;
	pcl::toROSMsg(cloud, cloudMsg);
	this->depthCloudPub_.publish(cloudMsg);
}

void occMap::publishMap(){
	pcl::PointXYZ pt;
	pcl::PointCloud<pcl::PointXYZ> cloud;

	Eigen::Vector3d minRange, maxRange;
	if (this->visGlobalMap_){
		// minRange = this->mapSizeMin_;
		// maxRange = this->mapSizeMax_;
		minRange = this->currMapRangeMin_;
		maxRange = this->currMapRangeMax_;
	}
	else{
		minRange = this->position_ - localMapSize_;
		maxRange = this->position_ + localMapSize_;
		minRange(2) = this->groundHeight_;
	}
	Eigen::Vector3i minRangeIdx, maxRangeIdx;
	this->posToIndex(minRange, minRangeIdx);
	this->posToIndex(maxRange, maxRangeIdx);
	this->boundIndex(minRangeIdx);
	this->boundIndex(maxRangeIdx);

	for (int x=minRangeIdx(0); x<=maxRangeIdx(0); ++x){
		for (int y=minRangeIdx(1); y<=maxRangeIdx(1); ++y){
			for (int z=minRangeIdx(2); z<=maxRangeIdx(2); ++z){
				Eigen::Vector3i pointIdx (x, y, z);

				// if (this->occupancy_[this->indexToAddress(pointIdx)] > this->pMinLog_){
				if (this->isOccupied(pointIdx)){
					Eigen::Vector3d point;
					this->indexToPos(pointIdx, point);
					if (point(2) <= this->maxVisHeight_){
						pt.x = point(0);
						pt.y = point(1);
						pt.z = point(2);
						cloud.push_back(pt);
					}
				}

			}
		}
	}

	cloud.width = cloud.points.size();
	cloud.height = 1;
	cloud.is_dense = true;
	cloud.header.frame_id = "map";


	sensor_msgs::PointCloud2 cloudMsg;
	pcl::toROSMsg(cloud, cloudMsg);
	this->mapVisPub_.publish(cloudMsg);

}

void occMap::publishInflatedMap(){
	pcl::PointXYZ pt;
	pcl::PointCloud<pcl::PointXYZ> cloud;

	Eigen::Vector3d minRange, maxRange;
	if (this->visGlobalMap_){
		// minRange = this->mapSizeMin_;
		// maxRange = this->mapSizeMax_;
		minRange = this->currMapRangeMin_;
		maxRange = this->currMapRangeMax_;
	}
	else{
		minRange = this->position_ - localMapSize_;
		maxRange = this->position_ + localMapSize_;
		minRange(2) = this->groundHeight_;
	}
	Eigen::Vector3i minRangeIdx, maxRangeIdx;
	this->posToIndex(minRange, minRangeIdx);
	this->posToIndex(maxRange, maxRangeIdx);
	this->boundIndex(minRangeIdx);
	this->boundIndex(maxRangeIdx);

	for (int x=minRangeIdx(0); x<=maxRangeIdx(0); ++x){
		for (int y=minRangeIdx(1); y<=maxRangeIdx(1); ++y){
			for (int z=minRangeIdx(2); z<=maxRangeIdx(2); ++z){
				Eigen::Vector3i pointIdx (x, y, z);
				// if (this->occupancy_[this->indexToAddress(pointIdx)] > this->pMinLog_){
				if (this->isInflatedOccupied(pointIdx)){
					Eigen::Vector3d point;
					this->indexToPos(pointIdx, point);
					if (point(2) <= this->maxVisHeight_){
						pt.x = point(0);
						pt.y = point(1);
						pt.z = point(2);
						cloud.push_back(pt);
					}
				}
			}
		}
	}

	cloud.width = cloud.points.size();
	cloud.height = 1;
	cloud.is_dense = true;
	cloud.header.frame_id = "map";

	sensor_msgs::PointCloud2 cloudMsg;
	pcl::toROSMsg(cloud, cloudMsg);
	this->inflatedMapVisPub_.publish(cloudMsg);	
}

int main(int argc, char **argv)
{

    ros::init(argc, argv, "map");

    ros::NodeHandle node_map;
    
    occMap safina_land(node_map) ;

    ROS_INFO("map_test runing ...");

    ros::spin();
    return 0;
}