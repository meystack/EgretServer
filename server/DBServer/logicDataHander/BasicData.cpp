#include "StdAfx.h"
#include<iostream>
#include <algorithm>
/************************************************************************/
/*  本文件实现了玩家基本数据的查询和保存                         */
/*	函数申明在LogicDBRequestHandler.h文件。								*/
/************************************************************************/

unsigned int CLogicDBReqestHandler::QueryActorFromDB(CDataPacketReader &inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId, nLoginServerId;
	ACTORDBDATA ActorData;
	inPacket >> nRawServerId >> nLoginServerId >> ActorData.nID;
	CDataPacket& retPack = allocProtoPacket(dcQuery);	
	retPack << nRawServerId << nLoginServerId;
	retPack << ActorData.nID;
	unsigned int nActorID = ActorData.nID;
	OutputMsg(rmNormal, _T("query actor[%u] data"), nActorID);
	//数据库连接是否就绪
	if (!m_pSQLConnection->Connected())
	{
		retPack << (char)reDbErr ;
	}
	else
	{
		//向数据库查询
		int nError = m_pSQLConnection->Query(szSQLSP_LoadCharBasicData, nActorID);
		if ( !nError )
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if ( pRow)
			{
				int nSlotCount =0;
				WORD wValue =0;
				retPack << (char)reSucc  ;
				sscanf(pRow[0], "%d", &ActorData.nPosX);
				sscanf(pRow[1], "%d", &ActorData.nPosY);
				sscanf(pRow[2], "%d", &ActorData.nSex);
				sscanf(pRow[3], "%d", &ActorData.nVocation);
				sscanf(pRow[4], "%d", &ActorData.nLevel);
				sscanf(pRow[5], "%d", &ActorData.nIcon);
				sscanf(pRow[6], "%lld", &ActorData.lExp);
				sscanf(pRow[7], "%s", &ActorData.name);
				sscanf(pRow[8], "%d", &ActorData.nFbHandle);
				sscanf(pRow[9], "%d", &ActorData.nScenceID);
				sscanf(pRow[10], "%d", &ActorData.nPKValue);
				sscanf(pRow[11], "%d", &ActorData.nBagGridCount);
				sscanf(pRow[12], "%d", &ActorData.nMedicineCardTime);
				sscanf(pRow[13], "%u", &ActorData.nBindYuanbao);
				sscanf(pRow[14], "%u", &ActorData.nNonBindYuanbao);
				sscanf(pRow[15], "%u", &ActorData.nNonBindCoin);
				

				if(pRow[16])
				{
					memcpy(&ActorData.quest,pRow[16],sizeof(ActorData.quest));
				}

				sscanf(pRow[17], "%d", &ActorData.nHp);
				sscanf(pRow[18], "%d", &ActorData.nMp);
				sscanf(pRow[19], "%d", &ActorData.nEvilPkStatus);
				
				sscanf(pRow[20], "%u", &ActorData.nGuildID);
				sscanf(pRow[21], "%d", &ActorData.nTeamID);
				sscanf(pRow[22], "%d", &ActorData.nSocialMask);
				sscanf(pRow[23], "%d", &ActorData.nGuildExp);
				if (pRow[24])
				{
					char createTime[64];
					memcpy(&createTime,pRow[24],sizeof(createTime));
					SYSTEMTIME sysTime;	
					if (strlen(createTime) > 10) 
					{
						sscanf(createTime, "%d-%d-%d %d:%d:%d", &sysTime.wYear, &sysTime.wMonth, &sysTime.wDay, &sysTime.wHour, &sysTime.wMinute, &sysTime.wSecond);
						CMiniDateTime miniTime;
						ActorData.nCreateTime = miniTime.encode(sysTime);
					}
				}
				sscanf(pRow[25], "%lld", &ActorData.lLastLoginIp);
				sscanf(pRow[26], "%d", &ActorData.nLastLogoutTime); //上次退出时间
				sscanf(pRow[27], "%d", &ActorData.nModelId);// 模型id
				sscanf(pRow[28], "%d", &ActorData.nDeport3ExpiredTime);
				sscanf(pRow[29], "%d", &ActorData.nDeport4ExpiredTime);

				if(pRow[30])
				{
					sscanf(pRow[30], "%s", &ActorData.accountName);
				}				
				sscanf(pRow[31], "%d", &ActorData.nEnterFbScenceID);				
				int nFbPos =0;
				sscanf(pRow[32], "%d", &ActorData.nEnterFbPos);
				sscanf(pRow[33], "%d", &ActorData.nActorDir);
				//sscanf(pRow[34], "%lld", &ActorData.lMultiExpUnused);
				if(pRow[34])
				{
					sscanf(pRow[34], "%lld", &ActorData.lMultiExpUnused);
				}
				//账户的ID
				if(pRow[35])
				{
					sscanf(pRow[35], "%u", &ActorData.nAccountId);
				}
			
				if(pRow[36])
				{
					memcpy(&ActorData.achieveEventData,pRow[36],sizeof(ActorData.achieveEventData));
				}

				if(pRow[37])
				{
					memcpy(&ActorData.achieveFinishState ,pRow[37],sizeof(ActorData.achieveFinishState));
				}

				sscanf(pRow[38], "%lld", &ActorData.csrevivepoint);
				sscanf(pRow[39], "%lld", &ActorData.fbrevivepoint);
				sscanf(pRow[40], "%lld", &ActorData.cityrevivepoint);
				sscanf(pRow[41], "%d", &ActorData.achievepoint);
				
				if(pRow[42])
				{
					memcpy(&ActorData.titles ,pRow[42],sizeof(ActorData.titles));
				}
				
				sscanf(pRow[43], "%d", &ActorData.nNextChangeVocTime);
				
				sscanf(pRow[44], "%d", &ActorData.nForeverCardFlag);
				sscanf(pRow[45], "%d", &ActorData.nMonthCardTime);		//
				
				if(pRow[46])
				{
					sscanf(pRow[46], "%d", &ActorData.nExpToday);
				}
				if(pRow[48])
				{
					sscanf(pRow[48], "%d", &ActorData.nAlmirahLv);
				}
				if(pRow[49])
				{
					sscanf(pRow[49], "%d", &ActorData.nFashionBattle);
				}
				if(pRow[50])
				{
					sscanf(pRow[50], "%d", &ActorData.nCardAwardFlag);
				}
				if(pRow[51])
				{
					sscanf(pRow[51], "%d", &ActorData.nConsumeYb);
				}

				//阵营贡献
				if(pRow[52])
				{
					sscanf(pRow[52], "%d", &ActorData.nZyContrToday);
				}

				//提取元宝的数目
				if(pRow[53])
				{
					sscanf(pRow[53], "%d", &ActorData.nDrawYbCount);
				}

				//活跃度
				if(pRow[54])
				{
					sscanf(pRow[54], "%d", &ActorData.nActivity);
				}

				// 上次登录的服务器ID
				if (pRow[55])
				{
					sscanf(pRow[55], "%d", &ActorData.nLastLoginServerId);
				}
				if(pRow[56])
				{
					sscanf(pRow[56], "%d", &ActorData.nMeritoriousDay);
				}
				if (pRow[57])
				{
					sscanf(pRow[57], "%d", &ActorData.nRecoverState); //回收状态
				}
				if (pRow[58])
				{
					sscanf(pRow[58], "%d", &ActorData.nLootPetId);
				}
				if (pRow[59])
				{
					sscanf(pRow[59], "%d", &ActorData.nPersonBossJifen);
				}
				if (pRow[60])
				{
					sscanf(pRow[60], "%d", &ActorData.nCircle);
				}
				if (pRow[61])
				{
					sscanf(pRow[61], "%d", &ActorData.nCircleSoul);
				}

				if (pRow[63])
				{
					sscanf(pRow[63], "%u", &ActorData.nBindCoin);
				}

				if (pRow[64])
				{
					sscanf(pRow[64], "%d", &ActorData.nAnger);
				}

				if (pRow[66])
				{
					sscanf(pRow[66], "%d", &ActorData.nCheckIns);
				}
				if (pRow[72])
				{
					sscanf(pRow[72], "%d", &ActorData.nWingPoint);
				}

				if (pRow[73])
				{
					sscanf(pRow[73], "%d", &ActorData.nCurnewTitel);
				}

				if (pRow[75])
				{
					sscanf(pRow[75], "%lld", &ActorData.nDimensionalKey);
				}

				if (pRow[76])
				{
					sscanf(pRow[76], "%lld", &ActorData.nDefaultLootPetDis);
				}

				if (pRow[77])
				{
					sscanf(pRow[77], "%lld", &ActorData.nRebateDrawYbCount);
				}

				if (pRow[78])
				{
					sscanf(pRow[78], "%d", &ActorData.nSwingLevel);
				}

				if (pRow[80])
				{
					sscanf(pRow[80], "%d", &ActorData.nSwingId);
				}

				if (pRow[81])
				{
					sscanf(pRow[81], "%d", &ActorData.nLoginDays);
				}

				if (pRow[83])
				{
					sscanf(pRow[83], "%d", &ActorData.nCSActorId);
				}

				if (pRow[84])
				{
					sscanf(pRow[84], "%d", &ActorData.nSalary);
				}
				if (pRow[85])
				{
					sscanf(pRow[85], "%d", &ActorData.nOffice);
				}
				if (pRow[86])
				{
					sscanf(pRow[86], "%d", &ActorData.nDepotCoin);
				}
				if (pRow[90])
				{
					sscanf(pRow[90], "%d", &ActorData.nSupperPlayLvl);
				}
				if (pRow[91])
				{
					sscanf(pRow[91], "%d", &ActorData.nFrenzy);
				}
				if (pRow[93])
				{
					sscanf(pRow[93], "%d", &ActorData.nBeKilledCount);
				}
				if (pRow[94])
				{
					sscanf(pRow[94], "%d", &ActorData.nKillMonsterCount);
				}
				if(pRow[96])
				{
					sscanf(pRow[96], "%d", &ActorData.nTotalOnlineMin);	  //灵玉经验
				}
				if(pRow[98])
				{
					sscanf(pRow[98], "%d", &ActorData.nLastLoginOpendayNo_);	  //上次登录开服天数
				}
				if(pRow[99])
				{
					sscanf(pRow[99], "%d", &ActorData.nCurCustomTitle);			//当前自定义称号
				}
				
				if(pRow[102])
				{
					sscanf(pRow[102], "%d", &ActorData.nBless);			//祝福值
				}
				if(pRow[103])
				{
					sscanf(pRow[103], "%d", &ActorData.nPrestige);	//今日战绩
				}
				if(pRow[104])
				{
					sscanf(pRow[104], "%d", &ActorData.nDeportGridCount);
				}
				int nCount = 0;
			    if (pRow[105])
				{
					sscanf(pRow[105], "%d", &nCount);
				}
				//ActorData.nBroatNum = HIWORD(nCount);
				//ActorData.nFlyShoes = LOWORD(nCount);
				ActorData.nBroatNum = LOWORD(nCount);
				ActorData.nFlyShoes = HIWORD(nCount);

				if (pRow[106])
				{
					sscanf(pRow[106], "%d", &ActorData.nRecyclepoints);
				}


				nCount = 0;
			    if (pRow[107])
				{
					sscanf(pRow[107], "%d", &nCount);
				}
				ActorData.nMeridianLv = LOWORD(nCount);
			  	if(pRow[108])
				{
					memcpy(&ActorData.quest ,pRow[108],sizeof(ActorData.quest));
				}
				if (pRow[109])
				{
					sscanf(pRow[109], "%d", &ActorData.nShutUpTime);
				}
				if (pRow[110])
				{
					sscanf(pRow[110], "%d", &ActorData.nOldSrvId);
				}
				retPack << ActorData;
				
				unsigned long long nTradingQuota = 0;
				if (pRow[111])
				{
					sscanf(pRow[111], "%lld", &nTradingQuota);//交易额度
				}
				retPack << nTradingQuota;
			}
			else
			{
				retPack << (BYTE)reNoActor ;
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmError,_T("%s装载玩家的数据失败%u"),__FUNCTION__,nActorID);
			retPack << (char)reDbErr ;
		}

	}
	flushProtoPacket(retPack);
	UpdateCharOnlineStatus(nActorID, 1);
	return nActorID;
}

