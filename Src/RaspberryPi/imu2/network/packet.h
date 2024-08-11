//
// 2019-01-08, jjuiddong
// Packet class
//	- push, pop packet data 
//	- packet format descript by iPacketHeader
//		- binary, ascii, json, etc...
//
// 2020-12-04
//	- refactoring
//	- expand external memory
//	- bugfix: full packet last data crash
//
// 2020-12-31
//	- memory alignment 4byte
//	- websocket binary packet need 4byte alignment
//		- send / recv
//		- int, uint, int64, uint64, float, double type, offset is always *4 multiple
//		- need more type?
//
// 2021-09-08
//	- packetheader refactoring
//
#pragma once


namespace network2
{
	class cPacket
	{
	public:
		cPacket();
		cPacket(iPacketHeader *packetHeader);
		cPacket(iPacketHeader *packetHeader, const BYTE *src, const int byteSize);
		cPacket(BYTE *src);
		cPacket(const cPacket &rhs);
		virtual ~cPacket();

		// call before write
		void InitWrite();
		// call before read
		void InitRead(); 
		// init read/write
		void Initialize();
		// 4byte algnment
		void Alignment4();
		// call before send packet
		void EndPack();
		void ShallowCopy(const cPacket &packet);
		bool InferPacketHeader();

		template<class T> void Append(const T &rhs);
		template<class T> void AppendPtr(const T *rhs, const size_t size);
		template<class T> void GetData(OUT T &rhs);
		template<class T> void GetDataPtr(OUT T *rhs, size_t size);
		void AddDelimeter();
		void AppendDelimeter(const char c);
		void GetDataString(OUT string &str);
		void GetDataString(OUT char buffer[], const uint maxLength);
		int GetDataString(const char delimeter1, const char delimeter2, OUT string &str);
		int GetDataAscii(const char delimeter1, const char delimeter2, OUT char *buff, const int buffLen);

		void SetPacketHeader(iPacketHeader *header);
		void SetProtocolId(const int protocolId);
		void SetPacketId(const uint packetId);
		void SetPacketSize(const short packetSize);
		void SetPacketOption(const uint mask, const uint option);
		void SetSenderId(const netid netId);
		void SetReceiverId(const netid netId);
		int GetProtocolId() const;
		uint GetPacketId() const;
		uint GetPacketSize() const;
		uint GetPacketOption(const uint mask);
		int GetWriteSize() const;
		netid GetSenderId() const;
		netid GetReceiverId() const;
		int GetHeaderSize() const;
		bool IsValid();
		void Read4ByteAlign();
		void Write4ByteAlign();

		cPacket& operator=(const cPacket &rhs);


	protected:
		template<class T> void Append2(const T &rhs);
		template<class T> void AppendPtr2(const T *rhs, const size_t size);
		template<class T> void GetData2(OUT T &rhs);
		template<class T> void GetDataPtr2(OUT T *rhs, size_t size);


	public:
		iPacketHeader *m_header; // reference
		netid m_sndId; // sender id
		netid m_rcvId; // receiver id
		bool m_is4Align; // 4byte alignment?, websocket binary packet
		int m_readIdx;
		int m_writeIdx;
		char m_lastDelim; // GetDataString, GetDataAscii, last delimeter
		bool m_emptyData; // GetDataAscii
		bool m_isOverflow; // write overflow? (m_writeIdx > m_bufferSize)
		int m_bufferSize; // default: DEFAULT_PACKETSIZE
		BYTE m_buffer[DEFAULT_PACKETSIZE];
		BYTE *m_data; // reference pointer m_buffer (to expand external memory access)
	};


	//--------------------------------------------------------------------------------------
	// Implements

#define MARSHALLING_4BYTE_ALIGN(enable, offset) \
		if ((enable)) offset = ((offset + 3) & ~3);


	template<class T>
	inline void cPacket::Append2(const T &rhs)
	{
		if (m_writeIdx + (int)sizeof(T) > m_bufferSize)
		{
			m_isOverflow = true;
			return;
		}

		memmove(m_data + m_writeIdx, &rhs, sizeof(T));
		m_writeIdx += (int)sizeof(T);
	}

	template<class T>
	inline void cPacket::Append(const T &rhs) { Append2(rhs); }

