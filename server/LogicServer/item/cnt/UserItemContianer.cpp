#include "StdAfx.h"
#include "UserItemContainer.h"

INT_PTR CUserItemContainer::AddItem(const ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, BYTE bNoBatchUse)
{
	if(param.wCount <= 0) return 0;	

	const CStdItem *pStdItem;
	INT_PTR Result = 0, nRemainCount, nDupedCount;
	
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	//通过物品ID查找物品配置对象
	pStdItem = stdItemProvider.GetStdItem(param.wItemId);
	if ( !pStdItem)//|| pStdItem->m_Flags.boDelete)
		return 0;
	//
	SendItemChangeLog(1, param.wCount, param.wItemId ,nLogIdent,lpSender);


	nRemainCount = param.wCount;
	//如果物品可以叠加，则尝试叠加到现有物品中
	if ( pStdItem->m_wDupCount > 1 )
	{
		//不断的尝试叠加到现有物品中，直到物品获取完成或不可继续叠加了
		while (nRemainCount > 0)
		{
			nDupedCount = OverlapToExists(param, nRemainCount, lpSender, nLogIdent, bNoBatchUse);
			if ( nDupedCount <= 0 )
				break;
			nRemainCount -= nDupedCount;
			Result += nDupedCount;		// 解决提示错误的bug
		}
	}
	
	//继续将未能叠加到已经存在的物品中的剩余物品作为新物品添加
	nDupedCount = __max(1, pStdItem->m_wDupCount);
	INT_PTR nAddCount ; //实际添加的数量
	CUserItem *pUserItem = NULL;
	while ( availableCount(pStdItem->m_nPackageType) > 0 && nRemainCount > 0 )
	{
		pUserItem = GetLogicServer()->GetLogicEngine()->AllocUserItem(true);
		pUserItem->wItemId			= param.wItemId;
		nAddCount					=  __min(nRemainCount, nDupedCount);	//实际添加的数量
		pUserItem->wCount			= (WORD)nAddCount;						//实际添加的数量 
		pUserItem->btFlag			= param.btBindFlag;
		pUserItem->btQuality		= param.btQuality;
		pUserItem->wStar			= param.wStar;							//强化星级
		pUserItem->bLostStar		= param.bLostStar;						//强化损失星级
		pUserItem->bInSourceType	= param.bInSourceType;					//铭刻等级
		pUserItem->nAreaId		= param.nAreaId;					//来源场景
		pUserItem->btLuck			= param.btLuck;							//幸运
		pUserItem->wIdentifySlotNum = param.wIdentifyslotnum;				//鉴定槽数量				
		CStdItem::AssignInstance  (pUserItem,pStdItem);
         //售卖类箱子道具时间设置
		//  if(pStdItem->m_btType == Item::itSellBox)
		//  {
		//       if(pStdItem->m_UseTime.tv > 0)
		// 	  {  
		// 		  pUserItem->time = pStdItem->m_UseTime.tv;
		// 		  pUserItem->time.startRecord(GetLogicServer()->GetLogicEngine()->getMiniDateTime());
		// 	  }
		//  }
		// if (param.nLeftTime > 0)
		// {
		// 	pUserItem->nCreatetime = param.nLeftTime;
		// 	pUserItem->nCreatetime.startRecord(GetLogicServer()->GetLogicEngine()->getMiniDateTime());
		// }
		memcpy(pUserItem->smithAttrs, param.nSmith, sizeof(param.nSmith));
		memcpy(pUserItem->cBestAttr, param.cBestAttr, sizeof(param.cBestAttr));
		if(!param.bInSourceType)
			pUserItem->setSource(CStdItem::iqOther,GetLogicServer()->GetLogicEngine()->getMiniDateTime());
		else
		{
			pUserItem->setSource(param.bInSourceType,param.nCreatetime,param.nAreaId,param.nDropMonsterId,param.cSourceName);
		}
		
		pUserItem->wPackageType = pStdItem->m_nPackageType;
		//将物品添加到容器中
		m_pUserItemList.push_back(pUserItem);
		addCostCount(pUserItem->wPackageType);
		//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的数据
		// if ( nLogIdent )
		// {
		// 	LogNewItem(pUserItem, pStdItem, lpSender, nLogIdent);
		// }
		//向角色发送获得物品的消息
		SendAddItem(pUserItem,nLogIdent, bNoBatchUse);
		//减少给予物品的数量并增加已获得的物品数量
		nRemainCount -= (nAddCount); //减去实际添加的数量
		Result += nAddCount; //结果添加实际的数量

	}
	
	OnAddItemById(param.wItemId, (int)Result , nLogIdent, pUserItem);  // 修改添加物品如果叠放了提示不对的bug
	OnDataModified();

	//统计产出
	GetLogicServer()->GetDataProvider()->GetStdItemProvider().AddItemOutPutCount(param.wItemId,param.wCount,nLogIdent);
	return Result;
}

