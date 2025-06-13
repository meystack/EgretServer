#include "StdAfx.h"
#include "Animal.h"
#include "../LogicServer.h"
#include "../skill/SkillSubSystem.h"
#include "config/HallowsConfig.h"


unsigned int * CAnimal::s_stateForbidMask =NULL;

#ifdef _DEBUG
extern std::string s_HurtValueMsg;
extern char s_HurtValueBuff[100];
#endif

 bool CAnimal::Init(void * data, size_t size)
 {
	 m_pAI =NULL;
	 
	if(Inherited::Init(data,size) ==false) return false;
	
	if(m_observerSystem.Create(this,data,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for observer system "));
		return false; //观察者子系统
	}
	if(m_moveSystem.Create(this,data,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for movement system "));
		return false; //移动子系统
	}

	if(m_propertySystem.Create(this,data,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for property system "));
		return false; // 属性子系统的初始化
	}
	
	if(m_buffSystem.Create(this,data,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for buff system "));
		return false; // buff子系统的初始化
	}
	
	if(m_skillSystem.Create(this,data,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for skill system "));
		return false; // 技能子系统的初始化
	}

	m_effectSystem.Create(this,data,size);
	return true;
 }
 //等级提升
 void CAnimal::OnLevelUp(int nUpdateVal)
 {
	 //需要重新计算属性
	CollectOperate(CEntityOPCollector::coRefAbility);	
	// 更新升级后的血、蓝
	if(!IsDeath())
		CollectOperate(CEntityOPCollector::coResetMaxHPMP);
 }

 VOID CAnimal::BeforeLogicRun(TICKCOUNT nCurrentTime)
 {
	 Inherited::BeforeLogicRun(nCurrentTime);
	 //清空汇总操作标记
	
 }

 int  CAnimal::GetAttackValue(INT_PTR nAttackType)
 {

	 int nAttackMax,nAttackMin;
	 switch(nAttackType)
	 {
	 case TYPE_PHYSICAL_ATTACK: //物理攻击类型
		 nAttackMax= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MAX);
		 nAttackMin= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MIN);
		 break;
	 case TYPE_MAGIC_ATTACK://魔法攻击类型
		 nAttackMax= GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX);
		 nAttackMin= GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MIN);
		 break;
	 case TYPE_WIZARD_ATTACK://道士攻击类型
		 nAttackMax= GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MAX);
		 nAttackMin= GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MIN);
		 break;
	case TYPE_HOLY_ATTACK: //神圣攻击
		 if (GetType() == enActor)
		 {
			 switch (((CActorProperty*)((CActor*)this)->GetPropertyPtr())->nVocation)
			 {
			 case enVocWarrior:
				nAttackMax= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MAX);
		 		nAttackMin= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MIN);
				break;
			 case enVocMagician:
				nAttackMax= GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MAX);
		 		nAttackMin= GetProperty<int>(PROP_CREATURE_MAGIC_ATTACK_MIN);
				break;
			 case enVocWizard:
				nAttackMax= GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MAX);
		 		nAttackMin= GetProperty<int>(PROP_CREATURE_WIZARD_ATTACK_MIN);
				break;
			 default:
			 	return 0;
			 }
		 }
		 else if (GetType() == enMonster)
		 {
			nAttackMax= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MAX);
		 	nAttackMin= GetProperty<int>(PROP_CREATURE_PHYSICAL_ATTACK_MIN);
		 }
		 break;
	 default:
		 return 0;
	 }
	 if (nAttackMin == 0) nAttackMin = 1;
	 if (nAttackMax == 0) nAttackMax = 1;
	 if(nAttackMin >= nAttackMax) return nAttackMax; //没什么算的
	 int nLuck = GetProperty<int>(PROP_CREATURE_LUCK); //幸运

	 //使用静态遍历，避免每次都去取一下
	 GLOBALCONFIG& globalConfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	 int nRandomVal = nAttackMax - nAttackMin + 1;
	 nRandomVal = wrand( nRandomVal );

	 int nAttackMaxRadom = 0;

	 if (nLuck == 0 )
	 {
		 return nAttackMin + nRandomVal;
	 }
	 else if (nLuck > 0)
	 {
		int nRandVal = nLuck * 3;
		// if (nLuck > 15)
		// {
		// 	nRandVal = 45;
		// }
		 
		if (wrand(100) < (unsigned long)nRandVal)
		{
			return nAttackMax;
		}
		else
		{
			return nAttackMin + nRandomVal;
		}
	 }
 }

 void  CAnimal::OnEntityDeath()
 {
	 Inherited::OnEntityDeath();	

	 bool bHasDrop = 0; //声音的id

	 if(enMonster == GetType())
	 {
		const PMONSTERCONFIG pMonster= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(GetProperty<int>(PROP_ENTITY_ID));
		if( pMonster !=NULL )
		{
			bHasDrop = ((CMonster*)this)->isHasDrop();
		}
	 }
	 char buff[128];
	 CDataPacket data(buff,sizeof(buff));
	 data << (BYTE)enDefaultEntitySystemID << (BYTE) sEntityDeath << (Uint64)GetHandle() << (Uint64 )m_killHandler << bHasDrop;
	 m_observerSystem.BroadCast(data.getMemoryPtr(),data.getPosition(),true);
	 
	 m_effectSystem.DelAllEffect(true); //死亡了需要删除所有的特效

	 m_buffSystem.OnDeadRemoveBuff();	 
}

