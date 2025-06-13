#include "StdAfx.h"
#include "GuildProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CGuildProvider::CGuildProvider()
	: Inherited(), m_DataAllocator(_T("GuildDataAlloc")), m_DataAward(_T("GuildAwardAlloc"))
{
	nMaxLevel = 0;
}

CGuildProvider::~CGuildProvider()
{
}

bool CGuildProvider::LoadGuildConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on Guild config"));
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CGuildProvider::readConfigs()
{
	if ( !openGlobalTable("GuildConfig") )
		return false;
	int nDef = 0;
	//读全局变量
	nLevelLimit = getFieldInt("levelLimit");
	//nCreateNeedItem = getFieldInt("createNeedItem");
	nCreateNeedYb = getFieldInt("createNeedYb");
	nAwardGuildMoney = getFieldInt("awardGuildMoney");

	nLeftTimeLimit = getFieldInt("leftTimeLimit");
	nImpeachcost = getFieldInt("impeachcost", &nDef);
	nLeftTimeLimit = nLeftTimeLimit * 3600;//配置文件是以小时为单位保存，实际使用是按秒

	// nCoinLimit = getFieldInt("coinLimit");
	// nAddCoinItemId = getFieldInt("addCoinItemId");
	// nMinGuildCoin = getFieldInt("minGuildCoin");
	// nNoticeGuildCoin = getFieldInt("noticeGuildCoin");
	// nDecGuildCoin = getFieldInt("decGuildCoinPerDay");

	//nTipsLevel = getFieldInt("tipsLevel");
	// if (!openFieldTable("tipsLevel"))
	// {
	// 	OutputMsg(rmError,_T("装载战绩配置失败，找不到GuildConfig.tipsLevel"));
	// 	return false;
	// }

	// if (enumTableFirst())		//
	// {
	// 	INT_PTR nIdx = 0;
	// 	do 
	// 	{
	// 		if( nIdx < 5 )
	// 		{
	// 			nTipsLevel[nIdx] = (int)getFieldInt(NULL);
	// 			nIdx++;
	// 		}
	// 	} while (enumTableNext());
	// }
	// closeTable(); //-- tipsLevel


	// nHelpNeedCoin = getFieldInt("HelpNeedCoin");


	// nSbkOwnerTitleId = getFieldInt("sbkOwnerTitleId");
	// nSbkMemberTitleId = getFieldInt("sbkMemberTitleId");



	// nGuildCoinGxRate = getFieldInt("GuildCoinGxRate"); 


	/*不需要了
	nGuildLeaderLogoutDay = getFieldInt("LeaderImpeachTime");
	nGuildLeaderImpeachHour = getFieldInt("LeaderImperchaHour");
	nGuildLeaderImpeachMin = getFieldInt("LeaderImperchaMin");
	*/

	// nRewardCoffie = getFieldInt("RewardCoffie");
	// nCallNeedCoin = getFieldInt("callNeedCoin");
	// nCallDoneTime = getFieldInt("callDoneTime");

	int nDeufatleId = 0;


	nProtectDay = getFieldInt("protectDay",&nDeufatleId);		//职位保护期
	nBuildNum   = getFieldInt("buildnum", &nDeufatleId);//建筑数量
	// m_GuildRelation.nUnionNeedCoin		= getFieldInt("unionNeedCoin");			//申请行会联盟
	m_GuildRelation.nDeclareLastTime	= getFieldInt("warLastTime", &nDeufatleId) * 3600;	//选择时限，单位：小时
	m_GuildRelation.nDeclareNeedCoin	= getFieldInt("declareWarNeedCoin", &nDeufatleId);
	nApproval	= getFieldInt("Approval", &nDeufatleId);
	// 	m_GuildRelation.nBidMoney			= getFieldInt("bidMoney");
	//closeTable();

	// if (feildTableExists("guildRelation") && openFieldTable("guildRelation"))
	// {
	// 	m_GuildRelation.nUnionNeedCoin		= getFieldInt("unionNeedCoin");			//申请行会联盟
	// 	m_GuildRelation.nDeclareLastTime	= getFieldInt("warLastTime") * 3600;	//选择时限，单位：小时
	// 	m_GuildRelation.nDeclareNeedCoin	= getFieldInt("declareWarNeedCoin");
	// 	m_GuildRelation.nBidMoney			= getFieldInt("bidMoney");
	// 	closeTable();
	// }

	// if (feildTableExists("guildUpgrade") && openFieldTable("guildUpgrade"))
	// {
	// 	m_GuildUpgrade.m_nUpgradeItem1 = getFieldInt("upgradeItem1");
	// 	m_GuildUpgrade.m_nUpgradeItem2 = getFieldInt("upgradeItem2");
	// 	m_GuildUpgrade.m_nUpgradeItem3 = getFieldInt("upgradeItem3");
	// 	closeTable();
	// }

	

	// if (feildTableExists("guildLevel") && openFieldTable("guildLevel"))
	// {
	// 	size_t nCount = lua_objlen(m_pLua,-1);
	// 	//循环读取等级的配置
	// 	if ( enumTableFirst() )
	// 	{
	// 		INT_PTR nIdx = 1;
	// 		CDataAllocator dataAllocator;
	// 		pLevelList = (GuildLevelConfigList*)dataAllocator.allocObjects(sizeof(*pLevelList));
	// 		pLevelList->nCount = (int)nCount;
	// 		pLevelList->pList = (GuildLevelConfig*)dataAllocator.allocObjects(sizeof(GuildLevelConfig)*nCount);
	// 		do 
	// 		{
	// 			//读取每个等级的配置
	// 			GuildLevelConfig &item = pLevelList->pList[nIdx-1];
	// 			item.nLevel = getFieldInt("level");
	// 			//如果该物品的ID不是期望的值，则输出错误并终止读取
	// 			if (item.nLevel != nIdx)
	// 			{
	// 				showErrorFormat(_T("unexpected Guild Level %d, request %d"), item.nLevel, nIdx);
	// 				endTableEnum();
	// 				break;
	// 			}
	// 			if (item.nLevel > nMaxLevel)
	// 			{
	// 				nMaxLevel = item.nLevel;
	// 			}
	// 			nIdx++;

	// 			//
	// 			item.nMaxSecLeader	= getFieldInt("maxFBZ");
	// 			item.nMaxTz			= getFieldInt("maxTZ");
	// 			item.nMaxMember		= getFieldInt("maxMember");
	// 			item.nNeedYb		= getFieldInt("needYb");
	// 			item.nWeekDecCoin	= getFieldInt("weekDecCoin");
	// 			item.nFruitLevel	= getFieldInt("fruitLevel");
	// 			item.nDepotPage		= getFieldInt("depotPage");
	// 		}
	// 		while (enumTableNext());

	// 		//原来的内存块释放到
	// 		m_DataAllocator.~CObjectAllocator();
	// 		//将临时的内存块复制到正式的
	// 		m_DataAllocator = dataAllocator;
	// 		//清空临时的
	// 		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	// 	}
	// 	closeTable();
	// }

	if (feildTableExists("dailyDonateLimit") && openFieldTable("dailyDonateLimit"))
	{
		/*
		size_t nCount = lua_objlen(m_pLua, -1);
		if (nCount != MAXMAINHALLLEVEL)
		{
			OutputMsg(rmError, _T("%s:行会当日最大可捐献资金配置数量%d与主殿最高等级%d不匹配"), __FUNCTION__, nCount, MAXMAINHALLLEVEL);
			return false;
		}
		*/
		if (enumTableFirst())
		{
			do 
			{	
				DailyDonateLimit stDailyDonateLimit;
				stDailyDonateLimit.m_nCoinLimit = (int)getFieldInt("coinLimit");
				stDailyDonateLimit.m_nItemLimit = (int)getFieldInt("itemLimit");
				m_DailyDonateLimitList.add(stDailyDonateLimit);
				//m_nMaxMainHallCoin[nIdx++] = getFieldInt(NULL);
			} while (enumTableNext());
		}
		closeTable();
	}
	if (feildTableExists("rankAward") && openFieldTable("rankAward"))
	{
		/*
		size_t nCount = lua_objlen(m_pLua, -1);
		if (nCount != MAXMAINHALLLEVEL)
		{
			OutputMsg(rmError, _T("%s:行会当日最大可捐献资金配置数量%d与主殿最高等级%d不匹配"), __FUNCTION__, nCount, MAXMAINHALLLEVEL);
			return false;
		}
		*/
		// size_t nCount = lua_objlen(m_pLua, -1);
		// if (nCount > 10)
		// {
		// 	return false;
		// }
		if (enumTableFirst())
		{
			do 
			{	
				CSSBKGUILDRANK cfg;
				if (feildTableExists("awards") && openFieldTable("awards"))
				{
					if(enumTableFirst()) {
						do {
							ACTORAWARD award;
							award.btType = (int)getFieldInt("type");
							award.wId = (int)getFieldInt("id");
							award.wCount = (int)getFieldInt("count");
							cfg.awards.emplace_back(award);
						}while(enumTableNext());
					}
					closeTable();
				}
				cfg.nValue = (int)getFieldInt("value");
				sbkguild.emplace_back(cfg);
			} while (enumTableNext());
		}
		closeTable();
	}
	if (feildTableExists("czrewards") && openFieldTable("czrewards"))
	{
		if(enumTableFirst()) {
			do {
				ACTORAWARD award;
				award.btType = (int)getFieldInt("type");
				award.wId = (int)getFieldInt("id");
				award.wCount = (int)getFieldInt("count");
				czrewards.emplace_back(award);
			}while(enumTableNext());
		}
		closeTable();
	}

	memset(czrewardsMailTT,0,sizeof(czrewardsMailTT));
	getFieldStringBuffer(("czrewardsMailTT"), czrewardsMailTT,sizeof(czrewardsMailTT));

	memset(czrewardsMailCT,0,sizeof(czrewardsMailCT));
	getFieldStringBuffer(("czrewardsMailCT"), czrewardsMailCT,sizeof(czrewardsMailCT));
	memset(rankAwardMailTT,0,sizeof(rankAwardMailTT));
	getFieldStringBuffer(("rankAwardMailTT"), rankAwardMailTT,sizeof(rankAwardMailTT));
	memset(rankAwardMailCT,0,sizeof(rankAwardMailCT));
	getFieldStringBuffer(("rankAwardMailCT"), rankAwardMailCT,sizeof(rankAwardMailCT));

	CMiniDateTime now_time = CMiniDateTime::now();
	int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
	char buff[50];
	if (feildTableExists("guilClose") && openFieldTable("guilClose"))
	{

		m_GuildClose.nTimeType = getFieldInt("TimeType",&nDef);
		
		// 合服次数达到的才开启
        m_GuildClose.nHefuTimes = getFieldInt("HefuTimes",&nDef);
        if (!CheckHefuTimes(m_GuildClose))
        {
			m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;
        }

		m_GuildClose.nAfterSrvDay = getFieldInt("AfterSrvDay",&nDef);

		// 开服前N天开，如果过了这个时间，永远不开
		m_GuildClose.nBeforeSrvDay = getFieldInt("BeforeSrvDay",&nDef);
		if (m_GuildClose.nBeforeSrvDay && nOpenServerDay > m_GuildClose.nBeforeSrvDay)
        {
			m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;
        }

		if (isExistString("RealTimeLt")) {
            getFieldStringBuffer("RealTimeLt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClose.nRealTimeLt);
        }else m_GuildClose.nRealTimeLt = 0;

		// 小于这个时间的才开启
        if (!CheckOpenTimeLT(m_GuildClose))
        {
            m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;       
        }

		if (isExistString("RealTimeGt")) {
            getFieldStringBuffer("RealTimeGt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClose.nRealTimeGt);
        }else m_GuildClose.nRealTimeGt = 0;

		// 大于这个时间的才开启
        if (!CheckOpenTimeGt(m_GuildClose))
        {
            m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;       
        }

		if (isExistString("HefuTimeLt")) {
            getFieldStringBuffer("HefuTimeLt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClose.nHefuTimeLt);
        }else m_GuildClose.nHefuTimeLt = 0;

		// 合服时间小于这个的才开启
        if (!CheckHefuTimeLt(m_GuildClose))
        {
            m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;       
        }

		if (isExistString("HefuTimeGt")) {
            getFieldStringBuffer("HefuTimeGt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClose.nHefuTimeGt);
        }else m_GuildClose.nHefuTimeGt = 0;

		// 合服时间大于这个的才开启
        if (!CheckHefuTimeGt(m_GuildClose))
        {
            m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;       
        }

		m_GuildClose.nIsOpen = getFieldInt("isOpen",&nDef);

		// 详细行会关闭时间
		if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
		{
			int nCount = (int)lua_objlen(m_pLua,-1);
			m_GuildClose.pTimeDetail = (OneTimePair*)m_DataAward.allocObjects(sizeof(OneTimePair) * nCount);
			for (size_t i = 1; i <= nCount; i++)
			{
				getFieldIndexTable(i);
				getFieldStringBuffer("StartTime",m_GuildClose.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
				getFieldStringBuffer("EndTime",m_GuildClose.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
				closeTable();
			}
			m_GuildClose.nTimeCount = nCount;
			m_GuildClose.nTimeIdx = 0;
			closeTable();	
		}

		// 计算时间（若算不到时间，说明行会一直开启）
		if (!UpdateTime(m_pLua, m_GuildClose))
		{
			m_GuildClose.nStartTime = 0;
			m_GuildClose.nEndTime = 0;
		}

		// 活动时间类型为非循环时间的活动，需要结束时间大于当前服务器时间，才说明可以开启
        /*
        	4:创角时间
            5:开服时间
        */
        if (m_GuildClose.nTimeType != 4 && m_GuildClose.nTimeType != 5 && m_GuildClose.nTimeType != 3 )
        {
            if ((unsigned int)m_GuildClose.nEndTime != 0 && (unsigned int)m_GuildClose.nEndTime <= now_time)
            {
                m_GuildClose.nStartTime = 0;
				m_GuildClose.nEndTime = 0;
        	}
        }

		closeTable();
	}

	if (feildTableExists("guildClear") && openFieldTable("guildClear"))
	{

		m_GuildClear.nTimeType = getFieldInt("TimeType",&nDef);
		
		// 合服次数达到的才开启
        m_GuildClear.nHefuTimes = getFieldInt("HefuTimes",&nDef);
        if (!CheckHefuTimes(m_GuildClear))
        {
			m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;
        }

		m_GuildClear.nAfterSrvDay = getFieldInt("AfterSrvDay",&nDef);

		// 开服前N天开，如果过了这个时间，永远不开
		m_GuildClear.nBeforeSrvDay = getFieldInt("BeforeSrvDay",&nDef);
		if (m_GuildClear.nBeforeSrvDay && nOpenServerDay > m_GuildClear.nBeforeSrvDay)
        {
			m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;
        }

		if (isExistString("RealTimeLt")) {
            getFieldStringBuffer("RealTimeLt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClear.nRealTimeLt);
        }else m_GuildClear.nRealTimeLt = 0;

		// 小于这个时间的才开启
        if (!CheckOpenTimeLT(m_GuildClear))
        {
            m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;       
        }

		if (isExistString("RealTimeGt")) {
            getFieldStringBuffer("RealTimeGt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClear.nRealTimeGt);
        }else m_GuildClear.nRealTimeGt = 0;

		// 大于这个时间的才开启
        if (!CheckOpenTimeGt(m_GuildClear))
        {
            m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;       
        }

		if (isExistString("HefuTimeLt")) {
            getFieldStringBuffer("HefuTimeLt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClear.nHefuTimeLt);
        }else m_GuildClear.nHefuTimeLt = 0;

		// 合服时间小于这个的才开启
        if (!CheckHefuTimeLt(m_GuildClear))
        {
            m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;       
        }

		if (isExistString("HefuTimeGt")) {
            getFieldStringBuffer("HefuTimeGt",buff,sizeof(buff));
            ReadTime(m_pLua, 1, buff, sizeof(buff), m_GuildClear.nHefuTimeGt);
        }else m_GuildClear.nHefuTimeGt = 0;

		// 合服时间大于这个的才开启
        if (!CheckHefuTimeGt(m_GuildClear))
        {
            m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;       
        }

		m_GuildClear.nIsOpen = getFieldInt("isOpen",&nDef);

		// 详细行会关闭时间
		if (feildTableExists("TimeDetail") && openFieldTable("TimeDetail"))
		{
			int nCount = (int)lua_objlen(m_pLua,-1);
			m_GuildClear.pTimeDetail = (OneTimePair*)m_DataAward.allocObjects(sizeof(OneTimePair) * nCount);
			for (size_t i = 1; i <= nCount; i++)
			{
				getFieldIndexTable(i);
				getFieldStringBuffer("StartTime",m_GuildClear.pTimeDetail[i-1].strStartTime,ATIVITY_TIME_LEN);
				getFieldStringBuffer("EndTime",m_GuildClear.pTimeDetail[i-1].strEndTime,ATIVITY_TIME_LEN);
				closeTable();
			}
			m_GuildClear.nTimeCount = nCount;
			m_GuildClear.nTimeIdx = 0;
			closeTable();	
		}

		// 计算时间（若算不到时间，说明行会一直开启）
		if (!UpdateTime(m_pLua, m_GuildClear))
		{
			m_GuildClear.nStartTime = 0;
			m_GuildClear.nEndTime = 0;
		}

		// 活动时间类型为非循环时间的活动，需要结束时间大于当前服务器时间，才说明可以开启
        /*
        	4:创角时间
            5:开服时间
        */
        if (m_GuildClear.nTimeType != 4 && m_GuildClear.nTimeType != 5 && m_GuildClear.nTimeType != 3 )
        {
            if ((unsigned int)m_GuildClear.nEndTime != 0 && (unsigned int)m_GuildClear.nEndTime <= now_time)
            {
                m_GuildClear.nStartTime = 0;
				m_GuildClear.nEndTime = 0;
        	}
        }

		closeTable();
	}


	// //状态行会召唤
	// if (feildTableExists("guildCall") && openFieldTable("guildCall"))
	// {
	// 	int nDef = 0;
	// 	nCallMemberNeed = (int)getFieldInt("guildCallNeed", &nDef);
	// 	closeTable();
	// }


	// //行会仓库的配置
	// if( feildTableExists("guildDepot") && openFieldTable("guildDepot") )
	// {
	// 	m_GuildDepot.nDepotMaxPage			= getFieldInt("depotMaxPage");					//行会仓库最大页数
	// 	m_GuildDepot.nDailyDepotPutInNum	= getFieldInt("dailyDepotPutInNum");			//每人每日可投入操作的上限
	// 	m_GuildDepot.nExchangeNeedCheckCircleMin	= getFieldInt("exchangeNeedCheckCircleMin");	//每人每日可投入操作的上限
	// 	m_GuildDepot.nExchangeWaitCheckHour	= getFieldInt("exchangeWaitCheckHour");			//审核等待时间（单位小时）

	// 	if( feildTableExists("itemIputIn") && openFieldTable("itemIputIn"))
	// 	{
	// 		if( enumTableFirst() )
	// 		{
	// 			do 
	// 			{
	// 				GuildDepotPutIn stPutIn;
	// 				stPutIn.nItemCircle = getFieldInt("itemCircle");

	// 				if (feildTableExists("itemLevel") && openFieldTable("itemLevel"))
	// 				{
	// 					if (enumTableFirst())		//
	// 					{
	// 						INT_PTR nIdx = 0;
	// 						do 
	// 						{
	// 							if( nIdx < 2 )
	// 							{
	// 								stPutIn.nItemLevel[nIdx] = (int)getFieldInt(NULL);
	// 								nIdx++;
	// 							}
	// 						} while (enumTableNext());
	// 					}
	// 					closeTable();
	// 				}

	// 				m_GuildDepot.m_ItemPutInList.add(stPutIn);

	// 				//注意：CVector内部嵌套的CVector要在主CVector执行add后，才能加子CVector
	// 				if (feildTableExists("addGX") && openFieldTable("addGX"))
	// 				{
	// 					INT_PTR nCount = m_GuildDepot.m_ItemPutInList.count();
	// 					GuildDepotPutIn &stPutInTmp = m_GuildDepot.m_ItemPutInList[nCount-1];
	// 					if( enumTableFirst() )
	// 					{
	// 						do 
	// 						{
	// 							int nPos = (int)getFieldInt(NULL);
	// 							stPutInTmp.addGxList.add(nPos);
	// 						}while (enumTableNext());
	// 					}
	// 					closeTable();
	// 				}
	// 			}while (enumTableNext());
	// 		}
	// 		closeTable();
	// 	}

	// 	if( feildTableExists("itemGetOut") && openFieldTable("itemGetOut"))
	// 	{
	// 		if( enumTableFirst() )
	// 		{
	// 			do 
	// 			{
	// 				GuildDepotGetOut stGetOut;
	// 				stGetOut.nItemCircle = getFieldInt("itemCircle");
	// 				if (feildTableExists("itemLevel") && openFieldTable("itemLevel"))
	// 				{
	// 					if (enumTableFirst())		//
	// 					{
	// 						INT_PTR nIdx = 0;
	// 						do 
	// 						{
	// 							if( nIdx < 2 )
	// 							{
	// 								stGetOut.nItemLevel[nIdx] = (int)getFieldInt(NULL);
	// 								nIdx++;
	// 							}
	// 						} while (enumTableNext());
	// 					}
	// 					closeTable();
	// 				}

	// 				m_GuildDepot.m_ItemGetOutList.add(stGetOut);

	// 				//注意：CVector内部嵌套的CVector要在主CVector执行add后，才能加子CVector
	// 				if (feildTableExists("needGX") && openFieldTable("needGX"))
	// 				{
	// 					INT_PTR nCount = m_GuildDepot.m_ItemGetOutList.count();
	// 					GuildDepotGetOut &stGetOutTmp = m_GuildDepot.m_ItemGetOutList[nCount-1];
	// 					if( enumTableFirst() )
	// 					{
	// 						do 
	// 						{
	// 							int nPos = (int)getFieldInt(NULL);
	// 							stGetOutTmp.needGxList.add(nPos);
	// 						}while (enumTableNext());
	// 					}
	// 					closeTable();
	// 				}
	// 			}while (enumTableNext());
	// 		}
	// 		closeTable();
	// 	}
	// 	closeTable();
	// }

	//行会神树的配置
	
	// if (feildTableExists("guildTree") && openFieldTable("guildTree"))
	// {
	// 	m_GuildTree.m_nDailyChargeLimit	= getFieldInt("dailyChargeLimit");
	// 	m_GuildTree.m_nDailyPickLimit	= getFieldInt("dailyPickLimit");
	// 	m_GuildTree.m_nFruitProtectTime	= getFieldInt("fruitProtectTime");

	// 	// if (feildTableExists("fruitLevel") && openFieldTable("fruitLevel")) 
	// 	// {
	// 	// 	if (enumTableFirst())
	// 	// 	{
	// 	// 		do 
	// 	// 		{	
	// 	// 			GuildFruitLevel stFruitLevel;
	// 	// 			stFruitLevel.m_nFruitId			= (int)getFieldInt("fruitId");
	// 	// 			stFruitLevel.m_nFruitNum		= (int)getFieldInt("fruitNum");
	// 	// 			getFieldStringBuffer("qualityName", stFruitLevel.m_szQualityName, sizeof(stFruitLevel.m_szQualityName));
	// 	// 			//OutputMsg(rmNormal, "CGuildProvider::readConfigs(),m_szQualityName=%s", stFruitLevel.m_szQualityName);

	// 	// 			if (feildTableExists("backGuildCoin") && openFieldTable("backGuildCoin"))
	// 	// 			{
	// 	// 				if (enumTableFirst())		//
	// 	// 				{
	// 	// 					INT_PTR nIdx = 0;
	// 	// 					do 
	// 	// 					{
	// 	// 						if( nIdx < 2 )
	// 	// 						{
	// 	// 							stFruitLevel.m_nBackGuildCoin[nIdx] = (int)getFieldInt(NULL);
	// 	// 							nIdx++;
	// 	// 						}
	// 	// 					} while (enumTableNext());
	// 	// 				}
	// 	// 				closeTable();
	// 	// 			}

	// 	// 			m_GuildTree.m_FruitLevelList.add(stFruitLevel);
	// 	// 			//m_nMaxMainHallCoin[nIdx++] = getFieldInt(NULL);
	// 	// 		} while (enumTableNext());
	// 	// 	}
	// 	// 	closeTable();
	// 	// }

	// 	closeTable();
	// }

	// if (feildTableExists("guildTask") && openFieldTable("guildTask"))
	// {
	// 	if (enumTableFirst())
	// 	{
	// 		int nLevelIdx = 0;
	// 		do 
	// 		{	
	// 			if( nLevelIdx < GUILD_LEVEL_MAX )
	// 			{
					
	// 				if (enumTableFirst())
	// 				{
	// 					do 
	// 					{	
	// 						GuildTask stTask;
	// 						stTask.m_nTaskType	= (short)getFieldInt("taskType");
	// 						stTask.m_nTaskId	= (int)getFieldInt("taskId");
	// 						stTask.m_nTaskObjId = (int)getFieldInt("taskObjId");
	// 						stTask.m_nTaskObjNum= (int)getFieldInt("taskObjNum");
	// 						m_GuildLevelTaskList[nLevelIdx].m_GuildTaskList.add(stTask);
	// 					} while (enumTableNext());
	// 				}
	// 				nLevelIdx ++;
	// 				//closeTable();		//closeTable() 与openFieldTable()对应
	// 			}

	// 		} while (enumTableNext());
	// 	}
	// 	closeTable();
	// }

	// if (feildTableExists("MaxCoinReward") && openFieldTable("MaxCoinReward"))
	// { 
		
	// 	size_t nCount = lua_objlen(m_pLua, -1);
	// 	m_CoinAward.reserve(nCount);
	// 	if(enumTableFirst())
	// 	{
	// 		int nDef = 0;
	// 		ACTORAWARD oneAward;
	// 		do 
	// 		{
	// 			oneAward.btType = (byte)getFieldInt("type");
	// 			oneAward.wId = (int)getFieldInt("id");
	// 			oneAward.wCount = (int)getFieldInt("count");
	// 			oneAward.btQuality = (byte)getFieldInt("quality",&nDef);
	// 			oneAward.btStrong = (byte)getFieldInt("strong", &nDef);
	// 			oneAward.btBind = (byte)getFieldInt("bind", &nDef);
				
	// 			m_CoinAward.add(oneAward);
	// 		} while (enumTableNext());
	// 	}
	// 	closeTable();
	// }

	// if(feildTableExists("FirstNewHundredSBKAward") && openFieldTable("FirstNewHundredSBKAward"))
	// {
	// 	size_t nCount = lua_objlen(m_pLua,-1);
	// 	//循环读取等级的配置
	// 	if ( enumTableFirst() )
	// 	{
	// 		INT_PTR nIdx = 0;
	// 		CDataAllocator dataAllocator;
	// 		pNewHundredList = (FirstNewHundredAwardList*)dataAllocator.allocObjects(sizeof(FirstNewHundredAwardList)*nCount);
	// 		nNewHundredCount = (int)nCount;
	// 		do 
	// 		{
	// 			FirstNewHundredAwardList &item = pNewHundredList[nIdx];
	// 			if (!openFieldTable("items"))
	// 			{
	// 				return false;
	// 			}
	// 			size_t nOneCount = lua_objlen(m_pLua,-1);
	// 			if (enumTableFirst())
	// 			{
	// 				item.nCount = (int)nOneCount;
	// 				item.pList = (ACTORAWARD*)dataAllocator.allocObjects(sizeof(ACTORAWARD)*nOneCount);
	// 				int sIdx = 0;
	// 				do 
	// 				{
	// 					int nDef = 0;
	// 					ACTORAWARD& TempItem = item.pList[sIdx];
	// 					TempItem.btType = (byte)getFieldInt("type");
	// 					TempItem.wId = (int)getFieldInt("id");
	// 					TempItem.wCount = (int)getFieldInt("count");
	// 					TempItem.btQuality = (byte)getFieldInt("quality",&nDef);
	// 					TempItem.btStrong = (byte)getFieldInt("strong", &nDef);
	// 					TempItem.btBind = (byte)getFieldInt("bind", &nDef);
	// 					sIdx++;
	// 				} while (enumTableNext());
	// 			}
	// 			closeTable();
	// 			nIdx++;
	// 		}
	// 		while (enumTableNext());

	// 		//原来的内存块释放到
	// 		m_DataAward.~CObjectAllocator();
	// 		//将临时的内存块复制到正式的
	// 		m_DataAward = dataAllocator;
	// 		//清空临时的
	// 		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	// 	}
	// 	closeTable();
	// }
	closeTable();
	OutputMsg(rmTip, "close Table");
	// if (!readGuildSiegeConfig())//读取行会攻城相关配置
	// {
	// 	return false;
	// }

	// //装载行会任务
	// if(!readGuildTaskConfig())
	// {
	// 	OutputMsg(rmError, "readGuildTaskConfig Error! you need check config");
	// 	return false;
	// }

	// if(!readGuildFruitLevelConfig())
	// {
	// 	OutputMsg(rmError, "readGuildFruitLevelConfig Error! you need check config");
	// 	return false;
	// }

	if(!readguildLevelConfig())
	{
		OutputMsg(rmError, "readguildLevelConfig Error! you need check config");
		return false;
	}

	if ( openGlobalTable("GuildDonateConfig") )
	{
		int count = (int)lua_objlen(m_pLua, -1);
		if(count < 0)
		{
			return false;
		}
		if (enumTableFirst())
		m_nDonate.clear();
		{
			do 
			{	
				DonateCfg donate;
				donate.nType = (int)getFieldInt("type");
				donate.nCost = (int)getFieldInt("cost");
				donate.nAddCoin = (int)getFieldInt("addcion");
				donate.nLimitTimes = (int)getFieldInt("limittimes");
				donate.nStaticCountType = (int)getFieldInt("staticcounttype");
				donate.nAddDonate = (int)getFieldInt("adddonate");
				m_nDonate.insert(std::make_pair(donate.nType, donate));
			} while (enumTableNext());

		}
		closeTable();
	}

	readGuilBuildConfig();
	return true;
}
////装载行会任务
bool CGuildProvider::readGuildTaskConfig()
{
	// if ( !openGlobalTable("GuildTaskConfig") )
	// 	return false;

	// if (enumTableFirst())
	// {
	// 	do 
	// 	{	
	// 		GuildTask stTask;
	// 		int level = (int)getFieldInt("level");

	// 		if(level > GUILD_LEVEL_MAX)
	// 			break;

	// 		stTask.m_nTaskType	= (short)getFieldInt("taskType");
	// 		stTask.m_nTaskId	= (int)getFieldInt("taskId");
	// 		stTask.m_nTaskObjId = (int)getFieldInt("taskObjId");
	// 		stTask.m_nTaskObjNum= (int)getFieldInt("taskObjNum");
	// 		//OutputMsg(rmTip, _T("readGuildTaskConfig level:%d, taskType:%d, taskId:%d"), level,stTask.m_nTaskType,stTask.m_nTaskId);
	// 		m_GuildLevelTaskList[level].m_GuildTaskList.add(stTask);
	// 	} while (enumTableNext());

	// }

	// closeTable();
	return true;
}

bool CGuildProvider::readGuilBuildConfig()
{
	if ( !openGlobalTable("GuildBuildConfig") )
		return false;
	//if (feildTableExists("guildLevelConfig") && openFieldTable("guildLevelConfig"))
	{
		int nDef = 0;
		size_t nCount = lua_objlen(m_pLua,-1);
		//循环读取等级的配置
		if ( enumTableFirst() )
		{
			INT_PTR nIdx = 1;
			do 
			{
				if ( enumTableFirst() )
				{
					INT_PTR nIdx = 1;
					do 
					{
						int nType = (int)getFieldInt("type", &nDef);
						int nLevel = (int)getFieldInt("level", &nDef);
						GuildBuildCfg build;
						build.nType	= nType;
						build.nLevel			= nLevel;
						build.nCost		= getFieldInt("cost");
						build.nLimit		= getFieldInt("guillvlimit");
						m_GuildBuilds[nType][nLevel] = build;
					}
					while (enumTableNext());
				}
			}
			while (enumTableNext());
		}
	}

	closeTable();
	return true;
}
////装载行会等级
bool CGuildProvider::readguildLevelConfig()
{
	if ( !openGlobalTable("GuildLevelConfig") )
		return false;
	//if (feildTableExists("guildLevelConfig") && openFieldTable("guildLevelConfig"))
	{
		size_t nCount = lua_objlen(m_pLua,-1);
		int nDef = 0;
		//循环读取等级的配置
		if ( enumTableFirst() )
		{
			INT_PTR nIdx = 1;
			CDataAllocator dataAllocator;
			pLevelList = (GuildLevelConfigList*)dataAllocator.allocObjects(sizeof(*pLevelList));
			pLevelList->nCount = (int)nCount;
			pLevelList->pList = (GuildLevelConfig*)dataAllocator.allocObjects(sizeof(GuildLevelConfig)*nCount);
			do 
			{
				//读取每个等级的配置
				int level = getFieldInt("nLevel");
				GuildLevelConfig &item = pLevelList->pList[level-1];
				item.nLevel = level;
				// //如果该物品的ID不是期望的值，则输出错误并终止读取
				// if (item.nLevel != nIdx)
				// {
				// 	showErrorFormat(_T("unexpected Guild Level %d, request %d"), item.nLevel, nIdx);
				// 	endTableEnum();
				// 	break;
				// }
				if (item.nLevel > nMaxLevel)
				{
					nMaxLevel = item.nLevel;
				}
				nIdx++;

				//
				item.nMaxSecLeader	= getFieldInt("nMaxSecLeader", &nDef);
				item.nMaxelders	= getFieldInt("nMaxelders", &nDef);
				item.nMaxElite			= getFieldInt("nMaxElite", &nDef);
				item.nMaxMember		= getFieldInt("nMaxMember", &nDef);
				// item.nNeedYb		= getFieldInt("needYb", &nDef);
				// item.nWeekDecCoin	= getFieldInt("weekDecCoin", &nDef);
				// item.nFruitLevel	= getFieldInt("fruitLevel", &nDef);
				// item.nDepotPage		= getFieldInt("depotPage", &nDef);
			}
			while (enumTableNext());

			//原来的内存块释放到
			m_DataAllocator.~CObjectAllocator();
			//将临时的内存块复制到正式的
			m_DataAllocator = dataAllocator;
			//清空临时的
			ZeroMemory(&dataAllocator, sizeof(dataAllocator));
		}
	}

	closeTable();
	return true;
}


//神树果实
bool CGuildProvider::readGuildFruitLevelConfig()
{
	// if ( !openGlobalTable("GuildFruitLevelConfig") )
	// 	return false;

	// if (enumTableFirst())
	// {
	// 	do 
	// 	{	
	// 		GuildFruitLevel stFruitLevel;
	// 		stFruitLevel.m_nFruitId			= (int)getFieldInt("fruitId");
	// 		stFruitLevel.m_nFruitNum		= (int)getFieldInt("fruitNum");
	// 		getFieldStringBuffer("qualityName", stFruitLevel.m_szQualityName, sizeof(stFruitLevel.m_szQualityName));
	// 		//OutputMsg(rmTip, _T("readGuildFruitLevelConfig fruitId:%d, fruitNum:%d, qualityName:%s"),stFruitLevel.m_nFruitId,stFruitLevel.m_nFruitNum,stFruitLevel.m_szQualityName);
	// 		if (feildTableExists("backGuildCoin") && openFieldTable("backGuildCoin"))
	// 		{
	// 			if (enumTableFirst())		//
	// 			{
	// 				INT_PTR nIdx = 0;
	// 				do 
	// 				{
	// 					if( nIdx < 2 )
	// 					{
	// 						stFruitLevel.m_nBackGuildCoin[nIdx] = (int)getFieldInt(NULL);
	// 						nIdx++;
	// 					}
	// 				} while (enumTableNext());
	// 			}
	// 			closeTable();
	// 		}
	// 		m_GuildTree.m_FruitLevelList.add(stFruitLevel);
	// 	} while (enumTableNext());
	// }

	// closeTable();
	return true;
}



bool CGuildProvider::readGuildSiegeConfig()
{
	// if ( !openGlobalTable("GuildSiegeConfig") )
	// 	return false;
	// int nIntDefault = 0;
	// nOpenSvrSingDays = getFieldInt("OpenSvrSingDays");
	// nAutoSignMainHallLevel = getFieldInt("autoSignMainHallLevel", &nIntDefault);
	// nDragonRoleBuffId = getFieldInt("dragonRoleBuffId", &nIntDefault);
	// nSiegeSceneId = getFieldInt("siegeSceneId", &nIntDefault);
	// nPalaceSceneId = getFieldInt("palaceSceneId", &nIntDefault);
	// /*
	// if (feildTableExists("stRealMasterModel") && openFieldTable("stRealMasterModel"))
	// {
	// 	if (enumTableFirst())
	// 	{
	// 		int nIndex = 0;
	// 		do 
	// 		{
	// 			nRealMasterModelId[nIndex] = getFieldInt(NULL);
	// 			if (nIndex >= ArrayCount(nRealMasterModelId))
	// 			{
	// 				break;
	// 			}
	// 			nIndex++;
	// 		} while (enumTableNext());
	// 	}
	// 	closeTable();
	// }
	// */
	// if (feildTableExists("siegeDay") && openFieldTable("siegeDay"))
	// {
	// 	if (enumTableFirst())
	// 	{
	// 		do 
	// 		{
	// 			BYTE nSiegeDay = (BYTE)getFieldInt(NULL);
	// 			vSiegeDayList.add(nSiegeDay);
	// 		} while (enumTableNext());
	// 	}
	// 	closeTable();
	// }
	// if (feildTableExists("cityPosName") && openFieldTable("cityPosName"))
	// {
	// 	if (enumTableFirst())
	// 	{
	// 		int index = stRealMaster;
	// 		do 
	// 		{
	// 			strcpy(sCityPosName[index++],getFieldString(NULL));
	// 		} while (enumTableNext());
	// 	}
	// 	closeTable();
	// }
	
	// if (feildTableExists("returnSiegeScene") && openFieldTable("returnSiegeScene"))
	// {
	// 	returnSiegeScene.nSceneId = getFieldInt("sceneId");
	// 	returnSiegeScene.nPosX = getFieldInt("posX");
	// 	returnSiegeScene.nPosY = getFieldInt("posY");
	// 	returnSiegeScene.nRadius = getFieldInt("radius");
	// 	closeTable();
	// }
	// closeTable();
	return true;
}
const char* CGuildProvider::GetGuildCityPosName(int nPos)
{
	// if (nPos >stNoCityPos && nPos < stMaxGuildCityPos)
	// {
	// 	return sCityPosName[nPos];
	// }
	return NULL;
}
void CGuildProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[GuildProvider]");
	s += sError;
	throw s;
}

/*物品放入到行会仓库增加的行会贡献
nItemPos取值：
itWeapon				= 1,	//武器
itDress					= 2,	//衣服
itHelmet				= 3,	//头盔
itNecklace				= 4,	//项链
itDecoration			= 5,    //勋章
itBracelet				= 6,	//手镯
itRing					= 7,	//戒指
itGirdle				= 8,	//腰带
itShoes					= 9,	//鞋子
*/
int CGuildProvider::GetGuildDepotPutInAddGx(int nItemCircle, int nItemLevel, int nItemPos)
{
	// for(INT_PTR i=0; i<m_GuildDepot.m_ItemPutInList.count(); i++)
	// {
	// 	GuildDepotPutIn &stPutIn = m_GuildDepot.m_ItemPutInList[i];
	// 	if( nItemCircle >= stPutIn.nItemCircle && ( nItemLevel >= stPutIn.nItemLevel[0] && nItemLevel <= stPutIn.nItemLevel[1] ) )
	// 	{
	// 		INT_PTR nCount = stPutIn.addGxList.count();
	// 		if( 0 < nItemPos && nItemPos <= nCount )
	// 		{
	// 			return stPutIn.addGxList[nItemPos-1];
	// 		}
	// 		else
	// 		{
	// 			return 0;
	// 		}
	// 	}
	// }
	return 0;
}

//物品从行会仓库取出所需的行会贡献
int CGuildProvider::GetGuildDepotGetOutNeedGx(int nItemCircle, int nItemLevel, int nItemPos)
{
	// for(INT_PTR i=0; i<m_GuildDepot.m_ItemGetOutList.count(); i++)
	// {
	// 	GuildDepotGetOut &stGetOut = m_GuildDepot.m_ItemGetOutList[i];
	// 	if( nItemCircle >= stGetOut.nItemCircle && ( nItemLevel >= stGetOut.nItemLevel[0] && nItemLevel <= stGetOut.nItemLevel[1] ) )
	// 	{
	// 		INT_PTR nCount = stGetOut.needGxList.count();
	// 		if( 0 < nItemPos && nItemPos <= nCount )
	// 		{
	// 			return stGetOut.needGxList[nItemPos-1];
	// 		}
	// 		else
	// 		{
	// 			return 0;
	// 		}
	// 	}
	// }
	return 0;
}


void CGuildProvider::GetSbkGuildAward(int nRankId, std::vector<ACTORAWARD>& award)
{
	int max = INT_MAX;
	for(int i = 0; i < sbkguild.size(); i++) {
		if(sbkguild[i].nValue <= nRankId  && sbkguild[i].nValue < max) {
			award.clear();
			award.insert(award.end(), sbkguild[i].awards.begin(), sbkguild[i].awards.end());
		}
	}
}

bool CGuildProvider::UpdateTime(lua_State * pLuaState, GUILDDATA& guildData, bool nNextTime)
{
    //nNextTime 直接开启下一轮
    bool result = false;
    CMiniDateTime now_time = CMiniDateTime::now();

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T(" 当前时间:(%d-%d-%d %d:%d:%d) "),
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    bool isAfterSrvDay = true;
    
    // 开服N天后才开，第N天仍不开，则计算时间时需要偏移
    if (guildData.nAfterSrvDay > 0)
    {
        int nOpenServerDay = GetLogicServer()->GetDaysSinceOpenServer();
        int nDiff = guildData.nAfterSrvDay - nOpenServerDay + 1;
        if (nDiff > 0)
        {
            now_time += nDiff*3600*24;
            isAfterSrvDay = false;
        }
        
#ifdef _DEBUG
        OutputMsg(rmTip, _T(" Diff=%d"), nDiff);
#endif
    }

#ifdef _DEBUG
    {
        SYSTEMTIME nowTime;
        memset(&nowTime,0,sizeof(nowTime));
        now_time.decode(nowTime);
        OutputMsg(rmTip, _T(" 当前调整时间:(%d-%d-%d %d:%d:%d) "),
            nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
    }
#endif

    // 开服前N天开，包括第N天，超过这个时间将不再开启
    if (guildData.nBeforeSrvDay > 0 &&
        GetLogicServer()->GetDaysSinceOpenServer() > guildData.nBeforeSrvDay)
    {
        return false;
    }

    switch (guildData.nTimeType)
    {
    case eActivityTimeType_KFSJ: // 开服时间
    {
        for (; guildData.nTimeIdx < guildData.nTimeCount; guildData.nTimeIdx++)
        {
            short curidx = guildData.nTimeIdx;

            // 获取结束时间
            guildData.nEndTime.tv = 0;
            if(strcmp(guildData.pTimeDetail[curidx].strEndTime, "-1"))
            {
                ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, guildData.nEndTime);
                guildData.nEndTime = GetLogicServer()->GetServerOpenTime().rel_today() + guildData.nEndTime;
            }
            // 已经结束的，换下一个点 --默认-1 为永久
            if (guildData.nEndTime != 0 && guildData.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, guildData.nStartTime);
            guildData.nStartTime = GetLogicServer()->GetServerOpenTime().rel_today() + guildData.nStartTime;
            if(nNextTime && guildData.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_GDSJ: // 固定时间
    {
        for (; guildData.nTimeIdx < guildData.nTimeCount; guildData.nTimeIdx++)
        {
            short curidx = guildData.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, guildData.nEndTime);

            // 已经结束的，换下一个点
            if (guildData.nEndTime <= now_time) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, guildData.nStartTime);

            // 开服时间检测
            if (guildData.nAfterSrvDay > 0 && (!isAfterSrvDay) && guildData.nStartTime < now_time)
            {
                SYSTEMTIME timeinfo;
                ReadFixedTimeValue(pLuaState, guildData.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, timeinfo);
                guildData.nStartTime = now_time.rel_today(timeinfo.wHour, timeinfo.wMinute, 0);
            }

            // 切换下一个时间点
            if(nNextTime && guildData.nStartTime < now_time) continue;

            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_HFSJ: // 合服时间
    {
        for (; guildData.nTimeIdx < guildData.nTimeCount; guildData.nTimeIdx++)
        {
            short curidx = guildData.nTimeIdx;

            // 获取结束时间
            ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN, guildData.nEndTime);
            guildData.nEndTime = GetLogicServer()->GetServerCombineTime() + guildData.nEndTime;

            // 已经结束的，换下一个点
            if (guildData.nEndTime <= now_time || nNextTime) continue;
            
            // 获取开始时间
            ReadTime(pLuaState, guildData.nTimeType, guildData.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN, guildData.nStartTime);
            guildData.nStartTime = GetLogicServer()->GetServerCombineTime() + guildData.nStartTime;
            if(nNextTime && guildData.nStartTime < now_time) continue;
            result = true;
            break;
        }
    }
    break; 
    case eActivityTimeType_XHSJ: // 循环时间
    {
        int nCurYear,nCurMon,nCurDay,nCurHour,nCurMin,nCurSec,nCurWeek;
        {
            SYSTEMTIME SysTime;// = GetGlobalLogicEngine()->getSysTime();
            now_time.decode(SysTime);
            nCurYear = SysTime.wYear;
            nCurMon = SysTime.wMonth;
            nCurDay = SysTime.wDay;
            nCurHour = SysTime.wHour;
            nCurMin = SysTime.wMinute;
            nCurSec = SysTime.wSecond;

            struct tm tmResult;
            memset(&tmResult, 0, sizeof(tmResult));
            tmResult.tm_isdst = 0;
            tmResult.tm_year = nCurYear - 1900;
            tmResult.tm_mon = nCurMon - 1;
            tmResult.tm_mday = nCurDay;
            mktime(&tmResult);
            nCurWeek = tmResult.tm_wday;
        }
        if (nCurWeek == 0)
        {
            nCurWeek = 7;
        }
        

        static int nStartWeek,nStartHour,nStartMinute;
        static int nEndWeek,nEndHour,nEndMinute;
        static CMiniDateTime openTime,closeTime;

        for (; guildData.nTimeIdx < guildData.nTimeCount; guildData.nTimeIdx++)
        {
            short curidx = guildData.nTimeIdx;

            // 获取结束时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, guildData.pTimeDetail[curidx].strEndTime, strnlen(guildData.pTimeDetail[curidx].strEndTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            int nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nEndWeek = lua_tonumber(pLuaState, -3);
                nEndHour = lua_tonumber(pLuaState, -2);
                nEndMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            closeTime.encode(nCurYear, nCurMon, nCurDay + (nEndWeek > 0?nEndWeek - nCurWeek:0), nEndHour, nEndMinute, 0);

//#ifdef _DEBUG
            {
                SYSTEMTIME nowTime;
                memset(&nowTime,0,sizeof(nowTime));
                closeTime.decode(nowTime);
                OutputMsg(rmTip, _T(" (%d)结束时间:(%d-%d-%d %d:%d:%d) "),curidx,
                    nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);
            }
//#endif
            
            // 已经结束的，换下一个点
            if (closeTime <= now_time) continue;
            
            // 获取开始时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, guildData.pTimeDetail[curidx].strStartTime, strnlen(guildData.pTimeDetail[curidx].strStartTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nStartWeek = lua_tonumber(pLuaState, -3);
                nStartHour = lua_tonumber(pLuaState, -2);
                nStartMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            openTime.encode(nCurYear, nCurMon, nCurDay + (nStartWeek > 0?nStartWeek - nCurWeek:0), nStartHour, nStartMinute, 0);

            if(nNextTime && openTime < now_time) continue;
            // 记录时间戳
            guildData.nStartTime = openTime;
            guildData.nEndTime = closeTime;
            result = true;
            break;
        }

        if (!result)
        {
            // 如果都不在时间内，那就是下一周了的第一个时间了
            guildData.nTimeIdx = 0;

            // 获取开始时间
            int nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, guildData.pTimeDetail[0].strStartTime, strnlen(guildData.pTimeDetail[0].strStartTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            int nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nStartWeek = lua_tonumber(pLuaState, -3);
                nStartHour = lua_tonumber(pLuaState, -2);
                nStartMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            openTime.encode(nCurYear, nCurMon, nCurDay + (nStartWeek > 0?nStartWeek - nCurWeek:0), nStartHour, nStartMinute, 0);

            // 获取结束时间
            nTop = lua_gettop(pLuaState);
            lua_getglobal(pLuaState, "string");
            lua_getfield(pLuaState, -1, "match");
            lua_pushlstring(pLuaState, guildData.pTimeDetail[0].strEndTime, strnlen(guildData.pTimeDetail[0].strEndTime, ATIVITY_TIME_LEN));
            lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
            nErr = lua_pcall(pLuaState, 2, 3, 0);
            if (!nErr)
            {
                nEndWeek = lua_tonumber(pLuaState, -3);
                nEndHour = lua_tonumber(pLuaState, -2);
                nEndMinute = lua_tonumber(pLuaState, -1);
            }else return false;
            lua_pop(pLuaState, 3);
            lua_settop(pLuaState, nTop);
            closeTime.encode(nCurYear, nCurMon, nCurDay + (nEndWeek > 0?nEndWeek - nCurWeek:0), nEndHour, nEndMinute, 0);

            int loop = (0 == nStartWeek) || (0 == nEndWeek) ? 1 : 7;

            // 计算矫正
            if (closeTime < openTime)
                openTime = openTime - loop * (24*3600);

            // 计算矫正后起始/结束时间
            guildData.nStartTime = openTime;
            guildData.nEndTime = closeTime;
            while (now_time > (unsigned int)guildData.nEndTime || (nNextTime && now_time > (unsigned int)guildData.nStartTime))
            {
                guildData.nStartTime = guildData.nStartTime + loop * (24*3600);
                guildData.nEndTime = guildData.nEndTime + loop * (24*3600);
            }

            result = true;
        }
        
        {
            //开服前n天开启的活动需要判定下次开启时间是否超过n
            if (guildData.nBeforeSrvDay > 0)
            {
                int nDay = (guildData.nStartTime.tv - GetLogicServer()->GetServerOpenTime().rel_today())/(3600*24)+1;
                if (nDay > guildData.nBeforeSrvDay)
                    return false;
            }
        }
    }
    break;
    }
    
    //#ifdef _DEBUG
        SYSTEMTIME starTime,endTime;
        memset(&starTime,0,sizeof(starTime));
        memset(&endTime,0,sizeof(endTime));
        guildData.nStartTime.decode(starTime);
        guildData.nEndTime.decode(endTime);

        OutputMsg(rmTip, _T(" 开始时间:(%d-%d-%d %d:%d:%d) 结束时间:(%d-%d-%d %d:%d:%d) "),
            starTime.wYear, starTime.wMonth, starTime.wDay, starTime.wHour, starTime.wMinute, starTime.wSecond, 
            endTime.wYear, endTime.wMonth, endTime.wDay, endTime.wHour, endTime.wMinute, endTime.wSecond);
    //#endif
    return result;
}

bool CGuildProvider::ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out)
{
    switch (nTimeType)
    {
    case 0: //day-hour:minute
    case 2:
    case 3:
    {
        static int nD,nH,nM = 3;

        int nTop = lua_gettop(pLuaState);
        lua_getglobal(pLuaState, "string");
		lua_getfield(pLuaState, -1, "match");
        lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
        lua_pushlstring(pLuaState, "(%d+)-(%d+):(%d+)", strnlen("(%d+)-(%d+):(%d+)",sizeof("(%d+)-(%d+):(%d+)")));
        int nErr = lua_pcall(pLuaState, 2, 3, 0);
        if (!nErr)
        {
            nD = lua_tonumber(pLuaState, -3);
            nH = lua_tonumber(pLuaState, -2);
            nM = lua_tonumber(pLuaState, -1);
        }else return false;
        lua_pop(pLuaState, 3);
        lua_settop(pLuaState, nTop);
        out = nD*24*3600 + nH*3600 + nM*60;
    }
    break;
    case 1: // year.month.day-hour:minute
    {
        SYSTEMTIME TimeResult;
        memset(&TimeResult,0,sizeof(TimeResult));

        int nTop = lua_gettop(pLuaState);
        lua_getglobal(pLuaState, "string");
		lua_getfield(pLuaState, -1, "match");
        lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
        lua_pushlstring(pLuaState, "(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)", strnlen("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)",sizeof("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)")));
        int nErr = lua_pcall(pLuaState, 2, 5, 0);
        if (!nErr)
        {
            TimeResult.wYear = lua_tonumber(pLuaState, -5);
            TimeResult.wMonth = lua_tonumber(pLuaState, -4);
            TimeResult.wDay = lua_tonumber(pLuaState, -3);
            TimeResult.wHour = lua_tonumber(pLuaState, -2);
            TimeResult.wMinute = lua_tonumber(pLuaState, -1);
        }else return false;
        lua_pop(pLuaState, 5);
        lua_settop(pLuaState, nTop);
		out.encode(TimeResult);
    }
    break;
    }
}

bool CGuildProvider::ReadFixedTimeValue(lua_State * pLuaState, LPCTSTR sKey, int nKeyLen, SYSTEMTIME& out)
{
    memset(&out,0,sizeof(out));

    int nTop = lua_gettop(pLuaState);
    lua_getglobal(pLuaState, "string");
    lua_getfield(pLuaState, -1, "match");
    lua_pushlstring(pLuaState, sKey, strnlen(sKey, nKeyLen));
    lua_pushlstring(pLuaState, "(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)", strnlen("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)",sizeof("(%d+)%.(%d+)%.(%d+)-(%d+):(%d+)")));
    int nErr = lua_pcall(pLuaState, 2, 5, 0);
    if (!nErr)
    {
        out.wYear = lua_tonumber(pLuaState, -5);
        out.wMonth = lua_tonumber(pLuaState, -4);
        out.wDay = lua_tonumber(pLuaState, -3);
        out.wHour = lua_tonumber(pLuaState, -2);
        out.wMinute = lua_tonumber(pLuaState, -1);
    }else return false;
    lua_pop(pLuaState, 5);
    lua_settop(pLuaState, nTop);
}

bool CGuildProvider::CheckOpenTimeLT(GUILDDATA& guildData)
{   
    //检测开服时间比配置小的才开启活动
    if(guildData.nRealTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() > guildData.nRealTimeLt)
        return false;
    return true;
}

bool CGuildProvider::CheckOpenTimeGt(GUILDDATA& guildData)
{
    //检测开服时间比配置大的才开启活动
    if(guildData.nRealTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerOpenTime() < guildData.nRealTimeGt)
        return false;
    return true;
}

bool CGuildProvider::CheckHefuTimeLt(GUILDDATA& guildData)
{
    //检测合服时间比配置小的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || guildData.nHefuTimeLt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() > guildData.nHefuTimeLt)
        return false;
    return true;
}

bool CGuildProvider::CheckHefuTimeGt(GUILDDATA& guildData)
{
    //检测合服时间比配置大的才开启活动
    if(GetLogicServer()->GetServerCombineTime() == 0 || guildData.nHefuTimeGt == 0)
        return true;
    if (GetLogicServer()->GetServerCombineTime() < guildData.nHefuTimeGt)
        return false;
    return true;
}

bool CGuildProvider::CheckHefuTimes(GUILDDATA& guildData)
{
    GLOBALCONFIG &data = GetLogicServer()->GetDataProvider()->GetGlobalConfig();

    if(guildData.nHefuTimes >= 3 && data.nMergeTimes < guildData.nHefuTimes)
        return false;
    
    if(guildData.nHefuTimes > 0  && guildData.nHefuTimes <= 2 && data.nMergeTimes != guildData.nHefuTimes)
        return false;
    
    return true;
}