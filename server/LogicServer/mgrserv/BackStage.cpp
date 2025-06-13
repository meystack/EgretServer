#include "StdAfx.h"
#include "BackStage.h"
#include "../entity/Actor.h"


using namespace jxSrvDef;
using namespace jxInterSrvComm;
using namespace jxInterSrvComm::DbServerProto;

#define LOGTIME_FLUCTUATE	60  

CBackStageSender::CBackStageSender(jxSrvDef::SERVERTYPE ServerType,INT ServerIndex,char* ServerName)
{
	this->ServerType = ServerType;
	this->ServerIndex = ServerIndex;
	strcpy(this->ServerName,ServerName);
	this->SetClientName(_T("后台"));
	
}

CBackStageSender::CBackStageSender()
{
	this->SetClientName(_T("后台"));
}

CBackStageSender::~CBackStageSender(void)
{

}

VOID CBackStageSender::SetServerName(LPCTSTR sServerName)
{
	_tcsncpy(ServerName, sServerName, ArrayCount(ServerName) - 1);
	ServerName[ArrayCount(ServerName) - 1] = 0;
}


//VOID CBackStageSender::SendCreateLocalLog(int nSpid,int Serverindex)
//{
//	CMiniDateTime curTime;
//	curTime.tv = GetGlobalLogicEngine()->getMiniDateTime();
//	if (curTime.tv >= m_tomorrowDateTime.tv)
//	{
//		// 更新Next New Day Timepoint
//		//m_tomorrowDateTime.tv += 2 * TIME_FLUCTUATE;
//		m_tomorrowDateTime.tv = m_tomorrowDateTime.rel_tomorrow() + wrand(LOGTIME_FLUCTUATE);	// 24点后TIME_FLUCTUATE秒分布
//
//		CDataPacket &pdata = allocProtoPacket(LOG_CREATETABLE_CMD); //分配一个 网络包
//		pdata << (int)nSpid;
//		pdata << (int)Serverindex;
//		flushProtoPacket(pdata);
//	}
//}

VOID CBackStageSender::OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket)
{
	switch(nCmd)
	{
	case SEND_COMMAND:
		{
			RevBackCommandOp(inPacket);
			break;
		}

	case GET_NOTICE_MGR:
		{
			GetGlobalLogicEngine()->GetChatMgr().SendNoticeToBackMgr();
			break;
		}

	default:
		break;
	}
}

jxSrvDef::SERVERTYPE CBackStageSender::getLocalServerType()
{
	return ServerType;
}

LPCSTR CBackStageSender::getLocalServerName()
{
	return ServerName;
}

