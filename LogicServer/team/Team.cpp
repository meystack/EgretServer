#pragma once
#include "StdAfx.h"
#include "Team.h"


bool CTeam::DelMemberBuf(CActor * pActor)
{
	if(pActor ==NULL)
		return false;
	int nBuffId = GetTeamKillMonsterExpBuffId(pActor);
	if (nBuffId > 0)
	{
		pActor->GetBuffSystem()->RemoveById(nBuffId);
	}
	
	// 删除头衔buff
	for (unsigned int i = 0; i < m_nBuffCount; ++i)
	{
		pActor->GetBuffSystem()->RemoveById(m_BuffId[i]);	
	}
	return true;
}
int CTeam::GetTeamKillMonsterExpBuffId(CActor* pActor)
{
	if(pActor ==NULL)
		return false;
	for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	{
		int nBuffId = GetLogicServer()->GetDataProvider()->GetTeamKillMonsterExp().GetTeamKillMonsterExpBufId(i+1);
		CDynamicBuff * pBuff = pActor->GetBuffSystem()->GetBuffById(nBuffId);
		if(pBuff)
		{
			return pBuff->GetId();
		}
	}
	return 0;
}
bool CTeam::UpdateBufMember()
{
	bool bNeedUpdate = false;
	INT_PTR nOnlineCount = GetOnlineUserCount();
	if(nOnlineCount < 1)
		return false;
	for(INT_PTR j=0; j< MAX_TEAM_MEMBER_COUNT; j++)
	{
		if(m_member[j].pActor &&  m_member[j].pActor->IsInited())  //在线玩家
		{
			CActor * pActor = m_member[j].pActor;
			int nTeamMemberCount = 0;
			for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
			{
				if(m_member[i].pActor &&  m_member[i].pActor->IsInited())  
				{
					unsigned int nActorId = m_member[i].pActor->GetProperty<unsigned int>(PROP_ENTITY_ID);
					if(pActor->GetId() != nActorId)
					{
						//是否超出了经验共享范围
						int nDis = GetLogicServer()->GetDataProvider()->GetGlobalConfig().teamLootMaxDistanceSquare;
						if(pActor->GetEntityDistanceSquare(m_member[i].pActor) <= nDis)
						{
							nTeamMemberCount++;
						}
					}
				}
			}
			if(nTeamMemberCount == 0)
			{
				//DelMemberBuf(pActor);
			}
			else
			{
				int nBuffId = GetLogicServer()->GetDataProvider()->GetTeamKillMonsterExp().GetTeamKillMonsterExpBufId(nTeamMemberCount);
				if(pActor->GetBuffSystem()->GetBuffById(nBuffId) == NULL)
				{
					bNeedUpdate = true;
					pActor->GetBuffSystem()->Append(nBuffId);
					//OutputMsg(rmWaning,"%s append id = %d",pActor->GetEntityName(), nBuffId);
				}
 
			}
		}
	}

	// std::map<int, bool> newBuffIdMap;
	// int oldBuffId[MAX_TEAM_BUFF];
	// memcpy(oldBuffId, m_BuffId, sizeof(oldBuffId));
	// //获取其他组队共享buff
	// for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; i++)
	// {
	// 	CActor* pActor = m_member[i].pActor;
	// 	if(pActor && pActor->IsInited() )
	// 	{
	// 		pActor->GetNewTitleSystem().GetTeamBuffIds(newBuffIdMap);
	// 		break;
	// 	}
	// }	
	// m_nBuffCount = 0;
	// for (std::map<int, bool>::iterator it = newBuffIdMap.begin(); m_nBuffCount < MAX_TEAM_BUFF - 1 && it != newBuffIdMap.end(); ++it)
	// {
	// 	m_BuffId[m_nBuffCount] = it->first;
	// 	if (m_BuffId[m_nBuffCount] != oldBuffId[m_nBuffCount])
	// 	{
	// 		bNeedUpdate = true;
	// 	}
	// 	m_nBuffCount++;
	// }
	// if (bNeedUpdate)
	// {
	// 	for(INT_PTR i=0 ;i < MAX_TEAM_MEMBER_COUNT; i++)
	// 	{
	// 		CActor* pActor = m_member[i].pActor;
	// 		if(pActor && pActor->IsInited() )
	// 		{
	// 			for (unsigned int i = 0; i < m_nBuffCount; ++i)
	// 			{
	// 				if (oldBuffId[i] != m_BuffId[i])
	// 				{
	// 					pActor->GetBuffSystem()->RemoveById(oldBuffId[i]);
	// 					if(!pActor->GetBuffSystem()->GetBuffById(m_BuffId[i]))
	// 					{
	// 						pActor->GetBuffSystem()->Append(m_BuffId[i]);
	// 					}
	// 				}
	// 			}
	// 			char buff[64];
	// 			CDataPacket data(buff,sizeof(buff));
	// 			data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsSendBuffIds ;
	// 			data << (BYTE)(m_nBuffCount + 1);
	// 			data << (unsigned int)GetTeamKillMonsterExpBuffId(pActor);//把组队经验buff也下发
	// 			data.writeBuf(m_BuffId, sizeof(int) * (m_nBuffCount));
	// 			pActor->SendData(data.getMemoryPtr(),data.getPosition());
	// 		}
	// 	}
		
	// }

	return true;
}

