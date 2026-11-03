#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <visualization_msgs/Marker.h>
#include <limits>
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>

/// 单个A*节点（栅格）信息
struct Node
{
    int x;          // 栅格坐标 x（列）
    int y;          // 栅格坐标 y（行）
    float g;        // g 值：从起点到当前节点的累计代价
    float h;        // h 值：启发函数（从当前到终点的估计代价）
    int parent_x;   // 父节点 x（用于回溯路径）
    int parent_y;   // 父节点 y
};

/// 优先队列比较函数：按 f = g + h 从小到大排序
struct NodeCompare
{
    bool operator()(const Node& a, const Node& b) const
    {
        return (a.g + a.h) > (b.g + b.h);
    }
};

/// A* 基于 OccupancyGrid 的简单路径规划器
class AStarOccupancy
{
public:
    AStarOccupancy(ros::NodeHandle& nh)
    {
        // 读取私有参数
        ros::NodeHandle pnh("~");
        pnh.param("map_topic", map_topic_, std::string("map"));   // 地图话题
        pnh.param("frame_id", frame_id_, std::string("map"));     // 坐标系
        pnh.param("occupied_threshold", occupied_thresh_, 50);    // 占据概率 >= 该值视为障碍


        // 订阅：地图、起点、终点
        map_sub_   = nh.subscribe(map_topic_, 1, &AStarOccupancy::mapCallback, this);
        start_sub_ = nh.subscribe("initialpose", 1, &AStarOccupancy::startCallback, this);
        goal_sub_  = nh.subscribe("move_base_simple/goal", 1, &AStarOccupancy::goalCallback, this);

        // 发布：路径（Path）和 Marker（障碍、路径线条）
        path_pub_        = nh.advertise<nav_msgs::Path>("astar_path", 1,true);
        obstacles_pub_   = nh.advertise<visualization_msgs::Marker>("astar_obstacles", 1,true);
        path_marker_pub_ = nh.advertise<visualization_msgs::Marker>("astar_path_marker", 1,true);

        have_map_   = false;
        have_start_ = false;
        have_goal_  = false;
    }

private:
    // ---------------- 参数与状态 ----------------
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

    // ROS 通信句柄
    ros::Subscriber map_sub_;
    ros::Subscriber start_sub_;
    ros::Subscriber goal_sub_;
    ros::Publisher path_pub_;
    ros::Publisher obstacles_pub_;
    ros::Publisher path_marker_pub_;

    // ---------------- 工具函数 ----------------

    /// 世界坐标(wx, wy) -> 栅格坐标(mx, my)
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

    /// 栅格坐标(mx, my) -> 世界坐标(wx, wy)
    void mapToWorld(int mx, int my, double& wx, double& wy) const
    {
        double origin_x = map_.info.origin.position.x;
        double origin_y = map_.info.origin.position.y;
        double res      = map_.info.resolution;

        wx = origin_x + (mx + 0.5) * res;
        wy = origin_y + (my + 0.5) * res;
    }

    /// 栅格 (x, y) -> 一维索引
    int index(int x, int y) const
    {
        return y * map_.info.width + x;
    }

    /// 是否在地图边界内
    bool isInBounds(int x, int y) const
    {
        return x >= 0 && y >= 0 &&
               x < (int)map_.info.width && y < (int)map_.info.height;
    }

    /// 该栅格是否可通行
    bool isFree(int x, int y) const
    {
        if (!isInBounds(x, y)) return false;
        int8_t v = map_.data[index(x, y)];
        if (v < 0)  // unknown 区域
            return false; // 简单处理：unknown 当作障碍
        return v < occupied_thresh_;
    }

    /// 启发函数：当前(x, y) 到目标(gx, gy) 的估计代价
    float heuristic(int x, int y, int gx, int gy) const
    {
        // 欧式距离（也可以改成曼哈顿）
        return std::sqrt((x - gx)*(x - gx) + (y - gy)*(y - gy));
    }

