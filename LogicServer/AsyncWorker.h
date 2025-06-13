#ifndef _ASYNC_WORKER_H_
#define _ASYNC_WORKER_H_

/*
* 单独运行的线程，用于非游戏主线程执行的操作
* 比如访问每个web接口。
* 增加新的功能，只需要定义线程的执行函数，以及执行完成后的回调函数
*/
#include "_osdef.h"
#include "Thread.h"

using namespace wylib::time::tick64;
using namespace wylib::container;
using namespace wylib::sync::lock;
using namespace wylib::thread;

class AsyncWorker: public wylib::thread::CBaseThread
{
public:
	typedef CBufferAllocator AsyncWorkerAlloc;

	typedef wylib::thread::CBaseThread Inherited;
	typedef void (*WordFunc)(void* cb);
	typedef void (*WorkerCallBack)(void* cb);
	typedef void (*FreeFunc)(void* cb);

	struct LuaParams
	{
		CScriptValueList req_;	// 请求的参数列表
		CScriptValueList result_;	// 执行完后返回的值
	};

	struct CBData
	{
		unsigned int guid_;		// 序列号，每个执行的函数都会分配一个唯一的id，用于回调
		WordFunc work_func_;	// 在本线程里执行的工作函数，比如访问web
		WordFunc after_func_;	// 在游戏主线程要执行的函数指针,即回调函数
		FreeFunc free_func_;
		void* params_;
		AsyncWorker* worker_;
		CBData(AsyncWorker* worker,	unsigned int guid)
        : guid_(guid)
        , work_func_(NULL), after_func_(NULL), free_func_(NULL)
        , params_(NULL), worker_(worker)
		{ }
	};
    
public:

	AsyncWorker(const char* lua_file = NULL);
	virtual ~AsyncWorker();

public:

	void LoadScript(const char* lua_file);
	void Reload() { m_isReload = true; }

	AsyncWorkerAlloc& GetAlloc() { return m_Alloc; }
	void SetWorkCallBack(WorkerCallBack wcb) { m_WorkCallBack = wcb; }

	// 发送执行lua的请求
	unsigned int SendLuaMessage(CScriptValueList& req, WordFunc func);

protected:

	void OnRountine();
	//单次逻辑处理
	int Run();
	void OnStart();
	void OnStop();
	// 新增一个线程执行的操作
	void AppendWork(CBData* cb);

private:
	static void FreeLuaParams(void* callback);
	static void ExecuteLua(void* cb);

private:
	CQueueList<CBData*> m_CallBackDataList;
	CCSLock				m_Lock;
	AsyncWorkerAlloc    m_Alloc;
	unsigned int	    m_Id;
	WorkerCallBack	    m_WorkCallBack;
	CLogicScript	    m_Script;	// 用于执行lua的代码
	bool			    m_isReload;	// 通知重新加载脚本
	char			    m_sLuaFilePath[128];	//文件名
};

#endif



