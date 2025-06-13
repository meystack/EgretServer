#pragma once

/************************************************************************/
/* 
/*                            LUA脚本值对象类
/*
/*    LUA中含有nil、bool、number、string、lightuserdata、userdata、cfunction
/* 等类型的值。CScriptValue类可用于描述和存储这些值中任意类型的值。这个类通常在与脚本
/* 交互的过程中作为脚本函数调用的参数以及存储脚本返回值的时候使用。
/*
/************************************************************************/

class CScriptValue
{
public:
	/*
	* 定义脚本值的数据类型
	***/
	enum ScriptValueType
	{
		vNull = LUA_TNIL,//nil值
		vBool = LUA_TBOOLEAN,//bool值
		vLightData = LUA_TLIGHTUSERDATA,//纯指针值
		vNumber = LUA_TNUMBER,//number值
		vString = LUA_TSTRING,//string值
		vCFunction = LUA_TFUNCTION,//C函数指针值
		vUserData = LUA_TUSERDATA,//userdata值
	};
	
	/*
	* 定义脚本值的值数据结构
	***/
	union ScriptValueData
	{
		bool	boVal;		//bool值
		LPCVOID pLightPtr;	//指针值
		double  numVal;		//number值
		struct {
			LPCSTR  str; //字符串指针
			SIZE_T dwLen;//字符串长度
		}		strVal;		//字符串值
		lua_CFunction cfnVal;//C函数值
		struct {
			LPCVOID ptr;  //用户数据指针
			SIZE_T dwSize;//用户数据长度
		}		userVal;	//用户数据值
	};

public:
	CScriptValue()
	{
		m_Type = vNull;
		m_Data.userVal.ptr = 0;
		m_Data.userVal.dwSize = 0;
	}
	~CScriptValue()
	{
		clear();
	}

	/* 清除数据 */
	inline void clear()
	{
		if ( m_Type != vNull )
		{
			if (m_Type == vString)
				free((LPVOID)m_Data.strVal.str);
			else if (m_Type == vUserData)
				free((LPVOID)m_Data.userVal.ptr);
			m_Type = vNull;
			m_Data.userVal.ptr = 0;
			m_Data.userVal.dwSize = 0;
		}
	}

	/* 获取数据类型 */
	inline ScriptValueType getType()
	{
		return m_Type;
	}

	/* 获取数据大小 
	 * 对于vNull类型的值，返回值为0,
	 * 对于vBool类型的值，返回值为sizeof(bool),
	 * 对于vLightData类型的值，返回值是sizeof(LPCVOID),
	 * 对于vNumber类型的值，返回值是sizeof(double),
	 * 对于vString类型的值，返回值是字符串的长度,
	 * 对于vCFunction类型的值，返回值是sizeof(lua_CFunction),
	 * 对于vUserData类型的值，返回值是用户数据的内存字节长度,
	*/
	inline SIZE_T getSize()
	{
		switch(m_Type)
		{
		case vBool:return sizeof(m_Data.boVal);
		case vLightData:return sizeof(m_Data.pLightPtr);
		case vNumber:return sizeof(m_Data.numVal);
		case vString:return m_Data.strVal.dwLen;
		case vCFunction:return sizeof(m_Data.cfnVal);
		case vUserData:return m_Data.userVal.dwSize;
		default: return 0;
		}
	}

