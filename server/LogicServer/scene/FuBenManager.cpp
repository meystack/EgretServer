#include "StdAfx.h"
#include "FuBenManager.h"
#include "../base/Container.hpp"

CFuBenHandleMgr*		CFuBenManager::m_FuBenMgr;			//
SceneHandleMgr*		CFuBenManager::m_SceneMgr;
CFuBenManager::RefreshPara			CFuBenManager::m_RefreshPara;
EntityVector*		CFuBenManager::m_pVisibleList;
//FubenHandleList::LinkNodeMgr* FubenHandleList::g_LinkNodeMgr;

CFuBenManager::CFuBenManager(void)
{
	ZeroMemory(m_anFubenCount, sizeof(m_anFubenCount));
	m_vMuiltyFbMap.clear();
}

VOID  CFuBenManager::Destroy()
{
	// 删除动态FB场景
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif
	{	
		CLinkedNode<CFuBen*> *pNode;
		CLinkedListIterator<CFuBen*> it(m_vAllFbList);
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			CFuBen* pFb = pNode->m_Data;
			if (pFb)
			{
				pFb->ExitAllActor();
				m_FuBenMgr->Release(pFb->GetHandle());//关闭副本,所有玩家会先退出副本
				it.remove(pNode);
			}
		}

		// 删除静态FB场景
		for(int i = 0; i < m_vStaticFbList.count(); i++)
		{
			CFuBen* pFb = m_vStaticFbList[i];
			if (pFb)
			{
				m_FuBenMgr->Release(pFb->GetHandle());//里面会执行副本的析构函数
			}
		}
		m_vStaticFbList.clear();

		// 删除预创建的动态FB（暂时还未用到的动态副本）
		for (INT_PTR i = 0; i < m_FreeFbList.count(); i++)
		{
			FUBENVECTOR* pFbList = m_FreeFbList[i];
			if (pFbList)
			{
				for(int j = 0; j < pFbList->count(); j++)
				{
					CFuBen* pFb = (*pFbList)[j];
					if (pFb)
					{
						m_FuBenMgr->Release(pFb->GetHandle());//里面会执行副本的析构函数
					}
				}
				pFbList->clear();
				delete pFbList;
			}
		}
		m_FreeFbList.clear();

		// 删除地图数据
		for(int i = 0; i < m_vMapDataList.count(); i++)
		{
			MAPDATA* md = m_vMapDataList[i];
			if(md) {
				delete md->pMapData;
				delete md;
			}
			
		}
		m_vMapDataList.clear();

	}

#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}	
#endif 
#endif
} 	

CFuBenManager::~CFuBenManager(void)
{
	Destroy();
}

CFuBen* CFuBenManager::CreateFuBen(int nFbId)
{
	// 验证FBId的有效性
	if ((nFbId < 0) || (nFbId >= m_vStaticFbList.count()))
	{
		return NULL;
	}

	// 若为多人副本，则先看看是否已生成
	if (m_vStaticFbList[nFbId]->GetConfig()->nFbEnterType == 2)
	{
		if(m_vMuiltyFbMap.find(nFbId) != m_vMuiltyFbMap.end())
			return m_vMuiltyFbMap[nFbId];
	}

	CFuBen* newFb = NULL;
#ifndef REUSEFB
	newFb = AllocNewFb(nFbId);
#else
	// 看是否有空闲的副本
	FUBENVECTOR* pList = m_FreeFbList[nFbId];
	if (pList && pList->count() > 0)
	{
		newFb =  (*pList)[pList->count()-1];
		pList->remove(pList->count()-1);
		newFb->OnReuse();
		newFb->SetFreeFlag(false);
	}else
	{
		// 没有空闲副本，创建一个新的Fb
		newFb = AllocNewFb(nFbId);
	}
#endif
	m_vAllFbList.linkAtLast(newFb);
	
	if(newFb)
	{
		if(GetLogicServer()->GetLocalClient())
		{
		}

		// 若为多人副本，则记录
		if (newFb->GetConfig()->nFbEnterType == 2)
		{
			m_vMuiltyFbMap[nFbId] = newFb;
		}

		// 创建副本事件
		CScriptValueList paramList;
		paramList << (int)CFubenSystem::enOnCreate;
		paramList << (int)newFb->GetConfig()->nFbConfType;
		paramList << (int)newFb->GetFbId();
		paramList << (int)newFb->GetConfig()->nFbEnterType;
		paramList << newFb;
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
		{
			OutputMsg(rmError,"[FubenSystem] OnEvent OnCreate 错误，副本ID=%d，副本类型=%d !",newFb->GetFbId(),newFb->GetConfig()->nFbConfType);
		}
	}
	return newFb;
}

