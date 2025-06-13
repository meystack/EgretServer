#pragma once


class CGuild;

class CGuildSystem:
	public CEntitySubSystem<enGuildSystemID,CGuildSystem,CActor>
{
public:
	//系统的错误码定义
	enum tagGuildError
	{
		qeSucc =0 ,//成功
		qeNotFree,	//已在某个帮派中
		qeFree,		//还没加入帮派，不能解散帮派
		qeLevel,	//等级不够创建帮派
		qeNoMoney,	//不够金钱创建帮派
		qeTimeErr,	//在职时间不够，不能解散帮派或者升职（降职），都需要72小时
		qeNoRight,	//没有执行的权限
	};


public:

	virtual bool Initialize(void *data,SIZE_T size);

	//定时处理，清理召集令
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);

	/*
	* Comments:清除召集令
	* @Return VOID:
	*/
	VOID ClearZJ();

	/*
	* Comments:处理客户端的数据包b
	* Param INT_PTR nCmd:帮派系统的消息id
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//角色登陆退出都要通知帮派组件更新在线玩家
	/*
	* Comments:用户登陆时候的处理
	* @Return VOID:
	*/
	VOID OnLogin();
	//下线
	void OnActorLoginOut();

	//玩家登陆时提示宣战的行会
	void OnLoginDeclarNotice();

	/*
	* Comments:用户退出游戏时的处理
	* @Return VOID:
	*/
	VOID Destroy();

	/*
	* Comments:把角色的数据保存到数据库中
	* Param PACTORDBDATA pData:
	* @Return void:
	*/
	virtual void Save(PACTORDBDATA pData);

	/*
	* Comments:保存指定的帮派数据
	* Param int SaveGid:帮派id
	* Param int SaveActorid:角色id
	* Param INT64 SaveGx:贡献值
	* Param int typetk:堂口和职位的信息
	* Param UINT SaveZJ:召集令的清空时间
	* Param UINT SaveZZ:在职的时间
	* Param UINT nJoinTime:加入行会的时间
	* @Return VOID:
	*/
	VOID Save(unsigned int SaveGid,unsigned int SaveActorid,int SaveGx,int typetk,UINT SaveZJ,UINT SaveZZ,int nTitle,int nPosType,
		unsigned int nLoginTine, int nModleId, unsigned int nJoinTime);
	/*
	* Comments:申请增加一个帮派，本函数应该是由npc的脚本调用，本函数做条件判断能否创建
	* Param LPCSTR sGuildName:帮派的名称
	* @Return BOOL:允许创建返回0,否则返回错误码，函数会向名称服务器发送信息
	*/
	int CreateGuild(CDataPacketReader &packet);

	/*
	* Comments:解散帮派
	* @Return int:达到解散的条件返回0，否则返回错误码
	*/
	int DeleteGuild();

	/*
	* Comments:数据服务器返回创建结果的时候调用
	* Param INT_PTR nGid:创建的帮派id
	* Param INT_PTR nErrorCode:错误码，成功的话是0
	* Param LPCSTR sGuildName:帮派名称
	* @Return VOID:
	*/
	VOID OnCreateGuild(unsigned int nGid,int nErrorCode,CGuild* pGuild);

	/*
	* Comments:解散帮派返回结果的时候调用
	* @Return VOID:
	*/
	VOID OnDeleteGuild(int nErrorCode);

	/*
	* Comments:取得所在帮会的等级
	* @Return int:
	*/
	int GetGuildLevel();

	/*
	* Comments:取得所在帮会的ID
	* @Return unsigned int:
	*/
	unsigned int GetGuildId();

	/*
	* Comments:增加玩家的帮派贡献值，这里要加上参数记录日志,调用脚本
	* Param int nValue:增加的值
	* @Return int:
	*/
	//VOID AddGuildGx(int nValue);

	//改变繁荣
	void ChangeGuildFr(int value);
	//沙巴克信息
	void SendSbkInfo();

	//领取捐献后奖励
	void GetDonateReward();

	//捐献帮派资金
	void GuildDonate(BYTE nType);
	
	/*
	* Comments:给玩家发送本帮派的复杂信息数据，这数据显示在帮派窗口的首页
	* @Return VOID:
	*/
	VOID SendGuildInfoResult();

	/*
	发送行会管理界面信息
	*/

	void sendGuilBuilds();
	//发送行会宣战列表
	void SendGuildWarList();
	/*
	* Comments:给玩家发送本帮派的简单信息数据，这数据供高频调用
	* @Return VOID:
	*/
	VOID SendGuildInfoSimple();

	/*
	* Comments:给客户端发送成员的列表信息
	* @Return VOID:
	*/
	VOID SendMemberList(unsigned int nActorId=0);

	/*
	* Comments:发送本服内的所有帮派信息
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID SendAllGuildInfo(CDataPacketReader &packet);
	//发送行会日志
	void SendGuildEventLog();

	//加入行会
	void JoinToGuild(unsigned int nGuildId);

	/*
	* Comments:发送行会名片
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	void SendGuildCardByRank(int nRank);

	/*
	* Comments:发送帮派名片数据包
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID SendGuildCard(CDataPacketReader &packet);

	/*
	* Comments:处理数据服务器返回的内容
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & reader:
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	/*
	* Comments:获取本人在帮派的地位
	* @Return int:返回地位值，参看 tagSocialMaskDef的定义
	*/
	int GetGuildPos();

	/*
	* Comments:获得某帮派技能的等级 
	* @Return void:
	*/
	int GetGuildSkillLevel(int skillId);

	/*
	* Comments:设置帮派技能的等级 
	* @Return void:
	*/
	void SetGuildSkillLevel(int skillId,int skillLevel);

	/*
	* Comments:设置本人的帮派地位
	* Param INT_PTR nPos:地位值，参看 tagSocialMaskDef的定义
	* @Return int:
	*/
	VOID SetGuildPos(INT_PTR nPos);

	/*
	* Comments:脱离某个帮派后的处理（有可能是被开除或者自己脱离的）
	* Param boType:是否给开除的，默认是TRUE，如果是FALSE，表示是自己脱离的,要设置nZZNext(在职时间）以限制他不能在规定时间内再加入其他帮派,或接受邀请
	* @Return VOID:
	*/
	VOID OnLeftGuild(BOOL boType);

	/*
	* Comments: 向所有帮派玩家广播
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* @Return void:
	*/
	void BroadCast(char * pData,SIZE_T size);

	/*
	* Comments:获取玩家的贡献值
	* @Return int:
	*/
	int GetGx();

	/*
	* Comments:设置贡献值
	* Param int nValue:
	* @Return VOID:
	*/
	VOID SetGx(int nValue);

	/*
	* Comments:提醒客户端更新帮派信息
	* @Return void:
	*/
	void NotifyUpdateGuildInfo();

	/*
	* Comments:取得在职的超时时间，都是用现在时间加上72小时
	* @Return UINT:
	*/
	static UINT GetZZNext();

	/*
	* Comments:比较玩家的帮派技能与帮派当前的技能大小 false大于等于帮派当前技能等级，true可以升级
	* Param int skillId:技能id
	* Param int skillLevel:当前人物的帮派技能等级
	* @Return UINT:
	*/
	bool CompareGuildSkillLevel(int skillId,int skillLevel);


	//设置参加帮派斗魂副本的标志
	void SetGuldBossFlag(bool bFlag){m_InGuildFuben = bFlag;}

	bool GetGuildBossFlag() {return m_InGuildFuben;}

	void SetGuildDartFlag(bool bFlag){m_InDartNow = bFlag;}

	bool GetGuildDartFlag(){return m_InDartNow;}

	//发送帮派事件给客户端
	void SendGuildEventRecord(int nPage,int nNum);

	//获得帮派职位名称
	char* GetGuildPosName(int posType);

	//查找匹配字符串
	bool NaiveStringMatching(char* srcStr,char *destStr);


	//返回搜索的结果
	void SendSearchGuildResult(CDataPacketReader &packet);

	//返回解散帮派的消息
	void SendDeleteGuildResult(int bType);

	//增加帮派累计贡献
	void AddTotalGuildGx(int value);

	//聊天消息
	void GuildGroupChat(CDataPacketReader &packet);



	//编辑
	void OnEditGuildTitle(CDataPacketReader &packet);

	//获取封号名称
	char* GetGuildTitleName();

	//设置成员封号
	void OnChangeActorTitle(CDataPacketReader &packet);

	//下发封号
	void SendGuildTitleList();

	//随机发送
	void SendWelcomeToNewMember(char* nMsg);

	//获得在本行会的累计贡献
	int GetTotalGuildGx() {return m_GuildGx;}

	bool GetCallGuildBossFlag() {return m_CallGuildBossFlag;}

	//设置我正在召唤boss
	void SetCallGuildBossFlag(bool value) 
	{
		m_CallGuildBossFlag = value;
		m_pGuild->SetUseGuildBoss(value);
	}

	//下发成员加入帮派或者离开帮派
	void SendGuildMemberAddOrLieve(CActor* pActor,BYTE bType,unsigned int nActorId,char* nName);

	//跟行会进行联盟
	void SetUnionGuild(unsigned int nGuildId,int nType);

	//解除联盟
	void RefuseGuildUnion(unsigned int nGuildId);

	//设置敌对行会
	void SetEnemryGuild(unsigned int nGuildId);

	//解除敌对行会
	void RefuseEnemryGuild(unsigned int nGuildId);

	VOID SendSuggestGuildToActor(int nLevel);

	//行会竞价排名
	VOID OnStartBidGuildRank();

	void JoinToGuildByRank(int nRank);


	//进入游戏
	virtual void OnEnterGame();  


	//被杀提示
	void OnBeKilledByOtherTips(CActor* pKiller,char* szSceneName);


	//修改名字后的处理
	void OnChangeNameGuildOp();

	//行会贡献有改变
	void ChangeGuildGx(int nGx,int nLogId = 0);

	//获取已上香次数
	int GetShxDoneTimes() { return m_HasShxTimes;}

	//设置已上香次数
	void SetShxDoneTimes(int nTimes) 
	{ 
		if(nTimes >= 0) 
			m_HasShxTimes = nTimes;
	}

	//获取增加的上香次数
	int GetShxAddTimes() { return m_AddShxTimes;}

	//设置增加上香次数
	void ChangeShxAddTimes(int nTimes) 
	{ 
		m_AddShxTimes += nTimes;
	}

	//获取当天的贡献值
	int GetTodayGxValue() {return m_TodayGx;}

	//改变当天上香的贡献值
	void ChangeTodayGx(int nValue);

	//获取已探险的次数
	int GetExploreTimes() {return m_ExploreTimes;}

	//设置已探险的次数
	void ChangeExploreTimes(int nTimes) 
	{
		m_ExploreTimes += nTimes; 
		if(m_ExploreTimes < 0)
			m_ExploreTimes = 0;
	}

	//获取探险获得物品id
	int GetExploreItemId() {return m_ExploreItemId;}

	//设置探险获得的物品id
	void SetExploreItemId(int nItemId) {m_ExploreItemId = nItemId;}

	//获取今日已挑战的次数
	int GetChallengeTimes() {return m_ChallengeTimes;}

	//设置已挑战的次数
	void SetChallengeTimes(int nTimes) {m_ChallengeTimes = nTimes;}

	//设置添加的次数
	void SetAddChallengeTimes(int nTimes) {m_AddChallengeTimes = nTimes;}

	//获取已添加的次数
	int GetAddChallengeTimes() {return m_AddChallengeTimes;}

	//返回行会建筑的信息
	void SendGuildbuildingInfo();

	//下发行会建筑冷却时间
	void SendGuildbuildCdTime();

	//加载个人行会
	void LoadActorBuildingData();

	//返回加载个人行会数据
	void OnRevActorBuildData(CDataPacketReader &reader);

	//新的一天重置一些数据
	void OnNewDayArriveOP(bool bState = false);

	//下发当天获得的贡献
	void SendTodayGetGuildgx();

	//下发行会捐献数据
	void SendGuildDonateData();

	//下发行会升级材料信息
	void SendGuildUpgradeItem();

	void SendGuildDonateInfo();

	//设置是否领取过奖励
	inline void SetCoinAwardFlag( byte nVal)
	{
		m_nGetCoinAwardFlag = nVal;
	}

	//获取是否领取过奖励
	inline int GetCoinAwardFlag()
	{
		return m_nGetCoinAwardFlag;
	}

	//获取当天捐献金币
	inline int GetTodayDonateCoin()
	{
		return m_nTodayDonateCoin;
	}

	//设置当天捐献金币
	inline void SetTodayDonateCoin(int nNewTodayCoin)
	{
		m_nTodayDonateCoin = nNewTodayCoin;
	}

	//获取当天捐献道具数量
	inline int GetTodayDonateItem()
	{
		return m_nTodayDonateItem;
	}

	//设置当天捐献道具数量
	inline void SetTodayDonateItem(int nNewTodayItem)
	{
		m_nTodayDonateItem = nNewTodayItem;
	}

	//设置帮派职位
	inline void SetGuildTitle(int nGuildPos)
	{
		switch(nGuildPos)
		{
		case smGuildLeader:
			m_nTitle = 1;
			break;
		case smGuildAssistLeader:
			m_nTitle = 2;
			break;
		default:
			m_nTitle = 0;
			break;
		}
		
	}

	//成功加入行会
	void OnJoinGuildSuccess();
	//是否为高级官员
	bool checkSbkGuildSeniorofficials();
