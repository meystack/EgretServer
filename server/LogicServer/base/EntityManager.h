#pragma once
//#include "Container.h"
#include <map>
#include <stack>
#include <list>
/***************************************************************/
/*
/*                     实体管理器的概念
/*
/*             是游戏的功能模块的管理器
/*
/***************************************************************/
class StringComp
{
public:	
	bool operator()(const char* const _Left, const char* const _Right) const
	{	
		return strcmp(_Left, _Right) < 0;
	}
};

//使用链表管理游戏世界里的实体
typedef Handle<unsigned int> EntityIndex; //
template<class T,int EntityCount =1024>
class CWorldEntityList
{
public: 
	CWorldEntityList(LPCTSTR lpszDesc,INT_PTR nRunTimeLinit =4) : m_entityMgr(lpszDesc)
	{
		m_runLimit = (int)nRunTimeLinit;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		m_interator.setList(m_list);
		m_cacheIter.setList(m_cacheList);
#else
		m_interator = m_EntityMap.begin();
#endif
	}

	//释放一个handle
	inline void Release(EntityHandle handle)
	{
		m_entityMgr.Release(handle);
	}

	//添加一个实体
	inline void Add( CEntity* pInputEntity)
	{
		#ifdef _DEBUG
			assert(pInputEntity->GetType() == T::GETTYPE());
		#endif
		
		T* pEntity = (T* )pInputEntity;
		if(pEntity ==NULL) return;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		//设置实体的节点为为当前的节点，为了删除的时候高效地删除
		pInputEntity->SetNode( (CList<CEntity*>::NodeType *)m_list.linkAfter(pEntity));
#else
		m_EntityMap[Uint64(pEntity->GetHandle())] = pInputEntity;
#endif
	}
	//删除一个实体
	inline bool Destroy(CEntity * pInputEntity)
	{
		#ifdef _DEBUG
			assert(pInputEntity->GetType() == T::GETTYPE());
		#endif

		T* pEntity = (T*)pInputEntity;
		if(pEntity ==NULL) return false;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		//CList<T*>::NodeType *pNode = (CList<T*>::NodeType *)pInputEntity->GetNode();
		CLinkedNode<T*> *pNode = (CLinkedNode<T*> *)pInputEntity->GetNode();
		pInputEntity->SetNode( NULL);
		if(pNode)
		{
			m_interator.remove(pNode);	
			return true;
		}
#else
		Iterator iter = m_EntityMap.find(pEntity->GetHandle());
		if (iter != m_EntityMap.end())
		{
			if (m_interator == iter) {
				m_EntityMap.erase(m_interator++);
				if (m_interator == m_EntityMap.end())
				{
					m_interator = m_EntityMap.begin();
				}
			}
			else m_EntityMap.erase(iter);
			return true;
		}
#endif
		return false;
	}
	//清除
	inline void Empty()
	{
		m_entityMgr.Empty();
#ifndef ENTITY_MANAGER_USE_STL_MAP
		m_list.clear();
		m_cacheList.clear();
#else
		m_EntityMap.clear();
		m_CacheMap.clear();
		m_interator = m_EntityMap.end();
#endif
	}

