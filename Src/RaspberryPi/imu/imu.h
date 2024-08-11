//
// 2024-08-03, jjuiddong
// IMU (Inertial Measurement Unit) class
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
	void CompFilter(float dt, float tau);
	void GetEstimatedAttitude(float dt);
	void RotateV(sVector3 *v, float *delta);

public:
	float m_roll; // degree
	float m_pitch; // degree
	float m_yaw; // degree

	cMPUXX50 m_6050; // gyro/accel sensor
	cHmc5883l m_compass; // magnitude sensor
	int m_accSmooth[3];
	uVector3 m_estG;
	short m_angle[2] = { 0, 0 }; // absolute angle inclination in multiple of 0.1 degree, 180 deg = 1800
	float m_anglerad[2] = { 0.0f, 0.0f }; // absolute angle inclination in radians

};