VOID CAnimal::ChangeHP(int hpDrop, CEntity * pKiller,bool bIgnoreDamageRedure, bool bIgnoreMaxDropHp, bool boSkillResult,int btHitType)
{
	//这里支持哪种每次只掉1滴血的情况
	if(hpDrop == 0) 
	{
		return ;
	}
	INT_PTR nKillType =-1;
	bool hasKiller = false;
	if(pKiller)
	{
		nKillType = pKiller->GetType();
		hasKiller =true;
		m_AttackerHandler = pKiller->GetHandle();
	}
	INT_PTR nSelfType = GetType();

	if(nKillType == enActor)
	{
		if ( ((CActor*)pKiller)->OnGetIsTestSimulator() || ((CActor*)pKiller)->OnGetIsSimulator())
		{ 
			hpDrop = 0;
		}
	}
	if(nSelfType == enActor)
	{
		if ( ((CActor*)this)->OnGetIsTestSimulator() || ((CActor*)this)->OnGetIsSimulator())
		{ 
			hpDrop = 0;
		}
	}

	int nHpDrop = 0;
	int absDrop = 0;
	//伤害吸收
	if(hpDrop < 0)
	{
		absDrop = -hpDrop; //掉了多少血
		
		//最原始的掉血，这里要计算目标的最大的掉血
		// int originalAbsDrop = absDrop;
		// int nFinalDamageValue = 0;
		int S7 = 10000;
		//[BUFF] 伤害抵消的BUFF状态 ( value:池值,  param:吸收万分比 )
		CBuffSystem * bufSystem =GetBuffSystem();
		CDynamicBuff *pBuff = bufSystem->GetFirstBuff(aDamageAbsorb);
		if(pBuff && absDrop>0 && !bIgnoreDamageRedure)
		{
			INT_PTR nBuffGroup = pBuff->btGroup; //buff 

			if(pBuff->value.nValue  > 0)
			{
				float fAbsorbRate = ((float)pBuff->pConfig->nParam / (float)10000);
				S7 -= pBuff->pConfig->nParam;
				int nBuffAbsorb = fAbsorbRate * absDrop;//吸收的伤害
				nBuffAbsorb = __min(nBuffAbsorb, pBuff->value.nValue);
				// nFinalDamageValue -= nBuffAbsorb;
				pBuff->value.nValue -= nBuffAbsorb;//池值降低
				#ifdef _DEBUG
				// S8
				SNPRINTFA(s_HurtValueBuff,100," //S7::%d",S7);
				s_HurtValueMsg += s_HurtValueBuff;
				#endif
			}
			if(pBuff->value.nValue <= 0)
			{
				bufSystem->Remove(aDamageAbsorb,nBuffGroup); //如果已经吸收完了，就去掉这个buff
			}
		}
		int S8 = 10000;
		//伤害加成 --mhl
		if(btHitType != htFire && pKiller && pKiller->isAnimal()) {
			//伤害加成
			S8 += ((CAnimal*)pKiller)->m_nDoubleAtkRate;
			// int nDouAtkValue = absDrop * (((CAnimal*)pKiller)->m_nDoubleAtkRate / 10000.0);
			// nFinalDamageValue += nDouAtkValue;
			#ifdef _DEBUG
			// S7
			SNPRINTFA(s_HurtValueBuff,100," //S8:%d;",S8);
			s_HurtValueMsg += s_HurtValueBuff;
			#endif
		}

		int S9 = 10000;
		//[NORMAL] 按比例吸收伤害，降低伤害的数值
		if( S9 > 0.0f && !bIgnoreDamageRedure ) 
		{
			S9 -=(int)(m_fDamgeAbsorbRate*10000);
			// int nDamgeAbsorb =(int) (absDrop * m_fDamgeAbsorbRate); //这里伤害减免了一定的数值
			// nFinalDamageValue -= nDamgeAbsorb;
			#ifdef _DEBUG
			// S9伤害减免比例
			SNPRINTFA(s_HurtValueBuff,100," //S9::%d",S9);
			s_HurtValueMsg += s_HurtValueBuff;
			#endif
		}

		//[NORMAL] 伤害转换为mp消耗
		int S10 = 10000;
		int fDamage2MpRate = GetHp2MpRate(); //获取血->蓝的转换比例
		if(fDamage2MpRate >0)
		{
			S10 -= fDamage2MpRate;
			int nMp = GetProperty<unsigned int >(PROP_CREATURE_MP) ; //获取玩家的蓝
			if(nMp >0)
			{
				int nDropMp = (int)(((fDamage2MpRate * absDrop)/10000.0)/50); 
				int nAbsorb = __min( nDropMp , nMp); //吸收的点数
				if(nAbsorb >0)
				{
					// nFinalDamageValue -= nAbsorb;
					//static float fRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().fBodyDamageRate;
					//nAbsorb = (int)(__min(nDropMp * fRate, nMp));

					if (nMp - nAbsorb >= 500)
					{
						ChangeMP(-1 * nAbsorb); //降低蓝
					}
				}
				
				#ifdef _DEBUG
				// S10
				SNPRINTFA(s_HurtValueBuff,100," //S10:%d;",S10);
				s_HurtValueMsg += s_HurtValueBuff;
				#endif

			}
		}
		//守护 --mhl
		bool bGuardEff = false;
		int S11 = 10000;

		int nGuardRand =wrand(10000); //取1万之间的随机数
		if( m_nGuardRate > 0 && nGuardRand <= m_nGuardRate) {
			S11 -= m_nGuardValue;
			// int nAbsorb = absDrop * (10000 - m_nGuardValue)/10000.0;
			// nFinalDamageValue -= nAbsorb;
			bGuardEff = true;
			#ifdef _DEBUG
			// S10
			SNPRINTFA(s_HurtValueBuff,100," //S11:%d;",S11);
			s_HurtValueMsg += s_HurtValueBuff;
			#endif
		}

		//切割--只对怪物生效 --mhl
		int S12 = 10000;
		if(nSelfType == enMonster && (pKiller && pKiller->isAnimal())) {
			S12 += ((CAnimal*)pKiller)->m_nCuttingRate;
			// int nCutDmage = absDrop * (((CAnimal*)pKiller)->m_nCuttingRate / 10000.0);
			// nFinalDamageValue += nCutDmage;
			#ifdef _DEBUG
			// S12
			SNPRINTFA(s_HurtValueBuff,100," //S12:%d;",S12);
			
			s_HurtValueMsg += s_HurtValueBuff;
			#endif
			// static int nGuardEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nGuardEffId;
			// CEntityMsg msg(CEntityMsg::emShowEntityEffect, pKiller->GetHandle());
			// msg.nParam1 = nGuardEffId;
			// this->PostEntityMsg(msg);
		}

		//pk伤害减免
		int S13 = 10000;
		if(nSelfType == enActor && nKillType == enActor)
		{
			S13 -= m_nPkDamageAbsorbRatio;
			// int nAbsorb = absDrop * (m_nPkDamageAbsorbRatio / 10000.0);
			// nFinalDamageValue -= nAbsorb;
			#ifdef _DEBUG
			// S12
			SNPRINTFA(s_HurtValueBuff,100," //S13:%d;",S13);
			s_HurtValueMsg += s_HurtValueBuff;
			#endif
		}
		//伤害流程计算 
		//s6 = s6*s7*s8*s9*s11*s12*s13
		absDrop = (int)(absDrop*(S7/10000.0)*(S8/10000.0)*(S9/10000.0)*(S10/10000.0)*(S11/10000.0)*(S12/10000.0)*(S13/10000.0));
		// absDrop += nFinalDamageValue;
		#ifdef _DEBUG
		// Z1
		SNPRINTFA(s_HurtValueBuff,100," //Z1:%d;",absDrop);
		s_HurtValueMsg += s_HurtValueBuff;
		#endif
		if(absDrop <= 0)
			absDrop = 1;

		//剧毒绝对(绝对值)
		if(m_nToxicAttckValue) 
		{
			#ifdef _DEBUG
				// J1
				SNPRINTFA(s_HurtValueBuff,100," //J1:%d;",m_nToxicAttckValue);
				s_HurtValueMsg += s_HurtValueBuff;
			#endif
			absDrop += m_nToxicAttckValue;

			static int nTXJDCJ = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nTXJDCJ;
			CEntityMsg msg2(CEntityMsg::emShowEntityEffect, GetHandle());
			msg2.nParam1 = nTXJDCJ;
			msg2.nParam3 = (int)m_nToxicAttckValue;
			this->PostEntityMsg(msg2);

			m_nToxicAttckValue = 0;
		}

		//冰冻伤害(绝对值)
		if(m_nSZASAttckValue)
		{
			#ifdef _DEBUG
				// J1
				SNPRINTFA(s_HurtValueBuff,100," //J2:%d;",m_nSZASAttckValue);
				s_HurtValueMsg += s_HurtValueBuff;
			#endif
			absDrop += m_nSZASAttckValue;

			static int nTXSZAS = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nTXSZAS;
			CEntityMsg msg2(CEntityMsg::emShowEntityEffect, GetHandle());
			msg2.nParam1 = nTXSZAS;
			msg2.nParam3 = (int)m_nSZASAttckValue;
			this->PostEntityMsg(msg2);

			m_nSZASAttckValue = 0;
		}

		//怪物收到攻击的时候需要计算一下声音
		if ((nSelfType == enPet || nSelfType == enActor || nSelfType == enHero) && hasKiller && IsInited() && nKillType == enActor && this != pKiller)
		{
			CActor *pAttacked = ((CActor*)this);
			if (nSelfType == enPet)
			{
				pAttacked = ((CPet *)this)->GetMaster();
			}
			else if(nSelfType == enHero)
			{
				pAttacked = ((CHero *)this)->GetMaster(); //主人
			}

			if (pAttacked)
			{
				CActorPacket pack ;
				pAttacked->AllocPacket(pack);
				pack << (BYTE) enSkillSystemID <<(BYTE)sAttackByActor;
				pack << (Uint64)(pKiller->GetHandle()) ;
				pack.flush();

				//动攻击黄名、白名玩家后，变成褐名
				((CActor*)this)->GetSkillSystem().SetInitiativeAttack(pKiller,pAttacked);

				if(bGuardEff) {
					static int nGuardEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nGuardEffId;
					CEntityMsg msg(CEntityMsg::emShowEntityEffect, pKiller->GetHandle());
					msg.nParam1 = nGuardEffId;
					this->PostEntityMsg(msg);
				}
				
			}
		}

		//掉血判断
		if(absDrop <=0)
		{
			OutputMsg(rmTip, _T("收到攻击，但无伤害!"));
			return;
		}
		nHpDrop = - absDrop;
	} 
	else
	{
		nHpDrop = hpDrop;
	}

	if(m_DieRefreshHpPro > 0 || m_MonsterDieHpPro > 0)
	{
		if (m_MonsterDieHpPro > 0)
		{
			m_DieRefreshHpPro = m_MonsterDieHpPro;
		}
	}

	//兵魂-救主灵刃
	if(pKiller)//&& btHitType != htFire)
	{
		if(nSelfType == enActor)
		{
			int nCurrentHP = GetProperty<unsigned int >(PROP_CREATURE_HP);
			int nMaxHP = GetProperty<unsigned int >(PROP_CREATURE_MAXHP);
			
			//救主灵刃CD时间为0，不触发
			if(m_nSaviorTime && (nCurrentHP - absDrop) < (nMaxHP * (float)GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nSaviorPercen / 10000) && m_nJZLRCDTime < GetGlobalLogicEngine()->getMiniDateTime())
			{
				bool bExitJZLRBuff = true;
				std::vector<int>::iterator iter = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vJZLRBuff.begin();
				for (; iter != GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vJZLRBuff.end(); ++iter)
				{
					CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(*iter);
					if(pBUFFCONFIG)
					{
						CDynamicBuff* pDynamicBuff = GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
						if(!pDynamicBuff)
						{
							bExitJZLRBuff = false;
							break;
						}
					}
				}

				if (bExitJZLRBuff)
				{
					//触发效果：抵御本次伤害并回复最大HP的百分比 + 绝对值
					nHpDrop = nMaxHP * (m_nSaviorRate / 10000.0) + m_nSaviorValue;
									
					//设置救主灵刃CD时间
					m_nJZLRCDTime = GetGlobalLogicEngine()->getMiniDateTime() + (m_nSaviorTime / 1000);
					
					//设置救主Buff的CD时间
					CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nJZBuffId);
					if(pBUFFCONFIG)
					{
						CDynamicBuff* pDynamicBuff = GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
						if( pDynamicBuff )
						{
							pDynamicBuff->m_nBuffCd = (m_nSaviorTime / 1000);
							GetBuffSystem()->OnBuffChangeCdTime(pDynamicBuff);
						}
					}

					//针对离线CD重置的处理
					((CActor*)this)->SaveJZLRCDTime(szJZLRCDTime, m_nJZLRCDTime);
					static int nTXJZLR = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nTXJZLR;
					CEntityMsg msg(CEntityMsg::emShowEntityEffect, GetHandle());
					msg.nParam1 = nTXJZLR;
					PostEntityMsg(msg);
				}
			}
		}
	}

	//OutputMsg(rmTip, _T("真实伤害  玩家name: , 伤害:%d, "),  nHpDrop);
  
	//Z2
	if(hpDrop < 0)
	{
		if (!bIgnoreMaxDropHp)	//是否忽略每次最大伤害限制
		{
			int nMaxDropHp = GetMaxDropHp();
			if(nMaxDropHp > 0) //每次掉血的数量
			{	
				if(-nHpDrop > nMaxDropHp ) //如果掉落的血超过了最大，就直接设置为掉落
				{
					nHpDrop = -nMaxDropHp;
				}
			}
		}
	}
	
	if (nSelfType == enMonster)
	{
		if(((CMonster*)this)->GetMonsterType() == MONSTERCONFIG::mtKungfu)
		{
			if (nKillType == enActor)
			{
				char cBuff[30];
				SNPRINTFA(cBuff,100,"伤害: %d",-nHpDrop);
				std::string sFinalHurt;
				sFinalHurt += cBuff;
				((CActor*)pKiller)->SendTipmsg(sFinalHurt.c_str(),tstFigthing);
			}
			return;
		}
	}

	Inherited::ChangeHP(nHpDrop,pKiller);

	//H1
	if(hpDrop < 0)
	{
		if(btHitType != htFire && pKiller && pKiller->isAnimal())
		{
			if( ((CAnimal*)pKiller)->m_nSuckBloodRatio > 0) 
			{	
				int nDamage = absDrop;
				if (!bIgnoreMaxDropHp)	//是否忽略每次最大伤害限制
				{
					int nMaxDropHp = GetMaxDropHp();
					if(nMaxDropHp > 0) //每次掉血的数量
					{
						if(nDamage > nMaxDropHp ) //如果掉落的血超过了最大，就直接设置为掉落
						{
							nDamage = nMaxDropHp;
						}	
					}
				}

				int	 nSuckBloodRatio =  ((CAnimal*)pKiller)->m_nSuckBloodRatio;	
				LONG64 nSuck = (LONG64)(nDamage*( (double)nSuckBloodRatio/10000.0) ); 

				
				int nSuckBloodRate = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSuckBloodRate;										//吸血属性(怪物)
				int nSuckBloodRatePlayer = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSuckBloodRatePlayer ;								//吸血属性 (玩家)万分比
				if(nSelfType == enActor && nKillType == enActor)//如果是玩家对玩家
				{ 
					nSuck = (LONG64)nSuck * ((double)nSuckBloodRatePlayer/10000.0);
				}
				else
				{ 
					nSuck = (LONG64)nSuck * ((double)nSuckBloodRate/10000.0);//* nSuckBloodRate/10000.0;
				}
				//OutputMsg(rmTip, _T("吸血伤害  玩家name: , 原始伤害:%d 吸血:%d 比例：%d "),  (int)absDrop, (int)nSuck, ((CAnimal*)pKiller)->m_nSuckBloodRatio);
				((CAnimal*)pKiller)->ChangeHP(nSuck);
				#ifdef _DEBUG
				// H1
				SNPRINTFA(s_HurtValueBuff,100," //H1:%d;",nSuck);
				s_HurtValueMsg += s_HurtValueBuff;
				#endif
			}
		}
	}
	
	if (nHpDrop < 0)//2017-03-29修改，掉血数据放在changehp后下发，并实时下发当前血量,不用等属性变化再刷新，会有延迟
	{
		int absDrop = -nHpDrop; //掉了多少血
		int nHp =  GetProperty<unsigned int >(PROP_CREATURE_HP);
		if(hasKiller && nKillType ==enActor && pKiller->IsInited() && this != pKiller)
		{
			if(absDrop >0 ) //有掉血的话，就需要给打出伤害这边的发一个消息包
			{
				if(GetType() != enGatherMonster)
				{
					int nMusicId=0;
					if(nSelfType ==enMonster)
					{
						const PMONSTERCONFIG pMonster= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(GetProperty<int>(PROP_ENTITY_ID));
						if( pMonster !=NULL )
						{
							int nRate = pMonster->bDamageMusicRate;
							int nId   = pMonster->bDamageMusicId;
							if(nRate >0 && nId >0)
							{
								if(nRate >=100 || nRate >= (int)(wrand(101)))
								{
									nMusicId = nId;
								}
							}
						}
					}
					//自己对怪物造成的伤害要播放一个声音，宝宝的不要
					unsigned int nCritDamage = 0;
					if (boSkillResult)
					{
						nCritDamage = pKiller->GetProperty<unsigned int>(PROP_ACTOR_CRIT_DAMAGE);
					}
					char buff[1024];
					CDataPacket outPack(buff, sizeof(buff)); //下发的数据包
					outPack << (BYTE) enSkillSystemID <<(BYTE)sSelfDamageOther;
					//GetHandle() 用于客户端查找 对应句柄的玩家 
					outPack <<GetHandle() << (int)absDrop << nMusicId << nCritDamage << nHp;
					if (boSkillResult)
					{
						pKiller->SetProperty<unsigned int>(PROP_ACTOR_CRIT_DAMAGE,0);
					}
					((CActor*)pKiller)->SendData(outPack.getMemoryPtr(), outPack.getPosition());

					if(nSelfType == enActor)
					{
						CActor *pAttacked = ((CActor*)this);
						if(pAttacked && pAttacked->IsInited())
							pAttacked->SendData(outPack.getMemoryPtr(), outPack.getPosition());
					}
				}
			}
		}
		//宠物打出血了，要告诉宠物的主人，通知是自己打出来的
		if(hasKiller && (nKillType ==enPet || nKillType == enHero))
		{
			if(absDrop >0 ) //有掉血的话，就需要给打出伤害这边的发一个消息包
			{
				if(GetType() != enGatherMonster)
				{
					
					CActor *pMaster = NULL;
					
					if(nKillType == enPet)
					{
						pMaster=((CPet*)pKiller)->GetMaster(); //主人
					}
					else if(nKillType == enHero)
					{
						pMaster=((CHero*)pKiller)->GetMaster(); //主人
					}

					if(pMaster && pMaster->IsInited())
					{
						CActorPacket pack ;
						pMaster->AllocPacket(pack);
						pack << (BYTE) enSkillSystemID <<(BYTE)sSelfDamageOther;
						pack << GetHandle() << (int)absDrop << (int)0 << (unsigned int)0<<nHp;
						pack.flush();						
					}
					
				}
			}
		}
	}
	
}

