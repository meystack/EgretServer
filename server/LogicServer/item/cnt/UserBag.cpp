#include "StdAfx.h"
#include "UserBag.h"
#include <bitset>

using namespace jxInterSrvComm::SessionServerProto;
const INT64 MinSeriesLmt = 1000000000000000000;
#define		ACCOUNTITEM			1	//账户绑定
#define		ACTORITEM			2		//角色绑定
#define		MAXITEMID			100000	//物品id超过这个数字，表示这是金钱


char CUserBag::s_ItemTipmsgBuff[] = {};
char CUserBag::s_ConsignItemLinkBuff[] ={};
char CUserBag::s_UserItemFormat[] = {};
char CUserBag::s_ItemMidStr1[] = {};
char CUserBag::s_ItemMidStr2[] = {};
int CUserBag::s_nEggYbCircleCount = 0;

template<>
const CUserBag::InheritedSybSystem::OnHandleSockPacket CUserBag::InheritedSybSystem::Handlers[]=
{
	&CUserBag::HandError,			 //错误处理 
	&CUserBag::HandAddItem,			 //添加道具			enBagSystemcDelItem
	&CUserBag::HandQueryItems,		 //获取背包列表		enBagSystemcQueryItem
	&CUserBag::HandDelItem,			 //删除物品			enBagSystemcDelItem
	&CUserBag::HandError,			 //错误处理  -- 
	&CUserBag::HandSlpitItem,		 //拆分				enBagSystemcBagSplit      5
	&CUserBag::HandMergeItem,		 //合并				enBagSystemcBagMerge
	&CUserBag::HandArrangeItemList,	 //整理背包			enBagSystemcArrangeBag
	&CUserBag::HandUseItem,			 //使用物品			enBagSystemcUseItem
	&CUserBag::HandleExChangeMoney,	 //兑换金币		encExChangeMoney
	&CUserBag::HandleRecoverItem,   //回收物品
	&CUserBag::HandBatchUseItem,	 //批量使用物品		enBagSystemcBatchUseItem

	&CUserBag::HandQueryItemsExtraInfo,	 //批量使用物品		enBagSystemcBatchUseItem
	&CUserBag::HandQueryItemsExtraInfoOne,	 //批量使用物
	// &CUserBag::HandGetBagEnlargeFee, //获取扩大背包费用 enBagSystemcGetBagEnlargeFee
	// &CUserBag::HandEnlargeBag,		 //扩大背包			enBagSystemcEnlargeBag
	// &CUserBag::HandMergeItemBindInfect,	 //合并道具（绑定感染） enBagSystemcUnBindToBind

	//&CUserBag::HandProcessItem,  //处理一件物品
	//&CUserBag::HandGetProcessItemConsume,  //获取处理一件装备需要的消耗
	//&CUserBag::HandGetItemProcessConfig,  //获取物品提升参数 10
	//&CUserBag::HandSourcePour, //灌注源泉
	//&CUserBag::HandActiveBagList,
	//&CUserBag::HandGetActiveBagItem,//丢弃物品
	//&CUserBag::HandDestroyCoin,  //丢弃金币
	//&CUserBag::HandGetUpgradeCfg,		//获取能升级的装备 15
	//&CUserBag::HandUseStrongItem,		//废弃
	//&CUserBag::HandSmashGoldEgg,		//废弃
	//&CUserBag::HandLuckAwardOpt,			//废弃
	//&CUserBag::HandGetPerfectCompTimes,		//废弃
	//&CUserBag::HandUseFiveAttrItem,			//废弃  20
	//&CUserBag::HandUseItemRecoverEx,		//废弃
	//
};

bool CUserBag::SetItemProperty(CUserItem * pUserItem,INT_PTR propID,INT_PTR nValue)
{
	if(propID <0 || propID >=Item::ipItemMaxProperty ) return false;
	if(pUserItem ==NULL) return false;
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL)
	{
		return false;
	}
	//是否是装备在身上
	int nItemPos = 0;
	int nHeroId = 0;
	//如果背包里找不到就在身上去找,判断强化的这个装备是身上的还是背包里的
	GetEquipItemPtr(pUserItem->series, nItemPos, nHeroId);

	if (nItemPos == 0)
	{
		OutputMsg(rmError,_T("无效的装备,装备增强失败，guid=%lld"),pUserItem->series.llId);
		return false;
	}

	if (nItemPos == 1)	//背包数据改变
	{
		OnDataModified(); //数据发生了改变,需要存盘
	}
	//PACKEDGAMEATTR attr;
	bool flag =false;
	bool bNeedRefreshAppear =false; //是否需要刷外观
	int nLogID =0; //记录日志的日志号
	int nOldValue = 0; //旧的数值
	int nParam =0;    //附加的参数
	int nAchieveEventID=0; //成就事件的ID
	int nAchieveParam =0;  //成就的参数

	switch(propID)
	{
	// case Item::ipItemDua:
	// 	nOldValue = pUserItem->wDura;
	// 	nLogID = GameLog::clEquipPropertyDurChange; //
	// 	//pUserItem->wDura =(WORD)nValue;
	// 	if (m_pEntity->GetType() == enActor)
	// 	{
	// 		if (pStdItem->m_btType == Item::itFunctionItem)
	// 		{
	// 			pUserItem->wDura =(WORD)nValue;
	// 			flag = true;
	// 		}
	// 		else
	// 		{
	// 			bool boForce = nValue > nOldValue;
	// 			((CActor *)m_pEntity)->GetEquipmentSystem().ChangeEquipDua(pUserItem, (int)(nValue - pUserItem->wDura), boForce);
	// 		}	
	// 	}
	// 	//CUserEquipment::SendItemDuaChange(pUserItem,(CActor*)m_pEntity);
	// 	break;

	// case Item::ipItemDuaMax:
	// 	nOldValue = pUserItem->wDuraMax;
	// 	nLogID = GameLog::clEquipPropertyMaxDurChange; //
	// 	pUserItem->wDuraMax =(WORD)nValue;
	// 	CUserEquipment::SendItemDuaChange(pUserItem,(CActor*)m_pEntity);
	// 	break;
		
	case Item::ipItemCount:
		nOldValue = pUserItem->wCount;
		nLogID = GameLog::clEquipPropertyCountChange; //
		pUserItem->wCount =(WORD)nValue;
		 flag =true;
		break;
		
	case Item::ipItemStrong:
		nOldValue = pUserItem->btStrong;
		nLogID = GameLog::clEQuipPropertyStrong; //
		pUserItem->btStrong =(BYTE)nValue;
		nAchieveParam = (int)nValue;

		if (pStdItem->isEquipment())
		{
			bNeedRefreshAppear =true; //需要刷外观
			// if (pStdItem->isBaseEquipment())	//只有基本装备能触发强化成就
			// {
			// 	nAchieveEventID = eAchieveEventEquipStrong; 
			// }
		}
		flag =true;
		break;
		
	case Item::ipItemQuality:
		nOldValue = pUserItem->btQuality;
		nLogID = GameLog::clEQuipPropertyQuality; //
		pUserItem->btQuality = (BYTE)nValue;
		//如果穿神器的话，需要刷新一下玩家的外观
		if(pUserItem->btQuality  == CStdItem::iqItemOrange)
		{
			bNeedRefreshAppear =true;
		}
		flag =true;
		break;
		
	case Item::ipItemBind:
		nOldValue = pUserItem->btFlag;
		nLogID = GameLog::clEquipPropertyBindChange; //

		pUserItem->btFlag = (BYTE)nValue;
		flag =true;
		break;	
	case Item::ipItemHole1IsOpen: // 这4个枚举要连续，否则出错
	case Item::ipItemHole2IsOpen:
	case Item::ipItemHole3IsOpen: 
	case Item::ipItemHole4IsOpen: 
		{
			/*
			INT_PTR nIndex(propID - Item::ipItemHole1IsOpen);
			nOldValue = (int)nIndex + 1;
			nLogID = GameLog::clEQuipPropertyHole; //
			pUserItem->wHoles[nIndex].open(nValue != 0);
			if(pStdItem->isEquipment())
			{
				nAchieveEventID = eAchieveEventEquipHole; 
				nAchieveParam = (int)((CActor *)m_pEntity)->GetEquipmentSystem().GetTotalHoleCount();
			}	
			flag =true;
			*/
		}
		break;
	case Item::ipItemHole1Item: // 这4个枚举要连续，否则出错
	case Item::ipItemHole2Item:
	case Item::ipItemHole3Item:
	case Item::ipItemHole4Item:
		{
			/*
			INT_PTR nIndex(propID - Item::ipItemHole1Item);
			nOldValue = (int)nIndex + 1;

			nLogID = GameLog::clEQuipPropertyInlay; //

			pUserItem->wHoles[nIndex].setInlayedItem(nValue);
			flag =true;
			bNeedRefreshAppear =true; //需要刷外观
			*/
		}
		break;

	case Item::ipItemRandForge: //随机锻造
		
		//nValue =GetGlobalLogicEngine()->GetRandValue() % CUserItem::MaxSmithAttrCount ;
		flag= RandAttrSelector::ItemSmithAttribute(pUserItem, (int)nValue);
		if(flag)
		{
			nOldValue = (int)nValue;
			nLogID = GameLog::clEQuipPropertyRandForge; 
		}
		break;
	case Item::ipItemForgeProtect:
		flag= RandAttrSelector::ItemSmithAttribute(pUserItem, (int)nValue, true);
		if(flag)
		{
			nOldValue = (int)nValue;
			nLogID = GameLog::clEQuipPropertyRandForge; 
		}
		break;
	/*
	case Item::ipItemForgeTimes: //精锻次数

		nOldValue =-1;
		nLogID = GameLog::clEQuipPropertyInject; //

		pUserItem->btSmithCount= (WORD)nValue;
		flag = true;
		break;
	*/
	case Item::ipItemForgeProperty1Value: //精锻的属性,这几个枚举要连续，否则出错
	case Item::ipItemForgeProperty2Value: //精锻的属性
	case Item::ipItemForgeProperty3Value: //精锻的属性
		{
			INT_PTR nIndex(propID - Item::ipItemForgeProperty1Value);
			nOldValue = (int)nIndex + 1;
			nLogID = GameLog::clEquipPropertyForge; //

			pUserItem->smithAttrs[nIndex].nValue = (int)nValue;
			flag = true;
		}
		break;
	case Item::ipItemInscriptExp:
		nOldValue = 1;
		nLogID = GameLog::clItemSpecSmith;
		pUserItem->nAreaId= (int)nValue;
		flag = true;
		break;
	case Item::ipItemLuck:
		nOldValue = pUserItem->btLuck;
		nLogID = GameLog::clEquipPropertyLuck;
		pUserItem->btLuck = (char)nValue;
		flag = true;
		break;
	case Item::ipItemSharp:
		nOldValue = pUserItem->btSharp;
		nLogID = GameLog::clEquipSharp;
		pUserItem->btSharp = (byte)nValue;
		flag = true;
		break;
	case Item::ipItemLostStar:					//强化损失星级
		nOldValue	= pUserItem->bLostStar;
		nLogID		= GameLog::clEQuipPropertyLostStar;
		pUserItem->bLostStar = (int)nValue;
		flag = true;
		break;
	case Item::ipItemInscriptLevel:		//铭刻等级
		nOldValue = pUserItem->bInSourceType;
		nLogID = GameLog::clEQuipPropertyInscriptLevel;
		pUserItem->bInSourceType = (int)nValue;
		flag = true;
		break;
	case Item::ipItemStar:
		nOldValue = pUserItem->wStar;
		nLogID = GameLog::clEquipStar;
		pUserItem->wStar = (int)nValue;
		flag = true;
		break;
	case Item::ipIdentifySlotNum:
		nOldValue = pUserItem->wIdentifySlotNum;
		nLogID = GameLog::clEquipIdentifySlotUnlock;
		pUserItem->wIdentifySlotNum = (WORD)nValue;
		flag = true;
		break;

	} 
	
	if (flag)	//更改成功
	{
		NotifyItemChange(pUserItem);
		if(nLogID)
		{
			//char sSeries[64]; 
			//sprintf(sSeries,"%lld",pUserItem->series.llId); //序列号
			if(GetLogicServer()->GetLocalClient() && pStdItem->m_Flags.recordLog)
			{
			}
		}
		if(nAchieveEventID)
		{
			((CActor*)m_pEntity)->GetAchieveSystem().OnEvent(nAchieveEventID,nAchieveParam); //跳跃一下
		}
		// BagQuestTarget(propID );			//触发任务强化相关的完成目标

		//是打了宝石
		// if(nAchieveEventID == eAchieveEventEquipEnlay) //检测宝石
		// {
		// 	((CActor*)m_pEntity)->GetEquipmentSystem().OnCheckDiamondLevel();
		// }
		// else if(nAchieveEventID == eAchieveEventEquipStrong) //强化
		// {
		// 	//此成就触发在 function EquipStrong( sysarg, args) 
		// 	//int nBrightStrongStarCount =  pUserItem->GetBrightStrongStarCount();
		// 	//((CActor*)m_pEntity)->GetAchieveSystem().OnEvent(aAchieveEventAllEquipStrongChange, nBrightStrongStarCount);
		// }

		//刷属性 //如果需要刷外观的话就刷外观
		if (nItemPos == 2)	//是身上装备，并且成功，需要重新刷属性
		{
			m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);
			if (bNeedRefreshAppear)	//刷外观
			{
				m_pEntity->CollectOperate(CEntityOPCollector::coRefFeature);
			}
		}
		if (nItemPos == 3 && nHeroId > 0)	//是英雄
		{
			//CHeroSystem& sys = ((CActor*)m_pEntity)->GetHeroSystem();
			//sys.SetHeroEquipChange();	//设置数据更改
			//sys.ResetProperty(sys.GetHeroPos(nHeroId), false, bNeedRefreshAppear);	//刷属性
		}
	}

	return flag;
}

/*
通知装备数据变化了
*/
void CUserBag::NotifyItemChange(CUserItem *pUserItem)
{
	CActorPacket pack;
	CDataPacket & data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)(enBagSystemID) << (BYTE)(enBagSystemsItemInfoChange);
	//data << * pUserItem;
	*pUserItem>>data;
	pack.flush();
}

VOID CUserBag::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;
 
	CActor *pActor = (CActor *)m_pEntity;
	int nSceneId = pActor->GetSceneID();
	int nFuBenId = pActor->GetFuBen()->GetFbId();

	int nOldSrvId = pActor->getOldSrvId();				//原9跨0
	int nRawSrvId = pActor->GetRawServerIndex();		//原9跨10009	
	int nServerId = GetLogicServer()->GetServerIndex();	//原9跨11

	//OutputMsg(rmError,"[WG][FubenSystem] ReqEnterFuben ERROR_USER 错误，副本ID=%d，玩家=%d name=%s!", nFbid, ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName()); 
	if(m_pEntity->GetHandle().GetType() != enActor) return; 
	if(nCmd >=1 && nCmd < enBagSystemcMaxBagCodeID)
	{
		(this->*Handlers[nCmd] )(packet);
	}
}

std::vector<int> DealDbBagCount(int bagCount)
{
	std::vector<int > t_counts;
	t_counts.clear();
	int wValue = LOWORD(bagCount);
	t_counts.push_back((int)HIWORD(bagCount));
	t_counts.push_back((int)HIBYTE(wValue));
	t_counts.push_back((int)LOBYTE(wValue));
}

bool CUserBag::Initialize(void *data,SIZE_T size)
{
	m_bGetActiveItemFlag = false;
	m_nUseXiuWeiCount = 0;

	ZeroMemory(m_aItemCDGroupExpireTimeTbl, sizeof(m_aItemCDGroupExpireTimeTbl));
	m_pExpItem =NULL;		//经验道具

	if ( !InheritedSybSystem::Initialize(data, size) )
	{
		OutputMsg(rmError,_T("bag InheritedSybSystem init fail"));
		return false;
	}
	m_pUserItemList.clear();
	// Inherited::clear();
	m_sellUserList.clear();
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("m_pEntity ==NULL"));
		return false;
	}
	
	if(m_pEntity->GetHandle().GetType() ==enActor)
	{
		PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
		if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
		{
			OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
			return false;
		}
		else
		{
			//默认的背包的数量
			SetBagNums( GetLogicServer()->GetDataProvider()->GetBagConfig().nMaxBagCount);
			//std::vector<int> counts = DealDbBagCount(pActorData->nBagGridCount);
			std::vector<int > counts;
			counts.clear();
			int wValue = LOWORD(pActorData->nBagGridCount);
			counts.push_back((int)HIWORD(pActorData->nBagGridCount));
			counts.push_back((int)HIBYTE(wValue));
			counts.push_back((int)LOBYTE(wValue));
			BAGCONFIG & dataConfig = GetLogicServer()->GetDataProvider()->GetBagConfig();
			for(int i = 0; i <m_nBagNums; i++)
			{
				unsigned int nDefaultBagGridCount = GetLogicServer()->GetDataProvider()->GetBagConfig().nDefaultBagCount[i];
				if(counts[i] > nDefaultBagGridCount)
				{
					nDefaultBagGridCount = counts[i];
				}
				PushBagMaxCount(nDefaultBagGridCount);

				m_BagCostCounts.push_back(0);
			}
			int nSlotCount = MAKELONG(MAKEWORD(m_BagMaxCounts[1],m_BagMaxCounts[2]), (WORD)m_BagMaxCounts[0]);	
			m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT, nSlotCount);
			setCapacity (nSlotCount); //设定格子的数量 --暂时用不到

		}
	}
	return true;
}



