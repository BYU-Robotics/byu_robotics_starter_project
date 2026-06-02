#include "rclcpp/rclcpp.hpp" // ROS 2 C++ client library
#include "geometry_msgs/msg/twist.hpp" // Message type for velocity and angular commands
#include "sensor_msgs/msg/joy.hpp" // Message type for joystick inputs


// This node subscribes to joystick inputs and publishes corresponding velocity commands for teleoperation.
class ControllerNode : public rclcpp::Node{
  private:
    // Instances of the subscription and publisher, using shared pointers for memory management

    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscription_; 
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr twist_publisher_;

    // Callback function that converts joystick inputs to Twist messages and publishes them
    // Parameter msg is a shared pointer to the incoming Joy message
    void joy_to_twist(const sensor_msgs::msg::Joy::SharedPtr msg){
      // Create a Twist message to publish
      auto twist_publish = geometry_msgs::msg::Twist();

      // Check if the joystick message has enough axes to control linear and angular velocities
      if(msg->axes.size() >= 4){ 
        // Map joystick axes to linear and angular velocities
        twist_publish.linear.x = msg->axes.at(1);
        twist_publish.angular.z = msg->axes.at(3);

        // Log the received joystick inputs for debugging purposes
        // RCLCPP_INFO(this->get_logger(), "Linear: %.2f Angular Z: %.2f", msg->axes.at(1), msg->axes.at(3)); // DEBUGGING
      }
      else{
        // If the joystick message does not have enough axes, set velocities to zero and log an error
        twist_publish.linear.x = 0.0;
        twist_publish.angular.z = 0.0;
        RCLCPP_INFO(this->get_logger(), "Controller Error"); 
      }

      // Publish the Twist message to the "telop" topic
      twist_publisher_->publish(twist_publish);
    }
  public:
    // Constructor for the ControllerNode class, which initializes the node and sets up the subscription and publisher
    ControllerNode():Node("ControllerNode"){
      // Create a subscription to the "joy" topic with a queue size of 10, and bind the joy_to_twist callback function to handle incoming messages
      joy_subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("joy",10,std::bind(&ControllerNode::joy_to_twist,this,std::placeholders::_1)); 

      // Create a publisher for the "telop" topic with a queue size of 10, which will publish Twist messages
      twist_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("telop",10);
    }
};

int main(int argc, char** argv){
  // Main function that initializes the ROS 2 system, creates an instance of the ControllerNode, and starts spinning to process callbacks
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ControllerNode>());
  rclcpp::shutdown();
  return 0;
}