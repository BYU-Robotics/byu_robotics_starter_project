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
#include <algorithm>
// PIN SETUP ON MICROCONTROLLER
uint8_t LEFT_THRUSTER_PIN_A = 12;
uint8_t LEFT_THRUSTER_PIN_B = 13;
uint8_t RIGHT_THRUSTER_PIN_A = 14;
uint8_t RIGHT_THRUSTER_PIN_B = 15;

uint8_t LEFT_THRUSTER_ENABLE_PIN = 25;
uint8_t RIGHT_THRUSTER_ENABLE_PIN = 26;


// SETTINGS
int REVERSE_MOTOR_LEFT = 1;
int REVERSE_MOTOR_RIGHT = 1;
bool MPU2_ACTIVE = true;
bool using_L298N = true;

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

bool imu_one_active = false;
bool imu_two_active = false;

MotorStates motor_state;



void setup() {
  Serial.begin(115200);

  mpu_init();
  microros_init();
  thruster_init();
}

void loop() {
  // publish all sensor data every loop
  publish_imu_data();
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(0));
  delay(20);
  
}
void microros_init(){

  // Define how data will be transmitted
  set_microros_serial_transports(Serial);
  allocator = rcl_get_default_allocator();

  // Initializes node
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "boat_controller_node", "", &support);
  
  // Create subscriber for cmd_vel topic
  rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel");
  
  // Create executor to run a function when recieving a message
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &subscriber, &twist_msg, &process_twist, ON_NEW_DATA);

  // Create publisher for boat_imu topic
  rclc_publisher_init_best_effort(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
    "boat_imu");
}
void mpu_init(){
  // First IMU at addres 0x68, AD0 pin is LOW
  if(mpu1.begin(0x68)){
    imu_one_active = true;
    mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu1.setFilterBandwidth(MPU6050_BAND_44_HZ);
  }
  
  // Second IMU at addres 0x69, AD0 pin is HIGH
  if(mpu2.begin(0x69)){
    imu_two_active = true;
    mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu2.setFilterBandwidth(MPU6050_BAND_44_HZ);
  } 

}
void thruster_init(){
  // Initialize thruster pins
  pinMode(LEFT_THRUSTER_PIN_A,OUTPUT);
  pinMode(LEFT_THRUSTER_PIN_B,OUTPUT);
  pinMode(RIGHT_THRUSTER_PIN_A,OUTPUT);
  pinMode(RIGHT_THRUSTER_PIN_B,OUTPUT);
  pinMode(LEFT_THRUSTER_ENABLE_PIN,OUTPUT);
  pinMode(RIGHT_THRUSTER_ENABLE_PIN,OUTPUT);

  motor_state = DRIVE;
}
void publish_imu_data(){
  // Publish data from one or two imus
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;
  if(imu_one_active && imu_two_active){
    
    mpu1.getEvent(&a1, &g1, &temp1);
    mpu2.getEvent(&a2, &g2, &temp2);

    // Averaging two IMUs for more accurate data, if both are active
    imu_msg.linear_acceleration.x = (a1.acceleration.x + a2.acceleration.x)/2;
    imu_msg.linear_acceleration.y = (a1.acceleration.y + a2.acceleration.y)/2;
    imu_msg.linear_acceleration.z = (a1.acceleration.z + a2.acceleration.z)/2;

    imu_msg.angular_velocity.x = (g1.gyro.x + g2.gyro.x)/2;
    imu_msg.angular_velocity.y = (g1.gyro.y + g2.gyro.y)/2;
    imu_msg.angular_velocity.z = (g1.gyro.z + g2.gyro.z)/2;
  }
  else if(imu_one_active){
    mpu1.getEvent(&a1, &g1, &temp1);
    imu_msg.linear_acceleration.x = (a1.acceleration.x);
    imu_msg.linear_acceleration.y = (a1.acceleration.y);
    imu_msg.linear_acceleration.z = (a1.acceleration.z);

    imu_msg.angular_velocity.x = (g1.gyro.x);
    imu_msg.angular_velocity.y = (g1.gyro.y);
    imu_msg.angular_velocity.z = (g1.gyro.z);
  }
  else if(imu_two_active){
    mpu2.getEvent(&a2, &g2, &temp2);
    imu_msg.linear_acceleration.x = (a2.acceleration.x);
    imu_msg.linear_acceleration.y = (a2.acceleration.y);
    imu_msg.linear_acceleration.z = (a2.acceleration.z);

    imu_msg.angular_velocity.x = (g2.gyro.x);
    imu_msg.angular_velocity.y = (g2.gyro.y);
    imu_msg.angular_velocity.z = (g2.gyro.z);
  }
  else{
    return;
  }
  

  auto return_Value = rcl_publish(&publisher, &imu_msg, NULL);

}

void process_twist(const void * msgin){
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
  // Process the received Twist message (e.g., control the boat based on cmd_vel)
  
  int normalize_linear = 255 * msg->linear.x;
  int normalize_angular = 255 * msg->angular.z;

  int left_thrust = normalize_linear + normalize_angular;
  int right_thrust = normalize_linear - normalize_angular;

  int abs_max_thrust = std::max(abs(left_thrust), abs(right_thrust));

  if(abs_max_thrust > 255){
    left_thrust = ((float)left_thrust/(float)abs_max_thrust)*255;
    right_thrust = ((float)right_thrust/(float)abs_max_thrust)*255;
  }
  
  if(using_L298N){
    switch(motor_state){
      case(DRIVE):
        analogWrite(LEFT_THRUSTER_ENABLE_PIN, abs(left_thrust));
        digitalWrite(LEFT_THRUSTER_PIN_A, left_thrust >= 0 ? LOW : HIGH);
        digitalWrite(LEFT_THRUSTER_PIN_B, left_thrust >= 0 ? HIGH : LOW);
        analogWrite(RIGHT_THRUSTER_ENABLE_PIN, abs(right_thrust));
        digitalWrite(RIGHT_THRUSTER_PIN_A, right_thrust >= 0 ? LOW : HIGH);
        digitalWrite(RIGHT_THRUSTER_PIN_B, right_thrust >= 0 ? HIGH : LOW);
        break;
      case(STOP):
        analogWrite(LEFT_THRUSTER_ENABLE_PIN, 0);
        analogWrite(RIGHT_THRUSTER_ENABLE_PIN, 0);
        digitalWrite(LEFT_THRUSTER_PIN_A, LOW);
        digitalWrite(LEFT_THRUSTER_PIN_B, LOW);
        digitalWrite(RIGHT_THRUSTER_PIN_A, LOW);
        digitalWrite(RIGHT_THRUSTER_PIN_B, LOW);
        break;
      default:
        break;
    }
  }
  else{
    switch(motor_state){
      case(DRIVE):
        analogWrite(LEFT_THRUSTER_PIN_A, constrain(left_thrust,0,255));
        analogWrite(LEFT_THRUSTER_PIN_B, constrain(-left_thrust,0,255));
        analogWrite(RIGHT_THRUSTER_PIN_A, constrain(right_thrust,0,255));
        analogWrite(RIGHT_THRUSTER_PIN_B, constrain(-right_thrust,0,255));
        break;
      case(STOP):
        analogWrite(LEFT_THRUSTER_PIN_A, 0);
        analogWrite(LEFT_THRUSTER_PIN_B, 0);
        analogWrite(RIGHT_THRUSTER_PIN_A, 0);
        analogWrite(RIGHT_THRUSTER_PIN_B, 0);
        break;
      default:
        break;
    }
  }
}