protected:

	/*
	* Comments:处理邀请玩家加入帮派的网络请求
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID InviteActor(CDataPacketReader &packet);

	/*
	* Comments:玩家接受还是拒绝加入帮派的网络包
	* Param BOOL result:TRUE表示接受
	* @Return VOID:
	*/
	VOID InviteResult(CDataPacketReader &packet);

	/*
	* Comments:用户提交的入会申请
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID JoinApply(CDataPacketReader &packet);

	/*
	* Comments:给用户显示本帮派中，申请加入的消息列表
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID JoinApplyMsgList();

	/*
	* Comments:客户端提交审核的结果
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID JoinApplyResult(CDataPacketReader &packet);

	/*
	* Comments:开除某个成员,客户端上传的消息
	* Param INT_PTR nActorid:
	* @Return VOID:
	*/
	VOID DeleteMemberRequest(unsigned int nActorid);

	/*
	* Comments:脱离当前的帮派
	* @Return VOID:
	*/
	VOID LeftGuild(BYTE bType);

	/*
	* Comments:处理禅让的请求包
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID LeaderChange(CDataPacketReader &packet);

	/*
	* Comments:处理用户的升职/降职
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID ChangeGuildPos(CDataPacketReader &packet);

	/*
	* Comments:更新公告信息
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID UpdateMemo(CDataPacketReader &packet);

	/*
	* Comments:取得每日的召集令和工资
	* @Return VOID:
	*/
	VOID GetZJ();

	/*
	* Comments:升级帮派
	* @Return VOID:
	*/
	VOID UpgradeGuild();

	//获取扩充人口的信息
	VOID GetUpdateGuildInfo();

	/*
	* Comments:设置聊天群组的id
	* param nCount：
	* @Return VOID:
	*/
	VOID SetGuildQGroupId(CDataPacketReader &packet);

	/*
	* Comments:设置语音频道
	* param nCount：
	* @Return VOID:
	*/
	VOID SetGuildYGroupId(CDataPacketReader &packet);

	/*
	* Comments:客户端发起的宣战信息
	* Param nObjGuildid:目标行会的id
	* @Return VOID:
	*/
	VOID DeclareWar(unsigned int nObjGuildid);

	/*
	* Comments:设置行会之间的关系
	* Param BYTE nType:1 联盟 2 敌对 3 解除联盟 4 解除敌对
	* Param nObjGuildId:对方行会ID
	* @Return VOID:
	*/
	VOID SetGuildRelation(BYTE nType, unsigned int nObjGuildId);
	
	/*
	* Comments:同意或者拒绝行会之间的同盟
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID WarRelationResult(CDataPacketReader &packet);

	/*
	* Comments:从背包移动物品到行会仓库
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID HandItemToGuildDepot(CDataPacketReader &packet);

	/*
	* Comments:从行会仓库取出物品到背包
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID GetItemFromGuildDepot(CDataPacketReader &packet);

	/*
	* Comments:获取仓库的物品列表
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID GetDepotItemList(CDataPacketReader &packet);

	/*
	* Comments:获取仓库的操作
	* Param CDataPacketReader & packet:
	* @Return VOID:
	*/
	VOID SendGuildDepotRecord(CDataPacketReader &packet);


	//请求求救
	void OnRequestHelp(CDataPacketReader &packet);

	//设置玩家直接加入
	void OnSetAddMemberFlag(CDataPacketReader &packet);

	void SendMemberFlag();
	
	/*
	* Comments:下发现在宣战、敌对状态的帮派列表
	* @Return VOID:
	*/
	VOID SendWarList();

	/*
	* Comments:请求帮派战历史记录
	* @Return VOID:
	*/
	VOID SendWarHistory();

	/*
	* Comments:下发升级到下一级所需要的物品数量等信息
	* @Return void:
	*/
	void SendUpLevelInfo();

	/*
	* Comments:判断时间是否已达到指定时间
	* Param UINT nTime:
	* @Return BOOL:
	*/
	static BOOL TimeOK(UINT nTime);



	//下发贡献排行列表
	void SendGxRankinglist();

	/* 
	* Comments:发送是否自动报名
	* @Return void:
	*/
	//void SendAutoSbkSignUp();
	void SendCallTimeRemain();

	void SendGuildOpOk(BYTE nType);

	/*
	* Comments:触发一个事件
	* Param INT_PTR nSubEvent: 子事件的ID
	* Param INT_PTR nParam1:参数1，不需要就直接按默认参数为-1
	* Param INT_PTR nParam2:参数2，不需要就直接按默认参数为-1
	* Param INT_PTR nParam3:参数3，不需要就直接按默认参数为-1
	* Param INT_PTR nParam4:参数4，不需要就直接按默认参数为-1
	* @Return void:
	*/
	void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);

	void PutInUpgradeGuildItem(int nCount1, int nCount2, int nCount3);

	//弹劾官员
	void ImpeachGuildMember(unsigned int nObjActorId);

	int GetDailyDonateCoinLimit();
	int GetDailyDonateItemLimit();

	static int QuickSortGuildDepotCmp(const void *a ,const void *b);

	//行会召唤
	void CallGuildMember();
	//行会召唤应答
	void CallGuildMemberBack(CDataPacketReader &packet);


	//行会建筑
	void UpdateGuildBuild(CDataPacketReader& packet);