VOID CFuBenManager::ReleaseFuBen(CFuBen* pFuBen)
{
	// 常规副本不能这里关闭
	if (!pFuBen->IsFb()) 
		return;

	// 动态副本关闭 -- 这里怎么没有加入到自由副本列表中，而是直接删除了？？
	CLinkedNode<CFuBen*> *pNode;
	CLinkedListIterator<CFuBen*> it(m_vAllFbList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CFuBen* pFbInList = pNode->m_Data;
		if (pFbInList == pFuBen)
		{
			// 若为多人副本，则取消记录
			if (pFuBen->GetConfig()->nFbEnterType == 2)
			{
				int nFbId = pFuBen->GetFbId();
				if(m_vMuiltyFbMap.find(nFbId) != m_vMuiltyFbMap.end())
				{
					pFuBen->ExitAllActor();
					m_vMuiltyFbMap.erase(m_vMuiltyFbMap.find(nFbId));
					return;
				}
					
			}

			//把所有玩家退出副本先
			pFuBen->ExitAllActor();
			m_FuBenMgr->Release(pFbInList->GetHandle());//里面会执行副本的析构函数
			it.remove(pNode);

			break;
		}
	}

}

INT_PTR CFuBenManager::ReloadNpcScript(LPCTSTR szSceneName,LPCTSTR szNpcName)
{
	INT_PTR result = 0;
	INT_PTR nCount = m_vStaticFbList.count();
	if (m_vStaticFbList.count() > 0)
	{
		CFuBen* pFuBen = m_vStaticFbList[0];
		if (pFuBen)
		{
			result += pFuBen->ReloadNpcScript(szSceneName,szNpcName);
		}
	}

	CLinkedNode<CFuBen*> *pNode;
	CLinkedListIterator<CFuBen*> it(m_vAllFbList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CFuBen* pFuBen = pNode->m_Data;
		if (pFuBen)
		{
			result += pFuBen->ReloadNpcScript(szSceneName,szNpcName);
		}
	}
	//空闲的副本npc也要重置
	for (INT_PTR i = 0; i < m_FreeFbList.count(); i++)
	{
		FUBENVECTOR* pFbList = m_FreeFbList[i];
		if (pFbList)
		{
			for(int j = 0; j < pFbList->count(); j++)
			{
				CFuBen* pFuBen = (*pFbList)[j];
				if (pFuBen)
				{
					result += pFuBen->ReloadNpcScript(szSceneName,szNpcName);
				}
			}			
		}
	}

	return result;
}

void CFuBenManager::ResetFbRefresh()
{
	m_RefreshPara.setFB(NULL);
	m_RefTimer.SetNextHitTime(0);
}

void CFuBenManager::TraceRefreshPos()
{
	char * pFbName="static";
	int  nFbId= -1;
	if(CFuBenManager::m_RefreshPara.pFuben && CFuBenManager::m_RefreshPara.pFuben->GetConfig())
	{
		pFbName= "Fuben";
		nFbId = (int)CFuBenManager::m_RefreshPara.pFuben->GetFbId();
	}
	TICKCOUNT nCurrentTick = _getTickCount(); //
	TICKCOUNT nNextTick =m_RefTimer.GetNextTime();
	
	int nDis =(int)( nNextTick  - nCurrentTick);

	if(nDis <0)
	{
		nDis =0;
	}
	int nScenePos = m_RefreshPara.nScenePos;
	int nMonsterPos = m_RefreshPara.nRefreshPos;
	OutputMsg(rmNormal,"Current Fb=%s,fbid=%d,dis= %d,scenePos=%d,monsterPos=%d",pFbName,nFbId,nDis,nScenePos,nMonsterPos);
}

