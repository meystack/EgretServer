

#include "StdAfx.h"
#include "Entity.h"
#include "../LogicEngine.h"

bool CEntity::Init(void * data, size_t size)
{
	return true;
}

void CEntity::Destroy()
{
	m_DynamicVar.clear();
	ClearAllEntityMsg();
	ClearScriptCallback();
}
void  CEntity::OnPropertyChange(int nPropID, UINT64 oldValue, UINT64 value)
{
	if(nPropID == PROP_CREATURE_HP)
	{
		if(value ==0)
		{
			if(isAnimal())
			{
				((CAnimal*)this)->AddState(esStateDeath);
			}
			PostEntityDeadMsg();
			//OnEntityDeath();				
		}
		else if(oldValue ==0)
		{
			if(isAnimal())
			{
				if(((CAnimal*)this)->HasState(esStateDeath))
				{
					((CAnimal*)this)->OnEntityRelive();	
					((CAnimal*)this)->RemoveState(esStateDeath);
				}
				if (this->GetType() == enActor)
				{
					((CActor*)this)->BroadRelive();
				}
			}
		}
	}
	//等级、头像、签名改变要调用，签名目前没有
	if ( GetType() == enActor  )
	{
		if (nPropID == PROP_ACTOR_PK_VALUE)
		{
			UpdateNameClr_OnPkValChanged(GetHandle(), (unsigned int)oldValue, (unsigned int)value);
		}
		else if(nPropID == PROP_CREATURE_MOVEONESLOTTIME ||  nPropID == PROP_CREATURE_ATTACK_SPEED) //玩家的攻击速度和移动速度有增加
		{
			((CActor*)this)->ResetSpeedTime();
		}
	}
}
void CEntity::ClearAllEntityMsg()
{
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;
	CEntityMsg *pMsg = 0;

	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (!pMsg->bWaitDel && pMsg->nForceCallWhenDestroy != 0)				// 实体销毁前强制执行部分消息	
			ProcessEntityMsg(*pMsg);
		pLogicEngine->DestoryEntityMsg(*pNode,this);
	}
	m_EntityMsgList.clear();
}

void CEntity::_PostEntityMsg(const CEntityMsg &msg,LPCSTR file, INT_PTR line)
{
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CEntityMsg *pMsg = pLogicEngine->AllocEntityMsg(file,line);
	*pMsg = msg;
	pMsg->SetAllocFileLine(file,line);
	pMsg->SetOwnerPtr(this); //获取消息的所属

	pMsg->bIsUsed =true;
	//调整消息时间，外部传递的是基于当前的时间
	if (msg.dwDelay > 0)
		pMsg->dwDelay += pLogicEngine->getTickCount();
	m_EntityMsgList.linkAtLast(pMsg);
}

void CEntity::ClearDelayDelEntityMsg()
{	
	CEntityMsg *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;		
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (pMsg->bWaitDel)
		{	
			it.remove(pNode);
			pLogicEngine->DestoryEntityMsg(pMsg,this);
		}
	}
	m_bHasClearMarkedMsg = false;
}

INT_PTR CEntity::DeleteEntityMsg(const INT_PTR nMsg)
{
	INT_PTR result = 0;
	CEntityMsg *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (!pMsg->bWaitDel && pMsg->nMsg == nMsg)
		{			
			pMsg->bWaitDel = true;
			if (!m_bHasClearMarkedMsg)
				m_bHasClearMarkedMsg = true;
			result++;
		}
	}
	return result;
}

void CEntity::UpdateEntityMsg(const CEntityMsg &msg)
{
	CEntityMsg *pMsg;
	INT_PTR nMsgId = msg.nMsg;
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;
	//在消息队列中查找消息
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		//找到则替换数据
		if (!pMsg->bWaitDel && pMsg->nMsg == nMsgId)
		{
			*pMsg = msg;
			//调整消息时间，外部传递的是基于当前的时间
			if (msg.dwDelay > 0)
				pMsg->dwDelay += GetGlobalLogicEngine()->getTickCount();
			return;
		}
	}
	//没有此消息则添加消息
	PostEntityMsg(msg);
}

void CEntity::ProcessEntityMsg(const CEntityMsg &msg)
{
}

