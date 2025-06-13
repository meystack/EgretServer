#include "StdAfx.h"
#include "../base/Container.hpp"

CPkSystem::CPkSystem()
{
	m_SubNext = (UINT)(GetGlobalLogicEngine()->getMiniDateTime() + GetLogicServer()->GetDataProvider()->GetPkConfig().pkSubTime);
	m_boCanPked = false;

}

CPkSystem::~CPkSystem()
{
}

bool CPkSystem::Initialize(void *data,SIZE_T size)
{
	return true;
}

VOID CPkSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();

	if ( nNow > m_SubNext && !m_pEntity->HasState(esStateDeath))
	{
		AddPkValue(-1);
		m_SubNext = nNow + (UINT)(GetLogicServer()->GetDataProvider()->GetPkConfig().pkSubTime);
	}
}

void CPkSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if (!m_pEntity) return;

	switch (nCmd)
	{
	case cInviteMatch:
		{
			break;
		}
	case cInviteResult:
		{
			break;
		}
	case cSetFreePkMode:
		{
			SetFreePkMode(packet);
			break;
		}
	case cOnFriendIntercede:
		{
			DoFriendIntercede(packet);
			break;
		}
	case cIntercedeMineral:
		{
			CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if (pNpc == NULL) return;
			CScriptValueList paramList, retParamList;
			paramList << m_pEntity;
			pNpc->GetScript().Call("IntercedeMineral",paramList,retParamList );
			break;
		}
	default:
		break;
	}
}

void CPkSystem::DoFriendIntercede(CDataPacketReader &reader )
{
	if (!m_pEntity)	
		return;
	CCLVariant& pVarMgr	= m_pEntity->GetActorVar();
	CCLVariant* pVar1 = pVarMgr.get(szIntercedeCount);
	int nIntercedeCount = 0;
	if( pVar1 != NULL )
	{
		nIntercedeCount = (int)(double)*pVar1;
	}
	CPkProvider& provider = GetLogicServer()->GetDataProvider()->GetPkConfig();
	if(provider.m_nCountPkValue <= nIntercedeCount)
	{
		return m_pEntity->SendOldTipmsgWithId(TpPK00012,ttFlyTip);
	}
	int nActorId = 0;
	reader >> nActorId;
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);	
	if(pActor == NULL)
	{
		return m_pEntity->SendOldTipmsgWithId(tpActorOffline,ttFlyTip);
	}
	const char* pActorName = pActor->GetEntityName();
	const char* pMyName = m_pEntity->GetEntityName();
	UINT nCurrPk = pActor->GetProperty<UINT>(PROP_ACTOR_PK_VALUE);

	pActor->SetProperty(PROP_ACTOR_PK_VALUE,nCurrPk - provider.m_nSubPkValue);
	pVarMgr.set(szIntercedeCount, nIntercedeCount+1);
	m_pEntity->SendOldTipmsgFormatWithId(TpPK00009,ttFlyTip,pActorName,provider.m_nSubPkValue);
	pActor->SendOldTipmsgFormatWithId(TpPK00010,ttFlyTip,pMyName,provider.m_nSubPkValue);
	pActor->SendOldTipmsgFormatWithId(TpPK00011,ttChatWindow,provider.m_nSubPkValue);
	CScriptValueList paramList;
}

void CPkSystem::SetFreePkMode( CDataPacketReader& reader )
{
	if (!m_pEntity)	
		return;
	//如果是强制pk模式，则不能修改
	int nPosX,nPosY;
	CScene* pScene = m_pEntity->GetScene();
	m_pEntity->GetPosition(nPosX,nPosY);
    if (pScene)
    {
        SCENEAREA* pArea = pScene->GetAreaAttri(nPosX, nPosY);
        if (pArea && pScene->HasMapAttribute(nPosX,nPosY,aaForbidSetpkMode,pArea))
        {
            return ;
        }
    }
	BYTE bMode = 0;
	reader >> bMode;
	SetPkMode(bMode);
}

