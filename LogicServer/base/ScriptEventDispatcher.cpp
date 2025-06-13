#include "StdAfx.h"
#include "ScriptEventDispatcher.h"

static int s_actor_event_handles[aeMaxEventCount] = {0};

void EventDispatcher::Init()
{
	memset(s_actor_event_handles,0,sizeof(s_actor_event_handles));
	OutputMsg(rmTip,_T("[EventDispatcher] Initialization!"));
}

bool EventDispatcher::OnActorEvent(CActor* pActor, INT_PTR nEventID, CScriptValueList & paramList, CScriptValueList & retParamList)
{
	if (nEventID > aeMaxEventCount || nEventID <= 0)
	{
		OutputMsg(rmError, "EventDispatcher error. (%d)", nEventID);
		return false;
	}
	CScriptValueList paramListTemp;
	int nIdx = 0;
	paramListTemp << nIdx;
	paramListTemp << nEventID;
	paramListTemp << pActor;
	paramListTemp += paramList;

	static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
	int nCount = s_actor_event_handles[nEventID];
	for (nIdx = 1; nIdx <= nCount; nIdx++)
	{
		*((CScriptValue*)paramListTemp) = nIdx;
		if (!gp->GetScript().CallModule("ActorEventDispatcher", "OnEvent", paramListTemp, retParamList))
		{
			//脚本错误，将以模态对话框的形式呈现给客户端
			const RefString& s = gp->GetScript().getLastErrorDesc();
			if (pActor) pActor->SendTipmsg((const char*)s, ttDialog);
			OutputMsg(rmError,"[ActorEvent] 错误 事件ID=%d, Idx=%d ! \n",nEventID,nIdx);
		}
	}
	return true;
}

bool EventDispatcher::RegistActorEvent(INT_PTR nEventID)
{
	if (nEventID > aeMaxEventCount || nEventID <= 0)
	{
		OutputMsg(rmError, "EventDispatcher error. (%d)", nEventID);
		return false;
	}
	s_actor_event_handles[nEventID]++;
	return true;
}
