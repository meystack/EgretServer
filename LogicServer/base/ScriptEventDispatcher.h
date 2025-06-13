#pragma once

class CActor;

// 玩家消息处理

class EventDispatcher
{
private:
	EventDispatcher(){};
	~EventDispatcher(){};

public:
	static void Init();
	// 处理玩家消息
	static bool OnActorEvent(CActor* pActor, INT_PTR nEventID, CScriptValueList & paramList, CScriptValueList & retParamList);
	// 注册脚本处理的消息
	static bool RegistActorEvent(INT_PTR nEventID);
};