char * CUserBag::GetItemDetailLogStr(const CUserItem *pUserItem )
{
	if(pUserItem ==NULL) return "";
	//数目:品质:强化_打孔的数目:宝石1:宝石2:宝石3_精锻度
	/*
	int nHoleCount =0; //物品开启了多少个孔
	for(INT_PTR i=0;i < CStdItem::MaxItemHole;i ++)
	{
		if( pUserItem->wHoles[i].opened())
		{
			nHoleCount ++;
		}
	}
	*/
	
	sprintf(CUserBag::s_ItemTipmsgBuff,"%d:%d:%d:%d_%d:%d_%d_%d",
		(int)pUserItem->wCount,         //数目
		(int)pUserItem->wStar,         //强化
		(int)pUserItem->bLostStar,		//当前强化损失的星级
		(int)pUserItem->btFlag,          //绑定的标记
		pUserItem->bInSourceType,
		pUserItem->nAreaId,
		pUserItem->wIdentifySlotNum,
		pUserItem->nDropMonsterId
		);

	return CUserBag::s_ItemTipmsgBuff;

}

char * CUserBag::GetItemMidStr1(const CUserItem * pUserItem)
{
	if (NULL == pUserItem)
	{
		return "";
	}
	sprintf(CUserBag::s_ItemMidStr1, "%d_%d_%d",
		(int)pUserItem->btQuality,		
		(int)pUserItem->btLuck,		
		(int)pUserItem->smithAttrs[0].nValue		
		);
	return CUserBag::s_ItemMidStr1;
}

char * CUserBag::GetItemMidStr2(const CUserItem * pUserItem)
{
	if (NULL == pUserItem)
	{
		return "";
	}
	sprintf(CUserBag::s_ItemMidStr2, "%d_%d",
		(int)pUserItem->smithAttrs[1].nValue,		//极品属性4
		(int)pUserItem->smithAttrs[2].nValue		//极品属性5
		);
	return CUserBag::s_ItemMidStr2;
}

/*
将物品（装备）属性拼接成字符串，客户端按照此顺序读取字符串，形成物品的链接
*/
char* CUserBag::GetUseItemLinkFormat(const CUserItem* pUserItem)
{
	if (pUserItem)
	{
		sprintf(s_UserItemFormat,"%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
			pUserItem->wItemId,						//1.物品ID
			pUserItem->btQuality,					//2.品质
			pUserItem->btStrong,					//3.强化（不使用）
			//pUserItem->wDura,						//4.耐久
			//pUserItem->wDuraMax,					//5.最大耐久
			pUserItem->wCount,						//6.物品数量
			pUserItem->btFlag,						//7.标记1-绑定，2-不可交易
			pUserItem->btHandPos,					//8.左右手
			pUserItem->wStar,						//9.强化星级
			pUserItem->bLostStar,					//10.强化失败损失的星级（客户端显示时，最大强化星级通过配置减去该值）
			pUserItem->bInSourceType,				
			pUserItem->nAreaId,					
			pUserItem->nCreatetime.tv,				//13.创建时间
			pUserItem->smithAttrs[0].nValue,		//14.鉴定属性
			pUserItem->smithAttrs[1].nValue,
			pUserItem->smithAttrs[2].nValue,
			pUserItem->smithAttrs[3].nValue,
			pUserItem->smithAttrs[4].nValue,
			pUserItem->btLuck						//19.幸运值
			);
	}
	
	return s_UserItemFormat;
}
char * CUserBag::GetConsignItemLink(const CUserItem *pUserItem, unsigned int nPrice)
{
	//v1 普通物品 弹tip;
	//v6 寄卖物品 弹购买窗口
	if (!pUserItem)
	{
		return NULL;
	}
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL)
	{
		return NULL;
	}
	char *pName = (char *)pStdItem->m_sName;
	sprintf(CUserBag::s_ConsignItemLinkBuff,"<#c%s#%s/v6;%s;%d;%d;%d;%u>",
		GetQualityColorStr(pUserItem->btQuality),//1
		pName,//2
		GetUseItemLinkFormat(pUserItem),
		pUserItem->series.t.time,
		pUserItem->series.t.wSeries,
		pUserItem->series.t.btServer,
		nPrice
		);
	
	return CUserBag::s_ConsignItemLinkBuff;
}
char * CUserBag::GetItemLink(WORD wItemId, const CUserItem *pUserItem )
{
	
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(wItemId);
	if(pStdItem ==NULL)
	{
		return NULL;
	}
	
	char *pName = (char *)pStdItem->m_sName;
	 
	if( (pStdItem->isEquipment()) && pUserItem ) //如果是叠加的话
	{
		//sprintf(CUserBag::s_ItemTipmsgBuff,"<#b%s/v%d,%d,%lld>","name",wItemId,nActorID,guid);
		//"<%s/v%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d> 
		//<物品名字/v
		//物品id,物品的品质,物品的强化,物品的耐久,物品的耐久上限，物品数量，物品的过期时间，
		//物品第1个孔的信息，物品第2个孔的信息，物品第3个孔的信息，物品第4个孔的信息
		//物品第1个精锻的信息，物品第2个精锻的信息，物品第3个精锻的信息
		//物品的剩余精锻次数>
		sprintf(CUserBag::s_ItemTipmsgBuff,"<#c%s#%s/v1;%s>",
			GetQualityColorStr(pUserItem->btQuality),//1
			pName,//2
			GetUseItemLinkFormat(pUserItem)
			);
		//sprintf(CUserBag::s_ItemTipmsgBuff,"<#b%s/v%d,%d,%d>",pStdItem->m_sName,wItemId,);

	}
	else
	{
		if(pStdItem->b_showQuality ==0)
		{
			sprintf(CUserBag::s_ItemTipmsgBuff,"<%s/v1,%d>",pName,wItemId);
		}
		else
		{
			sprintf(CUserBag::s_ItemTipmsgBuff,"<#c%s#%s/v1,%d>",GetQualityColorStr(pStdItem->b_showQuality),pName,wItemId);
		}
		//sprintf(CUserBag::s_ItemTipmsgBuff,"<%s/v1,%d>",pName,wItemId);
		
	}
	return CUserBag::s_ItemTipmsgBuff;
}

 //强化一件物品
void CUserBag::HandProcessItem(CDataPacketReader &packet)
{
	/*
	CScriptValueList paramList;
	CUserItem::ItemSeries itemGuid;

	((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeItemProcess);

	BYTE nGuidCount=0;
	packet >> nGuidCount;
	paramList << nGuidCount;
	
	BYTE nEntityId=0;
	for(INT_PTR i=0;i < nGuidCount; i++)
	{
		packet >> nEntityId;		//装备所属ID  id=0：玩家/背包 id>0:英雄ID
		packet >> itemGuid.llId;
		int nItemPos=0, nHeroId=0;
		CUserItem* pUserItem = GetEquipItemPtr(itemGuid, nItemPos, nHeroId, nEntityId);

		if (pUserItem ==NULL)
		{
			((CActor*)m_pEntity)->SendOldTipmsgWithId(tpItemNotExist);
			return;
		}

		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem ==NULL)
		{
			((CActor*)m_pEntity)->SendOldTipmsgWithId(tpItemNotExist);
			return;
		}
		
		paramList << pUserItem;
	}
	
	

	BYTE  nItemProcessType =0,nParaCount =0; //物品操作码
	packet >> nItemProcessType >> nParaCount;
	if(nParaCount >=8 ) 
	{
		OutputMsg(rmError,_T("HandProcessItem参数过多=%d"),nParaCount);
		return;
	}

	//玩家开始强化装备, [物品的ID,物品的指针,祈福枝的数量]
	
	paramList << nItemProcessType ;

	int  nParam ;
	for(int i=0; i< nParaCount; i++)
	{
		packet >> nParam;
		paramList << nParam;
	}

	((CActor*)m_pEntity)->OnEvent(paramList,paramList);
	*/
};

//获取强化一件装备需要的消耗
void CUserBag::HandGetProcessItemConsume(CDataPacketReader &packet)
{
	/*
	CScriptValueList paramList;
	CUserItem::ItemSeries itemGuid;

	((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeGetItemProcessConsume);

	BYTE nGuidCount=0;
	packet >> nGuidCount;
	paramList << nGuidCount;

	BYTE nEntityId=0;
	for(INT_PTR i=0;i < nGuidCount; i++)
	{
		packet >> nEntityId;		//装备所属ID  id=0：玩家/背包 id>0:英雄ID
		packet >> itemGuid.llId;
		int nItemPos=0, nHeroId=0;
		CUserItem* pUserItem = GetEquipItemPtr(itemGuid, nItemPos, nHeroId, nEntityId);
		
		if (pUserItem ==NULL)
		{
			((CActor*)m_pEntity)->SendOldTipmsgWithId(tpItemNotExist);
			return;
		}
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem ==NULL)
		{
			((CActor*)m_pEntity)->SendOldTipmsgWithId(tpItemNotExist);
			return;
		}

		paramList << pUserItem;
	}



	BYTE  nItemProcessType =0,nParaCount =0; //物品操作码
	packet >> nItemProcessType >> nParaCount;
	if(nParaCount >=8 ) 
	{
		OutputMsg(rmError,_T("HandGetProcessItemConsume参数过多=%d"),nParaCount);
		return;
	}

	//玩家开始强化装备, [物品的ID,物品的指针,祈福枝的数量]

	paramList << nItemProcessType ;

	int nParam ;
	for(int i=0; i< nParaCount; i++)
	{
		packet >> nParam;
		paramList << nParam;
	}
	
	((CActor*)m_pEntity)->OnEvent(paramList,paramList);
	*/
};

 //获取祈福枝的提升概率列表
void CUserBag::HandGetItemProcessConfig(CDataPacketReader &packet)
{
	/*
	CScriptValueList paramList;
	BYTE btType=0;

	packet >> btType;
	((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeGetItemProcessConfig);
	paramList << btType;
	((CActor*)m_pEntity)->OnEvent(paramList,paramList);
	*/
};


//使用物品的处理函数
bool CUserBag::OnUseItem(CUserItem *pUserItem,const CStdItem * pStdItem, int nCount, bool isHeroUse, int nParam,bool bNeedRes)
{
	int nType = pStdItem->m_btType;
	bool needDelete  = false;
	bool isSucceed = true; //是否成功
	bool isDeath =m_pEntity->IsDeath(); //是活的
	int  nValue;
	int nCDGroupId	= pStdItem->m_btColGroup;
	int nCDTime		= pStdItem->m_nCDTime;
	bool isEquip = pStdItem->isEquipment();
	int nUseCount = 1;
	
	if (!isEquip && CheckUseItemCD(nCDGroupId))
	{
		CActorPacket pack ;
		CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
		data <<  (BYTE) (enBagSystemID) << (BYTE) (enBagSystemsUseItemResult);
		data <<  (WORD)(pStdItem->m_nIndex);
		data << (BYTE)0;
		pack.flush();
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemInCd, tstUI);
		return false;
	}
	bool bNeedAddBuff =true;
	if( nType == Item::itSlowHpdMed || nType == Item::itSlowBluedMed)		//普通药品
	{
		
		CAnimal * pEeater = m_pEntity;
		if(isHeroUse) //是给英雄使用的
		{
			
			isDeath = pEeater->IsDeath();
		}
		//4个一组的
		if(isDeath || pEeater ==NULL)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemCanNotUseNow,tstUI);
			return false;
		}
		else if(pStdItem->m_StaticAttrs.nCount > 0)
		{	
			needDelete = true;	
			for(INT_PTR i=0; i< pStdItem->m_StaticAttrs.nCount; i++)
			{
				//添加Buff
				PGAMEATTR pattr = &pStdItem->m_StaticAttrs.pAttrs[i];
				int nBuffId = pattr->value.nValue;				
				CBuffProvider::BUFFCONFIG * pBuff = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
				if (pBuff == NULL)
				{
					continue;
				}
				if(pBuff->nType == aBuffExpAdd || pBuff->nType == aHpMpAdd ||  pBuff->nType == aHpAdd || pBuff->nType == aMpAdd)
				{	
					CDynamicBuff newBuff;
					int nBuffAddCount = nCount;
					CDynamicBuff * pExpAddBuff = ((CActor *)m_pEntity)->GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBuff->nType, pBuff->nGroup);
					if (pExpAddBuff == NULL)//批量使用时，不存在buff，先添加1个
					{
						((CActor *)m_pEntity)->GetBuffSystem()->Append(nBuffId);
						pExpAddBuff = ((CActor *)m_pEntity)->GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBuff->nType, pBuff->nGroup);
						nBuffAddCount--;
					}
					if (pExpAddBuff != NULL && nBuffAddCount > 0)
					{
						newBuff.wBuffId = pExpAddBuff->wBuffId;
						newBuff.dwTwice = pExpAddBuff->dwTwice;
						if ((double)pExpAddBuff->value.uValue + pBuff->dValue * nBuffAddCount > 0xFFFFFFFF)
						{
							newBuff.value.uValue = 0xFFFFFFFF;
						}
						else
						{
							newBuff.value.uValue = pExpAddBuff->value.uValue + (unsigned int)pBuff->dValue * nBuffAddCount;
						}
						newBuff.wActWait = pExpAddBuff->wActWait;
						newBuff.pConfig = pExpAddBuff->pConfig;
						((CActor *)m_pEntity)->GetBuffSystem()->Append(nBuffId, &newBuff);
					}
				}	
			}
		}
	}
	else if(nType == Item::itAddBuff ) //挂机卷轴加多倍经验buff
	{
		if(pStdItem->m_StaticAttrs.nCount > 0)
		{	
			for(INT_PTR i=0; i< pStdItem->m_StaticAttrs.nCount; i++)
			{
				//添加Buff
				PGAMEATTR pattr = &pStdItem->m_StaticAttrs.pAttrs[i];
				int nBuffId = pattr->value.nValue;	
				((CActor *)m_pEntity)->GetBuffSystem()->Append(nBuffId);
			}
			if(pStdItem->nTips)
				((CActor *)m_pEntity)->SendTipmsgFormatWithId(pStdItem->nTips,tstKillDrop,((CActor *)m_pEntity)->GetEntityName(), pStdItem->m_sName);
			needDelete = true;
		}
	}
	else if(nType == Item::itRecover ) //
	{
		int nState = m_pEntity->GetProperty<int>(PROP_ACTOR_RECOVERSTATE);
		if(nState == 1)
		{
			((CActor*)m_pEntity)->GiveDropAward(pStdItem->m_ndropGroupid, GameLog::Log_UseVipCard);
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmUseRecoverItem,tstUIWindows);

		}else
		{
			m_pEntity->SetProperty<int>(PROP_ACTOR_RECOVERSTATE, 1);
		}
		needDelete = true;
		
	}
	else if(nType == Item::itGuaJiJuanZhou ) //挂机卷轴加多倍经验buff
	{
		if( ((CActor *)m_pEntity)->GetBuffSystem()->Exists(aExpPower)) {
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmGuaJiJuanZhouOnUse,tstUI);
			return false ;
		} 

		if(pStdItem->m_StaticAttrs.nCount > 0)
		{	
			for(INT_PTR i=0; i< pStdItem->m_StaticAttrs.nCount; i++)
			{
				//添加Buff
				PGAMEATTR pattr = &pStdItem->m_StaticAttrs.pAttrs[i];
				int nBuffId = pattr->value.nValue;	
				((CActor *)m_pEntity)->GetBuffSystem()->Append(nBuffId);
				((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmGuaJiJuanZhouUseSucc,tstUI);	
			}
			needDelete = true;
		}
	}
	else if(nType == Item::itFastMedicament ) //速回药品
	{
		CAnimal * pEeater = m_pEntity;
		if(isHeroUse)
		{		
			isDeath = pEeater->IsDeath();
		}

		if(isDeath || pEeater ==NULL)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemCanNotUseNow,tstUI);
		}
		else if(pStdItem->m_StaticAttrs.nCount )
		{
			needDelete = true;
			float fFastMedicamentRenew = pEeater->GetFastMedicamentRenew();
			for(INT_PTR i=0; i< pStdItem->m_StaticAttrs.nCount; i++)
			{
				PGAMEATTR pattr = &pStdItem->m_StaticAttrs.pAttrs[i];
				switch(pattr->type)
				{
					//血增加
				case aHpAdd:
					{
						float fRate = fFastMedicamentRenew;
						pEeater->ChangeHP((int)(pattr->value.nValue * (1 + fRate)));
						break;
					}
				case aHpPower:   //血倍率增加
					pEeater->ChangeHP((int)((pattr->value.fValue+fFastMedicamentRenew) * m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) ));					
					break;
					//魔增加
				case aMpAdd:
					{
						float fRate = fFastMedicamentRenew;
						pEeater->ChangeMP((int)(pattr->value.nValue * (1+fRate)));
						break;
					}
				case aMpPower:  //魔倍率增加
					pEeater->ChangeMP((int)((pattr->value.fValue+fFastMedicamentRenew) * m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP) ));					
					break;
				case aExpAdd: //经验改变
					((CActor *)m_pEntity)->AddExp(pattr->value.nValue, GameLog::clItemAddExp, pStdItem->m_nIndex); //
					break;
					break;
				}
			}
		}
	}
	else if(nType == Item::itRandomMove)
	{
		CScene * pScene = m_pEntity->GetScene();
		if (!pScene)
		{
			return false;
		}

		CSkillResult::RandTransfer(m_pEntity->GetFuBen(), pScene, m_pEntity);
		needDelete = true;
	}
	else if(nType == Item::itHair)
	{
		m_pEntity->SetProperty<int>(PROP_ENTITY_ICON,pStdItem->m_wShape);
		needDelete = true;
	}
	else if(nType == Item::itDoubleExp)
	{
		Uint64 nNowexp =m_pEntity->GetProperty<Uint64>(PROP_ACTOR_MULTI_EXP_UNUSED);
		// int nMaxexp = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMaxDoubleExpValue;
		// if(nNowexp >= nMaxexp)
		// {
		// 	((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmdoubleExpFull, tstUI);
		// 	return false;
		// }
		if(((CActor*)m_pEntity)->checkGetMaxDouExp())
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmdoubleExpFull, tstUI);
			return false;
		}
		int lv =m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
		unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE), 1);	
		if(lv >= nMaxPlayerLevel)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmCircleToMaxLv, tstUI);
			return false;
		}
		int64_t nAddValue = 0;
		for(INT_PTR i=0; i< pStdItem->m_StaticAttrs.nCount; i++)
		{
			PGAMEATTR pattr = &pStdItem->m_StaticAttrs.pAttrs[i];
			nAddValue += pattr->value.nValue;
		}
		m_pEntity->SetProperty<Uint64>(PROP_ACTOR_MULTI_EXP_UNUSED, (Uint64)(nNowexp+nAddValue));
		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmUseItemAddExpGeneral, tstUI, nAddValue);
		needDelete = true;
	}
	else if(nType == Item::itFunctionItem || nType ==  Item::itDailyUse || nType == Item::itAutoUse)  //功能物品执行脚本(使用此类道具需要触发脚本)
	{
		int nDelType = 0;
		CScriptValueList paramList, retList;
		paramList << m_pEntity;
		paramList << nCount;
		paramList << (int)pUserItem->wItemId;
		paramList << pUserItem;

		paramList << nDelType;	//扣除物品类型 如果为0则在脚本删除物品 1则扣除耐久(次数)
		paramList << (int)nParam;
		
		((CActor*)m_pEntity)->OnItemEvent(paramList, retList);
		if (retList.count() > 1)  //通过脚本返回true或false
		{
			isSucceed = (bool)retList[1];
			needDelete = false;
		}
		if (retList.count() > 2) 
		{
			nUseCount = (int)retList[2];
		}
	}
	else if (nType == Item::itGift || nType == Item::itGold)
	{
		if(!bagIsEnough(BagGitf))
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmGiftNoBagNum, tstUI);
			return false;
		}
		//神装BOSS挑战卷 单独处理
		if(437 == pStdItem->m_nIndex)
		{
			if (((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(nSZBOSS_STATICTYPE) >= 100 )
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemLimitSZBossTime, tstUI);
				return false;
			}
			((CActor*)m_pEntity)->GetBossSystem().SendSZBossTimes();
		}
		bool bSend2Client = false;
		if(1051 == pStdItem->m_nIndex)
			bSend2Client = true;
		bool isSucceed = ((CActor*)m_pEntity)->GiveDropAward(pStdItem->m_ndropGroupid, GameLog::Log_UseGift,bSend2Client);
		if(isSucceed == true)
			needDelete= true;
	}
	else if (nType == Item::itBox)
	{
		if(!bagIsEnough(BagGitf))
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmGiftNoBagNum, tstUI);
			return false;
		}


		if (pStdItem->m_SelectItems.size() > 0)
		{
			for(int i = 0; i < pStdItem->m_SelectItems.size(); i++)
			{
				const SelectItemInfo& info = pStdItem->m_SelectItems[i];
				if(info.nIndex == nParam)
				{
					// for(int k = 0; k < info.items.size(); k++)
					// {
					// 	const ItemDeathSplit& item = info.items[k];
					// 	((CActor*)m_pEntity)->GiveAward(item.nType, item.nItemId, item.nCount);
					// }
					bool isSucceed = ((CActor*)m_pEntity)->GiveDropAward(info.m_ndropGroupid,GameLog::Log_UseBox);
					if(isSucceed == true)
						needDelete= true;
				}
			}
		}
	}
	else if (nType == Item::itSelectOneBox)
	{
		//次元首领	开启归属奖、勇斗奖、参与奖宝箱
		if(1085 == pUserItem->wItemId || 1086 == pUserItem->wItemId || 1087 == pUserItem->wItemId)
		{
			//次元钥匙不足提示
			if ( ((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY) <= 0)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmKuaFuDimensionalKeyNotEnough, tstUI);
				return false;
			}

			for(int i = 0; i < pStdItem->m_SelectItems.size(); i++)
			{
				const SelectItemInfo& info = pStdItem->m_SelectItems[i];
				for (size_t j = 0; j < info.items.size(); j++)
				{
					const ItemDeathSplit& item = info.items[j];
					if (item.nItemId == pUserItem->wItemId)
					{
						((CActor*)m_pEntity)->GetStaticCountSystem().OnAddDimensionalKey(-1 * item.nCount); 
        				((CActor*)m_pEntity)->SetProperty<unsigned int>(PROP_ACTOR_DIMENSIONALKEY, ((CActor*)m_pEntity)->GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_DIMENSIONAL_KEY));
						bool isSucceed = ((CActor*)m_pEntity)->GiveDropAward(info.m_ndropGroupid, GameLog::Log_UseBox);
						if(isSucceed == true)
							needDelete= true;
						break;
					}
				}
			}
		}
	}
	else if(nType == Item::itYuanBaoCharge)
	{
		bool isSucceed = ((CActor*)m_pEntity)->GiveDropAward(pStdItem->m_ndropGroupid,GameLog::Log_RechargeCard);
		if(isSucceed == true)
			needDelete= true;

	}
	else if(nType == Item::itRebateYuanBaoCharge) // 使用返利卡
	{
		((CActor*)m_pEntity)->SetRebateCharge(true);
		bool isSucceed = ((CActor*)m_pEntity)->GiveDropAward(pStdItem->m_ndropGroupid,GameLog::Log_RechargeCard);
		if(isSucceed == true)
			needDelete= true;

	}
	// else if (nType == Item::itSkillBook)
	// {
	//     if(pStdItem->m_nSillId > 0)
	// 	{
	// 	    INT_PTR  nSkillLevel = ((CActor *)m_pEntity)->GetSkillSystem().GetSkillLevel(pStdItem->m_nSillId);
	// 		if(nSkillLevel > 0)
	// 		{
	// 		    ((CActor *)m_pEntity)->SendOldTipmsgWithId(tpHasLearnSkill,ttTipmsgWindow + ttFlyTip);
	// 			return;
	// 		}
	// 	}
	// 	CActorPacket AP;
	// 	CDataPacket& DataPacket = ((CActor*)m_pEntity)->AllocPacket(AP);
	// 	DataPacket << (BYTE)enScriptMiscSystem << (BYTE)enScriptMiscSystemsUseSkillBookResult;
	// 	DataPacket << (BYTE)1;
	// 	DataPacket << (int)pUserItem->wItemId;
	// 	AP.flush();
	// }

	if(needDelete)
	{
		// if( nType == Item::itSlowHpdMed || nType == Item::itSlowBluedMed || nType == Item::itFastMedicament)	//药品 
		// {
		// 	if(bNeedAddBuff)				//没加buf就不扣道具
		// 	{
		// 		DeleteItem(pUserItem, nCount,"use item",GameLog::clUserItem);
		// 	}
		// 	else
		// 	{
		// 		isSucceed = false;
		// 	}
		// }
		// else
		{
			DeleteItem(pUserItem,nCount,"use item",GameLog::Log_UserItem);
			((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveCostItem, nCount,  pUserItem->wItemId);
		}
	}
	if(bNeedRes)
	{
		CActorPacket pack ;
		CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
		data <<  (BYTE) (enBagSystemID) << (BYTE) (enBagSystemsUseItemResult);
		data <<  (WORD)(pStdItem->m_nIndex);
		data << (BYTE) (isSucceed ? 1:0);
		pack.flush();
	}
	
	if (isSucceed && !isEquip )
	{
		ItemUseEnterCD(nCDGroupId, nCDTime);
	}
	if ( isSucceed )
	{
		// ((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtUseItem, pStdItem->m_nIndex, nUseCount);
		// ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemUseSuccess, tstUI, pStdItem->m_sName);
	}
	return isSucceed;
}

