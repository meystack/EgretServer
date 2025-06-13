#include "StdAfx.h"
#include "NetWorkMsgHandle.h"
//#include "mjEdcode.h"
#include "mjProto.h"

using namespace jxInterSrvComm::DbServerProto;

CNetWorkMsgHandle::CNetWorkMsgHandle()
{
	for (int i = 0; i < CCustomGateManager::MaxGateCount; i++)
	{
		m_GateUserDataList[i].setLock(&m_GateDataLock[i]);
	}

	for (int i = 0; i < CCustomGateManager::MaxGateCount; i++)
	{
		m_GateServerPushdownDataList[i].setLock(&m_GateServerPushdownLock[i]);
	}
	m_ItnMsgList.setLock(&m_ItnMsgListLock);
}

CNetWorkMsgHandle::~CNetWorkMsgHandle()
{
	//清理消息list
	for (INT_PTR nGateIdx=0; nGateIdx<CCustomGateManager::MaxGateCount; ++nGateIdx)
	{
		CQueueList<CDataPacket*> &DataList = m_GateUserDataList[nGateIdx];
		if ( DataList.appendCount() > 0 )
			DataList.flush();
		if ( DataList.count() > 0 )
		{
			CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(nGateIdx);
			logicgate->freeBackUserDataList(DataList);
		}
	}

	if (m_ItnMsgList.appendCount() > 0)
	{
		m_ItnMsgList.flush();
	}
	INT_PTR nCount = m_ItnMsgList.count();
	for (int i = 0; i < nCount; ++i )
	{
		PLOGICINTERNALMSG pAppMsg = m_ItnMsgList[i];
		if (pAppMsg->uMsg == SSM_DB_RET_DATA)
		{
			CDataPacket* pDataPacket = (CDataPacket*)(pAppMsg->uParam2);
			GetLogicServer()->GetDbClient()->FreeBackUserDataPacket(pDataPacket);
		}
		m_Allocator.FreeBuffer(pAppMsg);
	}
	m_ItnMsgList.clear();
}

//
VOID CNetWorkMsgHandle::PostUserDataList(const INT_PTR nGateIdx, CBaseList<CDataPacket*>& vList)
{
	if ((nGateIdx < 0) || (nGateIdx >= CCustomGateManager::MaxGateCount)) return;
	
	CQueueList<CDataPacket*> &DataList = m_GateUserDataList[nGateIdx];
	DataList.appendList(vList);
	vList.trunc(0);
}
//

void CNetWorkMsgHandle::DumpBuffAlloc(wylib::stream::CBaseStream& stream)
{
	CBuffAllocStatHelper::Dump(stream, &m_Allocator, _T("NetWorkMsgHandler::DataAllocator"), false);
	TCHAR szBuff[MAX_PATH];
	// Dump All logicGate
	for (INT_PTR nGateIdx = 0; nGateIdx < CCustomGateManager::MaxGateCount; ++nGateIdx)
	{
		CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(nGateIdx);
		if (logicgate && logicgate->connected())
		{
			logicgate->DumpBuffAlloc(stream);
			_stprintf(szBuff, _T("网关堆积数据包数量:%d\r\n"), logicgate->getPacketCount());
			CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);			
			// 本周期发送的数据包大小
			Uint64 nSendPacketSize = logicgate->GetSendPacketSize();
			_stprintf(szBuff, _T("累积发送网关数据包大小:%lld, 数量:%d\r\n"), (unsigned long long)(nSendPacketSize / 1024), 
				logicgate->GetSendPacketCount());
			CBuffAllocStatHelper::WriteStrToStream(stream, szBuff);			
			logicgate->ResetSendPacketSize();
		}
	}
}

