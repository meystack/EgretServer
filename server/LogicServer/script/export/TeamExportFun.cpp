#include "StdAfx.h"
#include "../interface/TeamExportFun.h"

namespace TeamFun
{
	void* getTeamCaptain( unsigned int nTeamId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			return pTeam->GetCaptin();
		}
		return NULL;
	}

	void* getTeamCaptain( void* pTeam )
	{
		if (pTeam)
		{
			return ((CTeam*)pTeam)->GetCaptin();
		}
		return NULL;
	}

	unsigned int getChallenge( unsigned int nTeamId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			return pTeam->GetChallengeId();
		}
		return 0;
	}

	void setChallenge( unsigned int nTeamId,unsigned int nChalllengeId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			return pTeam->SetChallengeId(nChalllengeId);
		}
	}

	void setTeamFubenId( unsigned int nTeamId,int nFbId )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			return pTeam->SetFbId(nFbId);
		}
	}

	int getDyanmicVar( lua_State *L )
	{
		unsigned int nTeamId = (unsigned int )lua_tonumber(L, 1);
		if( nTeamId > 0)
		{
			CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
			if (pTeam)
			{
				return LuaCLVariant::returnValue(L, pTeam->GetVar());
			}
		}
		return 0;
	}

	bool setTeamTimer( unsigned int nTeamId, unsigned int nExpiredTime )
	{
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			pTeam->SetExpiredTime(nExpiredTime);
			return true;
		}
		return false;
	}

};

