#include "StdAfx.h"
#include "AsyncWorker.h"
#include "BufferAllocator.h"
 
AsyncWorker::AsyncWorker(const char* lua_file)
: Inherited()
, m_WorkCallBack(NULL)
, m_isReload(false)
, m_Id(0)
{
	m_CallBackDataList.setLock(&m_Lock);

	LoadScript(lua_file);

	_asncpyt(m_sLuaFilePath, lua_file);
}

AsyncWorker::~AsyncWorker()
{
	m_CallBackDataList.flush();
	int count = m_CallBackDataList.count();

	for (int i = 0; i < count; ++i)
	{
		CBData* cb = m_CallBackDataList[i];
		if (!cb) continue;

		if (cb->params_ && cb->free_func_)
		{
			cb->free_func_(cb);
		}

		m_Alloc.FreeBuffer(cb);
	}

	m_CallBackDataList.clear();
}

int AsyncWorker::Run()
{
	if (m_isReload)
	{
		m_isReload = false;
		LoadScript(m_sLuaFilePath);
	}

	if (m_CallBackDataList.appendCount() <= 0) return 0;

	m_CallBackDataList.flush();
	int count = m_CallBackDataList.count();

	for (int i = 0; i < count; ++i)
	{
		CBData* cb = m_CallBackDataList[i];
		cb->work_func_(cb);

		this->m_WorkCallBack(cb);
	}

	m_CallBackDataList.clear();
	return count;
}

void AsyncWorker::OnRountine()
{
	OnStart();

	while (!terminated())
	{
		if (Run() <= 0)
			Sleep(1);
	}

	OnStop();
}

void AsyncWorker::OnStart()
{
	OutputMsg(rmError, "AsyncWorker start!");
}

void AsyncWorker::OnStop()
{
	OutputMsg(rmError, "AsyncWorker stop!");
}

void AsyncWorker::AppendWork(CBData* cb)
{
	m_CallBackDataList.append(cb);
}

void AsyncWorker::LoadScript(const char* lua_file)
{
	if (lua_file)
	{
		if (!m_Script.LoadScript(lua_file))
		{
			OutputMsg(rmError, "load AsyncWorker script fail!!!file:%s", lua_file);
		}
	}
}

unsigned int AsyncWorker::SendLuaMessage(CScriptValueList& req, WordFunc func)
{
	m_Id++;

	if (m_Id == 0) m_Id++;

	CBData* cb = (CBData*)m_Alloc.AllocBuffer(sizeof(*cb));
	new(cb)CBData(this, m_Id);

	cb->worker_ = this;
	cb->work_func_ = &AsyncWorker::ExecuteLua;
	cb->after_func_ = func;
	cb->free_func_ = &AsyncWorker::FreeLuaParams;
	cb->params_ = m_Alloc.AllocBuffer(sizeof(LuaParams));
	new(cb->params_)LuaParams();

	LuaParams* p = (LuaParams*)cb->params_;
	p->req_ = req;
	p->result_ << cb->guid_;
	AppendWork(cb);
	return cb->guid_;
}

void AsyncWorker::FreeLuaParams(void* callback)
{
	CBData* cb = (CBData*)callback;

	if (cb->params_)
	{
		((LuaParams*)cb->params_)->~LuaParams();
		cb->worker_->GetAlloc().FreeBuffer(cb->params_);
		cb->params_ = NULL;
	}
}

void AsyncWorker::ExecuteLua(void* cb)
{
	CBData* c = (CBData*)cb;
	LuaParams* p = (LuaParams*)c->params_;
	c->worker_->m_Script.Call("main", p->req_, p->result_);
}