	//缓存一个实体
	inline void Cache(CEntity * pInputEntity)
	{
		#ifdef _DEBUG
			assert(pInputEntity->GetType() == T::GETTYPE());
		#endif
		
		T* pEntity = (T*)pInputEntity;
		if(pEntity == NULL) return;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		if(CLinkedNode<T*> *pNode = (CLinkedNode<T*>*)pEntity->GetNode())
		{
			m_interator.remove(pNode);
		}
		pEntity->SetNode((CList<CEntity*>::NodeType *)m_cacheList.linkAfter(pEntity));
#else
		Iterator iter = m_EntityMap.find(pEntity->GetHandle());
		if (iter != m_EntityMap.end())
		{
			if (m_interator == iter) m_EntityMap.erase(m_interator++);
			else m_EntityMap.erase(iter);
		}
		m_CacheMap[pEntity->GetHandle()] = pEntity;
#endif
	}
	//删除一个缓存实体
	inline bool DestroyCache(CEntity * pInputEntity)
	{
		#ifdef _DEBUG
			assert(pInputEntity->GetType() == T::GETTYPE());
		#endif
		
		T* pEntity = (T*)pInputEntity;
		if(pEntity == NULL) return false;

#ifndef ENTITY_MANAGER_USE_STL_MAP
		if(CLinkedNode<T*> *pNode = (CLinkedNode<T*> *)pEntity->GetNode())
		{
			m_cacheIter.remove(pNode);
			pEntity->SetNode(NULL);
			return true;
		}
#else
		Iterator iter = m_CacheMap.find(pEntity->GetHandle());
		if (iter != m_CacheMap.end())
		{
			m_CacheMap.erase(iter);
		}
#endif
		return false;
	}
	//解除缓存
	inline void unCache(CEntity * pInputEntity)
	{
		#ifdef _DEBUG
			assert(pInputEntity->GetType() == T::GETTYPE());
		#endif
		
		T* pEntity = (T*)pInputEntity;
		if(pEntity == NULL) return;

#ifndef ENTITY_MANAGER_USE_STL_MAP
		if(CLinkedNode<T*> *pNode = (CLinkedNode<T*>*)pEntity->GetNode())
		{
			m_cacheList.remove(pNode);
		}
		pEntity->SetNode((CList<CEntity*>::NodeType *)m_list.linkAfter(pEntity));
#else
		Iterator iter = m_CacheMap.find(pEntity->GetHandle());
		if (iter != m_CacheMap.end())
		{
			m_CacheMap.erase(iter);
		}
		m_EntityMap[pEntity->GetHandle()] = pEntity;
#endif
	}

	/*
	* Comments:一次逻辑调用
	* Param TICKCOUNT tick:当前的tickCount
	* Param Uint64 nRunTimes:当前主循环运行了几轮
	* @Return bool: 是否超时，如果超时返回true，否则返回false
	*/
	bool Run(TICKCOUNT tick)
	{
		SF_TIME_CHECK();
   		DECLARE_TIME_PROF("CWorldEntityList::Run");
		TICKCOUNT nStart= _getTickCount();
		
		T *pEntity;

#ifndef ENTITY_MANAGER_USE_STL_MAP
		int nLoopLimit = m_list.count();
		CLinkedNode<T*> *pNode;
		while ((pNode=m_interator.next()) && (nLoopLimit > 0))
		{
			pEntity = *pNode;
			if(pEntity)
			{
				#ifdef _DEBUG
					assert(pEntity->GetType() == T::GETTYPE());
				#endif
				
				pEntity->RunOne(tick);
				if( _getTickCount() - nStart > m_runLimit ) //
				{
					return true; 
				}
			}
			--nLoopLimit;
		}
#else
		int nLoopLimit = m_EntityMap.size();
		//Iterator m_endIterator = m_EntityMap.end();
		while ((m_interator != m_EntityMap.end()) && (nLoopLimit > 0))
		{
			pEntity = (T*)(m_interator->second);
			if(pEntity)
			{
				#ifdef _DEBUG
					assert(pEntity->GetType() == T::GETTYPE());
				#endif
				
				pEntity->RunOne(tick);
				if( _getTickCount() - nStart > m_runLimit ) //
				{
					if (m_interator != m_EntityMap.end()) ++m_interator;
					return true; 
				}
			}
			if (m_interator != m_EntityMap.end()) ++m_interator;
			--nLoopLimit;
		}
		
		if (m_interator == m_EntityMap.end() || (nLoopLimit <= 0))
		{
			m_interator = m_EntityMap.begin();
		}
#endif
		return false;
	}
		
public:
#ifndef ENTITY_MANAGER_USE_STL_MAP
	CList<T*> m_list; //实体的列表
	CList<T*> m_cacheList; //实体的缓存列表
	wylib::container::CLinkedListIterator<T*> m_interator;  // 当前的迭代器
	wylib::container::CLinkedListIterator<T*> m_cacheIter;  // 缓存实体的迭代器
#else
	typedef typename std::map<Uint64,CEntity*>::iterator Iterator;
	typedef typename std::map<Uint64,CEntity*>::const_iterator ConstIterator;
	std::map<Uint64,CEntity*> m_EntityMap; //实体的列表 （handle,CEntity*）
	std::map<Uint64,CEntity*> m_CacheMap; //实体的缓存列表
	Iterator m_interator;  // 当前的迭代器
#endif
	HandleMgr<T,EntityIndex,EntityCount> m_entityMgr; //实体的管理器
	int m_runLimit;  //执行的最大的时间
	
};

typedef struct stCampTipData
{
	LPVOID		m_data;
	SIZE_T		m_nLen;
}CampTipData;

