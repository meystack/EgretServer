#pragma once
#include "StdAfx.h"
#include "LogicDataProvider.h"
#include "../base/Container.hpp"
bool CLogicDataProvider::LoadProperty(PLEVEL1PROPERTYTRANSFER pConfig,char * tableName)
{
	if (!openGlobalTable(tableName) )
	{
		return false;
	}
	INT_PTR i =0;
	if (enumTableFirst())
	{		
		INT_PTR i =0;	
		do
		{
			if( i >= MAX_LEVEL1_PROPERTY_COUNT ) return false;

			INT_PTR j=0;
			if (enumTableFirst())
			{
				do 
				{
					if (j >= MAX_LEVEL2_PROPERTY_COUNT ) return false;
					pConfig->m_level1PropertyTransTable[i][j]= (float)getFieldNumber(NULL);
					j++;
				} while (enumTableNext());
			}
			i++;
		} while (enumTableNext());
	}
	closeTable();
	return true;

}
 bool CLogicDataProvider:: LoadPropertyConfig(LPCTSTR sFilePath)
{

	LoadExpGetUpperLmtConfig();
	LoadActorNameColorConfig();
	//LoadLevelUpExpTable();
	LoadKillMonsterExpReduceTable();
	//LoadGlobalConfig(); //--还需要吗？
	LoadTeamKillMonsterExp();
	LoadTeamKillMonsterExpBuf();
	LoadRenownLimit();
	return true;
};

 bool CLogicDataProvider::LoadNameColorItem(const char *pItemName, unsigned int &clr)
 {
	 clr = DEFAULT_NAME_COLOR; 
	 if (!pItemName)  return false;
	 if (feildTableExists(pItemName) && openFieldTable(pItemName))
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

	 return true;
 }

 bool CLogicDataProvider::LoadActorNameColorConfig()
 {
	 if (!openGlobalTable("NameColor"))
	 {
		 OutputMsg(rmError, _T("NameColor 表找不到"));
		 return false;
	 }

	 // read all color config item
	 LoadNameColorItem("clr_normal",		m_nameClrTable.clr_normal);
	 LoadNameColorItem("clr_pk_otherside",	m_nameClrTable.clr_pk_otherside);
	 LoadNameColorItem("clr_pkval_over300", m_nameClrTable.clr_pkval_over300);
	 LoadNameColorItem("clr_pkval_over100", m_nameClrTable.clr_pkval_over100);
	 LoadNameColorItem("clr_pkval_over60",	m_nameClrTable.clr_pkval_over60);
	 LoadNameColorItem("clr_force_pk",		m_nameClrTable.clr_force_pk);
	 LoadNameColorItem("clr_teammates",		m_nameClrTable.clr_teammates);
	 LoadNameColorItem("clr_pkmode",		m_nameClrTable.clr_pkmode);
	 LoadNameColorItem("clr_slave",			m_nameClrTable.clr_slave);

	 LoadNameColorItem("clr_quality_white",			m_nameClrTable.clr_quality_white);
	 LoadNameColorItem("clr_quality_green",			m_nameClrTable.clr_quality_green);
	 LoadNameColorItem("clr_quality_blue",			m_nameClrTable.clr_quality_blue);
	 LoadNameColorItem("clr_quality_puple",			m_nameClrTable.clr_quality_puple);
	 LoadNameColorItem("clr_quality_orange",		m_nameClrTable.clr_quality_orange);
	 LoadNameColorItem("clr_quality_red",			m_nameClrTable.clr_quality_red);

	 closeTable();
	 return true;
 }
 

 bool CLogicDataProvider::LoadExpGetUpperLmtConfig()
 {
	//  CVector<unsigned int> &vecExpGetUpperLmt = GetLogicServer()->GetDataProvider()->GetExpGetUpperLmtCfg();
	//  vecExpGetUpperLmt.clear();
	//  if (!openGlobalTable("ExpUpLmtEveryDay"))
	//  {
	// 	 OutputMsg(rmError, _T("ExpUpLmtEveryDay 表找不到"));
	// 	 return false;
	//  }

	//  INT_PTR nCount = lua_objlen(m_pLua, -1);	 
	//  if (nCount > 0)
	//  {
	// 	 vecExpGetUpperLmt.reserve(nCount);
	// 	 if (enumTableFirst())
	// 	 {
	// 		 do 
	// 		 {
	// 			 vecExpGetUpperLmt.push((unsigned int)getFieldNumber(NULL));
	// 		 } while (enumTableNext());
	// 	 }

	//  }
	//  closeTable();
	 return true;
 }

 //装载经验升级表
 bool CLogicDataProvider::LoadLevelUpExpTable()
 {
	 //副本配置表
	 LEVELUPEXPTABLE &data = GetLogicServer()->GetDataProvider()->GetLevelUpExp();
	 if (!openGlobalTable("LevelUpExp"))
	 {
		 OutputMsg(rmWaning,_T("LevelUpExp 表找不到"));
		 return false;
	 }


	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	 if(nCount  <=0)
	 {
		 OutputMsg(rmWaning,_T("LevelUpExp 表为空"));
		 return true;
	 }
	 nCount = nCount + 1;
	 if(data.expTable.count() >0)
	 {
		 data.expTable.empty();
	 }
	 if(data.expRateTable.count() >0)
	 {
		 data.expRateTable.empty();
	 }

	 double nDefv = 0;
	 double nDefratev = 0 ;
	 data.expTable.reserve(nCount);
	 data.expTable.trunc(nCount);
	 data.expTable[0] = 0;

	 data.expRateTable.reserve(nCount);
	 data.expRateTable.trunc(nCount);
	 data.expRateTable[0] = 0;

	 if ( enumTableFirst() )
	 {
		 do 
		 {
			int level  = (int)getFieldNumber("level");
			LONGLONG value  = (LONGLONG)getFieldNumber("value", &nDefv);
			LONGLONG rate  = (LONGLONG)getFieldNumber("rate", &nDefratev);
			if (level >= nCount) return false;
			
			data.expTable[level] = (LONGLONG)value;
			data.expRateTable[level] = (LONGLONG)rate;
		 }
		 while (enumTableNext());
	 }
	 closeTable();
	 return true;
 }


 //装载每日经验上限表，与开服天数相关
 bool CLogicDataProvider::LoadExpLmtOpendayTable()
 {
	 //副本配置表
	 EXPLMTOPENDAY &dataLocalTable = GetLogicServer()->GetDataProvider()->GetExpLmtOpenday();
	 if (!openGlobalTable("ExpLmtOpenday"))
	 {
		 OutputMsg(rmWaning,_T("ExpLmtOpenday 表找不到"));
		 return false;
	 }

	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	 if(nCount  <=0)
	 {
		 OutputMsg(rmWaning,_T("ExpLmtOpenday 表为空"));
		 return true;
	 }
	 if(dataLocalTable.m_expLmtOpenday.size() >0)
	 {
		 dataLocalTable.m_expLmtOpenday.clear();
	 }

	 double nDefv = 0;
	ELEMTYPEOPENDAY pleholder(0);
	 dataLocalTable.m_expLmtOpenday[0] = pleholder;

	if ( enumTableFirst() )
	{
		 do 
		{
			ELEMTYPEOPENDAY tmp(0);
			int openday  = (int)getFieldNumber("openday");
			tmp.multilmt  = (LONGLONG)getFieldNumber("multiexplmt", &nDefv);
			if (openFieldTable("basicexplmt"))
			{
				
				if (enumTableFirst())
				{
					do
					{
					int idx  = (int)getFieldNumber("id");
					LONGLONG _num = (LONGLONG)getFieldNumber("num", &nDefv);
					int _damprate = (int)getFieldNumber("damprate");
					BASICLMT gg(_num, _damprate);
					tmp.m_basiclmt[idx]= gg ;
							
					} while (enumTableNext());
				}
			closeTable();
			}
			dataLocalTable.m_expLmtOpenday[openday] = tmp ;
		}
		 while (enumTableNext());
	}
	closeTable();
	return true;
 }




 bool CLogicDataProvider::LoadKillMonsterExpReduceTable()
 {
	 //副本配置表
	 KILLMONSTEREXPTABLE &data = GetLogicServer()->GetDataProvider()->GetKillMonsterExpReduce();
	 if (!openGlobalTable("KillLevelAboveMonsterExp"))
	 {
		 OutputMsg(rmWaning,_T("KillLevelAboveMonsterExp 表找不到"));
		 return false;
	 }


	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	 if(nCount  <=0)
	 {
		 OutputMsg(rmWaning,_T("KillLevelAboveMonsterExp 表为空"));
		 return true;
	 }
	 if(data.killMonsterAboveExpTable.count() >0)
	 {
		 data.killMonsterAboveExpTable.empty();
	 }
	 data.killMonsterAboveExpTable.reserve(nCount);
	 if ( enumTableFirst() )
	 {
		 INT_PTR nIdx = 0;
		 do 
		 {
			 data.killMonsterAboveExpTable.add( (WORD)getFieldNumber(NULL) );
			 nIdx ++;
		 }
		 while (enumTableNext());
	 }
	 closeTable();



	 if (!openGlobalTable("KillLevelBelowMonsterExp"))
	 {
		 OutputMsg(rmWaning,_T("KillLevelBelowMonsterExp 表找不到"));
		 return false;
	 }


	  nCount = lua_objlen(m_pLua, -1);
	 if(nCount  <=0)
	 {
		 OutputMsg(rmWaning,_T("KillLevelBelowMonsterExp 表为空"));
		 return true;
	 }
	 if(data.killMonsterBelowExpTable.count() >0)
	 {
		 data.killMonsterBelowExpTable.empty();
	 }
	 data.killMonsterBelowExpTable.reserve(nCount);
	 if ( enumTableFirst() )
	 {
		 INT_PTR nIdx = 0;
		 do 
		 {
			 data.killMonsterBelowExpTable.add( (WORD)getFieldNumber(NULL) );
			 nIdx ++;
		 }
		 while (enumTableNext());
	 }
	 closeTable();

	 return true;
 }

