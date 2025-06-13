
#include "StdAfx.h"
#include "EntityGrid.h"


//CEntityListMgr*	CEntityGrid::g_EntityListMgr;



CEntityGrid::CEntityGrid()
{
	m_nRow = 0;
	m_nCol = 0;
	//m_pGrid = NULL;
	m_pGrids = NULL;
	m_pScene = NULL;
}

CEntityGrid::~CEntityGrid()
{
	Release();
}

//释放资源
VOID CEntityGrid::Release()
{
	/*if (m_pGrid != NULL)
	{
		INT_PTR nCount = m_pScene->m_pMapData->getUnitCount();
		for(INT_PTR i = nCount-1; i > -1 ; --i)
		{
			CEntityList* plist = m_pGrid[i];
			if (plist != NULL)
			{
			ReleaseList(plist);
			}
			}
			GFreeBuffer(m_pGrid);
			}
			m_pGrid = NULL;*/

	if (m_pGrids)
	{
		for (INT_PTR i = 0; i < m_nCol; i++)
		{
			m_pGrids[i].~MapEntityGrid();
		}
		GFreeBuffer(m_pGrids);
	}
	m_pGrids = NULL;
	m_nRow = 0;
	m_nCol = 0;
	m_pScene = NULL;
}

MapEntityGrid::~MapEntityGrid()
{
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
#ifndef GRID_USE_STL_MAP
	// 这一步是有问题的，CEntityManager::DestroyEntity 调用会在 MapEntityGrid::m_entitys 处移除相应对象，这里循环是错误的。
	for (INT_PTR i = m_entitys.count()-1; i >= 0 ; i--)
	{		
		EntityHandle & hd = m_entitys[i].hd;
		CEntity* pEntity = em->GetEntity(hd);
		if (pEntity)
		{
			if (hd.GetType() != enActor && hd.GetType() != enPet && hd.GetType() != enHero)
			{
				em->DestroyEntity(hd);
			}
			else
			{
				OutputMsg(rmError,_T("destory scene error!!!!there is actor in this scene!"));
			}
		}
	}
	m_entitys.empty();
#else
	typedef typename EntityPosVector::iterator Iter;
	EntityPosVector entitys = m_entitys;
	Iter it = entitys.begin(), end = entitys.end();
	for (; it != end; it++)
	{
		const EntityHandle & hd = (*it).first;
		CEntity* pEntity = em->GetEntity(hd);
		if (pEntity)
		{
			if (hd.GetType() != enActor && hd.GetType() != enPet && hd.GetType() != enHero)
			{
				em->DestroyEntity(hd);
			}
			else
			{
				OutputMsg(rmError,_T("destory scene error!!!!there is actor in this scene!"));
			}
		}
	}
	m_entitys.clear();
#endif
}

void CEntityGrid::Reset()
{
	//if (m_pGrid != NULL)
	//{
	//	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	//	INT_PTR nCount = m_pScene->m_pMapData->getUnitCount();
	//	for(INT_PTR i = nCount-1; i > -1 ; --i)
	//	{
	//		CEntityList* plist = m_pGrid[i];
	//		if (plist != NULL)
	//		{
	//			CLinkedNode<EntityHandle> *pNode;
	//			CLinkedListIterator<EntityHandle> it(*plist);
	//			for (pNode = it.first(); pNode; pNode = it.next())
	//			{
	//				INT_PTR nType = pNode->m_Data.GetType();
	//				//怪物要清除，npc不清理，重用，角色在其他地方删,宠物会跟随玩家移出
	//				if (nType == enMonster || nType == enMovingNPC )
	//				{
	//					pEntityMgr->DestroyEntity(pNode->m_Data);
	//				}
	//			}
	//		}
	//	}
	//}
	/*
	if (m_pGrids)
	{
		CEntityManager* pEntityMgr	= GetGlobalLogicEngine()->GetEntityMgr();
		INT_PTR nGridCount			= m_pScene->m_pMapData->getUnitCount();
		for (INT_PTR j = 0; j < nGridCount; j++)
		{			
			EntityVector& entitys = m_pGrids[j].m_entitys;
			for (INT_PTR i = entitys.count()-1; i >= 0 ; i--)
			{
				INT_PTR nType = entitys[i].GetType();
				// 怪物要清除，npc不清理，重用，角色在其他地方删,宠物会跟随玩家移出
				switch(nType) //这几种实体不销毁
				{
				case enNpc:
				case enActor:
				case enHero:
				case enPet:
					break;
				default:
					pEntityMgr->DestroyEntity(entitys[i]);
					break;
				}	
			}
		}
	}
	*/

}

//释放一个list
VOID CEntityGrid::ReleaseList(CEntityList* el)
{
	if (el != NULL)
	{
		//先释放所有的实体，
		//玩家不在这里释放，另外地方处理
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		CLinkedNode<EntityHandle> *pNode;
		CLinkedListIterator<EntityHandle> it(*el);
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			if (pNode->m_Data.GetType() != enActor && pNode->m_Data.GetType() != enPet  && pNode->m_Data.GetType() != enHero)
			{
				pEntityMgr->DestroyEntity(pNode->m_Data);
			}
			else
			{
				OutputMsg(rmError,_T("destory scene error!!!!there is actor in this scene!"));
			}
		}

		//g_EntityListMgr->ReleaseDataPtr(el);
	}
}

