
#include "stdafx.h"
#include "global.h"
#include "mpuXX50.h"
#include "imu.h"

using namespace std;
using std::string;


cImu::cImu()
	: m_isCalibrate(false)
	, m_yaw(0)
{
}

cImu::~cImu()
{
}


// initialize imu
// i2c: ic2 bus name
bool cImu::Init(const string &i2c)
{
	// Initialize the IMU and set the senstivity values
	cout << "MPU-6050 initialize. Pass/Fail: ";
	const bool result1 = m_6050.Init(i2c, eGyroSensor::MPU6050);
	cout << result1 << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	// Calibrate the gyroscope
	cout << "Calibrating gyroscope, hold IMU stationary." << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(2));
	m_6050.Calibration(1000);

	m_isCalibrate = false;
	m_rot.SetIdentity();
	m_yaw = 0.f;

	// Display calibration values to user
	cout << "---------------------------------------" << endl;
	cout << "Gyroscope bias values:" << endl;
	cout << "\t X: " << m_6050.m_gyrOffset.x << endl;
	cout << "\t Y: " << m_6050.m_gyrOffset.y << endl;
	cout << "\t Z: " << m_6050.m_gyrOffset.z << endl;
	cout << "Gyroscope Resolution value: " << m_6050.m_gyrRes << endl;
	cout << "Accelerometer Resolution value: " << m_6050.m_accRes << endl;	
	cout << "---------------------------------------" << endl;
	cout << "---------------------------------------" << endl;
	cout << "accelerometer bias values:" << endl;
	cout << "\t X: " << m_6050.m_accOffset.x << endl;
	cout << "\t Y: " << m_6050.m_accOffset.y << endl;
	cout << "\t Z: " << m_6050.m_accOffset.z << endl;
	cout << "---------------------------------------" << endl;	
	std::this_thread::sleep_for(std::chrono::milliseconds(2));

	return true;
}


// process imu
// dt: delta second
bool cImu::Update(const float dt)
{
	m_6050.ReadCalData();

	// change space, opengl -> directx
	const Vector3 acc(m_6050.m_acc.x, m_6050.m_acc.y, -m_6050.m_acc.z);
	const Vector3 gyr(m_6050.m_gyr.x, m_6050.m_gyr.y, m_6050.m_gyr.z);
	const Vector3 mag(m_6050.m_mag.x, m_6050.m_mag.y, -m_6050.m_mag.z);

	const Vector3 gravityV = acc.Normal();

	// initialize calibration data
	if (!m_isCalibrate)
	{
		m_isCalibrate = true;
		m_initGravityV = gravityV;
		m_yaw = 0.f;
	}

	m_gbuffer = (gravityV * (1.f / LPF_SIZE)) + (m_gbuffer * ((float)(LPF_SIZE-1) / LPF_SIZE));
	m_gbuffer.Normalize();
	m_gravityV = m_gbuffer;
	m_acc = acc; // for debugging
	m_gyr = gyr; // for debugging
	m_mag = mag; // for debugging

	CalcRollPitchAngle(dt);
	CalcYawAngle(dt);
	return true;
}



// initialize yaw angle to prevent drift error
// yaw: y-axis rotation degree
bool cImu::InitYaw(const float yaw)
{
	m_yaw = yaw;
	return true;
}


// update pitch angle
bool cImu::CalcRollPitchAngle(const float deltaSeconds)
{
	const float dt = deltaSeconds;

	// opengl -> directx
	const Vector3 acc(m_6050.m_acc.x, m_6050.m_acc.y, -m_6050.m_acc.z);
	const Vector3 gyr(m_6050.m_gyr.x, m_6050.m_gyr.y, m_6050.m_gyr.z);
	// const Vector3 mag(m_6050.m_mag.x, m_6050.m_mag.y, -m_6050.m_mag.z);

	const Vector3 gravityV = acc.Normal();
	const Vector3 centerV = (m_initGravityV * m_rot).Normal();

	Quaternion q1;
	q1.SetRotationArc(centerV, gravityV);
	const Quaternion q2 = Quaternion().lerp(Quaternion(), q1, 0.02f).Normal();

	const float gx2 = ANGLE2RAD(gyr.x * dt * 0.98f);
	const float gy2 = ANGLE2RAD(gyr.y * dt * 0.98f);

	const Vector3 raxis = (Vector3(1, 0, 0) * m_rot.Inverse()).Normal();
	const Vector3 paxis = (Vector3(0, 1, 0) * m_rot.Inverse()).Normal();
	Quaternion qr(raxis, gx2);
	Quaternion qp(paxis, gy2);
	Quaternion q3 = qr * qp;

	m_rot = q3 * m_rot * q2;

	// forward vector is always x-axis
	// remove yaw axis rotation
	const Vector3 euler1 = m_rot.Euler();
	Quaternion recovery(Vector3(0,0,1), euler1.z);
	m_rot *= recovery.Inverse();
	//~
	
	const Vector3 euler2 = m_rot.Euler();
	m_rpy.x = euler2.x;
	m_rpy.y = euler2.y;
	m_sensingGravityV = gravityV;

	return true;
}


// update yaw angle no magnetic sensor
bool cImu::CalcYawAngle(const float deltaSeconds)
{
	const float dt = deltaSeconds;
	const Vector3 gyr(m_6050.m_gyr.x, m_6050.m_gyr.y, m_6050.m_gyr.z);
	m_yaw += -gyr.z * dt; // opengl -> directx space
	m_rpy.z = ANGLE2RAD(m_yaw);
	return true;
}
