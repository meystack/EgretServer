#pragma once

#include "../base/CustomLogicLuaConfig.h"
#define LuaTableLen() lua_objlen(m_pLua, -1)

template<typename T>
struct LuaArray
{
	unsigned int count;
	T* pDatas;
	LuaArray()
	{
		memset(this, 0, sizeof(*this));
	}
	~LuaArray(){
		if(pDatas){
			for (unsigned int i = 0; i < count; ++i)
			{
				pDatas[i].~T();
			}
			free(pDatas);
			pDatas = nullptr;
		}
	}
};

/// Lua 整数数组 例如：{1,2,3,4,5}
typedef LuaArray<int> IntArray;

/// 属性数组
typedef LuaArray<GAMEATTR> Attrs;

/// 把一些常用通用的方法提取到这个类
class CLuaConfigBase :
	public CCustomLogicLuaConfig
{
public:
	CLuaConfigBase(void);
	virtual ~CLuaConfigBase(void);

	bool Load(LPCTSTR sFilePath);

protected:
	/// 读取整个配置文件
	virtual bool ReadAllConfig() = 0;

	/// 读取整数数组，例如：tb = {1,2,3,4,5}
	bool ReadIntArray(LPCSTR lpTableName, IntArray& intArray);

	/// 读取属性表 {{type = 1, value = 1}, {type = 2, value = 2}, {type = 3, value = 3}, }
	// 假如 reserve0Name= "job",则对应配置{type = 1, value = 1, job =1},
	bool LoadAttri(PGAMEATTR pAttr, LPCSTR reserve0Name = NULL);
};
