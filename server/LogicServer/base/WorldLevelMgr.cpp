#include "StdAfx.h"
#include "WorldLevelMgr.h"
using namespace jxInterSrvComm::DbServerProto;

CWorldLevelMgr::CWorldLevelMgr(void)
{
}


CWorldLevelMgr::~CWorldLevelMgr(void)
{

}
bool  CWorldLevelMgr::Initialize()
{
	if (GetLogicServer()->IsCommonServer())
	{
		return true;
	}
	LoadTopLevelOneFromDb(enTopLevelWorldLevelSet);
	return true;
}
void CWorldLevelMgr::LoadTopLevelOneFromDb(int nType)
{
	CDataPacket& DataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadTopLevelActorData);
	int nServerId = GetLogicServer()->GetServerIndex();
	DataPacket << nServerId<<nServerId<<nType;
	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
}


void CWorldLevelMgr::LoadTopLevelFromDbResult(int nType, CDataPacketReader& inPacket)
{
	BYTE nActorCount = 0;
	inPacket >> nActorCount;
	if (nActorCount == 0)//没任何玩家记录
	{
		return;
	}
	inPacket >>  m_worldLevelInfo.nWorldLevel;
	SendAllActorWorldLevel();
}

void CWorldLevelMgr::SendAllActorWorldLevel()
{
	CVector<void *> actorList;
	GLOBALCONFIG& globalconfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	for (INT_PTR i = 0; i < actorList.count(); i++)
	{
		CActor *pActor = (CActor *)actorList[i];
		if (pActor && pActor->IsInited())
		{
			if (pActor->OnGetIsTestSimulator()
				|| pActor->OnGetIsSimulator() )
			{
				continue;
			}
			int nLevel = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
			if (IsWorldLevelIconOpen(nLevel))
			{
				CActorPacket pack;
				CDataPacket & data = pActor->AllocPacket(pack);
				data << (BYTE) enMiscSystemID << (BYTE) sWorldLevel;
				data << (int)m_worldLevelInfo.nWorldLevel;
				data<<  (float) getWorldLevelRate(nLevel);
				pack.flush();
			}
		}
	}
}


float  CWorldLevelMgr::getWorldLevelRate(int nLevel)
{
	float fRate = 0;
	return fRate;
}

bool CWorldLevelMgr::IsWorldLevelIconOpen(int nActorLevel)
{
	bool ret = false;
	// if (!IsWorldLevelOpen())
	// {
	// 	return ret;
	// }
	// GLOBALCONFIG& globalconfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	// if (nActorLevel >= globalconfig.nWorldLevelIconActorLevel)
	// {
	// 	ret = true;
	// }
	return ret;
}

//人物升级时检测并发送世界等级信息
void CWorldLevelMgr::CheckWorldLevel(CActor* pActor, bool isSend)
{
	if (pActor == NULL)
	{
		return;
	}
	if (isSend)
	{
		SendWorldLevel(pActor);
	}
}

bool CWorldLevelMgr::IsCanHaveWorldLevelExp(int nActorLevel)
{
	bool ret = false;
	// if (!IsWorldLevelOpen())
	// {
	// 	return ret;
	// }
	// GLOBALCONFIG& globalconfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	// int nNeedLevel =  globalconfig.nWorldLevelActorLevelValue;
	// if (nActorLevel >= nNeedLevel)
	// {
	// 	ret = true;
	// }
	return ret;
}

bool CWorldLevelMgr::IsWorldLevelOpen()
{
    // GLOBALCONFIG& globalconfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	// int nDays = GetLogicServer()->GetDaysSinceOpenServer();
	// return  nDays >= globalconfig.nWorldLevelOpenDay ?  true:false;
}

void CWorldLevelMgr::SendWorldLevel(CEntity* pEntity)
{
	if (pEntity == NULL)
	{
		return;
	}
	if (!IsWorldLevelOpen())
	{
		return;
	}
	CActor* pActor = (CActor*)pEntity;
	GLOBALCONFIG& globalconfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	int nLevel = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	if(!IsWorldLevelIconOpen(nLevel))
	{
	    return;
	}
	CActorPacket pack;
	CDataPacket & data = pActor->AllocPacket(pack);
	data << (BYTE) enMiscSystemID << (BYTE) sWorldLevel;
	data << (int)m_worldLevelInfo.nWorldLevel;
	data<<(float)getWorldLevelRate(nLevel);
	pack.flush();
}

void CWorldLevelMgr::ResetWorldLevel()
{
	ZeroMemory(&m_worldLevelInfo,sizeof(m_worldLevelInfo));
	ReloadWorldLevel();
	SendAllActorWorldLevel();
}
void CWorldLevelMgr::ReloadWorldLevel()
{
	LoadTopLevelOneFromDb(enTopLevelWorldLevelSet);
}
void CWorldLevelMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId,nType;
	CDataPacketReader inPacket(data,size);
	BYTE nErrorCode;
	inPacket >> nRawServerId >> nLoginServerId >> nType>>nErrorCode;

	switch(nCmd)
	{
	case dcLoadTopLevelActorData:
		{
			if (nErrorCode == reSucc)
			{
				LoadTopLevelFromDbResult(nType, inPacket);
			}else
			{
				OutputMsg(rmError,_T("CWorldLevelMgr dcLoadTopLevelActorData Error!code=%d"),nErrorCode);
			}

			break;
		}
	default:
		{
			break;
		}
	}
}
