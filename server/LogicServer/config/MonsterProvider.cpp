#include "StdAfx.h"
#include "MonsterConfig.h"
#include "MonsterProvider.h"
#include "../misc/caches/MonsterConfigCacher.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

LPCTSTR CMonsterProvider::MonsterCacheFile = _T("./data/runtime/cache/Monsters.cch");
const TCHAR* MONSTER_PROP_PRIOR_ATTACK_TARGET = _T("priorTarget");

int			  CMonsterProvider::s_nPropCount = 0;
CREATURBATTLEEDATA* CMonsterProvider::s_pPropConfig = NULL;
int			  CMonsterProvider::s_nFlagCount = 0;
EntityFlags*  CMonsterProvider::s_pFlagConfig = NULL;

CMonsterProvider::CMonsterProvider()
	:Inherited(), Inherited2(), m_DataAllocator(_T("MonsterDataAlloc"))
{
	m_bossGrowupHasChange =false;
	sNameNoShowNumber[0] = 0;
}

CMonsterProvider::~CMonsterProvider()
{
	if (s_pPropConfig)
	{
		GetLogicServer()->GetBuffAllocator()->FreeBuffer(s_pPropConfig);
		s_pPropConfig = NULL;
	}
	
	if (s_pFlagConfig)
	{
		GetLogicServer()->GetBuffAllocator()->FreeBuffer(s_pFlagConfig);
		s_pFlagConfig = NULL;
	}
}

/*
file是\data\config\activity\boss.txt
注意：在XGame中，BOSS相关功能，已经不通过BossConfig的配置了
*/
bool CMonsterProvider::LoadBossConfig(LPCTSTR sFilePath)
{
	// if (PreLoad(sFilePath))
	// {
	// 	m_bossInfoList.clear();
	// 	if(openGlobalTable("BossConfig"))
	// 	{
	// 		if (feildTableExists("Boss") && openFieldTable("Boss"))
	// 		{
	// 			BOSSINFO bossInfo;
	// 			if (enumTableFirst())
	// 			{
	// 				do 
	// 				{
	// 					bossInfo.nId = (WORD)getFieldInt("entityId");
	// 					bossInfo.btType = (byte)getFieldInt("bossType");
	// 					bossInfo.nSceneId = (WORD)getFieldInt("mobSceneId");
	// 					m_bossInfoList.add(bossInfo);
	// 				} while (enumTableNext());
	// 			}
	// 			closeTable();
	// 		}
	// 		closeTable();
	// 	}
	// 	setScript(NULL);
	// }
	// return true;
}

bool CMonsterProvider::PreLoad(LPCTSTR sFilePath)
{
	bool Result = false;

	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	LPCTSTR sText = NULL;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);

		//设置脚本内容
		if ( !setScript(sText) )
			showErrorFormat(_T("syntax error on %s config"), sFilePath);
		Result = true;
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Monster common config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp && sText)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load PreLoad config"));
	}

	return Result;
}

