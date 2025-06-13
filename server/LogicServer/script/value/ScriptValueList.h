#pragma once

/************************************************************************/
/*                           LUA脚本值列表
/*
/*   脚本值列表中最多允许保存MaxValueCount个值。此类适用于在与脚本交互中的函数调用参数
/* 表的传递以及返回值列表的获取操作的场合。
/*
/************************************************************************/
class CScriptValueList
{
public:
	static const INT_PTR MaxValueCount = 32;	//脚本值列表允许存储的值数量上限

public:
	CScriptValueList()
	{
		m_nCount = 0;
	}
	CScriptValueList(const CScriptValueList& valList)
	{
		//m_nCount = 0;
		operator = (valList);
	}
	//获取列表中的值数量
	inline INT_PTR count()
	{
		return m_nCount;
	}
	//将列表转换为ScriptValue数组的运算符重载
	inline operator CScriptValue* ()
	{
		return m_Values;
	}
	inline CScriptValue& operator[](int i)
	{
		return m_Values[i];
	}
	//添加一个值
	template <typename T>
	inline CScriptValueList& operator << (const T val)
	{
		if ( m_nCount < MaxValueCount )
		{
			m_Values[m_nCount++] = val;
		}
		return *this;
	}
	//从另一个列表中赋值
	inline CScriptValueList& operator = (const CScriptValueList &valList)
	{
		clear();
		for (UINT_PTR i=valList.m_nCount; i; --i)
		{
			m_Values[i-1] = valList.m_Values[i-1];
		}
		m_nCount = valList.m_nCount;
		return *this;
	}

	inline CScriptValueList& operator += (const CScriptValueList &valList)
	{
		for (UINT_PTR i = 0; i < valList.m_nCount && m_nCount < MaxValueCount; i++)
		{
			m_Values[m_nCount++] = valList.m_Values[i];
		}

		return *this;
	}

	//清空值列表
	inline void clear()
	{
		for (UINT_PTR i=m_nCount; i; --i)
		{
			m_Values[i-1].clear();
		}
		m_nCount = 0;
	}
	/* 将值列表中的值压入lua脚本中。
	 * 将列表中的值压入脚本中后，列表不会被清空
	 */
	void pushArgs(lua_State *pLua, INT_PTR nCount);
	/* 从lua脚本中取出返回值到值列表中，函数返回取出了多少个值。
	 * 如果列表中已经存在值，则之前的值将不会清空。
	 */
	INT_PTR getResults(lua_State *pLua, int nCount);
	/* 将lua中传入的参数保存到值列表中。
	 * 列表会被清空，从而存放新的值列表
	 * 函数返回读取了多少个参数
	 */
	INT_PTR getArguments(lua_State *pLua, INT_PTR nStackIdx);

	/* 将值列表保存到流中 */
	inline void loadFromStream(wylib::stream::CBaseStream &stm)
	{
		clear();
		stm.read(&m_nCount, sizeof(m_nCount));
		for (UINT_PTR i=0; i<m_nCount; ++i)
		{
			m_Values[i].loadFromStream(stm);
		}
	}

	/* 从流中读取值列表 */
	inline LONGLONG saveToStream(wylib::stream::CBaseStream &stm) const
	{
		LONGLONG dwPos = stm.getPosition();
		stm.write(&m_nCount, sizeof(m_nCount));
		for (UINT_PTR i=0; i<m_nCount; ++i)
		{
			m_Values[i].saveToStream(stm);
		}
		return stm.getPosition() - dwPos;
	}
protected:
	CScriptValue	m_Values[MaxValueCount];//值内存块
	UINT_PTR			m_nCount;//值数量
};