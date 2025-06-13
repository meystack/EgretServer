
#include "StdAfx.h"
#include "SSConfig.h"

//这里是一些永远ok的ip列表
static long long s_whiteip[]=
{
	inet_addr("14.23.154.74"),
	inet_addr("61.144.23.107"),
};

void CSSConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

bool CSSConfig::readConfig(CSSManager *lpSSManager)
{
	if ( !openGlobalTable("SessionServer") )
		return false;
	LPCSTR sVal;
	INT nVal=0;

	//服务名称
	sVal = getFieldString("ServerName");
	lpSSManager->SetServerName(sVal);
	
	
	
	
	nVal = getFieldInt("spguid"); //获得运营商的编号
	
	lpSSManager->SetSpGUID(nVal); //设置运营商的ID

	bool val = true;
	bool bOpen = getFieldBoolean("fcmOpen",&val); //获得运营商的编号
	lpSSManager->SetFcmOpen(bOpen);

	nVal = 0;

	//是否检测服务器的id
	nVal = getFieldInt("noserverid",&nVal); 

	lpSSManager->SetCheckServerIndex(nVal ==0);
	

	sVal = getFieldString("SPID");
	lpSSManager->SetSPID(sVal);


	int nDefalutValue = 1;
	bOpen = getFieldInt("checksign",&nDefalutValue) ?true:false; //获得运营商的编号
	lpSSManager->SetCheckPhpSign(bOpen); //是否开启
	
	TCHAR * szConfigFileName = _T("oio009okko0990jiolklk"); 
	sVal = getFieldString("key",szConfigFileName);
	lpSSManager->SetPhpKey((TCHAR*)sVal);

	nDefalutValue = 600;
	nDefalutValue = getFieldInt("phptime",&nDefalutValue); //获得运营商的编号
	lpSSManager->SetPhpTime(nDefalutValue);

	
	nDefalutValue =1;  //是否自动创建账户
	bOpen = getFieldInt("autoaccount",&nDefalutValue) ?true:false; //获得运营商的编号
	lpSSManager->SetAutoAccount(bOpen);


	nDefalutValue =0;  //是否封停账户就封IP
	bOpen = getFieldInt("sealaccountsealip",&nDefalutValue) ?true:false; //获得运营商的编号
	
	lpSSManager->SetSealAccountSealIp(bOpen); //设置封停账户就封停IP

	if(bOpen)

	{
		OutputMsg(rmError,"封停账户登陆就封停IP开启");
	}

	//单ip登陆多少就发消息到后天
	nDefalutValue =10;  //是否自动创建账户
	nDefalutValue= getFieldInt("iplog",&nDefalutValue) ; //获得运营商的编号

	lpSSManager->SetIpLogCount(nDefalutValue);


	nDefalutValue =0;
	
	//是不是md5的卡
	bOpen = getFieldInt("autocard",&nDefalutValue) ?true:false; //获得运营商的编号
	lpSSManager->SetCardMd5(bOpen);

	
	//网关服务配置
	/*
	if ( openFieldTable("GateService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		lpSSManager->SetGateServiceAddress(sVal, nVal);
		closeTable();
	}
	*/

	//会话服务配置
	if ( openFieldTable("SessionService") )
	{
		sVal = getFieldString("Address");
		nVal = getFieldInt("Port");
		lpSSManager->SetSessionServiceAddress(sVal, nVal);		
		closeTable();
	}
	//日志服务器地址配置
	if ( openFieldTable("LogServer") )
	{
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		lpSSManager->SetLogServerAddress(sVal, nVal);
		closeTable();
	}

	// AMServer配置
	if (openFieldTable("AMServer"))
	{
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		lpSSManager->SetAMServerAddr(sVal, nVal);
		closeTable();
	}

	

	//数据库配置
	if ( openFieldTable("SQL") )
	{
		LPCSTR sDBName, sDBUser, sDBPass;
		CHAR sPlantPass[128];

		memset(sPlantPass,0,sizeof(sPlantPass));
		sVal = getFieldString("Host");
		nVal = getFieldInt("Port");
		sDBName = getFieldString("DBName");
		sDBUser = getFieldString("DBUser");
		sDBPass = getFieldString("DBPass");
		
		int nDef =0;
		bool bUtf8  = getFieldInt("utf8", &nDef)?true:false; //读取是否支持utf8
		
		//DecryptPassword(sPlantPass, ArrayCount(sPlantPass), sDBPass, "JX_SS_DB_CFG_KEY");
		//lpSSManager->SetSQLConfig(sVal, nVal, sDBName, sDBUser, sPlantPass,bUtf8);
		lpSSManager->SetSQLConfig(sVal, nVal, sDBName, sDBUser, sDBPass,bUtf8);
		
		closeTable();
	}
	closeTable();
	return true;
}