	/*
	 * 从流中加载
	 */
	inline VOID loadFromStream(wylib::stream::CBaseStream &stm)
	{
		clear();

		stm.read(&m_Type, sizeof(BYTE));

		switch(m_Type)
		{
		case vBool: stm.read(&m_Data.boVal, sizeof(m_Data.boVal)); break;
		case vLightData: stm.read(&m_Data.pLightPtr, sizeof(m_Data.pLightPtr)); break;
		case vNumber: stm.read(&m_Data.numVal, sizeof(m_Data.numVal)); break;
		case vString: 
			stm.read(&m_Data.strVal.dwLen, sizeof(m_Data.strVal.dwLen)); 
			m_Data.strVal.str = (LPCSTR)malloc(m_Data.strVal.dwLen + sizeof(INT_PTR));
			stm.read((LPVOID)m_Data.strVal.str, m_Data.strVal.dwLen * sizeof(m_Data.strVal.str[0]));
			((LPSTR)(m_Data.strVal.str))[m_Data.strVal.dwLen] = 0;
			break;
		case vCFunction: stm.read(&m_Data.cfnVal, sizeof(m_Data.cfnVal)); break;
		case vUserData: 
			stm.read(&m_Data.userVal.dwSize, sizeof(m_Data.userVal.dwSize));
			m_Data.userVal.ptr = malloc(m_Data.userVal.dwSize);
			stm.read((LPVOID)m_Data.userVal.ptr, m_Data.userVal.dwSize);
			break;
		}

	}

	/*
	 * 保存到流中
	 */
	inline LONGLONG saveToStream(wylib::stream::CBaseStream &stm) const
	{
		LONGLONG dwPos = stm.getPosition();

		stm.write(&m_Type, sizeof(BYTE));

		switch(m_Type)
		{
		case vBool: stm.write(&m_Data.boVal, sizeof(m_Data.boVal)); break;
		case vLightData: stm.write(&m_Data.pLightPtr, sizeof(m_Data.pLightPtr)); break;
		case vNumber: stm.write(&m_Data.numVal, sizeof(m_Data.numVal)); break;
		case vString: 
			stm.write(&m_Data.strVal.dwLen, sizeof(m_Data.strVal.dwLen)); 
			stm.write(m_Data.strVal.str, m_Data.strVal.dwLen * sizeof(m_Data.strVal.str[0]));
			break;
		case vCFunction: stm.write(&m_Data.cfnVal, sizeof(m_Data.cfnVal)); break;
		case vUserData: 
			stm.write(&m_Data.userVal.dwSize, sizeof(m_Data.userVal.dwSize));
			stm.write(m_Data.userVal.ptr, m_Data.userVal.dwSize);
			break;
		}
		return stm.getPosition() - dwPos;
	}
public:
	/* 从int值的赋值运算函数 */
	inline CScriptValue& operator = (const int val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = (double)val;
		return *this;
	}
	/* 从unsigned int值的赋值运算函数 */
	inline CScriptValue& operator = (const unsigned int val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = (double)val;
		return *this;
	}
	/* 从int64值的赋值运算函数 */
	inline CScriptValue& operator = (const INT64 val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = (double)val;
		return *this;
	}

	/* 从unsigned int64值的赋值运算函数 */
	inline CScriptValue& operator = (const UINT64 val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = (double)val;
		return *this;
	}

