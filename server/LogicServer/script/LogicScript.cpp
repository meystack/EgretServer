#include "StdAfx.h"
#include "LogicScript.h"
#include "interface/LuaHelpExportFun.h"

extern  int luaopen_server (lua_State* tolua_S);

LPCSTR CLogicScript::ScriptInitFnName = "initialization";
LPCSTR CLogicScript::ScriptUninitFnName = "finalization";
LPCTSTR CLogicScript::ScriptInstanceKeyName = "#LSKK#";
CActorPacket *CLogicScript::ScriptActorPacket = NULL;

CLogicScript::CLogicScript()
	:Inherited()
{
	m_pNpcObject = NULL;
	m_sLastCall = NULL;
}

CLogicScript::~CLogicScript()
{
	if(m_pNpcObject)
	{
		/*
		if(m_pNpcObject->GetScene())
		{
			OutputMsg(rmTip,"~CLogicScript,scene=%s,name=%s",m_pNpcObject->GetScene()->GetSceneName(),m_pNpcObject->GetEntityName());
		}
		else
		{
			OutputMsg(rmTip,"~CLogicScript,name=%s",m_pNpcObject->GetEntityName());
		}
		*/

	}
	m_pNpcObject =NULL;
}

bool CLogicScript::setScript(LPCSTR sText, CSourceLineList * pLRList)
{
	m_LRList = *pLRList;
	
	bool result =Inherited::setScript(sText);
	return result;
}

bool CLogicScript::FunctionExists(LPCSTR sFnName)
{
	if ( !m_pLua || !sFnName || !sFnName[0] )
		return false;

	lua_getglobal(m_pLua, sFnName);
	bool result = lua_isfunction(m_pLua, -1) != 0;
	lua_pop(m_pLua, 1);
	return result;
}

bool CLogicScript::Call(LPCSTR sFnName, CScriptValueList &args, CScriptValueList &results, INT_PTR nResultCount)
{
	//SF_TIME_CHECK();
	SF_TIME_CHECK_NAME(sFnName, 1)
	bool Result = true;
	if(m_pLua ==NULL)
	{
		OutputMsg(rmError,_T("调用了未初始化的脚本%s"),sFnName);
		DbgAssert(FALSE);
		return false;
	}
	/** 检查上次脚本调用所由脚本申请的数据包是否被回收 **/
	DbgAssert(ScriptActorPacket == NULL);

	int nArgCount = &args ? (int)(args.count()) : 0;
	int nTop = lua_gettop(m_pLua);//获取当前的栈顶

	//压入错误处理函数
	lua_pushcfunction(m_pLua, ScriptCallErrorDispatcher);

	//压入函数以及参数
	m_sLastCall = sFnName;
	lua_getglobal(m_pLua, sFnName);
	if ( nArgCount > 0 )
		args.pushArgs(m_pLua, nArgCount);

#ifndef _DEBUG //bug版本不捕获异常，以便生成dump来更精确的定位问题
#ifdef WIN32
	__try
#endif
#endif
	{
		int nErr = lua_pcall(m_pLua, nArgCount, (int)nResultCount, -nArgCount - 2);
		//如果参数表和返回值表是同一个对象，则清空参数表。
		if ( &args == &results )
			args.clear();

		//如果脚本执行错误则进行错误处理
		if ( nErr )
		{
			ScriptCallError(sFnName, nResultCount > 0 ? results : *((CScriptValueList*)NULL));
			Result = false;
		}
		//脚本执行成功则取出返回值
		else
		{
			//如果期望的返回值是不限个，则计算本次函数调用的返回值数量
			if ( nResultCount == LUA_MULTRET )
				nResultCount = lua_gettop(m_pLua) - nTop;
			if ( nResultCount > 0  )
			{
				results.getResults(m_pLua, (int)nResultCount);
			}
		}
	}
#ifndef _DEBUG
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		Result = false;
	}
