//
// 2024-08-02, jjuiddong
// MPU-6050 sensor class
//  - reference
//    - MarkSherstan
//    - https://github.com/MarkSherstan/MPU-6050-9250-I2C-CompFilter?tab=readme-ov-file
//
#pragma once

#include "global.h"


enum lpf_e 
{
    INV_FILTER_256HZ_NOLPF2 = 0,
    INV_FILTER_188HZ,
    INV_FILTER_98HZ,
    INV_FILTER_42HZ,
    INV_FILTER_20HZ,
    INV_FILTER_10HZ,
    INV_FILTER_5HZ,
    INV_FILTER_2100HZ_NOLPF,
    NUM_FILTER
};

enum gyro_fsr_e 
{
    INV_FSR_250DPS = 0,
    INV_FSR_500DPS,
    INV_FSR_1000DPS,
    INV_FSR_2000DPS,
    NUM_GYRO_FSR
};

enum clock_sel_e 
{
    INV_CLK_INTERNAL = 0,
    INV_CLK_PLL,
    NUM_CLK
};

enum accel_fsr_e 
{
    INV_FSR_2G = 0,
    INV_FSR_4G,
    INV_FSR_8G,
    INV_FSR_16G,
    NUM_ACCEL_FSR
};


// this is the 1G measured acceleration., range: +-2G
const static unsigned short ACCEL_1G = 16384;


class cMPUXX50 
{
public:
  cMPUXX50();

  bool Init(const string &i2c, eGyroSensor sensor);
  void ReadCalData();
  float SetAccelRes(int Ascale);
  float SetGyroRes(int Gscale);
  void Calibration(int numCalPoints = 1000);


protected:
  int write2bytes(unsigned char byte0, unsigned char byte1);
  void ReadRawData(int acc[3], int gyr[3]);  


public:
  int m_fd; // i2c file description
  short m_rawAcc[3]; // accelerometer raw data
  short m_rawGyr[3]; // gyroscope raw data
  short m_rawMag[3]; // compass raw data
  short m_rawTemp; // temperature raw data
  Vector3 m_acc; // Raw data from accelerometer in G's.
  Vector3 m_gyr; // Raw angular rate from gyroscope in degree/s.
  Vector3 m_mag; // Raw data from compass in uT.
  float m_temp; // The temperature in Celsius degrees.

  float m_accRes; // accel resolution
  float m_gyrRes; // gyro resolution
  float m_accScale; // accel scale
  float m_gyrScale; // gyro scale
  Vector3 m_accOffset; // accelerometer calibration offset
  Vector3 m_gyrOffset; // gyro calibration offset
  int m_mpuLowPassFilter; // default:42
  unsigned char m_data[14]; // i2c buffer  
};

