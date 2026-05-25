#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <sensor_msgs/msg/imu.h>
#include <geometry_msgs/msg/twist.h>
#include "main.h"

// PIN SETUP ON MICROCONTROLLER
uint8_t LEFT_THRUSTER_PIN = 9;
uint8_t RIGHT_THRUSTER_PIN = 10;

// SETTINGS
int REVERSE_MOTOR_LEFT = 1;
int REVERSE_MOTOR_RIGHT = 1;
bool MPU2_ACTIVE = false;

// Variables
Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2;

rcl_publisher_t publisher;
rcl_subscription_t subscriber;
rcl_allocator_t allocator;
rclc_support_t support;
rclc_executor_t executor;
rcl_node_t node;
sensor_msgs__msg__Imu imu_msg;
geometry_msgs__msg__Twist twist_msg;

void setup() {
  Serial.begin(115200);

  microros_init();
  mpu_init();
  thruster_init();
}

void loop() {

  publish_imu_data();
}
void microros_init(){
  set_microros_serial_transports(Serial);
  allocator = rcl_get_default_allocator();

  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "boat_controller_node", "", &support);
  
  // Create subscriber for cmd_vel topic (commented out for now, as we are only publishing IMU data)
  rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel");
  
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &subscriber, &twist_msg, &process_twist, ON_NEW_DATA);

  rclc_publisher_init_best_effort(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
    "boat_imu_publisher");
}
void mpu_init(){
  while(!mpu1.begin()){
    Serial.println("Failed to Find MPU6050 - 1 Chip");
    delay(1000);
  }
  Serial.println("Found MPU6050-1");
  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setFilterBandwidth(MPU6050_BAND_44_HZ);

  if(MPU2_ACTIVE){
    while(!mpu2.begin()){
      Serial.println("Failed to Find MPU6050 - 2 Chip");
      delay(1000);
    }
    Serial.println("Found MPU6050 - 2");
    mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu2.setFilterBandwidth(MPU6050_BAND_44_HZ);
  } 
}
void thruster_init(){
  pinMode(LEFT_THRUSTER_PIN,OUTPUT);
  pinMode(RIGHT_THRUSTER_PIN,OUTPUT);
}
void publish_imu_data(){
  sensors_event_t a1, g1, temp1;
  mpu1.getEvent(&a1, &g1, &temp1);

  if(MPU2_ACTIVE){
    sensors_event_t a2, g2, temp2;
    mpu2.getEvent(&a2, &g2, &temp2);

    // Averaging two MPUS
    imu_msg.linear_acceleration.x = (a1.acceleration.x + a2.acceleration.x)/2;
    imu_msg.linear_acceleration.y = (a1.acceleration.y + a2.acceleration.y)/2;
    imu_msg.linear_acceleration.z = (a1.acceleration.z + a2.acceleration.z)/2;

    imu_msg.angular_velocity.x = (g1.gyro.x + g2.gyro.x)/2;
    imu_msg.angular_velocity.y = (g1.gyro.y + g2.gyro.y)/2;
    imu_msg.angular_velocity.z = (g1.gyro.z + g2.gyro.z)/2;
  }
  else{
    imu_msg.linear_acceleration.x = (a1.acceleration.x);
    imu_msg.linear_acceleration.y = (a1.acceleration.y);
    imu_msg.linear_acceleration.z = (a1.acceleration.z);

    imu_msg.angular_velocity.x = (g1.gyro.x);
    imu_msg.angular_velocity.y = (g1.gyro.y);
    imu_msg.angular_velocity.z = (g1.gyro.z);
  }
  

  auto return_Value = rcl_publish(&publisher, &imu_msg, NULL);
}

void process_twist(const void * msgin){
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
  // Process the received Twist message (e.g., control the boat based on cmd_vel)

  // For Debugging
  // Serial.print("Received cmd_vel - Linear X: ");
  // Serial.print(msg->linear.x);
  // Serial.print(", Angular Z: ");
  // Serial.println(msg->angular.z);
  
  int normalize_linear = 255 * msg->linear.x;
  int normalize_angular = 255 * msg->angular.z;

  int left_thrust = REVERSE_MOTOR_LEFT * (normalize_linear + normalize_angular);
  int right_thrust = REVERSE_MOTOR_RIGHT * (normalize_linear - normalize_angular);

  analogWrite(LEFT_THRUSTER_PIN,left_thrust);
  analogWrite(RIGHT_THRUSTER_PIN,right_thrust);

}