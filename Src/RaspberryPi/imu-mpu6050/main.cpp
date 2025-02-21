
#include "stdafx.h"
#include "global.h"
#include "mpuXX50.h"
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

		// send udp
		{
			using namespace network2::marshalling;
			network2::cPacket packet(&binPacketHeader);
			packet.SetProtocolId(0);
			packet.SetPacketId(1);
			const Vector3 acc = imu.m_acc * 100.f; 
			const Vector3 gyr = imu.m_gyr * 0.01f; 
			int mx = 0;
			int my = 0;
			int mz = 0;
			packet.InitWrite();
			packet << *(int*)&acc.x;
			packet << *(int*)&acc.y;
			packet << *(int*)&acc.z;
			packet << *(int*)&gyr.x;
			packet << *(int*)&gyr.y;
			packet << *(int*)&gyr.z;
			packet << mx;
			packet << my;
			packet << mz;
			packet << *(int*)&imu.m_rpy.x;
			packet << *(int*)&imu.m_rpy.y;
			packet << *(int*)&imu.m_rpy.z;
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
