#ifndef LOGIC_DB_REQUEST_HANDLER_H_
#define LOGIC_DB_REQUEST_HANDLER_H_

struct CUserItem;
struct tagOneSkillDbData;


/*
	逻辑DB请求处理类。
*/

// Forward Declaration
class CSQLConenction;
class ILogicDBRequestHost;

class CLogicDBReqestHandler
{
public:	
	CLogicDBReqestHandler();
	~CLogicDBReqestHandler();

public:
	inline void SetSQLConnection(CSQLConenction *pSQLConn) { m_pSQLConnection = pSQLConn; }

	inline CSQLConenction* GetSqlConnection() {return m_pSQLConnection;}
	
	inline void SetRequestHost(ILogicDBRequestHost *pHost) { m_pHost = pHost; }

	//追踪内存使用情况
	void Trace ();

	///*
	//* Comments: 获取连接对端的ServerId
	//* @Return int:
	//* @Remark:
	//*/
	//int getClientServerIndex();
	
	//////////////////////////////////////////////////////////////////////////
	// 杂DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void ExecuInitDb(CDataPacketReader &inPacket);
	void OnResponseDbSessionData(CDataPacketReader &inPacket);
	void UpdateLogicGateUserInfo(CDataPacketReader &inPacket);

	void LogicSendStopDbServerCmd(CDataPacketReader &inPacket);

	void OnReloadCrossConfig(CDataPacketReader &inPacket);

	//////////////////////////////////////////////////////////////////////////
	// 角色基本数据相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void SaveActorCustomInfo(CDataPacketReader& inPacket);//保存自定义数据
	void QueryActorCustomInfo(CDataPacketReader& inPacket);//查询自定义数据
	void QueryActorCustomInfoUnForbid(CDataPacketReader& inPacket);
	void SaveActorCustomInfoUnForbid(CDataPacketReader& inPacket);//保存自定义数据

	void QueryActorCustomInfoCustomTitleId(CDataPacketReader& inPacket);
	void SaveActorCustomInfoCustomTitleId(CDataPacketReader& inPacket);//保存自定义数据
 
	unsigned int QueryActorFromDB(CDataPacketReader &inPacket);
	void QueryActoSubSystemData(CDataPacketReader &inPacket);
	void SaveOfflineActorProperty(CDataPacketReader& inPacket);
	
	
	void QueryOfflineUserInfo(CDataPacketReader &inPacket);
	void QueryOfflineUserHeroInfo(CDataPacketReader &inPacket);
	void SaveActorToDB(CDataPacketReader &inPacket, bool bResp = true);
	void UpdateChallengeData(CDataPacketReader &inPacket);	
	void QueryActorExists(CDataPacketReader & inPacket);
	void UpdateCharStatus(CDataPacketReader & inPacket);
	void UpdateCharOnlineStatus(unsigned int nActorId, int nStatus);
	void ChangeActorTradingQuota(CDataPacketReader & inPacket);// 修改玩家交易额度

	//计数器
	void LoadCircleCount();
	void LoadActorStaticCounts(unsigned int nActorId, unsigned int nRawServerId, unsigned int nLoginServerId);
	void SaveActorStaticCounts(CDataPacketReader& packet);

	//强化
	void QueryStrengthenImpl(MYSQL_ROW pRow, CDataPacket& OutPacket);
	void LoadActorStrengthenInfo( unsigned int nActorID,unsigned int nRawServerId,unsigned int nLoginServerId);
	void SaveActorStrengthenInfo(CDataPacketReader& packet);

	//圣物dcSaveHallowsSystemInfo
	void SaveHallowsSystemInfo(CDataPacketReader& packet);
	void LoadHallowsSystemInfo(CDataPacketReader& packet);
	
	//复活特权
	void LoadReviveDurationSystemInfo(CDataPacketReader& packet);
	void SaveReviveDurationSystemInfo(CDataPacketReader& packet);

	//宠物
	void QueryLootPetData(unsigned int nActorId, unsigned int nRawServerId, unsigned int nLoginServerId);
	void SaveLootPetToDB(CDataPacketReader& packet, bool bResp = true);
	
	//返利活动
	void QueryRebateData(CDataPacketReader& packet);
	void QueryNextDayRebateData(CDataPacketReader& packet);
	void UpdateRebateData(CDataPacketReader& packet);
	void SaveRebateToDB(CDataPacketReader& packet);
	void ClearRebateData(CDataPacketReader& packet);

	//跨服
	void LoadCrossGuildId(CDataPacketReader& packet);

	//成就
	void LoadActorAchieveMentInfo(unsigned int nActorId, unsigned int nRawServerId, unsigned int nLoginServerId);
	void SaveActorAchieveMentInfo(CDataPacketReader& packet);

