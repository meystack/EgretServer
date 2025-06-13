#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#endif 
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include <CustomSocket.h>
#include "ShareUtil.h"
#include "BufferAllocator.h"
#include "AppItnMsg.h"
#include "DataPacket.hpp"
#include "SendPackPool.h"
#include "CustomWorkSocket.h"
#include "CustomClientSocket.h"
#include "ServerDef.h"
#include "LogType.h"
//#include "encrypt\CRC.h"
//#include "encrypt\Encrypt.h"
//#include "dataProcess\NetworkDataHandler.h"
#include "CustomJXClientSocket.h"
#include "LogSender.h"


using namespace jxSrvDef;

LogSender::LogSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName)
{
	this->ServerType = ServerType;
	this->ServerIndex = ServerIndex;
	strcpy(this->ServerName,ServerName);
	this->SetClientName(_T("日志"));
}

LogSender::LogSender()
{
	this->SetClientName(_T("日志"));
}

LogSender::~LogSender(void)
{
}

//设置服务器的编号
VOID LogSender::SetServerIndex(const INT nSerIdx)
{
	ServerIndex = nSerIdx;
}

VOID LogSender::SetServerName(LPCTSTR sServerName)
{
	_tcsncpy(ServerName, sServerName, ArrayCount(ServerName) - 1);
	ServerName[ArrayCount(ServerName) - 1] = 0;
}

void LogSender::SetServerSpid(LPCTSTR sSpid)
{
	_tcsncpy(m_sSpid, sSpid, ArrayCount(m_sSpid) - 1);
	m_sSpid[ArrayCount(m_sSpid) - 1] = 0;
}

VOID LogSender::SendLoginLog(WORD 	nLogid,int nUserid,LPCTSTR szAccount,
				LPCTSTR szLoginIp,LPCTSTR szLoginDescr, int nLevel,  int nYb, unsigned int nOnlineTime, unsigned int nActorId,unsigned int nSrvId)
{
	LOGDATALOGIN data;
	data.nCmdId = LOG_LOGIN_CMD;
	data.nLogid = nLogid;
	data.nUserid = nUserid;
	data.nLevel = nLevel;
	data.nYb = nYb;
	data.nOnlineTime = nOnlineTime;
	if (szAccount != NULL)
	{
		_asncpytA((data.szAccount),szAccount);
	}
	else
	{
		data.szAccount[0] = 0;
	}
	
	data.nServerIndex = ServerIndex;
	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
		
	if (szLoginIp != NULL)
	{
		_asncpytA((data.szLoginIp),szLoginIp);
	}
	else
	{
		data.szLoginIp[0] = 0;
	}
	data.nServerType = ServerType;
	if (szLoginDescr != NULL)
	{
		_asncpytA((data.szLoginDescr),szLoginDescr);				
	}
	else
	{
		data.szLoginDescr[0] = 0;
	}
	//_asncpytA(data.szSpid, m_sSpid);
	snprintf(data.szSpid, sizeof(data.szSpid), "%d", nActorId);

	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送登陆日志：CmdId=%d,Logid=%d,Userid=%d，Account=%s,ServerIndex=%d,LoginIp=%s,ServerType=%d,LoginDescr=%s"), 
	//	data.nCmdId,data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,data.szLoginIp,data.nServerType,data.szLoginDescr);
}

VOID LogSender::SendConsumeLog(WORD nLogid,int nActorid,LPCTSTR szAccount,
					LPCTSTR szCharName,BYTE nMoneytype,int nCount,int nPaymentCount,LPCTSTR szConsumeDescr,int nLevel, int nBalance, unsigned int nSrvId)
{
	LOGDATACONSUME data;
	data.nCmdId = LOG_CONSUME_CMD;
	data.nLogid = nLogid;
	data.nActorid = nActorid;
	data.nConsumeLevel =nLevel; //玩家的消费的等级
	if (szAccount != NULL)
	{
		_asncpytA((data.szAccount),szAccount);
	}
	else
	{
		data.szAccount[0] = 0;
	}
	data.nServerIndex = ServerIndex;

	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
		
	if (szCharName != NULL)
	{
		_asncpytA((data.szCharName),szCharName);
	}
	else
	{
		data.szCharName[0] = 0;
	}
	data.cMoneytype = nMoneytype;
	data.nBalance = nBalance;
	data.nConsumecount = nCount;
	data.nPaymentCount = nPaymentCount;
	if (szConsumeDescr != NULL)
	{
		_asncpytA((data.szConsumeDescr),szConsumeDescr);
	}
	else
	{
		data.szConsumeDescr[0] = 0;
	}
	//_asncpytA(data.szSpid, m_sSpid);
	snprintf(data.szSpid, sizeof(data.szSpid), "%d", nActorid);
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送消费日志：CmdId=%d,Logid=%d,Userid=%d，Account=%s,ServerIndex=%d,CharName=%s,ConsumeCount=%d,PaymentCount=%d,ConsumeDescr=%s"), 
	//	data.nCmdId,data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,data.szCharName,data.nConsumeCount,data.nPaymentCount,data.szConsumeDescr);
}

