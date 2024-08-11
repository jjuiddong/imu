
#include "global.h"
#include "hmc5883l.h"

using namespace std;


#define MAG_ADDRESS 0x1E // HMC5883L sensor address
#define MAG_DATA_REGISTER 0x03

#define HMC58X3_R_CONFA 0
#define HMC58X3_R_CONFB 1
#define HMC58X3_R_MODE 2
#define HMC58X3_X_SELF_TEST_GAUSS (+1.16f)       // X axis level when bias current is applied.
#define HMC58X3_Y_SELF_TEST_GAUSS (+1.16f)       // Y axis level when bias current is applied.
#define HMC58X3_Z_SELF_TEST_GAUSS (+1.08f)       // Z axis level when bias current is applied.
#define SELF_TEST_LOW_LIMIT  (243.0f / 390.0f)    // Low limit when gain is 5.
#define SELF_TEST_HIGH_LIMIT (575.0f / 390.0f)    // High limit when gain is 5.
#define HMC_POS_BIAS 1
#define HMC_NEG_BIAS 2



cHmc5883l::cHmc5883l()
  : m_fd(-1)
{	
  memset(m_mag, 0, sizeof(m_mag));
}

cHmc5883l::~cHmc5883l()
{	
}


// detect HMC5883L chip
bool cHmc5883l::Init(const string &i2c)
{
  // Start I2C
  if ((m_fd = open(i2c.c_str(), O_RDWR)) < 0) 
  {
      cout << "Failed to open the i2c bus (HMC5883L)" << endl;
      return false;
  }

  // Connect on 0x1E (HMC5883L)
  if (ioctl(m_fd, I2C_SLAVE, MAG_ADDRESS) < 0)
  {
    cout << "Failed to acquire bus access and/or talk to slave. (HMC5883L)" << endl;
    return false;
  }

  // Check if a valid connection has been established
  m_data[0] = 0x0A;
  write(m_fd, m_data, 1);
  const int ack = read(m_fd, m_data, 1);  
  if (!ack || (m_data[0] != 'H'))
  {
    cout << "error!, not found HMC5883L sensor fd: " << m_fd << ", ack:" << ack << endl;
  	return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  write2bytes(HMC58X3_R_CONFA, 0x010 + HMC_POS_BIAS);   // Reg A DOR = 0x010 + MS1, MS0 set to pos bias
  // Note that the very first measurement after a gain change maintains the same gain as the previous setting.
  // The new gain setting is effective from the second measurement and on.
  write2bytes(HMC58X3_R_CONFB, 0x60); // Set the Gain to 2.5Ga (7:5->011)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ReadRawData();

  // Collect 10 samples
  bool bret = true; // Error indicator
  int xyz_total[3] = { 0, 0, 0 }; // 32 bit totals so they won't overflow.
  for (int i = 0; i < 10; i++) 
  {
      write2bytes(HMC58X3_R_MODE, 1);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      ReadRawData(); // Get the raw values in case the scales have already been changed.

      // Since the measurements are noisy, they should be averaged rather than taking the max.
      xyz_total[X] += m_mag[X];
      xyz_total[Y] += m_mag[Y];
      xyz_total[Z] += m_mag[Z];

      // Detect saturation.
      if (-4096 >= std::min(m_mag[X], std::min(m_mag[Y], m_mag[Z]))) 
      {
          bret = false;
          break; // Breaks out of the for loop.  No sense in continuing if we saturated.
      }
  }

  // Apply the negative bias. (Same gain)
  write2bytes(HMC58X3_R_CONFA, 0x010 + HMC_NEG_BIAS); // Reg A DOR = 0x010 + MS1, MS0 set to negative bias.
  for (int i = 0; i < 10; i++) 
  {
      write2bytes(HMC58X3_R_MODE, 1);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      ReadRawData(); // Get the raw values in case the scales have already been changed.

      // Since the measurements are noisy, they should be averaged.
      xyz_total[X] -= m_mag[X];
      xyz_total[Y] -= m_mag[Y];
      xyz_total[Z] -= m_mag[Z];

      // Detect saturation.
      if (-4096 >= std::min(m_mag[X], std::min(m_mag[Y], m_mag[Z]))) 
      {
          bret = false;
          break; // Breaks out of the for loop.  No sense in continuing if we saturated.
      }
  }

  m_gain[X] = fabsf(660.0f * HMC58X3_X_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[X]);
  m_gain[Y] = fabsf(660.0f * HMC58X3_Y_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[Y]);
  m_gain[Z] = fabsf(660.0f * HMC58X3_Z_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[Z]);

  // leave test mode
  write2bytes(HMC58X3_R_CONFA, 0x70);   // Configuration Register A  -- 0 11 100 00  num samples: 8 ; output rate: 15Hz ; normal measurement mode
  write2bytes(HMC58X3_R_CONFB, 0x20);   // Configuration Register B  -- 001 00000    configuration gain 1.3Ga
  write2bytes(HMC58X3_R_MODE, 0x00);    // Mode register             -- 000000 00    continuous Conversion Mode
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Something went wrong so get a best guess
  if (!bret) 
  {
      m_gain[X] = 1.0f;
      m_gain[Y] = 1.0f;
      m_gain[Z] = 1.0f;
  }

	return true;
}


int cHmc5883l::write2bytes(unsigned char byte0, unsigned char byte1) 
{
  m_data[0] = byte0;
  m_data[1] = byte1;
  return write(m_fd, m_data, 2);
}


void cHmc5883l::ReadRawData()
{
  unsigned char buf[6];
  short mag[3];

  // Subroutine for reading the raw data
  buf[0] = MAG_DATA_REGISTER;
  write(m_fd, buf, 1);
  read(m_fd, buf, 6);
  // During calibration, magGain is 1.0, so the read returns normal non-calibrated values.
  // After calibration is done, magGain is set to calculated gain values.
  mag[X] = (short)(buf[0] << 8 | buf[1]) * m_gain[X];
  mag[Z] = (short)(buf[2] << 8 | buf[3]) * m_gain[Y];
  mag[Y] = (short)(buf[4] << 8 | buf[5]) * m_gain[Z];

  m_mag[X] = mag[X];
  m_mag[Y] = mag[Y];
  m_mag[Z] = mag[Z];
}