bool CMonsterProvider::LoadMonsters(LPCTSTR sFilePath, bool bBackLoad)
{
	if (!PreLoad(sFilePath))
	{
		return false;
	}
	if (!openGlobalTable("StdMonster"))
	{
		setScript(NULL);
		return false;
	}
	
	// 加载属性组
	if (feildTableExists("Props") && openFieldTable("Props"))
	{
		if (s_nPropCount = lua_objlen(m_pLua, -1))
		{
			int nSize = sizeof(CREATURBATTLEEDATA) * s_nPropCount;
			s_pPropConfig = (CREATURBATTLEEDATA*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(nSize);
			ZeroMemory(s_pPropConfig, nSize);
		}

		int nDef =0;
		double fDef =0.0f;
		int nPos = 0;
		if ( enumTableFirst() )
		{
			do
			{
				int nPropId = getFieldInt("id");
				nPos = nPropId - 1;
				if (nPos >= s_nPropCount || nPos < 0)
				{
					OutputMsg(rmError,_T("load Monster Prop id error!nPropId=%d"),nPropId);
					return false;
				}

				CREATURBATTLEEDATA* data = s_pPropConfig + nPos;

				data->nMaxHp = getFieldInt("nMaxHpAdd",&nDef);
				data->nMaxMp = getFieldInt("nMaxMpAdd",&nDef);

				data->nPhysicalAttackMin = getFieldInt("nPhysicalAttackMinAdd",&nDef);
				data->nPhysicalAttackMax = getFieldInt("nPhysicalAttackMaxAdd",&nDef);

				data->nMagicAttackMin = getFieldInt("nMagicAttackMinAdd",&nDef);
				data->nMagicAttackMax = getFieldInt("nMagicAttackMaxAdd",&nDef);

				data->nWizardAttackMin = getFieldInt("nWizardAttackMinAdd",&nDef);
				data->nWizardAttackMax = getFieldInt("nWizardAttackMaxAdd",&nDef);

				data->nHysicalDefenceMin = getFieldInt("nPhysicalDefenceMinAdd",&nDef);
				data->nHysicalDefenceMax = getFieldInt("nPhysicalDefenceMaxAdd",&nDef);

				data->nMagicDefenceMin = getFieldInt("nMagicDefenceMinAdd",&nDef);
				data->nMagicDefenceMax = getFieldInt("nMagicDefenceMaxAdd",&nDef);

				data->nHitrate = getFieldInt("nHitValueAdd",&nDef);
				data->nDogerate = getFieldInt("nDogValueAdd",&nDef);

				data->nMagicHitRate = getFieldInt("nMagicHitRateAdd",&nDef);
				data->nMagicDogerate = getFieldInt("nMagicDogerateAdd",&nDef);

				data->nHpRateRenew =(float) getFieldNumber("fHpRateRenew",&fDef);
				data->nMpRateRenew = (float)getFieldNumber("fMpRateRenew",&fDef);

				nDef = 1600;
				data->nMoveOneSlotTime = getFieldInt("nMoveOneSlotTime",&nDef);
				data->nAttackSpeed = getFieldInt("nAttackSpeed",&nDef);

				data->nLuck =0;
				data->nColor =0;

				nDef = 0;
				data->nHpRenew = getFieldInt("nHpRenew",&nDef);
				data->nMpRenew = getFieldInt("nMpRenew",&nDef);
				data->nDeductDizzyRate = getFieldInt("aDeductDizzyRate",&nDef);
				data->nDizzyTimeAbsorbAdd = getFieldInt("aDizzyTimeAbsorbAdd",&nDef);
			} while (enumTableNext());
		}
		closeTable();
	}
	else
	{
		setScript(NULL);
		return false;
	}
	
	// 加载标志组
	if (feildTableExists("Flags") && openFieldTable("Flags"))
	{
		if (s_nFlagCount = lua_objlen(m_pLua, -1))
		{
			int nSize = sizeof(EntityFlags) * s_nFlagCount;
			s_pFlagConfig = (EntityFlags*)GetLogicServer()->GetBuffAllocator()->AllocBuffer(nSize);
			ZeroMemory(s_pFlagConfig, nSize);
		}

		bool boDefVal = false;
		bool boDefValTrue = true;
		int nPos = 0;
		if ( enumTableFirst() )
		{
			do
			{
				int nFlagId = getFieldInt("id");
				nPos = nFlagId - 1;
				if (nPos >= s_nFlagCount || nPos < 0)
				{
					OutputMsg(rmError,_T("load Monster Flag id error!nFlagId=%d"),nFlagId);
					return false;
				}

				EntityFlags* data = s_pFlagConfig + nPos;
				data->DenyMove= getFieldBoolean("DenyMove", &boDefVal);
				data->DenyBeMove= getFieldBoolean("DenyBeMove", &boDefVal);
				data->DenyAttackActor= getFieldBoolean("DenyAttackActor", &boDefVal);
				data->DenyBeAttack= getFieldBoolean("DenyBeAttack", &boDefVal);
				data->DenyAttackedByActor= getFieldBoolean("DenyAttackedByActor", &boDefVal);
				data->CanAttackMonster= getFieldBoolean("CanAttackMonster", &boDefVal);
				data->DenyAutoAddMp= getFieldBoolean("DenyAutoAddMp", &boDefVal);
				data->DenyAutoAddHp= getFieldBoolean("DenyAutoAddHp", &boDefVal);
				data->CanSceneTelport= getFieldBoolean("CanSceneTelport", &boDefVal);
				data->CanSeeHideActor= getFieldBoolean("CanSeeHideActor", &boDefVal);
				data->DenyDieSubExp= getFieldBoolean("DenyDieSubExp", &boDefVal);
				data->DenyUseNearAttack= getFieldBoolean("DenyUseNearAttack", &boDefVal);
				data->DenyDizzyBuff= getFieldBoolean("DenyDizzyBuff", &boDefVal);
				data->DenyMoveForbidBuff= getFieldBoolean("DenyMoveForbidBuff", &boDefVal);
				data->DenyInnnerSkillForbidBuff= getFieldBoolean("DenyInnnerSkillForbidBuff", &boDefVal);
				data->DenyOutSkillForbidBuff= getFieldBoolean("DenyOutSkillForbidBuff", &boDefVal);
				data->DenyMoveSpeedSlowBuff= getFieldBoolean("DenyMoveSpeedSlowBuff", &boDefVal);
				data->DenySee= getFieldBoolean("DenySee", &boDefVal); //是否拒绝看到
				data->DenyInitiativeAttackActor= getFieldBoolean("DenyInitiativeAttackActor", &boDefVal); 
				data->DenyShowMasterName= getFieldBoolean("DenyShowMasterName", &boDefVal); 
				data->DenyShowName = getFieldBoolean("DenyShowName", &boDefVal);
				data->AttackSeeKill = getFieldBoolean("AttackSeeKill", &boDefVal);		//死亡见证
				data->noBeCrossed  = getFieldBoolean("noBeCrossed",	&boDefVal);	//是否不可穿（0-可穿，1-不可穿）
				// data->CanGrowUp = getFieldBoolean("CanGrowUp", &boDefVal);
				data->AttackKiller = getFieldBoolean("AttackKiller", &boDefVal);
				data->ShowNameByNumber = getFieldBoolean("ShowNameByNumber", &boDefVal);
				data->DenyBeCatch = getFieldBoolean("DenyBeCatch", &boDefVal);
				data->CanAlwaysEnter = getFieldBoolean("CanAlwaysEnter", &boDefVal);
				data->BeAttackNeedUseSkill = getFieldBoolean("BeAttackNeedUseSkill", &boDefVal);
				data->IdleUseSkill = getFieldBoolean("IdleUseSkill", &boDefVal);
				data->NeedSaveToLog = getFieldBoolean("NeedSaveToLog", &boDefVal);
				data->PetNoAttact = getFieldBoolean("PetNoAttact", &boDefVal);
				data->DenyAddNumber = getFieldBoolean("DenyAddNumber", &boDefVal);
				data->AttackPet = getFieldBoolean("AttackPet", &boDefVal);
				data->NoUseAStar = getFieldBoolean("noUseAStar", &boDefVal);
				data->MoveTelport = getFieldBoolean("moveTelport", &boDefVal);
				data->noAttackBack = getFieldBoolean("NoAttactBack", &boDefVal);
				data->noReturnHome = getFieldBoolean("noReturnHome",&boDefVal);
				data->attackToScript = getFieldBoolean("attackToScript",&boDefVal);
				data->DenySetDir = getFieldBoolean("DenySetDir", &boDefVal);
				data->bDeathTriggerScript = getFieldBoolean("bDeathTriggerScript", &boDefVal);
				data->bMobTriggerScript = getFieldBoolean("bMobTriggerScript", &boDefVal);
				data->bLiveTimeOutTriggerScript = getFieldBoolean("bLiveTimeOutTriggerScript", &boDefVal);
				data->boShowVestEntityName = getFieldBoolean("showVestEntityName", &boDefVal);
				
				//配置为CanGrowUp 为 true的BOSS就是成长BOSS
				// if( data->CanGrowUp )
				// {
				// 	BossGrowData stGrowBoss;
				// 	stGrowBoss.nMonsterId = oneMonster->nEntityId;
				// 	m_growBossList.add(stGrowBoss);
				// }

			} while (enumTableNext());
		}
		closeTable();
	}
	else
	{
		setScript(NULL);
		return false;
	}

	// 加载属性组
	if (feildTableExists("Drops") && openFieldTable("Drops"))
	{
		closeTable();
	}
	
	// 加载怪物
	if (feildTableExists("Monster") && openFieldTable("Monster"))
	{
		CBaseList<MONSTERCONFIG> monsterList;
		const INT_PTR nCount = lua_objlen(m_pLua, -1) + 1;
		if ( nCount <= 0 )
		{
			setScript(NULL);
			showError(_T("no monster data on monster config"));
			return false;
		}
		monsterList.reserve(nCount);
		MONSTERCONFIG *pMonsters= monsterList;
		ZeroMemory(pMonsters, sizeof(MONSTERCONFIG) * nCount);

		if ( enumTableFirst() )
		{
			INT_PTR nIdx = 0;
			CDataAllocator dataAllocator;
			do 
			{
				nIdx = getFieldInt("entityid");

				if (nIdx >= nCount)
				{
					OutputMsg(rmError, _T("monster index  overflow ,monster_id = %d"), nIdx);
					endTableEnum();
					break;
				}

				MONSTERCONFIG * pMonster = &monsterList[nIdx];

				//读取1个技能的配置配置数据
				if(ReadOneMonster( dataAllocator,pMonster) ==false)
				{
					OutputMsg(rmError,_T("monster config error, monsterID=%d "), pMonster->nEntityId);
					return false;
				}

				//如果该ID不是期望的值，则输出错误并终止读取
				// if (pMonster->nEntityId != nIdx)
				// {
				// 	showErrorFormat(_T("unexpected monster index %d,  %d expected"), pMonster->nEntityId, nIdx);
				// 	endTableEnum();
				// 	break;
				// }
				// nIdx++;
			}
			while (enumTableNext());

			completeRead(pMonsters, nCount, dataAllocator, bBackLoad);
		}

		closeTable();
		setScript(NULL);
		return true;
	}
	else
	{
		setScript(NULL);
		return false;
	}
		
	closeTable();
}

bool CMonsterProvider::LoadNpcs(LPCTSTR sFilePath)
{
	if (PreLoad(sFilePath))
	{
		if (openGlobalTable("Npc"))
		{
			m_npcList.clear();
			size_t nNpcCount = lua_objlen(m_pLua, -1);
			
			if (nNpcCount > 0)
			{
				m_npcList.reserve(nNpcCount);
				m_npcList.trunc(nNpcCount);
				if (enumTableFirst())
				{
					int nId = 0;
					int nDef = 0;
					do
					{
						nId = getFieldInt("id", &nDef);
						PNPCCONFIG pNpc = &m_npcList[nId-1];
						::new(pNpc) NPCCONFIG();
						if (pNpc->nId == nId)
						{
							OutputMsg(rmError, _T("NPC的ID重复了，当前ID为:%d，预期ID为%d"), pNpc->nId, nId);
							return false;
						}
						pNpc->nId = nId;
						BOOL isNameValid = false;
						strncpy(pNpc->sName, getFieldString("name","",&isNameValid), ArrayCount(pNpc->sName));
						if (!isNameValid)
						{
							OutputMsg(rmError,"Npc Id = %d name is invalid,please check Lang",pNpc->nId);
							return false;
						}
						strncpy(pNpc->sTalk, getFieldString("talk","hello!!!",&isNameValid), ArrayCount(pNpc->sTalk));
						pNpc->nModelId = getFieldInt("modelid");
						int nIconDefault = 1;
						pNpc->nIcon = getFieldInt("icon",&nIconDefault);
						int nDefFuncType = 0;
						int nDef_0 = 0;
						bool boDefault = false;
						//pNpc->boGlobal = getFieldBoolean("global",&boDefault);
						//int nDefAiID = 0;
						//pNpc->nAITypeId = getFieldInt("aitypeid", &nDefAiID);
						pNpc->bCanHideFlag = boDefault;
						//pNpc->bCanHideFlag = (bool)getFieldBoolean("bCanHide",&boDefault);

						//nDefAiID =0;
						//pNpc->bCamp =(BYTE)( getFieldInt("camp", &nDefAiID) ) ; //设置NPC的阵营 
						//pNpc->bCamp = (BYTE)nDefAiID;
						int nNPCIdleInterval = 120000; // NPC空闲执行间隔，默认为120s
						pNpc->nIdleInterval = getFieldInt("idleInterval", &nNPCIdleInterval);
						pNpc->nOpenDay = getFieldInt("openday", &nDef);
						BOOL boDef = FALSE;
						LPCTSTR stemp = getFieldString("title","",&boDef);
						if (stemp != NULL)
						{
							_asncpytA(pNpc->sTitle,stemp);
						}
						//stemp = getFieldString("say","",&boDef);
						//if (stemp != NULL)
						//{
						//	_asncpytA(pNpc->sSay,stemp);
						//}
						//int nDefClassID = 0;
						//pNpc->nClassId = getFieldInt("classId", &nDefClassID); //设置NPC的类型（如悬赏任务NPC）

						//m_npcList.add(pNpc);
					} while (enumTableNext());
				}
			}
			closeTable();
		}	
		setScript(NULL);
	}
	return true;
}

void CMonsterProvider::completeRead(MONSTERCONFIG *pMonsters, const INT_PTR nMonsterCount, CDataAllocator &dataAllocator, bool bBackLoad)
{
	if (bBackLoad)
	{
		m_Lock.Lock();
		//m_pStdItemsBack = pStdItems;
		m_MonsterBack.reserve(0);
		m_MonsterBack.trunc(0);
		m_MonsterBack.addArray(pMonsters, nMonsterCount);
		m_nMonsterCount = nMonsterCount;
		m_DataAllocatorBack.~CObjectAllocator();
		m_DataAllocatorBack = dataAllocator;
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
		m_Lock.Unlock();
		return;
	}
	//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
	reserve(nMonsterCount);
	trunc(0);
	addArray(pMonsters, nMonsterCount);
	//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
	m_DataAllocator.~CObjectAllocator();
	//将临时的物品属性申请器的内存数据拷贝到自身申请器中
	m_DataAllocator = dataAllocator;
	//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	

}
void CMonsterProvider::UpdateMonsterConfig()
{	
	reserve(m_nMonsterCount);
	trunc(0);
	//addArray(m_pStdItemsBack, m_nItemCount);
	addList(m_MonsterBack);
	m_DataAllocator.~CObjectAllocator();
	m_DataAllocator = m_DataAllocatorBack;

	//m_pStdItemsBack = 0;
	m_nMonsterCount	= 0;
	ZeroMemory(&m_DataAllocatorBack, sizeof(m_DataAllocatorBack));
}
bool  CMonsterProvider::ReadAllMonsters()
{

	
	OutputMsg(rmTip,_T("Load monster Finished"));
	return true;

}

bool CMonsterProvider::ReadOneMonster(CDataAllocator &dataAllocator, MONSTERCONFIG *pMonster)
{
	BOOL isNameValid = false;
	int nDef =1;
	double fDef =0.0;
	bool bDefault =false;
	
	pMonster->nEntityId = getFieldInt("entityid");
	pMonster->btEntityType = (BYTE)getFieldInt("entityType", &nDef); // 怪物实体类型
	pMonster->btMonsterType = (BYTE)getFieldInt("monsterType",&nDef);  //怪物的类型 默认普通怪
	strncpy(pMonster->szName, getFieldString("name","",&isNameValid), ArrayCount(pMonster->szName));
	if (!isNameValid)
	{
		OutputMsg(rmError,"monster Id = %d name is invalid,please check OldLang",pMonster->nEntityId);
		return false;
	}
	pMonster->nLevel = getFieldInt("level");
	pMonster->nOriginalLevel = getFieldInt("level");
	int nCircle = 0;
	pMonster->nCircle = getFieldInt("circle",&nCircle);
	pMonster->nModelId = getFieldInt("modelid");
	nDef = 0;
	pMonster->btDir = (BYTE)getFieldInt("dir", &nDef);				 //朝向
	nDef = 1;
	pMonster->wAiConfigId =(WORD)getFieldInt("aiConfigId",&nDef);    //怪物的ai

	//属性组 
	pMonster->cal.reset(); //重置计算器
	if(int nPropId = getFieldInt("propid"))
	{
		int nPos = nPropId - 1;
		if (nPos >= s_nPropCount || nPos < 0)
		{
			OutputMsg(rmError,_T("load Monster propid error! nEntityId=%d"), pMonster->nEntityId);
			return false;
		}
		CREATURBATTLEEDATA* pdata = s_pPropConfig + nPos;
		CPropertySystem::InitMonsterCalculator(*pdata, pMonster->cal);
	}
	pMonster->nFlagId = getFieldInt("flagid");
	//标志组
	if(int nFlagId = getFieldInt("flagid"))
	{
		int nPos = nFlagId - 1;
		if (nPos >= s_nFlagCount || nPos < 0)
		{
			OutputMsg(rmError,_T("load Monster flagid error! nEntityId=%d"), pMonster->nEntityId);
			return false;
		}
		EntityFlags* pdata = s_pFlagConfig + nPos;
		pMonster->pflags = pdata;
		
		// if( pMonster->pflags->CanGrowUp )	//配置为CanGrowUp 为 true的BOSS就是成长BOSS
		// {
		// 	BossGrowData stGrowBoss;
		// 	stGrowBoss.nMonsterId = pMonster->nEntityId;
		// 	// m_growBossList.add(stGrowBoss);
		// }
	}

	//技能
	ReadMonsterSkills(dataAllocator,pMonster);

	nDef =0;
	pMonster->bCamp = (BYTE)getFieldInt("camp",&nDef);		//怪物的阵营
	pMonster->nExp =  getFieldInt("exp",&nDef); 			//怪物的经验
	pMonster->nInnerExp =  getFieldInt("InnerExp",&nDef); 	//怪物的内劲经验
	nDef =1;
	pMonster->nattackLevel =(int)getFieldInt("attackLevel", &nDef);	//被攻击的优先级
	nDef = 0;
	pMonster->wReSelTargetRate = (WORD)getFieldInt("reSelTargetRate", &nDef);	
	nDef =2500;
	nDef = 500,
	pMonster->nAttackInterval = (int)getFieldInt("attackInterval", &nDef);
	nDef =0;
	pMonster->nMaxDropHp = (int)getFieldInt("maxDropHp", &nDef);
	nDef =0;
	pMonster->wDamageSkill	= (WORD)getFieldInt("damageSkill", &nDef);
	pMonster->bCreateRank = false;
	pMonster->nGatherLevel = 0;
	
	//怪难度等级
	nDef =0;
	pMonster->nMaxHardLv = getFieldInt("nMaxHardLevel", &nDef) ;
	
	pMonster->fLevelA	= (float)getFieldNumber("fLevelA", &fDef);
	pMonster->fLevelB	= (float)getFieldNumber("fLevelB", &fDef);
	pMonster->fLevelC	= (float)getFieldNumber("fLevelC", &fDef);

	pMonster->fPropA		= (float)getFieldNumber("fPropA", &fDef);
	pMonster->fPropB		= (float)getFieldNumber("fPropB", &fDef);
	pMonster->fPropC		= (float)getFieldNumber("fPropC", &fDef);

	//人形怪外观
	nDef =0;
	pMonster->nWeaponId = (WORD)getFieldInt("weaponid", &nDef);
	pMonster->nSwingId = (WORD)getFieldInt("swingid", &nDef);

	bool bDontRecordKillTime = false;
	pMonster->bRecordKillTime = getFieldBoolean("recordKillTime",  &bDontRecordKillTime);
	pMonster->nMonLog = getFieldInt("monLog", &nDef);
	// if( pMonster->bRecordKillTime )
	// {
	// 	BOSSKILL stBossKill;
	// 	stBossKill.nBossId = pMonster->nEntityId;
	// 	stBossKill.nLastKillTime = 0;
	// 	m_bossKillList.add(stBossKill);
	// }

	//下面是一些声音的配置
	pMonster->bAttackMusicId = getFieldInt("attackMusicId",&nDef);
	pMonster->bDieMusicId = getFieldInt("dieMusicId",&nDef);
	pMonster->bTargetMusicId = getFieldInt("targetMusicId",&nDef);
	pMonster->bDamageMusicId = getFieldInt("damageMusicId",&nDef);
	
	//如果没有配置的话就取全局的配置表
	pMonster->bAttackMusicRate = getFieldInt("attackMusicRate",&nDef);
	pMonster->bDieMusicRate = getFieldInt("dieMusicRate",&nDef);
	pMonster->bTargetMusicRate = getFieldInt("targetMusicTate",&nDef);
	pMonster->bDamageMusicRate = getFieldInt("damageMusicRate",&nDef);
	pMonster->nDropid = getFieldInt("dropid",&nDef);
	pMonster->nAscriptionOpen = getFieldInt("ascriptionopen", &nDef);

	//ReadMonsterDropRroup(dataAllocator,pMonster);
	//ReadMonsterSayInfo(dataAllocator, pMonster);
	// ReadMonsterFlags(dataAllocator,pMonster);
	ReadMonsterPriorAttackTarget(dataAllocator, pMonster);
	ReadMonsterColor(pMonster);	
	return true;
}

bool  CMonsterProvider::ReadMonsterSkills( CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster)
{
	
	//怪物的技能列表
	if ( feildTableExists("skills") && openFieldTable("skills") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;

		if ( nCount <= 0 )
		{
			oneMonster->skills.count =0; //没有技能
		}
		else
		{
			oneMonster->skills.count =nCount; //技能的动作
			oneMonster->skills.pData =(PMONSTERSKILL)dataAllocator.allocObjects(nCount * sizeof(MONSTERSKILL));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;
				PMONSTERSKILL pBaseLevel =(PMONSTERSKILL )oneMonster->skills.pData;
				PMONSTERSKILL  pOneSkill  =NULL;
				int nDef =0;
				do 
				{					
					pOneSkill= pBaseLevel + nIdx;
					pOneSkill->wSkillID = (WORD) getFieldInt("id"); 
					pOneSkill->bSkillLevel = (BYTE) getFieldInt("level"); 
					bool bDefAutoCD = false;
					pOneSkill->bAutoCDWhenEnterCombat = getFieldBoolean("autoCD", &bDefAutoCD);
					pOneSkill->bEvent =  getFieldInt("event", &nDef); //在什么事件下使用
					//pOneSkill->bCount = (BYTE) getFieldInt("count"); 
					//pOneSkill->nInterval =(unsigned int) getFieldInt("interval"); 
					nIdx++;
				}
				while (enumTableNext());

			}
		}	
		closeTable();
	}
	

	return true;

}

