#include "StdAfx.h"

CRebateSystem::CRebateSystem()
{
	m_RunningRebates.clear();			
	m_WaitingRebates.clear();			
	m_RunningSeq.clear();   
    m_WaitingSeq.clear();   

	m_SendNextDayAwardRebates.clear();
	m_SendImmediatelyAwardRebates.clear();
	m_ClearTimeOutRebates.clear(); 
}

CRebateSystem::~CRebateSystem()
{

}


bool CRebateSystem::Initialize(void *data,SIZE_T size)
{
    PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData == NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}

	LoadFromDB();
	return true;
}

void CRebateSystem::Destroy()
{

}

VOID CRebateSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{
	
	if(m_pEntity == NULL)
	{
		OutputMsg(rmError,_T("Rebate Data is Error..."));
		return;
	}
	if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadRebateData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return Rebate repeated,actorid=%u "),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}

		m_SendImmediatelyAwardRebates.clear();
		m_SendNextDayAwardRebates.clear();
		m_ClearTimeOutRebates.clear();
		
		CMiniDateTime nCurTime = GetGlobalLogicEngine()->getMiniDateTime();

		std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();

		int nCount = 0;
		reader >> nCount;
		
		RebateData rebateData;
		
		// 用于判断 返利活动 是否已经发放 立即发放 奖励
		bool isSendImmediatelyAward = true;

		// 用于判断 返利活动 是否已经发放 次日发放 奖励
		bool isSendNextDayAward = true;

		std::string strStatus = "";
        for(int i = 0; i < nCount; ++i)
        {
			isSendImmediatelyAward = true;
			isSendNextDayAward = true;
			strStatus.clear();
			memset(&rebateData, 0, sizeof(rebateData));
            reader.readBuf(&rebateData, sizeof(rebateData)); 
            // OutputMsg(rmTip,"[Rebate] 返利活动加载(%d)，活动Type=%d, 活动ID=%d !", i, rebateData.nActivityType, rebateData.nId);

			// 单笔返利不存数据库，针对之前数据库中存有单笔返利记录的，修改
			if ( rebateData.nId >= SingleRebateStartId && rebateData.nId < YBRebateStartId )
			{
				OutputMsg(rmError,"[Rebate] 数据库中 存在 单笔返利活动 BBBB ,活动ID=%d 玩家Id%d!",rebateData.nId, rebateData.nActorId);
				continue;
			}

			strStatus = intToString( rebateData.nStatus );

			//  清理 领取奖励后保存时间 超时的返利活动
			if ( rebateData.nClearTime && nCurTime > rebateData.nClearTime )
			{
				m_ClearTimeOutRebates[rebateData.nId] = rebateData;
			}

			if ( rebateGroupInfo.find(rebateData.nId) != rebateGroupInfo.end() ) 
			{
				// 返利活动 期间
				if ( nCurTime >= rebateData.nStartTime && nCurTime < rebateData.nEndTime )
				{	
					// 判断数据库中的 返利活动数据 是否与配置表中的 返利活动数据一致
					if ( rebateData.nTimeType == rebateGroupInfo[rebateData.nId].nTimeType && rebateData.nOfflineType == rebateGroupInfo[rebateData.nId].nOfflineType
						&& rebateData.nAtonce == rebateGroupInfo[rebateData.nId].nAtonce && rebateData.nOverlay == rebateGroupInfo[rebateData.nId].nOverlay
						&& m_RunningRebates.find(rebateData.nId) == m_RunningRebates.end() )
					{
						OutputMsg(rmError,"[Rebate] 数据库中的返利活动 加载到 运行列表 1111 ,活动ID=%d !",rebateData.nId);
						m_RunningRebates[rebateData.nId] = rebateData;
                    	m_RunningSeq.insert(&m_RunningRebates[rebateData.nId]);
					}
				}

				//  发放 已结束、立即发放、不可叠加、未发放奖励 的返利活动
				if ( nCurTime > rebateData.nEndTime && !rebateData.nOverlay && rebateData.nAtonce )
				{
					for (size_t i = 0; i < strStatus.length(); i++)
					{
						if ( strStatus[i] == '1' )
						{
							isSendImmediatelyAward = false;
						}
					}

					// 未发放奖励
					if ( !isSendImmediatelyAward )
					{
						m_SendImmediatelyAwardRebates[rebateData.nId] = rebateData;
					}
				}
				
				// 返利活动 已经结束，且为循环类型
				if ( nCurTime >= rebateData.nEndTime && rebateData.nTimeType == 3 && m_WaitingRebates.find(rebateData.nId) == m_WaitingRebates.end() )
				{
					if ( GetLogicServer()->GetDataProvider()->GetRebateProvider().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(), rebateGroupInfo[rebateData.nId]) && m_WaitingRebates.find(rebateData.nId) == m_WaitingRebates.end() )
                    {
						OutputMsg(rmError,"[Rebate] 数据库中的返利活动 加载到 等待列表 2222 ,活动ID=%d !",rebateData.nId);
						rebateData.nStartTime = rebateGroupInfo[rebateData.nId].nStartTime;
						rebateData.nEndTime = rebateGroupInfo[rebateData.nId].nEndTime;
						rebateData.nClearTime = 0;
						rebateData.nYBValue = 0;
						rebateData.nStatus = 0;
						m_WaitingRebates[rebateData.nId] = rebateData;
						m_WaitingSeq.insert(&m_WaitingRebates[rebateData.nId]);
                    }
				}					
			}
		}

		// 清理过期的 返利活动
		if ( m_ClearTimeOutRebates.size() )
		{
			ClearTimeOutRebates();
		}

		RebateGroupIterator iter = rebateGroupInfo.begin();
		memset(&rebateData, 0, sizeof(rebateData));
		for (; iter != rebateGroupInfo.end(); iter++)
        {
			// 返利活动 期间
			if ( nCurTime > (*iter).second.nStartTime && nCurTime < (*iter).second.nEndTime )
			{
				if (m_RunningRebates.find((*iter).second.nId) == m_RunningRebates.end())
                {				
					OutputMsg(rmError,"[Rebate] 配置中的返利活动 加载到 运行列表 3333 ,活动ID=%d !",(*iter).second.nId);
					rebateData.nId = (*iter).second.nId;							
					rebateData.nTimeType = (*iter).second.nTimeType;
					rebateData.nStartTime = (*iter).second.nStartTime;							
					rebateData.nEndTime = (*iter).second.nEndTime;											
					rebateData.nOfflineType = (*iter).second.nOfflineType;					
					rebateData.nAtonce = (*iter).second.nAtonce;						
					rebateData.nOverlay = (*iter).second.nOverlay;
					rebateData.nActorId = m_pEntity->GetId();;	
					rebateData.nClearTime = 0;						
					rebateData.nYBValue = 0;						
					rebateData.nStatus = 0;
					m_RunningRebates[rebateData.nId] = rebateData;
					m_RunningSeq.insert(&m_RunningRebates[rebateData.nId]);
				}
			}
				
			// 返利活动 未开始
			if ( nCurTime <= (*iter).second.nStartTime && m_WaitingRebates.find((*iter).second.nId) == m_WaitingRebates.end() )
			{
				OutputMsg(rmError,"[Rebate] 配置中的返利活动 加载到 等待列表 EEEE ,活动ID=%d !",rebateData.nId);		
				rebateData.nId = (*iter).second.nId;							
				rebateData.nTimeType = (*iter).second.nTimeType;
				rebateData.nOfflineType = (*iter).second.nOfflineType;					
				rebateData.nAtonce = (*iter).second.nAtonce;						
				rebateData.nOverlay = (*iter).second.nOverlay;						
				rebateData.nStartTime = (*iter).second.nStartTime;							
				rebateData.nEndTime = (*iter).second.nEndTime;
				rebateData.nActorId = m_pEntity->GetId();;	
				rebateData.nClearTime = 0;
				rebateData.nYBValue = 0;						
				rebateData.nStatus = 0;
				m_WaitingRebates[rebateData.nId] = rebateData;
				m_WaitingSeq.insert(&m_WaitingRebates[rebateData.nId]);
			}
			

			// 返利活动 已经结束，且为循环类型
			if ( nCurTime >= (*iter).second.nEndTime && (*iter).second.nTimeType == 3 )
			{
				if ( GetLogicServer()->GetDataProvider()->GetRebateProvider().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(), (*iter).second) )
                {
					if (m_WaitingRebates.find((*iter).second.nId) == m_WaitingRebates.end())
                	{
						OutputMsg(rmError,"[Rebate] 配置中的返利活动 加载到 等待列表 4444 ,活动ID=%d !",rebateData.nId);		
						rebateData.nId = (*iter).second.nId;							
						rebateData.nTimeType = (*iter).second.nTimeType;
						rebateData.nOfflineType = (*iter).second.nOfflineType;					
						rebateData.nAtonce = (*iter).second.nAtonce;						
						rebateData.nOverlay = (*iter).second.nOverlay;						
						rebateData.nStartTime = (*iter).second.nStartTime;							
						rebateData.nEndTime = (*iter).second.nEndTime;
						rebateData.nActorId = m_pEntity->GetId();;	
						rebateData.nClearTime = 0;
						rebateData.nYBValue = 0;						
						rebateData.nStatus = 0;
						m_WaitingRebates[rebateData.nId] = rebateData;
						m_WaitingSeq.insert(&m_WaitingRebates[rebateData.nId]);
					}
                }
			}
		}

		((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_REBATE_SYSTEM);

		// 跨天登录
		if ( ((CActor*)m_pEntity)->GetNewDayArrive() > 0 )
		{
			CheckNextDayAward();
		}
		
	
	} 
	else if ( nCmd == jxInterSrvComm::DbServerProto::dcLoadNextDayRebateData && nErrorCode == jxInterSrvComm::DbServerProto::reSucc )
	{
		CMiniDateTime nCurTime = GetGlobalLogicEngine()->getMiniDateTime();

		std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();

		int nCount = 0;
		reader >> nCount;

		RebateData rebateData;

		// 用于判断 返利活动 是否已经发放 次日发放 奖励
		bool isSendNextDayAward = true;
		std::string strStatus = "";
		m_SendNextDayAwardRebates.clear();
		for (int i = 0; i < nCount; i++)
		{
			isSendNextDayAward = true;
			strStatus.clear();
			memset(&rebateData, 0, sizeof(rebateData));
            reader.readBuf(&rebateData, sizeof(rebateData)); 

			strStatus = intToString( rebateData.nStatus );

			if ( rebateGroupInfo.find(rebateData.nId) != rebateGroupInfo.end() ) 
			{
				//  发放 已经结束、次日发放、未发放奖励 的多日返利活动 或者 次日发放、未发放奖励 的单日返利活动, 
				if ( (rebateData.nOfflineType == eRebateOfflineType_OneDayRecharge || rebateData.nOfflineType == eRebateOfflineType_OneDayConsume) 
				|| ( (rebateData.nOfflineType == eRebateOfflineType_DaysRecharge || rebateData.nOfflineType == eRebateOfflineType_DaysConsume) && nCurTime > rebateData.nEndTime ) 
				&& !nCurTime.isSameDay(rebateData.nEndTime) && !rebateData.nAtonce )
				{
					for (size_t i = 0; i < strStatus.length(); i++)
					{
						if ( strStatus[i] == '1' )
						{
							isSendNextDayAward = false;
						}
					}

					// 未发放奖励
					if ( !isSendNextDayAward )
					{
						m_SendNextDayAwardRebates[rebateData.nId] = rebateData;
					}
				}
			}
		}
		
		//  发放 已经结束、次日发放、未发放奖励 的多日返利活动 或者 次日发放、未发放奖励 的单日返利活动, 
		if ( m_SendNextDayAwardRebates.size() )
		{
			SendNextDayAward();
		}

		ClearOneDayRechargeAndOneDayConsumeYBNum();
	}
}

