//
// 2024-08-02, jjuiddong
// MPU-6050 sensor class
//  - reference
//    - MarkSherstan
//    - https://github.com/MarkSherstan/MPU-6050-9250-I2C-CompFilter?tab=readme-ov-file
//
#pragma once


class cMPUXX50 
{
public:
  cMPUXX50();

  bool Init(const string &i2c, eGyroSensor sensor);
  void ReadCalData();
  void CompFilter(float dt, float tau);
  float SetAccelRes(int Ascale);
  float SetGyroRes(int Gscale);
  void Calibration(int numCalPoints = 1000);


protected:
  int write2bytes(unsigned char byte0, unsigned char byte1);
  void ReadRawData(int acc[3], int gyro[3]);  


public:
  int m_fd; // i2c file description
  float m_accRes; // accel resolution
  float m_gyroRes; // gyro resolution
  float m_accScale; // accel scale
  float m_gyroScale; // gyro scale
  int m_gyroOffset[3]; // gyro callibration offset, x/y/z
  int m_accelOffset[3]; // accel callibration offset, roll/pitch/yaw
  int m_mpuLowPassFilter; // default:42
  unsigned char m_data[14]; // i2c buffer  
  int m_gyro[3]; // gyro sensor data applied callibration
  int m_acc[3]; // accel sensor data applied callibration

  // output
  float m_roll; // final output data, degree
  float m_pitch; // final output data, degree
  float m_yaw; // final output data, degree
  int m_temperature;
  float m_accelPitch; // accel pitch angle (degree/sec2)
  float m_accelRoll; // accel roll angle (degree/sec2)
};

