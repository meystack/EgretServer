#include "StdAfx.h"
#include "GameStore.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CGameStore::CGameStore()
	:Inherited()
{
	
}

CGameStore::~CGameStore()
{
	m_Merchands.empty();
	m_refreshConfig.empty();
}

bool CGameStore::LoadGameStore(LPCTSTR sFilePath)
{
	bool result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	//定义一个临时的商品表，将商品读取到此临时表。如果最终读取完成则拷贝到商品表中
	CVector<Merchandise> merchandList;
	
	try
	{
		//加载商城配置文件
		if (ms.loadFromFile(sFilePath) <= 0)
		{
			showErrorFormat(_T("unable to load GameStore config %s "), sFilePath);
			return false;
		}
		//对配置文件进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置配置脚本
		if ( !sText || !setScript(sText) )
		{
			showError(_T("parse config error"));
			return false;
		}
		//读取配置
		result = ReadShopConfig();//ReadConfig(merchandList);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load GameStore config"));
	}

	//如果读取配置成功则将临时商品表中的商品添加到我的商品列表中
	if (result)
	{
		m_Merchands.clear();
		m_Merchands.addList(merchandList);
		merchandList.empty();
	}
	//清空脚本内存
	setScript(NULL);
	return result;
}

bool CGameStore::LoadGameRefresh(LPCTSTR sFilePath)
{
	bool result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	
	//定义一个临时的商品表，将商品读取到此临时表。如果最终读取完成则拷贝到商品表中
	CVector<MerchanRefresh> refreshList; //商品的刷新

	try
	{
		//加载商城配置文件
		if (ms.loadFromFile(sFilePath) <= 0)
		{
			showErrorFormat(_T("unable to load GameRfreshStore config %s "), sFilePath);
			return false;
		}
		//对配置文件进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置配置脚本
		if ( !sText || !setScript(sText) )
		{
			showError(_T("parse config error"));
			return false;
		}
		//读取配置
		result = ReadRefreshConfig(refreshList);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load GameRefresh config"));
	}

	//如果读取配置成功则将临时商品表中的商品添加到我的商品列表中
	if (result)
	{
		m_refreshConfig.clear();
		m_refreshConfig.addList(refreshList);
		refreshList.empty();
	}
	//清空脚本内存
	setScript(NULL);
	return result;
}

bool CGameStore::ReadRefreshConfig(CBaseList<MerchanRefresh> &refreshList)
{
	if (!openGlobalTable("RefreshStore"))
		return false;
	//遍历刷新的列表
	
	m_labelCount.clear();
	INT_PTR nLabelId=0;
	if ( enumTableFirst() )
	{
		MerchanRefresh md;
		
		int nDef =-1;
		do 
		{
			nLabelId ++;
			int nCountLimit = getFieldInt("count");
			m_labelCount.add(nCountLimit);

			if ( openFieldTable("items") )
			{
				
				if ( enumTableFirst() )
				{
					do 
					{
						md.nMerchandiseId = getFieldInt("id");

						if(GetMerchandise(md.nMerchandiseId) ==NULL)
						{
							OutputMsg(rmError,_T("store refresh data, id=%d is invalid"),md.nMerchandiseId );
							continue;
						}
						md.nCount = getFieldInt("count");
						
						nDef =-1;
						md.nWeekDay  = (BYTE)getFieldInt("weekDay",&nDef);

						md.bMonth = (BYTE)getFieldInt("month",&nDef);

						md.bDay = (BYTE)getFieldInt("day",&nDef);


						nDef = 0;
						md.nPercent = getFieldInt("percent",&nDef);

						md.nOpenServerDay = (BYTE)getFieldInt("openServerDay",&nDef);

						md.nMergeServerDay= (BYTE)getFieldInt("mergeServerDay",&nDef);

						md.nBeforeMergeDay= (BYTE)getFieldInt("beforeMergeDay",&nDef);

						md.nLabelId = (BYTE)nLabelId;
						
						
						refreshList.add(md);
					}
					while (enumTableNext());
				}
				closeTable();
			}
		} while (enumTableNext());
		
	}
	closeTable();
	return true;

}

