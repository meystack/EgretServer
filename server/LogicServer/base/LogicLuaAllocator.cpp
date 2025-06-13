#include "StdAfx.h"
#include "LogicLuaAllocator.h"


CBufferAllocator* CLogicLuaAllocator::s_pBufferAllocator = NULL;	//统一的脚本内存管理器
LONG CLogicLuaAllocator::s_nAllocatorInitCount = 0;//内存管理器初始化计数

//初始化静态内存管理器
VOID CLogicLuaAllocator::InitializeAllocator()
{
	if (1 == InterlockedIncrement(&s_nAllocatorInitCount))
	{
		s_pBufferAllocator = new CBufferAllocator();
	}
}

VOID CLogicLuaAllocator::CheckMemory()
{
	//SF_TIME_CHECK(); //检测性能	
	s_pBufferAllocator->CheckFreeBuffers();
}

//析构静态内存管理器
VOID CLogicLuaAllocator::UninitializeAllocator()
{
	if (0 == InterlockedDecrement(&s_nAllocatorInitCount))
	{
		delete s_pBufferAllocator;
		s_pBufferAllocator = NULL;
	}
}

lua_State* CLogicLuaAllocator::createLuaVM()
{
	return s_pBufferAllocator 
		? lua_newstate((lua_Alloc)alloc, s_pBufferAllocator)
		: luaL_newstate();
}

LPVOID CLogicLuaAllocator::alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	CBufferAllocator *pAllocator = (CBufferAllocator*)ud;
#ifdef __ALLOC_NOT_USE_MEMORY_POOL___
	return pAllocator->ReallocBuffer(ptr,nsize,osize);
#else 
	return pAllocator->ReallocBuffer(ptr,nsize);	
#endif 
}

bool CCustomLogicLuaConfig::LoadFile(LPCTSTR sFilePath)
{
	using namespace wylib::stream;
	bool Result = false;		
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			OutputMsg(rmError,_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理

#ifdef __ROBOT_CLIENT__ 
		GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);
#else
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
#endif

		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
		{
			OutputMsg(rmError,_T("syntax error on  %s"),sFilePath);
			return false;
		}
		//读取标准物品配置数据
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load %s"),sFilePath);
		return false;
	}
	return true;
}
bool CCustomLogicLuaConfig::registLocalLibs()
{
	CLogicServer *pLogicServer = GetLogicServer();

	if (pLogicServer)
	{
		//导出CLangTextSection类
		LuaLangTextSection::regist(m_pLua);

		//导出语言包实例
		LuaLangTextSection::setGlobalValue(m_pLua, pLogicServer->GetTextProvider(), "OldLang");
	}

	return true;
}
