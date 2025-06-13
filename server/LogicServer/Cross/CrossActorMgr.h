#ifndef CROSSACTOR_MGR_H_
#define CROSSACTOR_MGR_H_
#include <map>
#include <mutex>
#define CROSS_SERVER_CAL 10000
/*
/*****************************************************************
            Copyright (c) 2021, 上海漫方网络科技有限公司
                    All rights reserved
       
    创建日期：  2021年04月28日 14时01分
    文件名称：  CrossServer.h
    说    明：  传送管理器。用于管理玩家从一个服务器传送到另外一个服务器的操作。包括玩家发起传送的数据打包、
	传送到目的地的数据解包、公共服务器发给普通服务器的数据定时存盘数据处理等等
	源逻辑服务器：											目标服务器					
		打包并发送角色基本数据和子系统数据		 接收到用户数据，先缓存起来。如果客户端连接上
												就创建角色，并且初始化角色数据。如果客户端超时还
												没有连接上，就关闭用户会话，删除角色缓存数据对象
    
    当前版本：  1.00
    作    者：  
    概    述： 	传送管理器类
*/

// 角色传送缓存数据。传送目的方接收到数据先缓存起来

typedef struct tagCrossDataGate
{
	int nGateIndex;//网关的编号
	Uint64  lLogicKey;              //逻辑初始化的key，用于查找
}CROSSDATAGATE;

struct CSCUserGameSet
{
	GAMESETSDATA        m_GsData;
	GAMESETSDATA2       m_GsData2;
};

//跨服数据
struct CSCUserData:
	public ACTORDBDATA
{
	std::vector<CUserItem>	m_Equips;	//装备信息
	CSCUserGameSet   m_sGameSet; //游戏设置
	std::vector<CSkillSubSystem::SKILLDATA> m_Skilldata;// 技能信息
	std::map<int, std::vector<StrengthInfo> > m_nStrengths;// 强化信息
	std::vector<CNewTitleSystem::NEWTITLEDATA> m_NewTitleDataList;// 称号信息
	std::vector<CReviveDurationSystem::ReviveDurationDATA> m_ReliveDataList;// 称号信息
	std::vector<CAlmirahItem> m_Almirah;	//时装信息
	std::vector<GhostData> m_ghost;	//神魔信息
	std::vector<SoulWeaponNetData> vSoulWeapon;	//兵魂信息
	std::vector<CUserItem> vUseBags;	//背包
	int nReliveState;//复活数据
	std::vector<CLootPetSystem::LOOTPETDATA> vLootPets;//宠物系统非 ai宠物
	std::vector<CNewTitleSystem::NEWTITLEDATA> m_CustomTitleDataList;// 称号信息
	GameUserDataOther otherData;
};


namespace CCROSSDATA
{
	/*---------------------------------------------------------
	/*     跨服服务器管理类
	/*	   处理跨服消息
	---------------------------------------------------------*/
	class CActorCrossCacheData
	{
	public:
		CActorCrossCacheData() : m_nDataType(-1), m_packet(0){}
		CActorCrossCacheData(int nType, CDataPacketReader *packet){
			m_nDataType = nType;
			m_packet = packet;
		}

	// ~CActorCrossCacheData()
	// {
	// 	if(m_packet)
	// 		SafeDelete(m_packet);
	// 	m_packet = nullptr;
	// }
		int GetDataType() const { return m_nDataType; }
	public:
		int				m_nDataType;		// 角色数据类型
		CDataPacketReader* 	m_packet;			// 数据包	
	};
	/*---------------------------------------------------------
	/*     跨服服务器管理类
	/*	   处理跨服消息
	---------------------------------------------------------*/
	class CActorCrossCache
	{
	public:

		CActorCrossCache()
		{
			m_nActorID	= 0;
			m_bAllDataReady = false;
			m_nCrossActorId = 0;
			m_nCrossSrvId = 0;
			m_nCsLoadStep = 0;
			// m_cache.reserve(MSG_CS_END);
		}

		unsigned int GetActorId()
		{
			return m_nActorID;
		}

		void SetActorId(unsigned int nActorID)
		{
			m_nActorID = nActorID;
		}

		

		void SetCrossActorId(unsigned int nActorID)
		{
			m_nCrossActorId = nActorID;
		}

		unsigned int GetCrossActorId()
		{
			return m_nCrossActorId;
		}

		unsigned int GetCrossSrvId()
		{
			return m_nCrossSrvId;
		}

