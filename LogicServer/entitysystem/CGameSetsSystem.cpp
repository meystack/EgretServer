#include "StdAfx.h"
#include "CGameSetsSystem.h"
#include "ActorDbData.h"
#include "../base/Container.hpp"

// //字符串分割函数
// std::vector<std::string> SplitStr(std::string str,const std::string pattern)
// {
//   	std::string::size_type pos;
//   	std::vector<std::string> result;
//   	if (str.length() != 0)
// 	{
// 		str+=pattern;//扩展字符串以方便操作
//   		int size=str.size();

// 		for(int i=0; i<size; i++)
// 		{
// 			pos=str.find(pattern,i);
// 			if(pos<size)
// 			{
// 			std::string s=str.substr(i,pos-i);
// 			result.push_back(s);
// 			i=pos+pattern.size()-1;
// 			}
// 		}
// 	}
//   return result;
// }

CGameSetsSystem::CGameSetsSystem()
{
	m_kbDataList.clear();
	m_kbAppDataList.clear();
}

CGameSetsSystem::~CGameSetsSystem()
{
}

bool CGameSetsSystem::Initialize(void *data, SIZE_T size)
{
	PACTORDBDATA pActorData = (ACTORDBDATA *)data;
	if (pActorData == NULL || size != sizeof(ACTORDBDATA))
	{
		OutputMsg(rmError, _T("data len error len=%d ,correct len=%d"), size, sizeof(ACTORDBDATA));
		return false;
	}
	//LoadFromDB();

	return true;
}

void CGameSetsSystem::Destroy()
{
	m_kbDataList.clear();
	m_kbAppDataList.clear();
}

VOID CGameSetsSystem::OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader &reader)
{
	if (m_pEntity == NULL)
	{
		OutputMsg(rmError, _T("LoadOtherGameSets Data is Error..."));
		return;
	}
	if (nCmd == jxInterSrvComm::DbServerProto::dcLoadOtherGameSets && nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if (HasDbDataInit())
		{
			OutputMsg(rmError, _T("db return NewTitle repeated,actorid=%u "), m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}

		// (1) 按键数据

		reader.readBuf(&m_GsData, sizeof(GAMESETSDATA));
		std::string sKb = m_GsData.m_vKbs;
		std::vector<std::string> results = SplitStr(sKb, "|");
		int nCount = results.size();
		m_kbDataList.clear();
		m_kbAppDataList.clear();
		int n_max = MAX_BROCASTKEY_SIZE + MAX_BROCASTKEYAPP_SIZE;
		for (int i = 0; i < n_max; i++)
		{
			if(i < MAX_BROCASTKEY_SIZE)
			{
				KEYBOARDDATA kbData;
				kbData.nPos = i;
				if (i >= nCount)//没有存，走默认配置
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbDataList.push_back(kbData);
					continue;
				}
				
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() < 2)
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbDataList.push_back(kbData);
					continue;
				}
				  
				kbData.nType = atoi(temp_res[0].c_str());
				int id = atoi(temp_res[1].c_str());
				if (kbData.nType == 1)
				{
					const OneSkillData *pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(id);
					if (pSkillConf == NULL ||(pSkillConf && pSkillConf->boIsDelete))
						id = 0;
				}
				else
				{
					const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(id);
					if (pStdItem == NULL)
					{
						id = 0;
					}
				}

				kbData.uId = id; 
				m_kbDataList.push_back(kbData);
			}

			if(i >= MAX_BROCASTKEY_SIZE && i < n_max)
			{ 
				KEYBOARDDATA kbData;
				kbData.nPos = i;
				if (i >= nCount)//没有存，走默认配置
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbAppDataList.push_back(kbData);
					continue;
				}
				
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() < 2)
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbAppDataList.push_back(kbData);
					continue;
				}
				  
				kbData.nType = atoi(temp_res[0].c_str());
				int id = atoi(temp_res[1].c_str());
				if (kbData.nType == 1)
				{
					const OneSkillData *pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(id);
					if (pSkillConf == NULL ||(pSkillConf && pSkillConf->boIsDelete))
						id = 0;
				}
				else
				{
					const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(id);
					if (pStdItem == NULL)
					{
						id = 0;
					}
				}

				kbData.uId = id;
				m_kbAppDataList.push_back(kbData);
			}
		}
 
		// 等级检查
		int level = ((CActor*)m_pEntity)->GetLevel();
		int circle = ((CActor*)m_pEntity)->GetCircleLevel();
		if(level > 1 || circle > 1)
		//if(0)
		{
			bool app_RECYLE = false;
			bool app_RANDOM = false;
			bool app_MEDICINE = false;
			for (int i = 0; i < m_kbAppDataList.size(); i++)
			{  
				KEYBOARDDATA &data = m_kbAppDataList[i];
				if (data.uId == eSPECIAL_HOTKEY_RECYLE )
				{
					app_RECYLE = true;
				}
				if (data.uId == eSPECIAL_HOTKEY_RANDOM)
				{
					app_RANDOM = true;
				}
				if (data.uId == eSPECIAL_HOTKEY_MEDICINE)
				{
					app_MEDICINE = true;
				}
			}
			////////////////////////////////////
			if (!app_RECYLE)
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.nPos == eSPECIAL_HOTKEY_RECYLE_POS
					 && data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_RECYLE; 
						data.nType = 1;
						app_RECYLE = true;
						break;
					} 
				}
			}
			if (!app_RECYLE)//原来格子已经被占用了
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_RECYLE; 
						data.nType = 1;
						app_RECYLE = true;
						break;
					} 
				}
			}
			/////////////////////////////////
			if (!app_RANDOM)
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.nPos == eSPECIAL_HOTKEY_RANDOM_POS
					 && data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_RANDOM; 
						data.nType = 1;
						app_RANDOM = true;
						break;
					} 
				}
			}
			if (!app_RANDOM)//原来格子已经被占用了
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_RANDOM; 
						data.nType = 1;
						app_RANDOM = true;
						break;
					} 
				}
			}
			
			/////////////////////////////////
			if (!app_MEDICINE)
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.nPos == eSPECIAL_HOTKEY_MEDICINE_POS
					 && data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_MEDICINE; 
						data.nType = 0;
						app_MEDICINE = true;
						break;
					} 
				}
			}
			if (!app_MEDICINE)//原来格子已经被占用了
			{
				int nCount = m_kbAppDataList.size();
				for (int i = 0; i < nCount; i++)
				{
					KEYBOARDDATA &data = m_kbAppDataList[i];
					if(data.uId == 0)
					{
						data.uId = eSPECIAL_HOTKEY_MEDICINE; 
						data.nType = 0;
						app_MEDICINE = true;
						break;
					} 
				}
			} 
		}
		// (2) 游戏设置数据

		GAMESETSDATA2& defaultSetting = GetLogicServer()->GetDataProvider()->GetGameSetting().m_DefaultGameSetting;
		if (reader.getAvaliableLength() == sizeof(GAMESETSDATA2))
		{
			reader.readBuf(&m_GsData2, sizeof(GAMESETSDATA2));
			if (m_GsData2.m_nVersion != GAMESET_VERSION)
			{
				memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
				m_GsData2.m_nVersion = GAMESET_VERSION;
			}
		}
		else if (reader.getAvaliableLength() == 0)
		{
			//memset(&m_GsData2, 0, sizeof(GAMESETSDATA2));
			memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
			m_GsData2.m_nVersion = GAMESET_VERSION;
		}
		else
		{
			memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
			m_GsData2.m_nVersion = GAMESET_VERSION;
			OutputMsg(rmError, _T("[Load GameSet] data len error len=%d ,require len=%d"), reader.getAvaliableLength(), sizeof(GAMESETSDATA2));
		}

		OnDbInitData();
		if (((CActor*)m_pEntity)->IsInited())
		{
			SendAllData();
		}
	}
}