    /// 核心：A* 搜索（教科书版逻辑）
    bool plan(int sx, int sy, int gx, int gy,
            std::vector<std::pair<int,int>>& out_path)
    {
        out_path.clear();                          // 先清空输出路径

        if (!have_map_) return false;              // 没有地图，直接失败
        if (!isFree(sx, sy) || !isFree(gx, gy))    // 起点或终点不可通行就失败
        {
            ROS_WARN("Start or goal is not free.");
            return false;
        }

        int w = map_.info.width;                   // 地图宽（栅格数）
        int h = map_.info.height;                  // 地图高（栅格数）

        // open：优先队列，按 f = g + h 最小优先（通过 NodeCompare 实现）
        std::priority_queue<Node, std::vector<Node>, NodeCompare> open;

        // 记录每个栅格当前已知的最小 G，初始化为 +∞
        std::vector<float> g_score(w * h, std::numeric_limits<float>::infinity());

        // closed：是否已经“处理完”这个栅格
        std::vector<bool> closed(w * h, false);

        // 父节点坐标，用于回溯路径
        std::vector<int> parent_x(w * h, -1);
        std::vector<int> parent_y(w * h, -1);

        // 小工具函数：2D (x,y) -> 1D 索引
        auto id = [&](int x, int y){ return index(x, y); };

        // 起点初始化
        int sidx = id(sx, sy);                     // 起点的一维索引
        g_score[sidx] = 0.0f;                      // 起点的最小 G = 0

        Node start;
        start.x = sx;                              // 起点 x
        start.y = sy;                              // 起点 y
        start.g = 0.0f;                            // G = 0
        start.h = heuristic(sx, sy, gx, gy);       // H = 起点到终点的启发距离
        start.parent_x = -1;                       // 起点没有父节点
        start.parent_y = -1;
        open.push(start);                          // 把起点放入 openlist

        // 4 邻接方向增量
        int dx4[4] = {1,-1,0,0};
        int dy4[4] = {0,0,1,-1};

        // 8 邻接方向增量 + 对应的代价（对角为 sqrt(2)）
        int dx8[8] = {1,-1,0,0,  1,1,-1,-1};
        int dy8[8] = {0,0,1,-1,  1,-1,1,-1};
        float cost8[8] = {1,1,1,1,
                        std::sqrt(2.0f),std::sqrt(2.0f),
                        std::sqrt(2.0f),std::sqrt(2.0f)};

        // 主循环：每次从 open 里取出 f 最小的节点扩展
        while (!open.empty())
        {
            Node current = open.top();             // 取 f 最小的节点（openlist 头）
            open.pop();                            // 从 openlist 中移除

            int idx = id(current.x, current.y);    // 当前节点的一维索引

            // 如果已经在 closed 里，说明之前处理过更优版本，跳过
            if (closed[idx])
                continue;

            // 如果当前节点的 g 比记录的最小 g_score 还大，说明它是“旧版本”，也跳过
            if (current.g > g_score[idx] + 1e-6)   // 加一点小 eps 防浮点误差
                continue;

            // 标记为 closed：后面不再从它出发扩展
            closed[idx] = true;

            // 记录父节点（是从谁扩展到它的）
            parent_x[idx] = current.parent_x;
            parent_y[idx] = current.parent_y;

            // 如果已经到达终点 => 回溯路径并返回成功
            if (current.x == gx && current.y == gy)
            {
                reconstructPath(sx, sy, gx, gy, parent_x, parent_y, out_path);
                return true;
            }

            // ========= 扩展邻居 =========

            for (int k = 0; k < 8; ++k)        // 遍历 8 个方向
            {
                int nx = current.x + dx8[k];   // 邻居 x
                int ny = current.y + dy8[k];   // 邻居 y

                if (!isInBounds(nx, ny)) continue; // 越界丢弃
                if (!isFree(nx, ny))     continue; // 是障碍/未知，丢弃

                int nidx = id(nx, ny);
                if (closed[nidx])        continue; // 已在 closed，丢弃

                // 从 current 走到邻居的“候选 G 值”
                float tentative_g = current.g + cost8[k];

                // 如果这条路径的 G 不优于当前记录的 G，按伪代码跳过
                if (tentative_g >= g_score[nidx])
                    continue;

                // 否则：这是到这个邻居更优的一条路径
                g_score[nidx] = tentative_g;   // 更新邻居的最优 G

                Node nb;
                nb.x = nx;
                nb.y = ny;
                nb.g = tentative_g;            // 新的 G
                nb.h = heuristic(nx, ny, gx, gy); // 重新算 H
                nb.parent_x = current.x;       // 记录父节点
                nb.parent_y = current.y;
                open.push(nb);                 // 丢进 openlist（queue 会按 f 自动排）
            }
            
        }

        // openlist 被耗尽仍然没到终点 => 起点和终点不连通，规划失败
        ROS_WARN("A* failed to find a path.");
        return false;
    }



