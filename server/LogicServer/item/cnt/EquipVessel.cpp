#include "StdAfx.h"
#include<algorithm>
#include "EquipVessel.h"
#include "../../base/Container.hpp" 
#include "script/interface/SystemParamDef.h"

#define  BASE_EQUIP_COUNT 10 //基本装备数(目前无勋章)
#define  RING_BANGLE_COUNT 2
INT_PTR  CEquipVessel::GetPetTakeItemErrorCode(const HERODBDATA*pData, const CStdItem* pStdItem, const CUserItem *pUserItem,CAnimal *pMaster)
{
	if(pData ==NULL || pStdItem ==NULL) return tpItemNotExist;
	const CStdItem::ItemUseCondition *pCond = pStdItem->m_Conditions.pConds;
	unsigned int nReqValue;
	if (pUserItem == NULL) return tpItemNotExist;

	for (INT_PTR i=pStdItem->m_Conditions.nCount-1; i>-1; --i)
	{
		switch(pCond->btCond)
		{
		case CStdItem::ItemUseCondition::ucLevel: //等级必须大于等于value级
			nReqValue = (unsigned int)pCond->nValue;
			//转数判断

			if ((unsigned int)pData->bLevel < (unsigned int)nReqValue) 
				return tpItemCannotTakeOnForLevel;

			break;

		case CStdItem::ItemUseCondition::ucGender: //性别编号必须等于value，0男1女
			if ((unsigned int)pData->bSex != (unsigned int)pCond->nValue) return tpItemCannotTakeOnForSex;
			break;
		case CStdItem::ItemUseCondition::ucJob: //职业编号必须等于value
			if ((unsigned int)pData->bVocation != (unsigned int)pCond->nValue && pCond->nValue != 0) return tpItemCannotTakeOnForVocation;
			break;
		case CStdItem::ItemUseCondition::ucBattlePower: //战力必须大于等于value
			{
				if ((unsigned int)pData->nScore < (unsigned int)pCond->nValue) 
					return tpItemCannotTakeOnForBattlePower;
				break;
			}

		}
		pCond++;
	}
	return tpNoError;
}
bool CEquipVessel::TakeOnEquip(CActor *pActor,CUserItem* pUserItem,const HERODBDATA *pPetData,INT_PTR nLocation ,bool isBagEquip)
{
	INT_PTR nPos;
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if (!pStdItem || pStdItem->m_nPackageType != 1)//no exist or type not is equipmentType
	{
		//TODO:向角色发送不可穿戴此物品的消息，物品不是一个可穿戴的装备
		if(pActor->GetType() ==enActor)
		{
			pActor->SendOldTipmsgWithId(tpItemCannotTakeOn,ttTipmsgWindow);
		}
		return false;
	}
	int nJop = pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
	nPos = GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType);
	if (nPos == -1)
	{
		return false;
	}
	//判断装备穿戴的条件是否满足

	INT_PTR nErrorCode=tpNoError;
	//是否需要检测条件，如果不需要检测条件的话就不检测
	if(pPetData ==NULL)
	{
		nErrorCode= CheckTakeOnCondition(pActor, pStdItem, pUserItem);
	}
	else
	{
		nErrorCode= GetPetTakeItemErrorCode(pPetData, pStdItem, pUserItem,pActor);
	}

	if ( nErrorCode != tpNoError )
	{
		if(pActor->GetType() ==enActor)
		{
			( (CActor*)pActor)->SendOldTipmsgWithId(nErrorCode,ttFlyTip + ttTipmsgWindow);	
		}
		return false;
	}

	CUserItem *pOldEquip =NULL; //当前有装备，需要换装

	bool bHero = pPetData? true : false; 
	CUserBag& bag= pActor->GetBagSystem();
	//如果当前的位置有装备的话
	if(m_Items[nPos])
	{
		pOldEquip = m_Items[nPos];
	}

	//自动绑定的
	if( pStdItem->m_Flags.autoBindOnTake)
	{		
		if (!pUserItem->binded())	// 修改Bug：发送的绑定装备装上去也提示"**装备后绑定"
		{			
			bag.SetItemProperty(pUserItem, Item::ipItemBind,  ufBinded);

			if(pActor->GetType() ==enActor && !(pStdItem->isGodStoveEquipment()))//神炉装备不提示
			{
				( (CActor*)pActor)->SendOldTipmsgFormatWithId(tpItemTakenOnBinded,
					ttTipmsgWindow,
					CUserBag::GetItemLink(pStdItem->m_nIndex,pUserItem)); 
			}
		}
	}
	bool isDelItem = true;
	//穿上物品
	if(pUserItem->wCount > 1) {
		CUserItem* pNewEquip = GetLogicServer()->GetLogicEngine()->AllocUserItem(true); //新的物品
		if(pNewEquip ==NULL) return false;
		
		CUserItem::ItemSeries newGuid = pNewEquip->series; //保存序列号
		memcpy(pNewEquip,pUserItem,sizeof(*pUserItem)); //先拷贝过去
		pNewEquip->series = newGuid;  //序列号是新的
		pNewEquip->wCount = (WORD)1; //新的数量
		pActor->RemoveConsume(0, pUserItem->wItemId, 1, -1, -1,-1,0, GameLog::Log_TakeOn);
		m_Items[nPos] = pNewEquip;
		isDelItem = false;
	}else{
		m_Items[nPos] = pUserItem;
	}

	if(!m_Items[nPos]) return false;
	//这里要向客户端发一个消息，告诉玩家装备上这件物品
	AfterEquipTakenOn(pActor,nPos,m_Items[nPos]);
	if (isBagEquip && isDelItem)
	{
		bag.RemoveItem(pUserItem->series,"take_on_equip",GameLog::Log_TakeOn,false); //这里会走一个删除流程
	}

	if(pOldEquip) //这个时候需要把这件装备加到背包里去
	{

		WORD nLogId = GameLog::Log_TakeOff;
		pActor->GetBagSystem().AddItem(pOldEquip, pActor->GetEntityName(),nLogId);
		//TakeOffEquip(pActor, pOldEquip, true, bHero);
	}

	return true;
}
bool CEquipVessel::TakeOnEquip(CActor *pActor,const CUserItem::ItemSeries &series,const HERODBDATA *pPetData,INT_PTR nLocation , bool isBagEquip)
{
	CUserBag& bag= pActor->GetBagSystem();
	//从角色的包裹中查找要穿戴的物品
	CUserItem* pUserItem = bag.FindItemByGuid(series);
	if (!pUserItem)
	{
		//TODO:向角色发送要穿戴的物品不存在的消息
		if(pActor->GetType() ==enActor)
		{
			( (CActor*)pActor)->SendOldTipmsgWithId(tpItemNotExist,ttTipmsgWindow);
		}

		return false;
	}
	//确认物品是否是一个可穿戴的装备
	return TakeOnEquip(pActor, pUserItem, pPetData, nLocation, true);
}