/*
bool  CMonsterProvider::ReadMonsterSkills( CDataAllocator &dataAllocator,MONSTERCONFIG *pMonster)
{

	//怪物的技能列表
	if ( feildTableExists("skills") && openFieldTable("skills") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) /2;

		if ( nCount <= 0 )
		{
			pMonster->skills.count =0; //没有技能
		}
		else
		{
		
			
			pMonster->skills.count =nCount; //技能的动作
			pMonster->skills.pData =(PMONSTERSKILL)dataAllocator.allocObjects(nCount * sizeof(MONSTERSKILL));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;
				PMONSTERSKILL  pOneSkill  =NULL;
				PMONSTERSKILL pBaseLevel =(PMONSTERSKILL )pMonster->skills.pData;
				do 
				{					
					if(nIdx %2 ==1)
					{
						pOneSkill->nSkillLevel = (WORD)getFieldInt(NULL);  //技能等级	
					}
					else
					{
						pOneSkill= pBaseLevel +(int)(nIdx >>1); //除以
						pOneSkill->nSkillID = (WORD) getFieldInt(NULL);  //技能的ID
					}
					nIdx++;
				}
				while (enumTableNext());

			}
		}	
		closeTable();
	}
	return true;

}
*/

bool CMonsterProvider::ReadMonsterColor(MONSTERCONFIG *pMonster)
{
	if (!pMonster) return false;
	unsigned int clr = DEFAULT_NAME_COLOR;
	const char *pKey = "monstercolor";
	if (feildTableExists(pKey) && openFieldTable(pKey))
	{
		INT_PTR count = lua_objlen(m_pLua, -1);
		if (count > 0)
		{
			if (enumTableFirst())
			{
				int elem[4] = {0}, idx = 0;				
				do 
				{
					if (idx < 4)
						elem[idx] = (int)getFieldNumber(NULL);
					idx++;
				} while (enumTableNext());

				clr = (COLORREF) ( ((BYTE)(elem[3])) | (((BYTE)(elem[2]))<<8) | (((BYTE)(elem[1]))<<16) | ((BYTE)(elem[0])<<24) );
			}
		}		
		closeTable();
	}

	pMonster->nMonsterColor = clr;
	return true;
}

