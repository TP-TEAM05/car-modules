cmake_minimum_required(VERSION 3.5)
project(car_serial_publisher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(serial_pub src/publisher_from_serial.cpp)
add_executable(udp_sub src/subscriber_from_udp.cpp)
add_executable(udp_pub src/controls_from_udp.cpp)
add_executable(serial_sub src/controls_to_serial.cpp)
ament_target_dependencies(serial_pub rclcpp std_msgs)
ament_target_dependencies(udp_sub rclcpp std_msgs)
ament_target_dependencies(udp_pub rclcpp std_msgs)
ament_target_dependencies(serial_sub rclcpp std_msgs)

install(TARGETS
  serial_pub
  udp_sub
  udp_pub
  serial_sub
  DESTINATION lib/${PROJECT_NAME})

ament_package()
