#pragma once


#define		MAXGUILDUSERCOUNT	100
#define		GUILDTIMEOUT		30	//操作数据库超时时间，30s
#define		MAXGUILDGXEXP		100000000		//贡献上限
#define		MINCOINSAVETOLOG	10000			//消费金币的最小值才记录日志


//攻城排行榜列id,对应GuildSiegeRankingColumnName
enum GuildSiegeRankingColumnId{
	GSRC_RealMasterId,				//君主id
	GSRC_EastProtectorId,			//东护法
	GSRC_SouthProtectorId,			//南护法
	GSRC_WestProtectorId,			//西护法
	GSRC_NorthProtectorId,			//北护法
	GCRC_IsAutoSignUp,				//是否自动报名了
	GSRC_Count,
};
//攻城排行榜列名，对应GuildSiegeRankingColumnId
static const LPCSTR GuildSiegeRankingColumnName[] = 
{
	"RealMasterId",				//君主id
	"EastProtectorId",			//东护法
	"SouthProtectorId",			//南护法	
	"WestProtectorId",			//西护法
	"NorthProtectorId",			//北护法
	"IsAutoSignUp",				//是否自动报名了
};


//玩家加入行会的申请
struct CJoinGuildApply
{
	unsigned int	nActorId;					//申请玩家ID
	unsigned int	nGuildId;					//批准的行会ID
	int				nSex;
	int				nLevel;
	char			nMenPai;
	int				nJob;
	int				nCircle;
	unsigned int	nCombatPower;				//战斗力（ZGame是攻击力）
	unsigned int    nLogintime;         //登录时间
	std::vector<unsigned int> listObjGuild;			//目标行会ID
	unsigned int	nVipGrade;//TODO: DELETE
	ACTORNAME		szActorName;
	CJoinGuildApply()
	{
		nActorId	= 0;
		nGuildId	= 0;
		nSex		= -1;
		nLevel		= 0;
		nMenPai		= 0;
		nJob		= 0;
		nCombatPower= 0;
		nVipGrade	= 0;
		nCircle     = 0;
		_asncpytA(szActorName, "");
		listObjGuild.clear();
	}

	void DeleteGuildApply(unsigned int nGuild)
	{
		if(listObjGuild.size() <= 0 )
			return;
		std::vector<unsigned int>::iterator it = listObjGuild.begin();
		for(; it != listObjGuild.end();)
		{
			if(*it == nGuild)
				it = listObjGuild.erase(it);
			else
			{
				it++;
			}
			
		}

	}
};

typedef std::map<unsigned int,CJoinGuildApply* > JoinGuildApplyMap;			//玩家申请加入行会

