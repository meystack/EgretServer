#include "StdAfx.h"
#include "EntityVarSubSystem.h"

void CActorVarSubSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(m_pEntity ==NULL) return;
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadVar &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		m_Var.loadFromMemory(reader.getOffsetPtr(), reader.getAvaliableLength());

		// 读取禁言数据
		m_pEntity->GetChatSystem()->LoadFromScriptData();	
		//m_pEntity->GetBuffSystem()->LoadFromScriptData();
	}
	else if(nCmd == jxInterSrvComm::DbServerProto::dcLoadVar &&  nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		OutputMsg(rmError,_T("装载玩家[%s]的装备的数据出错,actorid=%u"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
	}
	m_pEntity->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_ACTORVARSUB_SYSTEM);
}

void CActorVarSubSystem::Save(PACTORDBDATA pData)
{
	if(m_pEntity ==NULL) return;

	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	
	CDataPacket& dataPacket = pDBClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveVar);
	dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket << (unsigned int) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家ID
	//m_Var.set("2",222);
	//m_Var.set("1","111");
	size_t savesize = 0;
	CCLVariant *pContinueVar = &m_Var;

	dataPacket.setLength(4096);//预留4K空间
	INT_PTR nReserverCount =0;
	while (true)
	{
		savesize = pContinueVar->saveToMemory(dataPacket.getOffsetPtr(), dataPacket.getAvaliableCapacity(), &pContinueVar);
		dataPacket.adjustOffset(savesize);
		if (!pContinueVar)
			break;
		//这里修改了一处死循环，问题是大小大于4096将无法申请空间
		dataPacket.setLength(dataPacket.getLength() + 4096);//每次增长4K
		//OutputMsg(rmWaning,_T("actor id=%u, the var data too large"),(unsigned int ) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		nReserverCount ++;
		if(nReserverCount >10)
		{
			break;
		}
	}

	pDBClient->flushProtoPacket(dataPacket);
}
