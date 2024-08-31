//
// 2024-08-26, jjuiddong
// icm-20948 driver class
//  - reference
//		- Mateusz Malinowski
//			- https://github.com/mtmal/ICM20948-linux-i2c
//      - Wolfgang (Wolle) Ewald (ICM20948_WE)
//
#pragma once


enum eICM20948_GYRO_RANGE {
    ICM20948_GYRO_RANGE_250, 
    ICM20948_GYRO_RANGE_500, 
    ICM20948_GYRO_RANGE_1000, 
    ICM20948_GYRO_RANGE_2000,
};

enum eICM20948_DLPF {
    ICM20948_DLPF_0, 
    ICM20948_DLPF_1, 
    ICM20948_DLPF_2, 
    ICM20948_DLPF_3, 
    ICM20948_DLPF_4, 
    ICM20948_DLPF_5, 
    ICM20948_DLPF_6, 
    ICM20948_DLPF_7, 
    ICM20948_DLPF_OFF
};

enum eICM20948_ACC_RANGE {
    ICM20948_ACC_RANGE_2G, 
    ICM20948_ACC_RANGE_4G, 
    ICM20948_ACC_RANGE_8G, 
    ICM20948_ACC_RANGE_16G,
};


class cIcm20948
{
public:
    cIcm20948();
    virtual ~cIcm20948();

    bool Init(const string &i2c);
    bool Calibration();
    bool ReadSensor();
    bool SetAccRange(eICM20948_ACC_RANGE range);
    bool SetAccDLPF(eICM20948_DLPF dlpf);
    bool SetGyrRange(eICM20948_GYRO_RANGE range);
    bool SetGyrDLPF(eICM20948_DLPF dlpf);
    bool Sleep(bool isSleep);
    void Clear();


protected:
    void Reset();
    bool WhoAmI();
    bool WhoAmI_Mag();
    bool InitMag();
    bool ConfigureMasterI2C();
    bool SwitchBank(uchar newBank);
    bool ReadAllData();
    bool ReadRawGyro(short out[3]);
    bool ReadRawAcc(short out[3]);
    bool ReadRawMag(short out[3]);
    int Write1Byte(uchar bank, uchar addr, uchar data);
    uchar Read1Byte(uchar bank, uchar addr);
    uchar ReadNBytes(uchar bank, uchar addr, uchar length);
    int MagI2CRead(const uchar regAddr, const uchar length);
    int MagI2CWrite(const uchar regAddr, const uchar value);
    void Delay(const short miliseconds);


public:
	sIMUData m_data; // imu data    
    int m_fd; // i2c file description
    Vector3 m_accOffset; // accelerometer calibration offset
    Vector3 m_gyrOffset; // gyro calibration offset
    float m_accScale;
    float m_gyrScale;
    uint m_curBank;
    uchar m_buffer[32]; // i2c buffer  
};
