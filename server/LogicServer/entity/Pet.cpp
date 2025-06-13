#include "StdAfx.h"
#include "Pet.h"

CUpdateMask *CPet::s_pPetMask = NULL;


CPet::CPet()
	:Inherited()
{
	
	SetAttackLevel(100); //宠物都设置为100
}



//怪物暂时只下发少量的数据
VOID CPet::InitPetBroadcastmask()
{
	if (!s_pPetMask)
	{
		s_pPetMask = new CUpdateMask();
	}
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_HP);
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_MP);
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_MOVEONESLOTTIME); //移动速度改变了需要广播的
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_ATTACK_SPEED);
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_LEVEL); //宠物的等级
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_MAXHP);
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_MAXMP);
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_COLOR); //
	s_pPetMask->SetUpdateMaskFlag(PROP_CREATURE_DIZZY_STATUS);
}

void CPet::OnEntityDeath()
{
	Inherited::OnEntityDeath();
	//int nLoyalty =GetLogicServer()->GetDataProvider()->GetGlobalConfig().nPetDeathDropLoyalty; //宠物死亡的忠诚度的降低
	if(m_pMaster && m_nId >0) //是有效的宠物
	{
		m_pMaster->GetPetSystem().RemovePet(m_nId); //删除宠物
	}	
}

void CPet::LogicRun(TICKCOUNT nCurrentTime)
{
	if (m_3sTimer.CheckAndSet(nCurrentTime, true))
	{
		m_boAlwaysMoveFlag = false;
	}
	Inherited::LogicRun(nCurrentTime);
}

void CPet::SetBattleStatus (int nStatus)
{
	if(m_nBattleStatus == nStatus ) return; //设置怪物的状态
	//if(nStatus == PET_BATTLE_STATUE_STAY  )
	//{
	//	m_moveSystem.MoveIdle(); //就在旁边呆着
	//}
	//else if(nStatus == PET_BATTLE_STATUE_FOLLOW)
	//{
	//	//从站立状态到跟随状态，切换的时候，如果和主人比较远
	//	if(m_nBattleStatus ==PET_BATTLE_STATUE_STAY )
	////	{
//
	//	}
	//}
	m_nBattleStatus =nStatus;
	
	if (m_pMaster)
	{
		m_pMaster->SetProperty<unsigned int>(PROP_ACTOR_PET_STATUS,nStatus);
	}
}

void CPet::SetMaster(CActor *pActor)
{
	if (!pActor) return;
	
	m_pMaster =pActor;
	int nStatus = m_pMaster->GetProperty<unsigned int>(PROP_ACTOR_PET_STATUS);
	if ((nStatus != PET_BATTLE_STATUE_FOLLOW) && (nStatus != PET_BATTLE_STATUE_STAY))
	{
		m_nBattleStatus = PET_BATTLE_STATUE_FOLLOW;
		m_pMaster->SetProperty<unsigned int>(PROP_ACTOR_PET_STATUS,m_nBattleStatus);
	}
	else
	{
		m_nBattleStatus = nStatus;
	}
	
}
	
void CPet::ProcessEntityMsg(const CEntityMsg &msg)
{
	//switch(msg.nMsg)
	//{
	//	/*
	//case CEntityMsg::emGetQuestState:
	//	if (msg.nSender.GetType() == enActor)
	//	{
	//		pSender = GetEntityFromHandle(msg.nSender);
	//		if (pSender) ((CActor*)pSender)->GetQuestSystem()->SendNpcQuestState(this);
	//	}
	//	break;
	//	*/

	//default:
	//	Inherited::ProcessEntityMsg(msg);
	//	break;
	//}
	return Inherited::ProcessEntityMsg(msg);
}

//初始化ai
void  CPet::InitAi( int nAiId)
{
	unsigned int nID = GetProperty<unsigned int>(PROP_ENTITY_ID);
	MONSTERCONFIG *pData= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
	if(pData ==NULL)
	{
		OutputMsg(rmError,_T("%s,id=%d is Null "),__FUNCTION__,nID);
		return;
	}
	m_moveSystem.SetUseAstar(true); //使用A*
	m_pAI = GetGlobalLogicEngine()->GetAiMgr().CreateAi(nAiId,(CAnimal*)this);	
	m_attriFlag = *(pData->pflags);
	
	//最大的掉血
	SetMaxDropHp(pData->nMaxDropHp);
	SetAttackInterval(pData->nAttackInterval); //设置攻击间隔
}


