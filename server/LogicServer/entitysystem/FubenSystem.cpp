#include "StdAfx.h"
#define  TEAM_OUTPUT_CD 2
#define  TEAM_OUTPUT_OPEN 1
#define  TEAM_BEGIN_SECOND 2
bool CFubenSystem::Initialize(void *data,SIZE_T size)
{
	return true;
}

void CFubenSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	int nFbId = 0;
	packet >> nFbId;
	
	switch (nCmd)
	{
	case cEnterFuben:
		{
			ReqEnterFuben(nFbId);
			break;
		}
	case cExitFuben:
		{
			ReqExitFuben(nFbId);
			break;
		}
	case cGetFubenAward:
		{
			GetFubenAward(nFbId);
			break;
		}
	default:
		break;
	}
}

void CFubenSystem::SendRestTime()
{
	if (!m_pEntity) return;
	CScene* pScene = m_pEntity->GetScene();
	if (!pScene) return;
	CFuBen* pFb = m_pEntity->GetFuBen();
	if (!pFb) return;
	CActorPacket AP;
	CDataPacket& DataPacket = m_pEntity->AllocPacket(AP);
	int nSceneTime = pScene->GetSceneTime();
	nSceneTime = nSceneTime != 0?nSceneTime:pFb->GetFubenTime();
	AP << (BYTE)enFubenSystemID << (BYTE)sFubenRestTime << (int)nSceneTime;
	AP.flush();
}

void CFubenSystem::OnEnterGame()
{
}

bool CFubenSystem::ReqExitFuben( int nFubenId )
{	
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return false;
	}
	CScriptValueList paramList;

	// 常规退出检查
	CFuBen* pFb =  ((CActor*)m_pEntity)->GetFuBen();
	int nCurFbId = pFb->GetFbId();
	if (!pFb->IsFb())
	{
		// 这里是不在副本内却请求退出，TODO 加提示
		return false;
	}

	// 角色事件回调
	CActor* pActor = ((CActor*)m_pEntity);
	paramList << nFubenId;
	pActor->OnEvent(aeOnExitFuben,paramList,paramList);
	pActor->ReturnCity();
	// pActor->ReturnToStaticScene(pActor->m_nEnterFbScenceID, pActor->m_nEnterFbPosx, pActor->m_nEnterFbPosy);
	return true;
}