void CGameSetsSystem::LoadFromDB()
{
	if (!m_pEntity)
	{
		return;
	}
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator())
    {
        return;//假人不需要更新游戏设置
    }

	unsigned int nActorId = m_pEntity->GetId();
	CDataClient *pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadOtherGameSets);
		DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		DataPacket << (unsigned int)nActorId; 
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket); 
	}
}

void CGameSetsSystem::OnEnterScene()
{
	// if (((CActor*)m_pEntity)->m_isFirstLogin)
	// {
	// 	GAMESETSDATA2& defaultSetting = GetLogicServer()->GetDataProvider()->GetGameSetting().m_DefaultGameSetting;
	// 	memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
	// } 
    if (((CActor*)m_pEntity)->OnGetIsTestSimulator()
        || ((CActor*)m_pEntity)->OnGetIsSimulator())
    {
        return;//假人不需要发送游戏设置
    }
 
	SendAllData(); 
}

void CGameSetsSystem::SendAllData()
{
	if (!m_pEntity || m_pEntity->GetType() != enActor )
		return;

	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& dp = ((CActor*)m_pEntity)->AllocPacket(AP);
	dp << (BYTE)enGameSetsSystemID << (BYTE)sSendGameSetInfo;

	// 基础设置
	//dp.writeBuf(&m_GsData2.m_nBase, sizeof(m_GsData2.m_nBase));
	dp << (int)m_GsData2.m_nBase;

	// 系统设置
	//dp.writeBuf(&m_GsData2.m_nSystem, sizeof(m_GsData2.m_nSystem));
	dp << (short)m_GsData2.m_nSystem;

	// 药品设置
	//dp.writeBuf(&m_GsData2.m_nMedicine, sizeof(m_GsData2.m_nMedicine));
	dp << (short)m_GsData2.m_nMedicine;
	dp << (BYTE)m_GsData2.m_NmHp;
	dp << (BYTE)m_GsData2.m_NmMp;
	dp << (BYTE)m_GsData2.m_ImHp;
	dp << (BYTE)m_GsData2.m_ImMp;
	dp << (BYTE)m_GsData2.m_PerAdd1;
	dp << (int)m_GsData2.m_NmHpVal;
	dp << (int)m_GsData2.m_NmHpTime;
	dp << (int)m_GsData2.m_NmMpVal;
	dp << (int)m_GsData2.m_NmMpTime;
	dp << (int)m_GsData2.m_ImHpVal;
	dp << (int)m_GsData2.m_ImHpTime;
	dp << (int)m_GsData2.m_ImMpVal;
	dp << (int)m_GsData2.m_ImMpTime;
	dp << (int)m_GsData2.m_ValAdd1;
	dp << (int)m_GsData2.m_TimeAdd1;

	// 保护设置
	dp << (int)m_GsData2.m_nHp1Val;
	dp << (int)m_GsData2.m_nHp1Item;
	dp << (int)m_GsData2.m_nHp2Val;
	dp << (int)m_GsData2.m_nHp2Item;
	dp << (short)m_GsData2.m_nProt;
	//dp.writeBuf(&m_GsData2.m_nProt, sizeof(m_GsData2.m_nProt));
	
	// 挂机设置
	//dp.writeBuf(&m_GsData2.m_nHook, sizeof(m_GsData2.m_nHook));
	dp << (short)m_GsData2.m_nHook;
	dp << (BYTE)m_GsData2.m_nAutoSkillID;
	dp << (BYTE)m_GsData2.m_nPet;
	dp << (BYTE)m_GsData2.m_nHpLess;
	dp << (BYTE)m_GsData2.m_nHpLessSkill;

	// 回收设置
	//dp << (int)m_GsData2.m_nRecycle;
	dp << (int)m_GsData2.m_nRecycle[0];
	dp << (int)m_GsData2.m_nRecycle[1];
	dp << (int)m_GsData2.m_nRecycle[2];
	dp << (int)m_GsData2.m_nRecycle[3];

	// 物品设置
	dp << (BYTE)GAMESETSDATA2::nIntCount;
	dp.writeBuf(&m_GsData2.m_nItems, sizeof(m_GsData2.m_nItems));

	//dp.writeBuf(&m_GsData2, sizeof(m_GsData2));
	AP.flush();
}

void CGameSetsSystem::SaveToDB()
{
	if (!m_pEntity)
	{
		return;
	}
	if (!HasDataModified())
		return;

	CDataClient *pDbClient = GetLogicServer()->GetDbClient();
	if (!pDbClient->connected())
	{
		return;
	}
	unsigned int nActorId = m_pEntity->GetId();
	CDataPacket &DataPacket = pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveOtherGameSets);
	DataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	DataPacket << (unsigned int)nActorId;

	// (1) 按键数据
	DataPacket.writeBuf(&m_GsData, sizeof(GAMESETSDATA));

	// (2) 游戏设置数据
	DataPacket.writeBuf(&m_GsData2, sizeof(GAMESETSDATA2));

	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);

	SetDataModifyFlag(false);
}