bool CPkSystem::_CanAttack( CActor* pActor, CAnimal *pPet, bool boAttackNotice)
{
	//m_pEntity是攻击方
	bool result = true;

	if (!m_pEntity || !pActor || m_pEntity == pActor) return false;

	//除切磋之外. 要判断场景能否pk,双方都要处于非安全区域才能pk，否则不能pk(比武擂台除外)
	int x,y;
	CScene* pScene = m_pEntity->GetScene();
	m_pEntity->GetPosition(x,y);
	//注意：当攻击对方为新手保护期，其他玩家攻击时优先系统提示：对方新手保护，无法攻击
	if (pScene && pScene->HasMapAttribute(x,y,aaNewPlayerProtect))
	{
		if (m_pEntity->GetBuffSystem()->Exists(aNewPlayerProtect))
		{
			m_pEntity->SendOldTipmsgWithId(tpNewPlayerProtectSelf, ttFlyTip);	
			return false;
		}
		if (pActor->GetBuffSystem()->Exists(aNewPlayerProtect))
		{
			m_pEntity->SendOldTipmsgWithId(tpNewPlayerProtectOther, ttFlyTip);	
			return false;
		}
	}
	UINT myLevel = m_pEntity->GetProperty<UINT>(PROP_CREATURE_LEVEL);
	UINT otherLevel = pActor->GetProperty<UINT>(PROP_CREATURE_LEVEL);
	//INT_PTR startPkLevel = GetLogicServer()->GetDataProvider()->GetPkConfig().canPkStartLevel;
	//INT_PTR endPkLevel = GetLogicServer()->GetDataProvider()->GetPkConfig().canPkEndLevel;//终极杀戮等级：） 默认可以给人打
	//如果自己或者对方等级低于startPkLevel都绝对不能攻击
	///if (!pScene->HasMapAttribute(x,y,aaNotLevelProtect))
	//{
	//	if(myLevel < (UINT)startPkLevel)
	//	{
	//		m_pEntity->SendOldTipmsgWithId(tpNewPlayerProtectSelf, ttFlyTip);	
	//		return false;
	//	}
	//	if(otherLevel < (UINT)startPkLevel)
	//	{
	//		m_pEntity->SendOldTipmsgWithId(tpNewPlayerProtectOther, ttFlyTip);
	//		return false;
	//	}
	//}
	if (pScene && pScene->HasMapAttribute(x,y,aaSaft))
	{
		m_pEntity->SendOldTipmsgWithId(tpSafeAreaKill, ttFlyTip);
		return false;
	}
	if (pPet && pPet->GetType()==enPet)
	{
		pScene = pPet->GetScene();
		pPet->GetPosition(x,y);
	}
	else
	{
		pScene = pActor->GetScene();
		pActor->GetPosition(x,y);
	}
	if (pScene && pScene->HasMapAttribute(x,y,aaSaft))
	{
		m_pEntity->SendOldTipmsgWithId(tpSafeAreaKill, ttFlyTip);
		return false;
	}
	INT_PTR nPkMode = m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);

	//如果我是和平模式，无论如何都不能攻击
	if (nPkMode == fpPeaceful)
	{
		if (boAttackNotice && m_pEntity->GetTarget() == pActor->GetHandle())
		{
			SendPkModeStr();
		}		
		return false;
	}
	//如果他有保护buff，我打不到他，他也不能攻击别人，因为他肯定是和平模式
	if (pActor->GetPkSystem().IsProtected())
	{
		return false;
	}

	//沙巴克战的行会模式不攻击联盟行会的
	CGuildSystem* gs = m_pEntity->GetGuildSystem();
	CGuildSystem* otherGs = pActor->GetGuildSystem();
	if (!(gs->IsFree()) && !(otherGs->IsFree()))
	{
		
		if (nPkMode == fpGuild && GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart() && gs->GetGuildPtr()->GetRelationShipForId(otherGs->GetGuildPtr()->m_nGid) == 1)
		{
			return false;
		}
	}

	//如果杀戮值大于300且没有保护buff，任何人（非和平模式）都可以打他
	if (pActor->GetProperty<UINT>(PROP_ACTOR_PK_VALUE) >= (UINT)GetLogicServer()->GetDataProvider()->GetPkConfig().redName)
	{
		result = true;
		goto CANATTACK;
	}
	//如果大于40级且没有保护buff，任何人（非和平模式）都可以打他
	//if (otherLevel >= (UINT)endPkLevel)
	//{
		result = true;
	//	goto CANATTACK;
	//}

	//战场模式，暂无
	//end 战场
	//if (otherLevel >= (UINT)startPkLevel && otherLevel < (UINT)endPkLevel)
	//{
		//其他等级下，如果是和平模式，则不可以被PK
		if (pActor->GetProperty<int>(PROP_ACTOR_PK_MOD) == fpPeaceful) return false;
	//}
	
