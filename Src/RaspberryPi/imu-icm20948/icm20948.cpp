
#include "stdafx.h"
#include "global.h"
#include "icm20948.h"
#include "icm20948_register.h"

using namespace std;


cIcm20948::cIcm20948()
    : m_fd(-1)
    , m_curBank(-1)
    , m_accScale(1.f)
    , m_gyrScale(1.f)
{
}

cIcm20948::~cIcm20948()
{
    Clear();
}


// initialize accel/gyro/magnetic sensor
// i2c: i2c name
bool cIcm20948::Init(const string &i2c)
{
    m_accOffset = Vector3(0,0,0);
    m_gyrOffset = Vector3(0,0,0);

    // Start I2C
    if ((m_fd = open(i2c.c_str(), O_RDWR)) < 0) 
    {
        cout << "Failed to open the i2c bus" << endl;
        return false;
    }

    // Connect on 0x68 (icm-20948)
    if (ioctl(m_fd, I2C_SLAVE, 0x68) < 0)
    {
        cout << "Failed to acquire bus access and/or talk to slave." << endl;
        return false;
    }

    if (!WhoAmI())
    {
        Delay(2000);
        if (!WhoAmI())
        {
            cout << "not detect icm-20948 sensor" << endl;
            return false;
        }
    }

    Reset();

    Write1Byte(3, I2C_MST_CTRL, 0); // Reset I2C master clock

    ConfigureMasterI2C();

    SetGyrDLPF(ICM20948_DLPF_6); // lowest noise
    SetGyrRange(ICM20948_GYRO_RANGE_250); // highest resolution
    SetAccRange(ICM20948_ACC_RANGE_2G);
    SetAccDLPF(ICM20948_DLPF_6);

    Delay(1);
    Sleep(false);
    Write1Byte(2, ODR_ALIGN_EN, 1); // aligns ODR

    if (!InitMag())
    {
        cout << "not detect magnetic sensor" << endl;
        return false;
    }

    return true;
}


// initialize magnetic sensor
bool cIcm20948::InitMag()
{
    int counter = 0;
    bool flag = WhoAmI_Mag();
	while (!flag && (++counter <= 10))
	{
		Delay(100);
		flag = WhoAmI_Mag();
	}
    if (!flag)
        return false;

    Delay(1000);
    MagI2CWrite(AK09916_CNTL2, REG_VAL_MAG_MODE_100HZ);

    Delay(10);
    MagI2CRead(REG_ADD_MAG_DATA, MAG_DATA_LEN);

    return true;
}


// i2c mater configuration
bool cIcm20948::ConfigureMasterI2C()
{
	/* Read the current user control and update it with new configuration to enable I2C master */
	uchar temp = Read1Byte(0, USER_CTRL);
	Write1Byte(0, USER_CTRL, temp | REG_VAL_BIT_I2C_MST_EN);

	/* Set I2C master clock to recommended value. */
	Write1Byte(3, I2C_MST_CTRL, REG_VAL_I2C_MST_CTRL_CLK_400KHZ);

	Delay(10);
    return true;
}


// reset sensor
void cIcm20948::Reset()
{
	MagI2CWrite(AK09916_CNTL2, 0x00);
	Delay(100);

    Write1Byte(0, PWR_MGMT_1, ICM20948_RESET);
    Delay(10); // wait for registers to reset

    const int tempEnable = 1; // temperature
    Write1Byte(0, PWR_MGMT_1, RUN_MODE | (static_cast<uint8_t>(!tempEnable) << 3));

    uint8_t sensorsFlag = SENSORS_ON;
    Write1Byte(0, PWR_MGMT_2, sensorsFlag);
    Delay(10);

	// Reset all settings on magnetometer.
	// NOTE: this will log error as the value is immediately changed back to 0 by the sensor itself.
	MagI2CWrite(AK09916_CNTL3, REG_VAL_MAG_RESET);
	Delay(100);
}


// who am i?
bool cIcm20948::WhoAmI()
{
    const uchar res = Read1Byte(0, WHO_AM_I_ICM20948);
    return 0xea == res;
}