	/* 从double值的赋值运算函数 */
	inline CScriptValue& operator = (const double val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = val;
		return *this;
	}
	/* 从float值的赋值运算函数 */
	inline CScriptValue& operator = (const float val)
	{
		clear();
		m_Type = vNumber;
		m_Data.numVal = val;
		return *this;
	}
	/* 从bool值的赋值运算函数 */
	inline CScriptValue& operator = (const bool val)
	{
		clear();
		m_Type = vBool;
		m_Data.boVal = val;
		return *this;
	}
	/* 从const char*值的赋值运算函数 */
	inline CScriptValue& operator = (LPCSTR val)
	{
		clear();
		m_Type = vString;
		m_Data.strVal.dwLen = strlen(val);
		m_Data.strVal.str = (LPCSTR)malloc(m_Data.strVal.dwLen + sizeof(INT_PTR));
		memcpy((LPSTR)m_Data.strVal.str, val, m_Data.strVal.dwLen * sizeof(*m_Data.strVal.str));
		((LPSTR)(m_Data.strVal.str))[m_Data.strVal.dwLen] = 0;
		return *this;
	}
	/* 从char*值的赋值运算函数 */
	inline CScriptValue& operator = (LPSTR val)
	{
		return operator = ((LPCSTR)val);
	}
	/* 从任意类型的指针（LPCVOID）值的赋值运算函数 */
	inline CScriptValue& operator = (LPCVOID val)
	{
		clear();
		m_Type = vLightData;
		m_Data.pLightPtr = val;
		return *this;
	}
	/* 从LUAC函数指针（lua_CFunction）值的赋值运算函数 */
	inline CScriptValue& operator = (const lua_CFunction val)
	{
		clear();
		m_Type = vCFunction;
		m_Data.cfnVal = val;
		return *this;
	}
	/* 从ScriptValueData值的赋值运算函数 */
	inline CScriptValue& operator = (const CScriptValue& val)
	{
		switch(val.m_Type)
		{
		case vBool: return operator = (val.m_Data.boVal);
		case vLightData: return operator = (val.m_Data.pLightPtr);
		case vNumber: return operator = (val.m_Data.numVal);
		case vString: return operator = (val.m_Data.strVal.str);
		case vCFunction: return operator = (val.m_Data.cfnVal);
		case vUserData: return SetUserData(val.m_Data.userVal.ptr, val.m_Data.userVal.dwSize);
		default: 
			clear(); 
			return *this;
		}
	}
	/* 赋予指针对象值并拷贝对象内存 */
	inline CScriptValue&  SetUserData(LPCVOID data, SIZE_T dwSize)
	{
		clear();
		m_Type = vUserData;
		m_Data.userVal.dwSize = dwSize;
		m_Data.userVal.ptr = malloc(dwSize);
		memcpy((LPVOID)m_Data.userVal.ptr, data, dwSize);
		return *this;
	}

public:
	//转换为int的运算符重载
	inline operator int ()
	{
		if ( m_Type == vNumber )
			return (int)m_Data.numVal;
		else if ( m_Type == vString )
		{
			int Result = 0;
			sscanf(m_Data.strVal.str, "%d", &Result);
			return Result;
		}
		return 0;
	}
	//转换为int64的运算符重载
	inline operator INT64 ()
	{
		if ( m_Type == vNumber )
			return (INT64)m_Data.numVal;
		else if ( m_Type == vString )
		{
			INT64 Result = 0;
			sscanf(m_Data.strVal.str, "%lld", &Result);
			return Result;
		}
		return 0;
	}
	//转换为double的运算符重载
	inline operator double ()
	{
		if ( m_Type == vNumber )
			return m_Data.numVal;
		else if ( m_Type == vString )
		{
			double Result = 0;
			sscanf(m_Data.strVal.str, "%lf", &Result);
			return Result;
		}
		return 0;
	}
	//转换为float的运算符重载
	inline operator float ()
	{
		if ( m_Type == vNumber )
			return (float)m_Data.numVal;
		else if ( m_Type == vString )
		{
			float Result = 0;
			sscanf(m_Data.strVal.str, "%f", &Result);
			return Result;
		}
		return 0;
	}
	/* 转换为bool的运算符重载 */
	inline operator bool ()
	{
		switch(m_Type)
		{
		case vBool: return m_Data.boVal;
		case vLightData: return m_Data.pLightPtr != NULL;
		case vNumber: return m_Data.numVal != 0;
		case vString: return m_Data.strVal.str && _stricmp("true", m_Data.strVal.str) == 0;
		case vCFunction: return m_Data.cfnVal != NULL;
		case vUserData: return m_Data.userVal.ptr != NULL;
		default:
			return false;
		}
	}
	//转换为const char*的运算符重载
	inline operator LPCSTR ()
	{
		if ( m_Type == vString )
			return (LPSTR)m_Data.strVal.str;
		return NULL;
	}
	//转换为const lua_CFunction的运算符重载
	inline operator const lua_CFunction ()
	{
		if ( m_Type == vCFunction )
			return m_Data.cfnVal;
		return NULL;
	}
	//转换为const void*的运算符重载
	inline operator LPCVOID ()
	{
		if ( m_Type == vLightData || m_Type == vUserData )
			return (LPVOID)m_Data.pLightPtr;
		else if ( m_Type == vString )
			return (LPVOID)m_Data.strVal.str;
		return NULL;
	}
protected:
	ScriptValueType	m_Type;	//值类型
	ScriptValueData	m_Data;	//值
};