CANATTACK:
	if (result)
	{
		//还要判断你的模式
		result = CanAttackByMode(pActor);
		if (result)
		{
			DeleteProtectBuff();//清掉其保护buff
		}
	}
	return result;
}

bool CPkSystem::IsUnLimited(CEntity* pEntity)
{
	if (!m_pEntity) return false;
	
	int x,y;
	CScene* pScene = m_pEntity->GetScene();
	if(!pScene) return false;

	// 自己在安全区，受限制
	m_pEntity->GetPosition(x,y);
	if (pScene->HasMapAttribute(x,y,aaSaft))
	{
		//m_pEntity->SendTipmsgWithId(tpSafeAreaKill, ttFlyTip);
		((CActor*)m_pEntity)->SendTipmsg(_T("你在安全区中，不能攻击"),tstFigthing);
		return false;
	}
	
	// 对方在安全区，受限制
	pEntity->GetPosition(x,y);
	if (pScene->HasMapAttribute(x,y,aaSaft))
	{
		((CActor*)m_pEntity)->SendTipmsg(_T("对方在安全区中，不能攻击"),tstFigthing);
		return false;
	}

	// 对方有PK保护，受限制
	if(((CAnimal*)pEntity)->GetBuffSystem()->Exists(aPkProtectState))
	{
		((CActor*)m_pEntity)->SendTipmsg(_T("对方在PK保护中，不能攻击"),tstFigthing);
		return false;
	}

	return true;
}

bool CPkSystem::CanAttack(CActor* pActor, CAnimal * pPet, bool)
{
	if (!m_pEntity) return false;
	
	INT_PTR nPkMode = m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);
	switch (nPkMode)
	{
	case fpPeaceful:
		if (pActor->IsBadMan())
		{
			return true;
		}
		return false;
	case fpTeam:
		{
			UINT teamId	= m_pEntity->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
			UINT otherTeamId = pActor->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
			if (teamId && teamId == otherTeamId)
			{
				return false;
			}
			return m_pEntity != pActor;
		}
	case fpGuild:
		{
			UINT guildid = m_pEntity->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			UINT otherguildid = pActor->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			if (guildid && guildid == otherguildid)
			{
				return false;
			}
			return m_pEntity != pActor;
		}
	case fpPk:
		if (pPet && pPet->GetType() == enPet)
		{
			return true;
		}
		return m_pEntity != pActor;
	case fpZY:
		{
			if( GetZyType(pActor) == ztSameZY ) 
			{
				SendPkModeStr();
				return false;
			}
			return true;
		}
	case fpEvil:
		{
			UINT teamId	= m_pEntity->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
			UINT otherTeamId = pActor->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
			if (teamId && teamId == otherTeamId)
			{
				return false;
			}
			UINT guildid = m_pEntity->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			UINT otherguildid = pActor->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			if (guildid && guildid == otherguildid)
			{
				return false;
			}
		}
		break;
	default:
		return false;
	}

	// 在战争区域
	int x,y;
	CScene* pScene = m_pEntity->GetScene();
	m_pEntity->GetPosition(x,y);
	if (pScene && pScene->HasMapAttribute(x,y,aaZY))
	{
		return true;
	}
	
	// 善恶
	if (pActor->IsBadMan())
	{
		return true;
	}
	
	return false;
}

