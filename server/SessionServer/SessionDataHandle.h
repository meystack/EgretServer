#ifndef _SESSION_HANDLE_H_
#define _SESSION_HANDLE_H_

using namespace wylib::sync::lock;

class CSSManager;
class CSessionServer;
#include "AMProcto.h"
#include "AMClient_tx.h"
/************************************************************
*
* 数据引擎连接到会话服务器的客户端类
*
***********************************************************/

using namespace jxSrvDef;

class CSessionDataHandle  :
	public ISessionRequestHost
{
	friend class CSessionServer;
public:

	static const size_t MaxSSDataSize = 4096;//向会话服务器发送的单个数据包大小的最大值
	typedef VOID (CSessionDataHandle::*OnHandleSockPacket)(CDataPacketReader &packet);
	static const size_t MaxForwardLogicDataSize = 40960; // 最大数据包为40k
	//大型SQL查询语句缓冲长度
	static const SIZE_T dwHugeSQLBufferSize = 1024 * 1024 * 2;

	char					*m_pForwardDataBuff;	// 公共逻辑服务器SessionClient用于转发公共服务器消息的Buff，长度固定MaxForwardLogicDataSize
private:
	int						m_nCommServerId;	// 公共逻辑服务器Id，如果自己是公共服务器，那么此ID设置为0
	int                     m_nSelfServerId;    //自身的服务器ID,用于数据转发
	
	
	INT						m_nUserCount;		//在线人数
	INT						m_nUserLimit;		//人数上限
	INT						m_nMaxUserCount;	//在本次日志记录中的最高在线人数，记录人数日志后此值被还原为m_nUserCount
	CSessionServer*			m_pSSServer;		//所属会话服务器
	CSQLConenction*			m_pSQLConnection;	//数据库连接程序
	TICKCOUNT				m_dwDisconnectedTick;//断开连接的时间

	PCHAR					m_pHugeSQLBuffer;	//大型SQL查询语句缓冲，长度为dwHugeSQLBufferSize

	char                    m_sTrServerIndex[24] ; //格式化的服务器的index，用于连接的时候快速计算

	bool                    m_bIsCrossSession;    //是否是跨服的会话传输的client，跨服的会话有些东西要特殊处理下

protected:
	//VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	VOID DispatchRecvMsg(CDataPacketReader & reader);

	//VOID SendKeepAlive();

	//实现虚接口
	VOID OnPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket);


protected:

	/*
	* Comments: 分配数据包
	* Param const jxSrvDef::INTERSRVCMD nCmd:
	* @Return CDataPacket&:
	* @Remark:
	*/
	//virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd) 
	//{
	//}

	/*
	* Comments: 刷新数据包到发送队列
	* Param CDataPacket &packet
	* @Return void:
	* @Remark:
	*/
	//virtual void FlushDataPacket(CDataPacket &packet) {}


	/*
	* Comments:  是否是1个合法的新手卡
	* Parameter: unsigned int nAccountId
	* Parameter: UINT64 lCard:卡的号码
	* Parameter: char * sAccountName:账户的名字，如果不传入将会去查找
	* @Return  int:返回结果,0表示完全匹配，-1表示是新手卡，但是卡号不对，-2表示不是新手卡
	*/
	int isInvalidNewUserCard(unsigned int nAccountId, UINT64 lCard,char * sAccountName =NULL);


private:
	static const OnHandleSockPacket		SSSockPacketHandlers[];	//结构化网络数据包处理函数列表


	
