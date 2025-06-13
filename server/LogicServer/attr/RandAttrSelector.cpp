#include "StdAfx.h"
#include "RandAttrSelector.h"

inline bool AttrExists(PACKEDGAMEATTR &packedAttr, const PPACKEDGAMEATTR pAttrs, INT_PTR numAttrs,
	const bool boCheckValue = false)  
{
	while (numAttrs > 0)
	{
		numAttrs--;
		if ( packedAttr.type == pAttrs[numAttrs].type )
		{
			if ( !boCheckValue  )
				return true;
			//检查值是否相同
			if ( packedAttr.value == pAttrs[numAttrs].value )
				return true;
		}
	}
	return false;
}

inline void MakePackedAttrFromRandAttr(OUT PACKEDGAMEATTR &dest, int nType, const RANDOMATTVALUE& randValue)
{
	//LARGE_INTEGER c;
	double dval;

	//QueryPerformanceCounter(&c);
	unsigned long c = wrandvalue();
	double result ; //最终的计算的数值 
	dest.type = nType;
	if (nType < 1 || nType >= GameAttributeCount) return;

	switch(AttrDataTypes[nType])
	{
	case adSmall://有符号1字节类型
	case adShort://有符号2字节类型
	case adInt://有符号4字节类型
		result = 	(int) (randValue.minValue.nValue + (c % (randValue.maxValue.nValue - randValue.minValue.nValue + 1)));
		if(result >=0.0) 
		{
			dest.value = (WORD)result; 
			dest.sign = 0;//表示正数
		}
		else
		{
			dest.value = (WORD)(-result); 
			dest.sign = 1; //表述负数
		}
		
		break;
	case adUSmall://无符号1字节类型
	case adUShort://无符号2字节类型
	case adUInt://无符号4字节类型
		result =(unsigned int) ( randValue.minValue.uValue 
			+ (c % (randValue.maxValue.uValue - randValue.minValue.uValue + 1)) );
		if(result >=0.0) 
		{
			dest.value = (WORD)result; 
			dest.sign = 0;//表示正数
		}
		else  //如果无符号的出现了负数，那么就是0
		{
			dest.value = (WORD)(0); 
			dest.sign = 0; 
		}


		break;
	case adFloat://单精度浮点类型值
		dval = randValue.maxValue.fValue - randValue.minValue.fValue;
		dval = randValue.minValue.fValue + (c % ((int)(dval * 10000) + 1)) / 10000.0;
		result= (int) (dval / tagPackedGameAttribute::FloatValueBase);
		if(result >=0.0) 
		{
			dest.value = (WORD)result; 
			dest.sign = 0;//表示正数
		}
		else  //如果无符号的出现了负数，那么就是0
		{
			dest.value = (WORD)(-result); 
			dest.sign = 1; //表述负数
		}
		break;
	}
	
}

//一件装备上是否存精锻过一个属性了
inline bool IsAttriExist(PACKEDGAMEATTR* pAttr,INT_PTR nAttrCount,int nType, int nGroup)
{
	if(nType == 0) return false;
	int nGroupCount = 0;
	for (INT_PTR i = 0; i< nAttrCount; ++i)
	{
		if (pAttr[i].type == nType)
		{
			nGroupCount++;
		}
	}

	for (INT_PTR i = 0; i< nAttrCount; ++i)
	{
		if (pAttr[i].type == nType ) //输入属性相同
		{
			if (nGroupCount < nGroup)
			{
				continue;
			}
			return true;
		}
	}
	return false;
}