bool CEquipVessel::DirectRemoveEquip(CActor *pOwner,CUserItem *pUserItem, LPCSTR pSender, INT_PTR nLogIdent, bool boDeedFreeMem)
{
	if(pUserItem ==NULL) return false;
	INT_PTR nIndex= FindIndex(pUserItem->series);

	if(nIndex <0) return false;
	AfterEquipDirectRemove(pOwner,pUserItem);
	AfterEquipTakenOff(pOwner,pUserItem);
	pUserItem->btHandPos =0;
	m_Items[nIndex] =NULL;

	//记录日志
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if (pStdItem )
	{
		CUserBag::LogChangeItemCount(pUserItem, pStdItem, -pUserItem->wCount, 
			pSender, pOwner->GetProperty<unsigned int>(PROP_ENTITY_ID), pOwner->GetEntityName(), 
			nLogIdent, pOwner->GetAccount());
	}
	else
	{
		OutputMsg(rmError,"DirectRemoveEquip pStdItem is NULL ");
	}
	if (boDeedFreeMem)
	{
		GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
	}

	return true;
}


bool CEquipVessel::TakeOffEquip(CActor *pOner,CUserItem *pUserItem, bool boCheckGrid, bool boHero)
{
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if (!pStdItem)
	{
		return false;
	}
	if (boCheckGrid && pStdItem->isGodStoveEquipment())
	{
		return false;//神炉装备不能脱到背包
	}
	//如果背包已经满了则不能取下物品
	if ( boCheckGrid && !pOner->GetBagSystem().CanAddItem(pUserItem, true))
	{
		//TODO:向角色发送背包已满，无法取下物品的消息
		if(pOner->GetType() ==enActor)
		{
			( (CActor*)pOner)->SendTipmsgFormatWithId(tmEquipNoBagNum, tstUI); 
		}

		return false;
	}
	//先脱装备？ 可能添加物品那边有问题 导致脱装备失败
	AfterEquipTakenOff(pOner,pUserItem);
	WORD nLogId = boHero? GameLog::Log_TakeOff : GameLog::Log_TakeOff;
	if( boCheckGrid && pOner->GetBagSystem().AddItem(pUserItem, pOner->GetEntityName(),nLogId) ==0 )
	{
		return false;

	}
	else
	{
		pUserItem->btHandPos =0; //清掉装备的位置
		//AfterEquipTakenOff(pOner,pUserItem);
		return true;
	}

}


