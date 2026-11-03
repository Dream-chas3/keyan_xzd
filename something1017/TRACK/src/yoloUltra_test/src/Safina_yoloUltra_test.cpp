#include "yoloUltra_test/Safina_yoloUltra_test.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

#include "process/postprocess.h"

Safina_YoloUltra::Safina_YoloUltra(ros::NodeHandle node_yoloUltra)
    : node_(node_yoloUltra),
      pnh_("~")
{
    loadParams();
    initializeSubscribers();
    initializePublishers();

    const nn_error_e load_ret = yoloUltra.LoadModel(yoloUltra_model_file.c_str());
    if (load_ret != NN_SUCCESS)
        ROS_ERROR("Failed to load model: %s", yoloUltra_model_file.c_str());

    // Apply detection thresholds read from the YAML file.
    yolo::setObjectThreshold(static_cast<float>(object_threshold_));
    yolo::setNmsThreshold(static_cast<float>(nms_threshold_));

    // Camera intrinsics: K = [fx 0 cx; 0 fy cy; 0 0 1]. These are not used by
    // the current pixel-offset tracking path, but they are correct for any
    // future metric-distance work.
    camera_matrix_ = (cv::Mat_<double>(3, 3) <<
        camera_fx, 0.0, camera_cx,
        0.0, camera_fy, camera_cy,
        0.0, 0.0, 1.0);

    dist_coeffs_ = (cv::Mat_<double>(1, 5) <<
        -0.033186350017786026, 0.03535980358719826, -0.0002135216782335192,
        -0.0001933806634042412, -0.012397058308124542);

    PersonTracker::Params tracker_params;
    tracker_params.new_track_confidence = new_track_confidence_;
    tracker_params.match_confidence = match_confidence_;
    tracker_params.assoc_iou_threshold = assoc_iou_threshold_;
    tracker_params.assoc_center_ratio = assoc_center_ratio_;
    tracker_params.assoc_size_tol = assoc_size_tol_;
    tracker_params.track_loss_timeout_s = track_loss_timeout_s_;
    tracker_params.max_tracks = max_tracks_;
    tracker_params.primary_iou_min = primary_iou_min_;
    tracker_params.primary_center_ratio = primary_center_ratio_;
    tracker_params.primary_size_tol = primary_size_tol_;
    tracker_params.primary_predict_s = primary_predict_s_;
    tracker_params.max_predict_speed_ratio = max_predict_speed_ratio_;
    tracker_ = PersonTracker(tracker_params);

    ROS_INFO("yoloUltra_test ready: model=%s image=%dx%d",
             yoloUltra_model_file.c_str(), img_cols, img_rows);
    ROS_INFO("detection: object_threshold=%.2f nms=%.2f class_id=%d",
             object_threshold_, nms_threshold_, class_id_);
    ROS_INFO("tracker: new_track_conf=%.2f match_conf=%.2f iou=%.2f center_ratio=%.2f loss_timeout=%.1fs reacquire=%s",
             new_track_confidence_, match_confidence_, assoc_iou_threshold_,
             assoc_center_ratio_, track_loss_timeout_s_, reacquire_mode_.c_str());
    ROS_INFO("primary gate: center_ratio=%.2f size_tol=%.2f iou=%.2f predict=%.2fs reacquire_radius=%.2f",
             primary_center_ratio_, primary_size_tol_, primary_iou_min_,
             primary_predict_s_, reacquire_radius_ratio_);
    if (!require_offboard_)
        ROS_WARN("BENCH MODE: tracker/require_offboard=false. IDs are assigned in "
                 "POSCTL + SWA mid for ground display. This node has no flight-control output.");
    ROS_INFO("distance: depth_enabled=%d person_height=%.2f m filter_frames=%d",
             depth_enabled_ ? 1 : 0, person_height_m_, distance_filter_frames_);
}

Safina_YoloUltra::~Safina_YoloUltra() {}