void CGameSetsSystem::OnChangeVocation()
{
	m_kbDataList.clear();
	//m_kbAppDataList.clear(); 
 
	// //老号转职
	// for (std::vector<KEYBOARDDATA>::iterator it = m_kbDataList.begin(); it != m_kbDataList.end();) 
	// {
	// 	KEYBOARDDATA &tempData = (*it);
	// 	if (tempData.uId == eSPECIAL_HOTKEY_RECYLE
	// 		|| tempData.uId == eSPECIAL_HOTKEY_RANDOM
	// 		|| tempData.uId == eSPECIAL_HOTKEY_MEDICINE)
	// 	{
	// 		//++it;
	// 	}
	// 	else
	// 	{ 
	// 		tempData.nType = 0;
	// 		tempData.uId = 0; 
	// 		//it = m_kbDataList.erase(it);//返回值是当前被删除元素的下一个元素的迭代器
	// 	} 
	// 	++it;
	// }
	//老号转职
	for (std::vector<KEYBOARDDATA>::iterator it = m_kbAppDataList.begin(); it != m_kbAppDataList.end();) 
	{
		KEYBOARDDATA &tempData = (*it);
		if (tempData.uId == eSPECIAL_HOTKEY_RECYLE
			|| tempData.uId == eSPECIAL_HOTKEY_RANDOM
			|| tempData.uId == eSPECIAL_HOTKEY_MEDICINE)
		{
			//++it;
		}
		else
		{ 
			tempData.nType = 0;
			tempData.uId = 0; 
			//it = m_kbAppDataList.erase(it);//返回值是当前被删除元素的下一个元素的迭代器
		} 
		++it;
	}

	DealKeyBoard();
	
	GAMESETSDATA2& defaultSetting = GetLogicServer()->GetDataProvider()->GetGameSetting().m_DefaultGameSetting;
	memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
}

VOID CGameSetsSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader &packet)
{

	if (m_pEntity == NULL)
		return;
	if (m_pEntity->GetHandle().GetType() != enActor)
		return;
	switch (nCmd)
	{
	case cGetGameSetsInfo: //get
	{
		SendGameSetsData(); 
		break;
	}
	case cGetAppGameSetsInfo: //get
	{
		SendAppKeyBoardsInfo();
		break;
	} 
	case cUpdateKeyBoards: //update
	{
		UpdateGameSetsData(packet);
		break;
	}
	case cUpdateAppKeyBoards: //手机端的消息
	{
		UpdateGameSetsDataApp(packet);
		break;
	}
	case cDiscardKeyBoards: //update
	{
		DiscardGameSetsData(packet);
		break;
	}  
	case cDiscardAppKeyBoards: //手机端的消息
	{
		DiscardGameSetsDataApp(packet);
		break;
	}
	case cSetGameCheck:
	{
		SetGameCheck(packet);
	}
	break;
	case cSetGameValue:
	{
		SetGameValue(packet);
	}
	break;
	case cSetGameReset:
	{
		Reset(packet);
	}
	break;
	case cGameSetOneKeyItem:
	{
		OneKeyItem(packet);
	}
	break;

	default:
		break;
	}
}

void CGameSetsSystem::SetGameCheck(CDataPacketReader &packet)
{
	BYTE nSetType = 0; // 设置类型
	short nPos = 0; // 位置
	BYTE nValue = 0; // 0/1
	packet >> nSetType >> nPos >> nValue;
	nPos = nPos - 1;

	int nByteSize = 0;

	// 检查位置
	if (nPos < 0)
	{
		return;
	}
	OutputMsg(rmTip, _T("SetGameCheck [%s] : type:%d pos:%d value:%d"), m_pEntity->GetEntityName(),nSetType,nPos,nValue);

	switch (nSetType)
	{
	case EN_GAMESET_BASE:
		if (nPos >= sizeof(m_GsData2.m_nBase)*8) return;
		if (nValue) (m_GsData2.m_nBase) |= (1 << nPos); 
		else (m_GsData2.m_nBase) &= ~(1 << nPos);
		break;
	case EN_GAMESET_ITEM:
		{
			if (nPos >= sizeof(m_GsData2.m_nItems)*8) return;

			// 偏移位置
			int nIntOffs = nPos / 32;
			int nBitOffs = nPos % 32;
			int* pStart = m_GsData2.m_nItems + nIntOffs;

			if (nValue) (*pStart) |= (1 << nBitOffs);
			else (*pStart) &= ~(1 << nBitOffs);	
		}
		break;
	case EN_GAMESET_MEDI:
		if (nPos >= sizeof(m_GsData2.m_nMedicine)*8) return;
		if (nValue) (m_GsData2.m_nMedicine) |= (1 << nPos); 
		else (m_GsData2.m_nMedicine) &= ~(1 << nPos);
		break;
	case EN_GAMESET_PROT:
		if (nPos >= sizeof(m_GsData2.m_nProt)*8) return;
		if (nValue) (m_GsData2.m_nProt) |= (1 << nPos); 
		else (m_GsData2.m_nProt) &= ~(1 << nPos);
		break;
	case EN_GAMESET_HOOK:
		if (nPos >= sizeof(m_GsData2.m_nHook)*8) return;
		if (nValue) (m_GsData2.m_nHook) |= (1 << nPos); 
		else (m_GsData2.m_nHook) &= ~(1 << nPos);
		break;
	case EN_GAMESET_SYST:
		if (nPos >= sizeof(m_GsData2.m_nSystem)*8) return;
		if (nValue) (m_GsData2.m_nSystem) |= (1 << nPos); 
		else (m_GsData2.m_nSystem) &= ~(1 << nPos);
		break;
	case EN_GAMESET_RECYLE:
		if (nPos >= sizeof(m_GsData2.m_nRecycle)*8) return;
		int nIdx = nPos/(sizeof(int)*8);
		int nInerPos = nPos- nIdx*(sizeof(int)*8) ;
		if (nValue) (m_GsData2.m_nRecycle[nIdx]) |= (1 << nInerPos); //对应位设为1
		else (m_GsData2.m_nRecycle[nIdx]) &= ~(1 << nInerPos);//对应位设为0 

		// if (nValue) (m_GsData2.m_nRecycle) |= (1 << nPos); //对应位设为1
		// else (m_GsData2.m_nRecycle) &= ~(1 << nPos);//对应位设为0 
		break;
	}
	SetDataModifyFlag(true);
}