const CGameStore::PMERCHANDISE  CGameStore::GetMerchandise(const INT_PTR nId)const
{
	INT_PTR i;
	for (i=m_Merchands.count()-1; i>-1; --i)
	{
		if (m_Merchands[i].nId == nId)
		{
			return &m_Merchands[i];
		}
	}
	return NULL;
}
const CGameStore::PMERCHANDISE CGameStore::GetShopItem( WORD wItemId) const
{
	INT_PTR i;
	for (i=m_Merchands.count()-1; i>-1; --i)
	{
		if (m_Merchands[i].wItemId == wItemId)
		{
			return &m_Merchands[i];
		}
	}
	return NULL;
}

bool CGameStore::ReadConfig(CBaseList<Merchandise> &merchandList)
{
	//打开商城配置表
	if( !openGlobalTable("GameStore") )
	{
		return false;
	}
	LPCSTR sSPID = GetLogicServer()->GetVSPDefine().GetDefinition(CVSPDefinition::SPID);
	//遍历商城分类表
	if( enumTableFirst() )
	{
		Merchandise md;
		//获取当前的SPID
		
		int nDefault=0;

		m_Merchands.clear();
		
		int nDef =0;
		int nCountDef = 1;		//缺省每次购买1个物品
		int nJobDef = 0;		//无职业要求
		int nSexDef	= -1;		//无性别要求
		do 
		{
			//标签的id
			int labelId			=  getFieldInt("labelId",&nDefault); //分类的id
			int nTotalCount		=  getFieldInt("totalBuyLimit",&nDefault);  //最大能够购买的数目
			bool bDefault		= false;

			//是否是动态添加的
			bool bDynamicAdd	= getFieldBoolean("dynamicAdd",&bDefault); 

			//打开商品分类中的商品表
			if ( openFieldTable("items") )
			{
				//遍历单个商品分类中的所有商品
				if ( enumTableFirst() )
				{
					
					do 
					{
						nDef =0;
						md.nId					= getFieldInt("id");		//销售ID，商城中的任意商品必须都不一样
						md.wItemId				= getFieldInt("item");
						md.wItemCount			= getFieldInt("count", &nCountDef);
						md.bQuality				= (BYTE)getFieldInt("quality",&nDef);
						md.wQualityDataIndex	= (WORD)getFieldInt("qualityDataIndex",&nDef);
						md.bStrong				= (BYTE)getFieldInt("strong",&nDef);
						md.bLabelId				= (BYTE) labelId;							//分类的ID
						md.wLabelBuyLimit		= (WORD) nTotalCount;						//整个分类能买多少个
						md.bSaleLevel			= (BYTE)getFieldInt("saleLevel",&nDef);
						md.nJob					= (int)getFieldInt("job",&nJobDef);			//职业要求
						md.nSex					= (int)getFieldInt("sex",&nSexDef);			//性别要求
						md.bUse					= (BYTE)getFieldInt("use",&nDef);			// 立即使用要求
						md.bDynamicAdd			= bDynamicAdd;								//是否是动态添加的
						int nSingleBuyLimit		= getFieldInt("singleBuyLimit",&nDefault);  //单个商品每天购买的数目

						
						md.bSingleBuyLimit		= (WORD)nSingleBuyLimit; //单个商品的数目限制

						md.dFreshBuyLimit		= (WORD)getFieldInt("freshBuyLimit",&nDefault); 

						//读取物品的价格，如果价格有效则添加到商品表中
						if( ReadMerchandisePrice(sSPID, md) && md.dwPrice >= 0 )
						{
							//如果已经存在同ID的物品则报错
							for(int i=0; i < merchandList.count(); ++i)
							{
							     if( merchandList[i].nId == md.nId )
								 {
								     showErrorFormat(_T("store  id %d already exists"), md.nId);
									 break;
								 }
							}				
							merchandList.add(md);
						}
						else
						{
							OutputMsg(rmError,_T("store id =%d, price error"),md.nId );
						}
					} 
					while (enumTableNext());
				}
				closeTable();
			}
		} 
		while (enumTableNext());
	}
	closeTable();
	return true;
}