void CNetWorkMsgHandle::SendGateUserInfoToDB()
{
	CDataClient* pDataClient = GetLogicServer()->GetDbClient();
	CDataPacket& packet = pDataClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateGateUserInfo);
	WORD nCount = 0;
	packet << GetLogicServer()->GetServerIndex();
	size_t nPos = packet.getPosition();
	packet << nCount;
	GATEUSERINFO gi;
	for (INT_PTR i = 0; i < CCustomGateManager::MaxGateCount; i++)
	{
		CLogicGate* gate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(i);
		if (gate && gate->connected())
		{
			gi = gate->getGateUserInfo();
			packet.writeBuf(&gi, sizeof(gi));
			nCount++;
		}
	}

	char *pMem = packet.getPositionPtr(nPos);
	*((WORD *)pMem) = nCount;
	pDataClient->flushProtoPacket(packet);
}

VOID CNetWorkMsgHandle::RunOne()
{
	SF_TIME_CHECK(); //检测性能
	DECLARE_TIME_PROF("CNetWorkMsgHandle::RunOne");
	TICKCOUNT nStartTick = _getTickCount();
	/*
	if (m_SyncGateUserTimer.CheckAndSet(nStartTick))
	{
		SendGateUserInfoToDB();
	}
	*/
	
	for (INT_PTR nGateIdx=0; nGateIdx<CCustomGateManager::MaxGateCount; ++nGateIdx)
	{
		CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(nGateIdx);
		if(logicgate ==NULL /*|| !logicgate->connected()*/) continue;
		CBaseList<CDataPacket *> &  freeList = logicgate->GetLogicProcecdDataList();
		if(freeList.count() >0)
		{
			logicgate->freeBackUserDataList(freeList); //将空闲列表添加进去
		}
		CQueueList<CDataPacket*> &DataList = m_GateUserDataList[nGateIdx];
		if ( DataList.appendCount() > 0 ) //如果后面有追加的列的话
			DataList.flush();           //把追加的列刷到列表里去
		if ( DataList.count() > 0 )
		{
			for (INT_PTR i=0; i<DataList.count(); ++i)
			{
				CDataPacket* packet = (DataList[i]);
				//TRACE("datalist[%d]:%d\n",i,(int)packet);
				//处理数据
				LOGICUSERDATA GateUser;
				//jxSrvDef::INTERSRVCMD nCmdId;
				INT_PTR nSize = packet->getPosition();
				packet->setPosition(0);
				
				(*packet) >> GateUser ; //将网关的时间弄过来
				
				EntityHandle hHandle;
				hHandle.init(enActor ,GateUser.nHandleIndex); 
				CActor * pActor =(CActor *) GetEntityFromHandle(hHandle);
				if(pActor ==NULL)
				{
					OutputMsg(rmWaning, _T("recv data fail,Handle=%u entity has destroyed "), GateUser.nHandleIndex);
					continue; 
				}
			
				//(this->*Handlers[nCmdId])(pActor,*packet);
				pActor->PostNetMsg(packet,nSize- packet->getPosition()); //搞一个网络数据包过去
			}
			DataList.trunc(0);
			//logicgate->freeBackUserDataList(DataList);
		}

		//这里发送给网关的数据
		logicgate->SendToGate();
	}
	
	SetLoopTickCount(_getTickCount() - nStartTick); //记录这个花了多少时间
}



