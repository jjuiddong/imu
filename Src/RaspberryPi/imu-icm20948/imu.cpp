
#include "stdafx.h"
#include "global.h"
#include "icm20948.h"
#include "imu.h"

using namespace std;
using std::string;


cImu::cImu()
	: m_isInitialize(false)
	, m_magCenter(1.9f, -23.f, -3.0f)
	, m_isMagCalib(false)
	, m_magCalibT(0.f)
	, m_magCalibCheckT(0.f)
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
	cout << "ICM-20948 initialize. Pass/Fail: ";
	const bool result1 = m_icm.Init(i2c);
	cout << result1 << endl;
	
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	m_icm.Calibration();

	m_isInitialize = false;
	m_rpy = Vector3(0,0,0);
	m_rot.SetIdentity();

	// Display calibration values to user
	cout << "---------------------------------------" << endl;
	cout << "Gyroscope bias values:" << endl;
	cout << "\t X: " << m_icm.m_gyrOffset.x << endl;
	cout << "\t Y: " << m_icm.m_gyrOffset.y << endl;
	cout << "\t Z: " << m_icm.m_gyrOffset.z << endl;
	cout << "---------------------------------------" << endl;

	cout << "---------------------------------------" << endl;
	cout << "accelerometer bias values:" << endl;
	cout << "\t X: " << m_icm.m_accOffset.x << endl;
	cout << "\t Y: " << m_icm.m_accOffset.y << endl;
	cout << "\t Z: " << m_icm.m_accOffset.z << endl;
	cout << "---------------------------------------" << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(2));

	return true;
}


// process imu
// dt: delta second
bool cImu::Update(const float dt)
{
	m_icm.ReadSensor();

	// change space, opengl -> directx
	const Vector3 acc(m_icm.m_data.acc[0], m_icm.m_data.acc[1], -m_icm.m_data.acc[2]);
	const Vector3 gyr(m_icm.m_data.gyr[0], m_icm.m_data.gyr[1], m_icm.m_data.gyr[2]);
	const Vector3 mag(m_icm.m_data.mag[0], m_icm.m_data.mag[1], -m_icm.m_data.mag[2]);

	const Vector3 gravityV = acc.Normal();
	const Vector3 magV = (mag - m_magCenter).Normal();

	// initialize calibration data
	if (!m_isInitialize)
	{
		m_isInitialize = true;
		m_initGravityV = gravityV;

		const Plane plane(gravityV, 0.f);
		const Vector3 forward = Vector3(1, 0, 0);
		const Vector3 left = Vector3(0, 1, 0);
		const Vector3 v0 = plane.Projection(magV).Normal();
		const Vector3 v1 = plane.Projection(forward).Normal();
		const Vector3 v2 = plane.Projection(left).Normal();
		float a1 = acos(v0.DotProduct(v1));
		float a2 = acos(v0.DotProduct(v2));
		if (forward.CrossProduct(magV).Normal().DotProduct(gravityV) < 0.f)
			a1 = -a1;
		if (left.CrossProduct(magV).Normal().DotProduct(gravityV) < 0.f)
			a2 = -a2;
		m_compassAngle1 = a1;
		m_compassAngle2 = a2;
	}

	m_gbuffer = (gravityV * (1.f / LPF_SIZE)) + (m_gbuffer * ((float)(LPF_SIZE-1) / LPF_SIZE));
	m_mbuffer = (magV * (1.f / LPF_SIZE)) + (m_mbuffer * ((float)(LPF_SIZE - 1) / LPF_SIZE));
	m_gbuffer.Normalize();
	m_mbuffer.Normalize();
	m_gravityV = m_gbuffer;
	m_magV = m_mbuffer;
	m_acc = acc; // for debugging
	m_gyr = gyr; // for debugging
	m_mag = mag; // for debugging

	CalcRollPitchAngle(dt);
	CalcYawAngle(dt);
	return true;
}


// update pitch angle
bool cImu::CalcRollPitchAngle(const float deltaSeconds)
{
	const float dt = deltaSeconds;

	// opengl -> directx
	const Vector3 acc(m_icm.m_data.acc[0], m_icm.m_data.acc[1], -m_icm.m_data.acc[2]);
	const Vector3 gyr(m_icm.m_data.gyr[0], m_icm.m_data.gyr[1], m_icm.m_data.gyr[2]);
	const Vector3 mag(m_icm.m_data.mag[0], m_icm.m_data.mag[1], -m_icm.m_data.mag[2]);

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


// Lerps from angle a to b (both between 0.f and PI_TIMES_TWO), taking the shortest path
// https://gist.github.com/ctmatthews/be41d72e9d4c72d2236de687f6f53974
#define PI 3.14159265359f
#define PI_TIMES_TWO 6.28318530718f
float LerpRadians(float a, float b, float lerpFactor)
{
	float result;
	float diff = b - a;
	if (diff < -PI)
	{
		// lerp upwards past PI_TIMES_TWO
		b += PI_TIMES_TWO;
		result = lerp(a, b, lerpFactor);
		if (result >= PI_TIMES_TWO)
		{
			result -= PI_TIMES_TWO;
		}
	}
	else if (diff > PI)
	{
		// lerp downwards past 0
		b -= PI_TIMES_TWO;
		result = lerp(a, b, lerpFactor);
		if (result < 0.f)
		{
			result += PI_TIMES_TWO;
		}
	}
	else
	{
		// straight lerp
		result = lerp(a, b, lerpFactor);
	}

	return result;
}


// update yaw angle
bool cImu::CalcYawAngle(const float deltaSeconds)
{
	const float dt = deltaSeconds;

	const Vector3 gravityV = m_gravityV;
	const Vector3 magV = m_magV;
	const Quaternion irot = m_rot.Inverse();

	const Vector3 gv = gravityV;
	const Vector3 mv = magV;

	const Plane plane(gv, 0.f);
	const Vector3 forward(1, 0, 0);
	const Vector3 left(0, 1, 0);
	const Vector3 v0 = plane.Projection(mv).Normal();
	const Vector3 v1 = (forward * irot).Normal();
	const Vector3 v2 = (left * irot).Normal();

	const float d1 = gv.DotProduct(v1);
	if (abs(d1) > 0.5f)
	{
		// compare right vector
		m_compareType = 1;

		const float d = v2.DotProduct(v0);
		float a = acos(d);
		if (v2.CrossProduct(v0).Normal().DotProduct(gv) < 0.f)
			a = -a;

		const float target = m_compassAngle2 - a;
		m_rpy.z = LerpRadians(m_rpy.z, target, 0.05f);
	}
	else
	{
		m_compareType = 0;

		const float d = v1.DotProduct(v0);
		float a = acos(d);
		if (v1.CrossProduct(v0).Normal().DotProduct(gv) < 0.f)
			a = -a;

		const float target = m_compassAngle1 - a;
		m_rpy.z = LerpRadians(m_rpy.z, target, 0.05f);
	}

	m_sensingCompassV = mv;
	m_sensingGravityV = gv;
	m_projCompassV = v0;

	return true;
}
