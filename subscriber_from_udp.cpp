#include <functional>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <iostream>
#include <ctime>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;
using json = nlohmann::ordered_json;

// UDP client, use udp_client_config to define UDP server IP:port
#define PORT 4040
#define IP_ADDR "192.168.20.224"

void loadParams(std::string& ip, std::string& port){
  std::ifstream file("/home/reco1/ros2_ws/src/car_serial_publisher/src/udp_client_config");

  if (!file){
      perror("UDP client config file not found");
  }

  file >> ip;
  file >> port;

  file.close();
}

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
      : Node("udp_sub"), 
        sockfd_(-1)
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "topic_from_serial", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));

    // Setup UDP socket
    if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      //RCLCPP_INFO(this->get_logger(), "Cannot create socket");
    }

    std::string ip, port;
    loadParams(ip, port);

    memset(&servaddr_, 0, sizeof(servaddr_));
    servaddr_.sin_family = AF_INET;
    servaddr_.sin_port = htons(stoi(port));
    servaddr_.sin_addr.s_addr = inet_addr(ip.c_str());

  }

  ~MinimalSubscriber()
  {
    if (sockfd_ != 1)
    {
      close(sockfd_);
    }
  }

private:
  void topic_callback(const std_msgs::msg::String &msg)
  {
    std::vector<uint8_t> buffer;
    //RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());

    // parsing the data to JSON
    const std ::string message = msg.data;  // here we expect to come data as <A,B,C,D,E,F,G,H,I>
    std::string vin, longitude, latitude, dist_ultrasonic, dist_lidar, speed_front_left, speed_front_right, speed_rear_left, speed_rear_right, speed_mean;

    sscanf(message.c_str(), "<%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^>]>", &dist_ultrasonic[0], &dist_lidar[0], &speed_front_left[0], &speed_rear_right[0],
    &speed_rear_left[0], &speed_front_right[0], &speed_mean[0], &longitude[0], &latitude[0]);
    
    vin = "C4RF117S7U0000001";

    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    std::string timestamp_iso = to_iso_extended_string(t) + "Z";
    char time_str[50] = {0};
    memcpy(time_str, timestamp_iso.c_str(), timestamp_iso.length());

    json json_to_send;
    json vehicle;

    json_to_send["index"] = 1;
    json_to_send["type"] = "update_vehicle";
    json_to_send["timestamp"] = time_str;

    vehicle["vin"] = vin.c_str();
    vehicle["speed"] = std::stof(speed_mean);
    vehicle["longitude"] = std::stof(longitude);
    vehicle["latitude"] = std::stof(latitude);
    vehicle["gps_direction"] = 100;
    vehicle["front_ultrasonic"] = std::stof(dist_ultrasonic);
    vehicle["rear_ultrasonic"] = 56;
    vehicle["front_lidar"] = std::stof(dist_lidar);
    vehicle["speed_front_left"] = std::stof(speed_front_left);
    vehicle["speed_front_right"] = std::stof(speed_front_right);
    vehicle["speed_rear_left"] = std::stof(speed_rear_left);
    vehicle["speed_rear_right"] = std::stof(speed_rear_right);
    json_to_send["vehicle"] = vehicle;

    std::string str_to_send = json_to_send.dump();
    
    // Send message to UDP sever
    if (sendto(sockfd_, str_to_send.c_str(), str_to_send.length(), 0, (struct sockaddr *)&servaddr_, sizeof(servaddr_)) < 0)
    {
      //RCLCPP_INFO(this->get_logger(), "Error sending message");
    }

    //RCLCPP_INFO(this->get_logger(), "JSON sent");
  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  int sockfd_;
  struct sockaddr_in servaddr_;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MinimalSubscriber>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
