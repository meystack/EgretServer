
#include "StdAfx.h"
CDBConfig::CDBConfig(void)
{
}


CDBConfig::~CDBConfig(void)
{
}


//读取跨服配置表
bool CDBConfig::LoadCrossServerConfig()
{
	/*
	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile("crossserver.txt") <= 0 )
		{
			showError(_T("unable to load config from file crossserver.txt"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		if ( openGlobalTable("crossserver"))
		{
			int nCenterServerIndex = 0;
			if ( openFieldTable("map"))
			{	
				size_t nCount = lua_objlen(m_pLua,-1);

				if ( enumTableFirst() )
				{
					do 
					{
						int nStart = getFieldInt("start");
						int nEnd = getFieldInt("idend");
						int nCenterid = getFieldInt("centerid");

						if(ServerIndex >= nStart && ServerIndex <= nEnd && ServerIndex != nCenterid)
						{
							nCenterServerIndex = nCenterid;
							//break;
						}
					}while (enumTableNext());
				}

				closeTable();
			}

			if(nCenterServerIndex > 0)
			{
				if ( openFieldTable("center"))
				{		
					size_t nCount = lua_objlen(m_pLua,-1);

					if ( enumTableFirst() )
					{
						ZeroMemory(&DBCenterAddr, sizeof(DBCenterAddr));

						INT_PTR nIdx = 0;
						do 
						{		
							// 读取DBCenter服务器配置
							int nServerId = getFieldInt("id");
							if(nServerId == nCenterServerIndex)
							{
								getFieldStringBuffer("commondbsrv", DBCenterAddr.szAddr, sizeof(DBCenterAddr.szAddr));
								DBCenterAddr.nPort = getFieldInt("dbport");
							//	break;
							}

							nIdx++;
						}while (enumTableNext());
					}
					closeTable();//GateService
				}
			}
			
			closeTable();//关闭DBServer
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}
	*/
	//数据服不再读取配置了
	return true;
}

bool CDBConfig::ReadConfig(const char *pszConfigFileName)
{
	if (!pszConfigFileName)
		pszConfigFileName = "DBServerLinux.txt";
	bool result = false;
	CMemoryStream ms;
	try
	{
		if ( ms.loadFromFile(pszConfigFileName) <= 0 )
		//	if ( ms.loadFromFile("E:/WorkSpace/WorkSVN/tcgp/trunk/Phoenix/server/build/DBServer/DBServer.txt") <= 0 )
		{
			showError(_T("unable to load config from file DBServerLinux.txt"));
			return result;
		}
		if ( !setScript((LPCSTR)ms.getMemory()) )
		{
			showError(_T("parse config script failed"));
			return false;
		}
		if ( openGlobalTable("DBServer"))
		{
			getFieldStringBuffer(("ServerName"), ServerName,sizeof(ServerName));	

			ServerIndex = getFieldInt("ServerIndex");

			getFieldStringBuffer("BoyNameFile", m_sPrefixList, sizeof(m_sPrefixList));
			//getFieldStringBuffer("NameConnectList", m_sConnectList, sizeof(m_sConnectList));
			getFieldStringBuffer("GirlNameFile", m_sStuffixList, sizeof(m_sStuffixList));

			LPCTSTR stemp = getFieldString("EsqlToolPath","D:\\cqAdmin\\ESQL\\ESQLTool.exe");

			_asncpytA(m_EsqlToolPath,stemp);
			m_EsqlToolPath[sizeof(m_EsqlToolPath)-1] = 0;

			int nDef =0;
			DbConf.boUtf8=  getFieldInt("utf8", &nDef ) ?true:false; //数据库的编码是不是utf8
			if ( openFieldTable("GateService"))
			{				
				getFieldStringBuffer(("Address"), GateAddr.szAddr,sizeof(GateAddr.szAddr));	
				GateAddr.nPort = getFieldInt("Port");	
				closeTable();//GateService
			}
				
			if ( openFieldTable("DBService"))
			{				
				getFieldStringBuffer(("Address"), DataAddr.szAddr,sizeof(DataAddr.szAddr));	
				DataAddr.nPort = getFieldInt("Port");	
				closeTable();//DBService 
			}
			if ( openFieldTable("LogServer"))
			{				
				getFieldStringBuffer(("Address"), LogAddr.szAddr,sizeof(LogAddr.szAddr));	
				LogAddr.nPort = getFieldInt("Port");	
				closeTable();//LogServer 
			}
			
			/*
			if ( openFieldTable("SessionServer"))
			{				
				getFieldStringBuffer(("Address"), SessionAddr.szAddr,sizeof(SessionAddr.szAddr));	
				SessionAddr.nPort = getFieldInt("Port");	
				closeTable();//SessionServer  
			}
			*/

			
			if ( openFieldTable("NameServer"))
			{				
				getFieldStringBuffer(("Address"), NameAddr.szAddr,sizeof(NameAddr.szAddr));	
				NameAddr.nPort = getFieldInt("Port");	
				closeTable();//NameServer  
			}

			// 读取DBCenter服务器配置
			/*
			去掉了dbsencter的读取
			ZeroMemory(&DBCenterAddr, sizeof(DBCenterAddr));
			if (feildTableExists("DBCenter") && openFieldTable("DBCenter"))
			{
				getFieldStringBuffer("Address", DBCenterAddr.szAddr, sizeof(DBCenterAddr.szAddr));
				DBCenterAddr.nPort = getFieldInt("Port");
				closeTable();
			}
			*/

			if ( openFieldTable("SQL"))
			{				
				getFieldStringBuffer(("Host"), DbConf.szHost,sizeof(DbConf.szHost));	
				DbConf.nPort = getFieldInt("Port");	
				getFieldStringBuffer(("DBName"), DbConf.szDbName,sizeof(DbConf.szDbName));
				getFieldStringBuffer(("DBUser"), DbConf.szUser,sizeof(DbConf.szUser));
				char szTemp[100];
				//getFieldStringBuffer(("DBPass"), szTemp,sizeof(szTemp));
			//	getFieldStringBuffer(("Key"), DbConf.szKey,sizeof(DbConf.szKey));				
				//密码要解密
				//DecryptPassword(DbConf.szPassWord,sizeof(DbConf.szPassWord),szTemp,"123456abc123456a");
				getFieldStringBuffer(("DBPass"), DbConf.szPassWord,sizeof(DbConf.szPassWord));
				closeTable();//DB
				result = true;
			}
			closeTable();//关闭DBServer
		}
	}
	catch(RefString &s)
	{
		OutputMsg( rmNormal, s.rawStr() );
	}
	catch(...)
	{
		OutputMsg( rmNormal, _T("unexpected error on load config") );
	}

	if(result)
	{
		result = LoadCrossServerConfig();
	}
	return result;
}

void CDBConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}
