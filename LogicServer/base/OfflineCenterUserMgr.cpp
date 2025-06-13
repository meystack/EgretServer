#include "StdAfx.h"
#include "../base/Container.hpp"
#define MARRYBLESSNUM  10

//////////////////////////////////////////////////////////////

//为某个玩家添加离线信息
COfflineCenterUser * COfflineCenterUserMgr::AddCenterOffLineUser(unsigned int nActorId,bool &isExist)
{
	COfflineCenterUser * pUser = GetUserByActorId(nActorId);
	if(pUser ==NULL) //如果找不到这个玩家
	{
		isExist =false;
		pUser = m_FixedSizeAllocator.allocObject();
			
		if(pUser)
		{
			new (pUser)COfflineCenterUser();
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
void COfflineCenterUserMgr::RemoveAllUser()
{
	CLogicEngine *logic = GetLogicServer()->GetLogicEngine();
	OfflineUserMap::iterator iter = m_actoridMap.begin(),itEnd = m_actoridMap.end();
	while(iter !=itEnd )
	{
		COfflineCenterUser * pUser = iter->second;
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
			pUser->~COfflineCenterUser();

			m_FixedSizeAllocator.freeObject(pUser);
		}
		iter ++;
	}
	m_actoridMap.clear(); //清掉
	
}


//装载玩家的其他信息
void COfflineCenterUserMgr::OnLoadCenterActorData(unsigned int nActorId, CDataPacketReader &reader)
{
	ActorOfflineMsg msg;
	if (reader.getAvaliableLength() < sizeof(ActorOfflineMsg))
	{
		auto iter = m_noticeMap.find(nActorId);
		if (iter != m_noticeMap.end())
		{
			m_noticeMap.erase(iter);
		}
	}
	
	reader >> msg;
	COfflineCenterUser * pUser= GetUserByActorId(nActorId);
	if(pUser ==NULL)
	{
		OutputMsg(rmError,_T("%s,actorid=%d, not exist"),__FUNCTION__,nActorId);
		return;
	}
	memcpy(pUser,&msg,sizeof(msg));

	pUser->name[sizeof(pUser->name) -1] =0; //防止数据出错
	pUser->sGuildName[sizeof(pUser->sGuildName) -1] =0; //防止数据出错

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
			CNewTitleSystem::NEWTITLEDATA newTitleData;
			newTitleData.wId = wId;
			newTitleData.nRemainTime = nTime;
			pUser->m_NewTitleDataList.push_back(newTitleData);
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
}

//加载某个离线玩家信息
void COfflineCenterUserMgr::LoadCenterUser(unsigned nActorId, unsigned nViewerId, int nsys,int ncmd)
{
	if(nActorId ==0) return;

	//如果这个玩家已经装载了，那么不再装载
	bool isExist;
	//如果已经存在这样的一个用户的话就不需要装载了
	// AddCenterOffLineUser(nActorId,isExist);
	if (nViewerId) m_noticeMap[nActorId].push_back(std::bind(&COfflineCenterUserMgr::viewCenterByOfflineUser, this, std::placeholders::_1, nViewerId, nsys,ncmd));
	// if(isExist) return;
	//向Center服务器发数据数据包
	CLocalCrossClient* pCSClient = GetLogicServer()->GetCrossClient();
	if (pCSClient->connected())
	{
		CDataPacket& DataPacket = pCSClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cGetActorOfflineData);
		int nServerId = GetLogicServer()->GetCrossServerId();
        DataPacket << nServerId;
		DataPacket << (unsigned int) nActorId; //去装载跨服玩家的离线数据
		GetLogicServer()->GetCrossClient()->flushProtoPacket(DataPacket);
	}
}

//移除某个离线玩家信息（TODO：优化，添加一个freelist存储之）
void COfflineCenterUserMgr::RemoveUser(unsigned int nActorID)
{
	OfflineUserMap::iterator iter = m_actoridMap.find(nActorID);
	if (iter == m_actoridMap.end())
	{
		return;
	}

	COfflineCenterUser *pUser = iter->second;
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
		pUser->~COfflineCenterUser();

		m_FixedSizeAllocator.freeObject(pUser);
	}
	m_actoridMap.erase(iter);
	
}

void COfflineCenterUserMgr::viewCenterByOfflineUser(COfflineCenterUser *pUser, unsigned int nActorId, int nsys,int ncmd)
{
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if (!pActor) return;

	CActorPacket pack;
	CDataPacket & data = pActor->AllocPacket(pack);
	data << (BYTE) nsys << (BYTE) ncmd;
 
	// 基本数据
	viewCenterOfflineUserData(pUser, data);

	pack.flush();
}

void COfflineCenterUserMgr::viewCenterOfflineUserData(COfflineCenterUser *pUser, CDataPacket& data)
{
	if(!pUser) return;
	data << (unsigned int)pUser->nActorId;
	data.writeString(pUser->name);
	data << (BYTE)pUser->nVocation;
	data << (BYTE)pUser->nCircle;
	data << (unsigned int)pUser->nLevel;
	data << (BYTE)pUser->nSex;

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
} 
//查看离线玩家的资料
void COfflineCenterUserMgr::ViewCenterOffLineActor(CActor *pActor, unsigned int nActorId, int nsys,int ncmd)
{
	if (pActor == NULL || nActorId == 0) return;

	// 再从离线玩家找
	if(COfflineCenterUser *pUser = GetUserByActorId(nActorId))
	{
		viewCenterByOfflineUser(pUser, pActor->GetId(), nsys, ncmd);
	}
	else
	{
		// 最后只能从数据库找
		LoadCenterUser(nActorId, pActor->GetId(), nsys, ncmd);	
	}
	
	return;
}
//////////////////////////////////////////////////////////////