INT_PTR CUserItemContainer::GetAddItemNeedGridCount(const ItemOPParam& param)
{	
	// INT_PTR  nRemainCount, nDupedCount;
	int nRemainCount = param.wCount;//装备的数量
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	//通过物品ID查找物品配置对象
	const CStdItem * pStdItem= stdItemProvider.GetStdItem(param.wItemId);
	if ( !pStdItem )
		return nRemainCount;
	
	int nDupedCount = pStdItem->m_wDupCount; //能够叠加的数量
	if(nDupedCount > 1)
	{
		int value = nRemainCount % nDupedCount;
		nRemainCount = nRemainCount / nDupedCount;
		if(value > 0)
			nRemainCount++;
	}
	{
		return nRemainCount; //如果不能叠加直接返回物品的个数
	}
}

//能否添加一个物品
bool CUserItemContainer::CanAddItem(const ItemOPParam& param, bool bNeedCkeckCell)
{
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	const CStdItem * pStdItem= stdItemProvider.GetStdItem(param.wItemId);
	if ( !pStdItem )
		return false; //不合法的物品
	int nNum = 0;
	if (bNeedCkeckCell && pStdItem->m_nPackageType >= 1) 
		nNum = 10;
	return (availableCount(pStdItem->m_nPackageType)-nNum) >= GetAddItemNeedGridCount(param) ;
}

bool CUserItemContainer::CanAddItem( CUserItem * pUserItem, bool bNeedCkeckCell)
{
	if(pUserItem ==NULL) return false;
	ItemOPParam param;
	param.wItemId =pUserItem->wItemId;
	param.wCount =pUserItem->wCount;
	param.btQuality = pUserItem->btQuality;
	param.btBindFlag =pUserItem->btFlag;
	param.btStrong = pUserItem->btStrong;
	return CanAddItem(param, bNeedCkeckCell);
}

//能否添加一批物品
bool  CUserItemContainer::CanAddItems( CVector<ItemOPParam> &itemList)
{
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	INT_PTR nGridCount = availableMinCount(); //当前的格子的数量
	const CStdItem * pStdItem;
	for(INT_PTR i=0; i< itemList.count(); i++)
	{
		pStdItem = stdItemProvider.GetStdItem(itemList[i].wItemId);
		if ( !pStdItem )
			return false; //不合法的物品
		nGridCount -= GetAddItemNeedGridCount(itemList[i]);
		if(nGridCount < 0) return false;
	}
	return true; 
}

bool CUserItemContainer::SplitItem(const CUserItem::ItemSeries series,INT_PTR nCount)
{
	CUserItem* pUserItem = FindItemByGuid(series);
	if( pUserItem==NULL) return false; //找不到这个物品
	int nsubcount = 0;
	if(pUserItem->wPackageType >= 1 )
		nsubcount = 10;
	if((availableCount(pUserItem->wPackageType)-nsubcount) < 1) return false; //没有剩下格子
	OnDataModified();
	INT_PTR nLeftCount = pUserItem->wCount - nCount;
	//无法拆分出这么多个
	if(nLeftCount < 1) return false;
	CUserItem* pNewUserItem = GetLogicServer()->GetLogicEngine()->AllocUserItem(true); //新的物品
	if(pNewUserItem ==NULL) return false;
	
	CUserItem::ItemSeries newGuid = pNewUserItem->series; //保存序列号
	memcpy(pNewUserItem,pUserItem,sizeof(*pUserItem)); //先拷贝过去
	pNewUserItem->series = newGuid;  //序列号是新的
	pNewUserItem->wCount = (WORD)nCount; //新的数量
	pUserItem->wCount = (WORD) nLeftCount; // 剩下的数量
	SendItemCountChange(pUserItem);  //通知客户端发生改变
	m_pUserItemList.push_back(pNewUserItem);
	addCostCount(pUserItem->wPackageType);
	SendAddItem(pNewUserItem,GameLog::clEquipSplit); //通知剩下的数量发生改变
	
	return true;
}

