#include "StdAfx.h"
#include "../base/Container.hpp"
#define MARRYBLESSNUM  10

//////////////////////////////////////////////////////////////

//为某个玩家添加离线信息
COfflineUser * COfflineUserMgr::AddOffLineUser(unsigned int nActorId,bool &isExist)
{
	COfflineUser * pUser = GetUserByActorId(nActorId);
	if(pUser ==NULL) //如果找不到这个玩家
	{
		isExist =false;
		pUser = m_FixedSizeAllocator.allocObject();
			
		if(pUser)
		{
			new (pUser)COfflineUser();
			pUser->nActorId = nActorId;
			m_actoridMap.insert( std::make_pair(nActorId, pUser) );
		}
		return pUser;
	}
	else
	{
		isExist =true;
		return pUser;
	}
}

//移除所有离线玩家信息（TODO：优化，添加一个freelist存储之）
void COfflineUserMgr::RemoveAllUser()
{
	CLogicEngine *logic = GetLogicServer()->GetLogicEngine();
	OfflineUserMap::iterator iter = m_actoridMap.begin(),itEnd = m_actoridMap.end();
	while(iter !=itEnd )
	{
		COfflineUser * pUser = iter->second;
		if(pUser)
		{
			//释放物品的内存
			for(INT_PTR i=0; i< CEquipVessel::EquipmentCount; i++)
			{
				CUserItem *pItem = pUser->m_Equips[i];
				if(pItem)
				{
					logic->DestroyUserItem(pItem); //销毁
					pUser->m_Equips[i]= NULL;
				}
			}

			//释放其他内存
			pUser->m_Skilldata.clear();
			pUser->m_nStrengths.clear();
			pUser->m_NewTitleDataList.clear();
			pUser->m_Almirah.clear();

			//析构
			pUser->~COfflineUser();

			m_FixedSizeAllocator.freeObject(pUser);
		}
		iter ++;
	}
	m_actoridMap.clear(); //清掉
	
}

//装载玩家的基本信息
void COfflineUserMgr::OnLoadActorBasic(unsigned int nActorId, CDataPacketReader &reader)
{
	ActorOfflineMsg msg;
	if (reader.getAvaliableLength() < sizeof(ActorOfflineMsg))
	{
		auto iter = m_noticeMap.find(nActorId);
		if (iter != m_noticeMap.end())
		{
			m_noticeMap.erase(iter);
		}
		auto iter_2 = m_noticeCenterMap.find(nActorId);
		if (iter_2 != m_noticeCenterMap.end())
		{
			m_noticeCenterMap.erase(iter_2);
		}
	}
	
	reader >> msg;
	COfflineUser * pUser= GetUserByActorId(nActorId);
	if(pUser ==NULL)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, not exist"),__FUNCTION__,nActorId);
		return;
	}
	memcpy(pUser,&msg,sizeof(msg));
	pUser->nActorId = nActorId;
	pUser->name[sizeof(pUser->name) -1] =0; //防止数据出错
	pUser->sGuildName[sizeof(pUser->sGuildName) -1] =0; //防止数据出错
}

