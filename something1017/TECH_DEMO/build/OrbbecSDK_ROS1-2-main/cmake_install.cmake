# Install script for directory: /home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/orangepi/something1017/TECH_DEMO/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/msg" TYPE FILE FILES
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/DeviceInfo.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/Extrinsics.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/Metadata.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/IMUInfo.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/StreamStatus.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/DeviceStatus.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/DepthFilterParam.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/DepthFilterState.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/DepthFiltersStatus.msg"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/msg/StreamProfile.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/srv" TYPE FILE FILES
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetActionConfig.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetBool.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetBool.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetCameraInfo.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetCameraParams.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetDeviceConfig.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetDeviceInfo.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetInt32.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetAwbGain.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetFilter.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/GetString.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetInt32.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetActionConfig.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetAwbGain.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetString.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetArrays.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetBagRecording.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SetStreamProfile.srv"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/srv/SendActionCommand.srv"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/cmake" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/orbbec_camera-msg-paths.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/devel/include/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/roseus/ros" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/devel/share/roseus/ros/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/common-lisp/ros" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/devel/share/common-lisp/ros/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/gennodejs/ros" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/devel/share/gennodejs/ros/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process(COMMAND "/usr/bin/python3" -m compileall "/home/orangepi/something1017/TECH_DEMO/devel/lib/python3/dist-packages/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/python3/dist-packages/orbbec_camera")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/orbbec_camera.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/cmake" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/orbbec_camera-msg-extras.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/cmake" TYPE FILE FILES
    "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/orbbec_cameraConfig.cmake"
    "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/orbbec_cameraConfig-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/package.xml")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE PROGRAM FILES "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/common_benchmark_node.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE PROGRAM FILES "/home/orangepi/something1017/TECH_DEMO/build/OrbbecSDK_ROS1-2-main/catkin_generated/installspace/service_benchmark_node.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/liborbbec_camera.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/liborbbec_camera_nodelet.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liborbbec_camera_nodelet.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/orbbec_camera_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/orbbec_camera_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/list_devices_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_devices_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/list_depth_work_mode_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_depth_work_mode_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/list_camera_profile_mode_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/list_camera_profile_mode_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/ip_config_tool")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/ip_config_tool")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/set_device_ip")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/set_device_ip")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/firmware_update_tool")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/firmware_update_tool")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/service_benchmark_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/service_benchmark_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/image_sync_example_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:/home/orangepi/something1017/TECH_DEMO/devel/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/image_sync_example_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node"
         RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera" TYPE EXECUTABLE FILES "/home/orangepi/something1017/TECH_DEMO/devel/lib/orbbec_camera/action_command_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node"
         OLD_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64:/opt/ros/noetic/lib:"
         NEW_RPATH ":/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/orbbec_camera/action_command_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/SDK/lib" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera/SDK/licenses" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/licenses/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/orbbec_camera/" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/include")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/orbbec_camera/" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/include/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/launch")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/config")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE DIRECTORY FILES
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/meshes"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/urdf"
    "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/rviz"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/examples")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/nodelet_plugins.xml")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/orbbec_camera" TYPE FILE FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" TYPE DIRECTORY FILES "/home/orangepi/something1017/TECH_DEMO/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64/" FILES_MATCHING REGEX "/[^/]*\\.so$" REGEX "/[^/]*\\.so\\.[^/]*$")
endif()