template<typename T, int EntityCount = 1024>
class CMonsterEntityList : public CWorldEntityList<T, EntityCount>
{
public:
	CMonsterEntityList(LPCTSTR lpszDesc, INT_PTR nRunTimeLinit = 1, UINT_PTR nDivNum = 3) 
		: CWorldEntityList<T, EntityCount>(lpszDesc, nRunTimeLinit)
	{
		m_nDivNum = __max(nDivNum, 1);
	}
	
	bool Run(TICKCOUNT tick)
	{	
   		DECLARE_TIME_PROF("CMonsterEntityList::Run");
		TICKCOUNT nStart= _getTickCount();
		T *pEntity;
	
	
#ifndef ENTITY_MANAGER_USE_STL_MAP
		UINT_PTR nMaxRunCount = this->m_list.count() / m_nDivNum;
		CLinkedNode<T*> *pNode;
		UINT_PTR nRunCount = 0;
		while (pNode = this->m_interator.next())
		{
			pEntity = *pNode;
			if(pEntity)
			{
				#ifdef _DEBUG
					assert(pEntity->GetType() == T::GETTYPE());
				#endif

				pEntity->RunOne(tick);
				nRunCount++;
				if (_getTickCount() - nStart > this->m_runLimit )
					return true; 
				if (nRunCount >= nMaxRunCount)
					return false;
			}
		}
#else
		UINT_PTR nMaxRunCount = this->m_EntityMap.size() / m_nDivNum;
		nMaxRunCount = nMaxRunCount?nMaxRunCount:m_nDivNum;
		if (nMaxRunCount < m_nDivNum)  nMaxRunCount = m_nDivNum;
		
		UINT_PTR nRunCount = 0;
		while (this->m_interator != this->m_EntityMap.end())
		{
			pEntity = (T*)(this->m_interator->second);
			if(pEntity)
			{
				#ifdef _DEBUG
					assert(pEntity->GetType() == T::GETTYPE());
				#endif

				pEntity->RunOne(tick);
				nRunCount++;
				if( _getTickCount() - nStart > this->m_runLimit )
					return true;
				if (nRunCount >= nMaxRunCount)
					return false;
			}
			if (this->m_interator != this->m_EntityMap.end()) ++(this->m_interator);
		}
		if (this->m_interator == this->m_EntityMap.end())
		{
			this->m_interator = this->m_EntityMap.begin();
		}
#endif
		return false;
	}

	inline void SetDivNum(UINT_PTR nDivNum)
	{
		if (m_nDivNum != nDivNum && nDivNum > 0)
			m_nDivNum = nDivNum;
	}

private:
	UINT_PTR		m_nDivNum;		// 等分数。例如m_nDivNum=3表明将列表分为3等分，每次LogicRun的数目不超过总数量的1/3
};