bool CUserBag::CheckUseItemCD(int nGroupId)
{
	if (nGroupId < 0 || nGroupId >= s_nMaxItemCDGroupCount)
	{
		OutputMsg(rmError, _T("%s invalid item cd group[%d]"), __FUNCTION__, nGroupId);
		return true;
	}

	if (GetGlobalLogicEngine()->getTickCount() < m_aItemCDGroupExpireTimeTbl[nGroupId])
		return true;

	return false;
}

void CUserBag::ItemUseEnterCD(int nGroupId, int nCDTime)
{
	if (nGroupId < 0 || nGroupId >= s_nMaxItemCDGroupCount)
	{
		OutputMsg(rmError, _T("%s invalid item cd group[%d]"), __FUNCTION__, nGroupId);
		return;
	}

	m_aItemCDGroupExpireTimeTbl[nGroupId] = GetGlobalLogicEngine()->getTickCount() + nCDTime;
}

void CUserBag::HandUseItem(CDataPacketReader &packet)
{
	//判断区域内能不能用这个物品
	if (!m_pEntity)
		return;
	CUserItem::ItemSeries itemGuid;
	BYTE bIsHeroUse = 0; //是否是给英雄用的
	int nIndex = 0;
 	packet >> itemGuid.llId >> bIsHeroUse >> nIndex;
	CUserItem* pUserItem = Inherited::FindItemByGuid(itemGuid);
	if( pUserItem == NULL) 
		return ; //找不到这个物品

	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem == NULL) 
		return;

	CScene* pScene = m_pEntity->GetScene();
	int x, y;
	m_pEntity->GetPosition(x, y);
	
	//((CActor *)m_pEntity)->m_nFcmTime = 10;
	if (pScene && pScene->HasMapAttribute(x, y, aaNotItemId, pStdItem->m_nIndex))
	{
		return;
		//((CActor *)m_pEntity)->SendOldTipmsgWithId(tpItemRegionCanNotUseItem,ttFlyTip + ttTipmsgWindow);
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemRegionCanNotUseItem, tstUI);
	}
	if (m_pEntity->HasState(esStateStall))
	{
		((CActor *)m_pEntity)->SendOldTipmsgWithId(tpStallNoUseItem, ttFlyTip);
		return;
	}
	
	int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
	if(pStdItem->m_nOpenDaylimit != 0 
		&& pStdItem->m_nOpenDaylimit >= nOpenServerDay)
	{ 
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmUseItemOpenDaylimit, tstUI);
		return;
	}
	//特定使用次数 年月周的限制
	if(pStdItem->m_CanUseType > eITEM_USE_LIMIT_NULL 
		&& pStdItem->m_CanUseType < eITEM_USE_LIMIT_MAX) //使用次数限制
	{
		LONGLONG nStaticNum = 0;
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_DAY)
		{   
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDailyInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountDaily(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDaily(pStdItem->m_nIndex);
			} 
		}
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_WEEK)
		{ 
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeekInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountWeek(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeek(pStdItem->m_nIndex);
			} 
		}
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_MONTH)
		{  
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonthInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountMonth(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonth(pStdItem->m_nIndex);
			} 
		}
		if(nStaticNum <= 0)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmUseItemUselimit, tstUI);//提示新增
			return; 
		}
	}

	if (pStdItem->isEquipment()) //如果是装备的话
	{
		((CActor*)m_pEntity)->GetEquipmentSystem().TakeOn(itemGuid); //穿上装备
	}
	else
	{
		INT_PTR nTipmsgID = CUserEquipment::CheckTakeOnCondition(m_pEntity, pStdItem, pUserItem);
		if (nTipmsgID)
		{	
			if(m_pEntity->GetHandle().GetType() == enActor)
			{
				// 通知客户端使用物品失败
				CActorPacket pack ;
				CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
				data << (BYTE) (enBagSystemID) << (BYTE) (enBagSystemsUseItemResult);
				data << (WORD)(pStdItem->m_nIndex);
				data << (BYTE) 0;
				pack.flush();

				((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemCanNotUse, tstUI);
			}
			
			return; //不能使用
		}
		bool result = OnUseItem(pUserItem, pStdItem, 1, bIsHeroUse ? true : false, nIndex); 
		if (result)
		{ 
			if(pStdItem->m_CanUseType > eITEM_USE_LIMIT_NULL 
				&& pStdItem->m_CanUseType < eITEM_USE_LIMIT_MAX) //使用次数限制
			{
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_DAY)
				{   
					((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountDaily(pStdItem->m_nIndex, 1);
				}
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_WEEK)
				{
					((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountWeek(pStdItem->m_nIndex, 1);
				}
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_MONTH)
				{  
					((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountMonth(pStdItem->m_nIndex, 1);
				}
			}
		}
	}
}

void CUserBag::HandDelItem(CDataPacketReader &packet)
{
	CUserItem::ItemSeries itemGuid;
	packet >> itemGuid.llId;
	CUserItem *pUserItem = Inherited::FindItemByGuid(itemGuid);
	if(pUserItem ==NULL)
	{
		((CActor *)m_pEntity)->SendOldTipmsgWithId(tpItemNotExist, ttTipmsgWindow);
		return;
	}

	if ( GetLogicServer()->IsCommonServer() )
	{
		if ( !pUserItem->binded() )
		{
			((CActor*)m_pEntity)->SendOldTipmsgWithId(tpNotDropItemCommon, ttTipmsgWindow);
			return;
		}
	}
	
	
	int nPosX, nPosY;
	m_pEntity->GetPosition(nPosX, nPosY);

	if (m_pEntity->HasState(esStateStall))
	{
		return;
	}

	const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(NULL == pStdItem)
		return;

	bool canDelete = true; 
	WORD wItemId(pUserItem->wItemId);
	if( pStdItem->m_btType == Item::itQuestItem  )
	{
	}
	else
	{
		if(pStdItem->m_Flags.denyDestroy)
		{	
			canDelete = false;
		}
		
		if(strlen(pUserItem->cBestAttr) != 0)
		{
			canDelete = false;
		}
	}
	
	if(!canDelete) //如果不能删除的话
	{
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemCanNotDelete, tstUI);
		return;
	}

	if (((CActor *)m_pEntity)->GetDealSystem().FindDealItem(pUserItem))
	{
	    ((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmFailForDealing, tstUI);
		return;
	}
 
	WORD nCount= pUserItem->wCount;
	 
	DeleteItem(pUserItem,pUserItem->wCount,"self_del_item",GameLog::Log_SelfDelItem, true);
}
 
//丢弃金币
void CUserBag::HandDestroyCoin(CDataPacketReader &packet)
{

}

void CUserBag::HandQueryItems(CDataPacketReader &packet)
{ 
	//TestAddItem(266, 1);
	CActorPacket pack;
	INT_PTR nCount = Inherited::count();
	 
	CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
	data <<(BYTE)enBagSystemID << (BYTE)enBagSystemsInitBagItem;

	data <<(int)GetBagCount(0);
	data <<(int)GetBagCount(1);
	data <<(int)GetBagCount(2);
	data <<  (WORD) nCount;
	
	// CUserItem ** pItem =Inherited::operator CUserItem**();
	for(INT_PTR i=0; i< nCount; i++)
	{
		if(m_pUserItemList[i])
		{
			(CUserItem)*m_pUserItemList[i] >> data;
		}
	}
	pack.flush();
	// GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmNoMoreCoin, tstFigthing);
}
void CUserBag::HandQueryItemsExtraInfo(CDataPacketReader &packet)
{ 
	//TestAddItem(266, 1);
	CActorPacket pack;
	INT_PTR nCount = Inherited::count();
	 
	CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
	data <<(BYTE)enBagSystemID << (BYTE)sBagSystemQueryItemsExtraInfo;

	// data <<(int)GetBagCount(0);
	// data <<(int)GetBagCount(1);
	// data <<(int)GetBagCount(2);
	int ccount = 0;
	INT_PTR nOffer = data.getPosition(); 
	data <<  ccount;
	
	// CUserItem ** pItem =Inherited::operator CUserItem**();
	for(INT_PTR i = 0; i< nCount; i++)
	{
		if(m_pUserItemList[i])
		{ 
			const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(m_pUserItemList[i]->wItemId);
			if(pStdItem == NULL) 
				continue;
			
			//特定使用次数 年月周的限制
			if(pStdItem->m_CanUseType > eITEM_USE_LIMIT_NULL 
				&& pStdItem->m_CanUseType < eITEM_USE_LIMIT_MAX) //使用次数限制
			{
				LONGLONG nStaticNum = 0;
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_DAY)
				{   
					if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDailyInit(pStdItem->m_nIndex))
					{
						((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountDaily(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
						nStaticNum = pStdItem->m_CanUseCount;
					} 
					else
					{ 
						nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDaily(pStdItem->m_nIndex);
					} 
				}
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_WEEK)
				{ 
					if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeekInit(pStdItem->m_nIndex))
					{
						((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountWeek(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
						nStaticNum = pStdItem->m_CanUseCount;
					} 
					else
					{ 
						nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeek(pStdItem->m_nIndex);
					} 
				}
				if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_MONTH)
				{  
					if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonthInit(pStdItem->m_nIndex))
					{
						((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountMonth(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
						nStaticNum = pStdItem->m_CanUseCount;
					} 
					else
					{ 
						nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonth(pStdItem->m_nIndex);
					} 
				}
				
				data << pStdItem->m_nIndex; 
				data << (int)nStaticNum; 
				ccount++;
			} 
		}
	}
	INT_PTR nOffer2 = data.getPosition();  
	data.setPosition(nOffer);
	data << ccount; 
	data.setPosition(nOffer2); 

	pack.flush();
	// GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmNoMoreCoin, tstFigthing);
}
 
void CUserBag::HandQueryItemsExtraInfoOne(CDataPacketReader &packet)
{ 
	int nItemIndex = 0;
	packet >> nItemIndex;
	
	//TestAddItem(266, 1);
	CActorPacket pack;
	INT_PTR nCount = Inherited::count();
	 
	CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
	data <<(BYTE)enBagSystemID << (BYTE)sBagSystemQueryItemsExtraInfoOne;

	// data <<(int)GetBagCount(0);
	// data <<(int)GetBagCount(1);
	// data <<(int)GetBagCount(2); 
 
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemIndex);
	if(pStdItem == NULL) 
	{ 
		data << 0; 
		data << 0;  
		pack.flush();
		return;
	}
	
	//特定使用次数 年月周的限制
	if(pStdItem->m_CanUseType > eITEM_USE_LIMIT_NULL 
		&& pStdItem->m_CanUseType < eITEM_USE_LIMIT_MAX) //使用次数限制
	{
		LONGLONG nStaticNum = 0;
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_DAY)
		{   
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDailyInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountDaily(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexDaily(pStdItem->m_nIndex);
			} 
		}
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_WEEK)
		{ 
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeekInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountWeek(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexWeek(pStdItem->m_nIndex);
			} 
		}
		if(pStdItem->m_CanUseType == eITEM_USE_LIMIT_MONTH)
		{  
			if(((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonthInit(pStdItem->m_nIndex))
			{
				((CActor *)m_pEntity)->GetStaticCountSystem().OnReduceItemInnerStaticCountMonth(pStdItem->m_nIndex, - (pStdItem->m_CanUseCount + 1)); 
				nStaticNum = pStdItem->m_CanUseCount;
			} 
			else
			{ 
				nStaticNum = ((CActor *)m_pEntity)->GetStaticCountSystem().OnGetItemInnerIndexMonth(pStdItem->m_nIndex);
			} 
		}
		
		data << pStdItem->m_nIndex; 
		data << (int)nStaticNum;  
	}
	else
	{
		data << 0; 
		data << 0;  
		pack.flush();
		return;
	}
	pack.flush();
	// GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmNoMoreCoin, tstFigthing);
}
//添加道具
void CUserBag::HandAddItem(CDataPacketReader &packet)
{
	if (m_pEntity->HasState(esStateStall))
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;
	int nCitemid = 0;
	int nCitemNum = 0;
	WORD nCount =0;
	packet >>nCount; //拆分出来的数量
	WORD nItemId, nItemCount;
	byte btQuality, btStrong, btBind;
	std::vector<CUserItemContainer::ItemOPParam> v_iParams;
	int needCount = 0;
	int errorCode = 0;
	for(int i = 0; i < nCount; i++)
	{
		packet >> nItemId >> nItemCount >> btQuality >> btStrong >> btBind;
		CUserItemContainer::ItemOPParam iParam;
		iParam.wItemId = nItemId;
		iParam.wCount = (WORD)nItemCount;
		iParam.btQuality = btQuality;
		iParam.btStrong =  btStrong;
		iParam.btBindFlag = btBind;

		needCount += nItemCount;
		v_iParams.push_back(iParam);
	}
	if(needCount > availableMinCount())
	{
		errorCode = 1; 
	}
	if(v_iParams.size() == 0)
	{
		errorCode = 2;
	}
	if(!errorCode)
	{
		std::vector<CUserItemContainer::ItemOPParam>::iterator it = v_iParams.begin();
		for(;it!= v_iParams.end(); it++)
		{
			AddItem(*it,m_pEntity->GetEntityName(),GameLog::clAddItem);
		}
	}else
	{
		CActorPacket pack;
		CDataPacket &data= pActor->AllocPacket(pack);
		data << (BYTE)enBagSystemID << (BYTE)enBagSystemcAddItem <<(BYTE)errorCode;
		pack.flush();
	}
}


void  CUserBag::HandGetBagEnlargeFee(CDataPacketReader &packet)
{
	// BYTE nEnGrids = 0;				//扩展多少个格子
	// packet >> nEnGrids;
	// INT_PTR nFee = 0, nNeedItemCount = 0;
	// CActor *pActor = (CActor *)m_pEntity;
	// unsigned int nBagCount = pActor->GetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT);//当前的背包的数量
	// BAGCONFIG &config = GetLogicServer()->GetDataProvider()->GetBagConfig();
	// unsigned int nNewGrids = nBagCount + nEnGrids;
	// BYTE bEnLargeFlag = 1;
	// if (nBagCount == (unsigned int)config.nMaxBagCount)
	// {
	// 	pActor->SendOldTipmsgWithId(tpItemBagTooBig, ttFlyTip);
	// 	return;
	// }
	// if( nNewGrids > (unsigned int)config.nMaxBagCount ) //已经是最大的容量了
	// {
	// 	pActor->SendOldTipmsgFormatWithId(tpItemBagOverMax, ttFlyTip, nEnGrids);
	// 	return;
	// }
	// for (INT_PTR i = nBagCount + 1; i <= nNewGrids; i++)
	// {
	// 	INT_PTR nDiffCount = i;//- config.nDefaultBagCount;
	// 	nNeedItemCount += nDiffCount * config.nEnLargeFeeArray[spefItemCount];
	// 	nFee += nDiffCount * config.nEnLargeFeeArray[spefFeeVal];
	// }
	// CActorPacket pack;
	// CDataPacket &data= pActor->AllocPacket(pack);
	// data << (BYTE)enBagSystemID << (BYTE)enBagSystemsSendBagEnlargeFee <<(BYTE)bEnLargeFlag << (int)nFee <<int(nNeedItemCount)<< BYTE(nEnGrids);
	// pack.flush();
	
}

//扩展背包
bool CUserBag::EnlargeBag(int nAddGridCount)
{
	if(nAddGridCount <=0) return false;
	CActor *pActor = (CActor *)m_pEntity;
	INT_PTR nOldGrid = pActor->GetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT); //获取当前的网格的数量
	
	BAGCONFIG &config = GetLogicServer()->GetDataProvider()->GetBagConfig();
	if(config.nMaxBagCount  <=  nOldGrid) 
	{
		pActor->SendOldTipmsgWithId(tpItemBagTooBig,ttDialog ); 
		return false;
	}
	INT_PTR nNewGrid = nOldGrid + nAddGridCount; //新增加的格子之后的
	if(nNewGrid > config.nMaxBagCount ) //不能超过最大
	{
		nNewGrid =config.nMaxBagCount ; 
	}
	pActor->SetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT,(unsigned int)nNewGrid);
	pActor->SendOldTipmsgFormatWithId(tpItemEnlargeSucceed,ttTipmsgWindow,nNewGrid -nOldGrid );
	setCapacity(nNewGrid);
	return true;
}
//处理网络消息包，扩展背包
void CUserBag::HandEnlargeBag(CDataPacketReader &packet)
{
// {
// 	BYTE nEnGrids;
// 	packet >> nEnGrids;
// 	CActor * pActor = (CActor*)m_pEntity;
// 	INT_PTR nCurGrids = pActor->GetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT);
// 	BAGCONFIG & config = GetLogicServer()->GetDataProvider()->GetBagConfig();
// 	INT_PTR nNewGrids = nCurGrids + nEnGrids;
// 	if (nCurGrids == config.nMaxBagCount)
// 	{
// 		pActor->SendOldTipmsgWithId(tpItemBagTooBig, ttFlyTip);
// 		return;
// 	}
// 	if (nNewGrids > config.nMaxBagCount)
// 	{
// 		pActor->SendOldTipmsgFormatWithId(tpItemBagOverMax, ttFlyTip, nEnGrids);
// 		return;
// 	}
// 	unsigned int nNowDt = GetGlobalLogicEngine()->getMiniDateTime();
// 	int nCurAddSecs = nNowDt - m_nResetBagTime;
// 	if (nCurAddSecs < 0)
// 	{
// 		nCurAddSecs = 0;
// 	}
// 	int nCurTimes = pActor->GetProperty<int>(PROP_ACTOR_BAG_TIME) + nCurAddSecs;
// 	INT_PTR nFee = 0, nGiveExp = 0, nNeedTimes = 0;
// 	for (INT_PTR i = nCurGrids + 1; i <= nNewGrids; i++)
// 	{
// 		INT_PTR nDiffCount = i ;//- config.nDefaultBagCount;
// 		nGiveExp += config.nEnlargeGiveExp[nDiffCount-1];
// 		int nTimes = config.nEnlargeTimes[nDiffCount-1]*60;
// 		if (nCurTimes >= nTimes)
// 		{
// 			nCurTimes -= nTimes;
// 		}
// 		else
// 		{
// 			nFee += nDiffCount * config.nEnLargeFeeArray[spefFeeVal];
// 		}
// 	}
// 	if (nFee > 0 && pActor->GetMoneyCount(eMoneyType(config.nEnLargeFeeArray[spefFeeType])) < nFee)
// 	{
// 		pActor->SendOldTipmsgFormatWithId(tpItemMoneyNotEnoughEnlargeBag, ttFlyTip,
// 			nEnGrids, nFee, CMiscSystem::GetMoneyName(config.nEnLargeFeeArray[spefFeeType]));
// 		return;
// 	}
// 	LPCSTR sLogStr = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLogEnlargeBag);
// 	if (nFee > 0 && !pActor->ChangeMoney(config.nEnLargeFeeArray[spefFeeType], -1*nFee, GameLog::clEnlargeBag, 0, sLogStr))
// 	{
// 		return;
// 	}
// 	if (nGiveExp > 0)
// 	{
// 		pActor->AddExp(nGiveExp, GameLog::clEnlargeBag);
// 	}
// 	pActor->SetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT, (unsigned int)nNewGrids);
// 	pActor->SetProperty<int>(PROP_ACTOR_BAG_TIME, nCurTimes);
// 	m_nResetBagTime = nNowDt;
// 	pActor->SendOldTipmsgFormatWithId(tpItemEnlargeSucceed, ttFlyTip, nEnGrids, nGiveExp);
// 	setCapacity(nNewGrids);
}


void CUserBag::HandSlpitItem(CDataPacketReader &packet)
{
	if (m_pEntity->HasState(esStateStall))
	{
		return;
	}
	CUserItem::ItemSeries itemGuid;
	WORD nCount =0;
	packet >> itemGuid.llId; //物品的Guid
	packet >>nCount; //拆分出来的数量
	if( !Inherited::SplitItem(itemGuid,nCount) )
	{
		((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemSplitBagFail,tstUI); //通知拆分失败
	}
}

/*
道具合并叠加
两方道具必须都是绑定或者非绑定
*/
void CUserBag::HandMergeItem(CDataPacketReader &packet)
{
	if (m_pEntity->HasState(esStateStall))
	{
		return;
	}
	CUserItem::ItemSeries srcGuid,tgtGuid;
	packet >> srcGuid.llId;  // 源的GUID
	packet >> tgtGuid.llId;  // 目标GUID
	if( (tgtGuid.llId ==  srcGuid.llId)  || (! Inherited::MergeItem(srcGuid,tgtGuid)) )
	{
		((CActor*)m_pEntity)->SendOldTipmsgWithId(tpItemMergeFail,ttTipmsgWindow); //通知拆分失败
	}
}



INT_PTR CUserBag::GetBagEnlargeFeeErorCode(INT_PTR &nFee,INT_PTR & nGridCount,INT_PTR &nMoneyType)
{
	unsigned int nBagCount =m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_BAG_GRID_COUNT);//当前的背包的数量
	BAGCONFIG &config = GetLogicServer()->GetDataProvider()->GetBagConfig();
	bool bCanEnlared = true; //能否扩大背包格子
	if((unsigned int)config.nMaxBagCount  <=  nBagCount) //已经是最大的容量了
	{
		nFee =0; //费用
		nGridCount =0;
		nMoneyType =0;
		return tpItemBagTooBig;
	}
	else
	{
		/*
		int nEnlargeTimes = (config.nMaxBagCount - nBagCount)/ config.nEnlargeOneTime; //还能扩展多少次
		if(nEnlargeTimes <=0 )
		{
			nEnlargeTimes =1;
		}
		else if(nEnlargeTimes > MAX_BAG_ENLARGE_TIMES)
		{
			nEnlargeTimes = MAX_BAG_ENLARGE_TIMES; //防止溢出
		}
		nMoneyType =  config.enlargeFeeTypeArray[MAX_BAG_ENLARGE_TIMES -nEnlargeTimes]; //金钱的类型
		nFee = config.enlargeFeeArray[MAX_BAG_ENLARGE_TIMES -nEnlargeTimes];  //金钱的数值
		
		int nTime= (nBagCount - config.nDefaultBagCount) / config.nEnlargeOneTime; //第几次扩展
		if(nTime <0)
		{
			nTime =0;
		}
		else if(nTime >= MAX_BAG_ENLARGE_TIMES)
		{
			nTime = MAX_BAG_ENLARGE_TIMES-1;
		}

		nMoneyType =  config.enlargeFeeTypeArray[nTime]; //金钱的类型
		nFee = config.enlargeFeeArray[nTime];  //金钱的数值
		nGridCount = config.nEnlargeOneTime; //每次扩大的个数
		
		if( ((CActor *)m_pEntity)->GetMoneyCount(nMoneyType) < nFee ) //金钱不够
		{
			return tpItemMoneyNotEnoughEnlargeBag;
		}*/
		
	}
	return tpNoError;
}

void CUserBag::Save(PACTORDBDATA  pData)
{
	if(m_pEntity ==NULL) return;
	
	if(HasDbDataInit() ==false ) 
	{
		OutputMsg(rmWaning,"CUserBag::Save has not inited");	
		return; //如果没有装载玩家的数据就不存储
	}
	if( !HasDataModified()) return;//如果数据没有修改，并且没有数据没保存成功
	
	CDataPacket& dataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveBagItem);
	dataPacket << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket <<(unsigned int ) m_pEntity->GetId(); //玩家ID

	size_t count = Inherited::count();
	// OutputMsg(rmTip,_T("玩家 %s 保存背包数据count=%d"),m_pEntity->GetEntityName(), count);
	dataPacket  << (BYTE )itItemBag << (int)count  ; //玩家物品的数量
	// CUserItem ** pItem =Inherited::operator CUserItem**();
	for (UINT_PTR i=0; i<count; i ++ )
	{
		dataPacket <<(CUserItem) *m_pUserItemList[i]; //把一个物品打包过去
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
	//SetDataOnWayFlag(true);     //有数据在路上
	SetDataModifyFlag(false);  //数据的修改全部发往了db服务器
}

VOID CUserBag::OnTimeCheck(TICKCOUNT nTickCount)
{	
	// if (!m_pEntity || m_pEntity->GetType() != enActor)
	// {
	// 	return;
	// }
	// //检查物品到期时间
	// CMiniDateTime tc;
	// CUserItem *pUserItem, **pItemList;
	
	
	// if (m_ItemTimeChkTimer.CheckAndSet(nTickCount))
	// {
	// 	// pItemList = *this;
	// 	tc = GetLogicServer()->GetLogicEngine()->getMiniDateTime();
	// 	//此处必须降序循环，否则会造成内存错误！因为循环中有删除操作。
	// 	for (INT_PTR i=count()-1; i>-1; --i)
	// 	{
	// 		pUserItem = m_pUserItemList[i];
	// 		//物品过期！
	// 		if (pUserItem && (pUserItem->time.tv & CMiniDateTime::RecordFlag) && tc >= pUserItem->time)
	// 		{
	// 			//发送物品时间到期的提示消息
	// 			if (m_pEntity->GetType() == enActor)
	// 			{
	// 				SendItemTimeUp((CActor*)m_pEntity, pUserItem, 0);

	// 				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(((CUserItem*)pUserItem)->wItemId);
	// 				//记录日志
	// 				if (pStdItem && pStdItem->m_Flags.recordLog)
	// 				{
	// 					CUserBag::LogChangeItemCount(pUserItem, pStdItem, -pUserItem->wCount, 
	// 						"Item TimeUp", m_pEntity->GetId(), m_pEntity->GetEntityName(),
	// 						GameLog::clTimeupItem, ((CActor*)m_pEntity)->GetAccount());
	// 				}
	// 			}			

	// 			//删除物品
	// 			RemoveItem(i, "ITU", GameLog::clTimeupItem, true);

				
	// 		}
	// 	}
	// }
}

unsigned int CUserBag::GetItemLeftTime(CUserItem *pUserItem)
{
	return 0;
}

//添加物品
VOID CUserBag::OnAddItem(const CStdItem * pStdItem, INT_PTR nItemCount,INT_PTR nLogIdent,const CUserItem *pUserItem)
{
	if(pStdItem ==NULL) return;
	if(pStdItem->isEquipment()) 
	{
		/*if( pUserItem )
		{
			NotifyItemChange(pUserItem);			//获得装备，下发此装备的消息
		}
		*/
		return; //装备不处理，只处理普通物品
	}
	//数据发生了改变
	SendBagItemChangeTipmsg(pStdItem->m_nIndex ,nItemCount,pUserItem,nLogIdent);

	// if( pStdItem->m_btType == Item::itExpBox )		//经验魔盒
	// {
	// 	AssignExpItem();		//指定一个经验盒子作为当前吸收经验的盒子
	// }
}

VOID CUserBag::OnAddItemById(int itemId, INT_PTR nItemCount, INT_PTR nLogIdent,const CUserItem *pUserItem)
{
	// 为什么装备和普通物品区分开来发tipmsg？？？
	const CStdItem *pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(itemId);
	if (!pItem)  return;
	if(m_pEntity ==NULL || m_pEntity->IsInited() ==false) return;
	if (pItem->isEquipment()) 
	{
		/*if( pUserItem )
		{
			NotifyItemChange(pUserItem);			//获得装备，下发此装备的消息
		}
		*/
		return;
	}
	SendBagItemChangeTipmsg(itemId, nItemCount, pUserItem, nLogIdent);

	// if( pItem->m_btType == Item::itExpBox )		//经验道具(经验盒子)
	// {
	// 	AssignExpItem();		//指定一个经验盒子作为当前吸收经验的盒子
	// }
}

void CUserBag::SendBagItemChangeTipmsg(INT_PTR nItemID,INT_PTR nCount,const CUserItem * pUserItem,INT_PTR nLogIndex,  int nMsgId) const
{
	if ( GetHandle().GetType() != enActor || nCount ==0 )
	{
		return;
	}
	//获得actorID
	unsigned int nActorID = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);

	if(nCount >0) //获得物品
	{
		//脱下装备和合并,放到仓库不提示获得物品
		if (nLogIndex != GameLog::clTakeOffItem &&
			nLogIndex != GameLog::clEquipSplit  &&
			nLogIndex != GameLog::clDeport2Bag	&&
			nLogIndex != GameLog::clHeroTakeOffEquip
			)
		{
			((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(tpItemBagGetItem,ttTipmsgWindow,
				CUserBag::GetItemLink((int)nItemID,pUserItem),nCount);
			//这里的功能是用于提示客户端新获得了一件装备或者源泉，不是从身上脱下来的,仓库拿到的，或

			const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemID);

			if(pStdItem ==NULL) return;
			if (pUserItem)
			{
				bool isEquipment = pStdItem->isEquipment();
				CActorPacket pack;
				CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
				data << (BYTE)enBagSystemID << (BYTE)enBagSystemsBagGetNewItem << (pUserItem->series) <<(WORD)nItemID<<(WORD)nCount;
				pack.flush();
			}
		}
		else
		{
			return;
		}
	}
	else //失去
	{
		//背包到仓库，合并和穿上不提示失去物品
		if (nLogIndex != GameLog::clTakeOnItem &&
			nLogIndex != GameLog::clItemMerge &&
			nLogIndex != GameLog::clBag2Deport &&
			nLogIndex != GameLog::clHeroTakeOnEquip
			)
		{
			if (nMsgId == 0)
			{
				nMsgId = tpItemBagLostItem;
			}
			((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(nMsgId,ttTipmsgWindow,
				CUserBag::GetItemLink((int)nItemID,pUserItem),-nCount);
		}
		else
		{
			return ;
		}
	}
	if(nCount <0) return; //拾取物品不需要广播
	//if(nLogIndex == GameLog::clKillMonsterItem) //只有杀怪掉落的物品才广播
	//{
	//	CTeam *pTeam = ((CActor*)m_pEntity)->GetTeam();
	//	if( pTeam)
	//	{
	//		char * pTips = CUserBag::GetItemLink((int)nItemID,pUserItem);
	//		if(pTips ==NULL) return;
	//		const TEAMMEMBER* pMember=  pTeam->GetMemberList() ;
	//		for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT ; i++)
	//		{
	//			if(pMember[i].pActor && pMember[i].nActorID !=nActorID )
	//			{
	//				pMember[i].pActor->SendOldTipmsgFormatWithId(tpLootItem,ttTipmsgWindow,
	//					((CActor*)m_pEntity)->GetEntityName(),pTips);
	//			}
	//		}
	//	}
	//}
	

}

VOID CUserBag::LogNewItem(const CUserItem *pUserItem, const CStdItem *pStdItem, LPCSTR lpSender, const INT_PTR nLogIdent) const 
{
	if(pUserItem ==NULL || !pStdItem )return;
	if ( GetHandle().GetType() == enActor )
	{
	}
}

VOID CUserBag::LogItemCountChange(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nLogIdent) const 
{
	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
	}
}

VOID CUserBag::SendAddItem(const CUserItem *pUserItem,INT_PTR nLogIdent, BYTE bBatchUseShow,BYTE bNotice) const 
{
	if(m_pEntity ==NULL || pUserItem ==NULL ) return;
	//数据发生了改变
	

	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
		CActorPacket pack;
		CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
		data << (BYTE)enBagSystemID << (BYTE)enBagSystemsAddItem <<(BYTE)0;
		(CUserItem)(*pUserItem)>> data;
		data<< bBatchUseShow << bNotice;
		pack.flush();
		
		const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem)
		{
			if ( pStdItem->isEquipment() ) //装备需要发提示
			{
				(( CUserBag*)this)->SendBagItemChangeTipmsg(pUserItem->wItemId,pUserItem->wCount,(CUserItem*)pUserItem,nLogIdent ); //装备肯定是一件
			}
			//如果是任务物品或物品需要作为任务物品处理，这里脚本的事件通知
			if (pStdItem->m_btType == Item::itQuestItem || pStdItem->m_Flags.asQuestItem)
			{
				INT_PTR nItCount = ((CActor*)m_pEntity)->GetBagSystem().GetItemCount(pUserItem->wItemId);
				// ((CActor *)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtCollect,pUserItem->wItemId, nItCount,FALSE);
			}
		}
	}
}

//物品的叠加数量发生改变的时候发消息给玩家
//isGetNewCount=true 是获得新的物品数量，非整理或合并导致的数量变化
VOID CUserBag::SendItemCountChange(const CUserItem *pUserItem, bool isGetNewCount, BYTE bBatchUseShow) const
{
	if ( m_pEntity->GetHandle().GetType() == enActor )
	{
		if(m_pEntity->IsInited())
		{
			CActorPacket pack;
			CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
			data << (BYTE)enBagSystemID << (BYTE)enBagSystemsItemCountChange << (LONG64)pUserItem->series.llId << (WORD)pUserItem->wCount << isGetNewCount << bBatchUseShow;
			pack.flush();
			
			//如果是任务物品或物品需要作为任务物品处理，这里脚本的事件通知
			const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
			if (pStdItem && pStdItem->m_btType == Item::itQuestItem || pStdItem->m_Flags.asQuestItem)
			{
				INT_PTR nItCount = ((CActor*)m_pEntity)->GetBagSystem().GetItemCount(pUserItem->wItemId);
				// ((CActor *)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtCollect,pUserItem->wItemId,nItCount,FALSE);
			}

		}
	}
}
//删除物品
VOID CUserBag::OnDeleteItem(const CStdItem * pStdItem, INT_PTR nItemCount,INT_PTR nLogIdent, int nMsgId,const CUserItem *pUserItem)
{
	if(!pStdItem->isEquipment())
	{
		SendBagItemChangeTipmsg(pStdItem->m_nIndex,-nItemCount,pUserItem,nLogIdent, nMsgId);
	}
}

void CUserBag::SendItemChangeLog(int nType, int nCount, int nItemId, int nLogId, LPCSTR sDes)
{
	if(nLogId)
	{
		// int nType = 1; //add
		// if(nCount < 0)
		// 	nType = 2; //del
		if(nType == 1 && GameLog::Log_Deport2Bag != nLogId)
			((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtGetItem,nCount, nItemId);
		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
		if (pStdItem && pStdItem->m_Flags.recordLog)
		{
			GetLogicServer()->SendItemLocalLog(nLogId,(CActor*)m_pEntity,nItemId, pStdItem->m_btType,nCount,nType);

			char logs[1024] = {0};

			if(sDes && strlen(sDes) > 0) {
				sprintf_s(logs,1024,"%s|%s",pStdItem->m_sName,sDes);
			}else {
				sprintf_s(logs,1024,"%s",pStdItem->m_sName);
			}
			logs[1023] = 0;
			
			
			GetLogicServer()->GetLogClient()->SendItemLocalLog(nType, (WORD)nLogId, 
						(unsigned int)((CActor *)m_pEntity)->GetId(),
						(unsigned int)((CActor *)m_pEntity)->GetAccountID(), ((CActor *)m_pEntity)->GetAccount(), ((CActor *)m_pEntity)->GetEntityName(), (WORD)nItemId,
						(int)nCount, logs,((CActor *)m_pEntity)->getOldSrvId());
		}
	}
}

VOID CUserBag::SendDeleteItem(const CUserItem *pUserItem,INT_PTR nLogIdent) const 
{
	if(m_pEntity ==NULL) return;
	if ( m_pEntity->GetHandle().GetType() == enActor && m_pEntity->IsInited() )
	{
		CActorPacket pack;
		CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
		data << (BYTE)enBagSystemID << (BYTE)enBagSystemsDelItem <<(LONG64) pUserItem->series.llId;
		pack.flush();

		const CStdItem  * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem  &&  ( pStdItem->isEquipment() ) ) //装备需要发提示
		{
			//删除装备的，基本都是-1
			INT_PTR nCount = -pUserItem->wCount;
			if(nCount ==0) 
			{
				nCount =-1;
			}
			SendBagItemChangeTipmsg(pUserItem->wItemId,nCount,(CUserItem*)pUserItem,nLogIdent ); //装备肯定是一件
		}

	}
}

//db返回
VOID CUserBag::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载背包的数据出错,实体指针为MULL"));
		return;
	}
	//CUserItem data;
	//装载物品数据
	
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadBagItem &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回物品的列表
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("db return bag item repeated,actorid=%d "),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
			return;
		}
		
		int nCount =0;
		reader >> nCount;
		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
		OutputMsg(rmTip,_T("玩家 %s 装载背包数据count=%d"),m_pEntity->GetEntityName(), nCount);
		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem << reader;
			// INT_PTR nReadSize = reader.readBuf(&userItem,sizeof(CUserItem));
			// if( sizeof(CUserItem) !=  nReadSize) //读取内存
			// {
			// 	OutputMsg(rmError,"[%s] read net io error,size=%d ",__FUNCTION__,(int) nReadSize);
			// 	continue;
			// }
			if(availableCount(userItem.wPackageType))
			{
				CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
				if(pUerItem)
				{
					AddItem(pUerItem,m_pEntity->GetEntityName(),0,false, false); //不需要记录日志的，不需要通知客户端
					//addCostCount(pUerItem->wPackageType);
				}
			}
			else
			{
				OutputMsg(rmError,_T("玩家%s的物品数据出现错误， 背包类型%d 不足"),m_pEntity->GetEntityName(), userItem.wPackageType);
				//数据错误需要查看什么问题
				return;
			}
			// if(i < capacity())
			// {
			// 	INT_PTR nReadSize = reader.readBuf(&userItem,sizeof(CUserItem));
			// 	if( sizeof(CUserItem) !=  nReadSize) //读取内存
			// 	{
			// 		OutputMsg(rmError,"[%s] read net io error,size=%d ",__FUNCTION__,(int) nReadSize);
			// 		continue;
			// 	}
			// 	CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
			// 	if(pUerItem)
			// 	{
			// 		AddItem(pUerItem,m_pEntity->GetEntityName(),0,false); //不需要记录日志的，不需要通知客户端
			
			// 	}
			// }
			// else
			// {
			// 	OutputMsg(rmError,_T("玩家%s的物品数据出现错误，个数超过了%d,被丢弃"),m_pEntity->GetEntityName(),capacity());
			// }
		
		}
	    OnDbInitData(); //标记DB的数据初始化完毕
		//重置收集类的任务的进度值
		// if (m_pEntity->GetType() == enActor)
		// 	((CActor*)m_pEntity)->GetQuestSystem()->ResetQuestValues();
		
		AssignExpItem();		//指定一个经验盒子作为当前吸收经验的盒子

		return;
	}
	//zac
	switch(nCmd)
	{
		//存储db的数据成功
	case jxInterSrvComm::DbServerProto::dcSaveBagItem:
		{
			//保存的数据成功了，表明没有数据没有保存成功
			if(nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
			{
				//SetDataOnWayFlag(false);
			}
		}
		break;
	case jxInterSrvComm::DbServerProto::dcLoadActiveBag:
		{
			if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
			{
				OutputMsg(rmError,_T("load user active bag item error!"));
				return;
			}
			SendActiviItemByMail(reader);

			//返回活动背包列表，发送给客户端
			//SendActiveItem(reader,ACTORITEM);
			break;
		}
	case jxInterSrvComm::DbServerProto::dcGetActiveItem:
		{
			if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
			{
				((CActor *)m_pEntity)->SendOldTipmsgFormatWithId(tpActivityBagGetItemFail,ttDialog);
				return;
			}
			GetActiveItem(reader,ACTORITEM);
			break;
		}
	case jxInterSrvComm::DbServerProto::dcDeleteActiveItem:
		{
			if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
			{
				OutputMsg(rmError,_T("remove active bag item fail!error=%d"),nErrorCode);
			}
			break;
		}
	}
}


