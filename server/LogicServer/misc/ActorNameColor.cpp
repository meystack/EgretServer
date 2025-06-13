#include "StdAfx.h"

CActor* GetActor(EntityHandle entity)
{
	CActor *pActor = NULL;
	CEntity *pEntity	= GetEntityFromHandle(entity);
	if (pEntity && enActor == pEntity->GetType() && pEntity->IsInited())
		pActor = (CActor *)pEntity;

	return pActor;
}

unsigned int GetColorByQuality(int nQuality)
{
	ACTOR_NAME_CLR_TABLE& clr_table = GetLogicServer()->GetDataProvider()->GetActorNameClrCfg();
	switch(nQuality)
	{
	case 0: // 白色
		return clr_table.clr_quality_white;
		break;
	case 1:// 绿色
		return clr_table.clr_quality_green;
		break;
	case 2:// 蓝色
		return clr_table.clr_quality_blue;
		break;
	case 3:// 紫色
		return clr_table.clr_quality_puple;
		break;
	case 4:// 橙色
		return clr_table.clr_quality_orange;
		break;
	case 5:// 红色
		return clr_table.clr_quality_red;
		break;
	default:
		return clr_table.clr_quality_white;
		break;
	}
}

CMonster* GetMonster(EntityHandle entity)
{
	CMonster *pMonster = NULL;
	CEntity *pEntity = GetEntityFromHandle(entity);
	if (pEntity && pEntity->IsMonster())
		pMonster = (CMonster *)pEntity;

	return pMonster;
}


int GetPkLevelByPkVal(int pkVal)
{
	int level = 0;
	if (pkVal >= GetLogicServer()->GetDataProvider()->GetPkConfig().redName)											
	{
		level = 2;
	}
	else if (pkVal > 100)										
	{
		level = 1;
	}

	return level;
}

/*
红名是最高优先级
*/
unsigned int GetNameColorLookedByOther(EntityHandle selfHandle, EntityHandle otherHandle, bool bIncChallenge)
{
	ACTOR_NAME_CLR_TABLE& clr_table = GetLogicServer()->GetDataProvider()->GetActorNameClrCfg();
	unsigned int color	= clr_table.clr_normal;
	CActor *pSelf = GetActor(selfHandle);
	CActor *pOther = GetActor(otherHandle);
	if (!pSelf || !pOther) return color;		
	bool bLookatSelf = (pSelf == pOther) ? true : false;

	// 2、杀戮值超过300，任何人看到的都是红色
	unsigned int self_pkval = pSelf->GetProperty<unsigned int>(PROP_ACTOR_PK_VALUE);
	if (pSelf->GetNameColorData() == 3)		//红名									
	{
		color = clr_table.clr_quality_red;
		return color;
	}
	CGuildComponent& guildMgr = 	GetGlobalLogicEngine()->GetGuildMgr();
	CGuildProvider& guildConfig = GetLogicServer()->GetDataProvider()->GetGuildConfig();
	// 1. 两人在切磋或者是擂台战双方
	if (!bLookatSelf)
	{
		int nMyCamp = pSelf->GetProperty<int>(PROP_ACTOR_ZY);
		int nOtherCamp = pOther->GetProperty<int>(PROP_ACTOR_ZY);
		if(nOtherCamp > 0 && nMyCamp != nOtherCamp)
		{
			color = clr_table.clr_quality_red;
			return color;
		}
	}

	if(pSelf->IsNightFighting())		//在补给争夺战中
	{
		int nMyCamp		= pSelf->GetProperty<int>(PROP_ACTOR_ZY);
		int nOtherCamp  = pOther->GetProperty<int>(PROP_ACTOR_ZY);
		if( nMyCamp > 0 && nOtherCamp >0 && nMyCamp == nOtherCamp )		//本方
		{
			color = clr_table.clr_quality_blue;
			return color;
		}
		else if( nMyCamp > 0 && nOtherCamp >0 && nMyCamp != nOtherCamp )	
		{
			color = clr_table.clr_pkmode;
			return color;
		}
	}
	
	CGuild* pGuild = pOther->GetGuildSystem()->GetGuildPtr();
	if(pGuild)
	{
		CGuild* nGuild = pSelf->GetGuildSystem()->GetGuildPtr();

		if(nGuild)
		{
			//在联盟中某个行会存在宣战状态时，这个颜色才变化
			if(pGuild->OnGuildWar())
			{
				if(pGuild == nGuild || pGuild->GetRelationShipForId(nGuild->m_nGid) == 1)	//同行会或联盟关系
				{	
					color = clr_table.clr_teammates;
					return color;
				}
				else if(pGuild->GetRelationShipInWar(nGuild->m_nGid) == 1)	//宣战关系
				{
					color = clr_table.clr_pkmode;
					return color;

				}
			}

			if(guildMgr.IsGuildSiegeStart() && pSelf->GetSceneAreaFlag() == scfGuildSiege)//攻城
			{
				if (!bLookatSelf)
				{
					if(nGuild->m_SignUpFlag && (pGuild == nGuild || pGuild->GetRelationShipForId(nGuild->m_nGid) == 1))//同盟
					{	
						color = clr_table.clr_teammates;
						return color;
					}
					else if (nGuild->m_SignUpFlag)
					{
						color = clr_table.clr_pkmode;
						return color;
					}
				}
				else
				{
					if(nGuild->m_SignUpFlag)
					{
						color = clr_table.clr_teammates;
						return color;
					}
				}
			
			}
		}
		else 
		{
			if(guildMgr.IsGuildSiegeStart() && pSelf->GetSceneAreaFlag() == scfGuildSiege)//攻城期间
			{
				if(bLookatSelf && pGuild->m_SignUpFlag)
				{
					color = clr_table.clr_teammates;
					return color;
				}
				
			}
		}
	}
	
	else if(guildMgr.IsGuildSiegeStart() && pSelf->GetSceneAreaFlag() == scfGuildSiege)
	{
		CGuild* nGuild = pSelf->GetGuildSystem()->GetGuildPtr();
		if(nGuild)
		{
			if(nGuild->m_SignUpFlag &&!bLookatSelf)
			{
				color = clr_table.clr_pkmode;
				return color;
			}
		}

	}

	if (pSelf->GetNameColorData() == 2)		//褐色									
	{
		color = clr_table.clr_force_pk;
		return color;
	}

	if(pSelf->GetNameColorData() ==1)					//黄色
	{
		color = clr_table.clr_pkval_over60;
		return color;
	}
	if (bLookatSelf)											
		return color;
	
	return color;
}


