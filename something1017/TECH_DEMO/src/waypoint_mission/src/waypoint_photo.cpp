// Save the latest decoded gimbal-camera frame when the mission node asks.
//
// It subscribes to the image topic published by rtsp_test/rtsp_hw_pub_ultra
// (/rtsp_cam/image_raw, 1920x1080, hardware decoded on RK3588). Trigger payload
// is a filename base without extension; the JPEG is written to base + ".jpg".
// A result string is published on /waypoint_mission/photo_done:
//   "OK <path>" or "FAIL <reason>".
//
// This node never publishes flight setpoints, services, or gimbal commands.

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <std_msgs/String.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>

class PhotoNode {
public:
    PhotoNode() : nh_(), pnh_("~") {
        pnh_.param<std::string>("image_topic", image_topic_, "/rtsp_cam/image_raw");
        pnh_.param<std::string>("trigger_topic", trigger_topic_, "/waypoint_mission/photo_trigger");
        pnh_.param<std::string>("done_topic", done_topic_, "/waypoint_mission/photo_done");
        pnh_.param("max_frame_age", max_frame_age_, 0.6);
        pnh_.param("jpeg_quality", jpeg_quality_, 95);

        image_sub_ = nh_.subscribe(image_topic_, 1, &PhotoNode::image_cb, this);
        trigger_sub_ = nh_.subscribe(trigger_topic_, 5, &PhotoNode::trigger_cb, this);
        done_pub_ = nh_.advertise<std_msgs::String>(done_topic_, 5);

        ROS_INFO("waypoint_photo: image=%s trigger=%s done=%s quality=%d age<=%.2fs",
                 image_topic_.c_str(), trigger_topic_.c_str(), done_topic_.c_str(),
                 jpeg_quality_, max_frame_age_);
    }

private:
    void image_cb(const sensor_msgs::Image::ConstPtr& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_ = msg;
        latest_at_ = ros::Time::now();
        ++frames_;
    }

    void trigger_cb(const std_msgs::String::ConstPtr& msg) {
        const std::string base = msg->data;
        sensor_msgs::ImageConstPtr img;
        ros::Time at;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            img = latest_;
            at = latest_at_;
        }
        std_msgs::String out;
        if (!img) {
            out.data = "FAIL " + base + " no_image_topic_or_no_frame";
            done_pub_.publish(out);
            ROS_WARN("photo trigger rejected: no frame yet");
            return;
        }
        const double age = (ros::Time::now() - at).toSec();
        if (age > max_frame_age_) {
            out.data = "FAIL " + base + " stale_frame age=" + std::to_string(age);
            done_pub_.publish(out);
            ROS_WARN("photo trigger rejected: stale frame %.2fs", age);
            return;
        }

        const std::filesystem::path path(base + ".jpg");
        std::error_code ec;
        if (!path.parent_path().empty())
            std::filesystem::create_directories(path.parent_path(), ec);

        bool ok = false;
        try {
            cv_bridge::CvImageConstPtr cv = cv_bridge::toCvShare(img, "bgr8");
            std::vector<int> params{cv::IMWRITE_JPEG_QUALITY, jpeg_quality_};
            ok = cv::imwrite(path.string(), cv->image, params);
        } catch (const std::exception& e) {
            out.data = std::string("FAIL ") + base + " encode " + e.what();
            done_pub_.publish(out);
            ROS_ERROR("photo encode failed: %s", e.what());
            return;
        }

        if (ok) {
            out.data = "OK " + path.string();
            ROS_INFO("photo saved: %s (frame age %.2fs, %ux%u)", path.string().c_str(), age,
                     img->width, img->height);
        } else {
            out.data = "FAIL " + base + " write " + path.string();
            ROS_ERROR("photo write failed: %s", path.string().c_str());
        }
        done_pub_.publish(out);
    }

    ros::NodeHandle nh_, pnh_;
    ros::Subscriber image_sub_, trigger_sub_;
    ros::Publisher done_pub_;
    std::string image_topic_, trigger_topic_, done_topic_;
    double max_frame_age_ = 0.6;
    int jpeg_quality_ = 95;

    std::mutex mutex_;
    sensor_msgs::ImageConstPtr latest_;
    ros::Time latest_at_;
    std::uint64_t frames_ = 0;
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "waypoint_photo");
    PhotoNode node;
    ros::spin();
    return 0;
}
