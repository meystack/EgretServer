#pragma once

/***************************************************************/
/* 队伍的类，实现队伍的基本功能，包括队伍成员管理，队长管理
/* 队伍的物品的分配
/***************************************************************/


#define MAX_TEAM_BUFF 7 // 1个经验加成buff+6个组队buff

class CActor;

/// 队伍成员的数据结构
typedef struct tagTeamMember
{
	unsigned int	nActorID;	///< 玩家的actorID
	CActor *		pActor;     ///< 玩家的指针 
}TEAMMEMBER,*PTEAMMEMBER;

/// 离线玩家的基本信息，主要供一些入队的玩家查看用的
typedef struct tagOffLineTeamMember
{
	unsigned int	nActorID;	///< actorID
	WORD			wIconID;	///< 头像的id
	BYTE			btLevel;	///< 等级
	BYTE			btVocation; ///< 职业
	BYTE			btSex;      ///< 性别
	BYTE			btCircle;	///< 转生
	WORD			wReserver;	///< 保留
	ACTORNAME		name;       ///< 名字
	unsigned int	nVip;	///< vip
}OFFLINETEAMMEMBER, *POFFLINETEAMMEMBER;

class  CTeam 
{
public:	

	CTeam() : m_nBuffCount(0)
	{
		memset(this,0,sizeof(CTeam));
		//m_bLootType = tmTeamIteamStyleFree; //默认就是自由拾取
		m_bRoolItemMinLevel = CStdItem::iqItemWhite; //默认是绿装

		m_nFbId = 0;
		m_nChallengeId = 0;

		m_bTeamMaxMmemberCount = MAX_TEAM_MEMBER_COUNT;
		
		m_nTimerExpiredTime = 0;		
		ZeroMemory(m_BuffId, sizeof(m_BuffId));
		m_CallInfo=std::make_tuple(0,0,0,0,0);
	}
	
	/*
	* Comments: 添加成员
	* Param CActor * pActor: 成员的指针
	* @Return bool: 成功返回true，否则返回false
	*/
	bool AddMember(CActor * pActor,bool isNewMember = true);

	void SendMemberData(CActor* pActor, CActor* pMember = NULL, bool IsBroadcast = false);
	/*
	* Comments:删除队伍的一个成员，考虑到可能需要剔除离线的队友，这里不能使用指针
	* Param unsigned int nActorID: 队员的actorID
	* Param bool bNeedBroadcast: 是否需要广播,需要广播设置为true，这个时候将向队伍广播，否则只向自己发送
	* Param bool bNeedSelfDestroyTeam: 是否需要自动删除,true的话没人在线了就会删除队伍 
	* @Return bool: 成功返回true，否则返回false
	*/
	bool DelMember(unsigned int nActorID, bool bNeedBroadcast =true, bool bNeedSelfDestroyTeam = true);

	
	//当队伍里面人员变动更新经验加成buf
	bool UpdateBufMember();
	//像组队队员发消息
	VOID SendTeamChatMsg(char* msg, CActor* pActor = NULL);

	//删掉一个玩家的经验加成buf
	bool DelMemberBuf(CActor * pActor);

	//获取一个玩家当前身上的组队经验buffId
	int GetTeamKillMonsterExpBuffId(CActor* pActor);
	/*
	* Comments:设置一个玩家为队长
	* Param CActor * pActor:玩家的指针
	* @Return bool:成功返回true，否则返回false
	*/
	bool SetCaptin(CActor * pActor);
	
	//该队伍是否还有效
	bool IsTeamValid();

	/*
	* Comments:设置一个玩家为离线状态
	* Param unsigned int nActorID: 玩家的actorID
	* @Return bool:成功返回true，否则返回false
	*/
	bool SetUserOffline(CActor * pActor);

	//重新选一个在线的人为队长, 选等级最大的
	inline CActor * SelectCaptin();
	
	/*
	* Comments: 销毁一个队伍
	* @Return void:
	*/
	void DestroyTeam();
	
	/*
	* Comments: 获取在线人数的数量，因为这个使用得特别频繁，故放这里
	* @Return INT_PTR:返回在线人数的数量
	*/
	inline INT_PTR GetOnlineUserCount()
	{
		return m_bOnlinemMemberCount;
	}