void CAnimal::OnEntityRelive()
{
	if(m_pAI == NULL) return;

	CSkillSubSystem & skillSystem =  m_pAI->m_pSelf->GetSkillSystem();
	const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();

	//这里加这个是为了一个技能
	for(INT_PTR i=0; i< skills.count();i++)
	{
		if(skills[i].bEvent == mSkilleventAlive)
		{
			skillSystem.LaunchSkill(skills[i].nSkillID);
		}
	}
}

void CAnimal::ChangeName(const char* sName)
{
	//OutputMsg(rmNormal, _T("CAnimal::ChangeName(), pShowsNameName=%s"), sName);
	if(sName ==NULL) return;
	if( strcmp(GetEntityName(), sName) ==0) return; //如果没有改变就不弄
	//SetEntityName(sName);

	char buff[148];
	CDataPacket data(buff,sizeof(buff));

	data << (BYTE)enDefaultEntitySystemID <<(BYTE) sChangeShowName ;
	data << (Uint64) GetHandle(); //实体句柄
	data.writeString(GetShowName()); //将这个字符串写入

	if(GetType() ==enActor)
	{
		m_observerSystem.BroadCast(data.getMemoryPtr(), data.getPosition(),true); 
	}
	else
	{
		m_observerSystem.BroadCast(data.getMemoryPtr(), data.getPosition(),false); 
	}	
}