void CTeam::AddFriendIntimacyWithMember(CActor *pActor)
{
}

void CTeam::SendMemberData(CActor * pActor, CActor* pMember, bool IsBroadcast )
{
	char pdata[512];
	CDataPacket data(pdata, sizeof(pdata));
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsInitTeam; //发送队伍成员的信息
	int nTotalCount = GetMemberCount(); //总人数
	data << (BYTE)nTotalCount;//队友的人数，不包括自己

	for(int i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
	{
		if(m_member[i].pActor &&  m_member[i].pActor->IsInited() )  //在线玩家
		{
			CActor* pAc = m_member[i].pActor;
			data << (unsigned int)(pAc->GetProperty<unsigned int>(PROP_ENTITY_ID)); 
			data.writeString(pAc->GetEntityName());
			data << (int)(pAc->GetProperty<unsigned int>(PROP_CREATURE_LEVEL)); //等级
			data << (BYTE)(pAc->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE)); //转生
			data << (BYTE) (pAc->GetProperty<unsigned int>(PROP_ACTOR_VOCATION)); //职业
			data << (BYTE) (pAc->GetProperty<unsigned int>(PROP_ACTOR_SEX)); //性别
			data << (BYTE)1;
			int nSceneId = pAc->GetSceneID();
			data << nSceneId;//场景ID
			data<< (unsigned int)pAc->GetProperty<unsigned int>(PROP_CREATURE_MAXHP); //MAXHP
			data << (unsigned int)pAc->GetProperty<unsigned int>(PROP_CREATURE_HP); //
			data << (unsigned int)(pAc->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL));
		}
		else  //离线玩家
		{
			OFFLINETEAMMEMBER * pMsg = GetOffLineMsg(m_member[i].nActorID);
			if(pMsg)
			{
				data << pMsg->nActorID;  //actorID
				data.writeString(pMsg->name); //名字
				data << (int)pMsg->btLevel; //等级
				data << (BYTE)pMsg->btCircle;//转生
				data << (BYTE) pMsg->btVocation; //职业
				data << (BYTE) pMsg->btSex; //把性别
				data <<(BYTE) 0; //非在线状态
				data <<(int)0; //当前地图	
				data <<(unsigned int)0; //血量百分比	
				data <<(unsigned int)0; //血量百分比
				data << (unsigned int)(pMsg->nVip);
			}
		}	
	}
	if(m_pCaptin)
	{
		data <<(unsigned int) m_pCaptin->GetProperty<unsigned int>(PROP_ENTITY_ID); //把队长的actorID写入
	}
	else
	{
		data << (unsigned int) 0; //还没有队长
	}
	if(pMember)
	{
		CActorPacket ap;
		CDataPacket& dataPack= pMember->AllocPacket(ap);
		dataPack.writeBuf(pdata, data.getPosition());
		ap.flush();
	}
	if(pActor)
	{
		CActorPacket pack;
		CDataPacket& dataPack= pActor->AllocPacket(pack);
		dataPack.writeBuf(pdata, data.getPosition());
		pack.flush();
	}
	if(IsBroadcast)
	{
		BroadCast(data.getMemoryPtr(),data.getPosition());
	}
}

