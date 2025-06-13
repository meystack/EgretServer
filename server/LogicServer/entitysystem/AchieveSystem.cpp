#include "StdAfx.h"
#include "AchieveSystem.h"
#include "../script/interface/LogDef.h"

//网络数据处理
void  CAchieveSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &inPack )
{
	if(m_pEntity ==NULL) return;
	//这里进行数据处理	
	
	if(cGetAchieveData == nCmd )		 //获取成就的数据(注意：已经完成了的成就的数据是不下发的)
	{
		SendAchieveInfoByTab(inPack);
	}
	else if( cGetAchieveAwards ==nCmd)   //获取成就的奖励
	{
		GetAchieveAwardByTasKId(inPack);
	}
	else if( cPushAchieveRedPoint ==nCmd)   //请求红点
	{
		SendAchieveRedPoint();
	}
	else if( cGetMedalData ==nCmd)   //获取勋章的等级
	{
		GetActorMedalLevel(inPack);
	}
	else if( cUpMedalLevel ==nCmd)   //升级勋章
	{

		MedalLevelUP(inPack);
	}
	// else if( cGetNewAchieveAwards ==nCmd)   //获取成就的奖励
	// {
	// 	WORD nChieveID =0;					//成就的ID
	// 	inPack >> nChieveID; 
	// 	GetAchieveAwards(nChieveID);	
	// }
}

INT_PTR CAchieveSystem::GetGiveAwardLogId(INT_PTR nType)
{
	switch (nType)
	{
	case qatEquipment:
		return GameLog::clAchieveRewardItem;
	case qatBindMoney:
	case qatMoney:
	case qatYuanbao:
	case qatBindYb:
		 return GameLog::clAchieveRewardMoney;
	default:
		return GameLog::clAchieveAwardOther;
	}
	
}

//玩家能否获得这个成就的奖励
bool CAchieveSystem::CanGetAward(PACHIEVEAWARD pAward)
{
	if ( !m_pEntity || !pAward )
	{
		return false;
	}
	int nSex      = m_pEntity->GetProperty<int>(PROP_ACTOR_SEX); //性别
	int nVocation = m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION); //职业
	
	if(pAward->nSex >=0 && pAward->nSex != nSex) //职业不符合
	{
		return false;
	}
	if(pAward->nVocation >0 && pAward->nVocation != nVocation)
	{
		return false;
	}

	return true;
}

bool CAchieveSystem::GetGiftAwardsEvent(INT_PTR nAchieveID, byte & nCombineFlag)
{
	// CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	// CVector<int>* pEvents= provider.GetAtomEvents(aAchieveDrawYuBao);
	// if(pEvents ==NULL) return false;

	// for(INT_PTR i=0; i< pEvents->count();i++)
	// {
	// 	int nAchieveEvent= pEvents->get(i); //成就事件

	// 	PACHIEVEEVENT pAchieveEvent= provider.GetAchieveEvent(nAchieveEvent); //获取成就的事件6
	// 	if(pAchieveEvent ==NULL) continue;

	// 	for(INT_PTR j=0; j < pAchieveEvent->achieveIDList.count; j++)
	// 	{
	// 		int tmpAchieveID =  pAchieveEvent->achieveIDList[j]; 

	// 		if(tmpAchieveID == nAchieveID)
	// 		{
	// 			if (pAchieveEvent->conds.count > 1)
	// 			{
	// 				PACHIEVEDATA pAchieve= provider.GetAchieveData(nAchieveID);
	// 				if (!pAchieve->isDayRefresh)
	// 				{
	// 					nCombineFlag = 1;
	// 				}					
	// 			}
	// 			return true;
	// 		}
	// 	}
	// }

	return false;
}



bool CAchieveSystem::GetAchieveAwards(INT_PTR nAchieveID,bool defaultMode)
{
	if ( !m_pEntity )
	{
		return false;
	}
	//还没完成	|| 领过了
	if(IsAchieveFinished(nAchieveID) ==false ) 
	{
		//if(bGiftState)
		m_pEntity->SendOldTipmsgFormatWithId(tpIsAchieveFinished, ttFlyTip);
		SendAchieveResult((WORD)nAchieveID,0);
		return false; 
	}
	if(IsAchieveGiveAwards(nAchieveID))
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpIsAchieveGiveAwards, ttFlyTip);
		SendAchieveResult((WORD)nAchieveID,0);
		return false;
	}

	//脚本发奖
	CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	if (pNpc == NULL) return false;
	CScriptValueList paramList, retParamList;
	paramList << m_pEntity << nAchieveID;
	pNpc->GetScript().Call("OnGetAchieveAwards",paramList,retParamList);
	return true;
}

bool CAchieveSystem::GetRangeConditonFlag(int nValue, DataList<int> & datalist)
{
	if(datalist.count !=2) return false; //数据有错误
	int nMin = *(datalist.pData );
	int nMax = *(datalist.pData +1);
	if(nValue >=nMin && nValue <= nMax )
	{
		return true;
	}
	else
	{
		return false;
	}
	
}
bool CAchieveSystem::GetEnumConditionFlag(int nValue, DataList<int> & datalist)
{
	int * pBase = datalist.pData;
	for(INT_PTR i=0; i< datalist.count; i++)
	{
		if(*pBase == nValue)
		{
			return true;
		}
		pBase ++;
	}
	return false;
}

