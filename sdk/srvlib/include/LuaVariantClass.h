#pragma once

/************************************************************************
 
              基于CustomLuaClass类实现方式的CLVaraint导出类
 
       已测试全部功能可以使用，但此代码仅用作对CustomLuaClass的实现演示。
         CustomLuaClass在设计思想上不够完善，随可以使用，但仍有不足。
 
***********************************************************************/
class CLuaVariantClass :
	public CCustomLuaClass<CCLVariant, CLuaVariantClass>
{
public:
	typedef CCustomLuaClass<CCLVariant, CLuaVariantClass> Inherited;

protected:
	CCLVariant* create(lua_State *L);
	void destroy(lua_State *L, CCLVariant *ptr);
	int get(lua_State *L, CCLVariant *pInst);
	int set(lua_State *L, CCLVariant *pInst);
	int eq(lua_State *L, CCLVariant *pInst);
	int add(lua_State *L, CCLVariant *pInst);
	int sub(lua_State *L, CCLVariant *pInst);
	int mul(lua_State *L, CCLVariant *pInst);
	int div(lua_State *L, CCLVariant *pInst);
	int mod(lua_State *L, CCLVariant *pInst);
	int unm(lua_State *L, CCLVariant *pInst);
	int len(lua_State *L, CCLVariant *pInst);
	int lt(lua_State *L, CCLVariant *pInst);
	int le(lua_State *L, CCLVariant *pInst);
	int concat(lua_State *L, CCLVariant *pInst);
	int testCall(lua_State *L, CCLVariant *pInst);
private:
	void setVarFromTable(lua_State *L, CCLVariant *pVar, int vidx = -1);
	void setVarMember(lua_State *L, CCLVariant *pVar, const char* sName, int vidx = -1);

private:
	static const FuncCall fnList[];
public:
	CLuaVariantClass();
};
