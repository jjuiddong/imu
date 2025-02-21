//
// 2021-09-19, jjuiddong
// WebSocket Server with poco library
//	- poco library websocket server
//		- https://pocoproject.org/
//
#pragma once

#include "Poco/Net/Socket.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"

// Poco library class forward declaration
namespace Poco {
	namespace Net {
		class ServerSocket;
		class HTTPServer;
	}
}


namespace network2
{
	class cWebServer : public network2::cNetworkNode
	{
	public:
		cWebServer(
			cWebSessionFactory *sessionFactory = new cWebSessionFactory()
			, const StrId &name = "WebServer"
			, const int logId = -1
		);
		virtual ~cWebServer();

		bool Init(const int bindPort
			, const int packetSize = DEFAULT_PACKETSIZE
			, const int maxPacketCount = DEFAULT_PACKETCOUNT
			, const int sleepMillis = DEFAULT_SLEEPMILLIS
			, const bool isThreadMode = true
			, const bool isSpawnHttpSvr = true
		);
		bool Process();
		bool AddSession(const SOCKET sock, const Str16 &ip, const int port);
		bool RemoveSession(const netid netId);
		cWebSession* MoveSession(const netid netId);
		bool InsertSession(cWebSession *session);
		void SetLogId(const int logId);
		bool IsExist(const netid netId);
		cWebSession* FindSessionBySocket(const SOCKET sock);
		cWebSession* FindSessionByNetId(const netid netId);
		cWebSession* FindSessionByName(const StrId &name);
		void SetSessionListener(iSessionListener *listener);
		virtual void Close() override;

		// cNetworkNode interface 
		virtual SOCKET GetSocket(const netid netId) override;
		virtual netid GetNetIdFromSocket(const SOCKET sock) override;
		virtual void GetAllSocket(OUT map<netid, SOCKET> &out) override;
		virtual int Send(const netid rcvId, const cPacket &packet) override;
		virtual int SendImmediate(const netid rcvId, const cPacket &packet) override;
		virtual int SendAll(const cPacket &packet, set<netid> *outErrs = nullptr) override;


	protected:
		static unsigned ThreadFunction(cWebServer *server);
		bool ReceiveProcces();


	public:
		Poco::Net::ServerSocket *m_websocket;
		Poco::Net::HTTPServer *m_httpServer;

		bool m_isThreadMode; // thread mode?
		int m_maxBuffLen; // recv buffer size
		common::VectorMap<netid, cWebSession*> m_sessions;
		common::VectorMap<SOCKET, cWebSession*> m_sessions2; // reference
		fd_array m_sockets; // all sockets
		fd_array m_readSockets; // thread sync all sockets
		std::atomic<bool> m_isUpdateSocket; // sync m_sockets
		int m_maxFd;
		vector<cWebSession*> m_tempSessions; // temporal session
		cPacketQueue m_sendQueue;
		cPacketQueue m_recvQueue;
		cWebSessionFactory *m_sessionFactory;
		iSessionListener *m_sessionListener;

		std::thread m_thread;
		CriticalSection m_cs;
		int m_sleepMillis;
		char *m_recvBuffer;
		char *m_sendBuffer; // poco library sendFrame() has memory alloc
							// to avoid memory alloc, modified sendFrame() to sendFrame2()
							// use external memory this m_sendBuffer
		common::cTimer m_timer;
	};

}