	void LoadOfflineAchieveMentInfo(CDataPacketReader& packet);
	void SaveOfflineAchieveMentInfo(CDataPacketReader& packet);

	void SaveActorGhost(CDataPacketReader& packet);
	void LoadActorGhost(CDataPacketReader& packet);
	
	//////////////////////////////////////////////////////////////////////////
	// 背包物品以及活动背包相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryItemsImpl(MYSQL_ROW pRow, CUserItem& data);
	void QueryItems(unsigned int nActorID, unsigned int nItemStoreType, int nRawServerId, int nLoginServerId);
	void SaveItems(CDataPacketReader &inPacket, bool bResp = true);
	void LoadUserActiveItem(CDataPacketReader &inPacket);
	void GetUserActiveItem(CDataPacketReader &inPacket);
	void RemoveUserActiveItem(CDataPacketReader &inPacket);

	//衣橱系统保存
	void SaveAlmirahItem(CDataPacketReader & inPacket, bool bResp = true);
	void LoadAlmirahItem(unsigned int nActorId, int nRawServerId, int nLoginServerId);

	//////////////////////////////////////////////////////////////////////////
	// 游戏设置相关的db 请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryOtherGameSetsFromDB(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void SaveOtherGameSetsToDB(CDataPacketReader &inPacket, bool bResp = true);


