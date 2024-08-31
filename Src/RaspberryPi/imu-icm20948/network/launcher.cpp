
#include "../stdafx.h"
#include "launcher.h"

using namespace network2;
using namespace common;


// Start TCP Client
bool network2::LaunchTCPClient(const string ip, const int port
	, OUT SOCKET &out
	, const bool isLog // = true
	, const int clientSidePort //= -1
)
{
	SOCKET ssock = socket(AF_INET, SOCK_STREAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		if (isLog)
			dbg::Logc(2, "socket() error\n");
		return false;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	server_addr.sin_port = htons(port);
	int clen = sizeof(server_addr);

	int nRet = connect(ssock, (struct sockaddr*)&server_addr, clen);
	if (nRet == SOCKET_ERROR)
	{
		//clog::Error( clog::ERROR_CRITICAL, "connect() error ip=%s, port=%d\n", ip.c_str(), port );
		close(ssock);
		return false;
	}	

	out = ssock;
	
	return true;
}


// Start TCP Server
bool network2::LaunchTCPServer(const int port, OUT SOCKET &out, const bool isLog)
{
	int ssock;
	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		if (isLog)
			dbg::Logc(2, "socket() error\n");
		return false;
	}

	int option = 1;
	setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(ssock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		if (isLog)
			dbg::Logc(2, "bind() error port: %d\n", port);
		close(ssock);	
		return false;
	}

	if (listen(ssock, 8) < 0)
	{
		if (isLog)
			dbg::Logc(2, "listen() error\n");
		close(ssock);
		return false;
	}

	char szBuf[256] = {'\0', };
	int nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		if (isLog)
			dbg::Logc(2, "gethostname() error\n");
		close(ssock);
		return false;
	}

	out = ssock;
	return true;
}


// Start UDP Server
bool network2::LaunchUDPServer(const int port, OUT SOCKET &out, const bool isLog)
{

	SOCKET ssock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		if (isLog)
			dbg::Logc(2, "socket() error\n");
		return false;
	}

	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);
	bind(ssock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	char szBuf[256]= {'\0',};
	int nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		if (isLog)
			dbg::Logc(2, "gethostname() error\n");
		close(ssock);
		return false;
	}

	out = ssock;	

	return true;
}


// Start UDP Client
bool network2::LaunchUDPClient(const string ip, const int port
	, OUT struct sockaddr_in &sockAddr, OUT SOCKET &out, const bool isLog)
{
	struct hostent *lpHostEntry;
	lpHostEntry = gethostbyname(ip.c_str());
	if (NULL == lpHostEntry)
	{
		if (isLog)
			dbg::Logc(2, "gethostbyname() error\n");
		return false;
	}

	SOCKET ssock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		if (isLog)
			dbg::Logc(2, "error winsock version\n");
		return false;
	}

	bzero(&sockAddr, sizeof(sockaddr_in));
	sockAddr.sin_family = AF_INET;
	//sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	sockAddr.sin_addr = *((struct in_addr*)*lpHostEntry->h_addr_list);	
	sockAddr.sin_port = htons(port);
	int clen = sizeof(sockaddr_in);

	int nRet = connect(ssock, (struct sockaddr*)&sockAddr, clen);
	if (nRet == SOCKET_ERROR)
	{
		if (isLog)
			dbg::Logc(2, "connect() error ip=%s, port=%d\n", ip.c_str(), port);
		close(ssock);
		return false;
	}

	out = ssock;

	return true;
}