#endif
#endif
	m_sLastCall = NULL;
	//恢复堆栈
	lua_settop(m_pLua, nTop);
	/*
	   如果脚本出错且数据包没有被回收，则会导致整个游戏引擎就此不能正常工作。因此必须检测并回收脚本申请的数据包
	*/
	if (ScriptActorPacket){
			ScriptActorPacket->rollBack();
		ScriptActorPacket = NULL;
		OutputMsg(rmWaning, _T("[%s]collected an ActorPacket that because script error occured and the packet can not be flushed"),sFnName);
	}
	return Result;
}

bool CLogicScript::CallModule(LPCSTR sModuleName,LPCSTR sFnName, CScriptValueList &args, CScriptValueList &results, INT_PTR nResultCount)
{
	//SF_TIME_CHECK();
	SF_TIME_CHECK_NAME(sFnName, 1)
	bool Result = true;
	if(m_pLua ==NULL)
	{
		OutputMsg(rmError,_T("调用了未初始化的脚本%s"),sFnName);
		DbgAssert(FALSE);
		return false;
	}
	/** 检查上次脚本调用所由脚本申请的数据包是否被回收 **/
	DbgAssert(ScriptActorPacket == NULL);

	int nArgCount = &args ? (int)(args.count()) : 0;
	int nTop = lua_gettop(m_pLua);//获取当前的栈顶

	//压入错误处理函数
	lua_pushcfunction(m_pLua, ScriptCallErrorDispatcher);

	//压入函数以及参数
	lua_getglobal(m_pLua, sModuleName);
	if (!lua_istable(m_pLua, -1))
	{
		OutputMsg(rmError,_T("模块不存在：%s"),sModuleName);
		DbgAssert(FALSE);
		return false;
	}
	m_sLastCall = sFnName;
	lua_getfield(m_pLua, -1, sFnName);
	if (!lua_isfunction(m_pLua, -1))
	{
		OutputMsg(rmError,_T("非函数：%s,%s"),sModuleName,sFnName);
		DbgAssert(FALSE);
		return false;
	}
	
	//压入参数
	if ( nArgCount > 0 )
		args.pushArgs(m_pLua, nArgCount);

#ifndef _DEBUG //bug版本不捕获异常，以便生成dump来更精确的定位问题
#ifdef WIN32
	__try
#endif
#endif
	{
		int nErr = lua_pcall(m_pLua, nArgCount, (int)nResultCount, -nArgCount - 3);
		//如果参数表和返回值表是同一个对象，则清空参数表。
		if ( &args == &results )
			args.clear();

		//如果脚本执行错误则进行错误处理
		if ( nErr )
		{
			ScriptCallError(sFnName, nResultCount > 0 ? results : *((CScriptValueList*)NULL));
			Result = false;
		}
		//脚本执行成功则取出返回值
		else
		{
			//如果期望的返回值是不限个，则计算本次函数调用的返回值数量
			if ( nResultCount == LUA_MULTRET )
				nResultCount = lua_gettop(m_pLua) - nTop;
			if ( nResultCount > 0  )
			{
				results.getResults(m_pLua, (int)nResultCount);
			}
		}
	}
#ifndef _DEBUG
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		Result = false;
	}
#endif
#endif
	m_sLastCall = NULL;
	//恢复堆栈
	lua_settop(m_pLua, nTop);
	/*
	   如果脚本出错且数据包没有被回收，则会导致整个游戏引擎就此不能正常工作。因此必须检测并回收脚本申请的数据包
	*/
	if (ScriptActorPacket){
			ScriptActorPacket->rollBack();
		ScriptActorPacket = NULL;
		OutputMsg(rmWaning, _T("[%s]collected an ActorPacket that because script error occured and the packet can not be flushed"),sFnName);
	}
	return Result;
}

