#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <visualization_msgs/Marker.h>

#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <algorithm>

struct RRTStarNode
{
    double x;      // 世界坐标
    double y;
    int parent;    // 父节点索引，-1 表示根
    double cost;   // 从起点到该点的累计代价
};

class RRTStarOccupancy
{
public:
    RRTStarOccupancy(ros::NodeHandle& nh)
    {
        ros::NodeHandle pnh("~");
        pnh.param("map_topic", map_topic_, std::string("map"));
        pnh.param("frame_id", frame_id_, std::string("map"));
        pnh.param("occupied_threshold", occupied_thresh_, 50);

        // 默认参数
        pnh.param("max_iterations",  max_iterations_, 12000);
        pnh.param("step_size",       step_size_,      0.6);
        pnh.param("goal_tolerance",  goal_tolerance_, 0.6);
        pnh.param("neighbor_radius", neighbor_radius_,1.5);
        pnh.param("goal_bias",       goal_bias_,      0.3);

        map_sub_   = nh.subscribe(map_topic_, 1, &RRTStarOccupancy::mapCallback, this);
        start_sub_ = nh.subscribe("initialpose", 1, &RRTStarOccupancy::startCallback, this);
        goal_sub_  = nh.subscribe("move_base_simple/goal", 1, &RRTStarOccupancy::goalCallback, this);

        // latched = true
        path_pub_        = nh.advertise<nav_msgs::Path>("rrt_star_path", 1, true);
        tree_pub_        = nh.advertise<visualization_msgs::Marker>("rrt_star_tree", 1, true);
        path_marker_pub_ = nh.advertise<visualization_msgs::Marker>("rrt_star_path_marker", 1, true);
        obstacles_pub_   = nh.advertise<visualization_msgs::Marker>("rrt_star_obstacles", 1, true);

        have_map_ = have_start_ = have_goal_ = false;
    }

private:
    // 参数
    std::string map_topic_;
    std::string frame_id_;
    int occupied_thresh_;
    int max_iterations_;
    double step_size_;
    double goal_tolerance_;
    double neighbor_radius_;
    double goal_bias_;

    // 地图
    nav_msgs::OccupancyGrid map_;
    bool have_map_;

    // 起终点（世界坐标）
    double start_x_, start_y_;
    double goal_x_,  goal_y_;
    bool have_start_, have_goal_;

    // ROS
    ros::Subscriber map_sub_, start_sub_, goal_sub_;
    ros::Publisher path_pub_, tree_pub_, path_marker_pub_, obstacles_pub_;

    // 工具函数
    int index(int mx, int my) const
    {
        return my * map_.info.width + mx;
    }

    bool isInBounds(int mx, int my) const
    {
        return mx >= 0 && my >= 0 &&
               mx < (int)map_.info.width && my < (int)map_.info.height;
    }

    bool isCellFree(int mx, int my) const
    {
        if (!isInBounds(mx, my)) return false;
        int8_t v = map_.data[index(mx, my)];
        if (v < 0) return false;                // unknown 当障碍
        return v < occupied_thresh_;
    }

    bool worldToMap(double wx, double wy, int& mx, int& my) const
    {
        if (!have_map_) return false;
        double ox = map_.info.origin.position.x;
        double oy = map_.info.origin.position.y;
        double res = map_.info.resolution;
        mx = (int)std::floor((wx - ox) / res);
        my = (int)std::floor((wy - oy) / res);
        return isInBounds(mx, my);
    }

    void mapToWorld(int mx, int my, double& wx, double& wy) const
    {
        double ox = map_.info.origin.position.x;
        double oy = map_.info.origin.position.y;
        double res = map_.info.resolution;
        wx = ox + (mx + 0.5) * res;
        wy = oy + (my + 0.5) * res;
    }

    bool isFreeWorld(double wx, double wy) const
    {
        int mx, my;
        if (!worldToMap(wx, wy, mx, my)) return false;
        return isCellFree(mx, my);
    }

