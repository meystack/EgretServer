#pragma once
/***************************************************************/
/*
/*                     逻辑服务器网关连接对象
/*
/*              实现逻辑服务器中的网关通信以及网关用户管理
/*
/***************************************************************/
class CLogicGateUser;

class CLogicServer;
typedef	CQueueList<CDataPacket*>	DPVECTOR;
class CLogicGate :
	public CCustomServerGate
{
	friend class CLogicGateManager;
	
	enum
	{
		MAX_ACCE_PKG_SIZE = 256*1024,
	};
public:
	typedef CCustomServerGate Inherited;
	static const INT_PTR MAX_GATE_USER = 8192;	//最大网关用户数
	static int s_nMaxPacketSizes;	
public:
	CLogicGate();
	~CLogicGate();

	//
	inline void freeBackUserDataList(CBaseList<CDataPacket*> &list)
	{
		m_vFreeList.appendList(list);
		list.trunc(0);
	}

	inline VOID PostSetActorHandleIndex( int nServerSessionIndex,unsigned nHandleIndex,unsigned int nAccountId)
	{
		Inherited::PostInternalMessage(SSM_GATE_CREATE_ACTOR,  nServerSessionIndex, nHandleIndex,nAccountId);
	}
	
	//inline CDataPacket& allocSendPacket()
	//{
	//	return CSendPacketPool::allocSendPacket();
	//}

	//添加逻辑已经处理过的消息的列表,将他添加到该列表，以后回收利用
	inline void AddLogicProcessedData(CDataPacket * pData)
	{
		m_logicProcDataList.add(pData);
	}

	inline void AddLogicProcessedDataList(CBaseList<CDataPacket*> &list)
	{
		m_logicProcDataList.addList(list);
		list.clear();
	}

	//获取逻辑已经处理过的网络消息列表，需要回收利用
	inline CBaseList<CDataPacket *> & GetLogicProcecdDataList()
	{
		return m_logicProcDataList;
	}

	inline void flushSendPacket(CDataPacket& packet)
	{
		 CSendPacketPool::flushSendPacket(packet);
	}

	//设置网关的编号
	inline void SetGateIndex(int nIndex)
	{
		m_nGateIndex =nIndex;
	}

	inline CDataPacket* GetSendToGatePacket()
	{
		static int sSendPkgCount = 1;
		static const unsigned int sMax = ~(0x400-1);
		if (m_pSendToGatePacket == NULL)
		{
			CDataPacket& dp = allocSendPacket();
			dp.setPosition(0);
			m_pSendToGatePacket = &dp;

			if(m_bUseBigPacket)
			{
				GATEMSGHDR hdr;
				hdr.dwGateCode = RUNGATECODE;			
				hdr.nSocket	= 0;
				hdr.wSessionIdx= (WORD)1;
				hdr.wIdent		= GM_APPBIGPACK;
				hdr.wServerIdx = 1;
				hdr.nDataSize = 0;
				hdr.wTemp = RUNTEMPCODE;
				hdr.tickCount = 0;
				dp << hdr;
			}
		}
		m_nSendPacketCount++;
		sSendPkgCount++;
		if (sSendPkgCount >= INT_MAX)
			sSendPkgCount = 1;
		if (sSendPkgCount & sMax)
			CheckMemory(true);
		return m_pSendToGatePacket;
	}

	inline VOID SendToGate()
	{
		
		if (m_pSendToGatePacket != NULL)
		{
//#pragma __CPMSG__(调试结束后要删掉)
//			checkcdcd(*m_pSendToGatePacket);
			size_t nSize = m_pSendToGatePacket->getLength();
			m_ullSendPacketSize += nSize;
			/*if (m_pSendToGatePacket->getLength() > s_nMaxPacketSizes)
			{
				s_nMaxPacketSizes = m_pSendToGatePacket->getLength();				
			}*/
			if (m_bUseBigPacket)
			{
				GATEMSGHDR *pHeader = (GATEMSGHDR*)m_pSendToGatePacket->getMemoryPtr();
				pHeader->nDataSize =(int)( nSize - sizeof(GATEMSGHDR) );
			}
			FlushGateBigPacket(*m_pSendToGatePacket);
			//设置nULL，下次再申请
			m_pSendToGatePacket = NULL;
		}
	}

	void ResetSendPacketSize() { m_ullSendPacketSize = 0;  m_nSendPacketCount = 0; }
	unsigned long long GetSendPacketSize() const { return m_ullSendPacketSize; }
	int GetSendPacketCount() const { return m_nSendPacketCount; }	

	void DumpBuffAlloc(wylib::stream::CBaseStream& stream)
	{		
		CBuffAllocStatHelper::Dump(stream, &m_Allocator, _T("LogicGate::CustomWorkSock::Allocator"), false);
		
		/*
		CBufferAllocator::ALLOCATOR_MEMORY_INFO mi;		
		GetPackPoolAllocator()->GetMemoryInfo(&mi);
		CBuffAllocStatHelper::Dump(stream, mi, _T("LogicGate::CustomWorkSock::SendPackPool::Allocator"), false);
		*/
	}

	inline void checkcdcd(CDataPacket &pack)
	{
		PGATEMSGHDR pHdr;
		char *buf = pack.getMemoryPtr();
		char *pend = buf + pack.getLength();
		while (buf < pend)
		{
			pHdr = (PGATEMSGHDR)buf;
			if (pHdr->nDataSize <= 0)
			{
				//OutputMsg(rmTip, _T("checkcdcd %d"), pHdr->nDataSize);
				Assert(0);
			}
			if (pHdr->nDataSize > 0x8000)
			{
				//OutputMsg(rmTip, _T("checkcdcd %d"), pHdr->nDataSize);
				Assert(0);
			}
			Assert(pHdr->nDataSize > 0 && pHdr->nDataSize <= 0x8000 );
			Assert( pHdr->dwGateCode == RUNGATECODE);
			Assert( pHdr->wTemp == RUNTEMPCODE);
			buf += sizeof(*pHdr) + pHdr->nDataSize;
		}
	}
	//bool IsConnected() { return connected();}
	CCustomSocket::connected;
	CSendPacketPool::getPacketCount;	

	//通过
	CLogicGateUser * GetGateUserByKey(UINT64 lKey);


	/*
	* Comments:踢下一个用户
	* Param ACCOUNTDATATYPE accountId:账户的id
	* Param UINT64 lKey:玩家的key
	* @Return void:
	*/
	inline void PostKickUser(ACCOUNTDATATYPE accountId,UINT64 lKey)
	{
		PostInternalMessage(SSM_KICK_USER,accountId,lKey,0,0);
	}

	//关闭连接的网关程序
	inline void PostCloseGate()
	{
		PostInternalMessage(SSM_CLOSE_SERVER,0,0,0,0);
	}

protected:
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4);

	/*** 覆盖父类的逻辑处理函数集 ***/
	CCustomServerGateUser* CreateGateUser();
	VOID DestroyGateUser(CCustomServerGateUser* pUser);
	VOID OnOpenUser(CCustomServerGateUser *pGateUser);
	VOID OnCloseUser(CCustomServerGateUser *pGateUser);
	VOID OnGateClosed();
	VOID OnDispatchUserMsg(CCustomServerGateUser *pUser, char* pBuffer, SIZE_T nBufferSize);
	VOID OnRun();

	//向会话服务器发送登陆消息
	void Client2SesionCheckUserPasswd(CDataPacketReader &reader,CLogicGateUser* pGateUser); //校验用户名和密码

	//向会话服务器创建账户
	void Client2SesionCreateAccount(CDataPacketReader &reader,CLogicGateUser* pGateUser); //创建账户


	//向数据服发送的登陆消息
	void Client2DBGetActorList(CDataPacketReader &reader,CLogicGateUser* pGateUser);  //获取玩家列表
	void Client2DbDeleteActor(CDataPacketReader &reader,CLogicGateUser* pGateUser);    //删除用户
	void Client2DbCreateActor(CDataPacketReader &reader,CLogicGateUser* pGateUser);    //创建用户
	
	void Client2DbRandName(CDataPacketReader &reader,CLogicGateUser* pGateUser);    //随机名字
	void Client2DbCheckSecondPsw(CDataPacketReader &reader,CLogicGateUser* pGateUser);
	void Client2DbCreateSecondPsw(CDataPacketReader &reader,CLogicGateUser* pGateUser);
	

	//向db服发请求获取角色列表
	void SendDbGetActorList(CLogicGateUser* pGateUser);


	

	//重写母函数的这个函数，在传送的时候，登陆公共服务器的时候断开将导致问题，这里需要统一处理下
	virtual VOID KickUserByGlobalSessionId(const int nGlobalSessionId);

	//向客户端返回登陆的错误
	void ResponseLoginErrorAndCloseUser(CLogicGateUser *pUser,int nErrorCode);