public://攻城相关的
	//穿上/脱下龙袍
	void TakeOnOffDragonRobe(BYTE nType);
	//下发皇城职位信息
	void SendGuildCityPosInfo();
	///设置皇城职位
	void SetGuildCityPostion(BYTE nType, BYTE nState, const char* sName);
	//GM设置皇城职位
	void GMSetGuildCityPostion(const char* sGuildName, BYTE nType, BYTE nState, const char* sName);
	//获取皇城职位(攻城)
	int GetGuildCityPos();

	//下发攻城的行会列表
	void SendSignUpGuildList();
	
	

	/* 
	* Comments: 
	* Param int nType: 0出现，1移动，2消失
	* Param CScene * pScene: 指定场景，没有则为当前场景
	* @Return void:  
	*/
	void BroadCastGuildPlayerPos(int nType, CScene* pScene = NULL);
	//下发 其他行会玩家的位置
	void SendGuildPlayerPos(int nType,CVector<void*>& playerList);
	//检查是否在区域属性内，是则广播位置
	void CheckGuildPlayerPos(int nType);
public:		//行会神树(废弃)
	void SendGuildTreeData();
	void SendGuildFruitData();
	void SendGuildFruitPickData();

	int GetTodayChargeTreeNum()
	{
		return m_nTodayChargeTreeNum;
	}

	void SetTodayChargeTreeNum(int nTodayChargeTreeNum)
	{
		m_nTodayChargeTreeNum = nTodayChargeTreeNum;
	}

	int GetTodayPickFruitNum()
	{
		return m_nTodayPickFruitNum;
	}

	void SetTodayPickFruitNum(int nTodayPickFruitNum)
	{
		m_nTodayPickFruitNum = nTodayPickFruitNum;
	}

	void SendGuildTreeBackCoin();	//下发行会神树回馈资金