bool CEquipVessel::TakeOffEquip(CActor *pActor,const CUserItem::ItemSeries &series, bool boCheckGrid, bool boHero)
{
	

	//查找装备穿戴的位置
	INT_PTR nPos = FindIndex(series);
	if ( nPos < 0 )
	{
		//TODO:向角色发送装备物品不存在的消息
		if(boCheckGrid && pActor->GetType() ==enActor)
		{
			( (CActor*)pActor)->SendOldTipmsgWithId(tpItemNotExist,ttTipmsgWindow); 
		}
		return false;
	}
	else
	{
		if(TakeOffEquip(pActor,m_Items[nPos], boCheckGrid, boHero))
		{
			m_Items[nPos] =NULL;
			return true;
		}
		else
		{
			return false;
		}
	}
}
void CEquipVessel::CalcOneEquipAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, CAnimal* pMaster)
{
	if (!pUserItem)
	{
		return;
	}
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if (!pStdItem)
	{
		return;
	}
	bool bNeedAddProp = true;

	if (bNeedAddProp)//有耐久
	{
		if (pUserItem->btLuck)
		{
			GAMEATTR val;
			if (pUserItem->btLuck > 0)
			{
				val.type = aLuckAdd;
				val.value.nValue = pUserItem->btLuck;
			}
			
			if (pOriginalCal)
			{
				*pOriginalCal << val;
			}
			calc << val;
		}
		std::string bestAttr = pUserItem->cBestAttr;
		if(bestAttr != "")
		{
			std::vector< std::string > results = SplitStr(bestAttr, "|");
			int nCount = results.size();
			for (int i = 0; i< nCount; i++)
			{
				std::vector< std::string > temp_res = SplitStr( results[i], ",");
				if(temp_res.size() >=2 )
				{
					int type = atoi(temp_res[0].c_str());
					int value = atoi(temp_res[1].c_str());
					GAMEATTR val;
					val.type = type;
					val.value.nValue = value;
					calc << val;
				}
			}
		}
		pStdItem->CalcAttributes(calc, pUserItem->btQuality, pUserItem->btStrong, pUserItem, pMaster);
	}

	if (pOriginalCal)
	{
 		pStdItem->CalcAttributes(*pOriginalCal, pUserItem->btQuality, pUserItem->btStrong, pUserItem, pMaster);
	}
	/*** ★★★★★★★★★★★★★★★★计算装备鉴定属性★★★★★★★★★★★★★★★★ ***/
	for( INT_PTR j = 0; j < CUserItem::MaxSmithAttrCount; ++j)
	{
		if( pUserItem->smithAttrs[j].type)
		{
			if( bNeedAddProp )
			{
				calc << pUserItem->smithAttrs[j];
			}
			if( pOriginalCal )
			{
				*pOriginalCal << pUserItem->smithAttrs[j];
			}
		}
	}
	/*** ★★★★★★★★★★★★★★★★计算装备升星属性★★★★★★★★★★★★★★★★ ***/
	CalcOneEquipStarAttr(calc, pOriginalCal, pUserItem, pMaster, bNeedAddProp);


	/*** ★★★★★★★★★★★★★★★★计算装备铭刻属性★★★★★★★★★★★★★★★★ ***/
	//CalcOneEquipInscriptAttr(calc, pOriginalCal, pUserItem, pMaster, bNeedAddProp);

	/*** ★★★★★★★★★★★★★★★★计算启兵属性加成★★★★★★★★★★★★★★★★ ***/
	//(基础属性+品质属性)* (1+部位强化百分比)
	
}