void CLogicDBReqestHandler::QueryActoSubSystemData(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID;

	OutputMsg(rmNormal,_T("query actor[%u] subsystem"),nActorID);
	QuerySkillFromDB(nActorID, nRawServerId, nLoginServerId);
	QueryOtherGameSetsFromDB(nActorID, nRawServerId, nLoginServerId);
	LoadActorStaticCounts(nActorID, nRawServerId, nLoginServerId);
	LoadActorAchieveMentInfo(nActorID, nRawServerId, nLoginServerId);
	LoadActorStrengthenInfo(nActorID, nRawServerId, nLoginServerId);

	QueryPetData(nActorID, nRawServerId, nLoginServerId);
	QueryItems(nActorID, itItemBag, nRawServerId, nLoginServerId);
	QueryItems(nActorID, itItemEquip, nRawServerId, nLoginServerId);
	QueryItems(nActorID, itItemDepot, nRawServerId, nLoginServerId);

	//QueryMyBrother(nActorID, nRawServerId, nLoginServerId);
	QueryActorScriptData(nActorID, nRawServerId, nLoginServerId);

	//查询宠物物品，这里要严格按照这个顺序，后面的逻辑需要这里的数据
	QueryItems(nActorID, itItemHero, nRawServerId, nLoginServerId); 
	QueryPetSkillData(nActorID, nRawServerId, nLoginServerId);
	QueryRelationData(nActorID, nRawServerId, nLoginServerId);
	LoadAlmirahItem(nActorID, nRawServerId,nLoginServerId);	// 衣橱系统
	//QueryActorGemItemData(nActorID, nRawServerId, nLoginServerId);
	//QueryActorGemSuitData(nActorID, nRawServerId, nLoginServerId);	
	QueryActorMail(nActorID, nRawServerId, nLoginServerId);	
	QueryQuestFromDB(nActorID, nRawServerId, nLoginServerId);
	LoadActorFriendsData(nActorID, nRawServerId, nLoginServerId);
	LoadActorDeathData(nActorID, nRawServerId, nLoginServerId);
	QueryActorNewTitle(nActorID, nRawServerId, nLoginServerId);
	QueryActorCustomTitle(nActorID, nRawServerId, nLoginServerId);
	QueryLootPetData(nActorID, nRawServerId, nLoginServerId);//宠物
	
}
void CLogicDBReqestHandler::LoadCircleCount()
{
	if (!m_pSQLConnection->Connected())
		return;

	int nError = m_pSQLConnection->Query(szSQLSP_LoadCircleCount);
	CDataPacket &retPack = allocProtoPacket(dcLoadCircleCount);
	// retPack << nRawServerId << nLoginServerId;
	
	if (!nError)
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int ncount = m_pSQLConnection->GetRowCount();
		retPack <<(BYTE)nError;
		retPack <<(BYTE)ncount;
		int nValue = 0;
		while (pRow)
		{
			sscanf(pRow[0], "%d", &nValue);
			retPack <<(int)nValue;
			sscanf(pRow[1], "%d", &nValue);
			retPack << (int)nValue;
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	}
	else
	{
		retPack << (BYTE)nError;
	}

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::QueryActorCustomInfoUnForbid(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF() 
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	BYTE opreatCode = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID; 
	inPacket >> opreatCode; 
	unsigned int nDuration = 0;
	if((BYTE)opreatCode == (BYTE)MASK_BACKSTAGE_FORVID)
	{
		inPacket >> nDuration;  
	}
 
	INT_PTR nErroID = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCustomInfoUnForbid);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorID;
	retPack << opreatCode;
	INT_PTR nErrIdPosBegin = retPack.getPosition();
	retPack << (BYTE) nErroID;
 
	CActorCustomProperty data;
	int count1 = CountArray(data.cbCustomProperty); 
	std::string strCustomRule; 
	// 加载自定义数据
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorCustomInfo, nActorID);
	if ( !nErroID )
	{
		CUserItem data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)1;
		while (pRow)
		{    
			if( pRow[1] )				//判断下是否为空
			{ 
				strCustomRule = pRow[1];  
			} 

			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} 
	else
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	} 
	if (!strCustomRule.empty())//有读取到
	{  
		int nCustomRuleSize = strCustomRule.length() / 2;
		int ssize = std::min(nCustomRuleSize,   int(count1/ 2)); 
		//转换字符
		for (int i = 0; i < ssize; i++)
		{ 
			//获取字符
			TCHAR cbChar1 = strCustomRule[i*2];
			TCHAR cbChar2 = strCustomRule[i*2+1];

			//效验字符
			assert((cbChar1 >= _T('0'))&&(cbChar1 <= _T('9')) || (cbChar1 >= _T('A')) && (cbChar1 <= _T('F')));
			assert((cbChar2 >= _T('0'))&&(cbChar2 <= _T('9')) || (cbChar2 >= _T('A')) && (cbChar2 <= _T('F')));

			//生成结果
			if ((cbChar2>=_T('0'))&&(cbChar2<=_T('9'))) data.cbCustomProperty[i]+=(cbChar2-_T('0'));
			if ((cbChar2>=_T('A'))&&(cbChar2<=_T('F'))) data.cbCustomProperty[i]+=(cbChar2-_T('A')+0x0A);

			//生成结果
			if ((cbChar1>=_T('0'))&&(cbChar1<=_T('9'))) data.cbCustomProperty[i]+=(cbChar1-_T('0'))*0x10;
			if ((cbChar1>=_T('A'))&&(cbChar1<=_T('F'))) data.cbCustomProperty[i]+=(cbChar1-_T('A')+0x0A)*0x10;
		}
	}
	else
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reNoActor;//没有属性
		flushProtoPacket(retPack);
		return;
	}   

	BYTE * pDataint = (BYTE *)&data.cbCustomProperty[ACTORRULE_KICK_USER];  
  
	BYTE pDataintTmp = *pDataint;
	// 设置成功状态
	INT_PTR nErrIdPosEnd = retPack.getPosition();
	retPack.setPosition(nErrIdPosBegin);
	retPack << (BYTE)reSucc;
	retPack.writeBuf(&data, sizeof(CActorCustomProperty)); 
	if((BYTE)opreatCode == (BYTE)MASK_BACKSTAGE_FORVID)
	{
		retPack << nDuration; 
	} 

	//retPack << data;  
	retPack.setPosition(nErrIdPosEnd);

	flushProtoPacket(retPack);
} 