bool CFubenSystem::ReqEnterFuben(int nFbid)
{
	if (!m_pEntity) return false;	
	CScriptValueList paramList,retList;

	if(m_pEntity->GetScene() && m_pEntity->GetScene()->GetFuBen()->IsFb())
	{
		// 这里是在副本内请求进入副本，TODO 加提示
		if (m_pEntity->GetType() == enActor)
		{
			((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmInFuben,tstUI);
		}
		return false;
	}
	
	// 副本拷贝源
	CFuBen* static_fb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(nFbid);
	if(!static_fb)
	{
		// 这里找不到该副本的拷贝源，TODO 加提示
		//副本找不到，可能客户端发报机，打印当前用户 
		OutputMsg(rmError,"[WG][FubenSystem] ReqEnterFuben ERROR_USER 错误，副本ID=%d，玩家=%d name=%s!", nFbid, ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName()); 
		return false;
	}
	//OutputMsg(rmTip,"[FubenSystem] ReqEnterFuben ERROR_USER 错误，副本ID=%d，玩家=%d name=%s!", nFbid, ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName());

	// 配置
	COMMONFUBENCONFIG* pFubenConf = static_fb->GetConfig();
	if (!pFubenConf) { return false; }

	// 脚本进入检查
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	paramList << (int)enOnCheckEnter;
	paramList << (int)pFubenConf->nFbConfType;
	paramList << (int)nFbid;
	paramList << (int)pFubenConf->nFbEnterType;
	paramList << m_pEntity;
	if(!globalNpc->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, retList, 1))
	{
		OutputMsg(rmError,"[FubenSystem] OnEvent CheckEnter 错误，副本ID=%d，副本类型=%d !",nFbid,pFubenConf->nFbConfType);
	}
	if(retList.count() >= 0 && (bool)retList[0] == false)
	{
		// 条件不满足
		return false;
	}

	// 获取副本进入点
	INT_PTR nSceneId = pFubenConf->nDefSceneId;
	INT_PTR nPosX=0, nPosY=0;
	if(static_fb->GetScene(nSceneId))
	{
		CScene* pScene = static_fb->GetScene(nSceneId);
		SCENECONFIG* sceneConf = pScene->GetSceneData();
		nPosX = sceneConf->nDefaultX;
		nPosY = sceneConf->nDefaultY;
	}

	// 检查消耗
	if (pFubenConf->ConsumeList[0].type >= 0)
	{
		for (int i = 0; i < 3; i++)
		{	
			GOODS_TAG& consume = pFubenConf->ConsumeList[i];
			if (consume.type >=0)
			{
				if(!(((CActor*)m_pEntity)->CheckConsume(consume.type, consume.id, consume.count))){
					// ((CActor *)m_pEntity)->SendTipmsg("您的金币或道具不足，无法进入副本",tstUI);
					((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmNeedItemNotEnough,tstUI);
					return false;
				}
			}
		}
	}

	CScene* pLastScene = m_pEntity->GetScene();
	CFuBen* pLasfb = m_pEntity->GetFuBen();

	// 进入副本
	switch (pFubenConf->nFbEnterType)
	{
	case 0://单人
	case 2://多人
		{
			CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->CreateFuBen(nFbid);
			if (pFb)
			{
				pFb->Enter(m_pEntity, nSceneId, nPosX, nPosY,0,0,0,0,-1,pLastScene,pLasfb);
			}
			else
			{
				//创建失败 TODO 加提示
				return false;
			}
		}
		break;
	case 1://队伍
		{

		}
		break;
	default:
		break;
	}

	// 扣除消耗
	if (pFubenConf->ConsumeList[0].type >= 0)
	{
		for (int i = 0; i < 3; i++)
		{	
			GOODS_TAG& consume = pFubenConf->ConsumeList[i];
			if (consume.type >=0)
			{
				((CActor*)m_pEntity)->RemoveConsume(consume.type, consume.id, consume.count, -1, -1, -1, 0,GameLog::Log_JoinFuben, pFubenConf->fbName);
			}
		}
	}

	// 进入副本事件
	paramList.clear();
	paramList << (int)nFbid;
	m_pEntity->OnEvent(aeOnEnterFuben,paramList,paramList);
	return true;
}

void CFubenSystem::GetFubenAward(int nFbid)
{
	if (!m_pEntity) return;	
	CScriptValueList paramList;

	if (!m_pEntity->GetScene()) return;
	
	if(!m_pEntity->GetScene()->GetFuBen()->IsFb())
	{
		// 不在副本却请求领取奖励，TODO 加提示
		return;
	}
	
	// 副本拷贝源
	CFuBen* static_fb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(nFbid);
	if(!static_fb)
	{
		// 这里找不到该副本的拷贝源，TODO 加提示
		//副本找不到，可能客户端发报机，打印当前用户 
		OutputMsg(rmError,"[WG][FubenSystem] GetFubenAward ERROR_USER 错误，副本ID=%d，玩家=%d name=%s!", nFbid,  ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName()); 
		return;
	}

	// 配置
	COMMONFUBENCONFIG* pFubenConf = static_fb->GetConfig();
	if (!pFubenConf) { return; }

	// 脚本进入检查
    static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	paramList << (int)enOnGetAward;
	paramList << (int)pFubenConf->nFbConfType;
	paramList << (int)nFbid;
	paramList << (int)pFubenConf->nFbEnterType;
	paramList << this;
	paramList << (int)m_pEntity->GetSceneID();
	paramList << m_pEntity;
	if(!globalNpc->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
	{
		OutputMsg(rmError,"[FubenSystem] OnEvent OnGetAward 错误，副本ID=%d，副本类型=%d !",nFbid,pFubenConf->nFbConfType);
	}
}

//玩家登出
void CFubenSystem::OnActorLoginOut()
{
	// if(!m_pEntity) return;
	// CActor* pActor = (CActor*)m_pEntity;
	// int nTeamId =  pActor->GetProperty<int>(PROP_ACTOR_TEAMFUBEN_TEAMID);
	// unsigned int nActorId =  pActor->GetProperty<int>(PROP_ENTITY_ID);
	// if(nTeamId > 0)
	// {
	//       QuitFubenTeam(nTeamId,nActorId);
	// }
}