//背包系统接收来自于会话服进程的数据
VOID CUserBag::OnSsRetData( INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	
	if (!m_pEntity || m_pEntity->GetType() != enActor) return;
	switch(nCmd)
	{
	case sUserItemList:
		{
			if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
			{
				OutputMsg(rmError,_T("load user active bag item error!"));
				return;
			}
			//返回活动背包列表，发送给客户端
			SendActiveItem(reader,ACCOUNTITEM);
			break;
		}
	case sGetUserItem:
		{
			if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
			{
				((CActor *)m_pEntity)->SendOldTipmsgFormatWithId(tpActivityBagGetItemFail,ttDialog);
				return;
			}
			GetActiveItem(reader,ACCOUNTITEM);
			break;
		}
	case sDeleteUserItem:
		{
			if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
			{
				OutputMsg(rmError,_T("remove active bag item fail!error=%d"),nErrorCode);
			}
			break;
		}
	case sAddValueCard:
		{
			if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
			{				
				OnUseSACardResult(op_Failed, (int)nErrorCode, 0, 0, 0, true);				
				return;
			}
			OnGetSACardInfo(reader, false);
			break;
		}
	case sQueryAddValueCard:
		{
			if (nErrorCode != jxInterSrvComm::SessionServerProto::neSuccess)
			{				
				OnUseSACardResult(op_Failed, (int)nErrorCode, 0, 0, 0, false);				
				return;
			}
			OnGetSACardInfo(reader, true);
			break;
		}
	}
}