void Safina_YoloUltra::loadParams()
{
    pnh_.param<std::string>("model_file", yoloUltra_model_file,
        "/home/orangepi/something1017/TRACK/src/yoloUltra_test/weights/yolov11_s_det_int.rknn");

    pnh_.param<int>("camera_rows", img_rows, 720);
    pnh_.param<int>("camera_cols", img_cols, 1280);

    double fx = camera_fx, fy = camera_fy, cx = camera_cx, cy = camera_cy;
    pnh_.param<double>("camera_fx", fx, fx);
    pnh_.param<double>("camera_fy", fy, fy);
    pnh_.param<double>("camera_cx", cx, cx);
    pnh_.param<double>("camera_cy", cy, cy);
    camera_fx = static_cast<float>(fx);
    camera_fy = static_cast<float>(fy);
    camera_cx = static_cast<float>(cx);
    camera_cy = static_cast<float>(cy);

    pnh_.param<double>("detection/object_threshold", object_threshold_, 0.20);
    pnh_.param<double>("detection/nms_threshold", nms_threshold_, 0.25);
    pnh_.param<double>("detection/draw_confidence", draw_confidence_, 0.50);
    pnh_.param<int>("detection/class_id", class_id_, 0);

    pnh_.param<double>("tracker/new_track_confidence", new_track_confidence_, 0.50);
    pnh_.param<double>("tracker/match_confidence", match_confidence_, 0.20);
    pnh_.param<double>("tracker/assoc_iou_threshold", assoc_iou_threshold_, 0.30);
    pnh_.param<double>("tracker/assoc_center_ratio", assoc_center_ratio_, 0.08);
    pnh_.param<double>("tracker/assoc_size_tol", assoc_size_tol_, 0.60);
    pnh_.param<double>("tracker/track_loss_timeout_s", track_loss_timeout_s_, 1.50);
    pnh_.param<std::string>("tracker/reacquire_mode", reacquire_mode_, "new_track");
    pnh_.param<int>("tracker/max_tracks", max_tracks_, 20);
    pnh_.param<bool>("tracker/require_offboard", require_offboard_, true);
    pnh_.param<double>("tracker/primary_iou_min", primary_iou_min_, 0.15);
    pnh_.param<double>("tracker/primary_center_ratio", primary_center_ratio_, 0.10);
    pnh_.param<double>("tracker/primary_size_tol", primary_size_tol_, 0.35);
    pnh_.param<double>("tracker/primary_predict_s", primary_predict_s_, 0.20);
    pnh_.param<double>("tracker/max_predict_speed_ratio", max_predict_speed_ratio_, 0.50);
    pnh_.param<double>("tracker/reacquire_radius_ratio", reacquire_radius_ratio_, 0.25);
    pnh_.param<double>("tracker/reacquire_size_min_ratio", reacquire_size_min_ratio_, 0.50);
    pnh_.param<double>("tracker/reacquire_size_max_ratio", reacquire_size_max_ratio_, 2.00);
    pnh_.param<double>("tracker/reacquire_fallback_s", reacquire_fallback_s_, 1.50);

    pnh_.param<double>("offset/standoff_deadband_px", standoff_deadband_px_, 70.0);
    pnh_.param<double>("offset/standoff_full_scale_px", standoff_full_scale_px_, 150.0);

    pnh_.param<bool>("depth/enabled", depth_enabled_, true);
    pnh_.param<double>("depth/fx", depth_fx_, depth_fx_);
    pnh_.param<double>("depth/fy", depth_fy_, depth_fy_);
    pnh_.param<double>("depth/cx", depth_cx_, depth_cx_);
    pnh_.param<double>("depth/cy", depth_cy_, depth_cy_);
    pnh_.param<double>("depth/max_age_s", depth_max_age_s_, 0.30);
    pnh_.param<double>("depth/min_range_m", depth_min_range_m_, 0.30);
    pnh_.param<double>("depth/max_range_m", depth_max_range_m_, 8.0);
    pnh_.param<double>("depth/roi_x_min", depth_roi_x_min_, 0.30);
    pnh_.param<double>("depth/roi_x_max", depth_roi_x_max_, 0.70);
    pnh_.param<double>("depth/roi_y_min", depth_roi_y_min_, 0.25);
    pnh_.param<double>("depth/roi_y_max", depth_roi_y_max_, 0.65);
    pnh_.param<int>("depth/grid", depth_grid_, 7);
    pnh_.param<double>("depth/min_valid_ratio", depth_min_valid_ratio_, 0.30);
    pnh_.param<double>("depth/outlier_m", depth_outlier_m_, 0.50);

    pnh_.param<double>("distance/person_height_m", person_height_m_, 1.70);
    pnh_.param<int>("distance/filter_frames", distance_filter_frames_, 5);
    pnh_.param<double>("distance/min_bbox_height_px", min_bbox_height_px_, 80.0);

    std::vector<double> rotation, translation;
    if (pnh_.getParam("depth/extrinsic_rotation", rotation) && rotation.size() == 9)
        depth_R_ = rotation;
    else
    {
        depth_enabled_ = false;
        ROS_WARN("depth fusion disabled: depth/extrinsic_rotation must be a 9-element list");
    }
    if (pnh_.getParam("depth/extrinsic_translation", translation) && translation.size() == 3)
        depth_t_ = translation;
    else
    {
        depth_enabled_ = false;
        ROS_WARN("depth fusion disabled: depth/extrinsic_translation must be a 3-element list");
    }

    pnh_.param<int>("swa/up_max_pwm", swa_up_max_pwm_, 1211);
    pnh_.param<int>("swa/mid_max_pwm", swa_mid_max_pwm_, 1757);

    // ---- validation ----
    if (object_threshold_ < 0.0 || object_threshold_ > 1.0)
        object_threshold_ = 0.20;
    if (nms_threshold_ < 0.0 || nms_threshold_ > 1.0)
        nms_threshold_ = 0.25;
    if (draw_confidence_ < 0.0 || draw_confidence_ > 1.0)
        draw_confidence_ = 0.50;
    if (new_track_confidence_ < match_confidence_)
        new_track_confidence_ = match_confidence_;
    if (standoff_full_scale_px_ < 1.0)
        standoff_full_scale_px_ = 1.0;
    if (track_loss_timeout_s_ <= 0.0)
        track_loss_timeout_s_ = 1.50;
    if (assoc_size_tol_ < 0.0)
        assoc_size_tol_ = 0.0;
    if (assoc_size_tol_ > 1.0)
        assoc_size_tol_ = 1.0;
    if (primary_iou_min_ < 0.0)
        primary_iou_min_ = 0.0;
    if (primary_iou_min_ > 1.0)
        primary_iou_min_ = 1.0;
    if (primary_center_ratio_ <= 0.0)
        primary_center_ratio_ = 0.10;
    if (primary_size_tol_ < 0.0)
        primary_size_tol_ = 0.0;
    if (primary_size_tol_ > 1.0)
        primary_size_tol_ = 1.0;
    if (primary_predict_s_ < 0.0)
        primary_predict_s_ = 0.0;
    if (primary_predict_s_ > 2.0)
        primary_predict_s_ = 2.0;
    if (max_predict_speed_ratio_ <= 0.0)
        max_predict_speed_ratio_ = 0.50;
    if (reacquire_radius_ratio_ <= 0.0)
        reacquire_radius_ratio_ = 0.25;
    if (reacquire_size_min_ratio_ <= 0.0)
        reacquire_size_min_ratio_ = 0.50;
    if (reacquire_size_max_ratio_ < reacquire_size_min_ratio_)
        reacquire_size_max_ratio_ = 2.00;
    if (reacquire_fallback_s_ < 0.0)
        reacquire_fallback_s_ = 0.0;
    if (max_tracks_ < 1)
        max_tracks_ = 1;
    if (reacquire_mode_ != "new_track" && reacquire_mode_ != "any_visible")
    {
        ROS_WARN("Unknown tracker/reacquire_mode '%s', using 'new_track'", reacquire_mode_.c_str());
        reacquire_mode_ = "new_track";
    }
    if (swa_up_max_pwm_ >= swa_mid_max_pwm_)
    {
        ROS_WARN("SWA thresholds invalid (up_max=%d mid_max=%d), using 1211/1757",
                 swa_up_max_pwm_, swa_mid_max_pwm_);
        swa_up_max_pwm_ = 1211;
        swa_mid_max_pwm_ = 1757;
    }

    if (depth_grid_ < 3)
        depth_grid_ = 3;
    if (depth_grid_ > 15)
        depth_grid_ = 15;
    if (depth_grid_ % 2 == 0)
        depth_grid_ += 1;
    if (depth_max_age_s_ <= 0.0)
        depth_max_age_s_ = 0.30;
    if (depth_min_range_m_ < 0.10)
        depth_min_range_m_ = 0.10;
    if (depth_max_range_m_ <= depth_min_range_m_)
        depth_max_range_m_ = depth_min_range_m_ + 1.0;
    if (depth_roi_x_min_ < 0.0)
        depth_roi_x_min_ = 0.0;
    if (depth_roi_x_max_ > 1.0)
        depth_roi_x_max_ = 1.0;
    if (depth_roi_x_min_ >= depth_roi_x_max_)
    {
        depth_roi_x_min_ = 0.30;
        depth_roi_x_max_ = 0.70;
    }
    if (depth_roi_y_min_ < 0.0)
        depth_roi_y_min_ = 0.0;
    if (depth_roi_y_max_ > 1.0)
        depth_roi_y_max_ = 1.0;
    if (depth_roi_y_min_ >= depth_roi_y_max_)
    {
        depth_roi_y_min_ = 0.25;
        depth_roi_y_max_ = 0.65;
    }
    if (depth_min_valid_ratio_ < 0.0)
        depth_min_valid_ratio_ = 0.0;
    if (depth_min_valid_ratio_ > 1.0)
        depth_min_valid_ratio_ = 1.0;
    if (depth_outlier_m_ <= 0.0)
        depth_outlier_m_ = 0.50;
    if (person_height_m_ < 0.5)
        person_height_m_ = 1.70;
    if (distance_filter_frames_ < 1)
        distance_filter_frames_ = 5;
    if (min_bbox_height_px_ < 10.0)
        min_bbox_height_px_ = 80.0;
    if (depth_fx_ <= 0.0 || depth_fy_ <= 0.0)
    {
        depth_enabled_ = false;
        ROS_WARN("depth fusion disabled: invalid depth intrinsics");
    }
}