void CLogicDBReqestHandler::QueryActorCustomInfoCustomTitleId(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF() 
	
	int nRawServerId = 0, nLoginServerId = 0, nCutomTitleId = 0;
	unsigned int nActorID = 0;

	inPacket >> nRawServerId >> nLoginServerId >> nActorID; 
	inPacket >> nCutomTitleId; 
 
	INT_PTR nErroID = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCustomInfoCustomTitleId);
	retPack << nRawServerId << nLoginServerId << nActorID;
	INT_PTR nErrIdPosBegin = retPack.getPosition();
 
	CActorCustomProperty data;
	int count1 = CountArray(data.cbCustomProperty); 
	std::string strCustomRule; 
	// 加载自定义数据
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorCustomInfo, nActorID);
	if ( !nErroID )
	{
		CUserItem data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)1;
		while (pRow)
		{    
			if( pRow[1] )				//判断下是否为空
			{ 
				strCustomRule = pRow[1];  
			} 

			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} 
	else
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	} 
	if (!strCustomRule.empty())//有读取到
	{  
		int nCustomRuleSize = strCustomRule.length() / 2;
		int ssize = std::min(nCustomRuleSize,   int(count1/ 2)); 
		//转换字符
		for (int i = 0; i < ssize; i++)
		{ 
			//获取字符
			TCHAR cbChar1 = strCustomRule[i*2];
			TCHAR cbChar2 = strCustomRule[i*2+1];

			//效验字符
			assert((cbChar1 >= _T('0'))&&(cbChar1 <= _T('9')) || (cbChar1 >= _T('A')) && (cbChar1 <= _T('F')));
			assert((cbChar2 >= _T('0'))&&(cbChar2 <= _T('9')) || (cbChar2 >= _T('A')) && (cbChar2 <= _T('F')));

			//生成结果
			if ((cbChar2>=_T('0'))&&(cbChar2<=_T('9'))) data.cbCustomProperty[i]+=(cbChar2-_T('0'));
			if ((cbChar2>=_T('A'))&&(cbChar2<=_T('F'))) data.cbCustomProperty[i]+=(cbChar2-_T('A')+0x0A);

			//生成结果
			if ((cbChar1>=_T('0'))&&(cbChar1<=_T('9'))) data.cbCustomProperty[i]+=(cbChar1-_T('0'))*0x10;
			if ((cbChar1>=_T('A'))&&(cbChar1<=_T('F'))) data.cbCustomProperty[i]+=(cbChar1-_T('A')+0x0A)*0x10;
		}
	}
	else
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reNoActor;//没有属性
		flushProtoPacket(retPack);
		return;
	}   

	BYTE * pDataint = (BYTE *)&data.cbCustomProperty[ACTORRULE_KICK_USER];  
  
	BYTE pDataintTmp = *pDataint;
	// 设置成功状态
	INT_PTR nErrIdPosEnd = retPack.getPosition();
	retPack.setPosition(nErrIdPosBegin);
	retPack << (BYTE)0;// opreatCode(占位，暂未使用)
	retPack << (BYTE)reSucc;
	retPack.writeBuf(&data, sizeof(CActorCustomProperty)); 
	retPack << nCutomTitleId;  

	//retPack << data;  
	retPack.setPosition(nErrIdPosEnd);

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::SaveActorCustomInfoCustomTitleId(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId >> nActorID;

	int AvaliableLength = inPacket.getAvaliableLength();
	int ActorProLength = sizeof(CActorCustomProperty);
	if (AvaliableLength != ActorProLength)
	{
		OutputMsg(rmError,_T("%s Data size error UnForbid CActorCustomProperty %d : %d "), __FUNCTION__, AvaliableLength, ActorProLength);
		return;
	}

	CActorCustomProperty data;
	inPacket >> data;

	if (m_pSQLConnection->Connected() )
	{
		char *ptr = m_pHugeSQLBuffer;
		ptr += sprintf(ptr, szSQLSP_UpdateActorCustomInfo, nActorID);
 
		int count = CountArray(data.cbCustomProperty);
		ptr[0] = '"';
		ptr += 1;
		for (int i = 0; i < count; i++) 
		{ 
			ptr += sprintf(&(*ptr), ("%02X"), data.cbCustomProperty[i]);
			//ptr += 2; 
		}
		//ptr += mysql_escape_string(ptr, (const char*)&data, sizeof(data));
		ptr[0] = '"';
		ptr[1] = ')';
		ptr[2] = ';';
		ptr[3] = 0;
		ptr += 4;
		auto size = ptr - m_pHugeSQLBuffer;
		int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::QueryActorCustomInfo(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF() 
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID; 

	INT_PTR nErroID = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadCustomInfo);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorID;
	INT_PTR nErrIdPosBegin = retPack.getPosition();
	retPack << (BYTE) nErroID;
 
	CActorCustomProperty data;
	int count1 = CountArray(data.cbCustomProperty);
	// char szCustomRule[count1 + 1]; 
	// memset( szCustomRule, 0, sizeof(szCustomRule) );
	std::string strCustomRule;
	// for (int i = 0; i < CountArray(data.cbCustomProperty); i++) 
	// { 
	// 	_stprintf(&szCustomRule[i*2], _T("%02X"), data.cbCustomProperty[i]);
	// }
	// 加载自定义数据
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorCustomInfo, nActorID);
	if ( !nErroID )
	{
		CUserItem data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)1;
		while (pRow)
		{    
			if( pRow[1] )				//判断下是否为空
			{
				//_asncpytA(szCustomRule, pRow[1]);
				strCustomRule = pRow[1]; 
				//OutputMsg(rmError, _T("%s &"), strCustomRule.c_str());
			}
			else
			{
				//_asncpytA(szCustomRule, "");
			}

			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} 
	else
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	} 
	bool rightful = true;
	if (!strCustomRule.empty())//有读取到
	{  
		int nCustomRuleSize = strCustomRule.length() / 2;
		int ssize = std::min(nCustomRuleSize,   int(count1/ 2)); 
		//转换字符
		for (int i = 0; i < ssize; i++)
		{ 
			//获取字符
			TCHAR cbChar1 = strCustomRule[i*2];
			TCHAR cbChar2 = strCustomRule[i*2+1];

			//效验字符
			assert((cbChar1 >= _T('0'))&&(cbChar1 <= _T('9')) || (cbChar1 >= _T('A')) && (cbChar1 <= _T('F')));
			assert((cbChar2 >= _T('0'))&&(cbChar2 <= _T('9')) || (cbChar2 >= _T('A')) && (cbChar2 <= _T('F')));
            
			if(!((cbChar1 >= _T('0'))&&(cbChar1 <= _T('9')) || (cbChar1 >= _T('A')) && (cbChar1 <= _T('F'))))
			{
               rightful = false;
			   break;
			}

			if(!((cbChar2 >= _T('0'))&&(cbChar2 <= _T('9')) || (cbChar2 >= _T('A')) && (cbChar2 <= _T('F'))))
			{
               rightful = false;
			   break;
			}

			//生成结果
			if ((cbChar2>=_T('0'))&&(cbChar2<=_T('9'))) data.cbCustomProperty[i]+=(cbChar2-_T('0'));
			if ((cbChar2>=_T('A'))&&(cbChar2<=_T('F'))) data.cbCustomProperty[i]+=(cbChar2-_T('A')+0x0A);

			//生成结果
			if ((cbChar1>=_T('0'))&&(cbChar1<=_T('9'))) data.cbCustomProperty[i]+=(cbChar1-_T('0'))*0x10;
			if ((cbChar1>=_T('A'))&&(cbChar1<=_T('F'))) data.cbCustomProperty[i]+=(cbChar1-_T('A')+0x0A)*0x10;
		}
	}
	if(strCustomRule.empty() || !rightful)
	{
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reNoActor;//没有属性
		flushProtoPacket(retPack);
		return;
	}   

	// int * pDataint = (int *)&data.cbCustomProperty[2044];  
  
	// int pDataintTmp = *pDataint;
	// 设置成功状态
	INT_PTR nErrIdPosEnd = retPack.getPosition();
	retPack.setPosition(nErrIdPosBegin);
	retPack << (BYTE)reSucc; 
	retPack.writeBuf(&data, sizeof(CActorCustomProperty)); 
	//retPack << data;  
	retPack.setPosition(nErrIdPosEnd);

	flushProtoPacket(retPack);
} 
 
