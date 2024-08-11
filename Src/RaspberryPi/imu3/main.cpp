//
// 2024-08-07, jjuiddong
// https://github.com/libdriver/mpu6050/tree/master
// 실패. driver_mpu6050_dmp 프로그램은 오류가 나서 실행되지 않는다.
//
#include "stdafx.h"
// #include "serial.h"
#include "driver_mpu6050_fifo.h"
#include "gpio.h"

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



uint32_t i;
uint32_t times;
uint16_t len;
uint8_t (*g_gpio_irq)(void) = NULL;
static int16_t gs_accel_raw[128][3];
static float gs_accel_g[128][3];
static int16_t gs_gyro_raw[128][3];
static float gs_gyro_dps[128][3];
mpu6050_address_t addr;

// Main function
int main()
{
	/* gpio init */
	if (gpio_interrupt_init() != 0)
	{
	    return 1;
	}
	g_gpio_irq = mpu6050_fifo_irq_handler;

	/* init */
	addr = MPU6050_ADDRESS_AD0_LOW;
	if (mpu6050_fifo_init(addr) != 0)
	{
	    g_gpio_irq = NULL;
	    (void)gpio_interrupt_deinit();

	    cout << "error aa\n";
	    return 1;
	}

	/* delay 100 ms */
	mpu6050_interface_delay_ms(100);


	// cSerial serial;
	// if (!serial.Open("/dev/ttyAMA1", 115200))
	// {
	// 	cout << "Fail to Serial Open" << endl;
	// 	return -1;
	// }

	// cImu imu;
	// if (!imu.Init("/dev/i2c-1"))
	// 	return -1;

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


		len = 128;
	    /* read */
	    if (mpu6050_fifo_read(gs_accel_raw, gs_accel_g,
	                          gs_gyro_raw, gs_gyro_dps, &len) != 0)
	    {
	        (void)mpu6050_fifo_deinit();
	        g_gpio_irq = NULL;
	        (void)gpio_interrupt_deinit();
	        cout << "read error\n";

	        return 1;
	    }

	    mpu6050_interface_debug_print("mpu6050: %d/%d.\n", i + 1, times);
	    mpu6050_interface_debug_print("mpu6050: fifo %d.\n", len);
	    mpu6050_interface_debug_print("mpu6050: acc x[0] is %0.2fg.\n", gs_accel_g[0][0]);
	    mpu6050_interface_debug_print("mpu6050: acc y[0] is %0.2fg.\n", gs_accel_g[0][1]);
	    mpu6050_interface_debug_print("mpu6050: acc z[0] is %0.2fg.\n", gs_accel_g[0][2]);
	    mpu6050_interface_debug_print("mpu6050: gyro x[0] is %0.2fdps.\n", gs_gyro_dps[0][0]);
	    mpu6050_interface_debug_print("mpu6050: gyro y[0] is %0.2fdps.\n", gs_gyro_dps[0][1]);
	    mpu6050_interface_debug_print("mpu6050: gyro z[0] is %0.2fdps.\n", gs_gyro_dps[0][2]);
	     mpu6050_interface_delay_ms(100);


	    // /* read */
	    // if (mpu6050_basic_read(g, dps) != 0)
	    // {
	    //     (void)mpu6050_basic_deinit();
	    //     return 1;
	    // }
	        
	    // if (mpu6050_basic_read_temperature(&degrees) != 0)
	    // {
	    //     (void)mpu6050_basic_deinit();
	    //     return 1;
	    // }

	        


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
			// using namespace network2::marshalling;
			// network2::cPacket packet(&binPacketHeader);
			// packet.SetProtocolId(0);
			// packet.SetPacketId(1);
			// //int packet[14] = { 10000, 10001, 14*sizeof(int), 0, 0,0, 0,0, 0,0, 0, 0, '\r', '\n' };
			// int ax, ay, az, gx, gy, gz, mx, my, mz;
			// // cout << ax << ", " << ay << ", " << az << endl;
			// ax = imu.m_6050.m_acc[X];
			// ay = imu.m_6050.m_acc[Y];
			// az = imu.m_6050.m_acc[Z];
			// gx = imu.m_6050.m_gyro[X];			
			// gy = imu.m_6050.m_gyro[Y];
			// gz = imu.m_6050.m_gyro[Z];
			// mx = imu.m_compass.m_mag[X];
			// my = imu.m_compass.m_mag[Y];
			// mz = imu.m_compass.m_mag[Z];
			// packet.InitWrite();
			// packet << ax;
			// packet << ay;
			// packet << az;
			// packet << gx;
			// packet << gy;
			// packet << gz;
			// packet << mx;
			// packet << my;
			// packet << mz;
			// packet.EndPack();
			// udpClient.Send(network2::SERVER_NETID, packet);
		}		
		//~

		// imu.Update(dt);

		// Stabilize the data rate
		dt = timeSync(loopTimer);
	}

	/* deinit */
	(void)mpu6050_fifo_deinit();
	g_gpio_irq = NULL;
	(void)gpio_interrupt_deinit();

	return 1;
}
