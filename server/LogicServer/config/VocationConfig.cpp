#pragma once
#include "StdAfx.h"
#include "LogicDataProvider.h"
 bool CLogicDataProvider:: LoadVocationConfig(LPCTSTR sFilePath)
{
	// 角色初始化配置
	PVOCATIONINITTABLE pData = GetLogicServer()->GetDataProvider()->GetVocationConfig();
	if (openGlobalTable("ActorInitConfig"))
	{
		int nDef =100;
		int defaultPos = 3;

		pData->nDefaultMoveSpeed = getFieldInt("nMoveSpeed"); //读取
		pData->nDefaultAttackSpeed =  getFieldInt("nDefaultAttackSpeed"); //默认的攻击速度

		double fDef =0.15;
		fDef =0.95;
		
		nDef = 8500; 
		nDef =15;
		fDef =0.01;
		fDef =0.5;
		defaultPos= 50;

		if ( feildTableExists("InitAttrs") && openFieldTable("InitAttrs") )
		{
			PGAMEATTR pAttr = NULL;
			pData->initAttrs.nCount  = (int)lua_objlen(m_pLua,-1);
			if( pData->initAttrs.nCount > 0 )
			{
				pData->initAttrs.pAttrs  = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * pData->initAttrs.nCount);
				ZeroMemory(pData->initAttrs.pAttrs,	sizeof(GAMEATTR) * pData->initAttrs.nCount);
				pAttr = pData->initAttrs.pAttrs;
			}
			if (enumTableFirst())
			{
				if (!LoadAttri(pAttr)) 
				{
					return false;
				}
				pAttr ++;
			}//end if
			closeTable();
		}
		if(feildTableExists("nCreateInitFb"))
		{
			if ( openFieldTable("nCreateInitFb") ==false)
			{
				return false;
			}
			else
			{
				if (enumTableFirst())
				{
					do
					{
						CRESTEDEFAULTFb tmpDefaultFb;
						tmpDefaultFb.nToken = getFieldInt("token");
						tmpDefaultFb.nFbId = getFieldInt("nFbId");

						pData->nDefualtFb.add(tmpDefaultFb);

						int nCount = (int)pData->nDefualtFb.count();
						if ( openFieldTable("createPos") ==false)
						{
							return false;
						}
						else
						{
							int posCount = 0;
							pData->nDefualtFb[nCount-1].nPosCount = 0;
							if (enumTableFirst())
							{
								do
								{
									if (posCount >= MAX_CREATE_POSNUM) break;
									
									pData->nDefualtFb[nCount-1].nCreatePos[posCount].nScenceID =  getFieldInt("nScenceID");
									pData->nDefualtFb[nCount-1].nCreatePos[posCount].nPosX =  getFieldInt("nPosX");
									pData->nDefualtFb[nCount-1].nCreatePos[posCount].nPosY =  getFieldInt("nPosY");
									pData->nDefualtFb[nCount-1].nPosCount++;
									posCount++;
								}while (enumTableNext());
							}
						}

						closeTable();

					} while (enumTableNext());
				}

				closeTable();
			}
		}
		closeTable();
	} else return false;
	
	// 角色职业配置
	if (openGlobalTable("VocationConfig"))
	{
		INT_PTR i =0;
		if (enumTableFirst())
		{
			do
			{
				unsigned id = getFieldInt("id");
				if (id >= enMaxVocCount) return false;
				PVOCATIONINITCONFIG pVoc = &(pData->vocationTable[id]);

				
				//pVoc->nFbID = getFieldInt("nFbID");
				//pVoc->nScenceID = getFieldInt("nScenceID");
				//pVoc->nPosX = getFieldInt("nPosX");
				//pVoc->nPosY = getFieldInt("nPosY");
				pVoc->actorProps.count =0;
				pVoc->nDefaultMaleModelID = getFieldInt("nDefaultMaleModelID");
				pVoc->nDefaultFemaleModelID = getFieldInt("nDefaultFemaleModelID");
				pVoc->nNearAttackType =getFieldInt("nNearAttackType");
				int nDefaultIcon =0;
				pVoc->nManIcon = getFieldInt("nManIcon",&nDefaultIcon);
				pVoc->nFemaleIcon = getFieldInt("nFemaleIcon",&nDefaultIcon);
				int nDefaultDis =1;
				pVoc->nNearAttackDis =getFieldInt("nNearAttackDis",&nDefaultDis); //
				nDefaultDis = 2000;
				pVoc->nMaxAnger = getFieldInt("nMaxAnger",&nDefaultDis);
				pVoc->heroAiId = getFieldInt("heroAiId",&nDefaultDis,0);
				int nDefaultSkill = 0;
				pVoc->heroDefaultSkill = getFieldInt("heroDefaultSkill", &nDefaultSkill);
				int nDefaultFireDef = 0;
				pVoc->nFireDefenseRate = getFieldInt("nFireDefenseRate", &nDefaultFireDef);

				//人物转生属性加成
				if(feildTableExists("circleAddProp") && openFieldTable("circleAddProp"))
				{
					const INT_PTR nCount = (INT_PTR)lua_objlen(m_pLua, -1) ;
					if ( nCount <= 0 )
					{
						pVoc->circleAddAttrs.count =0; //没有数据
					}
					else
					{
						pVoc->circleAddAttrs.count = nCount; //转数属性数据
						pVoc->circleAddAttrs.pData = (CStdItem::AttributeGroup *)m_DataAllocator.allocObjects(nCount * sizeof(CStdItem::AttributeGroup));
						if ( enumTableFirst() )
						{
							INT_PTR nIdx = 0;

							CStdItem::AttributeGroup * pBaseData =(CStdItem::AttributeGroup *)pVoc->circleAddAttrs.pData;
							CStdItem::AttributeGroup * pOneData =NULL;
							do 
							{					
								pOneData = pBaseData + nIdx;
								if (!ReadAttributeTable(m_DataAllocator, pOneData))
								{
									OutputMsg(rmError, "Read circle add prop error!!");
									return false;
								}

								nIdx++;
							}while (enumTableNext());
						}
					}
					closeTable();
				}

				i++;
			} while (enumTableNext());
		}
		closeTable();
	} else return false;

	// 角色职业等级属性配置
	const int ROLE_JOB_COUNT = 3;
	if (openGlobalTable("LevelPropConfig"))
	{
		for (size_t i = 1; i <= ROLE_JOB_COUNT; i++)
		{
			int lv = 0;
			int job = 0;

			if (!getFieldIndexTable(i))
			{
				OutputMsg(rmError, "open job attr failed. l:%d, j:%d", lv, i);
				return false;
			}

			PVOCATIONINITCONFIG pVoc = &(pData->vocationTable[i]);
			const INT_PTR nCount = (INT_PTR)lua_objlen(m_pLua, -1) + 1;
			if ( nCount <= 0 )
			{
				pVoc->actorProps.count =0; //没有数据
			}
			else
			{
				pVoc->actorProps.count = nCount; //数据
				pVoc->actorProps.pData = (ENTITYLEVELPROPERTY *)m_DataAllocator.allocObjects(nCount * sizeof(ENTITYLEVELPROPERTY));
				ENTITYLEVELPROPERTY * pBaseData = (ENTITYLEVELPROPERTY *)pVoc->actorProps.pData;
				ENTITYLEVELPROPERTY * pOneData = NULL;
				if ( enumTableFirst() )
				{
					do 
					{
						int nlevel = getFieldInt("level");
						if (nlevel >= nCount)
						{
							OutputMsg(rmError, "读取等级配置错误. nlevel:%d, max:%d", nlevel, nCount);
							return false;
						}
						
						pOneData = pBaseData + nlevel;
						int ndef = 0;
						pOneData->levelProp[0] = getFieldInt("nMaxHpAdd", &ndef);
						pOneData->levelProp[1] = getFieldInt("nMaxMpAdd", &ndef);
						pOneData->levelProp[2] = getFieldInt("nPhysicalAttackMinAdd", &ndef);
						pOneData->levelProp[3] = getFieldInt("nPhysicalAttackMaxAdd", &ndef);
						pOneData->levelProp[4] = getFieldInt("nMagicAttackMinAdd", &ndef);
						pOneData->levelProp[5] = getFieldInt("nMagicAttackMaxAdd", &ndef);
						pOneData->levelProp[6] = getFieldInt("nWizardAttackMinAdd", &ndef);
						pOneData->levelProp[7] = getFieldInt("nWizardAttackMaxAdd", &ndef);
						pOneData->levelProp[8] = getFieldInt("nPhysicalDefenceMinAdd", &ndef);
						pOneData->levelProp[9] = getFieldInt("nPhysicalDefenceMaxAdd", &ndef);
						pOneData->levelProp[10] = getFieldInt("nMagicDefenceMinAdd", &ndef);
						pOneData->levelProp[11] = getFieldInt("nMagicDefenceMaxAdd", &ndef);
						pOneData->levelProp[12] = getFieldInt("nHitValueAdd", &ndef);
						pOneData->levelProp[13] = getFieldInt("nDogValueAdd", &ndef);
						pOneData->levelProp[14] = getFieldInt("nMagicHitRateAdd", &ndef);
						pOneData->levelProp[15] = getFieldInt("nMagicDogerateAdd", &ndef);
					}while (enumTableNext());
				}
			}
			closeTable();
		}
		closeTable();
	} else return false;

	return true;
};

