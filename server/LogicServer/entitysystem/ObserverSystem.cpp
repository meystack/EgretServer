#include "StdAfx.h"
#include "ObserverSystem.h"

//内存池
EntityVector* CObserverSystem::s_seqVisibleList = NULL;
CVector<EntityHandleTag>* CObserverSystem::s_OBEntityList;
SequenceEntityList CObserverSystem::s_orderVisibleSet;

//广播接口
void CObserverSystem::BroadCast(char * pBuff, SIZE_T nSize, bool bToSelf, bool bToNewAppearEntity/*= true*/)
{
	if(m_pEntity == NULL)
	{
		return;
	}

	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	INT_PTR nSelfType = m_pEntity->GetHandle().GetType() ; //自己的类型
	if (!((CActor*)m_pEntity)->CanNetworkCutDown()) 
	{ 	//老的广播消息
		EntityHandle handle = m_pEntity->GetHandle();
		for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
		{
			EntityHandle handle = m_sEntityList[i].m_handle;
			if (handle.GetType() != enActor)
			{
				continue;
			}
			
			if(bToNewAppearEntity  || (!bToNewAppearEntity && m_sEntityList[i].m_tag == eENTITY_APPEAR_UNCHANGE))
			{
				CActor * pActor =(CActor *) em->GetEntity(handle);
				if(pActor)
				{	
					pActor->SendData(pBuff,nSize);
				}
			}
		}

		if(bToSelf && nSelfType == enActor)
		{
			((CActor *)m_pEntity)->SendData(pBuff,nSize);
		}
	}
	else
	{
		if(nSelfType != enActor)
		{ 
			return;
		} 
 
		if(bToSelf)
		{ 
			((CActor *)m_pEntity)->SendData(pBuff, nSize);
		}
		//特殊场景的广播消息
		{	
			//扩大范围检索
			DECLARE_TIME_PROF("get list");
			CFuBenManager* fm = GetGlobalLogicEngine()->GetFuBenMgr();			
			fm->GetSeqVisibleList(m_pEntity, s_orderVisibleSet, -BIG_MOVE_GRID_COL_RADIO, 
				BIG_MOVE_GRID_COL_RADIO, -BIG_MOVE_GRID_COL_RADIO, BIG_MOVE_GRID_COL_RADIO);
		}
  
		SequenceEntityListIter itcheck = s_orderVisibleSet.begin();
		for(; itcheck != s_orderVisibleSet.end(); itcheck++)
		{  
			EntityHandle newHandle = *itcheck;
			
			if (newHandle.GetType() != enActor)//不是玩家
			{
				continue;
			} 
			CEntity *pEntity = em->GetEntity(newHandle);

			if (!pEntity)
			{
				OutputMsg(rmError, _T("视野里出现的实体的指针为空3977938, type = %d"), newHandle.GetType()); 
				continue;
			}
			EntityHandle myHandle = m_pEntity->GetHandle();
			//if(bToNewAppearEntity  || (!bToNewAppearEntity && m_sEntityList[i].m_tag == eENTITY_APPEAR_UNCHANGE))
			if (((CActor *)pEntity)->OnCheckCanSeeEntity(myHandle)) 
			{ 
				((CActor *)pEntity)->SendData(pBuff, nSize); 
			}
		}
	}
}

void CObserverSystem::Clear()
{
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	EntityHandle handle = m_pEntity->GetHandle();
	if (enActor == handle.GetType())
	{		
		for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
		{
			if (m_sEntityList[i].m_handle != handle)
			{
				CEntity *pEntity = em->GetEntity(m_sEntityList[i].m_handle);
				if(pEntity)
				{	
					EntityDisappear(m_sEntityList[i].m_handle);
				}
			}
		}
	}
	m_sEntityList.clear();

}

void CObserverSystem::ClearEntityList()
{
	m_sEntityList.clear();
}

void CObserverSystem::BroadCastEntityMsg(const CEntityMsg &msg, bool boToSelf)
{
	if(m_pEntity == NULL )return;
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	////遍历可视范围的玩家
	for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
	{
		CEntity *pEntity = em->GetEntity(m_sEntityList[i].m_handle);
		if(pEntity)
		{	
			pEntity->PostEntityMsg(msg);
		}
	}
	if (boToSelf) m_pEntity->PostEntityMsg(msg);

}

void CObserverSystem::OnEvent(INT_PTR nEventID,INT_PTR nParam1,INT_PTR nParam2,void * pData)
{
	
	INT_PTR nSelfType = m_pEntity->GetHandle().GetType() ; //自己的类型
	if( nSelfType != enActor && nSelfType != enPet)  return;

}

// TODO 这里要优化性能
void CObserverSystem::onActorAppear(CActor* pActor)
{	
	EntityHandleTag entityTag;
	entityTag.m_handle	= pActor->GetHandle();
	entityTag.m_tag		= eENTITY_APPEAR_UNCHANGE;
	EntityHandle handle = pActor->GetHandle();
	for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
	{
		if (m_sEntityList[i].m_handle == handle)
		{
			return;
		}
	}
	m_sEntityList.push(entityTag);
}

void CObserverSystem::onActorDisappear(CActor* pActor)
{
	//INT_PTR nType = m_pEntity->GetType();

	// 在原来的列表中找此实体
	EntityHandle handle = pActor->GetHandle();
	
	std::vector<EntityHandleTagEx> tmpEntityList; // 周围实体列表（采取数组的方式存储） 
	for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
	{
		EntityHandleTagEx tmp;
		tmp.m_tag = m_sEntityList[i].m_tag;
		tmp.m_handle = m_sEntityList[i].m_handle;
		tmp.type = eENTITY_SPC_NULL; 
		tmpEntityList.push_back(tmp);
		if (m_sEntityList[i].m_handle == handle)
		{
			m_sEntityList.remove(i);
			return;
		}
	}
	TRACE( _T("can't find actor(%I64x) entity in visible list!"), Uint64(handle));
}

int StrengthCompareFn(const void* lhs, const void* rhs)
{
	__int64 nLVal = *((__int64 *)lhs);
	__int64 nRVal = *((__int64 *)rhs);
	if (nLVal < nRVal)
		return -1;
	else if (nLVal == nRVal)
		return 0;
	return 1;
}

