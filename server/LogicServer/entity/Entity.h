
#pragma once

/***************************************************************/
/*
/*                     实体
/*
/***************************************************************/

class CObserverSystem;
class CMovementSystem;
class CBuffSystem;
class CUserBag;
class CQuestSystem;
class CAnimal;
//class CEntitySystem;
#include <list>
// 实体脚本回调参数结构定义
#define MAX_SCRIPT_FUN_NAME_LEN						40

// 实体传送类型
enum EntityTransferStype
{
	etsTeleport,			// 通过传送门传送
	etsOther,				// 其他方式传送
};

class OwnedBabyInfo
{
public:
	EntityHandle m_handle;				// 宝宝句柄
	bool		 m_bWatch;				// 是否监控此宝宝。宝宝被监控后，属性变化会同步到客户端	
};

class ScriptCallbackParam : public Counter<ScriptCallbackParam>
{
public:
	ScriptCallbackParam()
	{		
		init(0, 0, 0, 0, 0, "", 0);
	}

	void init (unsigned int handle, unsigned int nNpdId, unsigned int nDelay, unsigned int nInterval, int nCount, const char *pszName, const CScriptValueList *paramList)
	{
		if (!pszName || strlen(pszName) > MAX_SCRIPT_FUN_NAME_LEN)
			return;
		
		m_nHandle		= handle;
		m_nNpcId		= nNpdId;
		m_nDelayTime	= nDelay;
		m_nInterval		= nInterval;
		m_nCount		= nCount;			
		_asncpyt(m_szScriptFn, pszName);	
		if (paramList)
		{	
			m_paramList = *paramList;
		}		
	}
	
	unsigned int		m_nNpcId;								// 目标NPCID
	unsigned int		m_nHandle;								// 事件句柄
	unsigned int		m_nDelayTime;							// 第一次执行的延迟时间(ms为单位）
	unsigned int		m_nInterval;							// 两次执行间隔时间(ms为单位)
	int					m_nCount;								// 执行次数
	TICKCOUNT			m_nNextHitTime;							// 下一次执行的触发时间	
	CScriptValueList	m_paramList;							// 脚本参数列表
	char m_szScriptFn[MAX_SCRIPT_FUN_NAME_LEN+1];		// 回调函数名称	
};



//tolua_begin
class CEntity
{
public:

	CEntity()
		: m_hTest(Uint64(-1))
		, m_hEntityHandler(0)
		, m_targetHandler(0)
	{
		m_sEntityName[0] = 0;
		m_entityPos.nSceneId = 0;
		m_entityPos.pFb = NULL;
		m_entityPos.pScene = NULL;
		m_isInited = false;
		m_sTitle[0] = 0;
		m_nTraceFlag = 0;
		m_nMaxFollowDistSqrt = 0;	
		m_bHasClearMarkedMsg = false;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		m_pNode =NULL;
#endif
		m_boIsDestroy = false;
		m_showName[0] = 0;
		m_race = 0;
		m_idx  = 0;
	}
	~CEntity ()
	{
		m_sEntityName[0]=0;
		m_isInited =false;
		m_entityPos.nSceneId = 0;
		m_entityPos.pFb = NULL;
		m_entityPos.pScene = NULL;
		m_sTitle[0] = 0;
		m_boIsDestroy = false;
	}

	//实体初始化
	virtual  bool Init(void * data, size_t size);

	//销毁实体
	virtual void Destroy();

	/*
	  逻辑线程调用一次刷新
	  */
	inline VOID RunOne(TICKCOUNT nCurrentTime)
	{
		BeforeLogicRun(nCurrentTime);
		LogicRun(nCurrentTime);
		AfterLogicRun(nCurrentTime);
	}
	//例行逻辑准备函数，当例行逻辑函数调用前被调用
	virtual VOID BeforeLogicRun(TICKCOUNT nCurrentTime);
	//例行逻辑处理函数
	virtual VOID LogicRun(TICKCOUNT nCurrentTime);
	//例行逻辑收尾函数，当例行逻辑函数调用后被调用
	virtual VOID AfterLogicRun(TICKCOUNT nCurrentTime);


