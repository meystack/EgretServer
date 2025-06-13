#include "StdAfx.h"
#include "FuBen.h"
#include "../base/Container.hpp"
CFuBen::CFuBen()
{
	m_pConfig = NULL;
}

CFuBen::~CFuBen()
{ 
	RefreshFbPos(); 
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene)
		{
			CFuBenManager::m_SceneMgr->Release(pScene->GetHandle());//这里释放场景里面的npc等实体，角色应该在这之前传送出副本
		}
	}
	m_vSceneList.empty();
	m_DynamicVar.clear();
}
void CFuBen::RefreshFbPos()
{
	//如果当前的刷新位置和当前的副本一样的话，则重置位置
	if(CFuBenManager::m_RefreshPara.pFuben ==this)
	{
		OutputMsg(rmNormal,"FB Destroy reset fb  refresh pos,id=%d",(int)GetFbId());
		CFuBenManager::m_RefreshPara.setFB(NULL);
	}
}



VOID CFuBen::Init(bool boRun)
{
	m_DestoryTime = 0;
	m_nPlayerLevelSum = 0;
	m_nPlayerCount = 0;
	ZeroMemory(&m_nValues,sizeof(m_nValues));
	m_restTime = 0;

	m_ActorDieTime = 0;			//玩家打副本过程中一共死亡的次数
	m_KillMonsterCount = 0;		//一共杀死的怪物的数量
	ZeroMemory(m_szKillBossName,sizeof(m_szKillBossName));		//保存杀死boss的玩家的名字
	m_KillNameCount = 0;
	m_MonsterTotal = 0;
	m_boFree = false;

	m_DynamicVar.clear();

	m_nGuildId = 0;
}


void CFuBen::Reset()
{
	Init();//把变量重置一下
	//关键把把场景重置
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pSrcScene = m_vSceneList[i];
		if(pSrcScene) {
			pSrcScene->Reset();
		}
	}
	RefreshFbPos();
	m_boFree = true;
}

void CFuBen::Trace()
{
	if(m_pConfig)
	{
		OutputMsg(rmNormal,"Trace FB %d",m_pConfig->nFbId);
	}
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pSrcScene = m_vSceneList[i];
		if(pSrcScene)
		{
			pSrcScene->Trace();
		}
	}
}

int CFuBen::RunOne()
{
	DECLARE_TIME_PROF("CFuBen::RunOne");
	if (CFuBenManager::m_RefreshPara.pFuben == NULL)
	{
		CFuBenManager::m_RefreshPara.setFB(this);
	}
	if (CFuBenManager::m_RefreshPara.pFuben != this)
	{
		//正常终止
		return 0;
	}
	bool isFb = IsFb();

	//判断副本时间是否到，如果到的话触发脚本事件，脚本将所有玩家踢出
	if ( isFb && m_restTime != 0 && GetGlobalLogicEngine()->getMiniDateTime() > m_restTime )
	{	
		// 副本事件
		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		CScriptValueList paramList;
		paramList << (int)CFubenSystem::enOnTimeout;
		paramList << (int)GetConfig()->nFbConfType;
		paramList << (int)GetFbId();
		paramList << (int)GetConfig()->nFbEnterType;
		paramList << this;
		if(!globalNpc->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
		{
			OutputMsg(rmError,"[FubenSystem] OnEvent SceneTimeout 错误，副本ID=%d，副本类型=%d !",GetFbId(),GetConfig()->nFbConfType);
		}
		//return 2;//可以释放内存
	}
	INT_PTR nPlayerCount = 0;
	
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if(pScene) {
			nPlayerCount += pScene->GetPlayCount();
		}

		if (i != CFuBenManager::m_RefreshPara.nScenePos)
			continue;

		if (pScene && pScene->RunOne()) return 1;

		CFuBenManager::m_RefreshPara.setScene(i + 1);
	}

	CFuBenManager::m_RefreshPara.setFB(NULL);

	if (isFb)
	{
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();

		if (nPlayerCount <= 0)
		{
			if (m_DestoryTime != 0 && nNow > m_DestoryTime)
			{
				return 2;
			}
			if (m_DestoryTime == 0)
			{
				if(m_pConfig->nDestoryTime > 0)
				{
					m_DestoryTime = nNow + m_pConfig->nDestoryTime;
				}
				else if(m_pConfig->nDestoryTime == -1)
				{
					m_DestoryTime = 0;
				}
				else
				{
					m_DestoryTime = nNow + GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFubenDestoryTime;
				}
			}
		}
		else
		{
			m_DestoryTime = 0;
			TICKCOUNT nCurr = GetGlobalLogicEngine()->getTickCount();
			if (m_1sTimer.CheckAndSet(nCurr,true))
			{
				// 副本帧更新事件
				CScriptValueList paramList;
				paramList << (int)CFubenSystem::enOnUpdate;
				paramList << (int)GetConfig()->nFbConfType;
				paramList << (int)GetFbId();
				paramList << (int)GetConfig()->nFbEnterType;
				paramList << this;
				paramList << (int)nNow;
				if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
				{
					OutputMsg(rmError,"[FubenSystem] OnEvent OnUpdate 错误，副本ID=%d，副本类型=%d !",GetFbId(),GetConfig()->nFbConfType);
				}
			}
		}
	}

	return 0;
}


