#include "StdAfx.h"
#include "SkillConfig.h"
#include "SkillProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CSkillProvider::CSkillProvider()
	:Inherited(), Inherited2(), m_DataAllocator(_T("SkillDataAlloc"))
{

}

CSkillProvider::~CSkillProvider()
{
}

bool CSkillProvider::LoadSkills(LPCTSTR sFilePath)
{
	
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	LPCTSTR sText;

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
			showError(_T("syntax error on skill config"));
		//读取标准物品配置数据
		Result = ReadAllSkills();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load skill config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load skill config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}


bool CSkillProvider::ReadAllSkills()
{
	int nDef_1 = 1;
	int nDef_0 = 0;
	int nDef_999 = 999;
	bool bDef_false = false;
	bool bDef_true = true;

	//技能总表
	if (openGlobalTable("SkillConf"))
	{
		//获取配置中的技能数量
		INT_PTR nCount = lua_objlen(m_pLua, -1);

		//如果物品数量为0则输出未配置任何物品的错误
		if ( nCount <= 0 )
		{
			showError(_T("no skill data on Skill config"));
			return false;
		}

		++nCount;
		//预留数据空间
		reserve(nCount);
		trunc(nCount);
		OneSkillData *pSkills = this->Inherited::operator OneSkillData* ();

		//循环读取技能配置数据
		if (enumTableFirst())
		{
			//INT_PTR nIdx = 0;
			do 
			{
				INT_PTR nSkillId = getFieldInt("id");
				if (nSkillId >= nCount)
				{
					showErrorFormat(_T("skill id 过大 nSkillId:%d, nCount:%d"), nSkillId, nCount);
					endTableEnum();
					break;
				}
				
				OneSkillData * pSkill = &pSkills[nSkillId];
				//if (nSkillId != ++nIdx)
				//{
				//	showErrorFormat(_T("skill id 不按顺序递增 nSkillId:%d, nIdx:%d"), nSkillId, nIdx);
				//	endTableEnum();
				//	break;
				//}

				//构造
				new(pSkill) OneSkillData();
				
				pSkill->nSkillID = nSkillId;													//技能的ID
				getFieldStringBuffer("name",pSkill->sSkillName,sizeof(pSkill->sSkillName)); 	//读取技能名字
				pSkill->bVocation =  (BYTE)getFieldInt("vocation",&nDef_0); 					//职业
				pSkill->nSkillType =(BYTE) getFieldInt("skillType"); 							//技能的类型
				pSkill->bSkillClass  = (BYTE) getFieldInt("skillClass",&nDef_1); 				//技能的分类
				pSkill->bSingleEmptyEffect = getFieldBoolean("singleEffect",&bDef_false);		//单体远程空地展示特效
				pSkill->nCommonCdTime = getFieldInt("commonCd"); 					//使用的时候触发的公共的cd时间
				pSkill->bSpecialBuffCond = (BYTE)getFieldInt("specialBuffCond",&nDef_0); 		//特殊buff条件
				pSkill->bBeedCalPassiveProperty = getFieldBoolean("needCalPassiveProperty",&bDef_true); //是否需要计算被动属性
				pSkill->bPriority = (BYTE)getFieldInt("priority",&nDef_0);						//优先级
				pSkill->btClientDir = (BYTE)getFieldInt("direction",&nDef_0);					//使用客户端方向
				pSkill->wAttrId = (WORD)getFieldInt("attrid",&nDef_0);							//属性id
				pSkill->bIsSwitch = (BYTE) (getFieldBoolean("isSwitch",&bDef_false) ?1:0);	//是否手动开关
				pSkill->bIsAutoLearn = getFieldBoolean("isAutoLearn",&bDef_false);
				pSkill->nAutoLearnSkillLvl = getFieldInt("autoLearnSkillLvl",&nDef_999);
				pSkill->boIsDelete = getFieldBoolean("isDelete",&bDef_false);

				switch (pSkill->bVocation)
				{
				case enVocNone:
					m_None.push_back(pSkill);
					break;
				case enVocWarrior:
					m_Warrior.push_back(pSkill);
					break;
				case enVocMagician:
					m_Magician.push_back(pSkill);
					break;
				case enVocWizard:
					m_Wizard.push_back(pSkill);
					break;
				}
			}
			while (enumTableNext());
		}

		closeTable();//end SkillConfig
	} else return false;

	// 作用范围表
	static DataList<SKILLONERANGE> s_SkillRanges;
	if (openGlobalTable("SkillRangesConf"))
	{
		const INT_PTR nActRangeCount = lua_objlen(m_pLua, -1) + 1;
		s_SkillRanges.count = 0;

		if ( nActRangeCount > 1 )
		{
			s_SkillRanges.count = nActRangeCount;
			s_SkillRanges.pData = (PSKILLONERANGE)m_DataAllocator.allocObjects(nActRangeCount * sizeof(SKILLONERANGE));

			if ( enumTableFirst() )
			{
				//INT_PTR nIdx = 0;
				SKILLONERANGE *pBase = (PSKILLONERANGE)s_SkillRanges.pData;
				do 
				{
					INT_PTR nRangeId = getFieldInt("id");
					//if (nRangeId != ++nIdx)
					//{
					//	showErrorFormat(_T("nRangeId Error id非递增，nRangeId:%d, nIdx:%d"), nRangeId, nIdx);
					//	return false;
					//}
					
					SKILLONERANGE * pRange = pBase + nRangeId;
					{
						pRange->nStartX = (int)getFieldInt("xStart",&nDef_0);
						pRange->nEndX = (int)getFieldInt("xEnd",&nDef_0);
						pRange->nStartY = (int)getFieldInt("yStart",&nDef_0);
						pRange->nEndY = (int)getFieldInt("yEnd",&nDef_0);
						pRange->rangeType = (int)getFieldInt("rangeType",&nDef_0);
						pRange->rangeCenter = (int)getFieldInt("rangeCenter",&nDef_0);
						pRange->nForceHitTargetDis = (int)getFieldInt("forceHitTargetDis",&nDef_0);
						pRange->nDelay = getFieldInt("delay",&nDef_0);
						
						//目标筛选条件
						if ( feildTableExists("conds") && openFieldTable("conds") )
						{
							const INT_PTR nCount = lua_objlen(m_pLua, -1);
							pRange->targetConditions.count = 0;

							if ( nCount > 0 )
							{
								pRange->targetConditions.count = nCount;
								pRange->targetConditions.pData =(PTARGETSELCONDITION)m_DataAllocator.allocObjects(nCount * sizeof(TARGETSELCONDITION));

								if ( enumTableFirst() )
								{
									INT_PTR nIdx = 0;
									PTARGETSELCONDITION pBase =(PTARGETSELCONDITION)pRange->targetConditions.pData;
									do 
									{
										TARGETSELCONDITION * pCondiition = pBase + nIdx;
										pCondiition->nKey =(int)getFieldInt("cond");
										int nValue =  getFieldInt("param",&nDef_0);
										if(nValue)
										{
											pCondiition->value.wLo = (WORD)getFieldInt("value",&nDef_0);
											pCondiition->value.wHi = (WORD)nValue;
										}
										else
										{
											pCondiition->value.nValue = getFieldInt("value",&nDef_0);
										}
										nIdx++;
									}
									while (enumTableNext());
								}
							}
							closeTable();
						}

						//读取技能结果
						if ( feildTableExists("results") && openFieldTable("results") )
						{
							const INT_PTR nCount = lua_objlen(m_pLua, -1);
							pRange->skillResults.count = 0;

							if ( nCount > 0 )
							{
								pRange->skillResults.count = nCount;
								pRange->skillResults.pData =(SKILLRESULT *) m_DataAllocator.allocObjects(sizeof(SKILLRESULT) *nCount);

								if ( enumTableFirst() )
								{
									INT_PTR nIdx = 0;
									SKILLRESULT *pBase = pRange->skillResults.pData;
									do 
									{
										SKILLRESULT * pResult = pBase + nIdx;
										//读取1个等级的数据
										int nDef = 0;
										pResult->nId = getFieldInt("id",&nDef);
										pResult->nValue = getFieldInt("value",&nDef);
										pResult->nDelay = getFieldInt("delay",&nDef);
										pResult->nResultType = getFieldInt("resultType",&nDef);
										pResult->nParam1 = getFieldInt("param1",&nDef);
										pResult->nParam2 = getFieldInt("param2",&nDef);
										pResult->nParam3 = getFieldInt("param3",&nDef);
										pResult->nParam4 = getFieldInt("param4",&nDef);
										pResult->nParam5 = getFieldInt("param5",&nDef);
										pResult->nParam6 = getFieldInt("param6",&nDef);
										pResult->ignoreTargetDis=(BYTE)getFieldInt("ignoreTargetDis",&nDef); //是否忽视和目标的距离
										pResult->bBuffType = (BYTE)getFieldInt("buffType",&nDef);
										nIdx++;
									}
									while (enumTableNext());
								}
							}
							closeTable();
						}
						
					}
				}
				while (enumTableNext());
			}
		}
		closeTable();//end ActionRanges
	} else return false;

	//技能等级总表
	if (openGlobalTable("SkillsLevelConf"))
	{
		OneSkillData *pSkills = this->Inherited::operator OneSkillData* ();
		INT_PTR nSkillCount = this->Inherited::count();
		for (size_t nSkillId = 1; nSkillId < nSkillCount; nSkillId++)
		{
			//对应技能
			OneSkillData * pSkill = &pSkills[nSkillId];
			if (pSkill->nSkillID != nSkillId)
			{
				showErrorFormat(_T("unexpected skill id %d, %d expected"), pSkill->nSkillID, nSkillId);
				return false;
			}

			//打开对应技能的等级表
			if(getFieldIndexTable(nSkillId))
			{
				// 分配该技能的等级空间
				const INT_PTR nLevelCount = (INT_PTR)lua_objlen(m_pLua, -1) + 1;
				pSkill->levels.count = nLevelCount;
				SKILLONELEVEL *pBaseLevel = pSkill->levels.pData =(PSKILLONELEVEL)m_DataAllocator.allocObjects(nLevelCount * sizeof(SKILLONELEVEL));
				
				// 遍历各个等级
				if (enumTableFirst())
				{
					//INT_PTR nLevelIdx = 0;
					do 
					{
						INT_PTR nSkillId = getFieldInt("id");
						if (pSkill->nSkillID != nSkillId)
						{
							showErrorFormat(_T("nSkillId Error id:%d, nSkillId:%d"), pSkill->nSkillID, nSkillId);
							return false;
						}
						INT_PTR nLevel = getFieldInt("level");
						//if (nLevel != ++nLevelIdx)
						//{
						//	showErrorFormat(_T("nSkillLevel Error nLevel:%d, nLevelIdx:%d"), nLevel, nLevelIdx);
						//	return false;
						//}
						SKILLONELEVEL * pLevel  = pBaseLevel + nLevel;

						// 技能等级基本信息
						{
							pLevel->nLevel			= nLevel;								//技能等级
							pLevel->nSingTime		= getFieldInt("delayTime", &nDef_0);	//延迟时间
							pLevel->nCooldownTimes	= getFieldInt("cooldownTime", &nDef_0);	//冷却时间
							pLevel->nAttrIdx		= getFieldInt("attrIdx", &nDef_1);	//技能增加的属性索引（不是results）
							pLevel->isSceneEffect	= getFieldInt("isSceneEffect", &nDef_0) ? true:false;	//是否为场景特效
							pLevel->nHitId			= getFieldInt("hitId", &nDef_0);	//技能特效
							pLevel->nMaxPet			= getFieldInt("maxpet", &nDef_0);	//诱惑最大抓取宠物数
							
						}
						bool bDef = true;
						// 施法条件
						if ( feildTableExists("spellConds") && openFieldTable("spellConds") )
						{
							const INT_PTR nSpellCondsCount = (INT_PTR)lua_objlen(m_pLua, -1);

							if ( nSpellCondsCount <= 0 )
							{
								pLevel->spellConditions.count =0;
							}
							else
							{
								pLevel->spellConditions.count = nSpellCondsCount;
								pLevel->spellConditions.pData =(PSKILLTRAINSPELLCONDITION)m_DataAllocator.allocObjects(nSpellCondsCount *sizeof(SKILLTRAINSPELLCONDITION));
								
								if ( enumTableFirst() )
								{
									INT_PTR nSpellCondsIdx = 0;
									SKILLTRAINSPELLCONDITION *pBase = pLevel->spellConditions.pData;
									do 
									{
										bool boFalse = false;
										SKILLTRAINSPELLCONDITION * pCondition = pBase + nSpellCondsIdx;
										pCondition->nConditionID  = (BYTE)getFieldInt("cond");				//条件类型
										pCondition->nValue = getFieldInt64("value");   						//值
										pCondition->bConsumed =(BYTE)( getFieldBoolean("consume",&boFalse) ? 1 :0 );	//是否消耗
										pCondition->nCount = getFieldInt("count",&nDef_0);
										nSpellCondsIdx++;
									}
									while (enumTableNext());
								}
							}
							closeTable();
						}//end spellConds

						// 升级条件
						if ( feildTableExists("upgradeConds") && openFieldTable("upgradeConds") )
						{
							const INT_PTR nUpgradeCondsCount = lua_objlen(m_pLua, -1);

							if ( nUpgradeCondsCount <= 0 )
							{
								pLevel->trainConditions.count =0;
							}
							else
							{
								pLevel->trainConditions.count = nUpgradeCondsCount;
								pLevel->trainConditions.pData = (PSKILLTRAINSPELLCONDITION)m_DataAllocator.allocObjects(nUpgradeCondsCount *sizeof(SKILLTRAINSPELLCONDITION));
								
								if ( enumTableFirst() )
								{
									INT_PTR nUpgradeCondsIdx = 0;
									SKILLTRAINSPELLCONDITION *pBase =(PSKILLTRAINSPELLCONDITION)pLevel->trainConditions.pData;
									do 
									{
										SKILLTRAINSPELLCONDITION * pCondition = pBase + nUpgradeCondsIdx;
										pCondition->nConditionID  = (BYTE)getFieldInt("cond");				//条件类型
										pCondition->nValue = getFieldInt64("value");   						//值
										pCondition->bConsumed =(BYTE)( getFieldBoolean("consume") ? 1 :0 );	//是否消耗
										pCondition->nCount = getFieldInt("count",&nDef_1);
										nUpgradeCondsIdx++;
									}
									while (enumTableNext());
								}
							}
							closeTable();
						}//end upgradeConds

						// 技能效果范围
						if ( feildTableExists("actRange") && openFieldTable("actRange") )
						{
							const INT_PTR nActRangeCount = lua_objlen(m_pLua, -1);
							pLevel->pranges.count = 0;

							if ( nActRangeCount > 0 )
							{
								pLevel->pranges.count = nActRangeCount;
								PSKILLONERANGE *pBase = pLevel->pranges.pData = (PSKILLONERANGE*)m_DataAllocator.allocObjects(nActRangeCount * sizeof(PSKILLONERANGE));
								
								if ( enumTableFirst() )
								{
									INT_PTR nIdx = 0;
									do 
									{
										int id = getFieldInt(NULL);
										PSKILLONERANGE * pRange = pBase + nIdx;
										if(id >= s_SkillRanges.count)
										{
											showErrorFormat(_T("RangeId Error nIdx:%d, id:%d"), nIdx, id);
											return false;
										}
										*pRange = (PSKILLONERANGE)(s_SkillRanges.pData) + id;
										nIdx++;
									}
									while (enumTableNext());
								}
							}
							closeTable();
						}//end actRange
					}
					while (enumTableNext());
				}

				closeTable();//end SkillsLevelConfig[nSkillId]
			}
			else
			{
				OutputMsg(rmError, "open SkillsLevelConfig[%d] failed", nSkillId);
				return false;
			}
		}
		closeTable();//end SkillsLevelConfig
	} else return false;
	
	OutputMsg(rmTip,_T("Load skill Finished"));
	return true;
}

void CSkillProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

