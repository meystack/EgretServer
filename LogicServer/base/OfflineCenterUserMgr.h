#pragma once

/*
*跨服离线玩家的消息管理器，将一些排名前面的玩家的信息放到组件里保存，用于玩家查询，可以缓存离线玩家的装备，基础信息
*以及宝物，宠物这些数据
需要同步的接口
OnLoadActorOther
*/
#include <map>
#include <vector>
#include <functional>

class CActor;

//离线玩家的信息管理器
struct COfflineCenterUser:
	public ActorOfflineMsg
{
	CUserItem*	m_Equips[itMaxEquipPos];	//装备信息
	std::vector<CSkillSubSystem::SKILLDATA> m_Skilldata;// 技能信息
	std::map<int, std::vector<StrengthInfo> > m_nStrengths;// 强化信息
	std::vector<CNewTitleSystem::NEWTITLEDATA> m_NewTitleDataList;// 称号信息
	std::vector<CAlmirahItem> m_Almirah;	//时装信息
	std::vector<GhostData> m_ghost;	//神魔信息
	std::vector<SoulWeaponNetData> vSoulWeapon;	//兵魂信息
};

class COfflineCenterUserMgr:
	public CComponent
{
	
public:

	COfflineCenterUserMgr():m_FixedSizeAllocator(_T("OfflineCenterUserAllocator"))
	{

	}

	typedef std::map<unsigned int,COfflineCenterUser* > OfflineUserMap;			//玩家数据查找的迭代器
	/*
	* Comments:添加一个离线玩家的列表
	* Param unsigned int nActorId:玩家的actorid
	*Parambool &isExist 是否存在这个玩家
	* @Return COfflineCenterUser *:返回玩家的指针
	*/
	COfflineCenterUser * AddCenterOffLineUser(unsigned int nActorId,bool &isExist);
	//处理跨服玩家 数据返回
	void OnCenterReturnData(CDataPacketReader& reader);

	//topackt
	void ViewOffLineActorToPacket(unsigned int nActorId, CDataPacket& data);
	/*
	* Comments:通过玩家的actorid返回玩家的指针
	* Param unsigned int nActorId:玩家的actorid
	* Param  bool loadWhenNotExist:如果不存在数据就去DB装载
	* @Return COfflineCenterUser *:玩家的指针
	*/
	inline COfflineCenterUser * GetUserByActorId(unsigned int nActorId)
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
	* Comments:删除玩家的列表
	* Param unsigned int nActorID: 玩家的actorid
	* @Return void:
	*/
	void RemoveUser(unsigned int nActorID);


	//删除了所有的玩家的信息
	void RemoveAllUser();

	//销毁组件
	virtual VOID Destroy()
	{
		RemoveAllUser();
	}

	//初始化
	bool Initialize()
	{
		return true;
	}

	using CallBack = std::function<void(CActor*)>;

	//装载玩家的离线消息
	void LoadCenterUser(unsigned nActorId, unsigned nViewerId=0, int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);
	void viewCenterByOfflineUser(COfflineCenterUser *pUser,unsigned int nActorId,int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);

	//Comments:查看离线玩家的信息 优先使用actorid 如果actorid==0 则使用name
	void ViewCenterOffLineActor(CActor *pActor, unsigned int nActorId, int nsys = enEuipSystemID,int ncmd = sGetOtherEqip);

	void viewCenterOfflineUserData(COfflineCenterUser *pUser, CDataPacket& data);
public:

	//装载玩家的装备信息
	void OnLoadCenterActorData(unsigned int nActorId, CDataPacketReader &reader);
	void SetCenterOfflineUserData(CDataPacketReader& data);

private:

	OfflineUserMap  m_actoridMap;			//玩家的actorid到数据的索引（每天23点55分左右会重置）
	std::map<unsigned int, std::vector<std::function<void(COfflineCenterUser*)>>> m_noticeMap; //通知的列表
	CSingleObjectAllocator<COfflineCenterUser> m_FixedSizeAllocator;
};