//调试场景的信息
void CFuBenManager::Trace()
{
	INT_PTR nCount = m_vStaticFbList.count();	
	CFuBen* pFuBen = m_vStaticFbList[0];

	
	TraceRefreshPos();
	
	for(INT_PTR i=0; i<m_vStaticFbList.count(); i++ )
	{
		CFuBen* pFuBen = m_vStaticFbList[i];
		if(pFuBen)
		{
			pFuBen->Trace();
		}
	}

	CLinkedNode<CFuBen*> *pNode;
	CLinkedListIterator<CFuBen*> it(m_vAllFbList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CFuBen* pFuBen = pNode->m_Data;
		if(pFuBen)
		{
			pFuBen->Trace();
		}
	}
}


VOID CFuBenManager::RunOne()
{
	DECLARE_TIME_PROF("CFuBenManager::RunOne");
	SF_TIME_CHECK(); //检测性能
	
	//执行每个副本的runone
	TICKCOUNT nStartTick = _getTickCount(); //自行开始
	if (!m_RefTimer.CheckAndSet(nStartTick))
		return;
    
	TICKCOUNT nLoopLimit = GetLoopTickLimit();
	
	INT_PTR nCount = m_vStaticFbList.count();
	if (nCount > 0)
	{
		CFuBen* pFuBen = m_vStaticFbList[0];
		//找到上次执行到的副本
		if (  pFuBen->RunOne() ) 
		{
			//SetLoopTickCount(_getTickCount() -nStartTick);
			return; 
		}
	}

	//暂时屏蔽，外网可能有问题
	/*
	if(m_FreeFbTimer.CheckAndSet(nStartTick))
	{
		// 定时释放多余的空闲副本
		for (INT_PTR i = 0; i < m_FreeFbList.count(); i++)
		{
			FUBENVECTOR* pFbList = m_FreeFbList[i];
			if (pFbList && pFbList->count() > 2)
			{
				for(int j = pFbList->count() - 1; j >1 ; j--)
				{
					CFuBen* pFb = (*pFbList)[j];
					if (pFb)
					{
						m_FuBenMgr->Release(pFb->GetHandle());//里面会执行副本的析构函数
						pFbList->remove(j);
					}
				}
			}
		}
	}
	*/

	// 处理动态副本的更新
	CLinkedNode<CFuBen*> *pNode;
	CLinkedListIterator<CFuBen*> it(m_vAllFbList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CFuBen* pFuBen = pNode->m_Data;
		int ret = pFuBen->RunOne();
		if (ret == 2)
		{
			OutputMsg(rmTip,_T("release fuben!fubenid=%d,fhandle=%lld,freeFbCount=%d"),pFuBen->GetFbId(),pFuBen->GetHandle(),m_FreeFbList.count());
			pFuBen->ExitAllActor();
#ifndef REUSEFB			
			m_FuBenMgr->Release(pFuBen->GetHandle());
#else
			//放到空闲列表
			bool boAdd = false;
			if (m_FreeFbList.count() > pFuBen->GetFbId())
			{
				FUBENVECTOR* pList = m_FreeFbList[pFuBen->GetFbId()];
				if (pList)
				{
					pList->add(pFuBen);
					pFuBen->Reset();
					pFuBen->SetFreeFlag(true);
					boAdd = true;
					if(m_vMuiltyFbMap.find(pFuBen->GetFbId()) != m_vMuiltyFbMap.end()) {
						m_vMuiltyFbMap.erase(m_vMuiltyFbMap.find(pFuBen->GetFbId()));
						return;
					}
						
				}
			}
			if (!boAdd)
			{
				m_FuBenMgr->Release(pFuBen->GetHandle());
			}
#endif
			it.remove(pNode);
		}
		else if (ret == 1 || _getTickCount() - nStartTick > nLoopLimit )
		{
			//SetLoopTickCount(_getTickCount() -nStartTick);
			return; 
		}
	}
	m_RefreshPara.setFB(NULL);
	//SetLoopTickCount(_getTickCount() -nStartTick);
}