//设置实体的方向
void CEntity::SetDir(INT_PTR nDir )
{
	if (GetAttriFlag().DenySetDir)
	{
		return;
	}
	if (nDir >=0)
	{
		SetProperty<unsigned int>(PROP_ENTITY_DIR,(unsigned int)nDir);
	}
	else//那么随机设置一个方向
	{
		ULONGLONG randValue= GetGlobalLogicEngine()->GetRandValue(); 
		SetProperty<unsigned int>(PROP_ENTITY_DIR,(unsigned int)randValue % DIR_STOP);		
	}
}
INT_PTR CEntity::GetTargetDir(CEntity * pTarget)
{
	if(pTarget ==NULL || pTarget ==this) return DIR_STOP;

	int nTargetPosx,  nTargetPosy;
	pTarget->GetPosition(nTargetPosx,nTargetPosy); //目标的位置

	int nSelfPosx,  nSelfPosy; 
	GetPosition(nSelfPosx,nSelfPosy);

	return GetDir(nSelfPosx,nSelfPosy,nTargetPosx,nTargetPosy);
}

bool CEntity::getPosRangeByDir(INT_PTR &nX, INT_PTR &nY, INT_PTR &nDir, INT_PTR nRange) const
{
	int nSelfPosx,  nSelfPosy; 
	GetPosition(nSelfPosx,nSelfPosy);

	//for (int i = 0; i < 8; i++)
	//{
		//nDir = (nDir + i)%8;
		//for (int j = nRange; j >= 1; j--)
		//{
			if ((nDir >= 0 && nDir <= 1) || nDir == 7) {
				nSelfPosy -= nRange;
			}
			if (nDir >= 3 && nDir <= 5) {
				nSelfPosy += nRange;
			}
			if (nDir >= 1 && nDir <= 3) {
				nSelfPosx += nRange;
			}
			if (nDir >= 5 && nDir <= 7) {
				nSelfPosx -= nRange;
			}
			nX = nSelfPosx;
			nY = nSelfPosy;
			//if (this->GetScene()->CanCross(this->GetType(),nX,nY))
			//{
			//	return true;
			//}
		//}
	//}
	//return false;
}

void CEntity::SetTarget(const EntityHandle& handle) 
{	
	CEntity * pEntity = GetEntityFromHandle(handle );
	if(pEntity)
	{
		if (pEntity->GetScene() != GetScene())	//不在同一个场景 不设置目标
			return;

		INT_PTR nDir = GetProperty<unsigned int>(PROP_ENTITY_DIR);
		INT_PTR nTargetDir= GetTargetDir(pEntity);
		if( nDir != nTargetDir && nTargetDir != DIR_STOP)
		{
			SetDir(nTargetDir);
		}
	}
	
	if(m_targetHandler != handle)
	{
		m_targetHandler =handle; 
	}
}


INT_PTR CEntity::GetDir(INT_PTR nSourceX, INT_PTR nSourceY, INT_PTR nTargetX, INT_PTR nTargetY)
{
	INT_PTR nDir = DIR_UP;
	INT_PTR nSelfPosx = nTargetX - nSourceX;
	INT_PTR nSelfPosy = nTargetY - nSourceY;
	
	if(nSelfPosx >0)
	{
		if( nSelfPosy >0 )
		{
			nDir =DIR_DOWN_RIGHT  ;
		}
		else if(nSelfPosy ==0)
		{
			nDir = DIR_RIGHT;
		}
		else
		{
			nDir =DIR_UP_RIGHT ;
		}
	}
	else if(nSelfPosx ==0)
	{
		if( nSelfPosy >0 )
		{
			nDir =DIR_DOWN  ;
		}
		else if(nSelfPosy ==0)
		{
			nDir = DIR_STOP;
		}
		else
		{
			nDir =DIR_UP ;
		}
	}
	else
	{
		if( nSelfPosy >0 )
		{
			nDir = DIR_DOWN_LEFT ;
		}
		else if(nSelfPosy ==0)
		{
			nDir = DIR_LEFT;
		}
		else
		{
			nDir = DIR_UP_LEFT;
		}
	}
	return nDir;
}

void CEntity::TeleportOwnedEntity(CFuBen *pFB, 
								INT_PTR nSceneID, 
								INT_PTR nPosX, 
								INT_PTR nPosY, 
								INT_PTR nOwnerSceneId, 
								INT_PTR nOwnerPosX, 
								INT_PTR nOwnerPosY, 
								EntityTransferStype transStype)
{	
	if (GetType() == enActor) // 目前仅仅支持玩家传送的时候把归属实体传送过去！
	{
	}
}

