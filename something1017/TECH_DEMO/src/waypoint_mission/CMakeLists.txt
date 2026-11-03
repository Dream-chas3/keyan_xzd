cmake_minimum_required(VERSION 3.0.2)
project(waypoint_mission)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  roslib
  std_msgs
  geometry_msgs
  nav_msgs
  sensor_msgs
  mavros_msgs
  cv_bridge
  tf2
  tf2_geometry_msgs
)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)

catkin_package()

include_directories(
  src
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_executable(waypoint_mission
  src/waypoint_mission.cpp
  src/gcu_control.c
)
target_link_libraries(waypoint_mission ${catkin_LIBRARIES} yaml-cpp)

add_executable(waypoint_photo src/waypoint_photo.cpp)
target_link_libraries(waypoint_photo ${catkin_LIBRARIES} ${OpenCV_LIBS})