bool CLogicScript::LoadScript(LPCTSTR sFilePath)
{
	bool bResult = true;
#ifndef _DEBUG //bug版本不捕获异常，以便生成dump来更精确的定位问题
#ifdef WIN32
	__try	
#endif
#endif
	{

		wylib::stream::CMemoryStream ms;
		if (ms.loadFromFile(sFilePath) <= 0)
		{
			OutputMsg(rmError, _T("unable to load Script file %s"), sFilePath);
			return false;
		}

		//对脚本进行预处理
		CCustomLuaPreProcessor pp;
		CLogicServer *pLogicServer = GetLogicServer();
		if (pLogicServer)
		{
			pLogicServer->GetVSPDefine().RegisteToPreprocessor(pp);
		}
		LPCSTR sScript = pp.parse((LPCSTR)ms.getMemory(),sFilePath);
		if ( !sScript )
		{
			OutputMsg(rmError, _T("script file pre-process fail") );
			return false;
		}
		wylib::misc::CRefObject<CCustomLuaPreProcessor::CLineRangeList> LRList;
		INT_PTR nLRCount = pp.getLineRangeData(NULL, 0);
		if (nLRCount > 0)
		{
			LRList = new wylib::misc::CRefObjectImpl<CCustomLuaPreProcessor::CLineRangeList>();
			LRList.raw_ptr()->reserve(nLRCount);
			pp.getLineRangeData(*LRList.raw_ptr(), nLRCount);
			LRList.raw_ptr()->trunc(nLRCount);
		}
		//设置脚本
		return setScript(sScript, &LRList);
	}
#ifndef _DEBUG
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		bResult = false;
	}
#endif	
#endif
	return bResult;
}


bool CLogicScript::CheckScript(LPCTSTR sFilePath, char* sErr)
{
#ifdef WIN32
	wylib::stream::CMemoryStream ms;
	if (ms.loadFromFile(sFilePath) <= 0)
	{
		OutputMsg(rmError, _T("unable to load Script file %s"), sFilePath);
		return false;
	}

	//对脚本进行预处理
	CCustomLuaPreProcessor pp;
	CLogicServer *pLogicServer = GetLogicServer();
	if (pLogicServer)
	{
		pLogicServer->GetVSPDefine().RegisteToPreprocessor(pp);
	}
	LPCSTR sScript = pp.parse((LPCSTR)ms.getMemory(),sFilePath);
	if ( !sScript )
	{
		OutputMsg(rmError, _T("script file pre-process fail") );
		return false;
	}
	CSourceLineList LRList;
	INT_PTR nLRCount = pp.getLineRangeData(NULL, 0);
	if (nLRCount > 0)
	{
		LRList = new wylib::misc::CRefObjectImpl<CCustomLuaPreProcessor::CLineRangeList>();
		LRList.raw_ptr()->reserve(nLRCount);
		pp.getLineRangeData(*LRList.raw_ptr(), nLRCount);
		LRList.raw_ptr()->trunc(nLRCount);
	}
	// "[string "--[[File: GloabalFunction.txt]]..."]:33191: '}' expected (to close '{' at line 33188) near '{'"
	
	int nErr = luaL_dostring(m_pLua, sScript) ;
	if (nErr > 0 || sErr != NULL)
	{
		char newErrBuff[1024];
		char errBuff[1024];
		if (sErr != NULL)
		{
			sprintf(errBuff,"%s",sErr);
		}
		else
		{
			sprintf(errBuff,"%s",lua_tostring(m_pLua,-1));
			lua_pop(m_pLua,1);
		}
	
		char* pErrStart = strstr(errBuff,"]:")+2;
		if (!pErrStart)
		{
			return false;
		}
		char* pErr = pErrStart;
		char* pNum = pErrStart;
		bool bNumStart = false;
		bool bHasFirstNumber = false;
		while (*pErr!='\0')
		{
			if (*pErr >='0' && *pErr <= '9')
			{
				if (!bNumStart)
				{
					pNum = pErr;
					bNumStart = true;
				}				
			}
			else
			{
				if (bNumStart)
				{
					if (pNum != NULL)
					{
						//只有这两种格式的数字当作行数 :33191:  at line 33188
						if( (*(pNum-1)==':' && *pErr==':')||
							strncmp(pNum-5,"line ",5)==0)
						{
							char sNum[256];
							//pNum[psErr-pNum] =0;
							memmove(sNum,pNum,pErr-pNum);
							if (bHasFirstNumber)
							{
								strncat(newErrBuff,pErrStart,pNum-pErrStart);
							}		
							pErrStart = pErr;

							int nCurLineNumber = atoi(sNum);
							INT_PTR nSourceLineNumber =0;
							CCustomLuaPreProcessor::CLineRangeList *pLR = LRList;
							LPCSTR sSourceFileName = pLR->getSourceLineNumber(nCurLineNumber,nSourceLineNumber);
							if (!bHasFirstNumber)
							{
								//取第一个报错的行定位对应文件
								if (nSourceLineNumber > 0)
								{
									char longFilePathName[MAX_PATH];
									GetLongPathName(sSourceFileName,longFilePathName,sizeof(longFilePathName));
									sprintf(newErrBuff,"%s,\r\n[File: %s]:\r\n",sFilePath, longFilePathName);
								}
								bHasFirstNumber = true;
							}
							sprintf(newErrBuff,"%s%d",newErrBuff,nSourceLineNumber);
						}
						pNum = NULL;
					}
					bNumStart = false;
				}

			}
			pErr++;
		}
		strncat(newErrBuff,pErrStart,pErr-pErrStart);
		strcat(newErrBuff,"\r\n");
		OutputMsg(rmError,newErrBuff);
	}
	return nErr > 0? false:true;
#endif
}
//#ifdef _DEBUG
void CLogicScript::SaveProfiler(LPCTSTR sFilePath)
{
	wylib::stream::CMemoryStream ms;
	m_Profiler.Save(ms);
	ms.saveToFile(sFilePath);
}
//#endif

