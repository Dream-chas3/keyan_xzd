#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <visualization_msgs/Marker.h>

#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>

struct RRTNode
{
    double x;     // 世界坐标
    double y;
    int parent;   // 在节点数组中的父节点索引，-1 表示根
};

class RRTOccupancy
{
public:
    RRTOccupancy(ros::NodeHandle& nh)
    {
        ros::NodeHandle pnh("~");
        pnh.param("map_topic", map_topic_, std::string("map"));
        pnh.param("frame_id", frame_id_, std::string("map"));
        pnh.param("occupied_threshold", occupied_thresh_, 50);
        pnh.param("max_iterations", max_iterations_, 3000);
        pnh.param("step_size", step_size_, 0.3);          // 每次扩展步长（米）
        pnh.param("goal_tolerance", goal_tolerance_, 0.3);// 到终点判定阈值（米）

        map_sub_   = nh.subscribe(map_topic_, 1, &RRTOccupancy::mapCallback, this);
        start_sub_ = nh.subscribe("initialpose", 1, &RRTOccupancy::startCallback, this);
        goal_sub_  = nh.subscribe("move_base_simple/goal", 1, &RRTOccupancy::goalCallback, this);

        path_pub_       = nh.advertise<nav_msgs::Path>("rrt_path", 1,true);
        tree_pub_       = nh.advertise<visualization_msgs::Marker>("rrt_tree", 1,true);
        path_marker_pub_= nh.advertise<visualization_msgs::Marker>("rrt_path_marker", 1,true);
        obstacles_pub_  = nh.advertise<visualization_msgs::Marker>("rrt_obstacles", 1,true);

        have_map_   = false;
        have_start_ = false;
        have_goal_  = false;
    }

private:
    // 配置
    std::string map_topic_;
    std::string frame_id_;
    int occupied_thresh_;
    int max_iterations_;
    double step_size_;
    double goal_tolerance_;

    // 地图
    nav_msgs::OccupancyGrid map_;
    bool have_map_;

    // 起终点（世界坐标）
    double start_x_, start_y_;
    double goal_x_,  goal_y_;
    bool have_start_, have_goal_;

    // ROS
    ros::Subscriber map_sub_;
    ros::Subscriber start_sub_;
    ros::Subscriber goal_sub_;
    ros::Publisher path_pub_;
    ros::Publisher tree_pub_;
    ros::Publisher path_marker_pub_;
    ros::Publisher obstacles_pub_;

    // 工具函数
    int index(int mx, int my) const
    {
        return my * map_.info.width + mx;
    }

    bool isInBoundsMap(int mx, int my) const
    {
        return mx >= 0 && my >= 0 &&
               mx < (int)map_.info.width && my < (int)map_.info.height;
    }

    bool isCellFree(int mx, int my) const
    {
        if (!isInBoundsMap(mx, my)) return false;
        int8_t v = map_.data[index(mx, my)];
        if (v < 0) return false;                // unknown 当作障碍
        return v < occupied_thresh_;
    }

    bool worldToMap(double wx, double wy, int& mx, int& my) const
    {
        if (!have_map_) return false;
        double origin_x = map_.info.origin.position.x;
        double origin_y = map_.info.origin.position.y;
        double res      = map_.info.resolution;

        mx = (int)std::floor((wx - origin_x) / res);
        my = (int)std::floor((wy - origin_y) / res);

        if (!isInBoundsMap(mx, my)) return false;
        return true;
    }

    void mapToWorld(int mx, int my, double& wx, double& wy) const
    {
        double origin_x = map_.info.origin.position.x;
        double origin_y = map_.info.origin.position.y;
        double res      = map_.info.resolution;

        wx = origin_x + (mx + 0.5) * res;
        wy = origin_y + (my + 0.5) * res;
    }

    bool isFreeWorld(double wx, double wy) const
    {
        int mx, my;
        if (!worldToMap(wx, wy, mx, my)) return false;
        return isCellFree(mx, my);
    }

