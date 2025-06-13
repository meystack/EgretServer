
#include "StdAfx.h"
#include "HandleMgr.h"
#include "NoticeManager.h"
#include "../base/Container.hpp"
//const char sNoticeFile[] = "data/notice.txt";  
//const char sNSNNoticeFile[] = "data/config/WholeNotice/NSNNotice.config";

//int CNoticeManager::nMax_ChatRecord = 30;
using namespace jxInterSrvComm::DbServerProto;
bool CNoticeManager::Initialize()
{
	//GetLogicServer()->GetDataProvider()->GetGlobalConfig().
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen )
	{
		//开服前3天关闭防沉迷
		CMiniDateTime openTime = GetLogicServer()->GetServerOpenTime();

		SYSTEMTIME curSysTime;
		GetLocalTime(&curSysTime);
		CMiniDateTime nCurrentTime ;
		nCurrentTime.encode(curSysTime);
	}

	return ReloadFilterDb();	
}
bool CNoticeManager::ReloadFilterDb(int nHashFunCount,int nMemoryBytes)
{
	int nCount =0; 
	//  
	std::map<int, NSNNOTICE>& NSNNoticeConfs = GetLogicServer()->GetDataProvider()->GetNSNNoticeConfigs().getNoticeConf();
	std::map<int, NSNNOTICE>::iterator Tmpiter = NSNNoticeConfs.begin();
	for (;Tmpiter != NSNNoticeConfs.end();++Tmpiter)
	{
		NSNNOTICE& notice = Tmpiter->second;

		AddNotice(notice); 
	}
	return nCount >=1;
}  
VOID CNoticeManager::RunOne( TICKCOUNT nTickCount )
{	
	if (m_timer.CheckAndSet(nTickCount) && m_GameNotice.size() > 0)
	{
		bool isHadRemove = false; 
	
		//更新日期和时间 
		//SYSTEMTIME	tCurSysTime;
		//GetLocalTime(&tCurSysTime);
		//CMiniDateTime CurMiniDateTime;
		//UINT nowtime = CurMiniDateTime.encode(tCurSysTime);/
		UINT nowtime = GetGlobalLogicEngine()->getMiniDateTime();
		
		int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
		// NSN公告走5秒刷新 
		std::vector<NSNNOTICE>::iterator Tmpiter = m_GameNotice.begin();
		for (;Tmpiter != m_GameNotice.end();)
		{
			NSNNOTICE& notice = (*Tmpiter);

			// 结束
			if (notice.nBeforeSrvDay != 0 && nOpenServerDay > notice.nBeforeSrvDay)
			{ 
				Tmpiter = m_GameNotice.erase(Tmpiter);
				isHadRemove = true;
			}
			// 公告循环
			else
			{
				if (nowtime >= notice.nAfterSrvDay && nowtime > notice.nNextTime)
				{
					if (nowtime - notice.nNextTime > notice.nInterval)
					{
						if (notice.boIsTrundle)
							GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(notice.sContent, tstBigRevolving, 0);
						
						if (notice.boIsChat)
							GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(notice.sContent, tstChatSystem);
						
						notice.nNextTime = nowtime + notice.nInterval;
					}
				}
				++Tmpiter;
			}
		} 
		if (isHadRemove)
		{
			//SaveNotice();
		}
		
	}
} 