VOID CRebateSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
    if (!m_pEntity || m_pEntity->GetType() != enActor) 
	{
		return;
	}

	int nCount = 0;
	// 运行中的活动
	CMiniDateTime nCurTime = GetGlobalLogicEngine()->getMiniDateTime();
    RebateSetIterator runIter = m_RunningSeq.begin();
	std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	std::map<int, RebateData> tempRebateData;
    for ( ; runIter != m_RunningSeq.end(); )
    {
		nCount++;
		if ( 1000 == nCount )
		{
			OutputMsg(rmError,"[Rebate] 运行中的返利活动 循环有误 DDDD ,当前循环次数=%d !", nCount);
			break;
		}

		// 判断指针
		if ( !(*runIter) )
		{
			OutputMsg(rmError,"[Rebate] 运行中的返利活动 HHHH (*runIter) 指针非法 !");
			break;
		}

		if ( (*runIter)->nEndTime != 0 && nCurTime > (*runIter)->nEndTime )
        {	
			if(rebateGroupInfo.find((*runIter)->nId) != rebateGroupInfo.end())
            {
				bool result = GetLogicServer()->GetDataProvider()->GetRebateProvider().UpdateOpenTime(GetGlobalLogicEngine()->GetGlobalNpcLua(), rebateGroupInfo[(*runIter)->nId] );
				if (result && m_WaitingRebates.find((*runIter)->nId) == m_WaitingRebates.end() )
				{
					OutputMsg(rmError,"[Rebate] 运行中的返利活动 加载到 等待列表 5555 ,活动ID=%d !",(*runIter)->nId);
					RebateData &rebateData = m_WaitingRebates[(*runIter)->nId];
					rebateData.nActorId = (*runIter)->nActorId;						
					rebateData.nId = (*runIter)->nId;							
					rebateData.nTimeType = (*runIter)->nTimeType;												
					rebateData.nOfflineType = (*runIter)->nOfflineType;					
					rebateData.nAtonce = (*runIter)->nAtonce;						
					rebateData.nOverlay = (*runIter)->nOverlay;								
					rebateData.nStartTime = rebateGroupInfo[(*runIter)->nId].nStartTime;
					rebateData.nEndTime = rebateGroupInfo[(*runIter)->nId].nEndTime;
					rebateData.nClearTime = 0;
					rebateData.nYBValue = 0;						
					rebateData.nStatus = 0;
					m_WaitingSeq.insert(&m_WaitingRebates[rebateData.nId]);
				}

				// 已经结束 不可叠加、立即发放，且玩家在线
				int nStatus = 0;
				char sContent[1024];
				if ( !(*runIter)->nOverlay && (*runIter)->nAtonce && pDbClient->connected() )
				{
					nStatus = 0;
					tempRebateData.clear();
					std::string strStatus = intToString( (*runIter)->nStatus );
					for (int i = strStatus.length() - 1; i >= 0; i--)
					{
						if ( strStatus[i] == '1' )
						{
							if (rebateGroupInfo[(*runIter)->nId].RebatesInfo.find( strStatus.length() - i ) != rebateGroupInfo[(*runIter)->nId].RebatesInfo.end())
							{

								memset(&sContent, 0, sizeof(sContent));
								sprintf(sContent, rebateGroupInfo[(*runIter)->nId].RebatesInfo[strStatus.length() - i].sContent, rebateGroupInfo[(*runIter)->nId].RebatesInfo[strStatus.length() - i].nPay);

								CVector<ACTORAWARD> awardlist;
								for(int j = 0; (j < rebateGroupInfo[(*runIter)->nId].RebatesInfo[strStatus.length() - i].vecAwardList.size()) && (j < MAILATTACHCOUNT); j++) {

									ACTORAWARD actorAward = rebateGroupInfo[(*runIter)->nId].RebatesInfo[strStatus.length() - i].vecAwardList[j];
									if (actorAward.wId == 0) {
										continue;
									}
									actorAward.wCount = actorAward.wCount;
									awardlist.add(actorAward);
								}

								CMailSystem::SendMailByAward((*runIter)->nActorId, rebateGroupInfo[(*runIter)->nId].RebatesInfo[strStatus.length() - i].sTitle, sContent, awardlist);
							}
							nStatus = 2 * pow(10, i);
							RebateData &temp = tempRebateData[(*runIter)->nId];
							temp.nActorId = (*runIter)->nActorId;						
							temp.nId = (*runIter)->nId;							
							temp.nTimeType = (*runIter)->nTimeType;
							temp.nStartTime = (*runIter)->nStartTime;
							temp.nEndTime = (*runIter)->nEndTime;												
							temp.nOfflineType = (*runIter)->nOfflineType;					
							temp.nAtonce = (*runIter)->nAtonce;						
							temp.nOverlay = (*runIter)->nOverlay;						
							temp.nYBValue = (*runIter)->nYBValue;						
							temp.nStatus = nStatus;						
							
							temp.nClearTime = (*runIter)->nEndTime.today() + REBATEDATASAVETIME;
							continue;
						}
					}

					//更新数据库
					CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateRebateData);
					dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
					dataPacket << (unsigned int)m_pEntity->GetId();
					dataPacket << (unsigned int)tempRebateData.size();

					// 更新数据库中 已经结束、次日发放、（可叠加 或 不可叠加）返利活动的 状态码
					for ( RebateMapIterator iter = tempRebateData.begin(); iter != tempRebateData.end(); iter++ )
					{
						dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
						<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
					}

					GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
				}
			}
			OutputMsg(rmTip, "[Rebate] 运行中的返利活动，被释放 8888 ，活动ID=%d !", (*runIter)->nId);
			m_RunningRebates.erase(m_RunningRebates.find((*runIter)->nId));
        	runIter = m_RunningSeq.erase(runIter);
		}
		else 
		{
			runIter++;
		}
	}

	nCount = 0;
	// 等待中的活动
    RebateSetIterator waitIter = m_WaitingSeq.begin();
	for ( ; waitIter != m_WaitingSeq.end(); )
    {
		nCount++;
		if ( 1000 == nCount )
		{
			OutputMsg(rmError,"[Rebate] 等待中的返利活动 循环有误 CCCC ,当前循环次数=%d !", nCount);
			break;
		}

		// 判断指针
		if ( !(*waitIter) )
		{
			OutputMsg(rmError,"[Rebate] 运行中的返利活动 JJJJ (*waitIter) 指针非法 !");
			break;
		}

		if ( nCurTime >= (*waitIter)->nStartTime &&  nCurTime <= (*waitIter)->nEndTime)
		{
			if ( m_RunningRebates.find((*waitIter)->nId) != m_RunningRebates.end() )
			{
				OutputMsg(rmError,"[Rebate] 等待中的循环活动，却在运行列表中，活动ID=%d !",(*waitIter)->nId);
				m_WaitingRebates.erase(m_WaitingRebates.find((*waitIter)->nId));
            	waitIter = m_WaitingSeq.erase(waitIter);
				continue;
			}
			OutputMsg(rmTip, "[Rebate] 等待中的返利活动，加载到 运行列表中 6666 ，活动ID=%d !", (*waitIter)->nId);

			m_RunningRebates[(*waitIter)->nId] = *(*waitIter);
            m_RunningSeq.insert(&m_RunningRebates[(*waitIter)->nId]);

			// 删除
			OutputMsg(rmTip, "[Rebate] 等待中的返利活动，被释放 7777 ，活动ID=%d !", (*waitIter)->nId);
            m_WaitingRebates.erase(m_WaitingRebates.find((*waitIter)->nId));
            waitIter = m_WaitingSeq.erase(waitIter);
		}
		else
		{
			waitIter++;
		}
	}
}