// magnetic sensor who am i?
bool cIcm20948::WhoAmI_Mag()
{
    MagI2CRead(REG_ADD_MAG_WIA1, 2);
    return ((m_buffer[0] == REG_VAL_MAG_WIA1) && (m_buffer[1] == REG_VAL_MAG_WIA2));
}


// calibratin accel/gyro
bool cIcm20948::Calibration()
{
    Vector3 accRawVal, gyrRawVal;
    m_accOffset = Vector3(0,0,0);
    m_gyrOffset = Vector3(0,0,0);

	// Reset the offset so that we perform the fresh calibration
	Write1Byte(2, XG_OFFSET_H, 0);
	Write1Byte(2, XG_OFFSET_L, 0);
	Write1Byte(2, YG_OFFSET_H, 0);
	Write1Byte(2, YG_OFFSET_L, 0);
	Write1Byte(2, ZG_OFFSET_H, 0);
	Write1Byte(2, ZG_OFFSET_L, 0);    

    SetGyrDLPF(ICM20948_DLPF_6); // lowest noise
    SetGyrRange(ICM20948_GYRO_RANGE_250); // highest resolution
    SetAccRange(ICM20948_ACC_RANGE_2G);
    SetAccDLPF(ICM20948_DLPF_6);

    Delay(100);

    for(int i=0; i < 50; i++)
    {
        short acc[3];
        ReadRawAcc(acc);
        m_accOffset.x += (float)acc[0];
        m_accOffset.y += (float)acc[1];
        m_accOffset.z += (float)acc[2];
        Delay(10);
    }

    m_accOffset.x /= 50;
    m_accOffset.y /= 50;
    m_accOffset.z /= 50;
    m_accOffset.z -= 16384.0;

    for(int i=0; i < 50; i++)
    {
        short gyr[3];
        ReadRawGyro(gyr);
        m_gyrOffset.x += (float)gyr[0];
        m_gyrOffset.y += (float)gyr[1];
        m_gyrOffset.z += (float)gyr[2];
        Delay(1);
    }
    
    m_gyrOffset.x /= 50;
    m_gyrOffset.y /= 50;
    m_gyrOffset.z /= 50;

    return true;
}


// set accel range
bool cIcm20948::SetAccRange(eICM20948_ACC_RANGE range)
{
    uchar regVal = Read1Byte(2, ACCEL_CONFIG);
    regVal &= ~(0x06);
    regVal |= (range << 1);
    Write1Byte(2, ACCEL_CONFIG, regVal);
    m_accScale = powf(2.0, range + 1) / 32768;
    return true;
}


// set accel digital low pass filter
bool cIcm20948::SetAccDLPF(eICM20948_DLPF dlpf)
{
    uchar regVal = Read1Byte(2, ACCEL_CONFIG);
    if(ICM20948_DLPF_OFF == dlpf){
        regVal &= 0xFE;
        Write1Byte(2, ACCEL_CONFIG, regVal);
        return true;
    }
    else{
        regVal |= 0x01;
        regVal &= 0xC7;
        regVal |= (dlpf << 3);
    }
    Write1Byte(2, ACCEL_CONFIG, regVal);
    return true;
}


// set gyro range
bool cIcm20948::SetGyrRange(eICM20948_GYRO_RANGE range)
{
    uchar regVal = Read1Byte(2, GYRO_CONFIG_1);
    regVal &= ~(0x06);
    regVal |= (range << 1);
    Write1Byte(2, GYRO_CONFIG_1, regVal);
    m_gyrScale = ((range + 1) * 250.f) / 32768.f;
    return true;
}


// set gyro digital low pass filter
bool cIcm20948::SetGyrDLPF(eICM20948_DLPF dlpf)
{
    uchar regVal = Read1Byte(2, GYRO_CONFIG_1);
    if(ICM20948_DLPF_OFF == dlpf){
        regVal &= 0xFE;
        Write1Byte(2, GYRO_CONFIG_1, regVal);
        return true;
    }
    else{
        regVal |= 0x01;
        regVal &= 0xC7;
        regVal |= (dlpf << 3);
    }
    Write1Byte(2, GYRO_CONFIG_1, regVal);
    return true;
}