bool CLogicDataProvider::LoadZSLevelExpPercentConfig(LPCTSTR sFilePath)
{
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on ZSLevelExpPercent config"));
		} else {
			Result = LoadZSLevelExpPercentConfig();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load ZSLevelExpPercent error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on ZSLevelExpPercent "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

 bool CLogicDataProvider::LoadZSLevelExpPercentConfig()
 {
	KILLMONSTEREXPTABLE &data = GetLogicServer()->GetDataProvider()->GetKillMonsterExpReduce();
	data.killMonsterExpMap.clear();
	if (openGlobalTable("ZSLevelExpPercentConfig"))
	{
		if(enumTableFirst())
		{
			int ndef_key = 0, ndef_value = 0;
			do
			{
				data.killMonsterExpMap.insert(std::make_pair(getFieldInt("ZSlv", &ndef_key), getFieldInt("ExpPercent", &ndef_value)));
			} while(enumTableNext());
		} else {
			OutputMsg(rmWaning,_T("ZSLevelExpPercentConfig 表为空"));
			return false;
		}
		closeTable();
		return true;
	} else {
		OutputMsg(rmWaning,_T("ZSLevelExpPercentConfig 表找不到"));
		return false;
	}
 }
 
bool CLogicDataProvider::LoadGlobalConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadGlobalConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadGlobalConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CLogicDataProvider::LoadGlobalConfig()
{
	//副本配置表
	GLOBALCONFIG &data = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	if (!openGlobalTable("GlobalConf"))
	{
		OutputMsg(rmWaning,_T("GlobalConf 表找不到"));
		return false;
	}

	int nDef_0 = 0;
	data.nBlinkEnterEffect = getFieldInt("BlinkEnterEffect", &nDef_0);// 瞬移技能进入场景的特效id
	data.nScrollEnterEffect = getFieldInt("ScrollEnterEffect", &nDef_0);// 回城卷进入场景的技能特效id

	int nDefTime = 15;
//  data.nMonsterItemRefreshTime =(int) getFieldNumber("monsterItemRefreshTime");
	data.teamLootMaxDistanceSquare = (int) getFieldNumber("teamLootMaxDistanceSquare"); //传送门的传送距离

	data.nInitSceneId = (int)getFieldInt("initSceneId");
	data.nInitScenePosX = (int)getFieldInt("initScenePosX");
	data.nInitScenePosY = (int)getFieldInt("initScenePosY");

	data.nOnlineCountBase = (int)getFieldInt("nOnlineCountBase");

	data.nSceneRefreshMonsterTime = getFieldInt("sceneRefreshMonsterTime");

	data.nFubenDestoryTime = getFieldInt("fubenDestoryTime");
	data.nFubenDestoryTime = data.nFubenDestoryTime * 60;
	 
	data.nCrossMsgDay = (unsigned int)getFieldInt("CrossMsgDay");  //跨服消息限制

	data.bIsFcmOpen = getFieldBoolean("openFcm"); //防沉迷是否开启
	data.nFcmHalfExpMoneyTime = (unsigned int)getFieldInt("fcmHalfExpMoneyTime");  //防沉迷开始经验、金钱减半的时间(单位毫秒)
	data.nFcmZeroExpMoneyTime = (unsigned int)getFieldInt("fcmZeroExpMoneyTime");  //防沉迷开始经验、金钱没有的时间(单位毫秒)

	//盛趣防沉迷参数
	//SqKidsFcm = {{open=0,KidsFcmHolidayTime=300,KidsFcmWorkDayTime=180}},
	if (feildTableExists("SqKidsFcm") && openFieldTable("SqKidsFcm"))
	{
		if (enumTableFirst())
		{
			do 
			{
				int opentype =getFieldInt("open", &nDef_0);
				if(opentype <= 0)
				{
					data.bIsSqKidsFcmOpen = false; 
					enumTableNext();
					break;
				}
				//盛趣儿童防沉迷是否开启
				data.bIsSqKidsFcmOpen = true;

				data.nKidsFcmHolidayTime = getFieldInt("KidsFcmTime", &nDef_0);
				data.nKidsFcmWorkDayTime = getFieldInt("KidsFcmWorkDayTime", &nDef_0);  

				getFieldStringBuffer("KickSTime", data.strKidsFcmHolidayStartTime, GLOBAL_TIME_LEN);
				getFieldStringBuffer("KickETime", data.strKidsFcmHolidayEndTime, GLOBAL_TIME_LEN);
				getFieldStringBuffer("KickSTimeWD", data.strKidsFcmWorkDayStartTime, GLOBAL_TIME_LEN);
				getFieldStringBuffer("KickETimeWD", data.strKidsFcmWorkDayEndTime, GLOBAL_TIME_LEN);
			} while (enumTableNext());
		}		
		closeTable();	
	}
 

	int step = 0;
	if (feildTableExists("suckblood") && openFieldTable("suckblood"))
	{
		if (enumTableFirst())
		{
			do 
			{
				int rate = getFieldInt(NULL);
				if (0 == step)
				{
					data.nSuckBloodRate = rate;
				}
				else
				{ 
					data.nSuckBloodRatePlayer = rate;
				}
				
				step ++;
			} while (enumTableNext());
		}		
		closeTable();	
	}

	int nIndex = 0;
	if (feildTableExists("zhicanbuff") && openFieldTable("zhicanbuff"))
	{
		if (enumTableFirst())
		{
			do 
			{
				switch (nIndex)
				{
				case 0: //致残特效Id
					data.nCrippleEffectId = getFieldInt(NULL, &nDef_0);
					break;
				case 1:	//高级致残特效Id
					data.nAdvancedCrippleEffectId = getFieldInt(NULL, &nDef_0);
					break;
				default:
					break;
				}
				
				nIndex ++;
			} while (enumTableNext());
		}		
		closeTable();	
	}

	// 攻击、受击耐久消耗相关配置
	int nDefConsTotalCnt = 3, nDefConsRandCnt = 1, nLowerLimt = 1, nUpperLimit = 3, nDefEndureConsOnDeath = 10;
	//  data.nWeaponEndureConsumeTotalCnt		= getFieldInt("weaponEndureConsTotalCnt",			&nDefConsTotalCnt);
	//  data.nWeaponEndureConsumeRandCnt		= getFieldInt("weaponEndureConsRandCnt",			&nDefConsRandCnt);
	//  data.nDressEndureConsumeTotalCnt		= getFieldInt("dressEndureConsTotalCnt",			&nDefConsTotalCnt);
	//  data.nDressEndureConsumeRandCnt		= getFieldInt("dressEndureConsRandCnt",				&nDefConsRandCnt);	 	 
	//  data.nArmorEndureConsumeTotalCnt		= getFieldInt("armorEndureConsTotalCnt",			&nDefConsTotalCnt);
	//  data.nArmorEndureConsumeRandCnt		= getFieldInt("armorEndureConsRandCnt",				&nDefConsRandCnt);
	//  data.nWeaponEndureConsLowerLimit		= getFieldInt("weaponEndureConsLowerLimit",			&nLowerLimt);
	//  data.nWeaponEndureConsUpperLimit		= getFieldInt("weaponEndureConsUpperLimit",			&nUpperLimit);
	//  data.nDressEndureConsLowerLimit		= getFieldInt("dressEndureConsLowerLimit",			&nLowerLimt);
	//  data.nDressEndureConsUpperLimit		= getFieldInt("dressEndureConsUpperLimit",			&nUpperLimit);
	//  data.nArmorEndureConsLowerLimit		= getFieldInt("armorEndureConsLowerLimit",			&nLowerLimt);
	//  data.nArmorEndureConsUpperLimit		= getFieldInt("armorEndureConsUpperLimit",			&nUpperLimit);
	//  data.nEndureConsRateOnDeath			= getFieldInt("endureConsRateOnDeath",				&nDefEndureConsOnDeath);
	int nDefFriendChatLimit = 30;
	data.nFriendChatLimit					= getFieldInt("friendChatLmt",						&nDefFriendChatLimit);
	//int nDefChannelCD = 3000;
	//  data.nChatChannelCD[ciChannelSecret]			= getFieldInt("secretChannelChatCD",				&nDefChannelCD) * 1000;	 
	//  data.nChatChannelCD[ciChannelNear]				= getFieldInt("nearChannelChatCD",					&nDefChannelCD) * 1000;
	//  //data.nChatChannelCD[ciChannelHorn]				= getFieldInt("hornChannelChatCD",					&nDefChannelCD) * 1000;
	//  data.nChatChannelCD[ciChannelGuild]			= getFieldInt("guideChannelChatCD",					&nDefChannelCD) * 1000;
	// // data.nChatChannelCD[ciChannelMap]				= getFieldInt("localMapChannelChatCD",				&nDefChannelCD) * 1000;
	//  //data.nChatChannelCD[ciChannelZhenying]			= getFieldInt("occupationChannelChatCD",			&nDefChannelCD) * 1000;
	//  data.nChatChannelCD[ciChannelTeam]				= 0;
	//  //data.nChatChannelCD[ciChannelBigTeam]			= 0;
	//  //data.nChatChannelCD[ciChannelTipmsg]			= 0;
	//  data.nChatChannelCD[ciChannelSystem]			= 0;
	//  data.nChatChannelCD[ciChannelWorld]			= getFieldInt("worldChannelChatCD",					&nDefChannelCD) * 1000;
	//  //data.nChatChannelCD[ciChannelSell]				= getFieldInt("sellChannelChatCD",					&nDefChannelCD) * 1000;
	//  char sChatLimit[128];
	//  LPCSTR sSPID = GetLogicServer()->GetVSPDefine().GetDefinition(CVSPDefinition::SPID);	 
	//  if(sSPID !=NULL && strcmp(sSPID, "37w") == 0)
	//  {
	// 	 sprintf_s(sChatLimit, ArrayCount(sChatLimit), "sp37wLevelLmt");
	//  }
	//  else
	//  {
	// 	 sprintf_s(sChatLimit, ArrayCount(sChatLimit), "chatLevelLmt");
	//  }
	//  if (feildTableExists(sChatLimit) && openFieldTable(sChatLimit))
	//  {
	// 	 INT_PTR count = lua_objlen(m_pLua, -1);
	// 	 if (count > 0 && enumTableFirst())
	// 	 {
	// 		 int nDefLvlLmt = 1;
	// 		 INT_PTR idx = 0;
	// 		 do 
	// 		 {
	// 			 if (idx < ciChannelMax)
	// 			 {
	// 				 data.nChatLevelLmt[idx++] = getFieldInt(NULL, &nDefLvlLmt);
	// 			 }
	// 			 else
	// 			 {
	// 				 endTableEnum();
	// 				 break;
	// 			 }
	// 		 } while (enumTableNext());
	// 	 }
	// 	 closeTable();
	//  }

	data.nMaxDoubleExpValue = getFieldInt("maxexp",&nDef_0);
	data.nMaxRankTipId = getFieldInt("MaxRankTipId",&nDef_0);
	 if (feildTableExists("MessageColor") && openFieldTable("MessageColor"))
	 {

		if(data.nMessageColor.size() >0)
		{
			data.nMessageColor.clear();
		}
		const INT_PTR nCount = lua_objlen(m_pLua, -1);
		for(int i = 1; i <= nCount ; i++ )
		{
			LPCSTR result = getIndexString(i, NULL);
			char buff[1024];
			//INT_PTR size= getFieldStringBuffer(NULL,buff,sizeof(buff));
			SIZE_T dwLen = strlen(result);
			memcpy(buff, result, dwLen * sizeof(buff[0]));
			buff[dwLen] = 0;
			data.nMessageColor.push_back(buff);
		}
		//  if (nCount > 0 && enumTableFirst())
		//  {
		// 	char buff[1024];
		// 	INT_PTR nIdx = 0;
		// 	do 
		// 	{
		// 		INT_PTR size= getFieldStringBuffer(NULL,buff,sizeof(buff));
		// 		data.nMessageColor.push_back(buff);
		// 		nIdx ++;
		// 	}
		// 	while (enumTableNext());
		//  }
		 closeTable();
	 }
	data.nFirstChargeIds.clear();
	if (feildTableExists("FirstChargeID") && openFieldTable("FirstChargeID"))
	{

		if (enumTableFirst())
		 {
			do 
			{
				INT_PTR id= getFieldInt(NULL);
				data.nFirstChargeIds.push_back(id);
			}
			while (enumTableNext());
		 }
		closeTable();
	}

	memset(&data.QuestEquipData,0,sizeof(data.QuestEquipData));
	if (feildTableExists("QuestEquipData") && openFieldTable("QuestEquipData"))
	{
		INT_PTR count = lua_objlen(m_pLua, -1);
		if (count > 0 && enumTableFirst())
		{
			int nData = 1;
			INT_PTR idx = 0;
			do 
			{
				if (idx < THINGS_BUF_NUM)
				{
					data.QuestEquipData[idx++] = getFieldInt(NULL, &nData);
				}
				else
				{
					endTableEnum();
					break;
				}
			} while (enumTableNext());
		}
		closeTable();
	}

	int nDefault = 6000;
	nDefault = 100;

	nDefault=5000;
	data.nBindYuanbaoLimit = getFieldInt("bindYuanbaoLimit",&nDefault);//绑定元宝上限
	data.nMoneylimit = getFieldInt("moneylimit",&nDefault);//绑定元宝上限

	data.nBindmoneylimit = getFieldInt("bindmoneylimit",&nDefault);//绑定元宝上限
	data.nYuanbaolimit = getFieldInt("yuanbaolimit",&nDefault);//绑定元宝上限
	nDefault =5;
	data.nPetLevelAboveOwner = getFieldInt("petLevelAboveOwner",&nDefault); //宠物的等级最多比玩家多的等级


	double fDef =0.2;
	data.fPetDeathHpRate = (float)getFieldNumber("petDeathHpRate",&fDef);

	nDefault =72;
	nDefault =100;
	nDefault = 159;
	nDefault = 0;

	double fQstDef = 0.0;
	  
	 
	//data.nDropCoinIconId = getFieldInt("nDropCoinIconId", &nDefault); //掉落在地上的金钱使用的图标
	data.nMonsterGropDropRangeMax = getFieldInt("monsterGropDropRangeMax", &nDefault);//掉落组(group>0)掉落区间最大值

	nDefault = 5000;
	data.nMonsterVectResetInterval = getFieldInt("monsterVectResetInterval", &nDefault); //怪物归属重置间隔(单位ms)(5s没有打怪，清除怪物原来归属)
	nDefault = 0;
	data.nMonsterHatredKeepTime = getFieldInt("monsterHatredKeepTime",&nDefault);//boss怪物仇恨值记录时间（秒）（超过重置伤害值）
	double fMonsterHatredHeroRateDefault = 1.0;
	data.nTeamOpenLevel = getFieldInt("teamOpenLevel", &nDefault);
	//  data.fWriteNameBagDropRate = (float)getFieldNumber("fWriteNameBagDropRate", &fDef);	//白名背包掉落系数
	//  data.fWriteNameEquipDropRate = (float)getFieldNumber("fWriteNameEquipDropRate", &fDef);  //白名装备掉落系数
	//  data.fRedNameBagDropRate = (float)getFieldNumber("fRedNameBagDropRate", &fDef);			//红名背包掉落系数
	//  data.fRedNameEquipDropRate = (float)getFieldNumber("fRedNameEquipDropRate", &fDef);		//红名装备掉落系数
	//  data.nWriteNameBagDropNum = getFieldInt("nWriteNameBagDropNum", &nDefault);				//白名掉落数量
	//  data.nRedNameBagDropNum = getFieldInt("nRedNameBagDropNum", &nDefault);					//全部掉出
	//  data.nWriteNameEquipDropNum = getFieldInt("nWriteNameEquipDropNum", &nDefault);			//白名装备掉落数量
	//  data.nRedNameEquipDropNum = getFieldInt("nRedNameEquipDropNum", &nDefault);				//红包装备掉落数量

	//  data.nDeathDropExpTotalRate = getFieldInt("nDeathDropExpTotalRate", &nDefault); //经验掉落总系数,0为不开启,大于0表示掉落总系数
	//  data.nWriteNameExpDropRate = getFieldInt("nWriteNameExpDropRate", &nDefault); //白名经验掉落系数
	//  data.nWriteNameExpDropMax = getFieldInt("nWriteNameExpDropMax", &nDefault); //白名经验掉落上限
	//  data.nRedNameExpDropRate = getFieldInt("nRedNameExpDropRate", &nDefault);	//红名经验掉落系数
	//  data.nRedNameExpDropMax = getFieldInt("nRedNameExpDropMax", &nDefault); //红名经验掉落上限

	data.n2StepSkillHitRate = getFieldInt("n2StepSkillHitRate", &nDefault);			//技能攻击的时候，距离2格命中的概率，1表示万分之1
	data.n3StepSkillHitRate = getFieldInt("n3StepSkillHitRate", &nDefault);				//技能攻击的时候，距离3格命中的概率，1表示万分之1	
	data.n4StepSkillHitRate = getFieldInt("n4StepSkillHitRate", &nDefault);				//技能攻击的时候，距离3格命中的概率，1表示万分之1

	memset(&data.equipDropRates, 0, sizeof(data.equipDropRates));				//装备各部位系数
	if (feildTableExists("equipDropPartRates") && openFieldTable("equipDropPartRates"))
	{
		int nDropIdx = 0;
		int nTempVal = 0;
		if (enumTableFirst())
		{
			do 
			{
				if (nDropIdx >= itMaxEquipPos)
				{
					endTableEnum();
					break;
				}
				data.equipDropRates[nDropIdx++] = getFieldInt(NULL, &nTempVal);
			} while (enumTableNext());
		}		 
		closeTable();
	}

	data.fHeroWriteNameEquipDropRate = (float)getFieldNumber("fHeroWriteNameEquipDropRate", &fDef);	//英雄白名装备掉落系数
	data.fHeroRedNameEquipDropRate = (float)getFieldNumber("fHeroRedNameEquipDropRate", &fDef);		//英雄红名装备掉落系数
	data.nHeroWriteNameEquipDropNum = getFieldInt("nHeroWriteNameEquipDropNum", &nDefault);			//英雄白名装备掉落数量
	data.nHeroRedNameEquipDropNum = getFieldInt("nHeroRedNameEquipDropNum", &nDefault);				//英雄红包装备掉落数量

	memset(&data.heroEquipDropRates, 0, sizeof(data.heroEquipDropRates));					//英雄装备各部位系数
	if (feildTableExists("heroEquipDropRates") && openFieldTable("heroEquipDropRates"))
	{
		int nDropIdx = 0;
		int nTempVal = 0;
		if (enumTableFirst())
		{
			do 
			{
				if (nDropIdx >= itMaxEquipPos)
				{
					endTableEnum();
					break;
				}
				data.heroEquipDropRates[nDropIdx++] = getFieldInt(NULL, &nTempVal);
			} while (enumTableNext());
		}		 
		closeTable();
	}

	data.fBodyDamageRate = (float)getFieldNumber("fBodyDamageRate", &fDef);
	nDefault =0;
	if (feildTableExists("DizzyBuffTateJob") && openFieldTable("DizzyBuffTateJob"))
	{
		if (enumTableFirst())
		{
			do 
			{
				int rate =getFieldInt(NULL);
				data.nDizzyBuffTateJob.push_back(rate);
			} while (enumTableNext());
		}		
		closeTable();	
	}

	data.nGuardEffId = getFieldInt("GuardEffId", &nDefault); //守护
	data.nParalyEffId = getFieldInt("ParalyEffId", &nDefault); //麻痹
	data.nCritiEffId = getFieldInt("CritiEffId", &nDefault);//暴击
	data.nDizzyBuffMaxRate = getFieldInt("DizzyBuffMaxRate", &nDefault); //麻痹最大概率
	data.nDizzyBuffMaxTime = getFieldInt("DizzyBuffMaxTime", &nDefault); //麻痹最大时间(秒)

	//踢人几次就下线
	CActor::s_nKickQuickUserCount   = getFieldInt("quickTimeKick", &nDefault);
	CActor::s_nQuickBuffId			 = getFieldInt("quickBuffId", &nDefault);
	fDef =0.1;
	CActor::s_fKickQuickRate   = (float)getFieldNumber("quickRate", &fDef);
	//  data.nAutoSilencing = getFieldInt("nAutoSilencing", &nDefault); 
	//  data.nAutoSilencingSecond = getFieldInt("nAutoSilencingSecond", &nDefault); 
	//  data.nAutoSilenceLevel = getFieldInt("nAutoSilenceLevel", &nDefault); 
	//  data.nSameChatSliencingCount = getFieldInt("nSameChatSliencingCount", &nDefault); 
	//  data.nSameChatSliencingSecond = getFieldInt("nSameChatSliencingSecond", &nDefault); 
	//  data.nAutoSilenceSameIpShutup = getFieldInt("nAutoSilenceSameIpShutup", &nDefault);

	nDefault = 0; 

	data.nMailExpireDays = getFieldInt("MailExpireDays", &nDefault);

	data.nBossDigNeedStoreCount = getFieldInt("BossDigNeedStoreCount", &nDefault);
	data.nHammerItemId = getFieldInt("nHammerItemId", &nDefault);
	int nNewDefault = 70;
	data.nCanActivityFiveAttrLevel = getFieldInt("CanActivityFiveAttrLevel", &nNewDefault);
	int nStrongDefault = 10;
	data.nCanActivityFiveAttrStrongLevel = getFieldInt("CanActivityFiveAttrStrongLevel",&nStrongDefault);
	int nMoveSpeedDefault = 100;
	data.nMoveSpeedLimit = getFieldInt("moveSpeedLimit", &nMoveSpeedDefault);

	fDef = 0;
	data.nMailMaxCount = getFieldInt("nMailMaxCount", &nDefault);

	if (feildTableExists("callteam") && openFieldTable("callteam"))
	{
		if (enumTableFirst())
		{
			do 
			{
				int type =getFieldInt("type",&nDefault);
				int id =getFieldInt("id",&nDefault);
				int count =getFieldInt("count",&nDefault);
				data.vecTeamCallConsum.push_back( std::make_tuple(type,id,count) );
			} while (enumTableNext());
		}		
		closeTable();	
	}
	
	if (feildTableExists("callguild") && openFieldTable("callguild"))
	{
		if (enumTableFirst())
		{
			do 
			{
				int type =getFieldInt("type",&nDefault);
				int id =getFieldInt("id",&nDefault);
				int count =getFieldInt("count",&nDefault);
				data.vecGuildCallConsum.push_back( std::make_tuple(type,id,count) );
			} while (enumTableNext());
		}		
		closeTable();	
	}
	
	 LoadActorMsgFilter();
	 closeTable();
	 return true;
 }

 bool CLogicDataProvider::LoadActorMsgFilter()
 {
	 bool bIsCommonServer = GetLogicServer()->IsCommonServer();
	 if (!bIsCommonServer)
		 return true;

	//  const char *pFieldName = "commonServerMsgFilter";
	//  if (feildTableExists(pFieldName) && openFieldTable(pFieldName))
	//  {
	// 	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	// 	 if (nCount > 0 && enumTableFirst())
	// 	 {
	// 		 CActorMsgFilter &msgFilter = GetGlobalLogicEngine()->GetActorMsgFilter();
	// 		 do 
	// 		 {
	// 			 int nSystemId = getFieldInt("sys");
	// 			 /*int nMsgId = getFieldInt("msg");
	// 			 if (nSystemId < 0 || nSystemId > CActorMsgFilter::MAX_SUBSYSTEM_ID)
	// 			 {
	// 			 OutputMsg(rmError, _T("加载角色消息过滤配置错误，请检测GlobalConfig.commonServerMsgFilter配置"));
	// 			 }
	// 			 else
	// 			 {
	// 			 msgFilter.SetFilter(nSystemId, nMsgId, true);
	// 			 }*/
	// 			 if (nSystemId < 0 || nSystemId > CActorMsgFilter::MAX_SUBSYSTEM_ID)
	// 			 {
	// 				OutputMsg(rmError, _T("加载角色消息过滤配置错误，请检测GlobalConfig.commonServerMsgFilter配置"));
	// 			 }
	// 			 else
	// 			 {
	// 				 const char *pMsgName = "msg";
	// 				 if (feildTableExists(pMsgName) && openFieldTable(pMsgName))
	// 				 {
	// 					 const INT_PTR nCount = lua_objlen(m_pLua, -1);
	// 					 if (nCount > 0 && enumTableFirst())
	// 					 {
	// 						 do
	// 						 {
	// 							 int nMsgId = getFieldInt(NULL);
	// 							 msgFilter.SetFilter(nSystemId, nMsgId, true);
	// 						 }while(enumTableNext());
	// 					 }
	// 					 else
	// 					 {
	// 						 msgFilter.SetFilter(nSystemId, CMessageFilter::WILDCARD_MESSAGE_ID, true);
	// 					 }
	// 					 closeTable();
	// 				 }
	// 			 }
	// 		 } while (enumTableNext());
	// 	 }

	// 	 closeTable();
	//  }

	 return true;
 }

 bool CLogicDataProvider::LoadTeamKillMonsterExp()
 {
	 //杀怪经验加成配置表
	 TEAMKILLMONSTEREXP &data = GetLogicServer()->GetDataProvider()->GetTeamKillMonsterExp();
	 if (!openGlobalTable("TeamKillMonsterExpEnhance"))
	 {
		 OutputMsg(rmWaning,_T("TeamKillMonsterExpEnhance 表找不到"));
		 return false;
	 }


	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	 if(nCount  != ArrayCount(data.teamExp))
	 {
		 OutputMsg(rmWaning,_T("TeamKillMonsterExpEnhance 表个数=%d错误"),nCount);
		 return false;
	 }
	 
	
	 if ( enumTableFirst() )
	 {
		 INT_PTR nIdx = 0;
		 do 
		 {
			 data.teamExp[nIdx] =(float)( getFieldNumber(NULL) /(float)10000 );
			 nIdx ++;
		 }
		 while (enumTableNext());
	 }
	 closeTable();
	 return true;
 }

 bool CLogicDataProvider::LoadTeamKillMonsterExpBuf()
 {
	 //经验加成buf配置表
	 TEAMKILLMONSTEREXP &data = GetLogicServer()->GetDataProvider()->GetTeamKillMonsterExp();
	 if (!openGlobalTable("TeamKillMonsterExpbufId"))
	 {
		 OutputMsg(rmWaning,_T("TeamKillMonsterExpbufId 表找不到"));
		 return false;
	 }
	 INT_PTR nCount = lua_objlen(m_pLua, -1);
	 if(nCount  != ArrayCount(data.teamExpBuf))
	 {
		 OutputMsg(rmWaning,_T("TeamKillMonsterExpbufId 表个数=%d错误"),nCount);
		 return false;
	 }
	 if ( enumTableFirst() )
	 {
		 INT_PTR nIdx = 0;
		 do 
		 {
			 if (nIdx >= sizeof(data.teamExpBuf)/sizeof(int)) break;
			 
			 data.teamExpBuf[nIdx] = (int)(getFieldInt(NULL));
			 nIdx ++;
		 }
		 while (enumTableNext());
	 }
	 closeTable();
	 return true;
 }

 //装载各等级的最大的声望
 bool CLogicDataProvider::LoadRenownLimit()
 {
	RENOWNLIMIT & config= GetRenownConfig();
	
	if (!openGlobalTable("RenownLimitLeves"))
	{
		OutputMsg(rmWaning,_T("RenownLimitLeves 表找不到"));
		return false;
	}
	INT_PTR nCount = lua_objlen(m_pLua, -1);
	config.nMaxConfigCount = (int)nCount;

	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		do 
		{
			config.remowns [nIdx] = getFieldInt(NULL);
			nIdx ++;
		}
		while (enumTableNext());
	}
	closeTable();

	return true;
 }