//定时调用
VOID CObserverSystem::OnTimeCheck(TICKCOUNT nTick)
{
	DECLARE_TIME_PROF("CObserverSystem::OnTimeCheck");
	if(m_pEntity == NULL )
	{ 
		return;
	}
	
	EntityHandle hSelfHandle = m_pEntity->GetHandle();
	int nSelfType = hSelfHandle.GetType();
	bool bIsActor = (enActor == nSelfType) ? true : false;
	INT_PTR nOldUserCount = 0; // 本次更新中没有发生变化的实体数量
	// 对于NPC和采集怪，都不用处理
	if (enNpc == nSelfType)
	{  
		return;	
	}
	else
	{ 
		if (enActor == nSelfType && ((CActor*)m_pEntity)->CanNetworkCutDown())
		{ 
			NetworkCutDown(nTick);
			return;
		}
	}
	
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	bool bActorInNearby = true;
	if (enActor != nSelfType)	// // 对于怪物，直接保存周围玩家列表
	{			
		// 此方式为定时去更新实体列表。 当前采取玩家更新观察者的时候去主动更新其他实体的观察者列表
		nOldUserCount = m_sEntityList.count();
	}
	else
	{		
		{
			DECLARE_TIME_PROF("get list");
			CFuBenManager* fm = GetGlobalLogicEngine()->GetFuBenMgr();			
			fm->GetSeqVisibleList(m_pEntity, s_orderVisibleSet, -MOVE_GRID_ROW_RADIO, 
				MOVE_GRID_ROW_RADIO, -MIN_MOVE_GRID_COL_RADIO, MIN_MOVE_GRID_COL_RADIO);
		}
	
		INT_PTR nNewEntityCount = s_orderVisibleSet.size();
		{
			DECLARE_TIME_PROF("other handle size");			
			EntityHandleTag entityTag;
			entityTag.m_handle	= EntityHandle(); 

			entityTag.m_tag		= eENTITY_APPEAR_NEW;			
			s_OBEntityList->clear();
			INT_PTR nOldEntityCount = m_sEntityList.count();		// 上次周围实体数量
			INT_PTR i = 0;
			
			SequenceEntityListIter iter		= s_orderVisibleSet.begin();
			SequenceEntityListIter iterEnd	= s_orderVisibleSet.end();

#ifndef _DEBUG
			// const char * name = m_pEntity->GetEntityName();
			// OutputMsg(rmTip,_T("\n【%s】位置检查, 上一次%d人, 新检测%d人!!"), name, 
			// 	nOldEntityCount,
			// 	nNewEntityCount
			// );

			// for (size_t i = 0; i < m_sEntityList.count(); i++)
			// {				
			// 	CEntity *pEntity = em->GetEntity(m_sEntityList[i].m_handle);
			// 	const char * tname = pEntity ? pEntity->GetEntityName() : "no actor";
			// 	OutputMsg(rmTip, _T("上一次: %s"), tname);
			// }
			
			// for (; iter != iterEnd; iter++)
			// {				
			// 	CEntity *pEntity = em->GetEntity(*iter);
			// 	const char * tname = pEntity ? pEntity->GetEntityName() : "no actor";
			// 	OutputMsg(rmTip, _T("新检测: %s"), tname);
			// }
#endif
			// 比较
			iter = s_orderVisibleSet.begin();
			while (i < nOldEntityCount && iter != iterEnd)
			{
				EntityHandle newHandle = *iter;
				EntityHandleTag& tag = m_sEntityList[i];
				
				CEntity *pEntity = em->GetEntity(newHandle);
#ifndef _DEBUG
				// if (pEntity)
				// {
				// 	int nScenceId = pEntity->GetSceneID();//pEntity->m_entityPos.pScene->m_pSrcSceneData->nScenceId;
				// }
				// EntityHandle oldHandler = tag.m_handle;
#endif
				if (tag.m_handle == newHandle)
				{
					
					bool canSee = pEntity ? m_pEntity->CanSee(pEntity) : false;
					if (!pEntity)
					{
						OutputMsg(rmError, _T("视野里出现的实体的指针为空,type=%d"), newHandle.GetType());
						CScene* pScene = m_pEntity->GetScene();
						if (pScene)
							pScene->ClearIncorrectHandle(m_pEntity);
					}

					if (canSee)
					{
						tag.m_tag = eENTITY_APPEAR_UNCHANGE;
						s_OBEntityList->push(tag);
					}
					else
					{
						EntityDisappear(newHandle);
					}
										
					nOldUserCount++;
					i++;
					++iter;
				}
				else if (tag.m_handle < newHandle)
				{			
					EntityDisappear(tag.m_handle);
					i++;
				}
				else /*if (tag.m_handle > tmpHandle)*/
				{					
					if (newHandle == hSelfHandle)
					{
						++iter;
						continue;
					}

					bool bCanSee = pEntity ? m_pEntity->CanSee(pEntity) : false;
					if (!pEntity)
					{
						OutputMsg(rmError, _T("视野里出现的实体的指针为空,type=%d"), newHandle.GetType());
						CScene* pScene = m_pEntity->GetScene();
						if (pScene)
							pScene->ClearIncorrectHandle(m_pEntity);
					}
					if (bCanSee)
					{
						EntityAppear(newHandle, pEntity);						
						entityTag.m_handle	= newHandle;						
						s_OBEntityList->push(entityTag);					
					}				
					++iter;
				}				
			}

			// 剩余未消失完的，全部消失
			while (i < nOldEntityCount)
			{
				EntityDisappear(m_sEntityList[i].m_handle);
				++i;
			}

			// 剩余未出现完的，全部出现
			while (iter != iterEnd)
			{	
				EntityHandle newHandle = *iter;
				if (newHandle == hSelfHandle)
				{	
					++iter;
					continue;
				}
				
				CEntity *pEntity = em->GetEntity(newHandle);
				bool canSee = pEntity ? m_pEntity->CanSee(pEntity) : false;
				if (!pEntity)
				{
					OutputMsg(rmError, _T("视野里出现的实体的指针为空,type=%d"), newHandle.GetType());
					CScene* pScene = m_pEntity->GetScene();
					if (pScene)
						pScene->ClearIncorrectHandle(m_pEntity);
				}
				if (canSee)
				{					
					EntityAppear(newHandle, pEntity);					
					entityTag.m_handle	= newHandle;					
					s_OBEntityList->push(entityTag);		
				}
				++iter;
			}

			// 切换s_OBEntityList和m_sEntityList
			{
				m_sEntityList.clear();
				m_sEntityList.addList(*s_OBEntityList);
			}
		}
	}
	//跨地图传送需要在npc出现后 才给客户端发送 传送结果
	if(bIsActor) 
	{
		((CActor*)m_pEntity)->GetQuestSystem()->SendQuestTelResult();
		((CActor*)m_pEntity)->GetMoveSystem()->SendHandFlyShoesRelust(); 
	}
		
	if (enActor != nSelfType && !bActorInNearby)
		return;

	CheckEntitypPropery(nOldUserCount, m_sEntityList);
}