void CAchieveSystem::SetAchieveInstantFinish(INT_PTR nAchieveId,bool boGm)
{
	// CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	// PACHIEVEDATA pMyAchieve= provider.GetAchieveData(nAchieveId); 
	// if (!pMyAchieve)
	// 	return;

	// INT_PTR nAtomID = 0;
	// INT_PTR nParam1 = 0;
	// INT_PTR nParam2 = 0;
	// INT_PTR nParam3 = 0;
	// INT_PTR nParam4 = 0;

	// for (int i =0; i< pMyAchieve->conditions.count; i++)
	// {
	// 	ACHIEVECONDITION& pMyAchieveCond = pMyAchieve->conditions[i];
		
	// 	PACHIEVEEVENT pMyAchieveEvent = provider.GetAchieveEvent(pMyAchieveCond.wEventID);
	// 	if ( !pMyAchieveEvent )
	// 	{
	// 		continue;
	// 	}
	// 	PACHIEVEEVENTCOND pMyAchieveEventCondsBase = (PACHIEVEEVENTCOND)pMyAchieveEvent->conds.pData;
	// 	if ( !pMyAchieveEventCondsBase)
	// 	{
	// 		continue;
	// 	}
	// 	PACHIEVEEVENTCOND  pMyOneCondition  =NULL;
	// 	for (int k =0; k< pMyAchieveEvent->conds.count; k++)
	// 	{
	// 		pMyOneCondition = pMyAchieveEventCondsBase+k;
	// 		if (!pMyOneCondition)
	// 			continue;
	// 		if (pMyOneCondition->bParamID == 1)
	// 			nParam1 = pMyOneCondition->params[0];
	// 		if (pMyOneCondition->bParamID == 2)
	// 			nParam2 = pMyOneCondition->params[0];
	// 		if (pMyOneCondition->bParamID == 3)
	// 			nParam3 = pMyOneCondition->params[0];
	// 		if (pMyOneCondition->bParamID == 4)
	// 			nParam4 = pMyOneCondition->params[0];
	// 	}
	// 	if (pMyAchieveEvent)
	// 	{
	// 		nAtomID = pMyAchieveEvent->wAtomEventID;

	// 		if(nAtomID >= eMaxAtomEventID || nAtomID <0) return; //事件不对

	// 		if(m_atomEvents[nAtomID] <=0 )return; //没有订阅这个事件
	// 		CVector<int>* pEvents= provider.GetAtomEvents(nAtomID);
	// 		if(pEvents ==NULL) return;
	// 		for(INT_PTR i=0; i< pEvents->count();i++) 
	// 		{
	// 			int nAchieveEvent= pEvents->get(i); //成就事件
	// 			if(IsEventSubscribed(nAchieveEvent) ==false) continue; //一个成就事件是否订阅了
	// 			PACHIEVEEVENT pAchieveEvent= provider.GetAchieveEvent(nAchieveEvent); //获取成就的事件6
	// 			if(pAchieveEvent ==NULL) continue;
	// 			PACHIEVEEVENTCOND pBaseConds = (PACHIEVEEVENTCOND)pAchieveEvent->conds.pData;
	// 			PACHIEVEEVENTCOND  pOneCondition  =NULL;
	// 			bool isFinish =true;
	// 			for(INT_PTR j=0;j <pAchieveEvent->conds.count; j++)
	// 			{					
	// 				pOneCondition= pBaseConds + j;
	// 				INT_PTR  nValue = 0;
	// 				if(pOneCondition->bParamID  ==1)
	// 				{
	// 					nValue = nParam1;
	// 				}
	// 				else if(pOneCondition->bParamID  ==2)
	// 				{
	// 					nValue = nParam2;
	// 				}
	// 				else if(pOneCondition->bParamID  ==3)
	// 				{
	// 					nValue = nParam3;
	// 				}
	// 				else if(pOneCondition->bParamID  ==4)
	// 				{
	// 					nValue = nParam4;
	// 				}
	// 				else
	// 				{
	// 					continue;
	// 				}

	// 				if( pOneCondition->bParamType == eAchieveConditionNone )
	// 				{
	// 					continue;
	// 				}
	// 				else if( pOneCondition->bParamType == eAchieveConditionEnum) //枚举型的
	// 				{
	// 					isFinish = GetEnumConditionFlag((int)nValue,pOneCondition->params);
	// 				}
	// 				else if(pOneCondition->bParamType == eAchieveConditionRange)
	// 				{
	// 					isFinish = GetRangeConditonFlag((int)nValue,pOneCondition->params);
	// 				}
	// 				if(!isFinish )
	// 				{
	// 					break;
	// 				}
	// 			}
	// 			if(!isFinish ) continue;
	// 			INT_PTR nPos,nCount,nOldCount;
	// 			GetAchieveEventDataPosCount(nAchieveEvent,nPos,nCount); //这个的存储位置
	// 			nOldCount = nCount; //老的数量，这里暂时不用

	// 			//一次事件成功添加的数目
	// 			INT_PTR nPlusCount =1;
	// 			//if(pAchieveEvent->bPlusParamId >0)
	// 			//{
	// 			//	if(pAchieveEvent->bPlusParamId  ==1)
	// 			//	{
	// 			//		nPlusCount = nParam1;
	// 			//	}
	// 			//	else if(pAchieveEvent->bPlusParamId  ==2)
	// 			//	{
	// 			//		nPlusCount = nParam2;
	// 			//	}
	// 			//	else if(pAchieveEvent->bPlusParamId ==3)
	// 			//	{
	// 			//		nPlusCount = nParam3;
	// 			//	}
	// 			//}
	// 			nPlusCount = pMyAchieveCond.nCount;
				
				

	// 			//有哪些成就关注这个事件
	// 			INT_PTR nAchieveFinishCount=0;
	// 			bool    bHasWriteProcess =false ; //是否已经写了进度

	// 			for(INT_PTR j=0; j < pAchieveEvent->achieveIDList.count; j++)
	// 			{
	// 				int nAchieveID =  pAchieveEvent->achieveIDList[j]; 
	// 				if( IsAchieveFinished(nAchieveID) )
	// 				{
	// 					nAchieveFinishCount ++;
	// 					continue; //如果成就已经完成的话
	// 				}
	// 				PACHIEVEDATA pAchieve= provider.GetAchieveData(nAchieveID); 

	// 				INT_PTR nCondCount = pAchieve->conditions.count;
	// 				if(nCondCount <=0) continue;
	// 				PACHIEVECONDITION pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// 				PACHIEVECONDITION pEventCondition =NULL; //完成一个事件的条件

	// 				//查找到这个成就事件
	// 				for(INT_PTR k=0; k< nCondCount; k++)
	// 				{
	// 					if( pCond->wEventID ==  nAchieveEvent   )
	// 					{
	// 						pEventCondition = pCond;
	// 						break;
	// 					}
	// 					pCond++;
	// 				}
	// 				if(pEventCondition ==NULL) continue;
	// 				//if(pEventCondition->nCount  <=  nOldCount )  continue; //这个成就已经关注了
	// 				if (pEventCondition->nCount < nCount + nPlusCount)
	// 				{
	// 					if (pEventCondition->nCount > nCount)
	// 					{
	// 						nPlusCount = pEventCondition->nCount - nCount;
	// 					}
	// 				}
	// 				nCount += nPlusCount; //一次成就触发添加的数目
	// 				if(!bHasWriteProcess )
	// 				{
	// 					bHasWriteProcess = AchieveConditionPlus(pEventCondition,nCondCount,nPlusCount);
	// 				}
	// 				if(m_hasSendInitData)
	// 				{
	// 					CActorPacket pack;
	// 					m_pEntity->AllocPacket(pack);
	// 					pack << (BYTE )GetSystemID() << (BYTE) sAchieveEventTrigger << (WORD)nAchieveID <<WORD(nAchieveEvent)<<(int)nCount;
	// 					pack.flush();
	// 				}

					
	// 				bool isFinish  =true;
	// 				if(nCondCount >1  || pEventCondition->nCount > 1) //只有1个条件并且只需要完成1次的
	// 				{
	// 					pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// 					for(INT_PTR k=0; k< nCondCount; k++)
	// 					{
	// 						INT_PTR nDataPos,nDataCount;
	// 						GetAchieveEventDataPosCount(pCond->wEventID,nDataPos,nDataCount);
	// 						if( nDataCount < pCond->nCount )
	// 						{
	// 							isFinish =false;
	// 							break;
	// 						}
	// 						pCond++;
	// 					}
	// 				}
	// 				if(isFinish)
	// 				{
	// 					if(m_atomEvents[nAtomID] >0 )
	// 					{
	// 						m_atomEvents[nAtomID] --;
	// 					}
	// 					nAchieveFinishCount ++; 
	// 					SetAchieveFinished(nAchieveID,boGm);
	// 				}
	// 			}
	// 			if(nAchieveFinishCount >= pAchieveEvent->achieveIDList.count )
	// 			{
	// 				UnsubscribeEvent(nAchieveEvent); //取消订阅这个消息
	// 			}
	// 		}
	// 	}
	// }
}

