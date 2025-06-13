#pragma once

#define		MAXMEMOCOUNT		1024
#define		MAX_WAR_HISTORY		100
#define		MAXGGUILDCOIN		2000000000		//行会资金的上限 

#define  GUILD_UPGRADE_ITEM_IDX_1	1			//行会升级道具1
#define  GUILD_UPGRADE_ITEM_IDX_2	2			//行会升级道具2
#define  GUILD_UPGRADE_ITEM_IDX_3	3			//行会升级道具3


#define GUILDTREE_PERIOD_1  1   //幼苗期
#define GUILDTREE_PERIOD_2  2   //成熟期
#define GUILDTREE_PERIOD_3  3   //开花期
#define GUILDTREE_PERIOD_4  4   //结果期

#define GUILDFRUIT_PERIOD_1  1   //1级果实
#define GUILDFRUIT_PERIOD_2  2   //2级果实
#define GUILDFRUIT_PERIOD_3  3   //3级果实
#define GUILDFRUIT_PERIOD_4  4   //4级果实
#define GUILDFRUIT_PERIOD_5  5   //5级果实
#define GUILDFRUIT_PERIOD_6  6   //6级果实

//行会关系
#define GUILD_RELATION_NORMAL	0	//普通关系
#define GUILD_RELATION_UNION	1	//联盟关系
#define GUILD_RELATION_ENEMY	2	//敌对关系

//行会宣战关系（坑爹）
#define GUILD_WARSHIP_NO	0		//非宣战关系（包括联盟和普通关系）
#define GUILD_WARSHIP_YES	1		//宣战关系


using namespace jxSrvDef;

enum tagUpGuildSkillType
{
	uGuildLevel = 1,		///< 帮派等级
	uGuildCont	= 2,		///< 帮派资金
	uGuildYs	= 3,		///< 帮派玥石
};

enum GuildBuildType{
	gbtMainHall = 1,	///< 主殿
	gbExerciserooem,    //练功房
	gbAssemblyhall,     //议事厅
	gbtGuildShop,		///< 行会商店
	gbtMoonBox,			///< 月光宝盒
	gbtGuildBoss,		///< 封印兽碑
	//5行会仓库
	//6行会光环
	gbtTech = 7,		///< 行会科技
	//8行会熔炉
	gbtDragonThunder = 9,///< 震天龙弹

	gbtEnd,	///< 结束
	gbtCount = gbtEnd - 1,	
};

typedef struct tagGuildBuild
{
	int nType;
	int nLevel;
	tagGuildBuild()
	{
		nType = 0;
		nLevel = 0;
	}
}TAGGUILDBUILD, *PTAGGUILDBUILD;

//行会活动
typedef struct tagGuildActive
{
	unsigned char	nGuildActSts;				//活动状态，0-未开启，1-进行中，2-进行完毕
	unsigned int	nGuildActHandle;
	unsigned char	nFreshMonIdx;				//第N波刷怪，从1开始
	unsigned int	nActTimerHandle;			//活动timer句柄
	unsigned int	nFreshTimerHandle;			//刷新timer句柄
	int				nLevelWhenActive;			//活动开启时行会的等级(以防活动过程中行会升级)
	tagGuildActive()
	{
		memset(this,0,sizeof(*this));
	}
}GuildActive;


typedef CHandleList<ActorCommonNode,UINT>			ActorNodeList;


typedef struct tagApplyList
{
	unsigned int nActorId;	// 玩家id
	ACTORNAME	 szName;	//玩家昵称
	int          nLevel;    //玩家等级
	int          nCircle;   //玩家转数
	int          nSex;      //性别
	int          nJob;      //职业
	unsigned int          nVip;      //职业
	tagApplyList()
	{
		memset(this, 0, sizeof(*this));
	}
}ApplyList;


class CGuildComponent;
class CJoinGuildApply;
class CGuild
{
public:
	enum tagRelationShipType
	{
		rsReadyWar = 0,
		rsOnWar,
		rsPeaceful,
	};

	enum tagInterMsgId
	{
		imInvite,		//邀请加入
		imJoin,			//申请加入
		imCall,			//召唤
	};

	typedef struct tagInterMsg
	{
		BYTE			nMsgId;				//消息的类型
		EntityHandle	hActorHandle;		//目标的实体handle
		ACTORNAME		szName;				//目标的名称
		EntityHandle	hSrcHandle;			//发起人的handle，通常是召唤的时候用到
		ActorCommonNode Node;
		tagInterMsg()
		{
			hActorHandle = 0;
			hSrcHandle = 0;
			szName[0] = 0;
		}
	}InterMsg;

	typedef CHandleList<InterMsg,UINT> GuildInterMsgList;
	
public:
	/*
	* Comments:与某个帮派结束斗争（恢复和平）后的处理，通常是计算贡献值，战魂等
	* Param INT_PTR nGid:
	* @Return VOID:
	*/
	//VOID EndWar(INT_PTR nGid);
	//取得对外的公告信息
	inline LPCSTR GetOutMemo() {return m_sOutMemo;}
	//取得对内的公告信息
	inline LPCSTR GetInMemo()	{return m_sInMemo;}
	inline LPCSTR GetGroupMemo()	{return m_sGroupMemo;}