void CRebateSystem::OnEnterGame()
{

}

void CRebateSystem::LoadFromDB()
{
	if(!m_pEntity)  
	{
        return; 
    } 
	
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator())
	{ 
		return; 
	}
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadRebateData);
		DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)m_pEntity->GetId();
 
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 
	}
}

void CRebateSystem::SaveToDB()
{

}

void CRebateSystem::OnEnterScene()
{
	
}

void CRebateSystem::UpdateRebateData(int nYBValue, int nType)
{
	if(!m_pEntity) return;

	if (nYBValue < 0)
	{
		nYBValue = abs(nYBValue);
	}

	// 验证参数
	if ( nYBValue == 0 || (nType >= eRebateYBChangeType_MAX || nType <= eRebateYBChangeType_MIN) )
	{
		return;
	}

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if ( !pDbClient->connected() || !m_RunningRebates.size() )
    {
		OutputMsg(rmNormal, _T("m_RunningRebates.size() : %d"), m_RunningRebates.size());
		return;
	}

	std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();
	
	// 用于判断当前 返利活动是否存在可领取奖励
	// int nIndex = 0; 
	std::map<int, RebateData> SaveRebates;
	SaveRebates.clear();
	for (RebateSetIterator iter = m_RunningSeq.begin(); iter != m_RunningSeq.end(); )
	{
		// 判断指针
		if ( !(*iter) )
		{
			OutputMsg(rmTip, "[Rebate] 运行的返利活动 FFFF (*iter) 指针非法 !");
			break;
		}
		
		if ( rebateGroupInfo.find((*iter)->nId) == rebateGroupInfo.end() )
		{
			OutputMsg(rmTip, "[Rebate] 运行的返利活动，被释放 9999 ，活动ID=%d !", (*iter)->nId);
			m_RunningRebates.erase(m_RunningRebates.find((*iter)->nId));
        	iter = m_RunningSeq.erase(iter);
			continue;
		} 
		else
		{
			/*
			*	nOfflineType：活动类型
			*		1.单日累充：eRebateOfflineType_OneDayRecharge；
			*		2.多日累充：eRebateOfflineType_DaysRecharge；
			*		3.单日累消：eRebateOfflineType_OneDayConsume；
			*		4.多日累消：eRebateOfflineType_DaysConsume
			*/
			if ( nType == eRebateYBChangeType_Recharge && ( (*iter)->nOfflineType == eRebateOfflineType_OneDayConsume || (*iter)->nOfflineType == eRebateOfflineType_DaysConsume ) )
			{
				iter++; 
				continue;
			} else if( nType == eRebateYBChangeType_Consume && ( (*iter)->nOfflineType == eRebateOfflineType_OneDayRecharge || (*iter)->nOfflineType == eRebateOfflineType_DaysRecharge ) )
			{
				iter++;
				continue;
			}

			// 线下返利、元宝返利 增加对应活动的元宝值
			if ( (*iter)->nId < SingleRebateStartId || (*iter)->nId >= YBRebateStartId )
			{
				(*iter)->nYBValue += nYBValue;
			} else {
				(*iter)->nYBValue = nYBValue;
			}

			char sContent[1024];
			std::string strStatus = intToString( (*iter)->nStatus );
			for (RebateIterator iterator = rebateGroupInfo[(*iter)->nId].RebatesInfo.begin(); iterator != rebateGroupInfo[(*iter)->nId].RebatesInfo.end(); iterator++)
			{	

				strStatus = strStatus + '0';
				if ( (*iter)->nId < YBRebateStartId)
				{
					// 线下返利
					if ( (*iter)->nId < SingleRebateStartId && (*iter)->nYBValue < (*iterator).second.nPay )	
					{
						continue;
					}
					
					// 单笔返利
					if ( (*iter)->nId >= SingleRebateStartId && nYBValue != (*iterator).second.nPay )	
					{
						continue;
					}
				}
				else
				{
					// 元宝返利
					if ( (*iter)->nYBValue < (*iterator).second.nMinPay )
					{
						continue;
					}
				}
				
				if ( (*iter)->nId >= SingleRebateStartId && (*iter)->nId < YBRebateStartId )
				{
					OutputMsg(rmTip, "[Rebate] 玩家 %s 已完成 %d 元宝单笔返利", ((CEntity*)m_pEntity)->GetEntityName(), (*iterator).second.nPay);
				}
				

				/*
				*	nOverlay：是否可叠加
				*		   0：不可叠加
				*		   1：可叠加
				*/
				if ( (*iter)->nOverlay || ( (*iter)->nId >= SingleRebateStartId && (*iter)->nId < YBRebateStartId ) )	// 单笔返利 默认不可叠加，立刻发放
				{
					// 单笔返利 不检查领取状态(可以通过邮件记录查看 是否已发放奖励)
					if ( ( (*iter)->nId < SingleRebateStartId || (*iter)->nId >= YBRebateStartId ) && ( (*iterator).second.nIndex > strStatus.length() - 1 || strStatus[(*iterator).second.nIndex - 1] == '2') )
					{
						continue;
					}

					if ( strStatus[(*iterator).second.nIndex - 1] == '0' )
					{
						// 线下返利、单笔返利 立刻发放
						if ( (*iter)->nAtonce )
						{
							// 发放奖励
							memset(&sContent, 0, sizeof(sContent));
							sprintf(sContent, rebateGroupInfo[(*iter)->nId].RebatesInfo[(*iterator).second.nIndex].sContent, rebateGroupInfo[(*iter)->nId].RebatesInfo[(*iterator).second.nIndex].nPay);

							CVector<ACTORAWARD> awardlist;
							for(int j = 0; (j < (*iterator).second.vecAwardList.size()) && (j < MAILATTACHCOUNT); j++) {

								ACTORAWARD actorAward = (*iterator).second.vecAwardList[j];
								if (actorAward.wId == 0) {
									continue;
								}
								actorAward.wCount = actorAward.wCount;
								awardlist.add(actorAward);
							}

							CMailSystem::SendMailByAward((*iter)->nActorId, (*iterator).second.sTitle, sContent, awardlist);

							if ( (*iter)->nId  >= SingleRebateStartId && (*iter)->nId < YBRebateStartId )
							{
								OutputMsg(rmTip, "[Rebate] 玩家 %s 已完成 %d 元宝单笔返利 邮件已发送", ((CEntity*)m_pEntity)->GetEntityName(), rebateGroupInfo[(*iter)->nId].RebatesInfo[(*iterator).second.nIndex].nPay);
							}

							// 更新线下返利、元宝返利 领取状态(单笔返利不存储领取状态，可以通过邮件查看是否已发放奖励)
							if ( (*iter)->nId < SingleRebateStartId || (*iter)->nId >= YBRebateStartId )
							{
								(*iter)->nStatus += 2*pow(10, (*iterator).second.nIndex - 1);
							}
							
							
							if ( !(*iter)->nClearTime )
							{
								(*iter)->nClearTime = (*iter)->nEndTime.rel_today() + REBATEDATASAVETIME;
							}
						} else	// 线下返利、单笔返利、元宝返利 次日发放
						{
							// 更新线下返利、元宝返利 领取状态(单笔返利不存储领取状态，可以通过邮件查看是否已发放奖励)
							if ( (*iter)->nId < SingleRebateStartId || (*iter)->nId >= YBRebateStartId )
							{
								(*iter)->nStatus += pow(10, (*iterator).second.nIndex - 1);
							}
						}
					}
				} 
				else
				{
					m_RunningRebates[(*iter)->nId].nStatus = pow(10, (*iterator).second.nIndex - 1);
				}
			}

			// 单笔返利暂不存数据库
			if ( !( (*iter)->nId >= SingleRebateStartId && (*iter)->nId < YBRebateStartId ) )
			{
				SaveRebates[(*iter)->nId] = m_RunningRebates[(*iter)->nId];
			}

			iter++;
		}
	}
	

	// 保存到数据库
	if ( !SaveRebates.size() )
	{
		return;
	}

	CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveRebateData);
	dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket << (unsigned int)m_pEntity->GetId();
	dataPacket << (unsigned int)SaveRebates.size();

	for (RebateMapIterator iter = SaveRebates.begin(); iter != SaveRebates.end(); iter++)
	{
		dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
			<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
	}

	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
	
}