void Safina_YoloUltra::initializeSubscribers()
{
    sub_img = node_.subscribe("/camera/color/image_raw", 1,
                              &Safina_YoloUltra::readStream, this);
    sub_Rcin = node_.subscribe("/mavros/rc/in", 10,
                               &Safina_YoloUltra::readRcin, this);
    sub_state = node_.subscribe("/mavros/state", 10,
                                &Safina_YoloUltra::state_cb, this);
    sub_depth = node_.subscribe("/camera/depth/image_raw", 1,
                                &Safina_YoloUltra::readDepth, this);
}

void Safina_YoloUltra::initializePublishers()
{
    pub_yoloUltra_imgresult = node_.advertise<sensor_msgs::Image>("/yoloUltra_imgresult", 1);
    pub_track = node_.advertise<std_msgs::Float32MultiArray>("/Safina_track", 1);
    pub_target_id = node_.advertise<std_msgs::Int32>("/Safina_target_id", 1);
}

void Safina_YoloUltra::readRcin(const mavros_msgs::RCIn::ConstPtr &msg)
{
    if (msg->channels.size() <= 8)
        return;

    const int pwm = msg->channels[8];
    int new_state;
    if (pwm <= swa_up_max_pwm_)
        new_state = 0;       // up   -> protection / no tracking session
    else if (pwm <= swa_mid_max_pwm_)
        new_state = 1;       // mid  -> follow
    else
        new_state = 2;       // down -> land

    if (new_state != rc_state)
    {
        rc_state = new_state;
        updateSessionGate();
    }
}