bool CFuBen::Enter(CEntity * pEntity,INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight, INT_PTR nHeight, int nType ,
	int nValue, int nParam, CScene* pLastScene, CFuBen* pLastFb)
{
	if (pEntity == NULL) return false;
	if (pEntity->GetType() == enActor)
	{
		if(!((CActor*)pEntity)->IsSceneChangeModel())
		{
			// int nModleId = ((CActor*)pEntity)->GetAlmirahSystem().GetNowModeId();
			// ((CActor*)pEntity)->SetProperty<unsigned int>(PROP_ACTOR_SWINGAPPEARANCE,nModleId);
		}
		if (((CActor*)pEntity)->GetGmLevel() == 0)
		{
			// int nPkValue = pEntity->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE);
			// if (GetLogicServer()->GetDataProvider()->GetPkConfig().redName <= nPkValue) 
			// {		
			// 	nScenceID = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonSceneId;//红名地图
			// 	nPosX = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonPosX;
			// 	nPosY = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonPosY;
			// 	nWeight = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonWeight;
			// 	nHeight = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonHeight;
			// }
		}
	}

	CScene* pScene = GetScene(nScenceID);	
	if(pScene ==NULL)
	{
		OutputMsg(rmError,_T(" Enter fb fail, for sceneID=%d is not exist"),nScenceID);
		return false;
	}
	if (pEntity->GetType() == enNpc)
	{
		OutputMsg(rmError, _T("npc[%s]试图传送到%s，现已阻止"), pEntity->GetEntityName(),pScene->GetSceneName());
		return false;
	}
	bool bResult = pScene->EnterSceneRandPt(pEntity,nPosX,nPosY,nWeight,nHeight,nType,nValue,nParam);

	if (bResult)
	{
		// 进入副本事件
		if (this->IsFb())
		{
			CScriptValueList paramList;
			paramList << (int)CFubenSystem::enOnEnter;
			paramList << (int)GetConfig()->nFbConfType;
			paramList << (int)GetFbId();
			paramList << (int)GetConfig()->nFbEnterType;
			paramList << this;
			paramList << (int)nScenceID;
			paramList << pEntity;
			if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
			{
				OutputMsg(rmError,"[FubenSystem] OnEvent OnEnter 错误，副本ID=%d，副本类型=%d !",GetFbId(),GetConfig()->nFbConfType);
			}
		}
		
		// 离开副本事件
		if (pLastScene && pLastFb && pLastFb->IsFb() && pLastFb != this)
		{
			CScriptValueList paramList;
			paramList << (int)CFubenSystem::enOnExit;
			paramList << (int)pLastFb->GetConfig()->nFbConfType;
			paramList << (int)pLastFb->GetFbId();
			paramList << (int)pLastFb->GetConfig()->nFbEnterType;
			paramList << pLastFb;
			paramList << (int)pLastScene->GetSceneId();
			paramList << pEntity;
			if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
			{
				OutputMsg(rmError,"[FubenSystem] OnEvent OnExit 错误，副本ID=%d，副本类型=%d !",pLastFb->GetFbId(),pLastFb->GetConfig()->nFbConfType);
			}
		}
	}

	if (bResult && pEntity->GetType() == enActor)
	{
		if (CLocalSender * pLogger = GetLogicServer()->GetLocalClient())
		{
		}
	}
	
	return bResult;
}