	// Append specialization, int, uint, int64, float, double
	template<> inline void cPacket::Append<int>(const int &rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}
	template<> inline void cPacket::Append<uint>(const uint &rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}
	template<> inline void cPacket::Append<int64>(const int64 &rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}
	template<> inline void cPacket::Append<uint64>(const uint64 &rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}
	template<> inline void cPacket::Append<float>(const float&rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}
	template<> inline void cPacket::Append<double>(const double&rhs)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		Append2(rhs);
	}


	// size : copy byte size
	template<class T>
	inline void cPacket::AppendPtr2(const T *rhs, const size_t size)
	{
		if (m_writeIdx + (int)size > m_bufferSize)
		{
			m_isOverflow = true;
			return;
		}
		memmove(m_data + m_writeIdx, rhs, size);
		m_writeIdx += size;
	}

	template<class T>
	inline void cPacket::AppendPtr(const T *rhs, const size_t size)
	{
		AppendPtr2(rhs, size);
	}
	// AppendPtr specialization, int, uint, int64, uint64, float, double
	template<> inline void cPacket::AppendPtr<int>(const int *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}
	template<> inline void cPacket::AppendPtr<uint>(const uint *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}
	template<> inline void cPacket::AppendPtr<int64>(const int64 *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}
	template<> inline void cPacket::AppendPtr<uint64>(const uint64 *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}
	template<> inline void cPacket::AppendPtr<float>(const float *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}
	template<> inline void cPacket::AppendPtr<double>(const double *rhs, const size_t size)
	{
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_writeIdx);
		AppendPtr2(rhs, size);
	}


	// add delimeter
	inline void cPacket::AddDelimeter()
	{
		if (m_writeIdx + 1 > m_bufferSize)
		{
			m_isOverflow = true;
			return;
		}
		const int len = m_header->SetDelimeter(&m_data[m_writeIdx]);
		m_writeIdx += len;
	}

	// add delimeter
	// c: delimeter
	inline void cPacket::AppendDelimeter(const char c)
	{
		if (m_writeIdx + 1 > m_bufferSize)
		{
			m_isOverflow = true;
			return;
		}
		m_data[m_writeIdx++] = c;
	}

	// get data from m_readIdx index
	template<class T> 
	inline void cPacket::GetData2(OUT T &rhs)
	{
		if (m_readIdx + (int)sizeof(T) > m_writeIdx)
			return;
		memmove(&rhs, m_data + m_readIdx, sizeof(T));
		m_readIdx += (int)sizeof(T);
	}

	template<class T>
	inline void cPacket::GetData(OUT T &rhs) { GetData2(rhs); }
	// GetData specilization, int, uint, int64, uint64, float, double
	template<> inline void cPacket::GetData<int>(OUT int &rhs) { 
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs); 
	}
	template<> inline void cPacket::GetData<uint>(OUT uint &rhs) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs);
	}
	template<> inline void cPacket::GetData<int64>(OUT int64 &rhs) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs);
	}
	template<> inline void cPacket::GetData<uint64>(OUT uint64 &rhs) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs);
	}
	template<> inline void cPacket::GetData<float>(OUT float &rhs) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs);
	}
	template<> inline void cPacket::GetData<double>(OUT double &rhs) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetData2(rhs);
	}


	template<class T>
	inline void cPacket::GetDataPtr2(OUT T *rhs, size_t size)
	{
		if (m_readIdx + (int)size > m_writeIdx)
			return;
		memmove(rhs, m_data + m_readIdx, size);
		m_readIdx += size;
	}

	template<class T>
	inline void cPacket::GetDataPtr(OUT T *rhs, size_t size) {
		GetDataPtr2(rhs, size);
	}
	// GetData specialization, int, uint, int64, uint64, float, double
	template<> inline void cPacket::GetDataPtr<int>(OUT int *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}
	template<> inline void cPacket::GetDataPtr<uint>(OUT uint *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}
	template<> inline void cPacket::GetDataPtr<int64>(OUT int64 *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}
	template<> inline void cPacket::GetDataPtr<uint64>(OUT uint64 *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}
	template<> inline void cPacket::GetDataPtr<float>(OUT float *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}
	template<> inline void cPacket::GetDataPtr<double>(OUT double *rhs, size_t size) {
		MARSHALLING_4BYTE_ALIGN(m_is4Align, m_readIdx);
		GetDataPtr2(rhs, size);
	}


	// copy until meet null
	inline void cPacket::GetDataString(OUT string &str)
	{
		bool isLoop = true;
		char buff[128] = { '\0', };
		while (isLoop && (m_readIdx < m_writeIdx))
		{
			int i = 0;
			for (; i < ARRAYSIZE(buff)- 1 && (m_readIdx < m_writeIdx); ++i)
			{
				buff[i] = m_data[m_readIdx++];
				if ('\0' == m_data[m_readIdx - 1])
				{
					isLoop = false;
					break;
				}
			}
			buff[i] = '\0';
			str += buff;
		}
	}

	// copy until meet null
	inline void cPacket::GetDataString(OUT char buffer[], const uint maxLength)
	{
		for (int i = 0; i < (int)maxLength - 1 && (m_readIdx < m_writeIdx); ++i)
		{
			buffer[i] = m_data[m_readIdx++];
			if ('\0' == m_data[m_readIdx - 1])
				break;
		}
	}

	// read ascii data until meet delimeter
	// ignore delimeter if begin double quoto
	inline int cPacket::GetDataString(const char delimeter1, const char delimeter2, OUT string &str)
	{
		int len = 0;
		bool isLoop = true;
		bool isStart = true;
		bool isDoubleQuote = false;
		
		while (isLoop && (m_readIdx < m_writeIdx))
		{
			int i = 0;
			char c = '\0';
			char buff[128] = { '\0', };
			while ((m_readIdx < m_writeIdx) && (i < (ARRAYSIZE(buff) - 1)))
			{
				c = m_data[m_readIdx++];
				if (isStart && (c == '\"'))
				{
					isStart = false;
					isDoubleQuote = true;
					continue;
				}

				if (isDoubleQuote && ((c == '\"') || (c == '\0')))
				{
					isLoop = false;
					break;
				}
				if (!isDoubleQuote && ((c == delimeter1) || (c == delimeter2) || (c == '\0')))
				{
					isLoop = false;
					break;
				}

				buff[i++] = c;
				isStart = false;
				++len;
			}

			if (i < ARRAYSIZE(buff))
				buff[i] = '\0';
			str += buff;

			m_lastDelim = c;
			m_emptyData = (len <= 0);
		}

		return len;
	}


	// read ascii data until meet delimeter
	// delieter1,2: two delimeter
	inline int cPacket::GetDataAscii(const char delimeter1, const char delimeter2
		, OUT char *buff, const int buffLen)
	{
		int i = 0;
		char c = '\0';
		while ( (m_readIdx < m_writeIdx) && (i < (buffLen-1)))
		{
			c = m_data[m_readIdx++];
			if ((c == delimeter1) || (c == delimeter2) || (c == '\0'))
				break;
			buff[i++] = c;
		}
		if (i < buffLen)
			buff[i] = '\0';
		m_lastDelim = c;
		m_emptyData = (i <= 0);
		return i;
	}


	//--------------------------------------------------------------------------------------
	// Global Reserved Packet ID
	enum RESERVED_PACKETID
	{
		// basic protocol packet id
		PACKETID_CONNECT = 1,
		PACKETID_DISCONNECT,
		PACKETID_ACCEPT,
		PACKETID_ERROR_BIND,
		PACKETID_ERROR_CONNECT,
		
		// user protocol packet id
		PACKETID_USER = 10,
		PACKETID_UDPSVRMAP_CLOSE, // cUdpServerMap close packet
		PACKETID_UDPSVRMAP_ERROR, // cUdpServerMap error packet
	};

	class cNetworkNode;
	cPacket ConnectPacket(cNetworkNode *node, netid connectId);
	cPacket DisconnectPacket(cNetworkNode *node, netid disconnectId);
	cPacket AcceptPacket(cNetworkNode *node, SOCKET acceptSocket, const string &clientIP, int port);
	cPacket ErrorBindPacket(cNetworkNode *node);
	cPacket ErrorConnectPacket(cNetworkNode *node);
	//--------------------------------------------------------------------------------------
}
