
#include "global.h"
#include "mpuXX50.h"
using namespace std;


// IMU configuration and check registries
#define WHO_AM_I        0x75
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C


#define MPU_RA_CONFIG           0x1A
#define MPU_RA_GYRO_CONFIG      0x1B
#define MPU_RA_SMPLRT_DIV       0x19
#define MPU_RA_INT_PIN_CFG      0x37


// Other IMU registeries
#define PWR_MGMT_1  0x6B

// Accelerometer, temperature, and gyroscope data out registries
#define ACCEL_XOUT_H  0x3B
#define ACCEL_XOUT_L  0x3C
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_H  0x3F
#define ACCEL_ZOUT_L  0x40

#define TEMP_OUT_H    0x41
#define TEMP_OUT_L    0x42

#define GYRO_XOUT_H   0x43
#define GYRO_XOUT_L   0x44
#define GYRO_YOUT_H   0x45
#define GYRO_YOUT_L   0x46
#define GYRO_ZOUT_H   0x47
#define GYRO_ZOUT_L   0x48

// Full scale range
#define AFS_2G  0
#define AFS_4G  1
#define AFS_8G  2
#define AFS_16G 3

#define GFS_250DPS  0
#define GFS_500DPS  1
#define GFS_1000DPS 2
#define GFS_2000DPS 3



cMPUXX50::cMPUXX50() 
  : m_fd(-1)
  , m_accRes(1.f)
  , m_gyrRes(1.f)
  , m_accScale(1.f)
  , m_gyrScale(1.f)
  , m_mpuLowPassFilter(INV_FILTER_42HZ)
  , m_temp(0)  
  , m_rawTemp(0)
{
}


// detect mpu-6050 chip
// sensor: MPU6050 or MPU9250
bool cMPUXX50::Init(const string &i2c, eGyroSensor sensor) 
{
  m_accOffset = Vector3(0,0,0);
  m_gyrOffset = Vector3(0,0,0);

  // Start I2C
  if ((m_fd = open(i2c.c_str(), O_RDWR)) < 0) 
  {
      cout << "Failed to open the i2c bus" << endl;
      return false;
  }

  // Connect on 0x68 (mpu-6050)
  if (ioctl(m_fd, I2C_SLAVE, 0x68) < 0)
  {
    cout << "Failed to acquire bus access and/or talk to slave." << endl;
    return false;
  }

  // Check if a valid connection has been established
  m_data[0] = WHO_AM_I;
  write(m_fd, m_data, 1);
  read(m_fd, m_data, 1);
  unsigned char whoAmI = m_data[0];

  if ((whoAmI == 0x68 && sensor == eGyroSensor::MPU6050) 
    || (whoAmI == 0x71 && sensor == eGyroSensor::MPU9250)) {
    // Activate/reset the IMU
    write2bytes(PWR_MGMT_1, 0x80); // device reset
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    write2bytes(MPU_RA_SMPLRT_DIV, 0x00);
    write2bytes(PWR_MGMT_1, 0x03);
    write2bytes(MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0);  // INT_PIN_CFG   -- INT_LEVEL_HIGH, INT_OPEN_DIS, LATCH_INT_DIS, INT_RD_CLEAR_DIS, FSYNC_INT_LEVEL_HIGH, FSYNC_INT_DIS, I2C_BYPASS_EN, CLOCK_DIS
    write2bytes(MPU_RA_CONFIG, m_mpuLowPassFilter);  //CONFIG -- EXT_SYNC_SET 0 (disable input pin for data sync) ; default DLPF_CFG = 0 => ACC bandwidth = 260Hz  GYRO bandwidth = 256Hz)

    // update default resolution
    SetAccelRes(AFS_2G);
    SetGyroRes(GFS_250DPS);
    return true;
  }

  return false;
}


int cMPUXX50::write2bytes(unsigned char byte0, unsigned char byte1) 
{
  m_data[0] = byte0;
  m_data[1] = byte1;
  return write(m_fd, m_data, 2);
}


