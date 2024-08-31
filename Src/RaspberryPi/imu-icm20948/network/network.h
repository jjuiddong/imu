//
// 2022-06-01, jjuiddong
// network definition
//
#pragma once

#undef FD_SETSIZE
#ifndef FD_SETSIZE
	// low fd_setsize, low Performance
	#define FD_SETSIZE      1024
#endif /* FD_SETSIZE */


#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <algorithm>


#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)


typedef unsigned int u_int;
typedef int SOCKET;
typedef unsigned char BYTE;
typedef unsigned int DWORD;


namespace network2 {
	enum {
		// BUFFER_LENGTH = 512,
	};

	struct fd_array : fd_set
	{
		SOCKET fd_array[FD_SETSIZE];
		int fd_count;
	};
}


using namespace common;

typedef int netid;


#include "definition.h"
#include "session.h"
#include "websession.h"
#include "packetheader.h"
#include "packetheaderascii.h"
#include "packetheaderjson.h"
#include "packet.h"
#include "factory.h"
#include "protocol.h"
#include "protocol_handler.h"
#include "protocol_dispatcher.h"
#include "protocol_macro.h"
#include "networknode.h"
#include "socketbuffer.h"
#include "packetqueue.h"
#include "launcher.h"
#include "tcpserver.h"
#include "tcpclient.h"
#include "udpserver.h"
#include "udpclient.h"
#include "webserver.h"
#include "webclient.h"
#include "netcontroller.h"

#include "marshalling.h"
#include "marshalling_bin.h"
#include "marshalling_ascii.h"
#include "marshalling_json.h"
#include "protocol_basic_dispatcher.h"
#include "protocol_all_handler.h"
#include "definition_marshalling.h"

#include "prtcompiler/ProtocolDefine.h"

#include "utility/log.h"
#include "utility/packetlog.h"
#include "utility/utility.h"