void Safina_YoloUltra::state_cb(const mavros_msgs::State::ConstPtr &msg)
{
    const bool new_offboard = (msg->mode == "OFFBOARD");
    if (new_offboard != offboard_mode)
    {
        const bool entering_offboard = new_offboard && !offboard_mode;
        offboard_mode = new_offboard;

        if (!require_offboard_ && entering_offboard && tracking_session_active)
        {
            // Bench mode was displaying IDs in POSCTL. Restart the ID session
            // when the real flight starts, so the flight target selection is
            // identical to the normal OFFBOARD + SWA mid workflow.
            endSession();
            session_latched = (rc_state == 1);
            ROS_INFO("Bench mode: entering OFFBOARD, restarting ID session for flight");
        }
        updateSessionGate();
    }
}

void Safina_YoloUltra::readDepth(const sensor_msgs::ImageConstPtr &msg)
{
    try
    {
        last_depth_ = cv_bridge::toCvShare(msg, "16UC1")->image;
        last_depth_msg_ = msg;
        last_depth_time_ = ros::Time::now();
    }
    catch (const cv_bridge::Exception &e)
    {
        ROS_WARN_THROTTLE(2.0, "depth image conversion failed: %s", e.what());
    }
}

void Safina_YoloUltra::updateSessionGate()
{
    // Flight mode: IDs only while PX4 OFFBOARD + SWA mid.
    // Bench mode (require_offboard=false): IDs while SWA mid regardless of
    // PX4 mode, for ground display only. This node never publishes setpoints.
    const bool blocked = (rc_state == 2) || (require_offboard_ && !offboard_mode);
    if (blocked)
    {
        session_latched = false;
        if (tracking_session_active)
            endSession();
        return;
    }

    if (rc_state == 1)
        session_latched = true;

    if (session_latched && !tracking_session_active)
        startSession();
    else if (!session_latched && tracking_session_active)
        endSession();
}

