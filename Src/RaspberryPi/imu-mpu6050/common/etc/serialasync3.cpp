
#include "../common.h"
#include "serialasync3.h"

using namespace std;
using namespace common;


cSerialAsync3::cSerialAsync3()
	: m_state(eState::Disconnect)
	, m_sndQ(QUEUE_SIZE)
	, m_rcvQ(QUEUE_SIZE)
	, m_errCnt1(0)
{
}

cSerialAsync3::~cSerialAsync3()
{
	Close();
}


// initialize async serial
bool cSerialAsync3::Init(const string& device
	, const int baud //= -1
)
{
	Close();

	if (!m_serial.Open(device, baud))
		return false;

	m_state = eState::Connect;
	m_thread = std::thread(SerialThreadFunction, this);
	return true;
}


// read serial data from async buffer
// return read byte size
int cSerialAsync3::Read(uchar* buff, const uint size)
{
	RETV(m_rcvQ.empty(), 0);
	{
		AutoCSLock cs(m_rcs);

		// read data length
		uchar header[2] = {0, 0};
		int len = 0;
		if (m_rcvQ.pop(header, 2))
		{
			const ushort readLen = *(ushort*)header;
			if (size > (uint)readLen)
			{
				// read data from queue
				if (m_rcvQ.pop(buff, readLen))
					len = readLen;
			}
		}
		else
		{
			m_rcvQ.clear(); // excecption process
		}
		return len;
	}
	return 0;
}


// write serial data to async buffer
bool cSerialAsync3::Send(const uchar* buff, const uint size)
{
	RETV(0 == size, false);
	{
		AutoCSLock cs(m_wcs);
		uchar header[2] = { 0, 0 };
		*(ushort*)header = (ushort)size;
		m_sndQ.push(header, 2);
		m_sndQ.push(buff, size);
	}
	return true;
}


// is open seria?
bool cSerialAsync3::IsOpen() const
{
	return m_serial.IsOpen();
}


// parse packet (call from serial thread function)
// buff, size: parse buffer data and size
// data: read packet data
// dataSize: read packet size
// readSize: return total read size
// return: success?
bool cSerialAsync3::ParsePacket(uchar* buff, const int size
	, OUT uchar *data, OUT int* dataSize, OUT int *readSize)
{
	// packet formate
	//	- header: 0xAA, 1 byte
	//	- length : data length, 1 byte
	//	- data : packet data, length byte size - 1
	//	- parity bit : checksum xor bit, 1 byte

	// find header bit
	int startIdx = -1;
	for (int i = 0; i < size; ++i)
	{
		if (0xAA == buff[i])
		{
			startIdx = i;
			break;
		}
	}
	if (startIdx < 0)
	{
		// not found start bit
		*readSize = size;
		return false;
	}

	// check length
	if (startIdx == size - 1)
	{
		// if no more read? finish
		*readSize = startIdx; // remain start bit
		return false;
	}

	const int length = buff[startIdx + 1];
	if (startIdx + length > size)
	{
		// need more data
		*readSize = startIdx; // read before start bit
		return false;
	}

	const int s = startIdx + 2; // start index (2: header size)

	// check parity bit
	int parity = 0;
	for (int i = s; i < s + length - 1; ++i)
		parity = buff[i] ^ parity;
	if ((buff[s + length - 1] & 0xFF) != (parity & 0xFF))
	{
		++m_errCnt1;
		if (m_errCnt1 > 255)
			m_errCnt1 = 0;

		*readSize = s + length;
		return false;  // error return
	}
	//~

	// -1: except parity code
	int idx = 0;
	for (int i = s; i < (s + length - 1); ++i)
		data[idx++] = buff[i];

	*dataSize = length;
	*readSize = s + length;
	return true;
}


// make packet
// buff, size: source packet data
// data, dataSize: return packet with header, checksum
bool cSerialAsync3::MakePacket(uchar* buff, const int size
	, OUT uchar* data, OUT int* dataSize)
{
	data[0] = 0xAA;
	data[1] = (uchar)size + 1; // 1: parity bit
	for (int i = 0; i < size; ++i)
		data[i + 2] = buff[i];

	// make parity bit
	int parity = 0;
	for (int i = 0; i < size; ++i)
		parity = buff[i] ^ parity;
	data[size + 2] = (uchar)(parity & 0xFF);

	*dataSize = size + 3; // 3: header + parity bit
	return true;
}