class CEntityManager:
	public CComponent,
	public CSendPacketPool //实体管理器搞这个为了管理网络数据包，实现分时处理
{
public:
	// 实体脚本回调句柄和句柄管理器类型定义	
	//typedef Handle<int> EntityIndex; //
	typedef Handle<unsigned int>								EntitySCBHandle;
	typedef HandleMgr<ScriptCallbackParam, EntitySCBHandle, 64>	EntitySCBHdlMgr;
	
	
	typedef CWorldEntityList<CActor,64> ActorMgr;	
	typedef CMonsterEntityList<CMonster> MonsterMgr;
	typedef CMonsterEntityList<CNpc,64> NpcMgr;
	typedef CWorldEntityList<CDropItemEntity,128> CDropItemEntityMgr;  //掉落物品
	typedef CWorldEntityList<CTransfer,64> TransferMgr;  //传送门	
	typedef CMonsterEntityList<CPet, 128> PetMgr; // 宠物
	typedef CWorldEntityList<CFire, 512> FireMgr; // 火的管理器
	typedef CWorldEntityList<CHero, 64> HeroMgr; // 英雄的管理器

	CEntityManager();

	bool Initialize();
	VOID  Destroy();

	void CloseAllActor();

	//有玩家登陆了，将记录他的
	inline void OnActorLogin(unsigned int nAccountId, unsigned int nActorId, EntityHandle handle)
	{
		m_actorIdList.insert(std::make_pair(nActorId, handle));
	}

	inline void OnActorInit(const char* actorName, EntityHandle handle)
	{
		/*m_actorNameList.insert(std::make_pair(actorName, handle));*/
	}

	void OnActorClosed(CActor* pActor);

	/*
	* Comments: db返回数据
	* Param char * nCmd: 命令码
		typedef enum tagDBRecvCmd
		{
			dcQuery=1,			//查询数据
			dcSave,				//保存数据
			dcLoadActor = 3,	//加载用户认证信息
			dcLoadQuest,         //装置任务数据
			dcSaveQuest,		//保存用户的任务数据
			dcLoadSkill,        //装置技能数据
			dcSaveSkill,        //保持技能数据
		};
	* Param char * data: 数据指针
	* Param SIZE_T size: 数据长度
	* @Return void:
	*/
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size); 

	void OnDbNoticeFee(char * data,SIZE_T size);

	/*
	* Comments:会话服务器返回数据
	* Param INT_PTR nCmd:命令id
	* Param char * data:数据内容
	* Param SIZE_T size:数据大小
	* @Return void:
	*/
	void OnSsReturnData(INT_PTR nCmd,char * data,SIZE_T size); 


	/*
	* Comments: 接收到来自公共逻辑服务器转发的消息
	* Param char * data:
	* Param SIZE_T size:
	* @Return void:
	* @Remark:
	*/
	void OnRecvCommServerLogicMessage(char *data, SIZE_T size);

	/*
	* Comments: 接收到来自其他逻辑服务器转发的消息
	* Param char * data:
	* Param SIZE_T size:
	* @Return void:
	* @Remark:
	*/
	void OnRecvOtherLogicMessage(char *data, SIZE_T size);

	/*
	* Comments: 接收到来自公共服务器的数据
	* Param INT_PTR nCmd: 消息号
	* Param char * data: 数据内容
	* Param SIZE_T size: 数据长度
	* @Return void:
	* @Remark: 普通逻辑服务器调用此函数用于处理来自公共服务器的数据
	*/
	void OnCommonServerRetData(INT_PTR nCmd, char *data, SIZE_T size);

	/*
	* Comments: 接收来自普通服务器的数据
	* Param INT_PTR nCmd: 消息号
	* Param char * data: 数据内容
	* Param SIZE_T size: 数据长度
	* @Return void:
	* @Remark: 公告逻辑服务器调用此函数接收来自普通服务器的数据
	*/
	void OnLogicServerRetData(INT_PTR nCmd, char *data, SIZE_T size);

	/*
	* Comments: 创建一个实体
	* Param INT_PTR nEntityType: 实体的类型
	* Param EntityHandle & hHandle: 实体的handle
	* @Return CEntity *:实体的指针
	*/
	CEntity * CreateEntity(int nEntityType,EntityHandle  &hHandle); //创建一个实体,返回他的实体ID

	/*
	* Comments: 通过实体的handle销毁一个实体
	* Param EntityHandle nEntityID: 实体的handle
	* Param bool bDelayDestroy: 延迟删除
	* @Return bool:成功返回true，否则返回false
	*/
	bool DestroyEntity(EntityHandle nEntityID, bool bDelayDestroy = false) ; //销毁一个实体

	bool CachePet(CActor* pMaster, CVector<CPetSystem::PETDATA> *pPetList, long long nExpireTime);

	bool UnCachePet(CActor* pMaster, CVector<CPetSystem::PETDATA> *pPetList, CScene* pScene);

	/*
	* Comments: 通过实体handle返回实体指针
	* Param EntityHandle & hEntity: 实体 handle
	* @Return CEntity *:实体指针
	*/
	CEntity * GetEntity(const EntityHandle &hEntity);

	/*
	* Comments: 通过玩家的名字查找指针
	* Param char * name: 玩家名字
	* @Return CActor *: 数据指针
	*/
	inline CActor * GetActorPtrByName(const char * name)
	{
		if (!name) return NULL;		
		/*ActorName2Hdl::iterator iter = m_actorNameList.find(name);
		if (iter != m_actorNameList.end())
			return (CActor *)GetEntity(iter->second);
		return NULL;*/
		CActor* pActor = NULL;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if(0== strcmp(name,pActor->GetEntityName()) )
			{
				return pActor;
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if(0== strcmp(name,pActor->GetEntityName()) )
			{
				return pActor;
			}
		}
#endif
		return NULL;
	}

	/*
	* Comments: 通过actorID找玩家的指针
	* Param unsigned int nActorID: 玩家的角色id
	* @Return CActor *: 玩家的数据指针
	*/
	inline CActor *GetEntityPtrByActorID(unsigned int nActorID)
	{				
		ActorId2Hdl::iterator iter = m_actorIdList.find(nActorID);
		if (iter == m_actorIdList.end())
			return NULL;		
		return (CActor *)GetEntity(iter->second);
	}

	inline CActor *GetEntityPtrByAccountID(unsigned int nAccountID)
	{
		
		CActor *pActor;
		
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if( pActor->GetAccountID () ==nAccountID )
			{
				return pActor;
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if( pActor->GetAccountID () ==nAccountID )
			{
				return pActor;
			}
		}
#endif
		return NULL;
	}

	inline CActor *GetEntityPtrByAccountName(LPCSTR sAccount)
	{
		CActor *pActor;

#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if( strcmp(pActor->GetAccount(),sAccount) == 0)
			{
				return pActor;
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if( strcmp(pActor->GetAccount(),sAccount) == 0)
			{
				return pActor;
			}
		}
#endif
		return NULL;
	}

	inline CMonster *GetMonsterPtrByEntityId(const unsigned int nMonsterId)
	{
		CMonster *pMonster;

#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CMonster*>::Iterator it(m_monsterMgr.m_list);
		CList<CMonster*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pMonster = *pNode;
			if( nMonsterId == pMonster->GetProperty<unsigned int>(PROP_ENTITY_ID) )
			{
				return pMonster;
			}
		}