void CObserverSystem::NetworkCutDown(TICKCOUNT nTick)
{
	int limit_count = ((CActor*)m_pEntity)->CanNetworkCutDownDisplayCount();//限制数量 
	bool limit_full = false;//是否人数已满

	EntityHandle hSelfHandle = m_pEntity->GetHandle();
	int nSelfType = hSelfHandle.GetType();
	bool bIsActor = (enActor == nSelfType) ? true : false;
	INT_PTR nOldUserCount = 0; // 本次更新中没有发生变化的实体数量
	// 对于NPC和采集怪，都不用处理
	if (enNpc == nSelfType) return;	
	
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	bool bActorInNearby = true;
	if (enActor != nSelfType)	// // 对于怪物，直接保存周围玩家列表
	{			
		// 此方式为定时去更新实体列表。 当前采取玩家更新观察者的时候去主动更新其他实体的观察者列表
		nOldUserCount = m_sEntityList.count();
	}
	else
	{		
		{
			DECLARE_TIME_PROF("get list");
			CFuBenManager* fm = GetGlobalLogicEngine()->GetFuBenMgr();			
			fm->GetSeqVisibleList(m_pEntity, s_orderVisibleSet, -MOVE_GRID_ROW_RADIO, 
				MOVE_GRID_ROW_RADIO, -MIN_MOVE_GRID_COL_RADIO, MIN_MOVE_GRID_COL_RADIO);
		}
	
		INT_PTR nNewEntityCount = s_orderVisibleSet.size();
		{
			DECLARE_TIME_PROF("other handle size");			
			EntityHandleTag entityTag;
			entityTag.m_handle	= EntityHandle();
			entityTag.m_tag		= eENTITY_APPEAR_NEW;			
			s_OBEntityList->clear();
			INT_PTR nOldEntityCount = m_sEntityList.count();		// 上次周围实体数量
			INT_PTR i = 0;
			
			SequenceEntityListIter iter		= s_orderVisibleSet.begin();
			SequenceEntityListIter iterEnd	= s_orderVisibleSet.end();
 
			std::vector<EntityHandleTagEx> tmpEntityList; // 周围实体列表（采取数组的方式存储）
			std::map<unsigned long long, unsigned long long> tmpMapList;
			std::map<unsigned long long, unsigned long long> tmpAllMapList;
			
			//OutputMsg(rmWaning, _T("%lld视野里出现的实体数量 ：%lld"), nTick, (unsigned long long)nOldEntityCount);
			// CScene* pScene = m_pEntity->GetScene();
			// if (pScene)
			// {		
			// 	CFuBen * pFb= pScene->GetFuBen();
			// 	if(pFb)
			// 	{
			// 		OutputMsg(rmWaning, _T("%lld特殊场景场景 ：%lld 副本 ：%lld"), nTick, (int)pScene->GetSceneId(), (int)pFb->GetFbId());
			// 	} 
			// } 

			for(INT_PTR i = 0 ; i < nOldEntityCount; i++)
			{
				bool is_special = false;
				EntityHandleTag& tag = m_sEntityList[i];  
				
				unsigned long long handle001 = (unsigned long long)tag.m_handle.GetHandle();
				if (tmpAllMapList.insert(std::make_pair(handle001, handle001)).second)
				{
					//OutputMsg(rmWaning, _T("%lld++++++++++++++                         进入管理的对象000 ：%lld"), nTick, handle001); 
				}
				else
				{ 
					OutputMsg(rmWaning, _T("%lld特殊场景重复id000 ：%lld"), nTick, handle001);
				}
				//OutputMsg(rmWaning, _T("%lld视野的实体009999 ：%lld"),  nTick, (unsigned long long)tag.m_handle.GetHandle());
				CEntity *pEntity = em->GetEntity(tag.m_handle); 
				bool canSee = pEntity ? m_pEntity->CanSee(pEntity) : false;
				if (!pEntity)
				{
					OutputMsg(rmError, _T("%lld视野里出现的实体的指针为空1,type=%d"), nTick, tag.m_handle.GetType());
					CScene* pScene = m_pEntity->GetScene();
					if (pScene)
					{
						pScene->ClearIncorrectHandle(m_pEntity); 
						((CActor*)m_pEntity)->OnClearCanSeeEntity(tag.m_handle);
					} 
				}
				if (!canSee)
				{
					EntityDisappear(tag.m_handle); 
					((CActor*)m_pEntity)->OnClearCanSeeEntity(tag.m_handle);
					//OutputMsg(rmError, _T("%lld视野里出现的实体准备消失001 ：%lld"), nTick, (unsigned long long)tag.m_handle.GetHandle());
					//tmpEntityList.push_back(tag);
					continue; //自然消失的
				}

				//特殊怪物ID
				if ( pEntity && pEntity->GetType() == enMonster)// && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 385)
				{   
					is_special = true;
				}
				
				if(!is_special && limit_count <= s_OBEntityList->count())
				//if(!is_special && limit_count <= s_OBEntityList->count())
				{ 
					limit_full = true;//人数满 
					//EntityDisappear(tag.m_handle);
					EntityHandleTagEx tmp;
					tmp.m_handle = tag.m_handle;
					tmp.m_tag = tag.m_tag;
					tmp.type = eENTITY_SPC_FULL_DEL;  
					tmpEntityList.push_back(tmp); 
					//OutputMsg(rmError, _T("%lld视野里出现的实体准备消失002 ：%lld"), nTick,(unsigned long long)tag.m_handle.GetHandle());
					continue;
				}
 
				// if ( pEntity && pEntity->GetType() == enMonster)
				// {  
				// 	OutputMsg(rmWaning, _T("%lld---------1655165--------场景中得怪物ID ：%lld  "), nTick, (int)pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
				bool CanFind = false;
				SequenceEntityListIter itNew = s_orderVisibleSet.begin();
				for(; itNew != iterEnd; itNew++)
				{
					EntityHandle newHandle = *itNew;
					
					if (tag.m_handle == newHandle)//老的新的对上了
					{
						tag.m_tag = eENTITY_APPEAR_UNCHANGE;
						
						nOldUserCount++;
						CanFind = true;
						
						// EntityHandleTagEx tmp;
						// tmp.m_tag = tag.m_tag;
						// tmp.m_handle = tag.m_handle; 
						unsigned long long handle = (unsigned long long)tag.m_handle.GetHandle();
						if (tmpMapList.insert(std::make_pair(handle, handle)).second)
						{
							//OutputMsg(rmWaning, _T("%lld进入管理的对象001 ：%lld"), nTick, handle);
							s_OBEntityList->push(tag);
						}
						else
						{ 
							OutputMsg(rmError, _T("%lld重复id001 ：%lld"), nTick, handle);
						}
						//OutputMsg(rmTip, _T("%lld--------------视野里出现的加入新实体准备001 ：%lld"), nTick, (unsigned long long)tag.m_handle.GetHandle());
						break;
					}
				}  
				if (!CanFind)
				{
					EntityHandleTagEx tmp;
					tmp.m_tag = tag.m_tag;
					tmp.m_handle = tag.m_handle;
					tmp.type = eENTITY_SPC_OLD_DEL; 
					//EntityDisappear(tag.m_handle);  
					tmpEntityList.push_back(tmp); 
 
					//OutputMsg(rmError, _T("%lld视野里出现的实体准备消失003 ：%lld"), nTick, (unsigned long long)tag.m_handle.GetHandle());
				}
			}
			for(INT_PTR i = 0 ; i < tmpEntityList.size(); i++)
			{  
				EntityHandleTagEx& tag = tmpEntityList[i]; 
				EntityDisappear(tag.m_handle);
				((CActor*)m_pEntity)->OnClearCanSeeEntity(tag.m_handle);
			}
			nOldEntityCount = m_sEntityList.count();//实体变量已经变化
			
			//OutputMsg(rmError, _T("%lld视野里出现的实体数量变化221 ：%d"), nTick, (int)tmpMapList.size());
			//OutputMsg(rmError, _T("%lld视野里出现的实体数量变化222 ：%lld"), nTick, (unsigned long long)nOldEntityCount);
			EntityHandle bEntBuffer[300];
			EntityHandle bEntData[300];
			memset(bEntBuffer, 0, sizeof(bEntBuffer));
			memset(bEntData, 0, sizeof(bEntData));
			SequenceEntityListIter itcheck = s_orderVisibleSet.begin();
			int bNewCount = 0;
			for(; itcheck != iterEnd; itcheck++)
			{ 
				if(bNewCount > 300)
				{
					break;
				}
				EntityHandle newHandle = *itcheck;
				//unsigned long long handleTmp = (unsigned long long)newHandle.GetHandle();
				bEntData[bNewCount] = newHandle;
				bNewCount ++;
			} 
			BYTE bRandCount = 0, bPosition = 0;
			if(bNewCount > 0)
			{ 
				do
				{
					bPosition = rand() % (bNewCount - bRandCount);
					bEntBuffer[bRandCount++] = bEntData[bPosition];
					bEntData[bPosition] = bEntData[bNewCount - bRandCount];
				} while (bRandCount < bNewCount); 
			}

			//itcheck = s_orderVisibleSet.begin();
			//for(; itcheck != iterEnd; itcheck++)
			for(int k = 0; k < bRandCount; k++)
			{  
				//EntityHandle newHandle = *itcheck;
				EntityHandle& newHandle = bEntBuffer[k];

				bool is_special = false;
				if (newHandle == hSelfHandle)//自己的 不判断
				{	
					//++itcheck;
					continue;
				}
				
				unsigned long long handle001 = (unsigned long long)newHandle.GetHandle();
				if(tmpAllMapList.find(handle001) != tmpAllMapList.end())
				{ 
					continue;//已经处理过了
				}
				// for(INT_PTR i = 0 ; i < nOldEntityCount; i++)
				// {
				// 	EntityHandleTag& tagtmp = m_sEntityList[i];
				// 	if (tagtmp.m_handle == newHandle)//老的新的对上了
				// 	{
				// 		continue;//已经处理过了
				// 	}
				// }
				for(INT_PTR i = 0 ; i < tmpEntityList.size(); i++)
				{  
					EntityHandleTagEx& tagtmp = tmpEntityList[i]; 
					if (tagtmp.m_handle == newHandle)
					{
						continue;//已经处理过了
					}
				}

				CEntity *pEntity = em->GetEntity(newHandle);

				if (!pEntity)
				{
					OutputMsg(rmError, _T("%lld视野里出现的实体的指针为空222, type = %d"), nTick, newHandle.GetType());
					CScene* pScene = m_pEntity->GetScene();
					if (pScene)
					{
						pScene->ClearIncorrectHandle(m_pEntity);
						((CActor*)m_pEntity)->OnClearCanSeeEntity(newHandle);
					}
					//EntityDisappear(newHandle);
					continue;
				}

				bool canSee = pEntity ? m_pEntity->CanSee(pEntity) : false;
				if (!canSee)
				{
					//EntityDisappear(newHandle);
					continue;
				}							
				//特殊怪物ID
				if ( pEntity && pEntity->GetType() == enMonster)// && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 386)
				{  
					is_special = true;
				}	
				if(!is_special && limit_count <= s_OBEntityList->count())
				{
					limit_full = true;//人数满 
					//EntityDisappear(newHandle);
					continue;
				}
 
				((CActor*)m_pEntity)->OnAddCanSeeEntity(newHandle);
				EntityAppear(newHandle, pEntity);
				entityTag.m_handle	= newHandle; 
				
				// EntityHandleTagEx tmp;
				// tmp.m_tag = entityTag.m_tag;
				// tmp.m_handle = entityTag.m_handle;  
				unsigned long long handle = (unsigned long long)entityTag.m_handle.GetHandle();
				if (tmpMapList.insert(std::make_pair(handle, handle)).second)
				{
					//OutputMsg(rmWaning, _T("%lld进入管理的对象222 ：%lld"), nTick, handle);
					s_OBEntityList->push(entityTag);
				}
				else
				{ 
					OutputMsg(rmError, _T("%lld重复id222 ：%lld"), nTick, handle);
				}

				//OutputMsg(rmTip, _T("%lld--------------视野里出现的加入新实体准备222 ：%lld"), nTick, (unsigned long long)entityTag.m_handle.GetHandle());
			}
 
			// 切换s_OBEntityList和m_sEntityList
			{
				m_sEntityList.clear();
				m_sEntityList.addList(*s_OBEntityList);
			}
			nOldEntityCount = m_sEntityList.count();//实体变量已经变化
		}
	}
	//跨地图传送需要在npc出现后 才给客户端发送 传送结果
	if(bIsActor) 
	{
		((CActor*)m_pEntity)->GetQuestSystem()->SendQuestTelResult();
		((CActor*)m_pEntity)->GetMoveSystem()->SendHandFlyShoesRelust(); 
	}
		
	if (enActor != nSelfType && !bActorInNearby)
		return;

	CheckEntitypPropery(nOldUserCount, m_sEntityList);
}
void CObserverSystem::UpdateActorEntityProp()
{
	CheckEntitypPropery(m_sEntityList.count(), m_sEntityList);
}

