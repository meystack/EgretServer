#ifndef _SESSION_HANDLE_H_
#define _SESSION_HANDLE_H_
/*****************************************************************
            Copyright (c) 2021, 上海漫方网络科技有限公司
                    All rights reserved
       
    创建日期：  2021年04月28日 14时01分
    文件名称：  CrossDataHandle.h
    说    明：  跨服引擎分发消息
    
    当前版本：  1.00
    作    者：  
    概    述： 	跨服引擎分发消息类

*****************************************************************/
using namespace wylib::sync::lock;

class CCrossServerManager;
class CCrossServer;

class ISessionRequestHost
{
public:	
	/*
	* Comments: 分配数据包
	* Param const jxSrvDef::INTERSRVCMD nCmd:
	* @Return CDataPacket&:
	* @Remark:
	*/
	virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd) = 0;

	/*
	* Comments: 刷新数据包到发送队列
	* Param CDataPacket &packet
	* @Return void:
	* @Remark:
	*/
	virtual void FlushDataPacket(CDataPacket &packet) = 0;


};

using namespace jxSrvDef;
class CCrossDataHandle :public ISessionRequestHost
{
	friend class CCrossServer;
protected:
	//VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	VOID DispatchRecvMsg(CDataPacketReader & reader);

	//VOID SendKeepAlive();

	//实现虚接口
	VOID OnPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);
public:
	CCrossDataHandle( );
	~CCrossDataHandle();
	
	void  SetParam(CCrossServer *pSessionSrv, bool isSessonConnect  );

	//获取服务器的id
	inline int GetServerId()
	{
		return m_nSelfServerId;
	}

	/*
	* Comments: 处理来自逻辑服务器的请求传送到目标服务器消息
	* Param CatchDefaultPacket:
	* Param & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchRequestTransmit(CDataPacketReader &inPacket) {};

	/*
	* Comments: 收到战区的数据
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchGroupMessage(CDataPacketReader &inPacket) {};


	// VOID BroadcastMessage2Logic(CDataPacketReader &inPacket);

	VOID BroadcastMessage2Logic(const INT_PTR nServerIndex, LPCSTR sMsg, const size_t dwSize);
	/*
	* Comments: 保存跨服排行榜
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchSaveCsRank(CDataPacketReader &inPacket) {};

	/*
	* Comments: 获取跨服排行榜
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchLoadCsRank(CDataPacketReader &inPacket) {};
	
public:

	static const size_t MaxSSDataSize = 4096;//向会话服务器发送的单个数据包大小的最大值
	static const size_t MaxForwardLogicDataSize = 40960; // 最大数据包为40k
	char					*m_pForwardDataBuff;	// 公共逻辑服务器SessionClient用于转发公共服务器消息的Buff，长度固定MaxForwardLogicDataSize

private:
	int                     m_nSelfServerId;    //自身的服务器ID,用于数据转发
	CCrossServer*			m_pSSServer;		//所属会话服务器
	TICKCOUNT				m_dwDisconnectedTick;//断开连接的时间
	char                    m_sTrServerIndex[24] ; //格式化的服务器的index，用于连接的时候快速计算
	bool                    m_bIsCrossSession;    //是否是跨服的会话传输的client，跨服的会话有些东西要特殊处理下
};


#endif

