#ifndef SAFINA_YOLO_ULTRA_TEST_H
#define SAFINA_YOLO_ULTRA_TEST_H

#include <ros/ros.h>

#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int32.h>
#include <sensor_msgs/Image.h>
#include <mavros_msgs/RCIn.h>
#include <mavros_msgs/State.h>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <deque>
#include <string>
#include <vector>

#include "yolo_det_task/yoloUltra_custom.h"
#include "types/yolo_datatype.h"
#include "person_tracker.h"

class Safina_YoloUltra
{
public:
    Safina_YoloUltra(ros::NodeHandle node_yoloUltra);
    ~Safina_YoloUltra();

    // Model
    YoloUltraCustom yoloUltra;
    std::string yoloUltra_model_file;

    // Camera
    int img_rows = 720;
    int img_cols = 1280;
    float camera_fx = 613.3737182617188f;
    float camera_fy = 613.2047119140625f;
    float camera_cx = 644.221923828125f;
    float camera_cy = 363.62823486328125f;
    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;

    // ROS
    ros::Subscriber sub_img;
    ros::Subscriber sub_Rcin;
    ros::Subscriber sub_state;
    ros::Publisher pub_yoloUltra_imgresult;
    ros::Publisher pub_track;
    ros::Publisher pub_target_id;

    // SWA state (0=up, 1=mid, 2=down)
    int rc_state = -1;

    // Tracking session
    bool offboard_mode = false;
    bool tracking_session_active = false;
    bool session_latched = false;
    int primary_id = -1;
    bool waiting_for_new_primary = false;
    double primary_last_seen_s = 0.0;
    double primary_lost_time_s = 0.0;
    double primary_last_center_x = -1.0;
    double primary_last_center_y = -1.0;
    double primary_last_height = 0.0;
    double primary_last_velocity_x = 0.0;
    double primary_last_velocity_y = 0.0;

    // Output offsets for the current primary target
    float x_offset = 0.0f;
    float y_offset = 0.0f;

    // Last published target state (0=locked, -1=no target)
    int target_state = -1;

private:
    ros::NodeHandle node_;
    ros::NodeHandle pnh_;

    void loadParams();
    void initializeSubscribers();
    void initializePublishers();

    void readStream(const sensor_msgs::ImageConstPtr &img_msg);
    void readRcin(const mavros_msgs::RCIn::ConstPtr &msg);
    void state_cb(const mavros_msgs::State::ConstPtr &msg);
    void readDepth(const sensor_msgs::ImageConstPtr &msg);

    void updateSessionGate();
    void startSession();
    void endSession();

    void processTracking(cv::Mat &img, const std::vector<Detection> &detections);
    void computeOffsets(const cv::Rect &box);
    void drawDetections(cv::Mat &img, const std::vector<Detection> &detections);
    void drawTracks(cv::Mat &img);
    void publishTrackState(int state, float x, float y, int id,
                           float distance_m = 0.0f, int distance_source = 0,
                           bool distance_valid = false);

    bool computeDistance(const cv::Rect &box, double &distance_m, int &source);
    bool depthDistance(const cv::Rect &box, double &distance_m);
    bool bboxDistance(const cv::Rect &box, double &distance_m);

    static cv::Rect clampBox(const cv::Rect &box, int width, int height);

    // Detection parameters
    double object_threshold_ = 0.20;
    double nms_threshold_ = 0.25;
    double draw_confidence_ = 0.50;
    int class_id_ = 0;

    // Person tracker parameters
    double new_track_confidence_ = 0.50;
    double match_confidence_ = 0.20;
    double assoc_iou_threshold_ = 0.30;
    double assoc_center_ratio_ = 0.08;
    double assoc_size_tol_ = 0.60;
    double track_loss_timeout_s_ = 1.50;
    std::string reacquire_mode_ = "new_track";  // new_track | any_visible
    int max_tracks_ = 20;
    // Stricter primary-target gate.
    double primary_iou_min_ = 0.15;
    double primary_center_ratio_ = 0.10;
    double primary_size_tol_ = 0.35;
    double primary_predict_s_ = 0.20;
    double max_predict_speed_ratio_ = 0.50;
    // Re-acquire only near the last primary position/size.
    double reacquire_radius_ratio_ = 0.25;
    double reacquire_size_min_ratio_ = 0.50;
    double reacquire_size_max_ratio_ = 2.00;
    // If no near/size candidate appears, adopt the closest visible person
    // after this many seconds beyond the primary loss timeout. 0 disables.
    double reacquire_fallback_s_ = 1.50;
    // true: IDs only in OFFBOARD + SWA mid (flight).
    // false: IDs in POSCTL + SWA mid (ground bench display only).
    bool require_offboard_ = true;

    // Distance heuristic (pixels), fallback when depth is unavailable
    double standoff_deadband_px_ = 70.0;
    double standoff_full_scale_px_ = 150.0;

    // Depth fusion (Orbbec raw depth, not registered to color)
    ros::Subscriber sub_depth;
    sensor_msgs::ImageConstPtr last_depth_msg_;
    cv::Mat last_depth_;
    ros::Time last_depth_time_;
    bool depth_enabled_ = true;
    double depth_fx_ = 416.2961730957031;
    double depth_fy_ = 416.2961730957031;
    double depth_cx_ = 423.3375244140625;
    double depth_cy_ = 238.01251220703125;
    std::vector<double> depth_R_;           // depth -> color rotation, row-major
    std::vector<double> depth_t_;           // depth -> color translation (m)
    double depth_max_age_s_ = 0.3;
    double depth_min_range_m_ = 0.3;
    double depth_max_range_m_ = 8.0;
    double depth_roi_x_min_ = 0.30;
    double depth_roi_x_max_ = 0.70;
    double depth_roi_y_min_ = 0.25;
    double depth_roi_y_max_ = 0.65;
    int depth_grid_ = 7;
    double depth_min_valid_ratio_ = 0.30;
    double depth_outlier_m_ = 0.5;
    double person_height_m_ = 1.7;
    int distance_filter_frames_ = 5;
    double min_bbox_height_px_ = 80.0;
    std::deque<double> distance_history_;
    int distance_history_id_ = -1;
    float distance_m_ = 0.0f;
    int distance_source_ = 0;               // 0=none, 1=bbox height, 2=depth
    bool distance_valid_ = false;

    // SWA PWM thresholds (up <= up_max <= mid <= mid_max < down)
    int swa_up_max_pwm_ = 1211;
    int swa_mid_max_pwm_ = 1757;

    PersonTracker tracker_;
};

#endif // SAFINA_YOLO_ULTRA_TEST_H