VOID  CNoticeManager::Destroy()
{ 
/*
	for (INT_PTR i = m_GameNotice.count()-1; i >=0; i--)
	{
		NSNNOTICE& data = m_GameNotice[i];
		if (data.sMsg)
		{
			GFreeBuffer(data.sMsg);
			data.sMsg = NULL;
		}
	}	
*/
}
NSNNOTICE* CNoticeManager::GetNotice(int nId)
{
	for(int i = 0; i < m_GameNotice.size(); i++)
	{
		if (m_GameNotice[i].nId == nId)
		{
			return &m_GameNotice[i];
		}
	}
	return NULL;
}
bool CNoticeManager::AddNotice(NSNNOTICE &notice/*int nId, unsigned int nStartTime, unsigned int nEndTime, int nMins, int nDisplayPos, LPCSTR sMemo*/)
{
	if (!notice.sContent)
	{
		return false;
	}
	NSNNOTICE* pNoticeData = GetNotice(notice.nId);
	if (!pNoticeData)
	{ 
		m_GameNotice.push_back(notice); 
	}
	else
	{ 
        OutputMsg(rmError, _T("AddNotice error on Add same NSNnotice config"));
	} 
	return false;
}
void CNoticeManager::RemoveNoticeByIndex(INT_PTR index)
{
	if (index >= m_GameNotice.size()|| index < 0)
	{
		return;
	} 

	std::vector<NSNNOTICE>::iterator Tmpiter = m_GameNotice.begin();
	for (; Tmpiter != m_GameNotice.end(); )
	{
		NSNNOTICE& data = (*Tmpiter);
		if (index == data.nId)
		{ 
			Tmpiter = m_GameNotice.erase(Tmpiter);  
			break;
		}
		else
		{
			Tmpiter++;
		}
	}  
}
bool CNoticeManager::DeleteNotice( LPCSTR sMemo )
{
	if (!sMemo) 
		return false;
	bool result = false;
	
	std::vector<NSNNOTICE>::iterator Tmpiter = m_GameNotice.begin();
	for (; Tmpiter != m_GameNotice.end(); )
	{
		NSNNOTICE& data = (*Tmpiter);
		if (strcmp(sMemo, data.sContent) == 0)
		{ 
			Tmpiter = m_GameNotice.erase(Tmpiter);  
			result = true; 
		}
		else
		{
			Tmpiter++;
		}
	}
	if (result)
	{
		//SaveNotice();
	}
	else
	{
		OutputMsg(rmTip,"DeleteNotice Error, sMemo:%s",sMemo);
	}
	return result;
}
/*
void CNoticeManager::SaveNotice()
{
	wylib::stream::CMemoryStream ms;

	for (INT_PTR i=0; i < m_GameNotice.size(); i++)
	{
		NSNNOTICE& data = m_GameNotice[i];
		if (data.sContent)
		{
			//公告id|开始时间|结束时间|时间间隔(分钟)|显示位置|公告内容
			char buf[256];
			sprintf(buf,"%d|%d|%d|%d|%d|",data.nId,data.nStartTime, data.nEndTime, data.nMins, data.nPos);
			ms.write(buf, strlen(buf));

			ms.write(data.sMsg, strlen(data.sMsg));
			
			sprintf(buf,"\r\n");
			ms.write(buf, strlen(buf));
		}
	}
	ms.saveToFile(sNoticeFile);
	SendNoticeToBackMgr();
}

void CNoticeManager::ParseNotice( LPCTSTR sPtr )
{
	//char buf[1024];
	char * args[10];
	char sInPtr[1280];
	_asncpytA(sInPtr, sPtr);
	CBackStageSender::ParseArgStr(args, sInPtr,10);
	//公告id|开始时间|结束时间|时间间隔(分钟)|显示位置|公告内容
	int nId = args[0]?atoi(args[0]):0;
	unsigned int nStartTime = args[1]?(unsigned int)atof(args[1]):0;
	unsigned int nEndTime= args[2]?(unsigned int)atof(args[2]):0;
	int nMins = args[3]?atoi(args[3]):0;
	int nPos = args[4]?atoi(args[4]):0;
	if (args[5] != NULL && strlen(args[5]) > 0)
	{
		AddNotice(nId, nStartTime, nEndTime, nMins, nPos, args[5]);
	} 
}
*/
void CNoticeManager::GmSendOffMsgToActor(LPCSTR nActorName,LPCSTR nMsg)
{
	if (!nActorName || !nMsg) return;

	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));

	packet.writeString(nMsg);
	Assert(packet.getPosition() <= MAX_MSG_COUNT);

	CMsgSystem::AddOfflineMsgByName((char *)nActorName,CMsgSystem::mtReturnSuggestMsg,MsgBuff,packet.getPosition(),0);

} 
VOID CNoticeManager::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{ 
} 
/*
void CNoticeManager::SendNoticeToBackMgr()
{
	int nCount = (int)m_GameNotice.count();
	CDataPacket &pdata = GetLogicServer()->GetBackClient()->allocProtoPacket(SEND_NOTICE_MGR); //分配一个 网络包
	pdata << (int)GetLogicServer()->GetServerIndex();
	pdata << (int)MSS_ADDNOTICE;
	pdata << (int)nCount;

	for(int i=0;i<nCount;i++)
	{
		NSNNOTICE& data = m_GameNotice[i];
		pdata << data.nId << data.nStartTime << data.nEndTime << data.nMins << data.nPos;
		pdata.writeString(data.sMsg);
	}

	GetLogicServer()->GetBackClient()->flushProtoPacket(pdata);
} 
*/
bool CNoticeManager::DeleteAllNotice()
{
	m_GameNotice.clear();
	//SaveNotice();
	return true;
} 
bool CNoticeManager::DeleteNoticeById(int nId)
{
	bool result = false;
	for(int i = 0; i < m_GameNotice.size(); i++)
	{
		if (m_GameNotice[i].nId == nId)
		{
			RemoveNoticeByIndex(i);//这里只会删除一个
			result = true;
			break;
		}
	}
	if (result)
	{
		//SaveNotice();
	}
	return result;
}