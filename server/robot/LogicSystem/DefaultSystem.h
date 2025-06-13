#pragma once
#include<map>
/***************************************************************/
/*
/*                     默认子系统
/*    对应子系统ID为0的系统，主要处理一些逻辑基本消息
/***************************************************************/

class CDefaultSystem:
	public CEntitySystem<enDefaultEntitySystemID>
{
public:

	/*
	* Comments:收到了网络数据包的处理函数
	* Param INT_PTR nCmd:数据包的命令
	* Param CDataPacketReader & pack:读取器
	*/
	virtual void OnRecvData(INT_PTR nCmd,CDataPacketReader & pack);
		
	//定时执行
	virtual void OnTimeRun(TICKCOUNT currentTick ); 

	// 某坐标位置是否有(某类型)实体
	bool HasEntity(unsigned int  nEntityType, int nPosX, int nPosY);
	bool HasEntity(int nPosX, int nPosY);
	// 获取某位置某类型实体
	CClientEntity* GetEntity(unsigned int nEntityType, int nPosX, int nPosY,bool include_guard = false);

	// 获取其他玩家数据
	inline CClientActor* GetOtherData(EntityHandle handle)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			return &m_OtherActor[handle];
		}
		return NULL;
	}
	// 获取怪物数据
	inline int GetMonsterCount() { return m_Monster.size(); }
	inline CClientMonster* GetMonster(EntityHandle handle)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			return &m_Monster[handle];
		}
		return NULL;
	}
	CClientMonster* GetMonster(bool include_guard = false);
	CClientMonster* GetMonsterNot(EntityHandle handle,bool include_guard = false);
	CClientMonster* GetNearestMonster(int dis = 14,bool include_guard = false);
	CClientMonster* GetNearestMonsterNot(EntityHandle handle,int dis = 14,bool include_guard = false);
	// 获取宠物数据
	inline CClientPet* GetPet(EntityHandle handle)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			return &m_Pets[handle];
		}
		return NULL;
	}

	// 获取自己的位置
	void GetPosition(int &x, int &y) const;
	// 获取其他玩家位置
	bool GetOtherPos(EntityHandle handle, int& nX, int& nY);
	// 获取怪物位置
	bool GetMonsterPos(EntityHandle handle, int& nX, int& nY);
	// 获取宠物位置
	bool GetPetPos(EntityHandle handle, int& nX, int& nY);
	// 获取实体位置
	bool GetEntityPos(EntityHandle handle, int& nX, int& nY);

	// 请求移动
	void Move(int nX, int nY, int nStep, int nDir);
	//移动到目标点
	void MoveTo(int nEndX, int nEndY, INT_PTR nStep=2);
	// 请求复活
	void ReqRelive();
	//获取距离
	int GetDistance(int nPosX,int nPosy);
	//获取距离（横纵最长的）
	unsigned int GetMaxDistance(int nPosX,int nPosy);

	inline static INT_PTR GetDisSqare(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY,INT_PTR nTargetX, INT_PTR nTargetY)
	{
		nCurrentPosX -= nTargetX;
		nCurrentPosY -= nTargetY;
		return nCurrentPosX* nCurrentPosX + nCurrentPosY* nCurrentPosY;
	}
	//计算移动的方向和步子
	inline bool CalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep,INT_PTR nMinDisSquare=0,INT_PTR nMaxDisSquare =0);
	bool GetFanAvailablePos(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR &nStep);

private:

	//处理心跳包
	void ProcessHeartBeat(CDataPacketReader & recvPack);
	//处理玩家登陆失败
	void ProcessLoginFail(CDataPacketReader & recvPack);
	//处理主角创建包
	void ProcessMainActorCreated(CDataPacketReader & recvPack);
	//其他玩家进入视野
	void ProcessActorAppear(CDataPacketReader &packet);
	//怪物进入视野
	void ProcessMonsterAppear(CDataPacketReader &packet);
	//NPC进入视野
	void ProcessNPCAppear(CDataPacketReader &packet);
	//宠物出现在视野
	void ProcessPetAppear(CDataPacketReader &packet);
	//其他实体出现在视野
	void ProcessOtherAppear(CDataPacketReader &packet);
	// 处理实体消失
	void ProcessEntityDisappear(CDataPacketReader &packet);
	// 处理主玩家属性变更
	void ProcessMainActorPropChange(CDataPacketReader &packet);
	// 处理其他实体属性变更
	void ProcessOtherEntityPropChange(CDataPacketReader &packet);
	// 处理玩家进入场景
	void ProcessPlayerEnterScene(CDataPacketReader& packet);
	// 处理公共操作结果
	void ProcessCommonOpResult(CDataPacketReader& packet);
	// 处理其他实体移动
	void ProcessOtherEntityMove(CDataPacketReader& packet);
	// 处理重设主角位置
	void ProcessResetMainActorPos(CDataPacketReader& packet);
	// 处理实体死亡
	void ProcessEntityDeath(CDataPacketReader&);
	// 处理实体瞬间移动
	void ProcessEntityInstatnceMove(CDataPacketReader& packet);
	// 处理传送
	void ProcessEntityTransport(CDataPacketReader& packet);
	
	// 更新实体属性实现函数		
	void UpdateEntityPropImpl(CClientEntity* pEntity, CDataPacketReader& packet);
	// 更新玩家坐标。用于客户端和服务器走路不同步的时候，根据服务器位置来调整
	void SetPosition(const int nX, const int nY);

	// 地图坐标实体映射关系维护方法
	void AddMap(CClientEntity* pEntity, int nPosX, int nPosY);
	void DelMap(CClientEntity* pEntity, int nPosX, int nPosY);
	void ChgMap(CClientEntity* pEntity, int nOldPosX, int nOldPosY, int nPosX, int nPosY);

private:
	
	std::map<EntityHandle,CClientActor> m_OtherActor; 	//其他玩家
	std::map<EntityHandle,CClientMonster> m_Monster; 	//怪物
	std::map<EntityHandle,CClientNPC> m_Npc; 			//NPC
	std::map<EntityHandle,CClientPet> m_Pets; 			//宠物
	std::map<EntityHandle,CClientTransfer> m_Transfer; 	//传送门

	std::map<int, std::vector<CClientEntity*>> m_PosiEntityMap;	//地图坐标实体映射
	int m_nMapWidth;
	TICKCOUNT  m_heartbeart;
};

