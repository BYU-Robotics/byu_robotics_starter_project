#pragma once

void microros_init();
void mpu_init();
void thruster_init();
void process_twist(const void * msgin);
void publish_imu_data();

enum MotorStates{
  DRIVE,
  COAST,
  STOP
};
