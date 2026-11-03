#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <vector>
#include <cmath>
#include <algorithm>

int main(int argc, char** argv)
{
    ros::init(argc, argv, "fake_map_node");
    ros::NodeHandle nh("~");

    ros::Publisher map_pub = nh.advertise<nav_msgs::OccupancyGrid>("/map", 1, true);

    // 地图参数
    double resolution;
    int width, height;
    nh.param("resolution", resolution, 0.1); // m/cell
    nh.param("width",      width,      200); // cell
    nh.param("height",     height,     200); // cell

    nav_msgs::OccupancyGrid map;
    map.header.frame_id = "map";
    map.info.resolution = resolution;
    map.info.width  = width;
    map.info.height = height;
    map.info.origin.position.x = -width  * resolution / 2.0;
    map.info.origin.position.y = -height * resolution / 2.0;
    map.info.origin.orientation.w = 1.0;

    map.data.assign(width * height, 0);
    auto idx = [&](int x, int y){ return y * width + x; };

    // 0) 外围一圈墙
    for (int x = 0; x < width; ++x)
    {
        map.data[idx(x, 0)]         = 100;
        map.data[idx(x, height-1)]  = 100;
    }
    for (int y = 0; y < height; ++y)
    {
        map.data[idx(0, y)]         = 100;
        map.data[idx(width-1, y)]   = 100;
    }

    // 1) 网格线“走廊”墙（先画满，不挖洞）
    int step_x = width / 6;   // 横线间隔
    int step_y = height / 6;  // 竖线间隔

    // 横向墙
    for (int k = 1; k < 5; ++k)
    {
        int y = k * step_x;
        for (int x = step_x/4; x < width - step_x/4; ++x)  // 稍微内缩一点
            map.data[idx(x, y)] = 100;
    }

    // 竖向墙
    for (int k = 1; k < 5; ++k)
    {
        int x = k * step_y;
        for (int y = step_y/4; y < height - step_y/4; ++y)
            map.data[idx(x, y)] = 100;
    }

    // 2) 在所有横竖墙交叉点挖大一点的“十字路口”
    int gap_half_w = 6;  // 交叉口在 x 方向的半宽（格子数）
    int gap_half_h = 6;  // 交叉口在 y 方向的半高

    // 横线 y_list 和竖线 x_list
    std::vector<int> y_list, x_list;
    for (int k = 1; k < 5; ++k)
        y_list.push_back(k * step_x);
    for (int k = 1; k < 5; ++k)
        x_list.push_back(k * step_y);

    for (int y0 : y_list)
    {
        for (int x0 : x_list)
        {
            for (int dx = -gap_half_w; dx <= gap_half_w; ++dx)
            {
                for (int dy = -gap_half_h; dy <= gap_half_h; ++dy)
                {
                    int xx = x0 + dx;
                    int yy = y0 + dy;
                    if (xx > 0 && xx < width-1 && yy > 0 && yy < height-1)
                        map.data[idx(xx, yy)] = 0;  // 挖空
                }
            }
        }
    }

    // 3) 工具函数：矩形
    auto add_rect = [&](int cx, int cy, int half_w, int half_h)
    {
        for (int x = cx-half_w; x <= cx+half_w; ++x)
        {
            for (int y = cy-half_h; y <= cy+half_h; ++y)
            {
                if (x > 0 && x < width-1 && y > 0 && y < height-1)
                    map.data[idx(x, y)] = 100;
            }
        }
    };

    // 工具：圆形
    auto add_circle = [&](int cx, int cy, int r)
    {
        int r2 = r*r;
        for (int x = cx-r; x <= cx+r; ++x)
        {
            for (int y = cy-r; y <= cy+r; ++y)
            {
                if (x <= 0 || x >= width-1 || y <= 0 || y >= height-1) continue;
                int dx = x - cx;
                int dy = y - cy;
                if (dx*dx + dy*dy <= r2)
                    map.data[idx(x, y)] = 100;
            }
        }
    };

    // 工具：多边形（扫描线填充）
    auto add_polygon = [&](const std::vector<std::pair<int,int>>& pts)
    {
        if (pts.size() < 3) return;

        int min_y = pts[0].second, max_y = pts[0].second;
        for (auto& p : pts)
        {
            if (p.second < min_y) min_y = p.second;
            if (p.second > max_y) max_y = p.second;
        }
        min_y = std::max(min_y, 1);
        max_y = std::min(max_y, height-2);

        for (int y = min_y; y <= max_y; ++y)
        {
            std::vector<double> x_intersections;
            for (size_t i = 0; i < pts.size(); ++i)
            {
                auto p1 = pts[i];
                auto p2 = pts[(i+1) % pts.size()];
                int y1 = p1.second, y2 = p2.second;
                int x1 = p1.first,  x2 = p2.first;

                if ( (y >= std::min(y1, y2)) &&
                     (y <  std::max(y1, y2)) &&
                     (y1 != y2) )
                {
                    double x_int = x1 + (double)(x2 - x1) * (double)(y - y1) / (double)(y2 - y1);
                    x_intersections.push_back(x_int);
                }
            }
            if (x_intersections.size() < 2) continue;
            std::sort(x_intersections.begin(), x_intersections.end());

            for (size_t k = 0; k+1 < x_intersections.size(); k += 2)
            {
                int x_start = std::max(1, (int)std::ceil(x_intersections[k]));
                int x_end   = std::min(width-2, (int)std::floor(x_intersections[k+1]));
                for (int x = x_start; x <= x_end; ++x)
                    map.data[idx(x, y)] = 100;
            }
        }
    };

    // 4) 额外方块障碍
    add_rect(width*0.25, height*0.25, 8, 5);
    add_rect(width*0.7,  height*0.2,  6, 10);
    add_rect(width*0.3,  height*0.7,  10, 6);

    // 5) 圆形障碍
    add_circle(width*0.6, height*0.6, 12);
    add_circle(width*0.5, height*0.4, 7);
    add_circle(width*0.15, height*0.6, 6);

    // 6) 不规则多边形
    std::vector<std::pair<int,int>> poly1 = {
        { (int)(width*0.75), (int)(height*0.75) },
        { (int)(width*0.85), (int)(height*0.75) },
        { (int)(width*0.9),  (int)(height*0.85) },
        { (int)(width*0.8),  (int)(height*0.9)  },
        { (int)(width*0.7),  (int)(height*0.85) }
    };
    add_polygon(poly1);

    std::vector<std::pair<int,int>> poly2 = {
        { (int)(width*0.4),  (int)(height*0.55) },
        { (int)(width*0.5),  (int)(height*0.55) },
        { (int)(width*0.5),  (int)(height*0.65) },
        { (int)(width*0.45), (int)(height*0.65) },
        { (int)(width*0.45), (int)(height*0.7)  },
        { (int)(width*0.35), (int)(height*0.7)  },
        { (int)(width*0.35), (int)(height*0.6)  }
    };
    add_polygon(poly2);

    ros::Rate r(1);
    while (ros::ok())
    {
        map.header.stamp = ros::Time::now();
        map_pub.publish(map);
        ros::spinOnce();
        r.sleep();
    }
    return 0;
}