void CAnimal::ChangeShowName(int nRadius, LPCTSTR sName)
{
	
	if (sName ==NULL) return;
	if (strcmp(GetShowName(), sName) ==0) return; //如果没有改变就不弄
	CScene * pScene = GetScene();
	if (!pScene) return;
	SetShowName(sName);
	char sAllName[128];	
	if (GetType() == enNpc)
	{
		sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",sName,GetTitle());
	}
	else
	{
		strncpy(sAllName, sName, strlen(sName));
	}
	
	EntityVector &vecPlys = *(CFuBenManager::m_pVisibleList);
	vecPlys.clear();
	pScene->GetVisibleList(this, vecPlys, -nRadius, nRadius, -nRadius, nRadius);
	INT_PTR nEntityCount = vecPlys.count();
	if (nEntityCount > 0)
	{
		Uint64 hEntityHandle = (Uint64)GetHandle();
		CEntityManager * pMgr = GetGlobalLogicEngine()->GetEntityMgr();
		for (int i = 0; i < nEntityCount; i++)
		{
			CEntity * pActor = pMgr->GetEntity(vecPlys[i]);
			if (pActor && pActor->GetType() == enActor)
			{
				CActorPacket ap;
				CDataPacket & outPack = ((CActor*)pActor)->AllocPacket(ap);
				outPack << (BYTE)enDefaultEntitySystemID << (BYTE)sChangeShowName;
				outPack << hEntityHandle;
				outPack.writeString(sAllName); //名字=名字+'\'+称号，用\隔开
				ap.flush();	
			}
		}
	}			
}


void CAnimal::ChangeModel(int nRadius, int nModelId)
{
	CScene * pScene = GetScene();
	if (!pScene)
		return;
	if (GetProperty<unsigned int>(PROP_ENTITY_MODELID) == nModelId)
		return;
	SetProperty<unsigned int>(PROP_ENTITY_MODELID, nModelId);
	EntityVector &vecPlys = *(CFuBenManager::m_pVisibleList);
	vecPlys.clear();
	pScene->GetVisibleList(this, vecPlys, -nRadius, nRadius, -nRadius, nRadius);
	INT_PTR nEntityCount = vecPlys.count();
	if (nEntityCount > 0)
	{
		Uint64 hEntityHandle = (Uint64)GetHandle();
		CEntityManager * pMgr = GetGlobalLogicEngine()->GetEntityMgr();
		for (int i = 0; i < nEntityCount; i++)
		{
			CEntity * pActor = pMgr->GetEntity(vecPlys[i]);
			if (pActor && pActor->GetType() == enActor)
			{
				CActorPacket ap;
				CDataPacket & outPack = ((CActor*)pActor)->AllocPacket(ap);
				outPack << (BYTE)enDefaultEntitySystemID << (BYTE)sEntityPropertyChange;
				outPack << hEntityHandle << (BYTE)1 << (BYTE)PROP_ENTITY_MODELID << nModelId;
				ap.flush();	
			}
		}
	}		
}


void CAnimal::SetAttackType(int nType,bool bNeedBroadCast )
{ 
	m_nAttackType = nType;
	if(bNeedBroadCast)
	{
		char buff[128];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE) enDefaultEntitySystemID << (BYTE) sChangeAttackType
			<<	(Uint64)GetHandle()  << (BYTE) m_nAttackType;
		CObserverSystem * pSystem = GetObserverSystem();
		if(pSystem)
		{
			pSystem->BroadCast(data.getMemoryPtr(),data.getPosition(),false);
		}
	}
}

void CAnimal::SetCamp(int nCampId)
{
	if(GetType() != enActor)
	{
		m_nCamp = nCampId;
	}
	else
	{
		SetProperty<int>(PROP_ACTOR_ZY, nCampId);
	}
}