bool CTeam::AddMember(CActor * pActor, bool isNewMember)
{
	if(pActor ==NULL  ) return false;
	if( pActor->IsInited() ==false) return false;

	//这个玩家已经有队伍了,如果队伍是当前队伍的话，那么情况出现在玩家下线再上线的时候
	CTeam *pCurrentTime = pActor->GetTeam();
	if(pCurrentTime && pCurrentTime != this)//如果已经加入了别人的队伍
	{
		return false;		
	}

	unsigned int nActorID = pActor->GetId(); //玩家的actorID
	if(nActorID == 0) return false;

	//bool validFlag =false;
	bool isOldMember = false;
	INT_PTR nIndex = GetActorIndex(nActorID);

	if(nIndex < 0 || isNewMember) //这个玩家不存在该队伍里
	{
		//如果不能再加人了
		if( GetTeamMaxMemberCount() - GetMemberCount() <= 0 )
		{
			return false;
		}
		for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; ++i)
		{
			if(m_member[i].nActorID ==0 ) //空位
			{
				nIndex = i;
				AddFriendIntimacyWithMember(pActor);
				break;
			}
		}
	}
	else
	{
		isOldMember = true;
		if(m_member[nIndex].pActor && m_member[nIndex].pActor->IsInited() ) //重复加入队伍，玩家已经在这个队伍里了，就不让添加
		{
			return false;
		}
	}

	if(nIndex < 0) return false; //没有位置了
	INT_PTR nOnlineCount = GetOnlineUserCount();

	

	//第1个加入的人就是队长
	if(nOnlineCount == 0)
	{
		m_pCaptin = pActor;
		pActor->SetSocialMask(smTeamCaptin,true);
		pActor->SetSocialMask(smTeamMember,false);
	}
	else
	{
		pActor->SetSocialMask(smTeamMember,true);
	}

	//如果有在线的人，那么需要通知在线的人有一个人加入了
	m_member[nIndex].pActor = pActor;	// 真正加入
	m_member[nIndex].nActorID = nActorID;
	m_bOnlinemMemberCount ++;
	if( nOnlineCount > 0)
	{
		if(isNewMember)
		{
			BroadcastAddMember(pActor, (BYTE)1); //向在线的人广播这个玩家加入了队伍
		}
		else 
		{
			BroadcastMemberOnlineState(pActor,1);
		}

	}
	
	if(isOldMember)
	{
		ClearOffLineMsg(nActorID); //上线的时候要清掉离线消息
	}
	SendMemberData(pActor);
	pActor->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID,m_nTeamID);
	UpdateNameClr_OnJoinOrLeaveTeam(pActor->GetHandle(), m_nTeamID, true);

	pActor->GetTeamSystem().TeamMoveBroadcast();
	pActor->GetTeamSystem().SetAllowTeam(1); //允许组队

	return true;
}


