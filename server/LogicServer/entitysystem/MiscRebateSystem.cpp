#include "StdAfx.h"
#include "MiscSystem.h"
#include "FvMask.h" 
//#include "config/RebateConfig.h"  
#include "util_func_extend.h"  
 
static const LPCTSTR g_RebateFlagVarName = "RebateFlag";
static const LPCTSTR g_RebateOneDayRefeshKey = "RebateOneDayRefeshKey";		//每天刷新key
static const LPCTSTR g_RebatePayOneDay = "RebateFlagPayOneDay";				//每天充值
/*
void CMiscSystem::InitAllRebateKey()
{ 
	std::vector <RebateMaskInfo> maskVec;
	GetLogicServer()->GetDataProvider()->GetRebate().OnGetAllMaskInfo(maskVec);

	if(maskVec.empty())
	{
		return;
	}
	
	char strBuff[64];
	for (size_t i = 0; i < maskVec.size(); i++)
	{ 
		memset(strBuff, 0, sizeof(strBuff));
		sprintf(strBuff, "%d_%d", maskVec[i].Id, maskVec[i].index);
		m_RebateSet.insert(std::make_pair(strBuff, 0));
	}
	bool IsNewDay = false;		//新的一天
	bool IsFirstInit = false;	//第一次初始化
	
	if (!m_pEntity)
	{ 
		return;
	} 
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();	
	
	CCLVariant *pTileMemVar = var.get(g_RebateOneDayRefeshKey);

	CMiniDateTime nNow(GetGlobalLogicEngine()->getMiniDateTime());	 
 
	unsigned int nTodayTimeZero = nNow.today(); //获得零点时间
	if(!pTileMemVar)
	{ 
		var.set(g_RebateOneDayRefeshKey);
		pTileMemVar = var.get(g_RebateOneDayRefeshKey);
		IsNewDay = true;
		IsFirstInit = true;
	}
	if (!IsNewDay)//已经初始化过了
	{ 
		unsigned int lastTodayTime = (unsigned int)((double)*pTileMemVar); 
		if (lastTodayTime != nTodayTimeZero)
		{
			IsNewDay = true;
		} 
	} 
	pTileMemVar->set(g_RebateOneDayRefeshKey, nTodayTimeZero); //初始化,每天的领奖清0  
	if (IsFirstInit)
	{
		//初始化总表
		return;
	}
	
	if (IsNewDay)
	{
		//初始化 每天的领奖清0
	}  
}
std::string CMiscSystem::RebateMapAsString(std::map<std::string, int>& RebateSet)
{
	if(RebateSet.empty())
	{
		return "";
	}
	std::string str;
	std::map<std::string, int>::iterator it = RebateSet.begin();
	for (; it != RebateSet.end(); it++)
	{
		if (it != RebateSet.begin())
		{
			str += ",";
		} 
		str += it->first;
		str += ":";
		str += utility::toString(it->second);
	}
	
	return str;
}
bool CMiscSystem::StringAsRebateMap(std::map<std::string, int>& RebateSet, std::string str)
{
	if(str == "")
	{
		return false;
	} 


	std::vector<std::string> results = SplitStr(str, ",");
	int nCount = results.size();
	RebateSet.clear();  
	for (int i = 0; i < nCount; i++)
	{
		std::vector<std::string> temp_res = SplitStr(results[i], ":");
		if (temp_res.size() != 2)
		{ 
			//有错
			continue;
		}
			
		int state = atoi(temp_res[1].c_str());
		RebateSet.insert(std::make_pair(temp_res[0].c_str(), state));  
	}
	if (RebateSet.empty())
	{
		return false;
	}
	
	return true;
}
void CMiscSystem::InitAllRebatePayOneDay()
{ 
	if (!m_pEntity)
	{ 
		return;
	} 
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();	
	
	std::vector <RebateMaskInfo> maskVec;
	GetLogicServer()->GetDataProvider()->GetRebate().OnGetAllMaskInfoPayOneDay(maskVec);
	if(maskVec.empty())
	{
		return;
	}
	
	CCLVariant *pMemVar = var.get(g_RebatePayOneDay);

	if(!pMemVar)
	{
		var.set(g_RebatePayOneDay);
		pMemVar = var.get(g_RebatePayOneDay);
	}
	
	if (pMemVar)
	{ 
		for (size_t i = 0; i < maskVec.size(); i++)
		{
			char varName[64] = {0};
			sprintf(varName, "InitAllRebatePayOneDay_Id%d,index%d", maskVec[i].Id, maskVec[i].index);
			  
			pMemVar->set(varName, 1); //初始化,每天的领奖清0
		}
	} 
}
void CMiscSystem::InitAllRebatePayMoreDay()
{
	
}
void CMiscSystem::InitAllRebateConsumeOneDay()
{
	
}
void CMiscSystem::InitAllRebateConsumeMoreDay()
{
	
}
bool CMiscSystem::OnAddPayCount(int payNum)
{ 
	if (!m_pEntity)
	{ 
		return false;
	} 
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();	
 
	CCLVariant *pMemVar = var.get(g_RebateFlagVarName);

	if(!pMemVar)
	{
		var.set(g_RebateFlagVarName);
		pMemVar = var.get(g_RebateFlagVarName);
	}
	if (pMemVar)
	{
		char varName[64] = {0};
		sprintf(varName, "RebatePayId%d,index%d", 1, 2);
		pMemVar->set(varName, 1);
	}
}
bool CMiscSystem::OnAddConsumeCount(int ConsumeNum)
{ 
	if (!m_pEntity)
	{ 
		return false;
	} 
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();	
 
	CCLVariant *pMemVar = var.get(g_RebateFlagVarName);

	if(!pMemVar)
	{
		var.set(g_RebateFlagVarName);
		pMemVar = var.get(g_RebateFlagVarName);
	}
	if (pMemVar)
	{
		char varName[64] = {0};
		sprintf(varName, "RebateConsumeId%d,index%d", 1, 2);
		
		CCLVariant *pTmp = pMemVar->get(varName);
		int TempNum = ConsumeNum;
		if (pTmp)
		{
			int lastConsumeNum = (int)((double)*pTmp);
			TempNum += lastConsumeNum;
		}
		std::vector <RebateGroupInfo> RebateVec;
		GetLogicServer()->GetDataProvider()->GetRebate().GetRechargeAward(eRebateEvenType_ConsumeOneDay, TempNum, false, false, RebateVec);
 
		pMemVar->set(varName, TempNum); 
	}
}
*/