void CRebateSystem::ClearOneDayRechargeAndOneDayConsumeYBNum()
{
	if(!m_pEntity) return;
	
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if ( !pDbClient->connected() || !m_RunningRebates.size() )
    {
		return;
	}

	std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();
	
	CMiniDateTime nCurTime = GetGlobalLogicEngine()->getMiniDateTime();
	
	std::map<int, RebateData> SaveRebates;
	SaveRebates.clear();
	for (RebateSetIterator iter = m_RunningSeq.begin(); iter != m_RunningSeq.end(); )
	{
		// 判断指针
		if ( !(*iter) )
		{
			OutputMsg(rmTip, "[Rebate] 运行的返利活动 GGGG (*iter) 指针非法 !");
			break;
		}

		if ( rebateGroupInfo.find((*iter)->nId) == rebateGroupInfo.end() )
		{
			OutputMsg(rmTip, "[Rebate] 运行的返利活动，被释放 AAAA ，活动ID=%d !", (*iter)->nId);
			m_RunningRebates.erase(m_RunningRebates.find((*iter)->nId));
        	iter = m_RunningSeq.erase(iter);
			continue;
		} 
		else
		{
			if ( ((*iter)->nOfflineType == eRebateOfflineType_OneDayRecharge || (*iter)->nOfflineType == eRebateOfflineType_OneDayConsume)
			&& !nCurTime.isSameDay((*iter)->nStartTime) && (*iter)->nYBValue && (*iter)->nStatus )
			{
				m_RunningRebates[(*iter)->nId].nClearTime = 0;
				m_RunningRebates[(*iter)->nId].nYBValue = 0;
				m_RunningRebates[(*iter)->nId].nStatus = 0;
				SaveRebates[(*iter)->nId] = m_RunningRebates[(*iter)->nId];
			}

			iter++;
		}
	}

	// 保存到数据库
	if ( !SaveRebates.size() )
	{
		return;
	}

	CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveRebateData);
	dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket << (unsigned int)m_pEntity->GetId();
	dataPacket << (unsigned int)SaveRebates.size();
	
	for (RebateMapIterator iter = SaveRebates.begin(); iter != SaveRebates.end(); iter++)
	{
		dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
			<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
	}

	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
}