void CAnimal::OnAttacked(CAnimal * pEntity, bool bSetVest)
{
	bool bUpdateAttackStatus = true;
	INT_PTR nType =  GetType();
	 
	CAnimalAI* pAI = GetAI();
	if (pAI)
	{
		pAI->AttackedBy(pEntity);
		bUpdateAttackStatus = false;
	}
	if (pEntity )
	{
		INT_PTR nAttackEntityType = ((CEntity*)pEntity)->GetType();
		if ( nAttackEntityType == enActor || nAttackEntityType == enPet || nAttackEntityType == enHero || nAttackEntityType == enMonster )
		{
			if ( (nType == enMonster && GetAttriFlag().attackToScript) || nType != enMonster )
			{
				CFuBen* pFB = GetFuBen();
				if (pFB && pFB->IsFb())
				{
					// 副本实体受伤事件
					CScriptValueList paramList;
					paramList << (int)CFubenSystem::enOnAttacked;
					paramList << (int)pFB->GetConfig()->nFbConfType;
					paramList << (int)pFB->GetFbId();
					paramList << (int)pFB->GetConfig()->nFbEnterType;
					paramList << pFB;
					paramList << (int)GetSceneID();
					paramList << this;
					paramList << pEntity;
					if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
					{
						OutputMsg(rmError,"[FubenSystem] OnEvent OnAttacked 错误，副本ID=%d，副本类型=%d !",pFB->GetFbId(),pFB->GetConfig()->nFbConfType);
					}
				}
				if (pFB && !(pFB->IsFb()))
				{
					// 触发进入活动区域事件
					int nPosX,nPosY;
					this->GetPosition(nPosX,nPosY);
					CScene* pScene = this->GetScene();
					SCENEAREA* pArea = pScene->GetAreaAttri(nPosX, nPosY);
					if (pScene && pArea && pScene->HasMapAttribute(nPosX,nPosY,aaActivity,pArea))
					{
						static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
						CScriptValueList paramList;
						AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
						for (size_t i = 0; i < pAreaAttr->nCount; i++)
						{
							int nAtvId = pAreaAttr->pValues[i];
							// 个人活动
							if((this->GetType() == enActor) && ((CActor*)this)->GetActivitySystem().IsActivityRunning(nAtvId))
							{
								if (PersonActivtyData* pActivty = ((CActor*)this)->GetActivitySystem().GetActivity(nAtvId))
								{
									paramList.clear();
									paramList << (int)CActivitySystem::enOnAtvAreaAtk;
									paramList << (int)pActivty->nActivityType;
									paramList << (int)pActivty->nId;
									paramList << this;
									paramList << pEntity;
									if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
									{
										OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
									}
								}
							}
							// 全局活动
							else if(GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
							{
								if (GlobalActivityData* pActivty = GetGlobalLogicEngine()->GetActivityMgr().GetActivity(nAtvId))
								{
									paramList.clear();
									paramList << (int)CActivitySystem::enOnAtvAreaAtk;
									paramList << (int)pActivty->nActivityType;
									paramList << (int)pActivty->nId;
									paramList << this;
									paramList << pEntity;
									if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
									{
										OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnEnterArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
									}
								}
							}
						}
					}	
					
				}

				// CActor* pActor = NULL;
				// if (nAttackEntityType == enPet)
				// {
				// 	pActor = ((CPet*)pEntity)->GetMaster();
				// }
				// else if ( nAttackEntityType == enHero )
				// {
				// 	pActor = ((CHero*)pEntity)->GetMaster();
				// }
				// else
				// {
				// 	pActor = (CActor*)pEntity;
				// }
				// CScriptValueList paramList, retList;
				// CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
				// paramList<< (CAnimal*)this << (CActor*)pActor;
				// if (pNpc != NULL)
				// {
				// 	pNpc->GetScript().Call("OnMonsterAttackBy",paramList,retList );
				// }
			}
		}
	}

	if(! HasState(esStateBattle) )
	{
		if (bUpdateAttackStatus)
		{
			AddState(esStateBattle);	
		}
	}
	else
	{	
		// 玩家被非玩家攻击，不推迟离开战斗时间
		bool bPostpone = true;
		if (enActor == GetType() && pEntity->GetType() != enActor)
			bPostpone = false;

		if (bPostpone)
			postponeLeaveCombatTimer();
	}
}

void CAnimal::OnAttackOther(CAnimal * pEntity, bool bChgDura)
{
	if(! HasState(esStateBattle) )
	{
		AddState(esStateBattle );
	}
	else
	{
		// 玩家攻击其他非玩家实体，不推迟离开战斗时间
		bool bPostpone = true;
		if (enActor == GetType() && pEntity->GetType() != enActor)
			bPostpone = false;
		if (bPostpone)
			postponeLeaveCombatTimer();
	}
}

void CAnimal::OnEnterBattle()
{
	bool bPostpone = true;
	if (enActor == GetType())
	{
		EntityHandle eh = GetTarget();
		// 虽然enActor也是为0，这里最好别去掉，以防以后修改枚举类型！ marked by wp 2011.9.15
		if (!eh.IsNull() && eh.GetType() != enActor) 
			bPostpone = false;
	}
	if (bPostpone)
		postponeLeaveCombatTimer();
}

void CAnimal::postponeLeaveCombatTimer()
{	
	m_battleLeftTime = GetGlobalLogicEngine()->getTickCount() + getLeaveCombatTime();	
}

bool CAnimal::CheckCommonOpTickBefore(TICKCOUNT nTick)
{
	if ((m_tOpsTimer.GetNextTime() - nTick) > 10000)
	{
		m_tOpsTimer.SetNextHitTime(nTick);
	}
}

bool CAnimal::CheckCommonOpTick(TICKCOUNT nTick, bool bFailSendmsg, bool bMoveOp)
{
	if (GetType() == enActor && bMoveOp) 
	{	
		return true;
	}

	if( m_tOpsTimer.Check(nTick) ) return true;
	else
	{
		if(GetType() == enActor && bFailSendmsg)
		{
			/*OutputMsg(rmWaning,_T("Fail 下次的OP=%lld,current=%lld"),m_tOpsTimer.GetNextTime(), nTick );*/
			((CActor*)this)->SendOperateResult(false);
		}
		return false;
	}
}

void CAnimal::SetCommonOpNextTime(INT_PTR nNextTime,bool bSendResult2Client ,bool bResult, unsigned int nStep, unsigned int nSendPkgTick, bool bUseGateTime,bool bUseCheckCd,bool bNearAttackSuc)
{	
	if(GetType() == enActor)
	{
		CActor* pActor = (CActor *)this;
		pActor->SetCommonOpNextTimeImpl(nNextTime, bSendResult2Client, bResult, nStep, nSendPkgTick, bUseGateTime,bUseCheckCd,bNearAttackSuc );
	}
	else
	{
		m_tOpsTimer.SetNextHitTimeFromNow(nNextTime);
	}
}

VOID CAnimal::ChangeMP(int nValue)
{
	Inherited::ChangeMP(nValue);
}
 VOID CAnimal::LogicRun(TICKCOUNT nCurrentTime)
 {
	 //DECLARE_FUN_TIME_PROF()
	
	 if(IsInited() ==false ) return;
	  Inherited::LogicRun(nCurrentTime);
	 //3秒钟回血回蓝
	
	 /************************************************************************
	 * 注意：为了提高程序运行效率。这里定时器不要并排执行，按照精度由高到低进行
			 层级嵌套。如果因为逻辑功能需要调整定时精度，需谨慎！！！必要时需要
			 修改层级关系。
	 /************************************************************************/

	 //怪物和宠物才执行移动的检测
	  INT_PTR nType = GetType();
	  switch(nType)
	  {
	  case enNpc:
		  return;
	  case enMonster:
	  case enPet:
	  case enHero:
	  case enActor:
		  {
			  m_moveSystem.OnTimeCheck(nCurrentTime);
			  break;
		  }
	  }
	  
	if (m_t500ms.CheckAndSet(nCurrentTime,true))
	{
		if (m_pAI)
			m_pAI->UpdateAI(nCurrentTime);
		if(m_nDizzyTime && m_nDizzyTime < nCurrentTime)
			RemoveDizzyState();
	}
 
		// 更新观察者、技能子系统、技能AdornFlags
		if (m_tRefViewTimer.CheckAndSet(nCurrentTime, true))
		{
			m_observerSystem.OnTimeCheck(nCurrentTime);	
		} 
 

	 if(m_t1S.CheckAndSet(nCurrentTime, true))
	 {
		 m_skillSystem.OnTimeCheck(nCurrentTime); //技能的吟唱

		//定时检测buff子系统
		m_buffSystem.OnTimeCheck(nCurrentTime); 
		if (HasState(esStateBattle))
		{
			//只有玩家才去掉战斗状态
			if (nCurrentTime >= m_battleLeftTime && nType ==enActor) //要脱离战斗了
			{	
				RemoveState(esStateBattle); //脱离战斗
				/*
				if (m_pAI)
				{
					m_pAI->AttackStop(NULL);
				}
				*/

			}
			

		}
		// 更新效果子系统
		m_effectSystem.OnTimeCheck(nCurrentTime);	

		// 生物回血、回蓝处理
		if( m_t2s.CheckAndSet(nCurrentTime, true))
		{
			/*if(!IsDeath() && nType == enHero) //英雄回血不需要战斗状态
			{
				if(!GetAttriFlag().DenyAutoAddHp)
				{
					static int s_nHpRenewAdd = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nHpRenewAdd;
					static float s_fHpRenewRate =GetLogicServer()->GetDataProvider()->GetGlobalConfig().fHpRenewRate;
					int nMaxHp = 0;
					const CHeroSystem::HERODATA *pHero = ((CHero *)(this))->GetHeroPtr();
					nMaxHp = pHero->nBaseMaxHp;
					int nAddValue = (int) ( ( (int)(nMaxHp *  s_fHpRenewRate) + s_nHpRenewAdd )*(1 + (float)GetProperty<unsigned int>(PROP_CREATURE_HP_RATE_RENEW)/10000 ));
					nAddValue = nAddValue + GetProperty<int>( PROP_CREATURE_HP_RENEW );
					ChangeHP(nAddValue);
				}
				if(!GetAttriFlag().DenyAutoAddMp)
				{
					static int s_nMpRenewAdd = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMpRenewAdd;
					static float s_nMpRenewRate =GetLogicServer()->GetDataProvider()->GetGlobalConfig().fMpRenewRate;
					int nMaxMp = 0;
					const CHeroSystem::HERODATA *pHero = ((CHero *)(this))->GetHeroPtr();
					nMaxMp = pHero->nBaseMaxMp;
					int nAddValue = (int) ( ( (int)(nMaxMp * s_nMpRenewRate)  + s_nMpRenewAdd )*(1 + (float)GetProperty<unsigned int>(PROP_CREATURE_MP_RATE_RENEW)/10000 ));
					nAddValue = nAddValue + GetProperty<int>( PROP_CREATURE_MP_RENEW );
					ChangeMP(nAddValue);
				}		
			}
			else */if(!IsDeath() && !HasState(esStateBattle)) //死了不加血加蓝的
			{
				if (CDynamicBuff * pForbidHpRenewBuff = GetBuffSystem()->GetBuff(aForbidHpRenew, CDynamicBuff::AnyBuffGroup))
				{
					// 施毒术BUFF 禁止回血蓝
				}
				else
				{
					if(!GetAttriFlag().DenyAutoAddHp)
					{
						int nMaxHp = 0;
						if (nType == enActor)
						{
							nMaxHp = ((CActor *)(this))->m_nBase_MaxHp;
						}
						else
						{
							nMaxHp = GetProperty< int>( PROP_CREATURE_MAXHP);
						}

						if (GetProperty<unsigned int>(PROP_CREATURE_HP) < nMaxHp)
						{
							int nAddValue = (int) (nMaxHp * ((float)GetProperty<unsigned int>(PROP_CREATURE_HP_RATE_RENEW)/10000));
							nAddValue = nAddValue + GetProperty<int>( PROP_CREATURE_HP_RENEW );
							ChangeHP(nAddValue);
						}
					}
					if(!GetAttriFlag().DenyAutoAddMp)
					{
						int nMaxMp = 0;
						if (nType == enActor)
						{
							nMaxMp = ((CActor *)(this))->m_nBase_MaxMp;
						}
						else
						{
							nMaxMp = GetProperty< int>( PROP_CREATURE_MAXMP);
						}

						if (GetProperty<unsigned int>(PROP_CREATURE_MP) < nMaxMp)
						{
							int nAddValue = (int) (nMaxMp*((float)GetProperty<unsigned int>(PROP_CREATURE_MP_RATE_RENEW)/10000));
							nAddValue = nAddValue + GetProperty<int>( PROP_CREATURE_MP_RENEW );
							ChangeMP(nAddValue);
						}
					}
				}
			}
		}		
		if(m_nSaviorTime && m_nJZLRCDTime < GetGlobalLogicEngine()->getMiniDateTime())
		{
			UpdateJZLR();
		}
	}
 }

 VOID CAnimal::AfterLogicRun(TICKCOUNT nCurrentTime)
 {	 
	 Inherited::AfterLogicRun(nCurrentTime);
	 INT_PTR i;
	 //处理集中的汇总操作
	 for (i=0; i<CEntityOPCollector::CollectedOperationCount; ++i)
	 {
		 if ( m_OPCollector.isCollected((CEntityOPCollector::CollecteOPType)i) )
		 {
			 DoCollectedOperation((CEntityOPCollector::CollecteOPType)i);
		 }
	 }
	 m_OPCollector.Reset();
 }

 bool CAnimal::_CanAttack(CAnimal *pTargetEntity, CAnimal *pPet, bool boAttackNotice)
 {
	 if ( !pTargetEntity )
	 {
		 return false;
	 }
	 if(IsAttackImmune() || pTargetEntity->IsAttackImmune() ) return false;
	 INT_PTR nSrcType = GetType();
	 INT_PTR nTargetType = pTargetEntity->GetType();
	 if(this == pTargetEntity) return false; //自己不能攻击自己

	 if(nTargetType ==enNpc) return false;

	 if (!pTargetEntity->isAnimal()) return false;
	 
	 if(nSrcType ==enPet )
	 {
		 CPet *pPet = (CPet*)this;
		 if(pPet->GetMaster() == NULL) return false;
		 if (GetAttriFlag().AttackPet)
		 {
			 return true;
		 }
		 return  pPet->GetMaster()->CanAttack(pTargetEntity);
	 }
	 else if(nSrcType ==enHero)
	 {
		 CHero *pHero = (CHero*)this;
		 if(pHero->GetMaster() == NULL) return false;
		 return  pHero->GetMaster()->CanAttack(pTargetEntity);
	 }

	 if( nTargetType ==enPet)
	 {
		 CPet *pPet = (CPet*)pTargetEntity;
		 if(pPet->GetMaster() == NULL) return false;
		 return CanAttack(pPet->GetMaster(), pPet); //能否攻击他的主人
	 }
	 else if(nTargetType ==enHero)
	 {
		 CHero *pHero = (CHero*)pTargetEntity;
		 if(pHero->GetMaster() == NULL) return false;
		 return  CanAttack(pHero->GetMaster());
	 }

	 //不能被攻击
	 if(pTargetEntity->GetAttriFlag().DenyBeAttack)
	 {
		 return false;
	 }

	 //无敌buff不能攻击
	 if( nSrcType == enActor)
	 {
		 //如果玩家杀对方，对方存在pk保护状态，那么不能杀
		 if(  pTargetEntity->GetBuffSystem()->Exists(aPkProtectState) ) return false;

		 //玩家不能功能本阵营的怪物
		
		 if( nTargetType ==enMonster)
		 {
			 if(pTargetEntity->GetCampId()  )
			 {
				  if(pTargetEntity->GetCampId() == GetCampId()) return false;
			 }
		 }
		
		 if(nTargetType ==enActor)
		 {
			  return ((CActor *)this)->GetPkSystem().CanAttack((CActor*) pTargetEntity, pPet, boAttackNotice);
		 }

		 //怪物拒绝被玩家攻击
		 if(pTargetEntity->GetAttriFlag().DenyAttackedByActor)
		 {
			 return false;
		 }


		 if (pTargetEntity->IsMonster() )
		 {
			 CMonster *pMonster = (CMonster *)pTargetEntity;
			 if (pMonster->GetOwner() == GetHandle())  // 不能打归属于自己的怪
				 return false;	

			 //如果配置了行会id
			 if(pTargetEntity->GetNpcGuildId() && GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID) == pTargetEntity->GetNpcGuildId())
				 return false;

			return true;
		 }
	 }
	 else if(nSrcType == enMonster)
	 {
		 // 怪物不能打NPC和采集怪
		 if (enGatherMonster == nTargetType)
		 {	
			 return false;
		 }
		 // 怪物配置了阵营ID，并且目标怪物或者玩家的阵营相同，不能攻击
		 else if (enActor == nTargetType)
		 {
			 //如果配置了行会id
			 if(GetNpcGuildId() && pTargetEntity->GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID) == GetNpcGuildId())
				 return false;
		 }		 
		 // 怪物不打同阵营的玩家
		 if( GetCampId() && nTargetType ==enActor &&pTargetEntity->GetCampId() == GetCampId())
		 {
			return false;
		 }
	 }	 
	
	 //拒绝攻击玩家
	 if(nTargetType == enActor && GetAttriFlag().DenyAttackActor)
	 {
		return false;
	 }
	 
	 //如果目标是一个怪物，并且没有主人
	 if(nTargetType == enMonster && /*((CMonster*)pTargetEntity)->GetOwner().IsNull()) && */!GetAttriFlag().CanAttackMonster )
	 {
		 return false;
	 }
	
	 if(nSrcType ==enMonster) //如果释放者是一个怪物
	 {
		 return true;
	 }
	 return false;
 }