bool CGameStore::ReadMerchandisePrice(LPCSTR sCurrSPID, Merchandise &merchandise)
{
	//如果价格配置表不存在则返回false
	if ( !feildTableExists("price") )
		return false;
	//打开价格配置表
	if ( openFieldTable("price") )
	{
		LPCSTR sSPID;
		UINT dwPrice;			//原价

		//遍历价格配置表
		if (enumTableFirst())
		{
			merchandise.dwPrice		= 0;		//将价格标记为0以便不出售
			do 
			{
				sSPID = getFieldString("spid");			//运营商ID，不同的运营商，可能商品价格不同
				if ( *(PWORD)sSPID == '*' || !strcmp(sSPID, sCurrSPID) )
				{
					merchandise.btDealGoldType = getFieldInt("type");			//货币类型（0-金币，2-绑定元宝，3-元宝）
					//验证交易币种类型配置是否有效，如果无效则报错（这里从原货币类型改为奖励类型）
					if( merchandise.btDealGoldType < qatEquipment || merchandise.btDealGoldType >= qatAwardTypeCount )
						showErrorFormat(_T("invalid dealType value %d of merchandise %d"), merchandise.btDealGoldType, merchandise.nId);
					dwPrice		=  getFieldInt("price");							//原价
					if( *(PWORD)sSPID != '*' || merchandise.dwPrice == 0 )
					{
						merchandise.dwPrice = dwPrice;
						merchandise.boBind  = getFieldBoolean("bind");
					}
				}
			} 
			while (enumTableNext());
		}
		closeTable();
	}
	return true;
}

void CGameStore::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[GameStore]");
	s += sError;
	throw s;
}



bool CGameStore::LoadDynamicData()
{
	
	m_dynamicMerchands.LoadFromFile("./data/runtime/store/dynamicMerchans.rtd"); //动态的商品
	m_consumerMerchands.LoadFromFile("./data/runtime/store/consumeMerchans.rtd"); //玩家消耗的商品
	m_refreshTime.LoadFromFile("./data/runtime/store/refreshTime.rtd"); //刷新时间
	
	m_YBConsumeRank.LoadTodayConsumeRankFile();
	m_YBConsumeRank.LoadYesterdayConsumeRank();

	//如果保存的时间过期了，那么就需要刷新
	unsigned int nCurrentTime = CMiniDateTime::now();
	for(INT i=1;i <= m_labelCount.count(); i++)
	{
		unsigned int nLastRefTime=(unsigned int) GetRefreshTime(i);
		if(nLastRefTime + REFRESH_TIME_INTERVAL <= nCurrentTime ) //这里表示已经过期，可以继续刷新了
		{
			ResetDynamicMerchadise(i);
		}
	}
	return true; 
}


bool CGameStore::SaveDynamicData()
{	
	OutputMsg(rmTip,_T("Saving Store dynamic data"));
	m_dynamicMerchands.SaveToFile("./data/runtime/store/dynamicMerchans.rtd"); //动态的商品
	m_consumerMerchands.SaveToFile("./data/runtime/store/consumeMerchans.rtd"); //玩家消耗的商品
	m_refreshTime.SaveToFile("./data/runtime/store/refreshTime.rtd"); //刷新时间
	//m_YBConsumeRank.SaveTodayConsumeRankToFile();
	//m_YBConsumeRank.SaveYesterdayConsumeRankToFile();

	OutputMsg(rmTip,_T("Save Store dynamic data finish"));
	return true;
}

