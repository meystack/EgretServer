#include "stdafx.h"
//#include "SSManager.h"

CRobotManager::CRobotManager()
{
	m_hasStart = false;
}

CRobotManager::~CRobotManager()
{
	Shutdown();
}


bool CRobotManager::LoadConfig()
{
	if (!m_VSPDefines.LoadDefinitions(_T("./data/VSPDef.txt")))
	{
		return false;
	}

	if (!m_LanguageText.LoadFromFile(_T("./data/language/LangCode.config")))
	{
		return false;
	}

	if (!m_configLoader.LoadAllConfig())
	{
		OutputMsg(rmError, _T("Load Config Failed"));
		return false;
	}

	return true;
}


BOOL CRobotManager::Startup()
{
	if (!LoadConfig())
		return FALSE;

	m_hasStart =true;
	//配置的账户的数目
	INT_PTR nCount =m_accounts.count();
	//如果有账户限制的话就取限制的这些账户的数目，比如可能配置了最大只能登陆100个账户
	if(m_maxAccountCount >0) 
	{
		nCount = m_maxAccountCount < nCount ? m_maxAccountCount : nCount;
	}

	INT_PTR nGroupCount = nCount / MAX_THREAD_CLIENT_COUNT; //有多少个组
	if( nCount % MAX_THREAD_CLIENT_COUNT )
	{
		nGroupCount ++;
	}
	
	CRobotGroup groups;
	
	
	//先初始化这些机器人组
	OutputMsg(rmNormal, _T("Start GroupThread and Connect"));
	int nAccountIndex =0; 
	for(INT_PTR i=0; i< nGroupCount; i++)
	{
		m_robotGroups.add(groups);
		
		int nGroupMemberCount = MAX_THREAD_CLIENT_COUNT < nCount ? MAX_THREAD_CLIENT_COUNT : nCount;
		OutputMsg(rmNormal, _T("Start Group: %d"), (int)i);
		m_robotGroups[i].SetID((int)i);
		if (!m_robotGroups[i].Start())
		{
			OutputMsg(rmError, _T("Start Group Thread Failed"));
			return false;
		}

		if( false == m_robotGroups[i].Init(nAccountIndex,nAccountIndex + nGroupMemberCount -1) )
		{
			return FALSE;
		}

		nCount -= nGroupMemberCount; //剩余人数减少
		nAccountIndex += nGroupMemberCount; //位置的索引增加	
		Sleep(m_groupLoginInterval*1000);
	}

	////启动这些机器人
	//for(INT_PTR i=0; i< nGroupCount; i++)
	//{
	//	if( m_robotGroups[i].Start() ==false)
	//	{
	//		return false;
	//	}
	//}
	/*
	while(m_hasStart)
	{
		for(INT_PTR i=0; i< nGroupCount; i++)
		{
			m_robotGroups[i].SingleRun();
		}
		Sleep(10);
	}
	*/
	
	
	return TRUE;
}

VOID CRobotManager::Shutdown()
{
	if( !m_hasStart ) return ;
	for( INT_PTR i=0; i < m_robotGroups.count(); i++)
	{
		m_robotGroups[i].Stop();
	}
	m_hasStart = false;
}

SERVERCONFIG* CRobotManager::GetServerPtr()
{
	INT_PTR nCount = m_configs.count();
	
	if(nCount <=0) return NULL;
	if(nCount ==1) return &m_configs[0];
	int nRand = rand() % nCount;
	return &m_configs[nRand];
}

void CRobotManager::ShowServerAgentCount()
{
	if( !m_hasStart ) return ;
	int nTotal =0;        //总数目
	int nInSessionCount=0;//session服务器的数目
	int nInDbCount=0;      //db服务器
	int nLogicCount=0;     //逻辑服务器的数目 
	int nIdelCount =0;    //空闲状态

	int sessionStatus[CRobotClient::eStatusMax];  //session服务器各状态数目 
	int dbStatus[CRobotClient::eStatusMax];    //db服务器各状态数目 
	int logicStatus[CRobotClient::eStatusMax];  //db服务器各状态数目 
	
	memset(sessionStatus,0,sizeof(sessionStatus));
	memset(dbStatus,0,sizeof(sessionStatus));
	memset(logicStatus,0,sizeof(sessionStatus));


	for( INT_PTR i=0; i < m_robotGroups.count(); i++)
	{
		int nClientCount=  m_robotGroups[i].GetClientCount(); 
		nTotal += nClientCount;
		for(INT_PTR j=0; j< nClientCount;j ++)
		{
			CRobotClient * pClient =  m_robotGroups[i].GetClient(j);
			if(pClient ==NULL) continue;
			int nCommStatus =pClient->GetCurrentStatus(); //通信状态
			switch(pClient->GetCurrentServerType())
			{
			case CRobotClient::eServerSession:
				sessionStatus[nCommStatus] ++;
				nInSessionCount ++;
				break;
			case CRobotClient::eServerDbsever:
				dbStatus[nCommStatus] ++;
				nInDbCount ++;
				break;
			case CRobotClient::eServerLogic:
				logicStatus[nCommStatus] ++;
				nLogicCount ++;
				break;
			default:
				nIdelCount ++;
				break;
			}
		}
		
	}
	OutputMsg(rmNormal,_T("Total=%d,idle=%d,session=%d,db=%d,logic=%d"),nTotal,nIdelCount,nInSessionCount,nInDbCount,nLogicCount);
	
	OutputMsg(rmNormal,_T("session,idle=%d,reqsalt=%d,commu=%d,reqDisConnect=%d,commuDisConnect=%d"),sessionStatus[CRobotClient::eStatusIdle],
		sessionStatus[CRobotClient::eStatusEncryReq],sessionStatus[CRobotClient::eStatusCommunication],
		sessionStatus[CRobotClient::eStatusEncryReqDisConnect],sessionStatus[CRobotClient::eStatusCommuDisConnect]);

	OutputMsg(rmNormal,_T("dbserve,idle=%d,reqsalt=%d,commu=%d,reqDisConnect=%d,commuDisConnect=%d"),dbStatus[CRobotClient::eStatusIdle],
		dbStatus[CRobotClient::eStatusEncryReq],dbStatus[CRobotClient::eStatusCommunication],
		dbStatus[CRobotClient::eStatusEncryReqDisConnect],dbStatus[CRobotClient::eStatusCommuDisConnect]);

	OutputMsg(rmNormal,_T("logicsr,idle=%d,reqsalt=%d,commu=%d,reqDisConnect=%d,commuDisConnect=%d"),logicStatus[CRobotClient::eStatusIdle],
		logicStatus[CRobotClient::eStatusEncryReq],logicStatus[CRobotClient::eStatusCommunication],
		logicStatus[CRobotClient::eStatusEncryReqDisConnect],logicStatus[CRobotClient::eStatusCommuDisConnect]);
}

MapScenePoint* CRobotManager::GetRandomSceneId()
{
	Assert(m_mapPoints.count() > 0);
	unsigned long nIdx = wrand((unsigned long)m_mapPoints.count());
	
	return &m_mapPoints[nIdx];
}