//判断一个队伍是否是非法的
bool CTeam::IsTeamValid()
{
	if(m_pCaptin ==NULL ) return false;
	if(m_pCaptin->IsInited() ==false) return false; //队长的指针无效
	INT_PTR nOnlineCount =0;
	for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	{
		if( m_member[i].pActor )
		{
			if(m_member[i].pActor->IsInited() ==false)
			{
				return false;
			}
			else
			{
				nOnlineCount ++;
			}
		}
	}
	if( nOnlineCount <=0 )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CTeam::BroadCast(LPCVOID pData, INT_PTR nLen)
{
	for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	{
		if(m_member[i].pActor && m_member[i].pActor->IsInited() )
		{
			m_member[i].pActor->SendData(pData,nLen);
		}
	}
}

//并且广播经验加成buf图标
void CTeam::BroadCast(LPCVOID pData, INT_PTR nLen,INT_PTR nMyId )
{
	for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	{
		if(m_member[i].pActor && m_member[i].nActorID != nMyId)
		{
			m_member[i].pActor->SendData(pData,nLen);
		}
	}
}

//广播新增玩家
void CTeam::BroadcastAddMember(CActor* pActor, BYTE idx)
{
	if(!pActor)return;
	char buff[256];
	CDataPacket data(buff,sizeof(buff));
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsAddMember;
	WriteActorInfo(pActor,data, idx);
	BroadCast(data.getMemoryPtr(),data.getPosition());
	char sText1[1024];
	LPCTSTR sFormat1 = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmAddTeamMember);
	sprintf_s(sText1, sizeof(sText1), sFormat1, ( char *)pActor->GetEntityName());
	BroadcastTipmsgArgs(pActor->GetId(), tmAddTeamMember, tstUI, pActor->GetEntityName());
	SendTeamChatMsg(sText1);
	pActor->SendTipmsgFormatWithId(tmSelfAddTeam, tstUI);
}

//广播一个玩家上线
void CTeam::BroadcastMemberOnlineState(CActor* pActor, BYTE state)
{
	if(!pActor)return;
	char buff[256];
	CDataPacket data(buff,sizeof(buff));
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsMemOnlineState;
	// data << (unsigned int)(pActor->GetProperty<unsigned int>(PROP_ENTITY_ID));
	// data << state;
	WriteActorInfo(pActor,data, 1);
	BroadCast(data.getMemoryPtr(),data.getPosition(), (unsigned int)(pActor->GetProperty<unsigned int>(PROP_ENTITY_ID)));
}


void CTeam::BroadcastUpdateMember(CActor* pActor, int nPropId)
{
	if(!pActor)return;
	char buff[256];
	CDataPacket data(buff,sizeof(buff));
	// data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsUpdateMember;
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsMemOnlineState;
	WriteActorInfo(pActor,data, 1);
	BroadCast(data.getMemoryPtr(),data.getPosition());
}

void CTeam::WriteOffLineActorInfo(OFFLINETEAMMEMBER * pMsg,CDataPacket & data, BYTE idx)
{
	// data <<(Uint64)0;  //handle
	data << pMsg->nActorID;  //actorID
	data.writeString(pMsg->name); //名字

	data << (BYTE)pMsg->btLevel; //等级
	data << (BYTE)pMsg->btCircle;//转生
	data << (BYTE) pMsg->btVocation; //职业
	data << (BYTE) pMsg->btSex; //把性别
	data <<(BYTE) 0; //非在线状态

	data <<(BYTE)0; //当前地图
	//血量百分比	
	data <<(unsigned int)0; //
	data <<(unsigned int)0; //
	
}

void CTeam::WriteActorInfo(CActor * pActor,CDataPacket & data, int isOline)
{
	if(pActor && pActor->IsInited())
	{
		data << (unsigned int)pActor->GetId(); 
		data.writeString(pActor->GetEntityName());
		data << (int) pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL); //等级
		data << (BYTE) pActor->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE); //转生
		data << (BYTE) pActor->GetProperty<unsigned int>(PROP_ACTOR_VOCATION); //职业
		data << (BYTE) pActor->GetProperty<unsigned int>(PROP_ACTOR_SEX); //性别
		data << (BYTE)isOline;
		data << (int) pActor->GetSceneID();//场景ID
		data<< (unsigned int)pActor->GetProperty<unsigned int>(PROP_CREATURE_MAXHP); //MAXHP
		data << (unsigned int)pActor->GetProperty<unsigned int>(PROP_CREATURE_HP); //	
		data << (unsigned int)pActor->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL); //vip	
	}
}