class CGuildComponent:
	public CComponent
{
	/// 保存用户操作数据库的数据
	typedef struct tagUserOperData
	{
		Uint64			hActor;		///< 对应的玩家
		unsigned int	nTimeOut;	///< 超时时间,0表示已超时		
		int				nType;		///< 操作的类型
	}UserOperData;

public:	


	/*
	* Comments:从数据库获取所有帮派的列表
	* @Return VOID:
	*/
	VOID Load();

	//销毁
	VOID  Destroy();
	
	/*
	* Comments:数据服务器返回数据
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);	//

	/*
	* Comments:数据库返回增加帮派的结果
	* Param CDataPacketReader & inPacket:
	* Param INT_PTR nErrorCode:
	* @Return void:
	*/
	void AddGuildResult( CDataPacketReader &inPacket, int nErrorCode ) ;

	/*
	* Comments:数据库返回帮派成员等信息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID LoadGuildDetailResult( CDataPacketReader & inPacket ) ;

	/*
	* Comments:数据库返回帮派技能的信息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID LoadGuildSkillResult( CDataPacketReader & inPacket ) ;


	VOID LoadGuildEventResult( CDataPacketReader & inPacket ) ;


	void LoadGuildDepotResult(CDataPacketReader & inPacket);

	void LoadDepotRecordResult(CDataPacketReader & inPacket);

	void SaveAllGuildEvent();

	void LoadGuildApplyListsResult(CDataPacketReader &inPacket);
	
	/*
	* Comments:数据库返回帮派列表信息
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID LoadGuildListResult(CDataPacketReader &inPacket ) ;

	/*
	* Comments:增加一个帮派，向数据库服务器发送请求
	* Param CActor* pActor:角色
	* Param LPCSTR sGuildName:帮派名称
	* Param LPCSTR sFoundname:创始人的名称
	* Param int nZTType:阵营类型
	* Param WORD icon:帮派图标
	* @Return VOID:
	*/
	VOID SendDbAddGuild(unsigned int nActorid,Uint64 hEntity,LPCSTR sGuildName,LPCSTR sFoundname);	//增加一个帮派

	/*
	* Comments:向数据库发送解散一个帮派的消息
	* Param int id:帮派id
	* @Return VOID:
	*/
	VOID SendDbMsgDeleteGuild(Uint64 hEntity,unsigned int id);

	/*
	* Comments:读入一个帮派的详细数据
	* Param int nGid:帮派id
	* @Return VOID:
	*/
	VOID LoadGuildDetail(unsigned int nGid);

	/*
	* Comments:读取一个行会的申请列表
	* Param int nGid:帮派id
	* @Return VOID:
	*/
	VOID LoadGuildApplyLists(unsigned int nGid);

	
	VOID SaveGuildApplyReslut();

	/*
	* Comments:读入一个帮派的技能数据
	* Param int nGid:帮派id
	* @Return VOID:
	*/
	VOID LoadGuildSkill(unsigned int nGid);

	/*
	* Comments:读入一个帮派的事件
	* Param int nGid:帮派id
	* @Return VOID:
	*/
	void LoadGuildEvent(unsigned int nGid);

	//读入一个行会的仓库
	void LoadGuildDepot(unsigned int nGid);

	//读入行会的仓库记录
	void LoadGDepotRecord(unsigned int nGid);
	
	/*
	* Comments:调整排行，通常是帮派的等级、繁荣度等升了之后调用,调用前有可能pGuild还没在这个列表中
	* Param CGuild * pGuild:
	* @Return VOID:
	*/
	VOID AdjustRank(CGuild* pGuild);

	/*
	* Comments:根据帮派id查找，遍历查找
	* Param int nGid:
	* @Return CGuild*:
	*/
	CGuild* FindGuild(unsigned int nGid);

	/*
	* Comments:根据排名查找
	* Param int nGid:
	* @Return CGuild*:
	*/
	CGuild* FindGuildByRank(int nRank);

	/*
	* Comments:根据帮派名称查找，遍历查找
	* Param int nGid:
	* @Return CGuild*:
	*/
	CGuild* FindGuildByName(const char* nName);

	//根据排名找行会的名称
	char* GetGuildNameByRank(int nRank);

	//根据排名找行会首领的名称
	const char* GetGuildLeaderNameByRank(int nRank);

	//根据排名找行会的人数
	int GetGuildMemNumByRank(int nRank);

	//根据排名找行会的资金
	int GetGuildCoinByRank(int nRank);

	bool HasGuildByRank(int nRank);

	//GM设置某人为帮主
	void SetGuildLeaderByname(char* nGuildname,char* nName);
	//帮主替换统一处理
	void OnGuildLeaderChange(CGuild* pGuild, unsigned int nOldLeaderId, unsigned int nNewLeaderId, int nChangeType);
	//根据句柄来获取行会id
	unsigned int FindGuildIdByHandle(unsigned int nHandle);

	/*
	* Comments:解散一个帮派,这个函数应该在数据库返回结果之后
	* Param CGuild * pGuild:
	* Param CGuild * bType: 0 帮派解散，1 系统解散
	* @Return bool:
	*/
	bool DeleteGuild(CGuild* pGuild,int bType = 0);

	/*
	* Comments:定期检查是否有帮派信息需要保存
	* @Return VOID:
	*/
	VOID RunOne();

	/*
	* Comments:初始化帮派技能
	* @Return VOID:
	*/
	void InitGuildSKill(CGuild* pGuild);

	/*每周行会维护
	每周日凌晨扣除行会资金，资金不足则删除行会
	*/
	void MaintainGuildWeekly();

	//读取行会攻城数据
	void LoadGuildSiegeRanking();


	//后台删除行会
	int LogSenderDelGuild(char* sGuildName);


	//发送行会列表到公共服
	//void SendGuildListToDestServer();

	//公共服接收普通服发来的行会列表
	void OnRevGuildListFromServer(CDataPacketReader &reader,int nOtherServerId);

	//将行会神树回归到幼苗期
	void ProcGuildTreeToSapling();

	//处理行会每日数据
	void ProcDailyData();

	//下发行会战指挥面板到全体参战的成员
	void SendGuildSiegeCmdPanel(unsigned char nFlag);
public://攻城相关
	//是否在攻城战中
	inline bool IsGuildSiegeStart(){return m_GuildSiegeStartFlag > 0; }
	//RunOne处理占领10分钟逻辑
	void OnGuildSiegeRunOne();
	//攻城战结束
	void GuildSiegeEnd(unsigned int nGuildId);
	//今天是否开启攻城战
	bool IsGuildSiegeTodayOpen();
	//自动报名攻城战(脚本ScriptTimer调用)
	void AutoSignGuildSiege();
	//攻城当年重置下报名
	void ResetSignGuildsiege();

	void ClearGuildSiegeCommand();

	//是否能通过皇宫传送点
	bool CanPassPalaceTeleport(CActor* pActor);
	//保存行会攻城数据
	void SaveGuildSiegeRanking();
	//设置占领皇城行会的id
	void SetCityOwnGuildId(unsigned int nGuildId);
	//获取占领皇城行会的id
	unsigned int GetCityOwnGuildId() {return m_GuildId;}
	//开始攻城战
	void StartGuildSiege();
	//根据id获取皇城职位
	int getCityPostionById(unsigned int nActorId);
	//设置皇城职位,暂时不处理君主的
	void SetCityPos(int nPos, unsigned int nActorId);
	//获取玩家皇城职位名称
	const char* GetCityPosName(unsigned int nActorId);
	//获取沙巴克城的职位的姓名
	char* GetCityPostionInfo(int nPos, int &nJob,int &nSex);
	//设置攻城雕像
	void SetGuildSiegeMasterStatue();
	//设置城墙传送是否可见
	void SetWallTepotVisible(bool bVisible,unsigned int hScene,int x,int y);
	//获取皇城君主的id
	unsigned int GetCityOwnMasterId();
	//沙巴克战时更新名称颜色名字
	void UpdateGuildMemberName(CActor* pActor);
	//清除攻城占领数据(回收沙城)
	void ClearGuildSiegeData();

	//沙巴克行会昵称
	const char* getSbkGuildName();
	//沙巴克行会会长昵称
	const char* getSbkGuildLeaderName();

public:  //玩家申请加入行会
	//玩家申请加入行会（此时在线）
	int AddJoinGuildApply( CActor *pActor, unsigned int nGuildId );

	JoinGuildApplyMap& GetJoinGuildApplyMap()
	{
		return m_JoinGuildApplyMap;
	}

	int CheckJoinGuildApply( unsigned int nActorId, unsigned int nGuildId, char nResult );

	CGuild* ActorJoinGuildByApply( CActor* pActor );

	bool HasJoinGuildApply( unsigned int nGuildId );
	
	void LoadActorApplyResult(CDataPacketReader& inPacket);

	unsigned int  CheckIsJoinGuild(unsigned int nActorId)
	{
		std::map<unsigned int, unsigned int>::iterator it = m_GuildApplyResult.find(nActorId);
		if(it != m_GuildApplyResult.end())
		{
			return it->second;
		}
		return 0;
	}

	void AddJoinGuildid(unsigned int nActorId, unsigned int nGid)
	{
		if(nGid <=0 )
		{
			return;
		}
		std::map<unsigned int, unsigned int>::iterator it = m_GuildApplyResult.find(nActorId);
		if(it == m_GuildApplyResult.end())
		{
			m_GuildApplyResult.insert(std::make_pair(nActorId, nGid));
			ModifyData();
		}
	}
	void DeleteJoinGuildRelust(unsigned int nActorId)
	{
		if(nActorId <=0 )
		{
			return;
		}
		std::map<unsigned int, unsigned int>::iterator it = m_GuildApplyResult.find(nActorId);
		if(it != m_GuildApplyResult.end())
		{
			m_GuildApplyResult.erase(it);
			ModifyData();
		}
	}
	void getsbkOfflineData(CDataPacket& data );

private:
	
	/*
	* Comments:把新的帮派加到列表中，同时会进行排序
	* Param int nGid:
	* Param WORD nLevel:
	* Param LPCSTR sGuildname:
	* Param LPCSTR sFoundname:
	* Param LPCSTR sInMemo:
	* Param LPCSTR sOutMemo:
	* @Return CGuild*:
	*/
	CGuild* AddGuildToList(unsigned int nGid,WORD nLevel,int nCoin,int nYType,int nBidCoin,int nTime,int nFlag,int sFlag,LPCSTR qGroupId,LPCSTR yGroupId,
		LPCSTR sGuildname,LPCSTR sFoundname,LPCSTR sInMemo="",LPCSTR sOutMemo="",LPCSTR nyMemo="",LPCSTR groupMemo="");

	CJoinGuildApply* GetJoinGuildApply( unsigned int nActorId )
	{
		if( nActorId <=0 )
		{
			return NULL;
		}
		JoinGuildApplyMap::iterator iter = m_JoinGuildApplyMap.find( nActorId );
		if( iter == m_JoinGuildApplyMap.end() )
		{
			return NULL;		
		}

		return iter->second;
	}

	//删除某玩家的所有申请
	void RemoveActorJoinGuildApply( unsigned int nActorId )
	{
		if( nActorId <=0 )
		{
			return;
		}
		JoinGuildApplyMap::iterator iter = m_JoinGuildApplyMap.find( nActorId );
		if( iter == m_JoinGuildApplyMap.end() )
		{
			return ;		
		}
		else
		{
			CJoinGuildApply* pApply = iter->second;
			delete pApply;
			m_JoinGuildApplyMap.erase( iter );
		}
	}

	

public:
	CGuildComponent();
	~CGuildComponent();
	void ModifyData()
	{
		m_bIsModifyData = true;
	}
private:

	UserOperData* GetFreeUserOperData(INT_PTR& nIndex);
	VOID NotifyActor(Uint64 nIndex,Uint64	hHandle,int nType,int nErrorCode,unsigned int nGuildId,CGuild* pGuild=NULL);
public:
	CList<CGuild*>*					m_pGuildList;//	帮派列表
	CList<CGuild*>*					m_pRank;	//按排名，排名规则 1、等级 2、繁荣度
	bool							m_boTodaySign;					//今天攻城是否报名了(19点)
	unsigned int					m_GuildId;						//占领沙皇宫的行会id 攻城结束才有
	int								m_GuildSiegeStartFlag;			//攻城开始的标志 1开始 0结束
	//unsigned int					m_OccupyTime;					//临时占领的时刻
	//unsigned int					m_JadeSealOwnerId;				//临时玉玺占领玩家id
	unsigned int					m_PostionId[stMaxGuildCityPos]; //皇城职位的玩家id ,
private:
	JoinGuildApplyMap				m_JoinGuildApplyMap;
	CVector<UserOperData>			m_UserOperList;

	//玩家id 工会id
	std::map<unsigned int , unsigned int> m_GuildApplyResult;   //玩家申请结果 --主要处理未登录的玩家


	CTimer<60000>					m_timer;	//1分钟检查一次
	CTimer<3600000>					m_SecTimer;	//1小时检查一次3600000
	CTimer<1000>					m_1sTimer;	//1s检查一次
	CTimer<600000>					m_10MinsTimer;	//10分钟检查一次

	bool							m_DbInitGuild;			//是否从数据库加载成功
	int								m_GuidlDetailLoadCount; //行会人员具体信息加载成功数量
	bool                            m_bIsModifyData;       //数据是否变化 ---处理行会申请数据
	bool							m_bGuildClose;			//是否关闭行会
	bool 							m_bGuildClear;			//是否清理行会
public:
	static 	CBufferAllocator*		m_pAllocator;
//测试函数
public:
	static VOID test();
	//是否关闭行会
	bool IsGuildClose() {  return m_bGuildClose; }
	//清理本服所有行会
	bool ClearAllGuild();	
//跨服
	void SendSbkGuildRankAward(int nGuildId, int nRankId );
	void SendSbkGuildAward();
};