/*
道具合并，
bBindInfect：是否绑定感染，
绑定感染，如果源目标是绑定的，则目标也会变成绑定的
*/
bool CUserItemContainer::MergeItem(const CUserItem::ItemSeries srcSeries,const CUserItem::ItemSeries tgtSeries, bool bBindInfect)
{
	CUserItem* pSrcUserItem = FindItemByGuid(srcSeries);
	CUserItem* pTgtUserItem = FindItemByGuid(tgtSeries);
	if(pSrcUserItem ==NULL || pTgtUserItem ==NULL ) return false;
	if(pSrcUserItem->wItemId != pTgtUserItem->wItemId) return false;
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pTgtUserItem->wItemId);
	
	ItemOPParam param;
	param.wItemId = pSrcUserItem->wItemId;
	param.btQuality = pSrcUserItem->btQuality;
	param.btStrong = pSrcUserItem->btStrong;
	param.wCount = pSrcUserItem->wCount;
	param.btBindFlag = pSrcUserItem->btFlag;

	// 修改bug！ 只判断目标能否叠加会有问题，有时间限制的物品往没有时间限制的物品上叠加会成功。 2011.12。8 by 王鹏
	//能够叠加的数量
	INT_PTR nOverLapCount= __min( pSrcUserItem->wCount, GetCanOverlapCountEx(pSrcUserItem, pTgtUserItem,bBindInfect) ); 	
	if( nOverLapCount == 0 ) //叠加的数量为0
	{
		return false;
	}
	else
	{		

		INT_PTR nTotalCount =  pSrcUserItem->wCount + pTgtUserItem->wCount; //开始叠加前的数量
		INT_PTR nTargetCount = pTgtUserItem->wCount + nOverLapCount; //目标的数量
		
		INT_PTR nSrcCount =  pSrcUserItem->wCount - nOverLapCount;
		
		if( nTargetCount > pStdItem->m_wDupCount || nSrcCount <0) //计算错误了
		{
			return false; 
		}
		pTgtUserItem->wCount = (WORD)nTargetCount;
		if( bBindInfect && pSrcUserItem->binded() )		//需要绑定感染
		{
			pTgtUserItem->btFlag = pSrcUserItem->btFlag;	//如果源目标是绑定的，则目标也变成绑定的
		}

		if( bBindInfect )
		{
			NotifyItemChange(pTgtUserItem);		//通知客户端目标道具数据发生改变
		}
		else
		{
			SendItemCountChange(pTgtUserItem);  //通知客户端发生改变
		}

		if(nSrcCount == 0)
		{
			RemoveItem(pSrcUserItem->series,"item_merge",GameLog::Log_ItemMerge,true); //删除这个物品
		}
		else
		{
			pSrcUserItem->wCount = (WORD)nSrcCount ;
			if( bBindInfect )
			{
				NotifyItemChange(pSrcUserItem);		//通知客户端目标道具数据发生改变
			}
			else
			{
				SendItemCountChange(pSrcUserItem);  //通知客户端发生改变
			}
		}
		OnDataModified();
		return true;
	}
}

INT_PTR CUserItemContainer::AddItem(CUserItem *pUserItem, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedSendClient,bool bDupCount,BYTE bNotice)
{
	// 优先叠放
	if (!pUserItem)
	{
		return 0;
	}
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem((int)pUserItem->wItemId);
	if (!pStdItem)
		return 0;
	if (pUserItem->wCount <= 0)
	{
		OutputMsg(rmError,"[%s] remove item count=0,logid=%d,itemid=%d",__FUNCTION__,(int)nLogIdent,(int)pUserItem->wItemId);
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
		return 0;
	}
	//如果已经找到了，那说明出现了复制则不添加进去
	if (FindItemByGuid(pUserItem->series))
	{
		OutputMsg(rmWaning,_T("Copy Item is Coming...guid(%lld),LogId(%d)"),pUserItem->series.llId,(int)nLogIdent);
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
		return 0;
	}
	//
	SendItemChangeLog(1, pUserItem->wCount, pUserItem->wItemId ,nLogIdent,lpSender);

	OnDataModified();
	INT_PTR nAddCount =0; //实际上添加的个数

	INT_PTR nCount		= pUserItem->wCount;
	INT_PTR nStackCount = pStdItem->m_wDupCount;
	if (bDupCount && nStackCount > 0 && nCount != nStackCount)
	{
		INT_PTR nStackedCount = 0;
		ItemOPParam param;
		param.wItemId     = pUserItem->wItemId;
		param.wCount      = pUserItem->wCount;
		param.btBindFlag  = pUserItem->btFlag;		
		param.btQuality	  = pUserItem->btQuality;
		param.wStar 	  = pUserItem->wStar;
		nStackedCount	  = OverlapToExists(param, param.wCount, lpSender, nLogIdent);	
		nAddCount += nStackedCount;

		if (nCount <= nStackedCount)		// 全部堆叠完毕
		{
			pUserItem->wCount -= (WORD)nStackedCount;
			OnAddItemById(pUserItem->wItemId, nCount, nLogIdent, pUserItem);
			if(pUserItem->wCount <= 0)
			{
				GetGlobalLogicEngine()->DestroyUserItem(pUserItem); //数目已经叠加好了，删除指针
			}
			return nAddCount;
		}
		pUserItem->wCount -= (WORD)nStackedCount;
	}
 
	// 不能叠放的，用新格子
	if(availableCount(pStdItem->m_nPackageType) <=0) 
	{
		OutputMsg(rmWaning," CUserItemContainer::AddItem has no grid,itemid=%d,nLogIdent=%d",(int)pUserItem->wItemId,(int)nLogIdent);
		return nAddCount; //空间不够
	}
	if(pUserItem->wCount <=0)
	{
		OutputMsg(rmWaning," CUserItemContainer::AddItem count=0,itemid=%d,nLogIdent=%d",(int)pUserItem->wItemId,(int)nLogIdent);
		return nAddCount; //空间不够
	}
	pUserItem->wPackageType = pStdItem->m_nPackageType;
	m_pUserItemList.push_back(pUserItem);
	addCostCount(pUserItem->wPackageType);
	//如果物品需要记录日志，则向日志服务器记录日志
	if ( pStdItem && nLogIdent ) //没有nLogIdent 可能就是db返回的
	{
		LogNewItem(pUserItem, pStdItem, lpSender, nLogIdent);
	}
	if(bNeedSendClient )
	{
		OnAddItem(pStdItem,pUserItem->wCount,nLogIdent,pUserItem);
		SendAddItem(pUserItem,nLogIdent,0 ,bNotice);
	}
	 
	return (pUserItem->wCount + nAddCount); //添加的个数为已经添加的个数以及自己的个数之和 
}