void CGameSetsSystem::SetGameValue(CDataPacketReader &packet)
{
	enum GameSetValueType{

		// 药品设置

		MS_NmHp = 1,		// 普通红药百分比
		MS_NmMp = 2,		// 普通蓝药百分比
		MS_ImHp = 3,		// 瞬间红药百分比
		MS_ImMp = 4,		// 瞬间蓝药百分比

		MS_NmHpVal = 5,      // 普通剩余HP
		MS_NmHpTime = 6,     // 普通剩余HP 间隔
		MS_NmMpVal = 7,      // 普通剩余MP
		MS_NmMpTime = 8,     // 普通剩余MP 间隔
		MS_ImHpVal = 9,      // 瞬间剩余HP
		MS_ImHpTime = 10,    // 瞬间剩余HP 间隔
		MS_ImMpVal = 11,     // 瞬间剩余MP
		MS_ImMpTime = 12,    // 瞬间剩余MP 间隔

		// 保护设置

		MS_Hp1Val = 13,         // 血量1低于
		MS_Hp1Item = 14,       // 使用物品1
		MS_Hp2Val = 15,        // 血量2低于
		MS_Hp2Item = 16,       // 使用物品2

		// 挂机设置

		MS_AutoSkillID = 17,   // 自动施放的技能id
		MS_Pet = 18,           // 自动召唤的宠物
		MS_HpLess = 19,        // 血量低于百分比
		MS_HpLessSkill = 20,   // 血量低于，使用技能

		// 补充 药品设置

		MS_PerAdd1 = 21,		// 疗伤药百分比
		MS_ValAdd1 = 22,		// 疗伤药 剩余MP
		MS_TimeAdd1 = 23,		// 疗伤药 剩余MP 间隔

	};

	BYTE nEnum = 0; // 枚举值
	int nValue = 0; // 设定值
	packet >> nEnum >> nValue;

	if (nEnum <= 0) return;

	if (nEnum <= MS_ImMp)
	{
		BYTE* pTarget = (BYTE*)&m_GsData2.m_NmHp + (nEnum - MS_NmHp);
		*pTarget = nValue;
	}
	else if (nEnum <= MS_ImMpTime)
	{
		int* pTarget = (int*)&m_GsData2.m_NmHpVal + (nEnum - MS_NmHpVal);
		*pTarget = nValue;
	}
	else if (nEnum <= MS_Hp2Item)
	{
		int* pTarget = (int*)&m_GsData2.m_nHp1Val + (nEnum - MS_Hp1Val);
		*pTarget = nValue;
	}
	else if (nEnum <= MS_HpLessSkill)
	{
		BYTE* pTarget = (BYTE*)&m_GsData2.m_nAutoSkillID + (nEnum - MS_AutoSkillID);
		*pTarget = nValue;
	}
	else if (nEnum == MS_PerAdd1)
	{
		m_GsData2.m_PerAdd1 = nValue;
	}
	else if (nEnum <= MS_TimeAdd1)
	{
		int* pTarget = (int*)&m_GsData2.m_ValAdd1 + (nEnum - MS_ValAdd1);
		*pTarget = nValue;
	}
	
	SetDataModifyFlag(true);
}

void CGameSetsSystem::SendOneData(BYTE nSetType)
{
	CActor* pActor = (CActor*)m_pEntity;
	CActorPacket AP;
	CDataPacket& dp = ((CActor*)m_pEntity)->AllocPacket(AP);
	dp << (BYTE)enGameSetsSystemID << (BYTE)sSendOneGameSetInfo;
	dp << (BYTE)nSetType;

	switch (nSetType)
	{
	case EN_GAMESET_BASE:
		dp << (int)m_GsData2.m_nBase;
		break;
	case EN_GAMESET_ITEM:
		dp << (BYTE)GAMESETSDATA2::nIntCount;
		dp.writeBuf(&m_GsData2.m_nItems, sizeof(m_GsData2.m_nItems));
		break;
	case EN_GAMESET_MEDI:
		dp << (short)m_GsData2.m_nMedicine;
		dp << (BYTE)m_GsData2.m_NmHp;
		dp << (BYTE)m_GsData2.m_NmMp;
		dp << (BYTE)m_GsData2.m_ImHp;
		dp << (BYTE)m_GsData2.m_ImMp;
		dp << (BYTE)m_GsData2.m_PerAdd1;
		dp << (int)m_GsData2.m_NmHpVal;
		dp << (int)m_GsData2.m_NmHpTime;
		dp << (int)m_GsData2.m_NmMpVal;
		dp << (int)m_GsData2.m_NmMpTime;
		dp << (int)m_GsData2.m_ImHpVal;
		dp << (int)m_GsData2.m_ImHpTime;
		dp << (int)m_GsData2.m_ImMpVal;
		dp << (int)m_GsData2.m_ImMpTime;
		dp << (int)m_GsData2.m_ValAdd1;
		dp << (int)m_GsData2.m_TimeAdd1;
		break;
	case EN_GAMESET_PROT:
		dp << (int)m_GsData2.m_nHp1Val;
		dp << (int)m_GsData2.m_nHp1Item;
		dp << (int)m_GsData2.m_nHp2Val;
		dp << (int)m_GsData2.m_nHp2Item;
		dp << (short)m_GsData2.m_nProt;
		break;
	case EN_GAMESET_HOOK:
		dp << (short)m_GsData2.m_nHook;
		dp << (BYTE)m_GsData2.m_nAutoSkillID;
		dp << (BYTE)m_GsData2.m_nPet;
		dp << (BYTE)m_GsData2.m_nHpLess;
		dp << (BYTE)m_GsData2.m_nHpLessSkill;
		break;
	case EN_GAMESET_SYST:
		dp << (short)m_GsData2.m_nSystem;
		break;
	case EN_GAMESET_RECYLE:
		//dp << (int)m_GsData2.m_nRecycle;
		dp << (int)m_GsData2.m_nRecycle[0];
		dp << (int)m_GsData2.m_nRecycle[1];
		dp << (int)m_GsData2.m_nRecycle[2];
		dp << (int)m_GsData2.m_nRecycle[3];
		break;
	}
	AP.flush();
}