		void SetCrossSrvId(unsigned int srvId)
		{
			m_nCrossSrvId = srvId;
		}

		unsigned int GetAccountId()
		{
			return m_nAccountID;
		}

		void SetAccountId(unsigned int nAcctountId)
		{
			m_nAccountID = nAcctountId;
		}
		//设定玩家初始化数据保存超时时间
		void SetTimeOut(int nTime)
		{
			m_nInitTimeOut = nTime;
		}
		
		/*
		* Comments: 缓存角色传送数据
		* Param int nType: 数据类型。例如技能数据、背包物品数据等等
		* Param CDataPacket * reader: 数据包内容
		* @Return void:
		* @Remark:
		*/
		void CacheData(int nType, CDataPacketReader &reader);
		// {
		// 	// CActorCrossCacheData *pCacheData = GetCacheData(nType);
		// 	// if (pCacheData) {
		// 	// 	if(pCacheData->m_packet) {
		// 	// 		// (CDataPacket*)pCacheData->m_packet->~CDataPacket();
		// 	// 	}
		// 	// 	pCacheData->m_packet = packet;
		// 	// }
		// 	// else {
		// 	// 	// 添加一个新的
		// 	// 	CActorCrossCacheData cacheData(nType, packet);				
		// 	// 	m_cache.add(cacheData);
		// 	// 	m_nCsLoadStep++;
		// 	// 	if(m_nCsLoadStep == MSG_CS_END) {
		// 	// 		m_bAllDataReady = true;
		// 	// 	}
		// 	// }
		// 	int nCount =0;
		// 	if (nType == MSG_CSKILL) {
		// 		{
		// 			CSkillSubSystem::SKILLDATA  data;
		// 			reader >> nCount;
		// 			// CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
		// 			for(INT_PTR i=0; i < nCount; i++)
		// 			{
		// 				reader.readBuf(&data,sizeof(ONESKILLDBDATA) );
		// 				// data.bEvent =0;
		// 				// data.bMjMask =0;
		// 				// const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
		// 				// if(pConfig ==NULL) continue;
						
		// 				// data.bIsPassive = ((pConfig->nSkillType == stPassiveSkill)?1:0);
		// 				// data.bPriority = pConfig->bPriority;
		// 				// data.nSkillType = pConfig->nSkillType;

		// 				m_pUserData.m_Skilldata.push_back(data);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CACTOR) {
		// 		{
		// 			tagActorDbData msg;
		// 			if (reader.getAvaliableLength() < sizeof(tagActorDbData))
		// 			{
		// 				return;
		// 			}
					
		// 			reader >> msg;
		// 			memcpy(&m_pUserData,&msg,sizeof(msg));
		// 		}
		// 	}else if (nType == MSG_CGAMESET) {
		// 		{
		// 			reader.readBuf(&m_pUserData.m_sGameSet.m_GsData, sizeof(GAMESETSDATA));
		// 			reader.readBuf(&m_pUserData.m_sGameSet.m_GsData2, sizeof(GAMESETSDATA2));
		// 		}
		// 	}else if (nType == MSG_CEQUIP) {
		// 		{
		// 			CUserItem userItem;
		// 			reader >> nCount; //装备的数目
		// 			for (INT_PTR i=0 ; i < nCount; i++ )
		// 			{
		// 				userItem << reader;

		// 				m_pUserData.m_Equips.emplace_back(userItem);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CTITLE) {
		// 		// 加载称号信息
		// 		{
		// 			reader >> nCount;
		// 			for(INT_PTR i=0; i < nCount; i++)
		// 			{
		// 				unsigned int nValue = 0,  nTime = 0;
		// 				reader >> nValue >> nTime;
		// 				WORD wId = LOWORD(nValue);
		// 				CNewTitleSystem::NEWTITLEDATA newTitleData;
		// 				newTitleData.wId = wId;
		// 				newTitleData.nRemainTime = nTime;
		// 				m_pUserData.m_NewTitleDataList.push_back(newTitleData);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CFASH) {
		// 		{
		// 			reader >> nCount;
		// 			CAlmirahItem oneItem;
		// 			for(INT_PTR i=0; i < nCount; i++)
		// 			{
		// 				reader >> oneItem.nModelId;		
		// 				reader >> oneItem.nLv;
		// 				reader >> oneItem.btTakeOn;
		// 				m_pUserData.m_Almirah.push_back(oneItem);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CSTRENGTH) {
		// 		// 加载强化信息
		// 		{
		// 			reader >> nCount;
		// 			for(INT_PTR i=0; i < nCount; i++)
		// 			{
		// 				int nType = 0; 
		// 				StrengthInfo it;
		// 				reader >> nType >>it.nId >> it.nLv;
		// 				if(it.nId > 0)
		// 					m_pUserData.m_nStrengths[nType].push_back(it);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CGHOST) {
		// 		{
		// 			reader >> nCount;
		// 			GhostData oneItem;
		// 			for(INT_PTR i=0; i < nCount; i++)
		// 			{
		// 				reader >> oneItem.nId;		
		// 				reader >> oneItem.nLv;
		// 				reader >> oneItem.nBless;
		// 				m_pUserData.m_ghost.push_back(oneItem);
		// 			}
		// 		}
		// 	}else if (nType == MSG_CHALIDOME) {
		// 		// 加载圣物兵魂
		// 		{
		// 			reader >> nCount;