//装载玩家的其他信息
void COfflineUserMgr::OnLoadActorOther(unsigned int nActorId, CDataPacketReader &reader)
{
	COfflineUser * pUser= GetUserByActorId(nActorId);
	if(pUser ==NULL)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, not exist"),__FUNCTION__,nActorId);
		return;
	}

	CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();

	// 加载装备数据
	int nCount =0;
	{
		CUserItem userItem;
		reader >> nCount; //装备的数目
		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem << reader;
			if (CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem))
			{
				INT_PTR nPos =-1;
				const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUerItem->wItemId);
				if ( !pStdItem )
				{

					OutputMsg(rmError,_T("actorID=%d的玩家item=%d,装备非法"),nActorId,pUerItem->wItemId);
					pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
					continue;
				}
				else
				{
					nPos = CEquipVessel::GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType,pUerItem->btHandPos);
					
					if(nPos <0)
					{
						OutputMsg(rmError,_T("actorID=%d的玩家nPos=%d,location=%d,itemId=%d,guid=%lld,装备位置非法"),nActorId,nPos,pUerItem->btHandPos,pUerItem->wItemId,pUerItem->series);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
						continue;
					}
				
					if(nPos >=0 && pUser->m_Equips[nPos] ==NULL)
					{
						pUser->m_Equips[nPos] = pUerItem;
					}
					else
					{
						OutputMsg(rmError,_T("actorID=%u的玩家nPos=%d已经有装备了"),nActorId,nPos);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
					}
				}
			}
		}
	}

	// 加载技能数据
	{
		CSkillSubSystem::SKILLDATA  data;
		reader >> nCount;
		CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
		for(INT_PTR i=0; i < nCount; i++)
		{
			reader.readBuf(&data,sizeof(ONESKILLDBDATA) );
			data.bEvent =0;
			data.bMjMask =0;
			const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
			if(pConfig ==NULL) continue;
			
			data.bIsPassive = ((pConfig->nSkillType == stPassiveSkill)?1:0);
			data.bPriority = pConfig->bPriority;
			data.nSkillType = pConfig->nSkillType;

			pUser->m_Skilldata.push_back(data);
		}
	}
	
	// 加载强化信息
	{
		reader >> nCount;
		for(INT_PTR i=0; i < nCount; i++)
		{
            int nType = 0; 
            StrengthInfo it;
            reader >> nType >>it.nId >> it.nLv;
            if(it.nId > 0)
                pUser->m_nStrengths[nType].push_back(it);
		}
	}

	// 加载称号信息
	{
		reader >> nCount;
		for(INT_PTR i=0; i < nCount; i++)
		{
			unsigned int nValue = 0,  nTime = 0;
			reader >> nValue >> nTime;
			WORD wId = LOWORD(nValue);

			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetNetTitleConfig(wId);
			if ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide )
			{
				continue;
			}

			CNewTitleSystem::NEWTITLEDATA newTitleData;
			newTitleData.wId = wId;
			newTitleData.nRemainTime = nTime;
			pUser->m_NewTitleDataList.push_back(newTitleData);
		}

		reader >> nCount;
		for(INT_PTR i=0; i < nCount; i++)
		{
			unsigned int nValue = 0,  nTime = 0;
			reader >> nValue >> nTime;
			WORD wId = LOWORD(nValue);

			OneNewTitleConfig* pConfig = GetLogicServer()->GetDataProvider()->GetNewTitlesConfig().GetCustomTitleConfig(wId);
			if ( GetLogicServer()->IsCrossServer() && pConfig && pConfig->bCrossServerHide )
			{
				continue;
			}

			CNewTitleSystem::NEWTITLEDATA newTitleData;
			newTitleData.wId = wId;
			newTitleData.nRemainTime = nTime;
			pUser->m_CustomTitleDataList.push_back(newTitleData);
		}
	}

	// 加载时装信息
	{
		reader >> nCount;
		CAlmirahItem oneItem;
		for(INT_PTR i=0; i < nCount; i++)
		{
			reader >> oneItem.nModelId;		
			reader >> oneItem.nLv;
			reader >> oneItem.btTakeOn;
			pUser->m_Almirah.push_back(oneItem);
		}
	}
	// 加载神魔
	{
		reader >> nCount;
		GhostData oneItem;
		for(INT_PTR i=0; i < nCount; i++)
		{
			reader >> oneItem.nId;		
			reader >> oneItem.nLv;
			reader >> oneItem.nBless;
			pUser->m_ghost.push_back(oneItem);
		}
	}

	// 加载圣物兵魂
	{
		reader >> nCount;

		for(INT_PTR i = 0; i < nCount; i++)
		{  
			SoulWeaponNetData oneItem;
			oneItem.nId = i + 1;	
			reader >> oneItem.nLorderId >> oneItem.nStarId >> oneItem.nLvId; 
			reader.readString(oneItem.cBestAttr); 
			oneItem.cBestAttr[sizeof(oneItem.cBestAttr)-1] = 0;
			pUser->vSoulWeapon.push_back(oneItem);
		}
	}

	// 加载宠物系统非 ai宠物
	{
		reader >> nCount;
		for(INT_PTR i=0; i < nCount; i++)
		{
			int nType = 0, nId = 0, nTime = 0;
			reader >> nType >> nId >> nTime;

			WORD wType = LOWORD(nType);
			WORD wId = LOWORD(nId);

			CLootPetSystem::LOOTPETDATA lootData;
			lootData.wId = wId;
			lootData.nType = wType;
			lootData.nContinueTime = nTime;
			pUser->m_LootPetSystemDataList.push_back(lootData);
		}
	}

	// 通知玩家 
	auto it = m_noticeMap.find(nActorId);
	if (it != m_noticeMap.end())
	{
		auto& vec = it->second;
		for (size_t i = 0; i < vec.size(); i++)
		{
			auto func = vec[i];
			func(pUser);
		}
		m_noticeMap.erase(it);
	}
	auto it_2 = m_noticeCenterMap.find(nActorId);
	if (it_2 != m_noticeCenterMap.end())
	{
		auto& vec = it_2->second;
		for (size_t i = 0; i < vec.size(); i++)
		{
			auto func = vec[i];
			func(pUser);
		}
		m_noticeCenterMap.erase(it_2);
	}

}

//加载某个离线玩家信息
void COfflineUserMgr::LoadUser(unsigned nActorId, unsigned nViewerId, int nsys,int ncmd)
{
	if(nActorId ==0) return;

	//如果这个玩家已经装载了，那么不再装载
	bool isExist;
	//如果已经存在这样的一个用户的话就不需要装载了
	AddOffLineUser(nActorId,isExist);
	if (nViewerId) m_noticeMap[nActorId].push_back(std::bind(&COfflineUserMgr::viewByOfflineUser, this, std::placeholders::_1, nViewerId, nsys,ncmd));
	if(nsys < 0) m_noticeCenterMap[nActorId].push_back(std::bind(&COfflineUserMgr::viewByOfflineUser, this, std::placeholders::_1, nViewerId, nsys,ncmd));
	if(isExist) return;
	//向DB服务器发数据数据包
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadOfflineUserInfo);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		DataPacket << (unsigned int) nActorId; //去装载玩家的离线消息
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

//移除某个离线玩家信息（TODO：优化，添加一个freelist存储之）
void COfflineUserMgr::RemoveUser(unsigned int nActorID)
{
	OfflineUserMap::iterator iter = m_actoridMap.find(nActorID);
	if (iter == m_actoridMap.end())
	{
		return;
	}

	COfflineUser *pUser = iter->second;
	if (pUser)
	{
		CLogicEngine *logic = GetLogicServer()->GetLogicEngine();

		//释放物品的内存
		for(INT_PTR i=0; i< CEquipVessel::EquipmentCount; i++)
		{
			CUserItem *pItem = pUser->m_Equips[i];
			if(pItem)
			{
				logic->DestroyUserItem(pItem); //销毁
				pUser->m_Equips[i]= NULL;
			}
		}
	
		//释放其他内存
		pUser->m_Skilldata.clear();
		pUser->m_nStrengths.clear();
		pUser->m_NewTitleDataList.clear();
		pUser->m_Almirah.clear();

		//析构
		pUser->~COfflineUser();

		m_FixedSizeAllocator.freeObject(pUser);
	}

	m_actoridMap.erase(iter);
	
}

