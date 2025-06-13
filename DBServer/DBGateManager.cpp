
#include "StdAfx.h"
CDBGateManager::CDBGateManager(CDBServer *lpDBEngine)
:Inherited()
{
	m_dwReconnectSQLTick	= 0;
	m_pDBServer				= lpDBEngine;
	m_bLoadZyJobData = false;
	m_SQLConnection.SetMultiThread(TRUE);	
	m_nNextZyJobSaveTime = _getTickCount();
	m_bZyDataDirty = false;
	m_bJobDataDirty = false;
}

CDBGateManager::~CDBGateManager()
{
}

BOOL CDBGateManager::ConnectSQL()
{
	if ( !m_SQLConnection.Connected() )
	{
		if ( _getTickCount() >= m_dwReconnectSQLTick )
		{
			m_pDBServer->SetupSQLConnection(&m_SQLConnection);
			if ( m_SQLConnection.Connect() )
			{
				mysql_options(m_SQLConnection.GetMySql(), MYSQL_SET_CHARSET_NAME, "utf8");
				//if (m_SQLConnection.Exec("charset utf8"))
				//{
				//	OutputMsg( rmError, _T("设置utf8字符编码出错！！！"));
				//}
				//else
				//{
				//	m_SQLConnection.ResetQuery();
				//}
				OnDBConnected();
				return TRUE;
			}
			//如果连接SQL失败则将在5秒后重试
			m_dwReconnectSQLTick = _getTickCount() + 5 * 1000;
			return FALSE;
		}
	}
	return TRUE;
}

VOID CDBGateManager::AllocCharIdResult(const int nSessionId, const INT_PTR nResult, const int nCharId)
{
	INT_PTR i;
	for ( i=0; i<ArrayCount(m_Gates); ++i )
	{
		if ( m_Gates[i].NameServerAllocCharIdResult(nSessionId, nResult, nCharId) )
		{
			break;
		}
	}
}

VOID CDBGateManager::LogicResponseSessionData(const unsigned int nSessionId,unsigned int nActorId)
{
	for ( INT_PTR i=0; i< ArrayCount(m_Gates); ++i )
	{
		m_Gates[i].OnLogicRsponseSessionData(nSessionId,nActorId);
	}
}
VOID CDBGateManager::Initialize()
{
	INT_PTR i;
	for ( i=0; i<ArrayCount(m_Gates); ++i )
	{
		m_Gates[i].m_pSQLConnection = &m_SQLConnection;
		m_Gates[i].m_pDBServer = m_pDBServer;		
		m_Gates[i].SetGateManager(this);
		// 需保证在调用InitDBReqHandler之前， CDBGate对象已经设置好SQlConnection以及GateManager
		//m_Gates[i].InitDBReqHandler();
		//将网关添加到列表
		m_GateList[i] = &m_Gates[i];
	}
}

VOID CDBGateManager::Uninitialize()
{
	INT_PTR i;
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		//将网关从列表移除
		m_GateList[i] = NULL;
	}
}

VOID CDBGateManager::Stop()
{
	OutputMsg(rmNormal, _T("Save Zy Data..."));
	SaveZyData();
	OutputMsg(rmNormal, _T("Save Job Data..."));
	SaveJobData();
	OutputMsg(rmNormal, _T("Stop GateManager..."));
	Inherited::Stop();
	OutputMsg(rmNormal, _T("Stop GateManager End..."));
}

VOID CDBGateManager::OnResultGateUserExists(int nSessionId, const BOOL boOnline)
{
	m_pDBServer->PostQueryUserExistsBySessionIdResult(nSessionId, boOnline);
}

VOID CDBGateManager::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4)
{
	switch (uMsg)
	{
	case GTIM_ALLOC_CHARID_RESULT://申请角色ID返回（此消息由NSClient发送）(Param1=会话ID,Param2=操作结果,Param3=角色ID（仅在操作成功时有效）)
		AllocCharIdResult((int)uParam1, (int)uParam2, (int)uParam3);
		break;
	case LOGIC_RESPONSE_DB_SESSION_DATA:
		LogicResponseSessionData((unsigned int)uParam1, (unsigned int)uParam2);
		break;
	default:
		Inherited::DispatchInternalMessage(uMsg, uParam1, uParam2, uParam3,uParam4);
		break;
	}
}

