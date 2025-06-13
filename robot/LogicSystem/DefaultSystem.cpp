#include "stdafx.h"
#include "ShareUtil.h"
void CDefaultSystem::OnRecvData(INT_PTR nCmd,CDataPacketReader & recvPack)
{
	switch(nCmd)
	{
	case sHeartbeat: //收到心跳包
		ProcessHeartBeat(recvPack);
		break;
	case sUserLogin: //登陆失败
		ProcessLoginFail(recvPack);
		break;
	case sCreateMainActor: //主角创建		
		ProcessMainActorCreated(recvPack);
		break;
	case sOtherActorAppear: //别的玩家出现在视野 
		ProcessActorAppear(recvPack);
		break;
	case sMonsterAppear: //怪物实体出现在视野
		ProcessMonsterAppear(recvPack);
		break;
	case sNPCAppear: //NPC实体出现在视野
		ProcessNPCAppear(recvPack);
		break;
	case sPetAppear: //宠物实体出现在视野
		ProcessPetAppear(recvPack);
		break;
	case sEntityAppear: //其他实体出现在视野（传送门）
		ProcessOtherAppear(recvPack);
		break;
	case sEntityDisappear: //实体从视野消失
		ProcessEntityDisappear(recvPack);
		break;
	case sMainactorPropertyChange: //自己属性变更
		ProcessMainActorPropChange(recvPack);
		break;
	case sEntityPropertyChange: //其他实体属性变更
		ProcessOtherEntityPropChange(recvPack);
		break;
	case sEnterScene: //主角传送或者切换场景
		ProcessPlayerEnterScene(recvPack);
		break;
	case sOperatorResult: //公共操作结果
		ProcessCommonOpResult(recvPack);
		break;
	case sOtherEntityMove: //实体走/跑
	case sOtherEntityRun:
		ProcessOtherEntityMove(recvPack);	
		break;
	case sResetMainActorPos: //重设主角坐标
		ProcessResetMainActorPos(recvPack);
		break;
	case sEntityDeath: //实体死亡
		ProcessEntityDeath(recvPack);
		break;
	case sInstatnceMove://瞬间移动
		ProcessEntityInstatnceMove(recvPack);
		break;
	case sActorTransport://传送
		ProcessEntityTransport(recvPack);
		break;
	}
}

void CDefaultSystem::OnTimeRun(TICKCOUNT currentTick )
{
	
}

