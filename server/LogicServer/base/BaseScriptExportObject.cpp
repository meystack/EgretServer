#include "StdAfx.h"


LONG GlobalObjectMgr::sGlobalObjectMgrCount = 0;
GlobalObjectMgr* GlobalObjectMgr::sGlobalObjectMgr = NULL;

void GlobalObjectMgr::initialize()
{
	if (::InterlockedIncrement(&sGlobalObjectMgrCount) == 1)
	{
		sGlobalObjectMgr = new GlobalObjectMgr();
	}
}

void GlobalObjectMgr::uninitialize()
{
	if (::InterlockedDecrement(&sGlobalObjectMgrCount) == 0)
	{
		SafeDelete(sGlobalObjectMgr);
	}
}