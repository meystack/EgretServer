#include "stdafx.h"
#include "SrvConfig.h"

using namespace wylib::stream;

CSrvConfig::~CSrvConfig(void)
{
	m_platforms.empty();
}

bool CSrvConfig::ReadConfig(const char *pszFileName)
{
	//try
	//{
	//	CMemoryStream ms;
	//	ms.loadFromFile("AMServer.txt");
	//	setScript((LPCSTR)ms.getMemory());

	//	if ( openGlobalTable("AMServer"))
	//	{
	//		getFieldStringBuffer(("ServiceName"), m_ServiceName,sizeof(m_ServiceName));	
	//		if ( openFieldTable("Products"))
	//		{		
	//			if (enumTableFirst())
	//			{
	//				do 
	//				{
	//					PRODUCTCONF Product;
	//					getFieldStringBuffer(("Name"), Product.sName,sizeof(Product.sName));
	//					getFieldStringBuffer(("Spid"), Product.sSpid,sizeof(Product.sSpid));
	//					getFieldStringBuffer(("Host"), Product.sHost,sizeof(Product.sHost));
	//					Product.nPort =  getFieldInt("Port");	
	//					getFieldStringBuffer(("DBName"), Product.sDBName,sizeof(Product.sDBName));
	//					getFieldStringBuffer(("DBUser"), Product.sDBUser,sizeof(Product.sDBUser));
	//					char szTemp[100];
	//					getFieldStringBuffer(("DBPass"), szTemp,sizeof(szTemp));
	//					DecryptPassword(Product.sDBPass,sizeof(Product.sDBPass),szTemp,"123456abc123456a");
	//					memset(szTemp,0,sizeof(szTemp));
	//					getFieldStringBuffer(("PayKey"), szTemp,sizeof(szTemp));
	//					DecryptPassword(Product.sPayKey,sizeof(Product.sPayKey),szTemp,"123456abc123456a");
	//					Product.sAuthorizationIP = getFieldString("AuthorizationIP");
	//					m_ProductList.add(Product);
	//					
	//				} while (enumTableNext());
	//			}
	//			closeTable();//DB
	//		}
	//		if ( openFieldTable(("Server")))
	//		{
	//			if ( openFieldTable(("HttpServer")))
	//			{
	//				getFieldStringBuffer(("BindAddress"), m_HttpServerConf.sAddr,sizeof(m_HttpServerConf.sAddr));
	//				m_HttpServerConf.nPort = getFieldInt("Port");			
	//				closeTable();
	//			}
	//			if ( openFieldTable(("AmountServer")))
	//			{
	//				getFieldStringBuffer(("BindAddress"), m_AmountServerConf.sAddr,sizeof(m_AmountServerConf.sAddr));
	//				m_AmountServerConf.nPort = getFieldInt("Port");			
	//				closeTable();
	//			}
	//			closeTable();//DB
	//		}
	//		closeTable();//关闭Config
	//	}

	//}
	//catch(RefString &s)
	//{
	//	OutputMsg( rmNormal, s.rawStr() );
	//}
	//catch(...)
	//{
	//	OutputMsg( rmNormal, _T("unexpected error on load config") );
	//}

	OutputMsg(rmTip,"load file from %s",pszFileName);

	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(pszFileName) <= 0 )
		{
			showErrorFormat(_T("unabled to load config from %s"), pszFileName);
			bAMEngineStartSucceed = false;
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		//读取配置文件
		if ( !openGlobalTable("amconfig") )
		{
			OutputMsg(rmWaning,"No table amconfig found in config");
			bAMEngineStartSucceed = false;
			throw ;
		}
	
		int nValue = getFieldInt("httpport");
		m_nHttpport = nValue; //设置http端口
		nValue = getFieldInt("amport");
		m_nAmport = nValue ;     //设置am端口
		strncpy(m_sServerName,getFieldString("servername",""),sizeof(m_sServerName));
		if (!openFieldTable("platform"))
		{
			OutputMsg(rmWaning,"No table platform found in config");
			bAMEngineStartSucceed = false;
			throw ;
		}
		m_platforms.clear();

		size_t nCount = lua_objlen(m_pLua,-1);
		m_platforms.reserve(nCount); //保留这么多个数

		if (enumTableFirst())
		{
			int i=0;
			do 
			{
				PlatForm config;
				strncpy(config.spid,getFieldString("spid",""),sizeof(config.spid));
				strncpy(config.code,getFieldString("code",""),sizeof(config.code));
				strncpy(config.db,getFieldString("db",""),sizeof(config.db));
				strncpy(config.bianma,getFieldString("bianma","latin1"),sizeof(config.bianma));
				strncpy(config.host,getFieldString("host",""),sizeof(config.host));
			    char szTemp[128] ;
				char keykey[17] ;
				char passkey[17] ;
				keykey[0]='z',keykey[1]='*',keykey[6]='@',keykey[2]='i',keykey[7]='c',keykey[12]='%',keykey[3]='v',keykey[14]='1',
				keykey[11]='5',keykey[13]='9',keykey[8]='c',keykey[9]='3',keykey[15]='3',keykey[4]='l',keykey[5]='@',keykey[10]='6',keykey[16]=0 ;

				passkey[0]='u',passkey[1]='a',passkey[6]='z',passkey[2]='9',passkey[7]='3',passkey[12]='%',passkey[3]='k',passkey[14]='@',
				passkey[11]='&',passkey[13]='4',passkey[8]='g',passkey[9]='v',passkey[15]='8',passkey[4]='2',passkey[5]='@',passkey[10]='!',passkey[16]=0;
				getFieldStringBuffer(("user"), config.user,sizeof(config.user));
				//DecryptPassword(config.user,sizeof(config.user),szTemp,passkey);
				//memset(szTemp,0,sizeof(szTemp));
				//EncryptPassword(szTemp, sizeof(szTemp), "cqmaster", passkey);
				//DecryptPassword(szTemp,sizeof(szTemp),tmp,passkey);
				memset(szTemp,0,sizeof(szTemp));
				getFieldStringBuffer(("pass"), config.pass,sizeof(szTemp));
				//DecryptPassword(config.pass,sizeof(config.pass),szTemp,passkey);
				memset(szTemp,0,sizeof(szTemp));
				getFieldStringBuffer(("paykey"), szTemp,sizeof(szTemp));
				DecryptPassword(config.paykey,sizeof(config.paykey),szTemp,keykey);
				memset(szTemp,0,sizeof(szTemp));
				getFieldStringBuffer(("awdkey"), szTemp,sizeof(szTemp));
				DecryptPassword(config.awdkey,sizeof(config.awdkey),szTemp,keykey);
				memset(szTemp,0,sizeof(szTemp));
				getFieldStringBuffer(("awdkey"), szTemp,sizeof(szTemp));
				DecryptPassword(config.awdkey,sizeof(config.awdkey),szTemp,keykey);
				//memset(szTemp,0,sizeof(szTemp));
				//char tmp[128];
				//memset(tmp,0,sizeof(tmp));
				//EncryptPassword(tmp, sizeof(tmp), "1101994183T320140912113752", keykey);
				//DecryptPassword(szTemp,sizeof(szTemp),tmp,keykey);
				//memset(tmp,0,sizeof(tmp));
				//EncryptPassword(tmp, sizeof(tmp), "1101994183T320140912193716", keykey);
				//DecryptPassword(szTemp,sizeof(szTemp),tmp,keykey);
				//strncpy(config.pass,getFieldString("pass",""),sizeof(config.pass));
				//strncpy(config.paykey,getFieldString("paykey",""),sizeof(config.paykey));
				//strncpy(config.awdkey,getFieldString("awdkey",""),sizeof(config.awdkey));

				config.codeid =	getFieldInt("codeid");
				config.port =	getFieldInt("port");

				if (openFieldTable("hosts"))
				{
					int j=0;
					if (enumTableFirst())
					{
						do
						{
							long long nHost = inet_addr (getFieldString(NULL) );
							bool flag = true ;
							for( int k=0; k<j; ++k )
							{
								if( nHost == config.hosts[k] )
								{
									flag = false ;
									break ;
								}
							}
							if(j < MAX_HOST_COUNT && flag )
							{
								config.hosts[j]= nHost;
								j++;
							}
							if( j >= MAX_HOST_COUNT )
								break ;
						} while (enumTableNext());
					}
					config.hostcount = j;
					closeTable();
				}

				if (feildTableExists("contractid") && openFieldTable("contractid"))
				{
					int j=0;
					if (enumTableFirst())
					{
						do
						{
							memset(szTemp,0,sizeof(szTemp));
							strcpy(szTemp, getFieldString(NULL));
							DecryptPassword(config.constractid[j],sizeof(config.constractid[j]),szTemp,keykey);
							
							j++;
							if( j >= MAX_TAST_COUNT )
								break ;
						} while (enumTableNext());
					}
					config.taskcount = j;
					closeTable();
				}
				else
				{
					for(int j=0; j<MAX_TAST_COUNT; ++j)
					{
						config.constractid[j][0] = 0;
					}
					config.taskcount = 0;
				}

				m_platforms.add(config);
				i++;
			} while (enumTableNext());
		}
		closeTable();

		closeTable();

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

void CSrvConfig::ShowError(const LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[Configuration Error]");
	sErr += sError;
	//集中处理错误，为了简单起见，此处直接抛出异常。异常会在readConfig中被捕获从而立刻跳出对配置的循环读取。
	throw sErr;
}

PlatForm * CSrvConfig::GetPlatForm(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_platforms.count())
	{
		return &m_platforms[nIndex];
	}
	return NULL;
}