void CGameSetsSystem::Reset(CDataPacketReader &packet)
{
	BYTE nSetType = 0; // 设置类型
	packet >> nSetType;
	GAMESETSDATA2& defaultSetting = GetLogicServer()->GetDataProvider()->GetGameSetting().m_DefaultGameSetting;
	OutputMsg(rmTip, _T("CGameSetsSystem::Reset [%s] : type:%d"), m_pEntity->GetEntityName(),nSetType);

	switch (nSetType)
	{
	case EN_GAMESET_BASE:
		m_GsData2.m_nBase = defaultSetting.m_nBase;
		break;
	case EN_GAMESET_ITEM:
		memcpy(m_GsData2.m_nItems, defaultSetting.m_nItems, sizeof(m_GsData2.m_nItems));
		break;
	case EN_GAMESET_MEDI:
		m_GsData2.m_nMedicine = defaultSetting.m_nMedicine;
		m_GsData2.m_NmHp = defaultSetting.m_NmHp;
		m_GsData2.m_NmMp = defaultSetting.m_NmMp;
		m_GsData2.m_ImHp = defaultSetting.m_ImHp;
		m_GsData2.m_ImMp = defaultSetting.m_ImMp;
		m_GsData2.m_PerAdd1 = defaultSetting.m_PerAdd1;
		m_GsData2.m_NmHpVal = defaultSetting.m_NmHpVal;
		m_GsData2.m_NmHpTime = defaultSetting.m_NmHpTime;
		m_GsData2.m_NmMpVal = defaultSetting.m_NmMpVal;
		m_GsData2.m_NmMpTime = defaultSetting.m_NmMpTime;
		m_GsData2.m_ImHpVal = defaultSetting.m_ImHpVal;
		m_GsData2.m_ImHpTime = defaultSetting.m_ImHpTime;
		m_GsData2.m_ImMpVal = defaultSetting.m_ImMpVal;
		m_GsData2.m_ImMpTime = defaultSetting.m_ImMpTime;
		m_GsData2.m_ValAdd1 = defaultSetting.m_ValAdd1;
		m_GsData2.m_TimeAdd1 = defaultSetting.m_TimeAdd1;
		break;
	case EN_GAMESET_PROT:
		m_GsData2.m_nProt = defaultSetting.m_nProt;
		m_GsData2.m_nHp1Val = defaultSetting.m_nHp1Val;
		m_GsData2.m_nHp1Item = defaultSetting.m_nHp1Item;
		m_GsData2.m_nHp2Val = defaultSetting.m_nHp2Val;
		m_GsData2.m_nHp2Item = defaultSetting.m_nHp2Item;
		break;
	case EN_GAMESET_HOOK:
		m_GsData2.m_nHook = defaultSetting.m_nHook;
		m_GsData2.m_nAutoSkillID = defaultSetting.m_nAutoSkillID;
		m_GsData2.m_nPet = defaultSetting.m_nPet;
		m_GsData2.m_nHpLess = defaultSetting.m_nHpLess;
		m_GsData2.m_nHpLessSkill = defaultSetting.m_nHpLessSkill;
		break;
	case EN_GAMESET_SYST:
		m_GsData2.m_nSystem = defaultSetting.m_nSystem;
		break;
	case EN_GAMESET_RECYLE:
		//m_GsData2.m_nRecycle = defaultSetting.m_nRecycle;
		m_GsData2.m_nRecycle[0] = defaultSetting.m_nRecycle[0];
		m_GsData2.m_nRecycle[1] = defaultSetting.m_nRecycle[1];
		m_GsData2.m_nRecycle[2] = defaultSetting.m_nRecycle[2];
		m_GsData2.m_nRecycle[3] = defaultSetting.m_nRecycle[3];
		break;
	}
	SetDataModifyFlag(true);
	SendOneData(nSetType);
}

void CGameSetsSystem::OneKeyItem(CDataPacketReader &packet)
{
	BYTE nSetType = 0,nVal = 0; // 设置类型
	packet >> nSetType;
	packet >> nVal;

	if (!nSetType)
	{//左边
		if (nVal) m_GsData2.m_nItems[0] |= 0x55555554;	// 0101 0101 0101 0101 0101 0101 0101 0100
		else m_GsData2.m_nItems[0] &= 0xaaaaaaab;		// 1010 1010 1010 1010 1010 1010 1010 1011
	}
	else
	{//右边
		if (nVal) m_GsData2.m_nItems[0] |= 0xaaaaaaa8;	// 1010 1010 1010 1010 1010 1010 1010 1000 
		else m_GsData2.m_nItems[0] &= 0x55555557;		// 0101 0101 0101 0101 0101 0101 0101 0111
	}

	int count = sizeof(m_GsData2.m_nItems)/4;
	for (size_t i = 1; i < count; i++)
	{
		if (!nSetType)
		{//左边
			if (nVal) m_GsData2.m_nItems[i] |= 0x55555555;  // 0101 0101 0101 0101 0101 0101 0101 0101
			else m_GsData2.m_nItems[i] &= 0xaaaaaaaa;		// 1010 1010 1010 1010 1010 1010 1010 1010
		}
		else
		{//右边
			if (nVal) m_GsData2.m_nItems[i] |= 0xaaaaaaaa;	// 1010 1010 1010 1010 1010 1010 1010 1010
			else m_GsData2.m_nItems[i] &= 0x55555555;		// 0101 0101 0101 0101 0101 0101 0101 0101
		}
	}
	
	SetDataModifyFlag(true);
	SendOneData(EN_GAMESET_ITEM);
}

//获取数据
void CGameSetsSystem::SendGameSetsData()
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendGameKeyBoardnfo;
	INT_PTR nCount = m_kbDataList.size();
	DataPacket << (int)nCount;
	for (int i = 0; i < nCount; i++)
	{
		KEYBOARDDATA *tempData = &m_kbDataList[i];
		if (tempData)
		{
			DataPacket << (BYTE)tempData->nType;
			DataPacket << (int)tempData->nPos;
			DataPacket << (int)tempData->uId;
		}
	}
	AP.flush();
}

void CGameSetsSystem::SendAppKeyBoardsInfo()
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendAppGameKeyBoardsInfo;
	INT_PTR nCount = m_kbAppDataList.size();
	DataPacket << (int)nCount;
	for (int i = 0; i < nCount; i++)
	{
		KEYBOARDDATA *tempData = &m_kbAppDataList[i];
		if (tempData)
		{
			int client_pos = tempData->nPos - MAX_BROCASTKEY_SIZE;
			DataPacket << (BYTE)tempData->nType;
			DataPacket << (int)client_pos;
			DataPacket << (int)tempData->uId;
		}
	}
	AP.flush();
}