// read sensor data
bool cIcm20948::ReadSensor()
{
    ReadAllData();

    // Parse accelerometer data
    Vector3 accRaw;
    accRaw.x = (float)((short)(m_buffer[0] << 8) | (short)m_buffer[1]);
    accRaw.y = (float)((short)(m_buffer[2] << 8) | (short)m_buffer[3]);
    accRaw.z = (float)((short)(m_buffer[4] << 8) | (short)m_buffer[5]);

    m_data.rawAcc[0] = (short)accRaw.x;
    m_data.rawAcc[1] = (short)accRaw.y;
    m_data.rawAcc[2] = (short)accRaw.z;

    // Parse gyroscope data
    Vector3 gyrRaw;
    gyrRaw.x = (float)((short)(m_buffer[6] << 8) | (short)m_buffer[7]);
    gyrRaw.y = (float)((short)(m_buffer[8] << 8) | (short)m_buffer[9]);
    gyrRaw.z = (float)((short)(m_buffer[10] << 8) | (short)m_buffer[11]);

    m_data.rawGyr[0] = (short)gyrRaw.x;
    m_data.rawGyr[1] = (short)gyrRaw.y;
    m_data.rawGyr[2] = (short)gyrRaw.z;

    // Parse temperature data
    m_data.rawTemp = ((short)(m_buffer[12]) << 8) | (short)(m_buffer[13]);

    // Parse magnetic data
    Vector3 magRaw;
    magRaw.x = (float)((short)(m_buffer[15] << 8) | (short)m_buffer[14]);
    magRaw.y = -(float)((short)(m_buffer[17] << 8) | (short)m_buffer[16]);
    magRaw.z = -(float)((short)(m_buffer[19] << 8) | (short)m_buffer[18]);

    m_data.rawMag[0] = (short)magRaw.x;
    m_data.rawMag[1] = (short)magRaw.y;
    m_data.rawMag[2] = (short)magRaw.z;

    // calc accel/gyro/magnetic real data
    accRaw.x = (accRaw.x - m_accOffset.x) * m_accScale;
    accRaw.y = (accRaw.y - m_accOffset.y) * m_accScale;
    accRaw.z = (accRaw.z - m_accOffset.z) * m_accScale;

    gyrRaw.x = (gyrRaw.x - m_gyrOffset.x) * m_gyrScale;
    gyrRaw.y = (gyrRaw.y - m_gyrOffset.y) * m_gyrScale;
    gyrRaw.z = (gyrRaw.z - m_gyrOffset.z) * m_gyrScale;

    magRaw.x *= AK09916_MAG_LSB;
    magRaw.y *= AK09916_MAG_LSB;
    magRaw.z *= AK09916_MAG_LSB;

    // update final data
    m_data.acc[0] = accRaw.x;
    m_data.acc[1] = accRaw.y;
    m_data.acc[2] = accRaw.z;

    m_data.gyr[0] = gyrRaw.x;
    m_data.gyr[1] = gyrRaw.y;
    m_data.gyr[2] = gyrRaw.z;

    m_data.mag[0] = magRaw.x;
    m_data.mag[1] = magRaw.y;
    m_data.mag[2] = magRaw.z;

	m_data.temp = ((m_data.rawTemp - 21) / 333.87f) + 21.0f;

    return true;
}


// read all data
bool cIcm20948::ReadAllData()
{
    SwitchBank(0);

    m_buffer[0] = ACCEL_XOUT_H;
    write(m_fd, m_buffer, 1);
    const int res = read(m_fd, m_buffer, IMU_DATA_LEN);
    return true;
}


// read gyro sensor raw data
bool cIcm20948::ReadRawGyro(short out[3])
{
    ReadNBytes(0, GYRO_XOUT_H, GYRO_AND_ACC_DATA_LEN);
    out[0] = (short)(m_buffer[0] << 8) | (short)m_buffer[1];
    out[1] = (short)(m_buffer[2] << 8) | (short)m_buffer[3];
    out[2] = (short)(m_buffer[4] << 8) | (short)m_buffer[5];
    return true;
}


