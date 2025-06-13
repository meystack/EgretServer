#pragma once

/************************************************************

   CBP系统（Common Binary Property）通用二进制属性描述系统
		   从LUA配置文件中产生通用二进制属性数据文件

	 通用二进制属性描述系统用于在多种开发语言中以及服务器、客户端
 之间提供基于一份数据而产生的二进制的通用配置描述数据。避免为不
 同系统以及功能之间的数据配置文件独立设计不同的数据提供文件的工
 作。将所有的配置数据都发布为通用格式的属性描述文件，将使得所有
 系统以及功能之间的数据配置提供统一、规范化。

************************************************************/

using namespace wylib::container;

class CLuaToBP
	: public CCustomLuaConfig
{
public:
	typedef CCustomLuaConfig Inherited;
	//定义创建lua虚拟机函数类型
	typedef lua_State* (*LPCREATELUAFN) ();

public:
	/* 将配置文件生成为通用二进制属性描述数据并保存到数据流中
	 *sTableName 参数表示要发布的配置表的名称
	*/
	size_t saveToStream(LPCSTR sTableName, wylib::stream::CBaseStream& stream);
	/* 将配置文件生成为通用二进制属性描述数据并保存到文件中
	 *sTableName 参数表示要发布的配置表的名称
	*/
	size_t saveToFile(LPCSTR sTableName, LPCTSTR sFileName);
	/* 添加过滤路径，路径以'.'作为分隔符，支持通配符
	 *sPath 过滤路径
	*/
	void addFiltePath(LPCSTR sPath);
	/* 添加过滤路径，路径以'.'作为分隔符，支持通配符
	 *filterList 过滤路径列表
	*/
	void addFiltePathList(const CBaseList<LPCSTR> &filterList);
	/* 清空过滤路径表 */
	void clearFiltePath();
	/* 添加匹配器
	 *sMatching 匹配路径，路径以'.'作为分隔符，支持通配符
	*/
	void addMatcher(LPCSTR sMatching);
	/* 添加匹配路径列表，路径以'.'作为分隔符，支持通配符
	 *matcherList 匹配器列表
	*/
	void addMatcherList(const CBaseList<LPCSTR> &matcherList);
	/* 情况匹配器列表 */
	void clearMatchers();

public:
	CLuaToBP();
	//获取创建lua虚拟机函数
	LPCREATELUAFN getCreateLuaFn();
	//设置创建lua虚拟机函数
	void setCreateLuaFn(LPCREATELUAFN lpFn);

protected:
	void showError(LPCTSTR sError);
	lua_State *createLuaVM();

	bool isFiltePath(LPCSTR sPath);
	size_t writeBoolean(wylib::stream::CBaseStream& stream, LPCSTR sKeyName);
	size_t writeNumber(wylib::stream::CBaseStream& stream, LPCSTR sKeyName);
	size_t writeString(wylib::stream::CBaseStream& stream, LPCSTR sKeyName);
	size_t writeTable(wylib::stream::CBaseStream& stream, LPCSTR sKeyName);

private:
	LPCREATELUAFN		m_pCreateLuaFn;		//创建lua虚拟机函数，提供用于支持外部管理脚本内存
	CHAR				m_sNamePath[4096];	//当前表的名称路径，使用'.'做路径分隔符
	LPSTR				m_sNamePtr;			//当前表名称路径终止位置
	CBaseList<LPCSTR>	m_Filters;			//过滤器列表
	CBaseList<LPCSTR>	m_Matchers;			//匹配器列表
};