bool CEntity::Teleport(CFuBen* pFB,INT_PTR nSceneID, INT_PTR nPosX,INT_PTR nPosY)
{
	//只能在本副本内传送，不能跨副本
	int nOwnerPosX, nOwnerPosY;
	GetPosition(nOwnerPosX, nOwnerPosY);
	CFuBenManager *pFBMgr = GetGlobalLogicEngine()->GetFuBenMgr();
	//CFuBen *pFB = GetFuBen();
	bool bRet = pFBMgr->EnterFuBen(this, pFB, nSceneID, nPosX, nPosY);
	if (bRet && GetType() == enActor)
	{
	}
	return bRet;
}

VOID CEntity::SetLevel(unsigned int nLevel)
{
	if (nLevel > 999) return;
	
	int nOldValue = GetProperty<unsigned int>(PROP_CREATURE_LEVEL );
	if( nLevel !=  nOldValue )
	{
		SetProperty<unsigned int>(PROP_CREATURE_LEVEL,nLevel);
		int nNewLevel = (int)nLevel;

		/*
		//满级添加文明度
		if(GetType() == enActor)
		{
			if (nLevel == GetLogicServer()->GetDataProvider()->GetGlobalConfig().GetPlayerMaxLevel(GetProperty<unsigned int>(PROP_ACTOR_CIRCLE)))
			{
				GetGlobalLogicEngine()->GetCultureManager().CalculateCultureValue(1);
			}
		}
		*/
		if(nNewLevel > nOldValue)
			OnLevelUp(nNewLevel - nOldValue); //等级提升
	}
}

bool CEntity::TelportSceneByName(LPCSTR sSceneName,INT_PTR nPosX,INT_PTR nPosY, int nType)
{
	if(sSceneName ==NULL)
	{
		OutputMsg(rmWaning,_T("sSceneName =NULL"));
		return false;
	}
	if (enNpc == GetType())
	{
		OutputMsg(rmError, _T("%s:npc[%s]不能传送"), __FUNCTION__, GetEntityName());
		return false;
	}
	CFuBen * pFb =NULL;     //副本的指针
	CScene * pScene = GetScene(); //场景指针
	CScene * pLastScene = pScene;
	CFuBen * pLasfb = GetFuBen();
	if (pScene && strcmp(sSceneName,pScene->GetSceneName()) == 0)//本身就在这个场景
	{
		return pScene->EnterScene(this,nPosX,nPosY, nType);
	}
	if(GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneName(sSceneName,pFb,pScene))
	{
		if(pFb &&pScene )
		{
			int nTargetSceneId = pScene->GetSceneId();
			if (nTargetSceneId == 0)
			{
				OutputMsg(rmWaning,_T(" TelportSceneByName (0)fail, actorname=%s"),GetEntityName());
				return false;
			}
			int nOwnerPosX, nOwnerPosY;
			GetPosition(nOwnerPosX, nOwnerPosY);
			INT_PTR nOwnerSceneId = GetSceneID();
			bool bRet = pFb->Enter(this,nTargetSceneId,nPosX,nPosY,0,0,0,0,-1,pLastScene,pLasfb);
			
			if (bRet)
			{
				TeleportOwnedEntity(pFb, nTargetSceneId, nPosX, nPosY, nOwnerSceneId, nOwnerPosX, nOwnerPosY, etsOther);
			}
			return bRet;
		}
	}
	OutputMsg(rmWaning,_T(" TelportSceneByName fail, no sSceneName =%s"),sSceneName);
	return false;
}

