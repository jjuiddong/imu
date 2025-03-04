//
// 2024-08-01, jjuiddong
// - reference
//   - https://forums.raspberrypi.com/viewtopic.php?t=131208
//   - https://github.com/wooseokyourself/rpi-serial-api
//
#pragma once


namespace common
{
	class cSerial
	{
	public:
		cSerial(const std::string& deviceName = "", const int baud = -1);
		~cSerial();

		bool Open(const std::string& deviceName, int baud);
		bool Send(unsigned char* data, int len);
		bool Send(unsigned char value);
		bool Send(const std::string& value);
		int Read(unsigned char* data, int len = 1);
		bool IsOpen() const;
		void Close();
		bool NumberByteRcv(int& bytelen);


	public:
		int m_handle;
		std::string m_deviceName;
		int m_baud;
	};
}