VOID CDBGateManager::OnRun()
{
	if ( Started() )
	{
		ConnectSQL();
		TICKCOUNT curTime = _getTickCount();
		if (m_nNextZyJobSaveTime <= curTime)
		{
			if (m_bLoadZyJobData && m_SQLConnection.Connected())
			{
				SaveZyData();
				SaveJobData();				
			}
			m_nNextZyJobSaveTime = curTime +  1000 * 300; // 300s 存一次盘
		}
	}
}

VOID CDBGateManager::OnDBConnected()
{
	if (!m_bLoadZyJobData)
	{
		OutputMsg(rmNormal, _T("DBConnected, Load Zy and Job Data"));
		LoadZyInitData();
		LoadJobInitData();
		m_bLoadZyJobData = true;		
	}	
	else
	{
		OutputMsg(rmNormal, _T("DBReConnected, Save Zy and Job Data"));
		SaveZyData();
		SaveJobData();
	}
}

VOID CDBGateManager::LoadZyInitData()
{
	m_zyLock.Lock();
	int nError = m_SQLConnection.Query(szSQLSP_LoadZyDistributeData);
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection.CurrentRow();
		int nCount		= m_SQLConnection.GetRowCount();
		INT_PTR	nLastSrvIdx = -1;  // Invalid srv idx
		int nZy, nUserCount, nSrvIdx;
		PlayerZYDistribute zyDist;
		zyDist.m_nServerIdx = nLastSrvIdx;
		while (pRow)
		{
			sscanf(pRow[0], "%d",		&nZy);
			sscanf(pRow[1], "%d",		&nUserCount);
			sscanf(pRow[2], "%d",		&nSrvIdx);
			if (nZy > zyNone && nZy < zyMax && nUserCount > 0)	// 只保存nUserCount大于0的阵营
			{
				if (nLastSrvIdx != nSrvIdx)
				{
					if (zyDist.m_nServerIdx != -1)
						m_vecZyDist.add(zyDist);
					
					zyDist.Reset();
					zyDist.m_nServerIdx = nSrvIdx;
					nLastSrvIdx = nSrvIdx;
				}
									
				zyDist.m_anPlayerCount[nZy] = nUserCount;
			}

			pRow = m_SQLConnection.NextRow();
		}
		// add last server zy distribute info
		if (zyDist.m_nServerIdx != -1)
			m_vecZyDist.add(zyDist);

		m_SQLConnection.ResetQuery();
		m_bLoadZyJobData = true;
	}
	m_zyLock.Unlock();
}

VOID CDBGateManager::SaveZyData()
{
	m_zyLock.Lock();
	if (m_bLoadZyJobData && m_bZyDataDirty)
	{		
		if (m_SQLConnection.Connected())
		{			
			char szSql[1024];
			for (INT_PTR i = 0; i < m_vecZyDist.count(); i++)
			{
				// clean first
				PlayerZYDistribute& dist = m_vecZyDist[i];
				sprintf(szSql, szSQLSP_CleanZyDistributeData, (int)dist.m_nServerIdx);
				int nError = m_SQLConnection.RealExec(szSql, strlen(szSql));
				if (nError == reSucc)
				{					
					bool bFirstItem = true;
					sprintf(szSql, "%s", szSQLSP_AddZyDistributeData);										
					for (INT_PTR j = zyNone+1; j < zyMax; j++)
					{
						char szTemp[128];
						if (dist.m_anPlayerCount[j] > 0)
						{
							if (!bFirstItem)
							{
								strcat(szSql, ",");
							}
							if (bFirstItem)
								bFirstItem = false;

							sprintf(szTemp, "(%d,%d,%d)", (int)j, (int)dist.m_anPlayerCount[j], (int)dist.m_nServerIdx);
							strcat(szSql, szTemp);
						}						
					}
					m_SQLConnection.ResetQuery();

					nError = m_SQLConnection.RealExec(szSql, strlen(szSql));
					if (nError != reSucc)
					{
						OutputMsg(rmError, _T("%s add zy count record failed(srvidx=%d"), __FUNCTION__, (int)dist.m_nServerIdx);
					}
					else
					{
						m_SQLConnection.ResetQuery();
					}
				}
				
			}
			m_bZyDataDirty = false;
		}		
	}		
	m_zyLock.Unlock();
}