VOID LogSender::SendItemLocalLog(
	BYTE nType,
	WORD nLogid,
	int nActorid, 
	int nAccountid, 
	LPCTSTR szAccount,
	LPCTSTR szCharName,
	WORD wItemId,
	int nCount,
	LPCTSTR szConsumeDescr, 
	unsigned int nSrvId)
{
	LOGDATAITEM data;
	data.nCmdId = LOG_ITEAM_CMD;
	data.nLogid = nLogid;
	data.nActorid = nActorid;
	data.nType = nType;
	if (szAccount != NULL)
	{
		_asncpytA((data.szAccount),szAccount);
	}
	else
	{
		data.szAccount[0] = 0;
	}
	data.nServerIndex = ServerIndex;	
	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
	
	if (szCharName != NULL)
	{
		_asncpytA((data.szCharName),szCharName);
	}
	else
	{
		data.szCharName[0] = 0;
	}
	data.wItemId = wItemId;
	data.nCount = nCount;

	if (szConsumeDescr != NULL)
	{
		_asncpytA((data.szConsumeDescr),szConsumeDescr);
	}
	else
	{
		data.szConsumeDescr[0] = 0;
	}
	//_asncpytA(data.szSpid, m_sSpid);
	snprintf(data.szSpid, sizeof(data.szSpid), "%d", nAccountid);
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送消费日志：CmdId=%d,Logid=%d,Userid=%d，Account=%s,ServerIndex=%d,CharName=%s,ConsumeCount=%d,PaymentCount=%d,ConsumeDescr=%s"), 
	//	data.nCmdId,data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,data.szCharName,data.nConsumeCount,data.nPaymentCount,data.szConsumeDescr);
}




VOID LogSender::SendDealToLog(int nLogid, unsigned int nTime,
	int nActorid,int nTarActorid,
	INT nItemId,WORD nItemNum,WORD nMoneyType,INT64 nMoneyNum, int nDues, unsigned int nSrvId)
{
	LOGACTORDEALSAVE tmpLog;

	tmpLog.nLogId = nLogid;
	tmpLog.nMiniTime = nTime;

	tmpLog.nActorid = nActorid;
	tmpLog.nTarActorid = nTarActorid;

	tmpLog.nItemId = nItemId;
	tmpLog.nItemNum = nItemNum;
	tmpLog.nMoneyType = nMoneyType;
	tmpLog.nMoneyNum = nMoneyNum;
	tmpLog.nDues = nDues;
	tmpLog.nServerIndex = ServerIndex;
	tmpLog.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		tmpLog.nServerIndex = nSrvId;
	}
	_asncpytA(tmpLog.szSpid, m_sSpid);

	CDataPacket &pdata = allocProtoPacket(LOG_DEAL_CMD);
	pdata << tmpLog;
	flushProtoPacket(pdata);
}