int CLogicScript::ScriptCallErrorDispatcher(lua_State *L)
{
	lua_getglobal(L, ScriptInstanceKeyName);
	CLogicScript *s = (CLogicScript*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return s ? s->HandleCallError() : 0;
}

void CLogicScript::ScriptCallError(LPCSTR sFnName, CScriptValueList &results)
{
	CHAR sErrBuf[1024];
	LPCSTR sErrDesc = NULL;

	//从lua中获取错误描述
	INT nTop = lua_gettop(m_pLua);
	if ( nTop > 0 )
		sErrDesc = lua_tostring(m_pLua, -1);
	if ( !sErrDesc ) sErrDesc = "Undefined Error";
	
	LPCSTR sActorName, sSceneName;
	
	if (m_pNpcObject)
	{
		sActorName = m_pNpcObject->GetEntityName();//获取角色名称
		CScene *pScene = m_pNpcObject->GetScene();
		sSceneName = pScene ? pScene->GetSceneName() : "(null)";//获取所在场景名称
	}
	else sActorName = sSceneName = "(null)";
	SIZE_T nlen = _snprintf(sErrBuf, ArrayCount(sErrBuf)-1, "[Script]NPC(%s:%s) Error %s", 
		sSceneName, sActorName, sErrDesc);
	//nlen += _snprintf(sErrBuf, ArrayCount(sErrBuf)-1-nlen, m_sLastCall);


	sErrBuf[nlen] = 0;

	//从lua栈中弹出错误描述
	if ( nTop > 0 )
		lua_pop(m_pLua, 1);

	//如果返回值列表对象非空则写入错误值
	if ( &results )
		results << sErrBuf;
	//删除错误描述
	showError(sErrBuf);
}

int CLogicScript::HandleCallError()
{
	CHAR sErrBuf[1024];

	lua_Debug ar;
	INT_PTR nlen = 0, nLvl = 0, nLineNo;
	const char *sFileName, *sTag;
	CCustomLuaPreProcessor::CLineRangeList *pLR = m_LRList;
	//#ifdef _DEBUG
#ifdef WIN32
	WIN32_FIND_DATA fd;
#else
	DIR * dir ;
	dirent * ptr ;
#endif
	//#endif
	//定位到错误内容中只包含错误描述的部分，调过简单内容以及行号
	sFileName = lua_tostring(m_pLua, 1);
	sTag = strrchr(sFileName, ':');
	if (sTag) sFileName = sTag + 2;

	//拷贝错误内容
	nlen = strlen(sFileName);
	if (nlen >= ArrayCount(sErrBuf)) nlen = ArrayCount(sErrBuf)-1;
	strncpy(sErrBuf, sFileName, nlen);

	nlen += _snprintf(sErrBuf+nlen, ArrayCount(sErrBuf)-1-nlen, "\r\nthe stack trace is:\r\n");	

	while (lua_getstack(m_pLua, (int)nLvl, &ar))
	{
		lua_getinfo(m_pLua, "Snl", &ar);
		//lua:field:name(file:line)
		nlen += _snprintf(sErrBuf+nlen, ArrayCount(sErrBuf)-1-nlen, "  %s:%s:%s",
			ar.what, ar.namewhat, ar.name ? ar.name : "<NA>");
		//获取源代码位置
		sFileName = NULL;
		if (ar.source[0]=='@')
		{
			std::string filename = ar.source;
			int pos = filename.find_last_of('/');
			filename.erase(0,pos+1);
			sFileName = filename.c_str();
			nLineNo = ar.currentline;
		}
		else if (pLR)
		{
			sFileName = pLR->getSourceLineNumber(ar.currentline > -1 ? ar.currentline : ar.linedefined, nLineNo);
			if (sFileName)
			{
				//由于文件名可能被缩短成短文件，因此做一次搜索以便确定更为完整的文件名。
#ifdef WIN32
				HANDLE hd = FindFirstFile(sFileName, &fd);
				if (hd != INVALID_HANDLE_VALUE)
				{
					sFileName = fd.cFileName;
					FindClose(hd);
				}
				else
				{
					sFileName = strrchr(sFileName, '\\');
					if (sFileName) sFileName ++;
				}
#else
				sFileName = strrchr(sFileName, '/');
				if (sFileName) sFileName ++;

#endif
			}
		}
		//格式化源代码位置
		if (sFileName)
			nlen += _snprintf(sErrBuf+nlen, ArrayCount(sErrBuf)-1-nlen, "(%s:%d)\r\n",sFileName, nLineNo);
		else nlen += _snprintf(sErrBuf+nlen, ArrayCount(sErrBuf)-1-nlen, "\r\n");
		if (nlen >= ArrayCount(sErrBuf)-1)
			break;
		nLvl++;
	}

	//没有格式化出调用栈则输出跟调用函数
	nlen += _snprintf(sErrBuf+nlen, ArrayCount(sErrBuf)-1-nlen, "root call is : %s", m_sLastCall);

	sErrBuf[nlen] = 0;
	lua_pushlstring(m_pLua, sErrBuf, nlen);
	return 1;
}

lua_State *CLogicScript::createLuaVM()
{
	return lua_newstate(ScriptMemoryManager::luaAlloc, this);
}

bool CLogicScript::openBaseLibs()
{
	if ( !Inherited::openBaseLibs() )
		return false;
	return true;
}

bool CLogicScript::registLocalLibs()
{
	CLogicServer *pLogicServer = GetLogicServer();
	//luaL_openlibs(m_pLua);
	//导出LuaHelp函数库	
	luaL_register(m_pLua, "LuaHelp", LuaHelpExpot);	
	 //这里是tolua++导出的函数,所有的tolua++导出的只有一个入口
	luaopen_server(m_pLua);	
	//导出CLVariant类
	LuaCLVariant::regist(m_pLua);
	
	if (pLogicServer)
	{		
		//导出CLangTextSection类
		LuaLangTextSection::regist(m_pLua);		
		//导出语言包实例
		LuaLangTextSection::setGlobalValue(m_pLua, pLogicServer->GetTextProvider(), "OldLang");
	}	
	return true;
}

bool CLogicScript::callInit()
{	
	gc();//先释放一遍内存	
	//导出脚本自己的实例
	lua_pushlightuserdata(m_pLua, this);
	lua_setglobal(m_pLua, ScriptInstanceKeyName);
		
	//安装函数调用性能统计
//#ifdef _DEBUG
	m_Profiler.SetState(m_pLua, &m_LRList);
//#endif
	
	bool result = false;
	if ( FunctionExists(ScriptInitFnName) )
	{
		CScriptValueList args;
		args << m_pNpcObject;
		result = Call(ScriptInitFnName, args, args, 1);
	}	
		
	return result;
}

bool CLogicScript::callFinal()
{
	if(m_pNpcObject)
	{
		/*
		if(m_pNpcObject->GetScene())
		{
			OutputMsg(rmNormal,"callFinal,scene=%s,name=%s",m_pNpcObject->GetScene()->GetSceneName(),m_pNpcObject->GetEntityName());
		}
		else
		{
			OutputMsg(rmNormal,"callFinal,name=%s",m_pNpcObject->GetEntityName());
		}
		*/


	}
	bool result;
	if ( FunctionExists(ScriptUninitFnName) )
	{
		CScriptValueList args;
		args << m_pNpcObject;
		result = Call(ScriptUninitFnName, args, args, 1);
	}
	else result = false;
	//卸载函数调用性能统计
//#ifdef _DEBUG
	static bool pathExists = FDOP::DeepCreateDirectory("./data/runtime/SPS");
	if (pathExists && m_Profiler.GetMaxTotalTime() > 16)//最大栈消耗16毫秒以上的才记录
	{
		char sPath[256];
		wchar_t sWBuf[256];
		LPCTSTR sNpc, sScene;
		if (m_pNpcObject)
		{
			sNpc = m_pNpcObject->GetEntityName();
			CScene *pScene = m_pNpcObject->GetScene();
			sScene = pScene ? pScene->GetSceneName() : "";
		}
		else sNpc = sScene = "(null)";
		int nLen = _sntprintf(sPath, ArrayCount(sPath), _T("./data/runtime/SPS/%s-%s.txt"),sScene, sNpc);
		#ifdef WIN32
		nLen = MultiByteToWideChar(CP_UTF8, 0, sPath, nLen, sWBuf, ArrayCount(sWBuf) - 1);
		#else
		nLen = mbstowcs(sWBuf, sPath, nLen);
		#endif
		sWBuf[nLen] = 0;
#ifdef UNICODE
		SaveProfiler(sWBuf);
#else
		#ifdef WIN32
		nLen = WideCharToMultiByte(CP_ACP, 0, sWBuf, nLen, sPath, ArrayCount(sPath)-1, "?", NULL);
		#else
		nLen = wcstombs(sPath, sWBuf, ArrayCount(sPath)-1);
		#endif
		sPath[nLen] = 0;
		SaveProfiler(sPath);
#endif
	}
	m_Profiler.SetState(NULL);
//#endif

	//移除向脚本导出脚本的自己的实例
	lua_pushnil(m_pLua);
	lua_setglobal(m_pLua, ScriptInstanceKeyName);

	return true;
}

void CLogicScript::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	OutputMsg(rmError, sError);
}

bool CLogicScript::LoadBuff( LPCTSTR sFilePath )
{
	wylib::stream::CMemoryStream ms;
	if (ms.loadFromFile(sFilePath) <= 0)
	{
		OutputMsg(rmError, _T("unable to load Script file %s"), sFilePath);
		return false;
	}
	CCustomLuaPreProcessor pp;
	CLogicServer *pLogicServer = GetLogicServer();
	if (pLogicServer)
	{
		pLogicServer->GetVSPDefine().RegisteToPreprocessor(pp);
	}
	LPCSTR sScript = pp.parse((LPCSTR)ms.getMemory(),sFilePath);
	int nErrCode = luaL_dostring(m_pLua, sScript);
	if (nErrCode > 0)
	{
		OutputMsg(rmTip, _T("load %s error:%s"), sFilePath, lua_tostring(m_pLua, -1));
		lua_pop(m_pLua, 1);
	}
	else
	{
		OutputMsg(rmTip, _T("load %s success"), sFilePath);
		return true;
	}
	return false;
}
