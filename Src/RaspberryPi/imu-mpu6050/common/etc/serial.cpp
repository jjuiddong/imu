
extern "C" 
{
   #include <termios.h>
   #include <sys/ioctl.h>
   #include <unistd.h>
   #include <fcntl.h>
}
#include <iostream>
#include <string>
#include "serial.h"

using namespace common;
using namespace std;


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


bool cSerial::Open(const string &deviceName, const int baud)
{
   speed_t myBaud;
   switch (baud) 
   {
      case      50:   myBaud =      B50 ; break ;
      case      75:   myBaud =      B75 ; break ;
      case     110:   myBaud =     B110 ; break ;
      case     134:   myBaud =     B134 ; break ;
      case     150:   myBaud =     B150 ; break ;
      case     200:   myBaud =     B200 ; break ;
      case     300:   myBaud =     B300 ; break ;
      case     600:   myBaud =     B600 ; break ;
      case    1200:   myBaud =    B1200 ; break ;
      case    1800:   myBaud =    B1800 ; break ;
      case    2400:   myBaud =    B2400 ; break ;
      case    4800:   myBaud =    B4800 ; break ;
      case    9600:   myBaud =    B9600 ; break ;
      case   19200:   myBaud =   B19200 ; break ;
      case   38400:   myBaud =   B38400 ; break ;
      case   57600:   myBaud =   B57600 ; break ;
      case  115200:   myBaud =  B115200 ; break ;
      case  230400:   myBaud =  B230400 ; break ;
      case  460800:   myBaud =  B460800 ; break ;
      case  500000:   myBaud =  B500000 ; break ;
      case  576000:   myBaud =  B576000 ; break ;
      case  921600:   myBaud =  B921600 ; break ;
      case 1000000:   myBaud = B1000000 ; break ;
      case 1152000:   myBaud = B1152000 ; break ;
      case 1500000:   myBaud = B1500000 ; break ;
      case 2000000:   myBaud = B2000000 ; break ;
      case 2500000:   myBaud = B2500000 ; break ;
      case 3000000:   myBaud = B3000000 ; break ;
      case 3500000:   myBaud = B3500000 ; break ;
      case 4000000:   myBaud = B4000000 ; break ;

      default:
      std::cerr << "Serial: " << "undefinde baud rate" << std::endl;
   }

   if ((m_handle = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1)
     std::cerr << "Serial: " << "can't open the port: " << deviceName << std::endl;

   fcntl (m_handle, F_SETFL, O_RDWR);

   // Get and modify current options:
   struct termios options;   
   tcgetattr (m_handle, &options);

   cfmakeraw(&options);
   cfsetispeed(&options, myBaud);
   cfsetospeed(&options, myBaud);

   options.c_cflag |= (CLOCAL | CREAD);
   options.c_cflag &= ~PARENB;
   options.c_cflag &= ~CSTOPB;
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8;
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
   options.c_oflag &= ~OPOST;

   options.c_cc[VMIN]  =   0 ;
   options.c_cc[VTIME] = 100 ;   // Ten seconds (100 deciseconds)

   tcsetattr(m_handle, TCSANOW, &options);

   int status;
   ioctl(m_handle, TIOCMGET, &status);

   status |= TIOCM_DTR;
   status |= TIOCM_RTS;

   ioctl(m_handle, TIOCMSET, &status);
   m_baud = baud;

   usleep(10000);   // 10mS
   return true;
}


bool cSerial::IsOpen() const
{
   return(m_handle >=0);
}


bool cSerial::Send(unsigned char *data, int len)
{
   if(!IsOpen()) return false;
   // cout << "send1" << endl;
   int rlen = write(m_handle, data, len); 
   // cout << "send2: " << rlen << endl;
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


// blocking read
int cSerial::Read(unsigned char *data
    , int len //=1
)
{
   if(!IsOpen()) return -1;
   int lenRCV = 0;
   while(lenRCV < len)
   {
      const int rlen = read(m_handle, &data[lenRCV], len - lenRCV);
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