	//加载沙巴克成员信息
	void LoadSbkOffLinedata();
	void getsbkOfflineData(CDataPacket& data, int& nCount);

	/*
	取得行会等级，目前行会主殿的等级就是行会等级，不要直接使用m_nLevel
	*/
	inline int GetLevel() {  return GetGuildbuildingLev(gbtMainHall); /*return m_nLevel;*/}
	//取得帮派繁荣度
	inline int GetFr() {return m_nFanrong;}

	//获得帮派资金
	inline int GetCoin() {return m_GuildCoin;}

	//获得帮派玥石
	inline int GetGuildYs() {return m_GuildYs;}

	//修改帮派玥石
	void ChangeGuildYs(int value) {m_GuildYs += value;}

	//设置帮派资金
	inline void SetCoin(int newCoin) 
	{
		m_boUpdateTime = true;

		if(newCoin > MAXGGUILDCOIN)
		{
			newCoin = MAXGGUILDCOIN;
		}

		if(newCoin > 0)
			m_GuildCoin=newCoin;
		else
			m_GuildCoin = 0;

		NotifyGuildDonateData();	//发送给行会成员
	}

	void ChangeGuildCoin(int value, INT_PTR nLogId = 0,LPCTSTR pStr = "");
	



	//设置帮派玥石
	inline void SetGuildYs(int value) {m_GuildYs += value;}

	//设置对外的公告，boSave为true，表示稍后会保存到数据库中
	VOID SetOutMemo(LPCSTR sOut,bool boSave);
	//设置对内的公告，boSave为true，表示稍后会保存到数据库中
	VOID SetInMemo(LPCSTR sIn,bool boSave) ;
	//设置行会群公告
	VOID SetGuildGroupMemo(LPCSTR sIn,bool boSave);
	//设置帮派等级
	VOID SetGuildLevel(int value,bool boSave,bool boRank, bool boUpgrade) ;
	//设置繁荣度
	VOID SetFr(int value,bool boSave,bool boRank);

	void SetGuildFr(int value)
	{
		int gFr = GetFr();
		SetFr(gFr+value,TRUE,true);
	}

	//帮派销毁
	void Destroy();

	/*
	* Comments:给帮派的在线成员广播信息，实际上是调用每个CActor的SendTipmsgFormatWithId函数实现发送信息
	* Param INT_PTR nPos:大于等于这个职位的人才可以收到
	* Param INT_PTR nTipmsgID:参数参考CActor的SendTipmsgFormatWithId函数
	* Param INT_PTR nType:
	* Param ...:
	* @Return VOID:
	*/
	VOID SendGuildMsg(INT_PTR nPos,INT_PTR nTipmsgID,INT_PTR nType = ttTipmsgWindow, ...);

	/*
	* Comments:帮派的在线成员广播信息
	* Param INT_PTR nPos:大于等于这个职位的人才可以收到
	* Param LPCSTR sMsg:
	* Param INT_PTR nType:
	* @Return VOID:
	*/
	VOID SendGuildMsg(INT_PTR nPos,LPCSTR sMsg,INT_PTR nType = ttTipmsgWindow);
	/*
	* Comments:帮派的在线成员行会聊天信息
	* Param CActor* pActor
	* Param LPCSTR sMsg:
	*/
	VOID SendGuildChatMsg(CActor* pActor, char* msg);

	/*
	* Comments:给所有人成就
	* Param INT_PTR nEventId:成就id
	* @Return VOID:
	*/
	VOID GiveAchieve(INT_PTR nEventId,INT_PTR nParam1);

	/*
	* Comments: 向所有帮派玩家广播
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* @Return void:
	*/
	void BroadCast(LPCVOID pData,SIZE_T size, int nLevel = 0, int nCircle = 0);

	void BroadCastExceptSelf(LPCVOID pData,SIZE_T size,unsigned int nMyId);

	//向所有帮派玩家广播一个消息
	void BroadCastByMsgId(int nMsgId);
	/*
	* Comments: 帮派群聊天
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* @Return void:
	*/
	void GuildGroupBroadCast(LPCVOID pData,SIZE_T size);

	//向行会频道广播消息
	void BroadCastMsgInGuildChannel(char* szMsg, int nLevel = 0, int nCircle = 0);

	/*
	* Comments: 向帮会内所有在线玩家广播数据的兼容接口
	* Param LPCVOID lpData:
	* Param SIZE_T dwSize:
	* @Return void:
	*/
	inline void SendData(LPCVOID lpData, SIZE_T dwSize)
	{
		BroadCast(lpData, dwSize);
	}

	/*
	* Comments:提醒帮派成员的客户端更新帮派信息
	* @Return void:
	*/
	void NotifyUpdateGuildInfo();

	void NotifyGuildSimpleInfo();

	void NotifyGuildDonateData();

	void NotifyGuildTreeBackCoin();

	//提醒行会成员的客户端更新仓库
	void NotifyUpdateDepot();

	//提醒行会弹劾发生
	void NotifyImpeachMember(LPCTSTR strPosName, ACTORNAME strOldActorName, ACTORNAME strNewActorName);

	//发送求救信息
	void SendActorHelp(LPCSTR nName,int nSceneId,int x,int y);

