
#include "stdafx.h"
#include "global.h"
#include "icm20948.h"
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
	network2::cUdpClient udpClient;
	if (!udpClient.Init("192.168.0.19", 34001))
	{
		cout << "udp init error" << endl;
		return -1;
	}	

	cImu imu;
	if (!imu.Init("/dev/i2c-1"))
	 	return -1;

	sleep(2);

	network2::cPacketHeader binPacketHeader;

	// Run this forever
	while(true) 
	{
		// Record loop time stamp
		auto loopTimer = std::chrono::high_resolution_clock::now();

		imu.Update(dt);

		// send udp
		{
			using namespace network2::marshalling;
			network2::cPacket packet(&binPacketHeader);
			packet.SetProtocolId(0);
			packet.SetPacketId(1);
			packet.InitWrite();
			packet << imu.m_acc.x;
			packet << imu.m_acc.y;
			packet << imu.m_acc.z;
			packet << imu.m_gyr.x;
			packet << imu.m_gyr.y;
			packet << imu.m_gyr.z;
			packet << imu.m_mag.x;
			packet << imu.m_mag.y;
			packet << imu.m_mag.z;
			packet << imu.m_rpy.x; // roll
			packet << imu.m_rpy.y; // pitch
			packet << imu.m_rpy.z; // yaw
			packet.EndPack();
			udpClient.Send(network2::SERVER_NETID, packet);
		}		
	
		// cout << "acceleration values (x,y,z):" 
		// 	<< acc.x << ", " << acc.y << ", " << acc.z << endl;
		// cout << "Raw gyroscope values (x,y,z):" 
		// 	<< gyr.x << ", " << gyr.y << ", " << gyr.z << endl;
		// cout << "Magnetic values (x,y,z):" 
		// 	<< mag.x << ", " << mag.y << ", " << mag.z << endl;
		// cout << "*************************************" << endl;

		// Stabilize the data rate
		dt = timeSync(loopTimer);
	}
	return 1;
}
