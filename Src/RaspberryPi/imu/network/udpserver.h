//
// 2019-01-11, jjuiddong
// udp server
//
#pragma once


namespace network2
{

	class cUdpServer : public cNetworkNode
	{
	public:
		explicit cUdpServer(
			const StrId &name = "UdpServer"
			, const int logId = -1
		);
		virtual ~cUdpServer();

		bool Init(const int bindPort
			, const int packetSize = DEFAULT_PACKETSIZE
			, const int maxPacketCount = DEFAULT_PACKETCOUNT
			, const int sleepMillis = DEFAULT_SLEEPMILLIS
			, const bool isThreadMode = true
		);
		bool Process();
		bool ErrorSession(const netid netId);
		virtual void Close() override;

		// Override
		virtual SOCKET GetSocket(const netid netId) override;
		virtual netid GetNetIdFromSocket(const SOCKET sock) override;
		virtual void GetAllSocket(OUT map<netid, SOCKET> &out) override;
		virtual int Send(const netid rcvId, const cPacket &packet) override;
		virtual int SendImmediate(const netid rcvId, const cPacket &packet) override;
		virtual int SendAll(const cPacket &packet, set<netid> *outErrs = nullptr) override;


	protected:
		static unsigned ThreadFunction(cUdpServer *server);


	public:
		int m_maxBuffLen; // recv buffer size
		cPacketQueue m_recvQueue;

		std::thread m_thread;
		int m_sleepMillis;
		char *m_recvBuffer;
	};

}
