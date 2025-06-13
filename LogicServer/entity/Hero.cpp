#include "StdAfx.h"
#include "Hero.h"

CUpdateMask *CHero::s_pHeroMask = NULL;


CHero::CHero()
	:Inherited()
{
	m_pMaster =NULL;
	m_nMaxExp =0;
	m_nExp =0;
	m_nColor =0;
	m_nBattleStatus = Hero_BATTLE_STATUE_FOLLOW; //设置状态
	SetAttackLevel(100); //英雄都设置为100
}



//怪物暂时只下发少量的数据
VOID CHero::InitHeroBroadcastmask()
{
	if (!s_pHeroMask)
	{
		s_pHeroMask = new CUpdateMask();
	}
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_HP);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_MP);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_MOVEONESLOTTIME); //移动速度改变了需要广播的
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_ATTACK_SPEED);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_LEVEL); //英雄的等级
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_MAXHP);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_MAXMP);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_COLOR); //
	s_pHeroMask->SetUpdateMaskFlag(PROP_ENTITY_MODELID);
	s_pHeroMask->SetUpdateMaskFlag(PROP_ACTOR_SWINGAPPEARANCE);
	s_pHeroMask->SetUpdateMaskFlag(PROP_ACTOR_WEAPONAPPEARANCE);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_PYSICAL_DEFENCE_MAX);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_MAGIC_DEFENCE_MAX);
	s_pHeroMask->SetUpdateMaskFlag(PROP_CREATURE_LUCK);
}

void CHero::OnEntityDeath()
{
	Inherited::OnEntityDeath();
	if(m_pMaster ) //是有效的英雄
	{
		const CHeroSystem::HERODATA *pHero = GetHeroPtr();
		if (pHero)
		{
			//CVector<DROPITEM> listDropItem;
			m_pMaster->OnDeathDropItem();
		}
		m_pMaster->GetHeroSystem().SetHeroSleep( GetProperty<int>(PROP_ENTITY_ID)); //删除英雄
		
	}
	
}
void CHero::LogicRun(TICKCOUNT nCurrentTime)
{
	Inherited::LogicRun(nCurrentTime);
}
void CHero::SetBattleStatus (int nStatus)
{
	if(m_nBattleStatus == nStatus ) return; //设置怪物的状态
	if(nStatus == Hero_BATTLE_STATUE_STAY  )
	{
		GetMoveSystem()->MoveIdle(); //就在旁边呆着
	}
	else if(nStatus == Hero_BATTLE_STATUE_FOLLOW)
	{
		//从站立状态到跟随状态，切换的时候，如果和主人比较远
		if(m_nBattleStatus ==Hero_BATTLE_STATUE_STAY )
		{

		}
	}
	m_nBattleStatus =nStatus; 
}

void CHero::ProcessEntityMsg(const CEntityMsg &msg)
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
void  CHero::InitAi()
{
	unsigned int nID = GetProperty<int>(PROP_ENTITY_ID);
	if(m_pMaster ==NULL) return;
	const CHeroSystem::HERODATA *  pHero = m_pMaster->GetHeroSystem().GetHeroData(nID);
	
	if(pHero ==NULL) 
	{
		OutputMsg(rmTip,"%s,ivalid hero id=%d",__FUNCTION__ , nID);
		return;
	}
    int  nAi = GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroAI();
	//GetMoveSystem()->SetUseAstar(true); //使用A*
	m_pAI = GetGlobalLogicEngine()->GetAiMgr().CreateAi(nAi,(CAnimal*)this);	
	//GetEquipmentSystem().RefreshAppear();
}



void CHero::OnLevelUp(int nUpdateVal)
{
	Inherited::OnLevelUp(nUpdateVal);

	/*
	static unsigned int s_nHeroMaxLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nHeroMaxLevel; //英雄最大的等级

	CHeroProvider &provider =  GetLogicServer()->GetDataProvider()->GetHeroProvider(); //获得英雄的配置

	PHeroCONFIG pHero = provider.GetHeroData(GetProperty<int>(PROP_ENTITY_ID)); //获取英雄的指针
	if(pHero)
	{
		int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //等级

		PHeroLEVELCONFIG	pLevel = pHero->GetLevelData(nLevel);
		if(pLevel )
		{
			unsigned int nColor = pLevel->color; //新的颜色
			if(nColor != m_nColor) //这里要换一个名字
			{
				SetColor(nColor);//设置颜色
			}
		}

	}
	*/

}


const CHeroSystem::HERODATA * CHero::GetHeroPtr()
{
	int nId = GetProperty<int>(PROP_ENTITY_ID);
	if( m_pMaster ==NULL) return NULL;

	return m_pMaster->GetHeroSystem().GetHeroData(nId);
}


void CHero::SetColor(unsigned int color)
{
	if(m_nColor == color) return;
	
	if(m_nColor !=0)
	{
		char buff[128];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE)enDefaultEntitySystemID << (BYTE)sUpdateActorNameClr <<(UINT64) GetHandle() << color;
		GetObserverSystem()->BroadCast(data.getMemoryPtr(), data.getPosition());
	}
	m_nColor = color;

}
//增加经验
void  CHero::AddExp(int nChangeExp)
{
	//英雄最大的等级
	//static unsigned int s_nHeroMaxLevel = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nHeroMaxLevel; //英雄最大的等级

	/*
	unsigned int nLevel  = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //当前的等
	if( nLevel >= s_nHeroMaxLevel ) return; //已经到了最大的等级，那么就不需要升级了
	
	
	int nExp =0;
	int nNewExp = m_nExp + nChangeExp; //数值
	
	
	if(m_nMaxExp ==0)
	{
		m_nMaxExp=provider.GetMaxExp(nLevel,0); //获取本机升级需要的经验
	}
	unsigned int nNewLevel =0;
	bool hasChange =false;
	do 
	{
		unsigned int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); // 当前等级

		if (nLevel < (unsigned int)s_nHeroMaxLevel)	// 未到最高等级
		{	
			//nExp= provider.GetMaxExp(nLevel,0); //升到下一级需要多少经验
			if (m_nMaxExp <= 0)
				break;

			if(nNewExp >= m_nMaxExp)
			{
				nNewExp -= nExp;
				nLevel ++;
		
				((CEntity*)this)->SetLevel(nLevel);
				hasChange =true;
				m_nMaxExp = provider.GetMaxExp(nLevel,0); //设置为下级的经验
			}
			else
			{
				break;
			}
		}
		else							// 已经满级
		{	
			break;
		}
	} while (nNewExp > nExp);

	m_nExp = nNewExp;
	*/

	
}