void CDefaultSystem::ProcessHeartBeat(CDataPacketReader & recvPack)
{
	//收到心跳包以后马上发消息告诉服务器
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) GetSystemId() << (BYTE)cHeartbeat;
	pack << (unsigned int)_getTickCount(); 
	m_pClient->FlushProtoPacket(pack); 
}
void CDefaultSystem::ProcessLoginFail(CDataPacketReader & recvPack)
{
	BYTE nErrorCode = 0;
	recvPack >> nErrorCode;
	if(nErrorCode)
	{
		OutputMsg(rmNormal, "[%s] enter game fail, errorcode=%d ",m_pClient->GetAccount().name,int(nErrorCode));
	}
}
void CDefaultSystem::ProcessMainActorCreated(CDataPacketReader & recvPack)
{	
	WORD nPropSize = 0;
	EntityHandle handle;
	char szName[256];
	CClientActor& data = m_pClient->GetActorData();	
	recvPack >> handle;	
	recvPack >> nPropSize;
	Assert(nPropSize == data.GetPropertySize());
	recvPack.readBuf((char *)data.GetPropertyDataPtr(), nPropSize);	
	size_t nNameBuffLen = ArrayCount(szName)-1;	
	recvPack.readString(szName, nNameBuffLen);
	szName[nNameBuffLen] = '\0';
	data.SetHandle(handle);
	data.SetName(szName);
	OutputMsg(rmNormal, "[%s][%s] 主角创建 ", m_pClient->GetAccount().name, m_pClient->GetActorName());
	m_pClient->GetLogicAgent().OnEnterGame();
	char* sCnName = m_pClient->GetActorName();
	m_pClient->SetCnName(sCnName);
}
void CDefaultSystem::ProcessActorAppear(CDataPacketReader &packet)
{
	OtherActorPropData data;
	packet >> data.handle;
	packet.readString(data.szDecorateName, ArrayCount(data.szDecorateName)-1);
	size_t offset = offsetof(OtherActorPropData, wPosX);
	packet.readBuf( (char *)&data + offset, sizeof(OtherActorPropData) - offset);
	CClientActor ca;
	ca.SetHandle(data.handle);
	char * pOver = strchr(data.szDecorateName, '\\');
	if (pOver != NULL)
	{
		*pOver = 0;
	}
	ca.SetName(data.szDecorateName);
	ca.SetProperty<int>(PROP_ENTITY_POSX,					data.wPosX);
	ca.SetProperty<int>(PROP_ENTITY_POSY,					data.wPosY);
	ca.SetProperty<int>(PROP_ENTITY_MODELID,				data.nModelId);
	ca.SetProperty<int>(PROP_CREATURE_HP,					data.nHP);
	ca.SetProperty<int>(PROP_CREATURE_MP,					data.nMP);
	ca.SetProperty<int>(PROP_CREATURE_MAXHP,				data.nMaxHP);
	ca.SetProperty<int>(PROP_CREATURE_MAXMP,				data.nMaxMP);
	ca.SetProperty<int>(PROP_CREATURE_MOVEONESLOTTIME,		data.wMoveSpeed);
	ca.SetProperty<int>(PROP_ACTOR_SEX,						data.ucSex);
	ca.SetProperty<int>(PROP_ACTOR_VOCATION,				data.ucVocation);
	ca.SetProperty<int>(PROP_CREATURE_LEVEL,				data.ucLevel);
	ca.SetProperty<int>(PROP_ACTOR_CIRCLE,					data.nCircle);
	ca.SetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE,		data.nWeaponAppear);
	ca.SetProperty<int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE,	data.nSwingAppear);	
	ca.SetProperty<int>(PROP_ACTOR_SOCIALMASK,	data.nSocialMask);	
	ca.SetProperty<int>(PROP_ENTITY_ICON,	data.wIcon);	
	ca.SetProperty<int>(PROP_CREATURE_ATTACK_SPEED,	data.nAttackSpeed);	
	ca.SetProperty<int>(PROP_ENTITY_DIR,	data.ucDir);	
	ca.SetProperty<int>(PROP_CREATURE_STATE,	data.nStatus);	
	ca.SetProperty<int>(PROP_ACTOR_TEAM_ID,	data.nTeamId);	
	ca.SetProperty<int>(PROP_ACTOR_ZY,	data.ucCamp);	
	ca.SetProperty<int>(PROP_ACTOR_CURNEWTITLE,	data.nHeadTitle);	
	ca.SetProperty<int>(PROP_ACTOR_SUPPER_PLAY_LVL,	data.nSupLevel);	
	ca.SetProperty<int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE,	data.nSoldierSoulAppear);	
	ca.SetProperty<int>(PROP_ACTOR_WEAPON_ID,	data.nWeaponId);	
	ca.SetProperty<int>(PROP_ACTOR_GUILD_ID,	data.nGuildID);	
	ca.SetProperty<int>(PROP_ACTOR_MONSTER_MODELID,	data.nMonsterID);	
	ca.SetProperty<int>(PROP_ACTOR_MERITORIOUSPOINT,	data.nMeritoriousPoint);	
	ca.SetProperty<int>(PROP_ACTOR_NEXT_SKILL_FLAG,	data.nNextSkillFlag);	
	ca.SetProperty<int>(PROP_ACTOR_EVIL_PK_STATUS,	data.nEvilPkStatus);	
	ca.SetProperty<int>(PROP_ACTOR_PK_VALUE,	data.nPkValue);	

	m_OtherActor[ca.GetHandle()] = ca;
}
void CDefaultSystem::ProcessMonsterAppear(CDataPacketReader &packet)
{
	MonsterPropData data;
	BYTE nType;
	packet >> nType;
	packet >> data.handle;
	packet.readString(data.szDecorateName, ArrayCount(data.szDecorateName)-1);

	size_t offset = offsetof(MonsterPropData, wPosX);
	packet.readBuf( (char *)&data + offset, sizeof(MonsterPropData) - offset);

	//读buff
	BYTE ncount = packet.readAtom<BYTE>();
	for (size_t i = 0; i < ncount; i++)
	{
		packet.readAtom<WORD>();
		packet.readAtom<int>();
		packet.readAtom<BYTE>();
		packet.readAtom<GAMEATTRVALUE>();
	}

	//怪物类型
	BYTE nMonsterType = packet.readAtom<BYTE>();

	CClientMonster ca;
	ca.SetHandle(data.handle);
	char * pOver = strchr(data.szDecorateName, '\\');
	if (pOver != NULL)
	{
		*pOver = 0;
	}
	ca.SetName(data.szDecorateName);
	ca.SetProperty<int>(PROP_ENTITY_POSX,					data.wPosX);
	ca.SetProperty<int>(PROP_ENTITY_POSY,					data.wPosY);
	ca.SetProperty<int>(PROP_ENTITY_MODELID,				data.nModelId);
	ca.SetProperty<int>(PROP_ENTITY_DIR,					data.ucDir);
	ca.SetProperty<int>(PROP_CREATURE_LEVEL,				data.ucLevel);
	ca.SetProperty<int>(PROP_CREATURE_HP,					data.nHP);
	ca.SetProperty<int>(PROP_CREATURE_MP,					data.nMP);
	ca.SetProperty<int>(PROP_CREATURE_MAXHP,				data.nMaxHP);
	ca.SetProperty<int>(PROP_CREATURE_MAXMP,				data.nMaxMP);
	ca.SetProperty<int>(PROP_CREATURE_MOVEONESLOTTIME,		data.wMoveSpeed);
	ca.SetProperty<int>(PROP_CREATURE_ATTACK_SPEED,			data.wAttkSpeed);
	ca.SetProperty<int>(PROP_CREATURE_STATE,				data.nStatus);
	ca.SetProperty<int>(PROP_ENTITY_ID,						data.wMonsterId);
	ca.SetProperty<int>(PROP_MONSTER_LIVE_TIMEOUT,			data.nLiveTimeOut);
	ca.SetMonsterType(nMonsterType);
	m_Monster[ca.GetHandle()] = ca;

	CClientEntity* pEntity = &m_Monster[ca.GetHandle()];
	int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	AddMap(pEntity, x,y);
}
void CDefaultSystem::ProcessNPCAppear(CDataPacketReader &packet)
{
	char szDecorateName[256];
	Uint64 handle;
	packet >> handle;
	packet.readString(szDecorateName, ArrayCount(szDecorateName)-1);
	WORD nEntityId = packet.readAtom<WORD>();
	WORD nPosX = packet.readAtom<WORD>();
	WORD nPosY = packet.readAtom<WORD>();
	unsigned nModleId = packet.readAtom<unsigned>();
	BYTE nDir = packet.readAtom<BYTE>();
	
	CClientNPC ca;
	ca.SetHandle(handle);
	char * pOver = strchr(szDecorateName, '\\');
	if (pOver != NULL)
	{
		*pOver = 0;
	}
	ca.SetName(szDecorateName);
	ca.SetProperty<int>(PROP_ENTITY_ID,						nEntityId);
	ca.SetProperty<int>(PROP_ENTITY_POSX,					nPosX);
	ca.SetProperty<int>(PROP_ENTITY_POSY,					nPosY);
	ca.SetProperty<int>(PROP_ENTITY_MODELID,				nModleId);
	ca.SetProperty<int>(PROP_ENTITY_DIR,					nDir);

	m_Npc[ca.GetHandle()] = ca;

	CClientEntity* pEntity = &m_Npc[ca.GetHandle()];
	int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	AddMap(pEntity, x,y);
}
void CDefaultSystem::ProcessPetAppear(CDataPacketReader &packet)
{
	PetPropData data;
	BYTE nType;
	packet >> nType;
	packet >> data.handle;
	packet.readString(data.szDecorateName, ArrayCount(data.szDecorateName)-1);

	size_t offset = offsetof(PetPropData, wPosX);
	packet.readBuf( (char *)&data + offset, sizeof(PetPropData) - offset);
	CClientPet ca;
	ca.SetHandle(data.handle);
	char * pOver = strchr(data.szDecorateName, '\\');
	if (pOver != NULL)
	{
		*pOver = 0;
	}
	ca.SetName(data.szDecorateName);
	ca.SetProperty<int>(PROP_ENTITY_POSX,					data.wPosX);
	ca.SetProperty<int>(PROP_ENTITY_POSY,					data.wPosY);
	ca.SetProperty<int>(PROP_ENTITY_MODELID,				data.nModelId);
	ca.SetProperty<int>(PROP_ENTITY_DIR,					data.ucDir);
	ca.SetProperty<int>(PROP_CREATURE_LEVEL,				data.ucLevel);
	ca.SetProperty<int>(PROP_CREATURE_HP,					data.nHP);
	ca.SetProperty<int>(PROP_CREATURE_MP,					data.nMP);
	ca.SetProperty<int>(PROP_CREATURE_MAXHP,				data.nMaxHP);
	ca.SetProperty<int>(PROP_CREATURE_MAXMP,				data.nMaxMP);
	ca.SetProperty<int>(PROP_CREATURE_MOVEONESLOTTIME,		data.wMoveSpeed);
	ca.SetProperty<int>(PROP_CREATURE_ATTACK_SPEED,			data.wAttkSpeed);
	ca.SetProperty<int>(PROP_CREATURE_STATE,				data.nStatus);
	ca.SetProperty<int>(PROP_ENTITY_ID,						data.wEntityId);

	m_Pets[ca.GetHandle()] = ca;

	CClientEntity* pEntity = &m_Pets[ca.GetHandle()];
	int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	AddMap(pEntity, x,y);
}
void CDefaultSystem::ProcessOtherAppear(CDataPacketReader &packet)
{
	char szDecorateName[256];
	BYTE ntype;
	packet >> ntype;
	Uint64 handle;
	packet >> handle;
	packet.readString(szDecorateName, ArrayCount(szDecorateName)-1);
	WORD nPosX = packet.readAtom<WORD>();
	WORD nPosY = packet.readAtom<WORD>();
	unsigned nModleId = packet.readAtom<unsigned>();
	BYTE nDir = packet.readAtom<BYTE>();

	if (EntityHandle(handle).GetType() == enTransfer)
	{
		CClientTransfer ca;
		ca.SetHandle(handle);
		char * pOver = strchr(szDecorateName, '\\');
		if (pOver != NULL)
		{
			*pOver = 0;
		}
		ca.SetName(szDecorateName);
		ca.SetProperty<int>(PROP_ENTITY_POSX,					nPosX);
		ca.SetProperty<int>(PROP_ENTITY_POSY,					nPosY);
		ca.SetProperty<int>(PROP_ENTITY_MODELID,				nModleId);
		ca.SetProperty<int>(PROP_ENTITY_DIR,					nDir);

		m_Transfer[ca.GetHandle()] = ca;

		CClientEntity* pEntity = &m_Transfer[ca.GetHandle()];
		int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		AddMap(pEntity, x,y);
	}
}
void CDefaultSystem::ProcessEntityDisappear(CDataPacketReader &packet)
{
	EntityHandle handle;
	packet >> handle;	
	INT_PTR nEntityType = handle.GetType();
	if (enActor == nEntityType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			CClientEntity* pEntity = &m_OtherActor[handle];
			int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			DelMap(pEntity, x,y);
			m_OtherActor.erase(handle);
		}
	}
	else if (enMonster == nEntityType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			CClientEntity* pEntity = &m_Monster[handle];
			int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			DelMap(pEntity, x,y);
			m_Monster.erase(handle);
		}
	}
	else if (enPet == nEntityType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			CClientEntity* pEntity = &m_Pets[handle];
			int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			DelMap(pEntity, x,y);
			m_Pets.erase(handle);
		}
	}
	else if (enNpc == nEntityType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			CClientEntity* pEntity = &m_Npc[handle];
			int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			DelMap(pEntity, x,y);
			m_Npc.erase(handle);
		}
	}
	else if (enTransfer == nEntityType)
	{
		if (m_Transfer.find(handle) != m_Transfer.end())
		{
			CClientEntity* pEntity = &m_Transfer[handle];
			int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			DelMap(pEntity, x,y);
			m_Transfer.erase(handle);
		}
	}
}
void CDefaultSystem::ProcessMainActorPropChange(CDataPacketReader &packet)
{
	/*OutputMsg(rmNormal, _T("MainActor Prop Changed"));*/
	CClientActor& ca = m_pClient->GetActorData();
	UpdateEntityPropImpl(&ca, packet);
}
void CDefaultSystem::ProcessOtherEntityPropChange(CDataPacketReader &packet)
{
	//OutputMsg(rmNormal, _T("Other Entity Prop Changed"));
	EntityHandle handle;
	packet >> handle;
	INT_PTR nEntityType = handle.GetType();
	CClientEntity* pEntity = NULL;
	if (enMonster == nEntityType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			pEntity = &m_Monster[handle];
		}
	}
	else if (enActor == nEntityType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			pEntity = &m_OtherActor[handle];
		}
	}
	else if (enPet == nEntityType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			pEntity = &m_Pets[handle];
		}
	}
	else if (enNpc == nEntityType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			pEntity = &m_Npc[handle];
		}
	}
	else if (enTransfer == nEntityType)
	{
		if (m_Transfer.find(handle) != m_Transfer.end())
		{
			pEntity = &m_Transfer[handle];
		}
	}

	if (pEntity)
	{
		UpdateEntityPropImpl(pEntity, packet);
	}
}
void CDefaultSystem::UpdateEntityPropImpl(CClientEntity* pEntity, CDataPacketReader& packet)
{
	BYTE ucCount = 0, ucPropId = 0;
	int nPropVal = 0;
	packet >> ucCount;
	while (ucCount-- > 0)
	{
		packet >> ucPropId >> nPropVal;
		pEntity->SetProperty<int>(ucPropId, nPropVal);
	}
}
void CDefaultSystem::ProcessPlayerEnterScene(CDataPacketReader& packet)
{
	char szBuff[128];
	CClientActor& ca = m_pClient->GetActorData();
	packet.readString(szBuff, ArrayCount(szBuff));
	ca.SetSceneName(szBuff);
	packet.readString(szBuff, ArrayCount(szBuff));
	ca.SetMapName(szBuff);
	WORD wSceneId, wPosX, wPosY;
	WORD nFubenId = 0;
	unsigned int nAreaAttr[2];
	packet >> wSceneId >> nFubenId >> wPosX >> wPosY >> nAreaAttr[0] >> nAreaAttr[1];
	ca.SetSceneId(wSceneId);
	ca.SetFubenId(nFubenId);
	ca.SetProperty<int>(PROP_ENTITY_POSX, wPosX);
	ca.SetProperty<int>(PROP_ENTITY_POSY, wPosY);
	
	SCENEMAPCONF * pSc = GetRobotMgr()->GetGlobalConfig().GetSceneConfig().GetSceneConfig(wSceneId);
	if (pSc)
		m_nMapWidth = pSc->m_MapData.m_dwWidth;
	else
	{
		m_nMapWidth = 1000;
		OutputMsg(rmError, "进入场景，场景id错误：%d", wSceneId);
	}
	
	// 进入场景需要把原来的附近实体列表清除
	m_OtherActor.clear();
	m_Monster.clear();
	m_Pets.clear();
	m_Npc.clear();
	m_Transfer.clear();
	m_PosiEntityMap.clear();

	//初始化A星
	m_pClient->GetLogicAgent().GetAStar().initFromMap(&pSc->m_MapData);
}
void CDefaultSystem::ProcessCommonOpResult(CDataPacketReader& packet)
{
	bool bResult = false, bHasAck = false;
	packet >> bResult >> bHasAck;
	if (bHasAck)
	{
		unsigned int nTick;
		packet >> nTick >> nTick >> nTick;
	}
	int nX, nY;
	packet >> nX >> nY;

	m_pClient->GetLogicAgent().clearActionState();
	//m_pClient->GetLogicAgent().OnEvent(enAction_CommonOpResult, bResult);
}
void CDefaultSystem::ProcessOtherEntityMove(CDataPacketReader& packet)
{
	EntityHandle handle;
	WORD wPosX, wPosY;
	BYTE nDir;
	packet >> handle;
	packet >> wPosX >> wPosY >> nDir;
	CClientEntity* pEntity = NULL;
	INT_PTR nType = handle.GetType();
	Assert(!handle.IsNull());
	if (enMonster == nType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			pEntity = &m_Monster[handle];
		}
	}
	else if (enActor == nType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			pEntity = &m_OtherActor[handle];
		}
	}
	else if (enPet == nType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			pEntity = &m_Pets[handle];
		}
	}
	else if (enNpc == nType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			pEntity = &m_Npc[handle];
		}
	}
	else if (enTransfer == nType)
	{
		if (m_Transfer.find(handle) != m_Transfer.end())
		{
			pEntity = &m_Transfer[handle];
		}
	}
	
	if (pEntity)
	{
		int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		ChgMap(pEntity, x,y, wPosX,wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_POSX, wPosX);
		pEntity->SetProperty<int>(PROP_ENTITY_POSY, wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_DIR,	nDir);
	}
}
void CDefaultSystem::ProcessResetMainActorPos(CDataPacketReader& packet)
{
	WORD nX, nY;
	packet >> nX >> nY;
	SetPosition(nX, nY);
	m_pClient->GetLogicAgent().SetMovePoint(0,0);
	m_pClient->GetLogicAgent().ClearMovePath();
}
void CDefaultSystem::ProcessEntityDeath(CDataPacketReader& packet)
{
	EntityHandle handle;
	packet >> handle;
	INT_PTR nEntityType = handle.GetType();
	CClientEntity* pEntity = NULL;
	if (enMonster == nEntityType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			pEntity = &m_Monster[handle];
		}
	}
	else if (enActor == nEntityType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			pEntity = &m_OtherActor[handle];
		}
	}
	else if (enPet == nEntityType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			pEntity = &m_Pets[handle];
		}
	}
	else if (enNpc == nEntityType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			pEntity = &m_Npc[handle];
		}
	}

	if (pEntity)
	{
		pEntity->SetStateDeath();
	}
}
void CDefaultSystem::ProcessEntityInstatnceMove(CDataPacketReader& packet)
{
	EntityHandle handle;
	WORD wPosX, wPosY;
	BYTE nDir;
	packet >> handle;
	packet >> wPosX >> wPosY >> nDir;
	CClientEntity* pEntity = NULL;
	INT_PTR nType = handle.GetType();
	Assert(!handle.IsNull());
	if (enMonster == nType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			pEntity = &m_Monster[handle];
		}
	}
	else if (enActor == nType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			pEntity = &m_OtherActor[handle];
		}
	}
	else if (enPet == nType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			pEntity = &m_Pets[handle];
		}
	}
	else if (enNpc == nType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			pEntity = &m_Npc[handle];
		}
	}
	else if (enTransfer == nType)
	{
		if (m_Transfer.find(handle) != m_Transfer.end())
		{
			pEntity = &m_Transfer[handle];
		}
	}

	if (m_pClient->GetActorData().GetHandle()==handle)
	{
		pEntity = &m_pClient->GetActorData();
	}
	
	if (pEntity)
	{
		int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		ChgMap(pEntity, x,y, wPosX,wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_POSX, wPosX);
		pEntity->SetProperty<int>(PROP_ENTITY_POSY, wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_DIR,	nDir);
	}
}
void CDefaultSystem::ProcessEntityTransport(CDataPacketReader& packet)
{
	EntityHandle handle;
	WORD wPosX, wPosY;
	BYTE nDir;
	packet >> handle;
	packet >> wPosX >> wPosY >> nDir;
	CClientEntity* pEntity = NULL;
	INT_PTR nType = handle.GetType();
	Assert(!handle.IsNull());
	if (enMonster == nType)
	{
		if (m_Monster.find(handle) != m_Monster.end())
		{
			pEntity = &m_Monster[handle];
		}
	}
	else if (enActor == nType)
	{
		if (m_OtherActor.find(handle) != m_OtherActor.end())
		{
			pEntity = &m_OtherActor[handle];
		}
	}
	else if (enPet == nType)
	{
		if (m_Pets.find(handle) != m_Pets.end())
		{
			pEntity = &m_Pets[handle];
		}
	}
	else if (enNpc == nType)
	{
		if (m_Npc.find(handle) != m_Npc.end())
		{
			pEntity = &m_Npc[handle];
		}
	}
	else if (enTransfer == nType)
	{
		if (m_Transfer.find(handle) != m_Transfer.end())
		{
			pEntity = &m_Transfer[handle];
		}
	}
	
	if (m_pClient->GetActorData().GetHandle()==handle)
	{
		pEntity = &m_pClient->GetActorData();
	}
	
	if (pEntity)
	{
		int x = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		int y = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		ChgMap(pEntity, x,y, wPosX,wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_POSX, wPosX);
		pEntity->SetProperty<int>(PROP_ENTITY_POSY, wPosY);
		pEntity->SetProperty<int>(PROP_ENTITY_DIR,	nDir);
	}
}