//这个函数当前不用
bool RandAttrSelector::SelectItemSmithAttribute(OUT PACKEDGAMEATTR &packedAttr, const CUserItem* pUserItem,INT_PTR &nSelectIndex)
{
	//LARGE_INTEGER c;

	/*
	PACKEDGAMEATTR attr;
	const CStdItem* pStdItem;

	pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if ( !pStdItem || !pStdItem->m_SmithAttrs.nCount )
		return false;

	INT_PTR nAttrCount=0;// 本身已经有的精锻属性
	for (INT_PTR i = 0; i<CUserItem::MaxSmithAttrCount; ++i)
	{
		if ( pUserItem->smithAttrs[i].type )
		{
			nAttrCount ++;
		}
	}
	nSelectIndex = -1;
	//如果本身已经有那么多属性了
	if(nAttrCount >=  pStdItem->m_SmithAttrs.nCount)
	{
		return false;
	}
	/*
	* 策划的精锻思想是：
	*  随机随一个属性，如果属性类型和现在相同的话，值不相同就替换掉现在这个属性，否则再随一次
	*  如果属性类型和现在的不同，如果现在属性已经满的话，就随机替换一个，否则使用无属性的位置
	*/
	/*
	//随出来的属性的index
	INT_PTR  nRandIndex= wrandvalue()  % pStdItem->m_SmithAttrs.nCount;
	
	for(INT_PTR i=0; i<pStdItem->m_SmithAttrs.nCount ; i++ )
	{
		
		INT_PTR nIdx =( nRandIndex + i ) % pStdItem->m_SmithAttrs.nCount;
		INT_PTR nType = pStdItem->m_SmithAttrs.pAttrs[nIdx].type; //标准物品的属性
		
		for (INT_PTR i = 0; i<CUserItem::MaxSmithAttrCount; ++i)
		{
			if ( pUserItem->smithAttrs[i].type == nType ) //输入属性相同
			{
				//如果属性相同的话，要进去抽一次
				MakePackedAttrFromRandAttr(attr, pStdItem->m_SmithAttrs.pAttrs[nIdx]); //算一个属性出来

				//属性和值都相同，要重新转一次
				if(pUserItem->smithAttrs[i].nValue  ==  attr.nValue) //如果抽的相同的话
				{
					nType = 0;
				}
				else //如果属性相同，但是值不同的话，就替换现在的
				{
					packedAttr.nValue =attr.nValue;
					nSelectIndex = i; //这个属性被选中了
					return true;
				}
				break;
			}
		}
		
		//生成属性并判断属性是否存在
	
		if (nType)
		{
			MakePackedAttrFromRandAttr(attr, pStdItem->m_SmithAttrs.pAttrs[nIdx]); //算一个属性出来
			if(nSelectIndex  <0 ) //如果已经找到了位置，就不再找位置了，否则要找一个替换掉
			{
				if(nAttrCount < CUserItem::MaxSmithAttrCount)
				{
					nSelectIndex = nAttrCount;
				}
				else //要随机替换一个属性
				{
					nSelectIndex = wrandvalue() % CUserItem::MaxSmithAttrCount; //随机替换一个
				}
			}
			packedAttr.nValue =attr.nValue;
			return true;
		}

	}
	*/
	return false;
}
// bool RandAttrSelector::InitSmithByUserItem( CUserItem * pUserItem, int nQualityDataIndex)
// {
// 	if(!pUserItem)
// 	{
// 		return false;
// 	}
// 	if (pUserItem->btQuality == 0)
// 	{
// 		return false;
// 	}
// 	int smithArr[CUserItem::MaxSmithAttrCount];
// 	memset(smithArr, 0 , sizeof(smithArr));
// 	InitSmithById(pUserItem->wItemId, pUserItem->btQuality, nQualityDataIndex, smithArr);
// 	for (int i=0; i < ArrayCount(smithArr); i++)
// 	{
// 		pUserItem->smithAttrs[i].nValue = smithArr[i];
// 	}
// 	return true;
// }
// void RandAttrSelector::InitSmithById(WORD wItemId, BYTE btQuality, int nQualityDataIndex, int smithArr[CUserItem::MaxSmithAttrCount])
// {
// 	CStdItemProvider& provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
// 	const CStdItem * pStdItem = provider.GetStdItem(wItemId);
// 	if (pStdItem == NULL || btQuality <=0 || nQualityDataIndex <= 0)
// 	{
// 		return ;
// 	}
// 	DataList<int>* qualityDataList = provider.GetQualityIndexList(nQualityDataIndex);
// 	if (!qualityDataList)
// 	{
// 		return;
// 	}
// 	for (int i =0; i < btQuality ; i++)
// 	{
// 		int nSmithId = qualityDataList->pData[i];
// 		DataList<RANDOMATTRTERM> * pConfig = provider.GetSmithData(nSmithId);
// 		if (pConfig == NULL)
// 		{
// 			continue;
// 		}
// 		PACKEDGAMEATTR smith;
// 		bool boResult = RealSmithAttribute(pConfig,&smith);
// 		if( !boResult)
// 		{
// 			OutputMsg(rmNormal, _T("生成极品属性出错：物品id:%d, nQuality:%d, nQualityDataIndex:%d"), wItemId, btQuality, nQualityDataIndex);
// 			smith.nValue =0;
// 		}
// 		smithArr[i] = smith.nValue;
// 	}
// }

