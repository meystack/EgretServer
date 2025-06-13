#include "stdafx.h"

const CSkillSystem::SkillMessageHandler CSkillSystem::s_aHandler[] = {
	&CSkillSystem::DummySkillMsgHandler,
	&CSkillSystem::ProcessRcvInitSkillList,
	&CSkillSystem::LearnSkillResult
};

void CSkillSystem::OnRecvData(INT_PTR nCmd, CDataPacketReader& pack)
{
	if (nCmd < 0 || nCmd >= ArrayCount(s_aHandler))
		return;

	(this->*s_aHandler[nCmd])(pack);
}

void CSkillSystem::OnTimeRun(TICKCOUNT currTick)
{

}

void CSkillSystem::ProcessRcvInitSkillList(CDataPacketReader& packet)
{	
	BYTE ucSkillCount = 0;
	packet >> ucSkillCount;
	if (ucSkillCount > 0)
	{
		ClientSkillData data;
		packet >> data;
		m_SkillList.add(data);
		//OutputMsg(rmNormal, _T("[Skill] Recv Skill id=%d level=%d"), (int)data.wSkillId, (int)data.ucSkillLevel);
	}
	//OutputMsg(rmNormal, _T("[Skill] Recv Skill List, total skill count=%d"), (int)ucSkillCount);
}

void CSkillSystem::OnEnterGame()
{
	RequestSkillList();
}

void CSkillSystem::RequestSkillList()
{
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	pack << (BYTE)enSkillSystemID << (BYTE)cGetSkill;
	m_pClient->FlushProtoPacket(pack);
}
void CSkillSystem::LearnSkillResult(CDataPacketReader& packet)
{
	RequestSkillList();

}
void CSkillSystem::UseSkill(int nSkillId, EntityHandle targetHandle, int nX, int nY, int nDir)
{
	if (m_SkillList.count() <= 0 || targetHandle.IsNull())
		return;
		
	if (nSkillId <= 0)
	{
		// 取第一个技能？
		CClientActor& ca = m_pClient->GetActorData();
		int job = ca.GetProperty<int>(PROP_ACTOR_VOCATION);
		if (job == enVocWarrior)
		{
			NearAttack(targetHandle);
			return; 
		}
		if (job == enVocMagician)
		{
			nSkillId = 9;
		}
		if (job == enVocWizard)
		{
			nSkillId = 17;
		}
	}
	
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	pack << (BYTE)enSkillSystemID << (BYTE)cUseSkill << (WORD)nSkillId;
	pack << targetHandle << (WORD)nX << (WORD)nY << (BYTE)nDir;
	
	m_pClient->FlushProtoPacket(pack);
}

void CSkillSystem::NearAttack(EntityHandle targetHandle)
{
	int x = m_pClient->GetActorData().GetProperty<int>(PROP_ENTITY_POSX);
	int y = m_pClient->GetActorData().GetProperty<int>(PROP_ENTITY_POSY);
	int tarX,tarY;
	m_pClient->GetLogicAgent().GetDefaultSystem().GetEntityPos(targetHandle, tarX,tarY);
	INT_PTR nDir = CUtility::GetDir(x,y,tarX,tarY);
	CDataPacket& pack = m_pClient->AllocProtoPacket();
	pack << (BYTE)enSkillSystemID << (BYTE)cNearAttack << targetHandle << (BYTE)nDir << (WORD)0;
	m_pClient->FlushProtoPacket(pack);
}