//删除队伍成员
bool CTeam::DelMember(unsigned int nActorID,bool bNeedBroadcast,bool bNeedSelfDestroyTeam)
{
	INT_PTR nIndex = GetActorIndex(nActorID);
	if((nIndex < 0) || (nIndex >= MAX_TEAM_MEMBER_COUNT))return false; //没有这个玩家
	CActor* pActor = m_member[nIndex].pActor;
	
	//广播删除
	char buff[128];
	CDataPacket data(buff,sizeof(buff));

	unsigned int nOldTeamId = m_nTeamID;
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsDelMember;
	data << nActorID;

	if(bNeedBroadcast)
	{
		BroadCast(data.getMemoryPtr(),data.getPosition());
		char text1[1024];
		LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDelTeamMember);
		sprintf_s(text1, sizeof(text1), sFormat, (char*)(pActor ? pActor->GetEntityName() : GetOffLineMsg(nActorID)->name));
		SendTeamChatMsg(text1);
		BroadcastTipmsgArgs(nActorID, tmDelTeamMember, tstUI, pActor ? pActor->GetEntityName() : GetOffLineMsg(nActorID)->name);
	}

	if(pActor && pActor->IsInited())
	{
		pActor->SendTipmsgFormatWithId(tmSelfLeavTeam,tstUI);
	}
	// if(pActor)
	// {
	// 	pActor->GetTeamSystem().SetAllowTeam(0);
	// }

	m_member[nIndex].nActorID = 0;
	m_member[nIndex].pActor = NULL;
	if(pActor )
	{
		pActor->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID,0); //设置为无队伍
		pActor->SetSocialMask(smTeamMember,false); //设置玩家不在队伍
		
		if(m_bOnlinemMemberCount > 0)
		{
			m_bOnlinemMemberCount --;
		}
	}
	
	bool bNeedDestroy = false; //需要删除
	int nMemCount = GetMemberCount();
	//没有人在线，或者总人数少于1个，那么就删除队伍
	if(bNeedSelfDestroyTeam &&  nMemCount <= 1  ) 
	{
		bNeedDestroy = true;
		if(pActor)
			pActor->SetSocialMask(smTeamCaptin,false);
		//DestroyTeam();
	}
	else
	{
		ClearOffLineMsg(nActorID); //清除玩家的离线消息
		if(pActor) 
		{
			if( m_pCaptin == pActor)//如果队长退出队伍，需要新选择一个人为队长
			{
				pActor->SetSocialMask(smTeamCaptin,false); //设置玩家不是队长
				CActor* pCaptin= SelectCaptin();
				if(pCaptin)
				{
					SetCaptin(pCaptin); //设置一个人为新队长
				}
				else
				{
					m_pCaptin = NULL;
					bNeedDestroy = true;
				}
			}			
			UpdateNameClr_OnJoinOrLeaveTeam(pActor->GetHandle(), m_nTeamID, false);			
		}
	}

	//执行脚本，如果在队伍副本中，只剩下一个人，则要退出副本
	// if(pActor && pActor->IsInited())
	// {
		
	// 	CScriptValueList paramList;
	// 	//pActor->InitEventScriptVaueList(paramList,aeLeaveTeam);
	// 	paramList << this;
	// 	pActor->OnEvent(aeLeaveTeam,paramList,paramList);
		
	// }
	if(bNeedDestroy)
	{
		DestroyTeam();
		//广播删除
	}
	else
	{
		// 离开队伍，更新原队伍玩家平均等级发生变化，需要更新副本怪物强度
		GetGlobalLogicEngine()->GetTeamMgr().updateTeamMemFubenAveragePlayerLvl(nOldTeamId);
	}
	//DelMemberBuf(pActor);	//退队的人删掉经验加成buf
	//UpdateBufMember();
	//
	SendDestoyTeam(pActor);
	return true;
}

