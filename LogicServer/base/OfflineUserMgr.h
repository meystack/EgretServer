#pragma once

/*
*离线玩家的消息管理器，将一些排名前面的玩家的信息放到组件里保存，用于玩家查询，可以缓存离线玩家的装备，基础信息
*以及宝物，宠物这些数据
需要同步的接口
OnOfflineSynchronizationData
OnLoadActorOther
*/
#include <map>
#include <vector>
#include <functional>

class CActor;

/*
*英雄的离线消息
*/
struct COfflineUserHero:
	public HeroOfflineMsg
{
	CEquipVessel	equips;  //宠物的装备
};

struct OfferLog
{
	unsigned int nActorID;			//对方玩家ID
	ACTORNAME sName;		//对方玩家NAME
	char sItemName[32];		//物品名
	int  nItemId;			//物品ID
	int  nColor;			//物品配置(0-白色，5-红色)
	char sLog[100];			//操作logs
};

//离线玩家的信息管理器
struct COfflineUser:
	public ActorOfflineMsg
{
	CUserItem*	m_Equips[itMaxEquipPos];	//装备信息
	std::vector<CSkillSubSystem::SKILLDATA> m_Skilldata;// 技能信息
	std::map<int, std::vector<StrengthInfo> > m_nStrengths;// 强化信息
	std::vector<CNewTitleSystem::NEWTITLEDATA> m_NewTitleDataList;// 称号信息
	std::vector<CAlmirahItem> m_Almirah;	//时装信息
	std::vector<GhostData> m_ghost;	//神魔信息
	std::vector<SoulWeaponNetData> vSoulWeapon;	//兵魂信息
	std::vector<CLootPetSystem::LOOTPETDATA> m_LootPetSystemDataList;//宠物系统非 ai宠物
	std::vector<CNewTitleSystem::NEWTITLEDATA> m_CustomTitleDataList;// 称号信息
};

class COfflineUserMgr:
	public CComponent
{
	
public:

	COfflineUserMgr():m_FixedSizeAllocator(_T("OfflineUserAllocator")),
		m_OfflineUserHeroAllocator(_T("OfflineUserHeroAllocator"))
	{

	}

	typedef std::map<unsigned int,COfflineUser* > OfflineUserMap;			//玩家数据查找的迭代器
	typedef std::map<char, COfflineUserHero * > OfflineUserHeroMap;	//英雄查找迭代
	typedef std::map<unsigned int, OfflineUserHeroMap* > OfflineUserHerosMap;	//英雄列表查找迭代
	typedef std::map<unsigned int,BYTE > NeedBuyTypeMap;	//[求购店]物品求购类型字段
	/*
	* Comments:添加一个离线玩家的列表
	* Param unsigned int nActorId:玩家的actorid
	*Parambool &isExist 是否存在这个玩家
	* @Return COfflineUser *:返回玩家的指针
	*/
	COfflineUser * AddOffLineUser(unsigned int nActorId,bool &isExist);

	/*
	* Comments:添加一个离线玩家的英雄列表
	* Param unsigned int nActorId:玩家的actorid
	* Param unsigned int nHeroId:玩家英雄的nHeroId
	*Parambool &isExist 是否存在这个玩家
	* @Return COfflineUserHero *:返回玩家英雄的指针
	*/
	COfflineUserHero * AddOffLineUserHeros(unsigned int nActorId, unsigned int nHeroId, bool &isExist);
	//里显示同步数据
	void OnOfflineSynchronizationData(CActor* pActor);

	//添加一个英雄到英雄列表
	COfflineUserHero * AddOffLineUserHero(unsigned int nActorId, unsigned int nHeroId);
	
	//DB返回数据了
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);

	//topackt
	void ViewOffLineActorToPacket(unsigned int nActorId, CDataPacket& data);
	/*
	* Comments:通过玩家的actorid返回玩家的指针
	* Param unsigned int nActorId:玩家的actorid
	* Param  bool loadWhenNotExist:如果不存在数据就去DB装载
	* @Return COfflineUser *:玩家的指针
	*/
	inline COfflineUser * GetUserByActorId(unsigned int nActorId)
	{
		if(nActorId ==0) return NULL;
		OfflineUserMap::iterator iter = m_actoridMap.find(nActorId);
		if (iter == m_actoridMap.end())
		{
			return NULL;		
		}
		return iter->second;
	}

	/*
	* Comments:通过玩家的actorid返回玩家的指针
	* Param unsigned int nActorId:玩家的actorid
	* Param unsigned int nHeroId:玩家英雄的nHeroId
	* Param  bool loadWhenNotExist:如果不存在数据就去DB装载
	* @Return COfflineUserHero *:玩家的英雄指针
	*/
	inline COfflineUserHero * GetUserHero(unsigned int nActorId, unsigned int nHeroId, bool &isExist, bool loadWhenNotExist=false)
	{
		if(nActorId == 0 || nHeroId == 0) return NULL;
		OfflineUserHerosMap::iterator iter = m_actoridHerosMap.find(nActorId);
		if (iter != m_actoridHerosMap.end())
		{
			OfflineUserHeroMap *pHeros = iter->second;
			isExist = true;
			OfflineUserHeroMap::iterator it = pHeros->find((char)nHeroId);
			if (it != pHeros->end())
			{
				return it->second;
			}

		}
		else
		{
			isExist = false;
		}

		if (loadWhenNotExist)
		{
			LoadUserHero(nActorId, nHeroId);
		}
		return NULL;
	}

	/*
	* Comments:删除玩家的列表
	* Param unsigned int nActorID: 玩家的actorid
	* @Return void:
	*/
	void RemoveUser(unsigned int nActorID);

	//删除一个英雄
	void RemoveUserHero(unsigned int nHeroId, OfflineUserHeroMap *pHeros);


	//删除了所有的玩家的信息
	void RemoveAllUser();
	
	//删除了所有的玩家英雄的信息
	void RemoveAllUserHero();

	//销毁组件
	virtual VOID Destroy()
	{
		RemoveAllUser();
		RemoveAllUserHero();
	}

	//初始化
	bool Initialize()
	{
		return true;
	}

	using CallBack = std::function<void(CActor*)>;

	//装载玩家的离线消息
	void LoadUser(unsigned nActorId, unsigned nViewerId=0, int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);

	//装载玩家英雄的离线消息
	void LoadUserHero(unsigned nActorId, unsigned int nHeroId);

	//查看离线玩家的消息
	void ViewActor(CActor *pActor,char *name);
	
	//通过名字查询玩家的指针
	COfflineUser * GetUserByName(const char * name);

	void viewByOfflineUser(COfflineUser *pUser, unsigned int nActorId, int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);
	void viewByOnlineUser(CActor *pUser, CActor *pActor,int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);

	//Comments:查看离线玩家的信息 优先使用actorid 如果actorid==0 则使用name
	void ViewOffLineActor(CActor *pActor, unsigned int nActorId, int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);

	//查看离线英雄信息
	void ViewOffLineActorHero(CActor *pActor, unsigned int nActorId, unsigned int nHeroId, bool loadWhenNotExist=true, unsigned int nShowType=0);

	void viewOnlineUserData(CActor* pUser, CDataPacket& data);

	void viewOfflineUserData(COfflineUser *pUser, CDataPacket& data);