void RandAttrSelector::GenerateAttrsInSmith( int nSmithId, int nAttrNum, unsigned char nJob, unsigned char nAttrTypeMust )
{
	for( int i=0; i<nAttrNum; i++ )
	{
		unsigned char nAttrType = 0;
		int  nAttrValue= 0;
		GenerateOneAttrInSmith( nSmithId, nAttrType, nAttrValue, nJob, nAttrTypeMust );
	}
}

bool RandAttrSelector::ItemSmithAttribute(  CUserItem* pUserItem, int nSmithIdx, bool bReserverOld)
{
	
	return false;//2015-10-23 不要鉴定属性了
	////LARGE_INTEGER c;
	//
	////PACKEDGAMEATTR attr;
	//const CStdItem* pStdItem;
	//CStdItemProvider& provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	//pStdItem = provider.GetStdItem(pUserItem->wItemId);
	//if ( !pStdItem  )
	//	return false;

	////INT_PTR nAttrCount=0;// 本身已经有的精锻属性
	////for (INT_PTR i = 0; i<CUserItem::MaxSmithAttrCount; ++i)
	////{
	////	if ( pUserItem->smithAttrs[i].type )
	////	{
	////		nAttrCount ++;
	////	}
	////}
	////INT_PTR nSmithId = pStdItem->m_btSmithId; //精锻的id

	//
	//
	////如果本身的属性超过了最大的个数，本身就出现了问题
	///*if(nAttrCount >  CUserItem::MaxSmithAttrCount)
	//{
	//return false;
	//}*/
	//INT_PTR nSmithCount = pUserItem->btSmithCount; //表示该物品的精锻过的次数
	//if(!bReserverOld)
	//	nSmithCount ++;

	////玩家精锻的属性
	////最大的精锻的次数
	//INT_PTR nMaxSmishCount =  GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMaxSmithCount  ; 
	//if(nSmithCount > nMaxSmishCount)
	//{
	//	nSmithCount =  nMaxSmishCount;
	//}

	//BAGCONFIG & bag = GetLogicServer()->GetDataProvider()->GetBagConfig();

	//////能够精锻几个属性
	////INT_PTR nCanSmithPropCount =  0;
	////if(nSmithCount >= bag.nSmith3 )
	////{
	////	nCanSmithPropCount = 3;
	////}
	////else if(nSmithCount >= bag.nSmith2 )
	////{
	////	nCanSmithPropCount = 2;
	////}
	////else if(nSmithCount >= bag.nSmith1)
	////{
	////	nCanSmithPropCount = 1;
	////}
	////if(nCanSmithPropCount <=0) 
	////{
	////	OutputMsg(rmWaning,_T("改装备不能精锻"));
	////	return false;
	////}
	//DataList<RANDOMATTRTERM> * pConfig= provider.GetSmithData(pUserItem->wItemId );
	//if(pConfig ==NULL) return false; //没有配置精锻的数据

	//if( RealSmithAttribute(pConfig, nSmithIdx, nSmithCount,pUserItem->smithAttrs,CUserItem::MaxSmithAttrCount, bReserverOld))
	//{
	//	if(bReserverOld)
	//	{
	//		nSmithCount++;
	//		if(nSmithCount > nMaxSmishCount)
	//		{
	//			nSmithCount =  nMaxSmishCount;
	//		}
	//	}
	//	pUserItem->btSmithCount =(WORD)nSmithCount; //装备的精锻的次数
	//	return true;
	//}
	//else
	//{
	//	OutputMsg(rmNormal, _T("精段时无具体属性:物品id:%d,次数:%d"), pUserItem->wItemId, nSmithCount);
	//	return false;
	//}
	//
	
}