VOID CUserBag::Destroy()
{
	/*
	CList<CUserItem*>::Iterator it(m_sellUserList);
	CList<CUserItem*>::NodeType *pNode;
	CUserItem *pItem;
	//回收内存
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pItem = *pNode;
		it.remove(pNode);
		GetGlobalLogicEngine()->DestroyUserItem(pItem);
	}
	*/
	for(INT_PTR i= m_sellUserList.count() -1; i>=0; i--)
	{
		if(m_sellUserList[i]) {
			GetGlobalLogicEngine()->DestroyUserItem(m_sellUserList[i]);
		}
	}
	m_sellUserList.clear();

	Clear(NULL, 0);
	InheritedSybSystem::Destroy();
	
}

//添加一个卖给商店的物品，如果成功返回true
bool CUserBag::AddSellItem(CUserItem * itemPtr)
{
	/*
	CList<CUserItem*>::Iterator it(m_sellUserList);
	CList<CUserItem*>::NodeType *pNode;
	CUserItem *pItem;

	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pItem = * pNode;
		if(pItem== itemPtr) //已经存在了
		{
			return false;
		}
	}
	m_sellUserList.linkAtLast(itemPtr);
	*/
	for(INT_PTR i=0; i< m_sellUserList.count(); i++)
	{
		if(m_sellUserList[i] == itemPtr) return false;
	}
	m_sellUserList.add(itemPtr);
	return true;
}


//删除一个卖给物品的指针，如果成功返回true
bool CUserBag::DelSellItem(CUserItem *pUserItem)
{
	/*
	CList<CUserItem*>::Iterator it(m_sellUserList);
	CList<CUserItem*>::NodeType *pNode;
	CUserItem *pItem;

	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pItem = * pNode;
		if(pItem== pUserItem) //已经存在了
		{
			it.remove(pNode);
			return true;
		}
	}
	*/
	for(INT_PTR i=0; i< m_sellUserList.count(); i++)
	{
		if(m_sellUserList[i] == pUserItem) 
		{
			m_sellUserList.remove(i);
			return true;
		}
	}
	return false;
}

//获得卖给商店的物品的指针
CUserItem *  CUserBag::GetSellItem(unsigned long long guid)
{
	/*
	CList<CUserItem*>::Iterator it(m_sellUserList);
	CList<CUserItem*>::NodeType *pNode;
	CUserItem *pItem;

	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pItem = * pNode;
		if(pItem->series.llId == guid)
		{
			return pItem;
		}
	}
	*/
	
	for(INT_PTR i= m_sellUserList.count() -1; i > -1 ; i--)
	{
		if(m_sellUserList[i] && m_sellUserList[i]->series.llId == guid) 
		{
			return m_sellUserList[i];
		}
	}

	return NULL;
}

void CUserBag::HandActiveBagList( CDataPacketReader &packet )
{
	ReqActiveBagData();
}

void CUserBag::ReqActiveBagData()
{
	if (!m_pEntity || m_pEntity->GetType() != enActor) return;
	unsigned int nAccountId = ((CActor*)m_pEntity)->GetAccountID();
	unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);

	//向会话服务器请求活动背包数据
	CLogicSSClient* pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &data = 
		pSSClient->allocProtoPacket(cUserItemList); //分配一个 网络包
	data << (unsigned int)nAccountId;
	data << (unsigned int)nActorId;
	pSSClient->flushProtoPacket(data);

	//向数据服务器发送活动背包数据
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	CDataPacket &data1 = 
		pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadActiveBag); //分配一个 网络包
	data1 << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	data1 << (unsigned int)nAccountId;
	data1 << (unsigned int)nActorId;
	pDbClient->flushProtoPacket(data1);
}

void CUserBag::HandGetActiveBagItem( CDataPacketReader &packet )
{
	if (!m_pEntity || m_pEntity->GetType() != enActor) return;
	if(m_bGetActiveItemFlag)
	{
		OutputMsg(rmError,_T("Actor GetActiveItem err"));
		return;
	}

	unsigned int nAccountId = ((CActor*)m_pEntity)->GetAccountID();
	unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);

	INT64 nId = 0;
	BYTE nType = 0;
	packet >> nId;
	packet >> nType;

	if (nType == ACCOUNTITEM)
	{
		CLogicSSClient* pSSClient = GetLogicServer()->GetSessionClient();
		CDataPacket &data = 
			pSSClient->allocProtoPacket(cGetUserItem); //分配一个 网络包
		data << (unsigned int)nAccountId;
		data << (unsigned int)nActorId;
		data << (INT64)nId;
		pSSClient->flushProtoPacket(data);

		m_bGetActiveItemFlag = true;
	}
	else
	{
		CDataClient* pDbClient = GetLogicServer()->GetDbClient();
		CDataPacket &data = 
			pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcGetActiveItem); //分配一个 网络包
		data << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		data << (unsigned int)nAccountId;
		data << (unsigned int)nActorId;
		data << (INT64)nId;
		pDbClient->flushProtoPacket(data);

		m_bGetActiveItemFlag = true;
	}
}