void CUserItemContainer::DeleteAllItem()
{
	CUserItem **pUserItemList, *pUserItem;

	// pUserItemList = *this;
	for (INT_PTR i=count()-1; i>-1; --i)
	{
		pUserItem = m_pUserItemList[i];
		if(pUserItem ==NULL ) continue;
		DeleteItem(pUserItem,pUserItem->wCount,"self_del_item",GameLog::clUserDestroyItem,true);
	}
}

INT_PTR CUserItemContainer::DeleteItem(const ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, const int nDuraOdds/*, CActor* pActor*/)
{
	INT_PTR Result = 0, nCount = param.wCount;
	CUserItem **pUserItemList, *pUserItem;
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(param.wItemId);

	// pUserItemList = *this;
	for (INT_PTR i=count()-1; i>-1; --i)
	{
		pUserItem = m_pUserItemList[i];
		if(pUserItem ==NULL ) continue;
		if ( pUserItem->wItemId == param.wItemId )
		{
			if ( pUserItem->btQuality != param.btQuality && param.btQuality != (BYTE)-1 && param.btQuality != 255)
				continue;
			if ( pUserItem->btStrong != param.btStrong && param.btStrong != (BYTE)-1 )
				continue;

			if(param.btBindFlag != -1 && param.btBindFlag != 255)
			{
				if( (param.btBindFlag && 0==pUserItem->btFlag) || (param.btBindFlag==0 && pUserItem->btFlag) ) 
				{
					continue;
				}
			}

			// if ((pUserItem->wDuraMax - pUserItem->wDura) > nDuraOdds && nDuraOdds != -1)
			// 	continue;

			INT_PTR nDelCount = DeleteItem(pUserItem,nCount,lpSender,nLogIdent);
			Result += nDelCount;
			nCount -= nDelCount; 
			if(nCount  <=0 ) //已经全部删除完毕了
			{
				break;
			}
		}
	}
	OnDataModified();
	return Result;
}

INT_PTR  CUserItemContainer::DeleteOtherItem(CUserItem * pUserItem,INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory,bool bReqSB )
{
	if (pUserItem == NULL) return 0;

	INT_PTR Result = 0;
	CUserItem **pUserItemList, *pSelfItem;
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);

	// pUserItemList = *this;
	for (INT_PTR i = count()-1; i > -1; --i)
	{
		pSelfItem = m_pUserItemList[i];
		if (pSelfItem == NULL) continue;
		if ( pSelfItem->wItemId == pUserItem->wItemId )
		{
			if ( pSelfItem->btQuality != pUserItem->btQuality && pUserItem->btQuality != (BYTE)-1)
				continue;
			if (bReqSB)
			{
				if ( pSelfItem->btStrong != pUserItem->btStrong && pUserItem->btStrong != (BYTE)-1 )
					continue;

				if (pUserItem->btFlag != -1 && pUserItem->btFlag != 255)
				{
					if( (pUserItem->btFlag && 0 == pSelfItem->btFlag) || (pUserItem->btFlag == 0 && pSelfItem->btFlag) ) 
					{
						continue;
					}
				}
			}
			

			//如果为指定的物品 则排除
			if (pUserItem->series == pSelfItem->series)
			{
				continue;
			}

			INT_PTR nDelCount = DeleteItem(pSelfItem,nCount,lpSender,nLogIdent);
			Result += nDelCount;
			nCount -= nDelCount; 
			if(nCount <= 0 ) //已经全部删除完毕了
			{
				break;
			}
		}
	}
	OnDataModified();
	return Result;
}

	

