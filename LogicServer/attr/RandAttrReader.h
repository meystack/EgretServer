#pragma once

/************************************************************************/
/*
/*                            随机属性配置项读取器
/*
/************************************************************************/

class CRandAttrReader :
	public CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig Inherited;
	typedef CObjectAllocator<char>	CDataAllocator;
public:
	/*
	* Comments: 读取随机属性配置表
	* Param lua_State * L: lua虚拟机对象
	* Param PRANDOMATTRTERM pRandAttrTerms: 随机属性表（读取缓冲区）
	* Param INT_PTR numTermToRead: 要读取的属性数量（缓冲区结构数量）
	* @Return bool: 返回正数表示读取成功且返回值为读取的数量（不一定等于参数numTermToRead），返回负数表示读取失败
	*/
	INT_PTR readConfig(lua_State* L,  PRANDOMATTRTERM pRandAttrTerms, CDataAllocator &dataAllocator, INT_PTR numTermToRead);

protected:
	//读取物品属性配置值
	GAMEATTRVALUE readAttrValue(const tagGameAttributeType attrType, LPCSTR sName);
protected:
	//覆盖错误输出函数
	void showError(LPCTSTR sError);

public:
	~CRandAttrReader();
};