//通过名字获取离线玩家信息
COfflineUser * COfflineUserMgr::GetUserByName(const char * name)
{
	if (!name) return NULL;		

	OfflineUserMap::iterator iter = m_actoridMap.begin(),itEnd = m_actoridMap.end();
	while(iter != itEnd )
	{
		COfflineUser * pUser = iter->second;
		if(pUser)
		{
			if(strcmp(pUser->name,name) ==0)
			{
				return pUser;
			}
		}
		iter ++ ;
	}
	return NULL;

}

void COfflineUserMgr::viewByOfflineUser(COfflineUser *pUser, unsigned int nActorId, int nsys,int ncmd)
{
	if(nsys >= 0) {
		CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
		if (!pActor) return;

		CActorPacket pack;
		CDataPacket & data = pActor->AllocPacket(pack);
		data << (BYTE) nsys << (BYTE) ncmd;
		// 基本数据
		viewOfflineUserData(pUser, data);
		pack.flush();
	}
	else
	{
		SendCenterOfflineActor(pUser, ncmd);
	}
	
	
}

void COfflineUserMgr::viewOfflineUserData(COfflineUser *pUser, CDataPacket& data)
{
	if(!pUser) return;
	data << (unsigned int)pUser->nActorId;
	data.writeString(pUser->name);
	data << (BYTE)pUser->nVocation;
	data << (BYTE)pUser->nCircle;
	data << (unsigned int)pUser->nLevel;
	data << (BYTE)pUser->nSex;

	std::string strUserName = pUser->name;
	
	// 装备数据
	INT_PTR nEquipCount = 0;
	INT_PTR nPosBegin = data.getPosition();
	data << (BYTE)nEquipCount;
	for(INT_PTR i=0; i<itMaxEquipPos; i++)
	{
		if (CUserItem* pUserItem = pUser->m_Equips[i])
		{
			data << (BYTE)i;
			*pUserItem >> data;
			nEquipCount++;
		}
	}
	INT_PTR nPosEnd = data.getPosition();
	data.setPosition(nPosBegin);
	data << (BYTE)nEquipCount;
	data.setPosition(nPosEnd);

	// 技能数据
	INT_PTR nSkillCount = 0;
	nPosBegin = data.getPosition();
	data << (BYTE)pUser->m_Skilldata.size();
	for (INT_PTR i = 0 ; i < pUser->m_Skilldata.size(); i++)
	{
		CSkillSubSystem::SKILLDATA& skillData = pUser->m_Skilldata[i];
		const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(skillData.nSkillID);
		if (pSkill->boIsDelete) continue;
		data << (WORD)skillData.nSkillID << (BYTE)skillData.nLevel;
		data << (BYTE)(!skillData.nIsClosed);//技能是否激活可用
		nSkillCount++;
	}
	nPosEnd = data.getPosition();
	data.setPosition(nPosBegin);
	data << (BYTE)nSkillCount;
	data.setPosition(nPosEnd);

	// 属性数据
	INT_PTR nSize = sizeof(CActorOfflineProperty);
	data << (WORD)nSize;
	data.writeBuf(&pUser->actordata,nSize);

	// 工会名
	data.writeString(pUser->sGuildName);

	// 强化信息
	typedef std::map<int, std::vector<StrengthInfo> >::iterator strongMapIter;
	data <<(BYTE)(pUser->m_nStrengths.size());
	for(strongMapIter it = pUser->m_nStrengths.begin(); it != pUser->m_nStrengths.end(); it++)
	{
		std::vector<StrengthInfo>& v_list = it->second;
		data <<(BYTE)(it->first);
		data <<(BYTE)(v_list.size());
		for(int i = 0; i < v_list.size(); i++)
		{
			StrengthInfo& info = v_list[i];
			data<<(BYTE)(info.nId);
			data<<(int)(info.nLv);
		}
	}

	// 称号信息
	INT_PTR nDataCount = pUser->m_NewTitleDataList.size();
	data << int(nDataCount);
	for (int i=0; i<nDataCount;i++)
	{
		CNewTitleSystem::NEWTITLEDATA& newTitleData = pUser->m_NewTitleDataList[i];
		data << (unsigned int)newTitleData.wId;
		int nLastTime = newTitleData.nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
		nLastTime = nLastTime > 0 ? nLastTime : 0;
		data << (unsigned int)nLastTime;
	}
	
	// 自定义称号信息
	INT_PTR nDataCount2 = pUser->m_CustomTitleDataList.size();
	data << int(nDataCount2);
	for (int i=0; i < nDataCount2;i++)
	{
		CNewTitleSystem::NEWTITLEDATA& newTitleData = pUser->m_CustomTitleDataList[i];
		data << (unsigned int)newTitleData.wId;
		int nLastTime = newTitleData.nRemainTime - GetGlobalLogicEngine()->getMiniDateTime();
		nLastTime = nLastTime > 0 ? nLastTime : 0;
		data << (unsigned int)nLastTime;
	}

	// 时装信息
	nDataCount = (byte)pUser->m_Almirah.size();
	data << (byte)nDataCount;
	for (INT_PTR i = 0; i < nDataCount; i++)
	{
		CAlmirahItem & oneItem = pUser->m_Almirah[i];
		data << (int)oneItem.nModelId;
		data << (BYTE)oneItem.btTakeOn;
		data << (int)oneItem.nLv;
	}

	nDataCount = (byte)pUser->m_ghost.size();
	data << (byte)nDataCount;
	for (INT_PTR i = 0; i < nDataCount; i++)
	{
		GhostData & oneItem = pUser->m_ghost[i];
		data << (BYTE)oneItem.nId;
		data << (int)oneItem.nLv;
		data << (int)oneItem.nBless;
	}
	
	nDataCount = (BYTE)pUser->vSoulWeapon.size();
	data << (BYTE)nDataCount;
	for (INT_PTR i = 0; i < nDataCount; i++)
	{
		SoulWeaponNetData & oneItem = pUser->vSoulWeapon[i];
		data << (BYTE)oneItem.nId;
		data << (int)oneItem.nLorderId;
		data << (int)oneItem.nStarId;
		data << (int)oneItem.nLvId;
		data.writeString(oneItem.cBestAttr); 
	} 

	// 宠物系统非 ai宠物信息
	nDataCount = pUser->m_LootPetSystemDataList.size();
	data << int(nDataCount);
	for (int i=0; i<nDataCount;i++)
	{
		CLootPetSystem::LOOTPETDATA& lootData = pUser->m_LootPetSystemDataList[i];
		data << (WORD)lootData.wId;
		data << (WORD)lootData.nType;
		data << (int)lootData.nContinueTime;
	}
}

