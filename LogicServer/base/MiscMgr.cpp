#include "StdAfx.h"
#include "../base/Container.hpp" 
#include "../property/PropertySet.h"

static const char *g_szCompensateId			= _T("CompensateId");			//补偿方案ID
static const char *g_szCompensateBeginTime	= _T("CompensateBeginTime");	//补偿开始时间
static const char *g_szCompensateOeveTime	= _T("CompensateOverTime");		//补偿结束时间
static const char *g_szCompensateLevel		= _T("CompensateLevel");		//补偿要求等级
static const char *g_szCompensateTitle		= _T("CompensateTitle");		//补偿Title



CMiscMgr::CMiscMgr()
{
	m_sendlogout = 0;
	m_sendlogin = 0;
	m_tomorrow = 0;
	m_nBackStageKillActorTime = 0;
	m_serverTempCombineTime = 0;
	m_serverBenginCombineTime = 0;
}
CMiscMgr::~CMiscMgr()
{

}

VOID CMiscMgr::Init()
{
	CMiniDateTime nNow = GetGlobalLogicEngine()->getMiniDateTime();
	m_tomorrow = nNow.tomorrow();
	m_sendlogout = nNow.tomorrow() - 2*60;
	m_sendlogin = nNow.tomorrow() + 10;
	//InitCompensateRank();			//初始化补偿排行榜(由脚本执行)
}

bool CMiscMgr::HotUpdateScript(LPCTSTR sFileName)
{
	if (sFileName == NULL)
	{
		return false;
	}
	CNpc *pNpc = NULL;
	//刷新全局功能NPC
	if (!_stricmp(sFileName, "FUNCTION"))
	{
		pNpc = GetGlobalLogicEngine()->GetGlobalNpc();			
	}
	else if (!_stricmp(sFileName, "MONSTER"))
	{
		pNpc = GetGlobalLogicEngine()->GetMonFuncNpc();
	}
	else if(!_stricmp(sFileName, "ITEM"))
	{
		pNpc = GetGlobalLogicEngine()->GetItemNpc();
	}
	else
	{
		return false;
	}
	if (pNpc != NULL)
	{
		char sBuff[127];
		sprintf_s(sBuff, ArrayCount(sBuff), "data/hotUpdate/%s.txt", sFileName);
		return pNpc->LoadScriptBuff(sBuff);
	}
	return false;
}

bool CMiscMgr::KickAllActorByLevel(int nCircle,int nLevel,bool boDisApear)
{
	TICKCOUNT dwCurTick = GetLogicCurrTickCount();
	if ( dwCurTick < m_nBackStageKillActorTime )
	{
		return false;
	}
	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *tmpActor = (CActor *)actorList[i];
		if (tmpActor && tmpActor->IsInited() && !tmpActor->IsDestory() && !tmpActor->CheckLevel(nLevel, nCircle))
		{
			if ( boDisApear )
			{
				CDataClient *pDBClient = GetLogicServer()->GetDbClient();
				CDataPacket &packet = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateActorState);
				int nRawServer = tmpActor->GetRawServerIndex();
				int nServerId = GetLogicServer()->GetServerIndex();
				unsigned int nActorId = tmpActor->GetId();
				packet << nRawServer << nServerId << nActorId << (byte)1;
				pDBClient->flushProtoPacket(packet);
			}
			tmpActor->CloseActor(lwiBackStageTickActorByLevel, false);
		}		
	}
	m_nBackStageKillActorTime = dwCurTick + 5 * 60 * 1000;
	return true;
}