void CPkSystem::OnEndPk( CActor* pActor )
{
	if (!m_pEntity || !pActor) return;
	//计算哪方胜利
	CActor* pWin = m_pEntity;
	CActor* pLost = pActor;

	if(pLost == pWin) return;

	if ( pWin->GetProperty<UINT>(PROP_CREATURE_HP) < pLost->GetProperty<UINT>(PROP_CREATURE_HP))
	{
		pWin = pActor;
		pLost = m_pEntity;
	}
	//比武擂台不计算PK值
	int x,y;
	//CScene* pScene = pLost->GetScene();
	pLost->GetPosition(x,y);

	//如果是帮派战，记录帮派的击杀人数
	CGuild* pMyGuild = pWin->GetGuildSystem()->GetGuildPtr();
	CGuild* pOtherGuild = pLost->GetGuildSystem()->GetGuildPtr();
	if (pMyGuild && pOtherGuild)
	{
		// 行会PK
		if( pMyGuild->GetRelationShipInWar(pOtherGuild->m_nGid) == GUILD_WARSHIP_YES )	//宣战关系，不加PK值
		{
			pMyGuild->AddPkCount(pOtherGuild->m_nGid);	//失败方同时加上死亡数目
			pOtherGuild->AddDieCount(pMyGuild->m_nGid);
			return;
		}
	}

	//沙巴克战时
	if(pWin->GetSceneAreaFlag() == scfGuildSiege && GetGlobalLogicEngine()->GetGuildMgr().IsGuildSiegeStart())
	{
		return;
	}

	//褐名或红名
	if(pLost->GetNameColorData() > 1)
	{
		return;
	}

	/*
	//新增沙巴克老大不加pk值
	unsigned int nSbkMasterId = GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnMasterId();
	if (nSbkMasterId > 0 && pActor->GetId() == nSbkMasterId)
	{
		return;
	}
	*/
	//本人是沙巴克成员，击杀者不犯法
	/*unsigned int nGuildid= m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
	unsigned int nKillGuildId = pActor->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID);
    unsigned int nSbkGuildId = GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId();
	if ( nSbkGuildId > 0 &&  nSbkGuildId == nGuildid && nKillGuildId != nGuildid )
	{
		return;
	}*/

	/*被杀者是击杀者的仇人，击杀者不加PK值（同时删除被杀者不再是击杀者的仇人）
	  此处是在最后抵销一次
	*/
	// if ( pWin->GetFriendSystem().GetSocialFlag(pLost->GetId(),SOCIAL_ANIMAL) )
	// {
	// 	//pWin->GetFriendSystem().DoAddSocialRelation(RELATION_OP_ENIMY, 1, pLost->GetId(), NULL);  //通知击杀者删除仇人
	// 	return;
	// }

	CPkProvider& provider = GetLogicServer()->GetDataProvider()->GetPkConfig();
	int nPk = provider.m_pkValue;
	pActor->GetPkSystem().AddPkValue(nPk, m_pEntity);


	/* 取消队伍 修改为只个人获得 2011-10-10
	//如果是个人pk，则个人获取战魂值，否则是队伍获取
	CTeam* pTeam = pWin->GetTeam();
	if (pTeam)
	{
		//团队的的每个人都可以获得战魂
		const TEAMMEMBER* pMember=  pTeam->GetMemberList();
		for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
		{
			if(pMember[i].pActor && (pMember[i].pActor == pWin || pWin->IsInSameScreen(pMember[i].pActor) ))
			{
				pMember[i].pActor->GetPkSystem().GetZhanHun(pLost);
			}
		}

	}else
	{
		pWin->GetPkSystem().GetZhanHun(pLost);
	}
	*/
}

/*实际增加PK值
*/
void CPkSystem::AddPkValue( INT_PTR nValue, CActor * pKilledActor )
{
	//OutputMsg(rmNormal, "CPkSystem::AddPkValue: nValue=%d", nValue);

	if (!m_pEntity) return;
	CScene * pScene = m_pEntity->GetScene();
	if(!pScene) return;

	CPkProvider& provider = GetLogicServer()->GetDataProvider()->GetPkConfig();
	
	int x,y;

	m_pEntity->GetPosition(x,y);
	if (nValue > 0)
	{
		SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
		if(!pArea) return;
		//战斗区域不修改pk
		if (pScene->HasMapAttribute(x,y,aaWar,pArea)) return;
		//阵营战争区域不修改pk
		if (pScene->HasMapAttribute(x,y,aaZY,pArea)) return;
	}

	if (pKilledActor)
	{
		pKilledActor->GetPosition(x,y);
		{
			SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
			if(!pArea) return;
			//战斗区域不修改pk
			if (pScene->HasMapAttribute(x,y,aaWar,pArea)) return;
			//阵营战争区域不修改pk
			if (pScene->HasMapAttribute(x,y,aaZY,pArea)) return;	
		}
	}

	nValue = nValue + m_pEntity->GetProperty<int>(PROP_ACTOR_PK_VALUE);
	if (nValue < 0) nValue = 0;
	m_pEntity->SetProperty<UINT>(PROP_ACTOR_PK_VALUE,(UINT)nValue);	

	if (nValue > 0)//定期减少的不提示
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpPkChange,ttTipmsgWindow,nValue);
	}
}