void COfflineCenterUserMgr::OnCenterReturnData(CDataPacketReader& reader)
{
	// unsigned int nActorID=0;
	// if(size < sizeof(int)*2 + sizeof(nActorID) + sizeof(nErrorCode)) 
	// 	return ; //数据格式错误

	// reader >> nActorID;
	// // 查询数据不成功
	// OnLoadCenterActorData(nActorID,reader);
}



void COfflineCenterUserMgr::SetCenterOfflineUserData(CDataPacketReader& data)
{
	unsigned int nActorId = 0;

	data >> nActorId;
	bool isExist;
	//如果已经存在这样的一个用户的话就不需要装载了
	COfflineCenterUser *pUser = AddCenterOffLineUser(nActorId,isExist);
	if(pUser) { 
		data.readString(pUser->name, sizeof(pUser->name));
		BYTE nVocation = 0;
		BYTE nCircle = 0;
		unsigned int nLevel = 0;
		BYTE nSex = 0;
		data >> nVocation;
		data >> nCircle;
		data >> nLevel;
		data >> nSex;
		pUser->nVocation = nVocation;
		pUser->nCircle = nCircle;
		pUser->nLevel = nLevel;
		pUser->nSex = nSex;

		// 装备数据
		BYTE nEquipCount = 0;
		data >> nEquipCount;
		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
		for(INT_PTR i=0; i<nEquipCount; i++)
		{
			BYTE k = 0;
			data  >> k;
			userItem << data;
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
		pUser->m_Skilldata.clear();
		// 技能数据
		BYTE nSkillCount = 0;
		data >> nSkillCount;
		CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
		for (INT_PTR i = 0 ; i < nSkillCount; i++)
		{
			WORD nSkillID = 0;
			BYTE nLevel = 0;
			BYTE nIsClosed = 0;
			data >> nSkillID >> nLevel>> nIsClosed;
			CSkillSubSystem::SKILLDATA  data;
			data.nSkillID = nSkillID;
			data.nLevel = nLevel;
			data.nIsClosed = nIsClosed;
			data.bEvent =0;
			data.bMjMask =0;
			const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
			if(pConfig ==NULL) continue;
			
			data.bIsPassive = ((pConfig->nSkillType == stPassiveSkill)?1:0);
			data.bPriority = pConfig->bPriority;
			data.nSkillType = pConfig->nSkillType;

			pUser->m_Skilldata.push_back(data);
		}

		// 属性数据
		WORD nSize = sizeof(CActorOfflineProperty);
		data >> nSize;
		data.readBuf(&pUser->actordata,nSize);

		// 工会名
		data.readString(pUser->sGuildName,sizeof(pUser->sGuildName));

		// 强化信息
		// 加载强化信息
		{
			pUser->m_nStrengths.clear();
			BYTE nCount = 0;
			data >> nCount;
			int nLv = 0; 
			BYTE nid = 0;
			for(INT_PTR i=0; i < nCount; i++)
			{
				BYTE nType = 0; 
				BYTE nNum = 0;
				data >> nType >> nNum;
				for(int j = 0; j < nNum; j++) {
					
					data >> nid >> nLv;
					StrengthInfo it;
					it.nId = nid;
					it.nLv = nLv;
					if(it.nId > 0)
						pUser->m_nStrengths[nType].push_back(it);
				}	
			}
		}

			// 加载称号信息
		{
			pUser->m_NewTitleDataList.clear();
			int nCount = 0;
			data >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				unsigned int wId = 0,  nTime = 0;
				data >> wId >> nTime;
				CNewTitleSystem::NEWTITLEDATA newTitleData;
				newTitleData.wId = wId;
				newTitleData.nRemainTime = nTime;
				pUser->m_NewTitleDataList.push_back(newTitleData);
			}
		}

		// 加载时装信息
		{
			pUser->m_Almirah.clear();
			BYTE nCount = 0;
			data >> nCount;
			
			for(INT_PTR i=0; i < nCount; i++)
			{
				int nModelId = 0,  nLv = 0;
				BYTE btTakeOn = 0;
				data >> nModelId >> btTakeOn >> nLv;
				CAlmirahItem oneItem;
				oneItem.nModelId = nModelId;
				oneItem.nLv = nLv;
				oneItem.btTakeOn = btTakeOn;
				pUser->m_Almirah.push_back(oneItem);
			}
		}
		// 加载神魔
		{
			pUser->m_ghost.clear();
			BYTE nCount = 0;
			data >> nCount;
			for(INT_PTR i=0; i < nCount; i++)
			{
				BYTE nId = 0; 
				int nLv = 0, nBless = 0;
				data >> nId >> nLv >> nBless;
				GhostData oneItem;
				 oneItem.nId = nId;		
				oneItem.nLv = nLv;
				oneItem.nBless = nBless;
				pUser->m_ghost.push_back(oneItem);
			}
		}

		// 加载圣物兵魂
		{
			pUser->vSoulWeapon.clear();
			BYTE nCount = 0;
			data >> nCount;

			for(INT_PTR i = 0; i < nCount; i++)
			{  
				BYTE nId = 0;
				int nLorderId = 0, nStarId = 0,nLvId = 0;
				SoulWeaponNetData oneItem;
				data >> nId >> nLorderId >> nStarId >> nLvId; 
				oneItem.nId = nId;
				oneItem.nLorderId = nLorderId;
				oneItem.nStarId = nStarId;
				oneItem.nLvId = nLvId;
				data.readString(oneItem.cBestAttr, sizeof(oneItem.cBestAttr)); 
				oneItem.cBestAttr[sizeof(oneItem.cBestAttr)-1] = 0;
				pUser->vSoulWeapon.push_back(oneItem);
			}
		}
	}
} 