	//获取属性集的指针
	virtual  CPropertySet* GetPropertyPtr() {return NULL;}
	virtual  const CPropertySet* GetPropertyPtr() const {return NULL;}

	//获取属性集数据部分的长度
	virtual  INT_PTR GetPropertySize() const =0;
	inline const char*  GetPropertyDataPtr() const
	{
		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return NULL;
		return set->GetValuePtr(0);
	}
	/*
		设置一个属性的值
		*nPropID：属性的id
		*value : 属性的值
	*/
	virtual void OnEntityDeath(); //当实体死亡的时候
	// nUpdateVal: 变化值
	virtual void OnLevelUp(int nUpdateVal){}; //升级的时候

	//向实体发送消息
	void _PostEntityMsg(const CEntityMsg &msg,LPCSTR file, INT_PTR line);
	#define PostEntityMsg(msg) _PostEntityMsg(msg,__FILE__, __LINE__)

	//删除实体消息
	INT_PTR DeleteEntityMsg(const INT_PTR nMsg);

	// 删除延迟删除消息
	void ClearDelayDelEntityMsg();

	//向实体发送消息，如果存在同消息号的消息则更新原有消息
	void UpdateEntityMsg(const CEntityMsg &msg);

	//处理实体消息
	virtual void ProcessEntityMsg(const CEntityMsg &msg);
	
	

	/*
	* Comments: 实体是否是死亡的
	* @Return : 如果是死亡的返回true，否则返回false
	*/
	inline bool IsDeath()
	{
		if(isAnimal())
		{
			return GetProperty<unsigned int>(PROP_CREATURE_HP) ==0;
		}
		else
		{
			return false;
		}
	}

	//获取动态变量
	inline CCLVariant& GetDynamicVar(){ return m_DynamicVar; }

	/*
	* Comments: 设置实体的等级，如果等级改变会触发等级改变的函数
	* Param unsigned int nLevel: 等级的数量，也就是实体的当前的等级，比如37
	* @Return VOID:
	*/
	VOID SetLevel(unsigned int nLevel);

	VOID SetIndex(int nIdx){ m_idx = nIdx; }
	int  GetIndex(){ return m_idx; }

	//当属性发生调用的时候使用
	//template < class T>
	//void OnPropertyChange(int nPropID, const T& oldValue, const T& value);


	void  OnPropertyChange(int nPropID, UINT64 oldValue, UINT64 value);

	/*
	* Comments: 改变实体的等级
	* Param int nValue:改变的值，比如升一级是1，降1级是-1
	* @Return VOID:
	*/
	inline VOID ChangeLevel(int nValue)
	{
		if(nValue ==0) return;
		nValue += GetProperty<unsigned int>(PROP_CREATURE_LEVEL );
		nValue = __max(0,nValue); //不能少于0
		SetLevel(nValue);
	}


	/*
		取一个属性的值
		*nPropID：属性的id
		返回属性的值
	*/

	virtual	 int GetPropertySetTotalSize()const =0;
	//virtual	 int GetPropertySetTotalSize()const {return 0;}

	

	template < class T> 
	inline  T GetProperty(int nPropID) const
	{
		T temp=0;

		//int totalSize = GetPropertySetTotalSize();
		//int offset = sizeof(CUpdateMask) + (nPropID << 2);
		//DbgAssert(offset < totalSize);

		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return temp;
		return *(T*)set->GetValuePtr(nPropID);
	}

	virtual char * GetForbitMask() {return CUpdateMask::s_forbitUpdateMask;}