void CRebateSystem::SendNextDayAward()
{
	if(!m_pEntity) return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected() && m_SendNextDayAwardRebates.size() )
    {
		std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();
		char sContent[1024];
		int nStatus = 0;
		for ( RebateMapIterator iter = m_SendNextDayAwardRebates.begin(); iter!= m_SendNextDayAwardRebates.end(); iter++ )
		{
			nStatus = 0;
			std::string strStatus = intToString( (*iter).second.nStatus );
			for (int i = strStatus.length() - 1; i >= 0; i--)
			{
				if ( strStatus[i] == '1' )
				{
					if ( rebateGroupInfo.find( (*iter).second.nId ) != rebateGroupInfo.end() && rebateGroupInfo[(*iter).second.nId].RebatesInfo.find( strStatus.length() - i ) != rebateGroupInfo[(*iter).second.nId].RebatesInfo.end() )
					{

						memset(&sContent, 0, sizeof(sContent));
						sprintf(sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].nPay);

						CVector<ACTORAWARD> awardlist;
						if ( (*iter).second.nId < YBRebateStartId)
						{
							sprintf(sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].nPay);
							for(int j = 0; (j < rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].vecAwardList.size()) && (j < MAILATTACHCOUNT); j++) {

								ACTORAWARD actorAward = rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].vecAwardList[j];
								if (actorAward.wId == 0) {
									continue;
								}
								awardlist.add(actorAward);
							}
						}
						else 
						{
							// 元宝返利
							sprintf(sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sContent, (*iter).second.nYBValue);

							ACTORAWARD actorAward;
							actorAward.btType = 4;
							actorAward.wId = 4;
							actorAward.wCount = (int)( (*iter).second.nYBValue * (rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].nRebatePercentage / 10000.0));
							awardlist.add(actorAward);
						}
						

						CMailSystem::SendMailByAward((*iter).second.nActorId, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sTitle, sContent, awardlist);
						
						if ( (*iter).second.nOverlay )
						{
							nStatus += 2*pow(10, i);
						} else
						{
							nStatus = 2*pow(10, i);
						}
					}
				}
			}

			// 更新 次日发放、（可叠加 或 不可叠加）返利活动的 状态码
			if ( nStatus )
			{
				if ( m_RunningRebates.find((*iter).second.nId) != m_RunningRebates.end() )
				{
					m_RunningRebates[(*iter).second.nId].nStatus = nStatus;
					m_RunningRebates[(*iter).second.nId].nClearTime = (*iter).second.nEndTime.today() + REBATEDATASAVETIME;
				}

				(*iter).second.nStatus = nStatus;
				(*iter).second.nClearTime = (*iter).second.nEndTime.today() + REBATEDATASAVETIME;
			}
		}

		CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateRebateData);
		dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		dataPacket << (unsigned int)m_pEntity->GetId();
		dataPacket << (unsigned int)m_SendNextDayAwardRebates.size();

		// 更新数据库中 次日发放、（可叠加 或 不可叠加）返利活动的 状态码
		for ( RebateMapIterator iter = m_SendNextDayAwardRebates.begin(); iter!= m_SendNextDayAwardRebates.end(); iter++ )
		{
			dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
			<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
		}

		GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
		
		m_SendNextDayAwardRebates.clear();
	}
}