/*计算装备的强化升星属性
注意：策划规定不同等级的装备只是可以强化的星级上限不同，而每个星级的每个部位加的属性是相同的，并且最大星级放在最后。
所以配置直接取最后一段配置
EquipStar配置仅仅是供客户端显示和服务器扣费使用
*/
void CEquipVessel::CalcOneEquipStarAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, 
	CAnimal* pMaster, bool bNeedAddProp)
{
	if( pUserItem->wStar > 0 && pMaster )
	{
		const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
		if (pStdItem && pUserItem->wStar > 0)
		{
			ItemUpStarCfg* pCfg = GetLogicServer()->GetDataProvider()->GetUpStar().getItemUpStarCfg(pUserItem->wItemId, pUserItem->wStar);
			if (pCfg)
			{
				for( int k = 0; k < pCfg->attr.nCount; k++ )
				{
					calc <<  pCfg->attr.pAttrs[k];
				}
			}
		}
	}
}

/*计算装备的铭刻属性
注意：策划规定不同等级的装备只是可以强化的星级上限不同，而每个星级的每个部位加的属性是相同的，并且最大星级放在最后。
所以配置直接取最后一段配置
EquipInscriptCfg配置仅仅是供客户端显示和服务器扣费使用
*/
void CEquipVessel::CalcOneEquipInscriptAttr(CAttrCalc &calc, CAttrCalc *pOriginalCal,  CUserItem* pUserItem, 
	CAnimal* pMaster, bool bNeedAddProp)
{
}


/*
计算装备属性
*/
CAttrCalc & CEquipVessel::CalcAttr(CAttrCalc &calc,CAttrCalc *pOriginalCal ,CAnimal *pMaster,const HERODBDATA *pPetData)
{
	INT_PTR i;
	const CStdItem *pStdItem;
	CUserItem *pUserItem;
	//const CStdItem::UnitedAttributeGroup *pUnitedGroup;
	CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	
	// INT_PTR nOrangeEquipCount =0;                        //橙色装备的数目
	

	// INT_PTR nMatchAllSuitCount =0;  //能够匹配所有套装的件数
	bool nFlag = true;
	CAttrCalc cal;
	bool bNeedCalcEquipPosStrongDerive = true;//是否需要计算启兵衍生属性(戴上所有装备并所有装备耐久>0)
	for (i=0 ; i<EquipmentCount; i++)
	{
		
		bool isBasicEquips = IsBasicEquip(i); //是否是基本装备，12件
		pUserItem = m_Items[i];
		if(pUserItem ==NULL)
		{
			continue;
		}
		//计算套装 加成的基础装备属性
		if(!isBasicEquips && nFlag)
		{
			SuitAttrCal(cal);
			nFlag = false;
		}
		pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
		if (pPetData)
		{
			if (GetPetTakeItemErrorCode(pPetData,pStdItem, pUserItem,pMaster)) //如果装备的条件不满足了，则不计算属性了
			{
				continue;
			}
		}
		else if (pMaster)
		{
			if (CheckTakeOnCondition(pMaster,pStdItem, pUserItem)) //如果装备的条件不满足了，则不计算属性了
			{
				continue;
			}
		}
		//统一计算单件装备属性
		CalcOneEquipAttr(cal, pOriginalCal, pUserItem, pMaster);
		//单件装备套装加成
	}

	//内功共鸣属性
	
	if ( m_Items[itSilverNeedlePos] && m_Items[itSilverHairClaspPos] )
	{
		CalcResonanceAttr(cal, m_Items[itSilverNeedlePos], m_Items[itSilverHairClaspPos]);
	}
	
	if ( m_Items[itDeterminationPos] && m_Items[itHeartNourishingJadePos] )
	{
		CalcResonanceAttr(cal, m_Items[itDeterminationPos], m_Items[itHeartNourishingJadePos]);
	}

	if ( m_Items[itGoldSealPos] && m_Items[itGoldObsidianBeadPos] )
	{
		CalcResonanceAttr(cal, m_Items[itGoldSealPos], m_Items[itGoldObsidianBeadPos]);
	}
	
	if ( m_Items[itSandersPearlPos] && m_Items[itBlackSandalwoodStonePos] )
	{
		CalcResonanceAttr(cal, m_Items[itSandersPearlPos], m_Items[itBlackSandalwoodStonePos]);
	}

	calc <<cal;
	return calc;
}