/*
nAttrTypeMust:强制刷出某种属性
如果非此属性不在库中
nJob：职业要求

如果强制指定某属性，则可以忽略职业要求
*/
bool RandAttrSelector::RealSmithAttribute( DataList<RANDOMATTRTERM> * pConfig,PACKEDGAMEATTR* pAttr, unsigned char nAttrTypeMust,
	unsigned char nJob, PACKEDGAMEATTR* pAttrsAll )
{
	//精锻的算法为先从几个属性里随机选择一个，然后[min,max]之间随机一个属性的数值出来
	//type为属性类型；
	//prop为属性选择的概率，这里用不到，直接填0，如果使用的话表示该属性的权重
	//min表示属性的属性值下限，max表示上限，在根据配置产生具体属性值的时候，其结果将大于等于min且小于等于max；
	INT_PTR nTotalProp =0;  //总的概率的数值
	INT_PTR nPropCount = pConfig->count;//可用的属性的个数

	if( nAttrTypeMust > 0 )		//如果指定了属性，则忽略职业要求
	{
		nJob = 0;
	}
	for( INT_PTR i=0; i< nPropCount; i++ )
	{
		PRANDOMATTRTERM pInfo = pConfig->pData + i;
		unsigned char nAttrNum = HasAttrNumInAttrs( pAttrsAll, pInfo->type );
		/*
		if( nAttrTypeMust > 0 && pInfo->type != nAttrTypeMust )			//指定某种属性类型 且类型不匹配
		{
			continue;
		}
		
		if( nJob > 0 && pInfo->job > 0 && pInfo->job != nJob )			//指定某种职业 且该属性不是全职业 且职业不匹配
		{
			continue;
		}
		*/
		bool bIsSelfType = false;
		if( nAttrTypeMust > 0 && pInfo->type == nAttrTypeMust )					//指定了属性，且是本属性
		{
			bIsSelfType = true;		//是本属性类型
		}
		if( nAttrTypeMust > 0 && pInfo->type != nAttrTypeMust ||				//指定某种属性类型 且类型不匹配
			nAttrTypeMust > 0 && pInfo->lockMaxValue > 0 && pAttr->value >= pInfo->lockMaxValue  ||		//指定某种属性类型 且属性值大于锁定最大属性
			nJob > 0 && pInfo->job > 0 && pInfo->job != nJob  ||				//指定某种职业 且该属性不是全职业 且职业不匹配
			!bIsSelfType && pInfo->maxNum > 0 && nAttrNum >= pInfo->maxNum  	//不是本属性，且 maxNum > 0 且属性数量超过了maxNum
		)				//被过滤掉
		{
			continue;
		}
		nTotalProp += pInfo->prob;			//累积属性出现的概率
	}
	if( nTotalProp <=0 ) 
	{
		//OutputMsg(rmWaning,_T("属性的概率配置错误，总的概率为0"));
		return false;
	}
	
	unsigned int nHitValue = wrand( (unsigned int)nTotalProp ); //随机一个数出来
	INT_PTR nCalValue =0; 
	PRANDOMATTRTERM pRandAttr = NULL;
	for( INT_PTR i=0; i< nPropCount  ; i++ )
	{
		PRANDOMATTRTERM pTemp = pConfig->pData + i;				//判断第一个元素
		unsigned char nAttrNum = HasAttrNumInAttrs( pAttrsAll, pTemp->type );

		bool bIsSelfType = false;
		if( nAttrTypeMust > 0 && pTemp->type == nAttrTypeMust )					//指定了属性，且是本属性
		{
			bIsSelfType = true;		//是本属性类型
		}
		if( nAttrTypeMust > 0 && pTemp->type != nAttrTypeMust ||				//指定某种属性类型 且类型不匹配
			nAttrTypeMust > 0 && pTemp->lockMaxValue > 0 && pAttr->value >= pTemp->lockMaxValue  ||		//指定某种属性类型 且属性值大于锁定最大属性
			nJob > 0 && pTemp->job > 0 && pTemp->job != nJob  ||				//指定某种职业 且该属性不是全职业 且职业不匹配
			!bIsSelfType && pTemp->maxNum > 0 && nAttrNum >= pTemp->maxNum  	//不是本属性，且 maxNum > 0 且属性数量超过了maxNum
			)				//被过滤掉
		{
			continue;
		}

		nCalValue += pTemp->prob; 
		if( (unsigned int)nCalValue < nHitValue )
		{
			continue;
		}
		pRandAttr = pTemp;
		break;
	}
	if (!pAttr)
	{
		return false;
	}
	nPropCount = pRandAttr->valueList.count;
	nTotalProp = 0;
	for(INT_PTR i=0; i< nPropCount; i++ )
	{
		PRANDOMATTVALUE pValue  = pRandAttr->valueList+i;
		nTotalProp += pValue->prob; //概率
	}
	if( nTotalProp <=0 ) 
	{
		//OutputMsg(rmWaning,_T("极品属性值的概率配置错误，总的概率为0"));
		return false;
	}
	nCalValue =0; 
	nHitValue = wrand((unsigned int)nTotalProp); //随机一个数出来
	for(INT_PTR i=0; i< nPropCount  ; i++ )
	{
		PRANDOMATTVALUE pValue  = pRandAttr->valueList+i;
		nCalValue += pValue->prob; 
		if((unsigned int)nCalValue < nHitValue)
		{
			continue;
		}
		MakePackedAttrFromRandAttr(*pAttr, pRandAttr->type, *pValue); //精锻一个属性出来
		break;
	}
	
	return true;
}

