#pragma once

class CEntity;

class CLogicScript :
	public CCustomLuaScript
{
public:
	typedef CCustomLuaScript Inherited;
	typedef wylib::misc::CRefObject<CCustomLuaPreProcessor::CLineRangeList> CSourceLineList;
	//脚本初始化函数名称
	static LPCSTR ScriptInitFnName;
	//脚本析构函数名称
	static LPCSTR ScriptUninitFnName;
	//向脚本中保存此脚本对象指针的名称
	static LPCTSTR ScriptInstanceKeyName;
	/* 脚本中申请的ActorPacket指针，保存指针以便在角色申请数据包后出现脚本错误从而无法flush数据包的时候回收数据包，
	   如果脚本出错且数据包没有被回收，则会导致整个游戏引擎就此不能正常工作
	*/
	static struct CActorPacket *ScriptActorPacket;

public:
	CLogicScript();
	~CLogicScript();

	/*
	* Comments: 覆盖父类设置脚本内容的函数，增加对代码原始行记录器的保存
	* Param LPCSTR sText:
	* Param const CSourceLineList * pLRList:
	* @Return bool:
	*/
	bool setScript(LPCSTR sText, CSourceLineList * pLRList);

	//判断脚本中是否存在名为sFnName的函数
	bool FunctionExists(LPCSTR sFnName);
	/* 调用脚本函数，如果脚本调用成功，则函数返回true，否则返回false。
	 * 当调用脚本失败时，函数返回false并在返回值列表中存储1个错误描述内容的字符串值
	 * args				脚本参数列表
	 * results			脚本返回值列表
	 * nResultCount		期望的返回值数量，LUA_MULTRET表示不限制
	 */
	bool Call(LPCSTR sFnName, CScriptValueList &args, CScriptValueList &results, INT_PTR nResultCount = LUA_MULTRET);

	bool CallModule(LPCSTR sModuleName,LPCSTR sFnName, CScriptValueList &args, CScriptValueList &results, INT_PTR nResultCount = LUA_MULTRET);

	/*
	* Comments: 从文件加载脚本
	* Param LPCTSTR sFilePath:
	* @Return bool:
	*/
	bool LoadScript(LPCTSTR sFilePath);

//#ifdef _DEBUG
	/*
	* Comments: 保存脚本函数调用性能统计数据到文件
	* Param LPCTSTR sFilePath:
	* @Return void:
	*/
	void SaveProfiler(LPCTSTR sFilePath);

	/* 
	* Comments:执行一条或数条函数调用，声明等
	* Param LPCTSTR sFilePath:
	* @Return bool:
	*/
	bool LoadBuff(LPCTSTR sFilePath);

//#endif

	VOID SetEntity(CEntity* pEntity){m_pNpcObject = pEntity;}

	//检查脚本错误
	bool CheckScript(LPCTSTR sFilePath, char* sErr= NULL);

	lua_State* GetLuaState()
	{
		return m_pLua;
	}

protected:
	//脚本调用错误处理分派函数
	static int ScriptCallErrorDispatcher(lua_State *L);
protected:
	/* 以下函数集为本类中提供的虚函数集，如有必要，子类可以覆盖这些函数 */
	//调用脚本函数错误的处理函数
	virtual void ScriptCallError(LPCSTR sFnName, CScriptValueList &results);
	//脚本调用错误处理函数，函数应当返回处理错误后向lua中返回了多少个值
	virtual int HandleCallError();
protected:
	/* 下列函数集为覆盖父类的相关处理函数 */
	//创建脚本虚拟机
	virtual lua_State* createLuaVM();
	//打开基础库。默认会打开base、string、math以及table库。返回true表示成功。
	virtual bool openBaseLibs();
	//注册本地函数库。返回true表示成功。
	virtual bool registLocalLibs();
	//调用脚本的初始化函数。函数返回true表示没有错误发生。本类未提供调用初始化函数的实际功能。
	virtual bool callInit();
	//调用脚本的卸载函数。函数返回true表示没有错误发生。本类未提供调用卸载函数的实际功能。
	virtual bool callFinal();
	//显示脚本错误内容
	virtual void showError(LPCTSTR sError);

protected:
	CEntity*			m_pNpcObject;	//脚本对象所属的角色对象，例如NPC对象
	LPCSTR				m_sLastCall;	//当前调用的脚本函数名称，再函数调用完成后指针不再有效
	CSourceLineList		m_LRList;		//原始代码行记录表，用于获取更精确的函数代码位置
//#ifdef _DEBUG
	CCustomLuaProfiler	m_Profiler;		//脚本函数调用性能统计

//#endif
};
