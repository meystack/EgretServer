#pragma once

/*************************************
	实现逻辑服务器中各种网络包的处理
**************************************/

class CLogicEngine;
class CLogicServer;

typedef struct tagLogicUserData
{
	unsigned  nHandleIndex;
}LOGICUSERDATA,*PLOGICUSERDATA;


typedef struct tagLogicInternalIntMessage : public Counter<tagLogicInternalIntMessage>
{
	UINT		uMsg;	//消息号
	Uint64	uParam1;//消息参数1
	Uint64	uParam2;//消息参数2
	Uint64	uParam3;//消息参数3
	Uint64	uParam4;//消息参数4
	Uint64	uParam5;//消息参数5
	Uint64	uParam6;//消息参数6
	Uint64  uParam7;
	int     uParam8;
} LOGICINTERNALMSG, *PLOGICINTERNALMSG;



class CNetWorkMsgHandle:
	public CComponent
{


public:
	CNetWorkMsgHandle();
	virtual ~CNetWorkMsgHandle();
public:

	//处理数据包
	VOID RunOne();

	VOID PostUserDataList(const INT_PTR nGateIdx, CBaseList<CDataPacket*>& vList);

	//这个是给非逻辑线程发消息给逻辑 用的
	VOID PostInternalMessage(UINT uMsg, UINT64 uParam1=0, UINT64 uParam2=0, UINT64 uParam3=0,UINT64 uParam4=0,UINT64 uParam5 =0,UINT64 uParam6 =0,UINT64 uParam7=0,int uParam8 = 0);
	
	//逻辑线程调用,处理外部线程的消息
	VOID ProcessInternalMessages(); //

	/*
	VOID PostMsgSendToGate(INT_PTR nGateIdx,SOCKET nUserSocket, int nGateSessionIndex,WORD wServerSessionIdx,
		char* pUserData,SIZE_T nSize);
	CDataPacket& AllocPacket(CActorPacket &pack, INT_PTR nGateIdx,SOCKET nUserSocket, int nGateSessionIndex,WORD wServerSessionIdx);
	*/
	//CBufferAllocator* GetAllocator() { return &m_Allocator; }
	void DumpBuffAlloc(wylib::stream::CBaseStream& stream);
private:
	VOID DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4,UINT64 uParam5,UINT64 uParam6,UINT64 uParam7,int uParam8 = 0);
	
	// 发送网关用户信息给DB，用于DB选择服务器
	void SendGateUserInfoToDB();
private:
	CLogicEngine*						m_pLogicEngine;
	
	CQueueList<CDataPacket*>			m_GateUserDataList[CCustomGateManager::MaxGateCount];
	CCSLock								m_GateDataLock[CCustomGateManager::MaxGateCount];
	
	CQueueList<PLOGICINTERNALMSG>		m_ItnMsgList;		//内部消息列表
	CCSLock								m_ItnMsgListLock;		//内部消息列表锁

	//服务器器下发的数据的缓存
	CQueueList<CDataPacket*>			m_GateServerPushdownDataList[CCustomGateManager::MaxGateCount];
	CCSLock								m_GateServerPushdownLock[CCustomGateManager::MaxGateCount];

	//CDataPacket*						m_GateServerSendToGateServerMsg[CCustomGateManager::MaxGateCount];
	CTimer<60000>						m_SyncGateUserTimer;	// 同步网关用户定时器
	CBufferAllocator					m_Allocator;			//内存池

	//typedef  Handle<LOGICINTERNALMSG> MsgHandle;
	//HandleMgr<LOGICINTERNALMSG,MsgHandle>  m_memoryPool; //使用齐整内存，避免频繁申请内存

};