CUserItem* CEquipVessel::FindItemByID(const INT_PTR wItemId,CUserItem** pUserItemVec,INT_PTR nItemCount)
{
	for (INT_PTR i=0; i<nItemCount; i ++)
	{
		if ( pUserItemVec[i] && pUserItemVec[i]->wItemId == wItemId )
		{
			return pUserItemVec[i];
		}
	}
	return NULL;
}

INT_PTR CEquipVessel::GetEquipCountByID(const INT_PTR wItemId, const INT_PTR nQuality, const INT_PTR nStrong,const INT_PTR nBind,const INT_PTR nDuraOdds, void * pNoItem)
{
	INT_PTR Result = 0;
	CUserItem *pUserItem, *pNoGetItem = NULL;
	if (pNoItem != NULL)
	{
		pNoGetItem = (CUserItem*)pNoItem;
	}
	for (INT_PTR i= EquipmentCount-1; i>-1; --i)
	{
		pUserItem = m_Items[i];
		if (pUserItem && pUserItem->wItemId == wItemId )
		{
			if ( pUserItem->btQuality != nQuality && nQuality != -1)
				continue;
			if ( pUserItem->btStrong != nStrong && nStrong != -1 )
				continue;
			//判断绑定属性是否一致
			if(nBind != -1 && nBind != 255)
			{
				if( (nBind && 0==pUserItem->btFlag) || (nBind==0 && pUserItem->btFlag) ) 
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
			Result ++;
		}
	}
	return Result;
}

INT_PTR CEquipVessel::FindIndex(const CUserItem::ItemSeries series)
{
	for (INT_PTR i=0; i<EquipmentCount; ++i)
	{
		if ( m_Items[i] && m_Items[i]->series.llId == series.llId )
			return i;
	}
	return -1;
}

float  CEquipVessel::GetItemAttrSetScore( CUserItem* pUseItem,  CActor* pActor)
{
	if (!pUseItem)
	{
		return 0;
	}
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUseItem->wItemId);
	if (pStdItem)
	{
		CAttrCalc calc;
		CAttrCalc pOrignCalc;
		CalcOneEquipAttr(calc, &pOrignCalc, pUseItem, pActor);
		CAttrEval *pCal = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetEval(pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION));
		if (pCal)
		{
			return	pCal->GetAttrSetScore(pOrignCalc);
		}
	}
	return 0;
}

INT_PTR CEquipVessel::GetItemTakeOnPosition(const Item::ItemType btItemType, INT_PTR btLocation)
{
	if(btItemType > Item::itUndefinedType && btItemType < Item::itEquipMax)
	{
		INT_PTR nPos; 
		switch (btItemType)
		{
		case Item::itWeapon:
			nPos = itWeaponPos;
			break;
		case Item::itDress:
			nPos = itDressPos;
			break;
		case Item::itHelmet:
			nPos = itHelmetPos;
			break;
		case Item::itNecklace:
			nPos = itNecklacePos;
			break;
		case Item::itDecoration:
			nPos = itDecorationPos;
			break;
		case Item::itBracelet:
			nPos = itBraceletPos;
			break;
		case Item::itRing:
			nPos = itRingPos;
			break;
		case Item::itGirdle:
			nPos = itGirdlePos;
			break;
		case Item::itShoes:
			nPos = itShoesPos;
			break;
		case Item::itEquipDiamond:
			nPos = itEquipDiamondPos;
			break;
		case Item::itSzBambooHat:			//特戒
			nPos = itBambooHatPos;
			break;
		case Item::itSztFaceNail:			//幻武
			nPos = itFaceNailPos;
			break;
		case Item::itSzitCape:			//玄甲
			nPos = itCapePos;
			break;
		case Item::itSzitShiel:
			nPos = itShieldPos;
			break;

		case Item::itPearl:
			nPos = itPearlPos;
			break;
		case Item::itMagicpWeapon:
			nPos = itMagicpPos;
			break;
		case Item::itSilverNeedle:
			nPos = itSilverNeedlePos;
			break;
		case Item::itDetermination:
			nPos = itDeterminationPos;
			break;
		case Item::itGoldSeal:
			nPos = itGoldSealPos;
			break;
		case Item::itSandersPearl:
			nPos = itSandersPearlPos;
			break;
		case Item::itSilverHairClasp:
			nPos = itSilverHairClaspPos;
			break;
		case Item::itHeartNourishingJade:
			nPos = itHeartNourishingJadePos;
			break;
		case Item::itGoldObsidianBead:
			nPos = itGoldObsidianBeadPos;
			break;
		case Item::itBlackSandalwoodStone:
			nPos = itBlackSandalwoodStonePos;
			break;
		default:
			nPos = -1;
			break;
		}
		return nPos;
	}
	else 
	{
		return -1;
	}
}

