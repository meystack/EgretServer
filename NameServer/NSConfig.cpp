#include "StdAfx.h"

void CNameServerConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CNameServerConfig::readConfig(CNameServer *lpNameServer)
{
	if ( !openGlobalTable("NameServer") )
	{
		OutputMsg(rmError,"Can not find table NameServer, load fail");
		return false;
	}
	LPCSTR sDefalut="";
	LPCSTR sVal;
	INT nVal,nDefault =0;

	//服务名称
	sVal = getFieldString("ServerName",sDefalut);
	

	nVal = getFieldInt("spguid",&nDefault); 
	if(nVal >= 256)
	{
		OutputMsg(rmError,"spid=%d超过了范围,必须是0-255之间的数",(int)nVal);
		return false;
	}
	else
	{
		OutputMsg(rmTip,"spid=%d",(int)nVal);
	}

	//设置spid编号
	lpNameServer->SetSpId(nVal);
	lpNameServer->SetServerName(sVal);

	OutputMsg(rmTip,"spid=%s,spguid=%d",sVal,nVal);
	//名称服务配置
	if ( openFieldTable("NameService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		lpNameServer->SetServiceHost(sVal);
		lpNameServer->SetServicePort(nVal);
		closeTable();
	}
	else
	{
		OutputMsg(rmError,"Can not find  table NameService, load fail");
		return false;
		
	}
	//数据库配置
	if ( openFieldTable("SQL") )
	{
		LPCSTR sDBName, sDBUser, sDBPass;
		CHAR sPlantPass[128];

		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		sDBName = getFieldString("DBName");
		sDBUser = getFieldString("DBUser");
		sDBPass = getFieldString("DBPass");

		//DecryptPassword(sPlantPass, ArrayCount(sPlantPass), sDBPass, "123456abc123456a");
		//lpNameServer->SetSQLConfig(sVal, nVal, sDBName, sDBUser, sPlantPass);
		lpNameServer->SetSQLConfig(sVal, nVal, sDBName, sDBUser, sDBPass);
		closeTable();
	}
	else
	{
		OutputMsg(rmError,"Can not find  table SQL, load fail");
		return false;
		
	}
	closeTable();
	return true;
}

bool CNameServerConfig::loadConfig(CNameServer *lpNameServer)
{
	//static LPCTSTR szConfigFileName = _T("nameserver.txt");
	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(g_ConfigPath.c_str()) <= 0 )
		{
			OutputMsg(rmError,_T("unabled to load config from %s"), g_ConfigPath.c_str());
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		//读取配置文件
		result = readConfig(lpNameServer);
	}
	catch (RefString& s)
	{
		OutputMsg( rmError, (LPCTSTR)s );
	}
	catch (...)
	{
		OutputMsg( rmError, _T("unexpected error on load config") );
	}
	return result;
}