tagZhenying CDBGateManager::GetPriorityZY(int nSrvIdx)
{
	m_zyLock.Lock();	
	tagZhenying priorZy = zyNone;
	for (INT_PTR i = 0; i < m_vecZyDist.count(); i++)
	{
		PlayerZYDistribute& dist = m_vecZyDist[i];
		if (nSrvIdx == dist.m_nServerIdx)
		{
			// find zy in which less user
			INT_PTR nCount = 0x7FFFFFFF;
			for (INT_PTR j = zyNone+1; j < zyMax; j++)
			{
				if (dist.m_anPlayerCount[j] < nCount /*&& dist.m_anPlayerCount[j] >= 0*/)
				{
					priorZy = (tagZhenying)j;
					nCount = dist.m_anPlayerCount[j];
				}
			}
		}
	}

	if (priorZy == zyNone)
		priorZy = zyWuJi;
	m_zyLock.Unlock();
	return priorZy;
}

VOID CDBGateManager::LoadJobInitData()
{
	m_jobLock.Lock();
	int nError = m_SQLConnection.Query(szSQLSP_LoadJobDistributeData);
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection.CurrentRow();
		int nCount		= m_SQLConnection.GetRowCount();
		INT_PTR	nLastSrvIdx = -1;  // Invalid srv idx
		int nJob, nUserCount, nSrvIdx;
		PlayerJobDistribute jobDist;
		jobDist.m_nServerIdx = nLastSrvIdx;
		while (pRow)
		{
			sscanf(pRow[0], "%d",		&nJob);
			sscanf(pRow[1], "%d",		&nUserCount);
			sscanf(pRow[2], "%d",		&nSrvIdx);
			if (nJob > enVocNone && nJob < enMaxVocCount && nUserCount > 0) // 只保存nUserCount大于0的职业
			{
				if (nLastSrvIdx != nSrvIdx)
				{
					if (jobDist.m_nServerIdx != -1)
						m_vecJobDist.add(jobDist);

					jobDist.Reset();
					jobDist.m_nServerIdx = nSrvIdx;
					nLastSrvIdx = nSrvIdx;
				}

				jobDist.m_anPlayerCount[nJob] = nUserCount;
			}

			pRow = m_SQLConnection.NextRow();
		}
		// add last server zy distribute info
		if (jobDist.m_nServerIdx != -1)
			m_vecJobDist.add(jobDist);

		m_SQLConnection.ResetQuery();
		m_bLoadZyJobData = true;
	}
	m_jobLock.Unlock();
}

VOID CDBGateManager::SaveJobData()
{
	m_jobLock.Lock();
	if (m_bLoadZyJobData && m_bJobDataDirty)
	{		
		if (m_SQLConnection.Connected())
		{
			char szSql[1024];
			for (INT_PTR i = 0; i < m_vecJobDist.count(); i++)
			{
				// clean first
				PlayerJobDistribute& dist = m_vecJobDist[i];
				sprintf(szSql, szSQLSP_CleanJobDistributeData, (int)dist.m_nServerIdx);
				int nError = m_SQLConnection.RealExec(szSql, strlen(szSql));
				if (nError == reSucc)
				{
					bool bFirstItem = true;
					sprintf(szSql, "%s", szSQLSP_AddJobDistributeData);										
					for (INT_PTR j = enVocNone+1; j < enMaxVocCount; j++)
					{
						char szTemp[128];
						if (dist.m_anPlayerCount[j] > 0)
						{
							if (!bFirstItem) 
							{
								strcat(szSql, ",");
							}
							if (bFirstItem)
								bFirstItem = false;

							sprintf(szTemp, "(%d,%d,%d)", (int)j, (int)dist.m_anPlayerCount[j], (int)dist.m_nServerIdx);
							strcat(szSql, szTemp);
						}						
					}
					m_SQLConnection.ResetQuery();

					nError = m_SQLConnection.RealExec(szSql, strlen(szSql));
					if (nError != reSucc)
					{
						OutputMsg(rmError, _T("%s add job count record failed(srvidx=%d"), __FUNCTION__, (int)dist.m_nServerIdx);
					}
					else
					{
						m_SQLConnection.ResetQuery();
					}
				}

			}
			m_bJobDataDirty = false;
		}		
	}		
	m_jobLock.Unlock();
}