INT_PTR  CUserItemContainer::DeleteItem(CUserItem * pUserItem,INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory )
{
	INT_PTR Result = 0;
	if(pUserItem == NULL) return 0;
	//修改一个bug，在背包里删除一个物品需要判断是否在背包里存在
	//如果查找到这个物品不在这个容器里，需要删除
	CUserItem * pSelfItem = FindItemByGuid(pUserItem->series);
	if(pSelfItem == NULL || pSelfItem != pUserItem)
	{
		return 0;
	}
	
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return 0;
	//
	if ( pUserItem->wCount > nCount )
	{
		pUserItem->wCount -= (WORD)nCount;
		Result += nCount;
		//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的数据
		// if ( pStdItem /*&& pStdItem->m_Flags.recordLog*/ )
		// {
		// 	LogItemCountChange(pUserItem, pStdItem, -nCount, lpSender, nLogIdent);
		// }
		//
		//向角色发送删物品数量变更的消息
		SendItemChangeLog(2, nCount, pUserItem->wItemId ,nLogIdent,lpSender);
		SendItemCountChange(pUserItem);
		
	}
	else
	{
		INT_PTR nItemCount = pUserItem->wCount;
		Result += nItemCount;

		//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的数据
		// if ( pStdItem /*&& pStdItem->m_Flags.recordLog*/)
		// {
		// 	LogItemCountChange(pUserItem, pStdItem, -nItemCount, lpSender, nLogIdent);
		// }

		int nCount = pUserItem->wCount;
		pUserItem->wCount = 0;//将物品数量归零以便正确的记录日志
		RemoveItem(pUserItem->series,lpSender,nLogIdent,bNeedFreeMemory,nCount);
	}


	//OnDeleteItem(pStdItem,Result,nLogIdent, 0,pUserItem);
	OnDataModified();
	return Result;
}

bool CUserItemContainer::RemoveItem(const CUserItem::ItemSeries series, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedFreeMemory, int nCount )
{	INT_PTR nIdx = FindIndex(series);
	if ( nIdx > -1 )
	{
		return RemoveItem(nIdx,lpSender,nLogIdent,bNeedFreeMemory, nCount);
	}
	return false;
}

bool CUserItemContainer::RemoveItem(const INT_PTR nIndex, LPCSTR lpSender,INT_PTR nLogIdent,bool bNeedFreeMemory, int nCount)
{
	if ( (nIndex > -1) && (nIndex < m_pUserItemList.size()))
	{
		CUserItem *pUserItem = m_pUserItemList[nIndex];
		if(pUserItem ==NULL) return false;
		
		SendDeleteItem(pUserItem,nLogIdent); //通知客户端删除

		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if(pStdItem ==NULL) return false;

		//OnDeleteItem(pStdItem,pUserItem->wCount,nLogIdent, nMsgId, pUserItem);

		//
		if(pUserItem->wCount > nCount)
			nCount = pUserItem->wCount;
		SendItemChangeLog(2, nCount, pUserItem->wItemId ,nLogIdent,lpSender);


		if(nLogIdent != GameLog::Log_Deport2Bag && nLogIdent != GameLog::Log_DepotMerge)
			addCostCount(pUserItem->wPackageType, -1);
		if(bNeedFreeMemory)
		{
			GetGlobalLogicEngine()->DestroyUserItem(pUserItem); //销毁这个物品的内存，可以重用
		}
		m_pUserItemList.erase(m_pUserItemList.begin()+nIndex); //背包格子可以重用
		
		OnDataModified();
		return true;
	}
	return false;
}

CUserItem* CUserItemContainer::FindItem(const INT_PTR wItemId, const INT_PTR nQuality /* = -1 */, const INT_PTR nStrong /* = -1 */, WORD nMinCount)
{
	CUserItem **pUserItemList, *pUserItem;
	// pItemList = *this;
	for (INT_PTR i=count()-1; i>-1; --i)
	{
		pUserItem = m_pUserItemList[i];
		if(pUserItem ==NULL ) continue;
		if ( pUserItem->wItemId == wItemId )
		{
			if ( pUserItem->wCount < nMinCount )
				continue;
			if ( pUserItem->btQuality != nQuality && nQuality != -1)
				continue;
			if ( pUserItem->wStar != nStrong && nStrong != -1 )
				continue;
			
			return pUserItem;
		}
	}
	return NULL;
}