public:

	//装载玩家的基本信息
	void OnLoadActorBasic(unsigned int nActorId, CDataPacketReader &reader);
	//装载玩家的装备信息
	void OnLoadActorOther(unsigned int nActorId, CDataPacketReader &reader);

	//装载英雄数据
	void OnLoadActorHeroBasic(unsigned int nActorId, CDataPacketReader &reader);
	//装载英雄装备数据
	void OnLoadActorHeroEquips(unsigned int nActorId, CDataPacketReader &reader);

public:
	/*
	* Comments:发送悬赏任务日志
	* @Return void:
	*/
	void SendOfferLog(CActor* pActor);

	LPCSTR GetUserName(unsigned int nActorId);

public:
	VOID AddNeedBuyType(unsigned int nItemId, BYTE nBuyType)
	{
		m_NeedBuyTypeMap.insert( std::make_pair(nItemId, nBuyType) );
	}

	BYTE GetNeedBuyType(unsigned int nItemId)
	{
		if( nItemId == 0 ) return 0;
		NeedBuyTypeMap::iterator iter = m_NeedBuyTypeMap.find(nItemId);
		if (iter == m_NeedBuyTypeMap.end())
		{
			return 0;		
		}
		return iter->second;
	}

private:

	OfflineUserMap  m_actoridMap;			//玩家的actorid到数据的索引（每天23点55分左右会重置）
	OfflineUserHerosMap m_actoridHerosMap;	//英雄列表map
	std::map<unsigned int, std::vector<std::function<void(COfflineUser*)>>> m_noticeMap; //通知的列表
	std::map<unsigned int, std::vector<std::function<void(COfflineUser*)>>> m_noticeCenterMap; //通知的列表
	CSingleObjectAllocator<COfflineUser> m_FixedSizeAllocator;
	CSingleObjectAllocator<COfflineUserHero> m_OfflineUserHeroAllocator;
	
	NeedBuyTypeMap m_NeedBuyTypeMap;
public:
	//跨服相关
	void ViewCenterOffLineActor(int nSrvId, unsigned int nActorId);
	void SendCenterOfflineActor(COfflineUser* pUser,int nSrvId);
	void SendCenterOnlineUser(CActor* pUser,int nSrvId);
	void viewCenterOnlineUserData(CActor* pUser, CDataPacket& data);
};