// read accelerometer sensor raw data
bool cIcm20948::ReadRawAcc(short out[3])
{
    ReadNBytes(0, ACCEL_XOUT_H, GYRO_AND_ACC_DATA_LEN);
    out[0] = (short)(m_buffer[0] << 8) | (short)m_buffer[1];
    out[1] = (short)(m_buffer[2] << 8) | (short)m_buffer[3];
    out[2] = (short)(m_buffer[4] << 8) | (short)m_buffer[5];
    return true;
}


// read magnetic sensor raw data
bool cIcm20948::ReadRawMag(short out[3])
{
    ReadNBytes(0, EXT_SENS_DATA_00, MAG_DATA_LEN);
    out[0] = (short)(m_buffer[1] << 8) | (short)m_buffer[0];
    out[1] = (short)(m_buffer[3] << 8) | (short)m_buffer[2];
    out[2] = (short)(m_buffer[5] << 8) | (short)m_buffer[4];
    return true;
}


// set sleep sensor
bool cIcm20948::Sleep(bool isSleep)
{
    uchar regVal = Read1Byte(0, PWR_MGMT_1);
    if(isSleep){
        regVal |= ICM20948_SLEEP;
    }
    else{
        regVal &= ~ICM20948_SLEEP;
    }
    Write1Byte(0, PWR_MGMT_1, regVal);
    return true;
}


// switch bank
bool cIcm20948::SwitchBank(uchar newBank)
{
    if (m_curBank == newBank)
        return true; // nothing to do
    m_curBank = newBank;
    m_buffer[0] = REG_BANK_SEL;
    m_buffer[1] = newBank << 4;
    write(m_fd, m_buffer, 2);
    return true;
}


// send i2c data
// bank: bank id
// addr: register address
// data: send 1 byte data
int cIcm20948::Write1Byte(uchar bank, uchar addr, uchar data) 
{
    SwitchBank(bank);

    m_buffer[0] = addr;
    m_buffer[1] = data;
    return write(m_fd, m_buffer, 2);
}


// receive i2c data
// bank: bank id
// addr: register address
uchar cIcm20948::Read1Byte(uchar bank, uchar addr)
{
    SwitchBank(bank);

    m_buffer[0] = addr;
    write(m_fd, m_buffer, 1);
    read(m_fd, m_buffer, 1);
    return m_buffer[0];
}


// read n bytes
uchar cIcm20948::ReadNBytes(uchar bank, uchar addr, uchar length)
{
    SwitchBank(bank);

    m_buffer[0] = addr;
    write(m_fd, m_buffer, 1);
    const int len = read(m_fd, m_buffer, length);
    return len;
}


// magnetic sensor read
// regAddr: register address
// length: read byte size
// out: out data
int cIcm20948::MagI2CRead(const uchar regAddr, const uchar length)
{
    Write1Byte(3, I2C_SLV0_ADDR, I2C_ADD_ICM20948_AK09916 | I2C_ADD_ICM20948_AK09916_READ);
    Write1Byte(3, I2C_SLV0_REG, regAddr);
    Write1Byte(3, I2C_SLV0_CTRL, REG_VAL_BIT_SLV0_EN | length);

    const int len = ReadNBytes(0, EXT_SENS_DATA_00, length);
    return len;
}


// magnetic sensor write
int cIcm20948::MagI2CWrite(const uchar regAddr, const uchar value)
{
	Write1Byte(3, I2C_SLV0_ADDR, I2C_ADD_ICM20948_AK09916 | I2C_ADD_ICM20948_AK09916_WRITE);
	Write1Byte(3, I2C_SLV0_REG, regAddr);
	Write1Byte(3, I2C_SLV0_DO, value);
	Write1Byte(3, I2C_SLV0_CTRL, REG_VAL_BIT_SLV0_EN | 1);

	Delay(100);
	const int len = MagI2CRead(regAddr, 1);
    return len;
}


// sleep milisecond
void cIcm20948::Delay(const short miliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
}


// clear
void cIcm20948::Clear()
{
    if (m_fd > 0)
    {
        close(m_fd);
        m_fd = -1;
    }
    m_curBank = -1;
}
