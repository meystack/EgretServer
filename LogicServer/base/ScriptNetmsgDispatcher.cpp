#include "StdAfx.h"
#include "ScriptNetmsgDispatcher.h"

static NetMsgHandle s_net_msg_handles[256][256] = { 0 };

void ScriptNetMsgHandle(CActor* pActor, INT_PTR nSystemID, INT_PTR nCmd, CDataPacketReader& pack)
{
	static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
	CScriptValueList paramList, retList;
	paramList << nSystemID;
	paramList << nCmd;
	paramList << pActor;
	paramList << &pack;

	bool ret = gp->GetScript().Call("OnNetMsg", paramList, retList);
	if (!ret)
	{
		//脚本错误，将以模态对话框的形式呈现给客户端
		const RefString& s = gp->GetScript().getLastErrorDesc();
		if (pActor) pActor->SendTipmsg((const char*)s, ttDialog);
	}
	return;
}

void NetMsgDispatcher::Init()
{
	memset(s_net_msg_handles,0,sizeof(s_net_msg_handles));
	OutputMsg(rmTip,_T("[NetmsgDispatcher] Initialization!"));
}

bool NetMsgDispatcher::ProcessNetMsg(CActor* pActor, INT_PTR nSystemID, INT_PTR nCmd, CDataPacket& pack)
{
	if (nSystemID > 255 || nCmd > 255) return false;
	if (s_net_msg_handles[nSystemID][nCmd])
	{
		CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength());
		s_net_msg_handles[nSystemID][nCmd](pActor, nSystemID, nCmd, dp);
		return true;
	}

	return false;
}

bool NetMsgDispatcher::RegistNetMsgHandle(INT_PTR nSystemID, INT_PTR nCmd, NetMsgHandle func)
{
	if (nSystemID < 0 || nSystemID > 255 || nCmd < 0 || nCmd > 255)
	{
		OutputMsg(rmError, "RegisterNetMsgHandle error. (%d,%d)", nSystemID, nCmd);
		return false;
	}
	s_net_msg_handles[nSystemID][nCmd] = func;

	return true;
}

bool NetMsgDispatcher::RegisterScriptMsg(INT_PTR nSystemID, INT_PTR nCmd)
{
	return RegistNetMsgHandle(nSystemID, nCmd, ScriptNetMsgHandle);
}