//传送到场景的默认点
bool CEntity::TelportSceneDefaultPoint(LPCSTR sSceneName,INT_PTR nPointID)
{
	CFuBen * pFb =NULL;     //副本的指针
	CScene * pScene = NULL; //场景指针
	CScene * pLastScene = this->GetScene();
	CFuBen * pLasfb = this->GetFuBen();
	if(GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneName(sSceneName,pFb,pScene))
	{
		if(pFb &&pScene )
		{
			INT_PTR nPosX=0;
			INT_PTR nPosY=0;
			if(pScene->GetSceneAreaCenter(nPosX,nPosY,nPointID) ==false)
			{
				OutputMsg(rmWaning,_T("No default point in the scene"));
				return false;
			}
			int nOwnerPosX, nOwnerPosY;
			GetPosition(nOwnerPosX, nOwnerPosY);
			int nOwnerSceneId = GetSceneID();
			bool bRet = pFb->Enter(this,pScene->GetSceneId(),nPosX,nPosY,0,0,0,0,-1,pLastScene,pLasfb);
			if (bRet)
			{
				TeleportOwnedEntity(pFb, pScene->GetSceneId(), nPosX, nPosY, nOwnerSceneId, nOwnerPosX, nOwnerPosY, etsOther);
			}
			return bRet;
		}
	}
	OutputMsg(rmWaning,_T(" TelportSceneDefaultPoint fail, no sSceneName =%s,pointID=%d"),sSceneName,nPointID);
	return false;
}

/*
(nType, nValue) 对应的是（0，13）协议的（传送类型，传送类型值）
*/
bool CEntity::MoveToEntity(CEntity * pEntity,int nType, int nValue, int  nDest)
{
	if(pEntity ==NULL) return false;

	CFuBen * pFb = pEntity->GetFuBen();     //副本的指针
	CScene * pScene = pEntity->GetScene(); //场景指针
	CFuBen * pLasfb = pFb;
	CScene * pLastScene = pScene;
	if(pFb &&pScene )
	{
		int nPosX= 0; //x
		int nPosY = 0;//y
		pEntity->GetPosition(nPosX,nPosY);
		//---处理宠物跑到玩家前方
		INT_PTR nDir = (pEntity->GetProperty<int>(PROP_ENTITY_DIR) + 4) & 7; //玩家反方向
		INT_PTR nTargetPosX = nPosX;
		INT_PTR nTargetPosY = nPosY;
		CEntityGrid::NewPosition(nTargetPosX,nTargetPosY,nDir,1);
		
		int nOwnerPosX, nOwnerPosY;
		GetPosition(nOwnerPosX, nOwnerPosY);
		int nOwnerSceneId = GetSceneID();
		bool bRet = pFb->Enter(this,pScene->GetSceneId(),nTargetPosX,nTargetPosY,0,0,nType,nValue,-1,pLastScene,pLasfb);
		if (bRet)
			TeleportOwnedEntity(pFb, pScene->GetSceneId(), nTargetPosX, nTargetPosY, nOwnerSceneId, nOwnerPosX, nOwnerPosY, etsOther);
		if (!bRet)
		{
			int nNewPosX = nPosX, nNewPosY = nPosY;
			if(nDest > 0)
			{
				pScene->GetEntityAppearPos(nPosX, nPosY, nDest,nNewPosX, nNewPosY);
				if(nNewPosX > 0 && nNewPosY)
				{
					nPosX = nNewPosX;
					nPosY = nNewPosY;
				}
			}
			bRet = pFb->Enter(this,pScene->GetSceneId(),nPosX,nPosY,0,0,nType,nValue,-1,pLastScene,pLasfb);
		}
		return bRet;
	}
	//OutputMsg(rmWaning,_T(" MoveToEntity fail for pFb ==NULL || pScene ==NULL "));
	return false;
}
VOID CEntity::BeforeLogicRun(TICKCOUNT nCurrentTime)
{
}

VOID CEntity::LogicRun(TICKCOUNT nCurrentTime)
{
	if (m_bHasClearMarkedMsg)
		ClearDelayDelEntityMsg();

	if (m_EntityMsgList.count() > 0)
	{
		CEntityMsg *pMsg;
		CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
		CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
		CList<CEntityMsg*>::NodeType *pNode;
		//处理实体消息
		for (pNode=it.first(); pNode; pNode = it.next())
		{
			pMsg = *pNode;
			if(pMsg)
			{
				if (!pMsg->bWaitDel && nCurrentTime >= pMsg->dwDelay)
				{			
					ProcessEntityMsg(*pMsg);	
					pMsg->bWaitDel = true;	
					if (!m_bHasClearMarkedMsg)
						m_bHasClearMarkedMsg = true;
				}
			}
		}
	}

	// Handle Script Callback
	if (m_t1s.CheckAndSet(nCurrentTime, true))
		ProcessScriptCallback();
}

