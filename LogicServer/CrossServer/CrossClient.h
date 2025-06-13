#ifndef _CROSS_CLIENT_H_
#define _CROSS_CLIENT_H_
/*****************************************************************
            Copyright (c) 2021, 上海漫方网络科技有限公司
                    All rights reserved
       
    创建日期：  2021年04月28日 14时01分
    文件名称：  CrossClient.h
    说    明：  数据引擎连接到跨服服务器的客户端类
    
    当前版本：  1.00
    作    者：  
    概    述：  数据引擎连接到跨服服务器的客户端类

*****************************************************************/
using namespace wylib::sync::lock;

class CCrossServerManager;
class CCrossServer;

using namespace jxSrvDef;

class CCrossClient 
	: public CCustomJXServerClientSocket,
	public CCrossDataHandle
{
	friend class CCrossServer;
public:
	typedef CCustomJXServerClientSocket Inherited;
	typedef CCrossDataHandle InHandle;
protected:
	//VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	VOID DispatchRecvMsg(CDataPacketReader & reader);
	//实现虚接口
	VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
	bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData) ;
	virtual void OnRegDataValidated();

protected:
	VOID OnDisconnected();
	VOID OnError(INT errorCode);
	//覆盖父类例行执行的函数
	VOID OnRun();
	//由SessionServer调用的例行执行函数
	inline VOID Run(){ SingleRun(); }

protected:

	//重写收 发包的函数
	virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd) 
	{
		return Inherited::allocProtoPacket(nCmd);
	}
	
	virtual void FlushDataPacket(CDataPacket &packet)
	{
		return Inherited::flushProtoPacket(packet);
	}


public:
	CCrossClient(){}
	~CCrossClient(){}
	//反馈是否可以跨平台的消息给逻辑服
	void OnSendReqCommonPlatformResult(CDataPacketReader &inPacket);

private:
	int                     nCrossSrvId;//跨服id
	TICKCOUNT				m_dwDisconnectedTick;//断开连接的时间

	static const size_t MaxSSDataSize = 4096;//向会话服务器发送的单个数据包大小的最大值
	typedef VOID (CCrossClient::*OnHandleSockPacket)(CDataPacketReader &packet);
	static const size_t MaxForwardLogicDataSize = 40960; // 最大数据包为40k
	//大型SQL查询语句缓冲长度
	static const SIZE_T dwHugeSQLBufferSize = 1024 * 1024 * 2;
};


#endif

