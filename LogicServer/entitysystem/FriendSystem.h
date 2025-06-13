#pragma once

#define NUM_LINKMANLASTLY_MAX   99    //最大联系人数量
#define NUM_DEATHLIST_MAX   20    //战报记录
//社会关系
enum enSocialRelation
{
	SOCIAL_FRIEND = 1,			//好友 
	SOCIAL_BLACK  = 2,			//黑名单
	SOCIAL_FOLLOW = 3,			//关注
	//SOCIAL_REPORT = 4,          //战报
	// SOCIAL_DISCIPLE = 4,		//徒弟
	// SOCIAL_GRADUATE = 5,		//出师
	// SOCIAL_BROTHER = 6,			//结拜
	// SOCIAL_MARRIED = 7,			//结婚
	// SOCIAL_OLDMASTER = 8,		//原来的师傅
};

//社会关系操作
enum enRelationOpType
{
	RELATION_OP_FRIEND	= 1,			//好友 
	RELATION_OP_ENIMY	= 4,			//仇人
	RELATION_OP_MASTER	= 5,			//拜师
	RELATION_OP_STUDENT	= 6,			//收徒
};


//好友、仇敌等关系
typedef struct tagFriendRelation
{
	unsigned int		nActorId;	//玩家id
	char	sName[32];				//玩家姓名
	char	sGuildname[32];			//工会名称
	int		nLevel;					//等级
	int		nJop;					//职业
	int		nIcon;					//头像id
	int     nSex;					//性别
	int     ncircle;                //转数
	int     nIntimacy;				//亲密度
	int     nVip;				//vip
	long long  nWarTeamId;			//战队id
	int     ncolor;                  //颜色
	BYTE	nState;					//----关系 与enSocialRelation对应
	unsigned int		nTime;					//关系形成的时间

	// BYTE	nUpdateClient;				//0不需要更新到客户端 1需要更新到客户端
	tagFriendRelation()
	{
		memset(this,0,sizeof(*this));
	}
}FRIENDRELATION;

typedef struct tagFriendOffline
{
	unsigned int nFriendId;
	char sName[32];
	unsigned int nLastLogoutTime;
	tagFriendOffline()
	{
		memset(this,0,sizeof(*this));
	}
}FRIENDOFFLINE;

typedef struct tagFriend
{
	unsigned int nFriendId;
	char sName[32];
	tagFriend()
	{
		memset(this,0,sizeof(*this));
	}
}FRIEND;

typedef struct tagOffChatMsg
{
	unsigned int		nActorId;			//玩家id
	char	sName[32];			//姓名
	int		nTime;				//时间
	int     nIcon;				//头像id
	int     nLevel;				//等级
	int     nSex;				//性别
	char	sMsg[256];			//聊天消息

	tagOffChatMsg()
	{
		memset(this,0,sizeof(*this));
	}
}FRIENDCHATMSG;

//追踪玩家的信息
typedef struct tagTrackInfo
{
	unsigned int		nActorId;				//玩家id
	char	sName[32];				//玩家姓名
	int     nMapId;					//地图id
	int		nMapX;					//地图坐标x
	int		nMapY;					//地图坐标Y
	char	sMapName[32];			//地图名称

	tagTrackInfo()
	{
		memset(this,0,sizeof(*this));
	}
}TRACKACTOR;

//保存申请添加关系的消息列表
typedef struct tagSocialMsgList
{
	unsigned int		nActorId;				//玩家id
	int					nType;					// 对应关系类型

	tagSocialMsgList()
	{
		memset(this,0,sizeof(*this));
	}
}SOCIALMSGLIST;

typedef struct tagActorDeath
{
	unsigned int nActorId;
	unsigned int nKillerId;			//凶手ID
	char sKillerName[32];			//凶手Name
	int nSceneId;					//场景ID
	// char sSceneName[32];			//场景Name
	unsigned int nKillTime;			//时间
	// int nKillPosX;
	// int nKillPosY;
	BYTE   nType;// 
	tagActorDeath()
	{
		memset(this,0,sizeof(*this));
	}
}ACTORDEATH;

typedef struct tagDeathDropItem
{
	unsigned int nActorId;
	unsigned int nKillerId;			//凶手ID
	unsigned int nKillTime;			//时间
	int nItemId;
	int nItemCount;
	tagDeathDropItem()
	{
		memset(this,0,sizeof(*this));
	}
}DEATHDROPITEM;