void CUserBag::SendActiveItem( CDataPacketReader &reader,INT_PTR nType )
{
	int nCount = 0;
	reader >> nCount;

	
	CMailSystem::MAILATTACH stMailAtach[MAILATTACHCOUNT];
	int nMailItemCount = 0, nAllCount = nCount;
	byte nBindFlag = 0;
	unsigned int nMyId = m_pEntity->GetId();
	LPCSTR sTitle = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpActivityMailTitle);
	LPCSTR sContent = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpActivityMailContent);
	for (INT_PTR i = 0; i < nCount; i++)
	{
		INT64 nId = 0;
		int nItemId = 0;
		BYTE bBind = 0,bStrong = 0, bQuality = 0;
		int nItemCount = 0;

		reader >> nId;
		reader >> nItemId;
		reader >> bBind;
		reader >> bStrong;
		reader >> bQuality;
		reader >> nItemCount;
		nAllCount--;
		if (bBind > 0)
		{
			nBindFlag = bBind;
		}
		CMailSystem::MAILATTACH & oneItem = stMailAtach[nMailItemCount++];
		oneItem.item.wCount = nItemCount;
		if (nItemId >= MAXITEMID)
		{
			int nType = nItemId - MAXITEMID;
			switch(nType)
			{
			case mtBindCoin:
				oneItem.nType = qatBindMoney;
				break;
			case mtCoin:
				oneItem.nType = qatMoney;
				break;
			case mtBindYuanbao:
				oneItem.nType = qatBindYb;
				break;
			case mtYuanbao:
				oneItem.nType = qatYuanbao;
				break;
			default:
				oneItem.nType = qatBindMoney;
				break;
			}			
		}
		else
		{
			oneItem.nType = qatEquipment;
			oneItem.item.wItemId = nItemId;
		}
		if (nMailItemCount >= MAILATTACHCOUNT || nAllCount <= 0)
		{
			CMailSystem::SendMail(nMyId, sTitle, sContent, stMailAtach);
			memset(stMailAtach, 0, sizeof(stMailAtach));
			nMailItemCount = 0;
		}
		//删除物品
		if (nType == ACCOUNTITEM)
		{
			CDataPacket &data = GetLogicServer()->GetSessionClient()->allocProtoPacket(cDeleteUserItem); //通知数据库删除这个物品
			data << (int)nMyId;
			data << (INT64)nId;
			GetLogicServer()->GetSessionClient()->flushProtoPacket(data);
		}
		else
		{
			CDataClient* pDbClient = GetLogicServer()->GetDbClient();
			CDataPacket &data = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteActiveItem); //
			data << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
			data << (int)nMyId;
			data << (INT64)nId;
			pDbClient->flushProtoPacket(data);
		}
	}	

	if (nCount > 0 && m_pEntity->GetType() == enActor)//有活动背包物品，提示
	{
		((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(tpActivityItemNotity,ttDialog);
	}
	
}

void CUserBag::OnGetSACardInfo(CDataPacketReader &reader, bool bQuery)
{
	if (m_pEntity->GetType() != enActor) return;
	CActor *pActor = (CActor *)m_pEntity;
	UINT64 nSN = 0;
	int nType = 0, nSubType = 0;
	char szCreateTime[32] = {0};
	reader >> nSN >> nType >> nSubType;
	reader.readString(szCreateTime, ArrayCount(szCreateTime));		
	OnUseSACardResult(op_Succ, 0, nSN, nType, nSubType, bQuery);
	OutputMsg(rmNormal, _T("Query SACard OK! SN=%llu, type=%d, subType=%d card createtime:%s"), nSN, nType, nSubType, szCreateTime);
}

void CUserBag::OnUseSACardResult(int nResult, int nErrorCode, UINT64 nSN, int nType, int nSubType, bool bQuery)
{
	SACardInfo info;
	info.m_nSN			= nSN;
	info.m_nType		= nType;
	info.m_nSubType		= nSubType;
	info.m_nErrorCode	= nErrorCode;
	info.m_bQuery		= bQuery;
	CActor *pActor = (CActor *)m_pEntity;
	pActor->GetAsyncOpCollector().NotifyAsyncOpResult(asop_useSACard, nResult, &info);
}

void CUserBag::UseServicesAddedCard(UINT64 nCardSeries, bool bQueryOrUse)
{
	if (m_pEntity->GetType() != enActor) return;
	CActor *pActor = (CActor *)m_pEntity;
	unsigned int nUID = pActor->GetAccountID();
	unsigned int nCID = pActor->GetId();
		
	if (nCardSeries < MinSeriesLmt)
	{
		//暂时屏蔽
		//OnUseSACardResult(op_Failed, op_ParamError, nCardSeries, 0, 0, bQueryOrUse);
		//return;
	}

	if (pActor->GetAsyncOpCollector().StartAsyncOp(asop_useSACard, 10000, 0, true))
	{
		CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
		jxSrvDef::INTERSRVCMD cmd = cAddValueCard;
		if (bQueryOrUse)
			cmd = cQueryAddValueCard;
		CDataPacket &pack = pSSClient->allocProtoPacket(cmd);
		pack << nUID << nCID << nCardSeries;
		pack.writeString(m_pEntity->GetEntityName());
		pack.writeString( ((CActor*)m_pEntity)->GetAccount());
		pSSClient->flushProtoPacket(pack);
	}
}


void CUserBag::GetActiveItem( CDataPacketReader &reader,INT_PTR nType )
{
	//int nItemId = 0;
	//int nItemCount = 0;
	//INT64 nId = 0;
	//CUserItemContainer::ItemOPParam ItemPara;
	//LPCSTR sMemo = NULL;
	//reader >> nId;
	//reader >> nItemId;
	//reader >> ItemPara.btBindFlag;
	//reader >> ItemPara.btStrong;
	//reader >> ItemPara.btQuality;
	//reader >> nItemCount;
	//reader >> ItemPara.nSmith1 >> ItemPara.nSmith2 >> ItemPara.nSmith3 >> ItemPara.nBestSmith; 
	//reader >> sMemo;
	//
	//if (nItemId == 0)
	//{
	//	((CActor *)m_pEntity)->SendOldTipmsgFormatWithId(tpActivityBagGetItemFail,ttDialog);
	//	return;
	//}
	//BYTE nItemType = (nItemId >= MAXITEMID)? (nItemId-MAXITEMID):10;
	//bool boGetSucc = true;//是否成功领取
	//if (nItemType == 10)
	//{
	//	ItemPara.wCount = (BYTE)nItemCount;
	//	ItemPara.wItemId = (WORD)nItemId;
	//	if (CanAddItem(ItemPara))
	//	{
	//		AddItem(ItemPara,sMemo,GameLog::clServerGiveItem);//背包添加物品
	//	}
	//	else
	//	{
	//		((CActor *)m_pEntity)->SendOldTipmsgFormatWithId(tpBagIsFull,ttDialog);
	//		boGetSucc = false;
	//	}
	//}
	//else
	//{
	//	((CActor *)m_pEntity)->ChangeMoney(nItemType,nItemCount,GameLog::clServerGiveMoney,0,sMemo);//金钱类
	//}
	//if (boGetSucc)
	//{
	//	unsigned int nActorId = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
	//	if (nType == ACCOUNTITEM)
	//	{
	//		CDataPacket &data = GetLogicServer()->GetSessionClient()->allocProtoPacket(cDeleteUserItem); //通知数据库删除这个物品
	//		data << (unsigned int)nActorId;
	//		data << (INT64)nId;
	//		GetLogicServer()->GetSessionClient()->flushProtoPacket(data);
	//	}
	//	else
	//	{
	//		CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	//		CDataPacket &data = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteActiveItem); //
	//		data << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	//		data << (unsigned int)nActorId;
	//		data << (INT64)nId;
	//		pDbClient->flushProtoPacket(data);
	//	}

	//	//通知客户端
	//	CActorPacket AP;
	//	((CActor *)m_pEntity)->AllocPacket(AP);
	//	AP << (BYTE)enBagSystemID << (BYTE)sUpdateActiveBag << (INT64)nId << (BYTE)1 << (BYTE)nType;
	//	AP.flush();

	//	m_bGetActiveItemFlag = false;
	//}
}

VOID CUserBag::LogChangeItemCount(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nTgtId, LPCSTR sTargetName, const INT_PTR nLogIdent, LPCSTR sTargetAccount)
{
	//TODO:实现记录获得物品并叠加到现有物品的日志，日志号是nLogIdent，物品给予者是lpSender，物品变更的数量为nCountChg
	if(pUserItem ==NULL || !pStdItem)
		return;
	char sSeries[64]; 
	sprintf(sSeries,"%lld",pUserItem->series.llId); //序列号
	
	if(GetLogicServer()->GetLocalClient() && pStdItem->m_Flags.recordLog)
	{
	}
}

VOID CUserBag::SendItemTimeUp(CActor *pActor, const CUserItem *pUserItem, INT_PTR nPos)
{
	CActorPacket ap;
	pActor->AllocPacket(ap);

	ap << (BYTE)enBagSystemID << (BYTE)enBagSystemsItemTimeUp << (Uint64)pUserItem->series.llId << (BYTE)nPos;
	ap.flush();
}

void CUserBag::OnEnterScene()
{
	if (m_pEntity->GetType() != enActor) return;
	CActor *pActor = (CActor *)m_pEntity;
	CFuBen *pFb = pActor->GetFuBen();
	if (!pFb) return;
	CScene *pScene = pActor->GetScene();
	if (!pScene) return;

	int fbId	= pFb->GetFbId();
	int sceneId = pScene->GetSceneId();	

	const CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();		
	for (INT_PTR i = count()-1; i >= 0; i--)
	{
		CUserItem *pItem = (*this)[i];
		if(pItem)
		{
			const CStdItem *pStdItem = itemProvider.GetStdItem(pItem->wItemId);
			if (pStdItem)
			{			
				if (pStdItem->needDelWhenExitScene(sceneId))
				{		
					//记录日志
					if (pStdItem && pStdItem->m_Flags.recordLog)
					{
						CUserBag::LogChangeItemCount(pItem, pStdItem, -pItem->wCount, 
							"Exit scene destroy", pActor->GetProperty<unsigned int>(PROP_ENTITY_ID), pActor->GetEntityName(), 
							GameLog::clLeftSceneRemoveItem, pActor->GetAccount());
					}
					RemoveItem(i, "scene destroy", GameLog::clLeftSceneRemoveItem, true);
					
				}		
			}
		}
		
	}
}

INT_PTR CUserBag::RemoveItemByType(int nType, const char * sComment, int nLogId)
{
	const CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();		
	for (INT_PTR i = 0; i < count(); i++)
	{
		CUserItem *pItem = (*this)[i];
		if(!pItem)continue;
		const CStdItem *pStdItem = itemProvider.GetStdItem(pItem->wItemId);
		if (pStdItem && pStdItem->m_btType == nType)
		{
			RemoveItem(i, sComment, nLogId);
			return 1;
		}
	}

	return 0;
}

CUserItem* CUserBag::GetItemByType(int nType)
{
	const CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();		
	for (INT_PTR i = 0; i < count(); i++)
	{
		CUserItem *pItem = (*this)[i];
		if(!pItem)continue;
		const CStdItem *pStdItem = itemProvider.GetStdItem(pItem->wItemId);
		if (pStdItem && pStdItem->m_btType == nType)
		{
			return pItem;
		}
	}

	return NULL;
}

bool CUserBag::IsStrongLevel( INT_PTR nStrong )
{
	for(INT_PTR i=0; i< count(); i ++)
	{
		CUserItem *pItem = (*this)[i];
		if(pItem !=NULL && pItem->btStrong >= nStrong)
		{
			return true;
		}
	}
	return false;
}

/*ZGame不使用
bool CUserBag::IsForgeItem()
{
	for(INT_PTR i=0; i< count(); i ++)
	{
		CUserItem *pItem = (*this)[i];
		if(pItem !=NULL && pItem->btSmithCount > 0)
		{
			return true;
		}
	}
	return false;
}
*/

CUserItem* CUserBag::GetDuraFullItemById( INT_PTR nItemId )
{
	// for(INT_PTR i=0; i< count(); i ++)
	// {
	// 	CUserItem *pItem = (*this)[i];
	// 	if(pItem !=NULL && pItem->wItemId == nItemId)
	// 	{
	// 		const CStdItem * pStdItem  = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pItem->wItemId);
	// 		if(pStdItem ==NULL) continue;
	// 		int nMax =pStdItem->m_dwDura; //最大的耐久
	// 		int nCurrent =MAKELONG(pItem->wDura,pItem->wDuraMax); //当前玩家的耐久
	// 		if(nCurrent != nMax )continue;
	// 		return pItem;
	// 	}
	// }
	return NULL;
}

void CUserBag::BagQuestTarget( INT_PTR propId )
{
	// INT_PTR nValue = 0;
	// switch(propId)
	// {
	// case Item::ipItemStrong:
	// 	nValue = CQuestData::qstEquipStrong;
	// 	break;
	// case Item::ipItemHole1IsOpen:
	// case Item::ipItemHole2IsOpen:
	// case Item::ipItemHole3IsOpen:
	// case Item::ipItemHole4IsOpen:
	// 	nValue = CQuestData::qstEquipHole;
	// 	break;
	// case Item::ipItemHole1Item:
	// case Item::ipItemHole2Item:
	// case Item::ipItemHole3Item:
	// case Item::ipItemHole4Item:
	// 	nValue = CQuestData::qstEquipInjay;
	// 	break;
	// default:
	// 	break;
	// }
	// if(nValue)
		// ((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtStrengthen, nValue, 1);
}

void CUserBag::HandGetUpgradeCfg(CDataPacketReader &packet)
{
	/*
	CScriptValueList paramList;
	((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeGetCanUpGrdConfig);
	((CActor*)m_pEntity)->OnEvent(paramList,paramList);
	*/
}

INT_PTR CUserBag::GetAllItemCount()
{
	INT_PTR nAllCount = 0;
	for(INT_PTR i=0; i< count(); i ++)
	{
		CUserItem *pItem = (*this)[i];
		if(pItem !=NULL )
		{
			nAllCount += pItem->wCount;
		}
	}
	return nAllCount;
}

void CUserBag::ClearEquipSharp(bool boCast)
{
	INT_PTR nCount = count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CUserItem * pItem = (*this)[i];
		if (pItem != NULL && pItem->btSharp > 0)
		{
			pItem->btSharp = 0;
			if (boCast)
			{
				NotifyItemChange(pItem);
			}
		}
	}
}

CUserItem* CUserBag::GetItemByIdx( INT_PTR nIdx )
{
	if( nIdx < 0 || nIdx > count())
		return NULL;
	return (*this)[nIdx];
}

int CUserBag::GetBagItemIndex(CUserItem * itemPtr)
{
	if(itemPtr ==NULL) return -1;

	const CStdItem * pstdItem=NULL;
	CStdItemProvider &provider  =GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	size_t count = Inherited::count();

	for (int i=0; i<count; i ++ )
	{
		CUserItem *pUserItem =(CUserItem*) (*this)[i];
		if(pUserItem != itemPtr) continue;

		pstdItem =provider.GetStdItem(pUserItem->wItemId);
		if(pstdItem ==NULL)continue;
		return i;
	}
	return -1;
}

INT_PTR CUserBag::RemoveItemDura(CUserItem * pUserItem,INT_PTR nCount,LPCSTR sComment,INT_PTR nLogID,bool bNeedFreeMemory)
{
	// bool bNeedDelete = false;
	// const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	// if(pStdItem ==NULL) return 0;
	// INT_PTR nChgValue = (INT_PTR)(nCount * pStdItem->m_dwUseDurDrop);
	// if (nChgValue > 0)
	// {
	// 	if (pUserItem->wDura < nChgValue)
	// 	{
	// 		return 0;
	// 	}

	// 	if (!SetItemProperty(pUserItem, Item::ipItemDua, (INT_PTR)pUserItem->wDura-nChgValue))
	// 	{
	// 		return 0;
	// 	}

	// 	if (pUserItem->wDura <= 0)
	// 		bNeedDelete = true;
	// }

	// if (bNeedDelete)
	// {
	// 	DeleteItem(pUserItem, 1, sComment, nLogID, bNeedFreeMemory);
	// }

	// return nChgValue;

}

CUserItem * CUserBag::GetEquipItemPtr(const CUserItem::ItemSeries itemGuid, int &nItemPos, int &nHeroId, int nEntityId)
{
	CUserItem* pUserItem = NULL;
	if (nEntityId <= 0) //如果是人物则先找背包再找装备
	{
		//背包
		pUserItem = Inherited::FindItemByGuid(itemGuid);
		if (pUserItem)	
		{
			nItemPos = 1;	//背包
			nHeroId = 0;
			return pUserItem;
		}
		//玩家装备
		pUserItem = ((CActor*)m_pEntity)->GetEquipmentSystem().GetEquipByGuid(itemGuid);
		if (pUserItem)	
		{
			nItemPos = 2;	//玩家身上
			nHeroId = 0;
			return pUserItem;
		}

		//遍历英雄
		CVector<CHeroSystem::HERODATA> &heros = ((CActor*)m_pEntity)->GetHeroSystem().GetHeroList();
		for (INT_PTR i = 0; i < heros.count(); i++)
		{
			CHeroEquip *pEquips = &(heros[i].equips);
			if (pEquips) {
				pUserItem = pEquips->GetEquipByGuid(itemGuid);
				if (pUserItem)
				{
					nItemPos = 3;	//英雄身上
					nHeroId = heros[i].data.bID;	//英雄ID
					return pUserItem;
				}
			}
			
		}
	}
	else	//如果是英雄 则 nEntityId为英雄ID
	{
		const CHeroSystem::HERODATA *pHero = ((CActor*)m_pEntity)->GetHeroSystem().GetHeroData(nEntityId);
		if (pHero)
		{
			CHeroEquip equips = pHero->equips;
			pUserItem = equips.GetEquipByGuid(itemGuid);
			if (pUserItem)	
			{
				nItemPos = 3;	//英雄身上
				nHeroId = pHero->data.bID;
				return pUserItem;
			}
		}
	}
	nItemPos = 0;
	nHeroId = 0;
	return NULL;
}







