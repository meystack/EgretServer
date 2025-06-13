#pragma once

//和服务器连接的一个个用户，根据玩家和会话服、数据服、逻辑服等服务器的连接状态，可以分为几种用户
//本类是处理服务器的函数的基类
class CRobotClient;
class CAgent
{
public:
	CAgent()
	{
		m_pClient =NULL;
	}
	
	//初始化函数
	virtual void Init(CRobotClient * pClient)
	{
		m_pClient = pClient;
	}

	//与服务器断开了连接
	virtual void OnConnected(){}

	//于服务器断开了
	virtual void OnDisConnected(){}

	//派发网络消息
	virtual void OnRecv(CDataPacketReader & pack){}

	//定期调用
	virtual void OnTimeRun(TICKCOUNT tick){}

protected:
	CRobotClient* m_pClient; //与服务器的通信
};