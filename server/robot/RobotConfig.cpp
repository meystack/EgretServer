
#include "stdafx.h"
#include "RobotConfig.h"

void CRobotConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

//在这里读取配置
bool CRobotConfig::readConfig(CRobotManager *lpSSManager)
{

	if ( !openGlobalTable("RobotConfig") )
		return false;
	lpSSManager->m_maxAccountCount = getFieldInt("loginAccountLimit");
	lpSSManager->m_nServerIndex = getFieldInt("serverIndex");  //要登陆的服务器的编号
	lpSSManager->m_groupLoginInterval = getFieldInt("groupLoginInterval");
	getFieldStringBuffer("procName", lpSSManager->m_sConsoleName, sizeof(lpSSManager->m_sConsoleName));
	
	CBaseList<SERVERCONFIG> &configs = lpSSManager->GetServerConfig(); //获取配置列表
	configs.clear();
	
	//读取网关的配置
	if ( openFieldTable("gates") ==false)
	{
		OutputMsg(rmError,_T("No table gates found"));
		return false;
	}
	else
	{
		SERVERCONFIG oneConfig; //一个配置
		INT_PTR j =0;
		if (enumTableFirst())
		{
			do
			{
				getFieldStringBuffer("ip",oneConfig.ip,sizeof(oneConfig.ip) );
				oneConfig.nPort = getFieldInt("port");
				oneConfig.nMaxPort = getFieldInt("maxPort");
				configs.add(oneConfig);
			} while (enumTableNext());
		}
		closeTable();
	}

	//读取账户的列表
	CBaseList<LOGINACCOUNT>&     accounts =  lpSSManager->GetAccountList();
	accounts.clear();
	
	if ( openFieldTable("accounts") ==false)
	{
		OutputMsg(rmError,_T("No table accounts found"));
		return false;
	}
	else
	{
		LOGINACCOUNT oneConfig; //一个配置
		char name[32]; //名字

		INT_PTR j =0;
		if (enumTableFirst())
		{
			do
			{
				getFieldStringBuffer("name",name,sizeof(oneConfig.name) );
				getFieldStringBuffer("pass",oneConfig.pass,sizeof(oneConfig.pass) );
				int nDef =-1;
				char number[16];
				//accountStartId 账户开始的ID，如果是-1表示不使用前缀表示法，使用完整账户表示法，比如helloworld
				//accountEndId   账户结束的ID, 如果是-1表示不使用前缀表示法
				int nStartId = getFieldInt("accountStartId",&nDef);
				int nEndId  = getFieldInt("accountEndId",&nDef);
				oneConfig.bDoWord = (byte)getFieldInt("doWord", &nDef);
				if(nStartId >=0  && nEndId >=0 && nEndId >= nStartId)
				{
					for(int i= nStartId ; i<= nEndId; i++ )
					{
						strcpy(oneConfig.name,name);
						
						sprintf(number,"%d",i);
						strcat(oneConfig.name,number);
						accounts.add(oneConfig);
					}
				}
				else
				{
					strcpy(oneConfig.name,name);
					accounts.add(oneConfig);
				}
				
			} while (enumTableNext());
		}
		closeTable();
	}

	// 读取地图分布配置
	readMapPoints(lpSSManager);

	closeTable();
	OutputMsg(rmNormal,"Load Config finished");
	return true;
}

bool CRobotConfig::readMapPoints(CRobotManager *lpSSManager)
{
	if (feildTableExists("mappoints") && openFieldTable("mappoints"))
	{
		INT_PTR nCount = lua_objlen(m_pLua, -1);
		if (nCount > 0 && enumTableFirst())
		{
			CBaseList<MapScenePoint>& mapPoints = lpSSManager->GetMapPointList();
			do 
			{
				MapScenePoint mp;
				mp.nSceneId = getFieldInt("sid");
				getFieldStringBuffer("sname", mp.szSceneName, ArrayCount(mp.szSceneName));
				mp.nX = getFieldInt("x");
				mp.nY = getFieldInt("y");
				mp.nLevel = getFieldInt("level");
				mapPoints.add(mp);
			} while (enumTableNext());
		}
		
		closeTable();
	}
	return true;
}

bool CRobotConfig::loadConfig(CRobotManager *lpSSManager, const char * sConfigFile)
{
	static LPCTSTR szConfigFileName = _T("./data/robot/RobotConfig.txt");
	if (NULL == sConfigFile)
	{
		sConfigFile = szConfigFileName;
	}
	
	bool result = false;

	wylib::stream::CMemoryStream ms;
	try
	{
		//加载配置文件
		if ( ms.loadFromFile(sConfigFile) <= 0 )
		{
			showErrorFormat(_T("unabled to load config from %s"), sConfigFile);
			return false;
		}
		setScript((LPCSTR)ms.getMemory());
		//读取配置文件
		result = readConfig(lpSSManager);
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
