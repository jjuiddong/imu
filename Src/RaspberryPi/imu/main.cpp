
#include "stdafx.h"
#include "global.h"
#include "mpuXX50.h"
#include "hmc5883l.h"
#include "serial.h"
#include "imu.h"

using namespace std;


int sampleRate = 4000; // Microseconds (~250 Hz)
int time2Delay;
float dt; // second unit


// Time stabilization function
float timeSync(auto t1)
{
	// Find duration to sleep thread
	auto t2 = std::chrono::high_resolution_clock::now();
	time2Delay = std::chrono::duration<float, std::micro>(t2-t1).count();

	// Sleep thread
	std::this_thread::sleep_for(std::chrono::microseconds(sampleRate-time2Delay));

	// Calculate dt
	auto t3 = std::chrono::high_resolution_clock::now();
	dt = (std::chrono::duration<float, std::micro>(t3-t1).count()) * 1E-6;
	//std::cout << "\t" << 1/dt << std::endl;

	// Return dt and begin main loop again
	return dt;
}


// Main function
int main()
{
	// cSerial serial;
	// if (!serial.Open("/dev/ttyAMA1", 115200))
	// {
	// 	cout << "Fail to Serial Open" << endl;
	// 	return -1;
	// }

	cImu imu;
	if (!imu.Init("/dev/i2c-1"))
		return -1;

	sleep(2);

	network2::cUdpClient udpClient;
	if (!udpClient.Init("192.168.0.19", 34001))
	{
		cout << "udp init error" << endl;
		return -1;
	}	

	network2::cPacketHeader binPacketHeader;

	// Run this forever
	while(true) 
	{
		// Record loop time stamp
		auto loopTimer = std::chrono::high_resolution_clock::now();

		// send roll/pitch/yaw		
		//unsigned char teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0, 0, '\r', '\n' };
		// int y = (int)(imu.m_yaw * 100.f);
		// int p = (int)(imu.m_pitch * 100.f);
		// int r = (int)(imu.m_roll * 100.f);
		// y += 18000;
		// p += 18000;
		// r += 18000;
		// int y = imu.m_compass.m_mag[X];
		// int p = imu.m_compass.m_mag[Y];
		// int r = imu.m_compass.m_mag[Z];
        // teapotPacket[2] = (y & 0xFF00) >> 8;
        // teapotPacket[3] = (y & 0x00FF);
        // teapotPacket[4] = (p & 0xFF00) >> 8;
        // teapotPacket[5] = (p & 0x00FF);
        // teapotPacket[6] = (r & 0xFF00) >> 8;
        // teapotPacket[7] = (r & 0x00FF);
		// serial.Send(teapotPacket, 14);
		//~ teapot

		// send udp
		{
			using namespace network2::marshalling;
			network2::cPacket packet(&binPacketHeader);
			packet.SetProtocolId(0);
			packet.SetPacketId(1);
			//int packet[14] = { 10000, 10001, 14*sizeof(int), 0, 0,0, 0,0, 0,0, 0, 0, '\r', '\n' };
			int ax, ay, az, gx, gy, gz, mx, my, mz;
			// cout << ax << ", " << ay << ", " << az << endl;
			ax = imu.m_6050.m_acc[X];
			ay = imu.m_6050.m_acc[Y];
			az = imu.m_6050.m_acc[Z];
			gx = imu.m_6050.m_gyro[X];			
			gy = imu.m_6050.m_gyro[Y];
			gz = imu.m_6050.m_gyro[Z];
			mx = imu.m_compass.m_mag[X];
			my = imu.m_compass.m_mag[Y];
			mz = imu.m_compass.m_mag[Z];
			packet.InitWrite();
			packet << ax;
			packet << ay;
			packet << az;
			packet << gx;
			packet << gy;
			packet << gz;
			packet << mx;
			packet << my;
			packet << mz;
			packet.EndPack();
			udpClient.Send(network2::SERVER_NETID, packet);
		}		
		//~

		imu.Update(dt);

		// Stabilize the data rate
		dt = timeSync(loopTimer);
	}
	return 1;
}