bool CAnimal::CanAttack(CAnimal *pTargetEntity, CAnimal *, bool)
{
	// 同阵营判断
	if(pTargetEntity->GetCampId())
	{
		if(pTargetEntity->GetCampId() == GetCampId()) return false;
	}

	// 怪物的不能被攻击标志判断
	if (pTargetEntity->IsMonster())
	{
		if (pTargetEntity->GetAttriFlag().DenyBeAttack)
		{
			return false;
		}
		return true;
	}
	else
	{
		// 如果A是宠物，则看主人能否攻击目标
		if (GetType() == enPet)
		{
			CPet *pSelf = (CPet*)this;
			if(pSelf->GetMaster() == NULL) return false;
			return pSelf->GetMaster()->CanAttack(pTargetEntity);
		}
		// 如果A是玩家
		else if (GetType() == enActor)
		{
			CActor *pSelf = ((CActor *)this);
			// 如果B是宠物
			if (pTargetEntity->GetType() == enPet)
			{
				CPet *pTarPet = (CPet*)pTargetEntity;
				if(pTarPet->GetMaster() == NULL) return false;
				return pSelf->GetPkSystem().CanAttack(pTarPet->GetMaster(), pTarPet)
					&& pSelf->GetPkSystem().IsUnLimited(pTargetEntity);
			}
			// 如果B是玩家
			else// if (pTargetEntity->GetType() == enActor)
			{
				return pSelf->GetPkSystem().CanAttack((CActor*)pTargetEntity)
					&& pSelf->GetPkSystem().IsUnLimited(pTargetEntity);
			}
		}else
		{
			return true;
		}
		
	}
}