bool CFuBenManager::EnterFuBen(CEntity * pEntity,CFuBen* pFuBen ,INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY,int nType, int nValue, int nParam )
{
	if(pEntity ==NULL) return false;
	if(pFuBen ==NULL) 
	{
		pFuBen = m_vStaticFbList[0];
	}
	if(pFuBen ==NULL)
	{
		OutputMsg(rmWaning,_T("Enter fb fail for no such fb,nScenceID=%d"),nScenceID);
		return false;
	}

	CFuBen* pDyFb = NULL;
	CScene* pScene = NULL;
	if(pEntity->GetType() == enActor)
	{
		pDyFb = ((CActor*)pEntity)->GetFuBen();
		pScene = ((CActor*)pEntity)->GetScene();
	}

	return pFuBen->Enter(pEntity,nScenceID,nPosX,nPosY,0,0,nType, nValue, nParam,pScene, pDyFb);

	//return pFuBen->Enter(pEntity,nScenceID,nPosX,nPosY,0,0,nType, nValue, nParam);
}


bool CFuBenManager::GetFbPtrBySceneName(LPCTSTR sName, CFuBen *& pRetFb, CScene *& pRetScene)
{
	CScene * pScene =NULL;
	pRetFb =NULL;
	pRetScene =NULL;
	for(INT_PTR i =0 ; i< m_vStaticFbList.count(); i++)
	{
		CFuBen *pFb = m_vStaticFbList[i];
		if(pFb )
		{
			pScene = pFb->GetSceneByName(sName);
			if(pScene)
			{
				pRetFb = m_vStaticFbList[i];
				pRetScene = pScene;
				return true;
			}
		}
	}
	return false;
}

bool CFuBenManager::GetFbPtrBySceneId( unsigned int nSceneID, CFuBen *& pRetFb, CScene *& pRetScene)
{
	CScene * pScene =NULL;
	pRetFb =NULL;
	pRetScene =NULL;
	for(INT_PTR i =0 ; i< m_vStaticFbList.count(); i++)
	{
		CFuBen *pFb = m_vStaticFbList[i];
		if(pFb )
		{
			pScene = pFb->GetScene(nSceneID);
			if(!pScene)
			{
				continue;
			}
			pRetFb = m_vStaticFbList[i];
			pRetScene = pScene;
			return true;
		}
	}
	return false;
}


VOID CFuBenManager::AddStaticFuBen(CFuBen* pFb)
{
	//Assert(pFb);
	m_vStaticFbList.add(pFb);
}


bool CFuBenManager::GetFbPtrBySceneId( unsigned int nFbId, int nSceneID,  CFuBen *& pRetFb, CScene *& pRetScene)
{
	CScene * pScene =NULL;
	pRetFb =NULL;
	pRetScene =NULL;
	CFuBen *pFb = NULL;
	
	// 验证FBId的有效性
	if (nFbId >= m_vStaticFbList.count())
	{
		return false;
	}

	if (m_vStaticFbList[nFbId]->GetConfig()->nFbEnterType == 2)
	{
		if(m_vMuiltyFbMap.find(nFbId) != m_vMuiltyFbMap.end())
		{
			pFb = m_vMuiltyFbMap[nFbId];
			pScene = pFb->GetScene(nSceneID);
		}
	}
	if(!pFb)
	{
		for(INT_PTR i =0 ; i< m_vStaticFbList.count(); i++)
		{
			pFb = m_vStaticFbList[i];
			if(pFb) {
				pScene = pFb->GetScene(nSceneID);
				if(pScene)
					break;
			}
		}
	}
	if(!pFb && !pScene ) return false;
	pRetFb = pFb;
	pRetScene = pScene;
	return true;
	
}

CAbstractMap* CFuBenManager::GetMapData(const char* szFileName)
{
	MAPDATA* data = NULL;
	for(int i = 0; i < m_vMapDataList.count(); i++)
	{
		data = m_vMapDataList[i];
		if (data && strcmp(szFileName,data->szFileName) == 0)
		{
			return data->pMapData;
		}
	}
	//暂时没有数据，读入
	data = new MAPDATA();
	_asncpytA(data->szFileName,szFileName);
	data->pMapData = new CAbstractMap();
	data->pMapData->LoadFromFile(szFileName);
	m_vMapDataList.add(data);
	return data->pMapData;
}