/*
从smith属性库中抽取鉴定属性->pUserItem->smithAttrs
*/
void RandAttrSelector::GenerateItemSmithAttrsInSmith( CUserItem* pUserItem, int nSmithId, int nAttrNum, int nLockTypes[],
	unsigned char nJob)
{
	if( nAttrNum > CUserItem::MaxSmithAttrCount )    //要抽取的属性条数大于缓存容量
	{
		return;
	}

	CStdItemProvider& provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	if( nSmithId <= 0 )
	{
		return ;
	}

	DataList<RANDOMATTRTERM>* pConfig = provider.GetSmithData( nSmithId );
	if( pConfig == NULL )
	{
		return;
	}

	for( int i=0; i<nAttrNum; i++ )
	{
		int nLockType = nLockTypes[i];
		if( nLockType == enSmithLockType_No )
		{
			pUserItem->smithAttrs[i].nValue = 0;			//不锁定属性类型，属性清0，则可任意抽取属性
		}
	}

	for( int i=0; i<nAttrNum; i++ )
	{
		int nLockType = nLockTypes[i];
		GenerateOneAttrInSmithLib( pConfig, pUserItem->smithAttrs, &pUserItem->smithAttrs[i], nLockType, nJob );
	}
}

/*
从属性库中生成一条道具的属性，
可以支持不锁定、锁定、某类型属性最大数量
如果锁定属性
（1）则忽略职业要求；
（2）如果属性类型存在，则属性值变化；
（3）如果属性类型不存在，则属性类型和属性值都保持不变；
*/
void RandAttrSelector::GenerateOneAttrInSmithLib( DataList<RANDOMATTRTERM>* pConfig, PACKEDGAMEATTR* pAttrsAll, PACKEDGAMEATTR* pAttr, 
	unsigned char nLockType, unsigned char nJob )
{
	if( pConfig == NULL )
	{
		return;
	}

	//如果已经刷新有属性，并且锁定全部属性，则直接返回
	if( pAttr->nValue > 0 && nLockType == enSmithLockType_All )
	{
		return;
	}

	unsigned char nAttrTypeMust = 0;				//锁定的属性类型，0-属性类型不锁定
	if( nLockType == enSmithLockType_Type )			//锁定属性类型，属性值可以变化
	{
		nAttrTypeMust = pAttr->type;				//注意：如果pAttr->type为0，还是可以任意刷出某个属性
	}

	bool boResult = RealSmithAttribute( pConfig, pAttr, nAttrTypeMust, nJob, pAttrsAll );
	if( !boResult)		//抽取失败，当前属性不变
	{
		//OutputMsg( rmWaning, "抽取属性出错, 属性库中可能没有此属性(type=%d), 保留该属性数据不变", pAttr->type );
		//pAttr->nValue = 0;
	}

	//OutputMsg(rmWaning,"GenerateOneAttrInSmith, nLockType=%d, nAttrTypeMust=%d, nJob=%d, AttrType=%d, AttrValue=%d", 
	//	nLockType, nAttrTypeMust, nJob, pAttr->type, pAttr->value );
}