	/*
	* Comments:提醒帮派成员的客户端更新帮派信息
	* Param INT_PTR sType: 类型
	* Param INT_PTR sCount: 需要的量
	* Param bool sConsume: 是否需要扣除
	* @Return void:
	*/
	bool BcanUpGuildSkillLevel(CActor* pActor,int sType,int sCount,bool sConsume);

	/*
	* Comments:扣除帮派技能升级所需要的
	* Param INT_PTR sType: 类型
	* Param INT_PTR sCount: 需要的量
	* Param bool sConsume: 是否需要扣除
	* @Return void:
	*/
	bool DecountUpGuildSkill(int sType,int sCount,bool sConsume);

	//保存帮会技能信息到数据库
	void SaveDbGuildSkill(CActor* pActor,int skillId,int skillLevel);

	//设置挑战副本的句柄
	void SetBossChanllengeHandle(unsigned int bHandle){m_BossChanllengeHandle = bHandle;}

	//获得帮派副本句柄
	unsigned int GeBossChanllengeHandle() {return m_BossChanllengeHandle;}

	//设置行会镖车的句柄
	void SetGuildDartHandle(double nHandle) {m_GuildDartHandle = nHandle;}

	//返回行会镖车的句柄
	double GetGuildDartHandle() {return m_GuildDartHandle;}

	//设置行会镖车的到期时间
	void SetGuildExipredTime(int nValue) {m_GuildBiaoCheExpriedTime = nValue;}
public:
	CGuild();
	~CGuild();
	/*
	* Comments:给帮派增加一个成员,加入到列表中
	* Param CActor * pActor:如果pActor为NULL，表示当前不在线
	* Param nType:玩家地位,默认是帮众
	* @Return VOID:
	*/
	VOID AddMemberToList(CActor* pActor,INT_PTR nGuildPos=smGuildCommon);

	//处理未在线申请添加成员
	VOID AddMemberToList(ApplyList* pApply,INT_PTR nGuildPos = smGuildCommon);

	void SetMemberOnline(CActor* pActor);

	VOID InitGuild(CActor* pActor);

	//获得帮派成员的数量
	inline INT_PTR GetMemberCount()
	{
		return m_ActorOffLine.size();
	}

	//设置帮派成员的阵营（某场景之中才算）
	void SetMemberCamp(int nCampId, int nScendId);

	/*
	* Comments:给全帮派的在线玩家加buff，
	* Param nScenceId:在某场景加，0-任意场景
	* @Return void:
	*/
	void AddMemberBuffById(int nBuffId, int nScenceId=0);

	void DelMemberBuffById(int nBuffId);
	/*
	* Comments:获得帮派的人数上限
	* @Return int INT_PTR:
	*/
	int GetMaxMemberCount();
	/*
	* Comments:根据帮派的级别，得到可以有的最大的职务的数量,升职和降职不能使人数超过这个
	* @Return INT_PTR:
	*/
	INT_PTR MaxGuildPosCount(int nGuildPos);

	/*
	* Comments:根据帮派的级别，得到可以有的最大的堂主的数量,升职和降职不能使人数超过这个
	* @Return int INT_PTR:
	*/
	int GetMaxTkCount();
	/*
	* Comments:保存帮派的信息
	* @Return VOID:
	*/
	VOID Save();
	/*
	* Comments:保存帮派个人信息
	* @Return VOID:
	*/
	VOID SaveGuildMember(ActorCommonNode& actor);
	/*
	* Comments:添加一个帮派的内部消息到队列中
	* Param InterMsg Msg:
	* @Return BOOL:TRUE表示加入成功，否则表示已有本消息
	*/
	BOOL AddInterMsg(InterMsg Msg);

	/*
	* Comments:查看队列中是否有发给我的信息，如果有，删掉
	* Param InterMsg Msg:
	* @Return BOOL:TRUE表示有，否则表示无
	*/
	BOOL FindAndDeleteInterMsg(InterMsg& Msg);

	BOOL FindAndInterMsg(InterMsg& Msg);

	/*
	* Comments:在ActorOffLine中查找到这个角色，并设置它的handle
	* Param nActorid:角色的id
	* Param EntityHandle hHandle:这个角色的handle
	* @Return VOID:
	*/
	VOID FindAndSetHandle(unsigned int nActorid,EntityHandle hHandle);

	/*
	* Comments:根据角色id查找成员节点，这个性能不高，可以优化，ActorOffLine列表采用排序存储
	* Param INT_PTR nActor:
	* @Return ActorCommonNode*:
	*/
	ActorCommonNode* FindMember(unsigned int nActorid);

	//根据姓名查找
	ActorCommonNode* FindMemberByName(const char* nName);

	//根据角色id获得角色名称
	char* FindGuildMemberName(unsigned int nActorid);
	/*
	* Comments:把用户数据同步到ActorCommonNode中，因为如果用户在线，那处理的时候是直接用在线的数据，如果用户不在线
	* 就是读取ActorCommonNode里面的数据了。
	* 通常就是在用户下线后同步这个数据
	* Param CActor * pActor:
	* Param ActorCommonNode * pNode:
	* @Return VOID:
	*/
	VOID UpdateActorData(CActor* pActor,ActorCommonNode* pNode);


	VOID UpdateActorData(CJoinGuildApply* pApply,ActorCommonNode* pNode);