private:	//行会神树(废弃)
	void ChargeGuildTree();			//行会神树充能
	void PickGuildFruit();			//摘取行会神树果实
	
	int		m_nTodayChargeTreeNum;		//玩家当日已经充能的次数
	int		m_nTodayPickFruitNum;		//玩家当日已经采摘果实的次数


public:		//行会任务(废弃)
	void OnGuildTask(int nTaskType, int nTaskObjId, int nAddTaskSche, int nParam=-1);
	void SendGuildTaskInfo();
	
private:	//行会福利(废弃)	
	void GetGuildTaskAward();
	void GetGuildBenefit();
	void SendGuildTaskBenefitData();

private:
	void GetDepotItemCircleAndNum();			//获取行会仓库中哪些转数的装备的数量
	void GetDepotItemListNoCond(int nPage);
	//按条件过滤仓库物品
	void GetDepotItemListCond( int nPage, int nJobCond, int nGxCond, int nCircle, int nNeedCheck=0 );
	void CheckGuildDepotItemExchange( CDataPacketReader &packet );		//审核行会仓库物品兑换请求
	void SetExchangeNeedCheckCirclrMin(int nCircleMin);

	void OneKeyClearDepotItem( int nCircle );
private:	//沙城战
	void SendGuildSiegeCmdData( );