	inline void SetTeamID( unsigned int nTeamID)
	{
		m_nTeamID = nTeamID;
	}

	inline unsigned int GetTeamID()const
	{
		return m_nTeamID;
	}

	/*
	* Comments: 获取队伍的总人数
	* @Return INT_PTR: 返回队伍的总人数
	*/
	inline INT_PTR GetMemberCount()
	{
		INT_PTR nCount = 0;
		for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
		{
			if( m_member[i].nActorID  )
			{
				if(m_member[i].pActor )
				{
					if( ((CEntity*) m_member[i].pActor)->IsInited() ==false)
					{
						m_member[i].pActor = NULL;
						m_member[i].nActorID =0; //如果数据出现了问题，那么清掉这个玩家的数据
					}
					else
					{
						++nCount;
					}
				}
				else
				{
					++nCount;
				}
			}
		}
		return nCount; 
	}

	/*
	* Comments: 获取队长的指针
	* @Return CActor *: 返回队长的指针
	*/
	inline CActor* GetCaptin()
	{
		return m_pCaptin;
	}


	/*
	* Comments: 向在线的队友广播一个消息
	* Param void * pData: 数据的指针
	* Param INT_PTR nLen: 数据的长度
	* @Return void:
	*/
	void BroadCast(LPCVOID pData, INT_PTR nLen);
	
	/*
	* Comments:向在线的队友广播一个消息,除了自己
	* Param void * pData:
	* Param INT_PTR nLen:
	* Param int nMyId:自己的角色id
	* @Return void:
	*/
	void BroadCast(LPCVOID pData, INT_PTR nLen,INT_PTR nMyId);

	/*
	* Comments: 向队伍中在线成员广播数据包的兼容接口
	* Param LPCVOID lpData:
	* Param SIZE_T dwSize:
	* @Return void:
	*/
	inline void SendData(LPCVOID lpData, SIZE_T dwSize)
	{
		return BroadCast(lpData, dwSize);
	}

	inline CActor * GetActorByID(unsigned int nActorID)
	{
		for(INT_PTR i=0 ;i < MAX_TEAM_MEMBER_COUNT; i++)
		{
			if(m_member[i].nActorID == nActorID)
			{
				if(m_member[i].pActor  )
				{
					if(((CEntity*)m_member[i].pActor)->IsInited())
					{
						return m_member[i].pActor;
					}
					else
					{
						return NULL;
					}
				}
			}
		}
		return NULL;
	}

	/*
	* Comments: 向队伍的每一个人发生一条系统提示
	* Param INT_PTR nTipmsgID:提示的ID
	* Param INT_PTR nTipmsgType:提示的类型
	* @Return void:
	*/
	void BroadcastTipmsg(INT_PTR nTipmsgID, INT_PTR nTipmsgType= ttTipmsgWindow);

	/*
	* Comments: 向队伍里的玩家发送带参数的系统提示
	* @param uExcludeActorId 这个id不用发
	* Param INT_PTR nTipmsgID:系统ID
	* Param INT_PTR nTipmsgType: 提示类型
	* Param ...: 不定参数
	* @Return void:
	*/
	void BroadcastTipmsgArgs(unsigned int uExcludeActorId, INT_PTR nTipmsgID, INT_PTR nTipmsgType= ttTipmsgWindow,...); 
	
	/* 
	* Comments: 获取玩家在队伍列表里的位置
	* Param unsigned int nActorID: 玩家的actorID
	* @Return INT_PTR: 如果存在这个玩家返回这个玩家的数组下标，否则返回-1
	*/
	inline INT_PTR GetActorIndex(unsigned int nActorID)
	{
		if(nActorID ==0) return -1; //如果输入0，肯定是出了问题
		for(INT_PTR i=0 ;i < MAX_TEAM_MEMBER_COUNT; i++)
		{
			if(m_member[i].nActorID && m_member[i].nActorID == nActorID )
			{
				return i;
			}
		}
		return -1;
	}

	/*
	* Comments: 是否有队伍里的成员需要这个任务物品
	* Param INT_PTR nTeamID:
	* @Return bool:
	*/
	//bool HasTeamMemberNeedQuestItem(INT_PTR nTeamID);

	
	/*
	* Comments: 保存玩家的基本信息在组件里，
	* Param CActor * pActor:指针
	* Param unsigned int nActorID:actorID
	* @Return void:
	*/
	void SaveOfflineMsg(CActor * pActor,unsigned int nActorID);