//重新设置网格的大小
VOID CEntityGrid::Init(int nRowCount,int nColCount)
{		
	if (m_pGrids)
	{
		Release();
	}
	m_nRow = nRowCount;
	m_nCol = nColCount;
	//INT_PTR nGridCount = m_pScene->m_pMapData->getUnitCount();
	INT_PTR nSize = m_nCol * sizeof(MapEntityGrid);
	m_pGrids = (MapEntityGrid*)GAllocBuffer(nSize);
	for (INT_PTR i = 0; i < m_nCol; i++)
	{
		new (&m_pGrids[i])MapEntityGrid();
	}
}

bool CEntityGrid::CanAddEntity(CEntity * pEntity,INT_PTR& nX,INT_PTR& nY,bool bShowErr, bool bCanAlwaysEnter)
{
	//判断实体的坐标是否超出地图范围
	//and 判断这里是否是障碍位置
	if ( !pEntity )
	{
		return false;
	}
	INT_PTR nEntityType = pEntity->GetType();

	if (nX < 0 || nX >= m_nCol || nY < 0 || nY >= m_nRow)
	{
		//如果不对可以调整
		//OutputMsg(rmWaning,_T(" Moveto Pos is outRange! nx=%d,ny=%d,m_col=%d,m_row=%d,id=%d,name=%s, begin adjusting"),
		//	nX,nY,m_nCol,m_nRow,pEntity->GetProperty<int>(PROP_ENTITY_ID),pEntity->GetEntityName());

		m_pScene->GetSceneAreaCenter(nX,nY);

	}
	else
	{
		bool bSetToDefPt = false;
		if (!m_pScene->CanMove(nX,nY))
			bSetToDefPt = true;

		if (bSetToDefPt)
		{
			if (!bShowErr)
				return false;
			OutputMsg(rmWaning,_T("CEntityGrid::Moveto() Error!!Pos is can not move!nx=%d,ny=%d,id=%u,name=%s,mapid=%d")
				,nX,nY,pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),pEntity->GetEntityName(),m_pScene->GetSceneId());
			m_pScene->GetSceneAreaCenter(nX,nY);
			OutputMsg(rmError,_T("set new position!nx=%d,ny=%d"),nX,nY);
		}
	}

	//如果校验后还不行
	if (nX < 0 || nX >= m_nCol || nY < 0 || nY >= m_nRow || !m_pScene->CanMove(nX,nY))
	{
		if (bShowErr)
		{
			OutputMsg(rmError,_T("CEntityGrid::Moveto() Error!!Pos is outRange! nx=%d,ny=%d,m_col=%d,m_row=%d,id=%u,name=%s"),
				nX,nY,m_nCol,m_nRow,pEntity->GetProperty<int>(PROP_ENTITY_ID),pEntity->GetEntityName());
		}
		return false;
	}
	if (!CanCross(nEntityType,nX,nY,bCanAlwaysEnter))
	{
		//OutputMsg(rmError,_T("CEntityGrid::Moveto() Error!!Pos can not cross! nx=%d,ny=%d,m_col=%d,m_row=%d,id=%d,name=%s"),
		//	nX,nY,m_nCol,m_nRow,pEntity->GetProperty<int>(PROP_ENTITY_ID),pEntity->GetEntityName());
		return false;
	}
	return true;
}


// 场景中删除一个实体（实体死亡或退出游戏）
void CEntityGrid::DeleteEntity(CEntity * pEntity)
{
	if(pEntity == NULL) return;

	int nX , nY;	//实体的坐标
	//获取实体的坐标
	pEntity->GetPosition(nX,nY);

	MapEntityGrid* pGrid = GetList(nX);
	if (pGrid)
	{
		EntityHandle hEntity = pEntity->GetHandle();
		pGrid->removeEntity(hEntity);
	}

	pEntity->SetScene(NULL);
	pEntity->SetFuBen(NULL);
	//pEntity->SetPosition(-1,-1);//设置成一个不存在的位置
}


//移动一个实体
int CEntityGrid::Move(CEntity * pEntity,INT_PTR nDir,INT_PTR nSpeed)
{
	if (pEntity == NULL) return seNoEntity;
	int nOldX,nOldY;
	pEntity->GetPosition(nOldX,nOldY);

	INT_PTR nX = nOldX, nY = nOldY;	//实体的坐标
	//OutputMsg(rmTip,"can move begin x=%d,y=%d",nX,nY);

	bool result = CanMoveTo(pEntity, nX,nY,nDir,(BYTE)nSpeed);
	//OutputMsg(rmTip,"can move end x=%d,y=%d,nDir=%d,nSpeed=%d",nX,nY,nDir,nSpeed);
	if (!result)
	{
		return seCanNotMove;
	}

	bool bCanAlwaysEnter =false;
	if(pEntity->GetType() == enMonster)			//如果是可以穿人穿怪的
	{
		if(pEntity->GetAttriFlag().CanAlwaysEnter == true)
		{
			bCanAlwaysEnter = true;
		}
	}
	return MoveTo(pEntity,nX,nY,false,bCanAlwaysEnter);
}

