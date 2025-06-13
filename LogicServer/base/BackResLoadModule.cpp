#include "StdAfx.h"
#include "Container.hpp"


CBackResLoadModule::CBackResLoadModule()
{	

	m_bThreadStarted	= FALSE;
	m_bThreadRuning     = FALSE;
#ifdef WIN32		
	m_hStartEvent		= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStopEvent		= ::CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	memset( &m_hStartEvent, 0, sizeof(m_hStartEvent) ) ;
	memset( &m_hStopEvent, 0, sizeof(m_hStopEvent) ) ;
	sem_init( &m_hStartEvent, 0, 0 ) ;
	sem_init( &m_hStopEvent, 0, 0 ) ;
#endif
	StartWorkThread();
}

CBackResLoadModule::~CBackResLoadModule()
{
	StopWorkThread();
#ifdef WIN32	
	CloseHandle(m_hStartEvent);
	CloseHandle(m_hStopEvent);
#else
	sem_destroy(&m_hStartEvent);
	sem_destroy(&m_hStopEvent);
#endif
}

void CBackResLoadModule::Start()
{

	if (FALSE == InterlockedCompareExchange(&m_bThreadStarted, TRUE, FALSE))
	{
		OutputMsg(rmTip,"CBackResLoadModule start");
		#ifdef WIN32
		ResetEvent(m_hStopEvent);
		#endif
		//已经启动了就不需要等待了
		if( TRUE == InterlockedCompareExchange(&m_bThreadRuning, TRUE, TRUE))
		{

		}
		else
		{
			OutputMsg(rmTip,"CBackResLoadModule start: waiting for start event");
#ifdef WIN32
			WaitForSingleObject(m_hStartEvent, 10000);
#else
			struct timespec tm ;
			clock_gettime(CLOCK_REALTIME, &tm);
			tm.tv_sec += 10;
			sem_timedwait(&m_hStartEvent, &tm);
#endif
		}
		OutputMsg(rmTip,"CBackResLoadModule start finish ");
	}

}

void CBackResLoadModule::Stop()
{

	if (TRUE == InterlockedCompareExchange(&m_bThreadStarted, FALSE, TRUE))
	{
#ifdef WIN32
		ResetEvent(m_hStartEvent);
#endif
		StopWorkThread();
		//已经停止了就不需要等待了
		if( FALSE == InterlockedCompareExchange(&m_bThreadRuning, FALSE, FALSE))
		{

		}
		else
		{
#ifdef WIN32
			WaitForSingleObject(m_hStopEvent, 5000);
#else
			struct timespec tm ;
			clock_gettime(CLOCK_REALTIME, &tm);
			tm.tv_sec += 5;
			sem_timedwait(&m_hStopEvent, &tm);
#endif
		}

	}
}

void CBackResLoadModule::OnRountine()
{

	while (!terminated())
	{	
		if (TRUE == m_bThreadStarted)
		{
#ifdef WIN32
			SetEvent(m_hStartEvent);
#else
			sem_post(&m_hStartEvent);
#endif
			InterlockedCompareExchange(&m_bThreadRuning, TRUE, FALSE);
			while (TRUE == m_bThreadStarted)
			{
				CResLoadTask rldata = GetNextResLoadData();
				bool bIdle = false;
				switch(rldata.m_resLoadType)
				{
				case enResLoad_Item:
					LoadItemResImpl();
					break;
				case enResLoad_Monster:
					LoadMonsterResImpl();
					break;
				default:
					bIdle = true;
					break;
				}

				Sleep(bIdle ? IDLE_SLEEP_INTERVAL : TASK_END_SLEEP_INTERVAL);
			}
#ifdef WIN32
			SetEvent(m_hStopEvent);
#else
			sem_post(&m_hStopEvent);
#endif
			InterlockedCompareExchange(&m_bThreadRuning, FALSE, TRUE);
		}
		else
		{
			Sleep(IDLE_SLEEP_INTERVAL);			
		}
	}

	OutputMsg(rmNormal, _T("back load Thread exist"));
}
bool CBackResLoadModule::LoadMonsterResImpl()
{
	bool bResult = false;
	const CLogicDataProvider::DataProviderLoadTerm *pMonster = GetLogicServer()->GetDataProvider()->GetDataProviderLoadConfig(enCDType_Monster);
	if (!pMonster) {
		OutputMsg(rmError, _T("BackLoad Monster Config failed, can't get config file path"));
		return false;
	}

	bResult = GetLogicServer()->GetDataProvider()->GetMonsterConfig().LoadMonsters(pMonster->sFilePath, true);
	m_resLock.Lock();
	if (bResult)
	{		
		CResLoadTask rld;
		rld.m_resLoadType = enResLoad_Monster;
		m_finishedList.add(rld);		
	}
	CLinkedNode<CResLoadTask> *pNode = m_resLoadList.first();
	m_resLoadList.remove(pNode);
	m_resLock.Unlock();
	return bResult;
}
bool CBackResLoadModule::LoadItemResImpl()
{
	bool bResult = false;
	const CLogicDataProvider::DataProviderLoadTerm *pItem = GetLogicServer()->GetDataProvider()->GetDataProviderLoadConfig(enCDType_Item);
	if (!pItem) {
		OutputMsg(rmError, _T("BackLoad Item Config failed, can't get config file path"));
		return false;
	}
		
	bResult = GetLogicServer()->GetDataProvider()->GetStdItemProvider().LoadStdItems(pItem->sFilePath, true);
	m_resLock.Lock();
	if (bResult)
	{		
		CResLoadTask rld;
		rld.m_resLoadType = enResLoad_Item;
		m_finishedList.add(rld);		
	}
	CLinkedNode<CResLoadTask> *pNode = m_resLoadList.first();
	m_resLoadList.remove(pNode);
	m_resLock.Unlock();


	return bResult;
}