void Safina_YoloUltra::startSession()
{
    tracker_.reset();
    primary_id = -1;
    waiting_for_new_primary = false;
    primary_last_seen_s = 0.0;
    primary_lost_time_s = 0.0;
    primary_last_center_x = -1.0;
    primary_last_center_y = -1.0;
    primary_last_height = 0.0;
    primary_last_velocity_x = 0.0;
    primary_last_velocity_y = 0.0;
    tracking_session_active = true;
    x_offset = 0.0f;
    y_offset = 0.0f;
    target_state = -1;
    distance_history_.clear();
    distance_history_id_ = -1;
    distance_m_ = 0.0f;
    distance_source_ = 0;
    distance_valid_ = false;
    ROS_INFO("Person tracking session START (OFFBOARD + SWA mid), IDs start at 1");
}

void Safina_YoloUltra::endSession()
{
    tracker_.reset();
    primary_id = -1;
    waiting_for_new_primary = false;
    primary_last_seen_s = 0.0;
    primary_lost_time_s = 0.0;
    primary_last_center_x = -1.0;
    primary_last_center_y = -1.0;
    primary_last_height = 0.0;
    primary_last_velocity_x = 0.0;
    primary_last_velocity_y = 0.0;
    tracking_session_active = false;
    x_offset = 0.0f;
    y_offset = 0.0f;
    target_state = -1;
    distance_history_.clear();
    distance_history_id_ = -1;
    distance_m_ = 0.0f;
    distance_source_ = 0;
    distance_valid_ = false;
    ROS_INFO("Person tracking session END");
}

void Safina_YoloUltra::readStream(const sensor_msgs::ImageConstPtr &img_msg)
{
    cv::Mat img = cv_bridge::toCvShare(img_msg, "bgr8")->image;

    std::vector<Detection> detections;
    yoloUltra.Run(img, detections);

    drawDetections(img, detections);

    if (tracking_session_active)
    {
        processTracking(img, detections);
    }
    else
    {
        if (target_state != -1)
        {
            target_state = -1;
            x_offset = 0.0f;
            y_offset = 0.0f;
        }
        publishTrackState(-1, 0.0f, 0.0f, -1);
    }

    pub_yoloUltra_imgresult.publish(
        cv_bridge::CvImage(std_msgs::Header(), "bgr8", img).toImageMsg());
}

