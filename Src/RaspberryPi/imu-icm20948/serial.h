//
// 2024-08-01, jjuiddong
// - reference
//   - https://forums.raspberrypi.com/viewtopic.php?t=131208
//
#pragma once


class cSerial
{
public:
  cSerial(const std::string &deviceName="", int baud=-1);
  ~cSerial();

  bool Open(const std::string &deviceName, int baud);
  bool Send(unsigned char *data, int len);
  bool Send(unsigned char value);
  bool Send(const std::string &value);
  int Receive(unsigned char *data, int len);
  bool IsOpen();
  void Close();
  bool NumberByteRcv(int &bytelen);


public:
  int m_handle;
  std::string m_deviceName;
  int m_baud;
};