typedef struct tagDropItem
{
	int nItemId;
	int nItemCount;
	tagDropItem()
	{
		nItemId = 0;
		nItemCount = 0;
	}
}DROPITEM;

//最近联系人（私聊玩家）
typedef struct tagLinkManLately
{
	unsigned int	nActorId;				//玩家id
	char			sName[32];				//玩家姓名
	int				nLevel;					//等级
	int				nJob;					//职业
	int				nIcon;					//头像id
	int				nSex;					//性别
	char			nSignInfo[128];			//玩家签名
	tagLinkManLately()
	{
		memset(this,0,sizeof(*this));
	}
}LINKMANLATELY;


class CFriendSystem:public CEntitySubSystem<enFriendLogicSystemID, CFriendSystem, CActor> 
{
public:
	CFriendSystem();
	~CFriendSystem(){};

	virtual bool Initialize(void *data,SIZE_T size);

	/*
	* Comments:处理数据服务器返回的内容
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & reader:
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//操作好友、仇敌关系等
	void AddSocialRelation(CDataPacketReader &packet, int type);

	void ApplyDelete(CDataPacketReader &packet);


	// void DoAddSocialRelation(int nIndex, int nType, unsigned int nActorId, char* sName, BYTE bNotice=0);

	//添加好友
	void AskAddFriend(unsigned int nActorId,ACTORNAME sName);
	
     void dealApplyList(unsigned int nActorId, int type = 1);
	//添加关注
	void AddToFollowList(unsigned int nActorId,ACTORNAME sName);

	//添加好友反馈
	void AddFriendResult(CActor* pEntity,int nResult);

	//删除好友
	void DeleteFriends(unsigned int nActorId,char* sName, BYTE type);

	void SetFollowColor(CDataPacketReader &packet);

	FRIENDRELATION* GetFriendLationByActorId(unsigned int nActorid);

	// //好友变动更新组队经验加成图标
	// bool UpdateTemBuf();

	// //添加陌生人
	// void AddStranger(unsigned int nActorId,char* sName);

	// //删除陌生人
	// void DeleteStranger(unsigned int nActorId,char* sName);

	//添加到黑名单
	void AddToBlacklist(unsigned int nActorId,char* sName);

	// //删除黑名单
	// void DeleteBlacklist(unsigned int nActorId,char* sName);

	//添加到仇人(不使用了)
	void AddToEnemyList(CActor* pEntity);

	//删除仇人
	void DeleteEnemy(unsigned int nActorId,char* sName);

	//返回删除关系的消息
	unsigned int SendDeleteSocial(unsigned int nActorId,char* sName,int nType);

	//返回添加好友回馈
	void AddSocialResult(CDataPacketReader &packet);

	//执行添加好友回馈
	void DoAddFriend(BYTE nResult, unsigned int nActorId);

	//下发关系列表
	void SendSocialList(int type);
	void DealSocialList(CDataPacketReader &packet);

	// //下发关系列表(是否在线)
	// void SendSocialStsList();

	//查找关系 -1表示列表内没有
	int GetSocialResultById(unsigned int nActorId);

	//查找在列表中的索引
	int GetSocialIndexById(unsigned int nActorId,char* sName = NULL);

	//下发添加好友等的信息
	void SendAddFriendInfo(unsigned int nActorId,int nType, int nCmd = 0);

	//从数据库查找好友数据的结果
	void LoadFriendsFromDbResult(CDataPacketReader &reader);

	void LoadActorDeathFromDbResult(CDataPacketReader &reader);
	// void LoadActorDeathDropFromDbResult(CDataPacketReader &reader);

	//查找离线聊天消息
	// void LoadFriendChatMsgResult(CDataPacketReader &reader);

	//从数据库查找好友数据
	void LoadFriendsDataFromDb();

	void LoadActorDeathFromDb();
	// void LoadActorDeathDropFromDb();

	//保存到db
	void SaveToDb();

	void SaveActorDeathToDb();

	// void SaveDeathDropToDb();

	//在数据库中删除关系
	void SendDeleteMemToDb(unsigned int nActorId,unsigned int FriendId);


	//获取最大人数
	int GetSocialMaxNum(int nType);

	//获取当前关系的人数
	int GetSocialCurnum(int nType);

	//发送推荐好友的信息
	void SendCanAddFriendList(int nLevel1, int nLevel2, int nMaxNum);

	// //好友聊天
	// void OnFriendChat(CDataPacketReader &packet);

	// //请求可以拜师或者可以收徒的列表
	// void GetCanApplyMasterList(CDataPacketReader &packet);

	// //发送坐标扣除金币
	// void OnSendCosumeCoin();

	// //返回好友聊天
	// void SendFriendChat();

	//玩家下线保存信息
	void FriendLoginOut();

	//更新好友的信息
	void UpdateFriendData(CActor* pEntity);

	// //发送需要更新的好友等列表
	// void SendUpdateFriendData(BYTE nType);

	// //设置需要更新
	// void SetSocialNeedFresh(unsigned int nActorId);

	// //设置需要发送给客户端的标志
	// void UpdateSocialFlag(unsigned int nActorId);

	// //加载离线聊天消息
	// void LoadFriendChatMsg();

	//好友上线提示
	void FriendLoginNotice();

	//进入游戏
	virtual void OnEnterGame();  

	//判断是否存在某种关系
	unsigned int GetSocialRelationId(int nType);

	//判断是不是存在某种社会关系
	bool GetSocialFlag(unsigned int nActorId,int nType);

	//发送申请添加关系给被申请人
	void SendCMdSocialData(int nType,int ncmd, CActor* pEntity);

	// //检查是否可以为师父或者徒弟的条件 ntype 1:为师父 2：为徒弟
	// bool CanBeMasterOrSudent(CActor* pEntity,int nType);

	//设置社会关系
	int SetSocialFlag(unsigned int nActorId,int nType, bool nState);

	// //申请拜师
	// void ApplyMaster(unsigned int nActorId,char* sName);

	// //拜师反馈
	// void ApplyMasterResult(CActor* pEntity,int nResult);

	// //叛离师门
	// void DeleteMaster(unsigned int nActorId,char* sName);

	// void OnLevelUp();
	
	// //收徒
	// void AddStudent(unsigned int nActorId,char* sName);

	// //收徒反馈
	// void AddStudentResult(CActor* pEntity,int nResult);

	// //逐出师门
	// void DriveOutMaster(unsigned int nActorId,char* sName);

	// /*
	// * Comments:获取师徒的亲密度
	// * Param void * pEntity:玩家指针
	// * Param int nType:nActorId 对方的id
	// * @Return bool:成功返回true
	// */
	// int GetMasterIntimacy(unsigned int nActorId,int nType);