void COfflineUserMgr::viewOnlineUserData(CActor* pUser, CDataPacket& data)
{
	if(!pUser) 
		return;
	data << (unsigned int)pUser->GetId();
	data.writeString(pUser->GetEntityName());
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	data << (unsigned int)pUser->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_SEX);
	
	CUserEquipment &pEquip = pUser->GetEquipmentSystem();
	INT_PTR nEquipCount= pEquip.GetEquipCount();
	data << (BYTE)nEquipCount;
	for(INT_PTR i=0; i< itMaxEquipPos ;i ++)
	{
		CUserItem *pUserItem = (CUserItem *)((pEquip.m_Items[i]));
		if (pUserItem != NULL)
		{
			data <<(BYTE)i;
			(*pUserItem)>>data;
		}
	}

	CSkillSubSystem &pSkill = pUser->GetSkillSystem();
	const CVector<CSkillSubSystem::SKILLDATA> & skills= pSkill.GetSkills();
	INT_PTR nCountOffer = data.getPosition();
	data << (BYTE) skills.count();
	int nCount = 0;

	for (INT_PTR i = 0 ; i < skills.count(); i++)
	{
		CSkillSubSystem::SKILLDATA& skillData = skills[i];
		TICKCOUNT nLeftTick = skillData.tick  - GetGlobalLogicEngine()->getTickCount() ;
		if(nLeftTick <0) nLeftTick = 0;

		const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(skillData.nSkillID);
		if (pSkill->boIsDelete) continue;
		
		data << (WORD)skillData.nSkillID << (BYTE)skillData.nLevel;
		BYTE nActiveState = 0;//技能是否激活可用
		nActiveState = !skillData.nIsClosed;
		data << (BYTE)nActiveState;
		nCount++;
	}
	INT_PTR nFinishOffer = data.getPosition();
	data.setPosition(nCountOffer);
	data << (BYTE)nCount;
	data.setPosition(nFinishOffer);

	INT_PTR nSize = pUser->GetPropertySize();
	data << (WORD)nSize;
	data.writeBuf(pUser->GetPropertyDataPtr(),nSize);

	CGuild* pMyGuild =pUser->GetGuildSystem()->GetGuildPtr();
	if(NULL == pMyGuild)
	{
		data.writeString("");
	}
	else
	{
		data.writeString(pMyGuild->m_sGuildname);	//
	}
	pUser->GetStrengthenSystem().returnStrongInfo(data);
	pUser->GetNewTitleSystem().SendClientTitleInfo(data);
	pUser->GetNewTitleSystem().SendClientCustomTitleInfo(data);
	pUser->GetAlmirahSystem().returnInfo(data);
	pUser->GetGhostSystem().Data2Packet(data);
	pUser->GetHallowsSystem().OnReturnHallowsSoulWeaponInfo(data);

	// 加载宠物系统非 ai宠物
	pUser->GetLootPetSystem().OnReturnLootPetInfo(data);
}

void COfflineUserMgr::viewByOnlineUser(CActor *pUser, CActor *pActor,int nsys, int ncmd)
{
	if(nsys >= 0 ) {
		CActorPacket pack;
		CDataPacket &data = pActor->AllocPacket(pack);
		data << (BYTE)(nsys) << (BYTE) (ncmd);
		viewOnlineUserData(pUser, data);
		pack.flush();
	}
	else
	{
		SendCenterOnlineUser(pUser, ncmd);
	}
}

//查看离线玩家的资料
void COfflineUserMgr::ViewOffLineActor(CActor *pActor, unsigned int nActorId, int nsys,int ncmd)
{
	if (pActor == NULL || nActorId == 0) return;

	// 先从在线玩家找
	if (CActor* pUser = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId))
	{
		viewByOnlineUser(pUser, pActor,nsys,ncmd);
	}
	// 再从离线玩家找
	else if(COfflineUser *pUser = GetUserByActorId(nActorId))
	{
		viewByOfflineUser(pUser, pActor->GetId(), nsys, ncmd);
	}
	else
	{
		// 最后只能从数据库找
		LoadUser(nActorId, pActor->GetId(), nsys, ncmd);	
	}
	
	return;
}