bool CMiscMgr::OnBackClearAllMsg()
{  
	
	GetGlobalLogicEngine()->GetChatMgr().OnBackClearAllMsg();
	 
	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *tmpActor = (CActor *)actorList[i];
		if (tmpActor && tmpActor->IsInited() && !tmpActor->IsDestory())
		{
			if (tmpActor->OnGetIsTestSimulator()
				|| tmpActor->OnGetIsSimulator() )
			{
				continue;
			}
			CActorPacket pack;
			CDataPacket &data = tmpActor->AllocPacket(pack);
			data << (BYTE) enChatSystemID << (BYTE)sSendClearMsg;
			//data.writeBuf(pData,size);
			pack.flush();  
		}		
	} 
	return true;
}
VOID CMiscMgr::OnDbReturnData(INT_PTR nCmd, char * data, SIZE_T size)
{
	int nRawServerId, nLoginServerId;
	unsigned int nGid = 0;
	CDataPacketReader inPacket(data,size);
	BYTE nErrorCode;
	unsigned int nActorID;
	BYTE opreatCode = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID >> opreatCode >> nErrorCode;
	
	CActorCustomProperty CustomData;
	switch(nCmd)
	{
	case jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid:
	{
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			inPacket.readBuf(&CustomData, sizeof(CActorCustomProperty)); 
			
			CDataClient *pDBClient = GetLogicServer()->GetDbClient();
			CDataPacket &packet = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveCustomInfoUnForbid);
			int nServerId = GetLogicServer()->GetServerIndex();
			packet << nServerId << nServerId << nActorID;
			INT_PTR nSize = sizeof(CActorCustomProperty);  
			
			if(opreatCode == (BYTE)MASK_BACKSTAGE_UNFORVID)
			{ 
				BYTE * pDataint = (BYTE *)&CustomData.cbCustomProperty[ACTORRULE_KICK_USER];   
				*pDataint = 0;
			}
			else
			{ 
				unsigned int nDuration = 0;
				if((BYTE)opreatCode == (BYTE)MASK_BACKSTAGE_FORVID)
				{
					inPacket >> nDuration;  
				}

				BYTE * pDataint = (BYTE *)&CustomData.cbCustomProperty[ACTORRULE_KICK_USER];   
				*pDataint = 1;  
				unsigned int nFreePostTime = (unsigned int)(GetGlobalLogicEngine()->getMiniDateTime() + nDuration); 
 
				unsigned int * pDataUint = ( unsigned int *)&CustomData.cbCustomProperty[ACTORRULE_KICK_USER_FREETIME];  
					*pDataUint = nFreePostTime;  
			}

			packet.writeBuf(&CustomData, nSize);
			pDBClient->flushProtoPacket(packet); 
		}
		else
		{ 
		}
		break;

	}
	case jxInterSrvComm::DbServerProto::dcSaveCustomInfoUnForbid:
		break;
	case jxInterSrvComm::DbServerProto::dcLoadCustomInfoCustomTitleId:
	{
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			int nCutomTitleId = 0;
			inPacket.readBuf(&CustomData, sizeof(CActorCustomProperty));
			inPacket >> nCutomTitleId;  
			
			CDataClient *pDBClient = GetLogicServer()->GetDbClient();
			CDataPacket &packet = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveCustomInfoCustomTitleId);
			int nServerId = GetLogicServer()->GetServerIndex();
			packet << nServerId << nServerId << nActorID;
			INT_PTR nSize = sizeof(CActorCustomProperty);  
			
			int* pDataInt = (int*)&CustomData.cbCustomProperty[ACTORRULE_CUSTOMTITLE_ID];  
			*pDataInt = nCutomTitleId;  

			packet.writeBuf(&CustomData, nSize);
			pDBClient->flushProtoPacket(packet); 
		}
		break;
	}
	case jxInterSrvComm::DbServerProto::dcSaveCustomInfoCustomTitleId:
	{
		break;
	}
		
	}	
  
}