VOID CEntity::AfterLogicRun(TICKCOUNT nCurrentTime)
{
}

bool CEntity::IsInSameScreen( CEntity* pOther )
{
	if (GetScene() != pOther->GetScene()) return false;

	int myX = 0, myY = 0;
	GetPosition(myX,myY);
	int youX = 0, youY = 0;
	pOther->GetPosition(youX,youY);

	int abs = __max(myX,youX) - __min(myX,youX);
	if (abs > MOVE_GRID_COL_RADIO) return false;
	abs = __max(myY,youY) - __min(myY,youY);
	if (abs > MOVE_GRID_ROW_RADIO) return false;
	return true;
}

void CEntity::OnEntityDeath()
{
	// 死亡清除消息队列
	CEntityMsg *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (!pMsg->bWaitDel)
		{
			pMsg->bWaitDel = true;								// 标示消息等待删除		
			if (!m_bHasClearMarkedMsg)
				m_bHasClearMarkedMsg = true;
		}
	}

	// 副本事件处理
	CFuBen *pFB = this->GetFuBen();
	if (pFB && pFB->IsFb())
	{
		// 副本实体死亡事件
		CScriptValueList paramList;
		paramList << (int)CFubenSystem::enOnDeath;
		paramList << (int)pFB->GetConfig()->nFbConfType;
		paramList << (int)pFB->GetFbId();
		paramList << (int)pFB->GetConfig()->nFbEnterType;
		paramList << pFB;
		paramList << (int)this->GetSceneID();
		paramList << this;
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
		{
			OutputMsg(rmError,"[FubenSystem] OnEvent OnDeath 错误，副本ID=%d，副本类型=%d !",pFB->GetFbId(),pFB->GetConfig()->nFbConfType);
		}
	}

	//普通场景的活动区域死亡回调
	if (pFB && !(pFB->IsFb()))
	{
		// 触发进入活动区域事件
        int nPosX,nPosY;
        this->GetPosition(nPosX,nPosY);
        CScene* pScene = this->GetScene();
		SCENEAREA* pArea = pScene->GetAreaAttri(nPosX, nPosY);
		if (pScene && pArea && pScene->HasMapAttribute(nPosX,nPosY,aaActivity,pArea))
		{
    		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        	CScriptValueList paramList;
			AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
			for (size_t i = 0; i < pAreaAttr->nCount; i++)
			{
				int nAtvId = pAreaAttr->pValues[i];
				// 个人活动
				if((this->GetType() == enActor) && ((CActor*)this)->GetActivitySystem().IsActivityRunning(nAtvId))
				{
					if (PersonActivtyData* pActivty = ((CActor*)this)->GetActivitySystem().GetActivity(nAtvId))
					{
						paramList.clear();
						paramList << (int)CActivitySystem::enOnAtvAreaDeath;
						paramList << (int)pActivty->nActivityType;
						paramList << (int)pActivty->nId;
						paramList << this;
						if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
						{
							OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
						}
					}
				}
				// 全局活动
				else if(GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
				{
					if (GlobalActivityData* pActivty = GetGlobalLogicEngine()->GetActivityMgr().GetActivity(nAtvId))
					{
						paramList.clear();
        	            paramList << (int)CActivitySystem::enOnAtvAreaDeath;
        	            paramList << (int)pActivty->nActivityType;
        	            paramList << (int)pActivty->nId;
						paramList << this;
        	            if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
        	            {
        	                OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
        	            }
					}
				}
			}
		}	
		
	}

	//加到死亡列表中
	CScene* pSc = GetScene();
	if (pSc)
	{
		pSc->AddDeathEntity(this);
	}

	//if (GetType() == enActor)
	//{
	//	//群发通知队友
	//	CTeam* pTeam = ((CActor*)this)->GetTeam();
	//	if (pTeam)
	//	{
	//		char data[16];
	//		unsigned int nActorId = GetId();
	//		CDataPacket pack(data,sizeof(data));			
	//		pack << (BYTE)enTeamSystemID << (BYTE)sMemberDieRelive << (int)nActorId << (BYTE)0;//0:死亡，1：复活
	//		pTeam->BroadCast(data,pack.getPosition(),nActorId);
	//	}
	//}
}