bool NeedUpdateNameClrByPkValChanged(unsigned int oldVal, unsigned int newVal)
{
	if (GetPkLevelByPkVal(oldVal) != GetPkLevelByPkVal(newVal))
		return true;

	return false;
}

//玩家登陆时，将自己看自己名称的颜色发给自己
void UpdateNameClr_OnActorLogin(EntityHandle selfHandle)
{
	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;

	unsigned int color = GetNameColorLookedByOther(selfHandle, selfHandle);
	CActorPacket ap; 
	pActor->AllocPacket(ap);
	ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << selfHandle << color;
	ap.flush();
}

void UpdateNameClr_OnPkValChanged(EntityHandle selfHandle, unsigned int oldVal, unsigned int newVal)
{
	if (!NeedUpdateNameClrByPkValChanged(oldVal, newVal))
		return;

	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;

	// 广播给周围玩家以及玩家自身
	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(pActor,vec);
		for (INT_PTR i = 0; i < vec.count(); i++)
		{
			CActor *pOther = GetActor(vec[i]);
			if (pOther)
			{
				unsigned int color = GetNameColorLookedByOther(selfHandle, vec[i]);  // 可以是自己看自己
				CActorPacket ap; 
				pOther->AllocPacket(ap);
				ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << selfHandle << color;
				ap.flush();
			}
		}
	}
}


void UpdateNameClr_OnJoinOrLeaveTeam(EntityHandle selfHandle, int nTeamId, bool bJoin)
{
	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;
		
	unsigned int nActorId = pActor->GetId();
	//if (bJoin)  
	{
		CTeam *pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (pTeam)
		{
			const TEAMMEMBER *member = pTeam->GetMemberList();
			for (INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; i++)
			{
				CActor *pTeammate = member[i].pActor;
				if (member[i].nActorID != nActorId && pTeammate != 0 && pTeammate->IsInited())
				{
					EntityHandle teammateHandle = pTeammate->GetHandle();
					// 发给队友
					unsigned int color = GetNameColorLookedByOther(selfHandle, teammateHandle);
					CActorPacket ap;
					pTeammate->AllocPacket(ap);
					ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << selfHandle << color;
					ap.flush();

					// 发给自己
					CActorPacket apSelf;
					pActor->AllocPacket(apSelf);					
					color = GetNameColorLookedByOther(teammateHandle, selfHandle);
					apSelf << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << teammateHandle << color;
					apSelf.flush();
				}
			}
		}
	}	
}