void CAchieveSystem::OnEvent(INT_PTR nAtomID, INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4 )
{
	// if(nAtomID >= eMaxAtomEventID || nAtomID <0) return; //事件不对


	// if(m_atomEvents[nAtomID] <=0 )return; //没有订阅这个事件
	// CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	// CVector<int>* pEvents= provider.GetAtomEvents(nAtomID);
	// if(pEvents ==NULL) return;
	// for(INT_PTR i=0; i< pEvents->count();i++)
	// {
	// 	int nAchieveEvent= pEvents->get(i); //成就事件
		
	// 	if(IsEventSubscribed(nAchieveEvent) ==false) continue; //一个成就事件是否订阅了
		
	// 	PACHIEVEEVENT pAchieveEvent= provider.GetAchieveEvent(nAchieveEvent); //获取成就的事件6
	// 	if(pAchieveEvent ==NULL) continue;
		
	// 	//int nAchieveID = pAchieveEvent->bAchieveID; //成就的ID
		
	// 	PACHIEVEEVENTCOND pBaseConds = (PACHIEVEEVENTCOND)pAchieveEvent->conds.pData;
	// 	if ( !pBaseConds )
	// 	{
	// 		continue;
	// 	}
	// 	PACHIEVEEVENTCOND  pOneCondition  =NULL;
	// 	bool isFinish =true;
	// 	for(INT_PTR j=0;j <pAchieveEvent->conds.count; j++)
	// 	{					
	// 		pOneCondition= pBaseConds + j;
	// 		INT_PTR  nValue = 0;
	// 		if(pOneCondition->bParamID  ==1)
	// 		{
	// 			nValue = nParam1;
	// 		}
	// 		else if(pOneCondition->bParamID  ==2)
	// 		{
	// 			nValue = nParam2;
	// 		}
	// 		else if(pOneCondition->bParamID  ==3)
	// 		{
	// 			nValue = nParam3;
	// 		}
	// 		else if(pOneCondition->bParamID  ==4)
	// 		{
	// 			nValue = nParam4;
	// 		}
	// 		else
	// 		{
	// 			continue;
	// 		}

	// 		if( pOneCondition->bParamType == eAchieveConditionNone )
	// 		{
	// 			continue;
	// 		}
	// 		else if( pOneCondition->bParamType == eAchieveConditionEnum) //枚举型的
	// 		{
	// 			isFinish = GetEnumConditionFlag((int)nValue,pOneCondition->params);
	// 		}
	// 		else if(pOneCondition->bParamType == eAchieveConditionRange)
	// 		{
	// 			isFinish = GetRangeConditonFlag((int)nValue,pOneCondition->params);
	// 		}
	// 		if(!isFinish )
	// 		{
	// 			break;
	// 		}
	// 	}
	// 	if(!isFinish ) continue;
	// 	INT_PTR nPos,nCount,nOldCount;
	// 	GetAchieveEventDataPosCount(nAchieveEvent,nPos,nCount); //这个的存储位置
	// 	nOldCount = nCount; //老的数量，这里暂时不用

	// 	//一次事件成功添加的数目
	// 	INT_PTR nPlusCount =1;
	// 	if(pAchieveEvent->bPlusParamId >0)
	// 	{
	// 		if(pAchieveEvent->bPlusParamId  ==1)
	// 		{
	// 			nPlusCount = nParam1;
	// 		}
	// 		else if(pAchieveEvent->bPlusParamId  ==2)
	// 		{
	// 			nPlusCount = nParam2;
	// 		}
	// 		else if(pAchieveEvent->bPlusParamId ==3)
	// 		{
	// 			nPlusCount = nParam3;
	// 		}
	// 	}
	// 	nCount += nPlusCount; //一次成就触发添加的数目
		
	// 	//有哪些成就关注这个事件
	// 	INT_PTR nAchieveFinishCount=0;
	// 	bool    bHasWriteProcess =false ; //是否已经写了进度
	
	// 	for(INT_PTR j=0; j < pAchieveEvent->achieveIDList.count; j++)
	// 	{
	// 		int nAchieveID =  pAchieveEvent->achieveIDList[j]; 
	// 		if( IsAchieveFinished(nAchieveID) )
	// 		{
	// 			nAchieveFinishCount ++;
	// 			continue; //如果成就已经完成的话
	// 		}
	// 		PACHIEVEDATA pAchieve= provider.GetAchieveData(nAchieveID); 
			
	// 		INT_PTR nCondCount = pAchieve->conditions.count;
	// 		if(nCondCount <=0) continue;
	// 		PACHIEVECONDITION pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// 		PACHIEVECONDITION pEventCondition =NULL; //完成一个事件的条件

	// 		//查找到这个成就事件
	// 		for(INT_PTR k=0; k< nCondCount; k++)
	// 		{
	// 			if( pCond->wEventID ==  nAchieveEvent   )
	// 			{
	// 				pEventCondition = pCond;
	// 				break;
	// 			}
	// 			pCond++;
	// 		}
	// 		if(pEventCondition ==NULL) continue;
	// 		//if(pEventCondition->nCount  <=  nOldCount )  continue; //这个成就已经关注了

	// 		if(m_hasSendInitData)
	// 		{
	// 			CActorPacket pack;
 	// 			m_pEntity->AllocPacket(pack);
	// 			pack << (BYTE )GetSystemID() << (BYTE) sAchieveEventTrigger << (WORD)nAchieveID <<WORD(nAchieveEvent)<<(int)nCount;
	// 			pack.flush();
	// 		}

	// 		if(!bHasWriteProcess )
	// 		{
	// 			bHasWriteProcess = AchieveConditionPlus(pEventCondition,nCondCount,nPlusCount);
	// 		}
	// 		bool isFinish  =true;
	// 		if(nCondCount >1  || pEventCondition->nCount > 1) //只有1个条件并且只需要完成1次的
	// 		{
	// 			pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// 			for(INT_PTR k=0; k< nCondCount; k++)
	// 			{
	// 				INT_PTR nDataPos,nDataCount;
	// 				GetAchieveEventDataPosCount(pCond->wEventID,nDataPos,nDataCount);
	// 				if( nDataCount < pCond->nCount )
	// 				{
	// 					isFinish =false;
	// 					break;
	// 				}
	// 				pCond++;
	// 			}
	// 		}
	// 		if(isFinish)
	// 		{
	// 			if(m_atomEvents[nAtomID] >0 )
	// 			{
	// 				m_atomEvents[nAtomID] --;
	// 			}
	// 			nAchieveFinishCount ++; 
	// 			SetAchieveFinished(nAchieveID);
	// 		}
	// 	}
	// 	if(nAchieveFinishCount >= pAchieveEvent->achieveIDList.count )
	// 	{
	// 		UnsubscribeEvent(nAchieveEvent); //取消订阅这个消息
	// 	}
	// }
}





void CAchieveSystem::Save(PACTORDBDATA  pActorData)
{
	
	memcpy(pActorData->achieveFinishState, m_achieveFinishState,sizeof(m_achieveFinishState));
	memcpy(pActorData->achieveEventData, m_achieveEventData,sizeof(m_achieveEventData));
	memcpy(pActorData->titles,m_titles,sizeof(m_titles)); //当前的称号
	SaveData();
}

bool CAchieveSystem::Initialize(void *data,SIZE_T size)
{
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}
	//将db的数据拷贝过来
	memcpy( m_achieveFinishState,pActorData->achieveFinishState,sizeof(m_achieveFinishState));
	memcpy( m_achieveEventData,pActorData->achieveEventData,sizeof(m_achieveEventData));
	memcpy(m_titles,pActorData->titles,sizeof(m_titles)); //当前的称号
	//InitAchieveEvent(); //初始化成就事件
	

	return true;
}



void  CAchieveSystem::GetAchieveEventDataPosCount( INT_PTR nEventID,INT_PTR &nPos,INT_PTR &nCount )
{

	PACHIEVECONDITION pBase =  (PACHIEVECONDITION)&m_achieveEventData;
	if ( !pBase )
	{
		return;
	}
	for(INT_PTR i=0; i < m_nAchieveEventDbUseCount; i++ )
	{
		if(pBase->nCount && pBase->wEventID == (WORD)nEventID )
		{
			nPos   = i;
			nCount = pBase->nCount;
			return;
		}
		pBase ++;
	}
	nPos =-1;
	nCount =0;
}

bool    CAchieveSystem::AchieveConditionPlus(PACHIEVECONDITION pCondition,INT_PTR nConditionCount,INT_PTR progressPlus)
{
	if ( !pCondition )
	{
		return false;
	}
	INT_PTR nCount = pCondition->nCount;
	
	if(nCount >1 || nConditionCount >1) //需要存储
	{
		PACHIEVECONDITION pBase =  (PACHIEVECONDITION)&m_achieveEventData;
		if ( !pBase )
		{
			return false;
		}
		INT_PTR nEmptyId = -1; //空的位置，如果没有找到旧的，就从这里找一个新的放里边
		for(INT_PTR i=0; i < m_nAchieveEventDbUseCount; i++ )
		{
			if( pBase->wEventID == pCondition->wEventID )
			{
				if(pBase->nCount < pCondition->nCount)
				{
					int nNewProgress = (int)progressPlus + pBase->nCount ;
					pBase->nCount = nNewProgress;
				}
				return true;
			}
			else
			{
				if(pBase->wEventID ==0 && nEmptyId <0)
				{
					nEmptyId = i;
				}
			}
			pBase ++;
		}
		if(nEmptyId <0)
		{
			
			INT_PTR nTotalIntCount= sizeof(m_achieveEventData) >> 2;  //已经用了很多了，再也无法添加了
			if(m_nAchieveEventDbUseCount >= nTotalIntCount)
			{
				OutputMsg(rmError,_T("无法为成就添加更多的存储空间,eventID=%d"),pCondition->wEventID);
				return false; 
			}
			nEmptyId = m_nAchieveEventDbUseCount; //使用一个
			m_nAchieveEventDbUseCount ++;
		}
		pBase = (PACHIEVECONDITION)&m_achieveEventData;
		if ( !pBase )
		{
			return false;
		}
		if(nEmptyId > 0)
		{
			pBase += nEmptyId;
		}
		
		pBase->wEventID =   pCondition->wEventID ;
		//int nNewProgress = (int)progressPlus;
		//nNewProgress = min(nNewProgress,pCondition->nCount);

		pBase->nCount =  (int) progressPlus ;
		
		
		return true;
	}
	else
	{
		return false; //如果没有数据表示就是完成了
	}
}
void CAchieveSystem::OnLevelUp()
{
	InitAchieveEvent();
}