	template < class T> 
	inline void SetProperty(int nPropID, const T& value)
	{
		T oldValue = GetProperty<T>(nPropID);
		if(oldValue == value )return;
		CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return ;
		#ifdef _DEBUG
		static const int ENTITYDATA_Size = sizeof(ENTITYDATA);
		static const int CREATUREDATA_Size = sizeof(CREATUREDATA);
		static const int MONSTERDATA_Size = sizeof(MONSTERDATA);
		int totalSize = GetPropertySetTotalSize();
		int maskSize = sizeof(CUpdateMask);
		int offset = (nPropID << 2);
		int realSize = totalSize - maskSize;
		assert(offset < realSize);
		#endif
		set->SetValue(nPropID,value,this->GetForbitMask());
		OnPropertyChange(nPropID,(UINT64)oldValue,(UINT64)value);
	}

	//基础属性的读取
	inline LPCSTR GetEntityName() const {return m_sEntityName;}

	void SetEntityName(const char * name);

	//设置、获取称号
	inline LPCSTR GetTitle() const {return m_sTitle;}

	inline void SetTitle(char * name)
	{
		if(name ==NULL) return;
		_asncpytA( m_sTitle,name );
	}

	inline VOID GetPosition(int &x, int &y) const 
	{
		x = GetProperty<int>(PROP_ENTITY_POSX);
		y = GetProperty<int>(PROP_ENTITY_POSY);
	}
	
	inline VOID GetPosition(INT_PTR &x, INT_PTR &y) const 
	{
		x = GetProperty<int>(PROP_ENTITY_POSX);
		y = GetProperty<int>(PROP_ENTITY_POSY);
	}
	
	bool getPosRangeByDir(INT_PTR &nX, INT_PTR &nY, INT_PTR &nDir, INT_PTR nRange) const;

	inline const ENTITYPOS& GetPosInfo() const {return m_entityPos; } 
	inline WORD GetSceneID() const { return m_entityPos.nSceneId; }
	inline CFuBen* GetFuBen() const {return m_entityPos.pFb;}
	inline CScene* GetScene() const {return m_entityPos.pScene; };

	inline void SetSceneID(WORD nSceneID) { m_entityPos.nSceneId = nSceneID; }
	inline VOID SetFuBen(CFuBen* pFb) { m_entityPos.pFb = pFb;}
	inline void SetPosInfo(ENTITYPOS & posInfo){m_entityPos = posInfo;}
	//获取实体所处的场景对象指针
	inline void SetScene(CScene* pScene) {m_entityPos.pScene = pScene;}
	inline void SetPosition(int x, int y)
	{
		SetProperty<int>(PROP_ENTITY_POSX,x);
		SetProperty<int>(PROP_ENTITY_POSY,y);
	}

	inline unsigned int GetId()const
	{
		return GetProperty<unsigned int>(PROP_ENTITY_ID);
	}
	
	inline INT_PTR GetType() const {return m_hEntityHandler.GetType();}
	
	inline const EntityHandle& GetHandle() const {return m_hEntityHandler;}
	
	//是指
	inline void SetHandle (EntityHandle hHandle)
	{
		m_hEntityHandler =hHandle; 
	}
	
	inline VOID SetInitFlag(bool flag){m_isInited = flag;}

	inline bool IsInited() const {return m_isInited ;}

	inline void SetDestroyFlag(bool boflag) {m_boIsDestroy = boflag;}
	inline bool IsDestory() const {return m_boIsDestroy;}

	inline virtual CUpdateMask * GetBroadCastMask() {return NULL;} //获取广播的mask

	//判断是否是非生物
	inline bool isNonLive()
	{
		switch(GetType())
		{
		case enTransfer:
		case enFire:
		case enDropItem:
			return true;
		}
		return false;
	}
	//判断是否是Animal
	inline bool isAnimal()
	{
		switch(GetType())
		{
		case enActor:
		case enPet:
		case enMonster:
		case enGatherMonster:
		case enHero:
			return true;
		}
		return false;
	}
	/*
	* Comments: 是否是怪物
	* @Return bool: 是怪物返回true，否则返回false
	*/
	inline bool IsMonster()
	{
		switch(GetType())
		{
		case enMonster:
		case enGatherMonster:
			return true;
		}
		return false;
	}