void CUserBag::SendActiviItemByMail( CDataPacketReader & reader )
{
	//应该不使用，没维护代码
	int nCount = 0;
	reader >> nCount;

	unsigned int nMyId = m_pEntity->GetId();
	LPCSTR sTitle = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpActivityMailTitle);
	LPCSTR sContent = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpActivityMailContent);
	for (INT_PTR i = 0; i < nCount; i++)
	{
		byte nMailItemCount = 0;
		CMailSystem::MAILATTACH stMailAtach[MAILATTACHCOUNT];
		INT64 nId = 0;
		int nItemId = 0;
		BYTE bBind = 0,bStrong = 0, bQuality = 0;
		int nItemCount = 0;
		int nSmith[CUserItem::MaxSmithAttrCount], nInitSmith;
		int nRever1, nRever2;

		reader >> nId;
		reader >> nItemId;
		reader >> bBind;
		reader >> bStrong;
		reader >> bQuality;
		reader >> nItemCount;
		reader >> nSmith[0] >> nSmith[1] >> nSmith[2] >> nSmith[3] >> nSmith[4] >> nInitSmith >> nRever1 >> nRever2;
		

		CMailSystem::MAILATTACH & oneItem = stMailAtach[nMailItemCount++];
		oneItem.item.wCount = nItemCount;
		if (nItemId >= MAXITEMID)
		{
			int nType = nItemId - MAXITEMID;
			switch(nType)
			{
			case mtBindCoin:
				oneItem.nType = qatBindMoney;
				break;
			case mtCoin:
				oneItem.nType = qatMoney;
				break;
			case mtBindYuanbao:
				oneItem.nType = qatBindYb;
				break;
			case mtYuanbao:
				oneItem.nType = qatYuanbao;
				break;
			default:
				oneItem.nType = qatBindMoney;
				break;
			}			
		}
		else
		{
			CMailSystem::AddMailAttach(oneItem, oneItem.nType, nItemId, bQuality, bStrong, bBind > 0? true:false, nSmith);
			CMailSystem::SendMail(nMyId, sTitle, sContent,stMailAtach);
			
		}
		
		CDataClient* pDbClient = GetLogicServer()->GetDbClient();
		CDataPacket &data = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcDeleteActiveItem); //
		data << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		data << (int)nMyId;
		data << (INT64)nId;
		pDbClient->flushProtoPacket(data);
	}	

	if (nCount > 0 && m_pEntity->GetType() == enActor)//有活动背包物品，提示
	{
		((CActor*)m_pEntity)->SendOldTipmsgFormatWithId(tpActivityItemNotity,ttDialog);
	}
	
}

void CUserBag::HandUseStrongItem(CDataPacketReader &packet)
{

}
void CUserBag::HandSmashGoldEgg(CDataPacketReader &packet)
{

}
void CUserBag::HandLuckAwardOpt(CDataPacketReader &packet)
{

}
void CUserBag::HandGetPerfectCompTimes(CDataPacketReader &packet)
{

}
void CUserBag::HandUseFiveAttrItem(CDataPacketReader &packet)
{

}
void CUserBag::HandUseItemRecoverEx(CDataPacketReader &packet)
{

}

void CUserBag::BatchUseItemAtOnce(WORD wItemId, WORD wCount)
{
	CUserItem* pUserItem = Inherited::FindItem(wItemId, -1, -1, wCount);
	if( !pUserItem || pUserItem->wCount < wCount )
	{
		return;
	}
	if (wCount == 1)
	{
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem ==NULL) return;
		CScene* pScene = m_pEntity->GetScene();
		int x,y;
		m_pEntity->GetPosition(x,y);
		if (pScene && pScene->HasMapAttribute(x,y,aaNotItemId,pStdItem->m_nIndex))
		{
			//((CActor *)m_pEntity)->SendOldTipmsgWithId(tpItemRegionCanNotUseItem,ttFlyTip + ttTipmsgWindow);
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemRegionCanNotUseItem, tstUI);
			return;
		}
		if (m_pEntity->HasState(esStateStall))
		{
			((CActor *)m_pEntity)->SendOldTipmsgWithId(tpStallNoUseItem, ttFlyTip);
			return;
		}
		if (pStdItem->isEquipment()) //如果是装备的话
		{
			((CActor*)m_pEntity)->GetEquipmentSystem().TakeOn(pUserItem); //穿上装备
		}
		else
		{
			INT_PTR nTipmsgID =CUserEquipment::CheckTakeOnCondition(m_pEntity,pStdItem, pUserItem);
			if (nTipmsgID)
			{	
				((CActor *)m_pEntity)->SendOldTipmsgWithId(nTipmsgID,ttFlyTip + ttTipmsgWindow);
				return; //不能使用
			}
			OnUseItem(pUserItem, pStdItem);
		}
	}
	else
		BatchUseItem(pUserItem, wCount);
}



/*
批量使用物品，注意根据物品作用不同，批量处理的逻辑是不一样的
协议（8,8）
*/
void CUserBag::HandBatchUseItem(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	int itemId = 0;
	int wCount = 0;
	CUserItem::ItemSeries itemGuid;
	packet >> itemId >> wCount >>  itemGuid.llId;
	
	CUserItem* pUserItem = Inherited::FindItemByGuid(itemGuid);
	if(pUserItem == NULL) return;
	if(pUserItem->wCount < wCount)
		return;
	
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return;
	CScene* pScene = m_pEntity->GetScene();
	int x,y;
	m_pEntity->GetPosition(x,y);
	if (pScene && pScene->HasMapAttribute(x,y,aaNotItemId,pStdItem->m_nIndex))
	{
		//((CActor *)m_pEntity)->SendOldTipmsgWithId(tpItemRegionCanNotUseItem,ttFlyTip + ttTipmsgWindow);
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemRegionCanNotUseItem, tstUI);
		return;
	}
	if (m_pEntity->HasState(esStateStall))
	{
		((CActor *)m_pEntity)->SendOldTipmsgWithId(tpStallNoUseItem, ttFlyTip);
		return;
	}
	if (pStdItem->isEquipment()) //如果是装备的话
	{
		return;
		// ((CActor*)m_pEntity)->GetEquipmentSystem().TakeOn(itemGuid); //穿上装备
	}

	for(int i = 0 ; i < wCount; i++)
	{
		INT_PTR nTipmsgID =CUserEquipment::CheckTakeOnCondition(m_pEntity,pStdItem, pUserItem);
		if (nTipmsgID)
		{	
			if(m_pEntity->GetHandle().GetType() ==enActor)
			{
				// 通知客户端使用物品失败
				CActorPacket pack ;
				CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
				data <<  (BYTE) (enBagSystemID) << (BYTE) (enBagSystemsUseItemResult);
				data <<  (WORD)(pStdItem->m_nIndex);
				data << (BYTE) 0;
				pack.flush();

				((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmItemCanNotUse, tstUI);
			}
			return; //不能使用
		}
		bool result = OnUseItem(pUserItem, pStdItem, 1, true);
		if(!result)
			break;
	}
	CActorPacket pack ;
	CDataPacket &data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data <<  (BYTE) (enBagSystemID) << (BYTE) (enBagSystemsUseItemResult);
	data <<  (WORD)(pUserItem->wItemId);
	data << (BYTE) (1);
	pack.flush();
}

void CUserBag::BatchUseItem(CUserItem* pUserItem, WORD wCount)
{

	// if( !pUserItem || pUserItem->wCount < wCount )
	// {
	// 	return;
	// }
	// const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	// if (!pStdItem)
	// {
	// 	return;
	// }
	// BYTE btBacthType = pStdItem->m_btBatchType;
	// int nBatchValue = pStdItem->m_nBatchValue;
	// if ( btBacthType <= 0 || btBacthType >= batMAx || nBatchValue == 0 )
	// {
	// 	return;
	// }
	// CActor* pActor = (CActor*)m_pEntity;
	// CScene* pScene = m_pEntity->GetScene();
	// int x,y;
	// m_pEntity->GetPosition(x,y);
	// if (pScene && pScene->HasMapAttribute(x,y,aaNotItemId,pStdItem->m_nIndex))
	// {
	// 	//pActor->SendOldTipmsgWithId(tpItemRegionCanNotUseItem,ttFlyTip + ttTipmsgWindow);
	// 	pActor->SendTipmsgFormatWithId(tmItemRegionCanNotUseItem, tstUI);
	// 	return;
	// }
	// if (m_pEntity->HasState(esStateStall))
	// {
	// 	pActor->SendOldTipmsgWithId(tpStallNoUseItem, ttFlyTip);
	// 	return;
	// }
	// INT_PTR nTipmsgID =CUserEquipment::CheckTakeOnCondition(m_pEntity,pStdItem, pUserItem);
	// if (nTipmsgID)
	// {	
	// 	pActor->SendOldTipmsgWithId(nTipmsgID,ttFlyTip + ttTipmsgWindow);
	// 	return;
	// }
	// /*
	// //技能熟练度物品特殊处理
	// if( btBacthType ==  batSkillExpItem)
	// {
	// 	   pActor->GetSkillSystem().BatchUseSkillExpItem(pUserItem,pStdItem,wCount);
	// 	   return;
	// }
	// */
	// if( btBacthType == batNormalItem )
	// {
	// 	/*for( int i=0; i<wCount;i++ )
	// 	{*/
	// 		//pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtUseItem, 会在OnUseItem（）中触发
	// 	OnUseItem(pUserItem, pStdItem, wCount);
	// 	//}
	// 	return;
	// }

	// /*判断该道具是否有使用次数限制
	// 相关配置：ItemUseCountCfg
	// */
	// int nLastCount = 0;				//该道具可使用的剩余次数
	// int nMaxCount  = 0;				//该道具可使用的最大次数
	// CNpc * pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	// if (pNpc != NULL)
	// {
	// 	CScriptValueList paramList, retParamList;
	// 	paramList << pActor;
	// 	paramList << pUserItem->wItemId;
	// 	if( !pNpc->GetScript().Call("getItemUseCount", paramList, retParamList) )	//检查该道具是否有使用次数限制
	// 	{
	// 		const RefString & s = pNpc->GetScript().getLastErrorDesc();
	// 		return;
	// 	}

	// 	if( retParamList.count() > 2 )
	// 	{
	// 		nLastCount  = (int)retParamList[1];		//这里是1、2
	// 		nMaxCount	= (int)retParamList[2];		//
	// 	}
	// 	if( nMaxCount > 0 )					//该道具有使用限额
	// 	{
	// 		if( nLastCount <= 0 )			//今日已经没有可以使用的次数
	// 		{
	// 			((CActor *)m_pEntity)->SendOldTipmsgWithId(tpItemNoDuraCanNotUseItem,ttFlyTip + ttTipmsgWindow);
	// 			return;
	// 		}
	// 		if( wCount > nLastCount )		//如果大于剩余次数，则取剩余次数
	// 		{
	// 			wCount = nLastCount;
	// 		}
	// 	}
	// }

	// INT_PTR nCount = DeleteItem(pUserItem, wCount, "batch use item",GameLog::clUserItem);
	// if ( nCount < wCount )
	// {
	// 	return;
	// }

	// if( nMaxCount > 0 )
	// {
	// 	CScriptValueList paramList2, retParamList2;
	// 	paramList2 << pActor;
	// 	paramList2 << pUserItem->wItemId;
	// 	paramList2 << nCount;
	// 	if( !pNpc->GetScript().Call("AddDailyItemUseCount", paramList2, retParamList2) )
	// 	{
	// 		const RefString & s = pNpc->GetScript().getLastErrorDesc();
	// 		return;
	// 	}
	// }

	// bool boUse = false;
	// INT_PTR nTotalValue = (INT_PTR)nBatchValue * wCount;
	// INT_PTR nTotalValueAdd = nTotalValue;					//实际增加的数值
	// switch(btBacthType)
	// {
	// case batExp:
	// 	{
	// 		pActor->AddExp((INT_PTR)nTotalValue,GameLog::clBatchUseItem);
	// 		boUse = true;
	// 		break;
	// 	}
	// case batBindCoin:
	// 	{
	// 		pActor->ChangeMoney(mtBindCoin, nTotalValue, GameLog::clBatchUseItem, 0, "batch use item");
	// 		boUse = true;
	// 		break;
	// 	}
	// case batCoin:
	// 	{
	// 		pActor->ChangeMoney(mtCoin, nTotalValue, GameLog::clBatchUseItem, 0, "batch use item");
	// 		boUse = true;
	// 		break;
	// 	}
	// case batCirclePower:
	// 	{
	// 		pActor->ChangeCircleSoul((int) nTotalValue );
	// 		boUse = true;
	// 		break;
	// 	}
	// case batGuildCoin:
	// 	{
	// 		CGuild* pGuild = pActor->GetGuildSystem()->GetGuildPtr();
	// 		if (!pGuild)
	// 		{
	// 			return;
	// 		}
	// 		pGuild->ChangeGuildCoin((int)nTotalValue,GameLog::clBatchUseItem, "batch use item");
	// 		boUse = true;
	// 		break;
	// 	}
	// case batDragonSoulItem:
	// 	{
	// 		//pActor->ChangePropertyValue(PROP_ACTOR_DRAGONSOUL_VALUE, nTotalValue, tpAddDragonSoulValue, tpReduceDragonSoulValue );
	// 		boUse = true;
	// 		break;
	// 	}
	// case batBindGold:
	// 	{
	// 		pActor->ChangeMoney(mtBindYuanbao, nTotalValue, GameLog::clBatchUseItem, 0, "batch use item");	
	// 		boUse = true;
	// 		break;
	// 	}
	// case batBossJifen:
	// 	{
	// 		pActor->ChangeMoney(mtBossJiFen, nTotalValue, GameLog::clBatchUseItem, 0, "batch use item");	
	// 		boUse = true;
	// 		break;
	// 	}
	// default:break;
	// }
	// if ( boUse )
	// {
	// 	// pActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtUseItem, pStdItem->m_nIndex, wCount);

	// 	if( nMaxCount > 0 )		//有最大使用次数限制
	// 	{
	// 		CScriptValueList paramList3, retParamList3;
	// 		paramList3 << pActor;
	// 		paramList3 << pUserItem->wItemId;
	// 		paramList3 << nCount;
	// 		paramList3 << nTotalValueAdd;
	// 		paramList3 << (int)(nLastCount-nCount);
	// 		if( !pNpc->GetScript().Call("OnBatchUseItem", paramList3, retParamList3) )
	// 		{
	// 			const RefString & s = pNpc->GetScript().getLastErrorDesc();
	// 			return;
	// 		}
	// 	}
	// }
}

void CUserBag::HandArrangeItemList(CDataPacketReader & packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	ArrangeItemList(GameLog::Log_ItemMerge);
	CActorPacket pack;
	CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enBagSystemID << (BYTE)enBagSystemsArrangeBagOver;
	pack.flush();
}

/*
道具合并叠加（绑定感染）
两方道具允许绑定->非绑或非绑->绑定
*/
void CUserBag::HandMergeItemBindInfect(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor* pActor = (CActor*)m_pEntity;

	CUserItem::ItemSeries itemSrcGuid;			//源道具
	CUserItem::ItemSeries itemDesGuid;			//目标道具
	packet >> itemSrcGuid.llId >> itemDesGuid.llId ;
	CUserItem* pUserItemSrc	= Inherited::FindItemByGuid(itemSrcGuid);
	CUserItem* pUserItemDes	= Inherited::FindItemByGuid(itemDesGuid);
	if( !pUserItemSrc || !pUserItemDes )
	{
		return;
	}

	WORD nDesOldCount = pUserItemDes->wCount;			//目标道具的原数量
	if( Inherited::MergeItem(itemSrcGuid, itemDesGuid,true) )	//道具合并，绑定感染
	{
		//绑定污染，记录日志
		if (GetLogicServer()->GetLocalClient())
		{
		}
	}
}

/*
指定一个经验道具作为当前吸收经验道具
*/
void CUserBag::AssignExpItem()
{
	// size_t count = Inherited::count();

	// m_pExpItem =NULL;
	// unsigned int nMaxExp =0;
	// const CStdItem * pStdItem=NULL;
	// CStdItemProvider &provider  =GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	// //找一个经验最满的作为经验盒子
	// for( UINT_PTR i=0; i < count; i++ )
	// {
	// 	CUserItem *pUserItem =(CUserItem*) (*this)[i];
	// 	if( pUserItem ==NULL ) 
	// 	{
	// 		continue;
	// 	}
	// 	pStdItem =provider.GetStdItem( pUserItem->wItemId );
	// 	if( pStdItem == NULL )
	// 	{
	// 		continue;
	// 	}
	// 	if( pStdItem->m_btType == Item::itExpBox )
	// 	{
	// 		unsigned int newDura = MAKELONG( pUserItem->wDura,  pUserItem->wDuraMax );	//当前的经验
	// 		if( newDura < pStdItem->m_dwDura && newDura >= nMaxExp )				//经验的前提是没有满
	// 		{
	// 			m_pExpItem = pUserItem;
	// 			nMaxExp = newDura;
	// 		}
	// 	}
	// }
}

/*
在背包中获取一个指定类型的物品指针
有特殊搜索条件，需要另行处理
*/
CUserItem* CUserBag::GetOneItemByItemTypeInBag(BYTE nItemType)
{
	return NULL;			//没有符合条件的物品
}