bool CMiscMgr::OnBackForbidUserMis(int Actorid, INT_PTR nDuration)
{
	//处理时间
	// TICKCOUNT dwCurTick = GetLogicCurrTickCount();
	// if ( dwCurTick < m_nBackStageKillActorTime )
	// {
	// 	return false;
	// } 
	// m_nBackStageKillActorTime = dwCurTick + 5 * 60 * 1000;

	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	INT_PTR nLen = actorList.count();
	for (INT_PTR i = 0; i < nLen; i++)
	{
		CActor * pActor = (CActor *)actorList[i];
		if (pActor != NULL && Actorid == pActor->GetId())
		{
			if (pActor->OnGetIsTestSimulator()
			 || pActor->OnGetIsSimulator() )
			{
				continue;
			}

			CActorPacket pack;
			pActor->AllocPacket(pack);
			pack << (BYTE) enDefaultEntitySystemID <<(BYTE)sActorForbid;
			pack.flush();

			pActor->OnBackForbidUser(nDuration);
			//pActor->CloseActor(lwiBackStageForbidActor, false);
			pActor->CloseActor(lwiBackStageTickActor, false);
			//pActor->CloseActor(lwiBackStageTickAll, false);
			return true;
		}
	}
	//查询离线玩家
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid); 
		//CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadNewServerMail);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId << (unsigned int)Actorid << (BYTE)MASK_BACKSTAGE_FORVID;
		DataPacket << (unsigned int)nDuration;
		
		pDbClient->flushProtoPacket(DataPacket);
		OutputMsg(rmTip, _T("CActor::dcLoadCustomInfoForbid, actorID=%u"),	Actorid);
	}

	return true;
}

bool CMiscMgr::OnBackUnForbidUserMis(int Actorid)
{
	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	INT_PTR nLen = actorList.count();
	for (INT_PTR i = 0; i < nLen; i++)
	{
		CActor * pActor = (CActor *)actorList[i];
		if (pActor != NULL && Actorid == pActor->GetId())
		{
			pActor->OnBackUnForbidUser();    
			return true;
		}
	}  
 
	//补充玩家数据 ，增加一个消息
	// CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	// CDataPacket &packet = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid); 
	// packet << GetLogicServer()->GetServerIndex()  << GetLogicServer()->GetServerIndex() << (unsigned int)Actorid;
	// pDBClient->flushProtoPacket(packet);
	// OutputMsg(rmTip, _T("CActor::dcLoadCustomInfoUnForbid, actorID=%u"),	Actorid);
 
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid); 
		//CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadNewServerMail);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId << (unsigned int)Actorid << (BYTE)MASK_BACKSTAGE_UNFORVID;
		pDbClient->flushProtoPacket(DataPacket);
		OutputMsg(rmTip, _T("CActor::dcLoadCustomInfoUnForbid, actorID=%u"),	Actorid);
	}

	return true;
}

bool CMiscMgr::OnBackAddCustomTitle(unsigned int nActorId, int nCustomTitleId)
{
	OneNewTitleConfig* CutomTitle = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(nCustomTitleId);
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if ( !CutomTitle )
	{
		return false;
	}

	// 在线玩家 添加自定义称号 
	if (! pActor 
		|| pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		// 离线玩家 添加自定义称号
		CDataClient* pDbClient = GetLogicServer()->GetDbClient();
		if ( pDbClient && pDbClient->connected() )
		{
			CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomInfoCustomTitleId); 
			int nServerId = GetLogicServer()->GetServerIndex();
			DataPacket << nServerId << nServerId << (unsigned int)nActorId;
			DataPacket << nCustomTitleId;
			pDbClient->flushProtoPacket(DataPacket);

			OutputMsg(rmTip, _T("CMiscMgr::OnBackAddCustomTitle offline actor success, nActorId=%u, nCustomTitleId=%u"), nActorId, nCustomTitleId);

			return true;
		}
		else
		{
			OutputMsg(rmError, _T("CMiscMgr::OnBackAddCustomTitle offline actor fail, nActorId=%u, nCustomTitleId=%u"), nActorId, nCustomTitleId);

			return false;
		} 
		return true;
	}

	return pActor->GetNewTitleSystem().addCustomTitle(nCustomTitleId); 
}

