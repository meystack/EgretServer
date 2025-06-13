#pragma once
/***************************************************************/
/*
/*                     实体的观察者子系统
/* 定时更新实体可视范围的实体的实体出现和消失，玩家视野出现，消失实体要通知客户端
/* 1)维护实体的可视实体列表
/* 2)实体出现，消失通知
/* 3)可视范围范围的广播接口
/***************************************************************/
#pragma once

class CObserverTag{}; ///< 没有实质用途的类
/// 实体的出现，消失，删除标记
enum tagEntityAppear
{
	eENTITY_APPEAR_UNCHANGE, ///< 没有变化 
	eENTITY_APPEAR_NEW,      ///< 新出现的
	eENTITY_APPEAR_DELETE,   ///< 删除的
};

/// 实体的handle + 标记的结构体
struct EntityHandleTag
{
	EntityHandle	m_handle;
	BYTE			m_tag;
};
/// 实体的handle + 标记的结构体
struct EntityHandleTagEx
{
	EntityHandle	m_handle;
	BYTE			m_tag; 
	BYTE			type; 
};
enum 
{
	eENTITY_SPC_NULL,  
	eENTITY_SPC_NEW_DEL, 
	eENTITY_SPC_OLD_DEL,  
	eENTITY_SPC_FULL_DEL, 
};

/// 附近的一些玩家的一些事件,通过OnEvent接口转过来
enum tagNearEntityEvent
{
	neEventMove, ///< 跳跃
};

typedef CHandleList<EntityHandleTag,CObserverTag> CObserverEntityList;

class CObserverSystem:
	public CEntitySubSystem<enObserverSystemID,CObserverSystem,CAnimal>
{
public:
	
	/// 定时调用
	VOID OnTimeCheck(TICKCOUNT nTickCount) ;//check 	

	/**
	* @brief 向周围人广播数据
	* @param pBuff 数据指针 
	* @param nSize 数据长度
	* @param bToSelf 是否发给自己
	* @param bToNewAppearEntity 是否发送给刚出现的实体
	* @return void
	*/
	void BroadCast (char * pBuff,SIZE_T nSize,bool bToSelf =false,bool bToNewAppearEntity =true); 

	/// 广播实体消息,@param boToSelf 是否向自己发送
	void BroadCastEntityMsg(const CEntityMsg &msg, bool boToSelf = true);
	
	/// 销毁的时候调用
	void  Destroy() {
		m_sEntityList.clear();
	} ///< 这里要清除自己的观察者列表

	void Clear();

	void ClearEntityList();

	/**
	* @brief 其他实体要将一个事件转发给观察者范围的实体处理
	* @param   nEventID	事件的ID
	* @param   nParam1	整形参数1
	* @param   nParam2	整形参数2
	* @param   pData	指针参数
	* @return void 
	*/
	void OnEvent(INT_PTR nEventID,INT_PTR nParam1=0,INT_PTR nParam2=0,void * pData=NULL);

	EntityHandle FindEntityByName(const char *pEntityName);

	/**
	* @brief 获取可见玩家列表
	* @return CObserverEntityList&:
	*/
	CVector<EntityHandleTag>& GetVisibleList()
	{
		return m_sEntityList;
	}
	////能否看到一个实体
	//inline bool CanSee(CEntity * pEntity);

	/// 视野里出现了一个实体
	void EntityAppear(const EntityHandle &handle, CEntity* pEntity);
	
	/// 视野里消失了一个实体
	void EntityDisappear(const EntityHandle &handle) ;

	/**
	* @brief 发送下属位置改变
	* @return void
	*/
	void SendPosChangeToOwner();

	/**
	* @brief 清空观察者列表
	* @return void 
	*/
	void ClearObserveList();

	/**
	* @brief 将玩家变化的属性更新到客户端周围玩家
	* @return void 
	* @Remark 
	*/
	void UpdateActorEntityProp();

private:
	void EntityAppearActor(const EntityHandle &handle, CActor* pOtherActor);
	void EntityAppearDropItem(const EntityHandle &handle, CEntity* pItem);
	void EntityAppearSpecialEntity(const EntityHandle &handle, CEntity* pSpecialEntity);
	void EntityAppearMonster(const EntityHandle &handle, CEntity* pEntity);
	void EntityAppearPet(const EntityHandle &handle, CEntity* pEntity);
	void EntityAppearSlave(const EntityHandle &handle, CEntity* pEntity);
	void EntityAppearNpc(const EntityHandle &handle, CEntity* pEntity);
	void EntityAppearDefault(const EntityHandle &handle, CEntity* pEntity);
	
	
	/**
	* @brief 定时检测自己的属性改变，如果有改变就把这些数据下发到视野里的玩家
	* @param nOldUserCount 视野里老玩家的数量，也就是不是新出现或者删除的
	* @param vecVisiblePlayers 视野范围内实体
	* @return void
	*/	
	void CheckEntitypPropery(INT_PTR nOldUserCount, CVector<EntityHandleTag>& vecVisiblePlayers);

	/**
	* @brief owner是否是玩家
	* @return bool 有主人并且主人是玩家
	* @remark 
	*/
	bool IsOwnerActor();

	/*
	* @brief	广播属性变化给归属玩家
	* @return void 
	* @remark  注意：只有归属玩家不在视野范围内才广播属性
	*/
	void SendPropChangeToOwner();
	

	////////////////////////////////////////////////////////////////////////////////
	/// 这部分用于怪物观察者子系统的优化，玩家在更新周围实体列表的时候直接处理新出现的
	/// 和消失的，直接更新对方的观察者列表
	/// 注意：只针对怪物和采集怪
	/// 玩家进入怪物视野
	void onActorAppear(CActor* pActor);
	/// 玩家离开怪物视野
	void onActorDisappear(CActor* pActor);
	////////////////////////////////////////////////////////////////////////////////

	/*
	* Comments:是否需要采取push的方式玩家列表
	* Param INT_PTR nEntityType:实体类型
	* @Return bool:需要使用true
	*/
	inline bool NeedPushActor(INT_PTR nEntityType)
	{
		switch (nEntityType)
		{
		case enMonster:
		case enGatherMonster:
		case enPet:
		case enHero:
			return true;
		}
		return false;
	}

	VOID CheckList(EntityHandle ignore, EntityVector& appear, EntityVector& disappear, CVector<EntityHandleTag>* intersection);
 
	void NetworkCutDown(TICKCOUNT nTick);//跨服领主减少玩家
public:
	static SequenceEntityList s_orderVisibleSet;
	static CVector<EntityHandleTag> *s_OBEntityList; // 观察者实体列表，静态数组，用于数据中转用
	static EntityVector* s_seqVisibleList;
private:
	CVector<EntityHandleTag> m_sEntityList; // 周围实体列表（采取数组的方式存储）
};


