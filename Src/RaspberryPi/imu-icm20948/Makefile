CC = g++
CFLAGS = -g -std=c++14 -Wall -pedantic -pthread -Wno-unused-variable -Wno-unused-function -Wno-sign-compare -Wno-conversion-null -Wno-reorder -Wno-delete-non-virtual-dtor -Wno-switch -Wno-pointer-arith -Wno-pedantic -Wno-format -fpermissive -fconcepts-ts
LDFLAGS	= -L/usr/local/lib/arm-linux-gnueabihf -L/usr/local/lib
LDLIBS = -lwiringPi -l:libboost_system.a -lPocoNet -lPocoUtil -lPocoFoundation
TARGET = main
OBJECTS = \
./common/etc/filepath1.o\
./common/thread/thread1.o \
./common/etc/dbg.o \
./common/etc/date.o \
./common/thread/wqsemaphore.o \
./common/thread/tpsemaphore.o \
./network/session.o \
./common/math/line.o \
./common/math/matrix44.o \
./common/math/plane.o \
./common/math/plane3.o \
./common/math/quaternion.o \
./common/math/ray.o \
./common/math/transform.o \
./common/math/triangle.o \
./common/math/vector2.o \
./common/math/vector3.o \
./common/math/vector4.o \
./common/math/vectortype.o \
./network/packetheader.o \
./network/packetheaderascii.o \
./network/packet.o \
./common/etc/recttype.o \
./network/protocol_dispatcher.o \
./network/networknode.o \
./common/etc/genid.o \
./network/utility/log.o \
./network/utility/packetlog.o \
./common/etc/date2.o \
./network/socketbuffer.o \
./network/utility/utility.o \
./network/prtcompiler/ProtocolDefine.o \
./network/packetqueue.o \
./network/launcher.o \
./network/tcpserver.o \
./common/etc/timer.o \
./common/etc/stringfunc.o \
./network/tcpclient.o \
./network/udpserver.o \
./network/udpclient.o \
./network/netcontroller.o \
./network/protocol_basic_dispatcher.o \
./network/protocol_all_handler.o \
./network/websession.o \
./network/webserver.o \
./network/webclient.o \
main.o \
icm20948.o \
serial.o \
imu.o \

all : $(TARGET)

.cpp.o :
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@

$(TARGET) : $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS) 

clean :
	rm *.o

cleanall :
	rm -rf *.o $(TARGET)