void CLogicDBReqestHandler::SaveActorCustomInfoUnForbid(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID;
	int AvaliableLength = inPacket.getAvaliableLength();
	int ActorProLength = sizeof(CActorCustomProperty);
	if (AvaliableLength != ActorProLength)
	{
		OutputMsg(rmError,_T("%s Data size error UnForbid CActorCustomProperty %d : %d "), __FUNCTION__, AvaliableLength, ActorProLength);
		return;
	}

	CActorCustomProperty data;
	inPacket >> data;

	if (m_pSQLConnection->Connected() )
	{
		char *ptr = m_pHugeSQLBuffer;
		ptr += sprintf(ptr, szSQLSP_UpdateActorCustomInfo, nActorID);
 
		int count = CountArray(data.cbCustomProperty);
		ptr[0] = '"';
		ptr += 1;
		for (int i = 0; i < count; i++) 
		{ 
			ptr += sprintf(&(*ptr), ("%02X"), data.cbCustomProperty[i]);
			//ptr += 2; 
		}
		//ptr += mysql_escape_string(ptr, (const char*)&data, sizeof(data));
		ptr[0] = '"';
		ptr[1] = ')';
		ptr[2] = ';';
		ptr[3] = 0;
		ptr += 4;
		auto size = ptr - m_pHugeSQLBuffer;
		int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::SaveActorCustomInfo(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID;
	int AvaliableLength = inPacket.getAvaliableLength();
	int ActorProLength = sizeof(CActorCustomProperty);
	if (AvaliableLength != ActorProLength)
	{
		OutputMsg(rmError,_T("%s Data size error CActorCustomProperty %d : %d "), __FUNCTION__, AvaliableLength, ActorProLength);
		return;
	}

	CActorCustomProperty data;
	inPacket >> data;

	if (m_pSQLConnection->Connected() )
	{
		char *ptr = m_pHugeSQLBuffer;
		ptr += sprintf(ptr, szSQLSP_UpdateActorCustomInfo, nActorID);


		//扩展配置
		//TCHAR szCustomRule[CountArray(data.cbCustomProperty)*2+1] = {0};
		//TCHAR szCustomRule[CountArray(data)*2+1] = TEXT("");
		// for (int i = 0; i < CountArray(data.cbCustomProperty); i++) 
		// { 
		// 	_stprintf(&szCustomRule[i*2], _T("%02X"), data.cbCustomProperty[i]);
		// }
		int count = CountArray(data.cbCustomProperty);
		ptr[0] = '"';
		ptr += 1;
		for (int i = 0; i < count; i++) 
		{ 
			ptr += sprintf(&(*ptr), ("%02X"), data.cbCustomProperty[i]);
			//ptr += 2; 
		}
		//ptr += mysql_escape_string(ptr, (const char*)&data, sizeof(data));
		ptr[0] = '"';
		ptr[1] = ')';
		ptr[2] = ';';
		ptr[3] = 0;
		ptr += 4;
		auto size = ptr - m_pHugeSQLBuffer;
		int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::SaveOfflineActorProperty(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID;
	int AvaliableLength = inPacket.getAvaliableLength();
	int ActorProLength = sizeof(CActorOfflineProperty);
	if (AvaliableLength != ActorProLength)
	{
		OutputMsg(rmError,_T("%s Data size error %d : %d "), __FUNCTION__, AvaliableLength, ActorProLength);
		return;
	}

	CActorOfflineProperty data;
	inPacket >> data;

	if (m_pSQLConnection->Connected() )
	{
		char *ptr = m_pHugeSQLBuffer;
		ptr += sprintf(ptr,szSQLSP_UpdateActorOfflineData,nActorID);

		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, (const char*)&data, sizeof(data));
		ptr[0] = '"';
		ptr[1] = ')';
		ptr[2] = ';';
		ptr[3] = 0;
		ptr += 4;
		
		auto size = ptr - m_pHugeSQLBuffer;
		int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::QueryOfflineUserInfo(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID;
	bool ret = QueryOfflineUserBasicInfo(nActorID, nRawServerId, nLoginServerId);

	if (!ret) return;

	INT_PTR nErroID = reDbErr;
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadOfflineUserOther);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorID;
	INT_PTR nErrIdPosBegin = retPack.getPosition();
	retPack << (BYTE) nErroID;

	// 加载装备
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadCharItems, nActorID, itItemEquip);
	if ( !nErroID )
	{
		CUserItem data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while (pRow)
		{			
			QueryItemsImpl(pRow, data);
			data >> retPack;
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}

	// 加载技能
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadCharSkills, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			QuerySkillImpl(pRow, data);
			retPack << data;
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}
	
	// 加载强化信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorStrengthen, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			retPack << (int)atoi(pRow[0]); //ntype
			retPack << (int)atoi(pRow[1]); //pos
			retPack << (int)atoi(pRow[2]); //lv
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}

	// 加载称号信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorNewTitle, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			retPack << (unsigned int)atoi(pRow[0]);
			retPack << (unsigned int)atoi(pRow[1]);	
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}
	// 加载自定义称号信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadActorCustomTitle, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			retPack << (unsigned int)atoi(pRow[0]);
			retPack << (unsigned int)atoi(pRow[1]);	
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}

	// 加载时装信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadAlmirah, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			int nTempInt = 0;
			if (pRow[0]) sscanf(pRow[0], "%d", &nTempInt);
			retPack <<(WORD)nTempInt;

			nTempInt = 0;
			if (pRow[1]) sscanf(pRow[1], "%d", &nTempInt);
			retPack <<(int)nTempInt;

			nTempInt = 0;
			if (pRow[2]) sscanf(pRow[2], "%d", &nTempInt);
			retPack <<(byte)nTempInt;

			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}

	nErroID = m_pSQLConnection->Query(szSQLSP_LoadGhosts, nActorID);
	if ( !nErroID )
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			retPack << (int)atoi(pRow[0]); //id
			retPack << (int)atoi(pRow[1]); //lv
			retPack << (int)atoi(pRow[2]); //bless
			pRow = m_pSQLConnection->NextRow();
		}
		m_pSQLConnection->ResetQuery();
	}
	//加载圣物信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadGHallowseData, nActorID);
	if ( !nErroID )
	{ 
		int nRowCount = m_pSQLConnection->GetRowCount();
		INT_PTR pos = retPack.getPosition();
		retPack << (int)4;  

		int nLorderId1, nStarId1, nLvId1; 
		int nLorderId2, nStarId2, nLvId2; 
		int nLorderId3, nStarId3, nLvId3;  
		int nLorderId4, nStarId4, nLvId4; 
		
		char cBestAttr1[200];  //极品属性 
		memset(&cBestAttr1, 0, sizeof(cBestAttr1)); 
		
		char cBestAttr2[200];  //极品属性 
		memset(&cBestAttr2, 0, sizeof(cBestAttr2)); 

		char cBestAttr3[200];  //极品属性 
		memset(&cBestAttr3, 0, sizeof(cBestAttr3)); 

		char cBestAttr4[200];  //极品属性
		char cRefining4[200];  //洗炼属性
		memset(&cBestAttr4, 0, sizeof(cBestAttr4));
		memset(&cRefining4, 0, sizeof(cRefining4));

		MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
		while (pRow)
		{
			sscanf(pRow[1], "%d",&nLorderId1);
			sscanf(pRow[2], "%d",&nLorderId2);
			sscanf(pRow[3], "%d",&nLorderId3);
			sscanf(pRow[4], "%d",&nLorderId4);

			sscanf(pRow[5], "%d",&nStarId1);
			sscanf(pRow[6], "%d",&nStarId2);
			sscanf(pRow[7], "%d",&nStarId3);
			sscanf(pRow[8], "%d",&nStarId4);

			sscanf(pRow[9], "%d",&nLvId1);
			sscanf(pRow[10], "%d",&nLvId2);
			sscanf(pRow[11], "%d",&nLvId3);
			sscanf(pRow[12], "%d",&nLvId4);

			std::string str = "";
			if(pRow[13])
			{
				str = pRow[13]; 
			}
			if(str != "")
			{ 
				strcpy(cBestAttr1, str.c_str());
			}
			str = ""; 
			if(pRow[14])
			{
				str = pRow[14]; 
			}
			if(str != "")
			{ 
				strcpy(cBestAttr2, str.c_str());
			}
			str = "";
			if(pRow[15])
			{
				str = pRow[15]; 
			}
			if(str != "")
			{ 
				strcpy(cBestAttr3, str.c_str());
			}
			str = "";
			if(pRow[16])
			{
				str = pRow[16]; 
			}
			if(str != "")
			{ 
				strcpy(cBestAttr4, str.c_str());
			} 
 
			//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo1  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId1, nStarId1, nLvId1);
			//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo2  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId2, nStarId2, nLvId2);
			//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo3  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId3, nStarId3, nLvId3);
			//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo4  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId4, nStarId4, nLvId4);
	
			retPack << nLorderId1<< nStarId1 << nLvId1;
			retPack.writeString(cBestAttr1);  

			retPack << nLorderId2<< nStarId2 << nLvId2; 
			retPack.writeString(cBestAttr2);  

			retPack << nLorderId3<< nStarId3 << nLvId3; 
			retPack.writeString(cBestAttr3);  

			retPack << nLorderId4<< nStarId4 << nLvId4;  
			retPack.writeString(cBestAttr4);  
			pRow = m_pSQLConnection->NextRow();
		}  
		m_pSQLConnection->ResetQuery();  
	}

	// 加载宠物系统非 ai宠物信息
	nErroID = m_pSQLConnection->Query(szSQLSP_LoadLootPetData, nActorID);
	if ( !nErroID )
	{
		ONESKILLDBDATA data;
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		int nCount = m_pSQLConnection->GetRowCount();
		retPack << (int)nCount;
		while ( pRow)
		{
			retPack << atoi(pRow[0]);
			retPack << atoi(pRow[1]);
			retPack << atoi(pRow[2]);	
			pRow = m_pSQLConnection->NextRow();
		}

		m_pSQLConnection->ResetQuery();
		
	} else {
		retPack.setPosition(nErrIdPosBegin);
		retPack << (BYTE)reDbErr;
		flushProtoPacket(retPack);
		return;
	}

	// 设置成功状态
	INT_PTR nErrIdPosEnd = retPack.getPosition();
	retPack.setPosition(nErrIdPosBegin);
	retPack << (BYTE)reSucc;
	retPack.setPosition(nErrIdPosEnd);

	flushProtoPacket(retPack);
}

