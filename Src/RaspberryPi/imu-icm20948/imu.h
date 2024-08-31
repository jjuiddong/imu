//
// 2024-08-03, jjuiddong
// IMU (Inertial Measurement Unit) class
//	- reference
//		- Mateusz Malinowski
//			- https://github.com/mtmal/ICM20948-linux-i2c
//      - Wolfgang (Wolle) Ewald (ICM20948_WE)
//
#pragma once


class cImu
{
public:
	cImu();
	virtual ~cImu();

	bool Init(const string &i2c);
	bool Update(const float dt);


protected:
	bool CalcRollPitchAngle(const float deltaSeconds);
	bool CalcYawAngle(const float deltaSeconds);


public:
	cIcm20948 m_icm; // gyro/accel/magnetic sensor
	Vector3 m_acc; // accelerometer data (g) (directx space)
	Vector3 m_gyr; // gyroscope data (deg/s) (directx space)
	Vector3 m_mag; // magnetometer data (gauss) (directx space)
	Vector3 m_rpy; // sensor roll/pitch/yaw (radian) (directx space)
	Quaternion m_rot; // sensor rotation (directx space)

	bool m_isInitialize; // is initialized?

	enum { LPF_SIZE = 20 }; // lowpassfilter size
	Vector3 m_gbuffer; // gravity vector buffer
	Vector3 m_mbuffer; // magnetic vector buffer
	Vector3 m_gravityV; // filtered gravity vector
	Vector3 m_magV; // filtered magnitude vector
	Vector3 m_sensingCompassV; // sensing magnitude vector
	Vector3 m_projCompassV; // projection magnitude vector
	float m_compassAngle1; // initial forward-mag vector angle
	float m_compassAngle2; // initial right-mag vector angle
	int m_compareType; // 0:angle1 or 1:angle2

	Vector3 m_initGravityV; // initial gravity vector
	Vector3 m_sensingGravityV; // sensing gravity vector

	bool m_isMagCalib; // is magnetic sensor calibration mode?
	deque<std::pair<Vector3, Vector3>> m_magPoses; // magnetic sensor pos while calibration
													// gravity Vector, magnetic vector
	float m_magCalibT; // magnetic sensor calibration time, 0~5sec
	float m_magCalibCheckT; // magnetic sensor calibration check time
	Vector3 m_magCenter; // magnetic sensor center pos
	Vector3 m_magMin;
	Vector3 m_magMax;
};