void CAchieveSystem::InitAchieveEvent()
{
	// ResetAchieveGroup(); //重置了成就组的完成情况
	
	// memset(m_atomEvents,0,sizeof(m_atomEvents));
	// memset(m_subscribeEvent,0,sizeof(m_subscribeEvent));

	// PACHIEVECONDITION  pEvent= (PACHIEVECONDITION)&m_achieveEventData;
	// if ( !pEvent )
	// {
	// 	return;
	// }
	// INT_PTR nTotalIntCount= sizeof(m_achieveEventData) / sizeof(ACHIEVECONDITION) ;
	// for(INT_PTR i=0; i < nTotalIntCount; i++)
	// {

	// 	if( pEvent->nCount >0)
	// 	{
	// 		m_nAchieveEventDbUseCount =(int) (i + 1); //保存这份最大的值，免得每次都去遍历
	// 	}
	// 	pEvent ++;
	// }
	// CAchieveProvider & provider = GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	// INT_PTR nAchieveCount = provider.GetAchieveCount(); //获取成就的数量
	// nAchieveCount = __min(nAchieveCount,MAX_ACHIEVE_DATA_SIZE  <<2 );
	// for(INT_PTR i=0; i< nAchieveCount; i++) //成就的个数
	// {
	// 	if( IsAchieveFinished(i) ==false) //如果一个成就没有完成，需要记录一下
	// 	{
	// 		PACHIEVEDATA pAchieve=  provider.GetAchieveData(i);
	// 		if(pAchieve ==NULL) continue;
	// 		if(pAchieve->bIsDelete) continue; //如果已经删除了就不处理了
	// 		INT_PTR nGroupId = pAchieve->groupId;
	// 		//如果成就组没有激活或者成就不是默认激活的，那么该成就是不会激活的
	// 		if( IsGroupOpen(nGroupId)==false && false == pAchieve->isDefaultActive )
	// 		{
	// 			continue;
	// 		}
	// 		for(INT_PTR j=0 ; j< pAchieve->conditions.count; j++ )
	// 		{
				
	// 			PACHIEVECONDITION pCondition = &pAchieve->conditions.pData[j];
	// 			if ( !pCondition )
	// 			{
	// 				continue;
	// 			}
	// 			PACHIEVEEVENT pAchieveEvent= provider.GetAchieveEvent(pCondition->wEventID);
	// 			if(pAchieveEvent == NULL) continue;
	// 			if(pAchieveEvent->wAtomEventID >= eMaxAtomEventID) continue; //有问题
	// 			INT_PTR nDataPos,nDataCount;
	// 			GetAchieveEventDataPosCount(pCondition->wEventID,nDataPos,nDataCount);
	// 			if( nDataCount < pCondition->nCount) //没有完成成就的一个条件
	// 			{
	// 				SubscribeEvent(pCondition->wEventID); //订阅这个事件
	// 				m_atomEvents[pAchieveEvent->wAtomEventID] ++; //原子事件
	// 			}
	// 		}
	// 	}
	// 	else  //这里的代码是临时补救外服的bug的，下次要去掉
	// 	{
	// 		//TitleRepaire(i);
	// 	}
	// }
	
}
void CAchieveSystem::OnEnterGame()
{
	if ( !m_pEntity )
	{
		return;
	}
	
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator() )
	{
		return;
	}
	ActorAchievementComplete(nAchieveLevel, m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL)); //重新调用一下
	ActorAchievementComplete(nAchieveCircle, m_pEntity->GetProperty<int>(PROP_ACTOR_CIRCLE)); //重新调用一下
	FreshAllGroupTitle();			//刷新称号
}

bool CAchieveSystem::CheckGroupOpenCondition(PACHIEVEGROUP pGroup)
{
	if(pGroup== NULL) return false;

	if(pGroup->nLevel >0)
	{
		if(m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL) < pGroup->nLevel )
		{
			return false;
		}
	}
	if(pGroup->nVocation >0)
	{
		if(m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION) != pGroup->nVocation )
		{
			return false;
		}
	}

	if(pGroup->nCamp >0)
	{
		if(m_pEntity->GetCampId() != pGroup->nCamp )
		{
			return false;
		}
	}

	if(pGroup->nSex >= 0)
	{
		if(m_pEntity->GetProperty<int>(PROP_ACTOR_SEX) != pGroup->nSex )
		{
			return false;
		}
	}
	return true;
}

void CAchieveSystem::ResetAchieveGroup( )
{
	//设置哪个成就的分组是满足开启的tiao
	
	memset(m_achieveGroup,0,sizeof(m_achieveGroup));
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	INT_PTR nCount= provider.GetAchiveGroupCount(); //获取成就的分组的数目
	PACHIEVEGROUP pGroup;

	for(INT_PTR  i=0; i< nCount; i++)
	{
		pGroup= provider.GetAchiveGroup(i);
		SetGroupOpen(i,CheckGroupOpenCondition(pGroup));
	}
}

void CAchieveSystem::RefeshPerDay(bool isLogin)
{
	// if(m_pEntity == NULL || !m_pEntity->IsInited() || m_pEntity->IsDestory() )
	// {
	// 	return;
	// }
	// CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	// INT_PTR nAchieveCount = provider.GetAchieveCount(); //获取成就的数量

	// nAchieveCount = __min(nAchieveCount,MAX_ACHIEVE_DATA_SIZE  <<2 );

	// PACHIEVEDATA pAchieve;
	// INT_PTR  nBytePos;
	// INT_PTR  nBitPos;
	// INT_PTR  nXorBit;
	
	// for(INT_PTR i=0; i< nAchieveCount; i++) //成就的个数
	// {
	// 	pAchieve= provider.GetAchieveData(i);

	// 	if(pAchieve ==NULL) continue;
	// 	if(pAchieve->bIsDelete) continue; //如果已经删除了就不处理了

	// 	bool bRefresh = pAchieve->isDayRefresh;	//先判断是否每日刷新

	// 	if (!bRefresh && pAchieve->isMonthRefresh == true)	//如果每日不刷新 则判断是否没月刷新
	// 	{
	// 		if (!isLogin)
	// 		{
	// 			SYSTEMTIME sysTime;
	// 			GetLocalTime(&sysTime);
	// 			if (sysTime.wDay == 1)
	// 			{
	// 				bRefresh = true;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			CMiniDateTime nLastLogOutTime = m_pEntity->GetLastLogoutTime();
	// 			CMiniDateTime nLoginTime = m_pEntity->GetLoginTime();

	// 			if (!GetGlobalLogicEngine()->IsSameMonth(nLastLogOutTime, nLoginTime))	//判断是否同一个月
	// 			{
	// 				bRefresh = true;
	// 			}
	// 		}
			
	// 	}

	// 	//只刷新活跃度界面的
	// 	if (bRefresh == true)
	// 	{
	// 		nBytePos = i >> 2; //第多少个BYTE
	// 		nBitPos = (i &3) << 1;   //一个BYTE里的第几个Bit
	// 		nXorBit = (BYTE) ( (1 <<nBitPos) | (1 << (nBitPos+1)) ) ^ 255;
	// 		m_achieveFinishState[nBytePos] &= nXorBit;

	// 		PACHIEVECONDITION pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// 		PACHIEVECONDITION pBase =  (PACHIEVECONDITION)&m_achieveEventData;
	// 		//PACHIEVECONDITION pEnd = (PACHIEVECONDITION) &( m_achieveEventData +sizeof(m_achieveEventData));
	// 		if ( !pCond || !pBase )
	// 		{
	// 			return;
	// 		}
	// 		INT_PTR nTotalIntCount= sizeof(m_achieveEventData) / sizeof(*pBase);

	// 		for(INT_PTR j=0; j < nTotalIntCount; j++ )
	// 		{
	// 			if( pBase->wEventID == pCond->wEventID )
	// 			{
	// 				pBase->nCount = 0;
	// 				break;
	// 			}
	// 			pBase ++;
	// 		}
			
	// 	}
	// }

	// InitAchieveEvent(); //初始化成就事件

	
	// //发送数据到客户端
	// if(m_pEntity->IsInited())
	// {
	// 	CActorPacket pack;
	// 	CDataPacket &data=m_pEntity->AllocPacket(pack); 
	// 	data << (BYTE)GetSystemID() << (BYTE)sSendAchieveData;
	// 	data.writeBuf(m_achieveFinishState,sizeof(m_achieveFinishState));
	// 	data.writeBuf(m_achieveEventData,sizeof(m_achieveEventData));
	// 	pack.flush();
	// 	//m_hasSendInitData =true;
	// }

	

}

void CAchieveSystem::InitBabgeData()
{
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	int nCount = (int)provider.GetAchieveBabgeCount();

	PACHIEVEBABGE pBase =  (PACHIEVEBABGE)&m_BabgeState;
	int nSize = sizeof(m_BabgeState) / sizeof(*pBase);

	if(nCount > nSize) return;

	for(int i = 0;i<nCount;i++)
	{
		pBase->wBabgeID = i+1;
		pBase->nCount = 0;

		pBase++;
	}

	INT_PTR nAchieveCount = provider.GetAchieveCount(); //获取成就的数量

	nAchieveCount = __min(nAchieveCount,MAX_ACHIEVE_DATA_SIZE  <<2 );

	PACHIEVEDATA pAchieve;

	for(INT_PTR i=0; i< nAchieveCount; i++) //成就的个数
	{
		pAchieve= provider.GetAchieveData(i);

		if(pAchieve ==NULL) continue;
		if(pAchieve->bIsDelete) continue; //如果已经删除了就不处理了

		if(pAchieve->nBadgeId > 0 && IsAchieveFinished(i))		//完成一个徽章
		{
			SetAchieveBabgeState(pAchieve->nBadgeId);
		}

	}
}