void CDefaultSystem::GetPosition(int &x, int &y) const 
{	
	CClientActor& ca = m_pClient->GetActorData();
	x = ca.GetProperty<int>(PROP_ENTITY_POSX);
	y = ca.GetProperty<int>(PROP_ENTITY_POSY);
}
bool CDefaultSystem::GetOtherPos(EntityHandle handle, int& nX, int& nY)
{
	CClientActor* pOther = GetOtherData(handle);
	if (!pOther)
		return false;

	nX = pOther->GetProperty<int>(PROP_ENTITY_POSX);
	nY = pOther->GetProperty<int>(PROP_ENTITY_POSY);
	return true;
}
bool CDefaultSystem::GetMonsterPos(EntityHandle handle, int& nX, int& nY)
{	
	CClientMonster* pMonster = GetMonster(handle);
	if (!pMonster)
		return false;

	nX = pMonster->GetProperty<int>(PROP_ENTITY_POSX);
	nY = pMonster->GetProperty<int>(PROP_ENTITY_POSY);
	return true;
}
bool CDefaultSystem::GetPetPos(EntityHandle handle, int& nX, int& nY)
{
	CClientPet* pPet = GetPet(handle);
	if (!pPet)
		return false;

	nX = pPet->GetProperty<int>(PROP_ENTITY_POSX);
	nY = pPet->GetProperty<int>(PROP_ENTITY_POSY);
	return true;
}
bool CDefaultSystem::GetEntityPos(EntityHandle handle, int& nX, int& nY)
{
	int type = handle.GetType();
	switch(type)
	{
	case enActor:
		return GetOtherPos(handle,nX,nY);
	break;		
	case enMonster:
		return GetMonsterPos(handle,nX,nY);
	break;		
	case enPet:
		return GetPetPos(handle,nX,nY);
	break;					
	}
	return false;
}