bool CMonsterProvider::ReadMonsterFlags( CDataAllocator &dataAllocator,MONSTERCONFIG *oneMonster )
{
	if ( feildTableExists("flags") && openFieldTable("flags") )
	{
		bool boDefVal = false;
		bool boDefValTrue = true;
		oneMonster->pflags->DenyMove= getFieldBoolean("DenyMove", &boDefVal);
		oneMonster->pflags->DenyBeMove= getFieldBoolean("DenyBeMove", &boDefVal);
		oneMonster->pflags->DenyAttackActor= getFieldBoolean("DenyAttackActor", &boDefVal);
		oneMonster->pflags->DenyBeAttack= getFieldBoolean("DenyBeAttack", &boDefVal);
		oneMonster->pflags->DenyAttackedByActor= getFieldBoolean("DenyAttackedByActor", &boDefVal);
		oneMonster->pflags->CanAttackMonster= getFieldBoolean("CanAttackMonster", &boDefVal);
		oneMonster->pflags->DenyAutoAddMp= getFieldBoolean("DenyAutoAddMp", &boDefVal);
		oneMonster->pflags->DenyAutoAddHp= getFieldBoolean("DenyAutoAddHp", &boDefVal);
		oneMonster->pflags->CanSceneTelport= getFieldBoolean("CanSceneTelport", &boDefVal);
		oneMonster->pflags->CanSeeHideActor= getFieldBoolean("CanSeeHideActor", &boDefVal);
		oneMonster->pflags->DenyDieSubExp= getFieldBoolean("DenyDieSubExp", &boDefVal);
		oneMonster->pflags->DenyUseNearAttack= getFieldBoolean("DenyUseNearAttack", &boDefVal);
		oneMonster->pflags->DenyDizzyBuff= getFieldBoolean("DenyDizzyBuff", &boDefVal);
		oneMonster->pflags->DenyMoveForbidBuff= getFieldBoolean("DenyMoveForbidBuff", &boDefVal);
		oneMonster->pflags->DenyInnnerSkillForbidBuff= getFieldBoolean("DenyInnnerSkillForbidBuff", &boDefVal);
		oneMonster->pflags->DenyOutSkillForbidBuff= getFieldBoolean("DenyOutSkillForbidBuff", &boDefVal);
		oneMonster->pflags->DenyMoveSpeedSlowBuff= getFieldBoolean("DenyMoveSpeedSlowBuff", &boDefVal);
		oneMonster->pflags->DenySee= getFieldBoolean("DenySee", &boDefVal); //是否拒绝看到
		oneMonster->pflags->DenyInitiativeAttackActor= getFieldBoolean("DenyInitiativeAttackActor", &boDefVal); 
		oneMonster->pflags->DenyShowMasterName= getFieldBoolean("DenyShowMasterName", &boDefVal); 
		oneMonster->pflags->DenyShowName = getFieldBoolean("DenyShowName", &boDefVal);
		oneMonster->pflags->AttackSeeKill = getFieldBoolean("AttackSeeKill", &boDefVal);		//死亡见证
		oneMonster->pflags->noBeCrossed  = getFieldBoolean("noBeCrossed",	&boDefVal);	//是否不可穿（0-可穿，1-不可穿）
		// oneMonster->pflags->CanGrowUp = getFieldBoolean("CanGrowUp", &boDefVal);
		oneMonster->pflags->AttackKiller = getFieldBoolean("AttackKiller", &boDefVal);
		oneMonster->pflags->ShowNameByNumber = getFieldBoolean("ShowNameByNumber", &boDefVal);
		oneMonster->pflags->DenyBeCatch = getFieldBoolean("DenyBeCatch", &boDefVal);
		oneMonster->pflags->CanAlwaysEnter = getFieldBoolean("CanAlwaysEnter", &boDefVal);
		oneMonster->pflags->BeAttackNeedUseSkill = getFieldBoolean("BeAttackNeedUseSkill", &boDefVal);
		oneMonster->pflags->IdleUseSkill = getFieldBoolean("IdleUseSkill", &boDefVal);
		oneMonster->pflags->NeedSaveToLog = getFieldBoolean("NeedSaveToLog", &boDefVal);
		oneMonster->pflags->PetNoAttact = getFieldBoolean("PetNoAttact", &boDefVal);
		oneMonster->pflags->DenyAddNumber = getFieldBoolean("DenyAddNumber", &boDefVal);
		oneMonster->pflags->AttackPet = getFieldBoolean("AttackPet", &boDefVal);

		oneMonster->pflags->NoUseAStar = getFieldBoolean("noUseAStar", &boDefVal);
		oneMonster->pflags->MoveTelport = getFieldBoolean("moveTelport", &boDefVal);
		oneMonster->pflags->noAttackBack = getFieldBoolean("NoAttactBack", &boDefVal);
		oneMonster->pflags->noReturnHome = getFieldBoolean("noReturnHome",&boDefVal);
		oneMonster->pflags->attackToScript = getFieldBoolean("attackToScript",&boDefVal);
		oneMonster->pflags->DenySetDir = getFieldBoolean("DenySetDir", &boDefVal);
		oneMonster->pflags->bDeathTriggerScript = getFieldBoolean("bDeathTriggerScript", &boDefVal);
		oneMonster->pflags->bMobTriggerScript = getFieldBoolean("bMobTriggerScript", &boDefVal);
		oneMonster->pflags->bLiveTimeOutTriggerScript = getFieldBoolean("bLiveTimeOutTriggerScript", &boDefVal);
		oneMonster->pflags->boShowVestEntityName = getFieldBoolean("showVestEntityName", &boDefVal);
		
		// if( oneMonster->pflags->CanGrowUp )	//配置为CanGrowUp 为 true的BOSS就是成长BOSS
		// {
		// 	BossGrowData stGrowBoss;
		// 	stGrowBoss.nMonsterId = oneMonster->nEntityId;
		// 	// m_growBossList.add(stGrowBoss);
		// }

		closeTable();
	}
	return true;
}

