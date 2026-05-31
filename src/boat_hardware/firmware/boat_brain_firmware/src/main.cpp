// TODO Document this code for future reference and for others to understand the code better.
// TODO Test if publisher continues to publish even if one IMU fails, and if it recovers when the IMU starts working again, maybe send message to main system.
// TODO Implement a service to change settings on the IMUs, such as filter bandwidth and accelerometer range

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <QMC5883LCompass.h>
#include <MahonyAHRS.h>

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

// Gyro bias correction values (run calibration program and then copy the values here for better accuracy)
// Calibrated by Milkfries on May 30, 2026 in Provo, Utah
float GYRO_BIAS_X_1 = -.04;
float GYRO_BIAS_Y_1 = -.00;
float GYRO_BIAS_Z_1 = -.02;

float GYRO_BIAS_X_2 = -.04;
float GYRO_BIAS_Y_2 = .01;
float GYRO_BIAS_Z_2 = -.02;

// SETTINGS
int REVERSE_MOTOR_LEFT = 1;
int REVERSE_MOTOR_RIGHT = 1;
bool GPS_ENABLED = false; // GPS is not implemented yet, but this variable can be used in the future to enable or disable GPS functionality

// Sensor variables
Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2;
QMC5883LCompass compass;
Mahony filter;

// ROS2 variables
rcl_publisher_t publisher;
rcl_subscription_t subscriber;
rcl_allocator_t allocator;
rclc_support_t support;
rclc_executor_t executor;
rcl_node_t node;
geometry_msgs__msg__Twist twist_msg;

// State variables
bool imu_one_active = false;
bool imu_two_active = false;
MotorStates motor_state;

// Misc. variables
unsigned long last_msg_time = 0;
unsigned long loopTimer = 0;

void setup() {
  Serial.begin(921600);
  mpu_init();
  compass_init();
  filter_init();
  microros_init();
  thruster_init();
  last_msg_time = millis();
  loopTimer = micros();
}