void CPet::OnLevelUp(int nUpdateVal)
{
	// Inherited::OnLevelUp(nUpdateVal);

	// static unsigned int s_nPetMaxLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nPetMaxLevel; //宠物最大的等级
	// unsigned int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //等级
	// if (nLevel <= s_nPetMaxLevel && nLevel > 0 && nLevel < MAX_PET_LEVEL_COUNT)
	// {
	// 	unsigned int nColor = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nPetLevels[nLevel-1];
	// 	if (nColor != m_nColor)
	// 	{
	// 		SetColor(nColor);
	// 	}
	// }
}
/*
const CPetSystem::PETDATA * CPet::GetPetPtr()
{
	if(m_nId ==0 || m_pMaster ==NULL) return NULL;
	return NULL;
	return m_pMaster->GetPetSystem().GetPetData(m_nId);
}
*/

void CPet::SetColor(unsigned int color)
{
	if(m_nColor == color) return;
	
	if(m_nColor !=0)
	{
		char buff[128];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) GetHandle() << color;
		m_observerSystem.BroadCast(data.getMemoryPtr(), data.getPosition());
	}
	m_nColor = color;
}

//增加经验
void  CPet::AddExp(int nChangeExp)
{
	// //宠物最大的等级
	// static unsigned int s_nPetMaxLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nPetMaxLevel; //宠物最大的等级

	// unsigned int nLevel  = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //当前的等
	// if( nLevel >= s_nPetMaxLevel ) return; //已经到了最大的等级，那么就不需要升级了
	
	
	// int nExp =0;
	// int nNewExp = m_nExp + nChangeExp; //数值
	// CPetProvider &provider =  GetLogicServer()->GetDataProvider()->GetPetProvider(); //获得宠物的配置
	
	// if(m_nMaxExp ==0)
	// {
	// 	m_nMaxExp=provider.GetMaxExp(nLevel,0); //获取本机升级需要的经验
	// }
	// // unsigned int nNewLevel =0;
	// bool hasChange =false;
	// do 
	// {
	// 	unsigned int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); // 当前等级

	// 	if (nLevel < (unsigned int)s_nPetMaxLevel)	// 未到最高等级
	// 	{	
	// 		//nExp= provider.GetMaxExp(nLevel,0); //升到下一级需要多少经验
	// 		if (m_nMaxExp <= 0)
	// 			break;

	// 		if(nNewExp >= m_nMaxExp)
	// 		{
	// 			nNewExp -= nExp;
	// 			nLevel ++;
				
	// 			/*
	// 			SetProperty<unsigned int>(PROP_CREATURE_LEVEL,nLevel);
	// 			PPETCONFIG pPet = provider.GetPetData(GetProperty<int>(PROP_ENTITY_ID)); //获取宠物的指针
	// 			if(pPet)
	// 			{
	// 				PPETLEVELCONFIG	pLevel = pPet->GetLevelData(nLevel);
	// 				if(pLevel )
	// 				{
	// 					unsigned int nColor = pLevel->color; //新的颜色
	// 					if(nColor != m_nColor) //这里要换一个名字
	// 					{
	// 						SetColor(nColor);//设置颜色
	// 					}
	// 				}
	// 			}

	// 			 */
	// 			((CEntity*)this)->SetLevel(nLevel);
	// 			hasChange =true;
	// 			m_nMaxExp = provider.GetMaxExp(nLevel,0); //设置为下级的经验
	// 		}
	// 		else
	// 		{
	// 			break;
	// 		}
	// 	}
	// 	else							// 已经满级
	// 	{	
	// 		break;
	// 	}
	// } while (nNewExp > nExp);

	// /*
	// if(hasChange)
	// {
	// 	CollectOperate(CEntityOPCollector::coRefAbility);	//重新刷能力
	// }
	// */

	// m_nExp = nNewExp;
}

bool CPet::SetLiveTime( int nSecs )
{
	if (nSecs == 0)
	{
		return true;
	}
	unsigned int nNow = GetGlobalLogicEngine()->getMiniDateTime();
	if (m_nLiveTime == 0)
	{
		m_nLiveTime = nNow + nSecs;
		return true;
	}
	return false;
}

void CPet::OnKilledByEntity( CEntity * pKillerEntity )
{
	CAnimalAI * pAi = GetAI();
	if(pAi)
	{
		pAi->KillBy(); //被杀死了
	}
}