//检测实体的属性的改变，如果改变了，需要通知视野里的玩家更新属性
void CObserverSystem::CheckEntitypPropery(INT_PTR nOldUserCount, CVector<EntityHandleTag> &vecVisiblePlayers)
{	
	if(m_pEntity == NULL) return ;
	if(m_pEntity->IsInited() == false) 
		return;

	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	CUpdateMask *pMask = (CUpdateMask *)m_pEntity->GetPropertyPtr() ; //本身就是一个更新集合
	INT_PTR nUsedCount = pMask->GetUsedCount();
	if(nUsedCount == 0) return; //没有更新
			
	unsigned int nTeamId	= 0;
	CTeam *pTeam			= NULL;
	INT_PTR nTeamOnlinePlayerCnt = 0;
	if (m_pEntity->GetHandle().GetType() == enActor)
	{
		// 计算属性数量
		INT_PTR nSendCount = 0; //需要向客户端广播的属性的个数
		INT_PTR nTotalCount = nUsedCount << 3; //*8
		for (INT_PTR i=0;i < nTotalCount; i++)
		{
			if(pMask->GetUpdateMaskFlag(i))
				nSendCount++;
		}
		// 打包属性
		CActor *pActor = (CActor *) m_pEntity;
		nTeamId = pActor->GetProperty<int>(PROP_ACTOR_TEAM_ID);
		pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{ 
			nTeamOnlinePlayerCnt = pTeam->GetOnlineUserCount();
		}
		if (nSendCount > 0)
		{ 
			// 发给玩家自己
			CActorPacket pack;
			CDataPacket &packet =  pActor->AllocPacket(pack);
			packet <<BYTE(enDefaultEntitySystemID) <<(BYTE)(sMainactorPropertyChange) << (BYTE)nSendCount; //主角的属性发生改变
			for(INT_PTR i=0;i < nTotalCount; i++)
			{
				if(pMask->GetUpdateMaskFlag(i))
				{
					packet << (unsigned char) (i);		
					/*
					if(i==9  )
					{
						OutputMsg(rmWaning,_T("HP"));
					}
					if (i==133 || i == 134)
					{
						OutputMsg(rmWaning,_T("HPMMMMMMMMMMMMMMMM="));
					}
					*/
					/*
					高频调用方法，需要增加条件筛选掉不必要处理的逻辑。
					否则可会导致导量时出现服务器负载过重的情况。
					*/
					const char * dataPtr =  pActor->GetPropertyPtr()->GetValuePtr(i); //*2
					packet.writeBuf(dataPtr,4); //
				}
			}
			pack.flush(); 
		}
	}
	
	CUpdateMask *pBroadCastMask= m_pEntity->GetBroadCastMask();
	if(pBroadCastMask ==NULL) return;
	*pMask &= *pBroadCastMask; //与运算
	nUsedCount = pMask->GetUsedCount();
	if (nUsedCount > 0)	
		SendPropChangeToOwner();		// 广播给归属者（如果归属者不在附近）
	 
	// 视野内部没有玩家并且没有在队伍中
	if(nOldUserCount == 0 && nTeamOnlinePlayerCnt <= 0)
	{
		pMask->ClearUpdateMask();	//zac：这句代码会导致一个bug：在同屏瞬移中会暂时把玩家的可视列表清空，如果这个时候属性改变，则无法通知到同屏的玩家，所以这里不需要清除updatemask
		return; //视野里没有玩家就算属性改变了也不处理
	}
	
	char buff[1024];
	CDataPacket data(buff,sizeof(buff));//把玩家的数据发过去	
	if(nUsedCount >0)
	{	
		INT_PTR nTotalCount = nUsedCount << 3; //*8
		INT_PTR nSendCount =0; //需要向客户端广播的属性的个数
		for(INT_PTR i=0;i < nTotalCount; i++)
		{
			if(pMask->GetUpdateMaskFlag(i))
			{
				nSendCount ++;
			}
		}
		if(nSendCount >0)
		{
			data <<BYTE(enDefaultEntitySystemID) << BYTE(sEntityPropertyChange) << Uint64(m_pEntity->GetHandle());
			data <<  (BYTE) nSendCount;
			for(INT_PTR i=0;i < nTotalCount; i++)
			{
				if(pMask->GetUpdateMaskFlag(i))
				{
					const char* dataPtr = m_pEntity->GetPropertyPtr()->GetValuePtr(i); //*2
					data << (unsigned char ) i;
					/*
					if(i==7 && m_pEntity->GetType() ==enMonster )
					{
						unsigned int nHp = *(int*)dataPtr;
						OutputMsg(rmWaning,_T("HP=%d"),nHp);
					}
					*/

 					data.writeBuf(dataPtr,4); //这里要改一下
				}
			}
			// 广播给周围玩家
			BroadCast(data.getMemoryPtr(), data.getPosition());
			// 广播给队友
			if (nTeamOnlinePlayerCnt > 1 && pTeam)
			{
				if (pMask->GetUpdateMaskFlag(PROP_CREATURE_HP))//只更新血量
				{
					pTeam->BroadcastUpdateMember((CActor*)m_pEntity, PROP_CREATURE_HP);
				}
				
			}
		}		

		pMask->ClearUpdateMask();
	}
}