    // 线段碰撞检测：从 (x1,y1) 到 (x2,y2)，步进栅格检测是否穿过障碍
    bool isSegmentFree(double x1, double y1, double x2, double y2) const
    {
        if (!have_map_) return false;

        int mx1, my1, mx2, my2;
        if (!worldToMap(x1, y1, mx1, my1)) return false;
        if (!worldToMap(x2, y2, mx2, my2)) return false;

        int dx = std::abs(mx2 - mx1);
        int dy = std::abs(my2 - my1);
        int sx = (mx1 < mx2) ? 1 : -1;
        int sy = (my1 < my2) ? 1 : -1;
        int err = dx - dy;

        int x = mx1;
        int y = my1;
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

    double distance(double x1, double y1, double x2, double y2) const
    {
        double dx = x1 - x2;
        double dy = y1 - y2;
        return std::sqrt(dx*dx + dy*dy);
    }

    // 核心：RRT 规划
    bool plan(std::vector<std::pair<double,double>>& out_path,
              std::vector<RRTNode>& out_tree)
    {
        out_path.clear();
        out_tree.clear();

        if (!have_map_ || !have_start_ || !have_goal_) return false;
        if (!isFreeWorld(start_x_, start_y_) || !isFreeWorld(goal_x_, goal_y_))
        {
            ROS_WARN("Start or goal not in free space.");
            return false;
        }

        // 随机采样器范围使用整张地图的世界坐标范围
        double min_x = map_.info.origin.position.x;
        double min_y = map_.info.origin.position.y;
        double max_x = min_x + map_.info.width  * map_.info.resolution;
        double max_y = min_y + map_.info.height * map_.info.resolution;

        std::mt19937 gen(12345);
        std::uniform_real_distribution<double> dist_x(min_x, max_x);
        std::uniform_real_distribution<double> dist_y(min_y, max_y);
        std::uniform_real_distribution<double> dist_goal(0.0, 1.0);

        // 初始化树：根节点为起点
        RRTNode root;
        root.x = start_x_;
        root.y = start_y_;
        root.parent = -1;
        out_tree.push_back(root);

        int goal_index = -1;

        for (int i = 0; i < max_iterations_; ++i)
        {
            // 1) 采样点：大部分随机，小概率用终点（goal bias）
            double rx, ry;
            if (dist_goal(gen) < 0.1)  // 10% 直接采样 goal，帮助收敛
            {
                rx = goal_x_;
                ry = goal_y_;
            }
            else
            {
                rx = dist_x(gen);
                ry = dist_y(gen);
            }

            // 若采样点不在自由空间，跳过
            if (!isFreeWorld(rx, ry))
                continue;

            // 2) 找最近的树节点
            int nearest_id = -1;
            double min_dist = std::numeric_limits<double>::infinity();
            for (size_t j = 0; j < out_tree.size(); ++j)
            {
                double d = distance(rx, ry, out_tree[j].x, out_tree[j].y);
                if (d < min_dist)
                {
                    min_dist = d;
                    nearest_id = (int)j;
                }
            }
            if (nearest_id < 0) continue;

            // 3) 从最近点往采样点方向走一步（step_size_）
            double nx, ny;
            if (min_dist < step_size_)
            {
                nx = rx;
                ny = ry;
            }
            else
            {
                double vx = rx - out_tree[nearest_id].x;
                double vy = ry - out_tree[nearest_id].y;
                double norm = std::sqrt(vx*vx + vy*vy);
                vx /= norm;
                vy /= norm;
                nx = out_tree[nearest_id].x + vx * step_size_;
                ny = out_tree[nearest_id].y + vy * step_size_;
            }

            // 4) 检查从最近点到新点的线段是否无碰撞
            if (!isSegmentFree(out_tree[nearest_id].x, out_tree[nearest_id].y, nx, ny))
                continue;

            // 5) 加入树
            RRTNode new_node;
            new_node.x = nx;
            new_node.y = ny;
            new_node.parent = nearest_id;
            out_tree.push_back(new_node);

            // 6) 检查是否接近目标
            double d_goal = distance(nx, ny, goal_x_, goal_y_);
            if (d_goal < goal_tolerance_)
            {
                // 直接把目标作为一个节点连上去（可选，也可以直接用 new_node 做终点）
                RRTNode goal_node;
                goal_node.x = goal_x_;
                goal_node.y = goal_y_;
                goal_node.parent = (int)out_tree.size() - 1;
                out_tree.push_back(goal_node);
                goal_index = (int)out_tree.size() - 1;
                ROS_INFO("RRT reached goal at iteration %d, nodes=%zu", i, out_tree.size());
                break;
            }
        }

        if (goal_index < 0)
        {
            ROS_WARN("RRT failed to reach goal within max iterations.");
            return false;
        }

        // 从 goal_index 开始回溯路径
        int cur = goal_index;
        while (cur >= 0)
        {
            out_path.emplace_back(out_tree[cur].x, out_tree[cur].y);
            cur = out_tree[cur].parent;
        }
        std::reverse(out_path.begin(), out_path.end());
        return true;
    }

    // 发布 Path
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

    // 发布树（LINE_LIST）
    void publishTree(const std::vector<RRTNode>& tree)
    {
        if (!have_map_) return;
        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_tree";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_LIST;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.02;
        m.color.r = 0.0;
        m.color.g = 0.0;
        m.color.b = 1.0;
        m.color.a = 0.4;

        for (size_t i = 0; i < tree.size(); ++i)
        {
            if (tree[i].parent < 0) continue;
            geometry_msgs::Point p1, p2;
            p1.x = tree[i].x;
            p1.y = tree[i].y;
            p1.z = 0.01;

            p2.x = tree[tree[i].parent].x;
            p2.y = tree[tree[i].parent].y;
            p2.z = 0.01;

            m.points.push_back(p1);
            m.points.push_back(p2);
        }
        tree_pub_.publish(m);
    }

    // 发布最终路径 Marker（LINE_STRIP）
    void publishPathMarker(const std::vector<std::pair<double,double>>& path)
    {
        if (!have_map_) return;
        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_path";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_STRIP;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.05;
        m.color.r = 0.0;
        m.color.g = 1.0;
        m.color.b = 0.0;
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

    // 发布障碍物
    void publishObstacles()
    {
        if (!have_map_) return;

        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "rrt_obstacles";
        m.id = 0;
        m.type = visualization_msgs::Marker::CUBE_LIST;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = map_.info.resolution;
        m.scale.y = map_.info.resolution;
        m.scale.z = 0.05;
        m.color.r = 1.0;
        m.color.g = 0.0;
        m.color.b = 0.0;
        m.color.a = 0.7;

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
        if (!have_map_)
        {
            ROS_WARN("No map yet, cannot set start.");
            return;
        }
        start_x_ = msg->pose.pose.position.x;
        start_y_ = msg->pose.pose.position.y;
        if (!isFreeWorld(start_x_, start_y_))
        {
            ROS_WARN("RRT start not free.");
            return;
        }
        have_start_ = true;
        ROS_INFO("RRT start set: (%.3f, %.3f)", start_x_, start_y_);
        tryPlan();
    }

    void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
    {
        if (!have_map_)
        {
            ROS_WARN("No map yet, cannot set goal.");
            return;
        }
        goal_x_ = msg->pose.position.x;
        goal_y_ = msg->pose.position.y;
        if (!isFreeWorld(goal_x_, goal_y_))
        {
            ROS_WARN("RRT goal not free.");
            return;
        }
        have_goal_ = true;
        ROS_INFO("RRT goal set: (%.3f, %.3f)", goal_x_, goal_y_);
        tryPlan();
    }

    void tryPlan()
    {
        if (!(have_map_ && have_start_ && have_goal_))
            return;

        std::vector<std::pair<double,double>> path;
        std::vector<RRTNode> tree;
        if (plan(path, tree))
        {
            ROS_INFO("RRT path found, length: %zu, tree nodes: %zu",
                     path.size(), tree.size());
            publishTree(tree);
            publishPath(path);
            publishPathMarker(path);
        }
        else
        {
            ROS_WARN("RRT planning failed.");
        }
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "rrt_occupancy_node");
    ros::NodeHandle nh;
    RRTOccupancy rrt(nh);
    ros::spin();
    return 0;
}