bool CLogicDBReqestHandler::QueryOfflineUserBasicInfo(unsigned int nActorId, int nRawServerId, int nLoginServerId)
{
	if (!m_pSQLConnection->Connected())
		return false;

	bool ret = true;
	int nError = m_pSQLConnection->Query(szSQLSP_LoadCharOfflineData, nActorId);
	CDataPacket &retPack = allocProtoPacket(dcLoadOfflineUserBasic);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorId;

	ActorOfflineMsg msg;
	unsigned int ival = 0;
	
	if (!nError)
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if (pRow)
		{
			retPack << (BYTE)reSucc;
			sscanf(pRow[0], "%s", &msg.name);
			sscanf(pRow[1], "%d", &ival); msg.nSex = ival;
			sscanf(pRow[2], "%d", &msg.nLevel);
			sscanf(pRow[3], "%d", &ival); msg.nVocation = ival;
			sscanf(pRow[4], "%d", &ival); msg.nCircle = ival;
			if (pRow[5]) sscanf(pRow[5], "%s", &msg.sGuildName);
			if (pRow[6]) memcpy(&msg.actordata, pRow[6], sizeof(CActorOfflineProperty));
			retPack << msg;
			 
			//OutputMsg(rmTip,"13466486868684offline userdata------------------------------------------------------%s:%d", msg.name, nActorId); 
		}
		else
		{
			retPack << (BYTE)reNoActor ;
			ret = false;
		}
		m_pSQLConnection->ResetQuery();	
	}
	else
	{
		retPack << (BYTE)nError;
		ret = false;
	}

	flushProtoPacket(retPack);
	return ret;
}

void CLogicDBReqestHandler::QueryOfflineUserHeroInfo(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	BYTE btHeroId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> nActorID >> btHeroId;
	QueryOfflineHeroInfo(nActorID, btHeroId, nRawServerId, nLoginServerId);
	QueryOfflineHeroEquips(nActorID,btHeroId, nRawServerId, nLoginServerId);
	//QueryEquipedGemData(nActorID, nRawServerId, nLoginServerId);
}

void CLogicDBReqestHandler::QueryOfflineHeroInfo(unsigned int nActorId, unsigned int nHeroId, int nRawServerId, int nLoginServerId)
{
	if (!m_pSQLConnection->Connected())
		return;

	CDataPacket &retPack = allocProtoPacket(dcLoadOfflineUserHeroBasic);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorId;
	
	int nError = m_pSQLConnection->Query(szSQLSP_LoadHeroCharOfflineData, nActorId, nHeroId);
	if (!nError)
	{
		MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
		if (pRow)
		{
			retPack << (BYTE)reSucc;
			retPack << (BYTE)nHeroId;

			HeroOfflineMsg msg;
			int nValue =0;
			sscanf(pRow[0], "%d", &nValue);
			msg.bCircle = (BYTE)nValue;
			sscanf(pRow[1], "%d", &nValue);
			msg.bLevel = (BYTE)nValue;
			sscanf(pRow[2], "%d", &nValue);
			msg.bVocation = (BYTE)nValue;
			sscanf(pRow[3], "%d", &nValue);
			msg.bSex = (BYTE)nValue;
			sscanf(pRow[4], "%d", &nValue);
			msg.nScore = (unsigned int)nValue;
			retPack << msg;
		}
		else
		{
			retPack << (BYTE)reNoHero;
			retPack << (BYTE)nHeroId;
		}

		m_pSQLConnection->ResetQuery();
	}
	else
	{
		retPack << (BYTE)nError;
		retPack << (BYTE)nHeroId;
	}

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::QueryOfflineHeroEquips(unsigned int nActorID,unsigned int nHeroId, int nRawServerId, int nLoginServerId)
{
	DECLARE_FUN_TIME_PROF()
	INT_PTR nErroID = reDbErr;	
	
	CDataPacket& retPack = allocProtoPacket((WORD)dcLoadOfflineUserHeroEquips);
	retPack << nRawServerId << nLoginServerId;
	retPack << nActorID;

	if (!m_pSQLConnection->Connected())
	{
		retPack << (BYTE)nErroID;
		retPack << (BYTE)nHeroId;
	}
	else
	{
		//查询这种物品
		nErroID = m_pSQLConnection->Query(szSQLSP_LoadHeroOfflineEquips, nActorID, nHeroId);
		if (!nErroID)
		{
			retPack << (BYTE)nErroID;
			retPack << (BYTE)nHeroId;

			CUserItem data;
			memset(&data,0,sizeof(data));

			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
			retPack << (int)nCount;
			int  nValue = 0;
			WORD wValue = 0;
			while (pRow)
			{				
				data.series.llId= StrToInt64(pRow[0]);
				//itemID,Quality,strong
				sscanf(pRow[1], "%d",&nValue);//itemIdQuaStrong
				data.wItemId = LOWORD(nValue);
				wValue = HIWORD(nValue);
				data.btQuality = LOBYTE(wValue);
				data.btStrong = HIBYTE(wValue);

				// //耐久度
				// sscanf(pRow[2], "%d",&nValue);
				// data.wDura =  LOWORD(nValue);
				// data.wDuraMax = HIWORD(nValue);

				//背包类型与flag,镶嵌的的第1字节
				sscanf(pRow[3], "%d",&nValue);
				wValue = LOWORD(nValue);
				data.wPackageType = LOBYTE(wValue);
				data.btFlag = HIBYTE(wValue);
				data.wCount =  HIWORD(nValue);

				//镶嵌的2,3个word
				sscanf(pRow[4], "%d",&nValue);
				wValue = LOWORD(nValue);
				data.wIdentifySlotNum = HIWORD(nValue);		//bStrongResereLevel->wIdentifySlotNum
				data.bLostStar = LOBYTE(wValue);
				data.bInSourceType = HIBYTE(wValue);

				//时间
				sscanf(pRow[5], "%d",&data.nCreatetime.tv);

				//reserverd
				sscanf(pRow[6], "%d",&nValue);

				data.nDropMonsterId = LOWORD(nValue);			//btSmithCount -> nDropMonsterId	
				wValue = HIWORD(nValue);
				data.wStar = wValue; 

				//smith1
				sscanf(pRow[7], "%d",&data.smithAttrs[0]);
				//smith2
				sscanf(pRow[8], "%d",&data.smithAttrs[1]);
				//smith3
				sscanf(pRow[9], "%d",&data.smithAttrs[2]);

				sscanf(pRow[10], "%d",&nValue);
				wValue = HIWORD(nValue);
				data.btLuck    =  HIBYTE(wValue);
				data.btHandPos =  LOBYTE(wValue);
				wValue = LOWORD(nValue);
				data.btDeportId = HIBYTE(wValue);

				if (pRow[11])
				{
					sscanf(pRow[11], "%d",&data.nAreaId);
				}

				retPack << data;
				pRow = m_pSQLConnection->NextRow();
			}
			
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
			
		}
		else
		{
			OutputMsg(rmError,
				_T("%s装载英雄离线的equips数据失败nActorId=%u,nHeroId=%d"),
				__FUNCTION__,
				nActorID,
				nHeroId); 
			retPack << (BYTE)nErroID;
			retPack << (BYTE)nHeroId;
		}
	}

	flushProtoPacket(retPack);
}

void CLogicDBReqestHandler::SaveActorToDB(CDataPacketReader &inPacket, bool bResp)  
{
	DECLARE_FUN_TIME_PROF()		
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	if (inPacket.getAvaliableLength() != sizeof(ACTORDBDATA) + sizeof(unsigned long long))
	{
		OutputMsg(rmError,_T("%s Data size error"), __FUNCTION__);
		return;
	}
	
	ACTORDBDATA data;
	inPacket >> data;

	unsigned long long nTradingQuota = 0;
	inPacket >> nTradingQuota;

	CDataPacket* pPacket = NULL;
	if (bResp)
	{
		pPacket = &allocProtoPacket(dcSave);
		*pPacket << nRawServerId << nLoginServerId;
		*pPacket << data.nID;
	}
	//OutputMsg(rmTip, _T("Save Actor[%d] Basic Data"), data.nID);
	if (!m_pSQLConnection->Connected() )
	{
		OutputMsg(rmError,
				 _T("save actor data error!id=%d,x=%d,y=%d,sex=%d,vocation=%d,level=%d,icon=%d,exp=%lld,fb=%d,sceneid=%d"),
				 data.nID, 
				 data.nPosX,
				 data.nPosY,
				 data.nSex,
				 data.nVocation,
				 data.nLevel,
				 data.nIcon,
				 data.lExp,
				 data.nFbHandle,
				 data.nScenceID);

		if (pPacket)
			*pPacket << (char)reDbErr;
	}
	else
	{
		//int nSlotCount = MAKELONG(MAKEWORD(data.nBagGridCount,0), (WORD)data.nDeportGridCount);
		int nCount = MAKELONG(data.nBroatNum, data.nFlyShoes);
		int nValue = MAKELONG(data.nMeridianLv, 0);
		char *ptr = m_pHugeSQLBuffer;
		ptr += sprintf(ptr, 
					   szSQLSP_UpdateCharBasicData,
					   data.nID, 
					   data.nPosX,
					   data.nPosY,
					   data.nSex,
					   data.nVocation,
					   data.nLevel,
					   data.nIcon,
					   data.lExp,
					   data.nFbHandle,
					   data.nScenceID, //10
					   data.nPKValue,
					   data.nMedicineCardTime,
					   data.nBagGridCount,
					   data.nDeportGridCount,
					  
					   data.nBindYuanbao,
					   data.nNonBindCoin,
					   data.nNonBindYuanbao,
					  
					   data.nHp,
					   data.nMp,
					   data.nEvilPkStatus, //20
					 
					   data.nGuildID,
					   data.sGuildname,
					   data.nTeamID,
					   data.nSocialMask,
					   data.nGuildExp,
					   data.lLastLoginIp,
					   data.nLastLoginTime,
					   data.nLastLogoutTime,
					   data.nModelId, //将模型id存上
					   data.nDeport3ExpiredTime,			//没用上，可以利用的	//30
					   data.nDeport4ExpiredTime,			//没用上，可以利用的
					   data.nEnterFbScenceID,
					   data.nEnterFbPos,
					   data.nActorDir,
					   data.lMultiExpUnused,  //未使用的多倍经验
					   data.csrevivepoint,
					   data.fbrevivepoint,
					   data.cityrevivepoint,

					   data.achievepoint,

					   data.nNextChangeVocTime,//40
					   data.nForeverCardFlag,
					   data.nMonthCardTime,
					 
					   data.nExpToday,
					   0,
					  
					   data.nBindCoin,
					   data.nFashionBattle,
					   data.nCardAwardFlag,		
					   data.nEquipScore,
					   data.nConsumeYb,
				
					  
					   data.nDrawYbCount,//50
					   data.nActivity,
					   data.nBattlePower,
					   data.nLastLoginServerId,
					   data.nMeritoriousDay,
					   data.nRecoverState,
					   data.nLootPetId,
					   data.nPersonBossJifen,
					   data.nCircle,
					   data.nCircleSoul,
					   "",//60
					   data.nAnger,
					   0,
					   data.nCheckIns,
					   0,
					   0,
					   0,
					   0,
					   0,
					   0,
					   data.nCurnewTitel,//70
					   0,
					   data.nDimensionalKey,
					   data.nDefaultLootPetDis,
					   data.nRebateDrawYbCount,
					   data.nSwingLevel,
					   0,
					   data.nSwingId,
					   data.nLoginDays,
					   "",
					   data.nCSActorId,//80
					   data.nSalary,
					   data.nOffice,
					   data.nDepotCoin,
					   0,
					   0,
					   0,
					   data.nSupperPlayLvl,
					   data.nFrenzy,
					   0,
					   data.nBeKilledCount,//90
					   data.nKillMonsterCount,
					   data.nAlmirahLv,
					   0,
					   data.nTotalOnlineMin, 
					   0,
					   data.nLastLoginOpendayNo_,  //上次登录的开服天数
					   data.nCurCustomTitle,//当前自定义称号		
					   0,
					   0,
					   1,	//100						//在线标识，保存标1,logout再标0
					   data.nPlayerMaxAttack	,		//玩家最大攻击力，放在最后不load
					   data.nBless, //战绩
					   data.nPrestige, //今日战绩
					   nCount,
					   data.nRecyclepoints, //积分
					   nValue,
					   data.nShutUpTime, //禁言时间 //107
					   nTradingQuota //交易额度 //108
					   );
					    
		//OutputMsg(rmNormal,_T("data.nXiuWei=%d"),data.nXiuWei);

		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, data.quest, sizeof(data.quest));
		ptr[0] = '"';
		ptr[1] = ',';
		ptr += 2;
			
		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, data.achieveFinishState, sizeof(data.achieveFinishState));
		ptr[0] = '"';
		ptr[1] = ',';
		ptr += 2;

		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, data.achieveEventData, sizeof(data.achieveEventData));
		ptr[0] = '"';
		ptr[1] = ',';
		ptr += 2;

		ptr[0] = '"';
		ptr += 1;
		ptr += mysql_escape_string(ptr, data.titles, sizeof(data.titles));
		ptr[0] = '"';
		ptr[1] = ')';
		ptr += 2;
			
		int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();
			if (pPacket)
				*pPacket << (char)reSucc;
		}
		else
		{
			OutputMsg(rmError,
					  _T("save actor data error!id=%d,x=%d,y=%d,sex=%d,vocation=%d,level=%d,icon=%d,exp=%lld,fb=%d,sceneid=%d"),
					  data.nID, 
					  data.nPosX,
					  data.nPosY,
					  data.nSex,
					  data.nVocation,
					  data.nLevel,
					  data.nIcon,
					  data.lExp,
					  data.nFbHandle,
					  data.nScenceID);
			if (pPacket)
				*pPacket << (char)reDbErr;
		}
	}

	if (pPacket)
		flushProtoPacket(*pPacket);
}