bool CObserverSystem::IsOwnerActor()
{
	bool bOwnerIsActor	= false;												// 是否归属者者是玩家	
	// 归属者对象指针
	INT_PTR nType = m_pEntity->GetType() ;
	if (nType== enMonster)
	{
		/*CEntity *pOwner = GetEntityFromHandle(((CMonster *)m_pEntity)->GetOwner());
		if (pOwner && pOwner->GetType() == enActor)
		{
			bOwnerIsActor	= true;
			pOwnerActor		= (CActor *)pOwner;
		}*/
		EntityHandle hOwner = ((CMonster *)m_pEntity)->GetOwner();
		if (!hOwner.IsNull() && hOwner.GetType() == enActor)
			bOwnerIsActor = true;
	}
	else if(nType ==enPet || nType == enHero)
	{
		return true;		
	}

	return bOwnerIsActor;
}

void CObserverSystem::SendPropChangeToOwner()
{
	if (!IsOwnerActor()) return;
	CUpdateMask *pMask			= (CUpdateMask *)m_pEntity->GetPropertyPtr();
	CUpdateMask *pBroadCastMask = m_pEntity->GetBroadCastMask();
	if (!pMask || !pBroadCastMask) return;
	*pMask &= *pBroadCastMask;
	UINT_PTR nUsedCount = pMask->GetUsedCount();
	if (nUsedCount <= 0)
		return;

	INT_PTR nTotalCount = nUsedCount << 3;
	INT_PTR nSendCount	= 0;
	for (INT_PTR i=0;i < nTotalCount; i++)
	{
		if(pMask->GetUpdateMaskFlag(i))
			nSendCount ++;
	}
	if (nSendCount <= 0)
		return;
		
	CActor *pOwnerActor =NULL;
	if(m_pEntity->IsMonster())
	{
		pOwnerActor= (CActor *)GetEntityFromHandle(((CMonster *)m_pEntity)->GetOwner());
	}
	else if (m_pEntity->GetType() == enHero)
	{
		pOwnerActor= ((CHero*)m_pEntity)->GetMaster();
		
	}
	
	if (!pOwnerActor ) return;
	if(pOwnerActor->IsInited() ==false) return;

	// 同步属性更新到归属者
	char buff[1024];
	CDataPacket data(buff, sizeof(buff));
	data <<BYTE(enDefaultEntitySystemID) << BYTE(sBabyPropChange) << Uint64(m_pEntity->GetHandle());
	data << (BYTE) nSendCount;
	for (INT_PTR i = 0; i < nTotalCount; i++)
	{
		if(pMask->GetUpdateMaskFlag(i))
		{
			const char* dataPtr = m_pEntity->GetPropertyPtr()->GetValuePtr(i);
			data << (unsigned char)i;
			data.writeBuf(dataPtr, 4);
		}
	}
	pOwnerActor->SendData(data.getMemoryPtr(), data.getPosition());	
}


void CObserverSystem::SendPosChangeToOwner()
{
	//客户端其他地方实现了，不使用
	/*INT_PTR nType = m_pEntity->GetType() ;
	bool bOwnerIsActor =false;
	CActor *pOwnerActor  =NULL;
	if ( nType== enMonster)
	{
		EntityHandle hOwner = ((CMonster *)m_pEntity)->GetOwner();
		if (!hOwner.IsNull() && hOwner.GetType() == enActor)
		{
			pOwnerActor =  (CActor *)GetEntityFromHandle(hOwner);
		}
	}
	else if(nType ==enPet )
	{
		pOwnerActor = ((CPet*)m_pEntity)->GetMaster();
	}
	else if(nType ==enHero)
	{
		pOwnerActor = ((CHero*)m_pEntity)->GetMaster();
	}
	else
	{
		return;
	}
	
	if (!pOwnerActor || !pOwnerActor->IsInited()) return;

	char buff[1024];
	CDataPacket data(buff, sizeof(buff));
	data <<BYTE(enDefaultEntitySystemID) << BYTE(sBabyPosChange) << Uint64(m_pEntity->GetHandle());
	int nPosX, nPosY, nSceneId = -1;
	m_pEntity->GetPosition(nPosX, nPosY);
	CScene *pScene = m_pEntity->GetScene();
	if (pScene)
		nSceneId = pScene->GetSceneId();
	data << (unsigned short)nPosX << (unsigned short)nPosY << nSceneId;	
	pOwnerActor->SendData(data.getMemoryPtr(), data.getPosition());	*/
}

