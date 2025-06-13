#ifndef GATE_DB_REQUEST_HANDLER_H_
#define GATE_DB_REQUEST_HANDLER_H_


/*
* 用于用户登录的验证阶段的处理
*/
class CSQLConnection;
class CDataPacket;
class CDBDataServer;
class ILogicDBRequestHost;

class CGateDBReqestHandler
{
public:
	CGateDBReqestHandler();
	~CGateDBReqestHandler();

	bool Init(CSQLConenction *pConn, CDBDataServer *pMgr);

	/*
	* Comments: 查询指定账号角色列表
	* Param const int nAccountId: 账号ID
	* Param const int nServerId: 服务器ID
	* Param CDataPacket & packet: 输出数据包
	* @Return void:
	* @Remark: 输出内容包括：错误码（1Byte) + 角色数目（1Byte） + 角色信息列表。如果查询失败，只有错误码。
	*/
	void GetActorList(const unsigned int nAccountId, const int nServerId, CDataPacket &packet);

	
	void GetActorSecondPsw(const unsigned int nAccountId, const int nServerId, CDataPacket &packet);

	/*
	* Comments: 选择角色准备进入游戏
	* Param const int nServerId: 登录的逻辑服务器ID
	* Param const int nRawServerId: 登录的原始服务器ID
	* Param const int nAccountId: 账号ID
	* Param const int nActorId: 角色ID
	* Param const char * szAccount: 账号名
	* Param LONGLONG nIP: 登录IP地址
	* @Return int: 返回错误码。0表示成功。
	* @Remark:
	*/
	int StartEnterGame(const int nServerId, 
					   const int nRawServerId, 
					   const unsigned int nAccountId, 
					   const unsigned int nActorId, 
					   const char *szAccount, 
					   LONGLONG nIP);
	/*
	* Comments:获取登陆的消息包
	* Param const jxSrvDef::INTERSRVCMD nCmd:消息码
	* Param CDataPacketReader & inPacket:传入的读写器
	* Param CDataPacket & retPack:返回的读写器
	* @Return void:
	*/
	void OnRecvLoginMsg(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient);
	
	//创建角色
	VOID  OnCreateActorResult( INT_PTR nError,unsigned int nActorId,PCREATEACTORDATA pData,ILogicDBRequestHost * pDataClient);
	//创建跨服角色
	VOID  OnCreateCrossServerActorResult( INT_PTR nError,unsigned int nActorId,PCREATEACTORDATA pData,ILogicDBRequestHost * pDataClient);

private:
	/*
	* Comments: 获取最少使用的职业，用于创建角色默认选择的职业选择用
	* Param const int nServerId:
	* @Return tagActorVocation:
	* @Remark:
	*/
	int QueryLessJobReq(const int nServerId);

	//创建角色
	void OnCreateActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient );


	//删除角色
	void OnDeleteActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient );

	//随机名字
	void OnRandName( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient );


	/*
	* Comments: 获取当前可以选择的阵营列表，用于创建角色选择阵营用
	* Param const int nServerId:
	* @Return int: 返回可以选择的阵营对应位掩码。低3位有效，为0表示对应的阵营不可选；为1表示阵营可选；
				   返回值为7表示所有阵营都可以选择
	* @Remark: 
	*/
	int QueryZyList(const int nServerId);

	/*
	* Comments: 获取使用最少（最优先）的阵营
	* Param const int nServerId: 逻辑服务器ID
	* @Return tagZhenying:
	* @Remark:
	*/
	tagZhenying QueryZYReq(const int nServerId);

	//改名字
	void OnChaneActorNameToDb(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient);
	void OnCheckSecondPsw(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient);
	void OnCreateSecondPsw(CDataPacketReader & inPacket,ILogicDBRequestHost * pDataClient);
	INT_PTR QueryActorCount( INT_PTR nAccountId, INT_PTR nServerIndex );
	INT_PTR CreateCrossServerActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient);
	INT_PTR QueryCrossServerActor( CDataPacketReader& inPacket,ILogicDBRequestHost * pDataClient);

private:
	CSQLConenction		*m_pSQLConnection;
	CDBDataServer       *m_pDBServer; //是这个指针

};
#endif