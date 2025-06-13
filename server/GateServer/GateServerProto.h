#ifndef	_RUNGATE_PROTO_
#define	_RUNGATE_PROTO_

#ifndef	STDCALL
#define	STDCALL __stdcall
#endif

#ifndef	SCSTR
#define	SCSTR static const CHAR*
#endif


//网关核心版本
#define	RUNGATE_KRN_VERSION				MAKEFOURCC( 15, 5, 18, 1)
//用户接收包缓冲区默认大小
#define	SESSION_RECV_BUFSIZE			4096

//用户发送包缓冲区默认大小
#define	SESSION_SEND_BUFSIZE			8192

//每用户接收处理队列最长数据大小
#define	SESSION_MAX_RECVPROC_SIZE		8192

//用户数据队列处理内存增长单位
#define	SESSION_DATAGROW_SIZE			1024
//最大用户数据包序号错误数量
#define	SESSION_PACKETERR_MAX			32
//处理服务器数据最大时间
#define	RUNGATE_PROCESS_SERVERMSG_LIMIT	32	//32毫秒
//每次向用户发送数据包限制
#define	RUNGATE_ONCESENDUSE_LIMIT		2048
//发送检查超时
#define	RUNGATE_SENDCHECK_TIMEOUT		300


typedef GATEMSGHDR RUNGATEMSGHDR, *PRUNGATEMSGHDR;


//session的通信状态,具体见开发文档的定义
typedef enum tagSessionStatus
{
	enSessionStatusIdle =0, //空闲状态  
	//enEncryReq =1,      	//请求服务器的密钥状态
	//enEncryAck,     		//密钥确认阶段
	enConn,					//连接上
	enCommunication, 		//正常的通信阶段
} SESSIONSTATUS;

typedef enum tagSessionCloseReason
{
	enReasonAllSessionClose =1000, //所有的session都要关闭
	enReasonIdle =1001,       //长时间不活跃
	enReasonSenderBuffLarge =1002, //发送缓存太长
	enReasonSocketError =1003, //socket错误
	enReasonSendSockerError=1004, //发送的时候socket错误
	enReasonLargePackLeft =1005, //剩余过量的包
	 
	enReasonRecvLargerPack=1006, //所有的session都要关闭
	enReasonGM =1007, //GMClose
	enReasonKeyError =1011, //密钥错误
	enSycDataLengthError= 1012, //salt data length error
} SESSIONCLOSEREASON;


typedef	struct tagRunGateDataBuffer
{
	char	*lpBuffer;
	int		nSize;
	int		nOffset;
}RUNGATEDATABUF, *PRUNGATEDATBUF;

typedef struct sockaddr_in	SOCKADDRIN, *PSOCKADDRIN;

typedef enum tagRunSockASyncAction
{
	aaAccept = 1,		//AcceptEx
	aaRecv,				//WSARecv
	aaSend,				//WSASend
}RUNSOCKASYNCACTION;

#ifdef WIN32
//会话异步接受数据
typedef	struct tagRunSockOverlapped
{
	OVERLAPPED	Overlapped;
	WSABUF		WSABuf;
	BYTE		btAction;					//行为类型
	BYTE		btReceved[3];
	struct tagRunGateUserSession	*pUser;	//会话指针
}RUNSOCKOVERLAPPED, *PRUNSOCKOVERLAPPED;
#endif


#define OrderSwap16(A) (((uint16_t)(A) >> 8) | ((uint16_t)(A & 0xFF) << 8))
#define OrderSwap32(A) (OrderSwap16((uint32_t)(A) >> 16) | ((uint32_t)(OrderSwap16(A)) << 16))
#define OrderSwap64(A) (OrderSwap32((uint64_t)(A) >> 32) | ((uint64_t)(OrderSwap32(A)) << 32));

enum class emOpcode : uint8_t { //操作码定义类型
	MID = 0x0,		//标识一个中间数据包
	TXT = 0x1,		//标识一个text类型数据包
	BIN = 0x2,		//标识一个binary类型数据包
					//0x3 - 7：保留
	CLR = 0x8,		//标识一个断开连接类型数据包
	PIN = 0x9,		//标识一个ping类型数据包
	PON = 0xA,		//表示一个pong类型数据包
};