    // Bresenham 检测线段碰撞
    bool isSegmentFree(double x1, double y1, double x2, double y2) const
    {
        int mx1, my1, mx2, my2;
        if (!worldToMap(x1, y1, mx1, my1)) return false;
        if (!worldToMap(x2, y2, mx2, my2)) return false;

        int dx = std::abs(mx2 - mx1);
        int dy = std::abs(my2 - my1);
        int sx = (mx1 < mx2) ? 1 : -1;
        int sy = (my1 < my2) ? 1 : -1;
        int err = dx - dy;
        int x = mx1, y = my1;

        while (true)
        {
            if (!isCellFree(x, y)) return false;
            if (x == mx2 && y == my2) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x += sx; }
            if (e2 <  dx) { err += dx; y += sy; }
        }
        return true;
    }

    double dist(double x1, double y1, double x2, double y2) const
    {
        double dx = x1 - x2, dy = y1 - y2;
        return std::sqrt(dx*dx + dy*dy);
    }

    // RRT* 核心
    bool plan(std::vector<std::pair<double,double>>& out_path,
              std::vector<RRTStarNode>& tree)
    {
        out_path.clear();
        tree.clear();

        if (!(have_map_ && have_start_ && have_goal_)) return false;
        if (!isFreeWorld(start_x_, start_y_) || !isFreeWorld(goal_x_, goal_y_))
        {
            ROS_WARN("RRT* start or goal not free.");
            return false;
        }

        // 采样范围：起点-终点矩形附近加 padding，再裁剪到地图边界内
        double padding = 3.0;
        double min_x = std::min(start_x_, goal_x_) - padding;
        double max_x = std::max(start_x_, goal_x_) + padding;
        double min_y = std::min(start_y_, goal_y_) - padding;
        double max_y = std::max(start_y_, goal_y_) + padding;

        double map_min_x = map_.info.origin.position.x;
        double map_min_y = map_.info.origin.position.y;
        double map_max_x = map_min_x + map_.info.width  * map_.info.resolution;
        double map_max_y = map_min_y + map_.info.height * map_.info.resolution;
        min_x = std::max(min_x, map_min_x);
        min_y = std::max(min_y, map_min_y);
        max_x = std::min(max_x, map_max_x);
        max_y = std::min(max_y, map_max_y);

        std::mt19937 gen(12345);
        std::uniform_real_distribution<double> dx(min_x, max_x);
        std::uniform_real_distribution<double> dy(min_y, max_y);
        std::uniform_real_distribution<double> db(0.0, 1.0);  // goal bias

        // 根节点：起点
        RRTStarNode root;
        root.x = start_x_;
        root.y = start_y_;
        root.parent = -1;
        root.cost = 0.0;
        tree.push_back(root);

        int best_goal_idx = -1;
        double best_goal_cost = std::numeric_limits<double>::infinity();

        for (int it = 0; it < max_iterations_; ++it)
        {
            // 1) 采样（带 goal bias）
            double rx, ry;
            if (db(gen) < goal_bias_)
            {
                rx = goal_x_;
                ry = goal_y_;
            }
            else
            {
                rx = dx(gen);
                ry = dy(gen);
            }
            if (!isFreeWorld(rx, ry))
                continue;

            // 2) 最近节点
            int nearest = -1;
            double min_d = std::numeric_limits<double>::infinity();
            for (size_t i = 0; i < tree.size(); ++i)
            {
                double d = dist(tree[i].x, tree[i].y, rx, ry);
                if (d < min_d)
                {
                    min_d = d;
                    nearest = (int)i;
                }
            }
            if (nearest < 0) continue;

            // 3) 从最近节点朝采样点走一步
            double nx, ny;
            if (min_d < step_size_)
            {
                nx = rx;
                ny = ry;
            }
            else
            {
                double vx = rx - tree[nearest].x;
                double vy = ry - tree[nearest].y;
                double norm = std::sqrt(vx*vx + vy*vy);
                vx /= norm;
                vy /= norm;
                nx = tree[nearest].x + vx * step_size_;
                ny = tree[nearest].y + vy * step_size_;
            }

            if (!isFreeWorld(nx, ny)) continue;
            if (!isSegmentFree(tree[nearest].x, tree[nearest].y, nx, ny)) continue;

            // 4) 选择最优父节点
            int best_parent = nearest;
            double best_cost = tree[nearest].cost +
                               dist(tree[nearest].x, tree[nearest].y, nx, ny);

            for (size_t i = 0; i < tree.size(); ++i)
            {
                double d = dist(tree[i].x, tree[i].y, nx, ny);
                if (d > neighbor_radius_) continue;
                if (!isSegmentFree(tree[i].x, tree[i].y, nx, ny)) continue;

                double new_cost = tree[i].cost + d;
                if (new_cost < best_cost)
                {
                    best_cost = new_cost;
                    best_parent = (int)i;
                }
            }

            // 5) 插入新节点
            RRTStarNode new_node;
            new_node.x = nx;
            new_node.y = ny;
            new_node.parent = best_parent;
            new_node.cost = best_cost;
            tree.push_back(new_node);
            int new_idx = (int)tree.size() - 1;

            // 6) rewiring
            for (size_t i = 0; i < tree.size(); ++i)
            {
                if ((int)i == new_idx) continue;
                double d = dist(tree[i].x, tree[i].y, nx, ny);
                if (d > neighbor_radius_) continue;
                if (!isSegmentFree(tree[i].x, tree[i].y, nx, ny)) continue;

                double new_cost = best_cost + d;
                if (new_cost + 1e-6 < tree[i].cost)
                {
                    tree[i].cost = new_cost;
                    tree[i].parent = new_idx;
                }
            }

            // 7) 靠近 goal 则更新最优解
            double dg = dist(nx, ny, goal_x_, goal_y_);
            if (dg < goal_tolerance_ && best_cost < best_goal_cost)
            {
                best_goal_cost = best_cost;
                best_goal_idx = new_idx;
                ROS_INFO("RRT* found better goal: cost=%.3f, it=%d, nodes=%zu",
                         best_goal_cost, it, tree.size());
            }
        }

        if (best_goal_idx < 0)
        {
            ROS_WARN("RRT* failed to reach goal.");
            return false;
        }

        // 回溯：goal_node -> ... -> start
        int cur = best_goal_idx;
        while (cur >= 0)
        {
            out_path.emplace_back(tree[cur].x, tree[cur].y);
            cur = tree[cur].parent;
        }

        // 反转成 start -> ... -> goal_node
        std::reverse(out_path.begin(), out_path.end());
        return true;
    }

    void publishPath(const std::vector<std::pair<double,double>>& path)
    {
        if (!have_map_) return;
        nav_msgs::Path ros_path;
        ros_path.header.stamp = ros::Time::now();
        ros_path.header.frame_id = frame_id_;
        for (auto& p : path)
        {
            geometry_msgs::PoseStamped ps;
            ps.header = ros_path.header;
            ps.pose.position.x = p.first;
            ps.pose.position.y = p.second;
            ps.pose.position.z = 0.0;
            ps.pose.orientation.w = 1.0;
            ros_path.poses.push_back(ps);
        }
        path_pub_.publish(ros_path);
    }

    void publishTree(const std::vector<RRTStarNode>& tree)
    {
        if (!have_map_) return;
        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_star_tree";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_LIST;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.02;
        m.color.r = 0.2;
        m.color.g = 0.2;
        m.color.b = 1.0;
        m.color.a = 0.5;

        for (size_t i = 0; i < tree.size(); ++i)
        {
            if (tree[i].parent < 0) continue;
            geometry_msgs::Point p1, p2;
            p1.x = tree[i].x;              p1.y = tree[i].y;              p1.z = 0.01;
            p2.x = tree[tree[i].parent].x; p2.y = tree[tree[i].parent].y; p2.z = 0.01;
            m.points.push_back(p1);
            m.points.push_back(p2);
        }
        tree_pub_.publish(m);
    }

    void publishPathMarker(const std::vector<std::pair<double,double>>& path)
    {
        if (!have_map_) return;
        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_star_path";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_STRIP;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.05;
        m.color.r = 0.0;
        m.color.g = 1.0;
        m.color.b = 1.0;
        m.color.a = 1.0;

        for (auto& p : path)
        {
            geometry_msgs::Point pt;
            pt.x = p.first;
            pt.y = p.second;
            pt.z = 0.02;
            m.points.push_back(pt);
        }
        path_marker_pub_.publish(m);
    }

    void publishObstacles()
    {
        if (!have_map_) return;
        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_star_obstacles";
        m.id = 0;
        m.type = visualization_msgs::Marker::CUBE_LIST;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = map_.info.resolution;
        m.scale.y = map_.info.resolution;
        m.scale.z = 0.05;
        m.color.r = 1.0;
        m.color.g = 0.0;
        m.color.b = 0.0;
        m.color.a = 0.6;

        for (int y = 0; y < (int)map_.info.height; ++y)
        {
            for (int x = 0; x < (int)map_.info.width; ++x)
            {
                int8_t v = map_.data[index(x, y)];
                if (v >= occupied_thresh_)
                {
                    geometry_msgs::Point p;
                    mapToWorld(x, y, p.x, p.y);
                    p.z = 0.0;
                    m.points.push_back(p);
                }
            }
        }
        obstacles_pub_.publish(m);
    }

    // 回调
    void mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg)
    {
        map_ = *msg;
        have_map_ = true;
        ROS_INFO("Map received: %u x %u, res=%.3f",
                 map_.info.width, map_.info.height, map_.info.resolution);
        publishObstacles();
        tryPlan();
    }

    void startCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg)
    {
        if (!have_map_) { ROS_WARN("No map yet."); return; }
        start_x_ = msg->pose.pose.position.x;
        start_y_ = msg->pose.pose.position.y;
        if (!isFreeWorld(start_x_, start_y_))
        {
            ROS_WARN("RRT* start not free.");
            return;
        }
        have_start_ = true;
        ROS_INFO("RRT* start set: (%.3f, %.3f)", start_x_, start_y_);
        tryPlan();
    }

    void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
    {
        if (!have_map_) { ROS_WARN("No map yet."); return; }
        goal_x_ = msg->pose.position.x;
        goal_y_ = msg->pose.position.y;
        if (!isFreeWorld(goal_x_, goal_y_))
        {
            ROS_WARN("RRT* goal not free.");
            return;
        }
        have_goal_ = true;
        ROS_INFO("RRT* goal set: (%.3f, %.3f)", goal_x_, goal_y_);
        tryPlan();
    }

    void tryPlan()
    {
        if (!(have_map_ && have_start_ && have_goal_)) return;

        std::vector<std::pair<double,double>> path;
        std::vector<RRTStarNode> tree;
        if (plan(path, tree))
        {
            ROS_INFO("RRT* path found, len=%zu, nodes=%zu", path.size(), tree.size());
            publishTree(tree);
            publishPath(path);
            publishPathMarker(path);
        }
        else
        {
            ROS_WARN("RRT* planning failed.");
        }
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "rrt_star_occupancy_node");
    ros::NodeHandle nh;
    RRTStarOccupancy rrt_star(nh);
    ros::spin();
    return 0;
}