void CDefaultSystem::Move(int nX, int nY, int nStep, int nDir)
{
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	int nCmd = nStep > 1 ? cRun : cMove;	
	pack << (BYTE)enMoveSystemID << (BYTE)nCmd;
	pack << (WORD)nX << (WORD)nY << nDir << (unsigned int)_getTickCount();
	INT_PTR nDestX = nX, nDestY = nY;
	CUtility::NewPosition(nDestX, nDestY, nDir, nStep);
	SetPosition((WORD)nDestX, (WORD)nDestY);
	m_pClient->FlushProtoPacket(pack);
}
void CDefaultSystem::MoveTo(int nEndX, int nEndY, INT_PTR nStep)
{
	int nStartX, nStartY;
	m_pClient->GetLogicAgent().GetDefaultSystem().GetPosition(nStartX,nStartY);
	INT_PTR nDir = DIR_STOP;
	CalculateMoveParam(nStartX, nStartY, nEndX, nEndY, nDir, nStep);
	Move(nStartX, nStartY,  (int)nStep,  (int)nDir);
}
void CDefaultSystem::ReqRelive()
{
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	pack << (BYTE)enDefaultEntitySystemID << (BYTE)cChooseRelive;
	pack << (BYTE)1 << (BYTE)0;
	m_pClient->FlushProtoPacket(pack);

}
int CDefaultSystem::GetDistance(int nPosX,int nPosy)
{
	int nCurPosx,nCurPoyY;
	GetPosition(nCurPosx,nCurPoyY);
	nCurPosx -= nPosX;
	nCurPoyY -= nPosy;
	double nDis = nCurPosx*nCurPosx + nCurPoyY * nCurPoyY;
	return (int)(sqrt (nDis));
}
unsigned int CDefaultSystem::GetMaxDistance(int nPosX,int nPosy)
{
	int nCurPosx,nCurPoyY;
	GetPosition(nCurPosx,nCurPoyY);
	nCurPosx -= nPosX;
	nCurPoyY -= nPosy;
	return std::max(std::abs(nCurPosx),std::abs(nCurPoyY));
}