INT_PTR CEquipVessel::CheckTakeOnCondition(const CEntity *pMonster, const CStdItem* pStdItem, const CUserItem *pUserItem)
{
	const CStdItem::ItemUseCondition *pCond;
	if (pMonster == NULL
		 || pStdItem == NULL) 
	{
		return tpItemNotExist;
	}

	pCond = pStdItem->m_Conditions.pConds;
	INT_PTR nEntityType = pMonster->GetType();
	if (pUserItem == NULL)
	{
		return tpItemNotExist;
	}
   
	if(pStdItem->m_CanUseType > eITEM_USE_LIMIT_NULL 
		&& pStdItem->m_CanUseType < eITEM_USE_LIMIT_MAX) //使用次数限制
	{
		if(pStdItem->m_CanUseCount <= 0)
		{ 
			return tpNoError;
		}
	}
	for (INT_PTR i = pStdItem->m_Conditions.nCount - 1; i > -1; --i)
	{
		switch(pCond->btCond)
		{
		case CStdItem::ItemUseCondition::ucLevel: //等级必须大于等于value级
			if (pMonster->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) < (unsigned int)pCond->nValue)
				return tpItemCannotTakeOnForLevel;		
			break;
		case CStdItem::ItemUseCondition::ucGender: //性别编号必须等于value，0男1女
			if( nEntityType ==enActor &&
				pMonster->GetProperty<unsigned int>(PROP_ACTOR_SEX) != (unsigned int)pCond->nValue) return tpItemCannotTakeOnForSex;
			break;
		case CStdItem::ItemUseCondition::ucJob: //职业编号必须等于value
			if( nEntityType ==enActor &&
				pMonster->GetProperty<unsigned int>(PROP_ACTOR_VOCATION) != (unsigned int)pCond->nValue && pCond->nValue != 0) return tpItemCannotTakeOnForVocation;
			break;
		case CStdItem::ItemUseCondition::ucSocialMask:		
			{
				if(nEntityType ==enActor)
				{
					unsigned int nActorId = pMonster->GetId();
					//必须为沙巴克城主
					if(nActorId > 0 && nActorId != GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnMasterId())
					{
						return tpNotSbkCityOwner;
					}
				}
				break;
			}
		case CStdItem::ItemUseCondition::ucBattlePower: //战力必须大于等于value
			if (nEntityType == enActor)	//转数判断
			{
				if (pMonster->GetProperty<unsigned int>(PROP_ACTOR_BATTLE_POWER) < (unsigned int)pCond->nValue) 
					return tpItemCannotTakeOnForBattlePower;
			}
			break;
		case CStdItem::ItemUseCondition::ucMinCircle:		//大于等于这个转生才能使用
			{
				if (nEntityType == enActor)	//转数判断
				{
					if (pMonster->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE) < (unsigned int)pCond->nValue) 
					{
						/*就判断转数，没必要判断等级，等级在ucLevel判断
						INT_PTR nMinLv = pStdItem->GetUseConditionValue(CStdItem::ItemUseCondition::ucLevel);
						if(pMonster->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) < nMinLv)
						{
							return tpNoMoreCircleNotUse;
						}*/
						return tpNoMoreCircleNotUse;
					}
				}
				break;
			}
		case CStdItem::ItemUseCondition::ucMaxCircle:		//小于等于这个转生才能使用
			{
				if (nEntityType == enActor)	//转数判断
				{
					if (pMonster->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE) > (unsigned int)pCond->nValue) 
						return tpUpCircleCanNotUse;
				}
				break;
			}
		case CStdItem::ItemUseCondition::ucInternalWorkLevel:		//大于等于这个内功等级才能使用
			{
				if (nEntityType == enActor)	//内功判断
				{
					if (pMonster->GetProperty<unsigned int>(PROP_ACTOR_MERIDIALv) < (unsigned int)pCond->nValue) 
						return tmInternalWorkLevelNotEnough;
				}
				break;
			}
		case CStdItem::ItemUseCondition::ucGuildLevel:			 	//行会等级必须大于等于value级
			{
				if (nEntityType == enActor)	//行会等级
				{
					if (pMonster->GetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL) < (unsigned int)pCond->nValue) 
						return tmGuildLevelLimit;
				}
				break;
			}
		}
		++pCond;
	}

	return tpNoError;
}