void CGameStore::ResetLabelConfigData(INT nLabelID)
{
	if(nLabelID <=0 || nLabelID > m_labelCount.count())
	{
		OutputMsg(rmError,_T("%s,invalid labelid=%d"),__FUNCTION__,nLabelID);
		return;
	}
	INT_PTR nCountLimit = m_labelCount[nLabelID-1]; //该分类的数目的限制
	if(nCountLimit <=0 ) return ;
	//这里首先判断是开服以来的第几天
	int nDays= GetLogicServer()->GetDaysSinceOpenServer(); //开服以来的第几天
	if(nDays ==0 )
	{
		nDays =1;
	}
	//是合服以来的第几天
	int nMergeServerDay =GetLogicServer()->GetDaysSinceCombineServer();
	//是合服前第几天
	int nBeforeMergeDay = GetGlobalLogicEngine()->GetMiscMgr().GetDaysBeforeCombineServer();
	if(nBeforeMergeDay <=0)
	{
		nBeforeMergeDay =-2;
	}
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime); //获取当前的时间
	BYTE nCurrentMonth=(BYTE) sysTime.wMonth ; //月份
	BYTE nCurrentDay = (BYTE) sysTime.wDay ; //天

	INT_PTR nFindCount=0; //已经找到的数目
	CVector<MerchanRefresh> data;
	INT_PTR nOpenServerCount=0; // 开服第几天的数目为0

	//优先满足开服，然后满足日期,最后按星期筛选
	for( INT_PTR i=0; i < m_refreshConfig.count() ; i++)
	{
		if( m_refreshConfig[i].nLabelId == nLabelID) //如果称号
		{
			//openServerDay 开服的第几天 ,1表示开服第1天，2第2天，0表示不做开服的限制  
			BYTE bValue= m_refreshConfig[i].nOpenServerDay; //开服以来的第几天
			if( bValue != (BYTE)0) //这里优先开服的天数
			{
				if(  bValue == nDays )
				{
					data.insert(0,m_refreshConfig[i]); 
					nOpenServerCount ++;
				}
				continue;
			}
			BYTE bMonth =  m_refreshConfig[i].bMonth; //月
			BYTE bDay =  m_refreshConfig[i].bDay;//日

			//有月份或者日的的限制的
			if( bMonth  != BYTE(-1) || bDay != (BYTE)-1 )
			{
				//满足了日期了
				if( (bMonth == nCurrentMonth || bMonth == BYTE(-1)) &&
					(bDay == nCurrentDay || bDay == BYTE(-1)) 
					)
				{
					data.insert(nOpenServerCount,m_refreshConfig[i]);  //在后面添加
					continue;
				}
			}
			else
			{
				//weekDay表示星期几, -1表示不限制星期几，0表示周日，1表示周1 ,...6表示周6 

				if(m_refreshConfig[i].nMergeServerDay !=0)
				{
					if(nMergeServerDay == m_refreshConfig[i].nMergeServerDay)
					{
						data.add(m_refreshConfig[i]);  //在后面添加
						continue;
					}
				}
				else
				{
					if( m_refreshConfig[i].nBeforeMergeDay !=0)
					{
						if (nBeforeMergeDay > 0 && (m_refreshConfig[i].nBeforeMergeDay == BYTE(-1) || nBeforeMergeDay == m_refreshConfig[i].nBeforeMergeDay))
						{
							data.add(m_refreshConfig[i]);  //在后面添加
							continue;
						}
						
					}
					else
					{
						bValue = m_refreshConfig[i].nWeekDay ;
						if(bValue == BYTE(-1) || bValue == sysTime.wDayOfWeek)
						{
							data.add(m_refreshConfig[i]);  //在后面添加
						}
					}
				
				}

			}
		}
	}
	INT_PTR nCount =0;
	for(INT_PTR i=0; i < data.count(); i ++ )
	{
		unsigned int nRandom = GetGlobalLogicEngine()->GetRandValue(10000) + 1;
		if (((unsigned)data[i].nPercent) == 0 || nRandom <= ((unsigned)data[i].nPercent))
		{
			AddDynamicMerchadise(data[i].nMerchandiseId, data[i].nCount);//添加道具进去
			OutputMsg(rmTip,_T("Store add dynamic merchandise,id=%d,count=%d"),data[i].nMerchandiseId,data[i].nCount);

			nCount++;
			if(nCount >= nCountLimit)
			{
				break;
			}
		}

	}

}
void  CGameStore::ResetDynamicMerchadise(INT nLabel,bool bNeedBroadcast)
{
	
	CGameStore&  store= GetLogicServer()->GetDataProvider()->GetGameStore();

	CGameStore::PMERCHANDISE pMerchandise ;
	INT_PTR nCount = m_dynamicMerchands.RankCount();
	//将动态物品列表里该分类的数据全部删除
	for( INT_PTR i= nCount-1; i >=0;  i--)
	{
		CGameStoreRank::PSALADATA  pData = &m_dynamicMerchands[i];
		
		//获得商品的指针
		pMerchandise= store.GetMerchandise(pData->nId); 
		if(pMerchandise)
		{
			if(pMerchandise->bLabelId == (BYTE)nLabel )
			{
				m_dynamicMerchands.RemoveIndex(i); 
				m_dynamicMerchands.SetDataChange();
			}
		}
		else //如果这个道具已经失效了，直接删除
		{
			m_dynamicMerchands.RemoveIndex(i); 
			m_dynamicMerchands.SetDataChange();
		}
	}

	//删除玩家购买列表里这个分类的数据
	nCount = m_consumerMerchands.RankCount();
	for( INT_PTR i= nCount-1; i >=0;  i--)
	{
		CGameStoreRank::PSALADATA  pData = &m_consumerMerchands[i];
		pMerchandise= store.GetMerchandise(pData->nId); 
		if(pMerchandise)
		{
			if(pMerchandise->bLabelId == (BYTE)nLabel )
			{
				m_consumerMerchands.RemoveIndex(i); 
				m_consumerMerchands.SetDataChange();
			}
		}
		else //如果这个道具已经失效了，直接删除
		{
			m_consumerMerchands.RemoveIndex(i); 
			m_consumerMerchands.SetDataChange();
		}
	}
	//更新刷新的时间
	UpdateRefreshTime(nLabel);

	//重置该分类的数据
	ResetLabelConfigData (nLabel);

	// //如果需要广播的话,广播一个分类的数据发生了改变
	// if(bNeedBroadcast)
	// {
	// 	CGameStoreBuySystem::BroadLabelRefresh(nLabel);
	// }
}
void CGameStore::UpdateRefreshTime(INT nLabelId)
{
	bool isExist; 
	UINT nTime=(UINT) m_refreshTime.GetIdCount(nLabelId,isExist); //时间
	unsigned int nCurrentTime= CMiniDateTime::now();
	if(isExist) //存在这个分类
	{
		m_refreshTime.UpdateSelaCount(nLabelId,(INT)nCurrentTime);
		
	}
	else
	{
		if(nTime > nCurrentTime) 
		{
			m_refreshTime.UpdateSelaCount(nLabelId,(INT)(nTime - nCurrentTime));
		}
		else if(nTime <= nCurrentTime)
		{
			m_refreshTime.UpdateSelaCount(nLabelId,(INT)(nCurrentTime - nTime));
		}
	}
	
}