void CAchieveSystem::SetAchieveBabgeState(WORD nId)
{
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig(); //成就的数据中心
	int nCount = (int)provider.GetAchieveBabgeCount();

	PACHIEVEBABGE pBase =  (PACHIEVEBABGE)&m_BabgeState;
	if ( !pBase )
	{
		return;
	}
	int nSize = sizeof(m_BabgeState) / sizeof(*pBase);

	if(nCount > nSize) return;

	for(int i = 0;i<nCount;i++)
	{
		if(pBase->wBabgeID == nId)
		{
			pBase->nCount++;
			break;
		}
		pBase++;
	}
}
bool CAchieveSystem::ExchangeBadge(INT_PTR nId)
{
	
	bool ret = false;
	/*CAchieveProvider& provider = GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	PACHIEVEBADGE badge = provider.GetBadge(nId);
	if (!badge)
		return ret;
	int nAchievePoint = m_pEntity->GetProperty<int>(PROP_ACTOR_ACIEVEPOINT);
	CActor* pActor = ((CActor*)m_pEntity);
	if (nAchievePoint < badge->nAchievePoint)
	{
		pActor->SendOldTipmsgWithId(tpAchievePointLimited, ttFlyTip);
		return ret;
	}
	else
	{
		CUserItemContainer::ItemOPParam iParam;
		iParam.wItemId = badge->wItemId;
		iParam.wCount = 1;
		if (pActor->GetBagSystem().CanAddItem(iParam))
		{
			LPCTSTR sLog = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpAchieveAwardItem);
			pActor->GetBagSystem().AddItem(iParam, sLog,GameLog::clExchangeAcheive);
			m_pEntity->SetProperty<int>(PROP_ACTOR_ACIEVEPOINT, nAchievePoint - badge->nAchievePoint);
			pActor->SendOldTipmsgWithId(tpExchangeAchieveBage, ttFlyTip);
			ret = true;
		}
		else
		{

		}
	}*/
	return ret;
}

//把成就设置为未完成
void CAchieveSystem::SetAchieveUnFinished( INT_PTR nAchieveId )
{

	// byte nBytePos = (BYTE)(nAchieveId >> 2); //第多少个BYTE
	// byte nBitPos = (BYTE)((nAchieveId &3) << 1);   //一个BYTE里的第几个Bit
	// byte nXorBit = (BYTE) ( (1 <<nBitPos) | (1 << (nBitPos+1)) ) ^ 255;
	// m_achieveFinishState[nBytePos] &= nXorBit;

	// CAchieveProvider & provider = GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	// PACHIEVEDATA pAchieve = provider.GetAchieveData(nAchieveId);
	// if ( !pAchieve )
	// {
	// 	return;
	// }

	// PACHIEVECONDITION pCond =  (PACHIEVECONDITION)pAchieve->conditions.pData; 
	// PACHIEVECONDITION pBase =  (PACHIEVECONDITION)&m_achieveEventData;
	// //PACHIEVECONDITION pEnd = (PACHIEVECONDITION) &( m_achieveEventData +sizeof(m_achieveEventData));
	// if ( !pCond || !pBase )
	// {
	// 	return;
	// }
	// INT_PTR nTotalIntCount= sizeof(m_achieveEventData) / sizeof(*pBase);

	// for(INT_PTR j=0; j < nTotalIntCount; j++ )
	// {
	// 	if( pBase->wEventID == pCond->wEventID )
	// 	{
	// 		pBase->nCount = 0;
	// 		break;
	// 	}
	// 	pBase ++;
	// }
	// InitAchieveEvent();

	// if( pAchieve->groupId > 0 && !IsAllGroupTitleAchieveFinished(pAchieve->groupId) )   //相关成就没有全部完成
	// {
	// 	DelGroupTitle(pAchieve->groupId);
	// }
}

bool CAchieveSystem::IsAllGroupTitleAchieveFinished(int groupId)
{
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	PACHIEVEGROUP pGroup = provider.GetAchiveGroup(groupId);

	if( pGroup )
	{
		if( pGroup->titleId == 0 )   //没有称号
		{
			return false;
		}

		for(INT_PTR i=0; i< GROUP_ARCHIEVE_MAX; i++)
		{
			int nAchieveId = pGroup->mTitleNeedAchieves[i];
			if( nAchieveId >= 0 )
			{
				if( !IsAchieveFinished( nAchieveId ) )     //有一个没有完成
				{
					return false;
				}
			}
			else        //扫描完毕了
			{
				break;
			}
		}
		return true;
	}

	return false;
}

/*刷新成就头衔
都完成了，没有则加头衔
没有都完成，有则删除头衔
*/
void CAchieveSystem::FreshAllGroupTitle()
{
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	INT_PTR nCount= provider.GetAchiveGroupCount(); //获取成就的分组的数目
	for(int  groupId = 1; groupId < nCount; groupId ++)   //0-
	{
		if( IsAllGroupTitleAchieveFinished(groupId) )   //相关成就全部完成
		{
			AddGroupTitle(groupId);
		}
		else
		{
			DelGroupTitle(groupId);
		}
	}
}

//增加成就组称号
void CAchieveSystem::AddGroupTitle(int groupId)
{
	// CActor *pActor = (CActor*)m_pEntity;
	// CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	// PACHIEVEGROUP pGroup = provider.GetAchiveGroup(groupId);
	// if( pActor && pGroup )
	// {
	// 	if( pGroup->titleId > 0 )
	// 	{
	// 		pActor->GetNewTitleSystem().addNewTitle(pGroup->titleId);
	// 	}
	// }
}

void CAchieveSystem::DelGroupTitle(int groupId)
{
	CActor *pActor = (CActor*)m_pEntity;
	CAchieveProvider & provider= GetLogicServer()->GetDataProvider()->GetAchieveConfig();
	PACHIEVEGROUP pGroup = provider.GetAchiveGroup(groupId);
	if( pActor && pGroup )
	{
		pActor->GetNewTitleSystem().delNewTitle(pGroup->titleId);
	}
}
///////////////////////////新成就///////////////////////////////////////////
INT_PTR CAchieveSystem::GetAchieveVar(CCLVariant* pVar)
{
	INT_PTR result = 0;
	if (pVar)
	{
		if (pVar->type()== CCLVariant::vStr && pVar->len() > 0)
		{
			sscanf((const char*)*pVar,"%lld",&result);
		}
		else if (pVar->type() == CCLVariant::vNumber)//兼容下旧的数据，新的都用字符串存
		{
			result = (INT_PTR)((double)*pVar);
		}
	}
	return result;
}
void CAchieveSystem::SetAchieveVar(CCLVariant& pVar, const char* sName, INT_PTR nValue)
{
	char sVar[64];
	sprintf(sVar, "%lld",nValue);
	pVar.set(sName, sVar);
}
bool CAchieveSystem::IsAchieveFinished(INT_PTR nAchieveId)
{
	if(nAchieveId < 1)
		return false;
	CActor* pActor		= (CActor*)m_pEntity;
	CCLVariant& pVarMgr	= pActor->GetActorVar();
	CCLVariant *pFinishedData	= pVarMgr.get(g_szIsAchieveFinishedData);
	INT_PTR nFinishedData = GetAchieveVar(pFinishedData);
	nAchieveId = nAchieveId -1;
	INT_PTR a=1;
	a<<=nAchieveId;
	nFinishedData&=a;
	if(nFinishedData == a)
	{
		return true;
	}
	return false;
}

bool CAchieveSystem::IsAchieveGiveAwards(INT_PTR nAchieveId)
{
	if(nAchieveId < 1)
		return false;
	CActor* pActor		= (CActor*)m_pEntity;
	CCLVariant& pVarMgr	= pActor->GetActorVar();
	CCLVariant *pAchieveGiveAwardsData	= pVarMgr.get(g_szIsAchieveGiveAwardsData);
	INT_PTR nAchieveGiveAwardsData = GetAchieveVar(pAchieveGiveAwardsData);
	nAchieveId = nAchieveId -1;
	INT_PTR a=1;
	a<<=nAchieveId;
	nAchieveGiveAwardsData&=a;
	if(nAchieveGiveAwardsData == a)
	{
		return true;
	}
	return false;
}