bool CLogicDataProvider::LoadLevelUpExpConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadLevelUpExpTable config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadLevelUpExpTable();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load level config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load level config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}


bool CLogicDataProvider::LoadExpLmtOpendayConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on ExpLmtOpenday config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadExpLmtOpendayTable();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load explimitopenday config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load explimitopenday config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}













bool CLogicDataProvider::LoadChatSystemConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadChatSystemConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadChatSystemConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Guild config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

 
 bool CLogicDataProvider::LoadChatSystemConfig()
 {
	 //副本配置表
	CHATSYSTEMCONFIG &data = GetLogicServer()->GetDataProvider()->GetChatSystemConfig();
	if (!openGlobalTable("ChatSystemConfig"))
	{
		OutputMsg(rmWaning,_T("ChatSystemConfig 表找不到"));
		return false;
	}
	int nDefChannelCD = 3000;
	data.nChatChannelCD[ciChannelSecret]			= getFieldInt("secretChannelChatCD",				&nDefChannelCD) * 1000;	 
	data.nChatChannelCD[ciChannelNear]				= getFieldInt("nearChannelChatCD",					&nDefChannelCD) * 1000;
	data.nChatChannelCD[ciChannelGuild]			= getFieldInt("guideChannelChatCD",					&nDefChannelCD) * 1000;
	data.nChatChannelCD[ciChannelTeam]				= getFieldInt("teamChannelChatCD",					&nDefChannelCD) * 1000;
	data.nChatChannelCD[ciChannelWorld]			= getFieldInt("worldChannelChatCD",					&nDefChannelCD) * 1000;
	data.nChatChannelCD[ciChannelTipmsg]			= 0;
	data.nChatChannelCD[ciChannelSystem]			= 0;
	data.nChatChannelCD[ciChannelIntegrated]			= 0;
	char sChatLimit[128];
	LPCSTR sSPID = GetLogicServer()->GetVSPDefine().GetDefinition(CVSPDefinition::SPID);	 
	if(sSPID !=NULL && strcmp(sSPID, "37w") == 0)	// 暂时无用，备用一个区别的聊天限制等级，将来可以不同渠道使用
	{
		sprintf_s(sChatLimit, ArrayCount(sChatLimit), "sp37wLevelLmt");
	}
	else
	{
		sprintf_s(sChatLimit, ArrayCount(sChatLimit), "chatLevelLmt");
	}
	if (feildTableExists(sChatLimit) && openFieldTable(sChatLimit))
	{
		memset(&data.ChatLimit, 0, sizeof(data.ChatLimit));
		if (enumTableFirst())
		{
			int nDef = 1;
			do 
			{
				BYTE nChannelId = getFieldInt("channel", &nDef);
				
				if ( ciChannelSecret <= nChannelId && nChannelId < ciChannelMax)
				{
					tagChatChannelLimit &temp = data.ChatLimit[nChannelId];
					temp.btChannelId 	 = nChannelId;
					temp.wLevel 	 	 = getFieldInt("level", &nDef);
					temp.nRechargeAmount = getFieldInt("recharge", &nDef);
				} else {
					OutputMsg(rmError, _T("ChatSystemConfig chatLevelLmt channel 有误！ channel ：%d"), nChannelId);
				}
				
			} while (enumTableNext());
		}
		closeTable();
	}

	if (feildTableExists("rechargeopen") && openFieldTable("rechargeopen"))
	{
		INT_PTR count = lua_objlen(m_pLua, -1);
		if (count > 0 && enumTableFirst())
		{
			int nDefLvlLmt = 1;
			INT_PTR idx = 0;
			do 
			{
				if (idx < ciChannelMax)
				{
					data.nRechargeopen[idx++] = getFieldInt(NULL, &nDefLvlLmt);
				}
				else
				{
					endTableEnum();
					break;
				}
			} while (enumTableNext());
		}
		closeTable();
	}
	data.nVipChats.clear();
	if (feildTableExists("vipchat") && openFieldTable("vipchat"))
	{
		
		INT_PTR count = lua_objlen(m_pLua, -1);
		if (count > 0 && enumTableFirst())
		{
			int nDefLvlLmt = 0;
			INT_PTR idx = 0;
			do 
			{
				int nNum = getFieldInt(NULL, &nDefLvlLmt);
				data.nVipChats.push_back(nNum);
			} while (enumTableNext());
		}
		closeTable();
	}
	int 	 nDefault =0;
	data.nChatLogFlag = getFieldInt("nChatSaveLogFlag", &nDefault);
	data.nShowItemSize = getFieldInt("showCount", &nDefault);
	data.nAutoSilencing = getFieldInt("nAutoSilencing", &nDefault); 
	data.nAutoSilencingSecond = getFieldInt("nAutoSilencingSecond", &nDefault); 
	data.nAutoSilenceLevel = getFieldInt("nAutoSilenceLevel", &nDefault); 
	data.nSameChatSliencingCount = getFieldInt("nSameChatSliencingCount", &nDefault); 
	data.nSameChatSliencingSecond = getFieldInt("nSameChatSliencingSecond", &nDefault); 
	data.nAutoSilenceSameIpShutup = getFieldInt("nAutoSilenceSameIpShutup", &nDefault);
	int nDefaultCounter = 45;
	data.nCounter = getFieldInt("counter", &nDefaultCounter);
	data.nDisplayLeftTimesLimit = getFieldInt("nDisplay", &nDefault);
	closeTable();
	return true;
 }


 