bool CSSConfig::loadConfig(CSSManager *lpSSManager)
{
	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(g_ConfigPath.c_str()) <= 0 )
		{
			showErrorFormat(_T("unabled to load config from %s"), g_ConfigPath.c_str());
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		//读取配置文件
		result = readConfig(lpSSManager);

		//读取GM限制列表
		readGMConfig(lpSSManager);

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

//读取GM配置

bool CSSConfig::readGMConfig(CSSManager *lpSSManager)
{

	static LPCTSTR szConfigFileName = _T("GM.txt");
	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(szConfigFileName) <= 0 )
		{
			showErrorFormat(_T("unabled to load config from %s"), szConfigFileName);
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		
		if ( !openGlobalTable("GMConfig") )
		{
			OutputMsg(rmWaning,_T("GMConfig table not found"));
			return false;
		}

		int nDefalut=20;

		//设置1个ip可以登陆多少个账户
		int nCount  = getFieldInt("loginlimit",&nDefalut);

		lpSSManager->SetIpLoginCount(nCount);

		nDefalut =0;  //只要白名单成员才能通过

		int nWitePass  = getFieldInt("whitepass",&nDefalut);
		bool whitePassflag =false;

		if (nWitePass) whitePassflag=true;

		//设置是否只要白名单才能登陆
		lpSSManager->SetWhitePass(whitePassflag);

		//单ip如果登陆连续间隔低于这个值，认定为外挂，则需要直接封账户，封ip
		nDefalut=10;

		//设置登陆间隔，如果平均登陆的次数低于这些，则直接封账户，封IP，封
		nCount  = getFieldInt("interval",&nDefalut);
		lpSSManager->SetForbidInterval(nCount);

		nDefalut=5;
		nCount  = getFieldInt("kickcount",&nDefalut);
		lpSSManager->SetKickCount(nCount);

		nDefalut=0;
		nCount  = getFieldInt("gmip",&nDefalut);
		lpSSManager->SetGmIpLogin(nCount?true:false);
		
		nDefalut=0;
		nCount  = getFieldInt("autoseal",&nDefalut);
		lpSSManager->SetAutoSealIp(nCount?true:false);



		CBaseList<unsigned long long>&  gmips= lpSSManager->GetGmIpList(); //获取ip列表
		if (feildTableExists("gmloginip") && openFieldTable("gmloginip"))
		{		
			
		
			gmips.clear();

			if (enumTableFirst())
			{
				INT_PTR nIdx = 0;
				do 
				{
					char ipStr[128];
					getFieldStringBuffer(NULL, ipStr,sizeof(ipStr));
					//char *pStr = (char*)malloc(strlen(ipStr) +1);
					
					unsigned long long sip = inet_addr(ipStr);

					
					OutputMsg(rmTip,"Add GM ip=%s",ipStr);
					gmips.add(sip);

				} while (enumTableNext());
			}
			closeTable();
			
		}
		AddAlwayWhiteIp(gmips);

		CBaseList<unsigned long long>& robotIps= lpSSManager->GetCreateAcountIpList(); //获取ip列表

		if (feildTableExists("robotip") && openFieldTable("robotip"))
		{		
			
			robotIps.clear();

			if (enumTableFirst())
			{
				INT_PTR nIdx = 0;
				do 
				{
					char ipStr[128];
					getFieldStringBuffer(NULL, ipStr,sizeof(ipStr));
					//char *pStr = (char*)malloc(strlen(ipStr) +1);

					unsigned long long sip = inet_addr(ipStr);

					OutputMsg(rmTip,"Add robot ip=%s",ipStr);
					robotIps.add(sip);

				} while (enumTableNext());
			}
			closeTable();

			
		}
		AddAlwayWhiteIp(robotIps);
		
		CBaseList<unsigned long long>&  blackips= lpSSManager->GetBlackIpList(); //获取ip列表

		if (feildTableExists("blackip") && openFieldTable("blackip"))
		{		
			

			//blackips.clear();
			CSessionServer* pSrv= lpSSManager->getSessionServer();
			
			if (enumTableFirst())
			{
				INT_PTR nIdx = 0;
				do 
				{
					char ipStr[128];
					getFieldStringBuffer(NULL, ipStr,sizeof(ipStr));
					//char *pStr = (char*)malloc(strlen(ipStr) +1);

					unsigned long long sip = inet_addr(ipStr);
					bool flag =false;
					for(INT_PTR i=0;i < blackips.count() ; i++)
					{
						if(blackips[i] == sip)
						{
							flag =true;
							break;
						}
					}
					if(!flag)
					{
						OutputMsg(rmTip,"Add black ip=%s",ipStr);
						blackips.add(sip);
					}

					//OutputMsg(rmTip,"Add black ip=%s",ipStr);
					//blackips.add(sip);
					
				} while (enumTableNext());
			}
			closeTable();
		}

		CBaseList<unsigned long long>&  whiteips= lpSSManager->GetWhiteIpList(); //获取ip列表
		if (feildTableExists("whiteip") && openFieldTable("whiteip"))
		{		


			whiteips.clear();

			if (enumTableFirst())
			{
				INT_PTR nIdx = 0;
				do 
				{
					char ipStr[128];
					getFieldStringBuffer(NULL, ipStr,sizeof(ipStr));
					//char *pStr = (char*)malloc(strlen(ipStr) +1);

					unsigned long long sip = inet_addr(ipStr);

					OutputMsg(rmTip,"Add whiteip ip=%s",ipStr);
					whiteips.add(sip);

				} while (enumTableNext());
			}
			closeTable();
		}
		AddAlwayWhiteIp(whiteips);
		return true;

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

void CSSConfig::AddAlwayWhiteIp(CBaseList<unsigned long long > & ips)
{
	
	
	static int nSize = ArrayCount(s_whiteip);
	for(int i=0; i< nSize; i++)
	{
		unsigned long long nIp = s_whiteip[i];

		bool flag =false;
		for(int j=0; j < ips.count(); j++)
		{
			if(nIp == ips[j])
			{
				flag =true;
				break;
			}
		}
		if( !flag)
		{
			ips.add(nIp);
		}
	}
	
}