/*
从smith极品属性库中随机抽取一条属性
nLockType：锁定类型
enum enSmithLockType
{
	enSmithLockType_No,							//不锁定
	enSmithLockType_Type,						//锁定属性类型
	enSmithLockType_All,						//锁定全部（属性类型和属性值）
};
*/
void RandAttrSelector::GenerateOneAttrInSmith( int nSmithId, unsigned char& nAttrType, int& nAttrValue, unsigned char nJob,
	unsigned char nAttrTypeMust )
{
	/*如果已经刷新有属性，并且锁定全部属性，则直接返回
	if( nAttrType > 0 && nAttrValue > 0 && nLockType == enSmithLockType_All )
	{
		return;
	}
	*/

	CStdItemProvider& provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	if( nSmithId <= 0 )
	{
		return ;
	}

	DataList<RANDOMATTRTERM> * pConfig = provider.GetSmithData( nSmithId );
	if( pConfig == NULL )
	{
		return;
	}

	/*
	unsigned char nAttrTypeMust = 0;				//锁定的属性类型，0-属性类型不锁定
	if( nLockType == enSmithLockType_Type )
	{
		nAttrTypeMust = nAttrType;
	}
	*/

	PACKEDGAMEATTR smith;
	bool boResult = RealSmithAttribute( pConfig, &smith, nAttrTypeMust, nJob );
	if( !boResult)
	{
		OutputMsg( rmWaning, _T("抽取属性出错：SmithId:%d"), nSmithId );
		smith.nValue =0;
	}

	nAttrType	= smith.type;
	nAttrValue	= smith.value;
	//OutputMsg(rmWaning,"GenerateOneAttrInSmith, AttrType=%d, AttrValue=%d", nAttrType, nAttrValue );
}

unsigned char RandAttrSelector::HasAttrNumInAttrs( PACKEDGAMEATTR* pAttrsAll, unsigned char nAttrType )
{
	if( !pAttrsAll )
	{
		return 0;
	}
	unsigned char nAttrNum = 0;
	for( INT_PTR i = 0; i < CUserItem::MaxSmithAttrCount; i++ )
	{
		if( pAttrsAll[i].nValue && pAttrsAll[i].type == nAttrType)
		{
			nAttrNum += 1;
		}
	}
	return nAttrNum;
}