bool CMonsterProvider::ReadMonsterDropRroup(CDataAllocator &dataAllocator,MONSTERCONFIG *pMonster)
{

	//怪物的技能列表
	if ( feildTableExists("drops") && openFieldTable("drops") )
	{
		const INT_PTR nCount = lua_objlen(m_pLua, -1);
		memset(pMonster->drops.groupMemberCount,0,sizeof(pMonster->drops.groupMemberCount));
		memset(pMonster->drops.groupNextHitIndex,0,sizeof(pMonster->drops.groupNextHitIndex));

		if ( nCount <= 0 )
		{
			pMonster->drops.data.count =0; //没有技能
		}
		else
		{
			//pMonster->drops.data = dataAllocator.allocObjects(nCount * sizeof(ONEDROPDATA));
			pMonster->drops.data.count =nCount; //
			pMonster->drops.data.pData =(PONEDROPDATA)dataAllocator.allocObjects(nCount * sizeof(ONEDROPDATA));

			if ( enumTableFirst() )
			{
				CMiniDateTime nStartMiniDate, nOverMiniDate;
				SYSTEMTIME startTime, overTime; //时间
				memset(&startTime, 0, sizeof(SYSTEMTIME));
				memset(&overTime, 0, sizeof(SYSTEMTIME));
				INT_PTR nIdx = 0;
				PONEDROPDATA pBaseLevel =(PONEDROPDATA )pMonster->drops.data.pData;
				do 
				{
					PONEDROPDATA  pData = pBaseLevel +nIdx;
					int nDef = 0;
					pData->bAwardType  = (char)getFieldInt("type",&nDef); 
					pData->bStrong  =  (char)getFieldInt("strong",&nDef); 
					pData->bQuality  =  (char)getFieldInt("quality",&nDef); 
					pData->nQualityDataIndex = (WORD)getFieldInt("qualityDataIndex",&nDef); 
					pData->bBind = (char)getFieldInt("bind", &nDef);
					nDef=0;
					pData->nCount  =  (int)getFieldInt("count"); 
					pData->nMinCount  =  (int)getFieldInt("mincount",&nDef); 
					pData->nMaxCount  =  (int)getFieldInt("maxcount",&nDef); 
					pData->wItemID  =  (WORD)getFieldInt("id"); 
					pData->nPropability  = getFieldInt("propability"); 
					pData->bGroupId = (BYTE) getFieldInt("group");
					pData->nTime = getFieldInt("time", &nDef);
					nDef=0;
					pData->nGrowLv = (BYTE)getFieldInt("growLv", &nDef);
					int nDefLevel = 1000;
					pData->nGrowLvMax = (BYTE)getFieldInt("growLvMax",&nDefLevel);
					bool bDef =false;
					pData->bBroadCastType =  getFieldInt("broadType",&nDef); //广播的类型
					pData->bTriggerEvent  =  getFieldBoolean("triggerEvent",&bDef) ; //是否需要触发脚本
					//最低的强化
					pData->bMinStrong       =(char) getFieldInt("strongMin",&nDef );
					//最低的品质
					pData->bMinQuality       =(char) getFieldInt("qualityMin",&nDef );	
					//-1表示任何性别
					nDef =-1;
					pData->bSex       =(char) getFieldInt("sex",&nDef );
					//0表示任何的职业
					nDef =0;
					pData->bJob       =(char) getFieldInt("job",&nDef );	
					pData->nLootTime = getFieldInt("LootTime",&nDef);

					//掉落时间范围
					pData->nStartDate = pData->nEndDate = 0;

					nDef = 90;
					pData->nExpireTime = getFieldInt("ExpireTime",&nDef);

					LPCSTR sStartDate = getFieldString("startDate", "");
					if (strlen(sStartDate) > 0)
					{
						LPCSTR sEndDate = getFieldString("overDate", "");
						if (strlen(sEndDate) == 0)
						{
							sEndDate = sStartDate;
						}
						sscanf(sStartDate, "%d-%d-%d", &startTime.wYear, &startTime.wMonth, &startTime.wDay);
						sscanf(sEndDate, "%d-%d-%d", &overTime.wYear, &overTime.wMonth, &overTime.wDay);
						nStartMiniDate.encode(startTime);
						pData->nStartDate = nStartMiniDate.tv;

						overTime.wMinute = overTime.wSecond = 59;
						overTime.wHour = 23;
						nOverMiniDate.encode(overTime);
						pData->nEndDate = nOverMiniDate.tv;
					}
					if(pData->bGroupId > MAX_DROP_GROUP_COUNT)
					{
						OutputMsg(rmError,_T("怪物的掉落组的ID=%d过大，最大=%d"),pData->bGroupId,MAX_DROP_GROUP_COUNT);
						pData->bGroupId =0;
					}
					if(pData->bGroupId)
					{
						pMonster->drops.groupMemberCount[pData->bGroupId -1] ++; //组里的个数++ 
					}
					if(pData->nPropability ) //如果配置了概率，那么
					{
						//表示第几个怪物死亡的时候需要掉落这个物品
						pData->nSeed = (GetGlobalLogicEngine()->GetRandValue()% pData->nPropability) +1;
						//pData->wDeadTimes =0; //怪物的死亡个数
					}
					nIdx++;


				}
				while (enumTableNext());

			}
		}	
		closeTable();
	}
	return true;
}


bool CMonsterProvider::ReadMonsterSayInfo(CDataAllocator &dataAllocator, MONSTERCONFIG *oneMonster)
{
	MonsterSayList &sayList = oneMonster->monsterSayList;
	if (feildTableExists("talk") && openFieldTable("talk"))
	{
		sayList.nLowerInterval = getFieldInt("minInterval");
		sayList.nUpperInterval = getFieldInt("maxInterval");
		if (feildTableExists("info") && openFieldTable("info"))
		{
			INT_PTR nCount = lua_objlen(m_pLua, -1);
			if (nCount > 0)
			{
				CVector<MonsterSayInfo> m_sayInfoList;		// 所有的发言内容列表
				INT_PTR sayTypeInfoCount[mstMax] = {0};			// 没种类型发言配置数量。普通类型只有一条，血量过低发言可以多条
				m_sayInfoList.reserve(16);
				if (enumTableFirst())
				{
					do 
					{	
						unsigned int nType = (unsigned int)getFieldInt("type");
						if (nType < mstMax)
						{
							//MonsterSayInfo &info = sayList.vecSayTypeInfo[nType];						
							MonsterSayInfo info;
							info.nType = nType;
							sayTypeInfoCount[nType]++;
							info.nRate = getFieldInt("rate");
							info.nBCType = (MonsterSayBCType)getFieldInt("bctype");
							int nDefParam = 128;
							info.nTipType = getFieldInt("tiptype", &nDefParam);
							if (info.nRate > 100) info.nRate = 100;
							nDefParam = 0;
							info.nParam = getFieldInt("param", &nDefParam);
							info.nLimitLev = getFieldInt("limitLev", &nDefParam);
							bool bDefParam = false;
							info.bTransmit = getFieldBoolean("transmit", &bDefParam);
							info.nLimitTime = getFieldInt("limitTime", &nDefParam);
							info.nPosX = getFieldInt("posx", &nDefParam);
							info.nPosY = getFieldInt("posy", &nDefParam);
							if (feildTableExists("content") && openFieldTable("content"))
							{
								info.nCount = 0;
								INT_PTR nContentCount = lua_objlen(m_pLua, -1);
								if (nContentCount > 0)
								{
									CVector<char *> vecMsg;
									if (enumTableFirst())
									{
										do 
										{
											const char *p = getFieldString(NULL);
											INT_PTR nLen = 1;			// 至少分配一个字节存放对话内容
											if (p) nLen = strlen(p)+1;
											char *buff = (char *)malloc(sizeof(char) * nLen);			
											if (p)
												strcpy(buff, p);
											buff[nLen-1] = 0;
											vecMsg.add(buff);											
										} while (enumTableNext());

										info.nCount = (int)vecMsg.count();
										if (info.nCount > 0)
										{
											//info.pMsgList = new LPSTR[info.nCount];
											info.pMsgList = (LPSTR *)dataAllocator.allocObjects(sizeof(LPSTR) * info.nCount); 
											for (INT_PTR i = 0; i < info.nCount; i++)
											{
												char *buff = vecMsg[i];
												INT_PTR nBuffLen = strlen(buff)+1;
												char *p = (char *)(dataAllocator.allocObjects(sizeof(char) * nBuffLen));
												strcpy(p, buff);
												p[nBuffLen-1] = 0;
												info.pMsgList[i] = p;
												free(buff);
											}
										}
									}
								}
								closeTable();
							}
							m_sayInfoList.add(info);
							//sayList.vecSayInfo[nType] = info;							
						}						
					} while (enumTableNext());
				}
											
				for (INT_PTR i = 0; i < m_sayInfoList.count(); i++)
				{
					MonsterSayInfo& info = m_sayInfoList[i];
					MonsterSayTypeConfig& sayTypeInfo = sayList.vecSayTypeInfo[info.nType];					
					if (sayTypeInfo.pMonsterSayCfg == NULL)
						sayTypeInfo.pMonsterSayCfg = (MonsterSayInfo *)dataAllocator.allocObjects(sizeof(MonsterSayInfo) * sayTypeInfoCount[info.nType]);
					
					sayTypeInfo.pMonsterSayCfg[sayTypeInfo.nCount] = info;
					sayTypeInfo.nCount++;
				}
			}
			closeTable();
		}
		closeTable();
	}
	return true;
}