void CRebateSystem::CheckNextDayAward()
{
	if(!m_pEntity) return;
	
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadNextDayRebateData);
		DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)m_pEntity->GetId();
		DataPacket << (int)0;//struct RebateData : nAtonce // 0：次日发放；1：立刻发放奖励
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CRebateSystem::SendAward()
{
	if(!m_pEntity || !m_pEntity->IsInited()) return;

	// 发放 已结束、立即发放、未发放奖励 的返利活动
	if ( m_SendImmediatelyAwardRebates.size() )
	{
		SendImmediatelyAward();
	}
}

void CRebateSystem::SendImmediatelyAward()
{
	if(!m_pEntity) return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if ( pDbClient->connected() )
    {
		std::map<int, RebateGroupInfo>& rebateGroupInfo = GetLogicServer()->GetDataProvider()->GetRebateProvider().GetRebateConfig();
		char sContent[1024];
		int nStatus = 0;
		for ( RebateMapIterator iter = m_SendImmediatelyAwardRebates.begin(); iter!= m_SendImmediatelyAwardRebates.end(); iter++ )
		{
			nStatus = 0;
			std::string strStatus = intToString( (*iter).second.nStatus );
			for (int i = strStatus.length() - 1; i >= 0; i++)
			{
				if ( strStatus[i] == '1' )
				{
					if ( rebateGroupInfo.find( (*iter).second.nId ) != rebateGroupInfo.end() && rebateGroupInfo[(*iter).second.nId].RebatesInfo.find( strStatus.length() - i ) != rebateGroupInfo[(*iter).second.nId].RebatesInfo.end() )
					{
						
						memset(&sContent, 0, sizeof(sContent));
						sprintf(sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sContent, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].nPay);

						CVector<ACTORAWARD> awardlist;
						for(int j = 0; (j < rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].vecAwardList.size()) && (j < MAILATTACHCOUNT); j++) {

							ACTORAWARD actorAward = rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].vecAwardList[j];
							if (actorAward.wId == 0) {
								continue;
							}
							actorAward.wCount = actorAward.wCount;
							awardlist.add(actorAward);
						}

						CMailSystem::SendMailByAward((*iter).second.nActorId, rebateGroupInfo[(*iter).second.nId].RebatesInfo[strStatus.length() - i].sTitle, sContent, awardlist);
						
						if ( (*iter).second.nOverlay )
						{
							nStatus += 2*pow(10, i);
						} else
						{
							nStatus = 2*pow(10, i);
						}
					}
				}
			}

			// 更新 已经结束、不可叠加、立即发放 返利活动的 状态码
			if ( nStatus )
			{
				(*iter).second.nStatus = nStatus;
				(*iter).second.nClearTime = (*iter).second.nEndTime.today() + REBATEDATASAVETIME;
			}
		}

		CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcUpdateRebateData);
		dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		dataPacket << (unsigned int)m_pEntity->GetId();
		dataPacket << (unsigned int)m_SendImmediatelyAwardRebates.size();

		// 更新数据库中 已经结束、次日发放、（可叠加 或 不可叠加）返利活动的 状态码
		for ( RebateMapIterator iter = m_SendImmediatelyAwardRebates.begin(); iter!= m_SendImmediatelyAwardRebates.end(); iter++ )
		{
			dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
			<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
		}

		GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

		m_SendImmediatelyAwardRebates.clear();
	}
}

void CRebateSystem::ClearTimeOutRebates()
{
	if(!m_pEntity) return;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if ( pDbClient->connected() )
    {
		CDataPacket& dataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcClearRebateData);
		dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		dataPacket << (unsigned int)m_pEntity->GetId();
		dataPacket << (unsigned int)m_ClearTimeOutRebates.size();

		for ( RebateMapIterator iter = m_ClearTimeOutRebates.begin(); iter!= m_ClearTimeOutRebates.end(); iter++ )
		{
			OutputMsg(rmTip, " ClearTimeOutRebates 玩家 %s，活动Id : %d, 活动结束时间 ：%d, 活动清理时间 ：%d", ((CEntity*)m_pEntity)->GetEntityName(), (*iter).second.nId, (int)(*iter).second.nEndTime, (int)(*iter).second.nClearTime);
			dataPacket << (*iter).second.nId << (*iter).second.nTimeType << (*iter).second.nStartTime << (*iter).second.nEndTime 
			<< (*iter).second.nClearTime << (*iter).second.nOfflineType << (*iter).second.nAtonce << (*iter).second.nOverlay << (*iter).second.nYBValue << (*iter).second.nStatus;
		}

		GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

		m_ClearTimeOutRebates.clear();
	}
}