//在一个扇形区域里找一个能运动到的点，可以是1步到，也可以是2步到,优先2步到
bool CDefaultSystem::GetFanAvailablePos(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR &nStep)
{
	int nSceneId = m_pClient->GetActorData().GetSceneId();
	INT_PTR nNewPosX,nNewPosY,nNewDir,nMaxStep =nStep;
	if(nMaxStep ==0) nMaxStep =1;

	if(nDir == DIR_STOP)
	{
		nDir = (++ nDir )% DIR_STOP;
	}
	INT_PTR nEntityType = enActor;
	bool bCanEnterFire = true;
	INT_PTR nFinalStep =-1; //最终的移动的步伐，如果找到了一个，就返回
	CSceneConfigLoader& scl = GetRobotMgr()->GetGlobalConfig().GetSceneConfig();
	for(INT_PTR i=1 ;i <= nMaxStep; i++)
	{
		//先优先在这个方向找
		nNewPosX = nCurrentPosX;
		nNewPosY=  nCurrentPosY;
		if(nDir == DIR_STOP) continue;
		CUtility::NewPosition(nNewPosX, nNewPosY, nDir, i);//再取一次坐标

		if( scl.CanMove(nSceneId,(const int)nNewPosX,(const int)nNewPosY) ) //如果1格不能移动的话，那么2格也不需要尝试了，中间有间隔
		{
			nFinalStep =i;
		}
		else
		{
			break;
		}
	}
	
	//如果在该方向找到了，就去寻找
	if(nFinalStep >=1)
	{
		nStep = nFinalStep;
		return true;
	}

	//扇形查找的话，找到1格就够
	for(INT_PTR i=1; i<= nMaxStep; i++)
	{
		for(INT_PTR j=1; j <4; j++)
		{
			for(INT_PTR k=-1;k<=1; k++)
			{
				if(k ==0 || (j==4 && k==1)) continue; //在扇形的区间查找
				nNewPosX = nCurrentPosX;
				nNewPosY=  nCurrentPosY;
				nNewDir = (nDir + j*k +DIR_STOP)%DIR_STOP; //新的方向
				if(nNewDir ==DIR_STOP) continue;

				CUtility::NewPosition(nNewPosX, nNewPosY, nNewDir, i);//再取一次坐标
				if(scl.CanMove(nSceneId,(const int)nNewPosX,(const int)nNewPosY) )
				{
					nDir = nNewDir;
					nStep = i;
					return true;
				}
			}
		}
	}
	
	return false;
}
bool CDefaultSystem::CalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,
	INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep,INT_PTR nMinDisSquare,INT_PTR nMaxDisSquare)
{
	
	INT_PTR nDisSquare = GetDisSqare(nStartX,nStartY, nEndX,nEndY);
	
	 //距离已经到达了这么个间隔，不需要再移动了
	if(nDisSquare <= nMinDisSquare ) return false;

	//超过了最大的距离就不再移动了
	if( nMaxDisSquare && nDisSquare >= nMaxDisSquare )
	{
		return false; 
	}
	//已经运动到了终点
	if(nDisSquare <=0)
	{
		return false; //停止移动
	}
	else
	{
		
		//在一个扇形区域里找一个能运动到的点，可以是1步到，也可以是2步到,优先2步到
	     nDir = m_pClient->GetActorData().GetDir(nStartX, nStartY,nEndX,nEndY);
		return GetFanAvailablePos(nStartX,nStartY, nDir,nStep);

		
	}
	

	return false;
}