bool CMonsterProvider::ReadMonsterPriorAttackTarget(CDataAllocator &dataAllocator, MONSTERCONFIG *oneMonster)
{
	PriorAttackTarget &targetInfo = oneMonster->priorAttackTarget;
	if (feildTableExists(MONSTER_PROP_PRIOR_ATTACK_TARGET) && openFieldTable(MONSTER_PROP_PRIOR_ATTACK_TARGET))
	{
		INT_PTR nCount = lua_objlen(m_pLua, -1);
		if (nCount > 0)
		{
			if (enumTableFirst())
			{
				CVector<int> targetList;
				do
				{
					int monsterId = getFieldInt(NULL);
					if (monsterId != 0)
						targetList.add(monsterId);
				}while(enumTableNext());

				targetInfo.nCount = (int)targetList.count();
				if (targetInfo.nCount > 0)
				{
					targetInfo.pTargetIDList = (int *)dataAllocator.allocObjects(sizeof(int) * targetInfo.nCount);
					for (INT_PTR i = 0; i < targetInfo.nCount; i++)
					{
						targetInfo.pTargetIDList[i] = targetList[i];
					}
				}
			}
		}
		closeTable();
	}

	return true;
}
/*

bool CMonsterProvider::ReadMonsterDrop(CDataAllocator &dataAllocator,PDROPGROUP pGroupData)
{
	pGroupData->wPropability =(WORD)  getFieldInt("propability");  //概率

	if ( feildTableExists("dropdata") && openFieldTable("dropdata") )
	{
		const INT_PTR nCount = lua_objlen(m_pLua, -1);
		if ( nCount <= 0 )
		{
			pGroupData->dropData.count =0; //没有技能
		}
		else
		{
			pGroupData->dropData.count =nCount; //技能的动作
			pGroupData->dropData.pData =(PONEDROPDATA)dataAllocator.allocObjects(nCount * sizeof(ONEDROPDATA));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;
				PONEDROPDATA pBaseLevel =(PONEDROPDATA )pGroupData->dropData.pData;
				do 
				{
					PONEDROPDATA  pData = pBaseLevel +nIdx;
					pData->bStrong  =  (BYTE)getFieldInt("strong"); 
					pData->bQuality  =  (BYTE)getFieldInt("quality"); 
					pData->nCount  =  (int)getFieldInt("count"); 
					pData->wItemID  =  (WORD)getFieldInt("id"); 
					pData->wPropability  =  (WORD)getFieldInt("propability"); 
					//pData->nBinded  =  (BYTE)getFieldInt("bind"); 
					nIdx++;
				}
				while (enumTableNext());

			}
		}	
		closeTable();
	}
	return true;
}
*/

bool CMonsterProvider::readCacheData(DWORD dwSrcCRC32)
{
#ifdef _DEBUG
	MONSTERCONFIG *pMonsters;
	INT_PTR nMonCount;
	CMonsterConfigCacher cache;
	CDataAllocator allocator;

	if ( !cache.LoadFromCache(MonsterCacheFile, dwSrcCRC32, allocator, (void**)&pMonsters, nMonCount) )
		return false;
	completeRead(pMonsters, nMonCount, allocator);
	return true;
#else
	return false;
#endif
}

bool CMonsterProvider::saveCacheData(DWORD dwSrcCRC32)
{
#ifdef _DEBUG
	bool result = false;
	const MONSTERCONFIG *pMonsters = *this;
	CMonsterConfigCacher cache;

	try
	{
		result = cache.SaveToCache(MonsterCacheFile, dwSrcCRC32, m_DataAllocator, pMonsters, count());
		if ( !result )
		{
			showError(_T("save Monster config data cache failure!"));
		}
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on save Monster config cache!"));
	}
	return result;
#else
	return false;
#endif
}

void CMonsterProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

/*
	装载BOSS成长数据，某些BOSS随着被击杀的次数增加，等级要增加，要服务器重启后也能生效
*/
bool CMonsterProvider::LoadBossGrowData(LPCTSTR sFilePath)
{
	// if(!FileExists(sFilePath))
	// {
	// 	return true;
	// }
	// CFileStream sFile(sFilePath, CFileStream::faShareRead);
	// sFile.setPosition(0);
	// BossGrowHeader bossHdr;
	
	// if(sFile.read(&bossHdr, sizeof(bossHdr)) < sizeof(bossHdr))
	// {
	// 	OutputMsg(rmError, _T("read BossGrowData Header error"));
	// 	return false;
	// }
	// CVector<BossGrowData> bossGrowList;
	// LONG64 dwDataSize = sizeof(BossGrowData) * bossHdr.uLength;
	// bossGrowList.reserve(bossHdr.uLength);
	// bossGrowList.trunc(bossHdr.uLength);
	// if (dwDataSize > 0 && dwDataSize != sFile.read(&bossGrowList[0], dwDataSize))
	// {
	// 	bossGrowList.trunc(0);
	// 	OutputMsg(rmError, _T("read BossGrowData data error"));
	// 	return false;
	// }

	// int nListCount = (int)bossGrowList.count();
	// for (int i = 0; i < nListCount; i++)
	// {
	// 	BossGrowData * pBossData =& bossGrowList[i];
	// 	PMONSTERCONFIG pMonster = GetMonsterData(pBossData->nMonsterId);
	// 	if (pMonster == NULL)
	// 	{
	// 		continue;
	// 	}
	// 	int nMaxDeadCount = 0, nDis = 0;
	// 	pMonster->nDeadTimes = pBossData->nDeadCount;
	// 	if(pBossData->nHardLevel > pMonster->nOriginalLevel)
	// 	{
	// 		pMonster->nLevel = pBossData->nHardLevel;
	// 		nDis = pMonster->nLevel - pMonster->nOriginalLevel;	
	// 		nMaxDeadCount =(int)(pMonster->fLevelA * (float)(nDis * nDis) + pMonster->fLevelB * (float)(nDis) + pMonster->fLevelC);
	// 		if (pMonster->nMaxHardLv > pMonster->nLevel)
	// 		{
	// 			nMaxDeadCount = -1;
	// 		}			
	// 	}
	// 	pMonster->nMaxDeadTimes = nMaxDeadCount;
	// }
	return true;
}

/*
	清除BOSS成长数据，使之恢复到原始配置
*/
void CMonsterProvider::ClearBossGrow()
{
	// //INT_PTR nCount = m_bossInfoList.count();
	// INT_PTR nCount = m_growBossList.count();
	// for (INT_PTR i = 0; i < nCount; i++)
	// {
	// 	//BOSSINFO & oneBoss = m_bossInfoList[i];
	// 	BossGrowData & oneBoss = m_growBossList[i];
	// 	PMONSTERCONFIG pConfig = GetMonsterData(oneBoss.nMonsterId);
	// 	if (pConfig != NULL)
	// 	{
	// 		pConfig->nLevel = pConfig->nOriginalLevel;
	// 		pConfig->nMaxDeadTimes = pConfig->nMaxHardLv = 0;
	// 	}
	// }
	m_bossGrowupHasChange = true;
}