bool CGameSetsSystem::CheckSameTypeAndId(int pos, int type, int id, int &samepos)
{
	int nCount = m_kbDataList.size();
	for (int i = 0; i < nCount; i++)
	{
		KEYBOARDDATA data = m_kbDataList[i];
		if (pos != data.nPos && type == data.nType && (id == data.uId && id != 0))
		{
			samepos = data.nPos;
		}

		if (pos == data.nPos && type == data.nType && (id == data.uId && id != 0))
		{
			return false;
		}
	}
	return true;
}
bool CGameSetsSystem::CheckSameTypeAndIdApp(int pos, int type, int id, int &samepos)
{
	int nCount = m_kbAppDataList.size();
	for (int i = 0; i < nCount; i++)
	{
		KEYBOARDDATA data = m_kbAppDataList[i];
		if (pos != data.nPos && type == data.nType && (id == data.uId && id != 0))
		{
			samepos = data.nPos;
		}

		if (pos == data.nPos && type == data.nType && (id == data.uId && id != 0))
		{
			return false;
		}
	}
	return true;
}

void CGameSetsSystem::DealKeyBoard()
{
	char result[100];

	memset(m_GsData.m_vKbs, 0, sizeof(m_GsData.m_vKbs) / sizeof(char));

	int nCount = m_kbDataList.size();
	for (int i = 0; i < nCount; i++)
	{
		char buf[10];
		if (i != 0)
			strcat(m_GsData.m_vKbs, "|");

		KEYBOARDDATA data = m_kbDataList[i];
		sprintf(buf, "%d,%d", data.nType, data.uId);
		strcat(m_GsData.m_vKbs, buf);
	}
	//APP小键盘
	nCount = m_kbAppDataList.size();
	for (int i = 0; i < nCount; i++)
	{
		char buf[10];
		//if (i != 0)
			strcat(m_GsData.m_vKbs, "|");

		KEYBOARDDATA data = m_kbAppDataList[i];
		sprintf(buf, "%d,%d", data.nType, data.uId);
		strcat(m_GsData.m_vKbs, buf);
	}
	//OutputMsg(rmTip, _T("DealKeyBoard :%s"), m_GsData.m_vKbs);
}
void CGameSetsSystem::UpdateGameSetsDataApp(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;

	int nCType = 0, nCPos = 0, nCId = 0, lastPos = 0;
	packet >> nCPos >> nCType >> nCId >> lastPos; //读取了这些位置 -- 客户端默认位置从0 开始
 
	int sev_pos = nCPos + MAX_BROCASTKEY_SIZE;
	int sev_lastpos = lastPos + MAX_BROCASTKEY_SIZE;
	DoUpdateGameSetsDataApp(nCType, nCPos, nCId, lastPos, false);
}

void CGameSetsSystem::DoUpdateGameSetsDataApp(int nCType, int nCPos = 0, int nCId = 0, int lastPos, bool isAuto)
{
	CActor *pActor = (CActor *)m_pEntity;
	if(isAuto 
	   && nCId != eSPECIAL_HOTKEY_RECYLE 
	   && nCId != eSPECIAL_HOTKEY_RANDOM)
	{ 
		return;
	}
	if (isAuto 
	   && nCId == eSPECIAL_HOTKEY_RANDOM )
	{
		nCPos = 0;
	}
	
	//int pos = -1
	int sev_lastpos = -1;
	if (nCType == 1) //技能
	{
		CSkillSubSystem::PSKILLDATA pSkillData = m_pEntity->GetSkillSystem().GetSkillInfoPtr(nCId);
		//CSkillProvider *pProvider = m_pEntity->GetSkillSystem()->GetSkillProvider();
		if (!pSkillData)
			return;

		if(lastPos == -1)
		{ 
			sev_lastpos = -1;
		}
		else
		{ 
			//原位 需判断 ------
			bool res = CheckSameTypeAndIdApp(nCPos, nCType, nCId, lastPos);
			
			sev_lastpos = lastPos - MAX_BROCASTKEY_SIZE;
			if (!res)
				return;
		} 
	}
	else
	{
		CUserBag &bag = pActor->GetBagSystem();
		CUserItem *nItemInfo = bag.FindItem(nCId);
		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nCId);
		if ((!nItemInfo || nItemInfo->wPackageType != 0) && (!pStdItem || !pStdItem->m_Flags.canMoveKb))
		{
			return;
		}
		sev_lastpos = lastPos;
	}
	//原位 需判断 ------
	// bool res = CheckSameTypeAndId(nCPos, nCType, nCId);
	// if(!res )
	// 	return;
	if (nCPos == sev_lastpos)
		return;
		
	if (nCPos > 5 || nCPos < 0 )
		return;
	if (sev_lastpos > 5 || sev_lastpos < -1 )
		return; 
	if(nCType != 1 && sev_lastpos == -1)//不是技能的请求不能把技能顶掉(背包过来的)
	{
		if (m_kbAppDataList[sev_lastpos].nType == 1)
		{
			return;//技能不能被顶，不然就找不到了
		} 
	}
	if (sev_lastpos != -1)
	{
		KEYBOARDDATA temp_data = m_kbAppDataList[nCPos];
		m_kbAppDataList[nCPos].nType = nCType;
		m_kbAppDataList[nCPos].uId = nCId;

		m_kbAppDataList[sev_lastpos].nType = temp_data.nType;
		m_kbAppDataList[sev_lastpos].uId = temp_data.uId;
	}
	else
	{
		m_kbAppDataList[nCPos].nType = nCType;
		m_kbAppDataList[nCPos].uId = nCId;
	}

	DealKeyBoard();
	INT_PTR nCount = sev_lastpos != -1 ? 2 : 1;
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendAppKeyBoardsInfo;
	DataPacket << (int)nCount;
	KEYBOARDDATA *tempData = &m_kbAppDataList[nCPos];
	if (tempData)
	{
		int client_pos = tempData->nPos - MAX_BROCASTKEY_SIZE;
		DataPacket << (BYTE)tempData->nType;
		DataPacket << (int)client_pos;
		DataPacket << (int)tempData->uId;
	}
	if (nCount != 1)
	{
		KEYBOARDDATA *tempData = &m_kbAppDataList[sev_lastpos];
		if (tempData)
		{
			int client_pos = tempData->nPos - MAX_BROCASTKEY_SIZE;
			DataPacket << (BYTE)tempData->nType;
			DataPacket << (int)client_pos;
			DataPacket << (int)tempData->uId;
		}
	}
	AP.flush();
	SetDataModifyFlag(true);
}
 