void Safina_YoloUltra::processTracking(cv::Mat &img, const std::vector<Detection> &detections)
{
    std::vector<PersonDetection> person_detections;
    person_detections.reserve(detections.size());
    for (const auto &det : detections)
    {
        if (det.class_id != class_id_)
            continue;
        if (det.confidence < static_cast<float>(match_confidence_))
            continue;
        PersonDetection pd;
        pd.box = clampBox(det.box, img_cols, img_rows);
        pd.confidence = det.confidence;
        person_detections.push_back(pd);
    }

    const double now = ros::Time::now().toSec();
    tracker_.update(person_detections, now, img_cols, img_rows);

    // ---- primary target selection ----
    // Detect a removed primary first, so a person who appears in the same
    // frame as the timeout can be adopted immediately.
    if (primary_id != -1 && tracker_.find(primary_id) == nullptr)
    {
        ROS_WARN("Primary target ID=%d lost; holding and waiting for a new person", primary_id);
        primary_lost_time_s = primary_last_seen_s;
        primary_id = -1;
        waiting_for_new_primary = true;
    }

    if (primary_id == -1)
    {
        const PersonTrack *best = nullptr;
        if (!waiting_for_new_primary)
        {
            // First person of this session: the lowest ID is the first person
            // that appeared (IDs are assigned in appearance order).
            for (const auto &track : tracker_.tracks())
            {
                if (!track.matched_this_frame)
                    continue;
                if (best == nullptr || track.id < best->id)
                    best = &track;
            }
        }
        else
        {
            // The old primary left. By default only a new person near the
            // predicted last primary position AND with a similar box size can
            // become the new primary, so a passer-by elsewhere is ignored.
            const double diag = std::hypot(static_cast<double>(img_cols),
                                           static_cast<double>(img_rows));
            const double elapsed = std::max(0.0, now - primary_last_seen_s);
            const double horizon = std::min(elapsed, 1.5);
            const double predicted_x = primary_last_center_x +
                                       primary_last_velocity_x * horizon;
            const double predicted_y = primary_last_center_y +
                                       primary_last_velocity_y * horizon;

            for (const auto &track : tracker_.tracks())
            {
                bool eligible = false;
                if (reacquire_mode_ == "any_visible")
                {
                    eligible = track.matched_this_frame;
                }
                else
                {
                    const bool size_ok =
                        primary_last_height > 0.0 &&
                        track.box.height >= reacquire_size_min_ratio_ * primary_last_height &&
                        track.box.height <= reacquire_size_max_ratio_ * primary_last_height;
                    const bool near =
                        primary_last_center_x >= 0.0 && size_ok &&
                        std::hypot(track.center.x - predicted_x,
                                   track.center.y - predicted_y) <=
                            reacquire_radius_ratio_ * diag;
                    eligible = track.matched_this_frame &&
                               track.first_seen > primary_lost_time_s && near;
                }
                if (!eligible)
                    continue;
                if (best == nullptr || track.id < best->id)
                    best = &track;
            }

            // Fallback: if no near/size candidate appears after the loss
            // timeout plus a grace period, do not wait forever while a person
            // is visible. Pick the visible track closest to the predicted last
            // position and size (or the most confident one if no position was
            // recorded).
            if (best == nullptr && reacquire_fallback_s_ > 0.0 &&
                (now - primary_lost_time_s) >=
                    track_loss_timeout_s_ + reacquire_fallback_s_)
            {
                double best_score = std::numeric_limits<double>::infinity();
                for (const auto &track : tracker_.tracks())
                {
                    if (!track.matched_this_frame)
                        continue;

                    double score = 0.0;
                    if (primary_last_center_x >= 0.0)
                    {
                        const double dist = std::hypot(track.center.x - predicted_x,
                                                       track.center.y - predicted_y);
                        double size_penalty = 0.0;
                        if (primary_last_height > 0.0 && track.box.height > 0)
                            size_penalty = std::fabs(std::log(
                                static_cast<double>(track.box.height) / primary_last_height));
                        score = dist + 0.5 * diag * size_penalty;
                    }
                    else
                    {
                        score = -track.confidence;
                    }

                    if (score < best_score)
                    {
                        best_score = score;
                        best = &track;
                    }
                }

                if (best != nullptr)
                    ROS_INFO("Reacquire fallback: adopting visible ID=%d after %.1fs",
                             best->id, now - primary_lost_time_s);
            }

            if (best == nullptr)
                ROS_WARN_THROTTLE(1.0,
                    "Waiting for the original person near the last position/size");
        }

        if (best != nullptr)
        {
            primary_id = best->id;
            waiting_for_new_primary = false;
            ROS_INFO("Primary target acquired: ID=%d", primary_id);
        }
    }

    tracker_.setPrimary(primary_id);

    // ---- publish offsets for the primary target ----
    bool visible = false;
    cv::Rect target_box;
    if (primary_id != -1)
    {
        const PersonTrack *track = tracker_.find(primary_id);
        if (track != nullptr && track->matched_this_frame)
        {
            visible = true;
            target_box = track->box;
        }
    }

    if (visible)
    {
        primary_last_seen_s = now;
        primary_last_center_x = target_box.x + target_box.width * 0.5;
        primary_last_center_y = target_box.y + target_box.height * 0.5;
        primary_last_height = target_box.height;
        const PersonTrack *primary_track = tracker_.find(primary_id);
        if (primary_track != nullptr)
        {
            primary_last_velocity_x = primary_track->velocity.x;
            primary_last_velocity_y = primary_track->velocity.y;
        }
        computeOffsets(target_box);
        double dist = 0.0;
        int source = 0;
        const bool dist_ok = computeDistance(target_box, dist, source);
        distance_m_ = dist_ok ? static_cast<float>(dist) : 0.0f;
        distance_source_ = source;
        distance_valid_ = dist_ok;
        target_state = 0;
        publishTrackState(0, x_offset, y_offset, primary_id,
                          distance_m_, distance_source_, distance_valid_);
        ROS_INFO_THROTTLE(1.0,
            "primary ID=%d x_off=%.2f y_off=%.2f distance=%.2f m source=%s",
            primary_id, x_offset, y_offset, distance_m_,
            distance_source_ == 2 ? "depth" : (distance_source_ == 1 ? "bbox" : "none"));
    }
    else
    {
        target_state = -1;
        distance_m_ = 0.0f;
        distance_source_ = 0;
        distance_valid_ = false;
        publishTrackState(-1, 0.0f, 0.0f, primary_id, 0.0f, 0, false);
    }

    drawTracks(img);
}