#else
		ActorMgr::Iterator it = m_monsterMgr.m_EntityMap.begin();
		for (; it != m_monsterMgr.m_EntityMap.end(); ++it)
		{
			pMonster = (CMonster*)(it->second);
			if( nMonsterId == pMonster->GetProperty<unsigned int>(PROP_ENTITY_ID) )
			{
				return pMonster;
			}
		}
#endif
		return NULL;
	}

	VOID RunOne(); 

	/*
	* Comments: 向所有在线的玩家广播
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* Param int nLevel: 广播的玩家最低限制等级
	* @Return void:
	*/
	inline void BroadCast(char * pData,SIZE_T size, int nLevel = 0, int nCircle = 0)
	{
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if (pActor && pActor->IsInited() && pActor->CheckLevel(nLevel, nCircle))
			{
				CActorPacket pack;
				CDataPacket &data = pActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if (pActor && pActor->IsInited() && pActor->CheckLevel(nLevel, nCircle))
			{
				int nActorType = pActor->GetHandle().GetType();
				if (nActorType == enActor)
				{
					if(pActor->OnGetIsTestSimulator()
						|| pActor->OnGetIsSimulator() )
					{
						continue;
					}
				}
				CActorPacket pack;
				CDataPacket &data = pActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
#endif
	}
 
	/*
	* Comments: 向所有在线的玩家广播
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* Param int nLevel: 广播的玩家最低限制等级
	* @Return void:
	*/
	inline void BroadCastScene( char * pData,SIZE_T size, int nSceneId = 0, int nLevel = 0 )
	{
		if( nSceneId == 0 )
		{
			BroadCast(pData, size, nLevel);
			return;
		}
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if (pActor && pActor->IsInited() && pActor->CheckLevel(nLevel, 0) && pActor->GetSceneID() == nSceneId )
			{
				CActorPacket pack;
				CDataPacket &data = pActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if (pActor && pActor->IsInited() && pActor->CheckLevel(nLevel, 0) && pActor->GetSceneID() == nSceneId )
			{
				CActorPacket pack;
				CDataPacket &data = pActor->AllocPacket(pack);
				data.writeBuf(pData,size);
				pack.flush();
			}
		}
#endif
	}

	/*
	* Comments: 转发给武林盟主
	* Param char * pData:数据指针
	* Param SIZE_T size:数据长度
	* Param int nLevel: 转发的玩家最低限制等级
	* @Return void:
	*/
	inline void RetransmitToKungFuMaster(INT_PTR nZhenyinId, char * pData, SIZE_T size, int nLevel = 0)
	{
		/*
		CActor *pActor = GetKungFuMasterEntityPtr();
		if (pActor && pActor->IsInited() && pActor->GetProperty<unsigned>(PROP_ACTOR_ZY) != nZhenyinId
			&& pActor->GetProperty<int>(PROP_CREATURE_LEVEL) >= nLevel && pActor->GetSocialMask(smIsWulinMaster))
		{
			CActorPacket pack;
			CDataPacket &data = pActor->AllocPacket(pack);
			data.writeBuf(pData,size);
			pack.flush();
		}
		*/

	}

	/*
	* Comments: 广播阵营提示
	* Param const void * pData1:
	* Param SIZE_T size1:
	* Param const void * pData2:
	* Param SIZE_T size2:
	* Param const void * pData3:
	* Param SIZE_T size3:
	* @Return void:
	* @Remark:
	*/
	inline void BroadCampTipmsg(CampTipData data[3])
	{
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if(pActor && pActor->IsInited())
			{
				int nCampId = pActor->GetCampId();
				if (nCampId > 0 && nCampId <= 3)
				{
					CampTipData* pData = &data[nCampId-1];				
					if (pData->m_data != NULL && pData->m_nLen > 0)
					{
						CActorPacket pack;
						CDataPacket &data = pActor->AllocPacket(pack);
						data.writeBuf(pData->m_data, pData->m_nLen);
						pack.flush();
					}
				}
			}
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if(pActor && pActor->IsInited())
			{
				int nCampId = pActor->GetCampId();
				if (nCampId > 0 && nCampId <= 3)
				{
					CampTipData* pData = &data[nCampId-1];				
					if (pData->m_data != NULL && pData->m_nLen > 0)
					{
						CActorPacket pack;
						CDataPacket &data = pActor->AllocPacket(pack);
						data.writeBuf(pData->m_data, pData->m_nLen);
						pack.flush();
					}
				}
			}
		}
#endif
	}

	/*
	* Comments:获取在线玩家的id列表，好友模块需要
	* Param CVector<int> & IdList:
	* @Return void:
	*/
	inline void GetActorIdList(CVector<int>& IdList)
	{
		IdList.clear();
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			IdList.add(pActor->GetId());
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			IdList.add(pActor->GetId());
		}
#endif
	}

	/*
	* Comments:获取在线玩家列表
	* Param CVector<int> & IdList:
	* @Return void:
	*/
	inline
