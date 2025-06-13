#pragma once

class CSkillSystem : public CEntitySystem<enSkillSystemID>
{
public:	
	void OnRecvData(INT_PTR nCmd, CDataPacketReader& pack);
	void OnTimeRun(TICKCOUNT currTick);
	void OnEnterGame();

	/*
	* Comments: 使用技能
	* Param int nSkillId: 技能Id
	* Param EntityHandle targetHandle: 目标句柄
	* Param int nX: 目标位置（或者是鼠标位置）X
	* Param int nY: 目标位置（或者是鼠标位置）Y
	* Param int nDir: 施法者的朝向
	* @Return void:
	* @Remark:
	*/
	void UseSkill(int nSkillId, EntityHandle targetHandle, int nX, int nY, int nDir);
	// 接口协议里头还需要特效id、动作id？ 不是跟着技能走的?
	void NearAttack(EntityHandle targetHandle);
	INT_PTR GetSkillCount(){return m_SkillList.count();}
protected:
	// 请求技能列表
	void RequestSkillList();
	// 占位的技能消息处理	
	void DummySkillMsgHandler(CDataPacketReader&){}
	// 接受初始技能列表
	void ProcessRcvInitSkillList(CDataPacketReader& packet);

	void LearnSkillResult(CDataPacketReader& packet);
	typedef void (CSkillSystem::*SkillMessageHandler)(CDataPacketReader&);
private:
	CBaseList<ClientSkillData>		m_SkillList;
	static const SkillMessageHandler s_aHandler[];
};