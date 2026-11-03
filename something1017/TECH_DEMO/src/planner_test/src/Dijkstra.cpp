#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <visualization_msgs/Marker.h>

#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

/// 单个节点（栅格）信息
struct DNode
{
    int x;
    int y;
    float g;        // 从起点到当前节点的累计代价
    int parent_x;
    int parent_y;
};

/// 优先队列比较函数：按 g 从小到大排序
struct DNodeCompare
{
    bool operator()(const DNode& a, const DNode& b) const
    {
        return a.g > b.g;  // g 小的优先
    }
};

/// Dijkstra 基于 OccupancyGrid 的简单路径规划器
class DijkstraOccupancy
{
public:
    DijkstraOccupancy(ros::NodeHandle& nh)
    {
        // 读取私有参数
        ros::NodeHandle pnh("~");
        pnh.param("map_topic", map_topic_, std::string("map"));
        pnh.param("frame_id", frame_id_, std::string("map"));
        pnh.param("occupied_threshold", occupied_thresh_, 50); // >= 阈值视为障碍

        // 订阅：地图、起点、终点
        map_sub_   = nh.subscribe(map_topic_, 1, &DijkstraOccupancy::mapCallback, this);
        start_sub_ = nh.subscribe("initialpose", 1, &DijkstraOccupancy::startCallback, this);
        goal_sub_  = nh.subscribe("move_base_simple/goal", 1, &DijkstraOccupancy::goalCallback, this);

        // 发布：路径和可视化 Marker
        path_pub_        = nh.advertise<nav_msgs::Path>("dijkstra_path", 1,true);
        obstacles_pub_   = nh.advertise<visualization_msgs::Marker>("dijkstra_obstacles", 1,true);
        path_marker_pub_ = nh.advertise<visualization_msgs::Marker>("dijkstra_path_marker", 1,true);

        have_map_   = false;
        have_start_ = false;
        have_goal_  = false;
    }

private:
    // 配置
    std::string map_topic_;
    std::string frame_id_;
    int occupied_thresh_;

    // 地图
    nav_msgs::OccupancyGrid map_;
    bool have_map_;

    // 起终点（栅格坐标）
    int start_x_, start_y_;
    int goal_x_, goal_y_;
    bool have_start_, have_goal_;

    // ROS
    ros::Subscriber map_sub_;
    ros::Subscriber start_sub_;
    ros::Subscriber goal_sub_;
    ros::Publisher path_pub_;
    ros::Publisher obstacles_pub_;
    ros::Publisher path_marker_pub_;

    // ===== 工具函数 =====