/*
	保存BOSS成长数据，某些BOSS随着被击杀的次数增加，等级要增加，要服务器重启后也能生效
*/
bool CMonsterProvider::SaveBossGrowData(LPCTSTR sFilePath)
{
	// if(m_bossGrowupHasChange==false) return true; //压根就没有改变

	// CVector<BossGrowData> bossGrowList;
	// bossGrowList.clear();
	// //INT_PTR nCount = m_bossInfoList.count();
	// INT_PTR nCount = m_growBossList.count();		//成长BOSS，以CanGrowUp为准
	// for (INT_PTR i = 0; i < nCount; i++)
	// {
	// 	//BOSSINFO & oneBoss = m_bossInfoList[i];
	// 	BossGrowData &oneBoss = m_growBossList[i];
	// 	PMONSTERCONFIG pConfig = GetMonsterData(oneBoss.nMonsterId);
	// 	if (pConfig != NULL)
	// 	{
	// 		BossGrowData oneGrowData;
	// 		oneGrowData.nMonsterId = oneBoss.nMonsterId;
	// 		oneGrowData.nDeadCount = pConfig->nDeadTimes;
	// 		oneGrowData.nHardLevel = pConfig->nLevel;
	// 		bossGrowList.add(oneGrowData);
	// 	}
	// }
	
	// CFileStream sFile(sFilePath, CFileStream::faWrite|CFileStream::faShareRead,CFileStream::AlwaysCreate);
	// BossGrowHeader bossHdr;
	// bossHdr.uLength = (UINT32)bossGrowList.count();
	// if(sFile.write(&bossHdr, sizeof(bossHdr)) != sizeof(bossHdr))
	// {
	// 	OutputMsg(rmError, _T("write Boss Data Header size error"));
	// 	return false;
	// }
	// SIZE_T dwDataSize = sizeof(BossGrowData) * bossGrowList.count();
	// if (dwDataSize > 0 && sFile.write(&bossGrowList[0], dwDataSize) != dwDataSize)
	// {
	// 	OutputMsg(rmError, _T("write BossGrow Data size error"));
	// 	return false;
	// }
	// m_bossGrowupHasChange= false;
	return true;
}

bool CMonsterProvider::LoadWildBossConfig(LPCTSTR sFilePath)
{
	// if (PreLoad(sFilePath))
	// {
	// 	m_wildBossList.clear();
	// 	if(openGlobalTable("WildBossConfig"))
	// 	{
	// 		if (feildTableExists("WildBoss") && openFieldTable("WildBoss"))
	// 		{
	// 			WILDBOSS stWildBoss;
	// 			if (enumTableFirst())
	// 			{
	// 				do 
	// 				{
	// 					stWildBoss.nBossId = (WORD)getFieldInt("monsterId");
	// 					stWildBoss.nSceneId = (WORD)getFieldInt("sceneId");
	// 					m_wildBossList.add(stWildBoss);
	// 				} while (enumTableNext());
	// 			}
	// 			closeTable();
	// 		}
	// 		closeTable();
	// 	}
	// 	setScript(NULL);
	// }
	return true;
}

bool CMonsterProvider::LoadWorldBossConfig(LPCTSTR sFilePath)
{
	// if (PreLoad(sFilePath))
	// {
	// 	m_worldBossList.clear();
	// 	if(openGlobalTable("WorldBossConfig"))
	// 	{
	// 		if (feildTableExists("WorldBoss") && openFieldTable("WorldBoss"))
	// 		{
	// 			WORLDBOSS stWorldBoss;
	// 			if (enumTableFirst())
	// 			{
	// 				do 
	// 				{
	// 					stWorldBoss.nBossId = (WORD)getFieldInt("monsterId");
	// 					stWorldBoss.nSceneId = (WORD)getFieldInt("sceneId");
	// 					m_worldBossList.add(stWorldBoss);
	// 				} while (enumTableNext());
	// 			}
	// 			closeTable();
	// 		}
	// 		closeTable();
	// 	}
	// 	setScript(NULL);
	// }
	return true;
}

//设置BOSS被击杀的时间
void CMonsterProvider::SetBossLastKillTime( WORD nBossId, unsigned int nKillTime )
{
	// for(INT_PTR i=0; i<m_bossKillList.count(); i++)
	// {
	// 	BOSSKILL &stBossKill = m_bossKillList[i];
	// 	if( stBossKill.nBossId == nBossId )
	// 	{
	// 		stBossKill.nLastKillTime = nKillTime;
	// 	}
	// }
}

unsigned int CMonsterProvider::GetBossLastKillTime( WORD nBossId )
{
	// for(INT_PTR i=0; i<m_bossKillList.count(); i++)
	// {
	// 	BOSSKILL &stBossKill = m_bossKillList[i];
	// 	if( stBossKill.nBossId == nBossId )
	// 	{
	// 		return stBossKill.nLastKillTime;
	// 	}
	// }
	return 0;
}



bool CMonsterProvider::LoaddBossInfoConfig(LPCTSTR sFilePath)
{

	if (PreLoad(sFilePath))
	{
		m_bossInfoList.clear();
		if(openGlobalTable("BossConfig"))
		{
			if(enumTableFirst())
			{
				do
				{
					int nDefault = 0;
					BOSSCONFIGINFO cfg;
					cfg.nBossId = getFieldInt("entityid", &nDefault);
					cfg.nSerial = getFieldInt("Serial", &nDefault);
					cfg.nTab = getFieldInt("mold", &nDefault);
					cfg.nReborn = getFieldInt("reborn", &nDefault);
					cfg.nMaxNum = getFieldInt("number", &nDefault);
					cfg.nMapId = getFieldInt("map", &nDefault);
					cfg.nBossX = getFieldInt("x", &nDefault);
					cfg.nBossY = getFieldInt("y", &nDefault);
					cfg.nBossRange = getFieldInt("range", &nDefault);
					cfg.nNotice = getFieldInt("notice", &nDefault);
					cfg.nOpenCircle = getFieldInt("opencircle", &nDefault);
					cfg.nOpenDay = getFieldInt("openday", &nDefault);
					cfg.nDelivery = getFieldInt("delivery", &nDefault);
					cfg.nDeliveryX = getFieldInt("delivery_x", &nDefault);
					cfg.nDeliveryY = getFieldInt("delivery_y", &nDefault);
					cfg.nDeliveryRange = getFieldInt("delivery_range", &nDefault);
					cfg.nEndmap = getFieldInt("endmap", &nDefault);
					cfg.nDaliyLimit = getFieldInt("limittimes", &nDefault);
					cfg.nJiontime = getFieldInt("jiontime", &nDefault);
					cfg.nDalitStaticId = getFieldInt("staticType", &nDefault);
					cfg.nShowDay = getFieldInt("showday", &nDefault);
					cfg.nFubenId = getFieldInt("fbid", &nDefault);
					cfg.nIsOpen = getFieldInt("isopen", &nDefault);
					cfg.nLevellimit = getFieldInt("levellimit", &nDefault);
					cfg.nBerebornlimit = getFieldInt("berebornlimit", &nDefault);
					cfg.nVip = getFieldInt("vip", &nDefault);
					cfg.nTips = getFieldInt("tips", &nDefault);
					int nDef = 1;
					cfg.nNodeliver = getFieldInt("nodeliver", &nDef);
					cfg.nNoexpel = getFieldInt("noexpel", &nDefault);
					cfg.nNpcId = getFieldInt("npcid", &nDefault);
					cfg.nShowwindow = getFieldInt("showwindow", &nDefault);
					int id = cfg.nBossId * BOSS_ID_MUTIPLY + cfg.nMapId;

					if(cfg.nReborn == 0)
					{
						id = cfg.nSerial * BOSSSOCAIL_MUTIPLY;
					}
					AddBossTeleCfgTimes(id, cfg.nDaliyLimit);
					
					if(cfg.nNotice == 1)
						getFieldStringBuffer("content",cfg.nContent, sizeof(cfg.nContent));
					if(feildTableExists("timeslot") && openFieldTable("timeslot"))
					{
						int nDefault = 0;
						int nCount = (int)lua_objlen(m_pLua,-1);
                        cfg.pTimeSlotDetail = (BossTimePair*)m_pAllocator->AllocBuffer(sizeof(BossTimePair) * nCount);
                        for (size_t i = 1; i <= nCount; i++)
                        {
                            getFieldIndexTable(i);
                            cfg.pTimeSlotDetail[i-1].nStartTime = getFieldInt("StartTime", &nDefault);;
                            cfg.pTimeSlotDetail[i-1].nEndTime = getFieldInt("EndTime", &nDefault);;
                            closeTable();
                        }
                        cfg.nTimeSlotCount = nCount;
                        cfg.nTimeSlotIdx = 0;
		                closeTable();	
					}
					if(feildTableExists("time") && openFieldTable("time"))
					{
						if(enumTableFirst())
						{
							do
							{
								int ntime = getFieldInt(NULL);
								cfg.nReferTime.push_back(ntime);
							}while(enumTableNext());
						}

						closeTable();
					}
					if(feildTableExists("consume") && openFieldTable("consume"))
					{
						if(enumTableFirst())
						{
							do
							{
								tagKillBossCost cost;
								cost.nId = getFieldInt("id", &nDefault);
								cost.nCount = getFieldInt("count", &nDefault);
								cost.nType = getFieldInt("type", &nDefault);
								cfg.nCosts.push_back(cost);
							}while(enumTableNext());
						}
						closeTable();
					}
					int nId = cfg.nSerial * BOSSSOCAIL_MUTIPLY +cfg.nBossId;
					m_bossInfoList[nId] = cfg;

				}while(enumTableNext());
			}
			
			closeTable();
		}
		LoaddBossGrowConfig();
		LoaddShenZhuangBossConfig();
		setScript(NULL);
	}
	return true;
}


