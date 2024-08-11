
extern "C" 
{
   #include <asm/termbits.h>
   #include <sys/ioctl.h>
   #include <unistd.h>
   #include <fcntl.h>
}
#include <iostream>
using namespace std;

#include <string>
#include "serial.h"


cSerial::cSerial(
   const string &deviceName //=""
   , int baud //=-1
)
{
   m_handle = -1;
   if (!deviceName.empty())
      Open(deviceName, baud);
}

cSerial::~cSerial()
{
  if(m_handle >=0)
      Close();
}

bool cSerial::Open(const string &deviceName, int baud)
{
    struct termios tio;
    struct termios2 tio2;
    m_deviceName = deviceName;
    m_baud = baud;
    m_handle = open(m_deviceName.c_str(),O_RDWR | O_NOCTTY /* | O_NONBLOCK */);
    if(m_handle <0)
       return false;

    tio.c_cflag =  CS8 | CLOCAL | CREAD;
    tio.c_oflag = 0;
    tio.c_lflag = 0;       //ICANON;
    tio.c_cc[VMIN]=0;
    tio.c_cc[VTIME]=1;     // time out every .1 sec
    ioctl(m_handle,TCSETS,&tio);

    ioctl(m_handle,TCGETS2,&tio2);
    tio2.c_cflag &= ~CBAUD;
    tio2.c_cflag |= BOTHER;
    tio2.c_ispeed = baud;
    tio2.c_ospeed = baud;
    ioctl(m_handle,TCSETS2,&tio2);

    // flush buffer
    ioctl(m_handle,TCFLSH,TCIOFLUSH);

    return true;
}

bool cSerial::IsOpen()
{
   return(m_handle >=0);
}

bool cSerial::Send(unsigned char *data, int len)
{
   if(!IsOpen()) return false;
   int rlen = write(m_handle, data, len); 
   return(rlen == len);
}

bool cSerial::Send(unsigned char value)
{
   if(!IsOpen()) return false;
   int rlen = write(m_handle, &value, 1);
   return(rlen == 1);
}

bool cSerial::Send(const std::string &value)
{
   if(!IsOpen()) return false;
   unsigned int rlen = write(m_handle, value.c_str(), value.size()); 
   return(rlen == value.size());
}

int cSerial::Receive(unsigned char *data, int len)
{
   if(!IsOpen()) return -1;

   // this is a blocking receives
   int lenRCV = 0;
   while(lenRCV < len)
   {
      unsigned int rlen = read(m_handle, &data[lenRCV], len - lenRCV);
      lenRCV += rlen;
   }
   return lenRCV;
}

bool cSerial::NumberByteRcv(int &bytelen)
{
   if(!IsOpen()) return false;
   ioctl(m_handle, FIONREAD, &bytelen);
   return true;
}

void cSerial::Close()
{
   if(m_handle >=0)
      close(m_handle);
   m_handle = -1;
}