bool CLogicDataProvider::LoadDropItemConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadDropItemConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadDropItemConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load dropitem config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load dropitem config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
 
 bool CLogicDataProvider::LoadDropItemConfig()
 {
	 //副本配置表
	DROPITEMCONFIG &data = GetLogicServer()->GetDataProvider()->GetDropItemConfig();
	if (!openGlobalTable("DropItemConfig"))
	{
		OutputMsg(rmWaning,_T("DropItemConfig 表找不到"));
		return false;
	}
	int nDefault = 0;
	data.nDropItemExpireTime = getFieldInt("nDropItemExpireTime", &nDefault); //掉落在地上的物品的存活时间,单位秒
	data.nDropItemNeedTime = getFieldInt("nDropItemNeedTime", &nDefault); 
	data.nDropPetLootDistance = getFieldInt("nDropPetLootDistance", &nDefault); 
	data.nDropItemMasterProtectTime = getFieldInt("nDropItemMasterProtectTime", &nDefault); //掉落在地上的物品属于他的主人的时间,单位秒

	 memset(&data.dropCoinCounts,0,sizeof(data.dropCoinCounts));
	 if (feildTableExists("dropCoinCounts") && openFieldTable("dropCoinCounts"))
	 {
		 INT_PTR count = lua_objlen(m_pLua, -1);
		 if (count > 0 && enumTableFirst())
		 {
			 int nDefLvlLmt = 1;
			 INT_PTR idx = 0;
			 do 
			 {
				 if (idx < MAX_DROP_COIN_ICON_COUNT)
				 {
					 data.dropCoinCounts[idx++] = getFieldInt(NULL, &nDefLvlLmt);
				 }
				 else
				 {
					 endTableEnum();
					 break;
				 }
			 } while (enumTableNext());
		 }
		 closeTable();
	 }
	 memset(&data.dropCoinIcons,0,sizeof(data.dropCoinIcons));
	 if (feildTableExists("dropCoinIcons") && openFieldTable("dropCoinIcons"))
	 {
		 INT_PTR count = lua_objlen(m_pLua, -1);
		 if (count > 0 && enumTableFirst())
		 {
			 int nDefLvlLmt = 1;
			 INT_PTR idx = 0;
			 do 
			 {
				 if (idx < MAX_DROP_COIN_ICON_COUNT)
				 {
					 data.dropCoinIcons[idx++] = getFieldInt(NULL, &nDefLvlLmt);
				 }
				 else
				 {
					 endTableEnum();
					 break;
				 }
			 } while (enumTableNext());
		 }
		 closeTable();
	 }

	//  memset(&data.dropYuanBaoCounts,0,sizeof(data.dropYuanBaoCounts));
	//  if (feildTableExists("dropYuanBaoCounts") && openFieldTable("dropYuanBaoCounts"))
	//  {
	// 	 INT_PTR count = lua_objlen(m_pLua, -1);
	// 	 if (count > 0 && enumTableFirst())
	// 	 {
	// 		 int nDefLvlLmt = 1;
	// 		 INT_PTR idx = 0;
	// 		 do 
	// 		 {
	// 			 if (idx < MAX_DROP_COIN_ICON_COUNT)
	// 			 {
	// 				 data.dropYuanBaoCounts[idx++] = getFieldInt(NULL, &nDefLvlLmt);
	// 			 }
	// 			 else
	// 			 {
	// 				 endTableEnum();
	// 				 break;
	// 			 }
	// 		 } while (enumTableNext());
	// 	 }
	// 	 closeTable();
	// //  }
	//  memset(&data.dropYuanBaoIcons,0,sizeof(data.dropYuanBaoIcons));
	//  if (feildTableExists("dropYuanBaoIcons") && openFieldTable("dropYuanBaoIcons"))
	//  {
	// 	 INT_PTR count = lua_objlen(m_pLua, -1);
	// 	 if (count > 0 && enumTableFirst())
	// 	 {
	// 		 int nDefLvlLmt = 1;
	// 		 INT_PTR idx = 0;
	// 		 do 
	// 		 {
	// 			 if (idx < MAX_DROP_COIN_ICON_COUNT)
	// 			 {
	// 				 data.dropYuanBaoIcons[idx++] = getFieldInt(NULL, &nDefLvlLmt);
	// 			 }
	// 			 else
	// 			 {
	// 				 endTableEnum();
	// 				 break;
	// 			 }
	// 		 } while (enumTableNext());
	// 	 }
	// 	 closeTable();
	//  }

	 memset(&data.dropBindCoinCounts,0,sizeof(data.dropBindCoinCounts));
	 if (feildTableExists("dropBindCoinCounts") && openFieldTable("dropBindCoinCounts"))
	 {
		 INT_PTR count = lua_objlen(m_pLua, -1);
		 if (count > 0 && enumTableFirst())
		 {
			 int nDefLvlLmt = 1;
			 INT_PTR idx = 0;
			 do 
			 {
				 if (idx < MAX_DROP_COIN_ICON_COUNT)
				 {
					 data.dropBindCoinCounts[idx++] = getFieldInt(NULL, &nDefLvlLmt);
				 }
				 else
				 {
					 endTableEnum();
					 break;
				 }
			 } while (enumTableNext());
		 }
		 closeTable();
	 }
	 memset(&data.dropBindCoinIcons,0,sizeof(data.dropBindCoinIcons));
	 if (feildTableExists("dropBindCoinIcons") && openFieldTable("dropBindCoinIcons"))
	 {
		 INT_PTR count = lua_objlen(m_pLua, -1);
		 if (count > 0 && enumTableFirst())
		 {
			 int nDefLvlLmt = 1;
			 INT_PTR idx = 0;
			 do 
			 {
				 if (idx < MAX_DROP_COIN_ICON_COUNT)
				 {
					 data.dropBindCoinIcons[idx++] = getFieldInt(NULL, &nDefLvlLmt);
				 }
				 else
				 {
					 endTableEnum();
					 break;
				 }
			 } while (enumTableNext());
		 }
		 closeTable();
	 }
	data.nMonsterMaxCoinOneDrop = getFieldInt("monsterMaxCoinOneDrop", &nDefault); //怪物掉落的金币，每堆的最大值
	data.nMonsterMaxBindCoinOneDrop = getFieldInt("monsterMaxBindCoinOneDrop", &nDefault); //怪物掉落的绑定金币，每堆的最大值
	closeTable();
	return true;
 }



