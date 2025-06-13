#ifndef WIN32

#include "stdafx.h"
#include "LogConfig.h"

CLogConfig::CLogConfig(void)
{
	m_sUrl[0] = 0;
	m_nGag = 0;

	m_nSid = 0;
	m_nNumPost = 0;
	m_sPUrl[0] = 0;
	m_sgKey[0] = 0;
	m_sSign[0] = 0;
	m_nPort = 0;
}

CLogConfig::~CLogConfig(void)
{
}

char *  CLogConfig::ReadConfig()
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	LPCTSTR sFilePath="./Config.txt";
	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
		{
			return "Config.txt not found ";
		}

		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			return "syntax error ontxt";
		}

		if(openGlobalTable("Config") ==false)
		{
			return "Not global table Config";
		}

		char * pUrl=(char *)getFieldString("login_url");
		if(pUrl ==NULL)
		{
			return "login_url is null";
		}
		strncpy(m_sUrl,pUrl,sizeof(m_sUrl));

		int nGag =0;
		int nDef =0;
		nGag =getFieldInt("gagtime",&nDef);
		if(nGag ==0)
		{
			return "gagtime is 0";
		}
		m_nGag = nGag;


		int nSid =0;
		nSid =getFieldInt("sid",&nDef);
		m_nSid = nSid;

		int nNumPost =0;
		nNumPost =getFieldInt("num",&nDef);
		m_nNumPost = nNumPost;

		int nPort =0;
		nPort =getFieldInt("TcpPort",&nDef);
		m_nPort = nPort;

		char * pPUrl=(char *)getFieldString("post_url");
		if(pPUrl ==NULL)
		{
			return "post_url is null";
		}
		strncpy(m_sPUrl,pPUrl,sizeof(m_sPUrl));

		char * pgKey=(char *)getFieldString("gkey");
		if(pgKey ==NULL)
		{
			return "gkey is null";
		}
		strncpy(m_sgKey,pgKey,sizeof(m_sgKey));

		char * pSign=(char *)getFieldString("sign");
		if(pgKey ==NULL)
		{
			return "sign is null";
		}
		strncpy(m_sSign,pSign,sizeof(m_sSign));

		m_serverList.clear();

		if (feildTableExists("servers") && openFieldTable("servers"))
		{

			if (enumTableFirst())
			{
				do 
				{
					int nServerId = ((int)getFieldInt(NULL));
					m_serverList.push_back(nServerId);
				}while (enumTableNext());
			}
			closeTable();
		}

	}
	catch (RefString &s)
	{
		//OutputMsg(rmError, _T("load monster config error: %s"), s.rawStr());
		return "Error loadtxt";
	}
	catch (...)
	{
		return "Error loadtxt 2";
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return NULL;
}

void CLogConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError; 
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}

#endif
