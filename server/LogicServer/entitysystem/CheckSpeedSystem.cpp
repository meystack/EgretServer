#include "StdAfx.h"
#include "CheckSpeedSystem.h"

int		CCheckSpeedSystem::m_IntevalTime = 5;
int		CCheckSpeedSystem::m_DeviationTimes = 100;
bool		CCheckSpeedSystem::m_bOpenVerifyFlag = false;		//默认开启
int CCheckSpeedSystem::m_useSpeedHandle = enCloseActor;

int	CCheckSpeedSystem::m_nTimeRate = 100;			//时间比率
int	CCheckSpeedSystem::m_nStartCheck = 4;		    //开始检测的次数
int	CCheckSpeedSystem::m_nSpeedCheck = 58;			//加速检查值

int CCheckSpeedSystem::m_nUpLimit = 80;				//百分比上限参数
int CCheckSpeedSystem::m_nLowLimit = 10;			//百分比下限参数
int CCheckSpeedSystem::m_nUpLimit2 = 90;			//百分比上限参数			
int CCheckSpeedSystem::m_nLowLimit2 = 50;			//百分比下限参数

int	CCheckSpeedSystem::m_nRideTimes = 10;			//上下坐骑多少次			
int	CCheckSpeedSystem::m_nIntervalTime = 2;			//间隔这么久算1次
int	CCheckSpeedSystem::m_nClientTime = 5;			//间隔这么久清除

CCheckSpeedSystem::CCheckSpeedSystem()
{
	m_SendVerifyTime = 0;
	m_nOperCount = 0;
	m_nCurPos = 0;
	m_nCount = 0;

	m_nOperateTimes = 0;
	m_nAccTime = 0;
	m_nCheckCount = 0;
	m_nLastNextTime = 0;
	m_nClearCountTime = 0;

}

bool CCheckSpeedSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA))
	{
		return false;
	}

	return true;
}

void CCheckSpeedSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return;//假人不需要检测加速
    }
	switch(nCmd)
	{
	case cSendSpeedTimeData:
		{
			OnCheckUseSpeedTime(packet);
			break;
		}

	default:
		break;
	}
}

void CCheckSpeedSystem::OnEnterGame()
{
	if(!m_pEntity) return;

	SendSpeedTimeVerify();
}

void CCheckSpeedSystem::SendSpeedTimeVerify()
{
	if(!m_pEntity)
	{ 
		return;
	}

    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return;//假人不需要检测加速
    }
	m_SendVerifyTime = _getTickCount();
 
	CActorPacket AP;
	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enCheckSpeedSystemID << (BYTE)cSendSpeedTimeData; 
	DataPacket << (int)m_IntevalTime; 
	AP.flush(); 
}

void CCheckSpeedSystem::OnCheckUseSpeedTime(CDataPacketReader &packet)
{
	if(!m_pEntity) return;

	WORD nClientTimeRate = 0; //时间比率
	packet >> nClientTimeRate;

	//OutputMsg(rmTip,_T("TimeRate = %d"),nClientTimeRate);
	m_nCount ++;
	if (nClientTimeRate > 0 && m_nTimeRate > 0)
	{
		if (nClientTimeRate > m_nTimeRate + 5)  //超过标准值
		{
			if (m_nCount > m_nStartCheck) //前端要登录20秒后才取初始化值，忽略前三次值，
			{
				OutputMsg(rmWaning," Actorname=%s, kickout for TimeRate check, nClientTimeRate=%d, nTimeRate=%d, m_nCount=%d",m_pEntity->GetEntityName(), nClientTimeRate, m_nTimeRate, m_nCount);
				//((CActor*)m_pEntity)->CloseActor();	暂时屏蔽，切换场景会影响
			}
		}
	}
	
	TICKCOUNT CurTime = _getTickCount();
	if(m_SendVerifyTime > 0 && m_bOpenVerifyFlag && (CurTime > m_SendVerifyTime + m_IntevalTime*1000*4)	)	//延时过久
	{
		OutputMsg(rmWaning," Actorname=%s, delay for TimeRate check",m_pEntity->GetEntityName());
		UseSpeedToolHandle(lwiCheckSpeedSystemTick);
		return;
	}

	//OutputMsg(rmTip,_T("m_SendVerifyTime = %d"),CurTime-m_SendVerifyTime);
	if(CurTime < (m_SendVerifyTime + m_IntevalTime*1000 - m_DeviationTimes) )		//允许300毫秒的误差  使用了加速
	{
		OutputMsg(rmWaning,"OnCheckUseSpeedTime Actorname=%s, use speed tool, kickout",m_pEntity->GetEntityName());

		if (m_pEntity->s_boCheckQuickSealActor)
		{
			/*
			CDataClient *pDBClient = GetLogicServer()->GetDbClient();
			CDataPacket &packet = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateActorState);
			int nRawServer = m_pEntity->GetRawServerIndex();
			int nServerId = GetLogicServer()->GetServerIndex();
			unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家的ID
			packet << nRawServer << nServerId << nActorId << (byte)1;
			pDBClient->flushProtoPacket(packet);
			*/
		}
		m_nCheckCount++;
		m_nClearCountTime = CurTime + 60000;
		if (CActor::s_nQuickBuffId > 0 && m_nCheckCount > 3)
		{
			if (CActor::s_nQuickBuffId > 0)
			{
				((CActor*)m_pEntity)->GetBuffSystem()->Append(CActor::s_nQuickBuffId);
			}
			//((CActor*)m_pEntity)->GetBuffSystem()->Append(CActor::s_nQuickBuffId);
			m_nCheckCount = 0;
		}
		

		SendSpeedTimeVerify();
		//m_pEntity->CloseActor();
	}
	else
	{
		if (CurTime > m_nClearCountTime && m_nCheckCount > 0)
		{
			m_nCheckCount--;
			m_nClearCountTime = CurTime + 60000;
		}
		SendSpeedTimeVerify();
	}

}