	/*
	* Comments: 获取一个玩家身边的队友数量，并且把队友的列表添加pMember，如果pMember不空的话
	* Param CActor * pUser:玩家的指针
	* Param CActor * * pMember:队友的列表,如果有队友就会存储在这个数组里
	* @Return INT_PTR: 返回队友的数量
	*/
	INT_PTR GetNearTeamMember(CActor* pUser,CActor ** pMember =NULL);


	/*
	* Comments:获取队员
	* @Return const TEAMMEMBER*:
	*/
	inline const TEAMMEMBER* GetMemberList() {return m_member;}

	/*
	* Comments: 广播一个数据包给一个包裹的所有人
	* Param CDropBag * pBag:背包的指针
	* Param void * pData:数据指针
	* Param INT_PTR nSize:数据长度
	* @Return void:
	*/
	void BroadCastBagOwner(CDropBag * pBag, char * pData,SIZE_T nSize);

	/*
	* Comments:获取对应的副本id
	* @Return int:
	*/
	inline int GetFbId() {return m_nFbId;}

	/*
	* Comments:设置对应的副本id
	* Param int nFbId:
	* @Return VOID:
	*/
	inline VOID SetFbId (int nFbId) { m_nFbId = nFbId;}

	/*
	* Comments: 获取队伍能加入的最大人数
	* @Return INT_PTR:返回人数
	*/
	inline INT_PTR GetTeamMaxMemberCount()
	{
		if(m_bTeamMaxMmemberCount)
		{
			return m_bTeamMaxMmemberCount;
		}
		else
		{
			return MAX_TEAM_MEMBER_COUNT;
		}
	}
	/*
	* Comments: 设置队伍最大的人数
	* Param INT_PTR nCount:人数
	* @Return bool:成功返回true，否则返回false
	*/
	inline bool SetTeamMaxMeberCount(INT_PTR nCount)
	{
		if(nCount > MAX_TEAM_MEMBER_COUNT || nCount <= 0) return false;
		if(nCount == m_bTeamMaxMmemberCount) 
		{
			return true;
		}
		m_bTeamMaxMmemberCount = (BYTE)nCount;
		return true;
	}

	/*
	* Comments:获取所有在线成员的平均等级
	* @Return INT_PTR:
	*/
	INT_PTR GetAllAvgLevel();

	/*
	* Comments: 返回一个人在离线信息列表的信息
	* Param unsigned int nActorID:玩家的actorID
	* @Return OFFLINETEAMMEMBER *: 玩家的信息指针
	*/
	inline OFFLINETEAMMEMBER * GetOffLineMsg(unsigned int nActorID)
	{
		if(nActorID ==0) return NULL;
		for(INT_PTR i=0; i< MAX_OFFLINE_MEMBER_COUNT; i++)
		{
			if(m_offLineMember[i].nActorID == nActorID)
			{
				return &m_offLineMember[i];
			}
		}
		return NULL;
	}

	//获取本队伍的擂台id
	inline unsigned int GetChallengeId() {return m_nChallengeId;}
	//设置本队伍的擂台id
	void SetChallengeId(unsigned int hHandle);

	//队伍是否正在打擂台
	inline bool IsChallenge() { return m_nChallengeId != 0;}

	//获取变量对象
	inline CCLVariant& GetVar() { return m_DynamicVar; }

	void RunOne();

	/*
	* Comments:设置过期时间
	* Param unsigned int nExpireTime:过期那一刻
	* @Return void:
	*/
	void SetExpiredTime( unsigned int nExpireTime);
	//广播更新玩家信息
	void BroadcastUpdateMember(CActor* pActor, int nPropId);
	
	//发送解散组队消息--客户端不好处理
	void SendDestoyTeam(CActor* pActor);
private:


	//广播新加玩家
	void BroadcastAddMember(CActor * pActor, BYTE idx);

	//广播玩家在线状态
	void BroadcastMemberOnlineState(CActor* pActor, BYTE state);
	