void CFuBen::UpdateFbPlayerCountAndAverageLevel(CEntity *pEntity, bool bAdd)
{	
	if (!IsFb() || !pEntity || pEntity->GetType() != enActor) return;
	CActor *pActor = (CActor *)pEntity;
	unsigned int nLevel = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	int nSign = bAdd ? 1 : -1;
	m_nPlayerLevelSum += nLevel * nSign;
	m_nPlayerCount += nSign;


	//OutputMsg(rmNormal, _T("fb player info changed, sum=%d playercount=%d"), m_nPlayerLevelSum, m_nPlayerCount);
}


void CFuBen::OnFbPlayerLevelChanged(int updateLevel)
{
	if (updateLevel != 0)
	{
		m_nPlayerLevelSum += updateLevel;
		//OutputMsg(rmNormal, _T("fb player level changed, sum=%d updateLevel=%d playercnt=%d"), m_nPlayerLevelSum, updateLevel, m_nPlayerCount);
	}
}


bool CFuBen::CanEnter(CActor * pActor)
{
	//这里需要调用脚本执行
	return true;
}


//克隆一个跟本副本一样的新的副本,DstFuBen是克隆本
VOID CFuBen::Clone(CFuBen& DstFuBen,bool boRun)
{
	DstFuBen.Init(boRun);
	DstFuBen.m_nFbId = m_nFbId;
	DstFuBen.m_pConfig = m_pConfig;
	if (boRun)
	{
		DstFuBen.SetFubenTime(0);
	}

	//把本副本下的所有场景复制出来
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		//pSrcScene是原始场景，pNewone是它的克隆
		CScene* pSrcScene = m_vSceneList[i];
		if(pSrcScene) {
			CSceneHandle hHandle = 0;
			CScene* pNewone = CFuBenManager::m_SceneMgr->Acquire(hHandle);
			if(pNewone) {
				pNewone->SetHandle(hHandle);
				DstFuBen.m_vSceneList.add(pNewone);
				pNewone->Init(pSrcScene->GetSceneData(),pSrcScene->m_pMapData,&DstFuBen,NULL,boRun);	
				pNewone->CreateNpcTransport(boRun); //创建npc和传送门
				pSrcScene->Clone(*pNewone,boRun);
			}
			
		}
	}
}

//往副本增加场景实例，这里的场景不会被释放，其他场景都从这里的场景clone出来
//本函数只应该在程序启动时读取配置数据的时候执行
BOOL CFuBen::AddStaticScene(SCENECONFIG* SceneData,CFuBenManager* pFbMgr,CObjectAllocator<char>& dataAlloc)
{
	CSceneHandle hHandle;
	CScene* newone = CFuBenManager::m_SceneMgr->Acquire(hHandle);

	char sMapFullBuff[40]; //地图文件的全名
	sprintf(sMapFullBuff,"data/map/%s",SceneData->szMapFileName);
	CAbstractMap* map = pFbMgr->GetMapData(sMapFullBuff);
	m_vSceneList.add(newone);

	BOOL result = newone->Init(SceneData,map,this,&dataAlloc);
	newone->SetHandle(hHandle);
	newone->SetAreaConfig(dataAlloc);
	newone->SetTelePortTable(dataAlloc);
	newone->SetRefreshList(&(SceneData->vRefreshList));
	newone->CreateNpcTransport(); //创建npc和传送门

	//newone->RefreshMonster();

	return result;
}

void CFuBen::RefreshMonster()
{
	for (INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if(pScene) {
			pScene->RefreshMonster();
		}
	}
}

unsigned int CFuBen::TotalSceneMonsterCount()
{
	unsigned int nTotalCount = 0;
	for (INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if(pScene) {
			nTotalCount += pScene->GetMonsterCount(0,0);
		}
	}
	return nTotalCount;
}

unsigned int CFuBen::GetMonsterCount(int nMonsterId, int nScenId)
{
	unsigned int nTotalCount = 0;
	for (INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (nScenId == 0 || (pScene && pScene->GetSceneId() == nScenId))
		{
			nTotalCount += pScene->GetMonsterCount(nMonsterId,0);
		}
	}
	return nTotalCount;
}

void CFuBen::AddRefuseActor( UINT nActorid )
{
	m_RefuseList.linkAtLast(nActorid);
}

bool CFuBen::IsRefuseActor( UINT nActorid )
{
	CLinkedNode<EntityHandle> *pNode;
	CLinkedListIterator<EntityHandle> it(m_RefuseList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode->m_Data == nActorid)
		{
			return true;
		}
	}	
	return false;
}