#pragma pack(push,1)
typedef	struct tagWebSocketHead {
	uint8_t fin : 1;//标识是否为此消息的最后一个数据包
	uint8_t rsv1 : 1;//保留位1
	uint8_t rsv2 : 1;//保留位2
	uint8_t rsv3 : 1;//保留位3
	emOpcode opcode : 4;//操作码

	uint8_t mask : 1; //是否需要掩码
	uint8_t len : 7;//长度
	union {
		uint16_t v16;//长度为126时
		uint64_t v64;//长度为127时
	} ex_len;
	uint8_t mkey[4];
	uint8_t rh : 1;//head读取完成
	uint8_t rl : 1;//len读取完成
	uint8_t rk : 1;//mkey读取完成
	uint8_t rs : 5;//扩展保留
	tagWebSocketHead(void) { reset(); }
	void reset(void) { memset(this,0,sizeof(tagWebSocketHead)); }
	inline uint64_t GetLen(void) {
		if (len == 126) {
			return ex_len.v16;
		} else if (len == 127) {
			return ex_len.v64;
		}
		return len;
	}
	inline uint8_t GetLenNeedByte(void) {
		if (len == 126) {
			return 2;
		} else if (len == 127) {
			return 8;
		} 
		return 0;
	}
} WEBSOCKETHEAD, *PWEBSOCKETHEAD;
#pragma pack(pop)

//用户会话数据
typedef	struct tagRunGateUserSession
{
	SOCKET				nSocket;
	int					nIndex;
	int					nServerIdx;
	SOCKADDRIN			SockAddr;
	int					nRecvPacketCount;
	int					nSendPacketCount;
	WORD				wPacketError;
	BYTE				btPacketIdx;
	bool				boSendAvaliable;
	bool				boMarkToClose;  //标记为关闭状态
	bool				boRemoteClosed;   //远程方是否关闭,如超时或者主动关闭了客户端
	BYTE				btRcv2[2];
	TICKCOUNT			dwConnectTick;
	TICKCOUNT			dwCloseTick;
	TICKCOUNT			dwSendTimeOut;
	TICKCOUNT			dwClientMsgTick;
	TICKCOUNT			dwServerMsgTick;
	unsigned long long 	nVerifyIdx;	 //通信的时候消息验证码

	WEBSOCKETHEAD		webSocketHead;			// WebSocket封装
	bool				webSocketShakeHand;		// WebSocket握手标志
	unsigned short 		nKey;					// 服务端KEY

	RUNGATEDATABUF		RecvBuf;
	//CRITICAL_SECTION	SendBufLock;
	RUNGATEDATABUF		SendBuf;
#ifdef WIN32
	RUNSOCKOVERLAPPED	Overlapped;
#endif
	CHAR				sLoginAccount[jxSrvDef::ACCOUNT_NAME_BUFF_LENGTH];
	CHAR				sLoginCharName[jxSrvDef::ACTOR_NAME_BUFF_LENGTH];
	//int				nSessionId;//登录会话ID
	char				sRecvBuf[SESSION_RECV_BUFSIZE+1];//数据接收缓冲区 
	BYTE				btRcv3[3];
	short				nSessionSatus;  		//	tagSessionStatus里定义
	Encrypt             encryptHander;   		//	加密器
	int					nTimeStampOffset; 		//	当前发送队列中已经处理过时间戳的偏移量
	unsigned long long	ullSvrTimeDiff;			//	服务器时间和本地时间差
	TICKCOUNT			nLastSendPacketTime; 	//	上一次发包间隔
}RUNGATEUSERSESSION, *PRUNGATEUSERSESSION;

#define	INIT_SESSION_SEND(s)		//InitializeCriticalSection( &(s)->SendBufLock )
#define	UNINIT_SESSION_SEND(s)		//DeleteCriticalSection( &(s)->SendBufLock )
#define	LOCK_SESSION_SEND(s)		//EnterCriticalSection( &(s)->SendBufLock )
#define	TRYLOCK_SESSION_SEND(s)		TRUE//TryEnterCriticalSection( &(s)->SendBufLock )
#define	UNLOCK_SESSION_SEND(s)		//LeaveCriticalSection( &(s)->SendBufLock )

#endif