void CObserverSystem::EntityAppearActor(const EntityHandle &handle, CActor* pOtherActor)
{
	if(!m_pEntity || !pOtherActor)
	{ 
		return;
	}

	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	CActor *pSelf = (CActor *) m_pEntity;
	CScene* pScene = pOtherActor->GetScene();
	WORD nPosX = (WORD)pOtherActor->GetProperty<int>(PROP_ENTITY_POSX);
	WORD nPosY = (WORD)pOtherActor->GetProperty<int>(PROP_ENTITY_POSY);
	int nSex = pOtherActor->GetProperty<int>(PROP_ACTOR_SEX);
	

	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data = pSelf->AllocPacket(pack);
	CActorProperty * pProperty = (CActorProperty *) pOtherActor->GetPropertyPtr(); //玩家的属性集
	data <<BYTE(enDefaultEntitySystemID) <<(BYTE)sOtherActorAppear; //是别的玩家出现了
	data <<(Uint64) handle ;												//玩家句柄

	bool isset = false;
	if (pScene)
	{
		SCENEAREA* pArea = pScene->GetAreaAttri(nPosX, nPosY);
		if (pScene->HasMapAttribute(nPosX, nPosY, aaChangeName, pArea) )
		{
			AreaAttri& attri = pArea->attri[aaChangeName];
			if (nSex >= attri.nCount) nSex = attri.nCount - 1;
			
			int nIdx = attri.pValues[nSex];
			if (LPCTSTR pName = GetLogicServer()->GetDataProvider()->GetChangeName(nIdx-1))
			{
				data.writeString(pName);
				isset = true;
			}
		}
	}
	
	if (!isset)
		data.writeString(pOtherActor->GetShowName()); 						//玩家的名字写入

	data <<nPosX;  			//x
	data <<nPosY; 			//y
	data <<(int)pOtherActor->GetProperty<int>(PROP_ENTITY_MODELID);			//模型id

	data << (unsigned)pOtherActor->GetProperty<unsigned>(PROP_CREATURE_HP); //当前的血
	data << (unsigned)pOtherActor->GetProperty<unsigned>(PROP_CREATURE_MP); //当前的蓝

	data << (unsigned)pOtherActor->GetProperty<unsigned>(PROP_CREATURE_MAXHP); //最大的血
	data << (unsigned)pOtherActor->GetProperty<unsigned>(PROP_CREATURE_MAXMP); //最大的蓝

	data << (WORD) pOtherActor->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME); //移动速度
	data << (BYTE) nSex;     				//性别
	data << (BYTE) pOtherActor->GetProperty<unsigned>(PROP_ACTOR_VOCATION); 			//职业
	data << (WORD) pOtherActor->GetProperty<unsigned>(PROP_CREATURE_LEVEL);				//等级
	data << (unsigned int)pOtherActor->GetProperty<unsigned>(PROP_ACTOR_CIRCLE);		//转数
		
	int nWeapon = pOtherActor->GetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE);	//武器外观
	int nSwing = pOtherActor->GetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE);		//翅膀外观
	if (pOtherActor->IsSceneChangeModel())
	{
		nSwing = 0;
	}
	data << (int)nWeapon;													//武器外观
	data << (int)nSwing;													//翅膀外观
	data << (int)pOtherActor->GetProperty<int>(PROP_ACTOR_SOCIALMASK); 		//玩家社会关系
	data << (WORD)pOtherActor->GetProperty<int>(PROP_ENTITY_ICON);			//头像ID
	data << (WORD)pOtherActor->GetProperty<int>(PROP_CREATURE_ATTACK_SPEED);//攻击速度

	data << (BYTE)pOtherActor->GetProperty<unsigned int>(PROP_ENTITY_DIR); 				//下发朝向
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_CREATURE_STATE); 	//状态
	data << (unsigned int)0;
	data << (int)pOtherActor->GetProperty<int>(PROP_ACTOR_TEAM_ID);						//队伍的ID
	data << (BYTE) pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_ZY);				//阵营类型
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_CURNEWTITLE);//头衔
	data << (unsigned int)(GetNameColorLookedByOther(pOtherActor->GetHandle(), m_pEntity->GetHandle()));//名字颜色
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL);//超玩俱乐部等级
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE);	//兵魂外观
	data << (WORD)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_WEAPON_ID);						//正在装备的武器id
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);				//行会ID
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_MONSTER_MODELID);		//怪物模型id
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_MERITORIOUSPOINT);		//累计功勋
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_NEXT_SKILL_FLAG);		//战士下一次技能flag
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_EVIL_PK_STATUS);		//恶意PK状态
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE);				//PK值

	pOtherActor->GetBuffSystem()->AppendBuffData(data);												//玩家的Buff数据
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ENTITY_ID);				//玩家id
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_AREA_ATTR);	 //区域属性存储位
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_AREA_ATTR_0);
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_NORMAL_ATK_ACC);
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_FRENZY);
	data << (unsigned int)pOtherActor->GetProperty<unsigned int>(PROP_ACTOR_OFFICE);

	////宠物系统非 ai宠物
	data << (int)pOtherActor->GetProperty<int>(PROP_ACTOR_LOOT_PET_ID);
	data << (int)pOtherActor->GetProperty<int>(PROP_ACTOR_CURCUSTOMTITLE);

	pack.flush();
}

void CObserverSystem::EntityAppearDropItem(const EntityHandle &handle, CEntity* pItem)
{
	if(!m_pEntity|| !pItem)return;
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);
	CDropItemEntity* pOtherItem = (CDropItemEntity*)pItem;
	data <<BYTE(enLootSystemID) <<(BYTE)sDropItemAppear; //
	data << (unsigned int) handle.GetIndex();
	CUserItem * pUserItem = pOtherItem->GetItem();
	int nLeftTime = 0;

	unsigned int nMasterId = pOtherItem->GetMasterActorId();
	if (!nMasterId || nMasterId != m_pEntity->GetId())
	{
		nLeftTime = pOtherItem->GetCanPickUpTime() - GetGlobalLogicEngine()->getTickCount();
	}
	
	unsigned int nTeamId = pOtherItem->GetTeamId();
	unsigned int mytemid = pSelf->GetProperty<unsigned int >(PROP_ACTOR_TEAM_ID);
	if (nTeamId && nTeamId == mytemid)
	{
		nLeftTime = 0;
		// nLeftTime = pOtherItem->GetCanPickUpTime() - GetGlobalLogicEngine()->getTickCount();
	}
	if(nLeftTime < 0)
		nLeftTime = 0;

	if(pUserItem)
	{
		data <<(WORD)pUserItem->wItemId;										 //物品id
		data <<(WORD)pOtherItem->GetProperty<int>(PROP_ENTITY_POSX);    		 //坐标x
		data <<(WORD)pOtherItem->GetProperty<unsigned int>(PROP_ENTITY_POSY);    //坐标y
		data << (unsigned int)(nLeftTime/1000);									 //剩余时间
		data << (BYTE)(*(char*)pUserItem->cBestAttr);							 //是否极品
		data << (WORD)1;														 //数量
	}
	else
	{
		if (pOtherItem->GetMoneyType() == mtYuanbao) data <<(WORD)65535;		 //代表元宝
		else data <<(WORD)65534;												 //代表金币
		data <<(WORD)pOtherItem->GetProperty<int>(PROP_ENTITY_POSX);    		 //坐标x
		data <<(WORD)pOtherItem->GetProperty<unsigned int>(PROP_ENTITY_POSY);    //坐标y
		data << (unsigned int)(nLeftTime/1000);									 //剩余时间
		data << (BYTE)0;														 //是否极品
		data << (WORD)pOtherItem->GetMoneyCount();								 //数量
	}
	pack.flush();
}

void CObserverSystem::EntityAppearSpecialEntity(const EntityHandle &handle, CEntity* pSpecialEntity)
{
	if(!m_pEntity|| !pSpecialEntity)return;
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);

	data <<BYTE(enDefaultEntitySystemID) <<(BYTE)sSpecialEntityAppear; 
	data <<(Uint64) handle ;
	data << (WORD)pSpecialEntity->GetProperty<int>(PROP_ENTITY_MODELID); //模型的ID
	data <<(WORD)pSpecialEntity->GetProperty<int>(PROP_ENTITY_POSX);    //坐标x
	data <<(WORD)pSpecialEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);    //坐标y
	data.writeString(pSpecialEntity->GetEntityName());

	pack.flush();
}

