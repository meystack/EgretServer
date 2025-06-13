#pragma once

/************************************************************************/
/*                                                                      */
/*                           LUA脚本辅助处理函数库                         */
/*                                                                      */
/************************************************************************/
namespace LuaHelp
{
	/*
	* Comments: 将二进制数据指针保存到一个表中并将表压入栈顶，如果dataCount为0则会向栈顶压入nil值
	* Param lua_State * L: lua虚拟机对象
	* Param LPCVOID lpData: 数据缓冲区指针
	* Param SIZE_T sizeElement: 数据对象大小
	* Param SIZE_T dataCount: 缓冲区中数据的数量
	* @Return VOID:
	*/
	VOID PushDataPointerToTable(lua_State *L, LPCVOID lpData, SIZE_T sizeElement, SIZE_T elementCount);

	/*
	* Comments:将二进制数据指针保存到一个表中并将表压入栈顶，如果dataCount为0则会向栈顶压入nil值
	* Param lua_State * L:lua虚拟机对象
	* Param void * * lpData:数据缓冲区指针
	* Param SIZE_T elementCount:缓冲区中数据的数量
	* @Return VOID:
	*/
	VOID PushDataPointerToTable(lua_State *L, void** lpData, SIZE_T elementCount);
	/*
	* Comments: 将字符串列表保存到一个表中并将表压入栈顶，如果dataCount为0则会向栈顶压入nil值
	* Param lua_State * L: lua虚拟机对象
	* Param LPCVOID lpData: 数据缓冲区指针
	* Param SIZE_T sizeElement: 数据对象大小
	* Param SIZE_T dataCount: 缓冲区中数据的数量
	* @Return VOID:
	*/
	VOID PushStringsToTable(lua_State *L, LPCSTR *ppStrList, SIZE_T strCount);
	/*
	* Comments: 将一个LUA表对象序列化为ASCII脚本字符串
	* Param lua_State * L: lua虚拟机对象
	* Param INT_PTR stackId: 表在虚拟机中的栈索引
	* Param wylib::stream::CBaseStream & stm: 输出流对象
	* @Return SIZE_T: 返回表序列化后产生了多少个字节的数据
	*/
	SIZE_T FormatTableA(lua_State *L, INT_PTR stackId, wylib::stream::CBaseStream &stm);

	/*
	* Comments: 往一个Lua堆栈里写入一个数值列表，比如一个CVector<float>或者int a[10]
	* Param lua_State * L: 堆栈
	* Param T * pData: 数据的指针
	* Param INT_PTR elementCount: 数值的个数
	* @Return VOID:
	*/
	template<class T>
	VOID PushNumberVector(lua_State *L,T * pData,SIZE_T elementCount)
	{
		if(pData ==NULL) return;
		if (elementCount > 0)

		{
			lua_createtable(L, (int)elementCount, 0);
			for (SIZE_T i=1; i<=elementCount; ++i)
			{
				lua_pushnumber( L, (LUA_NUMBER)(* pData)  );
				lua_rawseti( L, -2, (int)i );
				pData ++;
			}
		}
		else lua_pushnil(L);
	}

};
