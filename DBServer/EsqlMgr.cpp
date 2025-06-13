#include "StdAfx.h"
#include "EsqlMgr.h"

static const LPCTSTR g_szEsqlFile = _T("./sql/");

CEsqlManager::CEsqlManager()
{
	m_DbSQl.empty();
}

CEsqlManager::~CEsqlManager()
{
	m_DbSQl.empty();
}

void CEsqlManager::EsqlMain(char* sToolPath,char* sDbName)
{
	if(!sToolPath || !sDbName) return;

	if (!FileExist(sToolPath) )
	{
		OutputMsg( rmNormal, _T("EsqlTool not exist") );
		return;
	}

	//if (!FileExist(g_szEsqlFile) )
	//{
	//	OutputMsg( rmNormal, _T("Esql file not exist") );
	//	return;
	//}

	GetAllFileName();

	OnRunEsqlTool(sToolPath,sDbName);

	DeleteEsqlFile();

	OutputMsg( rmNormal, _T("load Esql file finish") );
}

bool CEsqlManager::FileExist(LPCTSTR lpszFilePath)
{
#ifdef WIN32
	DWORD dwAttr = GetFileAttributes(lpszFilePath);
#else
	DWORD dwAttr = access(lpszFilePath, R_OK | W_OK);
#endif
	if ( dwAttr == (DWORD)-1 )
		return false;
	else 
		return true;
}

void CEsqlManager::GetAllFileName()
{
	//if (!FileExist(g_szEsqlFile) )
		//return;
#ifdef WIN32
	WIN32_FIND_DATA FindFileData; 
	HANDLE hFind = INVALID_HANDLE_VALUE; 
	char DirSpec[MAX_PATH] = ""; 
	DWORD dwError; 

	strncat (DirSpec, "*", 3); 

	hFind = FindFirstFile(DirSpec, &FindFileData); 

	if (hFind == INVALID_HANDLE_VALUE)  
	{ 
		OutputMsg( rmNormal, _T("no esql file") );
		return;
	}  
	else  
	{ 
		//OutputMsg( rmError, "First file name is %s\n", FindFileData.cFileName);
		FindNextFile(hFind, &FindFileData);

		while (FindNextFile(hFind, &FindFileData) != 0)  
		{ 
			if(bSqlFile(FindFileData.cFileName))
			{
				OutputMsg( rmNormal, "Next file name is %s", FindFileData.cFileName);
				int nId = GetEsqlId(FindFileData.cFileName);
				if(nId > 0)
				{
					AddEsqlToList(nId,FindFileData.cFileName);
				}
			}
		} 

		//for(int i =0;i<m_DbSQl.count();i++)
		//{
		//	OutputMsg( rmNormal, "m_DbSQl index=%d ",m_DbSQl[i].nId);
		//}

		dwError = GetLastError(); 
		FindClose(hFind); 
		if (dwError != ERROR_NO_MORE_FILES)  
		{ 
			OutputMsg( rmNormal, "FindNextFile error. ");
		} 
	}
#else

	DIR * dir ;
	dirent * ptr = NULL;
	char DirSpec[MAX_PATH] = ""; 
	DWORD dwError; 

	strncat (DirSpec, "*", 3);  

	dir = opendir( DirSpec ) ;
	if( dir == NULL )
	{
		OutputMsg( rmNormal, _T("no esql file") );
		return;
	}
	else  
	{

		while ((ptr = readdir( dir )) != NULL)  
		{ 
			if(bSqlFile(ptr->d_name))
			{
				OutputMsg( rmNormal, "Next file name is %s", ptr->d_name);
				int nId = GetEsqlId(ptr->d_name);
				if(nId > 0)
				{
					AddEsqlToList(nId,ptr->d_name);
				}
			}
		} 

		//for(int i =0;i<m_DbSQl.count();i++)
		//{
		//	OutputMsg( rmNormal, "m_DbSQl index=%d ",m_DbSQl[i].nId);
		//}

		dwError = GetLastError(); 
		closedir(dir) ;
		if (dwError != ENOENT)  
		{ 
			OutputMsg( rmNormal, "FindNextFile error. ");
		} 
	} 
#endif
}

