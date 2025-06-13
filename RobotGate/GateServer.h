#ifndef	_RUNGATE_H_
#define	_RUNGATE_H_

#ifndef	EXPORTCALL
#define	EXPORTCALL __stdcall
#endif


/*作为插件导出函数中查询发送线程状态的结构*/
typedef struct tagRunGateSendThreadQueryStruct
{
#ifdef _MSC_VER
	HANDLE				hThread;
#else
	pthread_t			hThread;
#endif
	DWORD				dwThreadId;
	TICKCOUNT			dwProcTick;
	TICKCOUNT			dwSleepTick;
}RUNGATESENDTHREADQUERYSTRUCT, *PRUNGATESENDTHREADQUERYSTRUCT;


/*作为插件导出函数中枚举会话信息的结构*/
typedef struct tagRunGateQuerySessionStruct
{
	SOCKET				nSocket;
	PRUNGATEUSERSESSION	pSession;
	SOCKADDRIN			SockAddr;
	int					nRecvPacketCount;
	int					nSendPacketCount;
	WORD				wPacketError;
	BYTE				btPacketIdx;
	bool				boSendAvaliable;
	unsigned long long					nVerifyIdx;		
	int					nRecvBufSize;
	int					nRecvCacheSize;
	int					nSendBufSize;
	int					nSendCacheSize;
	TICKCOUNT			dwActiveTick;
	TICKCOUNT			dwClientMsgTick;
	TICKCOUNT			dwServerMsgTick;
	//CHAR				sLoginAccount[32];
	//CHAR				sLoginCharName[32];
	int					nSessionId;
}RUNGATEQUERYSESSIONSTRUCT, *PRUNGATEQUERYSESSIONSTRUCT;

/*作为插件导出函数中枚举会话信息的回调函数类型*/
typedef INT (CALLBACK *RUNGATE_ENUMSESSION_ROUTINE)	(LPVOID lpRunSockProcesser, PRUNGATEQUERYSESSIONSTRUCT pSessionInfo);

/*作为插件导出函数中查询数据统计信息的结构*/
typedef struct tagRunDataStatics
{
	SIZE_T	dwProcessRecvSize;			//处理客户端接收数据大小
	SIZE_T	dwWaitSendUserSize;			//待发送给用户的数据包大小
	SIZE_T	dwWaitSendQueueSize;		//待发给用户的数据包队列大小
	SIZE_T	dwRecvSeverSize;			//接收服务器数据大小
	SIZE_T	dwSendUserSize;				//发送用户数据大小
	TICKCOUNT	dwLastProcUsrMsgTime;		//上次处理用户数据时间
	TICKCOUNT	dwLastProcSrvMsgTime;		//上次处理服务器数据时间
	TICKCOUNT	dwLastRecvSrvMsgTime;		//上次接收服务器数据时间
	TICKCOUNT	dwProcSrvThreadSleep;		//处理服务器数据线程休眠时间
	SIZE_T	dwSendQueueSize;			//待发数据队列数据包大小
}RUNDATASTATICS, *PRUNDATASTATICS;

#endif