		// 			for(INT_PTR i = 0; i < nCount; i++)
		// 			{  
		// 				SoulWeaponNetData oneItem;
		// 				oneItem.nId = i + 1;	
		// 				reader >> oneItem.nLorderId >> oneItem.nStarId >> oneItem.nLvId; 
		// 				reader.readString(oneItem.cBestAttr); 
		// 				oneItem.cBestAttr[sizeof(oneItem.cBestAttr)-1] = 0;
		// 				m_pUserData.vSoulWeapon.push_back(oneItem);
		// 			}
		// 		}
		// 	}
		// 	m_nCsLoadStep++;
		// 	if(m_nCsLoadStep == MSG_CS_END) {
		// 		m_bAllDataReady = true;
		// 	}
		// }

		/*
		* Comments: 获取指定类型的数据
		* Param const int nType:
		* @Return CActorCrossCacheData*:
		* @Remark:
		*/
		CActorCrossCacheData* GetCacheData(const int nType) {

			// for (INT_PTR i = 0; i < m_cache.count(); i++) {

			// 	if (nType == m_cache[i].GetDataType())
			// 		return &m_cache[i];
			// }
			
			return NULL;
		}
		/*
		* Comments: 初始化
		* Param const int nType:
		* @Return CActorCrossCacheData*:
		* @Remark:
		*/
		void InitActorCache();
		/*
		* Comments: 初始化
		* Param const int nType:
		* @Return CActorCrossCacheData*:
		* @Remark:
		*/
		void InitActorSystemCache();

		/*
		* Comments: 设置全部数据缓存完毕标记
		* Param bool flag:
		* @Return void:
		* @Remark:
		*/
		void SetAllDataReady(bool flag)
		{
			m_bAllDataReady = flag;
		}
		void SetReadyInit(bool flag)
		{
			m_bReadyInit = flag;
		}
		//设置步长
		void SetStep(int flag)
		{
			m_nCsLoadStep = flag;
		}


	public:
		unsigned int						m_nActorID;		    // 该角色的登录账号ID
		unsigned int						m_nCrossActorId;	// 跨服账号id
		unsigned int						m_nCrossSrvId;		// 跨服srvid
		unsigned int						m_nAccountID; 			// 账号的ID
		bool								m_bAllDataReady;	// 所有的传送数据都缓存OK
		bool                                m_bReadyInit;//是否需要初始化
		int                                 m_nCsLoadStep;//加载步骤
		int                                 m_nInitTimeOut;//初始化超时时间
		// CVector<CActorCrossCacheData>		m_cache;
		CSCUserData                         m_pUserData;//跨服数据                         
	};

	// 同一个服务器内的所有角色数据
	class CServerActorInfo
	{
	public:
		CServerActorInfo() {
			m_nStaticStartId = 0;
			m_actorDBData.clear();
			m_AllWaitInits.clear();
		}

		~CServerActorInfo() {
			if(m_actorDBData.size() > 0)
			{
				std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.begin();
				for (;iter != m_actorDBData.end(); iter++)
				{

					if(iter->second) {
						delete iter->second;
					}
					m_actorDBData.erase(iter);
				}
			}
		}
		/*
		* Comments: 获取指定角色ID的Cache数据对象
		* Param int nActorId:
		* @Return CActorCrossCache*:
		* @Remark:
		*/
		
		CActorCrossCache* GetActorCache(unsigned int nActorId)
		{
			std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.find(nActorId);
			if (iter != m_actorDBData.end())
				return iter->second;

			return NULL;
		}