//获取离线玩家名字
LPCSTR COfflineUserMgr::GetUserName(unsigned int nActorId)
{
	//OutputMsg(rmNormal, "COfflineUserMgr::GetUserName, nActorId=%d", nActorId);
	COfflineUser *pUser = GetUserByActorId(nActorId);
	if( pUser )
	{
		return pUser->name;
	}
	else
	{
		return "";
	}
}

//////////////////////////////////////////////////////////////

void COfflineUserMgr::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	//查询角色列表出错
	int nRawServerId = 0, nLoginServerId = 0;
	CDataPacketReader reader(data,size);
	unsigned int nActorID=0;
	BYTE nErrorCode = 0;
	if(size < sizeof(int)*2 + sizeof(nActorID) + sizeof(nErrorCode)) 
		return ; //数据格式错误

	reader >> nRawServerId >> nLoginServerId;
	reader >> nActorID;
	reader >> nErrorCode;

	// 查询数据不成功
	if (nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		if (nErrorCode == jxInterSrvComm::DbServerProto::reNoHero)
		{
			BYTE nHeroId = 0;
			reader >> nHeroId;
			OutputMsg(rmError,"%s Load HeroOffline Data Error!!ActorID = %d,nHeroId=%d,errorID=%d",__FUNCTION__,nActorID,nHeroId,nErrorCode);
		}
		else
			OutputMsg(rmError,"%s Load UserOffline Data Error!!ActorID = %d,errorID=%d",__FUNCTION__,nActorID,nErrorCode);
		
		return;
	}

	switch(nCmd)
	{
		case  jxInterSrvComm::DbServerProto::dcLoadOfflineUserBasic:
			OnLoadActorBasic(nActorID,reader);
			break;
		case  jxInterSrvComm::DbServerProto::dcLoadOfflineUserOther:
			OnLoadActorOther(nActorID,reader);
			break;
		case jxInterSrvComm::DbServerProto::dcLoadOfflineUserHeroBasic:
			OnLoadActorHeroBasic(nActorID, reader);
			break;
		case jxInterSrvComm::DbServerProto::dcLoadOfflineUserHeroEquips:
			OnLoadActorHeroEquips(nActorID,reader);
			break;
	}

}

/////////////////////////////////////////////////////////////

COfflineUserHero * COfflineUserMgr::AddOffLineUserHeros(unsigned int nActorId, unsigned int nHeroId, bool &isExist)
{
	bool bExist = false;
	COfflineUserHero *pHero = GetUserHero(nActorId, nHeroId, bExist);
	if (!bExist) //如果找不到这个英雄
	{
		//Heros *pHeros = new CVector<COfflineUserHero *>();
		OfflineUserHeroMap *pHeros = new OfflineUserHeroMap();
		if (pHeros)
		{
			isExist = true;
			m_actoridHerosMap.insert(std::make_pair(nActorId, pHeros));
		}

		return NULL;		
	}
	else
	{
		isExist = true;
		return pHero;
	}
}

COfflineUserHero * COfflineUserMgr::AddOffLineUserHero(unsigned int nActorId, unsigned int nHeroId)
{
	if(nActorId == 0 || nHeroId == 0) return NULL;
	OfflineUserHerosMap::iterator iter = m_actoridHerosMap.find(nActorId);
	if (iter != m_actoridHerosMap.end())
	{
		OfflineUserHeroMap *pHeros = iter->second;
		if (pHeros)
		{
			OfflineUserHeroMap::iterator it = pHeros->find((char)nHeroId);
			if (it != pHeros->end())
			{
				return it->second;
			}

			COfflineUserHero *pHero = m_OfflineUserHeroAllocator.allocObject();
			if (pHero)
			{
				new (pHero)COfflineUserHero();
				pHeros->insert(std::make_pair((char)nHeroId, pHero));
			}

			return pHero;
		}
		
	}

	return NULL;
}

void COfflineUserMgr::RemoveAllUserHero()
{
	CLogicEngine *logic = GetLogicServer()->GetLogicEngine();
	OfflineUserHerosMap::iterator iter = m_actoridHerosMap.begin(),itEnd = m_actoridHerosMap.end();
	while (iter != itEnd)
	{
		OfflineUserHeroMap *pHeros = iter->second;
		if (pHeros)
		{
			OfflineUserHeroMap::iterator it1 = pHeros->begin(),it2 = pHeros->end();
			while (it1 != it2)
			{
				COfflineUserHero *pHero = it1->second;
				if (pHero)
				{
					//释放物品的内存
					for (INT_PTR i=0; i< CEquipVessel::EquipmentCount; i++)
					{
						CUserItem *pItem = pHero->equips.m_Items[i];
						if(pItem)
						{
							logic->DestroyUserItem(pItem); //销毁
							pHero->equips.m_Items[i]= NULL;
						}
					}
					m_OfflineUserHeroAllocator.freeObject(pHero);
				}

				it1 ++;
			}

			pHeros->clear();
			SafeDelete(pHeros);
		}

		iter ++;
	}

	m_actoridHerosMap.clear(); //清掉

}

//装载玩家英雄的基本信息
void COfflineUserMgr::OnLoadActorHeroBasic(unsigned int nActorId, CDataPacketReader &reader)
{
	BYTE btHeroId = 0;
	reader >> btHeroId;
	HeroOfflineMsg msg;
	reader >> msg;
	bool bExist = false;
	COfflineUserHero * pHero = GetUserHero(nActorId, btHeroId, bExist);
	if (!bExist)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, heroid=%d not exist"),__FUNCTION__,nActorId, btHeroId);
		return;
	}
	
	if (pHero == NULL)
		return;
	memcpy(pHero, &msg, sizeof(msg));
}