public:
	CGuildSystem();
public:
	inline BOOL IsFree() {return m_pGuild == NULL;}
	//获取帮派的指针
	inline CGuild * GetGuildPtr(){return m_pGuild;} 
	inline void SetGuild(CGuild* pGuild) { m_pGuild = pGuild; }

	inline void SetJoinTime( unsigned int nJoinTime ) { m_nJoinTime = nJoinTime; }
	//依据错误码发送tips
	void SendErrorTipMsg(int errorCode, int nLeftTime, int nAotuLevel, int nLevelLimit);

public:		//申请加入行会的操作
	void SendJoinGuildApply();
private:
	CGuild*				m_pGuild;
	byte				m_nGetCoinAwardFlag;
public:
	BYTE				nTk;	//所属堂口
	//下次可领取召集令和工资福利的时间，初始是0，在非0的情况下，其是等于每日的凌晨0点的时间
	//领取召集令后，设置这个值为24小时后，到时间清除现有的所有召集令，并zjNext=0
	//nZjNext有3种情况，nZjNext==0表示召集令给清除，可以领取；nZjNext>现在时间表示已领取，下次领取的时间还没到 ；
	//nZjNext<现在时间 表示可领取
	UINT				nWealNext;
	UINT				nZzNext;	//在职的时间
	UINT				nTomorrow;
	bool				boWealClear;//是否有今天之前的召集令没清除，有的话是true
	BOOL				boSaveed;//判断是否有数据需要保存
	int					m_Score;		//膜拜积分
	bool				m_InGuildFuben;		//在参加帮派斗魂副本的标志
	bool				m_InDartNow;		//是否在帮派押镖
	int					m_GuildGx;			//在本行会累计贡献
	bool				m_CallGuildBossFlag; //是否召唤了帮派斗魂
	int					m_nTitle;			//封号序号(1-帮主、2-副帮主、3-堂主、4-成员)，与行会职位挂钩，
											//当职位的Name可编辑的情况下使用的，目前不使用，采用固定配置
	int					m_TodayGx;			//今天获得贡献

	int					m_HasShxTimes;		//拜关公上香的次数
	int					m_AddShxTimes;		//增加的上香次数

	int					m_ExploreTimes;		//行会月光宝盒探险次数
	int					m_ExploreItemId;		//月光宝盒抽奖获得的物品id

	int					m_ChallengeTimes;		//个人封印兽碑挑战的次数
	int					m_AddChallengeTimes;	//增加的挑战次数

	int					m_nTodayDonateCoin;		//当天可捐献的金币数量
	int					m_nTodayDonateItem;		//当天可捐献的道具数量

	unsigned int		m_nJoinTime;			//加入行会的时间（有行会，此值>0，否则为0）
	BYTE                m_nFastApply; //一键申请

};