int CCheckSpeedSystem::CheckSpeed(int nSpeedValue)
{	
	int result = -1;
	if (nSpeedValue < 0)
	{
		m_nOperCount++;
		return result;
	}

	if (nSpeedValue > 0)
	{
		if (m_nCurPos < SPEED_COUNT)
		{
			m_SpeedValues[m_nCurPos++] = nSpeedValue;
			m_nOperCount++;
		}

	}

	if (m_nCurPos >= SPEED_COUNT) 
	{
		if ( m_nCurPos * 100 <= m_nOperCount * m_nLowLimit ) //  rate <= 0.1
		{
			//////////////////////////////////////////////////////////////////////////
#if 0
			OutputMsg(rmTip,_T("........................ \n"));
			for (int i = 0; i < SPEED_COUNT; i++)
			{
				OutputMsg(rmTip,_T("speed value : %d \n"), m_SpeedValues[i]);
			}
#endif
			//////////////////////////////////////////////////////////////////////////
			result = enErrorJudge;
		}
		else if ( m_nCurPos * 100 >= m_nOperCount * m_nUpLimit ) // rate >= 0.8
		{
			//////////////////////////////////////////////////////////////////////////
#if 0
			OutputMsg(rmTip,_T("........................ \n"));
			for (int i = 0; i < SPEED_COUNT; i++)
			{
				OutputMsg(rmTip,_T("speed value : %d \n"), m_SpeedValues[i]);
			}
#endif
			//////////////////////////////////////////////////////////////////////////
			OutputMsg(rmTip,_T("name=%s,CheckSpeed ConfirmSpeed"), m_pEntity->GetEntityName());
			result = enConfirmSpeed;
		}
		else 
		{
			//////////////////////////////////////////////////////////////////////////
#if 0
			int nAverageSpeed = 0;
			for (int i = 0; i < SPEED_COUNT; i++)
			{
				nAverageSpeed += m_SpeedValues[i];
			}
			OutputMsg(rmTip,_T("........................ \n"), nAverageSpeed);
			for (int i = 0; i < SPEED_COUNT; i++)
			{
				OutputMsg(rmTip,_T("speed value : %d \n"), m_SpeedValues[i]);
			}

			nAverageSpeed /= SPEED_COUNT;

			OutputMsg(rmTip,_T("average value : %d \n"), nAverageSpeed);
#endif
			//////////////////////////////////////////////////////////////////////////

			//int nSpeedCheck = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSpeedCheck;
			int nSpeedCount = 0;
			for (int i = 0; i < SPEED_COUNT; i++)
			{
				if (m_SpeedValues[i] > m_nSpeedCheck)
				{
					nSpeedCount++;
				}
			}

			if (nSpeedCount * 100 >= SPEED_COUNT * m_nUpLimit2)
			{
				OutputMsg(rmTip,_T("name=%s,CheckSpeed nSpeedCount"), m_pEntity->GetEntityName());
				result = enConfirmSpeed;
			}
			else if (nSpeedCount * 100 <= SPEED_COUNT * m_nLowLimit2)
			{
				result = enErrorJudge;
			}
			else
			{
				OutputMsg(rmTip,_T("name=%s,CheckSpeed IntervalSpeed"), m_pEntity->GetEntityName());
				result = enIntervalSpeed;
			}		
		}
		//OutputMsg(rmTip,_T("m_nOperCount : %d \n"), m_nOperCount);
		memset(m_SpeedValues, 0, SPEED_COUNT * sizeof(int));
		m_nOperCount = 0;
		m_nCurPos = 0;
	}
	else
	{
		if (m_nOperCount >= SPEED_COUNT * 100) //防止保存的操作值过大, SPEED_COUNT / 0.1
		{
			memset(m_SpeedValues, 0, SPEED_COUNT * sizeof(int));
			m_nOperCount = 0;
			m_nCurPos = 0;
		}
	}

	return result;
}