//这个是逻辑线程处理的
VOID CNetWorkMsgHandle::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4,UINT64 uParam5,UINT64 uParam6,UINT64 uParam7,int uParam8 )
{
	//created entity，要转给实体管理器
	if (SSM_DB_RET_DATA == uMsg) //数据服务器返回的
	{
		//uParam1 CMD
		//uParam2 pData数据指针
		//OutputMsg(rmTip, _T("db return data :%d"), uParam1);
		CDataPacket* pDataPacket = (CDataPacket*)uParam2;
		if ( pDataPacket )
		{
			if(uParam1 == dcLoadOfflineUserBasic ||uParam1 == dcLoadOfflineUserOther || dcSaveOfflineInfo == uParam1 ||
				uParam1 == dcLoadOfflineUserHeroBasic || uParam1 == dcLoadOfflineUserHeroEquips)
			{
				GetGlobalLogicEngine()->GetOfflineUserMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 == dcLoadCustomInfoUnForbid 
					|| uParam1 == dcSaveCustomInfoUnForbid || uParam1 == dcLoadCustomInfoCustomTitleId || uParam1 == dcSaveCustomInfoCustomTitleId )
			{
				GetGlobalLogicEngine()->GetMiscMgr().OnDbReturnData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getPosition());
			
				//GetGlobalLogicEngine()->GetGuildMgr().OnDbReturnData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getPosition());
			}
			else if (uParam1 >= dcLoadGuildList && uParam1 <= dcLoadGuildDepotItem)
			{
				GetGlobalLogicEngine()->GetGuildMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 == dcAddMsg)
			{
				CMsgSystem::OnAddOfflineMsgDbReturn(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 == dcAddMsgByActorName)
			{
				CMsgSystem::OnAddOfflineMsgByNameDbReturn(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			} 
			else if (uParam1 >= dcLoadBaseRank && uParam1 <= dcSaveBaseRankCS )
			{
				GetGlobalLogicEngine()->GetRankingMgr().OnDbReturnData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getPosition());
			}
			else if (uParam1 == dcLoadCircleCount )
			{
				GetGlobalLogicEngine()->GetGlobalVarMgr().DelDbRetData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getPosition());
			}
			else if(dcLoadCombatRankInfo == uParam1 || dcUpdateCombatBaseInfo == uParam1)
			{
				//GetGlobalLogicEngine()->GetCombatRankMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if( uParam1 >= dcLoadTopLevelActor &&uParam1 <= dcLoadSwingLevelData)
			{
				
			}
			else if ( uParam1 >= dcLoadTopLevelActorData && uParam1 <= dcLoadTopLevelActorData)
			{
				GetGlobalLogicEngine()->GetWorldLevelMgr().OnDbReturnData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getMemorySize());
			}
			else if( uParam1 >= dcAddFilterWordsMsg && uParam1 <= dcBackRemoveMoney)
			{
				GetGlobalLogicEngine()->GetChatMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if ( uParam1 >= dcSaveConsignmentItem && uParam1 <= dcLoadConsignmentIncome)
			{
				GetGlobalLogicEngine()->GetConsignmentMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 >= dcLoadNewServerMail && uParam1 <= dcLoadNewServerMail)
			{
				GetGlobalLogicEngine()->GetMailMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 >= dcLoadActivityList && uParam1 <= dcSaveActivityList)
			{
				GetGlobalLogicEngine()->GetActivityMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			else if (uParam1 == dcNoticeFee)
			{
				GetGlobalLogicEngine()->GetEntityMgr()->OnDbNoticeFee(pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			// else if (uParam1 >= dcLoadBossInfo && uParam1 <= dcSaveBossInfo)
			// {
			// 	GetGlobalLogicEngine()->GetBossMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			// }
			else
			{
				GetGlobalLogicEngine()->GetEntityMgr()->OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			
			//处理完数据指针要还回
			GetLogicServer()->GetDbClient()->FreeBackUserDataPacket(pDataPacket);
		}
		return;
	}
	else if(SSM_GATE_CREATE_ACTOR == uMsg)
	{

		//uParam1 nSocket
		//uParam2 nActorID
		//uParam3 nGateID
		//uParam4 nGateSessionIndex
		//uParam5 nServerSessionIndex
		//uParam6 nAccountID
		OutputMsg(rmTip,"%s,account=%d,actorid=%d ",__FUNCTION__,(unsigned int)uParam6,(unsigned int)(uParam2));

		
		CLogicGate * pGate = (CLogicGate *)GetLogicServer()->GetGateManager()->getGate(uParam3);
		if(pGate==NULL)
		{
			OutputMsg(rmWaning,"%s gate id=[%d] is NULL",__FUNCTION__,uParam3);
			return;
		}

		CEntityManager *pMgr = GetGlobalLogicEngine()->GetEntityMgr();
		CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
		
		if(!GetLogicServer()->IsCrossServer())
		{
			CActor * pActor = pMgr->GetEntityPtrByAccountID((unsigned int)uParam6); //判断这个玩家是否登陆了
			if(pActor != NULL) //该账户已经有一个角色登陆了，就需要提示玩家登陆失败
			{
				//GetLogicServer()->GetGateManager()->KickUser((ACCOUNTDATATYPE)uParam6,pActor->GetGateKey() );
				while (1)
				{ 
					if (pActor->OnGetIsTestSimulator()
						|| pActor->OnGetIsSimulator() )
					{
						pActor->CloseActor(lwiSimulatorSameActorLogin, false);
						break;
					}
					
					if(pActor->GetGateKey() != uParam7)
					{
						CActorPacket ap;
						CDataPacket &outPack = pActor->AllocPacket(ap);
						outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
						ap.flush();
						pActor->CloseActor(lwiGateCreateKeyError, false);
						break;
					}
					break;
				}
				 
				//return; 
			}pActor = pMgr->GetEntityPtrByAccountID((unsigned int)uParam6); //判断这个玩家是否登陆了
			if(pActor != NULL) //该账户已经有一个角色登陆了，就需要提示玩家登陆失败
			{
				//GetLogicServer()->GetGateManager()->KickUser((ACCOUNTDATATYPE)uParam6,pActor->GetGateKey() );
				while (1)
				{ 
					if (pActor->OnGetIsTestSimulator()
						|| pActor->OnGetIsSimulator() )
					{
						pActor->CloseActor(lwiSimulatorSameActorLogin, false);
						break;
					}
					
					if(pActor->GetGateKey() != uParam7)
					{
						CActorPacket ap;
						CDataPacket &outPack = pActor->AllocPacket(ap);
						outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
						ap.flush();
						pActor->CloseActor(lwiGateCreateKeyError, false);
						break;
					}
					break;
				}
				 
				//return; 
			}pActor = pMgr->GetEntityPtrByAccountID((unsigned int)uParam6); //判断这个玩家是否登陆了
			if(pActor != NULL) //该账户已经有一个角色登陆了，就需要提示玩家登陆失败
			{
				//GetLogicServer()->GetGateManager()->KickUser((ACCOUNTDATATYPE)uParam6,pActor->GetGateKey() );
				while (1)
				{ 
					if (pActor->OnGetIsTestSimulator()
						|| pActor->OnGetIsSimulator() )
					{
						pActor->CloseActor(lwiSimulatorSameActorLogin, false);
						break;
					}
					
					if(pActor->GetGateKey() != uParam7)
					{
						CActorPacket ap;
						CDataPacket &outPack = pActor->AllocPacket(ap);
						outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
						ap.flush();
						pActor->CloseActor(lwiGateCreateKeyError, false);
						break;
					}
					break;
				}
				 
				//return; 
			}pActor = pMgr->GetEntityPtrByAccountID((unsigned int)uParam6); //判断这个玩家是否登陆了
			if(pActor != NULL) //该账户已经有一个角色登陆了，就需要提示玩家登陆失败
			{
				//GetLogicServer()->GetGateManager()->KickUser((ACCOUNTDATATYPE)uParam6,pActor->GetGateKey() );
				while (1)
				{ 
					if (pActor->OnGetIsTestSimulator()
						|| pActor->OnGetIsSimulator() )
					{
						pActor->CloseActor(lwiSimulatorSameActorLogin, false);
						break;
					}
					
					if(pActor->GetGateKey() != uParam7)
					{
						CActorPacket ap;
						CDataPacket &outPack = pActor->AllocPacket(ap);
						outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
						ap.flush();
						pActor->CloseActor(lwiGateCreateKeyError, false);
						break;
					}
					break;
				}
				 
				//return; 
			}
			//2017-1-11
			//登录客户端发来的nAccountID ，nActorID，没有校验角色是否属于对应账号的
			//导致封包修改角色id可以登录其他账号角色,在线的角色先判断，避免2个角色同时登录
			//CActor::Init 读取数据之后做了账号和id的校验
			pActor = pMgr->GetEntityPtrByActorID((unsigned int)uParam2);
			if (pActor != NULL) 
			{
				if (pActor->GetAccountID() != (unsigned int)uParam6)
				{
					pGate->PostKickUser((ACCOUNTDATATYPE)uParam6, uParam7);
					return;
				}
				else
				{
					pActor->CloseActor(lwiSameActorLogin, false);
				}
			}  
			jxSrvDef::GLOBALSESSIONOPENDATA sessionData;
			CLogicSSClient* pSSClient = GetLogicServer()->GetSessionClient();
			bool bRet = pSSClient->GetSessionData((int)uParam6, uParam7, &sessionData);
			if(bRet == false)
			{
				OutputMsg(rmWaning,"CNetWorkMsgHandle 创建角色的时候，找不到sessionID =%d,lkey=%lld的会话查找",(int)uParam6,uParam7);
				pGate->PostKickUser((ACCOUNTDATATYPE)uParam6, uParam7);
				return;
			}

			EntityHandle hHandle; 
			pActor = (CActor*) pMgr->CreateEntity(enActor, hHandle); //创建一个实体，如果实体创建不成功的话
			if(pActor ==NULL) //实体创建失败的话，要通知这个
			{
				OutputMsg(rmError, _T("create actor entity failed"));
				pGate->PostSetActorHandleIndex((int)uParam5, 0, (unsigned int)uParam6);
			}
			else
			{	
				pActor->SetSimulator(false);//标记不是模拟用户
				pActor->SetGateKey(uParam7);
				pActor->SetCreateIndex(uParam8);
				int nFcmTime = 0;
				nFcmTime = sessionData.dwFCMOnlineSec; 
				pSSClient->PostSetSessionPlayer(sessionData.nSessionId, pActor, uParam7);
				pActor->Onlogin(uParam1, uParam2, uParam3, uParam4, uParam5, (unsigned int)uParam6, nFcmTime, sessionData.nGmLevel, sessionData.nRawServerId, sessionData.nClientIPAddr);
				pGate->PostSetActorHandleIndex((int)uParam5, hHandle.GetIndex(), (unsigned int)uParam6);
				//跨服标记
				pSimuMgr->OnRemoveRuningCsActor(uParam2); 
			} 
		}
		else
		{
			//跨服同id 需要做处理
			CActor * pActor = pMgr->GetEntityPtrByActorID((unsigned int)uParam2);
			if (pActor != NULL) 
			{
				CActorPacket ap;
				CDataPacket &outPack = pActor->AllocPacket(ap);
				outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
				ap.flush();
				pActor->CloseActor(lwiSameActorLogin, false);
			}
 
			/*
			跨服同平台同账号同区处理--可能会存在效率问题
			*/
			int nCSSrvId = GetGlobalLogicEngine()->GetCrossMgr().GetLoginActorCSSrvId((unsigned int)uParam2);
		
			OutputMsg(rmTip, "[CrossLogin 12 ]this is corssserver,account=%d,actorid=%d ", (unsigned int)uParam6, (unsigned int)(uParam2));
			EntityHandle hHandle; 
			pActor = (CActor*) pMgr->CreateEntity(enActor, hHandle); //创建一个实体，如果实体创建不成功的话
			if(pActor ==NULL) //实体创建失败的话，要通知这个
			{
				OutputMsg(rmError, _T("create actor entity failed"));
				pGate->PostSetActorHandleIndex((int)uParam5, 0, (unsigned int)uParam6);
			}
			else
			{	
				pActor->SetSimulator(false);//标记不是模拟用户
				// int nCSSrvId = GetGlobalLogicEngine()->GetCrossMgr().GetLoginActorCSSrvId((unsigned int)uParam2);
				pActor->SetGateKey(uParam7);
				pActor->SetCreateIndex(uParam8);
				int nFcmTime = 0;
				pActor->Crosslogin(uParam1, uParam2, uParam3, uParam4, uParam5, (unsigned int)uParam6, nCSSrvId);
				pGate->PostSetActorHandleIndex((int)uParam5, hHandle.GetIndex(), (unsigned int)uParam6);
				//跨服标记
				pSimuMgr->OnRemoveRuningCsActor(uParam2);
			}
		} 
		return;
	}
	else if (SSM_SS_RET_DATA == uMsg)//会话服务器返回的数据
	{
		CDataPacket* pDataPacket = (CDataPacket*)uParam2;
		if ( pDataPacket )
		{
			GetGlobalLogicEngine()->GetEntityMgr()->OnSsReturnData(uParam1, pDataPacket->getMemoryPtr(), pDataPacket->getPosition());
			//处理完数据指针要还回
			GetLogicServer()->GetSessionClient()->FreeBackUserDataPacket(pDataPacket);
		}
		return;
	}
	else if(SSM_LOGIN_FAIL == uMsg) //玩家登陆失败
	{
		//uParam1 nSocket
		//uParam2 nErrorcode
		//uParam3 nGateID
		//uParam4 nGateSessionIndex
		//uParam5 nServerSessionIndex
		//uParam6 nAccountID
		//告诉客户端登陆失败了
		OutputMsg(rmWaning,"CNetWorkMsgHandle SSM_LOGIN_FAIL account=[%d] is NULL,socket=%d",(int)uParam6,(int)uParam1);
		CActorPacket pack;
		CActor::InitActorPacket(pack, (int)uParam3, (int)uParam4, (int)uParam5, (int)uParam1, __FILE__, __LINE__);
		pack << (BYTE)enDefaultEntitySystemID << (BYTE)sUserLogin << BYTE(uParam2);
		pack.flush();
		CLogicGate * pGate = (CLogicGate *)GetLogicServer()->GetGateManager()->getGate(uParam3);
		if(pGate)
		{
			pGate->PostCloseUser((int)uParam1,(int)uParam5);
		}
		else
		{
			OutputMsg(rmWaning,"%s gate id=[%d] is NULL", __FUNCTION__, uParam3);
		}
	}
	else if(SSM_GATE_USER_LOGOUT == uMsg)
	{
		//网关发过来消息，让玩家下线，这个时候直接把玩家踢下去
		//网关发过来的，让玩家下线的
		unsigned int nSessionId = (unsigned int)(uParam1);
		CActor * pActor = GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByAccountID((unsigned int)nSessionId); //判断这个玩家是否登陆了
		if(pActor )
		{
			if(pActor->GetGateKey() == uParam2)
			{
				pActor->CloseActor(lwiGateUserLogout, true);
			}
		}
		else
		{
			unsigned long long lKey = (unsigned long long)(uParam2);

			GetLogicServer()->GetSessionClient()->PostCloseSession(nSessionId, lKey); //session close
			//OutputMsg(rmError,_T("account=%d玩家登出的时候,account的玩家指针为空"),nSessionId);
		}
		return;
	}
	else if(SSM_SESSION_KICK_USER ==uMsg)
	{
		unsigned int nSessionId = (unsigned int)(uParam1);
		CActor * pActor = GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByAccountID((unsigned int)nSessionId); //判断这个玩家是否登陆了
		if(pActor)
		{
			if(pActor->IsInTransmitStatus() )
			{
				OutputMsg(rmTip,_T("account=%d玩家正在跨服状态，不踢下线"),nSessionId);
			}
			else
			{
				OutputMsg(rmTip,_T("account=%d玩家在其他服务器登陆，踢下线"),nSessionId);
				pActor->CloseActor(lwiSessionKickActor, false);
			}
		}
		else
		{
			OutputMsg(rmError,_T("SSM_SESSION_KICK_USER account=%d玩家登出的时候,account的玩家指针为空"),nSessionId);
		}
	}
	/*
	else if (SSM_FRIEND_INFO_UPDATE_SVR_ADDR == uMsg)
	{		
		unsigned int nActorID =(unsigned int)uParam1; //玩家的actorID
		CActor *pActor =GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorID);
		if (pActor)
		{	
			pActor->SyncFriendSvrAddr();
		}
		else
		{
			OutputMsg(rmWaning,_T("初始化好友服务器的地址的时候玩家已经下线,actorid=%d"),nActorID);
		}
	}	
	*/

	//else if (SSM_FRIEND_LIST == uMsg)
	//{
	//	unsigned int nActorID = (unsigned int )uParam1; //玩家的actorID
	//	CActor *pActor =GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorID);
	//	char * pAddr = (char *)uParam2;
	//	if(pAddr ==NULL) return;

	//	if (pActor && pActor->IsInited()) 
	//	{
	//		

	//		CDataPacketReader packet(pAddr,(UINT_PTR)uParam3 ); //这段数据是好友服务器传进来的
	//		LPCSTR sTag = strchr(packet.getOffsetPtr(), '/');
	//		if (sTag)
	//		{
	//			if (sTag > packet.getOffsetPtr())
	//			{
	//				char TempBuffer[128] ;
	//				INT_PTR nLen = DecodeBuffer(packet.getOffsetPtr(), TempBuffer, 
	//					sTag - packet.getOffsetPtr(), sizeof(TempBuffer)-1);
	//				TempBuffer[nLen] = 0;
	//				pActor->SetPersonalitySign(TempBuffer);
	//			}
	//			packet.setPosition(sTag - packet.getMemoryPtr() + 1);
	//			if (packet.getAvaliableLength() > 0)
	//			{
	//				CFriendClient::LOGICFRIENDDATA frdData[MAX_USER_FRIEND_COUNT] = {0};												
	//				CVector<int> friendList;
	//				INT_PTR nLen = DecodeBuffer(packet.getOffsetPtr(), frdData, packet.getAvaliableLength(), sizeof(frdData));
	//				for (int i = 0; i < MAX_USER_FRIEND_COUNT && frdData[i].nCharId != 0; i++)
	//				{							
	//					friendList.push(frdData[i].nCharId);
	//				}
	//				pActor->InitFriendList(friendList);
	//			}
	//			pActor->SyncFriendSvrAddr();

	//		}	
	//	}
	//	
	//	SafeDelete(pAddr);
	//	return;
	//}
	//else if (SSM_FRIEND_CHANGED == uMsg)
	//{	
	//	CActor *pActor = GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID((unsigned int)uParam1);		
	//	if (pActor)
	//	{
	//		if (uParam2 == 0)//添加好友
	//		{
	//			pActor->OnFriendChanged(true, (int)uParam3);				
	//		}
	//		else //删除好友
	//		{
	//			pActor->OnFriendChanged(false, (int)uParam3);
	//		}
	//	}
	//	return;
	//}
	else if ( SSM_MS_RET_DATA == uMsg)
	{
		return;
	}
	else if (SSM_LOGIC_2_COMMONLOGIC_DATA == uMsg)
	{
		// 逻辑服务器之间的数据交互
		CDataPacket* packet = (CDataPacket*)uParam2;		
		GetGlobalLogicEngine()->GetTransmitMgr().OnRecvMessage(0, (int)uParam1, *packet);
		CCommonServerClient *pClient = (CCommonServerClient *)(uParam3);
		if (pClient)
			pClient->FreeBackUserDataPacket(packet);
	}
	else if (SSM_COMMONLOGIC_2_LOGIC_DATA == uMsg)
	{
		CDataPacket* packet = (CDataPacket*)uParam2;
		GetGlobalLogicEngine()->GetTransmitMgr().OnRecvMessage(1, (int)uParam1, *packet);
		GetLogicServer()->GetCommonClient()->FreeBackUserDataPacket(packet);		
	}
	else if (SSM_LOGIC_2_CROSS_DATA == uMsg)
	{
		CDataPacket* packet = (CDataPacket*)uParam2;
		if(packet) {
			CDataPacketReader reader(packet->getMemoryPtr(),packet->getPosition());
			GetGlobalLogicEngine()->GetCrossMgr().OnRecvLogicMessage((int)uParam1, reader);
		}
		
		return;
	}
	else if (SSM_CROSS_2_LOGIC_DATA == uMsg)
	{
		CDataPacket* packet = (CDataPacket*)uParam2;
		if(packet) {
			CDataPacketReader reader(packet->getMemoryPtr(),packet->getPosition());
			GetGlobalLogicEngine()->GetCrossMgr().OnRecvCrossMessage((int)uParam1, reader);
			GetLogicServer()->GetCrossClient()->FreeBackUserDataPacket(packet);	
		}
		
		return;	
	}
	if (SSM_CROSS_SERVER_DB_DATA == uMsg) //数据服务器返回的
	{
		CDataPacket* pDataPacket = (CDataPacket*)uParam2;
		if ( pDataPacket )
		{
			if(uParam1 == dcGetCSActorId ||  uParam1 == dcCreateCSActorId)
			{
				GetGlobalLogicEngine()->GetCrossMgr().OnDbReturnData(uParam1,pDataPacket->getMemoryPtr(),pDataPacket->getPosition());
			}
			
			//处理完数据指针要还回
			GetLogicServer()->GetDbClient()->FreeBackUserDataPacket(pDataPacket);
		}
		return;
	}
	else if(SSM_CROSS_CEHCK_ACTORDATA == uMsg) //跨服玩家账号登录
	{
		GetGlobalLogicEngine()->GetCrossMgr().OnAddWaitInitList(uParam1, uParam2, uParam3, uParam4);
	}
	else if (SSM_NEWBACKMGR_DATA == uMsg )	//后台CMD
	{
		GetLogicServer()->GetBackClient()->HandleCommand(uParam1, uParam2, uParam3, uParam4);
	}
	else if (SSM_ASYNC_WORK_THREAD == uMsg) //异步工作线程
	{
		AsyncWorker::CBData* pCallBackData = (AsyncWorker::CBData*)uParam1;
		if (pCallBackData)
		{
			AsyncWorker::AsyncWorkerAlloc& alloc = pCallBackData->worker_->GetAlloc();

			if (pCallBackData->after_func_) 
				pCallBackData->after_func_(pCallBackData);

			if (pCallBackData->params_ && pCallBackData->free_func_)
				pCallBackData->free_func_(pCallBackData);

			alloc.FreeBuffer(pCallBackData);
		}
	}
}

//发送内部消息
VOID CNetWorkMsgHandle::PostInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, 
	UINT64 uParam3,UINT64 uParam4,UINT64 uParam5,UINT64 uParam6,UINT64 uParam7,int uParam8)
{
	PLOGICINTERNALMSG pAppMsg = (PLOGICINTERNALMSG)m_Allocator.AllocBuffer(sizeof(*pAppMsg));

	pAppMsg->uMsg = uMsg;
	pAppMsg->uParam1 = uParam1;
	pAppMsg->uParam2 = uParam2;
	pAppMsg->uParam3 = uParam3;
	pAppMsg->uParam4 = uParam4;
	pAppMsg->uParam5 = uParam5;
	pAppMsg->uParam6 = uParam6;
	pAppMsg->uParam7 = uParam7;
	pAppMsg->uParam8 = uParam8;
	m_ItnMsgList.append(pAppMsg);

	//OutputMsg(rmTip,_T("CNetWorkMsgHandle::PostInternalMessage(): addr=%d"),(int)pAppMsg);
}

VOID CNetWorkMsgHandle::ProcessInternalMessages()
{
	//SF_TIME_CHECK(); //检测性能
	INT_PTR i =0,nCount=0;
	PLOGICINTERNALMSG  pAppMsg;
	DECLARE_TIME_PROF("CNetWorkMsgHandle::ProcessInternalMessages");
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 	
	{
		m_ItnMsgList.flush();
		nCount = m_ItnMsgList.count();
		for ( i = 0; i < nCount; i ++ ) // i =12 修改了数据
		{
			pAppMsg = m_ItnMsgList[i];
			DispatchInternalMessage(pAppMsg->uMsg, pAppMsg->uParam1, pAppMsg->uParam2, pAppMsg->uParam3,pAppMsg->uParam4,pAppMsg->uParam5,pAppMsg->uParam6,pAppMsg->uParam7,pAppMsg->uParam8);
			m_Allocator.FreeBuffer(pAppMsg);

			//OutputMsg(rmTip,_T("CNetWorkMsgHandle ProcessInternalMessages():i=%d,addr=%d"),i,(int)pAppMsg);
		}
		m_ItnMsgList.clear();
	}
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		if( i < nCount)
		{
			pAppMsg = m_ItnMsgList[i]; //如果这个消息出问题了，那么就删除这个消息
			m_Allocator.FreeBuffer(pAppMsg);
			m_ItnMsgList.remove(0,i+1);
		}
		
	}	
#endif 
#endif 
}