CUserItem* CUserItemContainer::FindItemByGuid(const CUserItem::ItemSeries series   )
{
	CUserItem **pItemList, *Result = NULL;
	//pUserItemList = *this;
	int count1 = count();
	for (INT_PTR i = count1 - 1; i > -1; --i)
	{
		if ( m_pUserItemList[i] 
			&& m_pUserItemList[i]->series.llId == series.llId )
		{
			Result = m_pUserItemList[i];
			break; 
		}
	}
	return Result;
}

INT_PTR CUserItemContainer::FindIndex(const CUserItem::ItemSeries series)
{
	//CUserItem **pItemList = *this;
	for (INT_PTR i= count()-1; i>-1; --i)
	{
		if (  m_pUserItemList[i] && m_pUserItemList[i]->series.llId == series.llId )
			return i; 
	}
	return -1;
}

INT_PTR CUserItemContainer::GetItemCount(const INT_PTR wItemId, const INT_PTR nQuality , const INT_PTR nStrong ,const INT_PTR nBind, const INT_PTR nDuraOdds,void * pNoItem)
{
	INT_PTR Result = 0;
	CUserItem **pUserItemList, *pUserItem, *pNoGetItem = NULL;
	// pUserItemList = *this;
	int size = count();
	if (pNoItem != NULL)
	{
		pNoGetItem = (CUserItem*)pNoItem;
	}
	for (INT_PTR i=size-1; i>-1; --i)
	{
		pUserItem = m_pUserItemList[i];
		if (pUserItem && pUserItem->wItemId == wItemId )
		{
			if ( pUserItem->btQuality != nQuality && nQuality != -1  && nQuality != 255)
				continue;
			if ( pUserItem->btStrong != nStrong && nStrong != -1 && nStrong != 255)
				continue;
			//判断绑定属性是否一致
			if(nBind != -1 && nBind != 255)
			{
				if( (nBind && 0==pUserItem->btFlag) || (nBind==0 && pUserItem->btFlag) ) 
				{
					continue;
				}
				if(nBind > 0 &&  pUserItem->btFlag != nBind)
				{
				     continue;
				}
			}
			
			// if ((pUserItem->wDuraMax - pUserItem->wDura) > nDuraOdds && nDuraOdds != -1)
			// 	continue;

			if (pNoGetItem != NULL && pNoGetItem == pUserItem)
			{
				continue;
			}
			Result += pUserItem->wCount;
		}
	}
	return Result;
}

INT_PTR CUserItemContainer::Clear(LPCSTR lpSender /* = NULL */, const INT_PTR nLogIdent /* = 0 */)
{
	INT_PTR i, nCount;
	const CStdItem *pStdItem;
	const CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	//CUserItem **pItemList = *this;

	for (i=count()-1; i>-1; --i)
	{
		if(m_pUserItemList[i]) {
			//记录日志并向角色发送数据
			if ( nLogIdent )
			{
				nCount = m_pUserItemList[i]->wCount;
				m_pUserItemList[i]->wCount = 0;
				pStdItem = stdItemProvider.GetStdItem(m_pUserItemList[i]->wItemId);
				// if ( pStdItem /*&& pStdItem->m_Flags.recordLog*/)
				// 	LogItemCountChange(m_pUserItemList[i], pStdItem, -nCount, lpSender, nLogIdent);
				SendDeleteItem(m_pUserItemList[i],nLogIdent);
			}
			//释放物品的内存
			GetLogicServer()->GetLogicEngine()->DestroyUserItem(m_pUserItemList[i]);
		}
	}
	nCount = count();
	//Inherited::clear();
	m_pUserItemList.clear();
	OnDataModified();
	return nCount;
}

/*
计算道具叠加数量
bBindInfect:允许绑定道具与非绑定道具之间的合并
*/
INT_PTR CUserItemContainer::GetCanOverlapCountEx(const CUserItem *pSrcItem, const CUserItem *pDestItem, bool bBindInfect)
{	
	if (!pSrcItem || !pDestItem || pSrcItem->wItemId != pDestItem->wItemId)
		return 0;
	
	// if( pSrcItem->btQuality != pDestItem->btQuality || 
	// 	pSrcItem->btStrong != pDestItem->btStrong ||
	// 	pSrcItem->time.tv != pDestItem->time.tv
	//   )
	// 	return 0;

	// if( !bBindInfect )	//如果不需要绑定感染，就要判断两方道具是否同为绑定或者不绑定
	// {
	// 	if( pSrcItem->btFlag  != pDestItem->btFlag )
	// 	{
	// 		return 0;
	// 	}
	// }

	CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	const CStdItem *pStdItem = itemProvider.GetStdItem(pDestItem->wItemId);
	if (!pStdItem)
		return 0;

	INT_PTR nDupSpace = pStdItem->m_wDupCount - pDestItem->wCount;
	if(nDupSpace <=0)
	{
		return 0;
	}
	else
	{
		return nDupSpace;
	}
}