	void AddMember(ActorCommonNode& pNode);

/*
	* Comments:把用户数据同步到ActorCommonNode中，因为如果用户在线，那处理的时候是直接用在线的数据，如果用户不在线
	* 就是读取ActorCommonNode里面的数据了。
	* Param ApplyList * pActor:
	* Param ActorCommonNode * pNode:
	* @Return VOID:
	*/
	VOID UpdateActorData(ApplyList* pApply,ActorCommonNode* pNode);

	/*
	* Comments:处理数据
	* Param int nGid:帮派id
	* Param int nactorId:玩家id
	* Param int type:类型 1 插入 2删除 3 全部删除
	* @Return VOID:
	*/
	VOID DealGuildApply(unsigned int nGid, unsigned int nactorId, int type);

	/*
	* Comments:删除会员
	* Param nActorid：操作人的id
	* Param CActor * pActor:在线成员，如果他此刻不在线，pActor为NULL
	* Param ActorCommonNode * pNode:pActor为NULL，这个不能为空
	* Param boType:是否给开除的，默认是TRUE，如果是FALSE，表示是自己脱离的,要设置nZZNext(在职时间）以限制他不能在规定时间内再加入其他帮派,或接受邀请
	* @Return VOID:
	*/
	VOID DeleteMember(unsigned int nActorid,CActor* pActor,ActorCommonNode* pNode,BOOL boType);

	/*
	* Comments:在在线列表中删除指定的玩家，做这个操作通常是玩家下线，或者玩家被开除
	* Param EntityHandle & hHandle:
	* @Return VOID:
	*/
	VOID DeleteOnLineList(EntityHandle& hHandle);

	/*
	* Comments:增加一个敌对帮派
	* Param int nGuildId:敌对帮派的id
	* @Return VOID:
	*/
	VOID AddWarGuild(unsigned int nGuildId,CGuild* pGuild);

	/*
	* Comments:将同盟也添加为宣战对象
	* Param int nGuildId:敌对帮派的id
	* @Return VOID:
	*/
	VOID AddUnionWarGuild(unsigned int nGuildId,CGuild* pGuild);


	/*
	*/
	TAGGUILDBUILD*  GetBuildByType(int type) { return NULL;}
	
	/*
	* Comments:获取本帮派与对方帮派的宣战关系
	* Param int nOtherId:对方帮派的id
	* @Return int:返回帮派关系，见tagWarStatus的定义
	*/
	int GetRelationShipInWar(unsigned int nGuildId);

	//获取下次可以宣战的时间
	int GetCanWarNextTime(unsigned int nGuildId);

	/*
	* Comments:下发敌对帮派列表
	* Param CActor * pActor:
	* @Return VOID:
	*/
	VOID SendWarGuildList(CActor* pActor);

	/*
	* Comments:发送帮派列表给所有成员，这个通常是在帮派站列表改变的时候
	* @Return VOID:
	*/
	VOID SendWarGuildListToAllMember();

	/*
	* Comments:更新 在线成员 行会等级 属性
	* @Return VOID:
	*/
	VOID UpdateGuildMemberGuildLevelProperty();

	//更新名称显示
	void UpdateActorShowName();

	/*
	* Comments:发送帮派更改的升级的帮派技能给所有成员
	* @Return VOID:
	*/
	void SendGuildSkillToAllMember(int skillId,int skillLevel);

	/*
	* Comments:下发现在宣战、敌对状态的帮派列表,这个函数跟SendWarGuildList比较接近
	* @Return VOID:
	*/
	VOID SendWarList(CActor* pActor);

	/*
	* Comments:请求帮派战历史记录
	* @Return VOID:
	*/
	VOID SendWarHistory(CActor* pActor);

	/*
	* Comments:检查有没有帮派关系变化，如果有，需要下发信息给客户端，同时需要保存到数据库
	* @Return VOID:
	*/
	VOID CheckWarList();

	/*
	* Comments:击杀敌对帮派的玩家后，增加PK的数目
	* Param INT_PTR nGid:敌对帮派的玩家
	* Param INT_PTR nCount:数量,默认是加1
	* @Return VOID:
	*/
	VOID AddPkCount(INT_PTR nGid,INT_PTR nCount = 1);

	/*
	* Comments:增加帮派战死亡的数目
	* Param INT_PTR nGid:对敌帮派id
	* Param INT_PTR nCount:死亡数目
	* @Return VOID:
	*/
	VOID AddDieCount(INT_PTR nGid, INT_PTR nCount = 1);

	/*
	* Comments:增加帮派战死亡的数目
	* Param int nTime:发生的时间
	* Param char* nRecord:发生的事件
	* @Return VOID:
	*/
	void AddEventRecord(char* nRecord,int nEventId,int nParam1,int nParam2,int nParam3,char* nParam4,char* nParam5);

	//保存到数据库
	void SaveGuildEventtoDb(int Index,int nType);

	void SaveAllGuildEvent();

	//保存仓库物品到数据库
	void SaveGuildDepoItem();

	//沙巴克战时更新成员颜色
	void UpdateGuildActorNameInSbk();

	//保存仓库数据
	void SaveDepotData();

	//保存仓库记录
	void SaveGuildDepotRecord();

	//组装帮派消息
	void BuildUpEventRecord(int nIndex,int nEventId,int nParam1,int nParam2,int nParam3,char* szParam4,char* szParam5);

	//获得职位名称
	char* GetGuiPosName(int posType);