void loop() {
  // publish all sensor data every loop
  watchdog_check();
  if(looptime_check()){ // Publish at 50Hz, which is the output rate of the compass
    publish_imu_data();
  }
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));
}
void microros_init(){
  rmw_uros_sync_session(1000); // Synchronize with the micro-ROS agent, with a timeout of 1000 milliseconds (1 second)
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
void compass_init(){
  compass.init();
  compass.setMode(0x01, 0x04, 0x00, 0x00); // Set mode to continuous, 50Hz data output rate, 2G range, and 512 oversampling for better accuracy
  // compass.setSmoothing(5, true); // Number 1-10 higher is smoother but more lag, true is for advanced smoothing algorithm that is better for irregular movements (complex calculations), false is a simple moving average
  // compass.setCalibration( -100, 100, -100, 100, -100, 100); // Set calibration values calculated by the calibrate sketch in the QMC5883LCompass library
}
void filter_init(){
  // Initialize Mahony filter with sample frequency of 50Hz, and default values for two other parameters
  filter.begin(50);
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
  motor_state = DRIVE;
}
void publish_imu_data(){
  // Get data from IMU and compass, process it, and fill in the message to be published
  sensor_msgs__msg__Imu quaternion_msg;
  memset(&quaternion_msg, 0, sizeof(sensor_msgs__msg__Imu));
  float ax, ay, az; // Linear acceleration data
  float gx, gy, gz; // Angular velocity data
  float mx, my, mz; // Magnetic (compass) data

  // Get data from sensors
  get_imu_data(ax, ay, az, gx, gy, gz);
  if(GPS_ENABLED){
    get_compass_data(mx, my, mz);
  }

  // POSSIBLE CHANGE: Align axis in case they are not aligned with the boat's forward direction

  // Fill in the quaternion_msg with data from the IMU and compass
  float qx, qy, qz, qw; // Quaternion data
  if(GPS_ENABLED){
    filter.update(gx * RAD_TO_DEG, gy * RAD_TO_DEG, gz * RAD_TO_DEG, ax, ay, az, mx, my, mz); // Update Mahony filter with new data  }
  }
  else{
    filter.updateIMU(gx * RAD_TO_DEG, gy * RAD_TO_DEG, gz * RAD_TO_DEG, ax, ay, az); // Update Mahony filter without compass data
  }
  
  filter.getQuaternion(qx, qy, qz, qw);


  // Fill and publish the message
  quaternion_msg.orientation.x = qx;
  quaternion_msg.orientation.y = qy;
  quaternion_msg.orientation.z = qz;
  quaternion_msg.orientation.w = qw;
  quaternion_msg.linear_acceleration.x = ax;
  quaternion_msg.linear_acceleration.y = ay;
  quaternion_msg.linear_acceleration.z = az;
  quaternion_msg.angular_velocity.x = gx;
  quaternion_msg.angular_velocity.y = gy;
  quaternion_msg.angular_velocity.z = gz;

  // Fill in the header with timestamp and frame id
  int64_t time_ns = rmw_uros_epoch_nanos();
  quaternion_msg.header.stamp.sec = time_ns / 1000000000;
  quaternion_msg.header.stamp.nanosec = time_ns % 1000000000;
  quaternion_msg.header.frame_id.data = (char*)"imu_link"; // TODO Change frame id to something more appropriate if needed
  quaternion_msg.header.frame_id.size = strlen(quaternion_msg.header.frame_id.data);
  quaternion_msg.header.frame_id.capacity = quaternion_msg.header.frame_id.size + 1;
  auto return_Value = rcl_publish(&publisher, &quaternion_msg, NULL);
}

void process_twist(const void * msgin){
  last_msg_time = millis();
  motor_state = DRIVE;
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
  // Process the received Twist message (e.g., control the boat based on cmd_vel)
  
  // Normalize the linear and angular velocities to a range of -255 to 255 for motor control
  int normalize_linear = 255 * msg->linear.x;
  int normalize_angular = 255 * msg->angular.z;

  // Calculate the left and right thruster values based on the normalized linear and angular velocities
  int left_thrust = normalize_linear + normalize_angular;
  int right_thrust = normalize_linear - normalize_angular;

  // Get the absolute maximum thrust
  int abs_max_thrust = std::max(abs(left_thrust), abs(right_thrust));

  // Scale down left and right thrust values if the absolute maximum thrust exceeds 255 to maintain the ratio between them
  if(abs_max_thrust > 255){
    left_thrust = ((float)left_thrust/(float)abs_max_thrust)*255;
    right_thrust = ((float)right_thrust/(float)abs_max_thrust)*255;
  }
  
  // State machine for controlling the thrusters based on the motor state
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
void watchdog_check(){
  // Check if we have received a message from ROS2 in the last 2 seconds, if not, stop the boat for safety
  if(millis() - last_msg_time > 2000){
    motor_state = STOP;
  }
}
bool looptime_check(){
  if(micros() - loopTimer > 20000){ // Publish at 50Hz, which is the output rate of the compass
    loopTimer += 20000;
    return true;
  }
  return false;
}
void get_imu_data(float& ax, float& ay, float& az, float& gx, float& gy, float& gz){
  // Get data from one or two imus
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;
  bool mpu1_ok = imu_one_active && mpu1.getEvent(&a1, &g1, &temp1);
  bool mpu2_ok = imu_two_active && mpu2.getEvent(&a2, &g2, &temp2);

  // Apply gyro bias correction
  g1.gyro.x -= GYRO_BIAS_X_1;
  g1.gyro.y -= GYRO_BIAS_Y_1;
  g1.gyro.z -= GYRO_BIAS_Z_1;

  g2.gyro.x -= GYRO_BIAS_X_2;
  g2.gyro.y -= GYRO_BIAS_Y_2;
  g2.gyro.z -= GYRO_BIAS_Z_2;

  if(mpu1_ok && mpu2_ok){    
    // Averaging two IMUs for more accurate data, if both are active
    ax = (a1.acceleration.x + a2.acceleration.x)/2;
    ay = (a1.acceleration.y + a2.acceleration.y)/2;
    az = (a1.acceleration.z + a2.acceleration.z)/2;

    gx = (g1.gyro.x + g2.gyro.x)/2;
    gy = (g1.gyro.y + g2.gyro.y)/2;
    gz = (g1.gyro.z + g2.gyro.z)/2;
  }
  else if(mpu1_ok){
    ax = (a1.acceleration.x);
    ay = (a1.acceleration.y);
    az = (a1.acceleration.z);

    gx = (g1.gyro.x);
    gy = (g1.gyro.y);
    gz = (g1.gyro.z);
  }
  else if(mpu2_ok){
    ax = (a2.acceleration.x);
    ay = (a2.acceleration.y);
    az = (a2.acceleration.z);

    gx = (g2.gyro.x);
    gy = (g2.gyro.y);
    gz = (g2.gyro.z);
  }
  else{
    // If both IMUs are not working, send message to main system and stop the boat for safety
    ax = 0;
    ay = 0;
    az = 0;
    
    gx = 0;
    gy = 0;
    gz = 0;
    motor_state = STOP;
  }
}
void get_compass_data(float& mx, float &my, float& mz){
  compass.read();
  mx = compass.getX();
  my = compass.getY();
  mz = compass.getZ();
}