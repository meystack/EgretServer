#include "stdafx.h"
#include "SrvConfig.h"


using namespace wylib::stream;

CSrvConfig::CSrvConfig(void)
{
}


CSrvConfig::~CSrvConfig(void)
{
}

void CSrvConfig::ReadConfig(const char *pszFileName)
{
#ifdef WIN32
	try
#endif
	{
		OutputMsg(rmTip,"load file from %s",pszFileName);
		CMemoryStream ms;
		ms.loadFromFile(pszFileName);
		setScript((LPCSTR)ms.getMemory());

		if ( openGlobalTable("BackServer"))
		{
			int nDfaout = 0;
			nSpid = getFieldInt("spguid");
			CBackClientSocket::g_nSpid = nSpid;

			if ( openFieldTable("SQL"))
			{				
				getFieldStringBuffer(("Host"), DbConf.szHost,sizeof(DbConf.szHost));	
				DbConf.nPort =  getFieldInt("Port");	
				getFieldStringBuffer(("DBName"), DbConf.szDbName,sizeof(DbConf.szDbName));
				getFieldStringBuffer(("DBUser"), DbConf.szUser,sizeof(DbConf.szUser));
				char szTemp[100];
				//getFieldStringBuffer(("DBPass"), szTemp,sizeof(szTemp));
				//DecryptPassword(DbConf.szPassWord,sizeof(DbConf.szPassWord),szTemp,"123456abc123456a");
				getFieldStringBuffer(("DBPass"), DbConf.szPassWord, sizeof(DbConf.szPassWord));
				DbConf.nUseUtf8 = getFieldInt("utf8",&nDfaout);
				closeTable();
			}

			if ( openFieldTable(("Server")))
			{
				getFieldStringBuffer(("BindAddress"), SrvConf.szAddr,sizeof(SrvConf.szAddr));	
				SrvConf.nPort = getFieldInt("Port");			
				getFieldStringBuffer(("ServiceName"), SrvConf.szServiceName,sizeof(SrvConf.szServiceName));	
				SrvConf.nHttpPort = getFieldInt("HttpPort");
				closeTable();//DB
			}
			closeTable();//关闭Config
		}
	}
#ifdef WIN32
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
#endif
}

void CSrvConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}