bool CMiscMgr::OnBackDelCustomTitle(unsigned int nActorId, int nCustomTitleId)
{
	OneNewTitleConfig* CutomTitle = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(-nCustomTitleId);
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if ( !CutomTitle )
	{
		return false;
	}

	// 在线玩家 添加自定义称号		
	if (! pActor 
		|| pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		// 离线玩家 添加自定义称号
		CDataClient* pDbClient = GetLogicServer()->GetDbClient();
		if ( pDbClient && pDbClient->connected() )
		{
			CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadCustomInfoCustomTitleId); 
			int nServerId = GetLogicServer()->GetServerIndex();
			DataPacket << nServerId << nServerId << (unsigned int)nActorId;
			DataPacket << nCustomTitleId;
			pDbClient->flushProtoPacket(DataPacket);

			OutputMsg(rmTip, _T("CMiscMgr::OnBackDelCustomTitle offline actor success, nActorId=%u, nCustomTitleId=%u"), nActorId, nCustomTitleId);
		}
		else
		{
			OutputMsg(rmError, _T("CMiscMgr::OnBackDelCustomTitle offline actor fail, nActorId=%u, nCustomTitleId=%u"), nActorId, nCustomTitleId);
		}

		return true;
	}
 
	return pActor->GetNewTitleSystem().delCustomTitle(-nCustomTitleId); 
}

bool CMiscMgr::isInTimeRange( int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec )
{
	SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
	int year = sysTime.wYear;
	int month = sysTime.wMonth;
	int day = sysTime.wDay;
	return isInDateRange(year, month, day, nStartHour, nStartMin, nStartSec, year, month, day, nEndHour, nEndMin, nEndSec);
}

bool CMiscMgr::isInDateRange(int nStartYear,int nStartMonth,int nStartDay,int nStartHour,int nStartMin,int nStartSec,int nEndYear,int nEndMonth,int nEndDay,int nEndHour,int nEndMin,int nEndSec)
{
	CMiniDateTime start; 
	start.encode(nStartYear,nStartMonth,nStartDay,nStartHour,nStartMin,nStartSec);
	CMiniDateTime end ;
	end.encode(nEndYear,nEndMonth,nEndDay,nEndHour,nEndMin,nEndSec);
	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
	if ( ( nNow >= start && nNow <= end ) || ( nNow >= end && nNow <= start))
	{
		return true;
	} 
	return false;
}