VOID LogSender::SendOnlineLog(int nServerIndex,int nCount)
{
	LOGDATAONLINECOUNT data;
	data.nCmdId = LOG_ONLINE_CMD;
	data.nServerIndex = nServerIndex;
	data.nOnlineCount = nCount;
	_asncpytA(data.szSpid, m_sSpid);
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送在线日志：CmdId=%d,ServerIndex=%d,OnlineCount=%d"), 
	//	data.nCmdId,data.nServerIndex,data.nOnlineCount);
}
VOID LogSender::SendSuggestLog( int nType,char * sActorName,char * sAccount, char * sTitle, char * sDetail,int nLevel, unsigned int nSrvId)
{
	LOGGERDATASUGGEST data;
	data.nCmdId = LOG_SUGGEST_CMD;
	if (sActorName != NULL)
	{
		_asncpytA((data.szCharName),sActorName);
	}
	else
	{
		data.szCharName[0] = 0;
	}

	if (sAccount != NULL)
	{
		_asncpytA((data.szAccount),sAccount);
	}
	else
	{
		data.szAccount[0] = 0;
	}

	if (sTitle != NULL)
	{
		_asncpytA((data.szTitle),sTitle);
	}
	else
	{
		data.szTitle[0] = 0;
	}

	if (sDetail != NULL)
	{
		_asncpytA((data.szSuggest),sDetail);
	}
	else
	{
		data.szSuggest[0] = 0;
	}

	data.nType = (WORD)nType;
	data.nServerIndex= (WORD)ServerIndex;
	data.nActorLevel = nLevel;
	
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);

}
VOID LogSender::SendCommonLog(WORD nLogid,int nUserid,LPCTSTR szAccount,
					int nPara0,int nPara1,int nPara2,
					LPCTSTR szShortStr0,LPCTSTR szMidStr0,LPCTSTR szLongStr0,
					LPCTSTR szShortStr1,LPCTSTR szMidStr1,LPCTSTR szLongStr1,
					LPCTSTR szMidStr2,LPCTSTR szLongStr2, unsigned int nSrvId)
{
	LOGGERDATACOMMON data;
	data.nCmdId = LOG_COMMON_CMD;
	data.nLogid = nLogid;
	data.nActorid = nUserid;
	if (szAccount != NULL)
	{
		_asncpytA((data.szAccount),szAccount);
	}
	else
	{
		data.szAccount[0] = 0;
	}
	data.nServerIndex = ServerIndex;
	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
	data.nPara0 = nPara0;
	data.nPara1 = nPara1;
	data.nPara2 = nPara2;
	if (szShortStr0 != NULL)
	{
		_asncpytA((data.szShortStr0),szShortStr0);
	}
	else
	{
		data.szShortStr0[0] = 0;
	}
	if (szShortStr1 != NULL)
	{
		_asncpytA((data.szShortStr1),szShortStr1);
	}
	else
	{
		data.szShortStr1[0] = 0;
	}
	if (szMidStr0 != NULL)
	{
		_asncpytA((data.szMidStr0),szMidStr0);
	}
	else
	{
		data.szMidStr0[0] = 0;
	}
	if (szMidStr1 != NULL)
	{
		_asncpytA((data.szMidStr1),szMidStr1);
	}
	else
	{
		data.szMidStr1[0] = 0;
	}
	if (szMidStr2 != NULL)
	{
		_asncpytA((data.szMidStr2),szMidStr2);
	}
	else
	{
		data.szMidStr2[0] = 0;
	}
	if (szLongStr0 != NULL)
	{
		_asncpytA((data.szLongStr0),szLongStr0);
	}
	else
	{
		data.szLongStr0[0] = 0;
	}
	if (szLongStr1 != NULL)
	{
		_asncpytA((data.szLongStr1),szLongStr1);
	}
	else
	{
		data.szLongStr1[0] = 0;
	}
	if (szLongStr2 != NULL)
	{
		_asncpytA((data.szLongStr2),szLongStr2);
	}
	else
	{
		data.szLongStr2[0] = 0;
	}
	_asncpytA(data.szSpid, m_sSpid);
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送公共日志：CmdId=%d,Logid=%d,Userid=%d，Account=%s,ServerIndex=%d,para0=%d,para1=%d,para2=%d,ShortStr0=%s,ShortStr1=%s,MidStr0=%s,MidStr1=%s,MidStr2=%s,LongStr0=%s,LongStr1=%s,LongStr2=%s"), 
	//	data.nCmdId,data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,data.nPara0,data.nPara1,data.nPara2
	//	,data.szShortStr0,data.szShortStr1,data.szMidStr0,data.szMidStr1,data.szMidStr2,data.szLongStr0,data.szLongStr1,data.szLongStr2);
}

VOID LogSender::SendChatLog(int nChannleID, char * sActorName,unsigned int nActorId, char * msg)
{
	if(!sActorName || !msg) return;

	if(m_nFlag)
	{
		CDataPacket &pdata = allocProtoPacket(LOG_SENDCHATRECORD_CMD); //分配一个 网络包
		pdata << (int)nChannleID;
		pdata << (unsigned int)nActorId;
		pdata.writeString(sActorName);
		pdata.writeString(msg);

		flushProtoPacket(pdata);

	}
}

