//
// 2024-08-03, jjuiddong
// IMU (Inertial Measurement Unit) class
//
// - 2025-02-21
//	- only mpu-6050
//
#pragma once


class cImu
{
public:
	cImu();
	virtual ~cImu();

	bool Init(const string &i2c);
	bool Update(const float dt);
	bool InitYaw(const float yaw);


protected:
	bool CalcRollPitchAngle(const float deltaSeconds);
	bool CalcYawAngle(const float deltaSeconds);


public:
	cMPUXX50 m_6050; // gyro/accel sensor
	Vector3 m_acc; // accelerometer data (g) (directx space)
	Vector3 m_gyr; // gyroscope data (deg/s) (directx space)
	Vector3 m_mag; // magnetometer data (gauss) (directx space)
	Vector3 m_rpy; // sensor roll/pitch/yaw (radian) (directx space)
	float m_yaw; // gyroscope yaw angle to calc m_rpy (degree)
	Quaternion m_rot; // sensor pose

	bool m_isCalibrate; // is initialized?

	enum { LPF_SIZE = 20 }; // lowpassfilter size
	Vector3 m_gbuffer; // lowpass filter gravity vector
	Vector3 m_mbuffer; // lowpass filter magnetic vector
	Vector3 m_gravityV; // filtered gravity vector

	Vector3 m_initGravityV; // initial gravity vector
	Vector3 m_sensingGravityV; // sensing gravity vector	
};
