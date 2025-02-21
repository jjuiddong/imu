//
// 2025-02-17, jjuiddong
// async serial class
//	- dynamic length serial communication
//
// serial communication protocol
// packet formate
//	- header: 0xAA, 1 byte
//	- length : data length, 1 byte
//	- data : packet data, length byte size - 1
//	- parity bit : checksum xor bit, 1 byte
//
#pragma once


namespace common
{
	class cSerialAsync3
	{
	public:
		cSerialAsync3();
		virtual ~cSerialAsync3();

		bool Init(const string& device, const int baud = -1);
		int Read(uchar* buff, const uint size);
		bool Send(const uchar* buff, const uint size);
		bool IsOpen() const;
		bool Close();


	protected:
		bool ParsePacket(uchar* buff, const int size
			, OUT uchar *data, OUT int *dataSize, OUT int *readSize);
		bool MakePacket(uchar* buff, const int size
			, OUT uchar* data, OUT int* dataSize);

		static int SerialThreadFunction(cSerialAsync3* ser);


	public:
		enum { 
			BUFFER_SIZE = 512, // max packet size
			QUEUE_SIZE = 1024,// send/recv queue size
		};
		enum class eState { Disconnect, TryConnect, Connect };

		eState m_state;
		cSerial m_serial; // serial communication object
		cCircularQueue2<uchar> m_sndQ; // send queue
		cCircularQueue2<uchar> m_rcvQ; // receive queue
		int m_errCnt1; // parity bit error count for debug
		std::thread m_thread;
		CriticalSection m_wcs; // write cs
		CriticalSection m_rcs; // read cs
	};
}