	//////////////////////////////////////////////////////////////////////////
	// 技能相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QuerySkillImpl(MYSQL_ROW pRow, tagOneSkillDbData& data);
	void QuerySkillFromDB(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void SaveSkillToDB(CDataPacketReader &inPacket, bool bResp = true);

	//////////////////////////////////////////////////////////////////////////
	// 任务相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryQuestFromDB(CDataPacketReader &inPacket);
	void SaveQuestToDB(CDataPacketReader &inPacket, bool bResp = true);

	//////////////////////////////////////////////////////////////////////////
	// 脚本数据相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryActorScriptData(CDataPacketReader &inPacket);
	void QueryActorScriptData(unsigned int nActorId, int nRawServerId, int nLoginServerId);
	void SaveScriptDataToDB(CDataPacketReader &inPacket, bool bResp = true);

	//////////////////////////////////////////////////////////////////////////
	// 公会相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void LoadGuildList(CDataPacketReader &inPacket);
	void LoadGuildDetail(CDataPacketReader &inPacket);
	void AddGuild(CDataPacketReader &inPacket);
	void LoadGuildSkillDb(CDataPacketReader &inPacket);
	void DeleteMember(CDataPacketReader &inPacket);
	void SaveActorGuild(CDataPacketReader &inPacket, bool bResp = true);
	void LoadActorGuild(unsigned int nActorid);
	void AddGuildMember(CDataPacketReader &inPacket);
	void InitGuildSkillInfo(CDataPacketReader &inPacket);
	void SaveGuildSkill(CDataPacketReader &inPacket);
	void SaveGuildEventDb(CDataPacketReader &inPacket);
	void LoadGuildEventDb(CDataPacketReader &inPacket);
	void SaveGuild(CDataPacketReader &inPacket);
	void DeleteGuild(CDataPacketReader &inPacket);
	void AddMemberOffLine(CDataPacketReader &inPacket);
	void SaveGuildDepotDb(CDataPacketReader &inPacket);
	void LoadGuildDepoyFromDb(CDataPacketReader &inPacket);
	void LoadGuildDepotRecordDb(CDataPacketReader &inPacket);
	void SaveGuildDepotRecordDb(CDataPacketReader &inPacket);
	void LoadActorBuildingDb(CDataPacketReader &inPacket);
	void LoadGuildApplyList(CDataPacketReader &inPacket);
	void DealGuildApply(CDataPacketReader &inPacket);
	void LoadActorApplyResult(CDataPacketReader &inPacket);
	void SaveActorApplyResult(CDataPacketReader &inPacket);


	//////////////////////////////////////////////////////////////////////////
	// 活动相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void LoadActivityList(CDataPacketReader &inPacket);
	void SaveActivityList(CDataPacketReader &inPacket);
	void LoadActorActivity(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void SaveActorActivity(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);


	//////////////////////////////////////////////////////////////////////////
	// boss相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void LoadBossList(CDataPacketReader &inPacket);
	void SaveBossList(CDataPacketReader &inPacket);
	void LoadActorBossList(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void SaveActorBossList(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);
	
	//////////////////////////////////////////////////////////////////////////
	// 消息相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void LoadMsgListFromId(unsigned int nActorId,INT64 nMsgId, int nRawServerId, int nLoginServerId);
	void LoadMsgList(CDataPacketReader &inPacket);
	void DeleteMsg(CDataPacketReader &inPacket);
	void AddMsg(CDataPacketReader &inPacket);
	void AddMsgByName(CDataPacketReader &inPacket);

	//////////////////////////////////////////////////////////////////////////
	// 好友相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	int SaveFriendsData(unsigned int nActorid,int nRsType,int nFriendId,LPCSTR szName,UINT nParam);
	void SaveFriends(CDataPacketReader &inPacket, bool bResp = true);
	void LoadFriends(CDataPacketReader &inPacket);	
	int CleanFriends(unsigned int nActorid);


	//////////////////////////////////////////////////////////////////////////
	// 宠物相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryPetData(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void QueryPetDetailData(CDataPacketReader &inPacket); //装载宠物的装备/技能等数据
	void QueryPetSkillData(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void SavePetSkillData(CDataPacketReader &inPacket, bool bResp = true); //保存宠物的技能数据
	void SavePetToDB(CDataPacketReader &inPacket, bool bResp = true);

	//////////////////////////////////////////////////////////////////////////
	// 头衔相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void LoadBaseRank(CDataPacketReader &inPacket);
	void SaveBaseRank(CDataPacketReader &inPacket);	
	void SaveBaseRankCS(CDataPacketReader &inPacket);//跨服的排行榜保存数据
	 
	//////////////////////////////////////////////////////////////////////////
	// 关系相关的DB请求处理
	//////////////////////////////////////////////////////////////////////////
	void QueryRelationData(unsigned int nActorId, int nRawServerId, int nLoginServerId);
	void SaveRelation(CDataPacketReader &inPacket);
	
	//////////////////////////////////////////////////////////////////////////
	// 战力排行榜
	//////////////////////////////////////////////////////////////////////////
	VOID LoadActorCombatRank(CDataPacketReader &inPacket);
	VOID LoadActorCombatGameInfo(CDataPacketReader &inPacket);
	VOID UpdateActorCombatAward(CDataPacketReader &inPacket);
	VOID SaveActorCombatGameInfo(CDataPacketReader &inPacket);
	VOID LoadCombatRecord(CDataPacketReader &inPacket);
	VOID SaveCombatRecordToDb(CDataPacketReader &inPacket);

	//////////////////////////////////////////////////////////////////////////
	// 职业宗师竞技
	//////////////////////////////////////////////////////////////////////////
	VOID LoadActorCombatBaseInfo(CDataPacketReader &inPacket);
	VOID SaveActorCombatBaseInfo(CDataPacketReader &inPacket);
	VOID LoadActorCombatLog(CDataPacketReader &inPacket);
	VOID SaveActorCombatLog(CDataPacketReader &inPacket);

	//////////////////////////////////////////////////////////////////////////
	// 好友等
	//////////////////////////////////////////////////////////////////////////
	VOID LoadActorFriendsData(CDataPacketReader &inPacket);
	VOID LoadActorFriendsData(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	VOID SaveActorFriendsData(CDataPacketReader &inPacket);

	VOID LoadActorDeathData(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	VOID LoadActorDeathData(CDataPacketReader &inPacket);
	VOID SaveActorDeathData(CDataPacketReader &inPacket);

	VOID LoadActorDeathDropData(CDataPacketReader &inPacket);
	VOID SaveActorDeathDropData(CDataPacketReader &inPacket);

	VOID DeleteActorFriendData(CDataPacketReader &inPacket);
	VOID AddFriendChatMsg(CDataPacketReader &inPacket);
	VOID LoadFriendChatMsgData(CDataPacketReader &inPacket);
	VOID LoadFiendOfflineData(CDataPacketReader &inPacket);
	//后台操作
	VOID AddFilterWordsDbMsg(CDataPacketReader &inPacket);
	VOID BackRemoveItem(CDataPacketReader &inPacket);
	VOID BackRemoveMoney(CDataPacketReader &inPacket);

	VOID LoadTopLevelActorData(CDataPacketReader &inPacket);
	VOID LoadTopLevelActorOneData(CDataPacketReader& inPacket);//世界等级查询
	VOID LoadActorConsumeRank(CDataPacketReader &inPacket);
	VOID LoadActorTopicBattle(CDataPacketReader &inPacket);
	VOID LoadActorSwingLevelData(CDataPacketReader &inPacket);
	VOID LoadActorHeroTopicRankData(CDataPacketReader &inPacket);
	VOID LoadAllActivityData(CDataPacketReader &inPacket);

	VOID LoadCombineLevelActorData(CDataPacketReader &inPacket);
	VOID LoadCombineBattleActorData(CDataPacketReader &inPacket);

	VOID LoadActorGameSetData(CDataPacketReader &inPacket);

	VOID SaveActorGameSetData(CDataPacketReader &inPacket);

	//////////////////////////////////////////////////////////////////////////
	// 邮件
	//////////////////////////////////////////////////////////////////////////
	void QueryActorMail(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void AddActorOfflineMail(CDataPacketReader & inPacket);
	void SaveActorMail(CDataPacketReader & inPacket, bool bResp = true);
	void LoadActorMail(CDataPacketReader & inPacket);
	void LoadNewServerMail(int nServerId, int nRawServerId, int nMaxServerMailId);
	void AddNewServerMail(CDataPacketReader & inPacket);
	void SaveNewServerMail(CDataPacketReader & inPacket);
	VOID LoadActorNewTitleData(CDataPacketReader &inPacket);
	VOID SaveActorNewTitleData(CDataPacketReader &inPacket);
	VOID LoadActorCustomTitleData(CDataPacketReader &inPacket);
	VOID SaveActorCustomTitleData(CDataPacketReader &inPacket);
	
	//寄卖
	void SaveActorConsignment(CDataPacketReader &inPacket);//保存玩家的
	void SaveConsignmentItem(CDataPacketReader &inPacket);//保存系统的，分包
	//void SaveConsignmentIncome(CDataPacketReader &inPacket);
	void LoadConsignmentItem(CDataPacketReader &inPacket);
	void LoadConsignmentIncome(CDataPacketReader &inPacket);

	//充值
	void LoadFee(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);
	void DelFee(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId);
	//称号
	void QueryActorNewTitle(unsigned int nActorId, int nRawServerId, int nServerId);
	void QueryActorCustomTitle(unsigned int nActorId, int nRawServerId, int nServerId);
	void QueryActorWarChariot(unsigned int nActorId, int nRawServerId, int nServerId);
private:
	//////////////////////////////////////////////////////////////////////////
	// 申请和投递消息数据包
	//////////////////////////////////////////////////////////////////////////
	/*
	* Comments: 想宿主申请一个数据包，用于发送处理结果
	* Param const jxSrvDef::INTERSRVCMD nCmd:
	* @Return CDataPacket&:
	* @Remark:
	*/
	CDataPacket& allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd);

	/*
	* Comments: 投递数据包到宿主发送队列
	* Param CDataPacket & packet:
	* @Return void:
	* @Remark:
	*/
	void flushProtoPacket(CDataPacket &packet);

	//////////////////////////////////////////////////////////////////////////
	// 处理DB请求的内部实现函数
	//////////////////////////////////////////////////////////////////////////
	bool QueryOfflineUserBasicInfo(unsigned int nActorId, int nRawServerId, int nLoginServerId);
	void QueryOfflineHeroInfo(unsigned int nActorId, unsigned int nHeroId, int nRawServerId, int nLoginServerId);
	void QueryOfflineHeroEquips(unsigned int nActorID,unsigned int nHeroId, int nRawServerId, int nLoginServerId);
	void QueryEquipedGemData(unsigned int nActorID, int nRawServerId, int nLoginServerId);
	int SaveGoingQuest(unsigned int nActorid, int nTaskId, int id, int value);
	int CleanQuestData(unsigned int nActorId);
	int SaveRepeatQuest(unsigned int nActorid,int nTaskId, int qtime);
	void QueryQuestFromDB(unsigned int nActorId, int nRawServerId, int nLoginServerId);
	int GetGoingQuest(unsigned int nActorid, CDataPacket& out);

	int GetRepeatQuest(unsigned int nActorid, CDataPacket& out);
	
	void LoadFriends(unsigned int nActorId);
	/*
	* Comments:创建一个帮派成功后，调用本函数向逻辑服务器发送结果
	* Param int nErr:错误码
	* Param int index:消息在队列中的索引
	* Param Uint64 hHandle:创建这个帮派的角色的handle，以方便向他发送结果提示语
	* Param int nGid:创建成功的话，这个是帮派的id号
	* Param LPCSTR sGuildName:帮派名称
	* Param LPCSTR sFoundName:创始人名称
	* Param BYTE nZY:阵营编号
	* @Return VOID:
	*/
	void SendAddGuildResult(int nRawServerId, 
							int nLoginServerId,
							int nErr,
						    int index,
							Uint64 hHandle,
							unsigned int nGid,
							LPCSTR sGuildName,
							LPCSTR sFoundName,
							BYTE nZY,
							INT_PTR nIcon);

	


private:
	CSQLConenction			*m_pSQLConnection;
	ILogicDBRequestHost		*m_pHost;
	CBufferAllocator		m_Allocator;			// 内存池
	PCHAR					m_pHugeSQLBuffer;		// 大型SQL查询语句缓冲，长度为dwHugeSQLBufferSize
	
	static const SIZE_T dwHugeSQLBufferSize = 1024 * 1024 * 32;
};
	
#endif
