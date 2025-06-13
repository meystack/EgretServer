#include "StdAfx.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

ConsignmentProvider::ConsignmentProvider()
	: Inherited(), m_DataAllocator(_T("ConsignmentDataAlloc"))
{
	
}

ConsignmentProvider::~ConsignmentProvider()
{
}

bool ConsignmentProvider::LoadConsignConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on Consignment config"));
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Consignment config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Consignment config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool ConsignmentProvider::readConfigs()
{
	CDataAllocator dataAllocator;
	int nDefault = 0;
	double fDefault =0;
	if ( openGlobalTable("taxconfig") )
	{
		nMinSellPrice = getFieldInt("minSellPrice",&nDefault);
		nSellCost = getFieldInt("cost",&nDefault);
		nMaxSellPrice = getFieldInt("maxSellPrice",&nDefault);
		//nPageCount = getFieldInt("pageCount",&nDefault);
		nMaxShelfCount = getFieldInt("maxShelfCount",&nDefault);
		nMaxServerCont = getFieldInt("maxSeverCount",&nDefault);
		nSellTime = getFieldInt("sellTime",&nDefault) ;
		// nOpenLevel = getFieldInt("openlevel",&nDefault);
		fTaxRate = (float)getFieldNumber("tax", &fDefault);
		nOpenLevel = getFieldInt("openlv",&nDefault);
		nDealCd = getFieldInt("cd", &nDefault);
		nPrivateDealDistance = getFieldInt("distance", &nDefault);
		nMinDealLevel = getFieldInt("minlevel", &nDefault);
		nDuty = getFieldInt("duty", &nDefault);
		nDues = getFieldInt("dues", &nDefault);
		getFieldStringBuffer("tips", sDealLimitTips,ArrayCount(sDealLimitTips));
		if (feildTableExists("Auctiondemand") && openFieldTable("Auctiondemand"))
		{
			if(enumTableFirst())
			{
				int nId = 0;
				do
				{
					if(nId >= 2)
						break;
					nConsignLevel[nId] = getFieldInt(NULL);
					nId++;
				} while (enumTableNext());
				
			}
			closeTable();
		}
		if (feildTableExists("Jiaodemand") && openFieldTable("Jiaodemand"))
		{
			if(enumTableFirst())
			{
				int nId = 0;
				do
				{
					if(nId >= 2)
						break;
					nDealLevel[nId] = getFieldInt(NULL);
					nId++;
				} while (enumTableNext());
				
			}
			closeTable();
		}
		closeTable();
	}
	m_nAutoInConsignmentLists.clear();
	if ( openGlobalTable("TaxAutomaticConfig") )
	{

		if(enumTableFirst())
		{
			int nId = 0;
			do
			{
				TaxAutomaticConfig cfg;
				cfg.class1Id = getFieldInt("order", &nDefault);
				cfg.nOpenday = getFieldInt("openday", &nDefault);
				cfg.nEndday = getFieldInt("endday", &nDefault);
				cfg.nSpace = getFieldInt("space", &nDefault);
				cfg.nId = getFieldInt("id", &nDefault);
				cfg.nNumber = getFieldInt("number", &nDefault);
				cfg.nPrice = getFieldInt("price", &nDefault);
				m_nAutoInConsignmentLists[cfg.class1Id]= cfg;
			} while (enumTableNext());
			
		}
		closeTable();
	}


	

	// if ( !openGlobalTable("demandConfig") )
	// 	return false;
	// nOpenLevel = getFieldInt("openlv",&nDefault);
	// closeTable();
	//原来的内存块释放到
	m_DataAllocator.~CObjectAllocator();
	//将临时的内存块复制到正式的
	m_DataAllocator = dataAllocator;
	//清空临时的
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));

	return true;
}


void ConsignmentProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[ConsignmentProvider]");
	s += sError;
	throw s;
}
//获取二级分类所在的一级分类id
int ConsignmentProvider::getConsignTypeClass1Id(int nConsignType)
{
	for (INT_PTR i =0; i< classList.count; i++)
	{
		ConsingmentClass& class1 = classList.pData[i];
		for (INT_PTR j=0; j < class1.class2List.count; j++)
		{
			int nClass2Id = class1.class2List.pData[j];
			if (nClass2Id == nConsignType)
			{
				return class1.class1Id;
			}
		}
	}
	//OutputMsg(rmError,"getConsignTypeClass1Id error %d", nConsignType);太频繁,屏蔽了
	return 0;
}