//更新数据
void CGameSetsSystem::UpdateGameSetsData(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;

	int nCType = 0, nCPos = 0, nCId = 0, lastPos = 0;
	packet >> nCPos >> nCType >> nCId >> lastPos; //读取了这些位置 -- 客户端默认位置从0 开始
	DoUpdateGameSetsData(nCType, nCPos, nCId, lastPos);
}

void CGameSetsSystem::DoUpdateGameSetsData(int nCType, int nCPos = 0, int nCId = 0, int lastPos = 0)
{ 
	CActor *pActor = (CActor *)m_pEntity;
	//int pos = -1
	if (nCType == 1) //技能
	{
		CSkillSubSystem::PSKILLDATA pSkillData = m_pEntity->GetSkillSystem().GetSkillInfoPtr(nCId);
		//CSkillProvider *pProvider = m_pEntity->GetSkillSystem()->GetSkillProvider();
		if (!pSkillData)
			return;

		//原位 需判断 ------
		bool res = CheckSameTypeAndId(nCPos, nCType, nCId, lastPos);
		if (!res)
			return;
	}
	else
	{
		CUserBag &bag = pActor->GetBagSystem();
		CUserItem *nItemInfo = bag.FindItem(nCId);
		const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nCId);
		if ((!nItemInfo || nItemInfo->wPackageType != 0) && (!pStdItem || !pStdItem->m_Flags.canMoveKb))
		{
			return;
		}
	}
	//原位 需判断 ------
	// bool res = CheckSameTypeAndId(nCPos, nCType, nCId);
	// if(!res )
	// 	return;
	if (nCPos == lastPos)
		return;
	if (nCPos > 11 || lastPos > 11)
		return;
	if (lastPos != -1)
	{
		KEYBOARDDATA temp_data = m_kbDataList[nCPos];
		m_kbDataList[nCPos].nType = nCType;
		m_kbDataList[nCPos].uId = nCId;

		m_kbDataList[lastPos].nType = temp_data.nType;
		m_kbDataList[lastPos].uId = temp_data.uId;
	}
	else
	{
		m_kbDataList[nCPos].nType = nCType;
		m_kbDataList[nCPos].uId = nCId;
	}

	DealKeyBoard();
	INT_PTR nCount = lastPos != -1 ? 2 : 1;
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendKeyBoardsInfo;
	DataPacket << (int)nCount;
	KEYBOARDDATA *tempData = &m_kbDataList[nCPos];
	if (tempData)
	{
		DataPacket << (BYTE)tempData->nType;
		DataPacket << (int)tempData->nPos;
		DataPacket << (int)tempData->uId;
	}
	if (nCount != 1)
	{
		KEYBOARDDATA *tempData = &m_kbDataList[lastPos];
		if (tempData)
		{
			DataPacket << (BYTE)tempData->nType;
			DataPacket << (int)tempData->nPos;
			DataPacket << (int)tempData->uId;
		}
	}
	AP.flush();
	SetDataModifyFlag(true);
}
 
void CGameSetsSystem::DiscardGameSetsDataApp(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;

	int nCPos = 0;
	packet >> nCPos;
	if (nCPos < 0 || nCPos > 5)//数量限制
		return;

	m_kbAppDataList[nCPos].nType = 0;
	m_kbAppDataList[nCPos].uId = 0;

	DealKeyBoard();
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendAppKeyBoardsInfo;
	DataPacket << (int)1;
	KEYBOARDDATA *tempData = &m_kbAppDataList[nCPos];
	if (tempData)
	{
		int client_pos = tempData->nPos - MAX_BROCASTKEY_SIZE;
		DataPacket << (BYTE)tempData->nType;
		DataPacket << (int)client_pos;
		DataPacket << (int)tempData->uId;
	}
	AP.flush();
	SetDataModifyFlag(true);
} 
void CGameSetsSystem::DiscardGameSetsData(CDataPacketReader &packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	CActor *pActor = (CActor *)m_pEntity;

	int nCPos = 0;
	packet >> nCPos;
	if (nCPos < 0 || nCPos > 11)
		return;

	m_kbDataList[nCPos].nType = 0;
	m_kbDataList[nCPos].uId = 0;

	DealKeyBoard();
	CActorPacket AP;
	CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendKeyBoardsInfo;
	DataPacket << (int)1;
	KEYBOARDDATA *tempData = &m_kbDataList[nCPos];
	if (tempData)
	{
		DataPacket << (BYTE)tempData->nType;
		DataPacket << (int)tempData->nPos;
		DataPacket << (int)tempData->uId;
	}
	AP.flush();
	SetDataModifyFlag(true);
} 
void CGameSetsSystem::AutoGameSetSkill(int nCType, int nCId)
{
	CActor *pActor = (CActor *)m_pEntity;
	//int pos = -1
	bool bFlag = false;
	for(int i = 0; i < MAX_BROCASTKEY_SIZE; i++)
	{
		if(m_kbDataList[i].nType == 0 && m_kbDataList[i].uId == 0)
		{
			m_kbDataList[i].nType = nCType;
			m_kbDataList[i].uId = nCId;
			DealKeyBoard();
			SetDataModifyFlag(true);
			CActorPacket AP;
			CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
			DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendKeyBoardsInfo;
			DataPacket << (int)1;
			DataPacket << (BYTE)m_kbDataList[i].nType;
			int client_pos = m_kbAppDataList[i].nPos - MAX_BROCASTKEY_SIZE;
			DataPacket << (int)client_pos;
			DataPacket << (int)m_kbDataList[i].uId;
			AP.flush();
			break;
		}
	}
	// for(int i = 0; i < MAX_BROCASTKEYAPP_SIZE; i++)
	// {
	// 	if(m_kbAppDataList[i].nType == 0 && m_kbAppDataList[i].uId == 0)
	// 	{
	// 		m_kbAppDataList[i].nType = nCType;
	// 		m_kbAppDataList[i].uId = nCId;
	// 		DealKeyBoard();
	// 		SetDataModifyFlag(true);
	// 		CActorPacket AP;
	// 		CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
	// 		DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendAppKeyBoardsInfo;
	// 		DataPacket << (int)1;
	// 		DataPacket << (BYTE)m_kbAppDataList[i].nType;
			
	// 		int client_pos = m_kbAppDataList[i].nPos - MAX_BROCASTKEY_SIZE;
	// 		DataPacket << (int)client_pos;
	// 		DataPacket << (int)m_kbAppDataList[i].uId;
	// 		AP.flush();
	// 		return;
	// 	}
	// }
	SetDataModifyFlag(true);
}
void CGameSetsSystem::AutoGameSetSkillApp(int nCType, int nCId)
{
	CActor *pActor = (CActor *)m_pEntity;
	//int pos = -1
	bool bFlag = false;
	for(int i = 0; i < MAX_BROCASTKEYAPP_SIZE; i++)
	{
		if(m_kbAppDataList[i].nType == 0 && m_kbAppDataList[i].uId == 0)
		{
			m_kbAppDataList[i].nType = nCType;
			m_kbAppDataList[i].uId = nCId;
			DealKeyBoard();
			SetDataModifyFlag(true);
			CActorPacket AP;
			CDataPacket &DataPacket = ((CActor *)m_pEntity)->AllocPacket(AP);
			DataPacket << (BYTE)enGameSetsSystemID << (BYTE)sSendAppKeyBoardsInfo;
			DataPacket << (int)1;
			DataPacket << (BYTE)m_kbAppDataList[i].nType;
			
			int client_pos = m_kbAppDataList[i].nPos - MAX_BROCASTKEY_SIZE;
			DataPacket << (int)client_pos;
			DataPacket << (int)m_kbAppDataList[i].uId;
			AP.flush();
			return;
		}
	}
}