void CAchieveSystem::SetAchieveFinished(INT_PTR nAchieveId,bool boGm)
{
	if(nAchieveId < 1)
		return ;
		
	CActor* pActor		= (CActor*)m_pEntity;
	CCLVariant& pVarMgr	= pActor->GetActorVar();
	CCLVariant *pFinishedData	= pVarMgr.get(g_szIsAchieveFinishedData);
	INT_PTR nFinishedData = GetAchieveVar(pFinishedData);
	INT_PTR a=1;
	a<<=(nAchieveId -1);
	nFinishedData|=a;
	SetAchieveVar(pVarMgr, g_szIsAchieveFinishedData, nFinishedData);
	SendAchieveData();
	
}

void CAchieveSystem::SetAchieveGiveAwards(INT_PTR nAchieveId)
{
	if(nAchieveId < 1)
		return ;

	CActor* pActor		= (CActor*)m_pEntity;
	CCLVariant& pVarMgr	= pActor->GetActorVar();
	CCLVariant *pAchieveGiveAwardsData = pVarMgr.get(g_szIsAchieveGiveAwardsData);
	INT_PTR nAchieveGiveAwardsData = GetAchieveVar(pAchieveGiveAwardsData);
	
	INT_PTR a=1;
	a<<=(nAchieveId -1);
	nAchieveGiveAwardsData |= a;
	SetAchieveVar(pVarMgr, g_szIsAchieveGiveAwardsData, nAchieveGiveAwardsData);
	pActor->GetPropertySystem().ResertProperty();
	//((CActor *)m_pEntity)->SendOldTipmsgWithId(tpNewAchieve,ttFlyTip);
	SendAchieveData();
	//战士
	int nVocation = pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);	//职业
	enPropCreature ValueType = PROP_CREATURE_PHYSICAL_ATTACK_MAX;
	if(nVocation == enVocMagician)	//法师
	{
		ValueType = PROP_CREATURE_MAGIC_ATTACK_MAX;
	}
	if(nVocation == enVocWizard)	//道士
	{
		ValueType = PROP_CREATURE_WIZARD_ATTACK_MAX;
	}
	int nAttrValue = m_pEntity->GetProperty<unsigned int>(ValueType); 
	if( nAttrValue > ACHIEVE_ATTR_VALUE )			//此数值由策划定
	{
		//最大攻击达到某只完成一个成就任务
		// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtAttrMax, 0, nAttrValue, false);
	}
}

//下发成就数据
void CAchieveSystem::SendAchieveData()
{
	// CActor* pActor		= (CActor*)m_pEntity;
	// CCLVariant& pVarMgr	= pActor->GetActorVar();
	// CCLVariant *pFinishedData	= pVarMgr.get(g_szIsAchieveFinishedData);
	// CCLVariant *pAchieveGiveAwardsData	= pVarMgr.get(g_szIsAchieveGiveAwardsData);
	// INT_PTR nFinishedData = GetAchieveVar(pFinishedData);
	// INT_PTR nAchieveGiveAwardsData = GetAchieveVar(pAchieveGiveAwardsData);
	// BYTE nLen = 1;	
	// CActorPacket pack;
	// CDataPacket &data=m_pEntity->AllocPacket(pack); 
	// data << (BYTE)GetSystemID() << (BYTE)sSendNewAchieveData;
	// data << (BYTE)nLen;
	// data << (INT_PTR)nFinishedData << (INT_PTR)nAchieveGiveAwardsData;
	// pack.flush();
}

void CAchieveSystem::SendAchieveResult(WORD nId, BYTE bReslut)
{
	// CActorPacket pack;
	// CDataPacket &data=m_pEntity->AllocPacket(pack); 
	// data << (BYTE)GetSystemID() << (BYTE)sSendNewAchieveResult;
	// data << (WORD)nId;
	// data << (BYTE)bReslut;
	// pack.flush();
}

void CAchieveSystem::ActorAchievementComplete(int nAchieveType,int nValue, int nAchieveSubType, int nWay)
{
	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	
	int nKey = nAchieveType*1000 + nAchieveSubType;
	std::map<int, std::map<int, ACHIEVETYPECFG> >::iterator it = AchieveCfg.m_nAchieveTypes.find(nKey);
	if(it != AchieveCfg.m_nAchieveTypes.end())
	{
		std::map<int, ACHIEVETYPECFG>::iterator iter = it->second.find(nWay);
		if(iter != it->second.end())
		{
			ACHIEVETYPECFG& cfg = iter->second;
			if(cfg.nLog)
			{
				if(cfg.nValueType == 1) //累加
				{
					AddAchieveCompleteValue(nKey, nValue);
				}else if(cfg.nValueType == 2) //直接值
				{
					SetAchieveCompleteValue(nKey, nValue);
				}
				CheckTypeAchievementStatue(nAchieveType);
			}
			
			if(cfg.activityId.size() > 0)
			{
				for(int i = 0; i < cfg.activityId.size(); i++)
				{
					((CActor*)m_pEntity)->GetActivitySystem().UpdateActivityData(cfg.activityId[i], nAchieveType,nAchieveSubType, nValue);
				}
			}
		}
		
		
	}
}

void CAchieveSystem::CheckTypeAchievementStatue(int nAchieveType)
{
	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	nCanGetNum.clear();
	std::map<int, ACHIEVETASKCONFIG>::iterator it = AchieveCfg.m_AchieveTasks.begin();
	for(; it != AchieveCfg.m_AchieveTasks.end(); it++)
	{
		ACHIEVETASKCONFIG& info = it->second;
		bool bFlag = false;
		int nState = GetAchieveState(info.nId);

		if(nState == nAchieveComplete /*&& m_pEntity->CheckLevel(info.nLevel, info.nCircleLv) && GetLogicServer()->CheckOpenServer(info.nOpenday)*/)
			AddCanGet(info);
		
		if(nState > 0)
			continue;
		
		if(info.nParameter1 == nAchieveType)
		{	
			bFlag = CheckCompleteCondition(info.nConditionOne);
		}
		if(!bFlag && info.nParameter2 == nAchieveType)
		{
			bFlag = CheckCompleteCondition(info.nConditionTwo);
		}
		if(bFlag)
		{
			ChangeAchieveState(info, nAchieveComplete);
			if(nAchieveType == nAchieveCompleteAchieve)
				return;
			CheckTypeAchievementStatue(nAchieveCompleteAchieve);
		}
	}
}

void CAchieveSystem::SetAchieveCompleteValue(int nKey, int nValue)
{
	if(nValue <= 0)
		return;
	if(m_nAchieveInfo[nKey] < nValue)
	{
		m_nAchieveInfo[nKey] = nValue;
		isDataModify = true;
	}
	
}

void CAchieveSystem::AddAchieveCompleteValue(int nKey, int nValue)
{
	if(nValue <= 0)
		return;
	
	std::map<int, int>::iterator it = m_nAchieveInfo.find(nKey);
	if(it != m_nAchieveInfo.end())
	{
		m_nAchieveInfo[nKey] += nValue;
	}else
	{
		m_nAchieveInfo[nKey] = nValue;
	}
	isDataModify = true;
}

int CAchieveSystem::GetAchieveCompleteValue(int nKey)
{
	int nType = nKey / 1000;
	int nResult = 0;
	if(nType == nAchieveEquipment) {
		
		std::map<int, int>::iterator it = m_nAchieveInfo.begin();
		for(; it != m_nAchieveInfo.end(); it++)
		{
			int type = it->first/ 1000;
			if(nType == type && it->second > nResult && it->first >= nKey)
				nResult = it->second;
		}

	}else {
		std::map<int, int>::iterator it = m_nAchieveInfo.find(nKey);
		if(it != m_nAchieveInfo.end())
		{
			return m_nAchieveInfo[nKey];
		}
	}
	return nResult;
} 

int CAchieveSystem::GetAchieveCompleteValueByType(int nAchieveType, int nAchieveSubType)
{
	int nKey = nAchieveType*1000 + nAchieveSubType;
	std::map<int, int>::iterator it = m_nAchieveInfo.find(nKey);
	if(it != m_nAchieveInfo.end())
	{
		return m_nAchieveInfo[nKey];
	}
	
	return 0;
} 


