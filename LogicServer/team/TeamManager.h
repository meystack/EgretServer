
#pragma once

/***************************************************************/
/* 队伍管理器，管理游戏中的队伍
/***************************************************************/

class CTeamManager:
	public CComponent
{
public:
	typedef Handle<CTeam> TeamHandle; // 队伍的handle
	typedef HandleMgr<CTeam,TeamHandle> TeamMgr; //队伍管理器
	
	CTeamManager():m_mgr(_T("TeamHdlMgr")){}
	inline CTeam * CreateTeam(unsigned int& nTeamID)
	{
		TeamHandle hHandle;
		CTeam *pTeam= m_mgr.Acquire(hHandle);
		nTeamID= hHandle;
		if(pTeam)pTeam->SetTeamID(nTeamID);
		return pTeam;
	}

	inline void DestroyTeam(unsigned int nTeamID)
	{
		m_mgr.Release(nTeamID); //回收
	}

	inline CTeam *GetTeam(unsigned int nTeamID)
	{
		return m_mgr.GetDataPtr(nTeamID);
	}

	bool Initialize();
		
	VOID  Destroy()
	{
		m_mgr.Empty();
	}

	/**
	* @brief: 更新指定队伍中所有成员所在副本的队伍平均等级
	* @param[in]nTeamID: 队伍id
	*/
	void updateTeamMemFubenAveragePlayerLvl(unsigned int nTeamID);

	//处理队伍中的事情
	void RunOne( TICKCOUNT curTick );

private:
	TeamMgr m_mgr; //队伍管理器
	CTimer<3000>	m_timer;	//3秒执行一次
};