bool CFuBenManager::GetVisibleList(CEntity * pEntity,EntityVector& vVisibleList,int nLeft,int nRight,int nUp,int nDown)
{
	//根据实体的场景handle取得场景
	//const ENTITYPOS& Pos = pEntity->GetPosInfo();
	//CScene* pScene = Pos.pScene;//m_SceneMgr->GetDataPtr(Pos.nSceneHandler);
	CScene* pScene = pEntity->GetScene();
	if (pScene != NULL)
	{
		return pScene->GetVisibleList(pEntity,vVisibleList,nLeft,nRight,nUp,nDown);
	}
	return false;
}

bool CFuBenManager::GetVisibleList(CEntity * pEntity,CObserverEntityList& vVisibleList,int nLeft,int nRight,int nUp,int nDown, bool *pbActorInVisibleList)
{
	//根据实体的场景handle取得场景
	const ENTITYPOS& Pos = pEntity->GetPosInfo();	
	CScene* pScene = Pos.pScene;//m_SceneMgr->GetDataPtr(Pos.nSceneHandler);
	if (pScene != NULL)
	{
		int nPosX,nPosY;
		pEntity->GetPosition(nPosX,nPosY);
		return pScene->GetVisibleList(nPosX, nPosY,vVisibleList,nLeft,nRight,nUp,nDown,pbActorInVisibleList);
	}
	return false;
}

bool CFuBenManager::GetSeqVisibleList(CEntity * pEntity, SequenceEntityList& visibleList,int nLeft,int nRight,int nUp,int nDown)
{	
	if (!pEntity)
		return false;
	int nPosX,nPosY;
	pEntity->GetPosition(nPosX,nPosY);
	//根据实体的场景handle取得场景
	const ENTITYPOS& Pos = pEntity->GetPosInfo();
	CScene* pScene = Pos.pScene;//m_SceneMgr->GetDataPtr(Pos.nSceneHandler);
	if (pScene != NULL)
	{
		return pScene->GetSeqVisibleList(nPosX, nPosY, visibleList, nLeft,nRight,nUp,nDown);
	}
	return false;
}

COMMONFUBENCONFIG* CFuBenManager::GetConfig(int nFbid)
{
	if ((nFbid >= 0) && (nFbid < m_vStaticFbList.count()))
	{
		CFuBen *pFb = m_vStaticFbList[nFbid];
		if(pFb )
		{
			return pFb->GetConfig();
		}
	}
	return NULL;
}

int CFuBenManager::GetFbCountFromId( UINT nFbid,UINT* pList,size_t nBufSize )
{
	int ret = 0;
	CLinkedNode<CFuBen*> *pNode;
	CLinkedListIterator<CFuBen*> it(m_vAllFbList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CFuBen* pFbInList = pNode->m_Data;
		if (pFbInList->GetConfig()->nFbId == nFbid)
		{
			pList[ret++] = pFbInList->GetHandle();
		}
	}
	return ret;
}


void CFuBenManager::RefreshMonster()
{
	for (INT_PTR i = 0; i < m_vStaticFbList.count(); i++)
	{
		CFuBen* pFB = m_vStaticFbList[i];
		if(pFB) {
			pFB->RefreshMonster();
		}
	}
}

unsigned int CFuBenManager::TotalAllSceneMonsterCount()
{
	unsigned int nTotalCount = 0;
	for (INT_PTR i = 0; i < m_vStaticFbList.count(); i++)
	{
		CFuBen* pFB = m_vStaticFbList[i];
		if(pFB) {
			nTotalCount += pFB->TotalSceneMonsterCount();
		}
	}
	return nTotalCount;
}

CFuBen* CFuBenManager::AllocNewFb( int nFbId,bool boRun )
{
	// 验证FBId的有效性
	if ((nFbId < 0) || (nFbId >= m_vStaticFbList.count()))
	{
		return NULL;
	}

	CFuBen* newFb = NULL;
	CFuBen* fuben = m_vStaticFbList[nFbId];
	//从内存管理中获取一个新的CFuBen
	CFuBenHandle hHandle;
	newFb = m_FuBenMgr->Acquire(hHandle);
	fuben->Clone(*newFb,boRun);
	newFb->SetHandle( hHandle );
	if (nFbId < ArrayCount(m_anFubenCount))
		m_anFubenCount[nFbId]++;
	return newFb;
}