void CTeam::SendDestoyTeam(CActor* pActor)
{
	if(!pActor) return;
	// pActor->GetTeamSystem().SetAllowTeam(0);
	CActorPacket ap;
	CDataPacket& dataPack= pActor->AllocPacket(ap);
	dataPack << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsDestroyTeam;
	ap.flush();
}

void CTeam::BroadCastBagOwner(CDropBag * pBag, char * pData,SIZE_T nSize)
{
	for (INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
	{
		if(m_member[i].pActor && m_member[i].pActor->IsInited() &&pBag->GetBagOwnerFlag(i))
		{
			((CActor*)m_member[i].pActor)->SendData(pData,nSize);
		}
	}
}
	
bool CTeam::SetCaptin(CActor* pActor)
{
	if(!pActor || !pActor->IsInited()) return false; //防止数据出问题	
	
	unsigned int nActorID = pActor->GetId(); //玩家的actorID
	INT_PTR nIndex = GetActorIndex(nActorID);
	if((nIndex < 0) || (nIndex >= MAX_TEAM_MEMBER_COUNT))return false; //没有这个玩家
	if(pActor == m_pCaptin ) return false; //已经是同一个人了
	//这个队伍已经删除了
	if(m_member[nIndex].pActor == NULL )
	{
		return false;
	}
	if(m_pCaptin)
	{
		m_pCaptin->SetSocialMask(smTeamCaptin,false);
		m_pCaptin->SetSocialMask(smTeamMember,true);
	}
	m_pCaptin = pActor;
	pActor->SetSocialMask(smTeamCaptin,true);
	pActor->SetSocialMask(smTeamMember,false);
	//广播设置队长
	char buff[64];
	CDataPacket data(buff,sizeof(buff));
	data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsSetCaptin ;
	data << nActorID;
	BroadCast(data.getMemoryPtr(),data.getPosition());
	BroadcastTipmsgArgs(0, tmSetNewCaptin, tstUI,pActor->GetEntityName()); //队长已经移交给了xxx
	return true;
}


void CTeam::BroadcastTipmsg(INT_PTR nTipmsgID, INT_PTR nTipmsgType)
{
	for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; i++)
	{
		if(m_member[i].pActor && m_member[i].pActor->IsInited())
		{
			m_member[i].pActor->SendOldTipmsgWithId(nTipmsgID,nTipmsgType);
		}
	} 
}

void CTeam::BroadcastTipmsgArgs(unsigned int uExcludeActorId, INT_PTR nTipmsgID, INT_PTR nTipmsgType,...)
{
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipmsgID);
	
	for(INT_PTR i =0 ;i < MAX_TEAM_MEMBER_COUNT ; i++)
	{
		if((!uExcludeActorId || uExcludeActorId != m_member[i].nActorID) && m_member[i].pActor && m_member[i].pActor->IsInited())
		{
			va_list args;
			va_start(args, nTipmsgType);
			m_member[i].pActor->SendTipmsgWithArgs(sFormat,args,nTipmsgType);
			va_end(args);
		}
	} 
	
}


VOID CTeam::SendTeamChatMsg(char* msg, CActor* pActor)
{
	// char buff[1024];
	// CDataPacket outPack(buff, sizeof(buff)); //下发的数据包

	// outPack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelTeam;
	// if(pActor)
	// {
	// 	outPack.writeString(pActor->GetEntityName()); //把自己的名字写上去
	// 	outPack.writeString(msg);
	// 	outPack << pActor->GetLevel();
	// 	Uint64 actorid = Uint64(pActor->GetHandle());
	// 	outPack <<actorid;
	// }
	// else
	// {
	// 	outPack.writeString(""); //把自己的名字写上去
	// 	outPack.writeString(msg);
	// 	outPack << (int)0;
	// 	outPack <<(Uint64)0;
	// }
	// outPack << (BYTE)0;
	
	// BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
}