	//是否是英雄
	inline bool IsHero()
	{
		switch(GetType())
		{
		case enHero:
			return true;
		}
		return false;
	}

	//是否攻击免疫
	inline bool IsAttackImmune()
	{
		switch(GetType())
		{
		case enGatherMonster:
			return true;
		default:
			return false;
		}
	}

	//判断是否是NPC
	inline bool isNPC()
	{
		switch(GetType())
		{
		case enNpc:
			return true;
		}
		return false;
	}

	//判断是否要push到人物观察列表中
	inline bool PushActorAppear()
	{
		if (IsMonster())
		{
			return true;
		}
		switch(GetType())
		{
			case enNpc:
			case enHero:
			case enPet:
				return true;
		}
		return false;
	}
	//获取目标实体的handle
	inline const EntityHandle&  GetTarget() const {return m_targetHandler; }

	//设置目标实体的handle
	void SetTarget(const EntityHandle& handle) ;
	
	//还是设置实体目标

	
	/*
	* Comments:将实体传送到一个场景的一个坐标
	* Param LPCSTR sSceneName: 场景名字
	* Param INT_PTR nPosX:坐标的x
	* Param INT_PTR nPsY:坐标的y
	* @Return bool:成功返回true,否则返回false
	*/
	bool TelportSceneByName(LPCSTR sSceneName,INT_PTR nPosX,INT_PTR nPsY, int nType = 0);
	
	/*
	* Comments: 传送到场景的一个默认的点上
	* Param LPCSTR sSceneName: 场景的名字
	* Param INT_PTR nPointID:场景默认的点的ID，从0开始编号
	* @Return bool: 成功返回true，否则返回false
	*/
	bool TelportSceneDefaultPoint(LPCSTR sSceneName,INT_PTR nPointID);

	/*
	* Comments: 传送到一个实体的身边
	* Param CEntity * pEntity: 对方的实体指针
	* Param int nType : 传送类型
	* Param int nValue : 传送值
	* Param int nDest: 距离
	* @Return bool: 成功返回true，否则返回false
	*/
	bool MoveToEntity(CEntity * pEntity, int nType = 0, int nValue = 0,  int  nDest = 0 );

	/*
	* Comments:获取和一个实体距离的平方
	* Param CEntity * pEntity:目标实体的指针
	* @Return INT_PTR:返回两个实体的距离的平方
	*/
	inline INT_PTR GetEntityDistanceSquare(CEntity * pEntity)
	{
		if(pEntity ==this) return 0;
		if(pEntity ==NULL) return 100000;
		if(GetSceneID() != pEntity->GetSceneID())
		{
			return 100000; 
		}
		int nPosX, nPosY;
		pEntity->GetPosition(nPosX,nPosY); //获取实体的坐标
		int nSelfPosX, nSelfPosY;
		GetPosition(nSelfPosX,nSelfPosY); //自身的坐标
		return (nSelfPosX - nPosX)* (nSelfPosX - nPosX) + (nSelfPosY - nPosY)* (nSelfPosY - nPosY);
	}

	/*
	* Comments: 将实体传送到一个场景
	* Param CFuben* pFb:目的副本的指针
	* Param INT_PTR nSceneID:场景的id
	* Param INT_PTR nPosX:位置x
	* Param nPosY:位置y
	* @Return bool: 成功返回true,失败返回false
	*/
	bool Teleport(CFuBen* pFB,INT_PTR nSceneID, INT_PTR nPosX,INT_PTR nPosY);

	/*
	* Comments: 设置实体的朝向
	* Param INT_PTR nDir: 朝向，如果小于0，那么就左右随机取一个朝向
	* @Return void:
	*/
	void SetDir(INT_PTR nDir =-1);
	
	/*
	* Comments:获取面向目标实体的方向,比如目标在自己的前方
	* Param CEntity * pTarget:目标实体的指针
	* @Return INT_PTR:返回方向，比如左边，右边，上面，下面
	*/
	INT_PTR GetTargetDir(CEntity * pTarget);