//装载玩家英雄的装备信息
void COfflineUserMgr::OnLoadActorHeroEquips(unsigned int nActorId, CDataPacketReader &reader)
{
	BYTE btHeroId = 0;
	reader >> btHeroId;

	bool bExist = false;
	COfflineUserHero * pHero = GetUserHero(nActorId, btHeroId, bExist);
	if (!bExist)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, heroid=%d not exist"),__FUNCTION__,nActorId, btHeroId);
		return;
	}

	if (pHero == NULL)
		return;	

	int nCount =0;
	reader >> nCount; //装备的数目
	CUserItem userItem;

	CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
	for (INT_PTR i=0 ; i < nCount; i++ )
	{
		// INT_PTR nReadSize = reader.readBuf(&userItem,sizeof(CUserItem));

		// if( sizeof(CUserItem) !=  nReadSize) //读取内存
		// {
		// 	OutputMsg(rmError,"[%s] read net io error,size=%d ",__FUNCTION__,(int) nReadSize);\
		// 		continue;
		// }
		userItem << reader;

		CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	

		bool flag =true;
		if (pUerItem)
		{

			INT_PTR nPos =-1;
			const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUerItem->wItemId);

			if ( !pStdItem || ((nPos = CUserEquipment::GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType, pUerItem->btHandPos)) < 0) )
			{
				OutputMsg(rmError,_T("%s,actorID=%d玩家的HeroId=%d英雄nPos=%d装备位置非法"),__FUNCTION__,nActorId,btHeroId,nPos);
				pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
				pUerItem = NULL;
				continue;
			}
			else
			{
				if (nPos < CEquipVessel::EquipmentCount)
				{
					if(nPos >=0 && pHero->equips.m_Items[nPos] ==NULL)
					{
						pHero->equips.m_Items[nPos] = pUerItem;
					}
					else
					{
						OutputMsg(rmError,_T("%s,actorID=%d的玩家HeroId=%d英雄nPos=%d已经有装备了"), __FUNCTION__,nActorId,btHeroId,nPos);
						flag =false;
					}
				}
			}

			if(!flag)
			{
				pEngineer->DestroyUserItem(pUerItem);
				pUerItem = NULL;
			}
		}
		
	}

	int nSwingLevel = 0;
	int nFashion = 0, nFoot = 0;
	int nHeraldryId = 0;
	CActor *pActor = GetLogicServer()->GetLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	//获取玩家的武器，外观，用于查询
	CUserEquipment::GetAppear(&pHero->equips,pHero->bVocation,pHero->bSex,pHero->nModelId,pHero->nWeaponAppear, 
								pHero->nWingAppear,nSwingLevel,nHeraldryId,enHero,pActor);
}

void COfflineUserMgr::LoadUserHero(unsigned nActorId, unsigned int nHeroId)
{
	if ((nActorId == 0) || (nHeroId == 0)) return;

	//如果这个玩家已经装载了，那么不再装载
	bool bExist=false;
	//如果已经存在这样的一个用户的话就不需要装载了
	COfflineUserHero *pHero = GetUserHero(nActorId,nHeroId,bExist,false);
	if (bExist && pHero)
	{
		return;
	}

	if (!bExist)
	{
		AddOffLineUserHeros(nActorId, nHeroId,bExist);
	}

	if (pHero == NULL)
	{
		pHero = AddOffLineUserHero(nActorId, nHeroId);
	}
	
	if (!bExist || !pHero)
	{
		OutputMsg(rmError, "LoadUserHero error! ");
		return;
	}

	//向DB服务器发数据数据包
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadOfflineUserHeroInfo);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		DataPacket << (unsigned int) nActorId << (BYTE)nHeroId; //去装载英雄的离线消息
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}

}

void COfflineUserMgr::RemoveUserHero(unsigned int nHeroId, OfflineUserHeroMap *pHeros)
{
	OfflineUserHeroMap::iterator iter = pHeros->find((char)nHeroId);
	if (iter == pHeros->end())
	{
		return;
	}

	COfflineUserHero *pHero = iter->second;
	if (pHero)
	{
		CLogicEngine *logic = GetLogicServer()->GetLogicEngine();
		//释放物品的内存
		for(INT_PTR i=0; i< CEquipVessel::EquipmentCount; i++)
		{
			CUserItem *pItem = pHero->equips.m_Items[i];
			if(pItem)
			{
				logic->DestroyUserItem(pItem); //销毁
				pHero->equips.m_Items[i]= NULL;
			}
		}
		m_OfflineUserHeroAllocator.freeObject(pHero);
	}

	pHeros->erase(iter);

}