	//获取当前关系玩家名称
	char* GetSocialActorName(unsigned int nActorId);

	// //出师处理		nActorId 师父的id
	// void GraduationGiveMasterWard(unsigned int nActorId);

	// //师父领取奖励 nActorId 出师的徒弟id
	// void MasterGetAward(unsigned int nActorId);

	// //获取组队经验加成获得值
	// INT_PTR GetBeTeamAwardExp(INT_PTR nValue);

	// //升级给奖励提示	nActorId师父的id nIndex第一个奖励
	// void GiveUpLevelAwardTips(unsigned int nActorId,int nIndex);

	// //发送给他的同门 出师提示
	// void SendGrauateToMate(unsigned int nActorId,int nLevel);

	// //反馈结婚全服召集
	// void CallAllActorResult(CDataPacketReader &packet);

	// //获取在线好友队伍状态
	// void OnGetOnlineFriendTeamState(CDataPacketReader & packet);

	// // 获取最后那名玩家的信息，例如最新仇人
	// FRIENDRELATION* GetLastSocialActorInfo(int nType);

	// /*
	// * Comments: 徒弟孝敬给师父经验
	// * Param const int nExp:
	// * @Return void:
	// * @Remark:
	// */
	// void PupilToTeacherExp(INT_PTR nExp);

	// /*
	// * Comments: 师傅消费元宝，送给徒弟的绑定元宝
	// * Param const int nYuanbao: 师傅消费的元宝数量
	// * @Return void:
	// * @Remark:
	// */
	// void TeacherToPupilYBBind(const int nYuanbao);

	// //计算属性值
	// bool CalcAttributes(CAttrCalc &calc);

	// void OnDeath(CEntity* pEntity, const CVector<DROPITEM> &listDropItem);

	// void AddOnDeathItem(DEATHDROPITEM stDropitem);