void CObserverSystem::EntityAppearMonster(const EntityHandle &handle, CEntity* pEntity)
{
	if(!m_pEntity|| !pEntity)
	{
		return;
	} 
	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);

	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sMonsterAppear;
	bool isMonster = pEntity->IsMonster();
	data << (BYTE)nType;			// 类型
	data << (Uint64) handle;		// 句柄
	char sAllName[128];	
	const char *pMasterName = 0;
	WORD nMonsterId = 0;
	if (isMonster)
	{
		EntityHandle hMonster = ((CMonster *)pEntity)->GetOwner();
		if (!hMonster.IsNull())
		{
			CEntity *pMaster = GetEntityFromHandle(hMonster);
			if (pMaster && ! pEntity->GetAttriFlag().DenyShowMasterName)
				pMasterName = pMaster->GetEntityName();
		}
	}
	if(pEntity->GetAttriFlag().DenyShowName)
	{
		sprintf_s(sAllName, sizeof(sAllName), " \\");
	}
	else
	{
		sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",pEntity->GetShowName(), pMasterName ? pMasterName : pEntity->GetTitle());
	}
	data.writeString(sAllName);												//怪物名字	
	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_POSX);				//坐标x
	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);		//坐标y
	data <<(unsigned)pEntity->GetProperty<unsigned int>(PROP_ENTITY_MODELID);//模型id
	data << (BYTE)pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);		//方向	
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_LEVEL);		//等级
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_HP);		//当前hp
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MP);		//当前mp
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXHP);  //最大的血
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXMP);  //最大的蓝
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME); //移动速度
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_ATTACK_SPEED); //攻击速度
	data << (unsigned int) pEntity->GetProperty<unsigned int>(PROP_CREATURE_STATE); //状态
	data << (unsigned int) GetMonsterNameColorByOther(pEntity->GetHandle(), m_pEntity->GetHandle());	//名字颜色
	data << (WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);		//怪物id
	data << (unsigned int)pEntity->GetProperty<unsigned int>(PROP_MONSTER_LIVE_TIMEOUT); //怪物生命到期时间
	((CAnimal*)pEntity)->GetBuffSystem()->AppendBuffData(data);
	data << (BYTE)((CMonster *)pEntity)->GetMonsterType();

	unsigned int nBelongActorId = pEntity->GetProperty<unsigned int>(PROP_MONSTER_BELONG_ID);
	data << nBelongActorId;	 // 非活动副本Boss归属处理
	
	//OutputMsg(rmTip,"EntityAppearMonster, entityId=%d, entityName=%s, nLiveTimeout=%u",
	//	pEntity->GetId(), pEntity->GetEntityName(), nLiveTimeout);

	pack.flush();
}

void CObserverSystem::EntityAppearPet(const EntityHandle &handle, CEntity* pEntity)
{
	if(!m_pEntity|| !pEntity)return;
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);

	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sPetAppear;
	data << (BYTE)nType;
	data <<(Uint64) handle;				// 句柄
	char sAllName[128] = {0};
	unsigned int nMonsterId = 0;

	CActor * pOwner = ((CPet *)pEntity)->GetMaster();
	if (pOwner != NULL)
	{
		sprintf_s(sAllName, sizeof(sAllName), "%s\\%s",pEntity->GetEntityName(), pOwner->GetEntityName());
		data.writeString(sAllName);
	}
	else
	{
		data.writeString(pEntity->GetShowName()); 
	}

	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_POSX);    			//坐标x
	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);    	//坐标y
	data <<(unsigned)pEntity->GetProperty<unsigned int>(PROP_ENTITY_MODELID);  //模型id		
	data << (BYTE)pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);   		//朝向
	data << (BYTE) pEntity->GetProperty<unsigned>(PROP_CREATURE_LEVEL);		//等级
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_HP);		//当前Hp
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MP);		//当前Mp
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXHP); 	//最大的血
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXMP); 	//最大的蓝
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME); 	//移动速度
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_ATTACK_SPEED); 		//攻击速度
	data << (unsigned int)  pEntity->GetProperty<unsigned int>(PROP_CREATURE_STATE); //状态
	data << (unsigned int)  ((CPet*)pEntity)->GetColor(); //颜色

	if(pOwner)
	{
		data << (Uint64) pOwner->GetHandle();		//主人句柄
	}
	else
	{
		data << (Uint64)0;
	}

	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //宠物id
	((CAnimal*)pEntity)->GetBuffSystem()->AppendBuffData(data);

	pack.flush();
}

void CObserverSystem::EntityAppearSlave(const EntityHandle &handle, CEntity* pEntity)
{
	if(!m_pEntity|| !pEntity)return;
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);

	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sEntityAppear;
	data << (BYTE)nType;
	data <<(Uint64) handle;
	char sAllName[128] = {0};
	unsigned int nMonsterId = 0;
	if(nType == enPet)
	{
		CActor * pOwner = ((CPet *)pEntity)->GetMaster();
		if (pOwner != NULL && pOwner->IsSceneChangeModel())
		{
			sprintf_s(sAllName, sizeof(sAllName), "\\%s",pEntity->GetShowName(), pOwner->GetEntityName());
			data.writeString(sAllName);
		}
		else
		{
			data.writeString(pEntity->GetShowName()); 
		}
	} 
	else
	{
		if (nType == enHero && ((CHero*)pEntity)->GetMaster())
		{
			data.writeString(pEntity->GetShowName());
		}else
		{
			data.writeString(pEntity->GetEntityName());
		}
	}

	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_POSX);    //坐标
	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);    //坐标
	data <<(unsigned)pEntity->GetProperty<unsigned int>(PROP_ENTITY_MODELID);  //模型id		
	data << (BYTE)pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);   //朝向
	nMonsterId = pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	data << (BYTE) pEntity->GetProperty<unsigned>(PROP_CREATURE_LEVEL);
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_HP);
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MP);	
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXHP); //最大的血
	data << (unsigned)pEntity->GetProperty<unsigned>(PROP_CREATURE_MAXMP); //最大的蓝
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME); //移动速度
	data << (WORD) pEntity->GetProperty<unsigned>(PROP_CREATURE_ATTACK_SPEED); //攻击速度
	data << (unsigned int)  pEntity->GetProperty<unsigned int>(PROP_CREATURE_STATE); //状态
	data << (unsigned int)  pEntity->GetProperty<unsigned int>(PROP_CREATURE_COLOR); //添加了颜色

	data << (unsigned int)0;
	if(nType == enHero)
	{
		data << (unsigned int)0;
	}
	else
	{
		data << ((CPet*)pEntity)->GetColor();
	}

	BYTE byMTAT = 0;
	byMTAT  = (BYTE)((CAnimal*)pEntity)->GetAttackType();
	data << byMTAT;

	if(nType == enHero)
	{
		CHero *pHero = ((CHero *)pEntity);
		const CHeroSystem::HERODATA* pHeroData = pHero->GetHeroPtr();
		if(pHeroData)
		{
			data << (BYTE)pHeroData->data.bSex;  //性别
			data << (BYTE)pHeroData->data.bVocation;// 职业
			data << (BYTE)pHeroData->data.bCircle;  //转数
			data << pHero->GetProperty<unsigned int>(PROP_ACTOR_WEAPONAPPEARANCE);	//武器外观
			data << pHero->GetProperty<unsigned int>(PROP_ACTOR_SWINGAPPEARANCE);	//翅膀外观
			data << (unsigned int)0;
			data << pHero->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);	
		}
	}
	CActor *pOwner =NULL;
	if( nType == enPet || nType == enHero)
	{
		if(nType == enPet)
		{
			pOwner =   ((CPet*)pEntity)->GetMaster();
		}
		else
		{
			pOwner =   ((CHero*)pEntity)->GetMaster();
		}
		if(pOwner != m_pEntity)
		{
			data << (BYTE)0;
		}
		else
		{
			//((CAnimal*)pEntity)->GetBuffSystem()->WriteBuffData(data); //把宠物的buff数据写进去
			((CAnimal*)pEntity)->GetBuffSystem()->AppendBuffData(data);
		}
	}
	((CAnimal*)pEntity)->GetEffectSystem().AppendEffectData(data);

	if ( nType == enPet || nType == enHero )
	{
		if(pOwner)
		{
			data << (Uint64) pOwner->GetHandle();
			if( pOwner->HideOtherPlayerInfo() && pOwner != m_pEntity )
			{
				data << (BYTE)1;
			}
			else
			{
				data << (BYTE)0;
			}
		}
		else
		{
			data << (Uint64)0;
			data << (BYTE)0;  //不隐藏名字
		}
	}
	data <<(WORD)nMonsterId;
	
	//没有属性也要发""或者0，不允许没有值
	data.writeString("");
	data << (unsigned int)0;

	pack.flush();
}

