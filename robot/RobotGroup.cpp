/*机器人组
* 若干个机器人共有一个线程连接服务器
*/
#include "stdafx.h"
bool CRobotGroup::Start()
{
	if(m_hThread != INVALID_HANDLE_VALUE) return false;
	//if(m_clientCount <=0) return false;
	
	pthread_create(&m_hThread, NULL, ThreadRun, (void*)this);
	if ( !m_hThread )
	{
		OutputMsg( rmError, "CreateThread %s", GetLastError() );
		//return FALSE;
		return false;
	}
	OutputMsg( rmNormal, "CreateThread for %d client",m_clientCount);
	

	return true;
}


void  CRobotGroup::Stop()
{
	m_hasStart = false;
	for(int i=0; i< m_clientCount ;i ++)
	{
		m_clints[i].UnintSocketLib();
	}
	
}


bool CRobotGroup::Init(int nStartAccountIndex, int nEndAccountIndex)
{
	
	if(nEndAccountIndex<0 || nStartAccountIndex <0 || nEndAccountIndex < nStartAccountIndex ) return false;
    CBaseList<LOGINACCOUNT> & accouts = GetRobotMgr()->GetAccountList();
	int nCount =(int) accouts.count();

	//判断这些参数是否合法
	if(nEndAccountIndex >= nCount || nStartAccountIndex  >= nCount )
	{
		OutputMsg(rmError,_T("%s param start=%d,end=%d, is error"), __FUNCTION__,nStartAccountIndex,nEndAccountIndex );
		return false;
	}
	m_clientCount = nEndAccountIndex - nStartAccountIndex +1;
	if( m_clientCount > MAX_THREAD_CLIENT_COUNT)
	{
		OutputMsg(rmError,_T("%s param start=%d,end=%d, number out range"), __FUNCTION__,nStartAccountIndex,nEndAccountIndex );
		return false;
	}
	CBaseList<LOGINACCOUNT> &  accounts  = GetRobotMgr()->GetAccountList();  //获取全局的机器人管理器
	if(nEndAccountIndex >= accounts.count() )
	{
		OutputMsg(rmError,_T("%s param start=%d,end=%d,  too large"), __FUNCTION__,nStartAccountIndex,nEndAccountIndex );
		return false;
	}
	
	m_clints = new CRobotClient[m_clientCount];
	
	CRobotManager * pMgr = GetRobotMgr();

	SERVERCONFIG* pConfig= pMgr->GetServerPtr();  
	if(pConfig ==NULL)
	{
		OutputMsg( rmError, _T("No server find"));
		return false;
	}

	//遍历这些连接
	for(int i = 0; i < m_clientCount ; i++)
	{
		m_clints[i].m_account = accounts[i+nStartAccountIndex];
		m_clints[i].InitSocketLib();
		int nRandPort = pConfig->nPort +  (pConfig->nMaxPort > pConfig->nPort ? wrand(pConfig->nMaxPort - pConfig->nPort) : 0);
		m_clints[i].SetServerAddress(pConfig->ip, nRandPort);
		//m_clints[i].setBlockMode(false);
		OutputMsg(rmNormal, _T("[%s]connect server:%s,port:%d"), m_clints[i].m_account.name, pConfig->ip, nRandPort);
		m_clints[i].ConnectNextServer();
	}
	OutputMsg(rmNormal, _T("Init End"));
	m_hasStart =true;
	return true;

}

void CRobotGroup::SingleRun()
{
	for(INT_PTR i=0;i < m_clientCount ;i ++)
	{
		m_clints[i].SingleRun();
	}
}

VOID CRobotGroup::ThreadRun(void *lpThread)
{
	CRobotGroup *pRobotGroup = (CRobotGroup *)lpThread;
	CRobotManager * pMgr = GetRobotMgr();
	while(1)
	{
		if(!pRobotGroup->m_hasStart) 
			Sleep(1000);
		else
		{
			pRobotGroup->SingleRun();
			Sleep(100);
		}
	}
}