void CFuBen::ExitAllActor()
{
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene)
		{
			pScene->ExitAllActor();
		}
	}
}

int CFuBen::GetFubenActorCount()
{
	int nCount = 0;

	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene)
		{
			CEntityList& list = pScene->GetPlayList();
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
				if (pEntity && pEntity->GetType() == enActor)
				{
					nCount++;
				}		
			}
		}
	}

	return nCount;
}

CScene* CFuBen::GetScene(INT_PTR nScenceID)
{
	CScene* pResult = NULL;
	INT_PTR nCount = m_vSceneList.count();
	if (nCount <= 0) return NULL;
	for(INT_PTR i = 0; i < nCount; i++)
	{
		//找到所在的场景
		CScene* pScene = m_vSceneList[i];
		if (pScene != NULL && pScene->GetSceneId() == nScenceID)
		{
			return pScene;
		}
	}
	
	if (m_nFbId > 0)
	{
		pResult = m_vSceneList[0];
	}
	return pResult;
}

CScene * CFuBen::GetSceneByName(LPCTSTR sName)
{
	if(sName ==NULL) return NULL;
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		//找到所在的场景
		CScene* pScene = m_vSceneList[i];
		if (pScene != NULL && strcmp( pScene->GetSceneName() , sName ) ==0 )
		{
			return pScene;
		}
	}
	return NULL;
}

bool CFuBen::HasScene( int nSceneId )
{
	for(int i = 0; i < m_vSceneList.count(); i++)
	{
		//找到所在的场景
		CScene* pScene = m_vSceneList[i];
		if (pScene && pScene->GetSceneId() == nSceneId)
		{
			return true;
		}
	}
	return false;
}


int CFuBen::getPlayerAverageLvl() const
{
	int nAverageLevel = 0;
	if (m_nPlayerCount)
		nAverageLevel = m_nPlayerLevelSum / m_nPlayerCount;
	return nAverageLevel;
}

void CFuBen::updateFubenPlayerCount(int nCount)
{
	if (IsFb() && m_nPlayerCount != nCount)
		m_nPlayerCount = nCount;
}

int CFuBen::getPlayerCount() const
{
	return m_nPlayerCount;
}

UINT CFuBen::GetFubenTime()
{
	if (m_restTime == 0) return 0;
	int restTime = m_restTime - GetGlobalLogicEngine()->getMiniDateTime();
	return restTime < 0 ? 0 : restTime;
}

void CFuBen::_SetFubenTime( UINT nTime, LPCSTR file, INT_PTR line  )
{
	if(!IsFb())
	{
		OutputMsg(rmError,"SetFubenTime error, is static fb,time=%d,fun=%s,line=%d",
			(int)nTime,(char*)file,(int)line);
		return;
	}
	m_restTime = (nTime != 0)?GetGlobalLogicEngine()->getMiniDateTime() + nTime:0;
}

void CFuBen::SendData( LPCVOID lpData, SIZE_T dwSize )
{
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	for ( INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene)
		{
			CEntityList& list = pScene->GetPlayList();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
				if (pEntity && pEntity->GetType() == enActor)
				{
					((CActor*)pEntity)->SendData(lpData,dwSize);
				}		
			}
		}
	}
}

void CFuBen::OnReuse()
{
	for ( INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene)
		{
			pScene->OnReuse();
		}
	}
	SetFubenTime(0);
}

INT_PTR CFuBen::ReloadNpcScript( LPCTSTR szSceneName,LPCTSTR szNpcName )
{
	INT_PTR result = 0;

	for(INT_PTR i = 0; i < m_vSceneList.count(); i++)
	{
		CScene* pScene = m_vSceneList[i];
		if (pScene && (szSceneName == NULL || strcmp(pScene->GetSceneName(),szSceneName) == 0))
		{
			result += pScene->ReloadNpcScript(szNpcName);
		}
	}
	return result;
}

void CFuBen::SetFreeFlag( bool boFlag )
{
	m_boFree = boFlag;
	CFuBenManager::m_FuBenMgr->ReNew(m_hFbHandle);
}

bool CFuBen::change_create_monster_flag(bool flag){
	if(flag !=create_monster_flag){ create_monster_flag = flag ;}
	return create_monster_flag;
}