INT_PTR CUserItemContainer::GetCanOverlagCount(const ItemOPParam& param, const CUserItem * pUserItem)
{
	//物品能够堆叠的条件为：
	//1、物品ID相同
	//2、物品品质相同
	//3、物品强化等级相同
	//4、物品绑定标志位相同
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(param.wItemId);
	if ( !pStdItem || !pUserItem )
		return 0;
	if ( pUserItem->wItemId == param.wItemId && pUserItem->btQuality == param.btQuality 
		&& pUserItem->wStar == param.wStar
		&& (pUserItem->btFlag == param.btBindFlag)		
		)
	{
		INT_PTR nDupSpace = pStdItem->m_wDupCount - pUserItem->wCount;
		if(nDupSpace <=0)
		{
			return 0;
		}
		else
		{
			return nDupSpace;
		}
	}
	else
	{
		return 0;
	}
}

INT_PTR CUserItemContainer::OverlapToExists(const ItemOPParam& param, INT_PTR nCount, LPCSTR lpSender, const INT_PTR nLogIdent, BYTE bNoBatchUse)
{
	INT_PTR Result = 0;
	CUserItem **pUserItemList, *pUserItem;
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(param.wItemId);
	if ( !pStdItem )
		return 0;

	// pUserItemList = *this;
	for (INT_PTR i=count()-1; i>-1 && nCount > 0; --i)
	{
		pUserItem = m_pUserItemList[i];
		if(NULL == pUserItem ) continue;
		INT_PTR nDupSpace = GetCanOverlagCount(param,pUserItem);
		if ( nDupSpace > 0 )
		{
			if ( nDupSpace > nCount )
				nDupSpace = nCount;
			pUserItem->wCount += (WORD)nDupSpace;
			nCount -= nDupSpace;
			Result += nDupSpace;
			//如果物品需要记录流通日志，则需要向日志服务器发送记录日志的消息
			if (nLogIdent )
			{
				LogItemCountChange(pUserItem, pStdItem, nDupSpace, lpSender, nLogIdent);
			}
			//向角色发送物品数量变更的消息
			SendItemCountChange(pUserItem,true, bNoBatchUse);
		}
	}
	return Result;
}
void CUserItemContainer::ArrangeItemList(int nLogId)
{
	if (count() <= 1)
	{
		return;
	}
	int i = 0;
	CUserItem **pUserItemList, *pItemDest, *pItemSour;
	// pUserItemList = *this;
	while ( (i < count()) && (i >= 0) )
	{
		pItemDest = m_pUserItemList[i];
		if ( !pItemDest )
		{
			return;
		}
		const CStdItem *pStdItemEx = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pItemDest->wItemId);
		if (!pStdItemEx || pItemDest->wCount >= pStdItemEx->m_wDupCount )
		{
			i++;
			continue;
		}
		int maxCount = pStdItemEx->m_wDupCount;
		int j = i + 1;
		while ( (pItemDest->wCount < maxCount) && ( j < count() ))
		{
			pItemSour = m_pUserItemList[j];
			if ( pItemSour && pItemSour->wItemId == pItemDest->wItemId)
			{
				int dupCount = __min(maxCount - pItemDest->wCount, pItemSour->wCount);
				pItemDest->wCount += dupCount; 
				SendItemCountChange(pItemDest);
				if (dupCount >= pItemSour->wCount)
				{
					RemoveItem(pItemSour->series,"item_arrange",nLogId,true); 
				}
				else
				{
					pItemSour->wCount -= dupCount;
					SendItemCountChange(pItemSour);
					break;
				}
			}
			else
			{
				j++;
			}
		}
		i++;
	}
}



/*
剩余格数
*/
INT_PTR  CUserItemContainer::availableCount(WORD type)//type 从0开始
{
	//int maxBagsNum = GetLogicServer()->GetDataProvider()->GetBagConfig().nMaxBagCount;
	
	if(type > m_nBagNums -1)
	{
		return 0;
	}

	int result = m_BagMaxCounts[type] - m_BagCostCounts[type];

	return result < 0?0:result;
}


/*
获取所有类型背包最小的格数-----刘老板要求先这样做  一个背包满了 所有的物品都不能添加到背包
*/
INT_PTR  CUserItemContainer::availableMinCount()
{

	std::vector<int> results;
	for(int i = 0; i< m_nBagNums; i++)
	{
		int left = m_BagMaxCounts[i] - m_BagCostCounts[i];
		
		results.push_back(left < 0?0:left);
	}
	sort(results.begin(), results.end());
	return results[0];
}