bool CAnimal::IsFriend(CAnimal *pTargetEntity)
{
	// 同阵营判断
	if(pTargetEntity->GetCampId())
	{
		if(pTargetEntity->GetCampId() == GetCampId()) return false;
	}

	// 怪物的不能被攻击标志判断
	if (pTargetEntity->IsMonster() && pTargetEntity->GetAttriFlag().DenyBeAttack)
	{
		return false;
	}

	// 如果A是宠物，则看主人能否攻击目标
	if (GetType() == enPet)
	{
		CPet *pSelf = (CPet*)this;
		if(pSelf->GetMaster() == NULL) return false;
		return pSelf->GetMaster()->CanAttack(pTargetEntity);
	}
	// 如果A是玩家
	else if (GetType() == enActor)
	{
		CActor *pSelf = ((CActor *)this);
		// 如果B是宠物
		if (pTargetEntity->GetType() == enPet)
		{
			CPet *pTarPet = (CPet*)pTargetEntity;
			if(pTarPet->GetMaster() == NULL) return false;
			return pSelf->GetPkSystem().CanAttack(pTarPet->GetMaster());
		}
		// 如果B是玩家
		else// if (pTargetEntity->GetType() == enActor)
		{
			return pSelf->GetPkSystem().CanAttack((CActor*)pTargetEntity);
		}
	}
	return true;
}

 VOID CAnimal::DoCollectedOperation(CEntityOPCollector::CollecteOPType eOPType)
 {
	 INT_PTR nEntityType;
	 switch(eOPType)
	 {
	 case CEntityOPCollector::coRefAbility:
		 //TODO:实现刷新人物、被动技能、装备、经脉、BUFF等属性的全部属性刷新功能，并向角色发送属性变更的消息
//#pragma __CPMSG__(TODO:实现刷新人物、被动技能、装备、经脉、BUFF等属性的全部属性刷新功能，并向角色发送属性变更的消息)
		 GetPropertySystem().ResertProperty(); //重置属性
		 break;
	 case CEntityOPCollector::coRefFeature:
		 nEntityType =GetType();
		 if(nEntityType == enActor  )
		 {
			 ((CActor*)this)->GetEquipmentSystem().RefreshAppear(); //刷新外形
		 }
		 break;
	 case CEntityOPCollector::coTransport: //这个玩家是传送过来的，通知其他人，需要播放一个特效
		
			 //((CActor*)this)->NotifyNearbyTransport();
		this->GetMoveSystem()->SetMoveFlag( CMovementSystem::mfTransport, false); //设置为传送标记
		break;
	 case CEntityOPCollector::coResetMaxHPMP:
		 {			 
			 SetProperty(PROP_CREATURE_HP, GetProperty<unsigned int>(PROP_CREATURE_MAXHP));
			 SetProperty(PROP_CREATURE_MP, GetProperty<unsigned int>(PROP_CREATURE_MAXMP));
		 }
		 break;
	 case CEntityOPCollector::coRefMonsterDynProp:
		 {
			  GetPropertySystem().ResertProperty(); //重置属性
			  // 将怪物HP设置为满
			  //if (!HasState(esStateBattle))
			  //{
				  //SetProperty(PROP_CREATURE_HP, GetProperty<unsigned int>(PROP_CREATURE_MAXHP));				  
			  //}
			  break;
		 }
	 default:
		 break;
	 }
 }
 


 void CAnimal::ProcessEntityMsg(const CEntityMsg &msg)
 {
	 char buff[256];
	 CEntity * pEntity;
	 int x,y,dis;

	 //如果消息已经被写坏了，则不执行
	 if(msg.bIsUsed ==false)
	 {
		 OutputMsg(rmError,"name=%s,msgid=%d,ProcessEntityMsg is invalid",GetEntityName(),(int)msg.nMsg);
		 return ;
	 }
	 
	 switch(msg.nMsg)
	 {
	 case CEntityMsg::emRealLaunchSkill:
		{
			m_skillSystem.RealLaunchSkill((int)msg.nParam1,(CScene*)msg.pParam2,(int)msg.nParam3,
				(int)msg.nParam4,(int)msg.nParam5,(CEntity *)msg.pParam6,
				(int)msg.nParam7,(int)msg.nParam8);
		}
		break;
	case CEntityMsg::emSkillApplyOneRange:
		{
			m_skillSystem.SkillApplyOneRange((int)msg.nParam1,
				(CSkillSubSystem::PSKILLDATA)msg.pParam2,(PSKILLONERANGE)msg.pParam3,
				(CEntity *)msg.pParam4,(CScene*)msg.pParam5,(int)msg.nParam6,(int)msg.nParam7,
				(int)msg.nParam8,(int)msg.nParam9,(int)msg.nParam10);
		}
		break;
	case CEntityMsg::emShowSceneEffect:
		{
			CDataPacket data(buff,sizeof(buff));
			data << (BYTE)enDefaultEntitySystemID <<(BYTE) sAddSceneEffect ;
			data <<  (Uint64)msg.nSender;
			data << (WORD) msg.nParam1; //特效ID
			data << (WORD) msg.nParam2  << (WORD) msg.nParam3;  //x,y
			m_observerSystem.BroadCast(data.getMemoryPtr(), data.getPosition(),true);
		}
		break;
	 case CEntityMsg::emShowEntityEffect:
		{
			CDataPacket data(buff,sizeof(buff));
			data << (BYTE)enDefaultEntitySystemID <<(BYTE) sAddEffect;
			data << (Uint64)msg.nSender << (Uint64)GetHandle();
			data << (WORD) msg.nParam1; //特效ID
			data <<(unsigned int)msg.nParam3;//
			m_observerSystem.BroadCast(data.getMemoryPtr(), data.getPosition(),true);
			//m_effectSystem.AddEffect( msg.nParam1,msg.nParam2,msg.nParam3); //添加特效
		}
		break;
	case CEntityMsg::emSkillResult:
		{
			CAnimal* pSrc = (CAnimal*)GetEntityFromHandle(msg.nSender);
			if (!pSrc) return;

			CSkillResult * result=m_skillSystem.GetSkillResult();
			SKILLRESULTPARAM skillParam; //技能的参数
			skillParam.pTargetEntity = this;
			skillParam.pSrcEntity = pSrc;
			skillParam.nSkillID =  msg.nParam1;
			skillParam.nTargetPosX =  (WORD)msg.nParam2;
			skillParam.nTargetPosY =  (WORD)msg.nParam3;
			skillParam.btIgnoreDefence  =  (BYTE)msg.nParam4;
			skillParam.btIgnoreMaxDropHp  =  (BYTE)msg.nParam5;
					
			SKILLRESULT  sresult; //技能的结果
			sresult.nDelay = 0;
			sresult.nValue = msg.nParam5; 
			sresult.nId = msg.nParam6;
			sresult.nResultType = msg.nParam7;
			
			skillParam.pSkillResult = &sresult;
			result->DoResult(&skillParam,true); //无延迟立刻执行
		}
		break;

	 case CEntityMsg::emChangeHp:
		 if (msg.nParam1 > 0 || !IsDeath())
			ChangeHP((int)msg.nParam1);
		 break;
	 case CEntityMsg::emReUseMonster:
		 {
			 Reuse();
			 break;
		 }
	case CEntityMsg::emEntityTransfer:
		 {
			 int x,y;
			 GetPosition(x,y);
			 CObserverSystem* obs = GetObserverSystem();
			 TICKCOUNT tc = GetGlobalLogicEngine()->getTickCount();
			 obs->OnTimeCheck(tc);//先获取所有的可视玩家，因为在之前有可能被清空了
			GetMoveSystem()->BroadcastMove(x,y,sActorTransport);			
			break;
		 }
	 case CEntityMsg::emEntityScriptDelay:
		 {			 
			 OnEntityScriptDelayMsgCallback((unsigned int)(msg.nParam1), (int)msg.nParam2, (int)msg.nParam3, (int)msg.nParam4, (int)msg.nParam5, (int)msg.nParam6);
			 break;
		 }
	 case CEntityMsg::emEntityDead:
		 {
			 if(GetProperty<unsigned int>(PROP_CREATURE_HP) == 0)
			 {
				  OnEntityDeath();
			 }
			 break;
		 }
	 case CEntityMsg::emRealTransfer:
		 {
			 GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(this,(CFuBen *)NULL,msg.nParam1,msg.nParam2,msg.nParam3);
			 break;
		 }
	 default:
		 Inherited::ProcessEntityMsg(msg);
		 break;
	}
 }

 void  CAnimal::DestroyAi()
 {
	 if(m_pAI)
	 {
		 GetGlobalLogicEngine()->GetAiMgr().DestroyAi(m_pAI); //删除ai
		 m_pAI =NULL;
	 }
 }



 //初始化状态的禁止表，避免每次循环调用都去搞一下
 void CAnimal::InitStateForbidMask()
 {
	 if(s_stateForbidMask ) return ; //已经初始化好了
	 s_stateForbidMask = new unsigned int [esMaxStateCount];
	 //先都清0
	 memset(s_stateForbidMask,0,esMaxStateCount * sizeof(unsigned int));

	 //死亡状态
	 CAnimal::s_stateForbidMask[esStateDeath] = 0xFFFFFFFF; //死亡状态任何都禁止
	 unsigned int nMask=0;
	 
	 //移动状态禁止摆摊，打坐，和吟唱
	 CAnimal::SetFlag<unsigned int>( nMask,esStateStall,true );
	 CAnimal::SetFlag<unsigned int>( nMask,esStateSing,true );
	 CAnimal::s_stateForbidMask[esStateMove] = nMask;
	 
	 //禁止移动状态
	 nMask =0;
	 CAnimal::SetFlag<unsigned int>( nMask,esStateMove,true );
	 CAnimal::s_stateForbidMask[esStateMoveForbid] = nMask;

	 //摆摊状态的一些禁止状态
	 nMask =0;
	 CAnimal::SetFlag<unsigned int>( nMask,esStateMove,true );
	 CAnimal::SetFlag<unsigned int>( nMask,esStateSing,true );
	 CAnimal::s_stateForbidMask[esStateStall] = nMask;

	 //晕眩状态
	 nMask =0;
	 CAnimal::SetFlag<unsigned int>( nMask,esStateMove,true );
	 CAnimal::SetFlag<unsigned int>( nMask,esStateSing,true );
	 CAnimal::SetFlag<unsigned int>( nMask,esStateStall,true );
	 CAnimal::s_stateForbidMask[esStateDizzy] = nMask;

	 //吟唱状态
	 nMask = 0;
	 CAnimal::SetFlag<unsigned int>(nMask, esStateBattle, true);
	 CAnimal::SetFlag<unsigned int>(nMask, esStateSing, true);
	 CAnimal::s_stateForbidMask[esStateSing] = nMask;
 }
void CAnimal::DestroyStateForbidMask()
{
	//删除这个静态变量
	if(s_stateForbidMask != NULL)
	{
		delete[] s_stateForbidMask;
	}
};

