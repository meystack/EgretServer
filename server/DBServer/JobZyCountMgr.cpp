
#include "StdAfx.h"
#include "JobZyCountMgr.h"


CJobZyCountMgr::CJobZyCountMgr()
{
	m_bLoadJobData = false;
	m_bLoadZyData =false;	
	//m_nNextZyJobSaveTime = _getTickCount();
	m_bZyDataDirty = false;
	m_bJobDataDirty = false;

	m_bLoadNameData = false;
	m_SQLConnection    =NULL; //设置这个链接
	m_bLoadGuildNameData =false;  //是否装载过行会的数据

	
}
//装载行会名字
void CJobZyCountMgr::LoadGuildNameInitData()
{
	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return;
	if(m_bLoadGuildNameData) return; //已经装载过了
	m_guildNameLock.Lock();
	int nError = m_SQLConnection->Query("select guildname,serverindex,guildid from guildlist where status = 1");
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection->CurrentRow();
		int nCount		= m_SQLConnection->GetRowCount();

		while (pRow)
		{
			PlayerNameDistribute sNameList;
			if(pRow[0])
			{
				memcpy(&sNameList.sName,pRow[0],sizeof(sNameList.sName));
			}
			sscanf(pRow[1], "%d",&sNameList.nServerIdx);
			sscanf(pRow[2], "%d",&sNameList.guildid); 
			if(sNameList.nServerIdx > 0)
			{
				m_guildNameDist.add(sNameList);
			}
			pRow = m_SQLConnection->NextRow();
		}

		m_SQLConnection->ResetQuery();
		m_bLoadGuildNameData = true;
	}

	m_guildNameLock.Unlock();
}

void CJobZyCountMgr::LoadActorNameInitData()
{
	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return;
	if(m_bLoadNameData) return; //已经装载过了
	m_nameLock.Lock();

	int nError = m_SQLConnection->Query(szSQLSP_LoadActorNameInitData);
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection->CurrentRow();
		int nCount		= m_SQLConnection->GetRowCount();

		while (pRow)
		{
			PlayerNameDistribute sNameList;
			if(pRow[0])
			{
				memcpy(&sNameList.sName,pRow[0],sizeof(sNameList.sName));
			}
			sscanf(pRow[1], "%d",&sNameList.nServerIdx);

			if(sNameList.nServerIdx > 0)
			{
				m_vecNameDist.add(sNameList);
			}
			pRow = m_SQLConnection->NextRow();
		}

		m_SQLConnection->ResetQuery();
		m_bLoadNameData = true;
	}

	m_nameLock.Unlock();
}

VOID CJobZyCountMgr::LoadJobInitData()
{
	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return;
	if(m_bLoadJobData) return; //已经装载过了
	
	m_jobLock.Lock();
	int nError = m_SQLConnection->Query(szSQLSP_LoadJobDistributeData);
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection->CurrentRow();
		int nCount		= m_SQLConnection->GetRowCount();
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

			pRow = m_SQLConnection->NextRow();
		}
		if (jobDist.m_nServerIdx != -1)
			m_vecJobDist.add(jobDist);

		m_SQLConnection->ResetQuery();
		m_bLoadJobData = true;
	}
	m_jobLock.Unlock();
}



VOID CJobZyCountMgr::SaveJobData()
{
	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return;
	m_jobLock.Lock();
	if (m_bLoadJobData && m_bJobDataDirty)
	{		
		if (m_SQLConnection->Connected())
		{
			char szSql[1024];
			for (INT_PTR i = 0; i < m_vecJobDist.count(); i++)
			{
				// clean first
				PlayerJobDistribute& dist = m_vecJobDist[i];
				sprintf(szSql, szSQLSP_CleanJobDistributeData, (int)dist.m_nServerIdx);
				int nError = m_SQLConnection->RealExec(szSql, strlen(szSql));
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
					m_SQLConnection->ResetQuery();

					nError = m_SQLConnection->RealExec(szSql, strlen(szSql));
					if (nError != reSucc)
					{
						OutputMsg(rmError, _T("%s add job count record failed(srvidx=%d"), __FUNCTION__, (int)dist.m_nServerIdx);
					}
					else
					{
						m_SQLConnection->ResetQuery();
					}
				}

			}
			m_bJobDataDirty = false;
		}		
	}		
	m_jobLock.Unlock();
}