void CBackResLoadModule::RunOne(TICKCOUNT nCurrentTick)
{
	if (m_UpdateFinishListTimer.CheckAndSet(nCurrentTick))
	{		
		if (m_finishedList.count() > 0)
		{
			m_finishedResLock.Lock();
			m_finished2List.addArray(m_finishedList, m_finishedList.count());
			m_finishedList.clear();
			m_finishedResLock.Unlock();
		}		

		size_t itemCount = m_finished2List.count();
		if (itemCount > 0)
		{
			for (INT_PTR i = 0; i < (INT_PTR)itemCount; i++)
			{
				const CResLoadTask &task = m_finished2List[i];
				switch (task.m_resLoadType)
				{
				case enResLoad_Item:
					GetLogicServer()->GetDataProvider()->GetStdItemProvider().UpdateItemConfig();
					break;
				case enResLoad_Monster:
					GetLogicServer()->GetDataProvider()->GetMonsterConfig().UpdateMonsterConfig();
					GetLogicServer()->GetDataProvider()->LoadRunTimeData();//boss成长
					GetGlobalLogicEngine()->GetMonFuncNpc()->LoadScript(CLogicEngine::szMonsterFuncScriptFile,true);//读完配置之后，也要重新加载怪物脚本
					break;
				default:
					break;
				}
			}

			m_finished2List.clear();
		}
	}
}


CResLoadTask CBackResLoadModule::GetNextResLoadData()
{
	CResLoadTask rldata;
	m_resLock.Lock();
	if (m_resLoadList.count() > 0)
	{
		CLinkedNode<CResLoadTask> *pNode = m_resLoadList.first();
		rldata = *pNode;
		//m_resLoadList.remove(pNode);
	}
	m_resLock.Unlock();
	return rldata;
}

void CBackResLoadModule::AddFinishedTask(const CResLoadTask &task)
{
	m_finishedResLock.Lock();
	m_finishedList.add(task);
	m_finishedResLock.Unlock();
}

void CBackResLoadModule::LoadMonsterConfig()
{
	GetLogicServer()->GetDataProvider()->SaveRunTimeData();//boss成长,先保存，重读配置再读取
	LoadConfig(enResLoad_Monster);
}
void CBackResLoadModule::LoadConfig(BackResLoadType resLoadType)
{
	CResLoadTask rldata;
	rldata.m_resLoadType = resLoadType;

	m_resLock.Lock();
	// 在等待处理列表中查找是否已经有此类任务
	bool bHasSameTask = false;
	CLinkedListIterator<CResLoadTask> iter(m_resLoadList);
	CLinkedNode<CResLoadTask> *pNode;
	for (pNode = iter.first(); pNode; pNode = iter.next())
	{
		if (pNode->m_Data.m_resLoadType == rldata.m_resLoadType)
		{
			bHasSameTask = true;
			break;
		}
	}

	// 在完成列表中查找是否有此类任务
	if (!bHasSameTask)
	{
		m_finished2List.addArray(m_finishedList, m_finishedList.count());
		m_finishedList.clear();
		for (INT_PTR i = 0; i < m_finished2List.count(); i++)
		{
			if (m_finished2List[i].m_resLoadType == rldata.m_resLoadType)
			{
				bHasSameTask = true;
				break;
			}
		}
	}

	if (!bHasSameTask)
		m_resLoadList.linkAfter(rldata);

	m_resLock.Unlock();
}
void CBackResLoadModule::LoadItemConfig()
{
	LoadConfig(enResLoad_Item);
}