	//获取帮派名称
	inline LPCTSTR GetGuildName(){return m_sGuildname;}

	//获取帮派id
	inline unsigned int GetGuildId(){return m_nGid;}
	//根据类型获取名字
	char* GetCoinname(int nType);

	//根据仓库记录的索引获取记录
	char* GetDepotRecordMsg(int nIndex,char* sText);

	//获取召唤斗魂的标志
	bool GetUseGuildBoss() {return m_UseGuildBoss;}

	//设置召唤斗魂的标志
	void SetUseGuildBoss(bool value) {m_UseGuildBoss = value;}

	//帮派成员离开或加入
	void SendAddOrLeftMember(CActor* pActor,BYTE nType,unsigned int nActorId,char* nName);

	//设置语音频道id
	void SetYyType(int nYtype,bool bSave = true)
	{
		if(bSave == true)
			m_boUpdateTime= bSave;

		m_sYyType = nYtype;
	}

	inline int GetYType() {return m_sYyType;}

	//设置语音频道宣言
	void SetYyMemo(LPCSTR sIn,bool boSave)
	{
		_asncpytA(m_sYyMemo,sIn);
		if (boSave)
			m_boUpdateTime = boSave;
	}
	inline LPCSTR GetNextUpdateInmemo() {return m_sNextUpdateInMemo;}
	void SetNextUpdateInmemo(LPCSTR sIn)
	{
		_asncpytA(m_sNextUpdateInMemo, sIn);
	}

	void SetYyGridCompere(LPCSTR sGridName)
	{
		_asncpytA(m_sYyGrilCompere, sGridName);
	}

	inline LPCSTR GetYyGridCompere()
	{
		return m_sYyGrilCompere;
	}

	inline LPCSTR GetYyMemo() {return m_sYyMemo;}

	//gm改变帮主
	void GmChangeLeader(const char* nName);

	void SetCertification(int nCertification)
	{
		m_nCertification = nCertification;
		m_boUpdateTime = true;
	}
	int GetCertification()
	{
		return m_nCertification;
	}
	inline void SetQqGroupId(LPCSTR nqGroupId,bool boSave)
	{
		_asncpytA(m_sQqGroupId,nqGroupId);
		if (boSave)
			m_boUpdateTime = boSave;
	}

	inline LPCSTR GetQqGroupId() {return m_sQqGroupId;}

	inline void SetYyGroupId(LPCSTR strYGroupId,bool boSave)
	{
		_asncpytA(m_sYyGroupId,strYGroupId);
		if (boSave)
			m_boUpdateTime = boSave;
	}

	inline LPCSTR GetYyGroupId() {return m_sYyGroupId;}

	//根据行会id获取关系
	int GetRelationShipForId(unsigned int nGuildId);
	//根据行会id设置关系
	void SetRelationShipWithId(unsigned int nGuildId,int nType);

	//发送给行会有其他行会请求联盟
	bool SendWarUnionMsgToActor(unsigned int nGuildId,unsigned int nActorId,LPCSTR nGuildName);
	//客户端收到这个消息，更新行会列表
	void UpdateGuildList();

	//有人申请加入行会
	void SendActorApplyToGuild();

	//发送打开行会集结令的面板
	bool SendOpenBuildFormToClient(CActor* pActor);

	//行会开启buff
	void AddGuildBuff(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay =false, void *pGiver=0, int param = 0);

	//沙巴克城主开启buff
	void SbkMasterOpenBuff(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay =false, void *pGiver=0, int param = 0);

	//给盟友加buff
	void AddBuffToUnion(int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay =false, void *pGiver=0, int param = 0);

	/* 通过物品系列号查找用户物品对象
	 * boRemove	参数用于规定在找到物品后是否将物品从容器中移除（只是移除，并不会销毁）；
	 * @return	返回第一个匹配的用户物品指针
	*/
	int FindItemIndex(const CUserItem::ItemSeries series);

	//群发封号列表
	void SendTitleToAll();

	//群发行会建筑等级的变化
	void OnSendGuildBuildingChange(int nIndex,int nLevel);
	//发送行会日志
	void SendGuildEventLog(CActor* pActor);

	//下发行会挑战的结果
	void SendGuildChanllengeResult(unsigned int nActorId,int nType,int nResult);


	//有封号改变群发
	void SendChangeTitleToAll(unsigned int nActorId,BYTE nIndex);

	//是否正在宣战等
	bool OnGuildWar();

	//获取是否开启活动勇闯毒潭状态
	int GetToxicFreshTimes() { return m_ToxicFreshTimes;}

	//设置开启勇闯毒潭状态
	void SetToxicFreshTimes(int nTimes) {m_ToxicFreshTimes = nTimes;}

	//设置在启勇闯毒潭中击杀的个数
	void SetkilToxicNum(int nValue) {m_KillToxicNum = nValue;}

	//获取在启勇闯毒潭中击杀的个数
	int GetkillToxicNum() {return m_KillToxicNum;}

	//获取行会仓库的物品数量
	int GetGuildDepotCount();

	//获取行会仓库的最大页数
	int GetGuildDepotPageMax();

	int GetGuildDepotGridsCount()
	{
		return GetGuildDepotPageMax() * GUILDDEPOT_PAGE_GRIDS;
	}