		CActorCrossCache* GetActorCacheByAccountId(unsigned int nAccountId)
		{
			std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.begin();
			std::map<int, CActorCrossCache*>::iterator iter_end = m_actorDBData.end();
			for (; iter != iter_end; ++iter)
			{
				if (iter->second->GetCrossActorId() == nAccountId)
					return iter->second;
			}

			return NULL;
		}

		/*
		* Comments: 添加一个角色Cache数据
		* Param int nActorId:
		* @Return CActorCrossCache*: 返回此角色的CaChe对象指针
		* @Remark:
		*/
		CActorCrossCache* AddActorCache(unsigned int nActorId)
		{
			CActorCrossCache *pCache = GetActorCache(nActorId);
			if (pCache != NULL)
				return pCache;

			pCache = new CActorCrossCache();
			m_actorDBData[nActorId] = pCache;
			return pCache;
		}
				
		/*
		* Comments: 删除一个角色Cache数据
		* Param int nActorId:
		* @Return void:
		* @Remark:
		*/
		void RemoveActorCache(unsigned int nActorId)
		{
			std::map<int, CActorCrossCache*>::iterator iter = m_actorDBData.find(nActorId);
			if (iter != m_actorDBData.end())
			{

				if(iter->second) {
					delete iter->second;
				}
				m_actorDBData.erase(iter);
			}
		}

		/*
		* Comments: 踢出所有的玩家
		* @Return void:
		* @Remark:
		*/
		void CloseAllActor();

		/*
		* Comments: 例行更新角色传送缓存对象
		* Param TICKCOUNT nCurrTick:
		* @Return void:
		* @Remark:
		*/
		void RunOne(TICKCOUNT nCurrTick);
		//获取id
		int getStaticActorId(int nSrvId, int nActorId)
		{
			if(m_nStaticStartId == 0)
				m_nStaticStartId = nSrvId;
			if(m_AllActorId.find(nActorId) == m_AllActorId.end())
			{
				++m_nStaticStartId;
				m_AllActorId[nActorId] = m_nStaticStartId;
			}
			
			return m_AllActorId[nActorId];
		}

	public:
		std::map<int, CActorCrossCache*>		m_actorDBData;	// 角色Id --> 传送缓存数据
		std::map<int, tagCrossDataGate>         m_AllWaitInits; //等待初始化列表
		std::map<int, int>         m_AllActorId; //actorid -> nCrossActorid
		int  m_nStaticStartId = 0;
	};
}

/*---------------------------------------------------------
/*     跨服服务器管理类
/*	   处理跨服消息
---------------------------------------------------------*/
class CCrossMgr
{
public:
	enum
	{
		SERVER_INDEX_MAX = 1024,				// 最大的ServerIndex
	};
	CCrossMgr() :m_FixedSizeAllocator(_T("CCSUserDataAllocator")){
		m_ServerActorInfo.clear();
		m_LoginActors.clear();
	}
	~CCrossMgr() {
	}

	/*---------------------------------------------------------------------
	----- 跨服处理子服的消息
	---------------------------------------------------------------------*/
	/*
	* Comments: 接收逻辑服务器的传送相关消息
	* Param int nMsgId: 消息Id
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark:
	*/
	void OnRecvLogicMessage(int nMsgId, CDataPacketReader& packet);

	void GetCrosServersActorId(CDataPacketReader& packet);
	/*
	* Comments: 接受逻辑服玩家登录数据
	* Param CDataPacketReader & reader: 消息数据
	* @Return bool:
	* @Remark: 调用此函数接收处理角色传送各个模块的数据，这里接收到数据处理类似于角色登录从DB中加载数据处理
	*/
	bool AtkLoginData(CDataPacketReader& packet);

	/*
	* Comments: 处理跨服玩家请求的跨服唯一标识id
	* Param CDataPacketReader & reader: 消息数据
	* @Return bool:
	*/
	bool dealCrossLogin(CDataPacketReader& packet);

	/*
	* Comments:关闭玩家连接
	* Param int nActorId: 跨服玩家在跨服的唯一id
	* @Return bool:
	*/
	void CloseActor( unsigned int nCrossActorId);

	/*
	* Comments:客户端请求登录设置当前跨服玩家登录状态，用于初始化玩家数据
	* Param int nSrvId: 跨服区服id
	* Param int nCrossActorId: 跨服玩家在跨服的唯一id
	* @Return void:
	*/
	void LoginSetState(int nSrvId, unsigned int nCrossActorId);
	//初始化子系统
	void InitActorSystem(int nSrvId, unsigned int nCrossActorId);