bool CPkSystem::IsProtected()
{
	if (m_pEntity)
	{
		if(m_pEntity->GetBuffSystem()->Exists(aPkProtectState))
		{
			return true;
		}
	}
	return false;
}

void CPkSystem::DeleteProtectBuff()
{
	//删除pk保护的buff
	if (m_pEntity)
	{
		CBuffSystem *pSystem = m_pEntity->GetBuffSystem();
		pSystem->Remove(aPkProtectState,CDynamicBuff::AnyBuffGroup);
		if (pSystem->Exists(aNewPlayerProtect,CDynamicBuff::AnyBuffGroup))
		{
			m_pEntity->GetBuffSystem()->Remove(aNewPlayerProtect,CDynamicBuff::AnyBuffGroup);
			m_pEntity->SendOldTipmsgWithId(tpNewPlayerProtectDel, ttFlyTip);	
		}		
	}
}

void CPkSystem::SetPkMode( int mode,bool boForce,bool bNeedBroadCast)
{
	if (!m_pEntity || mode >= fpCount || mode < fpPeaceful) return;

	if (mode != fpPeaceful)
	{
		DeleteProtectBuff();//设置成非和平模式，删除保护buff
	}
	
	m_bLastMode = m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);//保存之前的模式
	//auto var(m_pEntity->GetDynamicVar());
	//var.set("lastPkMode", m_bLastMode);
	//如果pk模式没有改变的话是不需要广播的
	if(m_bLastMode ==mode)
	{
		return;
	}
	
	m_pEntity->SetProperty<int>(PROP_ACTOR_PK_MOD,mode);
 
	if(m_pEntity->IsInited()
	  && !((CActor*)m_pEntity)->OnGetIsTestSimulator()
	  && !((CActor*)m_pEntity)->OnGetIsSimulator())
	{
		CActorPacket AP;
		CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
		DataPacket << (BYTE)enPkSystemID << (BYTE)sSetFreePkMode << (BYTE)mode;
		AP.flush();

		m_pEntity->SendTipmsgFormatWithId(tmPKMod_1 + mode - 1,tstFigthing);
	}	

	//如果不需要广播就不广播，在进入游戏的时候是不需要广播的
	if(bNeedBroadCast)
	{
		UpdateNameClr_OnPkModeChanged(m_pEntity->GetHandle());
	}	
}

int CPkSystem::GetPkMode()
{
	if (!m_pEntity) return 0;
	return m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);
}

void CPkSystem::SendPkModeStr()
{
	int nTipId = 0;
	switch(GetPkMode())
	{
	case fpPeaceful:
		nTipId = 0;
		break;
	/*case fpTeam:
		nTipId = tpPkModeTeam;
		break;
	*/case fpGuild:
		nTipId = 0;
		break;
	case fpEvil:
		nTipId = 0;
		break;
	case fpPk:
		nTipId = 0;
		break;
	/*case fpZY:
		nTipId = tpPkModeCamp;
		break;*/
	default:
		break;
	}
	if (nTipId != 0)
	{
		m_pEntity->SendOldTipmsgWithId(nTipId, ttFlyTip);
	}

}

void CPkSystem::OnEnterGame()
{
	if(!m_pEntity)
	{
 		return;
	}
	m_pEntity->SetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS,0);
	//进入游戏的时候，设置pk模式是不需要广播的
	SetPkMode(m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD),true,false);
	// 这里强制下发Pk。在某些会改变PK模式的区域上线，进入场景会更新PK模式，然后进入游戏再次
	// 设置相同的PK模式会导致不会下发给客户端。而之前发送数据包的时候，客户端还没有创建角色
	// 导致客户端PK模式更新不正确的bug
	
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return; 
    }  
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enPkSystemID << (BYTE)sSetFreePkMode;
	DataPacket << (BYTE)m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);
	AP.flush(); 
}