    /// 根据 parent_x / parent_y 回溯出最终路径（栅格坐标）
    void reconstructPath(int sx, int sy, int gx, int gy,
                         const std::vector<int>& parent_x,
                         const std::vector<int>& parent_y,
                         std::vector<std::pair<int,int>>& out_path)
    {
        out_path.clear();
        int cx = gx;
        int cy = gy;

        // 从终点一直回溯到起点
        while (!(cx == sx && cy == sy))
        {
            out_path.emplace_back(cx, cy);
            int idx = index(cx, cy);
            int px = parent_x[idx];
            int py = parent_y[idx];

            // 如果中途出现父节点非法，说明没有完整路径
            if (px < 0 || py < 0)
            {
                out_path.clear();
                return;
            }
            cx = px;
            cy = py;
        }

        // 把起点也加入路径
        out_path.emplace_back(sx, sy);
        // 目前是从终点到起点，反转一下
        std::reverse(out_path.begin(), out_path.end());
    }

    // ---------------- 发布可视化 ----------------

    /// 发布 nav_msgs::Path，用于 RViz 的 Path 显示
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

    /// 发布障碍物栅格的 Marker（红色方块）
    void publishObstacles()
    {
        if (!have_map_) return;

        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "astar_obstacles";
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

        // 把所有占用栅格（>= 阈值）画出来
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

    /// 发布路径的线条 Marker（绿色折线）
    void publishPathMarker(const std::vector<std::pair<int,int>>& path)
    {
        if (!have_map_) return;

        visualization_msgs::Marker m;
        m.header.stamp = ros::Time::now();
        m.header.frame_id = frame_id_;
        m.ns = "astar_path";
        m.id = 0;
        m.type = visualization_msgs::Marker::LINE_STRIP;
        m.action = visualization_msgs::Marker::ADD;
        m.scale.x = 0.05;  // 线宽
        m.color.r = 0.0;
        m.color.g = 1.0;
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

    // ---------------- 回调函数 ----------------

    /// 地图回调：收到 OccupancyGrid 后保存，并画一次障碍物
    void mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg)
    {
        map_ = *msg;
        have_map_ = true;
        ROS_INFO("Map received: %u x %u, res=%.3f",
                 map_.info.width, map_.info.height, map_.info.resolution);

        publishObstacles();
        tryPlan();  // 如果起点和终点已经有了，就尝试规划
    }

    /// 起点回调：来自 RViz 的 2D Pose Estimate（/initialpose）
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
        ROS_INFO("Start set: (%d, %d)", start_x_, start_y_);
        tryPlan();
    }

    /// 终点回调：来自 RViz 的 2D Nav Goal（/move_base_simple/goal）
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
        ROS_INFO("Goal set: (%d, %d)", goal_x_, goal_y_);
        tryPlan();
    }

    /// 若 map + start + goal 都具备，则执行一次 A* 规划
    void tryPlan()
    {
        if (!(have_map_ && have_start_ && have_goal_))
            return;

        std::vector<std::pair<int,int>> path;
        if (plan(start_x_, start_y_, goal_x_, goal_y_, path))
        {
            ROS_INFO("A* path found, length: %zu", path.size());
            publishPath(path);
            publishPathMarker(path);
        }
        else
        {
            ROS_WARN("A* planning failed.");
        }
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "astar_occupancy_node");
    ros::NodeHandle nh;

    AStarOccupancy astar(nh);

    ros::spin();
    return 0;
}
