#pragma once

class CActor;
typedef void(*NetMsgHandle)(CActor* pActor, INT_PTR nSystemID, INT_PTR nCmd, CDataPacketReader& pack);

// 玩家消息处理

class NetMsgDispatcher
{
private:
	NetMsgDispatcher(){};
	~NetMsgDispatcher(){};

public:
	static void Init();
	// 处理玩家消息
	static bool ProcessNetMsg(CActor* pActor, INT_PTR nSystemID, INT_PTR nCmd, CDataPacket& pack);
	// 注册消息处理函数
	static bool RegistNetMsgHandle(INT_PTR nSystemID, INT_PTR nCmd, NetMsgHandle func);
	// 注册脚本处理的消息
	static bool RegisterScriptMsg(INT_PTR nSystemID, INT_PTR nCmd);
};