	//处理等待审核的兑换请求
	void ProcDepotItemInExchangeCheck();

public:		//沙城战
	//更新沙巴克的头衔
	void UpdateSbkGuildTitle(bool bFlag, bool boChangeNewTitle = true);

	//处理sbk 成就
	void UpdateSbkGuidAchieve();



	//【影魅合服】发送合服活动连续占领三次的行会奖励
	void SendCombineSbk3Award();

	/*获取行会的职位人数（实时计算）
	nGuildPos:1-官员，2-副帮主
	*/
	int GetGuildPosCount(int nGuildPos);

	//添加到行会贡献的列表里
	void AddToGuildGxList(unsigned int nActorId,int nGx);

	//获取贡献排名
	int GetGuildGxIndex(unsigned int nActorId);

	//获取行会已挑战的次数
	int GetGuildChallengeTimes() {return m_GuildChangleTimes;}

	//设置行会挑战的次数
	void SetGuildChallengeTimes(int nTimes) {m_boUpdateTime = true; m_GuildChangleTimes = nTimes;}

	//设置行会下次挑战的时间
	void SetGuildNextChallengeTime(int nTime) {m_boUpdateTime = true; m_ChanlgeLeftTimes = nTime;} 

	//获取行会下次挑战的时间
	int GetGuildNextChallengeTime() {return m_ChanlgeLeftTimes;}

	//设置行会封印兽碑的等级
	void SetGuildBossLevel(int nLevel) {m_boUpdateTime = true; m_GuildBossLevel = nLevel;}

	//获得行会封印兽碑
	int GetGuildBossLevel() {return m_GuildBossLevel;}

	/// 获取建筑的等级
	/// @param nType GuildBuildType
	int	GetGuildbuildingLev(int nType);

	//设置建筑的等级
	void ChangeGuildBuildingLev(int nType,int nValue);

	//获取建筑的剩余cd
	int GetGuildBuildingCdTime() {return m_UpLeftTime;}

	//设置建筑的剩余cd
	void SetGuildBuildingCdTime(int nTime);

	//下发行会建筑冷却时间
	void SendGuildBuildCdTime();

	//下发行会升级材料信息
	void SendGuildUpgradeItem();

	//计算建筑评分
	int CalcGuildBuildingSorce();


	//从在线中获取最高战力的那一个
	unsigned int GetTopBattleFromOnLineList();
	//自动切换行会掌门
	void AutoChangeLeader( unsigned int nLeaderId );
	
	/*弹劾掌门，不需要了
	void ImpeachLeader();
	*/

	void GetGuildMemberBasicData(unsigned int nActordId, int& nSex, int& nJob);

	char* GetGuildTitleName(int nGuildPos);

	void GetUpgradeItemCount(int& nItemNum1, int& nItemNum2, int& nItemNum3);
	void SetUpgradeItemCount(int nItemNum1, int nItemNum2, int nItemNum3);

	int GetAutoJoinLevel();
	void SetAutoJoinLevel(int nNewLevel);

	//每日数据处理
	void ProcDailyData();

	//添加申请
	int AddJoinApply(CActor* pActor);
	int DealApplyByActorId(unsigned int nActorId, int nResult);

	static void GetGuildEventLog(GUILDEVENTRECOUD &stEventRecord, int nGuildEventId, int nParam1, int nParam2, int nParam3, 
		char* szParam4, char* szParam5);

	bool CheckIsApply(unsigned int nActorId)
	{
		std::map<unsigned int, ApplyList>::iterator it = m_ApplyList.find(nActorId);
		if(it != m_ApplyList.end())
		{
			return true;
		}
		return false;
	}
	//申请列表大小
	int ApplySize(){ return m_ApplyList.size();}

	void NotifyGuildWarList();

	void AfterDonateDealGuild();
	bool static decrease(const ActorCommonNode &FirstActor, const ActorCommonNode &SecondActor);

	void SetCallMemInfo(unsigned int nActorId, int nSceneId, int nX, int nY, unsigned int nExpireTime)
	{
		m_CallInfo = std::make_tuple(nActorId, nSceneId, nX, nY, nExpireTime);
	}

	std::tuple<unsigned int,int,int,int,unsigned int>
	GetCallMemInfo()
	{
		return m_CallInfo; 
	}

private:
	//行会升级
	void OnGuildLevelUp();
	void ClearGuildDepotItemExchange( unsigned int nActorId );
public:
	unsigned int					m_nGid;		//帮派的id	
	UINT							m_nRank;		//本帮派的排名
	char							m_sGuildname[32];//帮派的名字
	// ACTORNAME						m_sFoundname;//创始人的名字
	ACTORNAME						m_sLeaderName; //会长昵称
	unsigned int                    m_nLeaderId; //会长id  用这个id 
	ACTORNAME						m_sQqGroupId;	//qq群id
	ACTORNAME						m_sYyGroupId;	//语音频道id
	ACTORNAME						m_sYyGrilCompere;	//美女主播

	int								m_nBidCoin;		//竞价数量

	int								m_CreateTime;		//行会创建的时间

	// ActorCommonNode*				m_pLeader;			//帮主 ---不用了
	int								m_nSecLeaderCount;	//当前副帮主的个数
	int								m_GuildTkCount;		//堂主的个数
	int								m_GuildEliteCount;		//精英的个数