#ifndef ENTITY_MANAGER_USE_STL_MAP
	const CList<CActor*>
#else
	const std::map<Uint64,CEntity*>&
#endif
	getActorMap()
	{
#ifndef ENTITY_MANAGER_USE_STL_MAP
		return m_actorMgr.m_list;
#else
		return m_actorMgr.m_EntityMap;
#endif
	}

	/*
	* Comments: 获取虚拟人的在线数量
	* @Return INT_PTR:返回虚拟人在线人数的总数
	*/
	inline INT_PTR GetOnlineSimulatorActorCount(INT_PTR& nTotalCount)
	{
		INT_PTR nActorCount = 0;
		CActor* pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		nTotalCount = m_actorMgr.m_list.count();
		CList<CActor*>::Iterator iter(m_actorMgr.m_list);
		CList<CActor*>::NodeType* pNode;
		for (pNode = iter.first(); pNode; pNode = iter.next())
		{
			pActor = *pNode;
			
			int nActorType = pActor->GetHandle().GetType();
			if (nActorType == enActor)
			{
				if(!pActor->OnGetIsTestSimulator()
					&& !pActor->OnGetIsSimulator() )
				{
					continue;//不是虚拟人
				}
			}
			if (pActor && pActor->GetGmLevel() <= 0)
				nActorCount++;
		}
#else
		nTotalCount = m_actorMgr.m_EntityMap.size();
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			
			int nActorType = pActor->GetHandle().GetType();
			if (nActorType == enActor)
			{
				if(!pActor->OnGetIsTestSimulator()
					&& !pActor->OnGetIsSimulator() )
				{
					continue;//不是虚拟人
				}
			}
			if (pActor && pActor->GetGmLevel() <= 0)
				nActorCount++;
		}
#endif
		return nActorCount;
	}

	/*
	* Comments: 获取在线人数的数量
	* @Return INT_PTR:返回在线人数的总数
	*/
	inline INT_PTR GetOnlineActorCount(INT_PTR& nTotalCount)
	{
		INT_PTR nActorCount = 0;
		CActor* pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		nTotalCount = m_actorMgr.m_list.count();
		CList<CActor*>::Iterator iter(m_actorMgr.m_list);
		CList<CActor*>::NodeType* pNode;
		for (pNode = iter.first(); pNode; pNode = iter.next())
		{
			pActor = *pNode;
			if (pActor && pActor->GetGmLevel() <= 0)
				nActorCount++;
		}
#else
		nTotalCount = m_actorMgr.m_EntityMap.size();
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if (pActor && pActor->GetGmLevel() <= 0)
				nActorCount++;
		}