int CBackStageSender::getLocalServerIndex()
{
	return ServerIndex;
}

 VOID CBackStageSender::OnConnected()
 {
	 SendSpidToBack();
	 SendServerState();
	 GetGlobalLogicEngine()->GetChatMgr().SendNoticeToBackMgr();
 }

 void CBackStageSender::SendServerState()
 {
	 CDataPacket &pdata = allocProtoPacket(REV_LOGICSERVERINFO); //分配一个 网络包
	 pdata << (int)ServerIndex;

	 char* pMsg = NULL;
	 pdata.writeString(pMsg?pMsg:"");

	 flushProtoPacket(pdata);

 }

 void CBackStageSender::SendSpidToBack()
 {
	 CDataPacket &pdata = allocProtoPacket(SEND_LOGICSPID); //分配一个 网络包
	 pdata << (int)GetLogicServer()->GetSpid();

	 flushProtoPacket(pdata);
 }

 void CBackStageSender::SendCommandResult(char* strCmmand,int nCmd,int nResult,char* sName,int nId)
 {
	 CDataPacket &pdata = allocProtoPacket(SEND_COMMAND_RESULT); //分配一个 网络包
	 pdata << (int)ServerIndex;
	 pdata << (int)nCmd;
	 pdata << (int)nResult;
	 pdata << (int)nId;
	 pdata.writeString(sName?sName:"");
	 pdata.writeString(strCmmand?strCmmand:"");
	 flushProtoPacket(pdata);

 }

 void CBackStageSender::RevBackCommandOp(CDataPacketReader &inPacket)
 {
	 int nServerIndex = 0;
	 inPacket >> nServerIndex;

	 int nOper = 0;
	 inPacket >> nOper;

	 int nId = 0;
	 inPacket >> nId;

	 //char sName[32];
	 char* sName = new char[32]; //(char*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(32); // 在 void CBackStageSender::HandleCommand(INT_PTR nCmd, UINT_PTR data,UINT_PTR sName,UINT_PTR nId)释放
	 inPacket.readString(sName, 32);

	 //char strCom[512];
	 char* strCom = new char[1280]; // (char*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(512); //在 void CBackStageSender::HandleCommand(INT_PTR nCmd, UINT_PTR data,UINT_PTR sName,UINT_PTR nId)释放
	 inPacket.readString(strCom, 1280);

	 OutputMsg(rmTip, "RevBackCommandOp! sName=%s",sName);

	 //这里把cmd和数据包转发过去
	 CLogicServer::s_pLogicEngine->GetNetWorkHandle()->PostInternalMessage(SSM_NEWBACKMGR_DATA,nOper,(INT_PTR)strCom,(INT_PTR)sName,nId);

 }

 void CBackStageSender::HandleCommand(INT_PTR nCmd, UINT_PTR data,UINT_PTR sName,UINT_PTR nId)
 {
	 char* strCom =  (char*)data;
	 if(strCom)
	 {
		 OutputMsg(rmTip, "HandleCommand! %d strCom=%s, strName=%s",nCmd, strCom, sName);
	 }
	 //result返回码，0是失败，大于0是成功
	 char srcStr[1280];
	 _asncpytA(srcStr,strCom);
	 char* stName = (char*)sName;
	 char strName[32];
	 _asncpytA(strName,stName);
	 int nResult = 0;
	 switch(nCmd)
	{ 
		case MSS_KICKPLAY: 
		{
			CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
			if (pActor)
			{
				CActorPacket pack;
				pActor->AllocPacket(pack);
				pack << (BYTE) enDefaultEntitySystemID <<(BYTE)sKictout;
				pack.flush();
			//pActor->CloseActor(lwiBackStageTickActor, false); //关闭玩家的连接
				nResult = 1;
			}
			else
			{
				nResult = 2;
			}
			break;
		} 
		case MSS_KICKUSER:
		{
			CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByAccountName(strCom);
			if (pActor)
			{
				CActorPacket pack;
				pActor->AllocPacket(pack);
				pack << (BYTE) enDefaultEntitySystemID <<(BYTE)sKictout;
				pack.flush();
				//pActor->CloseActor(lwiBackStageTickAccount, false); //关闭玩家的连接
				nResult = 1;
			}
			else
			{
				nResult = 2;
			}
			break;
		} 
		case MSS_KICK_USER_BY_ID:
		{
			int sTime = 0;
			int UserId = 0; 
			std::vector<std::string> results = SplitStr(strCom, "|");
			int nCount = results.size();
			if (nCount != 2)
			{	
				nResult = 0;//参数非法 
				break;
			}
			
			for (int i = 0; i < nCount; i++)
			{		
				if (0 == i)
				{
					UserId = atoi(results[i].c_str());
				} 	
				if (1 == i)
				{
					sTime = atoi(results[i].c_str());
				} 	 
			} 
			// std::string strId = strCom; 
			// int acterId = atoi(strId.c_str());
			if (GetGlobalLogicEngine()->GetMiscMgr().OnBackForbidUserMis(UserId, sTime * 60))
			{
				nResult = 1;
				break;
			}  
			 
			nResult = 0;
			break;
		} 
		 
		case MSS_RELEA_KICK_USER_BY_ID:
		{  
			std::string strId = strCom; 
			int UserId = atoi(strId.c_str());

			if (GetGlobalLogicEngine()->GetMiscMgr().OnBackUnForbidUserMis(UserId))
			{
				nResult = 1;
				break;
			} 
			// CVector<void *> actorList;
			// GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
			// INT_PTR nLen = actorList.count();
			// for (INT_PTR i = 0; i < nLen; i++)
			// {
			// 	CActor * pActor = (CActor *)actorList[i];
			// 	if (pActor != NULL && UserId == pActor->GetId())
			// 	{
			// 		pActor->OnBackUnForbidUser();    
			// 		nResult = 1;
			// 		break;
			// 	}
			// }  
			nResult = 0;
			break;
		} 
		
		// case MSS_KICK_USER_BY_NAME:
		// {  
		// 	int sTime = 0;
		// 	std::string UserName; 
		// 	std::vector<std::string> results = SplitStr(strCom, "|");
		// 	int nCount = results.size();
		// 	if (nCount != 2)
		// 	{	
		// 		nResult = 0;//参数非法 
		// 		break;
		// 	}
			
		// 	for (int i = 0; i < nCount; i++)
		// 	{		
		// 		if (0 == i)
		// 		{
		// 			UserName = (results[i].c_str());
		// 		} 	
		// 		if (1 == i)
		// 		{
		// 			sTime = atoi(results[i].c_str());
		// 		} 	 
		// 	}  

		// 	CVector<void *> actorList;
		// 	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		// 	INT_PTR nLen = actorList.count();
		// 	for (INT_PTR i = 0; i < nLen; i++)
		// 	{
		// 		CActor * pActor = (CActor *)actorList[i];
		// 		if (pActor != NULL && UserName == pActor->GetEntityName())
		// 		{
		// 			pActor->OnBackForbidUser(sTime * 60);  
		// 			pActor->CloseActor(lwiBackStageTickActor, false);
		// 			nResult = 1;
		// 			break;
		// 		}
		// 	} 
		// 	break;
		// }
		// case MSS_KICK_USER_BY_ACCOUNT:
		// { 
		// 	int sTime = 0;
		// 	int AccountId; 
		// 	std::vector<std::string> results = SplitStr(strCom, "|");
		// 	int nCount = results.size();
		// 	if (nCount != 2)
		// 	{	
		// 		nResult = 0;//参数非法 
		// 		break;
		// 	}
			
		// 	for (int i = 0; i < nCount; i++)
		// 	{		
		// 		if (0 == i)
		// 		{
		// 			AccountId = atoi(results[i].c_str());
		// 		} 	
		// 		if (1 == i)
		// 		{
		// 			sTime = atoi(results[i].c_str());
		// 		} 	 
		// 	}  

		// 	CVector<void *> actorList;
		// 	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		// 	INT_PTR nLen = actorList.count();
		// 	for (INT_PTR i = 0; i < nLen; i++)
		// 	{
		// 		CActor * pActor = (CActor *)actorList[i];
		// 		if (pActor != NULL && AccountId == pActor->GetAccountID())
		// 		{
		// 			pActor->OnBackForbidUser(sTime * 60);  
		// 			pActor->CloseActor(lwiBackStageTickActor, false);
		// 			nResult = 1;
		// 			break;
		// 		}
		// 	} 
		// 	break;
		// } 
		case MSS_FCMKICKPLAY: //防沉迷踢玩家下线
			 {
				 CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
				 if (pActor)
				 {
					CActorPacket pack;
					pActor->AllocPacket(pack);
					pack << (BYTE) enDefaultEntitySystemID <<(BYTE)sFcmKictout;//发送到客户端踢下线
					pack.flush(); 
					OutputMsg(rmTip, _T("玩家[%d]防沉迷被踢！"), pActor->GetId() );
				
					//pActor->CloseActor(lwiBackStageTickActor, false); //关闭玩家的连接
					 nResult = 1;
				 }
				 else
				 {
					 nResult = 2;
				 }
				 break;
			 }
		 case MSS_QUERYPLAYONLINE:
			 {
				 CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
				 if (pActor)
				 {
					 nResult = 1;
				 }
				 break;
			 }
		 case MSS_QUERYUSERONLINE:
			 {
				 CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByAccountName(strCom);
				 if (pActor)
				 {
					 nResult = 1;
				 }
				 break;
			 }
		 case MSS_ADDNOTICE:
			 {
				 //格式:公告id|开始时间|结束时间|时间间隔(分钟)|显示位置|公告内容
				 char * args[10];
				 ParseArgStr(args,strCom,10);
				 if (args[5]!= NULL  && strlen(args[5]) > 0)
				 {
					 int nId = GetArgValueByInt(args[0]);
					 CMiniDateTime nStartTime= 0,nEndTime=0;
					 if (args[1]!= NULL && strlen(args[1]) >0)
					 {
						 StdTimeStr2MiniTime(args[1], nStartTime);
					 }
					 if (args[2]!= NULL && strlen(args[2]) >0)
					 {
						 StdTimeStr2MiniTime(args[2], nEndTime);
					 }
					 int nMins = GetArgValueByInt(args[3]);
					 int nDisplayPos = GetArgValueByInt(args[4]);
					
				     nResult = GetGlobalLogicEngine()->GetChatMgr().AddNotice(nId, nStartTime, nEndTime, nMins, nDisplayPos, args[5]);
				 }		 
				
				 break;
			 }
		 case MSS_DELNOTICE:
			 {
				 nResult = GetGlobalLogicEngine()->GetChatMgr().DeleteNotice(strCom);
				 break;
			 }
		 case MSS_DELAY_UPHOLE:	//进入倒计时维护状态
			 {
				 int nTime = atoi(strCom);
				 GetGlobalLogicEngine()->SetStopServerTime(nTime);
				 nResult = 1;
				 break;
			 }

		 case MSS_CANLCE_UPHOLE://取消倒计时维护状态       
			 {
				 GetGlobalLogicEngine()->CancelStopServer();
				 nResult = 1;
				 break;
			 }
		 case MSS_SHUTUP:	//禁言
			 {
				 LPCSTR sTag = strchr(strCom,'|');
				 if (sTag)
				 {
					 strCom[sTag-strCom] = 0;
					 sTag++;
					 CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
					 if(pActor)
					 {
						 pActor->GetChatSystem()->SetShutup(true,atoi(sTag)*60);
						 nResult = 1;
					 }
				 }
				 break;
			 }
		 case MSS_RELEASESHUTUP://解禁言
			 {
				 CActor* pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
				 if (pActor)
				 {
					 nResult = 1;
					 pActor->GetChatSystem()->SetShutup(false,0);
				 }
				 break;
			 }
		 case MSS_OPEN_COMMONSERVER:
			 {
				 GetLogicServer()->SetStartCommonServer(true);
				 OutputMsg(rmTip,_T("mgr:Start CommonServer!"));
				 nResult = 1;
				 break;
			 }
		 case MSS_CLOSE_COMMONSERVER:
			 {
				 GetLogicServer()->SetStartCommonServer(false);
				 OutputMsg(rmTip,_T("mgr:Stop CommonServer!"));
				 nResult = 1;
				 break;
			 }
		 case MSS_SET_CHATLEVEL:
			 {
				 char* sTag = strchr(strCom,'|');
				 if (sTag)
				 {
					 strCom[sTag-strCom] = 0;
					 sTag++;

					 int nChannelId = atoi(strCom);
					 int nLevel = atoi(sTag);
					 if (nChannelId < 0 || nChannelId >= ciChannelMax)
					 {
						 if (nChannelId == 100)
						 {
							 GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFriendChatLimit = nLevel;
						 }
						 nResult = 1;
			
					 }
					 else
					 {
						 CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
						 gc.ChatLimit[nChannelId].wLevel = nLevel;
						 nResult = 1;
					 }
				 }

				 break;
			 }
		case MSS_SET_CHATRECHARGE:
			 {
				 char* sTag = strchr(strCom,'|');
				 if (sTag)
				 {
					 strCom[sTag-strCom] = 0;
					 sTag++;

					 int nChannelId = atoi(strCom);
					 int nRecharge = atoi(sTag);
					 if (nChannelId >= 0 || nChannelId < ciChannelMax)
					 {
						 CHATSYSTEMCONFIG &gc = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
						 gc.ChatLimit[nChannelId].nRechargeAmount = nRecharge;
						 nResult = 1;
					 }
				 }
				 break;
			 }
		 case MSS_DELAY_COMBINE://后台设置合服倒计时(param=倒计时小时数)
			 {
				 LPCSTR sTag = strchr(strCom,'|');
				 if (sTag)
				 {
					 strCom[sTag-strCom] = 0;
					 sTag++;

					 char* sParam = (char*)strCom;
					 CMiniDateTime nStart;
					 StdTimeStr2MiniTime(sParam, nStart);
					 unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
					 unsigned int nStartTime = nStart.tv < nCurrTime ? nCurrTime : nStart.tv;
					 unsigned int nCombineTime = nStartTime + atoi(sTag) * 60 * 60;
					 GetGlobalLogicEngine()->GetMiscMgr().SetServerTempCombineTime(nCombineTime, nStartTime);
					 nResult = 1;
				 }
				 break;
			 }
		 case MSS_SET_REFRESHCORSS:
			 {
				 GetLogicServer()->ReloadCrossConfig(); //重新加载跨服的配置
				 nResult = 1;
				 break;
			 }
		 case MSS_SET_COMMON_SRVID:
			 {
				 int nSrvId = atoi(strCom);

				 GetLogicServer()->SetCommonServerId(nSrvId);
				 GetLogicServer()->ReloadCrossConfig(false); 
				 nResult = 1;
				 break;
			 }
		 case MSS_CLOSE_NOTICE:
			 {
				 GetGlobalLogicEngine()->GetChatMgr().DeleteAllNotice();
				 nResult = 1;
				 break;
			 } 
		 case MSS_DELTE_NOTICEBYID:
			 {
				 int nId = atoi(strCom);
				 bool bResult = GetGlobalLogicEngine()->GetChatMgr().DeleteNoticeById(nId);
				 nResult = (int)bResult;
				 break;
			 }
		 case MSS_SET_SPEED_CHECK:
			 {
				 char* strStarCheck = strchr(strCom,'|');
				 if (strStarCheck)
				 {
					 strCom[strStarCheck-strCom] = 0;
					 strStarCheck++;

					 char* strSpeedCheck = strchr(strStarCheck,'|');
					 if (strSpeedCheck)
					 {
						 strStarCheck[strSpeedCheck-strStarCheck] = 0;
						 strSpeedCheck++;
					 }

					 if (strSpeedCheck)
					 {
						 CCheckSpeedSystem::m_nTimeRate = atoi(strCom);			//时间比率
						 CCheckSpeedSystem::m_nStartCheck = atoi(strStarCheck);	//开始检测的次数
						 CCheckSpeedSystem::m_nSpeedCheck = atoi(strSpeedCheck);	//加速检查值
						 nResult = 1;
					 }
				 }
				 break;
			 }
		 case MSS_SETCHECK_SPEED:
			 {
				 char* strUseSpeedHandle = strchr(strCom,'|');
				 int nHandle = 0;
				 if (strUseSpeedHandle)
				 {
					 strCom[strUseSpeedHandle-strCom] = 0;
					 strUseSpeedHandle++;
					 nHandle  = atoi(strUseSpeedHandle);
				 }
				 int nState = atoi(strCom);

				 if(nState == 0)
				 {
					 CCheckSpeedSystem::m_bOpenVerifyFlag = false;
				 }
				 else
				 {
					 CCheckSpeedSystem::m_bOpenVerifyFlag = true;
					 CCheckSpeedSystem::m_useSpeedHandle = nHandle;
				 }
				 nResult = 1;
				 break;
			 }
		 case MSS_SET_HOTUPDATE:
			 {
				 bool bResult = GetGlobalLogicEngine()->GetMiscMgr().HotUpdateScript(strCom);
				 nResult = (int)bResult;
				 break;
			 }
		 case MSS_KICK_ALLACTORS:
			 {
				 CVector<void *> actorList;
				 GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
				 INT_PTR nLen = actorList.count();
				 for (INT_PTR i = 0; i < nLen; i++)
				 {
					 CActor * pActor = (CActor *)actorList[i];
					 if (pActor != NULL)
					 {
						 pActor->CloseActor(lwiBackStageTickAll, false);
					 }
				 }
				 nResult = 1;
				 break;
			 }
		 case MSS_SET_ACTORSTATUS:
			 {
				 char* sTag = strchr(strCom,'|');
				 int nResult = 1;
				 if (sTag != NULL)
				 {
					 strCom[sTag-strCom] = 0;	
					 sTag++;
					 int nFlag = atoi(strCom); //flag为0删除 为1添加
					 char * sLevel = strchr(sTag, '|');
					 if (sLevel != NULL)
					 {
						 sTag[sLevel - sTag] = 0;
						 sLevel++;
						 int nLevel = atoi(sLevel);
						 if ( nLevel > 0 )
						 {
							 bool boDisApear = false;
							 if ( nFlag != 0 )
							 {
								 boDisApear = true;
							 }
							 nResult = GetGlobalLogicEngine()->GetMiscMgr().KickAllActorByLevel(atoi(sTag),nLevel,boDisApear);
						 }
					 }
				 }
				 break;
			 }
		 case MSS_SET_SESSIONCMD:
			 {
				 using namespace jxInterSrvComm::SessionServerProto;
				 CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
				 CDataPacket &pack = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cSendMgrMsgToSession);
				 pack.writeString(strCom?strCom:"");
				 pSSClient->flushProtoPacket(pack);
				 nResult = 1;
				 break;
			 }
		 case MSS_SEND_MAIL:
			 {
				 // "邮件标题|邮件内容|角色id|附件数量|
				 // 附件1类型|附件1物品id|附件1数量
				 // 附件2类型|附件2物品id|..."
				 char * args[64];
				 int nCount =  ParseArgStr(args,strCom,64);
				 
				 if (args[0] && args[1] && args[2])
				 {
					 nResult = 1;
					 const char* sTitle = args[0];
					 const char* sContent = args[1];
					 unsigned int nActorId = atol(args[2]);
					 int nAttachCount = GetArgValueByInt(args[3]);
					 CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
					 for(int i =0,argsIndex=4; i <nAttachCount && i < MAILATTACHCOUNT; i++)
					 {
						 mailAttach[i].nType = GetArgValueByInt(args[argsIndex++]);
						 mailAttach[i].item.wItemId =  GetArgValueByInt(args[argsIndex++]);
						 mailAttach[i].item.wCount = GetArgValueByInt(args[argsIndex++]);
						 if(mailAttach[i].nType != qatEquipment)
						{
							mailAttach[i].item.wItemId = mailAttach[i].nType;
						}
						if (mailAttach[i].nType == 0 && mailAttach[i].item.wItemId == 0)
						{
							OutputMsg(rmError,"error Attach=%d,strcom=",i,strCom);
							nResult = 0;
						}
					 }
					 if (nResult == 1)
					 {
						 if (nActorId == 0)
						 {
							 CVector<void *> actorList;
							 GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
							 for (INT_PTR i = 0; i < actorList.count(); i++)
							 {
								 CActor *pActor = (CActor *)actorList[i]; 
								 if (pActor && pActor->IsInited() && !pActor->IsDestory())
								 { 
									if (pActor->OnGetIsTestSimulator()
										|| pActor->OnGetIsSimulator() )
									{
										continue;
									} 

									 CMailSystem::SendMail(pActor->GetId(), sTitle, sContent,mailAttach);
								 }		
							 }
						 }
						 else
						 {
							 CMailSystem::SendMail(nActorId, sTitle, sContent,mailAttach);
						 }
					 }
					
				 }
				break;
			 }
		 case MSS_COMPENSATE:	//开启补偿（开关|补偿方案ID|补偿开始时间|补偿时间(分钟)|标题）
			 {
				 nResult = 0;
				 char* sFlag = strchr(strCom,'|');
				 if (sFlag != NULL)
				 {
					 strCom[sFlag-strCom] = 0;	//开关
					 sFlag++;
					 int nFlag = atoi(strCom);
					 if( nFlag == 0 )			//关闭补偿
					 {
						 CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
						 mgr.SetCompensateVar(0);	//设置ID为0关闭
						 mgr.DoScriptFunction("TerminateBackCompensate");
						 nResult = 1;
					 }
					 else if( nFlag == 2 )			//清空排行榜（领取数据）
					 {
						 CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
						 mgr.DoScriptFunction("ClearBackCompensateRank");
						 nResult = 1;
					 }
					 else		//打开补偿
					 {
						 CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
						 char* sComId = strchr(sFlag,'|');
						 if( sComId )
						 {
							 sFlag[sComId-sFlag] = 0;
							 sComId++;
							 int nComId = atoi(sFlag);

							 char* sLevel = strchr(sComId,'|');
							 if( sLevel )
							 {
								 sComId[sLevel-sComId] = 0;
								 sLevel++;
								 int nLevel = atoi(sComId);

								 char* sBeginTime = strchr(sLevel,'|');
								 if( sBeginTime )
								 {
									 sLevel[sBeginTime-sLevel] = 0;
									 sBeginTime++;
									 CMiniDateTime miniBeginTime;
									 CBackStageSender::StdTimeStr2MiniTime(sLevel, miniBeginTime);

									 char* sLastTime = strchr(sBeginTime,'|');		//持续时间
									 if( sLastTime )
									 {
										 sBeginTime[sLastTime-sBeginTime] = 0;
										 sLastTime++;
										 int nLastTime = atoi(sBeginTime) * 60;

										 char* szTitle = sLastTime;		//最后是补偿title

										 mgr.SetCompensateVar( nComId, miniBeginTime, nLastTime, nLevel, szTitle,true );
										 nResult = 1;
									 }
								 }
							 }
						 }
					 }
				 }
				 else
				 {
					 int nFlag = atoi(strCom);
					 if( nFlag == 0 )			//关闭补偿
					 {
						 CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
						 mgr.SetCompensateVar(0);	//设置ID为0关闭
						 mgr.DoScriptFunction("TerminateBackCompensate");
						 nResult = 1;
					 }
					 else if( nFlag == 2 )			//清空排行榜（领取数据）
					 {
						 CMiscMgr &mgr = GetGlobalLogicEngine()->GetMiscMgr();
						 mgr.DoScriptFunction("ClearBackCompensateRank");
						 nResult = 1;
					 }
				 }
				 break;
			 }
		 case MSS_GET_ONLINEACTOR:
			 {
				 
				 INT_PTR nTotalCount = 0;
				 
				INT_PTR nSimulatorPlayerCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineSimulatorActorCount(nTotalCount);
				INT_PTR nNonGMPlayerCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineActorCount(nTotalCount);
				//GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineActorCount(nTotalCount);

				 int nNoSameCount = GetGlobalLogicEngine()->GetEntityMgr()->GetOnLineActorCountNoSame();
				 nResult = MAKELONG((WORD)(nNonGMPlayerCount - nSimulatorPlayerCount),(WORD)nNoSameCount);
				 break;
			 }
		 case MSS_OPEN_THELINEACTIVITY:
			 {
				 //格式:开始时间|持续时间(分钟)|活动链接地址
				 
				 nResult = 0;
				 char * args[10];
				 int nCount =  ParseArgStr(args,strCom,10);
				 CMiniDateTime nStarTime;
				 int nLastTime = GetArgValueByInt(args[1]);
				 StdTimeStr2MiniTime(args[0], nStarTime);
				 if (nStarTime.tv > 0 && nLastTime > 0 && args[2] != NULL)
				 {
					 nResult = 1;
				 }
				 break;
			 }

		 case MSS_ADD_FILTERWORDS:
			 {
				 char* sTag = strchr(strCom,'|');
				 nResult = 1;
				 if (sTag)
				 {
					 strCom[sTag-strCom] = 0;
					 sTag++;

					 char* sreMin = strchr(sTag,'|');
					 sTag[sreMin-sTag] = 0;
					 sreMin++;
					 GetGlobalLogicEngine()->GetChatMgr().MgrAddFilterWords(strCom,atoi(sTag),atoi(sreMin),-1);
				 }
				 break;
			 }
		 case MSS_SET_EXP_RATE:
			 {
				 //格式:开始时间|持续时间(分钟)|经验倍率
				 nResult = 0;
				 char * args[10];
				 ParseArgStr(args,strCom,10);
				 CMiniDateTime nStarTime;
				 int nLastTime = GetArgValueByInt(args[1]);
				 StdTimeStr2MiniTime(args[0], nStarTime);
				 float fRate = args[2] != NULL? (float)atof(args[2]):0;
				 if (nStarTime.tv > 0 && nLastTime > 0 && fRate != 0)
				 {
					 nResult = 1;
				 }
				 break;
			 }
		 case MSS_SET_REMOVEMONEY:
			 {
				 char* strType = strchr(strCom,'|');
				 if (strType)
				 {
					 strCom[strType-strCom] = 0;
					 strType++;

					 char* strValue = strchr(strType,'|');
					 if (strValue)
					 {
						 strType[strValue-strType] = 0;
						 strValue++;

						 size_t nLen = strlen(strCom);

						 int nType = atoi(strType);
						 int nVal = atoi(strValue);

						 if (strCom != NULL && nVal > 0)
						 {
							 if (nType == mtYuanbao || nType == mtBindYuanbao || nType == mtCoin)
							 {
								 CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(strCom);
								 if (pActor != NULL)
								 {
									 LPCTSTR sLogStr = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpBackRemoveMoney);
									 if( pActor->ChangeMoney(nType, -nVal, GameLog::clGMBackRemoveMoney, 0, sLogStr) )
									 {
										nResult = 1;
									 }
								 }
								 else
								 {
									 CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcBackRemoveMoney);
									 int nServerId = GetLogicServer()->GetServerIndex();
									 DataPacket << nServerId << nServerId;
									 DataPacket.writeString(strCom);
									 DataPacket.writeString(srcStr);
									 DataPacket.writeString(strName);
									 DataPacket << (WORD)nType;
									 DataPacket << (int)nVal;
									 DataPacket << (int)nId;
									 GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
									 nResult = -1;
								 }
							 }
						 }
					 }
				 }
				 break;
			 }
		case MSS_SET_REMOVEITEM:
			{
				char* sTag = strchr(strCom,'|');
				if (sTag)
				{
					strCom[sTag-strCom] = 0;
					sTag++;

					char* sreMin = strchr(sTag,'|');
					sTag[sreMin-sTag] = 0;
					sreMin++;

					int nHost = atoi(strCom);

					CUserItem::ItemSeries guid;
					guid.llId = _atoi64(sTag);

					CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sreMin);
					if (pActor == NULL)
					{
						CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcBackRemoveItem);
						int nServerId = GetLogicServer()->GetServerIndex();
						DataPacket << nServerId << nServerId;
						DataPacket.writeString(sreMin);
						DataPacket.writeString(srcStr);
						DataPacket.writeString(strName);
						DataPacket << (WORD)nHost;
						DataPacket << guid;
						DataPacket << (int)nId;
						GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
						nResult = -1;
					}
					else
					{
						CUserItem * pItem = NULL;
						LPCTSTR sLogStr = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpBackRemoveItem);
						switch(nHost)
						{
							case itItemBag:
								{
									pItem = pActor->GetBagSystem().FindItemByGuid(guid);
									if (pItem != NULL)
									{
										if(pActor->GetBagSystem().DeleteItem(pItem, pItem->wCount, sLogStr,GameLog::Log_BackStage))
										{
											nResult = 1;
										}
									}
								}
								break;

							case itItemEquip:
								{
									INT_PTR nIndex = pActor->GetEquipmentSystem().FindIndex(guid);
									if (nIndex >= 0)
									{
										if(pActor->GetEquipmentSystem().DeleteEquip((int)nIndex, sLogStr, GameLog::clGMBackRemoveItem))
										{
											nResult = 1;
										}
									}				
								}
								break;
							case itItemDepot:
								if (pActor->GetDeportSystem().DeleteItemByGuid(guid, sLogStr, GameLog::clGMBackRemoveItem) > 0)
								{
									nResult = 1;
								}
								break;

							default:
								break;
						}
					}
				}
				break;
			}
		case MSS_ACT_TOTAL_CHARGE:
			{
				//格式:开始时间|持续时间(分钟)|方案id
				nResult = 0;
				char * args[10];
				ParseArgStr(args,strCom,10);
				CMiniDateTime nStarTime;
				StdTimeStr2MiniTime(args[0], nStarTime);
				int nLastTime = GetArgValueByInt(args[1]);
				int nPlanId = GetArgValueByInt(args[2]);
				if (nStarTime.tv > 0 && nLastTime > 0 && nPlanId != 0)
				{
					nResult = 1;
				}
				else if (nLastTime == 0)
				{
					nResult = 1;
				}
				break;
			}
		case MSS_ACT_DEL_GUILD:
			{
				nResult = GetGlobalLogicEngine()->GetGuildMgr().LogSenderDelGuild(strCom);
				break;
			}
		case MSS_ACT_DEL_GUILD_MEMO:
			{
				CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuildByName(strCom);
				if (pGuild)
				{
					pGuild->SetOutMemo("",true);
					nResult = 1;
				}
				break;
			}
		case MSS_ACT_SET_GUILD_MEMO:
			{
				//格式:行会id|公告|生效时间(即时生效则不传)
				char * args[10];
				ParseArgStr(args,strCom,10);
				unsigned int nGuildId = GetArgValueByInt(args[0]);
				CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
				if (pGuild && args[1]!= NULL)
				{
					if (args[2]== NULL)
					{
						pGuild->SetOutMemo(args[1],true);
						nResult = 1;
					}
					else
					{
						pGuild->SetNextUpdateInmemo(args[1]);
						CMiniDateTime nUpdateTime;
						StdTimeStr2MiniTime(args[2], nUpdateTime);
						pGuild->m_nNewInMemoUpdateTime = nUpdateTime;
						nResult = 1;
					}
				}
				break;
			}
		case MSS_SET_GUILD_LEADER:
			{
				//格式:行会id|新帮主名
				char * args[10];
				ParseArgStr(args,strCom,10);
				unsigned int nGuildId = GetArgValueByInt(args[0]);
				CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
				if (pGuild && args[1]!= NULL)
				{
					pGuild->GmChangeLeader(args[1]);
					nResult = 1;
				}
				break;
			}
		case MSS_SET_GUILD_CERTIFICATION:
		{
			//格式:行会id|是否认证(1是，0否)
			char * args[10];
			ParseArgStr(args,strCom,10);
			unsigned int nGuildId = GetArgValueByInt(args[0]);
			CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
			if (pGuild && args[1]!= NULL)
			{
				int nCertification = GetArgValueByInt(args[1]);
				pGuild->SetCertification(nCertification);
				nResult = 1;
			}
			break;
		}
		case MSS_SHUTUP_BY_ID:
		{
			//格式:人物id|是否禁言(1禁言，0解禁)|禁言时长(单位:分)
			char * args[10];
			ParseArgStr(args,strCom,10);
			unsigned int nActorId = GetArgValueByInt(args[0]);
			bool bForbid = GetArgValueByInt(args[1]) > 0 ? true : false;
			unsigned int nMins = GetArgValueByInt(args[2]);
			CChatSystem::SetOffLineShutUp(nActorId, bForbid, nMins*60);
			nResult = 1;
			break;
		}
		case MSS_BROAD_POP_NOTICE:
		{
			//格式:公告内容
			GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(strCom, tstPop);
			nResult = 1;
			break;
		}
		case MSS_UPDATE_BROAD_POP_NOTICE:
		{
			//格式:公告内容
			GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(strCom, tstUpdateClient);
			nResult = 1;
			break;
		} 
		case MSS_DELTE_GUILD:
		{
			int nId = atoi(strCom);
			CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nId);
			
			if(pGuild)
			{
				nResult = GetGlobalLogicEngine()->GetGuildMgr().DeleteGuild(pGuild, 1); //  GetGuildId());  
				
				GetGlobalLogicEngine()->GetGuildMgr().SendDbMsgDeleteGuild(0, pGuild->m_nGid);
				nResult = 1;
			}
			else
			{
				nResult = 2;
			} 
			break;
		}
		case MSS_SET_NEW_CDKEY_YXM:
		{
			//int nId = atoi(strCom);
			//根据在线的actorid查找
			std::string strGiftCode = strCom;
			// CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nId);
			// if (pActor)
			// {
			// 	//pActor->OnSetNewCdkGift(strGiftCode);
			// 	OutputMsg(rmTip, "[YXMCdkGift] MSS_SET_NEW_CDKEY_YXM online" );
			// 	// CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcBackRemoveItem);
			// 	// int nServerId = GetLogicServer()->GetServerIndex();
			// 	// DataPacket << nServerId << nServerId;
			// 	// DataPacket.writeString(srcStr);
			// 	// DataPacket << (int)nId;
			// 	// GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
			// 	nResult = 1;
			// } 
			// else
			// {
			// 	OutputMsg(rmTip, "[YXMCdkGift] MSS_SET_NEW_CDKEY_YXM offline" );
			// 	//CMailSystem::SendMail(nActorId, sTitle, sContent,mailAttach);
			// 	nResult = 0; //没有查找到在线玩家
			// }
 
			std::string realCode = "";
			std::string mailtitle = "";
			int SrvId = 0;
			int UserId = 0; 
			std::vector<std::string> results = SplitStr(strGiftCode, "|");
			int nCount = results.size();
			for (int i = 0; i < nCount; i++)
			{			
				int state = atoi(results[i].c_str());
				if (5 == i)
				{
					realCode = results[i].c_str();
				}
				if (3 == i)
				{
					SrvId = atoi(results[i].c_str());
				}
				if (2 == i)
				{
					UserId = atoi(results[i].c_str());
				} 
				if (6 == i)
				{
					mailtitle = results[i].c_str();
				} 
			}
			if(SrvId == 0 || realCode == "")
			{ 
				OutputMsg(rmTip, "[YXMCdkGift]error  CdkGift : Code %s",  realCode.c_str());
				return;
			}
			static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc(); 
			CScriptValueList paramList, retList; 
			// paramList << enMiscSystemID;
			// paramList << cUseNewCdkey; 
			//paramList << this;  

			std::string strSendBuff = "";
			strSendBuff.resize(1000);
			CDataPacket pack((char *)strSendBuff.data(), 1000); 
			pack << (int)SrvId;
			pack << (int)UserId;
			pack.writeString(realCode.c_str());
			pack.writeString(mailtitle.c_str()); 
			pack.setPosition(0);
			CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
			paramList << &dp; 
			bool ret = globalNpc->GetScript().CallModule("NewCdkey", "OnCMDBackStageCdKey", paramList, retList, 1); 
			if (!ret)
			{
				const RefString& s = globalNpc->GetScript().getLastErrorDesc();  
				OutputMsg(rmTip, "error OnCMDBackStageCdKey:%s", (const char*)s);
			} 
			paramList.clear();
			retList.clear();

			nResult = 1; 
			break;
		}
		case MSS_SET_NEW_37_BINDPHONE:
		{ 
			//int nId = atoi(strCom);
			//根据在线的actorid查找
			std::string strId = strCom; 
			int acterId = atoi(strId.c_str());
   
			OutputMsg(rmTip, "[Platform37] BindPhone:  actorid %s",   strId.c_str());
	 
			static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc(); 
			CScriptValueList paramList, retList;  

			std::string strSendBuff = "";
			strSendBuff.resize(1000);
			CDataPacket pack((char *)strSendBuff.data(), 1000);  
			pack << (int)acterId; 
			pack.setPosition(0);
			CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
			paramList << &dp; 
			bool ret = globalNpc->GetScript().CallModule("Platform37Gift", "OnCMDBackStageBindPhone", paramList, retList, 1); 
			if (!ret)
			{
				const RefString& s = globalNpc->GetScript().getLastErrorDesc();  
				OutputMsg(rmTip, "error On 37Game BindPhone errer:%s", (const char*)s);
			} 
			paramList.clear();
			retList.clear();

			nResult = 1; 
			break;
		}
		case MSS_SET_TRADINGQUOTA:
		{
			std::string strTemp = strCom;

			unsigned int nActorId = 0;
			unsigned long long nTradingQuota = 0; 

			std::vector<std::string> results = SplitStr(strTemp, "|");
			
			int nParamCount = results.size();
			if ( nParamCount != 2 )
			{
				return;
			}

			enum nParamType
			{
				eParamType_ACTORID = 0,// 玩家Id
				eParamType_TRADINGQUOTA = 1,// 交易额度
			};

			for (size_t i = 0; i < nParamCount; i++)
			{
				switch (i)
				{
				case eParamType_ACTORID:
					nActorId = atoi(results[i].c_str());
					break;
				case eParamType_TRADINGQUOTA:
					nTradingQuota = strtoull(results[i].c_str(), nullptr, 10);
					break;
				default:
					break;
				}
			}
			
			CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
			if (pActor != NULL)
			{
				pActor->SetProperty<unsigned long long>(PROP_ACTOR_TRADINGQUOTA, nTradingQuota);
			}
			else
			{
				CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcBackChangeTradingQuota);
				int nServerId = GetLogicServer()->GetServerIndex();
				DataPacket << nServerId << nServerId;
				DataPacket.writeString(results[eParamType_ACTORID].c_str());
				DataPacket.writeString(results[eParamType_TRADINGQUOTA].c_str());
				GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
				nResult = -1;
			}

			nResult = 1; 
			break;
		}
		case MSS_CLEAR_MSG:
		{   
			if (GetGlobalLogicEngine()->GetMiscMgr().OnBackClearAllMsg())
			{
				nResult = 1;
				break;
			}  
			nResult = 0;
			break;
		}
		case MSS_ADD_CUSTOMTITLE: // 添加自定义称号
		{   
			//格式:角色id|个性称号Id
			char* args[10];
			ParseArgStr(args,strCom,10);
			if ( args[0] && args[1] )
			{
				unsigned int nActorId = atoi(args[0]);
				int nCutomTitleId = atoi(args[1]);

				if ( GetGlobalLogicEngine()->GetMiscMgr().OnBackAddCustomTitle(nActorId, nCutomTitleId) )
				{
					nResult = 1;
					break;
				}
			}

			nResult = 0;
			break;
		}
		case MSS_DEL_CUSTOMTITLE: // 删除自定义称号
		{   
			//格式:角色id|个性称号Id
			char* args[10];
			ParseArgStr(args,strCom,10);
			if ( args[0] && args[1] )
			{
				unsigned int nActorId = atoi(args[0]);
				int nCutomTitleId = atoi(args[1]);

				if ( GetGlobalLogicEngine()->GetMiscMgr().OnBackDelCustomTitle(nActorId, -nCutomTitleId) )
				{
					nResult = 1;
					break;
				}
			}

			nResult = 0;
			break;
		}   
		default:break;
	 }
	 if (nResult >= 0)
	 {
		 SendCommandResult(srcStr,(int)nCmd,nResult,strName,(int)nId);
	 }
	 

	 delete [] strCom; // 这是在  void CBackStageSender::RevBackCommandOp(CDataPacketReader &inPacket) 里面new的
	 delete [] stName; // 这是在  void CBackStageSender::RevBackCommandOp(CDataPacketReader &inPacket) 里面new的
	 //GetLogicServer()->GetBuffAllocator()->FreeBuffer(strCom);
	 //GetLogicServer()->GetBuffAllocator()->FreeBuffer(stName);
 }

 char* CBackStageSender::GetStrDest(char* srcStr,char* strDest)
 {
	 if(!srcStr) return NULL;

	 strDest = _tcschr(srcStr,'|');
	 srcStr[strDest-srcStr] = 0;
	 strDest++;

	 return strDest;
 }
 int CBackStageSender::GetArgValueByInt(char* args)
 {
	if (args != NULL)
	{
		return atoi(args);
	}
	return 0;
 }
 unsigned int CBackStageSender::ParseArgStr(char **args, char *pArgStr, int nMaxArgCnt)
 {
	 int nArgCount = 0;				//参数的个数
	 int  nBlankCount =0;			//空格的数量
	 memset(args, 0, sizeof(char*) * nMaxArgCnt);
	 //while (*pArgStr && nArgCount < ArrayCount(args))
	 while (*pArgStr && nArgCount < nMaxArgCnt)
	 {
		 //取第一个空格的位置
		 char *ptr = strchr(pArgStr, '|'); //
		 //如果没有空格的话
		 if (!ptr) 
		 {
			 //如果前面出现过空格，那么说明后面是一个参数
			 if (nBlankCount >= 0)
			 {
				 args[nArgCount] = pArgStr;
				 nArgCount ++;
			 }
			 break;
		 }
		 //命令的话就不要处理
		 if (nBlankCount >= 0)
		 {
			 args[nArgCount] = pArgStr;
			 nArgCount++;
		 }
		 nBlankCount ++; //空格的数量++
		 *ptr = 0;
		 ptr++;
		 //跳过连续的空格
		 while (*ptr && (unsigned char)*ptr <= 0x20)
			 ptr++;
		 pArgStr = ptr; 
	 }

	 return nArgCount;
 }

 void CBackStageSender::StdTimeStr2MiniTime(const char* szTime, CMiniDateTime& miniTime)
 {
	//miniTime.encode(sStdTime);
	 SYSTEMTIME sysTime; //倒计时的开启时间
	 if (szTime != NULL && strlen(szTime) > 10) //是个基本合法的名字
	 {
		 sscanf(szTime, "%d-%d-%d-%d:%d:%d", &sysTime.wYear, &sysTime.wMonth, &sysTime.wDay, &sysTime.wHour, &sysTime.wMinute, &sysTime.wSecond);
	 }
	 else
	 {
		 GetLocalTime(&sysTime);
	 }
	 miniTime.encode(sysTime);
 }