// close serial
bool cSerialAsync3::Close()
{
	m_state = eState::Disconnect;
	if (m_thread.joinable())
		m_thread.join();
	m_serial.Close();
	m_sndQ.clear();
	m_rcvQ.clear();
	return true;
}


// serial thread function
int cSerialAsync3::SerialThreadFunction(cSerialAsync3* ser)
{
	if (ser->m_state != eState::Connect)
		return 0;

	uchar sndBuff[BUFFER_SIZE];
	uchar rcvBuff[BUFFER_SIZE];
	int readIdx = 0; // rcvBuff read index
	while (eState::Connect == ser->m_state)
	{
		// 1. Send Process
		if (!ser->m_sndQ.empty())
		{
			AutoCSLock cs(ser->m_wcs);

			uchar buff[2] = { 0, 0 };
			while (!ser->m_sndQ.empty())
			{
				// read data length
				int len = 0;
				if (ser->m_sndQ.pop(buff, 2))
				{
					const ushort readLen = *(ushort*)buff;
					if (sizeof(sndBuff) > (uint)readLen)
					{
						// read data from queue
						if (ser->m_sndQ.pop(sndBuff, readLen))
							len = readLen;
					}
					else
					{
						ser->m_sndQ.clear(); // excecption process
					}
				}
				else
				{
					ser->m_sndQ.clear(); // excecption process
				}

				if (len <= 0)
					break;

				int packetSize = 0;
				uchar outBuff[BUFFER_SIZE];
				ser->MakePacket(sndBuff, len, outBuff, &packetSize);
				if (0 == ser->m_serial.Send(outBuff, packetSize))
				{
					ser->m_state = eState::Disconnect;
					ser->m_serial.Close();
					cout << "terminate serial (1)" << endl;
					return 0; // fail
				}

				break; // finish, send only one
			}//~while
		}

		// 2. Receive Process
		int numByteRcv = 0;
		ser->m_serial.NumberByteRcv(numByteRcv);
		if (numByteRcv > 0)
		{
			const int minByteLen = min((int)(BUFFER_SIZE - readIdx), numByteRcv);
			const int readLen = ser->m_serial.Read(&rcvBuff[readIdx], minByteLen);
			if (readLen < 0)
			{
				ser->m_state = eState::Disconnect;
				ser->m_serial.Close();
				cout << "terminate serial (2)" << endl;
				return 0; // fail
			}

			readIdx += readLen;
			const int totalSize = readIdx; // total read byte size
			if (totalSize > 2) // (header size: 2 byte)
			{
				int parseLen = 0;
				int dataSize = 0;
				uchar outBuff[BUFFER_SIZE];
				if (ser->ParsePacket(rcvBuff, totalSize, outBuff, &dataSize, &parseLen))
				{
					AutoCSLock cs(ser->m_rcs);
					// 2 byte read len (ushort)
					// n byte read data
					const uint remainSize = ser->m_rcvQ.SIZE - ser->m_rcvQ.size();
					if (remainSize > (uint)dataSize)
					{
						uchar buff[2];
						*(ushort*)buff = (ushort)dataSize;
						ser->m_rcvQ.push(buff, 2); // store buffer length
						ser->m_rcvQ.push(outBuff, dataSize); // store buffer data
					}
					else
					{
						// error occurred!!, queue full
					}
				}

				// shift buffer (clear parse data)
				if ((parseLen > 0) && (totalSize > parseLen))
				{
					int idx = 0;
					for (int i = parseLen; i < totalSize; ++i)
						rcvBuff[idx++] = rcvBuff[i];
				}
				readIdx -= parseLen;
			}
		}//~if numByteRcv > 0

		std::this_thread::sleep_for(std::chrono::milliseconds(0)); // thread context swithing
	}//~while

	return 1;
}