	int								m_AddMemberFlag;	//0 需要审核 1 自动添加

	//不在线的成员名字,这个列表的成员不会被删除
	//所有帮众的列表，如果上线了，hActorHandle会设置成新的handle，否则就是0
	// ActorNodeList					m_ActorOffLine;
	std::vector<ActorCommonNode>    m_ActorOffLine;
	//在线的成员
	CEntityList						m_ActorOnLine;

	GUILDEVENTRECOUD				m_EventRecord[MAX_EVENTCOUNT];		//记录帮派事件
	std::map<unsigned int,ApplyList>              m_ApplyList;                       //申请列表
	int								m_EventCount;						//当前记录的事件个数

	CVector<WarRelationData>		m_WarList;						//行会关系、状态帮派列表

	CVector<INTERRELATIONMSG>		m_UiounMsgList;					//请求为联盟的列表

	CVector<GUILDDEPOTITEM>			m_GuildDepotBag;				//行会仓库

	CVector<GUILDDEPOTMSG>			m_GuildDepotRecord;				//行会仓库操作记录

	CVector<GUILDMEMGXLIST>			m_GuildMemGxList;				//行会贡献排名列表
	CVector<TAGGUILDBUILD>         m_GuildBuildList;              //行会建筑

	bool							m_bSaveDepotRecord;				//是否需要保存行会仓库与仓库记录

	int								m_TileUpdateTime;				//封号修改的时间
	GUIlDTITLES						m_GuildTitles[7];				//封号修改


	int								m_SignUpFlag;					//今天是否报名 0 未报名 1 已报名
	int								m_nCertification;				//官方认证（拉人）
	int								m_ToxicFreshTimes;				//是否开启勇闯毒潭活动
	int								m_KillToxicNum;					//在勇闯毒潭中击杀怪物的个数
	//////////////////////////////////////////////////////
	
	//堂主的列表
	BYTE							m_nZytype;		//所属阵营
	//0 已解散，1正常
	bool							m_boStatus;
	bool							m_boUpdateTime;//更新数据的时间，如果0或者小于现在时间，表示无须更新
	
	CVector<WarGuildHistory>		m_WarListHistory;

	//以下是帮派内部的消息，比如邀请、申请加入等
	GuildInterMsgList				m_InterMsgList;

	CVector<GUILDSKILLS>			m_GuildSkillList;		//帮派等级的列表

	//因为下发给客户端的接口，需要返回申请加入的消息的个数，而队列中的消息是包含了申请加入的消息和邀请加入的消息，所以用这个变量专门来保存这个数量
	int								m_nJoinMsgCount;

	WORD							m_nIcon;
	
	bool							m_UseGuildBoss;						//是否正在召唤斗魂

	double					m_GuildDartHandle;					//行会镖车的句柄

	int								m_GuildBiaoCheExpriedTime;		//行会镖车到期时间 

	int								m_MainHallLev;					//主殿的等级
	int								m_ExerciseRoomlv;				//练功房
	int								m_AssemblyHallLv;				//议事厅

	int								m_GuildShopLev;					//行会商店等级
	int								m_GuildBossLevel;				//行会封印兽碑的等级
	int								m_MoonBoxLevel;					//月光宝盒的等级

	int								m_GuildChangleTimes;			//行会封印兽碑挑战次数
	int								m_ChanlgeLeftTimes;				//行会封印兽碑下次挑战的时间

	int								m_UpLeftTime;					//升级的剩余时间

	unsigned int					m_BossChanllengeHandle;			//挑战的封印兽碑的句柄

	BYTE							m_btTechLevel;					//行会科技等级
	BYTE							m_btDragonThunderLevel;			///< 震天龙弹等级
	WORD							m_nThunderPower;				///< 震天龙弹填充进度
	WORD							m_nFillThunderTimes;			///< 震天龙弹填充次数

	unsigned int					m_nCallTime;						//召集的时间
	unsigned int					m_nNewInMemoUpdateTime;			//下次更新m_sNextUpdateInMemo的时间
	int								m_nAutoJoinLevel;					//允许玩家自动加入帮派的等级
	int								m_nExchangeCheckCircleMin;			//需要兑换审核的最低转数
	//具体行会升级道具id见guild.txt
	int								m_nUpgradeItemNum1;	//行会升级道具1的数量
	int								m_nUpgradeItemNum2;	//行会升级道具2的数量
	int								m_nUpgradeItemNum3;	//行会升级道具3的数量
	std::tuple<unsigned int,int,int,int,unsigned int> m_CallInfo; //召唤者id，场景id，x, y, 过期时间

private:

	WORD							m_nLevel;		// 等级 这几个值改变的时候要设置nUpdateTime，以保存到数据库中
	int								m_GuildCoin;		//行会资金
	int								m_nDailyGuildCoinDonated;	//行会成员每天捐献的资金

	char							m_sInMemo[MAXMEMOCOUNT];		//内部公告信息
	char							m_sOutMemo[MAXMEMOCOUNT];		//外部公告信息
	char							m_sNextUpdateInMemo[MAXMEMOCOUNT];//下次更新内部公告消息
	char							m_sGroupMemo[MAXMEMOCOUNT];		//群公告

	int								m_sYyType;						//语音频道类型
	char							m_sYyMemo[MAXMEMOCOUNT];		//语音频道宣言