#endif
		return nActorCount;
	}

	inline void GetOnlineAcotrHandleList(CVector<EntityHandle> &actorList)
	{
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			actorList.push(pActor->GetHandle());
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			actorList.push(pActor->GetHandle());
		}
#endif
	}

	int GetOnLineActorCountNoSame();
	 
	inline void GetOnlineAcotrPtrList(CVector<void*> &actorList, int nMinLevel = 0)
	{
		CActor *pActor;
#ifndef ENTITY_MANAGER_USE_STL_MAP
		CList<CActor*>::Iterator it(m_actorMgr.m_list);
		CList<CActor*>::NodeType *pNode;
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pActor = *pNode;
			if (nMinLevel > 0 && pActor->GetProperty<int>(PROP_CREATURE_LEVEL) < nMinLevel)
			{
				continue;
			}
			actorList.add(pActor);
		}
#else
		ActorMgr::Iterator it = m_actorMgr.m_EntityMap.begin();
		for (; it != m_actorMgr.m_EntityMap.end(); ++it)
		{
			pActor = (CActor*)(it->second);
			if (nMinLevel > 0 && pActor->GetProperty<int>(PROP_CREATURE_LEVEL) < nMinLevel)
			{
				continue;
			}
			actorList.add(pActor);
		}
#endif
	}

	//获取广播数据的指针
	inline char * GetBroadcastDataPtr()
	{
		return m_broadcastBuff;
	}
	
	/*
	* Comments: 全服发布广播
	* Param char * sTipmsg: tipmsg的指针
	* Param int nTipmsgType: 公告的显示类型，聊天栏，还是弹出框等
	* @Return void:
	*/
	void BroadcastTipmsg(char * sTipmsg,int nTipmsgType =ttTipmsgWindow,int nLevel = 0);

	/*
	* Comments: 全服发布公告
	* Param char * sTipmsg: tipmsg的指针
	* Param int nTipmsgType: 公告的显示类型，聊天栏，还是弹出框等
	* @Return void:
	*/
	void BroadNotice(char * sTipmsg, int nMsgType, int nLevel = 0);
	
	/*
	* Comments: 通过ID广播tipmsg
	* Param int nTipmsgID: 提示的ID
	* Param int nTipmsgType: 提示的内容
	* @Return void:
	*/
	void BroadcastTipmsgWithID(int nTipmsgID, int nTipmsgType =ttTipmsgWindow);

	/*
	* Comments: 带参数全服广播
	* Param int nTipmsgID: 提示的ID
	* Param int nTipmsgType: 提示的显示类型
	* Param char * sParam1:参数1
	* Param char * sParam2:参数2
	* Param char * sParam3:参数3
	* @Return void:
	*/
	void BroadTipmsgWithParams(int	nTipmsgID,int nTipmsgType=ttTipmsgWindow,...);
	
	// 带参数全服广播 跨服--->原服
	void BroadTipmsgWithParamsToCs(int	nTipmsgID, int nTipmsgType, ...);
	
	/*
	* Comments:格式化系统提示的字符串,这个接口可以给外部使用
	* Param int nTipmsgID: 系统提示的ID
	* Param int nTipmsgType: 系统提示的类型
	* Param ...: 可变参数
	* @Return INT_PTR: 返回数据长度
	*/
	INT_PTR FormatTipmsg(int nTipmsgID,va_list &args,int nTipmsgType=ttTipmsgWindow);
	

	/*
	* Comments:更新怪物句柄，为怪物死亡立即刷新时实体复用适用。
	* Param CEntity * pEntity: 怪物实体对象指针
	* @Return void:
	*/
	void updateMonsterHandle(CEntity* pEntity);


	/*
	* Comments: 创建脚本事件参数对象
	* Param EntitySCBHandle & handle: SCB句柄
	* @Return ScriptCallbackParam*:
	*/
	ScriptCallbackParam* CreateScriptCallbackParam(EntitySCBHandle &handle);


	/*
	* Comments: 销毁脚本时间参数对象
	* Param EntitySCBHandle & handle:
	* @Return void:
	*/
	void DestroyScriptCallbackParam(EntitySCBHandle &handle);


	/*
	* Comments: 获取脚本回调时间对象
	* Param const EntitySCBHandle & handle:
	* @Return ScriptCallbackParam*:
	*/
	ScriptCallbackParam* GetScriptCallbackParam(const EntitySCBHandle &handle);

	//玩家向管理器申请一个网络包，用于缓存
	CDataPacket * AllocDatapack()
	{
		if (m_freePacketList.count() <= 0)
		{
			allocSendPacketList(m_freePacketList,512);
		}
		CDataPacket* pTempData = m_freePacketList.pop();//得到一个空闲的Datapacket
		if(pTempData )
		{
			pTempData->setPosition(0);
		}
		return pTempData;
	}

	//回收一个网络消息包
	void FreeDataPack(CDataPacket * pData)
	{
		m_freePacketList.add(pData); //回收一个
	}

	//获取一个实体一次循环最长需要的时间
	inline int GetOneEntityTime()
	{
		return m_nOneEntityTime;
	}

	//设置一个实体的一次循环最长需要多长时间
	inline void SetOneEntityTime(int nTime)
	{
		m_nOneEntityTime = nTime;
	}

	/*
	* Comments: 设置怪物LogicRun分组更新的等分数目
	* Param UINT_PTR nDivNum: 分组更新的等分数目
	* @Return void:
	* @Remark:
	*/
	inline void SetMonsterDivNum(UINT_PTR nDivNum)
	{
		m_monsterMgr.SetDivNum(nDivNum);
	}

	/*
	* Comments: 设置NPC LogicRun分组更新的等分数目
	* Param UINT_PTR nDivNum: 分组更新的等分数目
	* @Return void:
	* @Remark:
	*/
	inline void SetNpcDivNum(UINT_PTR nDivNum)
	{
		m_npcMgr.SetDivNum(nDivNum);
	}

	/*
	* Comments: 设置宠物LogicRun分组更新的等分数目
	* Param UINT_PTR nDivNum: 分组更新的等分数目
	* @Return void:
	* @Remark:
	*/
	inline void SetPetDivNum(UINT_PTR nDivNum)
	{
		m_petMgr.SetDivNum(nDivNum);
	}

	//统计实体的内存数据
	void DumpEntityAllocData(wylib::stream::CBaseStream& stream);

	//统计实体的数量
	int GetActorCount(){ return m_actorIdList.size();};
 