void CLogicDBReqestHandler::UpdateChallengeData(CDataPacketReader& inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerIdx = 0, nLoginServerId = 0;
	inPacket >> nRawServerIdx >> nLoginServerId;

	int nErrorCode = reDbErr;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQL_UpdateChallengeData, nRawServerIdx);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
	}

	CDataPacket& retPack = allocProtoPacket((WORD)dcUpdateChallengeData);
	retPack << nRawServerIdx << nLoginServerId;
	
	retPack << (BYTE)nErrorCode;
	flushProtoPacket(retPack);
}


void CLogicDBReqestHandler::QueryActorExists( CDataPacketReader & inPacket )
{
	DECLARE_FUN_TIME_PROF()
	int nRawServer = 0, nServerId = 0, nReturnCode = 0, nStoreId=0, nItemCount=0;
	unsigned int nActorId = 0, nQueryActorId = 0;
	ACTORNAME sQueryName;
	inPacket >> nRawServer >> nServerId >> nActorId >> nStoreId >> nItemCount;
	inPacket.readString(sQueryName, ArrayCount(sQueryName));
	int nErrorId = reSucc;
	if(!m_pSQLConnection->Connect())
		nErrorId = reDbErr;
	else
	{
		nErrorId = m_pSQLConnection->Query(szSQLSP_GetValidActorIdByName, sQueryName, nServerId);
		if(!nErrorId)
		{
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			if (pRow)
			{
				nReturnCode = 1;
				nQueryActorId = atoi(pRow[0]);
			}
			m_pSQLConnection->ResetQuery();
		}
	}
	CDataPacket& retPack = allocProtoPacket(dcQueryActorExists);
	retPack << (int)nRawServer <<(int)nServerId << (unsigned int)nActorId;
	retPack << (BYTE)nErrorId << (BYTE)nReturnCode << (int)nStoreId << (int)nItemCount;
	retPack.writeString(sQueryName);
	retPack << nQueryActorId;
	flushProtoPacket(retPack);
}
void CLogicDBReqestHandler::UpdateCharOnlineStatus(unsigned int nActorId, int nStatus)
{
	int nErrorId = reSucc;
	if(!m_pSQLConnection->Connect())
	{
		nErrorId = reDbErr;
	}
	else
	{
		nErrorId = m_pSQLConnection->Exec(szSQLSP_UpdateCharOnlineStatus, nActorId, nStatus);
		if (!nErrorId)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}
void CLogicDBReqestHandler::UpdateCharStatus(CDataPacketReader & inPacket)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nServerId = 0;
	unsigned int nActorId = 0;
	byte nStatusVal = 0;
	inPacket >> nRawServerId >> nServerId >> nActorId >> nStatusVal;
	int nErrorId = reSucc;
	if(!m_pSQLConnection->Connect())
	{
		nErrorId = reDbErr;
	}
	else
	{
		nErrorId = m_pSQLConnection->Exec(szSQLSP_UpdateCharStatus, nStatusVal, nActorId);
		if (!nErrorId)
		{
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::SaveActorStaticCounts(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		 nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteStaticCounts, nActorID);
		 if(!nErrorCode)
		 {
			 m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		 }
		 if(nErrorCode == reSucc)
		 {
			 int nCount = 0;
			
			 packet >> nCount; //取物品的数量
			 //insert into xxx (actorID,...) Values 
			 if(nCount >0 ) //如果有物品的话
			 {
				 sprintf(m_pHugeSQLBuffer,szSQLSP_InsertStaticCounts); //表头放这里

				 char buff[1024];
				 int nType =0;
				 int nValue=0;
				 for(INT_PTR i =0;i < nCount ; i++)
				 {
					 strcat(m_pHugeSQLBuffer," (");
					 packet >> nType>>nValue; //装载入物品的数据

					 sprintf(buff,"%u,%d,%d",nActorID, nType, nValue);
					 strcat(m_pHugeSQLBuffer,buff);
					 strcat(m_pHugeSQLBuffer," )");
					 if(i < nCount -1)
					 {
						 strcat(m_pHugeSQLBuffer,",");	
					 }
				 }
				 //std::cout <<m_pHugeSQLBuffer <<std::endl;
				 nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				 if(!nErrorCode)  
					 m_pSQLConnection->ResetQuery(); 
			 }
		 }
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveStaticCount);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}


void CLogicDBReqestHandler::LoadActorStaticCounts( unsigned int nActorID,unsigned int nRawServerId,unsigned int nLoginServerId)
{
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadStaticCounts, nActorID);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadStaticCount);
			OutPacket << nRawServerId << nLoginServerId << nActorID << (BYTE)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				OutPacket << (int)atoi(pRow[1]);
				OutPacket << (int)atoi(pRow[2]);	
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}



void CLogicDBReqestHandler::SaveActorStrengthenInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		 nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteActorStrengthen, nActorID);
		 if(!nErrorCode)
		 {
			 m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		 }
		 if(nErrorCode == reSucc)
		 {
			 int nCount = 0;
			
			 packet >> nCount; //取数量
			 if(nCount >0 ) 
			 {
				 sprintf(m_pHugeSQLBuffer,szSQLSP_InsertActorStrengthen); //表头放这里

				 char buff[1024];
				 int nType =0;
				 int nPos=0;
				 int nlv = 0;
				 for(INT_PTR i =0;i < nCount ; i++)
				 {
					 strcat(m_pHugeSQLBuffer," (");
					 packet >> nType>>nPos >>nlv; //

					 sprintf(buff,"%u,%d,%d,%d",nActorID, nType, nPos,nlv);
					 strcat(m_pHugeSQLBuffer,buff);
					 strcat(m_pHugeSQLBuffer," )");
					 if(i < nCount -1)
					 {
						 strcat(m_pHugeSQLBuffer,",");	
					 }
				 }
				 nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				 if(!nErrorCode)  
					 m_pSQLConnection->ResetQuery(); 
			 }
		 }
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveActorStrengthenInfo);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}