	/*---------------------------------------------------------------------
	----- 子服处理跨服的消息
	---------------------------------------------------------------------*/
	/*
	* Comments: 接收跨服服务器的相关消息
	* Param int nMsgId: 消息Id
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark:
	*/
	void OnRecvCrossMessage(int nMsgId, CDataPacketReader& packet);

	/*
	* Comments:处理跨服服务器发送的奖励
	* Param CDataPacketReader & reader: 消息数据
	* @Return void:
	*/
	void AtkCorssServerMail(CDataPacketReader &reader);

	/*
	* Comments: 与连接的对端逻辑服务器断开时调用此函数。
	* Param int nServerIdx:
	* @Return void:
	* @Remark:对于普通逻辑服务器而言，在断开与公共服务器的连接时，如果有玩家已经传送到公共服务器，需要通知
			SessionServer断开此玩家的会话；对于公共服务器而言，如果与某个普通逻辑服务器断开，那么需要将
			所有此逻辑服务器的玩家踢下线。
	*/
	void OnLogicServerDisconnected(int nServerIdx);

	/*-------------------------------
	---- 通用接口
	---------------------------------*/
	/*
	* Comments: 获取玩家在跨服唯一标识id
	* Param int nCrossSrvId:
	* @Return void:
	* @Remark:
	*/
	int GetActorCrossActorId(int nCrossSrvId, int nActorId);

	/*
	* Comments: 管理器例行处理
	* Param TICKCOUNT nCurrTick:
	* @Return void:
	* @Remark:
	*/
	void RunOne(TICKCOUNT nCurrTick);

	/*
	* Comments: 发送消息给连接的逻辑服务器
	* Param void * pData:
	* Param SIZE_T size:
	* Param int nServerIdx: 目标逻辑服务器的ServerIndex
	* @Return void:
	* @Remark:
	*/
	void SendMessage(void *pData, SIZE_T size, int nServerIdx);

	/*
	* Comments: 获取角色所在的原生服务器ServerIndex
	* Param CActor * pActor:
	* @Return int:
	* @Remark:
	*/
	int GetActorRawServerIndex(CActor *pActor);

	/*
	* Comments: 请求将玩家传送到跨服服务器并缓存
	* Param CActor * pActor: 角色指针
	* @Return bool:
	* @Remark: 此函数发送消息给跨服服务器
	*/
	bool SendLoginData(CActor *pActor);

	/*
	* Comments: 设置玩家区服
	* Param int nCrossActorId: 角色id
	* @Return bool:
	* @Remark: 此函数发送消息给跨服服务器
	*/
	void AddLoginActors(int nCrossActorId, int nCSSrvID)
	{
		std::lock_guard<std::mutex> lock(m_nlock);
		m_LoginActors[nCrossActorId] = nCSSrvID;
	}

	int  GetLoginActorCSSrvId(int nCrossActorId)
	{
		std::lock_guard<std::mutex> lock(m_nlock);
		if(m_LoginActors.size()) {
			if(m_LoginActors.find(nCrossActorId) != m_LoginActors.end())
				return m_LoginActors[nCrossActorId];
		}
		return 0;
	} 
	//添加玩家到初始化等待列表
	void OnAddWaitInitList(int nActorCSSrvid, unsigned int nCrossActorId, int nGateIndex, Uint64 lLogicKey);
	void GetServerActorInfo(int nSrvId, CCROSSDATA::CServerActorInfo* actorinfo);

	//活得聊天信息
	bool OnGetChatData(CDataPacketReader& packet);

	//处理db数据
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);
	
	void KickAllCrossServerActor();
protected:
	//用于缓存传送到本服务器的玩家的打包数据
	std::map<int, CCROSSDATA::CServerActorInfo>	m_ServerActorInfo;	
	std::map<int, int>	m_LoginActors;	
	std::mutex	m_nlock;
	int	 m_nCrossStartActorId; //跨服玩家id 初始值
	CSingleObjectAllocator<CSCUserData> m_FixedSizeAllocator;
	// static uint32_t             m_nStartActorId;
	// 对于普通逻辑服务器，还需要记录传送到公共服务器的玩家的信息，包括账号ID、角色ID、玩家上线时间，用于
	// 定时数据存盘的检测以及与公共服务器断开连接时关闭此部分玩家会话用
	
	CTimer<30000>			m_t1minute ; 			// 1分定时器
	CTimer<5000>			m_t5 ; 		 
};
#endif //CROSSACTOR_MGR_H_