	/*
	* Comments: 把一个玩家的信息写进去
	* Param CActor * pActor:
	* Param unsigned int nActorID:
	* Param CDataPacket & pack:
	* @Return void:
	*/
	void WriteActorInfo(CActor * pActor,CDataPacket & pack, int isOnline);

	/*
	* Comments: 写不在线的玩家的信息
	* Param OFFLINETEAMMEMBER * pMsg: 信息指针
	* Param CDataPacket & data:数据读写器
	* @Return void:
	*/
	void WriteOffLineActorInfo(OFFLINETEAMMEMBER * pMsg,CDataPacket & data, BYTE idx);


	/*
	* Comments: 该物品是否需要分配（队长模式和队伍模式的高级物品）
	* Param INT_PTR nItemID: 物品的id
	* @Return bool:需要返回true，否则返回false
	*/
	inline bool NeedAssigned(CDropItem &item)
	{
		if(item.btType <Item::itEquipMax /*&& (m_bLootType== tmTeamItemStyleTeam ||m_bLootType== tmTeamItemStyleCaptin )*/ )
		{
			return item.item.btQuality >= m_bRoolItemMinLevel;
		}
		else
		{
			return false;
		}
	}
	
	/*
	* Comments: 获取离线玩家的消息
	* Param unsigned int nActorID:
	* @Return INT_PTR:
	*/
	inline INT_PTR GetOffLineActorIndex(unsigned int nActorID);



	/*
	* Comments:  清除一个玩家的离线消息
	* Param unsigned int nActorID:玩家的actorID
	* @Return :
	*/
	inline void ClearOffLineMsg(unsigned int nActorID)
	{
		for(INT_PTR i=0 ;i < MAX_OFFLINE_MEMBER_COUNT ;i++)
		{
			if(m_offLineMember[i].nActorID == nActorID)
			{
				memset( &m_offLineMember[i],0,sizeof(m_offLineMember[i])); //把这段内存清空
			}
		}
	}

	// 队伍加入成员
	void OnMemberAdded();


	void AddFriendIntimacyWithMember(CActor *pActor);

public:
	void SetCallMemInfo(unsigned int nActorId, int nSceneId, int nX, int nY, unsigned int nExpireTime)
	{
		m_CallInfo = std::make_tuple(nActorId, nSceneId, nX, nY, nExpireTime);
	}

	std::tuple<unsigned int,int,int,int,unsigned int>
	GetCallMemInfo()
	{
		return m_CallInfo; 
	}

	static int m_nMaxTeamLootDistanceSquare;// 最大的拾取距离的平分
private:
	TEAMMEMBER m_member[MAX_TEAM_MEMBER_COUNT]; //5人的队伍名单
	//BYTE m_bLootType; //队伍的物品的分配方式
	BYTE m_bOnlinemMemberCount; //队伍在线的人数
	BYTE m_nLastLootSlotID; //上一次获得物品的玩家的巢位,用于轮流拾取,从1到5才是有效的
	BYTE m_bRoolItemMinLevel; // Roll物品的最低等级，比如蓝装
	BYTE m_bTeamMaxMmemberCount; //队伍最大的人数
	BYTE m_bReserver;
	WORD m_wReserver;

	CActor*				m_pCaptin;        ///< 队长的指针 

	unsigned int		m_nTeamID;        ///< 自身的ID，这个考虑到一些内部逻辑要使用	

	OFFLINETEAMMEMBER	m_offLineMember[MAX_OFFLINE_MEMBER_COUNT]; //存离线玩家的基本信息，供查询用
	
	//zac:记录这个队伍对应的副本id（在创建副本队伍的时候设置），默认是0，表示本队伍没打算进入任何副本
	//退出副本后要改回0
	int					m_nFbId;

	unsigned int		m_nChallengeId;			///< 擂台的id，没有则为0

	CCLVariant			m_DynamicVar;			///< 导出给lua层用

	UINT				m_nTimerExpiredTime;	///< 过期时间

	unsigned int m_nBuffCount; // 不包括经验buff
	int m_BuffId[MAX_TEAM_BUFF]; // 保存,全部保存共享buff(不包含组队经验buff)
	std::tuple<unsigned int,int,int,int,unsigned int> m_CallInfo; //召唤者id，场景id，x, y, 过期时间
};
