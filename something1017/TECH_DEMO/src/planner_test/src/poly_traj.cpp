#include <ros/ros.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <visualization_msgs/Marker.h>

#include <vector>
#include <cmath>

class PolyTrajNode
{
public:
    PolyTrajNode(ros::NodeHandle& nh)
    {
        ros::NodeHandle pnh("~");
        pnh.param("input_path_topic",   input_path_topic_,   std::string("astar_path"));
        pnh.param("output_path_topic",  output_path_topic_,  std::string("poly_traj_path"));
        pnh.param("marker_topic",       marker_topic_,       std::string("poly_traj_marker"));
        pnh.param("samples_per_segment",samples_per_segment_,20);
        pnh.param("min_points",         min_points_,         4);
        pnh.param("downsample_step",    downsample_step_,    5);  // 每隔多少个 A* 点取一个

        path_sub_ = nh.subscribe(input_path_topic_, 1, &PolyTrajNode::pathCallback, this);
        path_pub_ = nh.advertise<nav_msgs::Path>(output_path_topic_, 1, true);
        marker_pub_ = nh.advertise<visualization_msgs::Marker>(marker_topic_, 1, true);
    }

private:
    std::string input_path_topic_;
    std::string output_path_topic_;
    std::string marker_topic_;
    int samples_per_segment_;
    int min_points_;
    int downsample_step_;

    ros::Subscriber path_sub_;
    ros::Publisher  path_pub_;
    ros::Publisher  marker_pub_;

    struct Pt2 { double x, y; };

    Pt2 catmullRom(const Pt2& p0, const Pt2& p1,
                   const Pt2& p2, const Pt2& p3,
                   double t) const
    {
        double t2 = t*t;
        double t3 = t2*t;

        double x = 0.5 * ((2.0 * p1.x) +
                          (-p0.x + p2.x) * t +
                          (2.0*p0.x - 5.0*p1.x + 4.0*p2.x - p3.x) * t2 +
                          (-p0.x + 3.0*p1.x - 3.0*p2.x + p3.x) * t3);

        double y = 0.5 * ((2.0 * p1.y) +
                          (-p0.y + p2.y) * t +
                          (2.0*p0.y - 5.0*p1.y + 4.0*p2.y - p3.y) * t2 +
                          (-p0.y + 3.0*p1.y - 3.0*p2.y + p3.y) * t3);

        return {x, y};
    }

    void pathCallback(const nav_msgs::Path::ConstPtr& msg)
    {
        const auto& in_path = *msg;
        size_t n = in_path.poses.size();
        if (n < (size_t)min_points_)
        {
            ROS_WARN("PolyTraj: input path too short (%zu points), skip.", n);
            return;
        }

        // 1) 抽稀原始 A* 路径
        std::vector<Pt2> key_pts;
        key_pts.reserve(n / downsample_step_ + 2);

        // 起点
        key_pts.push_back({in_path.poses.front().pose.position.x,
                           in_path.poses.front().pose.position.y});

        for (size_t i = downsample_step_; i < n - 1; i += downsample_step_)
        {
            Pt2 p;
            p.x = in_path.poses[i].pose.position.x;
            p.y = in_path.poses[i].pose.position.y;
            key_pts.push_back(p);
        }

        // 终点
        key_pts.push_back({in_path.poses.back().pose.position.x,
                           in_path.poses.back().pose.position.y});

        if (key_pts.size() < (size_t)min_points_)
        {
            ROS_WARN("PolyTraj: key points too few (%zu), skip.", key_pts.size());
            return;
        }

        // 2) 用抽稀后的 key_pts 做 Catmull-Rom 样条
        std::vector<Pt2> smooth_pts;

        // 有效段，从 key_pts[0] 到 key_pts[last]
        for (size_t i = 0; i + 3 <= key_pts.size(); ++i)
        {
            const Pt2& p0 = key_pts[i];
            const Pt2& p1 = key_pts[i+1];
            const Pt2& p2 = key_pts[i+2];
            const Pt2& p3 = key_pts[i+3];

            for (int s = 0; s < samples_per_segment_; ++s)
            {
                double t = (double)s / (double)samples_per_segment_;
                Pt2 sp = catmullRom(p0, p1, p2, p3, t);
                smooth_pts.push_back(sp);
            }
        }
        // 补上最后一个关键点（终点）
        smooth_pts.push_back(key_pts.back());

        // 3) 发布平滑 Path
        nav_msgs::Path out_path;
        out_path.header.stamp = ros::Time::now();
        out_path.header.frame_id = in_path.header.frame_id.empty() ? "map" : in_path.header.frame_id;

        for (const auto& p : smooth_pts)
        {
            geometry_msgs::PoseStamped ps;
            ps.header = out_path.header;
            ps.pose.position.x = p.x;
            ps.pose.position.y = p.y;
            ps.pose.position.z = 0.0;
            ps.pose.orientation.w = 1.0;
            out_path.poses.push_back(ps);
        }
        path_pub_.publish(out_path);

        // 4) 发布 Marker
        visualization_msgs::Marker m;
        m.header = out_path.header;
        m.ns = "poly_traj";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_STRIP;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.05;
        m.color.r = 1.0;
        m.color.g = 1.0;
        m.color.b = 0.0;
        m.color.a = 1.0;

        for (const auto& p : smooth_pts)
        {
            geometry_msgs::Point gp;
            gp.x = p.x;
            gp.y = p.y;
            gp.z = 0.02;
            m.points.push_back(gp);
        }
        marker_pub_.publish(m);

        ROS_INFO("PolyTraj: input %zu pts, key %zu pts, output %zu samples.",
                 n, key_pts.size(), smooth_pts.size());
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "poly_traj_node");
    ros::NodeHandle nh;

    PolyTrajNode node(nh);

    ros::spin();
    return 0;
}