private:
	//处理用户列表
	VOID ProcessUsers();
	VOID AddAllGateUsersToFreeList();

	/*
	有一个帐户登陆了，判断一下是不是正确的
	*/
	VOID OnUserLogin(CLogicGateUser* pUser,unsigned nAccountID,unsigned nActorID,int nLoginIndex=0);

private:
	CLogicServer					*m_pLogicServer;			//所属逻辑服务器
	CLogicGateUser					m_GateUsers[MAX_GATE_USER];	//静态的网关用户数组
	CBaseList<CLogicGateUser*>		m_FreeUserList;	//空闲网关用户列表

	CBaseList<CDataPacket*>			m_vMsgList;	//临时用来中转数据内容,数据中是一连串的完整包
	DPVECTOR						m_vFreeList;	//用来存放空闲的数据包
	CCSLock							m_vFreeListLock;
	//CCSLock                         m_msgLock;   //消息的锁

	CDataPacket*					m_pSendToGatePacket;
	CBaseList<CDataPacket *>        m_logicProcDataList; //逻辑已经处理过的数据包 
	CTimer<5000>					m_ProcessUserInterval;
	//void							
	unsigned long long				m_ullSendPacketSize;  // 发送数据包大小
	int								m_nSendPacketCount; // 发送数据包数量
	bool							m_bUseBigPacket;	// 是否使用大数据包（用于提高网络吞吐量）
	//int                             m_nGateIndex;       //网关的编号
public:
	/*------------------------
	//跨服相关逻辑
	-------------------------*/
	
	//用户验证客户端账号（虚假的）--可以发送登录请求了
	void SimulateClient2SesionCheckUserPasswd(CDataPacketReader &pack,CLogicGateUser* pLogicGateUser);
};