void Safina_YoloUltra::computeOffsets(const cv::Rect &box)
{
    const float center_x = box.x + box.width * 0.5f;
    // Distance from the image bottom to the feet. This is the same heuristic
    // the original demo used; it is not a metric distance.
    const float bottom = static_cast<float>(img_rows - (box.y + box.height));
    const float y_px = static_cast<float>(img_cols) * 0.5f - center_x;
    const float radial = std::sqrt(y_px * y_px + bottom * bottom);

    float x_norm = (radial - standoff_deadband_px_) / standoff_full_scale_px_;
    x_norm = std::max(-1.0f, std::min(1.0f, x_norm));

    x_offset = x_norm;
    y_offset = y_px / (static_cast<float>(img_cols) * 0.5f);
}

bool Safina_YoloUltra::bboxDistance(const cv::Rect &box, double &distance_m)
{
    const cv::Rect b = clampBox(box, img_cols, img_rows);
    if (b.height < min_bbox_height_px_)
        return false;
    // Reject boxes cut off by the image border: the height would be wrong.
    if (b.y <= 4 || b.y + b.height >= img_rows - 4)
        return false;

    const double d = static_cast<double>(camera_fy) * person_height_m_ /
                     static_cast<double>(b.height);
    if (d < depth_min_range_m_ || d > depth_max_range_m_)
        return false;
    distance_m = d;
    return true;
}

bool Safina_YoloUltra::depthDistance(const cv::Rect &box, double &distance_m)
{
    if (!depth_enabled_ || last_depth_.empty() || !last_depth_msg_)
        return false;
    if ((ros::Time::now() - last_depth_time_).toSec() > depth_max_age_s_)
        return false;
    if (last_depth_.type() != CV_16UC1 || last_depth_.cols < 2 || last_depth_.rows < 2)
        return false;
    if (depth_R_.size() != 9 || depth_t_.size() != 3)
        return false;

    const cv::Rect b = clampBox(box, img_cols, img_rows);
    if (b.width < 2 || b.height < 2)
        return false;

    // Sample the torso region, not the whole box, to avoid background pixels.
    const double x0 = b.x + depth_roi_x_min_ * b.width;
    const double x1 = b.x + depth_roi_x_max_ * b.width;
    const double y0 = b.y + depth_roi_y_min_ * b.height;
    const double y1 = b.y + depth_roi_y_max_ * b.height;

    const int n = depth_grid_;
    std::vector<double> samples;
    samples.reserve(n * n);

    for (int iy = 0; iy < n; ++iy)
    {
        const double v = (n == 1) ? (y0 + y1) * 0.5
                                  : y0 + (y1 - y0) * static_cast<double>(iy) / (n - 1);
        for (int ix = 0; ix < n; ++ix)
        {
            const double u = (n == 1) ? (x0 + x1) * 0.5
                                      : x0 + (x1 - x0) * static_cast<double>(ix) / (n - 1);

            // Color camera ray, then rotate into the depth camera frame.
            const double dx = (u - camera_cx) / camera_fx;
            const double dy = (v - camera_cy) / camera_fy;
            const double ddx = depth_R_[0] * dx + depth_R_[3] * dy + depth_R_[6];
            const double ddy = depth_R_[1] * dx + depth_R_[4] * dy + depth_R_[7];
            const double ddz = depth_R_[2] * dx + depth_R_[5] * dy + depth_R_[8];
            if (std::fabs(ddz) < 1e-6)
                continue;

            const double ud = depth_fx_ * ddx / ddz + depth_cx_;
            const double vd = depth_fy_ * ddy / ddz + depth_cy_;
            if (ud < 0.0 || vd < 0.0 ||
                ud >= last_depth_.cols - 1 || vd >= last_depth_.rows - 1)
                continue;

            const uint16_t raw = last_depth_.at<uint16_t>(
                static_cast<int>(vd + 0.5), static_cast<int>(ud + 0.5));
            if (raw == 0)
                continue;

            const double z = raw * 0.001;  // mm -> m
            if (z < depth_min_range_m_ || z > depth_max_range_m_)
                continue;
            samples.push_back(z);
        }
    }

    const size_t min_samples = static_cast<size_t>(
        std::max(1.0, depth_min_valid_ratio_ * n * n));
    if (samples.size() < min_samples)
        return false;

    std::sort(samples.begin(), samples.end());
    const double median = samples[samples.size() / 2];

    // Reject depth pixels that are far from the median (background/edges).
    std::vector<double> kept;
    kept.reserve(samples.size());
    for (double z : samples)
        if (std::fabs(z - median) <= depth_outlier_m_)
            kept.push_back(z);
    if (kept.size() * 2 < samples.size())
        kept.swap(samples);

    std::sort(kept.begin(), kept.end());
    const double result = kept[kept.size() / 2];
    if (result < depth_min_range_m_ || result > depth_max_range_m_)
        return false;

    distance_m = result;
    return true;
}

