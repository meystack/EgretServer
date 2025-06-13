#pragma once
//跨服子系统

class CCrossSystem:
	public CEntitySubSystem<enCrossServerSystemID,CCrossSystem,CActor>
{
public:
	CCrossSystem();
	~CCrossSystem();

	virtual bool Initialize(void *data,SIZE_T size);

	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);
	//获取玩家在跨服的唯一id
	void GetCrossServerActorId();
	//请求登录
	void LoginCrossServer();
	//
	void SendCrossServerInfo(int nCrossActorId, int nSouthActorId);

private:
	bool			m_bCrossWorldOne;							//是否为跨服天下第一
};