int CEntityGrid::MoveTo(CEntity * pEntity,INT_PTR nNewX,INT_PTR nNewY,bool boNew, bool bCanAlwaysEnter)
{
	if (pEntity == NULL) return seNoEntity;
	if (pEntity->GetType() == enNpc && pEntity->IsInited())
	{
		OutputMsg(rmError,"CEntityGrid::MoveTo npc x=%d,y=%d,handle=%lld",nNewX,nNewY,Uint64(pEntity->GetHandle()));
		//return seCanNotMove;
	}
	MapEntityGrid* pGrid=NULL;
	MapEntityGrid* pOldGrid =NULL;
	//OutputMsg(rmTip,"CEntityGrid::MoveTo x=%d,y=%d",nNewX,nNewY);
	EntityHandle hEntity = pEntity->GetHandle();

	if (!CanAddEntity(pEntity,nNewX,nNewY,true, bCanAlwaysEnter))
	{
		return seOutRange;
	}

	int nOldX = 0 ,nOldY = 0;
	if (boNew)
	{

		pGrid = GetList(nNewX);
		if (!pGrid)
		{
			OutputMsg(rmError, _T("%s get grid null[%d, %d]"), __FUNCTION__, nNewX, nNewY);
			return seOutRange;
		}
		pGrid->addEntity(hEntity, (int)nNewY);
	}
	else
	{
		pEntity->GetPosition(nOldX, nOldY);
		pOldGrid = GetList(nOldX);
		pGrid = GetList(nNewX);
		if (!pGrid)
		{
			OutputMsg(rmError, _T("%s get grid null[%d, %d] when remove"), __FUNCTION__, nNewX, nNewY);
			return seOutRange;
		}

		if(pOldGrid == pGrid) //在同1列
		{
			pGrid->setEntityPos(hEntity,(int)nNewY);
		}
		else
		{
			if (pOldGrid)
			{
				pOldGrid->removeEntity(hEntity);
			}
			pGrid->addEntity(hEntity,(int)nNewY);
		}
	}
	pEntity->SetPosition((int)nNewX,(int)nNewY);
	pEntity->SetScene(m_pScene);
	pEntity->SetSceneID(m_pScene->GetSceneId());
	pEntity->SetFuBen(m_pScene->GetFuBen());
	if (pEntity->GetType() == enActor )
	{
		CActor* pActor = (CActor*)pEntity; 
		SCENEAREA* pArea = m_pScene->GetAreaAttri(nOldX,nOldY);	
		if (boNew || !m_pScene->IsSameArea(nOldX,nOldY,nNewX,nNewY))
		{
			if (!boNew)
			{
				//删除原来区域给的buff
				
				pActor->RemoveAreaBuff(pArea);

				if (m_pScene->HasMapAttribute(nOldX,nOldY,aaForcePkMode,pArea))//恢复上一次的pk模式
				{
					CPkSystem& pk = pActor->GetPkSystem();
					pk.SetPkMode(pk.m_bLastMode,true);
				}

				// 退出的时候需要移除原来的区域设置的 ‘不可见玩家名称区域’属性
				if (m_pScene->HasMapAttribute(nOldX,nOldY,aaCannotSeeName, pArea))
				{
					//pActor->CollectOperate(CEntityOPCollector::coRefFeature);
					pActor->SetSocialMask(smHideHeadMsg, false);
					CHero * pHero = pActor->GetHeroSystem().GetBattleHeroPtr();
					if(pHero)
					{
						pHero->SetSocialMask(smHideHeadMsg, false);
						//pHero->CollectOperate(CEntityOPCollector::coRefFeature);
					}
				}
				pActor->SetSceneAreaFlag(0);
			}
			CrossArea(pActor);
		}
		//如果这是个传送点，要传送到新的地方
		//执行上面这个函数后，可能玩家已经不在这个地方了，因为有可能传送到其他地方了

		SCENEAREA* pNewArea = m_pScene->GetAreaAttri(nNewX,nNewY);	
		if (m_pScene->HasMapAttribute(nNewX,nNewY,aaForcePkMode,fpUnion,pNewArea))
		{
			if (!m_pScene->HasMapAttribute(nOldX,nOldY,aaForcePkMode,fpUnion,pArea) || boNew)
			{
				//刚登陆在行会模式地图上需要再调用BroadCastGuildPlayerPos，因为guildsystem没初始化好
				//在CGuildSystem::OnLogin实现
				pActor->GetGuildSystem()->BroadCastGuildPlayerPos(gptEnter);
			}
			else
			{
				pActor->GetGuildSystem()->BroadCastGuildPlayerPos(gptMove);
			}
		}
		else if (m_pScene->HasMapAttribute(nOldX,nOldY,aaForcePkMode,fpUnion,pArea))
		{
			pActor->GetGuildSystem()->BroadCastGuildPlayerPos(gptDisappear);
			
		}
		//如果是玩家在队伍里，要广播消息
		pActor->GetTeamSystem().TeamMoveBroadcast();
		/*if(pActor->HasState(esStateHide))
		{
			pActor->RemoveState(esStateHide);
			if (pActor->GetBuffSystem()->Exists(aHide))
			{
				pActor->GetBuffSystem()->Remove(aHide,-1);
			}
		}*/

	}

	int nowX,nowY;
	pEntity->GetPosition(nowX,nowY);
	if ((!boNew) && pEntity->GetScene() == m_pScene && nowX == nNewX && nowY == nNewY &&  pEntity->GetAttriFlag().CanSceneTelport)
	{
		TelePort(pEntity);
	}

	//碰到了火，需要掉血
	if(pGrid)
	{
		if(pEntity->IsInited() && pEntity->isAnimal() ) //是一个动物并且已经初始化了
		{
			EntityPosVector  & entitys=	pGrid->m_entitys;		// 格子中的实体列表 

#ifndef GRID_USE_STL_MAP
			for(INT_PTR i=0; i<entitys.count(); i++ )
			{
				if(entitys[i].pos != nNewY ) continue;
				EntityHandle &hd = entitys[i].hd;		
#else
			typedef typename EntityPosVector::iterator Iter;
			Iter it = entitys.begin(), end = entitys.end();
			for (; it != end; it++)
			{
				if((*it).second != nNewY) continue;
				EntityHandle hd = (*it).first;
#endif
				INT_PTR nType  = hd.GetType();
				if (nType == enFire)
				{
					CFire *pFire = (CFire *)GetGlobalLogicEngine()->GetEntityMgr()->GetEntity( hd);
					if(pFire)
					{
						pFire->OnCollideFire((CAnimal*)pEntity);
					}
				}

			}
		}

	}

	return seSucc;
}


bool CEntityGrid::CanMoveTo(CEntity* pEntity, INT_PTR& nX,INT_PTR& nY,INT_PTR nDir,BYTE nSpeed )
{
	if (pEntity->GetType() == enNpc && pEntity->IsInited())
	{
		return false;
	}
	//计算下一个位置，同时要检查经过的每个点是否能经过,性能不高
	switch(nDir)
	{
	case DIR_UP:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nY--;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_UP_LEFT:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX--;nY--;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_UP_RIGHT:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX++;nY--;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_DOWN:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nY++;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_DOWN_LEFT:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX--;nY++;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_DOWN_RIGHT:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX++;nY++;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_RIGHT:		
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX++;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	case DIR_LEFT:
		for (INT_PTR i = 0; i < nSpeed; i++)
		{
			nX--;
			if (!CanMoveThisGrid(pEntity, nX,nY)) return false;
		}
		break;
	default:
		break;
	}
	return true;
}


VOID CEntityGrid::NewPosition(INT_PTR& nX,INT_PTR& nY,INT_PTR nDir,INT_PTR nSpeed)
{
	//计算下一个位置
	switch(nDir)
	{
	case DIR_UP:
		nY = nY - nSpeed;
		break;
	case DIR_UP_LEFT:
		nY = nY - nSpeed;
		nX = nX - nSpeed;
		break;
	case DIR_UP_RIGHT:
		nY = nY - nSpeed;
		nX = nX + nSpeed;
		break;
	case DIR_DOWN:
		nY = nY + nSpeed;
		break;
	case DIR_DOWN_LEFT:
		nY = nY + nSpeed;
		nX = nX - nSpeed;
		break;
	case DIR_DOWN_RIGHT:
		nY = nY + nSpeed;
		nX = nX + nSpeed;
		break;
	case DIR_RIGHT:
		nX = nX + nSpeed;
		break;
	case DIR_LEFT:
		nX = nX - nSpeed;
		break;
	default:
		break;
	}
}

MapEntityGrid* CEntityGrid::GetList(INT_PTR x)
{
	if(m_pScene ==NULL ||  m_pScene->m_pMapData ==NULL ) return NULL;
	//INT_PTR nPosIdx = m_pScene->m_pMapData->getPosIndex((DWORD)x, (DWORD)y);
	//if (nPosIdx >= 0)
	if(x >=0 && x< m_nCol)
	{
		return &m_pGrids[x];
	}
	return NULL;
}

//VOID CEntityGrid::SetList(INT_PTR x,INT_PTR y,CEntityList* pList)
//{
//	INT_PTR nPosIdx = m_pScene->m_pMapData->getPosIndex((DWORD)x, (DWORD)y);
//	//int index = m_pScene->m_pMapData->getMoveableIndex((DWORD)x,(DWORD)y);
//	//Assert(nPosIdx >= 0 && m_pGrid[nPosIdx] == NULL);	
//	m_pGrid[nPosIdx] = pList;
//}

void CEntityGrid::SendAreaConfig(SCENEAREA* pArea,CActor *pActor)
{
	CActorPacket pack;
	CDataPacket & data = pActor->AllocPacket(pack);
	pack << (BYTE)enDefaultEntitySystemID << (BYTE)sSceneAreaAttri;
	data.writeString(pArea->NoTips > 0 ? "":pArea->szName);
	pack << (BYTE)(sizeof(pArea->attrBits)/sizeof(pArea->attrBits[0]));
	data.writeBuf(pArea->attrBits,sizeof(pArea->attrBits));
	pack.flush();
	pActor->SetProperty<Uint64>(PROP_ACTOR_AREA_ATTR,*(Uint64*)(pArea->attrBits));
}

void CEntityGrid::CrossArea( CActor* pActor )
{
	int x,y;
	pActor->GetPosition(x,y);
	SCENEAREA* pArea = m_pScene->GetAreaAttri(x,y);	//直接返回区域属性，后面的函数判断直接用这个,避免多次获取
	if(!pArea) return;

	// 加BUFF的区域
	if (m_pScene->HasMapAttribute(x,y,aaAddBuff,pArea))
	{
		//自动添加buff
		AreaAttri& attri = pArea->attri[aaAddBuff];
		if (attri.pValues && attri.nCount > 0)
		{
			CBuffSystem* bs = pActor->GetBuffSystem();
			for (INT_PTR i = 0; i < attri.nCount; i++)//增加N个buff
			{
				int nBuffId = attri.pValues[i];
				bs->Append(nBuffId);
			}
		}		
	}

	//减buff区域
	if (m_pScene->HasMapAttribute(x,y,aaLeaveDelBuf,pArea))
	{
		//自动删除buff
		AreaAttri& attri = pArea->attri[aaLeaveDelBuf];
		if (attri.pValues && attri.nCount > 0)
		{
			CBuffSystem* bs = pActor->GetBuffSystem();
			for (INT_PTR i = 0; i < attri.nCount; i++)//删除N个buff
			{
				int nBuffId = attri.pValues[i];
				CBuffProvider::BUFFCONFIG * pConfig = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
				if (pConfig != NULL)
				{
					bs->Remove((GAMEATTRTYPE)pConfig->nType, pConfig->nGroup); 
				}

			}
		}		
	}

	// 安全区 加技能buff
	if (m_pScene->HasMapAttribute(x,y,aaSaft,pArea))
	{
		//自动添加buff
		AreaAttri& attri = pArea->attri[aaSaft];
		for (INT_PTR i = 0; i < attri.nCount; i++)
		{
			int nSkillID = attri.pValues[i];
			const OneSkillData * pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
			const CSkillSubSystem::SKILLDATA * pSkillData = pActor->GetSkillSystem().GetSkillInfoPtr(nSkillID);
			if (pSkillConf && pSkillData)
			{
				const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
				if (pSLevelConf)
				{
					for(INT_PTR rangeID=0; rangeID < pSLevelConf->pranges.count; rangeID++)
					{
						PSKILLONERANGE pRangeConf = pSLevelConf->pranges[rangeID];
						INT_PTR nResultCount = pRangeConf->skillResults.count;
						for(INT_PTR resultID = 0;  resultID < nResultCount; resultID ++)
						{
							PSKILLRESULT pSkillResult = &(pRangeConf->skillResults[resultID]);
							if (pSkillResult->nResultType == srSkillResultAddBuff)
							{
								if (pSkillResult)
								{
									pActor->GetBuffSystem()->Append(pSkillResult->nId);
								}
							}
						}
					}
				}//end pSLevelConf
			}//end (pSkillConf && pSkillData)
		}//end for
	}//end aaSaft

	//判断当前是否复活点
	if (m_pScene->HasMapAttribute(x,y,aaSaftRelive,pArea))
	{
		//是复活点
		int nPosX = pArea->Center[0];
		int nPosY = pArea->Center[1];
		int nSceneId = m_pScene->GetSceneId();
		pActor->SetRelivePoint(nPosX,nPosY,nSceneId,m_pScene->GetFuBen()->IsFb());
	}

	if (m_pScene->HasMapAttribute(x,y,aaNotCallPet,pArea))
	{
		pActor->GetPetSystem().RemoveAllPets();
	}

	//改变模型区域
	//改变武器区域
	if (m_pScene->HasMapAttribute(x, y, aaChangeModel, pArea)||
		m_pScene->HasMapAttribute(x, y, aaChangeWeapon, pArea))
	{
		pActor->CollectOperate(CEntityOPCollector::coRefFeature);
		//pActor->GetEquipmentSystem().RefreshAppear(); //刷外观
	}

	//禁用技能区域
	if (m_pScene->HasMapAttribute(x, y, aaNotSkillId, pArea))
	{
		AreaAttri * pAreaAttr = &pArea->attri[aaNotSkillId];
		for (size_t i = 0; i < pAreaAttr->nCount; i++)
		{
			int nSkillId = pAreaAttr->pValues[i];
			pActor->GetSkillSystem().SetSkillClose(nSkillId, true);
			CSkillSubSystem::SKILLDATA * pSkillData = pActor->GetSkillSystem().GetSkillInfoPtr(nSkillId);
			pActor->GetSkillSystem().NoticeOneSkillData(pSkillData);
		}
	}
	
	//安全区域
	if (m_pScene->HasMapAttribute(x,y,aaCity,pArea) )
	{
		//回城点
		int nPosX = pArea->Center[0];
		int nPosY = pArea->Center[1];
		pActor->SetCityPoint(nPosX,nPosY,m_pScene->GetSceneId());
	}

	// 触发进入活动区域事件
	if (m_pScene->HasMapAttribute(x,y,aaActivity,pArea))
	{
    	static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        CScriptValueList paramList;
		AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
		for (size_t i = 0; i < pAreaAttr->nCount; i++)
		{
			int nAtvId = pAreaAttr->pValues[i];
			// 个人活动
			if(pActor->GetActivitySystem().IsActivityRunning(nAtvId))
			{
				if (PersonActivtyData* pActivty = pActor->GetActivitySystem().GetActivity(nAtvId))
				{
					paramList.clear();
					paramList << (int)CActivitySystem::enOnEnterArea;
					paramList << (int)pActivty->nActivityType;
					paramList << (int)pActivty->nId;
					paramList << pActor;
					if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
					{
						OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
					}
					pActor->CrossActivityArea();
				}
			}
			// 全局活动
			else if(GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
			{
				if (GlobalActivityData* pActivty = GetGlobalLogicEngine()->GetActivityMgr().GetActivity(nAtvId))
				{
					paramList.clear();
                    paramList << (int)CActivitySystem::enOnEnterArea;
                    paramList << (int)pActivty->nActivityType;
                    paramList << (int)pActivty->nId;
					paramList << pActor;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
                    }
					pActor->CrossActivityArea();
				}
			}
		}
	}
	
	//如果不是重配地图，把这个坐标点保存下来
	if (!m_pScene->HasMapAttribute(x,y,aaReloadMap,pArea))
	{
		pActor->SetNotReloadMapPos(m_pScene->GetSceneId(),x,y);
	}

	if (m_pScene->HasMapAttribute(x,y,aaForcePkMode,pArea))	//强制转换pk模式
	{
		CPkSystem& pk = pActor->GetPkSystem();
		AreaAttri& attri = pArea->attri[aaForcePkMode];
		if (attri.nCount > 0)
		{
			pk.SetPkMode(attri.pValues[0],true);
		}
	}
	
	if (m_pScene->HasMapAttribute(x,y,asSceneAreaMode,pArea))//进入某个区域给玩家设置标记
	{
		AreaAttri& attri = pArea->attri[asSceneAreaMode];
		if (attri.nCount > 0)
		{
			pActor->SetSceneAreaFlag(attri.pValues[0]);
		}
	}
	else
	{
		pActor->SetSceneAreaFlag(0);
	}

	if (m_pScene->HasMapAttribute(x, y, aaCannotSeeName, pArea))
	{
		//pActor->CollectOperate(CEntityOPCollector::coRefFeature);
		pActor->SetSocialMask(smHideHeadMsg, true);
		CHero * pHero = pActor->GetHeroSystem().GetBattleHeroPtr();
		if(pHero)
		{
			//pHero->CollectOperate(CEntityOPCollector::coRefFeature);
			pHero->SetSocialMask(smHideHeadMsg, true);
		}
	}
	else
	{
		pActor->SetSocialMask(smHideHeadMsg, false);
		CHero * pHero = pActor->GetHeroSystem().GetBattleHeroPtr();
		if(pHero)
		{
			pHero->SetSocialMask(smHideHeadMsg, false);
			//pHero->CollectOperate(CEntityOPCollector::coRefFeature);
		}
	}
	
	//发送区域属性到客户端
	if(pActor->IsInited())
	{
		SendAreaConfig(pArea, pActor);
	}
}

bool CEntityGrid::CanPassPort( CEntity * pEntity,int nPassId)
{
	if(pEntity == NULL) return false;

	if(nPassId <= 0) return true;

	//只有战力沙巴克的行会才能通过
	if(nPassId == tpGuildSiege)
	{
		if (pEntity->GetType() == enActor)
		{
			return GetGlobalLogicEngine()->GetGuildMgr().CanPassPalaceTeleport((CActor*)pEntity);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CEntityGrid::TelePort(  CEntity * pEntity )
{
	if(!pEntity || !m_pScene || !(m_pScene->m_pTelePortTable)) return;
	int nX,  nY ;
	pEntity->GetPosition(nX,nY);
	INT_PTR nOwnerSceneId = pEntity->GetSceneID();
	INT_PTR nIndex = m_pScene->m_pTelePortTable[nY*m_pScene->m_nWidth + nX];
	if (nIndex != 0 && m_pScene->m_pSceneData && nIndex <= m_pScene->m_pSceneData->TeleportList.nCount)
	{
		TeleportConfig& config = m_pScene->m_pSceneData->TeleportList.pList[nIndex-1];
		INT_PTR sid = config.nSceneId,px = config.nToPosX, py = config.nToPosY;

		if(config.bUse == false) return;

		if( CanPassPort(pEntity,config.nPassId) == false) return;

		//计算要传送的位置
		CFuBen* pFuBen = m_pScene?m_pScene->GetFuBen():NULL;
		if (!pFuBen) return;

		if(GetLogicServer()->GetDaysSinceOpenServer()<config.nOpendayLimit) {
			//
			if (pEntity->GetType() == enActor)	
			{
				
				((CActor*)pEntity)->SendTipmsgFormatWithId(tmUnreachOpenServer,tstUI);
			}
			return ;
		}

		if (pFuBen->IsFb())
		{
			if (sid < 0 || !pFuBen->HasScene((int)sid))
			{
				CFuBen* pTemp = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
				if (pTemp)	//静态场景
				{
					if (sid >= 0 && pTemp->HasScene((int)sid))
					{
						pFuBen = pTemp;
					}else
					{
						//传到进入副本前的位置
						if (pEntity->GetType() == enActor)	//只有玩家才可以传出副本
						{
							pFuBen = pTemp;
							((CActor*)pEntity)->GetEnterFbPos(sid,px,py);
						}
					}
				}
			}
		}
		if (config.nToPosRadius)//目标点范围随机
		{
			int nRandX, nRandY; 
			CScene* pToScene = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0)->GetScene(sid);
			if (pToScene)
			{
				if (pToScene->GetRandomPoint(pEntity,(int)px - config.nToPosRadius,(int)py - config.nToPosRadius,(int)px + config.nToPosRadius, (int)py + config.nToPosRadius, nRandX, nRandY))
				{
					px = nRandX;
					py = nRandY;
				}
			}
		}
		
		bool bRet = pEntity->Teleport(pFuBen,sid,px,py); 
				
		if (bRet)
		{
			CFuBen *pFB = pEntity->GetFuBen();
			pEntity->TeleportOwnedEntity(pFB, sid, px, py, nOwnerSceneId, nX, nY, etsTeleport);
		}
	}
}

bool CEntityGrid::CanCross(INT_PTR nEntityType, INT_PTR x, INT_PTR y, bool bCanAlwaysEnter, bool bCanAroudFire)
{
	
	MapEntityGrid* pGrid = GetList(x);
	if (!pGrid)
		return false;
	//不能移动的点不行
	if(!m_pScene->CanMove(x,y))
	{
		return false;
	}
	EntityPosVector& entitys = pGrid->m_entitys;

#ifndef GRID_USE_STL_MAP
	if (entitys.count() <= 0)
		return true;
#else
	if (entitys.empty())
		return true;
#endif
	CEntity * pEntity;

	//不管什么情况，都能进入场景，比如火，npc，火墙，掉落物品等

	bool canAlwaysEnter = true; 
	
	switch (nEntityType)
	{
	case enActor:
	case enMonster:
	case enPet:
	case enGatherMonster:
	case enHero:
		canAlwaysEnter =false;
		break;
	}
	if (bCanAlwaysEnter)
		canAlwaysEnter = true;

	if(canAlwaysEnter) return true;


	/*
	1.	NPC在任何时候都不能被穿。
	2.	怪物在任何时候都不能穿人穿怪。
	3.	宠物在任何时候都不能穿人穿怪。
	4.	穿人
		a.	在可以穿人的地图，玩家移动时候可以穿过其他玩家，可以与其他玩家重合。
		b.	在不能穿人的地图，玩家移动时候不能穿过其他玩家，不能与其他玩家重合。已经存在玩家的坐标点视为不可移动点。
	5.	穿怪
		a.	在可以穿怪的地图，玩家移动时候可以穿过怪物，可以与怪物重合。
		c.	在不能穿怪的地图，玩家移动时候不能穿过穿怪，不能与怪物重合。已经存在怪物的坐标点视为不可移动点。
	*/
	
	bool canCrossMan = m_pScene->HasMapAttribute(x,y,aaCrossMan)?true:false;//能穿人
	bool canCrossMonster = m_pScene->HasMapAttribute(x,y,aaCrossMonster) ?true:false;//能穿怪

	bool isMan = (nEntityType == enActor); //是否是一个人，穿人和穿怪只是针对人来处理的
	
	if(isMan) //人的处理
	{
#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = 0; i < entitys.count(); i++)
		{
			bool isTargetMan = false;
			if(entitys[i].pos !=  y) continue;
			EntityHandle hd = entitys[i].hd;
#else
		typedef typename EntityPosVector::iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			bool isTargetMan = false;
			if((*it).second != y) continue;
			EntityHandle hd = (*it).first;
#endif
			switch(hd.GetType())
			{
		
			case enActor:
				pEntity = GetEntityFromHandle(hd) ; //死人是能传的
				if(pEntity)
				{
					if(pEntity && !pEntity->IsDeath() && !canCrossMan )
					{
						return false;
					}
					isTargetMan = true;
				}
				break;
			case enMonster: //部分怪不能穿
				{
					pEntity = GetEntityFromHandle(hd) ; //
					CMonster* pMonster = (CMonster*) pEntity;
					if( pMonster && (!pMonster->IsDeath()) && pMonster->GetAttriFlag().noBeCrossed )
					{
						return false;
					}
				}
			case enPet:
			case enHero:
			case enGatherMonster:
				pEntity = GetEntityFromHandle(hd) ; //死人是能传的
				if(pEntity)
				{
					if(pEntity && !pEntity->IsDeath() && (!canCrossMonster && !isTargetMan) )
					{
						return false;
					}
				}
				break;
			case enNpc:
				return false;
			}
		}
	}
	else
	{

#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = 0; i < entitys.count(); i++)
		{
			if(entitys[i].pos !=  y) continue;
			EntityHandle  _hd = entitys[i].hd;
#else
		EntityPosVector& entitys = pGrid->m_entitys;
		typedef typename EntityPosVector::iterator Iter;
		Iter it = entitys.begin(), end = entitys.end();
		for (; it != end; it++)
		{
			if((*it).second != y) continue;
			EntityHandle _hd = (*it).first;
#endif
			switch(_hd.GetType())
			{
			case enNpc:
				return false;
			case enActor:
			case enMonster:
			case enPet:
			case enGatherMonster:
			case enHero:
				pEntity = GetEntityFromHandle(_hd) ; //死人是能传的
				if(pEntity && !pEntity->IsDeath() )
				{
					return false;
				}
				break;
			case enFire:
				{
					if (bCanAroudFire)
					{
						return false;
					}
					break;
				}
			}
		}

	}
	return true;
}

bool CEntityGrid::CanMoveThisGrid(CEntity* pEntity, INT_PTR x ,INT_PTR y )
{	
	INT_PTR nEntityType = pEntity->GetType() ;
	bool isActor = nEntityType== enActor ? true : false;
	
	if (x < 0 || x >= m_nCol || y < 0 || y >= m_nRow)
		return false;

	bool bCanAlwaysEnter =false;
	if(nEntityType == enMonster)			//如果是可以穿人穿怪的
	{
		if(pEntity->GetAttriFlag().CanAlwaysEnter == true)
		{
			bCanAlwaysEnter = true;
		}
	}

	if ( !m_pScene->CanMove(x,y) || !CanCross(nEntityType,x,y,bCanAlwaysEnter) )
		return false;

	return true;
}

bool CEntityGrid::GetMoveablePoint(CEntity *pEntity, CScene * pScene,INT_PTR x,INT_PTR y,INT_PTR &nResultX,INT_PTR &nResultY,bool bCanAlwaysEnter,INT_PTR nType)
{
	if(pEntity ==NULL && pScene ==NULL ) return false;
	//一共48个，从里边到外边遍历，找到一个合适的位置
	static int s_matrix[][2] =
	{
		//第1圈8个
		{0,0},
		{-1,-1},
		{0,-1},
		{1,-1},
		{-1,0},
		{1,0},
		{-1,1},
		{0,1},
		{1,1},

		//第2圈16个
		{-2,-2},
		{-1,-2},
		{0,-2},
		{1,-2},
		{2,-2},
		{-2,-1},
		{2,-1},
		{-2,0},
		{2,0},
		{-2,1},
		{2,1},
		{-2,2},
		{-1,2},
		{0,2},
		{1,2},
		{2,2},


		//第3圈24个
		{-3,-3},
		{-2,-3},
		{-1,-3},
		{0,-3},
		{1,-3},
		{2,-3},
		{3,-3},
		{-3,-2},
		{3,-2},
		{-3,-1},
		{3,-1},
		{-3,0},
		{3,0},
		{-3,1},
		{3,1},
		{-3,2},
		{3,2},
		{-3,3},
		{-2,3},
		{-1,3},
		{0,3},
		{1,3},
		{2,3},
		{3,3},


		//第4
		{-4,-4},
		{-3,-4},
		{-2,-4},
		{-1,-4},
		{0,-4},
		{1,-4},
		{2,-4},
		{3,-4},
		{4,-4},
		{-4,-3},
		{4,-3},
		{-4,-2},
		{4,-2},
		{-4,-1},
		{4,1},
		{-4,0},
		{4,0},
		{-4,1},
		{4,1},
		{-4,2},
		{4,2},
		{-4,3},
		{4,3},
		{-4,4},
		{-3,4},
		{-2,4},
		{-1,4},
		{0,4},
		{1,4},
		{2,4},
		{3,4},
		{4,4},

		//第5圈
		{-5,-5},
		{-4,-5},
		{-3,-5},
		{-2,-5},
		{-1,-5},
		{0,-5},
		{1,-5},
		{2,-5},
		{3,-5},
		{4,-5},

		{5,-5},
		{-5,-4},
		{5,-4},
		{-5,-3},
		{5,-3},
		{-5,-2},
		{5,-2},
		{-5,-1},
		{5,1},
		{-5,0},
		{5,0},
		{-5,1},
		{5,1},
		{-5,2},
		{5,2},
		{-5,3},
		{5,3},
		{-5,4},
		{5,4},
		{-5,5},
		{-4,5},
		{-3,5},
		{-2,5},
		{-1,5},
		{0,5},
		{1,5},
		{2,5},
		{3,5},
		{4,5},
		{5,5},

		//第6圈
		{-6,-6},
		{-5,-6},
		{-4,-6},
		{-3,-6},
		{-2,-6},
		{-1,-6},
		{0,-6},
		{1,-6},
		{2,-6},
		{3,-6},
		{4,-6},
		{5,-6},
		{6,-6},
		{-6,-5},
		{6,-5},
		{-6,-4},
		{6,-4},
		{-6,-3},
		{6,-3},
		{-6,-2},
		{6,-2},
		{-6,-1},
		{6,1},
		{-6,0},
		{6,0},
		{-6,1},
		{6,1},
		{-6,2},
		{6,2},
		{-6,3},
		{6,3},
		{-6,4},
		{6,4},
		{-6,5},
		{6,5},
		{-6,6},
		{-5,6},
		{-4,6},
		{-3,6},
		{-2,6},
		{-1,6},
		{0,6},
		{1,6},
		{2,6},
		{3,6},
		{4,6},
		{5,6},
		{6,6},
	};

	//在周围找一个可以刷的地方
	INT_PTR   nDX, nDY;
	if(pScene ==NULL && pEntity)
	{
		pScene = pEntity->GetScene();
	}
	if (!pScene) return false;
	
	INT_PTR nEntityType = nType;
	if(pEntity)
	{
		nEntityType = pEntity->GetType() ;
	}

	bool canAlwaysEnter = true; //不管什么情况，都能进入场景，比如火，npc，火墙，掉落物品等
	switch (nEntityType)
	{
	case enActor:
	case enMonster:
	case enPet:
	case enGatherMonster:
	case enHero:
		canAlwaysEnter =false;
		break;
	}

	if (bCanAlwaysEnter)
	{
		canAlwaysEnter = true;
	}

	CEntityGrid &grid = pScene->GetGrid();

	//这里先轮一次，如果旁边48个格子都有了，那么进行第2次遍历，不过这种情况是比较少的
	for(INT_PTR j=0; j< ArrayCount(s_matrix); j++)
	{
		nDX = x + s_matrix[j][0];
		nDY = y + s_matrix[j][1];
		//如果这个点能够走，npc,火这些直接不用判断
		
		if( pScene->CanMove(nDX,nDY) && (  canAlwaysEnter || grid.CanCross(nEntityType,nDX,nDY) ) )
		{
			nResultX = nDX;
			nResultY = nDY;
			return true;
		}
	}
	
	return false;
}