int  CEquipVessel::GetTotalStar()  const
{
	CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	const CUserItem *pUserItem = NULL;
	const CStdItem *pStdItem = NULL;
	int  nStar = 0 ; 
	for(INT_PTR i = 0; i < EquipmentCount; ++i)
	{
		bool isBasicEquips = IsBasicEquip(i); //是否是基本装备
		if(!isBasicEquips)
			continue;
		pUserItem = m_Items[i];
		if (!pUserItem)
		{
			continue;
		}
		pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
		if (!pStdItem)
		{
			continue;
		}
		nStar = nStar + pUserItem->wStar;
	}
	return nStar;
}


INT_PTR CEquipVessel::GetSuitMinStar() const
{
	CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	INT_PTR nCount = 0;
	const CUserItem *pUserItem = NULL;
	const CStdItem *pStdItem = NULL;
	int  nStar = 0 ; //全身装备最低升星等级
	int  nEquipStarCount = 0; //已升星的装备数量
	//for(INT_PTR i = 0; i < itBloodSoulPos; ++i)
	for(INT_PTR i = 0; i < itMaxEquipPos; ++i)
	{
		bool isBasicEquips = IsBasicEquip(i); //是否是基本装备
		if(!isBasicEquips)
		    continue;
		pUserItem = m_Items[i];
		if (!pUserItem)
		{
			continue;
		}
		pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
		if (!pStdItem)
		{
			continue;
		}
		if(pUserItem->wStar > 0 && nStar == 0)
		{
		    nStar = pUserItem->wStar;
		    nEquipStarCount++;
		}else if(pUserItem->wStar > 0 && nStar > 0)
		{
		    if(pUserItem->wStar < nStar)
			{
			    nStar = pUserItem->wStar;
				nEquipStarCount++;
			}else
			{
				nEquipStarCount++;
			}
		}
	}
	if(nEquipStarCount <BASE_EQUIP_COUNT) 
	{
	    return 0;
	}else
	{
	   return nStar;
	}
}
INT_PTR CEquipVessel::GetMinStrongLevel() const
{
	CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	INT_PTR nMinStrongLevel = 100;       //最低的强化的属性
	const CUserItem *pUserItem = NULL;
	const CStdItem *pStdItem = NULL;
	for(INT_PTR i = 0; i < EquipmentCount; ++i)
	{
		bool isBasicEquips = IsBasicEquip(i); //是否是基本装备，12件
		pUserItem = m_Items[i];
		if(pUserItem ==NULL)
		{
			//如果是基本装备没有装备，玩家的最低的宝石和强化等级直接设置为0(全身才有效)
			if(isBasicEquips)
			{
				nMinStrongLevel = 0; 
			}
			break;
		}
		pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
		if (!pStdItem)
			break;

		if(isBasicEquips) //如果是基本的装备，就是12件
		{
			//全身最低的强化的等级
			if( pUserItem->btStrong < nMinStrongLevel)
			{
				nMinStrongLevel = pUserItem->btStrong;
			}
		}
	}

	return nMinStrongLevel < CStdItem::MaxItemStrong && nMinStrongLevel > 0 ? nMinStrongLevel : 0;
}