float cMPUXX50::SetAccelRes(int Ascale) 
{
  // Set the full scale range for the accelerometer
  switch (Ascale)
  {
    case AFS_2G:
      m_accRes = 16384.0f;
      write2bytes(ACCEL_CONFIG, 0x00);
      return m_accRes;
    case AFS_4G:
      m_accRes = 8192.0f;
      write2bytes(ACCEL_CONFIG, Ascale << 3);
      return m_accRes;
    case AFS_8G:
      m_accRes = 4096.0f;
      write2bytes(ACCEL_CONFIG, Ascale << 3);
      return m_accRes;
    case AFS_16G:
      m_accRes = 2048.0f;
      write2bytes(ACCEL_CONFIG, Ascale << 3);
      return m_accRes;
    default:
      return 0;
  }
}


float cMPUXX50::SetGyroRes(int Gscale) 
{
  // Set the full scale range for the gyroscope
  switch (Gscale)
  {
    case GFS_250DPS:
      m_gyrRes = 131.0f;
      m_gyrScale = (1.f / 131.0f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, 0x00);
      return m_gyrRes;
    case GFS_500DPS:
      m_gyrRes = 65.5f;
      m_gyrScale = (1.f / 65.5f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyrRes;
    case GFS_1000DPS:
      m_gyrRes = 32.8f;
      m_gyrScale = (1.f / 32.8f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyrRes;
    case GFS_2000DPS:
      m_gyrRes = 16.4f;
      m_gyrScale = (1.f / 16.4f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyrRes;
    default:
      return 0;
  }
}


void cMPUXX50::ReadRawData(int acc[3], int gyr[3]) 
{
  // Subroutine for reading the raw data
  m_data[0] = ACCEL_XOUT_H;
  write(m_fd, m_data, 1);
  read(m_fd, m_data, 14);

  // Read data - Temperature falls between accel and gyro registers
  acc[0] = (short)(m_data[0] << 8 | m_data[1]);
  acc[1] = (short)(m_data[2] << 8 | m_data[3]);
  acc[2] = (short)(m_data[4] << 8 | m_data[5]);

  gyr[0] = (short)(m_data[8]  << 8 | m_data[9]);
  gyr[1] = (short)(m_data[10] << 8 | m_data[11]);
  gyr[2] = (short)(m_data[12] << 8 | m_data[13]);

  m_rawTemp = (short)(m_data[6] << 8 | m_data[7]);
}


// read calibrationed data
void cMPUXX50::ReadCalData() 
{
  int acc[3], gyr[3];
  ReadRawData(acc, gyr);

  // Convert accelerometer values to g's
  m_acc.x = (acc[0] - m_accOffset.x) / m_accRes;
  m_acc.y = (acc[1] - m_accOffset.y) / m_accRes;
  m_acc.z = (acc[2] - m_accOffset.z) / m_accRes;

  // Convert gyro values to degrees per second  
  m_gyr.x = (gyr[0] - m_gyrOffset.x) / m_gyrRes;
  m_gyr.y = (gyr[1] - m_gyrOffset.y) / m_gyrRes;
  m_gyr.z = (gyr[2] - m_gyrOffset.z) / m_gyrRes;
}


void cMPUXX50::Calibration(int numCalPoints) 
{
  m_accOffset = Vector3(0,0,0);
  m_gyrOffset = Vector3(0,0,0);

  if (numCalPoints <= 0)
    return;

  // Run calibration for given number of points
  for (int i = 0; i < numCalPoints; i++)
  {
    int acc[3], gyr[3];
    ReadRawData(acc, gyr);

    m_gyrOffset.x += gyr[0];
    m_gyrOffset.y += gyr[1];
    m_gyrOffset.z += gyr[2];
    m_accOffset.x += acc[0];
    m_accOffset.y += acc[1];
    m_accOffset.z += acc[2];
  }

  // Find the averge offset values
  m_gyrOffset.x = (m_gyrOffset.x + (numCalPoints / 2)) / numCalPoints;
  m_gyrOffset.y = (m_gyrOffset.y + (numCalPoints / 2)) / numCalPoints;
  m_gyrOffset.z = (m_gyrOffset.z + (numCalPoints / 2)) / numCalPoints;

  m_accOffset.x = (m_accOffset.x + (numCalPoints / 2)) / numCalPoints;
  m_accOffset.y = (m_accOffset.y + (numCalPoints / 2)) / numCalPoints;
  m_accOffset.z = (m_accOffset.z + (numCalPoints / 2)) / numCalPoints - ACCEL_1G;
}

