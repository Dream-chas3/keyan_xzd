# CMake generated Testfile for 
# Source directory: /home/orangepi/something1017/TECH_DEMO/src/land_test
# Build directory: /home/orangepi/something1017/TECH_DEMO/build/land_test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(land_motion_limits "/home/orangepi/something1017/TECH_DEMO/devel/lib/land_test/land_motion_limits_test")
set_tests_properties(land_motion_limits PROPERTIES  _BACKTRACE_TRIPLES "/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;195;add_test;/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;0;")
add_test(land_task "/home/orangepi/something1017/TECH_DEMO/devel/lib/land_test/land_task_test")
set_tests_properties(land_task PROPERTIES  _BACKTRACE_TRIPLES "/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;198;add_test;/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;0;")
add_test(land_rc_gate "/home/orangepi/something1017/TECH_DEMO/devel/lib/land_test/land_rc_gate_test")
set_tests_properties(land_rc_gate PROPERTIES  _BACKTRACE_TRIPLES "/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;201;add_test;/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;0;")
add_test(land_takeoff_replay "/home/orangepi/something1017/TECH_DEMO/devel/lib/land_test/land_takeoff_replay_test" "/home/orangepi/something1017/TECH_DEMO/src/land_test/test/data/takeoff_hover_20261027.csv")
set_tests_properties(land_takeoff_replay PROPERTIES  _BACKTRACE_TRIPLES "/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;204;add_test;/home/orangepi/something1017/TECH_DEMO/src/land_test/CMakeLists.txt;0;")
