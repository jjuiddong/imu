//
// 2024-08-03, jjuiddong
// global definition
//
#pragma once


#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <string>
#include <cstring>
#include <math.h>

using std::string;


#ifndef ushort
    typedef unsigned short ushort;
#endif

#ifndef uint
    typedef unsigned int uint;
#endif

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

enum eSensorAxis
{
    X = 0,
    Y,
    Z
};

enum eSensorAttitude
{
    ROLL = 0,
    PITCH,
    YAW,
};


// gyro sensors type
enum class eGyroSensor
{
  MPU6050,
  MPU9250,
};


// A structure to hold all data from IMU.
struct sIMUData
{
	float updatePeriod; // The period at which IMU data is being pulled from the sensor.
	short rawTemp; // temperature raw data
	short rawGyr[3]; // gyroscope raw data
	short rawAcc[3]; // accelerometer raw data
	short rawMag[3]; // compass raw data
	float temp; // The temperature in Celsius degrees.
	float gyr[3]; // Raw angular rate from gyroscope in rad/s.
	float acc[3]; // Raw data from accelerometer in G's.
	float mag[3]; // Raw data from compass in uT.
	float quat[4]; // Quaternion representation of IMU orientation.
	float angles[3]; // Euler angles representation of IMU orientation in degrees.

	// Basic constructor which initialises all variables with default values.
	sIMUData() : updatePeriod(0.0f), temp(0.0f)
	{
		rawGyr[0] = rawGyr[1] = rawGyr[2] = 0;
		rawAcc[0] = rawAcc[1] = rawAcc[2] = 0;
		rawMag[0] = rawMag[1] = rawMag[2] = 0;
		gyr[0] = gyr[1] = gyr[2] = 0.0f;
		acc[0] = acc[1] = acc[2] = 0.0f;
		mag[0] = mag[1] = mag[2] = 0.0f;
		quat[0] = 1.0f;
		quat[1] = quat[2] = quat[3] = 0.0f;
		angles[0] = angles[1] = angles[2] = 0.0f;
	};
};




// this is the 1G measured acceleration., range: +-8G
//const static unsigned short ACCEL_1G = 512 * 8;
const static unsigned short ACCEL_1G = 16384;