    bool worldToMap(double wx, double wy, int& mx, int& my) const
    {
        if (!have_map_) return false;
        double origin_x = map_.info.origin.position.x;
        double origin_y = map_.info.origin.position.y;
        double res      = map_.info.resolution;

        mx = (int)std::floor((wx - origin_x) / res);
        my = (int)std::floor((wy - origin_y) / res);

        if (mx < 0 || my < 0 ||
            mx >= (int)map_.info.width || my >= (int)map_.info.height)
            return false;
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

    int index(int x, int y) const
    {
        return y * map_.info.width + x;
    }

    bool isInBounds(int x, int y) const
    {
        return x >= 0 && y >= 0 &&
               x < (int)map_.info.width && y < (int)map_.info.height;
    }

    bool isFree(int x, int y) const
    {
        if (!isInBounds(x, y)) return false;
        int8_t v = map_.data[index(x, y)];
        if (v < 0)  // unknown
            return false;
        return v < occupied_thresh_;
    }

    // ===== 核心：Dijkstra 搜索（按累计路程 g 最小） =====
    bool plan(int sx, int sy, int gx, int gy,
              std::vector<std::pair<int,int>>& out_path)
    {
        out_path.clear();                         // 先清空输出路径

        if (!have_map_) return false;             // 没有地图，直接失败
        if (!isFree(sx, sy) || !isFree(gx, gy))   // 起点或终点不可通行就失败
        {
            ROS_WARN("Start or goal is not free.");
            return false;
        }

        int w = map_.info.width;                  // 地图宽（栅格数）
        int h = map_.info.height;                 // 地图高（栅格数）

        // open：优先队列，按 g 最小优先（Dijkstra 没有启发式，只看 g）
        std::priority_queue<DNode, std::vector<DNode>, DNodeCompare> open;

        // 记录每个栅格当前已知的最小 G，初始化为 +∞
        std::vector<float> g_score(w * h, std::numeric_limits<float>::infinity());

        // closed：是否已经“处理完”这个栅格
        std::vector<bool> closed(w * h, false);

        // 父节点坐标，用于回溯路径
        std::vector<int> parent_x(w * h, -1);
        std::vector<int> parent_y(w * h, -1);

        // 小工具：二维 (x,y) -> 一维索引
        auto id = [&](int x, int y){ return index(x, y); };

        // 起点初始化
        int sidx = id(sx, sy);                    // 起点索引
        g_score[sidx] = 0.0f;                     // 起点到自己的路程为 0

        DNode start;
        start.x = sx;                             // 起点 x
        start.y = sy;                             // 起点 y
        start.g = 0.0f;                           // G = 0
        start.parent_x = -1;                      // 起点无父节点
        start.parent_y = -1;
        open.push(start);                         // 起点入 openlist

        // 八邻接方向和代价
        int dx8[8] = {1,-1,0,0,  1,1,-1,-1};      // x 方向偏移
        int dy8[8] = {0,0,1,-1,  1,-1,1,-1};      // y 方向偏移
        float cost8[8] = {1,1,1,1,                // 直行代价 = 1
                          std::sqrt(2.0f),std::sqrt(2.0f),
                          std::sqrt(2.0f),std::sqrt(2.0f)}; // 对角 = sqrt(2)

        // 主循环：每次从 open 中取“路程最小”的点做中心点
        while (!open.empty())
        {
            // 取 g 最小的节点
            DNode current = open.top();
            open.pop();

            int idx = id(current.x, current.y);   // 当前节点索引

            // 如果已经在 closed，说明处理过更优版本，跳过
            if (closed[idx])
                continue;

            // 如果当前节点的 g 比记录的 g_score 还大，说明它是旧版本，也跳过
            if (current.g > g_score[idx] + 1e-6)
                continue;

            // 放入 closed：之后不再从它扩展
            closed[idx] = true;

            // 记录父节点（用于后续回溯路径）
            parent_x[idx] = current.parent_x;
            parent_y[idx] = current.parent_y;

            // 如果当前节点就是目标点，结束搜索并回溯路径
            if (current.x == gx && current.y == gy)
            {
                reconstructPath(sx, sy, gx, gy, parent_x, parent_y, out_path);
                return true;                      // 找到路径
            }

            // ========= 遍历相邻的八个点 =========
            for (int k = 0; k < 8; ++k)
            {
                int nx = current.x + dx8[k];      // 邻居 x
                int ny = current.y + dy8[k];      // 邻居 y

                if (!isInBounds(nx, ny)) continue; // 不在地图内，跳过
                if (!isFree(nx, ny))     continue; // 是障碍/unknown，跳过

                int nidx = id(nx, ny);
                if (closed[nidx])        continue; // 已在 closed，跳过

                // 从 current 走到这个邻居的“新累计路程”
                float tentative_g = current.g + cost8[k];

                // 如果这条路径的路程不比之前记录的更短，则跳过
                if (tentative_g >= g_score[nidx])
                    continue;

                // 否则：更新这个邻居的最优路程和父节点，并加入 openlist
                g_score[nidx] = tentative_g;

                DNode nb;
                nb.x = nx;
                nb.y = ny;
                nb.g = tentative_g;               // 更新后的累计路程
                nb.parent_x = current.x;          // 父节点是 current
                nb.parent_y = current.y;
                open.push(nb);                    // 丢进 openlist
            }
        }

        // openlist 被耗尽，还没到目标 => 起点和终点不连通
        ROS_WARN("Dijkstra failed to find a path.");
        return false;
    }

    /// 回溯路径
    void reconstructPath(int sx, int sy, int gx, int gy,
                         const std::vector<int>& parent_x,
                         const std::vector<int>& parent_y,
                         std::vector<std::pair<int,int>>& out_path)
    {
        out_path.clear();
        int cx = gx;
        int cy = gy;

        while (!(cx == sx && cy == sy))
        {
            out_path.emplace_back(cx, cy);
            int idx = index(cx, cy);
            int px = parent_x[idx];
            int py = parent_y[idx];

            if (px < 0 || py < 0)
            {
                out_path.clear();
                return;
            }
            cx = px;
            cy = py;
        }
        out_path.emplace_back(sx, sy);
        std::reverse(out_path.begin(), out_path.end());
    }

    // ===== 可视化 =====

    void publishPath(const std::vector<std::pair<int,int>>& path)
    {
        if (!have_map_) return;

        nav_msgs::Path ros_path;
        ros_path.header.stamp = ros::Time::now();
        ros_path.header.frame_id = frame_id_;

        for (auto& p : path)
        {
            geometry_msgs::PoseStamped pose;
            pose.header = ros_path.header;
            double wx, wy;
            mapToWorld(p.first, p.second, wx, wy);
            pose.pose.position.x = wx;
            pose.pose.position.y = wy;
            pose.pose.position.z = 0.0;
            pose.pose.orientation.w = 1.0;
            ros_path.poses.push_back(pose);
        }
        path_pub_.publish(ros_path);
    }

    void publishObstacles()
    {
        if (!have_map_) return;

        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "dijkstra_obstacles";
        m.id = 0;
        m.type = visualization_msgs::Marker::CUBE_LIST;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = map_.info.resolution;
        m.scale.y = map_.info.resolution;
        m.scale.z = 0.05;
        m.color.r = 0.0;  // 和 A* 区分一下颜色
        m.color.g = 0.0;
        m.color.b = 1.0;
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

    void publishPathMarker(const std::vector<std::pair<int,int>>& path)
    {
        if (!have_map_) return;

        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "dijkstra_path";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_STRIP;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.05;
        m.color.r = 1.0;
        m.color.g = 0.5;
        m.color.b = 0.0;
        m.color.a = 1.0;

        for (auto& pxy : path)
        {
            geometry_msgs::Point p;
            mapToWorld(pxy.first, pxy.second, p.x, p.y);
            p.z = 0.02;
            m.points.push_back(p);
        }
        path_marker_pub_.publish(m);
    }

    // ===== 回调 =====

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
        double wx = msg->pose.pose.position.x;
        double wy = msg->pose.pose.position.y;
        int mx, my;
        if (!worldToMap(wx, wy, mx, my))
        {
            ROS_WARN("Start pose out of map.");
            return;
        }
        start_x_ = mx;
        start_y_ = my;
        have_start_ = true;
        ROS_INFO("Dijkstra start set: (%d, %d)", start_x_, start_y_);
        tryPlan();
    }

    void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
    {
        if (!have_map_)
        {
            ROS_WARN("No map yet, cannot set goal.");
            return;
        }
        double wx = msg->pose.position.x;
        double wy = msg->pose.position.y;
        int mx, my;
        if (!worldToMap(wx, wy, mx, my))
        {
            ROS_WARN("Goal pose out of map.");
            return;
        }
        goal_x_ = mx;
        goal_y_ = my;
        have_goal_ = true;
        ROS_INFO("Dijkstra goal set: (%d, %d)", goal_x_, goal_y_);
        tryPlan();
    }

    void tryPlan()
    {
        if (!(have_map_ && have_start_ && have_goal_))
            return;

        std::vector<std::pair<int,int>> path;
        if (plan(start_x_, start_y_, goal_x_, goal_y_, path))
        {
            ROS_INFO("Dijkstra path found, length: %zu", path.size());
            publishPath(path);
            publishPathMarker(path);
        }
        else
        {
            ROS_WARN("Dijkstra planning failed.");
        }
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "dijkstra_occupancy_node");
    ros::NodeHandle nh;

    DijkstraOccupancy dijkstra(nh);

    ros::spin();
    return 0;
}