bool CEsqlManager::bSqlFile(char* fileName)
{
	if(!fileName) return false;

	size_t nLen = strlen(fileName);
	if(nLen <= 4) return false;
	char* t = &fileName[nLen-5];
	if(strcmp(t,".esql") == 0) return true;

	return false;
}

int CEsqlManager::GetEsqlId(char* strEsql)
{
	char sName[255];
	if(strEsql)
	{
		int j=0;
		for(int i=0;i<strlen(strEsql);i++)
		{
			if(strEsql[i] >= 49 && strEsql[i] <58)
			{
				sName[j++] = strEsql[i];
			}
			else
			{
				break;
			}
		}
		if(j > 0 && j < 255)
		{
			sName[j] = 0;
		}

		int nId = atoi(sName);

		return nId;
	}
	return -1;
}

void CEsqlManager::AddEsqlToList(int nId,char *strEsql)
{
	if(!strEsql) return;

	DBSQL tmpEsql;
	tmpEsql.nId = nId;
	_asncpytA(tmpEsql.sFileName,strEsql);
	int index = -1;
	for(int i=0;i<m_DbSQl.count();i++)
	{
		if(tmpEsql.nId < m_DbSQl[i].nId)
		{
			index = i;
			break;
		}
	}

	if(index >= 0)
	{
		m_DbSQl.insert(index,tmpEsql);
	}
	else
	{
		m_DbSQl.add(tmpEsql);
	}
}

void CEsqlManager::OnRunEsqlTool(char* sToolPath,char* sDbName)
{
	INT_PTR nCount = m_DbSQl.count();
	if(nCount <= 0) return;

	char buf[1000];  
#ifdef WIN32
	GetCurrentDirectory(1000,buf);  //得到当前工作路径 
	strcat(buf,"\\");
#else
	getcwd(buf, 1000);  //得到当前工作路径 
	strcat(buf,"/");
#endif

	char dbId[32];

	if(sDbName)
	{
		int j=0;
		for(int i=0;i<strlen(sDbName);i++)
		{
			if(sDbName[i] >= '0' && sDbName[i] <= '9')
			{
				dbId[j++] = sDbName[i];
			}
		}
		if(j >= 0 && j < 32)
		{
			dbId[j]=0;
		}
	}
	else
	{
		dbId[0]=0;
	}
	OutputMsg( rmNormal, "GetCurrentDirectory %s,serveindex=%s",buf,dbId?dbId:"");

	for(int i = 0;i<nCount;i++)
	{
		char sPath[300];
		char curpath[1000];
		_asncpytA(curpath,buf);
		strcat(curpath,m_DbSQl[i].sFileName);

		char* filename = "";
		sprintf_s(sPath, sizeof(sPath),"%s %s %s %s",sToolPath,curpath,sDbName,dbId[0] !=0 ?dbId:"0");

		OutputMsg( rmNormal, "OnRunEsqlTool: %s",sPath);

		//找到文件的路径
		char dir[MAX_PATH];
		strcpy(dir,sToolPath);
		for(size_t j= strlen(dir) -1; j >=0; j--)
		{
			if(dir[j] =='\\')
			{
				dir[j]=0;
				break;
			}
		}
		
		SetCurrentDirectory(dir);	 
		system(sPath);
		
		Sleep(100);
		
	}
	SetCurrentDirectory(buf);
}

void CEsqlManager::DeleteEsqlFile()
{
	INT_PTR nCount = m_DbSQl.count();
	if(nCount <= 0) return;

	char buf[1000];  
#ifdef WIN32
	GetCurrentDirectory(1000,buf);  //得到当前工作路径 
	strcat(buf,"\\");
#else
	getcwd(buf, 1000);  //得到当前工作路径 
	strcat(buf,"/");
#endif
	for(int i = 0;i<nCount;i++)
	{
		char sPath[1000];
		_asncpytA(sPath,buf);
		strcat(sPath,m_DbSQl[i].sFileName);
		DeleteFileA(sPath);
	}
	m_DbSQl.empty();
}