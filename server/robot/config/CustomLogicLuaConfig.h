#pragma once

/************************************************************************/
/* 
/*              可提供默认的lua虚拟机内存管理函数的脚本配置类
/* 
/************************************************************************/
class CCustomLogicLuaConfig :
	public CCustomLuaConfig
{
public:
	typedef CCustomLuaConfig Inherited;

protected:
	//创建虚拟机对象
	inline lua_State* createLuaVM()
	{
		return CLogicLuaAllocator::createLuaVM();
	}
	//注册本地库
	bool registLocalLibs();

public:
	static CBufferAllocator* s_pBufferAllocator;
	static LONG s_nAllocatorInitCount;
};