void CAnimal::Reuse()
{
	//清除所有buff
	CBuffSystem* pBuff = GetBuffSystem();
	if (pBuff)
	{
		pBuff->Clear(false);
	}

	//删除所有的消息
	//ClearDelayDelEntityMsg();

	//重置属性
	GetPropertySystem().ResertProperty();
	ClearDestoryTime();

}

void CAnimal::Say(MonsterSayBCType nStyle, const char *pMsg, int tipType, unsigned int nLimitLev)
{
	if (!pMsg || strlen(pMsg) <= 0) return;
	CVector<EntityHandle> vecNearEntityList, vecBCEntityList;
	switch (nStyle)
	{
	case mssNear:
		GetNearActorList(vecBCEntityList);
		break;
	case mssScene:
		GetSceneActorList(GetScene(), vecBCEntityList);
		break;
	case mssFuben:
		GetFubenActorList(vecBCEntityList);
		break;
	case mssWorld:
		GetWorldActorList(vecBCEntityList);
		break;
	case mssCamp:
		GetCampActorList(vecBCEntityList);
		break;
	case mssSelf:
		vecNearEntityList.add(GetHandle());
		break;
	default:
		break;
	}

	INT_PTR nCount = 0;
	size_t nLen = 1024;
	// 对于阵营广播，直接广播在中间
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
	if (mssCamp == nStyle)
	{
		nCount = vecBCEntityList.count();		
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CEntity *pEntity = em->GetEntity(vecBCEntityList[i]);
			if (pEntity && pEntity->GetType() == enActor 
				&& pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) >= nLimitLev)
			{
				((CActor *)pEntity)->SendTipmsg(pMsg, tipType);
			}
		}
		return;
	}

	// 附近频道不在系统频道显示(如果当前是频道类型广播，并且当前不是附近频道)
	if(GetType() != enActor && (ttChatWindow & tipType) && nStyle != mssNear)
	{
		nCount = vecBCEntityList.count();
		if (nCount <= 0) return;	
		char buff[1124]={0};
		CDataPacket outPack(buff,sizeof(buff));
		outPack << (BYTE)enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelSystem;		
		outPack.writeString(GetEntityName());		
		if (strlen(pMsg) < nLen) nLen = strlen(pMsg);
		outPack.writeString(pMsg, nLen);
		outPack << (BYTE)0 << (BYTE)0 << (int)0 << (BYTE)0 << (BYTE)0 ;
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CEntity *pEntity = em->GetEntity(vecBCEntityList[i]);
			if (pEntity && pEntity->GetType() == enActor 
				&& pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) >= nLimitLev)
			{
				((CActor *)pEntity)->SendData(outPack.getMemoryPtr(), outPack.getPosition());
			}
		}
		return;
	}	

	// 附近头顶泡泡
	CVector<EntityHandle> *pEntityList ;
	/*
	if(mssNear == nStyle)
	{
		GetNearActorList(vecNearEntityList);
		pEntityList = &vecNearEntityList;
	}
	else
	{
		pEntityList = &vecBCEntityList;
	}
	*/
	pEntityList = &vecBCEntityList;
	
	nCount = pEntityList->count();	
	if (nCount > 0)
	{
		//如果是玩家的话，比如拾取的时候给玩家的广播
		
		if( GetType() == enActor)
		{
			for (INT_PTR  i = 0; i <  pEntityList->count(); i++)
			{
				CEntity *pEntity = em->GetEntity( (*pEntityList)[i]);
				if (pEntity && pEntity->GetType() == enActor 
					&& pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) >= nLimitLev)
				{
					((CActor *)pEntity)->SendTipmsg(pMsg, tipType);
				}
			}
			return;
		}

		
		char buff[1124]={0};
		CDataPacket outPack(buff,sizeof(buff));

		bool isMonsterNearSay = (mssNear == nStyle); //是怪物在附近广播

		if( isMonsterNearSay )
		{
			outPack << (BYTE)enChatSystemID << (BYTE)sNonPlayerChatNear;
			outPack << (INT64)GetHandle();
			if (strlen(pMsg) < nLen) nLen = strlen(pMsg);
			outPack.writeString(pMsg, nLen);
			outPack << (BYTE)(tipType == ttChatWindow ? 0 : 1);
		}

		for (INT_PTR  i = 0; i <  pEntityList->count(); i++)
		{
			CEntity *pEntity = em->GetEntity( (*pEntityList)[i]);
			if (pEntity && pEntity->GetType() == enActor &&pEntity->IsInited() 
				&& pEntity->GetProperty<unsigned int>(PROP_CREATURE_LEVEL) >= nLimitLev)
			{
				if (isMonsterNearSay)
				{
					((CActor *)pEntity)->SendData(outPack.getMemoryPtr(), outPack.getPosition());
				}
				else
				{
					((CActor *)pEntity)->SendTipmsg(pMsg, tipType);
				}
			}
		}
	}
}

void CAnimal::GetNearActorList(CVector<EntityHandle> &vecEntityList)
{
	//GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(this,vecEntityList);

	//CObserverEntityList &otherList = GetObserverSystem()->GetVisibleList();
	CObserverEntityList otherList;
	GetGlobalLogicEngine()->GetFuBenMgr()->GetVisibleList(this, otherList);
	{
		CLinkedListIterator<EntityHandleTag> iter(otherList);
		CLinkedNode<EntityHandleTag> *pNode = 0;
		for (pNode = iter.first(); pNode; pNode = iter.next())
		{
			CEntity *pEntity = GetEntityFromHandle(pNode->m_Data.m_handle);
			if (pEntity && pEntity->GetType() == enActor)
			{
				vecEntityList.add(pEntity->GetHandle());
			}
		}
	}
}

void CAnimal::GetSceneActorList(CScene *pScene, CVector<EntityHandle> &vecEntityList)
{	
	if (!pScene) return;
	CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CEntityList &list = pScene->GetPlayList();
	CLinkedListIterator<EntityHandle> it(list);
	CLinkedNode<EntityHandle> *pNode = NULL;
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		CEntity *pEntity = pEntityMgr->GetEntity(pNode->m_Data);
		if (pEntity && enActor == pEntity->GetType())
			vecEntityList.add(pEntity->GetHandle());
	}
}

void CAnimal::GetFubenActorList(CVector<EntityHandle> &vecEntityList)
{
	CScene *pScene = GetScene();
	if (!pScene) return;
	CFuBen *pFb = pScene->GetFuBen();
	if (!pFb || !pFb->IsFb()) return;
	CFuBen::SceneList& list = pFb->GetSceneList();
	for (INT_PTR i = 0; i < list.count(); i++)
	{
		CScene *pScene = list[i];
		GetSceneActorList(pScene, vecEntityList);
	}
}

void CAnimal::GetWorldActorList(CVector<EntityHandle> &vecEntityList)
{
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrHandleList(vecEntityList);
}

void CAnimal::GetCampActorList(CVector<EntityHandle> &vecEntityList)
{
	int nCampId = GetCampId();
	if (nCampId == INVALID_CAMP_ID) return;

	CVector<EntityHandle> vecList;
	CEntityManager *pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
	pEntityMgr->GetOnlineAcotrHandleList(vecList);
	for (INT_PTR i = 0; i < vecList.count(); i++)
	{
		CActor *pActor = (CActor *)GetEntityFromHandle(vecList[i]);
		if (pActor && pActor->IsInited() && pActor->GetCampId() == nCampId)
			vecEntityList.push(vecList[i]);
	}
}

//发送复活戒指的CD时间
void CAnimal::SendReliveRingCd()
{
	if(GetType() == enActor )		//玩家
	{
		TICKCOUNT tickNow = GetLogicCurrTickCount();
		if( m_nextHpReliveCd >= tickNow )
		{
			CActorPacket pack ;
			((CActor*)this)->AllocPacket(pack);
			pack << (BYTE) enEuipSystemID <<(BYTE)sSendReliveRingCd;
			pack << (unsigned short)(m_nextHpReliveCd - tickNow );
			pack.flush();
		}
	}
}

//设置麻痹
void CAnimal::SetDizzyTime(int nTime)
{
	TICKCOUNT tickNow = GetLogicCurrTickCount();
	m_nDizzyTime = tickNow + nTime;
	SetDizzyState();
}

//更新救主灵刃特效以及当前CD时间
void CAnimal::UpdateJZLR()
{
	std::vector<int>::iterator iter = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vJZLRBuff.begin();
	for (; iter != GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.vJZLRBuff.end(); ++iter)
	{
		CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(*iter);
		if(pBUFFCONFIG)
		{
			CDynamicBuff* pDynamicBuff = GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
			if(!pDynamicBuff)
			{
				GetBuffSystem()->Append(*iter);
			}
		}
	}
}