void CLogicDBReqestHandler::QueryStrengthenImpl(MYSQL_ROW pRow, CDataPacket& OutPacket)
{
	OutPacket << (int)atoi(pRow[0]); //ntype
	OutPacket << (int)atoi(pRow[1]); //pos
	OutPacket << (int)atoi(pRow[2]); //lv
}

void CLogicDBReqestHandler::LoadActorStrengthenInfo(unsigned int nActorID,unsigned int nRawServerId,unsigned int nLoginServerId)
{
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadActorStrengthen, nActorID);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadActorStrengthenInfo);
			OutPacket << nRawServerId << nLoginServerId << nActorID << (BYTE)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				QueryStrengthenImpl(pRow, OutPacket);
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}



void CLogicDBReqestHandler::LoadActorAchieveMentInfo( unsigned int nActorID,unsigned int nRawServerId,unsigned int nLoginServerId)
{
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadAchieveMent, nActorID);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadAchieveMent);
			OutPacket << nRawServerId << nLoginServerId << nActorID << (BYTE)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				OutPacket << (int)atoi(pRow[1]);
				OutPacket << (int)atoi(pRow[2]);	
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}



void CLogicDBReqestHandler::SaveActorAchieveMentInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteAchieveMent, nActorID);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
		if(nErrorCode == reSucc)
		{
			int nCount = 0;
		
			packet >> nCount; //取物品的数量
			//insert into xxx (actorID,...) Values 
			if(nCount >0 ) //如果有物品的话
			{
				sprintf(m_pHugeSQLBuffer,szSQLSP_InsertAchieveMent); //表头放这里

				char buff[1024];
				int nType =0;
				int nValue=0;
				for(INT_PTR i =0;i < nCount ; i++)
				{
					strcat(m_pHugeSQLBuffer," (");
					packet >> nType>>nValue; //装载入物品的数据

					sprintf(buff,"%u,%d,%d",nActorID, nType, nValue);
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer," )");
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}
				//std::cout <<m_pHugeSQLBuffer <<std::endl;
				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if(!nErrorCode)  
					m_pSQLConnection->ResetQuery(); 
			}
		}
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveStaticCount);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}


void CLogicDBReqestHandler::SaveOfflineAchieveMentInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_DelOfflineAchieveMent);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		}
		if(nErrorCode == reSucc)
		{
			int nCount = 0;
		
			packet >> nCount; //取物品的数量
			if(nCount >0 ) //如果有物品的话
			{
				sprintf(m_pHugeSQLBuffer,szSQLSP_InsertOfflineAchieveMent); //表头放这里

				char buff[1024];
				unsigned int nActorId =0;
				int nSubtype=0;
				int nType =0;
				int nValue=0;
				int nway = 0;
				for(INT_PTR i =0;i < nCount ; i++)
				{
					strcat(m_pHugeSQLBuffer," (");
					packet >> nActorId>>nType>>nSubtype; //装载入物品的数据
					packet >> nValue>>nway;
					sprintf(buff,"%u,%d,%d,%d,%d",nActorID, nType, nSubtype,nValue,nway);
					strcat(m_pHugeSQLBuffer,buff);
					strcat(m_pHugeSQLBuffer," )");
					if(i < nCount -1)
					{
						strcat(m_pHugeSQLBuffer,",");	
					}
				}
				//std::cout <<m_pHugeSQLBuffer <<std::endl;
				nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				if(!nErrorCode)  
					m_pSQLConnection->ResetQuery(); 
			}
		}
	}
}




void CLogicDBReqestHandler::LoadOfflineAchieveMentInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	packet >> nRawServerId >> nLoginServerId;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_InsertOfflineAchieveMent);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadofflineAchieve);
			OutPacket << nRawServerId << nLoginServerId << (BYTE)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				OutPacket << (unsigned int)atoi(pRow[0]);
				OutPacket << (int)atoi(pRow[1]);	
				OutPacket << (int)atoi(pRow[2]);
				OutPacket << (int)atoi(pRow[3]);	
				OutPacket << (int)atoi(pRow[4]);	
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
	}
}

void CLogicDBReqestHandler::SaveActorGhost(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没有连接上
	}
	else 
	{
		 nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteGhosts, nActorID);
		 if(!nErrorCode)
		 {
			 m_pSQLConnection->ResetQuery(); //函数调用里没有重置数据的
		 }
		 if(nErrorCode == reSucc)
		 {
			 int nCount = 0;
			
			 packet >> nCount; //取物品的数量
			 //insert into xxx (actorID,...) Values 
			 if(nCount >0 ) //如果有物品的话
			 {
				 sprintf(m_pHugeSQLBuffer,szSQLSP_InsertGhosts); //表头放这里

				 char buff[1024];
				 for(INT_PTR i =0;i < nCount ; i++)
				 {
					 strcat(m_pHugeSQLBuffer," (");
					 GhostData data;
            		packet >> data.nId >> data.nLv >> data.nBless;

					 sprintf(buff,"%u,%d,%d,%d",nActorID, data.nId,data.nLv, data.nBless);
					 strcat(m_pHugeSQLBuffer,buff);
					 strcat(m_pHugeSQLBuffer," )");
					 if(i < nCount -1)
					 {
						 strcat(m_pHugeSQLBuffer,",");	
					 }
				 }
				 //std::cout <<m_pHugeSQLBuffer <<std::endl;
				 nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
				 if(!nErrorCode)  
					 m_pSQLConnection->ResetQuery(); 
			 }
		 }
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveGhostInfo);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}


void CLogicDBReqestHandler::LoadActorGhost(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId;
	packet >> nActorID;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadGhosts, nActorID);
		if (!nErrorCode)
		{
			CDataPacket& OutPacket = allocProtoPacket(dcLoadGhostInfo);
			OutPacket << nRawServerId << nLoginServerId << nActorID << (BYTE)nErrorCode;
			int nRowCount = m_pSQLConnection->GetRowCount();
			OutPacket << (int)nRowCount;
			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
			while(pRow)
			{
				OutPacket << (int)atoi(pRow[0]);
				OutPacket << (int)atoi(pRow[1]);	
				OutPacket << (int)atoi(pRow[2]);
				pRow = m_pSQLConnection->NextRow();
			}
			flushProtoPacket(OutPacket);
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
}

//
void CLogicDBReqestHandler::SaveHallowsSystemInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId >> nActorID;
	
	//return; 
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected; //db没连接上
	}
	else 
	{
		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_UpdateHallowseData); 
 
		// 保存运行中的
		int nCount = 0;  
	
		packet >> nCount; //取物品的数量  
		if(nCount == 4) //目前圣物只有冰魂，四个pos
		{ 
			int nLorderId1, nStarId1, nLvId1; 
			int nLorderId2, nStarId2, nLvId2; 
			int nLorderId3, nStarId3, nLvId3;  
			int nLorderId4, nStarId4, nLvId4; 
			
			char cBestAttr1[200];  //极品属性
			char cRefining1[200];  //洗炼属性
			memset(&cBestAttr1, 0, sizeof(cBestAttr1));
			memset(&cRefining1, 0, sizeof(cRefining1));
			
			char cBestAttr2[200];  //极品属性
			char cRefining2[200];  //洗炼属性
			memset(&cBestAttr2, 0, sizeof(cBestAttr2));
			memset(&cRefining2, 0, sizeof(cRefining2));
			
			char cBestAttr3[200];  //极品属性
			char cRefining3[200];  //洗炼属性
			memset(&cBestAttr3, 0, sizeof(cBestAttr3));
			memset(&cRefining3, 0, sizeof(cRefining3));

			char cBestAttr4[200];  //极品属性
			char cRefining4[200];  //洗炼属性
			memset(&cBestAttr4, 0, sizeof(cBestAttr4));
			memset(&cRefining4, 0, sizeof(cRefining4));

			packet >> nLorderId1 >> nStarId1 >> nLvId1;
			
			packet.readString(cBestAttr1, ArrayCount(cBestAttr1));
			cBestAttr1[sizeof(cBestAttr1)-1] = 0;
			packet.readString(cRefining1, ArrayCount(cRefining1));
			cRefining1[sizeof(cRefining1)-1] = 0;

			packet >> nLorderId2 >> nStarId2 >> nLvId2;
			
			packet.readString(cBestAttr2, ArrayCount(cBestAttr2));
			cBestAttr2[sizeof(cBestAttr2)-1] = 0;
			packet.readString(cRefining2, ArrayCount(cRefining2));
			cRefining2[sizeof(cRefining2)-1] = 0;

			packet >> nLorderId3 >> nStarId3 >> nLvId3;
			
			packet.readString(cBestAttr3, ArrayCount(cBestAttr3));
			cBestAttr3[sizeof(cBestAttr3)-1] = 0;
			packet.readString(cRefining3, ArrayCount(cRefining3));
			cRefining3[sizeof(cRefining3)-1] = 0;

			packet >> nLorderId4 >> nStarId4 >> nLvId4;

			packet.readString(cBestAttr4, ArrayCount(cBestAttr4));
			cBestAttr4[sizeof(cBestAttr4)-1] = 0;
			packet.readString(cRefining4, ArrayCount(cRefining4));
			cRefining4[sizeof(cRefining4)-1] = 0;

			//OutputMsg(rmNormal, _T("========================SaveHallowsSystemInfo1  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId1, nStarId1, nLvId1);
			//OutputMsg(rmNormal, _T("========================SaveHallowsSystemInfo2  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId2, nStarId2, nLvId2);
			//OutputMsg(rmNormal, _T("========================SaveHallowsSystemInfo3  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId3, nStarId3, nLvId3);
			//OutputMsg(rmNormal, _T("========================SaveHallowsSystemInfo4  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId4, nStarId4, nLvId4);
        
			char *ptr = m_pHugeSQLBuffer;
			ptr += sprintf(ptr, szSQLSP_UpdateHallowseData, nActorID, 
			nLorderId1, nLorderId2, nLorderId3, nLorderId4,
			nStarId1, nStarId2, nStarId3, nStarId4,
			nLvId1, nLvId2, nLvId3, nLvId4,
			cBestAttr1, cBestAttr2, cBestAttr3, cBestAttr4,
			cRefining1, cRefining2, cRefining3, cRefining4
			); 
			
			auto size = ptr - m_pHugeSQLBuffer;
			int nError = m_pSQLConnection->RealExec(m_pHugeSQLBuffer, ptr - m_pHugeSQLBuffer);
			if ( !nError )
			{
				m_pSQLConnection->ResetQuery();
			}  
		 }
	}
	
	if (nErrorCode)
	{
		CDataPacket& retPack = allocProtoPacket(dcSaveHallowsSystemInfo);
		retPack << nRawServerId << nLoginServerId;
		retPack << nActorID << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}