/*
增加经验道具（或类似功能道具）吸收的经验
通过装备耐久
*/
void CUserBag::ChangeHpPotValue(CUserItem *pEquip,int nValue)
{
	// if(pEquip == NULL) return;

	// const CStdItem * pStdItemd = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pEquip->wItemId);
	// if(pStdItemd == NULL) return;

	// if( Item::itExpBox == pStdItemd->m_btType )
	// {
	// 	unsigned int nEquipValue = MAKELONG( pEquip->wDura,  pEquip->wDuraMax);		//高低位合并后，才是当前经验（耐久度）
	// 	if( nEquipValue >= pStdItemd->m_dwDura ) 
	// 	{
	// 		return ; //已经满了
	// 	}

	// 	/*
	// 	byte nVipRate = ((CActor *)m_pEntity)->GetVipPrivile(ExpItemCollectRate);		//VIP比率
	// 	if (nVipRate > 0)
	// 	{
	// 		nValue = (int)(nValue * nVipRate/100 + nValue);
	// 	}
	// 	*/

	// 	nEquipValue += nValue; 
	// 	if(nEquipValue <0) nEquipValue =0; // 最小是0

	// 	bool flag =false;
	// 	if(nEquipValue >= pStdItemd->m_dwDura)
	// 	{
	// 		nEquipValue = pStdItemd->m_dwDura;
	// 		flag =true;
	// 	}

	// 	pEquip->wDura    =  LOWORD(nEquipValue);	//低16位
	// 	pEquip->wDuraMax =  HIWORD(nEquipValue);	//高16位

	// 	if(flag)
	// 	{
	// 		AssignExpItem();
	// 	}
	// 	SetDataModifyFlag(true);
	// }
}

CUserItem*  CUserBag::GetItemPtrHighestStar(WORD  nItemId)
{
	const CStdItem * pStdItemd = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemId);
	if(pStdItemd == NULL  || !pStdItemd->isEquipment()) return NULL;
	CUserItem* pUserItem = NULL;
	CUserItem *pItem = NULL;
    for (INT_PTR i = count()-1; i >= 0; --i)
    {
        pItem = (*this)[i];
		if(pItem && pItem->wItemId == nItemId)
		{
		    if(pUserItem == NULL)
			{
			    pUserItem =  pItem;
			}else
			{
				if(pItem->wStar > pUserItem->wStar)
				{
				    pUserItem = pItem;
				}
			}
		}
    }
	return pUserItem;
}

bool CUserBag::HasItemIdentifyAttrs( CUserItem* pUserItem )
{
	if( !pUserItem )
	{
		return false;
	}

	for( INT_PTR i = 0; i < CUserItem::MaxSmithAttrCount; i++ )
	{
		if( pUserItem->smithAttrs[i].nValue )		//存在鉴定属性
		{
			return true;
		}
	}
	return false;
}
/*
装备鉴定属性转移
*/
void CUserBag::TransferItemIdentifyAttrs( CUserItem* pSrcUserItem, CUserItem* pDesUserItem )
{
}

void CUserBag::TestAddItem(int itemid, int itemNum)
{
	CUserItemContainer::ItemOPParam iParam;
	iParam.wItemId = itemid;
	iParam.wCount = (WORD)itemNum;
	iParam.btQuality = 1;
	iParam.btStrong =  1;
	iParam.btBindFlag = 1;

	AddItem(iParam,m_pEntity->GetEntityName(),GameLog::clGMAddItem);


	CUserItem* pUserItem = Inherited::FindItem(266, -1, -1, 1);
	if( pUserItem==NULL) return ; //找不到这个物品
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return;

	OnUseItem(pUserItem,pStdItem,1,true, 0);

}



bool CUserBag::AddItemByItemId(INT_PTR nItemID, INT_PTR nCount, INT_PTR nStar, INT_PTR nLostStar,INT_PTR nBind, INT_PTR bInSourceType, INT_PTR nAreaId,INT_PTR nTime)
{
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nItemID);
	if (pStdItem == NULL)
	{
		return false ;//参数错误
	}
	if (nCount <=0 )
	{
		return false;
	}
	if (pStdItem->m_wDupCount <= 1)
	{
		for (WORD i = 0; i < (WORD)nCount; i++ )
		{
			CUserItem* pUserItem = GetLogicServer()->GetLogicEngine()->AllocUserItem(true);
			pUserItem->wItemId			= (WORD)nItemID;
			pUserItem->wCount			= (WORD)1;
			pUserItem->wStar			= (WORD)nStar;
			pUserItem->bLostStar		= (BYTE)nLostStar;		//TODO
			pUserItem->bInSourceType	= (BYTE)bInSourceType;
			pUserItem->nAreaId		= (int)nAreaId;
			pUserItem->btFlag			= (BYTE)nBind;
			pUserItem->btQuality		= (BYTE)0;

			CStdItem::AssignInstance  (pUserItem,pStdItem);
			if(nTime > 0)
			{
				pUserItem->nCreatetime = (unsigned int)nTime;
				pUserItem->nCreatetime.startRecord(GetLogicServer()->GetLogicEngine()->getMiniDateTime());
			}
			INT_PTR nAddItemCount = 0;
			nAddItemCount = AddItem(pUserItem,m_pEntity->GetEntityName(),GameLog::clAddItem);

			if( nAddItemCount <=0)
			{
				return false;
			}
		}
	}
	else
	{
		CUserItemContainer::ItemOPParam itemData;
		itemData.wItemId		= (WORD)nItemID;
		itemData.btQuality		= (BYTE)0;
		itemData.wCount			= (WORD)nCount;
		itemData.wStar			= (WORD)nStar;
		itemData.bLostStar		= (BYTE)nLostStar;
		itemData.bInSourceType	= (BYTE)bInSourceType;
		itemData.nAreaId	= (int)nAreaId;
		itemData.btBindFlag		= (BYTE)nBind;
		INT_PTR nAddItemCount	= 0;
		nAddItemCount = AddItem(itemData,m_pEntity->GetEntityName(),GameLog::clGMAddItem);

		if( nAddItemCount >0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}



void CUserBag::HandleExChangeMoney(CDataPacketReader &packet)
{
	int nCount = 0;
	BYTE nType = 0;
	packet >> nType;
	packet >> nCount;
	if(nType == 1) 
	{
		int nCoin = ((CActor *)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_COIN);
		if(nCoin < nCount)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmCoinNotEnough, tstUI);
			return;
		}
		((CActor *)m_pEntity)->ChangeCoin(-nCount, GameLog::Log_MoneyExChange, 0, "");
		((CActor *)m_pEntity)->ChangeBindCoin(nCount, GameLog::Log_MoneyExChange, 0, "");
		}
	else
	{
		int nCoin = ((CActor *)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_YUANBAO);
		if(nCoin < nCount)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmNomoreYubao, tstUI);
			return;
		}
		int nBYb = ((CActor *)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_BIND_YUANBAO);
		unsigned int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBindYuanbaoLimit;
		if(nBYb + nCount > nMax)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmBindYuanbaoFull, tstUI);
			return;
		}
		((CActor *)m_pEntity)->ChangeMoney(mtYuanbao, -nCount, GameLog::Log_MoneyExChange, 0, "");
		((CActor *)m_pEntity)->ChangeMoney(mtBindYuanbao, nCount, GameLog::Log_MoneyExChange, 0, "");
	}
	
	
}


void CUserBag::HandleRecoverItem(CDataPacketReader &packet)
{
	int nType = 0;
	packet >> nType;
	bool isSuccess = true;
	
	int nRecoveCount = 0;
	if(nType >= 0) //一键
	{
		CUserItem::ItemSeries itemSrcGuid;			//源道具
		packet >> itemSrcGuid.llId ;
		char nNormal[1024] = {0};
		// int nBest = 0;
		packet.readString(nNormal);
		std::bitset<1024> nNormalId(nNormal);
		char nBest[1024] = {0};
		// int nBest = 0;
		packet.readString(nBest);
		std::bitset<1024> nBestId(nBest);
		// packet >> nBest ;
		int nCount = Inherited::count();
		// CUserItem** pItems = Inherited::operator CUserItem**();
		for(int i = nCount-1; i > -1; --i)
		{
			CUserItem* pItem = m_pUserItemList[i];

			if(!pItem || pItem->wCount <= 0)
				continue;
			const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pItem->wItemId);
			if(!pStdItem)
			{
				continue;
			}
			bool recoverBest = false;
			if(nType < 100 && strlen(pItem->cBestAttr) != 0 )
				continue;
			if (nType < 100 ) //锻造回收的逻辑
			{
				if(pStdItem->m_nItemlvl > nType)
					continue;
				//一键回收
				if(nType == 4 && !pStdItem->nRecycling )
					continue;
			}
			else if(nType >= 1000000) //一键回收
			{
				if(pStdItem->nItemlevel <= 0)
					continue;
				
				int localType = nType%1000000;
				recoverBest = true;
				// if(strlen(pItem->cBestAttr))
				// {
				// 	int nRes = nBestId[pStdItem->nItemlevel-1];//nBest & (1<<(pStdItem->nItemlevel-1));
				// 	if(nRes)
				// 		recoverBest = true;
				// }else
				{
					int nFlag = nNormalId[pStdItem->nItemlevel-1];//localType & (1<<(pStdItem->nItemlevel-1));
					if(!nFlag)
						continue;
				}
				
			}
			else
			{
				break;
			}

			if(strlen(pItem->cBestAttr) != 0 && !recoverBest)
				continue;
			
			if(!bagIsEnough(BagRecover))
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRecoverNoBagNum, tstUI);
				break;
			}
			isSuccess = RecoverItem(pItem, pStdItem,recoverBest);
			if(!isSuccess)
			{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemNotExist, tstUI);
			}
			nRecoveCount++;
		}
	}
	else
	{
		CUserItem::ItemSeries itemSrcGuid;			//源道具
		packet >> itemSrcGuid.llId ;
		
		CUserItem* pCuserItem = Inherited::FindItemByGuid(itemSrcGuid);
		if(!pCuserItem)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemNotExist, tstUI);
			return;
		}
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pCuserItem->wItemId);
		if(!pStdItem)
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmItemNotExist, tstUI);
			return;	
		}
		if(!bagIsEnough(BagRecover))
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmRecoverNoBagNum, tstUI);
			return;
		}
		isSuccess = RecoverItem(pCuserItem, pStdItem);
		nRecoveCount++;
	}
	// DealUserItem();
	((CActor*)m_pEntity)->GetAchieveSystem().ActorAchievementComplete(nAchieveRecover, nRecoveCount);
	((CActor*)m_pEntity)->GetQuestSystem()->OnQuestEvent(CQuestData::qtRecover, nRecoveCount);
	CActorPacket pack;
	CDataPacket& data = ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)(enBagSystemID) << (BYTE)enBagSystemsRecoverResult;
	data << isSuccess << (BYTE)nType;
	pack.flush();
}


bool CUserBag::RecoverItem(CUserItem* pItem, const CStdItem* pStdItem, bool IsreCoverBast)
{
	if(!pItem || !pStdItem)
	{
		return false;
	}
	if(pStdItem->m_nRecoverId)
	{
		int wCount = pItem->wCount;

		std::string nExtraAttr = pItem->cBestAttr;
		int nResult = DeleteItem(pItem, pItem->wCount, "self_del_item", GameLog::Log_Recover, true);
		if(nResult == 0 || nResult != wCount)
		{
			return false;
		}
		int nValue = 0;
		if(nExtraAttr.length() > 0)
		{
			std::vector<std::string> results = SplitStr(nExtraAttr, "|");
			int nCount = results.size();
			for (int i = 0; i < nCount; i++)
			{
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() >= 2)
				{
					if(atoi(temp_res[0].c_str()) == 5) //体力按照10点1次算
					{
						nValue += (atoi(temp_res[1].c_str()))/10;
					}else
					{
						nValue += atoi(temp_res[1].c_str());
					}
				}
			}
		}
		// int nRecoverPoint = std::max((nValue-5), 0)*10 +std::max((nValue-2), 0)*3+ nValue;
		// if(nRecoverPoint > 0)
		// {
		// 	((CActor*)m_pEntity)->GiveAward(qaIntegral, qaIntegral, nRecoverPoint,0,0,0,0,GameLog::Log_Recover);
		// }
		if(pItem->wStar > 0) {
			ItemUpStarCfg* cfg = GetLogicServer()->GetDataProvider()->GetUpStar().getItemUpStarCfg(pItem->wItemId, pItem->wStar);
			if(cfg) {
				for(auto it : cfg->recoverys) {
					((CActor*)m_pEntity)->GiveAward(it.btType, it.wId, it.wCount*wCount,0,0,0,0,GameLog::Log_Recover);
				}
			}
		}
		std::vector<DROPGOODS> info;
		GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(pStdItem->m_nRecoverId, info);
		int nNum = info.size();
		for(int i = 0; i < nNum; i++)
		{
			DROPGOODS& award = info[i];
			int notherGet = 0;
			notherGet = ((CActor*)m_pEntity)->GetMiscSystem().GetRecoverGetGold(award.info.nType, award.info.nId);
			if(notherGet > 0)
				notherGet = award.info.nCount*(notherGet/100.0);
			((CActor*)m_pEntity)->GiveAward(award.info.nType, award.info.nId, (award.info.nCount+notherGet)*wCount,0,0,0,0,GameLog::Log_Recover);
		}
	}

	return true;

}



void CUserBag::DealUserItem()
{
	int nCount = Inherited::count();
	// CUserItem** pItems = Inherited::operator CUserItem**();
	for(int i = nCount-1; i > -1; --i)
	{
		CUserItem* pItem = m_pUserItemList[i];
		if(!pItem)
			continue;
		
		printf("DealUserItem:%d\n",pItem->wCount);
	}
}

//暂时先这样处理 后续找到问题在还原
void CUserBag::DebugRevoceItem(int nType)
{
	int nCount = Inherited::count();
	// CUserItem** pItems = Inherited::operator CUserItem**();
	std::vector<uint64> recoveLists;
	recoveLists.clear();
	for(int i = nCount-1; i > -1; --i)
	{
		CUserItem* pItem = m_pUserItemList[i];

		if(!pItem || pItem->wCount <= 0)
			continue;
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pItem->wItemId);
		if(!pStdItem)
		{
			continue;
		}

		if(strlen(pItem->cBestAttr) != 0 )
			continue;
		if(pStdItem->m_nItemlvl > nType)
			continue;
		if(pStdItem->m_nRecoverId)
			recoveLists.push_back(pItem->series.llId);
	}

	if(recoveLists.size() > 0)
	{
		int nSize = recoveLists.size();
		for(int k = 0; k < nSize ; k++)
		{
			CUserItem::ItemSeries itemGuid;
			itemGuid.llId = recoveLists[k];
			CUserItem *pUserItem = Inherited::FindItemByGuid(itemGuid);
			if(pUserItem ==NULL)
			{
				continue;
			}
			const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
			if(!pStdItem)
			{
				continue;
			}
			RecoverItem(pUserItem, pStdItem);
		}
		recoveLists.clear();
	}
	// DealUserItem();
}


void CUserBag::OnEnterGame()
{
	if (((CActor*)m_pEntity)->m_isFirstLogin)
	{
		int nVocation = ((CActor*)m_pEntity)->GetProperty<unsigned>(PROP_ACTOR_VOCATION);
		std::vector<ItemAward>& itemList = GetLogicServer()->GetDataProvider()->GetFirstLoginAwards(nVocation);
		for (size_t i = 0; i < itemList.size(); i++)
		{
			ItemAward one = itemList[i];
			const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(one.nId);
			if(pStdItem && pStdItem->m_nPackageType == 0)
			{
				((CActor*)m_pEntity)->GetGameSetsSystem().AutoGameSetSkill(0, one.nId);
				((CActor*)m_pEntity)->GetGameSetsSystem().AutoGameSetSkillApp(0, one.nId); 
			}
			
			((CActor*)m_pEntity)->GiveAward(one.nType, one.nId, one.nCount,0,0,0,0,GameLog::Log_FirstLogin);
		}
	}
}



//跨服数据
void CUserBag::SendMsg2CrossServer(int nType)
{
	if(m_pEntity ==NULL) return;
	
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	packet << ((CActor*)m_pEntity)->GetCrossActorId();
	packet << ((CActor*)m_pEntity)->GetAccountID();

	size_t count = Inherited::count();
	packet  << (int)count  ; //玩家物品的数量
	for (UINT_PTR i=0; i<count; i ++ )
	{
		(CUserItem) *m_pUserItemList[i] >> packet;
	}
	pCrossClient->flushProtoPacket(packet);
}


VOID CUserBag::OnCrossInitData(std::vector<CUserItem>& vUseBags)
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载玩家装备的数据出错,实体指针为MULL"));
		return;
	}
	//物品的列表
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if(HasDbDataInit())
		{
			OutputMsg(rmError,_T("OnCrossInitData return bag item repeated,actorid=%d "),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
			return;
		}
		int nCount = vUseBags.size();
		// reader >> nCount;
		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
		OutputMsg(rmTip,_T("玩家 %s 装载背包数据count=%d"),m_pEntity->GetEntityName(), nCount);
		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem = vUseBags[i];
			if(availableCount(userItem.wPackageType))
			{
				CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
				if(pUerItem)
				{
					AddItem(pUerItem,m_pEntity->GetEntityName(),0,false, false); //不需要记录日志的，不需要通知客户端
				}
			}
			else
			{
				OutputMsg(rmError,_T("玩家%s的物品数据出现错误， 背包类型%d 不足"),m_pEntity->GetEntityName(), userItem.wPackageType);
				//数据错误需要查看什么问题
				return;
			}
		}
		((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CUSEBAG); //完成一个步骤
	    OnDbInitData(); //标记DB的数据初始化完毕
		return;
	}
}