unsigned int CEntity::RegScriptCallback(unsigned int nNpcId, unsigned int nDelayTime, unsigned int nInterval, int nCount, const char *pszFuncname, const CScriptValueList &args)
{
	if (!pszFuncname) return 0;
	if (nCount < 0) nCount = -1;
	if (strlen(pszFuncname) > MAX_SCRIPT_FUN_NAME_LEN)
	{
		OutputMsg(rmError, _T("RegScriptCallback failed, scriptFnName is too long"));
		return 0;
	}
	if (nInterval < 5000) nInterval = 5000;  // 性能限制，最小间隔时间5s

	CEntityManager::EntitySCBHandle scbhandle;
	ScriptCallbackParam *param = GetGlobalLogicEngine()->GetEntityMgr()->CreateScriptCallbackParam(scbhandle);
	if (!param)
	{
		OutputMsg(rmNormal, _T("RegScriptCallback failed , reason: CreateScriptCallbackParam error"));
		return 0;
	}
	
	CScriptValueList paramList;
	paramList << this;
	if (nNpcId > 0)
	{
		paramList << nNpcId;
		paramList << pszFuncname;
	}
	paramList += args;
	param->init(scbhandle, nNpcId, nDelayTime, nInterval, nCount, pszFuncname, &paramList);
	INT_PTR nIdx = m_scriptCBList.add(param);
	if (0 == nDelayTime)
	{
		OnScriptCallback(nIdx);
		if (param->m_nCount == 0)
		{
			m_scriptCBList.remove(nIdx);
			GetGlobalLogicEngine()->GetEntityMgr()->DestroyScriptCallbackParam((CEntityManager::EntitySCBHandle&)(param->m_nHandle));
		}
	}
	else
	{
		param->m_nNextHitTime = (GetGlobalLogicEngine()->getTickCount()/1000)*1000 + param->m_nDelayTime;
	}

	return scbhandle;
}

void CEntity::UnregScriptCallback(unsigned int handle)
{	
	for (INT_PTR i = 0; i < m_scriptCBList.count(); i++)
	{
		ScriptCallbackParam *param = m_scriptCBList[i];	
		if (param->m_nHandle == handle)
		{
			param->m_nCount = 0;
			break;
		}
	}
}

void CEntity::UnregScriptCallback(const char *pFnName)
{
	if (!pFnName) return;
	for (INT_PTR i = 0; i < m_scriptCBList.count(); i++)
	{
		ScriptCallbackParam *param = m_scriptCBList[i];	
		if (param && strcmp(pFnName, param->m_szScriptFn) == 0)
		{
			param->m_nCount = 0;		
		}
	}
}

void CEntity::OnScriptCallback(INT_PTR nIdx)
{
	if (nIdx < 0 || nIdx >= m_scriptCBList.count())
		return;
	
	
	ScriptCallbackParam *param = m_scriptCBList[nIdx];
	CScriptValueList ret;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("OnNpcEvent", param->m_paramList, ret, 0))
	{
		if (param->m_paramList.count() >= 3)
		{
			CScriptValue& sValue = param->m_paramList[2];
			if (sValue.getType() == CScriptValue::vString)
			{
				OutputMsg(rmError,_T("OnScriptCallback Function: %s"),(LPCTSTR)sValue);
			}
		}
		
	}

	if (param->m_nCount > 0)
		param->m_nCount--;
	if (param->m_nCount != 0)	
		param->m_nNextHitTime = (GetGlobalLogicEngine()->getTickCount()/1000)*1000 + param->m_nInterval;
}

void CEntity::ProcessScriptCallback()
{
	TICKCOUNT curTime = GetGlobalLogicEngine()->getTickCount();
	bool bNeedClear = false;
	for (INT_PTR i = 0; i < m_scriptCBList.count(); i++)
	{
		ScriptCallbackParam *param = m_scriptCBList[i];
		if (param)
		{
			if (curTime >= param->m_nNextHitTime)
				OnScriptCallback(i);
			if (!bNeedClear && param->m_nCount == 0)
				bNeedClear = true;
		}
	}
	if (bNeedClear)
	{
		for (INT_PTR i = m_scriptCBList.count()-1; i >= 0; i--)
		{
			ScriptCallbackParam *param = m_scriptCBList[i];
			if (param && param->m_nCount == 0)
			{
				m_scriptCBList.remove(i);
				GetGlobalLogicEngine()->GetEntityMgr()->DestroyScriptCallbackParam((CEntityManager::EntitySCBHandle&)(param->m_nHandle));
			}
		}
	}	
}