int CDBGateManager::GetPriorityJob(int nSrvIdx)
{
	m_jobLock.Lock();	
	int priorJob = enVocNone;
	for (INT_PTR i = 0; i < m_vecJobDist.count(); i++)
	{
		PlayerJobDistribute& dist = m_vecJobDist[i];
		if (nSrvIdx == dist.m_nServerIdx)
		{
			// find jobs in which less user
			INT_PTR nCount = 0x7FFFFFFF;
			static INT_PTR sValidVoc[] = {1, 2, 3};
			static const INT_PTR snValidVocCount = ArrayCount(sValidVoc);
			//for (INT_PTR j = enVocNone+1; j < enMaxVocCount; j++)
			for (INT_PTR j = 0; j < snValidVocCount; j++)
			{				
				INT_PTR nVoc = sValidVoc[j];
				if (dist.m_anPlayerCount[nVoc] < nCount/* && dist.m_anPlayerCount[j] >= 0*/)
				{
					priorJob = (tagActorVocation)nVoc;
					nCount = dist.m_anPlayerCount[nVoc];
				}
			}
		}
	}

	if (priorJob == enVocNone)
		priorJob = 1;
	m_jobLock.Unlock();
	return (int)priorJob;
}

int CDBGateManager::GetOptionalZy(int nSrvIdx)
{
	m_zyLock.Lock();
	int nOptMask = 7;		// all zy are optional
	for (INT_PTR i = 0; i < m_vecZyDist.count(); i++)
	{
		PlayerZYDistribute& dist = m_vecZyDist[i];
		if (nSrvIdx == dist.m_nServerIdx)
		{
			INT_PTR nTotalCount = 0;
			for (INT_PTR i = zyNone+1; i < zyMax; i++)
				nTotalCount += dist.m_anPlayerCount[i];

			if (nTotalCount > 0)
			{
				float nRate;
				for (INT_PTR j = zyNone+1; j < zyMax; j++)
				{
					nRate = (dist.m_anPlayerCount[j] / (float)nTotalCount);
					if (nRate > 0.35)
					{
						nOptMask &= ~(1 << (j-1));
					}
				}
			}
		}
	}
	m_zyLock.Unlock();
	return nOptMask;
}

void CDBGateManager::OnNewPlayerCreated(int nSrvIdx, tagZhenying zy, tagActorVocation voc)
{
	//OutputMsg(rmNormal, _T("create new player, zy=%d, voc=%d"), (int)zy, (int)voc);
	if (zy > zyNone && zy < zyMax)
	{
		m_zyLock.Lock();
		INT_PTR i = 0;
		for (; i < m_vecZyDist.count(); i++)
		{
			if (m_vecZyDist[i].m_nServerIdx == nSrvIdx)
			{
				m_vecZyDist[i].m_anPlayerCount[zy]++;
				break;
			}
		}
		// add new node
		if (i == m_vecZyDist.count())
		{
			PlayerZYDistribute dist;
			dist.m_nServerIdx = nSrvIdx;
			dist.m_anPlayerCount[zy] = 1;
			m_vecZyDist.add(dist);
		}
		m_bZyDataDirty = true;
		m_zyLock.Unlock();
	}

	if (voc > enVocNone && zy < enMaxVocCount)
	{
		m_jobLock.Lock();
		INT_PTR i = 0;
		for (; i < m_vecJobDist.count(); i++)
		{
			if (m_vecJobDist[i].m_nServerIdx == nSrvIdx)
			{
				m_vecJobDist[i].m_anPlayerCount[voc]++;
				break;
			}
		}
		// add new node
		if (i == m_vecJobDist.count())
		{
			PlayerJobDistribute dist;
			dist.m_nServerIdx = nSrvIdx;
			dist.m_anPlayerCount[voc] = 1;
			m_vecJobDist.add(dist);
		}
		m_bJobDataDirty = true;
		m_jobLock.Unlock();
	}
}