bool Safina_YoloUltra::computeDistance(const cv::Rect &box, double &distance_m, int &source)
{
    if (primary_id != distance_history_id_)
    {
        distance_history_.clear();
        distance_history_id_ = primary_id;
    }

    double raw = 0.0;
    if (depthDistance(box, raw))
        source = 2;  // depth
    else if (bboxDistance(box, raw))
        source = 1;  // bbox height fallback
    else
    {
        distance_m = 0.0;
        source = 0;
        return false;
    }

    distance_history_.push_back(raw);
    while (static_cast<int>(distance_history_.size()) > distance_filter_frames_)
        distance_history_.pop_front();

    std::vector<double> values(distance_history_.begin(), distance_history_.end());
    std::sort(values.begin(), values.end());
    distance_m = values[values.size() / 2];
    return true;
}

void Safina_YoloUltra::drawDetections(cv::Mat &img, const std::vector<Detection> &detections)
{
    for (const auto &det : detections)
    {
        if (det.class_id != class_id_)
            continue;
        if (det.confidence < static_cast<float>(draw_confidence_))
            continue;

        cv::rectangle(img, det.box, cv::Scalar(0, 255, 0), 1);
        char text[64];
        std::snprintf(text, sizeof(text), "person %.2f", det.confidence);
        cv::putText(img, text, cv::Point(det.box.x, std::max(15, det.box.y - 5)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    }
}

void Safina_YoloUltra::drawTracks(cv::Mat &img)
{
    for (const auto &track : tracker_.tracks())
    {
        if (!track.matched_this_frame)
            continue;

        const bool is_primary = (track.id == primary_id);
        const cv::Scalar color = is_primary ? cv::Scalar(0, 255, 255) : cv::Scalar(255, 128, 0);
        const int thickness = is_primary ? 3 : 1;

        cv::rectangle(img, track.box, color, thickness);
        char text[64];
        std::snprintf(text, sizeof(text), "ID:%d %.2f", track.id, track.confidence);
        cv::putText(img, text, cv::Point(track.box.x, std::max(15, track.box.y - 8)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2, cv::LINE_AA);
    }
}

void Safina_YoloUltra::publishTrackState(int state, float x, float y, int id,
                                         float distance_m, int distance_source,
                                         bool distance_valid)
{
    std_msgs::Float32MultiArray msg;
    msg.data.push_back(static_cast<float>(state));
    msg.data.push_back(x);
    msg.data.push_back(y);
    msg.data.push_back(static_cast<float>(id));
    msg.data.push_back(distance_m);
    msg.data.push_back(static_cast<float>(distance_source));
    msg.data.push_back(distance_valid ? 1.0f : 0.0f);
    pub_track.publish(msg);

    std_msgs::Int32 id_msg;
    id_msg.data = id;
    pub_target_id.publish(id_msg);
}

cv::Rect Safina_YoloUltra::clampBox(const cv::Rect &box, int width, int height)
{
    const int x1 = std::max(0, std::min(box.x, width - 1));
    const int y1 = std::max(0, std::min(box.y, height - 1));
    const int x2 = std::max(x1 + 1, std::min(box.x + box.width, width));
    const int y2 = std::max(y1 + 1, std::min(box.y + box.height, height));
    return cv::Rect(x1, y1, x2 - x1, y2 - y1);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "yoloUltra");
    ros::NodeHandle node_yoloUltra;

    Safina_YoloUltra safina_yoloUltra(node_yoloUltra);

    ROS_INFO("yoloUltra_test running ...");

    ros::spin();
    return 0;
}