//查看离线玩家英雄的资料
void COfflineUserMgr::ViewOffLineActorHero(CActor *pActor, unsigned int nActorId, unsigned int nHeroId, bool loadWhenNotExist, unsigned int nShowType)
{
	COfflineUserHero *pHero = NULL;
	//如果有玩家的actorid，则优先使用actorid查找，速度更快
	if (nActorId == 0 || nHeroId == 0)
		return;

	bool bExist = false;
	pHero= GetUserHero(nActorId, nHeroId, bExist, loadWhenNotExist); //如果不存在就去装载 

	if (pHero ==NULL)
	{
		pActor->SendOldTipmsgWithId(tpNotHeroMsg);  //这里基本不会执行的，所以提示一下对方不在线了
		return;
	}

	INT_PTR nEquipCount = pHero->equips.GetEquipCount(); //获取装备数目
	CActorPacket pack;
	CDataPacket & data = pActor->AllocPacket(pack);
	data << (BYTE) enEuipSystemID << (BYTE)sViewOfflineUserHero ;
	
	data << (BYTE)pHero->bVocation;
	data << (BYTE)pHero->bCircle;
	data << (BYTE)pHero->bLevel;
	data << (BYTE)pHero->bSex;
	data << (unsigned int)pHero->nScore;
	data << (unsigned int)pHero->nModelId;
	data << (unsigned int)pHero->nWeaponAppear;
	data << (unsigned int)pHero->nWingAppear;
	
	data << (BYTE) nEquipCount;
	for(INT_PTR i=0; i< CEquipVessel::EquipmentCount ;i ++)
	{
		if(pHero->equips.m_Items[i])
		{
			data << *(pHero->equips.m_Items[i]);
		}
	}

	data << (BYTE)nShowType;
	/*
	bool hasGen = pUser->gemData.m_nUID !=0; //是否有宝物

	data << hasGen ;

	if(hasGen)
	{
		data.writeBuf( &pUser->gemData, sizeof(GemPropData) );
	}
	*/
	pack.flush();
}


//查看离线玩家的资料
void COfflineUserMgr::ViewOffLineActorToPacket(unsigned int nActorId, CDataPacket& data)
{
	// 先从在线玩家找
	if (CActor* pUser = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId))
	{
		data <<(BYTE)1;
		viewOnlineUserData(pUser, data);
	}
	// 再从离线玩家找
	else if(COfflineUser *pUser = GetUserByActorId(nActorId))
	{
		data <<(BYTE)1;
		viewOfflineUserData(pUser, data);
	}
	else
	{
		data <<(BYTE)0;
	}
	return;
}


//离线同步玩家的其他信息 ---离线需要同步
void COfflineUserMgr::OnOfflineSynchronizationData(CActor* pActor)
{
	if(!pActor || !pActor->IsInited()) return;
	unsigned int nActorId = pActor->GetId();
	bool isExist;
	//如果已经存在这样的一个用户的话就不需要装载了
	COfflineUser * pUser= AddOffLineUser(nActorId,isExist);
	if(isExist || pUser == NULL)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, not exist"),__FUNCTION__,nActorId);
		return;
	}
	memcpy(pUser->name, pActor->GetEntityName(), sizeof(pUser->name));
	memcpy(pUser->sGuildName,pActor->GetGuildName(), sizeof(pUser->sGuildName));
	pUser->name[sizeof(pUser->name) -1] =0; //防止数据出错
	pUser->sGuildName[sizeof(pUser->sGuildName) -1] =0; //防止数据出错
	pUser->nActorId = nActorId;
	pUser->nLevel = pActor->GetLevel();
	pUser->nSex = pActor->GetSex();
	pUser->nVocation = pActor->GetJob();
	pUser->nCircle = pActor->GetCircleLevel();
	INT_PTR nSize = pActor->GetPropertySize();
	// pUser->actordata = std::move(pActor->GetPropertySystem());
	memcpy(&pUser->actordata,pActor->GetPropertyPtr(), sizeof(pUser->actordata));

	CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();

	// 加载装备数据
	{
		CUserEquipment& equip = pActor->GetEquipmentSystem();
		for (INT_PTR i=0 ; i < itMaxEquipPos; i++ )
		{
			if(!equip.m_Items[i]) continue;
			if (CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,equip.m_Items[i]))
			{
				INT_PTR nPos =-1;
				const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUerItem->wItemId);
				if ( !pStdItem )
				{

					OutputMsg(rmError,_T("actorID=%d的玩家item=%d,装备非法"),nActorId,pUerItem->wItemId);
					pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
					continue;
				}
				else
				{
					nPos = CEquipVessel::GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType,pUerItem->btHandPos);
					
					if(nPos <0)
					{
						OutputMsg(rmError,_T("actorID=%d的玩家nPos=%d,location=%d,itemId=%d,guid=%lld,装备位置非法"),nActorId,nPos,pUerItem->btHandPos,pUerItem->wItemId,pUerItem->series);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
						continue;
					}
				
					if(nPos >=0 && pUser->m_Equips[nPos] ==NULL)
					{
						pUser->m_Equips[nPos] = pUerItem;
					}
					else
					{
						OutputMsg(rmError,_T("actorID=%u的玩家nPos=%d已经有装备了"),nActorId,nPos);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
					}
				}
			}
		}
	}

	// 加载技能数据
	{
		CSkillSubSystem & skillSystem = ((CAnimal*)pActor)->GetSkillSystem();
		auto& skillLists = skillSystem.GetSkills();
		for(INT_PTR i=0; i < skillLists.count(); i++)
		{
			auto& it = skillLists[i];
			CSkillSubSystem::SKILLDATA  data;
			memcpy(&data, &it, sizeof(data));
			pUser->m_Skilldata.push_back(data);
		}
	}

	// 加载称号信息
	{
		CNewTitleSystem & titlesys = pActor->GetNewTitleSystem();
		auto& lists = titlesys.getNewTitleInfos();
		CNewTitleSystem::NEWTITLEDATA data;
		for(INT_PTR i=0; i < lists.count(); i++)
		{
			CNewTitleSystem::NEWTITLEDATA& newTitleData = lists[i];
			data.wId = newTitleData.wId;
			data.nRemainTime = newTitleData.nRemainTime;
			pUser->m_NewTitleDataList.push_back(data);
		}

		auto& lists2 = titlesys.getCustomTitleInfos();
		
		for(INT_PTR i=0; i < lists2.count(); i++)
		{
			CNewTitleSystem::NEWTITLEDATA data2;
			CNewTitleSystem::NEWTITLEDATA& newTitleData = lists2[i];
			data2.wId = newTitleData.wId;
			data2.nRemainTime = newTitleData.nRemainTime;
			pUser->m_CustomTitleDataList.push_back(data2);
		}
	}
	
	// 加载强化信息
	{
		CStrengthenSystem & strengthSys = pActor->GetStrengthenSystem();
		auto lists = strengthSys.GetStrongInfos();
		pUser->m_nStrengths = lists;
	}

	

	// 加载时装信息
	{
		CAlmirahSystem & almirahsys = pActor->GetAlmirahSystem();
		auto lists = almirahsys.getAlmirahItemLists();
		CAlmirahItem oneItem;
		for(INT_PTR i=0; i < lists.count(); i++)
		{
			oneItem = lists[i];
			pUser->m_Almirah.push_back(oneItem);
		}
	}
	// 加载神魔
	{
		CGhostSystem & ghostsys = pActor->GetGhostSystem();
		auto lists = ghostsys.getGhostLists();
		for(auto& it : lists)
		{
			pUser->m_ghost.push_back(it.second);
		}
	}
}
//跨服相关