bool CLogicDataProvider::LoadDeathDropConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadDeathDropConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadDeathDropConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load dropitem config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load dropitem config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

 
 bool CLogicDataProvider::LoadDeathDropConfig()
 {
	 //副本配置表
	DEATHDROPCONFIG &data = GetLogicServer()->GetDataProvider()->GetDeathDropRateConfig();
	if (!openGlobalTable("DeathDropConfig"))
	{
		OutputMsg(rmWaning,_T("DeathDropConfig 表找不到"));
		return false;
	}
	int nDefault = 0;
	data.nWriteNameBagDropRate = getFieldInt("fWriteNameBagDropRate", &nDefault); //
	data.nWriteNameEquipDropRate = getFieldInt("fWriteNameEquipDropRate", &nDefault); 
	data.nRedNameBagDropRate = getFieldInt("fRedNameBagDropRate", &nDefault); //
	data.nRedNameEqiupRate = getFieldInt("fRedNameEquipDropRate", &nDefault); //
	data.nMaxEquipDropNum = getFieldInt("MaxEquipDropNum", &nDefault); //
	closeTable();
	return true;
 }


bool CLogicDataProvider::LoadBagRemainConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadBagRemainConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadBagRemainConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoadBagRemainConfig config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadBagRemainConfig config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
 


 bool CLogicDataProvider::LoadBagRemainConfig()
 {
	BAGREMAINCONFIG &data = GetLogicServer()->GetDataProvider()->GetBagRemainConfig();
	if (!openGlobalTable("BagRemainConfig"))
	{
		OutputMsg(rmWaning,_T("BagRemainConfig 表找不到"));
		return false;
	}

	if (enumTableFirst())
	{
		do 
		{
			int nId = getFieldInt("idx");
			if(feildTableExists("bagremain") && openFieldTable("bagremain"))
			{
				if(enumTableFirst())
				{
					do
					{
						int id = getFieldInt(NULL);
						data.m_nBagRemain[nId].push_back(id);
					} while (enumTableNext());
					
				}
				closeTable();
			}
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

bool CLogicDataProvider::LoadHookExpConfig(LPCTSTR sFilePath)
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
		{
			showError(_T("syntax error on LoadHookExpConfig config"));
			FILE* fp = fopen("scripterror.txt", "wb");
			if (fp)
			{
				fputs(sText, fp);
				fclose(fp);
			}
			return false;
		}
		Result = LoadHookExpConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoadHookExpConfig config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load HookExpConfig config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}
 
bool CLogicDataProvider::LoadHookExpConfig()
{
	HookExpConfig &data = GetLogicServer()->GetDataProvider()->GethookExpCfg();
	data.m_HookMaps.clear();
	if (!openGlobalTable("hookexpConfig"))
	{
		OutputMsg(rmWaning,_T("hookexpConfig 表找不到"));
		return false;
	}
	int nDef = 0;
	if (enumTableFirst())
	{
		do 
		{
			tagHookExp hook;
			int nId = getFieldInt("idx");
			hook.nBasePdValue= getFieldInt("BasePdValue", &nDef);
			hook.nPdPercent= getFieldInt("PdPercent", &nDef);
			data.m_HookMaps.insert(std::make_pair(nId, hook));
		} while (enumTableNext());
	}
	closeTable();
	return true;
}



bool CLogicDataProvider::LoadMergeTimesConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	LPCTSTR sText;

	try
	{
		if(!FDOP::FileExists( sFilePath))
		{

		}
		else {
			//从文件加载配置脚本
			if ( ms.loadFromFile(sFilePath) > 0 )
			{
				//对配置脚本进行预处理
				GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
				sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
				//设置脚本内容
				if ( !setScript(sText) )
				{
					showError(_T("syntax error on LoadMergeTimesConfig config"));
					FILE* fp = fopen("scripterror.txt", "wb");
					if (fp)
					{
						fputs(sText, fp);
						fclose(fp);
					}
					return false;
				}
				LoadMergeTimesConfig();
			}
		}
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LoadMergeTimesConfig config error: %s"), s.rawStr());
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Guild config"));
	}
		
		
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}

bool CLogicDataProvider::LoadMergeTimesConfig()
{
	//副本配置表
	GLOBALCONFIG &data = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	data.nMergeTimes = 0;
	if (openGlobalTable("MergeTimes"))
	{
		int nDef_0 = 0;
		data.nMergeTimes = getFieldInt("mergetimes", &nDef_0);// 瞬移技能进入场景的特效id
		closeTable();
	}
	return true;
}