void  CTeam::DestroyTeam()
{
	//广播删除
	char data[512];
	CDataPacket dataPack(data, sizeof(data));
	dataPack << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsDestroyTeam;
	for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; i++)
	{
		if(m_member[i].pActor && m_member[i].pActor->IsInited())
		{
			CActor* pActor = m_member[i].pActor;
			pActor->SetProperty<unsigned int>(PROP_ACTOR_TEAM_ID,0); //设置为无队伍
			pActor->SetSocialMask(smTeamMember,false); //设置玩家不在队伍
			pActor->SetSocialMask(smTeamCaptin,true);

			CActorPacket ap;
			CDataPacket& dataPack= pActor->AllocPacket(ap);
			dataPack.writeBuf(data, dataPack.getPosition());
			ap.flush();
			// DelMember(m_member[i].nActorID,false,false); //这里不广播，只发给自己
		}
	} 
	//如果有进入副本，从fubenmaganger队列删除本队伍
	//GetGlobalLogicEngine()->GetFuBenMgr()->DelTeam(this);

	//这里会执行删除，故上面就不需要去检测删除队伍了
	GetGlobalLogicEngine()->GetTeamMgr().DestroyTeam(m_nTeamID); //申请删除一个队伍
}

void CTeam::SaveOfflineMsg(CActor * pActor,unsigned int nActorID)
{	
	//已经存在这个玩家的离线信息就不要保存
	INT_PTR nIndex =-1;
	for(INT_PTR i=0; i< MAX_OFFLINE_MEMBER_COUNT ;i++)
	{
		if(m_offLineMember[i].nActorID == 0)
		{
			nIndex = i;
			break;
		}
	}
	if(nIndex >=0)
	{
		m_offLineMember[nIndex].btLevel= (BYTE)pActor->GetProperty<unsigned int>( PROP_CREATURE_LEVEL);
		m_offLineMember[nIndex].btVocation = (BYTE)pActor->GetProperty<unsigned int>( PROP_ACTOR_VOCATION);
		m_offLineMember[nIndex].wIconID = (WORD)pActor->GetProperty<unsigned int>( PROP_ENTITY_ICON);
		m_offLineMember[nIndex].nActorID =  pActor->GetProperty<unsigned int>( PROP_ENTITY_ID);
		m_offLineMember[nIndex].btSex  = (BYTE)pActor->GetProperty<unsigned int>( PROP_ACTOR_SEX); //性别
		m_offLineMember[nIndex].nVip  = (unsigned int)pActor->GetProperty<unsigned int>( PROP_ACTOR_SUPPER_PLAY_LVL); //性别
		strcpy( m_offLineMember[nIndex].name ,pActor->GetEntityName());
	}	
}

INT_PTR CTeam::GetNearTeamMember(CActor * pUser,CActor ** pMember )
{
	//拾取的距离的平分
	int nDis=GetLogicServer()->GetDataProvider()->GetGlobalConfig().teamLootMaxDistanceSquare;
	if(m_bOnlinemMemberCount ==0) return 0; //没有在线的人
	INT_PTR nCount=0;
	for (INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT ; i++)
	{
		if(m_member[i].pActor &&  m_member[i].pActor->IsInited())
		{
			if(m_member[i].pActor != pUser && pUser->GetEntityDistanceSquare(m_member[i].pActor) <= nDis ) //在拾取范围里
			{
				if(pMember != NULL)
				{
					pMember[nCount]= m_member[i].pActor;
				}
				nCount ++; //参加拾取的人
			}
		}
	}
	return nCount;
}