void CCheckSpeedSystem::CheckUseSpeed(TICKCOUNT nCurrTime, INT_PTR nNextTime)
{
	if(!m_pEntity) return;
	if(nNextTime <= 0) return;

	int nSpeedValue = -1;
	if (m_nOperateTimes >= 1)
	{
		TICKCOUNT nInterval =(TICKCOUNT) ((m_nAccTime - nCurrTime));
#if 0
		OutputMsg(rmTip,_T("CheckUseSpeed -> m_nAccTime : %lld"), m_nAccTime);
		OutputMsg(rmTip,_T("CheckUseSpeed -> nCurrTime : %lld"), nCurrTime);
		OutputMsg(rmTip,_T("CheckUseSpeed -> nNextTime : %lld"), nNextTime);
		OutputMsg(rmTip,_T("CheckUseSpeed -> nInterval : %lld"), nInterval);
#endif 	
		if (nInterval > 0 && m_nLastNextTime > 0)
		{
			//OutputMsg(rmTip,_T("CheckUseSpeed -> nInterval : %lld"), nInterval);
			if(NULL == ((CActor*)m_pEntity)->GetBuffSystem()->GetFirstBuff(aMoveSpeedAdd))
			{

				nSpeedValue = (int)(nInterval * 1000 / m_nLastNextTime); //nSpeedValue  = 加速比 * 1000 / nNextTime
				//OutputMsg(rmTip,_T("nInterval time : %d"), nInterval); ///
			}
		}

		switch (CheckSpeed(nSpeedValue))
		{
		case enErrorJudge:
			break;
		case enIntervalSpeed:
			{
				OutputMsg(rmWaning," Actorname=%s, mabi for speed check, ",m_pEntity->GetEntityName());
				static const char * sBuffName = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpGMQuickBuffName);
				if(sBuffName ==NULL)
				{
					sBuffName ="Forbid";
				}
				if (CActor::s_nQuickBuffId > 0)
				{
					((CActor*)m_pEntity)->GetBuffSystem()->Append(CActor::s_nQuickBuffId);
				}
				//((CActor*)m_pEntity)->GetBuffSystem()->Append(aDizzy,1,1,10,1,sBuffName); //麻痹

			}
			break;
		case enConfirmSpeed:
			{
				OutputMsg(rmWaning," Actorname=%s, useSpeedToolHandle for speed check, ",m_pEntity->GetEntityName());
				UseSpeedToolHandle(lwiCheckSpeedConfirmSpeed);
			}
			break;
		default:
			break;
		}

	}

	if (0 == m_nOperateTimes)
	{
		m_nOperateTimes++;
	}

	m_nLastNextTime = nNextTime;
	m_nAccTime = nCurrTime + nNextTime; //计算下次正常到达时间

}

void CCheckSpeedSystem::KickToBlackHouse()
{
	TriggerEvent(enKickToBlackHouse);
}
void CCheckSpeedSystem::TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	/*
	if (!m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	CScriptValueList paramList;
	pActor->InitEventScriptVaueList(paramList,aeBlackHouse);
	paramList << (int)nSubEvent;
	if(nParam1 >=0)
	{
		paramList << (int)nParam1;
		if(nParam2 >=0)
		{
			paramList << (int)nParam2;
			if(nParam3 >=0)
			{
				paramList << (int)nParam3;
			}
		}	
	}

	pActor->OnEvent(paramList,paramList);
	*/
}
void CCheckSpeedSystem::UseSpeedToolHandle(int nWayId)
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor )
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;
	switch(m_useSpeedHandle)
	{
	case enCloseActor:
		pActor->CloseActor(nWayId);
		break;
	case enBlackHouse:
		KickToBlackHouse();
		break;
	default:
		break;
	}
}
