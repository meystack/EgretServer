#pragma once
/***************************************************************/
/*
/*                     模拟器子系统基类
/* 模拟器的功能模块,比如组队子系统,技能子系统，继承这个基类去处理各种消息和逻辑
/***************************************************************/


template<int SUBSYSTEMID>
class CEntitySystem
{
public:


public:
	CEntitySystem()
	{
		m_pClient =NULL;
	}

	/*
	* Comments:收到了网络数据包的处理函数
	* Param INT_PTR nCmd:数据包的命令
	* Param CDataPacketReader & pack:读取器
	* @Return void:
	*/
	virtual void OnRecvData(INT_PTR nCmd,CDataPacketReader & pack){};
	
	 //定时执行
	virtual void OnTimeRun(TICKCOUNT currentTick ){} 

	//获取子系统的ID
	inline int GetSystemId(){return SUBSYSTEMID;}
	
	//初始化
	void Init(CRobotClient * pClient){m_pClient =pClient;}

	//进入游戏，收到了服务器发送的主角创建的包
	virtual void OnEnterGame(){}

protected:
	CRobotClient *  m_pClient ;    //用于发送数据包和接收数据包        
};