bool  CTeam::SetUserOffline(CActor * pActor)
{
	if(pActor == NULL ) return false;
	//if(pActor->IsInited() ==false) return false;

	unsigned int nActorID = ((CEntity*)pActor)->GetProperty<unsigned int>(PROP_ENTITY_ID);//获取玩家的actorid
	INT_PTR nIndex = GetActorIndex(nActorID);
	if((nIndex <0) || (nIndex >= MAX_TEAM_MEMBER_COUNT))return false; //没有这个玩家
	if(m_member[nIndex].pActor == NULL) return false; //这个玩家没有在线
	m_member[nIndex].pActor =NULL;
	m_bOnlinemMemberCount --;

	bool bNeedDestroy =false; 
	if(m_bOnlinemMemberCount <=0) //如果所有的人都离线了，可以删除这个队伍了
	{		
		bNeedDestroy = true;		
	}
	else
	{
		SaveOfflineMsg(pActor,nActorID); //保存离线消息
		char buff[64];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE)enTeamSystemID << (BYTE) enTeamSystemsMemberLogout ;
		data << nActorID ;
		BroadCast(data.getMemoryPtr(),data.getPosition());
		if(m_pCaptin == pActor) //如果队长下线了，需要新选择一个人为队长
		{
			CActor* pCaptin= SelectCaptin();
			if(pCaptin)
			{
				SetCaptin(pCaptin); //设置一个人为新队长
			}
			else
			{
				bNeedDestroy =true; //选不出队长了，就删除
			}		
		}
	}
	if(bNeedDestroy)
	{
		DestroyTeam();
	}
	else
	{
		// 玩家（如果在队伍中）下线更新副本玩家平均等级
		GetGlobalLogicEngine()->GetTeamMgr().updateTeamMemFubenAveragePlayerLvl(m_nTeamID);
		SendMemberData(NULL, NULL, true);
	}
	//UpdateBufMember();
	return true;
}

 INT_PTR CTeam::GetAllAvgLevel()
 {
	 INT_PTR nTotal = 0;
	 INT_PTR nCount = 0;
	 for (INT_PTR i =0; i< MAX_TEAM_MEMBER_COUNT; i++)
	 {
		 if(m_member[i].pActor && m_member[i].pActor->IsInited())
		 {
			 nTotal += m_member[i].pActor->GetProperty<int>(PROP_CREATURE_LEVEL);
			 nCount++;
		 }
		 //else if (m_member[i].nActorID != 0)
		 //{
			// OFFLINETEAMMEMBER * pMsg = GetOffLineMsg(m_member[i].nActorID);
			// if (pMsg)
			// {
			//	 nTotal += pMsg->btLevel;
			//	 nCount++;
			// }
		 //}
	 }
	 return nCount?nTotal/nCount:0;
 }

 void CTeam::SetChallengeId(unsigned int hHandle)
 {
	 m_nChallengeId = hHandle;
	//  for(INT_PTR i=0 ;i < MAX_TEAM_MEMBER_COUNT; i++)
	//  {
	// 	 if(m_member[i].pActor && hHandle== 0 )
	// 	 {
	// 		 m_member[i].pActor->RemoveState(esChallenge);
	// 	 }
	//  }
 }

 void CTeam::RunOne()
 {
	if ( m_nTimerExpiredTime )
	{
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		if(nNow >= m_nTimerExpiredTime)
		{
			SetExpiredTime(0);
			/*
			CActor* pActor = GetCaptin();
			if(pActor)
			{
				CScriptValueList paramList;
				pActor->InitEventScriptVaueList(paramList,aeTeamTimer);
				paramList << (unsigned int)m_nTeamID;
				pActor->OnEvent(paramList,paramList);
			}
			*/
		}
	}
 }

 void CTeam::SetExpiredTime( unsigned int nExpireTime )
 {
	  m_nTimerExpiredTime = nExpireTime;
 }

 CActor* CTeam::SelectCaptin()
 {
	 unsigned int nMaxLv = 0;
	 CActor* pNewCaption = nullptr;
	 for(INT_PTR i = 0; i < MAX_TEAM_MEMBER_COUNT; i++)
	 {
		 CActor* pActor = m_member[i].pActor;
		 if(pActor && pActor->IsInited() )
		 {
			 unsigned int level = pActor->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
			 if (level > nMaxLv)
			 {
				 nMaxLv = level;
				 pNewCaption = pActor;
			 }
		 }
	 }
	 return pNewCaption;
 }