void CLogicDBReqestHandler::LoadHallowsSystemInfo(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId;
	packet >> nActorID;
	
	CDataPacket& OutPacket = allocProtoPacket(dcLoadHallowsSystemInfo);
	OutPacket << nRawServerId << nLoginServerId << nActorID;

	int nCount = 0;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		OutPacket << (BYTE)reDbErr; 
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_LoadGHallowseData, nActorID);
		if (!nErrorCode)
		{ 
            OutPacket << (BYTE)reSucc;
			int nRowCount = m_pSQLConnection->GetRowCount();
            INT_PTR pos = OutPacket.getPosition();
			OutPacket << (int)0;
			nCount = 0;
			//OutPacket << (int)nRowCount;

			int nLorderId1, nStarId1, nLvId1; 
			int nLorderId2, nStarId2, nLvId2; 
			int nLorderId3, nStarId3, nLvId3;  
			int nLorderId4, nStarId4, nLvId4; 
			
			char cBestAttr1[50];  //极品属性
			char cRefining1[50];  //洗炼属性
			memset(&cBestAttr1, 0, sizeof(cBestAttr1));
			memset(&cRefining1, 0, sizeof(cRefining1));
			
			char cBestAttr2[50];  //极品属性
			char cRefining2[50];  //洗炼属性
			memset(&cBestAttr2, 0, sizeof(cBestAttr2));
			memset(&cRefining2, 0, sizeof(cRefining2));
			
			char cBestAttr3[50];  //极品属性
			char cRefining3[50];  //洗炼属性
			memset(&cBestAttr3, 0, sizeof(cBestAttr3));
			memset(&cRefining3, 0, sizeof(cRefining3));

			char cBestAttr4[50];  //极品属性
			char cRefining4[50];  //洗炼属性
			memset(&cBestAttr4, 0, sizeof(cBestAttr4));
			memset(&cRefining4, 0, sizeof(cRefining4));

			MYSQL_ROW  pRow = m_pSQLConnection->CurrentRow();
            while (pRow)
			{
				sscanf(pRow[1], "%d",&nLorderId1);
				sscanf(pRow[2], "%d",&nLorderId2);
				sscanf(pRow[3], "%d",&nLorderId3);
				sscanf(pRow[4], "%d",&nLorderId4);
 
				sscanf(pRow[5], "%d",&nStarId1);
				sscanf(pRow[6], "%d",&nStarId2);
				sscanf(pRow[7], "%d",&nStarId3);
				sscanf(pRow[8], "%d",&nStarId4);

				sscanf(pRow[9], "%d",&nLvId1);
				sscanf(pRow[10], "%d",&nLvId2);
				sscanf(pRow[11], "%d",&nLvId3);
				sscanf(pRow[12], "%d",&nLvId4);

				std::string str = "";
				if(pRow[13])
				{
					str = pRow[13]; 
				}
				if(str != "")
				{ 
					strcpy(cBestAttr1, str.c_str());
				}
				str = ""; 
				if(pRow[14])
				{
					str = pRow[14]; 
				}
				if(str != "")
				{ 
					strcpy(cBestAttr2, str.c_str());
				}
				str = "";
				if(pRow[15])
				{
					str = pRow[15]; 
				}
				if(str != "")
				{ 
					strcpy(cBestAttr3, str.c_str());
				}
				str = "";
				if(pRow[16])
				{
					str = pRow[16]; 
				}
				if(str != "")
				{ 
					strcpy(cBestAttr4, str.c_str());
				}
				str = "";
				if(pRow[17])
				{
					str = pRow[17]; 
				}
				if(str != "")
				{ 
					strcpy(cRefining1, str.c_str());
				}
				str = "";
				if(pRow[18])
				{
					str = pRow[18]; 
				}
				if(str != "")
				{ 
					strcpy(cRefining2, str.c_str());
				}

				str = "";
				if(pRow[19])
				{
					str = pRow[19]; 
				}
				if(str != "")
				{ 
					strcpy(cRefining3, str.c_str());
				}

				str = "";
				if(pRow[20])
				{
					str = pRow[20]; 
				}
				if(str != "")
				{ 
					strcpy(cRefining4, str.c_str());
				} 
				//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo1  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId1, nStarId1, nLvId1);
				//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo2  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId2, nStarId2, nLvId2);
				//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo3  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId3, nStarId3, nLvId3);
				//OutputMsg(rmNormal, _T("************************SaveHallowsSystemInfo4  nLorderId=%d, nStarId=%d, nLvId=%d,"),  nLorderId4, nStarId4, nLvId4);
        
				OutPacket << nLorderId1<< nStarId1 << nLvId1;
				OutPacket.writeString(cBestAttr1); 
				OutPacket.writeString(cRefining1); 

				OutPacket << nLorderId2<< nStarId2 << nLvId2; 
				OutPacket.writeString(cBestAttr2); 
				OutPacket.writeString(cRefining2); 

				OutPacket << nLorderId3<< nStarId3 << nLvId3; 
				OutPacket.writeString(cBestAttr3); 
				OutPacket.writeString(cRefining3); 

				OutPacket << nLorderId4<< nStarId4 << nLvId4;  
				OutPacket.writeString(cBestAttr4); 
				OutPacket.writeString(cRefining4); 
				nCount++;
				pRow = m_pSQLConnection->NextRow();
			} 

            int* pCount = (int*)OutPacket.getPositionPtr(pos);
			*pCount = nCount;
			m_pSQLConnection->ResetQuery();
			OutPacket << nCount;
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
    flushProtoPacket(OutPacket);
}

//跨服
void CLogicDBReqestHandler::LoadCrossGuildId(CDataPacketReader& packet)
{
	DECLARE_FUN_TIME_PROF()
	int nRawServerId = 0, nLoginServerId = 0;
	unsigned int nActorID = 0;
	packet >> nRawServerId >> nLoginServerId;
	packet >> nActorID;
	
	CDataPacket& OutPacket = allocProtoPacket(dcLoadCrossGuildId);
	OutPacket << nRawServerId << nLoginServerId << nActorID;

	int nCount = 0;
	int nErrorCode = reSucc;
	if (!m_pSQLConnection->Connected())
	{
		OutPacket << (BYTE)reDbErr; 
	}
	else
	{
		nErrorCode = m_pSQLConnection->Query(szSQLSP_QueryeCrossServerGuildId, nActorID);
		if (!nErrorCode)
		{ 
            OutPacket << (BYTE)reSucc;
			int nResult = 0;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();

			nResult = 0;
			if (pRow && pRow[0]) sscanf(pRow[0], "%d", &nResult);
			OutPacket <<(int)nResult;

			nResult = 0;
			if (pRow && pRow[1]) sscanf(pRow[1], "%d", &nResult);
			OutPacket <<(int)nResult;

			m_pSQLConnection->ResetQuery();
		}
		else
		{
			OutputMsg(rmTip, _T("%s"), mysql_error(m_pSQLConnection->GetMySql()));
		}
	}
    flushProtoPacket(OutPacket);
}


void CLogicDBReqestHandler::ChangeActorTradingQuota(CDataPacketReader & inPacket)
{
	DECLARE_FUN_TIME_PROF()

	int nRawServerId = 0, nLoginServerId = 0, nErrorCode = 0;
	std::string strActorId = "";
	std::string strTradingQuota = "";
	unsigned int nActorId = 0;
	unsigned long long nTradingQuota = 0;

	inPacket >> nRawServerId >> nLoginServerId;
	inPacket >> strActorId >> strTradingQuota;
	
	nActorId = atoi(strActorId.c_str());
	nTradingQuota = strtoull(strTradingQuota.c_str(), nullptr, 10);

	if (!m_pSQLConnection->Connected())
	{
		nErrorCode = reDbNotConnected;
	}
	else
	{
		nErrorCode = m_pSQLConnection->Exec(szSQLSP_ChangeTradingQuota, nActorId, nTradingQuota);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}
	}

	if( nErrorCode != reSucc)
	{
		OutputMsg(rmError,_T("%s修改玩家交易额度 失败%u"),__FUNCTION__,nActorId);
		CDataPacket& retPack = allocProtoPacket(dcBackChangeTradingQuota);
		retPack << nRawServerId << nLoginServerId << nActorId << (BYTE) nErrorCode;
		flushProtoPacket(retPack);
	}
}
