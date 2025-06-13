#include "StdAfx.h"
#include "GateServerConfig.h"

void CGateServerConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CGateServerConfig::readConfig(CGateConfigList &configList, const char *pszFileName)
{
	GATECONFIG config;
	if ( !openGlobalTable(pszFileName) )
		return false;
	//遍历网关配置表，每个网关配置表均会产生一个网关对象
	if ( enumTableFirst() )
	{
		do
		{
			//读取网关服务配置表
			if ( !openFieldTable("LocalService") )
				return false;
			getFieldStringBuffer("ServerName", config.GateServer.sName, ArrayCount(config.GateServer.sName));

			//设置标题
			char buff[256];

			strcpy(buff,"lcwl-");
			strcat(buff,config.GateServer.sName);
			strcat(buff,"-V");
			in_addr ia;
			ia.s_addr = RUNGATE_KRN_VERSION;
			strcat(buff,inet_ntoa(ia));
			SetConsoleTitle(buff );

			getFieldStringBuffer("Address", config.GateServer.sAddress, ArrayCount(config.GateServer.sAddress));
			config.GateServer.nPort = getFieldInt("Port");
			config.GateServer.nMaxSession = getFieldInt("MaxSession");
			config.GateServer.SendThreadCount = getFieldInt("SendThreadCount");
			closeTable();

			//读取后台服务器配置表
			if ( !openFieldTable("BackServer") )
				return false;
			getFieldStringBuffer("Host", config.BackServer.sHost, ArrayCount(config.BackServer.sHost));
			config.BackServer.nPort = getFieldInt("Port");
			closeTable();

			//将网关配置加到配置列表中
			configList.add(config);
		}
		while (enumTableNext());
	}
	closeTable();
	return true;
}

bool CGateServerConfig::loadConfig(CGateConfigList &configList, char *pszFileName)
{

	char * szConfigFileName =pszFileName;
	if(szConfigFileName ==NULL)
	{
		szConfigFileName= _T("GateServer.txt");
	}
	OutputMsg(rmTip,"load file from %s",szConfigFileName);

	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile(szConfigFileName) <=0 )
		{
			showErrorFormat(_T("unable to load %s"), szConfigFileName );
			return false;
		}
	
		if( !setScript((LPCSTR)ms.getMemory()) )
		{
			return false;
		}
		result = readConfig(configList,"GateServer");
	}
	catch(RefString &s)
	{
		OutputMsg(rmError, s.rawStr());
	}
	catch(...)
	{
		_tprintf(_T("unexpected error on load gate config"));
	}
	return result;
}