//BagType
bool CUserItemContainer::bagIsEnough(int nType, int nNum)
{
	std::vector<int>& count = GetLogicServer()->GetDataProvider()->getBagRemainConfig(nType);
	
	for(int i = 0; i< m_nBagNums; i++)
	{
		if( m_BagMaxCounts[i] - m_BagCostCounts[i] < count[i] + nNum)
			return false;
	}
	return true;
}


bool  CUserItemContainer::bagLeftEnoughGiveNum(int num)
{

	return availableMinCount() >= num?true:false;
}


/*
使用格数
*/
void CUserItemContainer::addCostCount(WORD type, int nNum)//type 从0开始
{

	if(type > m_nBagNums -1)
	{
		return ;
	}
	m_BagCostCounts[type] += nNum;

	if(m_BagCostCounts[type] < 0) //删除
		m_BagCostCounts[type] = 0;
}


int CUserItemContainer::GetBagCount(WORD type)//type 从0开始
{

	if(type > m_nBagNums -1)
	{
		return 0;
	}

	return m_BagMaxCounts[type];
}


INT_PTR CUserItemContainer::AddItem2depot(CUserItem *pUserItem, LPCSTR lpSender, const INT_PTR nLogIdent,bool bNeedSendClient,bool bDupCount,BYTE bNotice)
{
	// 优先叠放
	if (!pUserItem)
	{
		return 0;
	}
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem((int)pUserItem->wItemId);
	if (!pStdItem)
		return 0;
	if (pUserItem->wCount <= 0)
	{
		OutputMsg(rmError,"[%s] remove item count=0,logid=%d,itemid=%d",__FUNCTION__,(int)nLogIdent,(int)pUserItem->wItemId);
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
		return 0;
	}
	//如果已经找到了，那说明出现了复制则不添加进去
	if (FindItemByGuid(pUserItem->series))
	{
		OutputMsg(rmWaning,_T("Copy Item is Coming...guid(%lld),LogId(%d)"),pUserItem->series.llId,(int)nLogIdent);
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
		return 0;
	}
	//
	// SendItemChangeLog(1, pUserItem->wCount, pUserItem->wItemId ,nLogIdent);

	OnDataModified();
	INT_PTR nAddCount =0; //实际上添加的个数

	INT_PTR nCount		= pUserItem->wCount;
	INT_PTR nStackCount = pStdItem->m_wDupCount;
	// if (bDupCount && nStackCount > 0 && nCount != nStackCount)
	// {
	// 	INT_PTR nStackedCount = 0;
	// 	ItemOPParam param;
	// 	param.wItemId     = pUserItem->wItemId;
	// 	param.wCount      = pUserItem->wCount;
	// 	param.btBindFlag  = pUserItem->btFlag;		
	// 	param.btQuality	  = pUserItem->btQuality;
	// 	param.wStar 	  = pUserItem->wStar;
	// 	nStackedCount	  = OverlapToExists(param, param.wCount, lpSender, nLogIdent);	
	// 	nAddCount += nStackedCount;

	// 	if (nCount <= nStackedCount)		// 全部堆叠完毕
	// 	{
	// 		pUserItem->wCount -= (WORD)nStackedCount;
	// 		OnAddItemById(pUserItem->wItemId, nCount, nLogIdent, pUserItem);
	// 		if(pUserItem->wCount <=0)
	// 		{
	// 			GetGlobalLogicEngine()->DestroyUserItem(pUserItem); //数目已经叠加好了，删除指针
	// 		}
	// 		return nAddCount;
	// 	}
	// 	pUserItem->wCount -= (WORD)nStackedCount;
	// }


	// 不能叠放的，用新格子
	if(availableCount() <=0) 
	{
		OutputMsg(rmWaning," CUserItemContainer::AddItem has no grid,itemid=%d,nLogIdent=%d",(int)pUserItem->wItemId,(int)nLogIdent);
		return nAddCount; //空间不够
	}
	if(pUserItem->wCount <=0)
	{
		OutputMsg(rmWaning," CUserItemContainer::AddItem count=0,itemid=%d,nLogIdent=%d",(int)pUserItem->wItemId,(int)nLogIdent);
		return nAddCount; //空间不够
	}
	pUserItem->wPackageType = pStdItem->m_nPackageType;
	m_pUserItemList.push_back(pUserItem);
	//如果物品需要记录日志，则向日志服务器记录日志
	// if ( pStdItem && nLogIdent ) //没有nLogIdent 可能就是db返回的
	// {
	// 	LogNewItem(pUserItem, pStdItem, lpSender, nLogIdent);
	// }
	if(bNeedSendClient )
	{
		OnAddItem(pStdItem,pUserItem->wCount,nLogIdent,pUserItem);
		SendAddItem(pUserItem,nLogIdent,0 ,bNotice);
	}
	
	
	return (pUserItem->wCount + nAddCount); //添加的个数为已经添加的个数以及自己的个数之和 
}