/*根据行会模式，判断能否攻击对方
队友，任何情况下都不能攻击
*/
bool CPkSystem::CanAttackByMode( CActor* pActor )
{
	if (!m_pEntity) return false;
	
	bool result = true;
	//策划说任何情况同一队伍的都不可以打
	UINT teamId			= m_pEntity->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
	UINT otherTeamId	= pActor->GetProperty<UINT>(PROP_ACTOR_TEAM_ID);
	if (teamId != 0 && teamId == otherTeamId)
	{
		result = false;
		SendPkModeStr();
		return result;
	}
	
	INT_PTR nPkMode = m_pEntity->GetProperty<int>(PROP_ACTOR_PK_MOD);	
	switch(nPkMode)
	{
	case fpPeaceful:
		{
			result = false;
			SendPkModeStr();
			break;
		}
	case fpGuild:	//行会模式（仅同行会）
		{
			UINT guildid = m_pEntity->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			UINT otherguildid = pActor->GetProperty<UINT>(PROP_ACTOR_GUILD_ID);
			if (guildid != 0 && guildid == otherguildid)
			{
				result = false;
				SendPkModeStr();
			}
			break;
		}
	case fpZY:
		{
			if( GetZyType(pActor) == ztSameZY ) 
			{
				result = false;
				SendPkModeStr();
			}
			break;
		}
	case fpPk:
		{
			result = true;
			break;
		}
	
	case fpUnion:	//联盟(组队、行会、联盟都算)
		{
			//result = false;
			/*如果是结盟阵营，不能攻击
			CCamp* pCamp = m_pEntity->GetCamp();
			CCamp* pTargetCamp = pActor->GetCamp();
			if (pCamp == pTargetCamp || pCamp->IsCampAllied(pTargetCamp))
				result = false;
			else
				result = true;
			break;
			*/
			CGuild* pMyGuild	= ((CActor*)m_pEntity)->GetGuildSystem()->GetGuildPtr();
			CGuild* pObjGuild	= pActor->GetGuildSystem()->GetGuildPtr();
			if( pMyGuild && pObjGuild )
			{
				//同行会或者行会联盟
				if( pMyGuild == pObjGuild || pMyGuild->GetRelationShipForId( pObjGuild->GetGuildId() ) == GUILD_RELATION_UNION )
				{
					return false;
				}
			}
			break;
		}
	case fpEvil:
		{
			//为红名的就可以攻击
			if(pActor->GetNameColorData() > 1)
			{
				result = true;
			}
			else
			{
				result = false;
				SendPkModeStr();
			}

			break;
		}
	}
	return result;
}

INT_PTR CPkSystem::GetZyType( CActor* pActor )
{
	if (!m_pEntity || !pActor) return ztSameZY;
	UINT myCamp = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_ZY);
	UINT otherCamp = pActor->GetProperty<unsigned int>(PROP_ACTOR_ZY);
	if (myCamp > 0 && otherCamp > 0)
	{
		if(myCamp != otherCamp)
			return ztEnemyZY;
		else
			return ztSameZY;
	}
	return ztNeutralZY;
}

void CPkSystem::EnterPK(CActor* pEntity)
{
	if (!m_pEntity) return;
	
	if(pEntity->GetType() == enActor)
	{
		SendPkToClient();
		m_pEntity->SendOldTipmsgWithId(tpEnterPK,ttFlyTip);
		pEntity->SendOldTipmsgWithId(tpEnterPK,ttFlyTip);
	}
	else
	{
		m_pEntity->SendOldTipmsgWithId(tpEnterF,ttFlyTip);
	}
}

void CPkSystem::SendPkToClient()
{
	if (!m_pEntity)
	{
	 	return;
	}
	if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
		|| ((CActor*)m_pEntity)->OnGetIsSimulator() )
	{
		return;
	}
	//B 原处于 非战斗状态， A 攻击 B 令其进入 战斗状态时， 才会弹出 打他丫滴！ 窗口。
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	DataPacket << (BYTE)enPkSystemID << (BYTE)sKillHe;
	DataPacket.writeString("");
	AP.flush();
}