int CJobZyCountMgr::GetPriorityJob(int nSrvIdx)
{
	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return 1;
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


void CJobZyCountMgr::OnNewPlayerCreated(int nSrvIdx, tagZhenying zy, tagActorVocation voc,char sName[32])
{
	//OutputMsg(rmNormal, _T("create new player, zy=%d, voc=%d"), (int)zy, (int)voc);
	/*
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
	*/

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

	if(sName && nSrvIdx > 0)
	{
		PlayerNameDistribute nameList;
		nameList.nServerIdx = nSrvIdx;
		_asncpytA(nameList.sName,sName);

		m_vecNameDist.add(nameList);
	}
}


//当新行会创建了以后
void CJobZyCountMgr::OnNewGuildNameCreate(int nSrvIdx, char* sName, int nGuildId)
{
	if(sName ==NULL) return ;
	if(IsGuildNameExist(sName,nSrvIdx))
	{
		OutputMsg(rmWaning,"Guild is exist =%s",sName);
		return ;
	}
	//是否存在
	m_guildNameLock.Lock();
	PlayerNameDistribute nameList;
	nameList.nServerIdx = nSrvIdx;
	nameList.guildid = nGuildId; 
	_asncpytA(nameList.sName,sName);
	m_guildNameDist.add(nameList);
	
	m_guildNameLock.Unlock();
	OutputMsg(rmTip,"create guild %s",sName);
}

bool CJobZyCountMgr::RemoveGuildById(int Guild)
{ 
	for(int i = 0; i < m_guildNameDist.count(); i++)
	{
		PlayerNameDistribute* nameDist = &m_guildNameDist[i];
		if((  Guild == nameDist->guildid))
		{
			m_guildNameDist.remove(i);
			return true;
		}
	}

	return false;
}
//行会名字是否存在
bool CJobZyCountMgr::IsGuildNameExist(char* sName,int nServerindex)
{
	if(sName )
	{
		for(int i=0;i<m_guildNameDist.count();i++)
		{
			PlayerNameDistribute* nameDist = &m_guildNameDist[i];
			if((nServerindex <0 ||  nServerindex == nameDist->nServerIdx) && strcmp(sName,nameDist->sName) == 0 )
			{
				return true;
			}
		}
	}
	else
	{
		return false;
	}

	return false;
}

VOID CJobZyCountMgr::LoadZyInitData()
{
	if(m_bLoadZyData ) return;

	if(m_SQLConnection ==NULL || m_SQLConnection->Connected() ==false) return ;
	m_zyLock.Lock();
	int nError = m_SQLConnection->Query(szSQLSP_LoadZyDistributeData);
	if (reSucc == nError)
	{
		MYSQL_ROW pRow	= m_SQLConnection->CurrentRow();
		int nCount		= m_SQLConnection->GetRowCount();
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

			pRow = m_SQLConnection->NextRow();
		}
		// add last server zy distribute info
		if (zyDist.m_nServerIdx != -1)
			m_vecZyDist.add(zyDist);

		m_SQLConnection->ResetQuery();
		m_bLoadZyData = true;
	}
	m_zyLock.Unlock();
}




VOID CJobZyCountMgr::SaveZyData()
{
	if(m_SQLConnection->Connected() ==false) return ;
	m_zyLock.Lock();
	if (m_bLoadZyData && m_bZyDataDirty)
	{		
		if (m_SQLConnection->Connected())
		{			
			char szSql[1024];
			for (INT_PTR i = 0; i < m_vecZyDist.count(); i++)
			{
				// clean first
				PlayerZYDistribute& dist = m_vecZyDist[i];
				sprintf(szSql, szSQLSP_CleanZyDistributeData, (int)dist.m_nServerIdx);
				int nError = m_SQLConnection->RealExec(szSql, strlen(szSql));
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
					m_SQLConnection->ResetQuery();

					nError = m_SQLConnection->RealExec(szSql, strlen(szSql));
					if (nError != reSucc)
					{
						OutputMsg(rmError, _T("%s add zy count record failed(srvidx=%d"), __FUNCTION__, (int)dist.m_nServerIdx);
					}
					else
					{
						m_SQLConnection->ResetQuery();
					}
				}

			}
			m_bZyDataDirty = false;
		}		
	}		
	m_zyLock.Unlock();
}



tagZhenying CJobZyCountMgr::GetPriorityZY(int nSrvIdx)
{
	m_zyLock.Lock();	
	tagZhenying priorZy = zyNone;
	for (INT_PTR i = 0; i < m_vecZyDist.count(); i++)
	{
		PlayerZYDistribute& dist = m_vecZyDist[i];
		if (nSrvIdx == dist.m_nServerIdx)
		{
			
			INT_PTR nCount = 0x7FFFFFFF;
			for (INT_PTR j = zyNone+1; j < zyMax; j++)
			{
				if (dist.m_anPlayerCount[j] < nCount)
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


int CJobZyCountMgr::GetOptionalZy(int nSrvIdx)
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

bool CJobZyCountMgr::IsActorNameExist(char* sName,int nServerindex)
{

	if(sName )
	{
		for(int i=0;i<m_vecNameDist.count();i++)
		{
			PlayerNameDistribute* nameDist = &m_vecNameDist[i];
			if((nServerindex <0 ||  nServerindex == nameDist->nServerIdx) && strcmp(sName,nameDist->sName) == 0 )
			{
				return true;
			}
		}
	}
	else
	{
		return false;
	}

	return false;
}

void CJobZyCountMgr::ChangeNameToList(char* sName,char* srcName,int nServerIndex)
{
	if(sName && srcName && nServerIndex > 0)
	{
		for(int i=0;i<m_vecNameDist.count();i++)
		{
			PlayerNameDistribute* nameDist = &m_vecNameDist[i];
			if( strcmp(srcName,nameDist->sName) == 0 && nServerIndex == nameDist->nServerIdx)
			{
				_asncpytA(nameDist->sName,sName);
				break;
			}
		}
	}
	else if(!srcName && sName)
	{
		PlayerNameDistribute nameDist;
		nameDist.nServerIdx = nServerIndex;
		_asncpytA(nameDist.sName,sName);
		m_vecNameDist.add(nameDist);
	}
}

/*
int CJobZyCountMgr::GetStrLenUtf8(LPCTSTR sName)
{
	LPCSTR sptr;
	int nLen = 0;
	unsigned int ch;
	for ( sptr = sName; *sptr; ++sptr )
	{
		ch = (unsigned int)*sptr;
		if ( ch >= 0x7F )
		{
			sptr += 2;//UTF-8,留意++sptr
		}
		nLen++;
	}
	return nLen;
}
*/


