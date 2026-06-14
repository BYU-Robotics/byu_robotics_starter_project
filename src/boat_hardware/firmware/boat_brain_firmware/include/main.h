#pragma once

void microros_init();
void mpu_init();
void compass_init();
void filter_init();
void thruster_init();
void process_twist(const void * msgin);
void watchdog_check();
bool looptime_check();
void publish_imu_data();
void get_imu_data(float &ax, float &ay, float &az, float &gx, float &gy, float &gz);
void get_compass_data(float &mx, float &my, float &mz);

enum MotorStates{
  DRIVE,
  COAST,
  STOP
};