	/*
	* Comments:判断两个实体是否在同一屏幕
	* Param CEntity * pOther:另一个实体
	* @Return bool:同屏返回true
	*/
	bool IsInSameScreen(CEntity* pOther);

	/*
	* Comments: 获得起点到终点2个点的朝向
	* Param INT_PTR nSourceX: 起点的x
	* Param INT_PTR nSourceY: 起点的y
	* Param INT_PTR nTargetX: 终点的x
	* Param INT_PTR nTargetY: 终点的y
	* @Return INT_PTR:返回方向
	*/
	static INT_PTR GetDir(INT_PTR nSourceX, INT_PTR nSourceY, INT_PTR nTargetX, INT_PTR nTargetY);

	/*
	* Comments: 注册一个脚本函数回调
	* Param void * pTargetNPC: 目标NPC对象指针,如果是空就是全局脚本
	* Param unsigned int nDelayTime: 第一次执行的延迟时间
	* Param unsigned int nInterval: 执行间隔
	* Param int nCount: 脚本执行次数
	* Param const char * pszFuncname: 脚本函数名称	
	* Param const CScriptValueList &args： 脚本参数列表
	* @Return unsigned int: 成功返回SCB句柄，否则返回0
	*/
	unsigned int RegScriptCallback(unsigned int nNpcId, unsigned int nDelayTime, unsigned int nInterval, int nCount, const char *pszFuncname, const CScriptValueList &args);


	/*
	* Comments: 注销一个脚本函数回调
	* Param unsigned int handle:脚本函数回调句柄
	* @Return void:
	*/
	void UnregScriptCallback(unsigned int handle);

	/*
	* Comments:注销一个脚本函数回调
	* Param const char * pFnName: 脚本函数名称
	* @Return void:
	*/
	void UnregScriptCallback(const char *pFnName);

	/*
	* Comments: 执行脚本回调
	* Param INT_PTR nIdx: 脚本回调参数索引
	* @Return void:
	*/
	void OnScriptCallback(INT_PTR nIdx);

	//设置怪物的属性标志位
	inline void SetAttriFlag(const EntityFlags flag)
	{
		m_attriFlag = flag;
	}


	inline EntityFlags& GetAttriFlag() { return m_attriFlag; }

	void Say(MonsterSayBCType nStyle, const char *pMsg);

	
	// 实体Trace标记相关定义
	enum EntityTraceType
	{
		ettActorSkillDamage		= 0x00000001,		// 输出玩家对别人的技能伤害
		ettActorBeSkillDamage	= 0x00000010,		// 输出对玩家的技能伤害
	};

	inline void SeEntityTraceFlag(EntityTraceType nType, bool bSet)
	{
		if (bSet)
			m_nTraceFlag |= nType;
		else
			m_nTraceFlag &= ~nType;
	}

	inline bool GetEntityTraceFlag(EntityTraceType nType)
	{
		return ((m_nTraceFlag & nType) != 0) ? true : false;
	}

	/*
	* Comments: 设置实体最大跟随距离
	* Param unsigned int nMaxDist:
	* @Return void:
	*/
	inline void SetMaxFollDist(unsigned int nMaxDist) { m_nMaxFollowDistSqrt = nMaxDist * nMaxDist; }
	
#ifndef ENTITY_MANAGER_USE_STL_MAP
	//设置在实体管理器里的Node
	inline void SetNode (CList<CEntity*>::NodeType * pNode){m_pNode = pNode; }
	inline CList<CEntity*>::NodeType *  GetNode () {return m_pNode;}
#endif

