
#include "StdAfx.h"
#include "HandleMgr.h"
#include "ChatManager.h"
#include "../base/Container.hpp"
const char sNoticeFile[] = "data/notice.txt";
const int nOneNoticeInSec = 30;//30秒内最多只刷一次后台公告。
const int nDefaultMins = 1;		//1分钟起
static char* forbitfile("data/forbitword.txt");
static char* filterfile("data/fw.txt");

using namespace jxInterSrvComm::DbServerProto;
int CChatManager::nMax_ChatRecord = 30;

bool CChatManager::ChatCsRecordSort(CHATRECORD tmp1, CHATRECORD tmp2)
{
	return tmp1.msgTime < tmp2.msgTime;
}
bool CChatManager::Initialize()
{
	//GetLogicServer()->GetDataProvider()->GetGlobalConfig().
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().bIsFcmOpen )
	{
		//开服前3天关闭防沉迷
		CMiniDateTime openTime     = GetLogicServer()->GetServerOpenTime();

		SYSTEMTIME curSysTime;
		GetLocalTime(&curSysTime);
		CMiniDateTime nCurrentTime ;
		nCurrentTime.encode(curSysTime);
	}

	return ReloadFilterDb();	
}
bool  CChatManager::ReloadFilterDb(int nHashFunCount,int nMemoryBytes)
{
	int nCount =0;

	if(!FDOP::FileExists(forbitfile))
	{
		OutputMsg(rmError,_T("No forbitword table")); //找不到防沉迷的表
		return false;
	}
	
	if(!FDOP::FileExists( filterfile ))
	{
		OutputMsg(rmError,_T("No Fw table")); //找不到防沉迷的表
		return false;
	}

#ifdef USE_HASK_FILTER
	m_forbitWards.Destroy();
	nCount= m_forbitWards.Load(forbitfile,nMemoryBytes,nHashFunCount);
	if(nCount < 1)return false;

	m_hashFilter.Destroy();
	nCount= m_hashFilter.Load(filterfile,nMemoryBytes,nHashFunCount);
	if(nCount < 1)return false;
#else
	if(m_pForbitWords)
	{
		FreeMBCSFilter(m_pForbitWords);
		m_pForbitWords = NULL;
	}
	m_pForbitWords = CreateMBCSFilter(NULL);
	nCount = LoadMBCSFilterWords(m_pForbitWords, forbitfile);
	if(nCount < 1)return false;

	if(m_pFilterWords)
	{
		FreeMBCSFilter(m_pFilterWords);
		m_pFilterWords =NULL;
	}
	m_pFilterWords = CreateMBCSFilter(NULL);
	nCount=LoadMBCSFilterWords(m_pFilterWords,"data/fw.txt");
	if(nCount < 1)return false;
#endif

	// 读取游戏的公告
	if(!FDOP::FileExists( LPCTSTR(sNoticeFile) ))
	{
		//OutputMsg(rmError,sNoticeFile); //找不到防沉迷的表
	}else
	{
		wylib::stream::CMemoryStream NoticeStream;
		if (NoticeStream.loadFromFile("data/notice.txt") > 0)
		{
			LPCTSTR sPtr = (LPCTSTR)NoticeStream.getMemory();
			LPCTSTR sTag;
			while (*sPtr)
			{
				//搜索换行符
				sTag = _tcschr(sPtr, '\n');
				if (sTag)
				{
					if (*(LPTSTR)(sTag - 1) == '\r')
					{
						*(LPTSTR)(sTag - 1) = 0;
					}
					*(LPTSTR)sTag = 0;
					sTag++;					
					if (*sTag)
					{
						ParseNotice(sPtr);
						sPtr = sTag;
					}
				}
				if (NULL == sTag || !(*sTag))
				{
					ParseNotice(sPtr);
					break;
				}
			}
		}
		
	}
	return nCount >=1;
}


VOID CChatManager::RunOne( TICKCOUNT nTickCount )
{
	if (m_timer.CheckAndSet(nTickCount) && m_GameNotice.count() > 0)
	{
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		bool isHadRemove = false;
		//DECLARE_FUN_TIME_PROF()
		//发送全服公告
		INT_PTR nLen = m_GameNotice.count();
		for (INT_PTR i = nLen -1; i >=0; i--)
		{
			NOTICEDATA& data = m_GameNotice[i];
			
			if (data.nStartTime > 0 &&  data.nStartTime > nNow)//未开始
			{
				continue;
			}
			if (data.nStartTime == 0)//即时发送
			{
				//GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(data.sMsg,data.nPos);
				GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(data.sMsg,data.nPos);
				m_timer.SetNextHitTimeFromNow(nOneNoticeInSec*1000);
				RemoveNoticeByIndex(i);
				isHadRemove = true;
			}
			if (data.nEndTime > 0 && data.nEndTime < nNow)//已经结束了
			{
				RemoveNoticeByIndex(i);
				isHadRemove = true;
				continue;
			}
			if (nNow >= data.nHitTime)
			{
				//GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(data.sMsg,data.nPos);
				GetGlobalLogicEngine()->GetEntityMgr()->BroadNotice(data.sMsg, data.nPos);
				data.nHitTime = nNow + data.nMins * 60;
				m_timer.SetNextHitTimeFromNow(nOneNoticeInSec*1000);
				if (data.nEndTime > 0 && data.nHitTime > data.nEndTime)
				{
					RemoveNoticeByIndex(i);
					isHadRemove = true;
				}
			}
		}
		if (isHadRemove)
		{
			SaveNotice();
		} 
	}
	static bool gRunOnce = false; 

	if (!gRunOnce
		&& m_HalfTimer.CheckAndSet(nTickCount) 
		&& !GetLogicServer()->IsCrossServer())
	{
		gRunOnce = true;//只能跑一次 ，万一跨服比正式服早开服
		
		if(OnCheckCanMsg())
		{  
			UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();  
			CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
			CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendReqChat);
			int nServerId = GetLogicServer()->GetCrossServerId();
			OutputMsg(rmTip, _T("[CrossLogin] CChatManager ") );
			packet << nServerId ;
			packet << (int)0;//发送0 要全部数据
			packet << nNow;//发送时间
			pCrossClient->flushProtoPacket(packet);
			
			m_HalfTimer.SetNextHitTimeFromNow(60*1000);
		}
	}
}