bool CMonsterProvider::LoaddBossGrowConfig()
{

	m_growBossList.clear();
	if(openGlobalTable("BossGrowConfig"))
	{
		if(enumTableFirst())
		{
			do
			{
				int nDefault = 0;
				BOSSGROWDATA cfg;
				cfg.nBossId = getFieldInt("entityid", &nDefault);
				cfg.nFDropA = getFieldInt("fPropA", &nDefault);
				cfg.nFDropB = getFieldInt("fPropB", &nDefault);
				cfg.nFDropC = getFieldInt("fPropC", &nDefault);
				if(feildTableExists("nMaxHardLevel") && openFieldTable("nMaxHardLevel"))
				{

					BOSSMAXHANDLEVEL& lvcfg= cfg.mHandLvs;
					lvcfg.nType = getFieldInt("type", &nDefault);
					if(feildTableExists("level") && openFieldTable("level"))
					{
						if(enumTableFirst())
						{
							do
							{
								MAXHARDLEVEL lv;
								lv.nLevel = getFieldInt("level", &nDefault);
								lv.nLimit = getFieldInt("limit", &nDefault);
								lv.nLimitNum = getFieldInt("limitnum", &nDefault);
								lvcfg.nGrowLv.push_back(lv);
							}while(enumTableNext());
						}
						closeTable();
					}
					closeTable();
				}
				m_growBossList[cfg.nBossId] = cfg;

			}while(enumTableNext());
		}
		
		closeTable();
	}
	return true;
}


bool CMonsterProvider::LoaddShenZhuangBossConfig()
{

	// nOpenlevel = 0;
	m_openCfgs.clear();
	if(openGlobalTable("ShenZhuangBossConfig"))
	{

		int nDefault = 0;
		BossOpenConfig cfg;
		cfg.nOpenlevel = getFieldInt("level", &nDefault);
		cfg.nOpenday = getFieldInt("openday", &nDefault);
		cfg.nId = getFieldInt("id", &nDefault);
		m_openCfgs.push_back(cfg);
		cfg.nOpenlevel = getFieldInt("pidlevel", &nDefault);
		cfg.nOpenday = getFieldInt("popenday", &nDefault);
		cfg.nId = getFieldInt("pid", &nDefault);
		m_openCfgs.push_back(cfg);
		closeTable();
	}
	return true;
}


int tagBossConfigInfo::GetNextReferTime(int nLastReferTime)
{
	// int nNowTime = time(NULL);
	SYSTEMTIME	m_CurSysTime;
	GetLocalTime(&m_CurSysTime);
	CMiniDateTime CurMiniDateTime;
	CurMiniDateTime = CurMiniDateTime.encode(m_CurSysTime);
	int nNowTime = CurMiniDateTime.tv;
	int nZerotime = CMiniDateTime::today();
	int nSize = nReferTime.size();
	int nNextTime = 0;
	
	if(nReborn == 2)
	{	
		nNextTime = nLastReferTime + nReferTime[0];
	}
	else if(nReborn == 3)
	{
		// 初始化跨服次元首领下次更新时间
		if (!nLastReferTime)
		{
			// 今日跨服次元首领活动暂未开启
			if (nNowTime < (pTimeSlotDetail[0].nStartTime + nZerotime) )
			{
				nNextTime = pTimeSlotDetail[0].nStartTime + nZerotime;
			} else if(nNowTime > (pTimeSlotDetail[nTimeSlotCount - 1].nEndTime + nZerotime) ) {	// 今日跨服次元首领活动结束，初始化下次更新时间为次日首次更新时间
				nNextTime = pTimeSlotDetail[0].nStartTime + nZerotime + 24*3600;
			} else { 
				// 今日跨服次元首领活动已开启，但暂未结束
				int nFlag = 0;	// 							
				for (size_t i = 0; i < nTimeSlotCount; ++i)
				{
					if ( (pTimeSlotDetail[i].nStartTime + nZerotime) <= nNowTime && nNowTime <= (pTimeSlotDetail[i].nEndTime + nZerotime) )
					{
						nNextTime = nNowTime;
						break;
					}

					if (nNowTime > (pTimeSlotDetail[i].nEndTime + nZerotime))
					{
						nFlag = i + 1;
					}
				}

				if (!nNextTime && nFlag)
				{
					nNextTime = pTimeSlotDetail[nFlag].nStartTime + nZerotime;
				}
			}		
		} else {
			// 今日跨服次元首领活动结束，初始化下次更新时间为次日首次更新时间
			if ( (nLastReferTime + nReferTime[0]) > (pTimeSlotDetail[nTimeSlotCount - 1].nEndTime + nZerotime) )
			{
				nNextTime = pTimeSlotDetail[0].nStartTime + nZerotime + 24*3600;
			} else {
				// 今日跨服次元首领活动已开启，但暂未结束
				int nFlag = 0;	// 							
				for (size_t i = 0; i < nTimeSlotCount; ++i)
				{
					if ( (pTimeSlotDetail[i].nStartTime + nZerotime) <= (nLastReferTime + nReferTime[0]) && (nLastReferTime + nReferTime[0]) <= (pTimeSlotDetail[i].nEndTime + nZerotime) )
					{
						nNextTime = (nLastReferTime + nReferTime[0]);
						break;
					}

					if ((nLastReferTime + nReferTime[0]) > (pTimeSlotDetail[i].nEndTime + nZerotime))
					{
						nFlag = i + 1;
					}
				}

				if (!nNextTime && nFlag)
				{
					nNextTime = pTimeSlotDetail[nFlag].nStartTime + nZerotime;
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < nSize; i++)
		{
			// int nTime = getNowZeroTime() + nReferTime[i];
			int nTime = nZerotime + nReferTime[i];
			
			if(nNowTime >= nTime)
			{
				// int nextDay = getNowZeroTime() + nReferTime[i] + 24*3600;
				int nextDay = nZerotime + nReferTime[i] + 24*3600;
				if(nNextTime == 0)
					nNextTime = nextDay;
				
				if(nextDay < nNextTime)
					nNextTime = nextDay;
			}
			else
				return nTime;
		}
	}
	
	return nNextTime;
}


int BOSSGROWDATA::GetBossGrowLv(int OpenserverDay, int nMaxLevel, CGlobalVarMgr* pGGlobVar)
{
	int nlv = 0;
	if(mHandLvs.nType == 0)
		return nlv;
	
	if( mHandLvs.nGrowLv.size() == 0)
		return 0;
		
	for(int k = 0; k < mHandLvs.nGrowLv.size(); k++)
	{
		int limit = 0;
		MAXHARDLEVEL& lvCfg = mHandLvs.nGrowLv[k];
		if(mHandLvs.nType == 1) //转生等级
		{
			if(pGGlobVar)
				limit = pGGlobVar->GetCountByCircleLevel(lvCfg.nLimit);
		}
		else if(mHandLvs.nType == 2) //开服天数
		{ 
			limit = OpenserverDay;
		}
		else
		{
			limit = nMaxLevel;
		}
		if(limit > lvCfg.nLimitNum && lvCfg.nLevel > nlv )
			nlv = lvCfg.nLevel;
	}

	return nlv;
}