VOID LogSender::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	//空函数，本身不处理日志服务器下发的消息
	switch(nCmd)
	{
	case LOG_LOOKCHATMSG_CMD:
		{
			int nFlag = 0;
			inPacket >> nFlag;
			SetChatLogFlag(nFlag);
			break;
		}
	}
}

jxSrvDef::SERVERTYPE LogSender::getLocalServerType()
{
	return ServerType;
}

LPCSTR LogSender::getLocalServerName()
{
	return ServerName;
}

int LogSender::getLocalServerIndex()
{
	return ServerIndex;
}

VOID LogSender::OnConnected()
{
	CDataPacket &pdata = allocProtoPacket(LOG_SENDSERVERINDEX_CMD); //分配一个 网络包
	pdata << (int)ServerIndex;
	flushProtoPacket(pdata);
}



VOID LogSender::SendKillDropLog(int nActorid,LPCTSTR sAccount,LPCTSTR sCharName,
	LPCTSTR sMonName,LPCTSTR sSceneName,LPCTSTR sItemName,int nCount, int nKilltime, int nPosX, int nPosY, unsigned int nSrvId)
{

	LOGKILLDROP data;
	
	data.nCmdId = LOG_KILLDROP_CMD;
	data.nServerIndex = ServerIndex;
	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
	data.nActorid = nActorid;
	data.nKilltime = nKilltime;
	if (sAccount != NULL)
	{
		_asncpytA((data.srtAccount),sAccount);
	}
	else
	{
		data.srtAccount[0] = 0;
	}
	if (sCharName != NULL)
	{
		_asncpytA((data.strName),sCharName);
	}
	else
	{
		data.strName[0] = 0;
	}
	if (sMonName != NULL)
	{
		_asncpytA((data.strMonsetName),sMonName);
	}
	else
	{
		data.strMonsetName[0] = 0;
	}
	if (sSceneName != NULL)
	{
		_asncpytA((data.strSceneName),sSceneName);
	}
	else
	{
		data.strSceneName[0] = 0;
	}
	if (sItemName != NULL)
	{
		_asncpytA((data.strItemName),sItemName);
	}
	else
	{
		data.strItemName[0] = 0;
	}
	data.nCount = nCount;
	data.nPosx = nPosX;
	data.nPosy = nPosY;
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
	//OutputMsg( rmTip, _T("发送公共日志：CmdId=%d,Logid=%d,Userid=%d，Account=%s,ServerIndex=%d,para0=%d,para1=%d,para2=%d,ShortStr0=%s,ShortStr1=%s,MidStr0=%s,MidStr1=%s,MidStr2=%s,LongStr0=%s,LongStr1=%s,LongStr2=%s"), 
	//	data.nCmdId,data.nLogid,data.nUserid,data.szAccount,data.nServerIndex,data.nPara0,data.nPara1,data.nPara2
	//	,data.szShortStr0,data.szShortStr1,data.szMidStr0,data.szMidStr1,data.szMidStr2,data.szLongStr0,data.szLongStr1,data.szLongStr2);
}




VOID LogSender::SendJoinAtvToLog(int nActorid,LPCTSTR sAccount,LPCTSTR sCharName,int nAtvId, int nIndex, int nJoinTime, unsigned int nSrvId)
{

	LOGATVSCHEDULE data;
	
	data.nCmdId = LOG_JOINATV_CMD;
	data.nServerIndex = ServerIndex;
	data.nSrvId = ServerIndex;
	if(nSrvId > 0)
	{
		data.nServerIndex = nSrvId;
	}
	data.nActorid = nActorid;
	data.nAtvID = nAtvId;
	if (sAccount != NULL)
	{
		_asncpytA((data.srtAccount),sAccount);
	}
	else
	{
		data.srtAccount[0] = 0;
	}
	if (sCharName != NULL)
	{
		_asncpytA((data.strName),sCharName);
	}
	else
	{
		data.strName[0] = 0;
	}


	data.nIndex = nIndex;
	data.nJoinTime = nJoinTime;
	////组装成网络包
	CDataPacket &pdata = allocProtoPacket(data.nCmdId); //分配一个 网络包
	pdata << data;
	flushProtoPacket(pdata);
}