	//////////////////////////////////////////////////////////////////

	int								m_GuildYs;			//帮派玥石
	int								m_nFanrong;			// 繁荣度

//------------------------------行会神树-----------------------------------
public:
	void GetGuildTreeData(int &nTreeLevel, int &nTreeDegree)
	{
		nTreeLevel		= m_nGuildTreeLevel;
		nTreeDegree		= m_nGuildTreeDegree;
	}

	void SetGuildTreeData(int nTreeLevel, int nTreeDegree);

	void GetGuildFruitData(int &nFruitLevel, int &nFruitDegree, int &nFruitNum)
	{
		nFruitLevel		= m_nGuildFruitLevel;
		nFruitDegree	= m_nGuildFruitDegree;
		nFruitNum		= m_nGuildFruitNum;
	}

	void SetGuildFruitData(int nFruitLevel, int nFruitDegree, int nFruitNum);

	void ResetGuildTreeAndFruitByGuildLevel();

	//根据行会等级获取果实最低等级
	int GetGuildFruitLevelByGuildlevel();

	//根据果实等级设置果实数量
	int GetGuildFruitNumByFruitlevel(int nFruitLevel);

	unsigned int GetGuildTreeFruitTime()
	{
		return m_nGuildTreeFruitTime;
	}

	void SetGuildTreeFruitTime(unsigned int nFruitTime)
	{
		if( nFruitTime >= 0 )
		{
			m_nGuildTreeFruitTime = nFruitTime;
			m_boUpdateTime = true;
		}
	}

	//把神树设置为幼苗期
	void ProcGuildTreeToSapling(unsigned int nNow);
	//行会神树反馈行会资金
	void ProcGuildTreeBackCoin();

	void NotifyTreeAndFruitData(int nType);

	const GuildFruitLevel* GetGuildFruitLevel(int nFruitLevel);
public:
	int				m_nGuildTreeLevel;			//行会神树等级(不使用)
	int				m_nGuildTreeDegree;			//行会神树进度，即神树精华(不使用)
	int				m_nGuildFruitLevel;			//行会神树果实等级（品质）(不使用)
	int				m_nGuildFruitDegree;		//行会神树果实进度（亮灯）(不使用)
	int				m_nGuildFruitNum;			//行会神树果实进度数量(不使用)
	unsigned int	m_nGuildTreeFruitTime;		//行会神树进入成熟期的时间(不使用)
	unsigned int	m_nGuildTreeBackCoin;		//行会神树反馈的资金(不使用)
	
//------------------------行会任务-----------------------------------------
public:
	//行会任务
	void OnGuildTask(int nTaskType, int nTaskObjId, int nAddTaskSche, int nParam=-1);
	void RefreshGuildTask(bool bForce=false);
	void GetGuildTaskData(int &nTaskId, int &nTaskSche);
	
	int GetDailyGuildCoinDonated()
	{
		return m_nDailyGuildCoinDonated;
	}

	void SetDailyGuildCoinDonated(int nNewGuildCoin)
	{
		m_nDailyGuildCoinDonated = nNewGuildCoin;
		m_boUpdateTime = true;
	}

	void AddDailyGuildCoinDonated(int nAddGuildCoin)
	{
		m_nDailyGuildCoinDonated += nAddGuildCoin;
		m_boUpdateTime = true;
	}

	int GetGuildTaskObjId(int nTaskId);
	int GetGuildTaskObjNum(int nTaskId);

	void SetGuildTaskId(int nTaskId)
	{
		if( nTaskId > 0 )
		{
			m_nGuildTaskId = nTaskId;
		}
	}

	void SetGuildTaskSche(int nTaskSche)
	{
		if( nTaskSche > 0 )
		{
			m_nGuildTaskSche = nTaskSche;
		}
	}
private:
	short GetGuildTaskType(int nTaskId);

	const GuildTask* GetGuildTaskById(int nTaskId);
public:
	int		m_nGuildTaskId;					//行会任务ID
	int		m_nGuildTaskSche;				//行会任务进度

	//------------------------攻城战-----------------------------------------
public:
	void NotifyGuildSiegeCmdPanel(unsigned char nFlag);
	void SetGuildSiegeCommand(CActor* pActor, int nCmdIdx, int nCmdMsgIdx);

	void SendGuildSiegeCmdPanel(CActor* pActor, unsigned char nFlag);
	void SendGuildSiegeCommand(CActor* pActor);

	unsigned int GetGuildSiegeCommandTime()
	{
		return m_nGuildSiegeCmdTime;
	}
private:
	void NotifyGuildSiegeCommand();
private:
	int					m_nGuildSiegeCmdIdx;				//下发指令的
	int					m_nGuildSiegeCmdMsgIdx;				//下发指令的相关消息（随机一条）
	int					m_nGuildSiegeCmdSceneId;			//帮主下发指令时的场景
	int					m_nGuildSiegeCmdPosX;				//帮主下发指令时的x、y坐标
	int					m_nGuildSiegeCmdPosY;
	unsigned int		m_nGuildSiegeCmdTime;				//下发指令的时间
	DECLARE_OBJECT_COUNTER(CGuild)

public:
	//跨服相关
	void SendSbkGuildRankAward(int nRankId);
	void SendSbkGuildAward();
};
