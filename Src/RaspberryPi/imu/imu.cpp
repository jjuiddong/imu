
#include "global.h"
#include "mpuXX50.h"
#include "hmc5883l.h"
#include "imu.h"

using namespace std;
using std::string;


cImu::cImu()
	: m_roll(0.f)
	, m_pitch(0.f)
	, m_yaw(0.f)
{
	m_estG.v.x = 0.f;
	m_estG.v.y = 0.f;
	m_estG.v.z = 0.f;
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

	// Display calibration values to user
	cout << "---------------------------------------" << endl;
	cout << "Gyroscope bias values:" << endl;
	cout << "\t X: " << m_6050.m_gyroOffset[X] << endl;
	cout << "\t Y: " << m_6050.m_gyroOffset[Y] << endl;
	cout << "\t Z: " << m_6050.m_gyroOffset[Z] << endl;
	cout << "Gyroscope Resolution value: " << m_6050.m_gyroRes << endl;
	cout << "Accelerometer Resolution value: " << m_6050.m_accRes << endl;	
	cout << "---------------------------------------" << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(2));

	// initialize magnitude sensor
	cout << "HMC5883L initialize. Pass/Fail: ";
	const bool result2 = m_compass.Init(i2c);
	cout << result2 << endl;
	cout << "---------------------------------------" << endl;
	cout << "Magnitude gain:" << endl;
	cout << "\t X: " << m_compass.m_gain[X] << endl;
	cout << "\t Y: " << m_compass.m_gain[Y] << endl;
	cout << "\t Z: " << m_compass.m_gain[Z] << endl;
	cout << "---------------------------------------" << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(2));

	return true;
}


// process imu
// dt: delta second
bool cImu::Update(const float dt)
{
	CompFilter(dt, 0.98f);

	// cout << std::setprecision(3) << m_roll  << ", ";
	// cout << std::setprecision(3) << m_pitch << ", ";
	// cout << std::setprecision(3) << m_yaw;

	if (0)
	{
		cout << setfill(' ') << setw(5) << m_6050.m_acc[X] << ", ";
		cout << setfill(' ') << setw(5) << m_6050.m_acc[Y] << ", ";
		cout << setfill(' ') << setw(5) << m_6050.m_acc[Z] << ", ";
		cout << setfill(' ') << setw(5) << m_6050.m_gyro[X] << ", ";
		cout << setfill(' ') << setw(5) << m_6050.m_gyro[Y] << ", ";
		cout << setfill(' ') << setw(5) << m_6050.m_gyro[Z] << ", ";
		cout << setfill(' ') << setw(5) << m_compass.m_mag[X] << ", ";
		cout << setfill(' ') << setw(5) << m_compass.m_mag[Y] << ", ";
		cout << setfill(' ') << setw(5) << m_compass.m_mag[Z];
	}

	return true;
}


// Complementary filter
// dt: delta second
void cImu::CompFilter(float dt, float tau)
{
	m_6050.ReadCalData();
	m_6050.CompFilter(dt, tau);
	m_compass.ReadRawData();

	// GetEstimatedAttitude(dt);

	m_roll = m_6050.m_roll;
	m_pitch = m_6050.m_pitch;
	m_yaw = m_6050.m_yaw;
}


// dt: delta second
void cImu::GetEstimatedAttitude(float dt)
{
	int accMag = 0;
	float deltaGyroAngle[3];	
	const float scale = m_6050.m_gyroScale * dt;

	for (int axis = 0; axis < 3; ++axis)
	{
		deltaGyroAngle[axis] = m_6050.m_gyro[axis] * scale;
		m_accSmooth[axis] = m_6050.m_acc[axis];
		accMag += (int32_t)m_accSmooth[axis] * m_accSmooth[axis];
	}
	accMag = accMag * 100 / ((int)ACCEL_1G * ACCEL_1G);
	
	RotateV(&m_estG.v, deltaGyroAngle);

    // Apply complimentary filter (Gyro drift correction)
    // If accel magnitude >1.15G or <0.85G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
    // To do that, we just skip filter, as EstV already rotated by Gyro
    const int gyro_cmpf_factor = 600;
	#define INV_GYR_CMPF_FACTOR   (1.0f / ((float)gyro_cmpf_factor + 1.0f))

	cout << "accMag= " << accMag << endl;

    if (72 < (ushort)accMag && (ushort)accMag < 133) 
    {
        for (int axis = 0; axis < 3; axis++)
        	m_estG.a[axis] = 
        		(m_estG.a[axis] * (float)gyro_cmpf_factor + m_accSmooth[axis]) * INV_GYR_CMPF_FACTOR;
    }

	cout << "accSmooth= " << m_accSmooth[0] << ", " << m_accSmooth[1] << ", " << m_accSmooth[2] << endl;
    cout << "estG= " << m_estG.v.x << ", " << m_estG.v.y << ", " << m_estG.v.z << endl;

    m_anglerad[ROLL] = atan2f(m_estG.v.y, m_estG.v.z);
    m_anglerad[PITCH] = atan2f(-m_estG.v.x
    	, sqrtf(m_estG.v.y * m_estG.v.y + m_estG.v.z * m_estG.v.z));
    m_roll = lrintf(m_anglerad[ROLL] * (1800.0f / M_PI));
    m_pitch = lrintf(m_anglerad[PITCH] * (1800.0f / M_PI));

    cout << "anglerad= " << m_anglerad[ROLL] << ", " << m_anglerad[PITCH] << endl;

}


// Rotate Estimated vector(s) with small angle approximation, according to the gyro data
void cImu::RotateV(sVector3 *v, float *delta)
{
    sVector3 v_tmp = *v;

    // This does a  "proper" matrix rotation using gyro deltas without small-angle approximation
    float mat[3][3];
    float cosx, sinx, cosy, siny, cosz, sinz;
    float coszcosx, sinzcosx, coszsinx, sinzsinx;

    cosx = cosf(delta[ROLL]);
    sinx = sinf(delta[ROLL]);
    cosy = cosf(delta[PITCH]);
    siny = sinf(delta[PITCH]);
    cosz = cosf(delta[YAW]);
    sinz = sinf(delta[YAW]);

    coszcosx = cosz * cosx;
    sinzcosx = sinz * cosx;
    coszsinx = sinx * cosz;
    sinzsinx = sinx * sinz;

    mat[0][0] = cosz * cosy;
    mat[0][1] = -cosy * sinz;
    mat[0][2] = siny;
    mat[1][0] = sinzcosx + (coszsinx * siny);
    mat[1][1] = coszcosx - (sinzsinx * siny);
    mat[1][2] = -sinx * cosy;
    mat[2][0] = (sinzsinx) - (coszcosx * siny);
    mat[2][1] = (coszsinx) + (sinzcosx * siny);
    mat[2][2] = cosy * cosx;

    v->x = v_tmp.x * mat[0][0] + v_tmp.y * mat[1][0] + v_tmp.z * mat[2][0];
    v->y = v_tmp.x * mat[0][1] + v_tmp.y * mat[1][1] + v_tmp.z * mat[2][1];
    v->z = v_tmp.x * mat[0][2] + v_tmp.y * mat[1][2] + v_tmp.z * mat[2][2];
}