//查看离线玩家的资料
void COfflineUserMgr::ViewCenterOffLineActor(int nSrvId, unsigned int nActorId)
{
	if ( nActorId == 0) return;

	// 先从在线玩家找
	if (CActor* pUser = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId))
	{
		viewByOnlineUser(pUser, NULL,-1, nSrvId);
	}
	// 再从离线玩家找
	else if(COfflineUser *pUser = GetUserByActorId(nActorId))
	{
		viewByOfflineUser(pUser,0, -1, nSrvId);
	}
	else
	{
		// 最后只能从数据库找
		LoadUser(nActorId,0,-1,nSrvId);	
	}
	
	return;
}


void COfflineUserMgr::SendCenterOfflineActor(COfflineUser* pUser,int nSrvId)
{
	char buff[4096];
	CDataPacket outPack(buff, sizeof(buff));
	outPack <<(unsigned int)nSrvId;
	viewOfflineUserData(pUser, outPack);
	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sGetActorOfflineData, outPack.getMemoryPtr(), outPack.getPosition());
}
void COfflineUserMgr::SendCenterOnlineUser(CActor* pUser,int nSrvId)
{
	char buff[4096];
	CDataPacket outPack(buff, sizeof(buff));
	outPack <<(unsigned int)nSrvId;
	viewCenterOnlineUserData(pUser, outPack);
	GetLogicServer()->GetCrossClient()->SendRawServerData(jxInterSrvComm::CrossServerProto::sGetActorOfflineData, outPack.getMemoryPtr(), outPack.getPosition());
}

void COfflineUserMgr::viewCenterOnlineUserData(CActor* pUser, CDataPacket& data)
{
	if(!pUser) 
		return;
	data << (unsigned int)pUser->GetId();
	data.writeString(pUser->GetEntityName());
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_VOCATION);
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE);
	data << (unsigned int)pUser->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	data << (BYTE)pUser->GetProperty<unsigned int>(PROP_ACTOR_SEX);
	
	CUserEquipment &pEquip = pUser->GetEquipmentSystem();
	INT_PTR nEquipCount= pEquip.GetEquipCount();
	data << (BYTE)nEquipCount;
	for(INT_PTR i=0; i< itMaxEquipPos ;i ++)
	{
		CUserItem *pUserItem = (CUserItem *)((pEquip.m_Items[i]));
		if (pUserItem != NULL)
		{
			data <<(BYTE)i;
			(*pUserItem)>>data;
		}
	}

	CSkillSubSystem &pSkill = pUser->GetSkillSystem();
	const CVector<CSkillSubSystem::SKILLDATA> & skills= pSkill.GetSkills();
	INT_PTR nCountOffer = data.getPosition();
	data << (BYTE) skills.count();
	int nCount = 0;

	for (INT_PTR i = 0 ; i < skills.count(); i++)
	{
		CSkillSubSystem::SKILLDATA& skillData = skills[i];
		TICKCOUNT nLeftTick = skillData.tick  - GetGlobalLogicEngine()->getTickCount() ;
		if(nLeftTick <0) nLeftTick = 0;

		const OneSkillData * pSkill =GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(skillData.nSkillID);
		if (pSkill->boIsDelete) continue;
		
		data << (WORD)skillData.nSkillID << (BYTE)skillData.nLevel;
		BYTE nActiveState = 0;//技能是否激活可用
		nActiveState = !skillData.nIsClosed;
		data << (BYTE)nActiveState;
		nCount++;
	}
	INT_PTR nFinishOffer = data.getPosition();
	data.setPosition(nCountOffer);
	data << (BYTE)nCount;
	data.setPosition(nFinishOffer);

	INT_PTR nSize = pUser->GetPropertySize();
	data << (WORD)nSize;
	data.writeBuf(pUser->GetPropertyDataPtr(),nSize);

	CGuild* pMyGuild =pUser->GetGuildSystem()->GetGuildPtr();
	if(NULL == pMyGuild)
	{
		data.writeString("");
	}
	else
	{
		data.writeString(pMyGuild->m_sGuildname);	//
	}
	pUser->GetStrengthenSystem().returnStrongInfo(data);
	pUser->GetNewTitleSystem().SendClientTitleInfo(data);
	pUser->GetAlmirahSystem().returnInfo(data);
	pUser->GetGhostSystem().Data2Packet(data);
	pUser->GetHallowsSystem().OnReturnHallowsSoulWeaponInfo(data);
}