//跨服数据
void CGameSetsSystem::SendMsg2CrossServer(int nType) {
	if(!m_pEntity) return;
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	packet << m_pEntity->GetCrossActorId();
	packet << m_pEntity->GetAccountID();
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "), nType, nActorID);
	//数据
	packet.writeBuf(&m_GsData, sizeof(GAMESETSDATA));

	// (2) 游戏设置数据
	packet.writeBuf(&m_GsData2, sizeof(GAMESETSDATA2));

	pCrossClient->flushProtoPacket(packet); 
} 

VOID CGameSetsSystem::OnCrossInitData(GAMESETSDATA& GsData, GAMESETSDATA2& GsData2)
{
	if (m_pEntity == NULL)
	{
		OutputMsg(rmError, _T("LoadNewTitle Data is Error..."));
		return;
	}
	{
		//需要判断是否重复装载玩家的数据，避免出现装载复制
		if (HasDbDataInit())
		{
			OutputMsg(rmError, _T("db return NewTitle repeated,actorid=%u "), m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
			return;
		}

		// (1) 按键数据
		memcpy(&m_GsData, &GsData,sizeof(GAMESETSDATA));
		// reader.readBuf(&m_GsData, sizeof(GAMESETSDATA));
		std::string sKb = m_GsData.m_vKbs;
		std::vector<std::string> results = SplitStr(sKb, "|");
		int nCount = results.size();
		m_kbDataList.clear();
		m_kbAppDataList.clear();
		
		int n_max = MAX_BROCASTKEY_SIZE + MAX_BROCASTKEYAPP_SIZE;
		for (int i = 0; i < n_max; i++)
		//for (int i = 0; i < 12; i++)
		{
			if(i < MAX_BROCASTKEY_SIZE)
			{
				KEYBOARDDATA kbData;
				kbData.nPos = i;
				if (i >= nCount)//没有存，走默认配置
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbDataList.push_back(kbData);
					continue;
				}
				
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() < 2)
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbDataList.push_back(kbData);
					continue;
				}
				  
				kbData.nType = atoi(temp_res[0].c_str());
				int id = atoi(temp_res[1].c_str());
				if (kbData.nType == 1)
				{
					const OneSkillData *pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(id);
					if (pSkillConf == NULL ||(pSkillConf && pSkillConf->boIsDelete))
						id = 0;
				}
				else
				{
					const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(id);
					if (pStdItem == NULL)
					{
						id = 0;
					}
				}

				kbData.uId = id; 
				m_kbDataList.push_back(kbData);
			}
			

			if(i >= MAX_BROCASTKEY_SIZE && i < n_max)
			{ 
				KEYBOARDDATA kbData;
				kbData.nPos = i;
				if (i >= nCount)//没有存，走默认配置
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbAppDataList.push_back(kbData);
					continue;
				}
				
				std::vector<std::string> temp_res = SplitStr(results[i], ",");
				if (temp_res.size() < 2)
				{
					kbData.nType = 0;
					kbData.uId = 0;
					m_kbAppDataList.push_back(kbData);
					continue;
				}
				  
				kbData.nType = atoi(temp_res[0].c_str());
				int id = atoi(temp_res[1].c_str());
				if (kbData.nType == 1)
				{
					const OneSkillData *pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(id);
					if (pSkillConf == NULL ||(pSkillConf && pSkillConf->boIsDelete))
						id = 0;
				}
				else
				{
					const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(id);
					if (pStdItem == NULL)
					{
						id = 0;
					}
				}

				kbData.uId = id;
				m_kbAppDataList.push_back(kbData);
			}
		} 
		// (2) 游戏设置数据
		memcpy(&m_GsData2, &GsData2, sizeof(GAMESETSDATA2));
		// GAMESETSDATA2& defaultSetting = GetLogicServer()->GetDataProvider()->GetGameSetting().m_DefaultGameSetting;
		// if (reader.getAvaliableLength() == sizeof(GAMESETSDATA2))
		// {
		// 	reader.readBuf(&m_GsData2, sizeof(GAMESETSDATA2));
		// 	if (m_GsData2.m_nVersion != GAMESET_VERSION)
		// 	{
		// 		memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
		// 		m_GsData2.m_nVersion = GAMESET_VERSION;
		// 	}
		// }
		// else if (reader.getAvaliableLength() == 0)
		// {
		// 	//memset(&m_GsData2, 0, sizeof(GAMESETSDATA2));
		// 	memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
		// 	m_GsData2.m_nVersion = GAMESET_VERSION;
		// }
		// else
		// {
		// 	memcpy(&m_GsData2, &defaultSetting, sizeof(GAMESETSDATA2));
		// 	m_GsData2.m_nVersion = GAMESET_VERSION;
		// 	OutputMsg(rmError, _T("[Load GameSet] data len error len=%d ,require len=%d"), reader.getAvaliableLength(), sizeof(GAMESETSDATA2));
		// }

		OnDbInitData();
	}
	((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CGAMESET); //完成一个步骤
}