VOID CAchieveSystem::SendAchieveInfoByTab(CDataPacketReader& pack)
{
	if(!m_pEntity) return;

	BYTE nTab = 0;
	pack >> nTab;
	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data << (BYTE)enAchieveSystemID << (BYTE)sSendAchieveData;
	data <<(BYTE) nTab;
	int nResult = 0;
	int nPos = data.getPosition();
	data << nResult;
	std::map<int, ACHIEVETASKCONFIG>::iterator it = AchieveCfg.m_AchieveTasks.begin();
	for(;it != AchieveCfg.m_AchieveTasks.end(); it++)
	{
		ACHIEVETASKCONFIG& info = it->second;
		if(info.nTab == nTab && m_pEntity->CheckLevel(info.nLevel, info.nCircleLv) && GetLogicServer()->CheckOpenServer(info.nOpenday) &&
		( info.nPreachievement == 0 || (info.nPreachievement != 0 && GetCompleteAchieveState(info.nPreachievement ) ) ))
		{

			data <<info.nId;
			data <<(int)(info.GetAchieveTaskConditionNum(1));
			data <<(int)(GetActorCompleteConditionValue(info.nConditionOne));
			data <<(int)(info.GetAchieveTaskConditionNum(2));
			data <<(int)(GetActorCompleteConditionValue(info.nConditionTwo));
			int state = GetAchieveState(info.nId);
			data<<(BYTE)(state);
			nResult++;
		}
	}
	int *Ptr = (int*)data.getPositionPtr(nPos);
	*Ptr = (int)nResult;
	ap.flush();
}




VOID CAchieveSystem::GetAchieveAwardByTasKId(CDataPacketReader& pack)
{
	if(!m_pEntity) return;

	int nTaskId = 0;
	pack >> nTaskId;
	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	std::map<int, ACHIEVETASKCONFIG>::iterator it = AchieveCfg.m_AchieveTasks.find(nTaskId);
	int nErrorCode = 1;
	if(it != AchieveCfg.m_AchieveTasks.end())
	{
		ACHIEVETASKCONFIG& info = it->second;
		if(info.nId == nTaskId)
		{
			bool nResult = false;
			if(GetCompleteAchieveState(nTaskId))
			{
				m_pEntity->SendTipmsgFormatWithId(tmAchieveAwardIsGet, tstUI);
				return;
			}
			nResult = CheckCompleteCondition(info.nConditionOne);
			if(!nResult)
				nResult = CheckCompleteCondition(info.nConditionTwo);
			
			if(!nResult)
			{
				m_pEntity->SendTipmsgFormatWithId(tmAchieveLimitCondition, tstUI);
				return;
			}
			if(info.nAwards.size() > 0)
			{
				for(int k = 0; k < info.nAwards.size(); k++)
				{
					ACHIEVEAWARDCONFIG& award = info.nAwards[k];
					m_pEntity->GiveAward(award.nType, award.nId, award.nCount,0,0,0,0,GameLog::Log_Achieve);
				}
			}
			nErrorCode = 0;
			ChangeAchieveState(info, nAchieveIsGet);
			SubCanGet(info);
		}
	}

	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data <<(BYTE)enAchieveSystemID << (BYTE)sGetAchieveAwards;
	data <<(int)nTaskId;
	data<<(BYTE)nErrorCode;
	ap.flush();
}

bool CAchieveSystem::CheckCompleteCondition(ACHIEVECONDITIONCONFIG& condition)
{
	int nReslut = GetActorCompleteConditionValue(condition);
	if(condition.nType > 0)
	{
		if(nReslut  >= condition.nLimitValue)
			return true;
	}else
	{
		int nNum = 0;
		int nSize = condition.nValue.size();
		if(nReslut >= nSize)
			return true;
	}
	return false;
}




int CAchieveSystem::GetActorCompleteConditionValue(ACHIEVECONDITIONCONFIG& condition)
{
	int nResult = 0;
	if(condition.nType > 0)
	{
		nResult =  GetAchieveCompleteValue(condition.nType);
	}else
	{
		int nSize = condition.nValue.size();
		for(int i = 0; i < nSize; i++)
		{
			int id = condition.nValue[i];
			if(id <= MAX_ACHIEVE_DATA_SIZE && id > 0)
			{
				if(GetAchieveState(id)  > 0 )
					nResult++;
			}	
		}
	}
	return nResult;
}

int CAchieveSystem::GetAchieveState(int nTaskid)
{
	int nIndex = nTaskid / 8;
	int nBit = nTaskid% 8 - 1;
	if(nBit < 0)
		nBit = 7;
	int nRet = 0;
	if(nIndex < MAX_ACHIEVE_DATA_SIZE)
	{
		BYTE nState = m_achieveFinishState[nIndex];//已经领取的
		nRet = (nState & (1 << nBit)) > 0? nAchieveIsGet : 0;

		if(!nRet)
		{
			nState = m_achieveEventData[nIndex];//完成可领取
			nRet = (nState & (1 << nBit)) > 0? nAchieveComplete : 0;
		}
	}
	return nRet;
}

int CAchieveSystem::GetCompleteAchieveState(int nTaskid)
{
	int nIndex = nTaskid / 8;
	int nBit = nTaskid% 8 - 1;
	if(nBit < 0)
		nBit = 7;
	int nRet = 0;
	if(nIndex < MAX_ACHIEVE_DATA_SIZE)
	{
		BYTE nState = m_achieveFinishState[nIndex];//已经领取的
		return nState & (1 << nBit);
	}
	return nRet;
}

void CAchieveSystem::ChangeAchieveState(ACHIEVETASKCONFIG& info, int nState)
{
	int nIndex = info.nId / 8;
	int nBit = info.nId % 8 -1;
	if(nBit < 0)
		nBit = 7;
	if(nIndex < MAX_ACHIEVE_DATA_SIZE)
	{
		if(nState == nAchieveComplete)
		{
			BYTE nState = m_achieveEventData[nIndex]; //完成可领取
			nState |= (1 << nBit);
			m_achieveEventData[nIndex] = (BYTE)nState;
		}
		else {
			BYTE nState = m_achieveFinishState[nIndex]; //已经领取的
			nState |= (1 << nBit);
			m_achieveFinishState[nIndex] = (BYTE)nState;

			nState = m_achieveEventData[nIndex]; //完成可领取
			nState -= (1 << nBit);
			m_achieveEventData[nIndex] = (BYTE)nState;
		}
	}
	if(nState != nAchieveComplete)
		return;
	// if(m_pEntity->CheckLevel(info.nLevel, info.nCircleLv) && GetLogicServer()->CheckOpenServer(info.nOpenday) )
	// {
	// 	AddCanGet(info);
		
	// }

	AddCanGet(info);
}
//保存玩家成就完成进度情况
void CAchieveSystem::SaveData()
{

	if(!m_pEntity) return;

    if(!isInitData) return;
    
    if(!isDataModify) return;

    CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveAchieveMent);
    dataPacket <<((CActor*)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();

    dataPacket <<(unsigned int)m_pEntity->GetId(); //玩家id
    size_t count = m_nAchieveInfo.size();
    dataPacket <<(int)count;
    std::map<int, int>::iterator it = m_nAchieveInfo.begin();

    for(; it != m_nAchieveInfo.end(); it++)
    {
        dataPacket <<(int)(it->first);
        dataPacket <<(int)(it->second);
    }
    GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);

    isDataModify = false;
}



void CAchieveSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet)
{
    if(!m_pEntity) return;

    if(nCmd == jxInterSrvComm::DbServerProto::dcLoadAchieveMent && nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回数据
    {
        int nCount = 0;
        packet >> nCount;
        m_nAchieveInfo.clear();
        for(int i = 0; i < nCount; i++)
        {
            int nType = 0; 
            int nValue = 0;
            packet >> nType >> nValue;
            m_nAchieveInfo[nType] = nValue;
        }
    }
    isInitData = true;
    ((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_ACHIEVE_SYSTEM); //完成一个步骤
} 
VOID CAchieveSystem::test()
{
	
} 
void CAchieveSystem::AddCanGet(ACHIEVETASKCONFIG& nInfo)
{
	nCanGetNum[nInfo.nTab].push_back(nInfo.nId);
	// if( (nInfo.nPreachievement == 0 || (nInfo.nPreachievement != 0 && GetAchieveState(nInfo.nPreachievement) == 2)) && 
	// 			m_pEntity->CheckLevel(nInfo.nLevel, nInfo.nCircleLv) && GetLogicServer()->CheckOpenServer(nInfo.nOpenday))
	// {
	// 	CActorPacket ap;
	// 	CDataPacket& data = m_pEntity->AllocPacket(ap);
	// 	data <<(BYTE)enAchieveSystemID << (BYTE)sPushAchieveRedPoint;
	// 	data <<(BYTE)1;
	// 	data <<(BYTE)nInfo.nTab;
	// 	ap.flush();
	// }
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator() )
	{
		return;
	}
	
	SendAchieveRedPoint(); 
}

void CAchieveSystem::SubCanGet(ACHIEVETASKCONFIG& nInfo)
{
	std::map<int, std::vector<int> >::iterator it = nCanGetNum.find(nInfo.nTab);
	if(it != nCanGetNum.end())
	{
		std::vector<int>::iterator iter = it->second.begin();
		for(; iter != it->second.end(); iter++)
		{
			if(nInfo.nId == *iter)
			{
				it->second.erase(iter);
				break;
			}
		}
		if(nCanGetNum[nInfo.nTab].size() == 0)
			nCanGetNum.erase(it);
	}
	
}

void CAchieveSystem::SendAchieveRedPoint()
{
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator())
	{
		return;
	}
	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data <<(BYTE)enAchieveSystemID << (BYTE)sPushAchieveRedPoint;
	int pos = data.getPosition();
	int nCount = 0;
	data <<(BYTE)(nCanGetNum.size());
	for(std::map<int, std::vector<int> >::iterator it = nCanGetNum.begin(); it != nCanGetNum.end(); it++)
	{
		for( std::vector<int>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
		{
			ACHIEVETASKCONFIG* cfg = AchieveCfg.GetAchieveTask(*iter);
			if(cfg)
			{
				if( (cfg->nPreachievement == 0 || (cfg->nPreachievement != 0 && GetAchieveState(cfg->nPreachievement) == 2)) && 
				m_pEntity->CheckLevel(cfg->nLevel, cfg->nCircleLv) && GetLogicServer()->CheckOpenServer(cfg->nOpenday))
				{
					data <<(BYTE)(it->first);
					nCount++;
					break;
				}
			}
		}
		
	}
	BYTE* pCount = (BYTE*)data.getPositionPtr(pos);
	*pCount = nCount;
	ap.flush();
}

//start of 勋章系统

bool CAchieveSystem::medalcheckgoods(const std::vector<GOODS_TAG>& goods_vec){
	bool flag = true ;
	std::vector<GOODS_TAG>::const_iterator it = goods_vec.begin();
	for( ; it != goods_vec.end(); it++){
		if(!(((CActor*)m_pEntity)->CheckConsume(it->type,it->id ,it->count))){
			flag = false ;
		}
	}
	return flag ;
}

bool CAchieveSystem::medalcheckachieve(const std::vector<int>& achievement_vec){
	bool flag = true ;
	std::vector<int>::const_iterator it = achievement_vec.begin();
	for( ; it != achievement_vec.end(); it++){
		if(!GetAchieveState(*it)){
			flag = false ;
		}
	}
	return flag ;
}



bool  CAchieveSystem::IsSatisfyMedalLevelUp(ELEM_MEDALCFG& condition,int Max_level ) //是否满足
{
	LONGLONG level = m_pEntity->GetStaticCountSystem().GetStaticCount(nMedalLevel) ;
	int nOpenDay = GetLogicServer()->GetDaysSinceOpenServer();

	std::vector<int> & need_achievement = condition.need_achieve_id_vec ;
	std::vector<GOODS_TAG>& need_goods = condition.need_goods_vec ;
	
	
	if( level >= Max_level){
		m_pEntity->SendTipmsgFormatWithId(tmMedalTouchTopLevel, tstUI);//满级失败
		return false;
	}else if ( !( ((CActor*)m_pEntity)->CheckLevel( condition.need_level , condition.need_circle) ) ){  			//等级，转身等级不满足
		m_pEntity->SendTipmsgFormatWithId(tmMedalLvUpFail, tstUI); //条件不满足
		return false ;
	}else if(!(nOpenDay>=condition.need_openday)){																	//开服天数不满足
		m_pEntity->SendTipmsgFormatWithId(tmMedalLvUpFail, tstUI); //条件不满足
		return false ;
	}else if(!(((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_Prestige) >=condition.need_popularity)){	//声望不满足
		m_pEntity->SendTipmsgFormatWithId(tmMedalLvUpFail, tstUI); //条件不满足
		return false ;
	}else if(!medalcheckachieve(need_achievement)){																	//前置成就不满足
		m_pEntity->SendTipmsgFormatWithId(tmMedalLvUpFail, tstUI); //条件不满足
		return false ;
	}else if( !medalcheckgoods(need_goods)){																		//物品不满足
		m_pEntity->SendTipmsgFormatWithId(tmMedalLvUpFail, tstUI); //条件不满足
		return false ;
	}

	return true ;
}


bool  CAchieveSystem::MedalLevelupConsumeGoods(ELEM_MEDALCFG& condition) //扣除
{
	
	//unsigned int old_num = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_Prestige) ;
	//((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_Prestige,old_num-condition.need_popularity) ; //扣除声望,用下面的统一接口

	const std::vector<GOODS_TAG>& goods_vec=condition.need_goods_vec ;
	std::vector<GOODS_TAG>::const_iterator it = goods_vec.begin();
	for( ; it != goods_vec.end(); it++){
		if(0 == it->type && 0 == it->id  )
		{
			if(it->id == condition.now_medal_id)
			 ((CActor*)m_pEntity)->GiveAward(it->type,condition.next_medal_id, 1, 0,0,0,0,
			 GameLog::Log_Medal,"激活勋章");//特殊处理当0级时候不需要扣除勋章，直接发下一级勋章
		}else if(0 == it->type && it->id == condition.now_medal_id) 
		{
			((CActor*)m_pEntity)->RemoveConsume(it->type,it->id ,it->count,-1,-1,-1,0,
			GameLog::Log_Medal,"替换勋章");
			((CActor*)m_pEntity)->GiveAward(it->type,condition.next_medal_id, it->count, 0,0,0,0,
			GameLog::Log_Medal,"替换勋章");
		}else 
		{
			((CActor*)m_pEntity)->RemoveConsume(it->type,it->id ,it->count,-1,-1,-1,0,
			GameLog::Log_Medal);
		}

		
	}
	

	return true ;
}



VOID CAchieveSystem::GetActorMedalLevel(CDataPacketReader& pack) //客户端请求等级
{
	if(!m_pEntity) return;
	int nErrorCode = 0;

	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data <<(BYTE)enAchieveSystemID << (BYTE)sSendMedalData;
	data <<(int)(((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(nMedalLevel)) ;

	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	LONGLONG level = m_pEntity->GetStaticCountSystem().GetStaticCount(nMedalLevel) ;
	if(AchieveCfg.m_MedalMap.find(level) !=AchieveCfg.m_MedalMap.end())
	{
		const std::vector<int>& achievement_vec = AchieveCfg.m_MedalMap[level].need_achieve_id_vec ;
		std::vector<int>::const_iterator it = achievement_vec.begin();
		data<<(BYTE)achievement_vec.size();

		for( ; it != achievement_vec.end(); it++){
			
			data<<(int)(*it);
			data<<(int)GetAchieveState(*it)	;	
		}	
	}else { data<<(WORD)0; }


	data<<(BYTE)nErrorCode;
	ap.flush();
}

VOID CAchieveSystem::MedalLevelUP(CDataPacketReader& pack)       //客户端请求升级
{
	int nErrorCode = 1;
	int Max_level_cfg= 0 ;

	CAchieve& AchieveCfg = GetLogicServer()->GetDataProvider()->GetNewAchieveConfig();
	LONGLONG level = m_pEntity->GetStaticCountSystem().GetStaticCount(nMedalLevel) ;
	for(int i = 0 ;AchieveCfg.m_MedalMap.find(i) !=AchieveCfg.m_MedalMap.end();i++ ){
		Max_level_cfg = i;
	}

	if(AchieveCfg.m_MedalMap.find(level) !=AchieveCfg.m_MedalMap.end())
	{
		if(IsSatisfyMedalLevelUp(AchieveCfg.m_MedalMap[level],Max_level_cfg)){
			MedalLevelupConsumeGoods(AchieveCfg.m_MedalMap[level]) ;
			(((CActor*)m_pEntity)->GetStaticCountSystem().SetStaticCount(nMedalLevel,level+1)) ;
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmMedalLvUpSuccess,tstUI);
			nErrorCode = 0 ;
		}
	}

	CActorPacket ap;
	CDataPacket& data = m_pEntity->AllocPacket(ap);
	data <<(BYTE)enAchieveSystemID << (BYTE)sUpMedalLevelResult;
	data <<(int)(((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(nMedalLevel)) ;
	data<<(BYTE)nErrorCode;
	ap.flush();
}
// end of 勋章系统