void CMiscMgr::DoScriptFunction(LPCTSTR szFuncName)
{
	CScriptValueList paramList, retParamList;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	if (!pNpc->GetScript().Call(szFuncName, paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
}

void CMiscMgr::DoScriptFunction( LPCTSTR szFuncName, int nFlag )
{
	CScriptValueList paramList, retParamList;
	paramList<<nFlag;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	if (!pNpc->GetScript().Call(szFuncName, paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
}


void CMiscMgr::DoScriptFunction(CActor* pActor, LPCTSTR szFuncName)
{
	if (pActor&&!pActor->IsInited())
	{
		return;
	}
	CScriptValueList paramList, retParamList;
	paramList<<pActor;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	if (!pNpc->GetScript().Call(szFuncName, paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
}

void CMiscMgr::DoScriptFunction(CActor* pActor, LPCTSTR szFuncName, int nFlag)
{
	if (pActor&&!pActor->IsInited())
	{
		return;
	}
	CScriptValueList paramList, retParamList;
	paramList<<pActor<<nFlag;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	if (!pNpc->GetScript().Call(szFuncName, paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
}

void CMiscMgr::DoScriptFunction(CActor* pActor, LPCTSTR szFuncName, int nFlag, int nParam, int nParam2)
{
	if (pActor&&!pActor->IsInited())
	{
		return;
	}
	CScriptValueList paramList, retParamList;
	paramList<<pActor<<nFlag<<nParam << nParam2;
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return;
	if (!pNpc->GetScript().Call(szFuncName, paramList, retParamList))
	{
		const RefString &s = pNpc->GetScript().getLastErrorDesc();
		OutputMsg(rmError, (LPCSTR)s);
	}
}

int CMiscMgr::GetDaysBeforeCombineServer()
{
	unsigned int nCurrentTime = CMiniDateTime::now();
	unsigned int nStartTime = 0;
	unsigned int combineServerTime = GetServerTempCombineTime(nStartTime); //获取合服的时间
	if(nCurrentTime >= combineServerTime || combineServerTime <= 0 || nStartTime > 0) //还没到合服时间
	{
		return 0;
	}
	else  
	{
		unsigned int nToday = CMiniDateTime::today(); //今日早上凌晨的时间
		int nSec = ( int)(combineServerTime - nToday); 
		return (int)(nSec / (3600 * 24))  +1;    //中间间隔了多少天
	}
}

bool CMiscMgr::IsFriendEach(CActor* pActor1, CActor* pActor2)
{
	return pActor1->GetFriendSystem().GetSocialFlag(pActor2->GetId(),SOCIAL_FRIEND) && 
		pActor2->GetFriendSystem().GetSocialFlag(pActor1->GetId(),SOCIAL_FRIEND);
}

VOID CMiscMgr::RunOne(CMiniDateTime& minidate,TICKCOUNT tick)
{
	if(m_1minuteTimer.CheckAndSet(tick)) //每分钟执行一次
	{
		CheckScriptTimer(minidate,tick);
	}

	/*
	if (m_5minuteTimer.CheckAndSet(tick))	//每5分钟执行一次
	{
		//补偿方案操作
		int nComId = 0;
		unsigned int nOverTime = 0;
		GetCompensateVar(nComId, nOverTime);
		if (nComId > 0 && minidate >= nOverTime)
		{
			SetCompensateVar(0);
		}
	}
	*/
}


//检测脚本的定时调用
void CMiscMgr::CheckScriptTimer(CMiniDateTime& minidate,TICKCOUNT tick)
{
	CScriptTimerConfig &config= GetLogicServer()->GetDataProvider()->GetScriptTimerConfig();
	SCRIPTTIMER * pTimer;
	
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	CMiniDateTime curTime;
	curTime.tv = GetGlobalLogicEngine()->getMiniDateTime();

	// 盛趣要求0点前几分钟发一下登出日志
	if (m_sendlogout.tv > 0 && curTime.tv >= m_sendlogout.tv)
	{
		m_sendlogout.tv = m_sendlogout.tv + 24*3600;	// 24点后TIME_FLUCTUATE秒分布

		CVector<void *> actorList;
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		INT_PTR nLen = actorList.count();
		for (INT_PTR i = 0; i < nLen; i++)
		{
			CActor * pActor = (CActor *)actorList[i];
			if (pActor != NULL && pActor->IsInited() && !pActor->IsDestory())
			{
				if (pActor->OnGetIsTestSimulator()
					|| pActor->OnGetIsSimulator() )
				{
					continue; //假人不统计在线时长
				}
				int nYb = pActor->GetProperty<int>(PROP_ACTOR_YUANBAO);
				int nLevel = pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
				GetLogicServer()->GetLogClient()->SendLoginLog(ltLogout,pActor->GetAccountID(),pActor->GetAccount(),pActor->GetIp(),pActor->GetEntityName(),nLevel, nYb ,0, pActor->GetId(),pActor->getOldSrvId());
			}
		}
	}

	// 渠道YY要求发登陆日志不要准点00:00:00
	if (m_sendlogin.tv > 0 && curTime.tv >= m_sendlogin.tv)
	{
		m_sendlogin.tv = m_sendlogin.tv + 24*3600;	// 24点后TIME_FLUCTUATE秒分布

		CVector<void *> actorList;
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		INT_PTR nLen = actorList.count();
		for (INT_PTR i = 0; i < nLen; i++)
		{
			CActor * pActor = (CActor *)actorList[i];
			if (pActor != NULL && pActor->IsInited() && !pActor->IsDestory())
			{
				if (pActor->OnGetIsTestSimulator()
					|| pActor->OnGetIsSimulator() )
				{
					continue; //假人不统计　
				}
				int nYb = pActor->GetProperty<int>(PROP_ACTOR_YUANBAO);
				int nLevel = pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
				int nOnlineTime = GetGlobalLogicEngine()->getMiniDateTime() -  pActor->GetLoginTime();//在线时间
				if(nOnlineTime <0) nOnlineTime =0;
				pActor->SetLoginTime(GetGlobalLogicEngine()->getMiniDateTime());
				GetLogicServer()->GetLogClient()->SendLoginLog(ltEntryGame,pActor->GetAccountID(),pActor->GetAccount(),pActor->GetIp(),pActor->GetEntityName(),nLevel, nYb ,0, pActor->GetId(),pActor->getOldSrvId());
			}
		}
	}
	
	if (m_tomorrow.tv > 0 && curTime.tv >= m_tomorrow.tv)
	{
		m_tomorrow.tv = m_tomorrow.rel_tomorrow();	// 24点后TIME_FLUCTUATE秒分布
		
		//
		CVector<void *> actorList;
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		INT_PTR nLen = actorList.count();
		for (INT_PTR i = 0; i < nLen; i++)
		{
			CActor * pActor = (CActor *)actorList[i];
			if (pActor != NULL && pActor->IsInited() && !pActor->IsDestory())
			{
				if (pActor->OnGetIsTestSimulator()
					|| pActor->OnGetIsSimulator() )
				{
					continue; //假人不统计　
				}

				if (curTime.tv >= pActor->GetTomorrowTime())
				{
					pActor->OnNewDayArrive();
					pActor->GetAchieveSystem().ActorAchievementComplete(nAchieveLogin, 1);
				}				
			}
		}
		OutputMsg(rmTip,_T("Tomorrow [%s] time =%d, is Next day!"), __FUNCTION__, (int)m_tomorrow.tv);
	}


	/*
	*检测脚本的调用，定时执行1分钟检测一次，判断当前时间是否满足条件，如果满足条件就执行脚本
	*这个功能是简化脚本的定时的开发订阅和取消定时调用的繁琐的工作，直接将这个功能一起执行，
	*脚本只需要写调用函数，由引擎定时去执行该npc身上的脚本
	*/
	for(INT_PTR i=0; i< config.count(); i++)
	{
		pTimer = (SCRIPTTIMER*)&(config.get(i));
		//OutputMsg(rmTip,_T("checking function =%s"),pTimer->sFuncName);
		//NPC的指针，争取只调用一次,免得每次去查找效率很低
		//if(pTimer->pNpc ==NULL)
		//{
			//pTimer->pNpc= CNpc::GetNpcPtr(pTimer->sSceneName,pTimer->sNpcName);
			//if(pTimer->pNpc ==NULL) continue;
		//}

		int nDays = GetLogicServer()->GetDaysSinceOpenServer();
		//检查开服第几天是否满足
		if(pTimer->serverday.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->serverday.count; i++)
			{
				if( nDays ==  *(pTimer->serverday.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}

		//检查开服第几天不能用
		if(pTimer->notopenserverdays.count >0 )
		{
			bool flag =true;
			for ( INT_PTR i=0; i<  pTimer->notopenserverdays.count; i++)
			{
				if( nDays ==  *(pTimer->notopenserverdays.pData +i) ) 
				{
					flag =false;
					break;
				}

			}
			if(!flag) continue;
		}

		int nCombineDays = GetLogicServer()->GetDaysSinceCombineServer();
		//检查合服第几天是否满足
		if(pTimer->combineserverday.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->combineserverday.count; i++)
			{
				if( nCombineDays ==  *(pTimer->combineserverday.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}

		//检查合服第几天不能用
		if(pTimer->notCombineserverday.count >0 )
		{
			bool flag =true;
			for ( INT_PTR i=0; i<  pTimer->notCombineserverday.count; i++)
			{
				if( nCombineDays ==  *(pTimer->notCombineserverday.pData +i) ) 
				{
					flag =false;
					break;
				}

			}
			if(!flag) continue;
		}

		int nBeforeCombineDays = GetGlobalLogicEngine()->GetMiscMgr().GetDaysBeforeCombineServer();
		//检查合服前第几天是否满足
		if(pTimer->beforeCombineday.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i < pTimer->beforeCombineday.count; i++)
			{
				if( nBeforeCombineDays ==  *(pTimer->beforeCombineday.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}

		//检查合服前第几天不能用
		if(pTimer->notBeforeCombineday.count >0 )
		{
			bool flag =true;
			for ( INT_PTR i=0; i<  pTimer->notBeforeCombineday.count; i++)
			{
				if( nBeforeCombineDays ==  *(pTimer->notBeforeCombineday.pData +i) ) 
				{
					flag =false;
					break;
				}

			}
			if(!flag) continue;
		}

		//检测月的条件是否满足
		if(pTimer->months.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->months.count; i++)
			{
				if( sysTime.wMonth ==  *(pTimer->months.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}
		//检测天
		if(pTimer->days.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->days.count; i++)
			{
				if( sysTime.wDay ==  *(pTimer->days.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}

		//检测周，是否在这个列表里
		if(pTimer->weeks.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->weeks.count; i++)
			{
				if( sysTime.wDayOfWeek ==  *(pTimer->weeks.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}

		//检测小时
		if(pTimer->hours.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->hours.count; i++)
			{
				if( sysTime.wHour ==  *(pTimer->hours.pData +i) ) 
				{
					flag =true;
					break;
				}

			}
			if(!flag) continue;
		}
		
		//检测分钟
		if(pTimer->minutes.count >0 )
		{
			bool flag =false;
			for ( INT_PTR i=0; i<  pTimer->minutes.count; i++)
			{
				INT_PTR nMiniteDis =  sysTime.wMinute - *(pTimer->minutes.pData +i);
				TICKCOUNT nTickDis = tick- pTimer->tick ; //两次调用之间的tick误差
				if( (nMiniteDis ==0 && nTickDis >= 60000)  || (nMiniteDis ==1 && nTickDis >= 90000 )  )	//把时间间隔由60000修改到90000 防止出现同一分钟执行几次的情况
				{
					//如果时间满足了就开始调用
					if( !GetGlobalLogicEngine()->GetGlobalNpc()->Call(pTimer->sFuncName) )
					{
						OutputMsg(rmError,_T("func [%s] function =%s,call error"),__FUNCTION__,pTimer->sFuncName);
					}
					pTimer->tick = tick; //记录上次的调用时间
					break;
				}
			}			
		}
	}
}
CScene* CMiscMgr::GetScenePtrById( int nSceneId)
{
	CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if (pFb != NULL)
	{
		return pFb->GetScene(nSceneId);
	}
	return NULL;
}

//获取系统静态变量
INT_PTR CMiscMgr::GetSystemStaticVarValue(LPCTSTR szVarName,INT_PTR nDefaultValue)
{
	CCLVariant& pVarMgr	= GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
	INT_PTR nValue = nDefaultValue;
	CCLVariant* pVar = pVarMgr.get(szVarName);
	if( pVar != NULL )
	{
		nValue = (INT_PTR)(double)*pVar;
	}
	return nValue;
}

//设置系统静态变量
void CMiscMgr::SetSystemStaticVarValue(LPCTSTR szVarName, INT_PTR nValue)
{
	CCLVariant& pVarMgr	=GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar();
	pVarMgr.set(szVarName, (double)nValue);
}

INT_PTR CMiscMgr::GetSystemDyanmicVarValue(LPCTSTR szVarName,INT_PTR nDefaultValue)
{
	CCLVariant& pVarMgr	= GetGlobalLogicEngine()->GetDyanmicVar();
	INT_PTR nValue = nDefaultValue;
	CCLVariant* pVar = pVarMgr.get(szVarName);
	if( pVar != NULL )
	{
		nValue = (INT_PTR)(double)*pVar;
	}
	return nValue;
}

KillerData*  CMiscMgr::AddKillerData(unsigned int nKillerActorId, unsigned int nBeKilledActorId)
{
	KillerData data;
	data.nKillerActorId = nKillerActorId;
	data.nBeKilledActorId = nBeKilledActorId;
	INT_PTR nIndex = m_killerData.add(data);
	return &m_killerData[nIndex];
}
KillerData* CMiscMgr::GetKillData(unsigned int nKillerActorId, unsigned int nBeKilledActorId)
{
	for (INT_PTR i =0; i <m_killerData.count(); i++)
	{
		KillerData& data = m_killerData[i];
		if (data.nKillerActorId == nKillerActorId && data.nBeKilledActorId == nBeKilledActorId)
		{
			return &data;
		}
	}
	return NULL;
}

//获取某个排行榜的达到限额要求的数量
int CMiscMgr::GetRankLimitCount(CRanking *pRanking, int nLimit)
{
	INT_PTR idx = 0;
	if ( pRanking )
	{
		for( idx=0; idx < pRanking->GetItemCount(); idx++ )
		{
			CRankingItem *pItem = pRanking->GetItem(idx);
			if( pItem )
			{
				if( pItem->GetPoint() < nLimit )
				{
					return (int)idx;
				}
			}
		}
	}
	return (int)idx;
}

void CMiscMgr::GetCompensateVar(int &nId, unsigned int &nBeginTime, unsigned int &nOverTime)
{
	CGlobalVarMgr *pVarMgr = &GetGlobalLogicEngine()->GetGlobalVarMgr();
	CCLVariant &var = pVarMgr->GetVar();

	CCLVariant *pId			= var.get(g_szCompensateId);
	CCLVariant *pBeginTime	= var.get(g_szCompensateBeginTime);
	CCLVariant *pOverTime	= var.get(g_szCompensateOeveTime);
	if (pId && pOverTime)
	{
		nId			= (int)((double)*pId);
		nBeginTime	= (unsigned int)((double)*pBeginTime);
		nOverTime	= (unsigned int)((double)*pOverTime);
	}
	else
	{
		nId			= 0;
		nBeginTime  = 0;
		nOverTime   = 0;
	}
}

/*
nBeginTime:补偿开始时间，秒
nLastTime :补偿结束时间，秒
*/
int CMiscMgr::SetCompensateVar(int nId, unsigned int nBeginTime, unsigned int nLastTime, int nLevelLimit, char* szTitle, bool bClearRank)
{
	CGlobalVarMgr *pVarMgr = &GetGlobalLogicEngine()->GetGlobalVarMgr();
	CCLVariant &var = pVarMgr->GetVar();
	if (nId > 0)
	{
		var.set(g_szCompensateId, nId);
		var.set(g_szCompensateBeginTime, nBeginTime);
		var.set(g_szCompensateOeveTime,  nBeginTime+nLastTime);
		var.set(g_szCompensateLevel,  nLevelLimit);
		var.set(g_szCompensateTitle,  szTitle);

		DoScriptFunction("ClearBackCompensateRank");		//通知脚本
		DoScriptFunction("BackCompensateMailAll");//邮件发补偿
		return nId;
	}
	else
	{
		CCLVariant *pVar = NULL;
		pVar = var.get(g_szCompensateId);
		if (pVar)
		{
			pVar->clear();
		}

		pVar = var.get(g_szCompensateBeginTime);
		if (pVar)
		{
			pVar->clear();
		}

		pVar = var.get(g_szCompensateOeveTime);
		if (pVar)
		{
			pVar->clear();
		}

		pVar = var.get(g_szCompensateLevel);
		if (pVar)
		{
			pVar->clear();
		}

		pVar = var.get(g_szCompensateTitle);
		if (pVar)
		{
			pVar->clear();
		}
	}

	return 0;
}

//是否靠近场景NPC，该场景只能有一个此NPC
bool CMiscMgr::IsNearBySceneNpc(CActor *pActor, int nSceneId, int nNpcId )
{
	CScene *pScene = GetScenePtrById( nSceneId );
	if( pScene )
	{
		CNpc *pNpc = pScene->GetNpc( nNpcId );
		if( pNpc )
		{
			return pActor->IsInSameScreen(pNpc);
		}
	}
	return false;
}
