#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/joy.hpp"

class ControllerNode : public rclcpp::Node{
  private:
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscription_; 
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr twist_publisher_;
    void joy_to_twist(const sensor_msgs::msg::Joy::SharedPtr msg){
      auto twist_publish = geometry_msgs::msg::Twist();
      if(msg->axes.size() >= 4){ 
        twist_publish.linear.x = msg->axes.at(1);
        twist_publish.angular.z = msg->axes.at(3);
        // RCLCPP_INFO(this->get_logger(), "Linear: %.2f Angular Z: %.2f", msg->axes.at(1), msg->axes.at(3)); // DEBUGGING
      }
      else{
        twist_publish.linear.x = 0.0;
        twist_publish.angular.z = 0.0;
        RCLCPP_INFO(this->get_logger(), "Controller Error"); 
      }
      twist_publisher_->publish(twist_publish);
    }
  public:
    ControllerNode():Node("ControllerNode"){
      joy_subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("joy",10,std::bind(&ControllerNode::joy_to_twist,this,std::placeholders::_1)); 
      twist_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("telop",10);
    }
};

int main(int argc, char** argv){
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ControllerNode>());
  rclcpp::shutdown();
  return 0;
}