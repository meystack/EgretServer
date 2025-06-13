#include "StdAfx.h"
#include "QuestProvider.h"
using namespace wylib::stream;

CRankProvider::CRankProvider() : m_DataAllocator(_T("RankDataAlloc"))
{
	m_RankConfigs.clear();
	nMinRankId = 3;
}
CRankProvider::~CRankProvider()
{

}

bool CRankProvider::LoadRank(LPCTSTR sFilePath)
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
			showError(_T("syntax error on quest config"));
		//读取标准物品配置数据

		Result	= ReadRankItem();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load quest config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load quest config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CRankProvider::ReadRankItem()
{
    int idx = 0;
	if ( openGlobalTable("ranktitleconfig") )
    {
        if ( enumTableFirst() )
        {
            do 
            {
                if ( enumTableFirst() )
                {
                    do 
                    {
                        
                        int rank = getFieldInt("enRankLevel", &idx);
                        int rankid = getFieldInt("rank", &idx);
                        int nId = rank *100 + rankid;
                        int titleid = getFieldInt("titleid", &idx);
                        m_RankConfigs[nId] = titleid;
                    }
                    while (enumTableNext());
                }
            }
            while (enumTableNext());
        }

        closeTable();

    }
	

    if (openGlobalTable("RankConfig") )
    {
        nMinRankId = getFieldInt("minrank", &idx);
        closeTable();
    }
	return true;
}