unsigned int CGameStore::GetRefreshTime(INT nLableId)
{
	bool isExist; 

	INT nValue= m_refreshTime.GetIdCount(nLableId,isExist);
	if(isExist)
	{
		return (unsigned int)nValue;
	}
	else
	{
		return 0;
	}
}

//获取角色的消费数据
CVector<CGameStoreRank::SaleData>& CGameStore::GetActcorConsumeData(INT nActorId)
{
	m_actorConsumeList.clear();
	
	for(INT_PTR i=0;i < m_consumerMerchands.RankCount(); i++)
	{
		CGameStoreRank::SaleData * pSale =& m_consumerMerchands[i];
		if(pSale->nActorId ==nActorId)
		{
			m_actorConsumeList.add(*pSale);
		}
	}
	return m_actorConsumeList;
}




bool CGameStore::ReadShopConfig()
{
	//打开商城配置表
	m_shops.clear();
	if( !openGlobalTable("ShopConfig") )
	{
		return false;
	}
	//遍历商城分类表
	if( enumTableFirst() )
	{
		int nDef =0;
		do 
		{
			if( enumTableFirst() )
			{
				do 
				{
					if( enumTableFirst() )
					{
						do 
						{
							
							//标签的id
							int shoptype			=  getFieldInt("shoptype"); //分类的id
							int nTabshop =  getFieldInt("Tabshop",&nDef);  //
							ShopConfig shop;
							shop.nShopId=  getFieldInt("shopid",&nDef);  //
							shop.nLimitLv=  getFieldInt("levellimit",&nDef);  //
							shop.nReincarnationlimit=  getFieldInt("reincarnationlimit",&nDef);  //
							shop.nFlag=  getFieldInt("flag",&nDef);  //
							shop.nDaylimit=  getFieldInt("daylimit",&nDef);  //
							shop.nStaticType=  getFieldInt("statictype",&nDef);  //
							shop.nGuildLevelLimit = getFieldInt("guildLevel",&nDef);
											
							// 购买商品广播消息
							if(isExistString("tips"))
							{
								getFieldStringBuffer("tips", shop.tips, ArrayCount(shop.tips) );
							} else {
								ZeroMemory( shop.tips, sizeof(shop.tips) );
							}

							//打开商品分类中的商品表
							if ( feildTableExists("shop") &&  openFieldTable("shop"))
							{
								shop.shop.nType = getFieldInt("type", &nDef);
								shop.shop.nItemId = getFieldInt("id", &nDef);
								shop.shop.nCount = getFieldInt("count", &nDef);
								closeTable();
							}

							//打开商品分类中的商品表
							if ( feildTableExists("npc") &&  openFieldTable("npc"))
							{
								if ( enumTableFirst() )
								{
									do
									{
										int npcId = (int)getFieldInt(NULL);
										shop.nNpc.push_back(npcId);

									}while(enumTableNext());
								}
								closeTable();
							}

							if( feildTableExists("price") && openFieldTable("price"))
							{
								shop.price.nType = getFieldInt("type", &nDef);
								shop.price.nId = getFieldInt("id", &nDef);
								shop.price.nCount = getFieldInt("count", &nDef);
								closeTable();
							}
							shop.buyLimit.clear();
							if( feildTableExists("buytype") && openFieldTable("buytype"))
							{
								if ( enumTableFirst() )
								{
									do
									{
										ShopBuyType buyCfg;
										buyCfg.nType = getFieldInt("type", &nDef);
										buyCfg.limits.clear();
										if(feildTableExists("limit") && openFieldTable("limit"))
										{
											if(enumTableFirst())
											{
												do
												{
													int nValue = getFieldInt("value",&nDef );
													int nTimes = getFieldInt("times",&nDef );
													buyCfg.limits[nValue] = nTimes;
												}while(enumTableNext());
											}
											closeTable();
										}
										shop.buyLimit.push_back(buyCfg);
									}while(enumTableNext());
								}
								closeTable();
							}
							m_shops[shoptype][nTabshop].push_back(shop);
						}while(enumTableNext());
					}

				}while(enumTableNext());

			}

		}while(enumTableNext());
	}
	closeTable();
	return true;
}

int CGameStore::getLimitTimes(std::vector<ShopBuyType>&buyLimit, CActor* pActor, int nOpenDay)
	{
		int times = -1;
		if(buyLimit.size() <= 0)
			return 0;
		std::vector<ShopBuyType>::iterator it = buyLimit.begin();

		for(;it != buyLimit.end(); it++)
		{
			ShopBuyType& infoCfg = *it;
			int nValue = 0;
			switch (infoCfg.nType)
			{
			case BuyShopLimitLevel:
				if(pActor)
				{
					nValue = pActor->GetLevel();
				}
				/* code */
				break;
			case BuyShopLimitCircle:
				if(pActor)
				{
					nValue = pActor->GetCircleLevel();
				}
				/* code */
				break;
			case BuyShopLimitOpenServerDay:
				nValue = nOpenDay;
				/* code */
				break;
			default:
				break;
			}
			if(infoCfg.limits.size() <= 0)
				continue;
			std::map<int, int>::iterator ot = infoCfg.limits.begin();
			for(; ot != infoCfg.limits.end(); ot++)
			{
				if(nValue >= ot->first /*&& ot->second > times */)
				{
					times = ot->second;
				}
			}

		}
		return times;
	}