private:	

	ActorMgr				m_actorMgr;				//玩家
	MonsterMgr				m_monsterMgr;			//monster
	NpcMgr					m_npcMgr;				//NPC的管理器
	CDropItemEntityMgr		m_dropItemEntityMgr; 	//掉落管理器
	TransferMgr				m_transferMgr; 			//传送门的管理器
	EntitySCBHdlMgr			m_entitySCBHdlMgr;		//实体脚本回调事件句柄管理器
	PetMgr                  m_petMgr;          		//宠物管理器
	FireMgr                 m_fireMgr;        		//火的管理器
	HeroMgr                 m_heroMgr;         		//英雄管理器

	CBaseList<CDataPacket*> m_freePacketList ; //空闲的数据包列表

	//游戏中怪物、npc等实体执行runone需要限定总时间
	//这个变量记录当前执行到第几个实体的逻辑，则在下一个逻辑循环中从这个实体开始执行
	//int						m_nEntityRunPos;	
	int                     m_nOneEntityTime; //一个实体处理的最大的时间
	CVector<EntityHandle>	m_waitRemoveEntityList; // 等待删除实体列表。在每次logicRun中删除
	char m_broadcastBuff[4096]; //广播消息用的
	Uint64                  m_logicRunTimes;  //逻辑循环调用了多少次

	typedef std::map<unsigned int, EntityHandle>			ActorId2Hdl;
	typedef std::map<const char*, EntityHandle, StringComp> ActorName2Hdl;
	typedef std::map<int, int>								Monster2Dead;

	ActorId2Hdl				m_actorIdList;
	ActorName2Hdl 			m_actorNameList;
	CTimer<1000>            m_1secTimer;  		//1秒定时器
	CTimer<5000>            m_5secTimer;  		//5秒去检测这些火
	CTimer<60000>			m_1minsTimer;		//每分钟发送怪的死亡次数

	std::map<int, CVector<CPetSystem::PETDATA> > m_petCache;
	std::stack<std::pair<int,long long> > m_petExpire;

};