bool CEquipVessel::CanActivityFiveAttr(BYTE btPos)
{
	return false;
}

bool CEquipVessel::IsFiveAttrPos(BYTE btPos)
{
	return false;
}


LPSTR CEquipVessel::getEquipName(const CStdItem* pItem )
{
	if(!pItem) return NULL;
	if(pItem->m_btType > Item::itUndefinedType && pItem->m_btType < Item::itEquipMax)
	{
		INT_PTR nPos; 
		switch (pItem->m_btType)
		{
		case Item::itWeapon:
			nPos = tmEquipName1;
			break;
		case Item::itDress:
			nPos = tmEquipName2;
			break;
		case Item::itHelmet:
			nPos = tmEquipName3;
			break;
		case Item::itNecklace:
			nPos = tmEquipName4;
			break;
		case Item::itDecoration:
			nPos = tmEquipName5;
			break;
		case Item::itBracelet:
			nPos = tmEquipName6;
			break;
		case Item::itRing:
			nPos = tmEquipName7;
			break;
		case Item::itGirdle:
			nPos = tmEquipName8;
			break;
		case Item::itShoes:
			nPos = tmEquipName9;
			break;
		case Item::itEquipDiamond:
			nPos = tmEquipName10;
			break;
		case Item::itSzBambooHat:			//特戒
			nPos = tmEquipName11;
			break;
		case Item::itSztFaceNail:			//幻武
			nPos = tmEquipName12;
			break;
		case Item::itSzitCape:			//玄甲
			nPos = tmEquipName13;
			break;
		case Item::itSzitShiel:
			nPos = tmEquipName14;
			break;
		case Item::itPearl:
			nPos = tmEquipName15;
			break;
		case Item::itMagicpWeapon:
			nPos = tmEquipName16;
			break;
		default:
			return NULL;
		}
		return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nPos);;
	}
	
	return NULL;
}

LPSTR CEquipVessel::getEquipNameDesc(const CStdItem* pItem )
{
	if(!pItem) return NULL;
	INT_PTR nPos = tmEquipQuality0; 
	switch (pItem->b_showQuality)
	{
		case 0:
			nPos = tmEquipQuality0;
			break;
		case 1:
			nPos = tmEquipQuality1;
			break;
		case 2:
			nPos = tmEquipQuality2;
			break;
		case 3:
			nPos = tmEquipQuality3;
			break;
		case 4:
			nPos = tmEquipQuality4;
			break;
		case 5:
			nPos = tmEquipQuality5;
			break;
	}
	
	return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nPos);
}


LPSTR CEquipVessel::getItemColorDesc(const CStdItem* pItem )
{
	if(!pItem) return NULL;
	INT_PTR nPos = tmItemQuality0; 
	switch (pItem->b_showQuality)
	{
		case 0:
			nPos = tmItemQuality0;
			break;
		case 1:
			nPos = tmItemQuality1;
			break;
		case 2:
			nPos = tmItemQuality2;
			break;
		case 3:
			nPos = tmItemQuality3;
			break;
		case 4:
			nPos = tmItemQuality4;
			break;
		case 5:
			nPos = tmItemQuality5;
			break;
	}
	
	return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nPos);
}

void CEquipVessel::CalcResonanceAttr(CAttrCalc &cal,CUserItem *pEquipment,CUserItem *pStone)
{
	const CStdItem* pEquipmentStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pEquipment->wItemId);
	const CStdItem* pStoneStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pStone->wItemId);

	if ( pEquipmentStdItem && pStoneStdItem )
	{
		AttriGroup* pAttr = GetLogicServer()->GetDataProvider()->GetResonanceItemProvider().GetResonanceItemConfig(pStoneStdItem->m_wResonanceId, pEquipmentStdItem->m_nIndex);
		if (pAttr)
		{
			for (size_t i = 0; i < pAttr->nCount; i++)
			{
				cal << pAttr->pAttrs[i];
			}
		}
	}
	
}