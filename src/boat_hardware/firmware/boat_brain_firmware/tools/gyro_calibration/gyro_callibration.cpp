// This program is used to calculate the gyro bias of the two IMUs on the boat for better accuracy in the main program.
// To use, upload this program to the boat, open the serial monitor, and send the character 's' to start the calibration process. 
// The program will then read 500 samples from each IMU, average the gyro readings, and print the bias values to the serial monitor. 
// These values can then be copied and pasted into the main program for gyro bias correction.
// The calibration should be done in a stable environment with the boat stationary for best results.
// It can be ran multiple times as needed to ensure accurate bias values.

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2;

void mpu_init();
void calculate_bias();

void setup() {
  Serial.begin(115200);
  mpu_init();
}

void loop() {
  calculate_bias();
}
void mpu_init(){
  // First IMU at addres 0x68, AD0 pin is LOW
  if(mpu1.begin(0x68)){
    mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu1.setFilterBandwidth(MPU6050_BAND_44_HZ);
  }
  // Second IMU at addres 0x69, AD0 pin is HIGH
  if(mpu2.begin(0x69)){
    mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu2.setFilterBandwidth(MPU6050_BAND_44_HZ);
  }
}
void calculate_bias(){
  boolean wait_for_command = true;

  // Wait for user to send 's' to start calibration
  while (wait_for_command) {
    if(Serial.available() > 0){
      char c = Serial.read();
      if(c == 's' || c == 'S'){
        wait_for_command = false;
      }
    }
    delay(10);
  }
  

  // Clear any remaining serial input
  while(Serial.available() > 0){
    Serial.read();
  }

  Serial.println("Calculating IMU bias...");
  float gx1, gy1, gz1; 
  float gx2, gy2, gz2;

  gx1 = gy1 = gz1 = 0;
  gx2 = gy2 = gz2 = 0;

  delay(1000);
  // Get data from sensors
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;

  // Read 500 samples from each IMU and average the gyro readings to calculate bias
  for(int i = 0; i < 500; i++){
    mpu1.getEvent(&a1, &g1, &temp1);
    mpu2.getEvent(&a2, &g2, &temp2);

    gx1 += g1.gyro.x;
    gy1 += g1.gyro.y;
    gz1 += g1.gyro.z;

    gx2 += g2.gyro.x;
    gy2 += g2.gyro.y;
    gz2 += g2.gyro.z;
    delay(2);
  }

  gx1 /= 500;
  gy1 /= 500;
  gz1 /= 500; 

  gx2 /= 500;
  gy2 /= 500;
  gz2 /= 500;

  // Output bias values to serial monitor
  Serial.print("Done! Bias values: \n\n");
  Serial.print("IMU 1 Bias - GX: ");
  Serial.print(gx1);
  Serial.print(" GY: ");
  Serial.print(gy1);
  Serial.print(" GZ: ");
  Serial.println(gz1);

  Serial.print("IMU 2 Bias - GX: ");
  Serial.print(gx2);
  Serial.print(" GY: ");
  Serial.print(gy2);
  Serial.print(" GZ: ");
  Serial.println(gz2);

  Serial.println("\nCopy and paste these values into the main program for gyro bias correction.");
  Serial.println("\nPress s to run the calibration again if needed.\n");
}