void CObserverSystem::EntityAppearNpc(const EntityHandle &handle, CEntity* pEntity)
{
	if(!m_pEntity|| !pEntity)
	{
		return;
	}
	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	CActor *pSelf = (CActor *) m_pEntity;
	//INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);
	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sNPCAppear;
	data << (Uint64) handle;
	char sAllName[128];	
	const char *pMasterName = NULL;
	if(pEntity->GetAttriFlag().DenyShowName)
	{
		sprintf_s(sAllName, sizeof(sAllName), " \\");
	}
	else
	{
		sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",pEntity->GetShowName(), pMasterName ? pMasterName : pEntity->GetTitle());
	}
	data.writeString(sAllName);
	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_ID);    					//id
	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_POSX);    				//坐标x
	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);    		//坐标y
	data <<(unsigned)pEntity->GetProperty<unsigned int>(PROP_ENTITY_MODELID);   //模型id
	data << (BYTE)pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);   		//朝向
	//data << (byte)pSelf->GetQuestSystem()->GetQuestStateType((CNpc*)pEntity);

	pack.flush();
}

void CObserverSystem::EntityAppearDefault(const EntityHandle &handle, CEntity* pEntity)
{
	if(!m_pEntity|| !pEntity)
	{
		return; 
	}
		
	int nActorType = m_pEntity->GetHandle().GetType();
	if (nActorType == enActor)
	{
		if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
			|| ((CActor*)m_pEntity)->OnGetIsSimulator())
		{
			return; 
		} 
	}
	CActor *pSelf = (CActor *) m_pEntity;
	INT_PTR nType = handle.GetType();
	CActorPacket pack;
	CDataPacket &data =pSelf->AllocPacket(pack);
	data <<(BYTE)enDefaultEntitySystemID <<(BYTE)sEntityAppear;
	char sAllName[128];	
	const char *pMasterName = NULL;

	data << (BYTE)nType;
	data <<(Uint64) handle;
	if(pEntity->GetAttriFlag().DenyShowName)
	{
		sprintf_s(sAllName, sizeof(sAllName), " \\");
	}
	else
	{
		sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",pEntity->GetShowName(), pMasterName ? pMasterName : pEntity->GetTitle());
	}
	data.writeString(sAllName); 
	WORD nMonsterId = 0;
	data <<(WORD)pEntity->GetProperty<int>(PROP_ENTITY_POSX);    //坐标
	data <<(WORD)pEntity->GetProperty<unsigned int>(PROP_ENTITY_POSY);    //坐标
	data <<(unsigned)pEntity->GetProperty<unsigned int>(PROP_ENTITY_MODELID);  //模型id		
	data << (BYTE)pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);   //朝向
	data <<(WORD)nMonsterId;

	//没有属性也要发""或者0，不允许没有值
	data.writeString("");
	data << (unsigned int)0;

	pack.flush();
}

//出现了一个实体,如果是玩家要发消息到客户端
void CObserverSystem::EntityAppear(const EntityHandle &handle, CEntity* pEntity)
{
	//如果是一个玩家,那么需要发送创建的消息	
	CActor *pSelf = (CActor *) m_pEntity;
	if(m_pEntity ==NULL || pSelf ==NULL)return;
	INT_PTR nType = handle.GetType();
	switch(nType)
	{
		case enActor: 
			{
				EntityAppearActor(handle,(CActor*)pEntity); 
				break;
			}
		case enDropItem:
			{
				EntityAppearDropItem(handle,pEntity);
				break;
			}
		case enFire:
			{
				EntityAppearSpecialEntity(handle,pEntity);
				break;
			}
		case enMonster:
		case enGatherMonster:
			{
				EntityAppearMonster(handle,pEntity);
				break;
			}
		case enPet:
			{
				EntityAppearPet(handle,pEntity);
				break;
			}
		case enHero:
			{
				EntityAppearSlave(handle,pEntity);
				break;
			}
		case enNpc:
			{
				EntityAppearNpc(handle,pEntity);
				break;
			}
		default:
			{
				EntityAppearDefault(handle,pEntity);
				break;
			}
			

	}
	if (pEntity->PushActorAppear())
	{
		if (pEntity && NeedPushActor(pEntity->GetType()))
			((CAnimal*)pEntity)->GetObserverSystem()->onActorAppear((CActor*)m_pEntity);
	}
}

//实体消失
void CObserverSystem::EntityDisappear(const EntityHandle &handle)
{
	CActor * pActor =(CActor *)m_pEntity;
	if(pActor ==NULL) return;

	//次元Boss归属玩家不从视野中移除
	CEntity* pEntity = GetEntityFromHandle(handle);
	if ( pEntity && pEntity->GetType() == enActor && ((CActor *)pEntity)->IsBeLongBossActor() && pEntity->GetFuBen() && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
	{
		return;
	}

	//次元Boss怪物不从视野中移除
	if ( pEntity && pEntity->GetType() == enMonster && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 386 && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
	{
		return;
	}

	CActorPacket pack;
	CDataPacket &data = ((CActor *) m_pEntity)->AllocPacket(pack);		
	//其他的实体和掉落物品不同处理
	if(handle.GetType() != enDropItem)
	{
		data << (BYTE)enDefaultEntitySystemID << (BYTE)sEntityDisappear << (Uint64)handle;
	}
	else
	{
		//拾取物品使用新的消息格式
		data <<(BYTE)enLootSystemID << (BYTE)sDropItemDisAppear << (unsigned int)handle.GetIndex();
	}
	pack.flush();

	// 玩家的视野里头出现其他非玩家实体
	if (NeedPushActor(handle.GetType()) )
	{
		CEntity* pEntity = GetEntityFromHandle(handle);
		if (pEntity )
		{
			((CAnimal*)pEntity)->GetObserverSystem()->onActorDisappear((CActor*)m_pEntity);
		}
	}
}


EntityHandle CObserverSystem::FindEntityByName(const char *pEntityName)
{
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
	{
		EntityHandle handle = m_sEntityList[i].m_handle;
		CEntity *pEntity = em->GetEntity(handle);
		if (pEntity)
		{
			const char *pIterName = pEntity->GetEntityName();
			if (strlen(pEntityName) == strlen(pIterName) && !strcmp(pEntityName, pEntity->GetEntityName()))
				return handle;
		}
	}
	return EntityHandle();
}


void CObserverSystem::ClearObserveList()
{ 
	if(m_pEntity ==NULL) return;
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	EntityHandle handle = m_pEntity->GetHandle();
	if (enActor == handle.GetType())
	{
		for (INT_PTR i = 0; i < m_sEntityList.count(); i++)
		{
			if (m_sEntityList[i].m_handle != handle)
			{
				CEntity* pEntity = em->GetEntity(m_sEntityList[i].m_handle);
				if (pEntity && NeedPushActor(pEntity->GetType()))
				{
					((CAnimal *)pEntity)->GetObserverSystem()->onActorDisappear((CActor *)m_pEntity);
				}
			}
		}
	}
	m_sEntityList.clear();
}
