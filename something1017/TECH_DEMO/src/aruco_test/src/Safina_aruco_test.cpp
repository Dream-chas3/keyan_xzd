#include "aruco_test/Safina_aruco_test.h"
#include <algorithm>
#include <stdexcept>

Safina_Aruco::Safina_Aruco(ros::NodeHandle node_aruco) : node_aruco(node_aruco) {
    ros::NodeHandle p("/aruco_marker");
    p.param("original_enabled",original_enabled_,original_enabled_);
    p.param("five_enabled",five_enabled_,five_enabled_);p.param("prefer_original",prefer_original_,prefer_original_);
    p.param("original_id",original_id_,original_id_);p.param("five_id",five_id_,five_id_);
    p.param("original_size_m",original_size_,original_size_);p.param("five_size_m",five_size_,five_size_);
    p.param("original_yaw_offset_deg",original_offset_,original_offset_);
    p.param("five_yaw_offset_deg",five_offset_,five_offset_);
    p.param("calibrated_width",calibrated_width_,calibrated_width_);
    p.param("calibrated_height",calibrated_height_,calibrated_height_);
    p.param("require_calibrated_resolution",require_resolution_,require_resolution_);
    dictionary_5=cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_250);
    dictionary_ori=cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    if((!original_enabled_&&!five_enabled_)||original_id_<0||original_id_>=dictionary_ori->bytesList.rows||
       five_id_<0||five_id_>=dictionary_5->bytesList.rows||!std::isfinite(original_size_)||original_size_<=0||
       !std::isfinite(five_size_)||five_size_<=0||!std::isfinite(original_offset_)||!std::isfinite(five_offset_)||
       calibrated_width_<=0||calibrated_height_<=0)throw std::runtime_error("Invalid ArUco ID/size/calibration parameters");
    p.param("preview_width",preview_width_,preview_width_);
    p.param("preview_hz",preview_hz_,preview_hz_);
    p.param("opencv_threads",opencv_threads_,opencv_threads_);
    if(preview_width_<160||!std::isfinite(preview_hz_)||preview_hz_<=0||preview_hz_>30||opencv_threads_<1)
        throw std::runtime_error("Invalid ArUco preview/threads parameters");
    cv::setNumThreads(opencv_threads_);
    p.param("roi_enabled",roi_enabled_,roi_enabled_);
    p.param("full_search_scale",full_search_scale_,full_search_scale_);
    p.param("full_search_period",full_search_period_,full_search_period_);
    p.param("roi_margin_px",roi_margin_,roi_margin_);
    if(full_search_scale_<0.5||full_search_scale_>1||full_search_period_<0.2||full_search_period_>2||roi_margin_<40)
        throw std::runtime_error("Invalid detector search configuration");
    p.param("color_topic",color_topic_,color_topic_);
    p.param("preview_color",preview_color_,preview_color_);
    p.param("original_adaptive_max",original_adaptive_max_,original_adaptive_max_);
    p.param("five_adaptive_max",five_adaptive_max_,five_adaptive_max_);
    p.param("full_native_period",full_native_period_,full_native_period_);
    auto make_params=[&](int max_window) {
        if(max_window<3||max_window>255)throw std::runtime_error("adaptiveThreshWinSizeMax out of range");
        if(max_window%2==0)++max_window;
        cv::Ptr<cv::aruco::DetectorParameters> params=cv::aruco::DetectorParameters::create();
        params->adaptiveThreshWinSizeMax=max_window;
        params->cornerRefinementMethod=cv::aruco::CORNER_REFINE_SUBPIX;
        params->cornerRefinementWinSize=3;
        return params;
    };
    detector_params_orig_=make_params(original_adaptive_max_);
    detector_params_five_=make_params(five_adaptive_max_);
    if(!std::isfinite(full_native_period_)||full_native_period_<=0)throw std::runtime_error("Invalid full_native_period");
    clahe_=cv::createCLAHE(2.0,cv::Size(8,8));
    camera_matrix_=(cv::Mat_<double>(3,3)<<2042.0851788597752,0.0,959.0675902004489,
        0.0,2042.8993693539358,467.50991502443503,0.0,0.0,1.0);
    dist_coeffs_=(cv::Mat_<double>(1,5)<<-0.4849208954761301,0.3575460777522052,
        0.0024881576093297698,-0.0002765905391136276,0.0);
    initializePublishers();initializeSubscribers();
    preview_thread_=std::thread(&Safina_Aruco::previewLoop,this);
    ROS_INFO("ArUco configured: ORIGINAL id=%d side=%.4f m (adaptive max %d), 5X5 id=%d side=%.4f m (adaptive max %d), calibration=%dx%d",
        original_id_,original_size_,detector_params_orig_->adaptiveThreshWinSizeMax,
        five_id_,five_size_,detector_params_five_->adaptiveThreshWinSizeMax,
        calibrated_width_,calibrated_height_);
}
void Safina_Aruco::initializeSubscribers() {
    node_aruco.param<std::string>("img_topic_",img_topic_,"/rtsp_cam/image_raw");
    sub_img=node_aruco.subscribe(img_topic_,1,&Safina_Aruco::readStream,this,ros::TransportHints().tcpNoDelay());
    if(preview_color_&&!color_topic_.empty())
        sub_color=node_aruco.subscribe(color_topic_,1,&Safina_Aruco::readColor,this,ros::TransportHints().tcpNoDelay());
}
void Safina_Aruco::initializePublishers() {
    pub_aruco_imgresult=node_aruco.advertise<sensor_msgs::Image>("/aruco_imgresult",1);
    pub_aruco_poseresult=node_aruco.advertise<geometry_msgs::PoseStamped>("/aruco_poseresult",1,false);
    metrics_pub_=node_aruco.advertise<std_msgs::String>("/aruco_detector/metrics",1);
    pub_target_id_=node_aruco.advertise<std_msgs::Int32>("/aruco_target_id",1);
}
Safina_Aruco::~Safina_Aruco() {
    {std::lock_guard<std::mutex> lock(preview_mutex_);preview_stopping_=true;}
    preview_cv_.notify_all();
    if(preview_thread_.joinable())preview_thread_.join();
}
inline double Safina_Aruco::calculateRotationAngle(const std::vector<cv::Point2f>& corners) {
    const auto edge=corners[1]-corners[0];return std::atan2(edge.y,edge.x);
}
void Safina_Aruco::readColor(const sensor_msgs::ImageConstPtr& msg) {
    std::lock_guard<std::mutex> lock(color_mutex_);
    color_msg_=msg;color_at_=ros::WallTime::now().toSec();
}
void Safina_Aruco::readStream(const sensor_msgs::ImageConstPtr& image) {
    const ros::Time received=ros::Time::now();const double started=ros::WallTime::now().toSec();
    const bool preview=pub_aruco_imgresult.getNumSubscribers()>0;
    std::string search_mode="none";std::string reason="no_configured_id";std::ostringstream seen;seen<<'[';bool first=true;
    geometry_msgs::PoseStamped result;result.header=image->header;
    if(result.header.stamp.isZero()) {
        result.header.stamp=received;
        ROS_WARN_THROTTLE(10,"Image has no capture stamp; stamping detector receipt. RTSP capture latency remains unknown.");
    }
    if(result.header.frame_id.empty())result.header.frame_id="gimbal_camera_optical_frame";
    result.pose.position.z=-1;result.pose.orientation.w=1;
    std_msgs::Int32 selected;selected.data=-1;
    double detector_ended=started;
    try {
        // Detection runs on mono8 supplied by rtsp_hw_pub_ultra: no BGR copy and
        // no colour conversion. A bgr8 stream (older publisher / test fixture)
        // still works through the grey fallback.
        cv_bridge::CvImageConstPtr source=cv_bridge::toCvShare(image);
        cv::Mat gray,gray_storage;
        if(source->encoding=="mono8"||source->encoding=="8UC1")gray=source->image;
        else if(source->encoding=="bgr8"||source->encoding=="8UC3") {
            cv::cvtColor(source->image,gray_storage,cv::COLOR_BGR2GRAY);gray=gray_storage;
        } else {
            cv::Mat any=cv_bridge::toCvCopy(image)->image;
            if(any.channels()==3){cv::cvtColor(any,gray_storage,cv::COLOR_BGR2GRAY);gray=gray_storage;}
            else {gray_storage=any;gray=gray_storage;}
        }
        if(require_resolution_&&(gray.cols!=calibrated_width_||gray.rows!=calibrated_height_)) {
            reason="resolution_mismatch";
            ROS_ERROR_THROTTLE(5,"ArUco resolution %dx%d differs from calibration %dx%d; target invalid",
                gray.cols,gray.rows,calibrated_width_,calibrated_height_);
        } else {
            std::vector<int> ids[2];std::vector<std::vector<cv::Point2f>> corners[2];
            std::vector<std::vector<cv::Point2f>> preview_corners;std::vector<int> preview_ids;
            cv::Mat inverted_gray;
            const cv::TermCriteria criteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS,20,0.02);
            auto detect=[&](const cv::Rect& area,double scale,bool enhance,const char* mode) {
                if(area.width<8||area.height<8)return;
                search_mode=mode;
                cv::Mat region=gray(area),work;
                if(scale<1)cv::resize(region,work,cv::Size(),scale,scale,cv::INTER_AREA);else work=region;
                if(enhance){cv::Mat equalized;clahe_->apply(work,equalized);work=equalized;}
                cv::Mat inverse;
                if(five_enabled_)cv::bitwise_not(work,inverse);
                preview_corners.clear();preview_ids.clear();
                for(int family=0;family<2;++family) {
                    ids[family].clear();corners[family].clear();
                    if((family==0&&!original_enabled_)||(family==1&&!five_enabled_))continue;
                    cv::aruco::detectMarkers(family==0?work:inverse,family==0?dictionary_ori:dictionary_5,
                        corners[family],ids[family],family==0?detector_params_orig_:detector_params_five_);
                    for(auto& box:corners[family])for(auto& point:box) {
                        point.x=point.x/float(scale)+area.x;point.y=point.y/float(scale)+area.y;
                    }
                    // Full-pixel refinement uses the unchanged calibration matrix.
                    for(size_t k=0;k<ids[family].size();++k) {
                        if(ids[family][k]!=(family==0?original_id_:five_id_))continue;
                        auto& points=corners[family][k];bool inside=true;
                        for(const auto& pt:points)inside=inside&&pt.x>4&&pt.y>4&&pt.x<gray.cols-5&&pt.y<gray.rows-5;
                        if(!inside)continue;
                        if(family==0)cv::cornerSubPix(gray,points,cv::Size(3,3),cv::Size(-1,-1),criteria);
                        else {
                            // Inverted grey is only built when a 5X5 target is
                            // actually present; it was previously computed for
                            // every frame regardless of detections.
                            if(inverted_gray.empty())cv::bitwise_not(gray,inverted_gray);
                            cv::cornerSubPix(inverted_gray,points,cv::Size(3,3),cv::Size(-1,-1),criteria);
                        }
                    }
                    if(preview)for(size_t k=0;k<ids[family].size();++k) {
                        preview_corners.push_back(corners[family][k]);preview_ids.push_back(ids[family][k]);
                    }
                }
            };
            auto found=[&] {
                return std::find(ids[0].begin(),ids[0].end(),original_id_)!=ids[0].end()||
                       std::find(ids[1].begin(),ids[1].end(),five_id_)!=ids[1].end();
            };
            const cv::Rect whole(0,0,gray.cols,gray.rows);
            const bool tracked=roi_enabled_&&last_seen_>=0&&started-last_seen_<0.8&&tracking_roi_.area()>0;
            if(tracked&&started-last_full_search_<full_search_period_) {
                detect(tracking_roi_&whole,1.0,false,"roi");
                if(!found())detect(tracking_roi_&whole,1.0,true,"roi_clahe");
            }
            if(!found()) {
                detect(whole,full_search_scale_,false,"full_scaled");
                // Periodic full-resolution fallback retains small-marker acquisition.
                if(!found()&&started-last_native_search_>=full_native_period_){detect(whole,1.0,false,"full_native");last_native_search_=started;}
                last_full_search_=started;
            }
            detector_ended=ros::WallTime::now().toSec();
            int index[2]={-1,-1};
            for(int family=0;family<2;++family) {
                const int wanted=family==0?original_id_:five_id_;
                auto at=std::find(ids[family].begin(),ids[family].end(),wanted);
                if(at!=ids[family].end())index[family]=std::distance(ids[family].begin(),at);
                for(int id:ids[family]) {
                    if(!first)seen<<',';first=false;
                    seen<<"{\"family\":\""<<(family==0?"ORIGINAL":"5X5_INVERTED")<<"\",\"id\":"<<id<<'}';
                }
            }
            int family=last_family_;
            if(family<0||index[family]<0) {
                const int preferred=prefer_original_?0:1;
                family=index[preferred]>=0?preferred:(index[1-preferred]>=0?1-preferred:-1);
            }
            if(family>=0&&last_family_>=0&&family!=last_family_&&started-last_seen_<0.8) {
                if(pending_family_==family)++pending_count_;else{pending_family_=family;pending_count_=1;}
                if(pending_count_<2){family=-1;reason="family_switch_confirming";}
            } else {pending_family_=-1;pending_count_=0;}
            if(family>=0) {
                std::vector<std::vector<cv::Point2f>> one{corners[family][index[family]]};
                std::vector<cv::Vec3d> rvecs,tvecs;
                cv::aruco::estimatePoseSingleMarkers(one,family==0?original_size_:five_size_,
                    camera_matrix_,dist_coeffs_,rvecs,tvecs);
                const auto& v=tvecs.at(0);
                if(std::isfinite(v[0])&&std::isfinite(v[1])&&std::isfinite(v[2])&&v[2]>0) {
                    result.pose.position.x=v[0];result.pose.position.y=v[1];result.pose.position.z=v[2];
                    const double offset=(family==0?original_offset_:five_offset_)*CV_PI/180.0;
                    const double raw=calculateRotationAngle(one[0])+offset;
                    tf2::Quaternion q;q.setRPY(0,0,std::atan2(std::sin(raw),std::cos(raw)));
                    result.pose.orientation=tf2::toMsg(q);selected.data=family==0?original_id_:five_id_;
                    last_family_=family;last_seen_=started;reason="selected_pose_valid";
                    cv::Rect box=cv::boundingRect(one[0]);
                    const int margin=std::max(roi_margin_,int(std::max(box.width,box.height)*0.7));
                    tracking_roi_=cv::Rect(box.x-margin,box.y-margin,box.width+2*margin,box.height+2*margin)&whole;
                    ROS_INFO_THROTTLE(1,"ArUco family=%s id=%d x=%.3f y=%.3f depth=%.3f edge_yaw=%.1f deg",
                        family==0?"ORIGINAL":"5X5",selected.data,v[0],v[1],v[2],raw*180.0/CV_PI);
                }
            }
            if(preview) {
                std::lock_guard<std::mutex> lock(preview_mutex_);
                preview_snapshot_.source=image;
                preview_snapshot_.corners=preview_corners;
                preview_snapshot_.ids=preview_ids;
                preview_snapshot_.selected=selected.data;
                preview_snapshot_.header=result.header;
                preview_has_data_=true;
                preview_cv_.notify_one();
            }
        }
    } catch(const std::exception& error) {reason="processing_exception";ROS_ERROR_THROTTLE(2,"ArUco image processing: %s",error.what());}
    if(selected.data<0&&started-last_seen_>0.8){last_family_=-1;pending_family_=-1;pending_count_=0;tracking_roi_=cv::Rect();}
    pub_aruco_poseresult.publish(result);pub_target_id_.publish(selected);
    if(window_start_<0)window_start_=started;
    ++window_frames_;
    if(started-window_start_>=1.0){processed_fps_=window_frames_/(started-window_start_);window_start_=started;window_frames_=0;}
    const double ended=ros::WallTime::now().toSec();
    double color_age=-1;
    {std::lock_guard<std::mutex> lock(color_mutex_);if(color_msg_&&color_at_>0)color_age=std::max(0.0,ended-color_at_);}
    seen<<']';std::ostringstream metrics;
    metrics<<"{\"reason\":\""<<reason<<"\",\"selected_id\":"<<selected.data
        <<",\"detected\":"<<seen.str()<<",\"processing_ms\":"<<(ended-started)*1000
        <<",\"detect_ms\":"<<(detector_ended-started)*1000
        <<",\"preview_ms\":"<<last_preview_ms_.load()
        <<",\"processed_fps\":"<<processed_fps_
        <<",\"search_mode\":\""<<search_mode<<"\",\"source_stamp_known\":"<<(!image->header.stamp.isZero()?"true":"false")
        <<",\"input_age_seconds\":"<<(image->header.stamp.isZero()?-1:(received-image->header.stamp).toSec())
        <<",\"encoding\":\""<<image->encoding<<"\""
        <<",\"color_age_ms\":"<<(color_age<0?-1:color_age*1000)
        <<",\"width\":"<<image->width<<",\"height\":"<<image->height
        <<",\"original_id\":"<<original_id_<<",\"five_id\":"<<five_id_
        <<",\"five_size_m\":"<<five_size_<<",\"preview_hz\":"<<preview_hz_
        <<",\"adaptive_max\":["<<detector_params_orig_->adaptiveThreshWinSizeMax<<','
        <<detector_params_five_->adaptiveThreshWinSizeMax<<"]}";
    std_msgs::String report;report.data=metrics.str();metrics_pub_.publish(report);
}
void Safina_Aruco::previewLoop() {
    std::unique_lock<std::mutex> lock(preview_mutex_);
    while(!preview_stopping_) {
        preview_cv_.wait_for(lock,std::chrono::milliseconds(10));
        if(preview_stopping_)break;
        if(!preview_has_data_||pub_aruco_imgresult.getNumSubscribers()==0)continue;
        const double now=ros::WallTime::now().toSec();
        if(last_preview_>=0&&now-last_preview_<1.0/preview_hz_)continue;
        const PreviewSnapshot snapshot=preview_snapshot_;
        lock.unlock();
        const double started=ros::WallTime::now().toSec();
        try {
        cv::Mat canvas;
        {
            std::lock_guard<std::mutex> color_lock(color_mutex_);
            if(preview_color_&&color_msg_) {
                cv_bridge::CvImageConstPtr color=cv_bridge::toCvShare(color_msg_);
                if(color&&color->image.type()==CV_8UC3)last_color_canvas_=color->image.clone();
            }
        }
        // Keep the last colour frame even while a long full-image search blocks
        // the colour callback: falling back to grey here made the preview flash
        // grey every few seconds whenever the marker left the image.
        if(!last_color_canvas_.empty())canvas=last_color_canvas_.clone();
        else if(snapshot.source) {
            cv_bridge::CvImageConstPtr source=cv_bridge::toCvShare(snapshot.source);
            if(source) {
                const cv::Mat& base=source->image;
                const double scale=std::min(1.0,double(preview_width_)/std::max(1,base.cols));
                cv::Mat small;
                if(scale<1)cv::resize(base,small,cv::Size(),scale,scale,cv::INTER_AREA);else small=base;
                if(small.channels()==1)cv::cvtColor(small,canvas,cv::COLOR_GRAY2BGR);
                else if(small.channels()==3)canvas=small.clone();
            }
        }
        if(canvas.empty()) {last_preview_ms_.store(0.0);lock.lock();continue;}
        const int ref_w=snapshot.source?snapshot.source->width:canvas.cols;
        const int ref_h=snapshot.source?snapshot.source->height:canvas.rows;
        const double sx=double(canvas.cols)/std::max(1,ref_w),sy=double(canvas.rows)/std::max(1,ref_h);
        if(!snapshot.corners.empty()) {
            std::vector<std::vector<cv::Point2f>> scaled=snapshot.corners;
            for(auto& box:scaled)for(auto& point:box){point.x*=sx;point.y*=sy;}
            for(size_t k=0;k<scaled.size();++k) {
                const bool selected=k<snapshot.ids.size()&&snapshot.ids[k]==snapshot.selected;
                const cv::Scalar color=selected?cv::Scalar(0,255,0):cv::Scalar(0,180,255);
                const int thickness=selected?3:2;
                std::vector<cv::Point> poly;
                poly.reserve(scaled[k].size());
                for(const auto& point:scaled[k])poly.emplace_back(cvRound(point.x),cvRound(point.y));
                cv::polylines(canvas,poly,true,color,thickness,cv::LINE_AA);
                for(const auto& point:scaled[k])cv::circle(canvas,point,selected?5:3,color,-1,cv::LINE_AA);
                // Marker centre as a red dot (black rim keeps it visible on any background).
                cv::Point2f center(0,0);
                for(const auto& point:scaled[k])center+=point;
                center*=0.25f;
                cv::circle(canvas,center,selected?6:4,cv::Scalar(0,0,0),1,cv::LINE_AA);
                cv::circle(canvas,center,selected?4:3,cv::Scalar(0,0,255),-1,cv::LINE_AA);
                if(k<snapshot.ids.size()) {
                    const std::string label="id "+std::to_string(snapshot.ids[k]);
                    const cv::Point at(cvRound(scaled[k][0].x),cvRound(scaled[k][0].y)-10);
                    cv::putText(canvas,label,at,cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,0,0),4,cv::LINE_AA);
                    cv::putText(canvas,label,at,cv::FONT_HERSHEY_SIMPLEX,0.8,color,2,cv::LINE_AA);
                }
            }
        }
        cv::putText(canvas,snapshot.selected>=0?"SELECTED TARGET: "+std::to_string(snapshot.selected):"NO CONFIGURED TARGET",
            cv::Point(24,48),cv::FONT_HERSHEY_SIMPLEX,1.1,
            snapshot.selected>=0?cv::Scalar(0,255,0):cv::Scalar(0,0,255),2);
        cv::Mat display;
        if(canvas.cols>preview_width_) {
            cv::resize(canvas,display,cv::Size(preview_width_,
                std::max(1,int(canvas.rows*double(preview_width_)/canvas.cols))),0,0,cv::INTER_AREA);
        } else display=canvas;
        pub_aruco_imgresult.publish(cv_bridge::CvImage(snapshot.header,"bgr8",display).toImageMsg());
        {
            std::lock_guard<std::mutex> guard(preview_mutex_);
            last_preview_=now;
            last_preview_ms_.store((ros::WallTime::now().toSec()-started)*1000.0);
        }
        } catch(const std::exception& error) {
            ROS_ERROR_THROTTLE(5,"ArUco preview: %s",error.what());
        }
        lock.lock();
    }
}

int main(int argc,char** argv) {
    ros::init(argc,argv,"aruco");ros::NodeHandle node;
    try {Safina_Aruco detector(node);ros::spin();}
    catch(const std::exception& error){ROS_FATAL("ArUco initialization failed: %s",error.what());return 1;}
    return 0;
}
