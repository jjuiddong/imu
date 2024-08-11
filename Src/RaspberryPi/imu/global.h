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


struct sVector3
{
    float x;
    float y;
    float z;
};


union uVector3
{
    float a[3];
    sVector3 v;
};




// this is the 1G measured acceleration., range: +-8G
//const static unsigned short ACCEL_1G = 512 * 8;
const static unsigned short ACCEL_1G = 16384;

