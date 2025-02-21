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
#include "common/common.h"

using std::string;
using namespace common;


#ifndef ushort
    typedef unsigned short ushort;
#endif

#ifndef uint
    typedef unsigned int uint;
#endif


// gyro sensors type
enum class eGyroSensor
{
  MPU6050,
  MPU9250,
};
