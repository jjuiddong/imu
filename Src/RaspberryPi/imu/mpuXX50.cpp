
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
  , m_gyroRes(1.f)
  , m_accScale(1.f)
  , m_gyroScale(1.f)
  , m_mpuLowPassFilter(INV_FILTER_42HZ)
  , m_roll(0.f)
  , m_pitch(0.f)
  , m_yaw(0.f)
  , m_temperature(0)  
  , m_accelPitch(0.f)
  , m_accelRoll(0.f)
{
  memset(m_gyro, 0, sizeof(m_gyro));
  memset(m_acc, 0, sizeof(m_acc));
}


// detect mpu-6050 chip
// sensor: MPU6050 or MPU9250
bool cMPUXX50::Init(const string &i2c, eGyroSensor sensor) 
{
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
      m_gyroRes = 131.0f;
      m_gyroScale = (1.f / 131.0f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, 0x00);
      return m_gyroRes;
    case GFS_500DPS:
      m_gyroRes = 65.5f;
      m_gyroScale = (1.f / 65.5f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyroRes;
    case GFS_1000DPS:
      m_gyroRes = 32.8f;
      m_gyroScale = (1.f / 32.8f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyroRes;
    case GFS_2000DPS:
      m_gyroRes = 16.4f;
      m_gyroScale = (1.f / 16.4f) * (M_PI / 180.f);
      write2bytes(GYRO_CONFIG, Gscale << 3);
      return m_gyroRes;
    default:
      return 0;
  }
}


void cMPUXX50::ReadRawData(int acc[3], int gyro[3]) 
{
  // Subroutine for reading the raw data
  m_data[0] = ACCEL_XOUT_H;
  write(m_fd, m_data, 1);
  read(m_fd, m_data, 14);

  // Read data - Temperature falls between accel and gyro registers
  acc[0] = (short)(m_data[0] << 8 | m_data[1]);
  acc[1] = (short)(m_data[2] << 8 | m_data[3]);
  acc[2] = (short)(m_data[4] << 8 | m_data[5]);

  gyro[0] = (short)(m_data[8]  << 8 | m_data[9]);
  gyro[1] = (short)(m_data[10] << 8 | m_data[11]);
  gyro[2] = (short)(m_data[12] << 8 | m_data[13]);

  m_temperature = (short)(m_data[6] << 8 | m_data[7]);
}


void cMPUXX50::ReadCalData() 
{
  // Get new data
  int acc[3], gyro[3];
  ReadRawData(acc, gyro);

  // Convert accelerometer values to g's
  // m_imu.ax = m_imuRaw.ax / m_aRes;
  // m_imu.ay = m_imuRaw.ay / m_aRes;
  // m_imu.az = m_imuRaw.az / m_aRes;

  // // Remove gyro offset
  // m_imu.gx = m_imuRaw.gx - m_gyroOffset[X];
  // m_imu.gy = m_imuRaw.gy - m_gyroOffset[Y];
  // m_imu.gz = m_imuRaw.gz - m_gyroOffset[Z];

  // // Convert gyro values to degrees per second
  // m_imu.gx /= m_gRes;
  // m_imu.gy /= m_gRes;
  // m_imu.gz /= m_gRes;


  // Convert accelerometer values to g's
  m_acc[ROLL] = acc[ROLL] - m_accelOffset[ROLL];
  m_acc[PITCH] = acc[PITCH] - m_accelOffset[PITCH];
  m_acc[YAW] = acc[YAW] - m_accelOffset[YAW];

  // Remove gyro offset
  m_gyro[X] = gyro[X] - m_gyroOffset[X];
  m_gyro[Y] = gyro[Y] - m_gyroOffset[Y];
  m_gyro[Z] = gyro[Z] - m_gyroOffset[Z];
}


// Complementary filter
// dt: delta second
void cMPUXX50::CompFilter(float dt, float tau) 
{
  // Read calibrated data
  ReadCalData();

  const float ax = m_acc[ROLL] / m_accRes;
  const float ay = m_acc[PITCH] / m_accRes;
  const float az = m_acc[YAW] / m_accRes;
  const float gx = m_gyro[X] / m_gyroRes;
  const float gy = m_gyro[Y] / m_gyroRes;
  const float gz = m_gyro[Z] / m_gyroRes;

  m_accelPitch = atan2(ay, az) * (180 / M_PI);
  m_accelRoll = atan2(ax, az) * (180 / M_PI);

  m_roll = (tau)*(m_roll - gy * dt) + (1.f - tau)*(m_accelRoll);
  m_pitch = (tau)*(m_pitch + gx * dt) + (1.f - tau)*(m_accelPitch);
  m_yaw += gz * dt;


  // Complementary filter
  // m_accelPitch = atan2((float)m_imu.ay, (float)m_imu.az) * (180 / M_PI);
  // m_accelRoll = atan2((float)m_imu.ax, (float)m_imu.az) * (180 / M_PI);

  // m_roll = (tau)*(m_roll - (float)m_imu.gy * dt) + (1.f - tau)*(m_accelRoll);
  // m_pitch = (tau)*(m_pitch + (float)m_imu.gx * dt) + (1.f - tau)*(m_accelPitch);
  // m_yaw += (float)m_imu.gz * dt;
}


void cMPUXX50::Calibration(int numCalPoints) 
{
  memset(m_gyroOffset, 0, sizeof(m_gyroOffset));
  memset(m_accelOffset, 0, sizeof(m_accelOffset));

  if (numCalPoints <= 0)
    return;

  // Run calibration for given number of points
  for (int i = 0; i < numCalPoints; i++)
  {
    int acc[3], gyro[3];
    ReadRawData(acc, gyro);

    m_gyroOffset[X] += gyro[X];
    m_gyroOffset[Y] += gyro[Y];
    m_gyroOffset[Z] += gyro[Z];
    m_accelOffset[ROLL] += acc[ROLL];
    m_accelOffset[PITCH] += acc[PITCH];
    m_accelOffset[YAW] += acc[YAW];
  }

  // Find the averge offset values
  m_gyroOffset[X] = (m_gyroOffset[X] + (numCalPoints / 2)) / numCalPoints;
  m_gyroOffset[Y] = (m_gyroOffset[Y] + (numCalPoints / 2)) / numCalPoints;
  m_gyroOffset[Z] = (m_gyroOffset[Z] + (numCalPoints / 2)) / numCalPoints;

  m_accelOffset[ROLL] = (m_accelOffset[ROLL] + (numCalPoints / 2)) / numCalPoints;
  m_accelOffset[PITCH] = (m_accelOffset[PITCH] + (numCalPoints / 2)) / numCalPoints;
  m_accelOffset[YAW] = (m_accelOffset[YAW] + (numCalPoints / 2)) / numCalPoints - ACCEL_1G;
}

