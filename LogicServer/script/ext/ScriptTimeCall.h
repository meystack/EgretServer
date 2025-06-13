#pragma once

class CScriptTimeCallManager : 
	protected CTimeCaller,
	public CComponent
{
protected:
	typedef CTimeCaller Inherited;

public:
	CScriptTimeCallManager();
	~CScriptTimeCallManager();

public:
	/*
	* Comments: 注册一个脚本定时调用函数
	* Param CNpc * pNPC: NPC对象
	* Param LPCSTR sFnName: 函数名称
	* Param LPCSTR sNextCall: 下次调用日期和时间，标准日期表示法（2011-03-04 0:0:0）
	* Param INT nSecInterval: 以秒为周期的调用周期
	* Param bool boCallOnNextBoot: 在下次启动的时候是否恢复调用
	* Param const CScriptValueList & args: 脚本函数调用参数列表
	* @Return HANDLE: 如果注册调用函数成功，则返回定时调用句柄，否则返回NULL。
	* ★注意★	
	*
	*  1、同一个函数是不允许被多次注册为自动调用函数的；
	*
	*  2、sNextCall表示的日期和时间为一个已经过去的时间，则会自动根据函数调用周期值机选下次执
	*  行时间为现在之后的最近的一次时间。例如：一1天为周期注册函数在2010-01-01 12:30执行，而
	*  当前时间是2011-03-04 17:53，那么次函数的下次执行时间为2011-03-05 12:30；
	*
	*  3、参数boCallOnNextBoot决定是否在下次启动中的时候恢复调用此函数。提供这个参数
	*  的意义在于解决某些回调函数需要执行的时间可能恰巧在服务器维护期间，由于程序关闭而
	*  导致函数没有被调用，继而使得部分重要数据没有处理的问题。
	*     例如，注册了一个即将在0:30执行的函数，维护发生在0：0到1:0则程序启动时会检查
	*  调用到此函数在维护期间需要执行，因此会立刻调用一次此函数。如果维护在0:0到0:20则
	*  由于函数没有被漏执行，则不会调用。
	*
	*  4、函数调用参数最多支持CScriptValueList::MaxValueCount(16)个，请不要在参数表
	*  中传递指针型参数，除非这个函数不会再下次启动时被调用(boCallOnNextBoot = false)。
	*
	*/
	HANDLE RegisterTimeCall(CNpc *pNPC, LPCSTR sFnName, LPCSTR sNextCall, UINT dwSecInterval, 
		bool boCallOnNextBoot, const CScriptValueList &args);
	/*
	* Comments: 删除一个被注册为定时调用的函数
	* Param HANDLE hTimeCall: 在RegisterTimeCall中返回的调用函数句柄
	* @Return bool: 如果删除成功，则函数返回true否则返回false。
	*/
	bool UnregisterTimeCall(HANDLE hTimeCall);
	/*
	* Comments: 通过脚本对象以及函数名称，查找被注册的回调函数对象句柄
	* Param CNpc * pNPC: NPC对象，如果为NULL则表示匹配任何脚本中注册的此名称的函数
	* Param LPCSTR sFnName: 函数名称
	* @Return HANDLE:
	*/
	HANDLE GetTimeCall(CNpc *pNPC, LPCSTR sFnName);
	/*
	* Comments: 检查并调用已经到达运行时间的回调对象
	* Param ULONGLONG lCurTick: 当前TickCount
	* Param ULONGLONG lRunTickLimit: 对Run函数运行的时间限制，单位是毫秒，0表示不限制
	* @Return INT_PTR: 返回检查了多少个回调对象
	*/
	INT_PTR Run(ULONGLONG lRunTickLimit = 0);
	
	/*
	* Comments: 调用在程序停止期间需要调用的函数(BootCall)
	* @Return INT_PTR: 函数返回补充调用了多少个函数
	* ★注意★	
	*    此函数调用后，BootCall列表将被清空以防止重复调用
	*/
	INT_PTR RunBootCalls();
	/*
	* Comments: 加载存储的BootCall数据
	* Param LPCTSTR sFilePath: 文件路径
	* @Return INT_PTR: 函数返回读取了多少个BootCall数据
	*/
	INT_PTR LoadBootCalls(LPCTSTR sFilePath);
	/*
	* Comments: 保存BootCall数据到文件
	* Param LPCTSTR sFilePath: 文件路径
	* @Return INT_PTR: 函数返回正数表示向文件中报错了多少个BootCall数据，返回负数表示发生错误。
	*/
	INT_PTR SaveBootCalls(LPCTSTR sFilePath);

protected:
	/*
	* Comments: 覆盖父类返回当前时间的值，此处将返回逻辑线程中的同步的MiniDateTime从而使得可以通过修改系统时间来在现实时间中提前运行一个函数。
	* @Return ULONGLONG:
	*/
	ULONGLONG GetCurrentTick();

protected:
	/* 定义脚本定时调用数据结构 */
	class ScriptCallInfo : public Counter<ScriptCallInfo>
	{
	public:
		CNpc*				pNPC;			//NPC对象
		HANDLE				hCall;			//定时调用句柄
		CHAR				sFn[64];		//脚本名称
		CHAR				sNPCName[64];	//NPC名称（保存用于输出未取消的函数，以及在下次启动的时候恢复执行函数）
		CHAR				sSceneName[64];	//NPC所在场景名称
		CScriptValueList	args;			//脚本调用参数表
		BOOL				boBootCall;		//是否在下次启动时恢复调用
		BOOL				boMarkedRemove;	//是否标记为删除
	public:
		~ScriptCallInfo(){};
	};

	/* 定义脚本定时调用数据存储结构 */
	class ScriptBootCallData : public ScriptCallInfo
	{
	public:
		CMiniDateTime	nNextCall;		//下次调用时间
		DWORD			dwSecInterval;	//调用周期，单位为秒
	public:
		bool loadFromStream(wylib::stream::CBaseStream &stm);
		void saveToStream(wylib::stream::CBaseStream &stm);
	};

	/* 定义BootCall文件头结构 */
	struct ScriptBootCallFileHeader
	{
		static const FileHeaders::FILEIDENT		FileIdent;	//声明BootCall文件头标志
		static const FileHeaders::FILEVERSION	FileVersion;//声明BootCall文件版本号

		FileHeaders::FILEIDENT	 ident;		//文件标识，固定为BootCallFileHeader::FileIdent
		FileHeaders::FILEVERSION version;	//文件版本，固定为BootCallFileHeader::FileVersion
		DWORD	dwDataCRC32;	//BootCall数据段CRC32效验值
		UINT	dwNumCalls;		//BootCall数据记录数量
		DWORD	dwSizeData;		//sizeof(ScriptBootCallData)的大小
		DWORD	dwDataSize;		//数据段长度
		CHAR	btReserve[40];	//保留字节，促使文件头为64字节
	};
protected:
	//脚本定时调用集中分派函数
	static VOID STDCALL ScriptTimeCallDispatcher(CScriptTimeCallManager *lpManager, CList<ScriptCallInfo>::NodeType *pCallInfoNode, INT_PTR nCount);
	/*
	* Comments: 移除所有回调函数对象或移除所有被标记为移除的函数
	* Param BOOL boJustRemoveMarked: 是否仅移除被标记为移除的函数
	* @Return VOID:
	*/
	VOID RemoveAll(BOOL boJustRemoveMarked);
	/*
	* Comments: 将一个脚本定时回调对象保存到BootCall列表中
	* Param const ScriptCallInfo & sc: 脚本定时回调对象
	* @Return VOID:
	*/
	VOID AddToBootCall(const ScriptCallInfo& callInfo);
	/*
	* Comments: 清空BootCall列表
	* @Return VOID:
	*/
	VOID ClearBootCallList();
protected:
	CList<ScriptCallInfo>		m_CallList;	//脚本调用数据列表
	CScriptValueList			m_SRetList;	//脚本调用返回值存储列表(用于优化性能)
	CList<ScriptBootCallData>	m_BootCallList;//下次启动时需要恢复执行的回调记录列表
	INT_PTR						m_nMarkedRemoveCount;//标记为移除的定时回调数量
};