
#include "StdAfx.h"

//#include "FlyShoesConfig.h"
	CFlyShoes::CFlyShoes(){

	}
	CFlyShoes::~CFlyShoes(){

	}

    //读取配置config
    bool CFlyShoes::ReadIndexConfig(){
if (!openGlobalTable("FlyTable"))
	{
		return false;
	}

	if (enumTableFirst())
	{
		do 
		{
			int nId = (WORD)getFieldInt("id"); 
			FLYSHOESINDEX index ;
			index.nIndexItem = nId;
			index.nCost = getFieldInt("cost");

			m_Index[nId]=index;

		} while (enumTableNext());
	}
closeTable();

#ifdef _DEBUG
	OutputMsg(rmTip, _T("[%s] m_Index[1]=(%d) m_Index[1].nCost= (%d)"), "ReadIndexConfig", m_Index[1].nIndexItem, m_Index[1].nCost);
#endif

return true;
}

	//读取等级限制config
    bool CFlyShoes::ReadTableConfig(){

	if (!openGlobalTable("FlyLevel"))
	{
		return false;
	}
	int nDef = 0;
	if (enumTableFirst())
	{
		do 
		{
			int nId = (WORD)getFieldInt("id"); 
			FLYSHOESCONFIG table ;
			table.nButtonId = nId;
			table.nOpenLevel = getFieldInt("openlevel");
			table.nOpenCircle = getFieldInt("opencircle");
			table.nOpenDay = getFieldInt("openday");
			table.nShowLevel = getFieldInt("level");
			table.nShowCircle = getFieldInt("circle");
			table.nShowDay = getFieldInt("day");
			table.nMoney = getFieldInt("yuanbao");
			table.nNpcId = getFieldInt("npcid", &nDef);
			//打开子表
			if (!openFieldTable("destination"))
			{
				return false;
			}
			table.nDestination = getFieldInt("id");
			//打开子表
			if (feildTableExists("range") && openFieldTable("range"))
			{
			// for(int i = 0 ; i<8 ; i++)
			if(enumTableFirst())
			{
				int  i = 0;
				do
				{
					if (i >= sizeof(table.nPointRange)/sizeof(int)) break;
					
					table.nPointRange[i] = getFieldInt(NULL);
					i++;
				} while (enumTableNext());

			}
            closeTable();
				// table.nPointRange[i] = getFieldInt(NULL);
        	} 
			closeTable();

			m_ButtonTable[nId]=table;

		} while (enumTableNext());
	}
	closeTable();

#ifdef _DEBUG
		OutputMsg(rmTip, _T("[%s] m_ButtonTable[1].nButtonId=(%d) m_ButtonTable[1].nDestination= (%d) "), \
		"ReadTableConfig", m_ButtonTable[1].nButtonId, m_ButtonTable[1].nDestination);
#endif
	return true;

}

    //加载索引表
    bool CFlyShoes::LoadIndex(LPCTSTR sFilePath)
{
	bool boResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("%s unable to load from %s"), __FUNCTION__, sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s syntax error on Load config:%s"), __FUNCTION__, sFilePath);
		//读取标准物品配置数据
		boResult = ReadIndexConfig();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s config"), sFilePath);
	}
	//销毁脚本虚拟机
	setScript(NULL);
#ifdef _DEBUG
	
	OutputMsg(rmTip, _T("bool CFlyShoes::LoadIndex(LPCTSTR sFilePath)"));
#endif
	return boResult;
}

    //加载限制表
    bool CFlyShoes::LoadTable(LPCTSTR sFilePath)
{
	bool boResult = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("%s unable to load from %s"), __FUNCTION__, sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("%s syntax error on Load config:%s"), __FUNCTION__, sFilePath);
		//读取标准物品配置数据
		boResult = ReadTableConfig();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s config"), sFilePath);
	}
	//销毁脚本虚拟机
	setScript(NULL);
#ifdef _DEBUG
	OutputMsg(rmTip, _T("bool CFlyShoes::LoadTable(LPCTSTR sFilePath)"));
#endif
	return boResult;
}