public:
	CSessionDataHandle( );
	~CSessionDataHandle();
	
	
	void  SetParam(CSessionServer *pSessionSrv,CSQLConenction *pSqlConn, bool isSessonConnect  );


	//获取公共服的id
	inline int GetCommonServerId() const  
	{ 
		if( m_nCommServerId == 0)
		{
			return m_nSelfServerId;
		}
		else
		{
			return m_nCommServerId;
		}
	}

	//获取服务器的id
	inline int GetServerId()
	{
		return m_nSelfServerId;
	}


	VOID CatchDefaultPacket(CDataPacketReader &inPacket);
	VOID CatchCloseSession(CDataPacketReader &inPacket);
	VOID CatchUpdateSession(CDataPacketReader &inPacket);
	VOID CatchQuerySessionResult(CDataPacketReader &inPacket);
	VOID CatchOnlineCount(CDataPacketReader &inPacket);
	VOID CatchLoadUserItem(CDataPacketReader &inPacket);
	VOID CatchGetUserItem(CDataPacketReader &inPacket);
	VOID CatchRemoveUserItem(CDataPacketReader &inPacket);
	VOID CatchAddValueCard(CDataPacketReader &inPacket);
	VOID CatchQueryAddValueCard(CDataPacketReader &inPacket);
	// 查询元宝数量
	VOID CatchQueryYuanbaoCount(CDataPacketReader &inPacket);
	// 提取元宝
	VOID CatchWithdrawYuanbao(CDataPacketReader &inPacket);

	//确认登陆了
	VOID CatchConformOpenSession(CDataPacketReader &inPacket);

	/*
	* Comments: 处理来自逻辑服务器的请求传送到目标服务器消息
	* Param CatchDefaultPacket:
	* Param & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchRequestTransmit(CDataPacketReader &inPacket);

	/*
	* Comments: 收到战区的数据
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchGroupMessage(CDataPacketReader &inPacket);

	//检测用户名密码
	VOID CatchCheckUserPassword(CDataPacketReader &inPacket);

	//创建账户
	VOID CatchCreateAccount(CDataPacketReader &inPacket);


	// 发送AMC应答消息给逻辑服务器
	void sendAMCMsgAck(const CAMClient::AMOPDATA & data);

	//发送任务给逻辑服务器应答
	void SendAmTaskAck(const CAMClient::TASKMSG & data);

	/*
	* Comments: 公告消息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchBroadcastMessage(CDataPacketReader &inPacket);

	/*
	* Comments: 保存跨服排行榜
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchSaveCsRank(CDataPacketReader &inPacket);

	/*
	* Comments: 获取跨服排行榜
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchLoadCsRank(CDataPacketReader &inPacket);

	/*
	* Comments: 清除跨服排行数据
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchClearCsRank(CDataPacketReader &inPacket);
	

	//设置跨服的数据库ID
	VOID SetCommonServerId(CDataPacketReader &inPacket);
	
	//检测玩家是否合法，返回错误码
	int CheckUserValid(PCHAR name, PCHAR passwd, GLOBALSESSIONOPENDATA & sessiDonata,PCHAR md5,PCHAR sCurrentTime,bool checkMd5,bool bCommonSrvLogin,PCHAR identity,bool &isWhiteIpLogin,int nRawServerindex=0);

	/*
	* Comments: 收到后台的消息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatGetContrlCenterMsg(CDataPacketReader &inPacket);


	
	/*
	* Comments: 收到腾讯平台的消息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatQQMsg(CDataPacketReader &inPacket);

	/*
	* Comments: 收到连接会话公告服的数据
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatSendCommonPlatformData(CDataPacketReader &inPacket);

	/*
	* Comments: 请求传送到跨平台
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatRetranToCommonPlatform(CDataPacketReader &inPacket);

	/*
	* Comments: 封禁IP，并且把这个区的玩家全部踢下线
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	* @Remark:
	*/
	VOID CatchSealIp(CDataPacketReader &inPacket);


	/*
	* Comments:  解封玩家的IP
	* Parameter: CDataPacketReader & inPacket:
	* @Return  VOID:
	*/
	VOID CatchUnSealIp(CDataPacketReader &inPacket);


	//字符串的ip转整型的
	long long GetIntIp(char *sIp);

};


#endif