int CChatManager::Filter(char *pInput)
{
	if(pInput ==NULL) return NULL;
	
	int nTotalFilterCount=0; //共有多少个屏蔽字符

	char *pInputPtr = pInput;
	char *pFilterWord;
	int nFilteWordLen;
    DECLARE_TIME_PROF("CChatManager::Filter");

#ifdef USE_HASK_FILTER
	while (pFilterWord = m_hashFilter.Filter( pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		nTotalFilterCount ++;
		pInputPtr = pFilterWord + nFilteWordLen;
	}

	while (pFilterWord = m_forbitWards.Filter( pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		++nTotalFilterCount;
		pInputPtr = pFilterWord + nFilteWordLen;
	}

	pInputPtr = pInput;
	while (pFilterWord = m_hashFilter.Filter2( pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		++nTotalFilterCount;
		pInputPtr = pFilterWord + nFilteWordLen;
	}

	pInputPtr = pInput;
	while (pFilterWord = m_forbitWards.Filter2( pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		++nTotalFilterCount;
		pInputPtr = pFilterWord + nFilteWordLen;
	}

#else
	while (pFilterWord = MatchMBCSFilterWord(m_pFilterWords, pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		nTotalFilterCount ++;
		pInputPtr = pFilterWord + nFilteWordLen;
	}

	while (pFilterWord = MatchMBCSFilterWord(m_pForbitWords, pInputPtr, &nFilteWordLen))
	{
		memset(pFilterWord, '*', nFilteWordLen);
		++nTotalFilterCount;
		pInputPtr = pFilterWord + nFilteWordLen;
	}
#endif

	return nTotalFilterCount;
}

/*
* Comments: 一个字符串是否和谐
* Param char * pInput:字符串指针
* @Return bool:和谐返回true，否则返回false
*/
bool CChatManager::IsStrInvalid(char * pInput, UINT_PTR nMode, int* pLen)
{
	int nFilteWordLen;
	int* pFilteWordLen(pLen ? pLen : &nFilteWordLen);
	if (1 == nMode)
	{
#ifdef USE_HASK_FILTER
		if ( m_hashFilter.Filter( pInput, pFilteWordLen))
		{
			return false;
		}
#else
		if ( MatchMBCSFilterWord(m_pFilterWords, pInput, pFilteWordLen))
		{
			return false;
		}
#endif
	}else if(2 == nMode){
#ifdef USE_HASK_FILTER
		if ( m_forbitWards.Filter( pInput, pFilteWordLen))
		{
			return false;
		}
#else
		if ( MatchMBCSFilterWord(m_pForbitWords, pInput, pFilteWordLen)){
			return false;
		}
#endif
	}

	return true;
}

VOID  CChatManager::Destroy()
{
#ifdef USE_HASK_FILTER
	m_hashFilter.Destroy();
	m_forbitWards.Destroy();
#else
	if(m_pFilterWords)
	{
		FreeMBCSFilter(m_pFilterWords);
	}
	if(m_pForbitWords)
	{
		FreeMBCSFilter(m_pForbitWords);
	}
#endif

	for (INT_PTR i = m_GameNotice.count()-1; i >=0; i--)
	{
		NOTICEDATA& data = m_GameNotice[i];
		if (data.sMsg)
		{
			GFreeBuffer(data.sMsg);
			data.sMsg = NULL;
		}
	}	
}
CChatManager::NOTICEDATA* CChatManager::GetNotice(int nId)
{
	for(int i = 0; i < m_GameNotice.count(); i++)
	{
		if (m_GameNotice[i].nId == nId)
		{
			return &m_GameNotice[i];
		}
	}
	return NULL;
}
bool CChatManager::AddNotice(int nId, unsigned int nStartTime, unsigned int nEndTime, int nMins, int nDisplayPos, LPCSTR sMemo)
{
	if (!sMemo)
	{
		return false;
	}
	NOTICEDATA* pNoticeData = GetNotice(nId);
	if (!pNoticeData)
	{
		NOTICEDATA data;
		data.nId = nId;
		m_GameNotice.add(data);
		pNoticeData = GetNotice(nId);
	}
	if (pNoticeData)
	{
		if (nMins == 0)
		{
			nMins = nDefaultMins;
		}
		INT_PTR len = strlen(sMemo);
		if (pNoticeData->sMsg)
		{
			GFreeBuffer(pNoticeData->sMsg);
			pNoticeData->sMsg = NULL;
		}
		pNoticeData->sMsg = (char*)GAllocBuffer(len+1);
		strcpy(pNoticeData->sMsg,sMemo);
		pNoticeData->nPos = nDisplayPos;
		pNoticeData->nStartTime = nStartTime;
		pNoticeData->nEndTime = nEndTime;
		pNoticeData->nMins = nMins;
		pNoticeData->nHitTime = nStartTime;
	
		SaveNotice();
		return true;
	}
	return false;
}
void CChatManager::RemoveNoticeByIndex(INT_PTR index)
{
	if (index >= m_GameNotice.count()|| index < 0)
	{
		return;
	}
	NOTICEDATA& data = m_GameNotice[index];
	if (data.sMsg)
	{
		GFreeBuffer(data.sMsg);
		data.sMsg = NULL;
	}
	m_GameNotice.remove(index);//释放内存
}
bool CChatManager::DeleteNotice( LPCSTR sMemo )
{
	if (!sMemo) return false;
	bool result = false;
	for (INT_PTR i = m_GameNotice.count()-1; i >=0; i--)
	{
		NOTICEDATA& data = m_GameNotice[i];
		if (strcmp(sMemo,data.sMsg) == 0)
		{
			RemoveNoticeByIndex(i);
			result = true;
		}
	} 
	if (result)
	{
		SaveNotice();
	}
	else
	{
		OutputMsg(rmTip,"DeleteNotice Error, sMemo:%s",sMemo);
	}
	return result;
}

void CChatManager::SaveNotice()
{
	wylib::stream::CMemoryStream ms;

	for (INT_PTR i=0; i < m_GameNotice.count(); i++)
	{
		NOTICEDATA& data = m_GameNotice[i];
		if (data.sMsg)
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

void CChatManager::ParseNotice( LPCTSTR sPtr )
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
	//int nDis = 0;
	//const char* sNumber = strchr(sPtr,'|');
	//if (sNumber)
	//{
	//	NOTICEDATA data;
	//	INT_PTR len = sNumber-sPtr+1;
	//	data.sMsg = (char*)GAllocBuffer(len);//用|分隔
	//	ZeroMemory(data.sMsg,len);
	//	strncpy(data.sMsg,sPtr,len-1);
	//	data.nPos = _StrToInt(sNumber+1);
	//	const char * sMinsPtr = strchr(sNumber + 1, '|');
	//	data.nMins = 5;
	//	data.nTime = 0;
	//	if (sMinsPtr != NULL)
	//	{
	//		data.nMins = _StrToInt(sMinsPtr + 1);
	//	}
	//	m_GameNotice.add(data);
	//}
	/*if (sscanf(sPtr,"%s|%d",buf,&nDis) >= 2)
	{
		
	}*/
}

void CChatManager::GmSendOffMsgToActor(LPCSTR nActorName,LPCSTR nMsg)
{
	if (!nActorName || !nMsg) return;

	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));

	packet.writeString(nMsg);
	Assert(packet.getPosition() <= MAX_MSG_COUNT);

	CMsgSystem::AddOfflineMsgByName((char *)nActorName,CMsgSystem::mtReturnSuggestMsg,MsgBuff,packet.getPosition(),0);

}

void CChatManager::MgrAddFilterWords(char* strWords,int nParam,int nTag,int nRecog)
{
	if(strWords == NULL) return;

	if(nTag == 2)				//添加或者删除屏蔽字的数据表里
	{
		CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcAddFilterWordsMsg);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		DataPacket << (int)nParam;
		DataPacket << (int)nRecog;	//后台通信标示
		DataPacket.writeString(strWords);
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
	else if(nTag == 1)			//添加到屏蔽字库
	{
		if(nParam == 1)
		{
			int nResult = 0;
			
			if(IsStrInvalid(strWords) == true)
			{
#ifdef USE_HASK_FILTER
				m_hashFilter.AddFilterWord(strWords);
#else
				AddMBCSFilterStrToTable(m_pFilterWords,strWords);
#endif
				nResult = 0;
			}
			else
			{
				nResult = 1;
			}
		}
	}
}

bool CChatManager::AddKeyword( char* pKeyword, int mode){
	const size_t nLen(strlen(pKeyword));
	int nMatchLen(0);

	if (1 == mode){
		if(!(!IsStrInvalid(pKeyword, mode, &nMatchLen) && nLen == nMatchLen))	// 不存在，才添加
		{
			return 
#ifdef USE_HASK_FILTER
			m_hashFilter.AddFilterWord(pKeyword)
#else
			AddMBCSFilterStrToTable(m_pFilterWords,pKeyword)
#endif
			&& AppendLine2File(filterfile, pKeyword)/* && ReloadFilterDb()*/;
		}
	}else if (2 == mode){
		if(!(!IsStrInvalid(pKeyword, mode, &nMatchLen) && nLen == nMatchLen))
		{
			return
#ifdef USE_HASK_FILTER
			m_forbitWards.AddFilterWord(pKeyword)
#else
			AddMBCSFilterStrToTable(m_pForbitWords,pKeyword)
#endif
			&& AppendLine2File(forbitfile, pKeyword)/* && ReloadFilterDb()*/;
		}
	}
	return false;
}

inline
bool CChatManager::AppendLine2File(const char* pFileName, const char* pLine){
	FILE* fp = fopen(pFileName,"a+");
	if(!fp)return false;
	bool ret = !fputs(pLine, fp) && !fputs("\n", fp);
	fclose(fp);
	return ret;
}

VOID CChatManager::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId, nParam, nRecog = 0;
	CDataPacketReader inPacket(data,size);
	BYTE nErrorCode;
	//char srcStr[512];
	//char strName[32];
	inPacket >> nRawServerId >> nLoginServerId >> nParam >> nRecog >> nErrorCode;

	switch(nCmd)
	{
	case dcAddFilterWordsMsg:
		{
			break;
		}
	case  dcBackRemoveItem:
		{
			ACTORNAME sName;
			CUserItem::ItemSeries guid;
			char strName[33];//后台操作用户名
			char srcStr[512];//参数字符串	
			if (nErrorCode == 0)
			{
				nErrorCode = 1;
				inPacket.readString(sName, ArrayCount(sName));
				inPacket.readString(srcStr, ArrayCount(srcStr));
				inPacket.readString(strName, ArrayCount(strName));
				inPacket >> guid;
				if(GetLogicServer()->GetLocalClient())
				{
				}
			}
			if (GetLogicServer()->GetBackClient())
				GetLogicServer()->GetBackClient()->SendCommandResult(srcStr,MSS_SET_REMOVEITEM, nErrorCode,strName,nRecog);
			break;
		}
	case dcBackRemoveMoney:
		{
			int nType = 0;
			ACTORNAME sName;
			char strName[33];//后台操作用户名
			char srcStr[512];//参数字符串	
			inPacket.readString(sName, ArrayCount(sName));
			inPacket.readString(srcStr, ArrayCount(srcStr));
			inPacket.readString(strName, ArrayCount(strName));
			inPacket >> nType;
			if (nErrorCode == 0)
			{
				nErrorCode = 1;
				if(GetLogicServer()->GetLocalClient())
				{
				}
			}
			if (GetLogicServer()->GetBackClient())
				GetLogicServer()->GetBackClient()->SendCommandResult(srcStr,MSS_SET_REMOVEMONEY, nErrorCode,strName,nRecog);

			break;
		}

	default:
		{
			break;
		}

	}
}


void CChatManager::SendNoticeToBackMgr()
{
	int nCount = (int)m_GameNotice.count();
	CDataPacket &pdata = GetLogicServer()->GetBackClient()->allocProtoPacket(SEND_NOTICE_MGR); //分配一个 网络包
	pdata << (int)GetLogicServer()->GetServerIndex();
	pdata << (int)MSS_ADDNOTICE;
	pdata << (int)nCount;

	for(int i=0;i<nCount;i++)
	{
		NOTICEDATA& data = m_GameNotice[i];
		pdata << data.nId << data.nStartTime << data.nEndTime << data.nMins << data.nPos;
		pdata.writeString(data.sMsg);
	}

	GetLogicServer()->GetBackClient()->flushProtoPacket(pdata);
}




bool CChatManager::DeleteAllNotice()
{
	m_GameNotice.clear();
	SaveNotice();
	return true;
}

bool CChatManager::DeleteNoticeById(int nId)
{
	bool result = false;
	for(int i = 0; i < m_GameNotice.count(); i++)
	{
		if (m_GameNotice[i].nId == nId)
		{
			RemoveNoticeByIndex(i);
			result = true;
			break;
		}
	}
	if (result)
	{
		SaveNotice();
	}
	return result;
}
int CChatManager::GetChatShutUpIpCount(unsigned long nIp)
{
	std::map<unsigned long,int>::iterator iter = m_chatShutUpIpCountMap.find(nIp);
	if (iter != m_chatShutUpIpCountMap.end())
	{
		return iter->second;
	}
	return 0;
}
void CChatManager::AddChatShutUpIpCount(unsigned long nIp)
{
	std::map<unsigned long,int>::iterator iter = m_chatShutUpIpCountMap.find(nIp);
	if (iter != m_chatShutUpIpCountMap.end())
	{
		iter->second ++;
	}
	else
	{
		m_chatShutUpIpCountMap.insert(std::make_pair(nIp, 1));
	}
}

void CChatManager::AddChatRecord(const char* msg, int nSize)
{
	if(msg) 
	{
		if(m_nChatRecords.size() >= CChatManager::nMax_ChatRecord)
			m_nChatRecords.erase(m_nChatRecords.begin());
		
		CHATRECORD reocrd;
		// memset(&chatMsg, 0, sizeof(chatMsg));
		memcpy(&reocrd.msg, msg, sizeof(reocrd.msg));
		reocrd.msgType = (byte)eCS_CHATRCS_TYPE_SYS;
		reocrd.msgCount = nSize;
		reocrd.msgTime = CMiniDateTime::now();
		reocrd.intVal = wrand(1000); 
		m_nChatRecords.emplace_back(reocrd);

		if (GetLogicServer()->IsCrossServer())
		{
			//跨服服务公布消息
			OnBroadTipToBranch(reocrd); 
			//OutputMsg(rmTip, "errerCode1646:em->BroadTipmsgWithParamsToCs:--------------------");
		} 
	}
}
void CChatManager::sendHistoryChatMsg(CActor* pActor)
{
	if(!pActor) return; 
	
	if (pActor->OnGetIsTestSimulator()
		|| pActor->OnGetIsSimulator() )
	{
		return;
	}
	if(m_nChatRecords.size() > 0)
	{
		for(auto& msg : m_nChatRecords) {
			CHATRECORD& it = msg;
			// CDataPacket outPack(it.msg, sizeof(it.msg)); //下发的数据包
			CActorPacket pack;
			CDataPacket & data = pActor->AllocPacket(pack);
			data.writeBuf(it.msg,sizeof(it.msg));
			pack.flush();
		}
	}
}

void CChatManager::addShowItem(CUserItem* pUserItem)
{
	if(!pUserItem) return;
	int nsize = GetLogicServer()->GetDataProvider()->GetChatSystemConfig().nShowItemSize;
	if(m_nShowItem.size() > 0 && m_nShowItem.size() >= nsize) {
		m_nShowItem.erase(m_nShowItem.begin());
	}
	CUserItem use;
	memcpy(&use, pUserItem, sizeof(use));
	m_nShowItem.emplace_back(use);
}

void CChatManager::GetShowItem(CUserItem& userItem, LONG64 iid)
{
	if(m_nShowItem.size() >0 ) {
		for(auto& it : m_nShowItem) {
			if(it.series.llId == iid) {
				userItem = it;
			}
		}
	} 
}  

//后台清理消息
bool CChatManager::OnBackClearAllMsg()
{ 
	int count = m_nChatRecords.size();

	char str[1024];

	std::vector<CHATRECORD>::iterator iter = m_nChatRecords.begin();
	for(; iter != m_nChatRecords.end();)
	{
		BYTE nTypeSystemID = 0;
		BYTE nTypeChat = 0;
		BYTE nChannelId = 0;
		CHATRECORD &tmp = (*iter);
		memcpy(&str, tmp.msg, sizeof(str));  
		CDataPacketReader data(str, sizeof(str));
		data >> nTypeSystemID >> nTypeChat >> nChannelId; 
		if(nChannelId != ciChannelTipmsg)
		{
			iter = m_nChatRecords.erase(iter);
		} 
		else
		{ 
			iter++;
		} 
	}
	return true;
}
//原服请求消息
void CChatManager::OnMakeChatRecordPacket(
	CDataPacket &pdata, 
	int nPageNum,
	UINT nNow)
{ 
	if (nPageNum == 0)//如果是首次
	{
		pdata << (int)eCS_CHATRECORDSREQ_BEGIN;
	}
	else
	{ 
		pdata << (int)eCS_CHATRECORDSREQ_MID;
	}
	int count = m_nChatRecords.size();
	INT_PTR nOffer = pdata.getPosition();  
	pdata << (int)m_nChatRecords.size();
	//OutputMsg(rmTip, "OnMakeChatRecordPacket1:---------------count----------------%d", count);
	char str[1024];
	     
	BYTE nTypeSystemID = 0;
	BYTE nTypeChat = 0;
	BYTE nChannelId = 0;
	char tmpC;
	char outstr[1024];
	int ccount = 0;
	static int gPageSize = 10; 
	int index = gPageSize * nPageNum;
	for( int i = index; i < count; ++i)
	{
		if(ccount >= gPageSize)
		{
			break;
		}
		memset(str, 0, sizeof(str)); 
		memset(outstr, 0, sizeof(outstr)); 

		CHATRECORD &tmp = m_nChatRecords[i]; 
		memcpy(&str, tmp.msg, sizeof(str)); 

		CDataPacketReader data(str, sizeof(str));
		data >> nTypeSystemID >> nTypeChat >> nChannelId; 
		if(nChannelId != ciChannelTipmsg)
		{
			continue;
		}
		data >> tmpC; 
		data >> tmpC; 
		data >> tmpC; 
		data.readString(outstr, ArrayCount(outstr)); 
		std::string sstr = outstr;
		
		pdata << tmp.llId;
		pdata << tmp.msgCount; 
		pdata.writeString(sstr.c_str());
		//OutputMsg(rmTip, "OnMakehatRecordPacket:--------------->>>>>>>>>>>-------第--%d个key---- Time: %d,---%s", i, tmp.msgTime, sstr.c_str());
		ccount++;
	}
	if ( (ccount + index) < count)
	{
		int nextPage = (ccount + index)/ gPageSize; 
		//OutputMsg(rmTip, "OnMakehatRecordPacket:--------------->>>>>>>>>>>------ 有继续包: nextPage%d", nextPage);
		pdata << (int)nextPage; 
	}
	else
	{
		//OutputMsg(rmTip, "OnMakehatRecordPacket:--------------->>>>>>>>>>>------ 无继续包" );
		pdata << (int)0; 
	} 

	INT_PTR nOfferEnd = pdata.getPosition();  
	pdata.setPosition(nOffer);
	pdata << ccount;
	pdata.setPosition(nOfferEnd);
}
//历史包
void CChatManager::OnAddChatRecordPacketByCs(CDataPacketReader &pdata)
{ 
	int first = 0;
	pdata >> first;
	if (first == (int)eCS_CHATRECORDSREQ_BEGIN)
	{
		//OutputMsg(rmTip, "CChatManager::+++++++++++++++++++++++++++++++%----- 首包" );
		m_nChatCsRecords.clear();
		m_mapChatCs.clear();
	}
	else
	{ 
		//OutputMsg(rmTip, "CChatManager::+++++++++++++++++++++++++++++++%----- 次包" );
	}
	
	char str[1024];
	int count = 0;
	pdata >> count;
	int msgCount = 0;			 //内容长度
	unsigned int msgTime = 0;	 //创建时间
	//OutputMsg(rmTip, "OnAddChatRecordPacketByCs1:+++++++++++++++++++++++++++++++%d", count);
	for( int i = 0; i < count; ++i)
	{ 
		CHATRECORD tmp;
		pdata >> tmp.llId;
		pdata >> tmp.msgCount; 
		tmp.msgType = (byte)eCS_CHATRCS_TYPE_SYS; //这个参数就不传了，直接设置
		memset(str, 0, sizeof(str));
		pdata.readString(str, ArrayCount(str));
		std::string sstr = str; 
		memcpy(&tmp.msg, sstr.c_str(), sstr.length());  
 
		if (!m_mapChatCs.insert(std::make_pair(tmp.llId, tmp.msgTime)).second)
		{
			continue;
		}
		
		AddChatCsRecord(tmp);  
		//OutputMsg(rmTip, "OnAddChatRecordPacketByCs:+++++++++++++++++++++++Time: %d++++++++%s ,", tmp.msgTime, sstr.c_str());
		//AddChatCsRecord(sstr.c_str(), sstr.length());
		
		//GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(const_cast<char*>(sstr.c_str()), sstr.length()); 
	}

	int nextPage = 0;
	pdata >> nextPage;
	//OutputMsg(rmTip, "CChatManager::+++++++++++++++++++++++++++++++%-----需要继续 %d " ,nextPage);
	/* 服务器总是有偶发的 一直发送消息
	if (nextPage > 0)//根据需要发送分页包
	{
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();  
		CLocalCrossClient *pCSClient = GetLogicServer()->GetCrossClient();
		CDataPacket &packet = pCSClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendReqChat);
		int nServerId = GetLogicServer()->GetCrossServerId();
		//OutputMsg(rmTip, _T("[CrossLogin] CChatManager ") );
		packet << nServerId ;
		packet << (int)nextPage;//发送0 要全部数据
		packet << nNow;//发送时间

		pCSClient->flushProtoPacket(packet);
	}
	else
	*/
	{
		// //最后一个包加上战斗信息 
		// int fightCount = m_nCsFightRecords.size(); 
		// OutputMsg(rmTip, "CChatManager::OnAddChatRecordPacketByCs:m_nCsFightRecords:--------------------FightRecords: %d", fightCount);
		// for( int k = 0; k < fightCount; ++k)
		// { 
		// 	CHATRECORD &tmp = m_nCsFightRecords[k];     
		// 	if (!m_mapChatCs.insert(std::make_pair(tmp.llId, tmp.msgTime)).second)
		// 	{
		// 		continue;
		// 	}
			
		// 	AddChatCsRecord(tmp);  
		// }
		//没有分包就排序下
		std::sort(m_nChatCsRecords.begin(), m_nChatCsRecords.end(), ChatCsRecordSort);
	}
}

bool CChatManager::AddChatCsRecord(CHATRECORD& Record)
{ 
	int ccount = m_mapChatCs.size();
	if(m_nChatCsRecords.size() >= CChatManager::nMax_ChatRecord)
	{
		std::vector<CHATRECORD>::iterator it = m_nChatCsRecords.begin();
 
		std::map<LONGLONG, unsigned int>::iterator iter = m_mapChatCs.find((*it).llId);
		if (iter != m_mapChatCs.end())
		{
			m_mapChatCs.erase(iter);
		}

		m_nChatCsRecords.erase(it);
	}
	
	ccount = m_mapChatCs.size();	
	m_nChatCsRecords.emplace_back(Record);
	//m_mapChatCs.insert(std::make_pair(Record.llId, Record.msgTime)); 
	return false;
} 
// void CChatManager::AddFightRecord(const char* msg, int nSize)
// {
// 	if(msg) 
// 	{
// 		if(m_nCsFightRecords.size() >= CChatManager::nMax_ChatRecord)
// 			m_nCsFightRecords.erase(m_nCsFightRecords.begin());
		
// 		CHATRECORD reocrd;
// 		// memset(&chatMsg, 0, sizeof(chatMsg));
// 		memcpy(&reocrd.msg, msg, sizeof(reocrd.msg));
// 		reocrd.msgType = (byte)eCS_CHATRCS_TYPE_FIGHT;
// 		reocrd.msgCount = nSize;
// 		reocrd.msgTime = CMiniDateTime::now();
// 		reocrd.intVal = wrand(1000); 
// 		m_nCsFightRecords.emplace_back(reocrd);

// 		std::string sstr = reocrd.msg;
// 		OutputMsg(rmTip, "CChatManager::AddFightRecord:--------------------Time: %d---%s ,", reocrd.msgTime, sstr.c_str());
// 	} 
// }
// 带参数全服广播 跨服--->原服
//跨服发给原服
void CChatManager::OnBroadTipToBranch(CHATRECORD& reocrd)
{  
	int nLength = sizeof(CHATRECORD);
	std::string strBuff = "";
	strBuff.resize(nLength + 100);
	///char buff[4196];
	//memset(buff, 0, sizeof(buff));
	CDataPacket outPack(strBuff.data(), (nLength + 100));
	//CDataPacket &outPack = allocSendPacket(); 
	outPack << (unsigned int)nLength; 
	outPack.writeBuf(&reocrd, nLength);

	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sSendBroadSysTipmsg, outPack.getMemoryPtr(), outPack.getPosition()); 
}
//新收到的跨服pk消息
bool CChatManager::OnCsAddPkMsgRecord(CDataPacketReader &pdata)
{ 
	if(!OnCheckCanMsg())
	{  
		return true; 
	}

	unsigned int nLength = 0; 
	pdata >> nLength;  
	
	std::string strBuff = "";
	strBuff.resize(nLength + 100); 
	//char buff[5024];
	//memset(buff, 0, sizeof(buff)); 
	pdata.readBuf((char *)strBuff.data(), nLength);
	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast((char *)strBuff.data(), nLength);
	//加入到本地战斗消息缓存 
 
	CDataPacketReader subpacket(strBuff.data(), nLength); 
	BYTE nChatSystemID = 0;
	BYTE nSendTipmsg = 0;
	WORD nTipmsgType = 0; 
	WORD nTipmsgCOunt = 0; 
	subpacket >> nChatSystemID >> nSendTipmsg >> nTipmsgType; 
	subpacket >> nTipmsgCOunt;
	std::string strBuff1 = "";
	strBuff1.resize(nTipmsgCOunt + 100);  
	subpacket.readBuf((char *)strBuff1.data(), nTipmsgCOunt); 
	if(nTipmsgCOunt > 0) 
	{ 
		CHATRECORD reocrd;
		// memset(&chatMsg, 0, sizeof(chatMsg));
		memcpy(&reocrd.msg, strBuff1.data(), nTipmsgCOunt);
		reocrd.msgType = (byte)eCS_CHATRCS_TYPE_FIGHT;
		reocrd.msgCount = nTipmsgCOunt;
		reocrd.msgTime = CMiniDateTime::now();
		reocrd.intVal = wrand(1000);  
		if (!GetLogicServer()->IsCrossServer())
		{
			if (!m_mapChatCs.insert(std::make_pair(reocrd.llId, reocrd.msgTime)).second)
			{
				OutputMsg(rmTip, "CChatManager::OnCsAddPkMsgRecord:--------------------Time: %d---%s ,", reocrd.msgTime, strBuff1.c_str());
				return false;
			}
			AddChatCsRecord(reocrd);//添加到跨服队列
			OnSendPkCsChat(reocrd);	//原服发送Pk闪烁消息
		} 
	} 
	return true;
}
//新收到的跨服广播消息
bool CChatManager::OnCsAddSysMsgRecord(CDataPacketReader &pdata)
{
	if(!OnCheckCanMsg())
	{  
		return true; 
	}

	unsigned int nLength = 0; 
	pdata >> nLength;

	//std::string strBuff = "";
	//strBuff.resize(nLength + 100); 
	if(nLength != sizeof(CHATRECORD))
	{
		OutputMsg(rmTip, "CChatManager::OnCsAddSysMsgRecord:SIZE errer:--------------------mgs SIZE: %d", nLength);
		return false;
	}
	//char subBuff[5024];
	//memset(subBuff, 0, sizeof(subBuff)); 
	//CDataPacketReader packet(strBuff.data(), (nLength + 100)); 
	CHATRECORD reocrd;
	pdata.readBuf(&reocrd, nLength); 
	std::string str = reocrd.msg;//测试
 
	if (!GetLogicServer()->IsCrossServer())
	{
		/*if (!m_mapChatCs.insert(std::make_pair(reocrd.llId, reocrd.msgTime)).second)
		{
			std::string sstr = reocrd.msg; 
			OutputMsg(rmTip, "CChatManager::OnCsAddSysMsgRecord:Insert errer:--------------------mgs Time: %d---%s", reocrd.msgTime, sstr.c_str());
			return false;
		}
		*/
		//AddChatCsRecord(reocrd);//添加到跨服队列 
		OnSendCsSysChat(reocrd);
	}

	return true;
}
//原服发给客户端 历史 //用不到了
void CChatManager::OnSendHistoryCsSysMsg(CActor* pActor)
{   
	char strBuff[10240];
	memset(strBuff, 0, sizeof(strBuff));
	CActorPacket ap;
	CDataPacket& outPack = pActor->AllocPacket(ap);
	//CDataPacket outPack(strBuff, sizeof(strBuff));
	outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
	int ccount = 0;
	INT_PTR nOffer = outPack.getPosition(); 
	outPack << ccount;  
	for (size_t i = 0; i < m_nChatCsRecords.size(); i++)
	{ 
		//if(nMsgTime != 0 && m_nChatCsRecords[i].msgTime <= nMsgTime)
		//{
		//	continue;
		//} 
		outPack << (byte) m_nChatCsRecords[i].msgType;  
		outPack << (unsigned int) m_nChatCsRecords[i].msgTime; 
		std::string sstr = m_nChatCsRecords[i].msg; 
		outPack.writeString(sstr.c_str());
		ccount++;
	}
	INT_PTR nOffer2 = outPack.getPosition();  
	outPack.setPosition(nOffer);
	outPack << ccount;  
	outPack.setPosition(nOffer2); 
	ap.flush();  
} 
//原服发送Pk闪烁消息
bool CChatManager::OnSendPkCsChat(CChatManager::CHATRECORD& record)
{  
	//发送数据
	std::string strSendBuff = "";
	strSendBuff.resize(sizeof(record) + 100);   
	CDataPacket outPack((char *)strSendBuff.data(), sizeof(record) + 100);
	//CDataPacket& outPack = m_pEntity->AllocPacket(ap);
	//CDataPacket outPack(strBuff, sizeof(strBuff));
	outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
	int ccount = 0;
	INT_PTR nOffer = outPack.getPosition(); 
	outPack << 1;   
	{   
		outPack << (byte) record.msgType;  
		outPack << (unsigned int) record.msgTime;  
		std::string sstr = record.msg;
		outPack.writeString(sstr.c_str()); 
	}
	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());

	return true;
}
//原服发给客户端
bool CChatManager::OnSendCsSysChat(CChatManager::CHATRECORD& record)
{
	BYTE nTypeSystemID = 0;
	BYTE nTypeChat = 0;
	BYTE nChannelId = 0;
	char tmpC;  
	std::string str_record = "";
	str_record.resize(sizeof(record.msg) + 1);  
	memcpy((char *)str_record.data(), record.msg, sizeof(record.msg)); 

	CDataPacketReader data((char *)str_record.data(), sizeof(record.msg) + 1);
	data >> nTypeSystemID >> nTypeChat >> nChannelId; 
	if(nChannelId != ciChannelTipmsg)
	{
		OutputMsg(rmTip, "CChatManager::OnSendCsSysChat:errer--------------------nChannelId: %d ,", (int)nChannelId);
		return false;
	}
	data >> tmpC; 
	data >> tmpC; 
	data >> tmpC; 
	
	//WORD pMsgLen = 0;
	//data >> pMsgLen; 
	std::string outstr_record = ""; 
	outstr_record.resize(1025);
	data.readString((char *)outstr_record.data(), 1025); 
 
	//保存数据
	CHATRECORD sys_reocrd;
	// memset(&chatMsg, 0, sizeof(chatMsg));
	memcpy(&sys_reocrd.msg, outstr_record.data(), 1025);
	std::string sstr = sys_reocrd.msg;
	sys_reocrd.msgType = (byte)eCS_CHATRCS_TYPE_SYS;
	sys_reocrd.msgCount = sstr.length();
	sys_reocrd.msgTime = CMiniDateTime::now();//逻辑服自己规划时间
	sys_reocrd.intVal = wrand(1000);   
	
	int chatcount = m_mapChatCs.size();
	if (!m_mapChatCs.insert(std::make_pair(sys_reocrd.llId, sys_reocrd.msgTime)).second)
	{
		OutputMsg(rmTip, "CChatManager::OnSendCsSysChat:--------------------Time: %d---%s ,", sys_reocrd.msgTime, outstr_record.c_str());
		return false;
	}
	AddChatCsRecord(sys_reocrd);//添加到跨服队列 
 
	//发送数据
	std::string strSendBuff = "";
	strSendBuff.resize(sizeof(sys_reocrd) + 100);   
	CDataPacket outPack((char *)strSendBuff.data(), sizeof(sys_reocrd) + 100);
	//CDataPacket& outPack = m_pEntity->AllocPacket(ap);
	//CDataPacket outPack(strBuff, sizeof(strBuff));
	outPack << (BYTE) enChatSystemID << (BYTE)sSendReqCsChat <<(BYTE)ciChannelTipmsg; 
	int ccount = 0;
	INT_PTR nOffer = outPack.getPosition(); 
	outPack << 1;   
	{  
		outPack << (byte) sys_reocrd.msgType;  
		outPack << (unsigned int) sys_reocrd.msgTime;  
		outPack.writeString(sstr.c_str()); 
	}
	GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
	return true;
}
//是否可以跨服消息
bool CChatManager::OnCheckCanMsg()
{ 
	if(GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCrossMsgDay > 0)
	{ 
		//如果配置了限制跨服信息 
		int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
		if (nOpenServerDay < GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCrossMsgDay)
		{
			return false;
		}
	} 
	return true;
}