void CEntity::ClearScriptCallback()
{
	for (INT_PTR i = 0; i < m_scriptCBList.count(); i++)
	{
		ScriptCallbackParam *param = m_scriptCBList[i];
		if (param)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->DestroyScriptCallbackParam((CEntityManager::EntitySCBHandle&)(param->m_nHandle));
		}
	}

	m_scriptCBList.clear();
}

void CEntity::PostEntityDeadMsg()
{
	if (GetType() == enMonster && ((CMonster*)this)->GetMonsterType() ==4 && GetFuBen() &&  GetFuBen()->GetFbId() ==0)
	{
		OutputMsg(rmNormal, _T("Boss消失 PostEntityDeadMsg"));
	}
	
	CEntityMsg msg(CEntityMsg::emEntityDead, GetHandle());
	msg.nForceCallWhenDestroy = true;
	msg.dwDelay = 0;	
	PostEntityMsg(msg);
}

bool CEntity::CanSee(CEntity* pEntity)
{
	bool canSee = true;
	INT_PTR nType = GetType();

	//次元Boss 归属玩家不在视野中，也能看到
	if ( pEntity && pEntity->GetType() == enActor && ((CActor *)pEntity)->IsBeLongBossActor() && pEntity->GetFuBen() && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
	{
		return true;
	}

	//次元Boss 怪物不在视野中，也能看到
	if ( pEntity && pEntity->GetType() == enMonster && pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID) == 386 && pEntity->GetFuBen()->GetFbId() == 23 && pEntity->GetSceneID() == 317 && pEntity->GetProperty<unsigned int>(PROP_CREATURE_HP) > 0 )
	{
		return true;
	}

	//如果能看到隐身的，就返回true
	if (m_attriFlag.CanSeeHideActor) return true;

	//如果目标不可见的话
	if (pEntity->GetAttriFlag().DenySee)
		return false;

	//玩家能看到所有的
	if(nType ==enActor)
		return true;

	if(pEntity->isAnimal())
	{
		if( ((CAnimal*)pEntity)->GetBuffSystem()->Exists(aHide) )
		{
			//能看到队友
			/*
			if( GetType() == enActor && pEntity->GetType() == enActor ) 
			{
				if( ((CActor*)this)->GetTeam()!= NULL &&  ((CActor*)this)->GetTeam() == ((CActor*)pEntity)->GetTeam() )
				{
					return true;
				}
			}
			*/

			//能看见目标
			//if (GetTarget() == pEntity->GetHandle())
			//{
			//	return true;
			//}
			
			//宝宝能够看到他的主人
			if(nType== enMonster  && pEntity->GetType() == enActor )
			{
				if( ((CMonster*)this)->GetOwner() == pEntity->GetHandle() )
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return true;
}

void CEntity::SetEntityName(const char * name)
{
	if(name ==NULL) return;
	_asncpytA( m_sEntityName,name );
	if(IsMonster() && m_attriFlag.ShowNameByNumber == false)
	{
		INT_PTR nLen = strlen(name);
		if (nLen > 2)
		{
			if (m_sEntityName[nLen - 1] >='0' && m_sEntityName[nLen - 1] <='9' )
			{
				m_sEntityName[nLen - 1] = 0;
			}
			if (m_sEntityName[nLen - 2] >= '0' && m_sEntityName[nLen - 2] <='9')
			{
				m_sEntityName[nLen - 2] = 0;
			}
		}
	}
	if (GetType() == enActor)
	{
		if (!((CActor *)this)->IsInRawServer())
		{
			LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpServerIdPattern);
			int nRawSId = ((CActor *)this)->GetRawServerIndex();
			char buff[256];
			sprintf(buff, sFormat, nRawSId);
			int nLen = __min((int)(strlen(buff)), 255);
			buff[nLen] = '\0';
			strcat(buff, m_sEntityName);
			_asncpytA(m_sEntityName, buff);
		}
	}

	SetShowName(m_sEntityName);

}

void CEntity::Face2Target(CEntity* pTarget){
	SetDir( GetTargetDir(pTarget) );
}
