#pragma once
/***************************************************************/
/*
/* 队伍子系统
/* 玩家队伍相关的网络消息的处理，各种队伍相关的处理
/* 队伍的一些需要广播的消息也放要放到这里处理
/***************************************************************/

#include <map>

class CTeamSystem:
	public CEntitySubSystem<enTeamSystemID,CTeamSystem,CActor>
{
	
public:
	typedef CEntitySubSystem<enTeamSystemID,CTeamSystem,CActor> Inherid;
	friend class CEntitySubSystem<enTeamSystemID,CTeamSystem,CActor>;

	/*
	 *处理网络数据包
	 *nCmd ，分派到该系统里命令，从0开始
	 *packet，网络数据
	*/
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//子系统初始化,如果是玩家需要输入玩家的存盘数据
	virtual bool Initialize(void *data,SIZE_T size);

	void Destroy();
	
	/*
	* Comments: 向队伍里的人广播一条消息，如果没有队伍就提示客户端
	* Param void * data: 数据指针
	* Param SIZE_T size:数据长度
	* @Return void:
	*/
	void BroadCast(void * data,SIZE_T size);

	//进入游戏
	virtual void OnEnterGame();

	/*
	* Comments:角色移动时，广播消息给队友
	* @Return void:
	*/
	void TeamMoveBroadcast( );

	/*
	* Comments:邀请某人组队
	* Param CActor * pMember:
	* @Return void:
	*/
	void InviteJoinTeam( CActor* pMember);

	/// 创建队伍
	CTeam *CreateTeam(CActor* pMember = nullptr);
	int IsAllowTeam() const {return m_nAllowTeam;}
	void SetAllowTeam(int type);
	
protected:
	//错误的消息
	void HandleError(CDataPacketReader &packet){};

	// 1 邀请加入队伍
	void HandleInvite(CDataPacketReader &packet);

	// 2 退出队伍
	void HandleLeave(CDataPacketReader &packet);

	// 3 申请加入队伍
	void HandleApplyJoinTeam(CDataPacketReader &packet);

	// 4 设置队长
	void HandleSetCaptin(CDataPacketReader &packet);
	
	// 5 踢人
	void HandleKickMember(CDataPacketReader &packet);
	
	// 6 解散队伍
	void HandleDestroyTeam(CDataPacketReader &packet);

	// 7 回复申请入队
	void HandleApplyJoinTeamReply(CDataPacketReader &packet);
	
	// 8 回复邀请入队
	void HandleInviteJoinTeamReply(CDataPacketReader &packet);

	// 9 获得附近的队伍信息
	void GetNearTeamInfo(CDataPacketReader & packet);

	// 10 创建队伍
	void AskCreateTeam(CDataPacketReader & packet);

	// 11 获得附近的角色信息
	void GetNearActorsInfo(CDataPacketReader & packet);

	/// 申请加入某个team
	void ApplyJoinOneTeam(CTeam* pTeam);

	//获取我方组队信息
	void GetMyTeamInfo(CDataPacketReader& packet);
	
	void HandleSetIsAllow(CDataPacketReader& packet);
	void HandleGetAllowTeamState(CDataPacketReader& packet);
private:

	/*
	* Comments: 检查是否在队伍里，并且是队长
	* Param bool bWithTipmsg:是否需要提示
	* @Return CTeam* &: 如果在队伍里，并且是队长，返回 =true，否则为false ,CTeam返回为队伍指针，免得再取一次 
	*/
	bool CheckIsCaptin(CTeam* & pTeam, bool bWithTipmsg = true);

	unsigned int				nUpdataDt;				// 上次刷新时间，防止恶意刷新	
	unsigned int				m_nUpdateActorListDt;	// 上次刷新时间，防止恶意刷新	
	int                        m_nAllowTeam;                 //是否开始自动组队
	std::map<unsigned int, int>	m_NearTeam;				// 用来去掉重复的队伍id,附近队伍id
	CVector<unsigned int>		m_NoTeamActor;			// 附近未组队的角色
	bool                        m_nInit;                //登录初始化--防止再次添加离线数据
};