void CDefaultSystem::SetPosition(const int nX, const int nY)
{
	CClientActor& ca = m_pClient->GetActorData();
	ca.SetProperty<int>(PROP_ENTITY_POSX, nX);
	ca.SetProperty<int>(PROP_ENTITY_POSY, nY);
}

void CDefaultSystem::AddMap(CClientEntity* pEntity, int nPosX, int nPosY)
{
	int idx = nPosX + nPosY*m_nMapWidth;
	m_PosiEntityMap[idx].push_back(pEntity);
}
void CDefaultSystem::DelMap(CClientEntity* pEntity, int nPosX, int nPosY)
{
	int idx = nPosX + nPosY*m_nMapWidth;
	auto it = m_PosiEntityMap.find(idx);
	if (it != m_PosiEntityMap.end())
	{
		auto tarit = it->second.begin();
		for (; tarit != it->second.end(); ++tarit)
		{
			if (*tarit == pEntity)
			{
				it->second.erase(tarit);
				return;
			}
		}
	}
}
void CDefaultSystem::ChgMap(CClientEntity* pEntity, int nOldPosX, int nOldPosY, int nPosX, int nPosY)
{
	DelMap(pEntity,nOldPosX,nOldPosY);
	AddMap(pEntity,nPosX,nPosY);
}
CClientMonster* CDefaultSystem::GetMonster(bool include_guard)
{
	typedef typename std::map<EntityHandle,CClientMonster>::iterator IterType;
	IterType it = m_Monster.begin();
	for (; it != m_Monster.end(); it++)
	{
		if (include_guard)
			return &(it->second);
		else if (!it->second.IsGuard())
			return &(it->second);
	}
	return NULL;
}
CClientMonster* CDefaultSystem::GetMonsterNot(EntityHandle handle,bool include_guard)
{
	std::map<EntityHandle, CClientMonster>::iterator it = m_Monster.begin();
	for (; it != m_Monster.end(); ++it)
	{
		if (it->first != handle)
		{
			if (include_guard)
				return &(it->second);
			else if (!it->second.IsGuard())
				return &(it->second);
		}
	}
	return NULL;
}
CClientMonster* CDefaultSystem::GetNearestMonster(int dis,bool include_guard)
{
	if (m_Monster.size() > 0)
	{
		int nPosX,nPosY;
		GetPosition(nPosX,nPosY);
		for (int i = 1; i < dis; i++)
		{
			//上
			{
				int y = nPosY - i;
				for (int x = nPosX - i; x < nPosX + i; x++)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						return (CClientMonster*)pEntity;
				}
			}
			//右
			{
				int x = nPosX + i;
				for (int y = nPosY - i; y < nPosY + i; y++)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						return (CClientMonster*)pEntity;
				}
			}
			//下
			{
				int y = nPosY + i;
				for (int x = nPosX + i; x > nPosX - i; x--)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y, include_guard))
						return (CClientMonster*)pEntity;
				}
			}
			//左
			{
				int x = nPosX - i;
				for (int y = nPosY + i; y > nPosY - i; y--)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						return (CClientMonster*)pEntity;
				}
			}
		}
	}
	return NULL;
}
CClientMonster* CDefaultSystem::GetNearestMonsterNot(EntityHandle handle, int dis = 14,bool include_guard = false)
{
	if (m_Monster.size() > 0)
	{
		int nPosX,nPosY;
		GetPosition(nPosX,nPosY);
		for (int i = 1; i < dis; i++)
		{
			//上
			{
				int y = nPosY - i;
				for (int x = nPosX - i; x < nPosX + i; x++)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						if (pEntity->GetHandle() != handle)
							return (CClientMonster*)pEntity;
				}
			}
			//右
			{
				int x = nPosX + i;
				for (int y = nPosY - i; y < nPosY + i; y++)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						if (pEntity->GetHandle() != handle)
							return (CClientMonster*)pEntity;
				}
			}
			//下
			{
				int y = nPosY + i;
				for (int x = nPosX + i; x > nPosX - i; x--)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y, include_guard))
						if (pEntity->GetHandle() != handle)
							return (CClientMonster*)pEntity;
				}
			}
			//左
			{
				int x = nPosX - i;
				for (int y = nPosY + i; y > nPosY - i; y--)
				{
					if (CClientEntity* pEntity = GetEntity(enMonster, x,y))
						if (pEntity->GetHandle() != handle)
							return (CClientMonster*)pEntity;
				}
			}
		}
	}
	return NULL;
}
bool CDefaultSystem::HasEntity(unsigned int  nEntityType, int nPosX, int nPosY)
{
	int idx = nPosX + nPosY*m_nMapWidth;
	auto it = m_PosiEntityMap.find(idx);
	if (it != m_PosiEntityMap.end())
	{
		auto tarit = it->second.begin();
		for (; tarit != it->second.end(); ++tarit)
		{
			if ((*tarit)->GetHandle().GetType() == nEntityType)
			{
				return true;
			}
		}
	}
	return false;
}
bool CDefaultSystem::HasEntity(int nPosX, int nPosY)
{
	int idx = nPosX + nPosY*m_nMapWidth;
	auto it = m_PosiEntityMap.find(idx);
	if (it != m_PosiEntityMap.end())
	{
		if (it->second.size() > 0)
		{
			return true;
		}
	}
	return false;
}
CClientEntity* CDefaultSystem::GetEntity(unsigned int nEntityType, int nPosX, int nPosY,bool include_guard = false)
{
	int idx = nPosX + nPosY*m_nMapWidth;
	auto it = m_PosiEntityMap.find(idx);
	if (it != m_PosiEntityMap.end())
	{
		auto tarit = it->second.begin();
		for (; tarit != it->second.end(); ++tarit)
		{
			if ((*tarit)->GetHandle().GetType() == nEntityType)
			{
				switch (nEntityType)
				{
				case enActor:
					if (m_OtherActor.find((*tarit)->GetHandle()) != m_OtherActor.end())
						return &m_OtherActor[(*tarit)->GetHandle()];
					break;

				case enMonster:
					if (m_Monster.find((*tarit)->GetHandle()) != m_Monster.end())
					{
						if (include_guard) return &m_Monster[(*tarit)->GetHandle()];
						else if (!m_Monster[(*tarit)->GetHandle()].IsGuard()) return &m_Monster[(*tarit)->GetHandle()];
					}
					break;

				case enNpc:
					if (m_Npc.find((*tarit)->GetHandle()) != m_Npc.end())
						return &m_Npc[(*tarit)->GetHandle()];
					break;

				case enPet:
					if (m_Pets.find((*tarit)->GetHandle()) != m_Pets.end())
						return &m_Pets[(*tarit)->GetHandle()];
					break;

				case enTransfer:
					if (m_Transfer.find((*tarit)->GetHandle()) != m_Transfer.end())
						return &m_Transfer[(*tarit)->GetHandle()];
					break;
				}
			}
		}
	}
	return NULL;
}