void CFuBenManager::Dump()
{
	OutputMsg(rmNormal, _T("-------------------Dump Fuben Info-------------------"));	
	INT_PTR nCount = ArrayCount(m_anFubenCount);
	for (INT_PTR i = 0; i < nCount; i++)
	{
		if (m_anFubenCount[i] > 0)
			OutputMsg(rmNormal, _T("Fuben[%-2d] count:%-4d"), (int)i, m_anFubenCount[i]);
	}	
	OutputMsg(rmNormal, _T("-------------------Dump Fuben Info-------------------"));
}

void CFuBenManager::DumpFbDataAlloc(wylib::stream::CBaseStream& stream)
{
	if(m_SceneMgr)
	{
		TCHAR szBuff[MAX_PATH] = {0};
		int FuBenSize = m_SceneMgr->GetUserDataSize();
		_stprintf(szBuff, _T("m_SceneMgr:%d\r\n"),FuBenSize);
		CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	

	}
	if(m_FuBenMgr)
	{
		TCHAR szBuff[MAX_PATH] = {0};
		int FuBenSize = m_FuBenMgr->GetUserDataSize();
		_stprintf(szBuff, _T("m_FuBenMgr:%d\r\n"),FuBenSize);
		CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);	
	}
}

bool CFuBenManager::IsNeedShowFubenForm(int nFbId)
{
	INT_PTR nCount = GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefualtFb.count();
	if(nCount > 0)
	{
		for(INT_PTR i=0;i<nCount;i++)
		{
			CRESTEDEFAULTFb &tmpDefaultFb = GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefualtFb[i];
			if(tmpDefaultFb.nFbId == nFbId)
			{
				return false;
			}
		}
	}

	return true;
}

void CFuBenManager::GetRandomCreatePos(int nToken,int& nScenceID,int& nPosX,int& nPosY)
{
	INT_PTR nCount = GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefualtFb.count();
	if(nCount <= 0)
	{
		return;
	}
	
	for(INT_PTR i=0;i<nCount;i++)
	{
		CRESTEDEFAULTFb &tmpDefaultFb = GetLogicServer()->GetDataProvider()->GetVocationConfig()->nDefualtFb[i];
		if(tmpDefaultFb.nToken == nToken)
		{
			if(tmpDefaultFb.nPosCount > 0)
			{
				int nRandIndex = wrandvalue() % tmpDefaultFb.nPosCount;
				if(nRandIndex >= 0 && nRandIndex < tmpDefaultFb.nPosCount && tmpDefaultFb.nCreatePos[nRandIndex].nScenceID > 0)
				{
					nScenceID = tmpDefaultFb.nCreatePos[nRandIndex].nScenceID;
					nPosX = tmpDefaultFb.nCreatePos[nRandIndex].nPosX;
					nPosY = tmpDefaultFb.nCreatePos[nRandIndex].nPosY;
				}
			}
			break;
		}
	}
}

void CFuBenManager::CreateFreeFb()
{
#ifdef REUSEFB
	m_FreeFbList.add(NULL);
	for (INT_PTR i = 1; i < m_vStaticFbList.count(); i++)
	{
		CFuBen* pFb = m_vStaticFbList[i];//fb的id必须是连续的
		if (pFb)
		{
			FUBENVECTOR* pList = new FUBENVECTOR();
			m_FreeFbList.add(pList);
			for (INT_PTR j = 0; j < 2; j++)
			{
				CFuBen* pFree = AllocNewFb(pFb->GetFbId(),false);
				pList->add(pFree);
				pFree->SetFreeFlag(true);
			}
		}
	}
#endif
}

CFuBen* CFuBenHandleMgr::GetDataPtr( CFuBenHandle handle )
{
	CFuBen* pfb = Inherited::GetDataPtr(handle);
	if (pfb && !pfb->IsFree())
	{
		return pfb;
	}
	return NULL;
}

const CFuBen* CFuBenHandleMgr::GetDataPtr( CFuBenHandle handle ) const
{
	const CFuBen* pfb = Inherited::GetDataPtr(handle);
	if (pfb && !((CFuBen*)pfb)->IsFree())
	{
		return pfb;
	}
	return NULL;
}

CFuBenHandleMgr::CFuBenHandleMgr(LPCTSTR lpszDesc):Inherited(lpszDesc)
{
}