	/*
	* Comments: 传送归属实体
	* Param CFuBen * pFB: 目的地副本指针
	* Param INT_PTR nSceneID: 目的地场景ID
	* Param INT_PTR nPosX: 目的地位置X坐标
	* Param INT_PTR nPosY:目的地位置Y坐标
	* Param INT_PTR nOwnerSceneId: 传送前Owner所在场景Id
	* Param INT_PTR nOwnerPosX: 传送前Owner地位置X坐标
	* Param INT_PTR nOwnerPosY:传送前Owner地位置Y坐标
	* Param EntityTransferStype transStype: 传送方式
	* @Return void:
	* @Remark: 目前仅仅支持传送玩家拥有的实体
	*/
	void TeleportOwnedEntity(CFuBen *pFB, 
							INT_PTR nSceneID, 
							INT_PTR nPosX, 
							INT_PTR nPosY, 
							INT_PTR nOwnderSceneId, 
							INT_PTR nOwnerPosX, 
							INT_PTR nOwnerPosY, 
							EntityTransferStype transStype);



	//显示显示的名字
	inline LPCTSTR GetShowName(){return m_showName;}

	//设置显示的名字
	inline void SetShowName( LPCTSTR name )
	{
		if(name ==NULL) return ;
		_asncpytA(m_showName,name);
	}

	/*
	* Comments: 能否看得见实体
	* Param CEntity * pEntity: 目标实体指针
	* @Return bool: 如果自己能够看得见目标实体，返回true；否则返回false
	* @Remark:
	*/
	virtual bool CanSee(CEntity* pEntity);

	/**
	面向pTarget实体
	*/
	void Face2Target(CEntity* pTarget);

protected:
	/*
	* Comments: 处理实体脚本回调
	* @Return void:
	*/
	void ProcessScriptCallback();

	/*
	* Comments: 清除脚本回调
	* @Return void:
	*/
	void ClearScriptCallback();

	/*
	* Comments:清空所有的实体消息
	* @Return void:
	* @Remark:
	*/
	void ClearAllEntityMsg();

	/*
	* Comments: 投递一个实体死亡消息
	* @Return void:
	*/
	void PostEntityDeadMsg();

protected:
	char				m_sEntityName[64];		//所有的长度用玩家名字的长度
	char                m_showName[160];    //显示的名字，玩家显示的名字 名字\\帮派\\帮派排名，其他NPC可能改玩家的名字
	ENTITYPOS			m_entityPos;
	// TODO 待修改
	Uint64				m_hTest;			//这里是为了解决 宠物的句柄m_hEntityHandler被修改的bug，这里再提供一段8字节内存，以替代被修改的句柄。
	EntityHandle		m_hEntityHandler;	//实体句柄 
	bool				m_isInited ;		//是否初始化好了

	bool				m_boIsDestroy;		//实体是否正在释放
	//种族（用于图腾怪等客户端需要特殊处理的怪物，配置entityType为普通怪物，race为特别的种类）如果不配置或者为0的话，使用entityType的值，
	//否则的话表示怪物里的种族，方便客户端显示，比如15表示图腾怪,以及其他后面其他方便扩展的其他怪物
	BYTE                m_race;             //种族
	int					m_idx;				//如果创建多个相同的实体，可以区别不同的idx



	EntityHandle        m_targetHandler;    //实体的目标的句柄
	CList<CEntityMsg*>	m_EntityMsgList;	//实体消息队列
	CCLVariant			m_DynamicVar;		//实体动态变量	
	ACTORNAME			m_sTitle;			//实体的称号，如“铁匠”“商人”等	
	CVector<ScriptCallbackParam*> m_scriptCBList; // 脚本回调列表
	CTimer<1000>		m_t1s;
	EntityFlags		    m_attriFlag;			//怪物的属性标志位,怪物刚创建，默认就是配置表的设置，可由程序动态修改
	int					m_nTraceFlag;				// 实体Trace标记	
	unsigned int			m_nMaxFollowDistSqrt;			// 最大跟随距离的平方	

#ifndef ENTITY_MANAGER_USE_STL_MAP
	CList<CEntity*>::NodeType *m_pNode;
#endif

	bool					m_bHasClearMarkedMsg;				// 标记是否有标记位Delete的消息需要清楚
	static CActorProperty * m_defaultProperty;            //一个默认的属性器

	DECLARE_OBJECT_COUNTER(CEntity)
};

//tolua_end