	// /*获取最近一次死亡的时间
	// 0：没有死亡
	// */
	// unsigned int GetDeadTimeRecent()
	// {
	// 	int nCount = (int)m_ActorDeathList.count();
	// 	if( nCount > 0 )
	// 	{
	// 		return m_ActorDeathList[nCount-1].nKillTime;
	// 	}
	// 	return 0;
	// }

	// //void AddDeathDropItem(int nItemId, int nItemCount);
	// //void AttachKillAndDeathDrop();

	// void AddLinkManLately(CActor *pActorOther);		//添加最近联系人

	// void AddIntimacy(unsigned int nActorId, int nType,int nAddValue);

	// void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);
	void AddDeath(CEntity* pEntity, int nType = 0);
	void dealAllAcceptApply();
private:

	// void GetExpToTeacher();

	// //获取徒弟列表
	// void GetPupilList(CVector<int> &PupulIdList);

	// //获取第一个社会关系的玩家ID
	// unsigned int GetFirstSocialActor(int nType);

	// //出师
	// void GraduateMaster();

	// //一键删除好友
	// void FastRemoveFriends( CDataPacketReader & packet );
	// void FastRemoveFriends( const CVector<FRIEND> &FriendList );

	// //装载装载N天内不上线的玩家信息
	// void LoadFriendOfflineFromDb();
	// void LoadFriendsOfflineFromDbResult(CDataPacketReader &reader);
	// void NoticeFriendOffline(int nType, const CVector<FRIENDOFFLINE> &FriendOfflineListIn);

	/*添加到消息列表
	nActorId 玩家id 
	nState 关系类型
	返回 true表示可以申请添加 false表是已申请过
	*/
	// bool AddToSocialMsgList(unsigned int nActorId,int nState);

	// /*判断是否存在这个玩家的申请消息
	// nActorId 玩家id 
	// nState 关系类型
	// 返回 true存在 false 不存在
	// */
	// bool IsExitSocialMsg(unsigned int nActorId,int nState);

	// void SendActorDeathInfo();
	// void GetDeathDropList(unsigned int nKillerId, unsigned int nDeathTime, CVector<DEATHDROPITEM>& m_DropList);

	// //如果m_ActorDeathList超过了最大数，则删掉最先的数据
	// void TruckActorDeathData();

	// void SendLinkManLately();

	// //初始化玩家心情
	// void InitActorMood();

	// //修改心情
	// void AlterActorMood(CDataPacketReader &packet);

	// //下发玩家心情
	// void SendActorMood();

	// //追踪玩家
	// void OnTrackActor( char* sName );

	// void DoTrackActor(CActor* pObjActor, char* sName);

	// void SendTrackActorInfo();

	// void SendTrackAlarmInfo(CActor *pActor);

	// int GetDailyTrackNum();
	// void SetDailyTrackNum(int nNewNum);
	// unsigned int GetTrackFee();

	// void SendTrackActorFee();

	// //推荐好友
	// void RecommendFriends();

	bool CheckActorIdIsApply(unsigned int nActorId);
	void dealAppleList(unsigned int nActorId, int type = 1);
	//被杀
	void DeleteDeathData();
	
	void SendToClientActorDeathInfo();

private:
	CVector<FRIENDRELATION>		m_FriendList;				//好友、关注 黑名单等关系列表
	CVector<unsigned int >               m_applyList;                 //申请id
	bool						m_bSaveDbFlag;				//需要保存到数据库的标志
	// TRACKACTOR					m_TrackInfo;				//追踪的玩家信息      --不用了吧

	int							m_FirstSendList;			//0 是首次 1 非首次

	// CVector<FRIENDCHATMSG>		m_OffineChatMsg;			//离线聊天消息        --不用了
 
	// CVector<FRIENDOFFLINE>		m_FriendOfflineList;		//N天不上线的好友消息 --不用了

	// CVector<SOCIALMSGLIST>		m_SocialMsgList;			//消息列表，添加好友等关系的消息  --不用了

	CVector<ACTORDEATH>			m_ActorDeathList;          //战报
	//ACTORDEATH					m_ActorDeath;
	// CVector<DEATHDROPITEM>		m_DeathDropItemList;       //--不用了
	// CVector<LINKMANLATELY>		m_LinkManLatelyList;		//最近联系人（私聊的） --不用了
};