void UpdateNameClr_OnEnterOrLeavePK(EntityHandle selfHandle, EntityHandle otherSideHandle)
{
	CActor *pActor = GetActor(selfHandle);
	CActor *pOther = GetActor(otherSideHandle);
	if (!pActor || !pOther) return;
	
	// 发给对方
	unsigned int color = GetNameColorLookedByOther(selfHandle, otherSideHandle);
	CActorPacket ap;
	pOther->AllocPacket(ap);
	ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << selfHandle << color;
	ap.flush();

	// 发给自己
	CActorPacket apSelf;
	pActor->AllocPacket(apSelf);					
	color = GetNameColorLookedByOther(otherSideHandle, selfHandle);
	apSelf << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr << otherSideHandle << color;
	apSelf.flush();
	
}

unsigned int GetMonsterNameColorByOther(EntityHandle monsterHandle, EntityHandle otherHandle)
{
	unsigned int color = DEFAULT_NAME_COLOR;
	CMonster *pMonster = GetMonster(monsterHandle);
	CActor *pActor = GetActor(otherHandle);
	if (!pMonster || !pActor) return color;
	int nMonsterId = pMonster->GetProperty<int>(PROP_ENTITY_ID);
	PMONSTERCONFIG pCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
	if (!pCfg) return color;
	//if (pMonster->GetOwner().GetHandle() == otherHandle)
	//{
	//	ACTOR_NAME_CLR_TABLE& clr_table = GetLogicServer()->GetDataProvider()->GetActorNameClrCfg();
	//	return clr_table.clr_slave;
	//}
	return pCfg->nMonsterColor;
}

/*
将自己名称颜色改变，发给周围玩家（包括本人）
*/
void UpdateNameClr_OnPkModeChanged( EntityHandle selfHandle )
{
	
	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;

	// 广播给周围玩家以及玩家自身
	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(pActor,vec);
		for (INT_PTR i = 0; i < vec.count(); i++)
		{
			CActor *pOther = GetActor(vec[i]);
			if (pOther && pOther->IsInited())
			{
				if(pOther->OnGetIsTestSimulator()
					|| pOther->OnGetIsSimulator())
				{
					continue;
				} 

				unsigned int color = GetNameColorLookedByOther(selfHandle, vec[i]);  // 可以是自己看自己
				CActorPacket ap;
				pOther->AllocPacket(ap);	
				ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) selfHandle << color;
				ap.flush();
			}
		}
	}
}

/*
将周围玩家的名称颜色改变，发给本人
*/
void UpdateOtherNameClr_GiveSelf(EntityHandle selfHandle)
{
	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;

	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(pActor,vec);
		for (INT_PTR i = 0; i < vec.count(); i++)
		{
			CActor *pOther = GetActor(vec[i]);
			if (pOther && pOther->IsInited())
			{
				unsigned int color = GetNameColorLookedByOther(vec[i],selfHandle);  // 可以是自己看自己
				CActorPacket ap; 
				pActor->AllocPacket(ap);				
				ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) vec[i] << color;
				ap.flush();
			}
		}
	}
}

void BroadCastNameClr_Scene(CActor* pActor)
{
	if (!pActor) return;

	// 广播给周围玩家以及玩家自身
	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(pActor,vec);
		for (INT_PTR i = 0; i < vec.count(); i++)
		{
			CActor *pOther = GetActor(vec[i]);
			if (pOther && pOther->IsInited())
			{
				unsigned int color = GetNameColorLookedByOther(pActor->GetHandle(), vec[i]);  // 可以是自己看自己
				CActorPacket ap; 
				pOther->AllocPacket(ap);				
				ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) pActor->GetHandle() << color;
				ap.flush();
			}
		}
	}
}

void SendNameClr_ToSelf(CActor* pActor)
{
	if (!pActor) return;

	// 广播给玩家自身
	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		unsigned int color = GetNameColorLookedByOther(pActor->GetHandle(),pActor->GetHandle());  //自己看自己
		CActorPacket ap; 
		pActor->AllocPacket(ap);				
		ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) pActor->GetHandle() << color;
		ap.flush();
	}
}

//通知本人周围其他玩家更新他本人的名称颜色
void NotifyOtherNameClr_GiveThemSelf(EntityHandle selfHandle)
{
	CActor *pActor = GetActor(selfHandle);
	if (!pActor) return;

	// 广播给周围玩家以及玩家自身
	CScene *pScene = pActor->GetScene();
	if (pScene)
	{
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(pActor,vec);
		for (INT_PTR i = 0; i < vec.count(); i++)
		{
			CActor *pOther = GetActor(vec[i]);		//其他玩家
			if (pOther && pOther->IsInited())
			{
				unsigned int color = GetNameColorLookedByOther(vec[i], vec[i]);  // 其他玩家看他自己
				CActorPacket ap; 
				pOther->AllocPacket(ap);				
				ap << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64)vec[i]<< color;
				ap.flush();
			}
		}
	}
}

