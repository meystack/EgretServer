#include "StdAfx.h"
#include "UserEquip.h"
#include "../../skill/SkillSubSystem.h"

static LPCTSTR	sEquipPosStrongLevelScriptName	= "sEquipPosStrongLevel";		//保存在ScriptData
//以下顺序，与装备子系统的消息对应
template<>
const CUserEquipment::Inherited::OnHandleSockPacket CUserEquipment::Inherited::Handlers[]=
{
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandTakeOnEquip, //装上装备
	&CUserEquipment::HandTakeOffEquipWithGuid, //脱下一件装备，参数是装备的GUID
	&CUserEquipment::HandGetEquip, //获取自身的装备
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandGetOtherEquip, //获取其他玩的装备
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandGetRankOtherEquip, //获取其他玩的装备
	&CUserEquipment::HandGetCenterRankOtherEquip, //获取其他玩的装备
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandError,    //错误处理
	&CUserEquipment::HandGetBelongActorInfo,//获取非活动副本Boss归属玩家信息
};


VOID CUserEquipment::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;
	if(m_pEntity->GetHandle().GetType() != enActor) return; 
	//if(nCmd >=1 && nCmd < cMaxEquipCodeID)
	if(nCmd >=1 && nCmd < ArrayCount(Handlers))
	{
		(this->*Handlers[nCmd] )(packet);
	}
}

CAttrCalc & CUserEquipment::CalcAttributes(CAttrCalc &cal,CAttrCalc *pOriginalCal)
{
	if(m_pEntity ==NULL )
	{
		return cal;
	}
	return CalcAttr(cal,pOriginalCal,m_pEntity);
}

CAttrCalc & CUserEquipment::CalcFashionAttr(CAttrCalc & cal)
{
	return cal;
}

bool CUserEquipment::Initialize(void *data, SIZE_T size)
{
	memset(m_Items, 0, sizeof(m_Items));
	memset(m_ChangeDura, 0, sizeof(m_ChangeDura));
	m_nSenderRingCd = m_nEquipScore =0; //装备的评分
	return true;
}


//定时回调
VOID CUserEquipment::OnTimeCheck(TICKCOUNT nTickCount)
{
	if ( !m_pEntity )
	{
		return;
	}
	if (m_pEntity->GetType() != enActor)
		return;
	//unsigned  int nCurrent;
}


void CUserEquipment::HandTakeOnEquip(CDataPacketReader &packet)
{
	//服务器判断对应穿装备的位置
	CUserItem::ItemSeries series;
 	packet >> series.llId;
	TakeOn(series);
	DealEquipmentAchieve();
}

void CUserEquipment::HandGetOfflineEquip(CDataPacketReader &packet)
{
	char name[32];
	unsigned int nActorId =0;
	packet.readString(name,sizeof(name));
	name[sizeof(name) -1] =0;
	packet >>nActorId ;
	
	GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActor(((CActor *)m_pEntity), nActorId);
}

void CUserEquipment::HandGetHeroOfflineEquip(CDataPacketReader &packet)
{
	unsigned int nActorId =0;
	BYTE btHeroId = 0;
	packet >> nActorId;
	packet >> btHeroId;

	GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActorHero(((CActor *)m_pEntity), nActorId, btHeroId);
}

void CUserEquipment::HandEquipInsure(CDataPacketReader &packet)
{

}


void CUserEquipment::HandGetOtherEquip(CDataPacketReader &packet)
{
	if (((CActor *)m_pEntity)->HasMapAttribute(aaCannotViewOther))
	{
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmDisableOperator);
		return;
	}

	//char msg[54];
	unsigned int nActorId = 0;
	packet >> nActorId;
	BYTE nType = 0;
	packet >> nType;
	if(nType == 1) 
	{	
		GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActor((CActor *)m_pEntity, nActorId);
	}else
	{
		GetGlobalLogicEngine()->GetOfflineCenterUserMgr().ViewCenterOffLineActor((CActor *)m_pEntity, nActorId);
	}	
}

void CUserEquipment::HandGetBelongActorInfo(CDataPacketReader &packet)
{
	unsigned int nActorId = 0;
	packet >> nActorId;
	unsigned int nBossId = 0;
	packet >> nBossId;
	
	CActorPacket pack;
    CDataPacket & outPack = ((CActor *)m_pEntity)->AllocPacket(pack);
	outPack << (BYTE)enEuipSystemID << (BYTE)sSendBelongActorInfo;
	outPack << (BYTE)0;//暂未使用
	outPack << (int)nBossId;//暂未使用

	//归属玩家是否在线(0：离线；1：在线)
	BYTE isBelongActorOffline = 0;
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	if ( pActor )
	{
		isBelongActorOffline = 1;
		outPack << (BYTE)isBelongActorOffline;

		outPack << (unsigned int)nActorId;
		outPack << (BYTE)pActor->GetProperty<int>(PROP_ACTOR_VOCATION);;
		outPack << (BYTE)pActor->GetProperty<int>(PROP_ACTOR_SEX);
		outPack.writeString(pActor->GetEntityName());

		CGuild* pGuild = pActor->GetGuildSystem()->GetGuildPtr();
		if ( pGuild )
		{
			outPack.writeString(pGuild->GetGuildName());

			//归属玩家是否沙巴克成员(0：非SBK成员；1：SBK成员)
			BYTE isSBKMember = 0;
			if ( GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId() == pGuild->GetGuildId() )
			{
				isSBKMember = 1;
			}
			outPack << (BYTE)isSBKMember;
		}
		else
		{
			outPack.writeString("");//归属玩家行会名字
			outPack << (BYTE)0;//归属玩家是否沙巴克成员(0：非SBK成员；1：SBK成员)
		}
		
		outPack << (int)pActor->GetProperty<int>(PROP_CREATURE_MAXHP);
		outPack << (int)pActor->GetProperty<int>(PROP_CREATURE_HP);
		outPack << (Uint64)pActor->GetHandle();
	}
	else
	{
		outPack << (BYTE)isBelongActorOffline;
	}
	pack.flush();
}

void CUserEquipment::HandGetRankOtherEquip(CDataPacketReader &packet)
{
	if (((CActor *)m_pEntity)->HasMapAttribute(aaCannotViewOther))
	{
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmDisableOperator);
		return;
	}

	//char msg[54];
	unsigned int nActorId = 0;
	packet >> nActorId;
	GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActor((CActor *)m_pEntity, nActorId ,enEuipSystemID,sGetRankOtherEquip);
}


void CUserEquipment::HandGetCenterRankOtherEquip(CDataPacketReader &packet)
{
	if (((CActor *)m_pEntity)->HasMapAttribute(aaCannotViewOther))
	{
		((CActor *)m_pEntity)->SendTipmsgFormatWithId(tmDisableOperator);
		return;
	}

	//char msg[54];
	unsigned int nActorId = 0;
	packet >> nActorId;
	BYTE nType = 0;
	packet >> nType;
	if(nType == 1) 
	{
		GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActor((CActor *)m_pEntity, nActorId,enEuipSystemID,sGetCenterRankOtherEquip);
	}
	else
	{
		GetGlobalLogicEngine()->GetOfflineCenterUserMgr().ViewCenterOffLineActor((CActor *)m_pEntity, nActorId ,enEuipSystemID,sGetCenterRankOtherEquip);
	}
}

void CUserEquipment::HandSetFathionClothFlag (CDataPacketReader &packet)
{
	BYTE btOperate = 0;
	packet >> btOperate;

	unsigned int nPrp = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);

	unsigned int nMask = 0;
	if (btOperate == 1)
	{
		nMask = 1 << smHideWeaponExtend;
	}
	else
	{
		nMask = 1 << smHideFationCloth;
	}

	unsigned int nNewValue = nPrp&nMask;

	if(nNewValue)
	{
		nPrp &= ~(nMask);
	}
	else
	{
		nPrp |= nMask;
	}

	m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK,nPrp);
	m_pEntity->CollectOperate(CEntityOPCollector::coRefFeature); //需要刷一下外观
}
void CUserEquipment::HandTakeOffEquipWithGuid (CDataPacketReader &packet)
{
	CUserItem::ItemSeries series;
	packet >> series.llId;
	BYTE nPos;
	packet >> nPos;
	if(nPos >= itWeaponPos && nPos < itMaxEquipPos && m_Items[nPos])
	{
		if(m_Items[nPos]->series.llId == series.llId)
		{
			TakeOff(series);

			return;
		}
	}
	m_pEntity->SendTipmsgWithId(tmDataError,tstUI);
}

void CUserEquipment::HandGetEquip(CDataPacketReader &packet)
{
	if(!m_pEntity) return;
	//把当前的装备数据下发到客户端
	//OutputMsg(rmTip,_T("[%d]下发装备的数据"),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
	CActorPacket pack;
	CDataPacket &data = ((CActor *)m_pEntity)->AllocPacket(pack);
	// INT_PTR nCount= GetEquipCount(); //获取装备的数量
	BYTE nCount = 0;
	// int pos = data.getPosition();
	data <<(BYTE)enEuipSystemID<<  (BYTE)sGetUserEquip;
	int pos = data.getPosition();
	data << BYTE(nCount);
	for (INT_PTR i=0; i< itMaxEquipPos; i++ )
	{
		if(m_Items[i])
		{
			(CUserItem)*m_Items[i]>>data;
			data << (BYTE)i;
			nCount++;
			//data << (CUserItem)*m_Items[i] << (BYTE)i;
		}
	}
	BYTE* pB = (BYTE*) data.getPositionPtr(pos);
	*pB = (BYTE)nCount;
	pack.flush();
}



void CUserEquipment::EquipChange(int nPos)
{
	if((nPos < 0) || (nPos >= EquipmentCount)) return;
	if(!m_pEntity) return;

	CActorPacket pack;
	CDataPacket &data = ((CActor *)m_pEntity)->AllocPacket(pack);
	data <<(BYTE)enEuipSystemID<<  (BYTE)sEquipDataChage << BYTE(nPos);

	if(m_Items[nPos])
	{
		(CUserItem)*m_Items[nPos]>>data;
	}
	pack.flush();
}

void CUserEquipment::Save(PACTORDBDATA  pData)
{
	if(m_pEntity ==NULL) return;
	
	CDataPacket& dataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveEquipItem);
	dataPacket << ((CActor *)m_pEntity)->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket <<(unsigned int ) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家ID

	size_t count = GetEquipCount();
	dataPacket  << (BYTE )itItemEquip;
	int pos = dataPacket.getPosition();
	int nCount = 0;
	dataPacket << (int)nCount  ; //玩家物品的数量
	if(count > 0)
	{
		CStdItemProvider & itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
		for (INT_PTR i=0; i<EquipmentCount; i ++ )
		{
			if(m_Items[i])
			{
				const CStdItem *pStdItem = itemProvider.GetStdItem(m_Items[i]->wItemId);
				if (pStdItem != NULL)
				{
					INT_PTR nPos = GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType, (INT_PTR)m_Items[i]->btHandPos);
					if (nPos < 0)
					{
						OutputMsg(rmError, _T("%s:玩家[%s]的装备[%d,%lld],位置[m_btType:%d，handPos:%d,equipPos:%d]存盘异常"), __FUNCTION__, m_pEntity->GetEntityName(),(int)m_Items[i]->wItemId, m_Items[i]->series,(int)pStdItem->m_btType, (int)m_Items[i]->btHandPos,(int)i);
					}
				}
				if(m_Items[i]->wItemId == 0) {
					OutputMsg(rmError, _T("玩家[%s]的装备[%d,%lld],位置[wPackageType:%d，handPos:%d,equipPos:%d]装备异常"), m_pEntity->GetEntityName(),(int)m_Items[i]->wItemId, m_Items[i]->series,(int)m_Items[i]->wPackageType, (int)m_Items[i]->btHandPos,(int)i);
					continue;
				}
				nCount++;
				// OutputMsg(rmTip, _T("玩家[%d]的装备[%d,%lld],位置[wPackageType:%d，handPos:%d,equipPos:%d]"), m_pEntity->GetId(),(int)m_Items[i]->wItemId, m_Items[i]->series,(int)m_Items[i]->wPackageType, (int)m_Items[i]->btHandPos,(int)i);
				dataPacket <<(CUserItem) *m_Items[i]; //把一个物品打包过去
			}
		}
		int* pCount = (int*)dataPacket.getPositionPtr(pos);
        *pCount = nCount;
	}
	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
}


void CUserEquipment::SetEquipItemProp(INT_PTR nSlotId, INT_PTR nPropId, INT_PTR nVal)
{
}


//改变装备的耐久度
bool CUserEquipment::ChangeEquipDua(CUserItem * pEquip,int nValue,bool bForce)
{
	return true;
}

void CUserEquipment::OnEquipDurChanged(CUserItem *pEquip,const CStdItem * pStdItem)
{
}

void CUserEquipment::SendItemDuaChange(CUserItem * pEquip, CActor *pActor)
{
}

VOID CUserEquipment::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载玩家装备的数据出错,实体指针为MULL"));
		return;
	}
	//装载物品数据
	//装载装备信息
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadEquipItem &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回物品的列表
	{
		int nCount =0;
		reader >> nCount;
		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
		
		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem<< reader;
			// INT_PTR nReadSize = reader.readBuf(&userItem,sizeof(CUserItem));
			// if( sizeof(CUserItem) !=  nReadSize) //读取内存
			// {
			// 	OutputMsg(rmError,"[%s] read net io error,size=%d ",__FUNCTION__,(int) nReadSize);
			// 	continue;
			// }
			CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
			
			if(pUerItem)
			{
				INT_PTR nPos =-1;
				const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUerItem->wItemId);
				
				if ( !pStdItem )
				{

					OutputMsg(rmError,_T("actorID=%d的玩家item=%d,装备非法"),m_pEntity->GetId(),pUerItem->wItemId);
					pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
					continue;
				}
				else
				{
					nPos = GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType,pUerItem->btHandPos);
					
					if(nPos <0)
					{
						OutputMsg(rmError,_T("actorID=%d的玩家nPos=%d,location=%d,itemId=%d,guid=%lld,装备位置非法"),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),nPos,pUerItem->btHandPos,pUerItem->wItemId,pUerItem->series);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
						continue;
					}
				

					if(nPos >=0 && m_Items[nPos] ==NULL)
					{
						m_Items[nPos] = pUerItem;
						EquipTakenInitTimer(nPos,pStdItem);
					}
					else
					{
						OutputMsg(rmError,_T("actorID=%u的玩家nPos=%d已经有装备了"),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),nPos);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
					}
				}
			}
		}
		((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_EQUIP_SYSTEM); //完成一个步骤
	}
	else if(nCmd == jxInterSrvComm::DbServerProto::dcLoadEquipItem &&  nErrorCode != jxInterSrvComm::DbServerProto::reSucc)
	{
		OutputMsg(rmError,_T("装载玩家[%s]的装备的数据出错,actorid=%d"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
		((CActor*)m_pEntity)->CloseActor(lwiEquipDbFail, false); //如果装载装备的数据出错就踢出
	}
}

//装备穿上去的时候，如果是源泉物品需要算一下cd时间
void CUserEquipment::EquipTakenInitTimer(INT_PTR nPos,const CStdItem * pStdItem)
{
}

 void CUserEquipment::AfterEquipTakenOn(CActor * pOnwer,INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation)
 {
	if(pOnwer ==NULL) return;
	//return;
	const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if (!pStdItem)
	{
		return;
	}
	{
		CActorPacket pack;
		CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
		data <<(BYTE)enEuipSystemID<<  (BYTE)sTakeOnEquip;
		(CUserItem)*pUserItem >> data;
		data << (BYTE) nPos;
		pack.flush();
	}
	
	
	//设置角色刷新属性的标记
	((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	//设置角色刷新外形的标记
	((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefFeature);

	/*任务内容 1、穿戴多少件多少等级的装备
			   2、穿戴多少件熔铸装备*/ 

	if (pOnwer->GetType() == enActor)
	{
		CheckEquipCount();
	}
 }
 void CUserEquipment::CheckEquipCount()
 {
 }

 //判断 是否穿戴了某个id的装备
 bool CUserEquipment::GetEquipByItemId(WORD nItemId)
 {
	//for(INT_PTR i=itWeaponPos; i <=  itSpecialRingPos; i++)
	for(INT_PTR i=itWeaponPos; i <=  itEquipDiamondPos; i++)
	 {
		 if(m_Items[i] && m_Items[i]->wItemId == nItemId) 
		 {
				return true;
		 }
	 }
	 return false;	
 }

 //判断 穿戴多少件多少转的装备
 int CUserEquipment::GetEquipCircleCount(int nCircle)
 {
	 int nCount = 0;
	//for(INT_PTR i=itWeaponPos; i <=  itSpecialRingPos; i++)
	 for(INT_PTR i=itWeaponPos; i <=  itEquipDiamondPos; i++)
	 {
		 if(m_Items[i]) 
		 {
			 const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(m_Items[i]->wItemId);
			 if(pStdItem ==NULL) 
				 return false;
			 int Circle = 0;
			
			 for (INT_PTR i = pStdItem->m_Conditions.nCount-1; i>-1; --i)
			 { 
				 CStdItem::ItemUseCondition& pCond = pStdItem->m_Conditions.pConds[i];
				 if(pCond.btCond == CStdItem::ItemUseCondition::ucMinCircle)
				 {
					 Circle = pCond.nValue;
					 break;
				 }
			 }
			 if(nCircle <= Circle)
			 {
				 nCount++;
			 }
		 }
	 }
	 return nCount;	
 }

 int CUserEquipment::GetEquipLevelCount(int nLevel) 
 {
	 //任务事件，穿戴多少件多少等级的装备
	int nCount = 0;
	return nCount;
 }

 int CUserEquipment::GetEquipMeltingCount(int nLevel) 
 {
	 //任务事件，穿戴多少件熔铸装备
	int nCount = 0;
	return nCount;
 }

bool CUserEquipment::TakeOn(CUserItem* pUserItem)
{
	if( EquipBaseClass::TakeOnEquip((CActor*)m_pEntity ,pUserItem, NULL, 0, false) == false ) return false; //如果上级没有成功
	//不要在这里添加代码,添加到AfterEquipTakenOn
	return true;
}
bool CUserEquipment::TakeOn(const CUserItem::ItemSeries &series)
{
	if( EquipBaseClass::TakeOnEquip((CActor*)m_pEntity ,series, NULL) == false ) return false; //如果上级没有成功
	//不要在这里添加代码,添加到AfterEquipTakenOn
	return true;
}


//获取升星衍生的光效
const INT_PTR CUserEquipment::GetStarAppear(const CEquipVessel *pEquips)
{
	INT_PTR nStarCount = pEquips->GetSuitMinStar();       //获取星星数
	CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	CStdItemProvider::PSTARDERIVE pStar = stdItemProvider.GetStarDerive(nStarCount);
	return pStar!=NULL ?pStar->wAppear:0;
}

int  CUserEquipment::GetEquipAppear(CUserItem * pUserItem)
{
	if(NULL ==pUserItem ) return 0;

	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	ENTITYAPPEARVALUE nNewAppear ;
	nNewAppear.nValue =0;
	const CStdItem * pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
	if(pStdItem ==0) return 0;
	nNewAppear.wLow = pStdItem->m_wShape;
	return nNewAppear.nValue;
}

int  CUserEquipment::GetFootAppear(const CUserItem *pUserItem, bool showAppear)
{
	if (NULL ==pUserItem) return 0;

	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	ENTITYAPPEARVALUE nNewAppear ;
	nNewAppear.nValue =0;
	const CStdItem *pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
	if (pStdItem == 0) return 0;
	if (showAppear)
	{
		nNewAppear.wLow = pStdItem->m_wShape;
	}

	return nNewAppear.nValue;
}

bool CUserEquipment::NeedShowFashion()
{
	bool hasFashion =false;
	unsigned int nPrp = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);
	unsigned int nMask = 1<< smHideFationCloth ;
	if(nPrp & nMask)  //如果需要隐藏外观的话，就不显示
	{
		hasFashion =false;
	}
	else
	{
		hasFashion =true;
	}
	return hasFashion;
}

bool CUserEquipment::NeedShowWeaponExtend(CActor *pMaster, const HERODBDATA *pPetData)
{
	if(!pMaster) return false;
	bool hasFashion =false;
	unsigned int nPrp = pMaster->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);
	unsigned int nMask = 1<< smHideWeaponExtend ;
	if (nPrp & nMask)  //如果需要隐藏外观的话，就不显示
	{
		hasFashion = false;
	}
	else
	{
		hasFashion = true;
	}
	return hasFashion;
}

/*
获取模型、武器、翅膀、兵魂等外观
模型优先级：角色衣服外观 < 时装、玄甲外观 < buff外观 < 场景(或者状态)改变模型
*/
void CUserEquipment::GetAppear(const CEquipVessel *pEquips,int nVocation, int nSex,
	int& nModelId,int &nWeaponAppear, int &nWingAppear,int &nSwingLevel, int& nSoldierSoulAppear,
	int nEntityType,CActor *pMaster)
{
	const CStdItem *pStdItem;
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	bool hasWing = false;		//角色是否有翅膀

	SCENEAREA* pArea = NULL;
	if (CActor* pActor = (CActor*)pMaster)
	{
		if (CScene* pScene = pMaster->GetScene())
		{
			int x,y;
			pActor->GetPosition(x,y);
			pArea = pScene->GetAreaAttri(x,y);
		}
	}

	if(pEquips !=NULL)
	{
		CUserItem * pUserItem = NULL;

		//身体外观
		{
			ENTITYAPPEARVALUE nNewAppear;
			nNewAppear.nValue = 0;
			nNewAppear.wLow = 0;

			ENTITYAPPEARVALUE SoulAppear;
			SoulAppear.nValue = 0;
			SoulAppear.wLow = 0;
			// 改变外形的区域中，不管穿什么衣服都一样外形
			if (pArea && pArea->attri[aaChangeModel].bType == aaChangeModel)
			{
				AreaAttri * pAreaAttr = &pArea->attri[aaChangeModel];
				if (nSex >= pAreaAttr->nCount) nSex = pAreaAttr->nCount - 1;
				nNewAppear.nValue = pAreaAttr->pValues[nSex];
				//pActor->SetProperty<unsigned int>(PROP_ENTITY_MODELID, pAreaAttr->pValues[nSex]);
			}
			else
			{
				// 获取衣服外形id
				pUserItem = pEquips->m_Items[itDressPos];
				if(pUserItem !=NULL) 
				{
					pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
					if ( pStdItem ) {
						// nNewAppear.wHigh = pStdItem->m_wBack;
						nNewAppear.wLow = pStdItem->m_wShape;
						SoulAppear.wLow = pStdItem->m_wShape;
						SoulAppear.wHigh = pStdItem->m_wBack;
					}
					
				} else nNewAppear.wLow = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationDefaultModel(nVocation,nSex);


				/*再处理 时装外观和buff外观
				处理BUFF改变外观,保留宝石特效
				buff改变外观的优先级高于玄甲外观
				*/
				if( nEntityType == enActor && pMaster )	//场景或状态不要求改变模型外观
				{
					
					//时装
					FashionActive *cfg = pMaster->GetAlmirahSystem().GetFashionModeId(2);
					if(cfg)
					{
						SoulAppear.wLow = cfg->nDisplay;
						SoulAppear.wHigh = cfg->nBack;
						
					}
					CBuffSystem * pBuffSystem = pMaster->GetBuffSystem();
					CDynamicBuff *pBuff = pBuffSystem->GetBuff(aBuffChangeModel, CDynamicBuff::AnyBuffGroup);
					if (pBuff != NULL)
					{
						nNewAppear.wLow = nSex ? pBuff->pConfig->nParam: pBuff->value.nValue;
					}
				}
			}
			nModelId = nNewAppear.nValue;
			nSoldierSoulAppear = SoulAppear.nValue;
		}

		//武器外观
		{
			ENTITYAPPEARVALUE nNewAppear;
			nNewAppear.nValue = 0;
			nNewAppear.wLow = 0;
			
			// 改变外形的区域中，不管穿什么衣服都一样外形
			if (pArea && pArea->attri[aaChangeWeapon].bType == aaChangeWeapon)
			{
				AreaAttri * pAreaAttr = &pArea->attri[aaChangeWeapon];
				if (nSex >= pAreaAttr->nCount) nSex = pAreaAttr->nCount - 1;
				nNewAppear.nValue = pAreaAttr->pValues[nSex];
				//pActor->SetProperty<unsigned int>(PROP_ENTITY_MODELID, pAreaAttr->pValues[nSex]);
			}
			else
			{
				// 获取武器外形id
				pUserItem = pEquips->m_Items[itWeaponPos];
				if(pUserItem !=NULL) 
				{
					pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
					if (pStdItem != NULL)
					{
						nNewAppear.wHigh = pStdItem->m_wBack;
						nNewAppear.wLow = pStdItem->m_wShape;			//获取武器的外观
						if(pMaster)
							pMaster->SetProperty<unsigned int>(PROP_ACTOR_WEAPON_ID, pStdItem->m_nIndex);
					}
					else
					{
						if (pMaster)
							pMaster->SetProperty<unsigned int>(PROP_ACTOR_WEAPON_ID, 0);
					}	
				}

				/*BUFF改变武器外观,兵魂跟随身上实体武器变化
				buff改变外观的优先级高于幻武外观
				*/
				if( nEntityType == enActor && pMaster )
				{

					//时装
					FashionActive * cfg = pMaster->GetAlmirahSystem().GetFashionModeId(1);
					if(cfg)
					{
						nNewAppear.wLow = cfg->nDisplay;
						nNewAppear.wHigh = cfg->nBack;
					}
					

					CBuffSystem * pBuffSystem = pMaster->GetBuffSystem();
					CDynamicBuff *pBuff = pBuffSystem->GetBuff(aBuffChangeModel, CDynamicBuff::AnyBuffGroup);
					if (pBuff != NULL)
					{
						nNewAppear.wLow  = nSex ? pBuff->pConfig->nParam3: pBuff->pConfig->nParam2;
					}
				}
			}
			nWeaponAppear= nNewAppear.nValue;
		}

		//翅膀外观
		{
			// ENTITYAPPEARVALUE nNewAppear;
			// nNewAppear.nValue = 0;
			// WORD nCurShapeSwing = 0;
			// if (pMaster != NULL)
			// {
			// 	nCurShapeSwing = pMaster->GetProperty<unsigned int>(PROP_ACTOR_SWINGAPPEARANCE);
			// 	if ( nCurShapeSwing > 0)
			// 	{
			// 		hasWing = true;
			// 	}				
			// }
			// nNewAppear.wLow = nCurShapeSwing;
			
			// //翅膀高位保存 套装光效
			// // if (showDiamond)
			// // {
			// // 	nNewAppear.wHigh = 0;//(WORD)GetSuitAppear(pEquips);	//套装光效
			// // }
			
			// nWingAppear= nNewAppear.nValue;
			//时装
			FashionActive *cfg = pMaster->GetAlmirahSystem().GetFashionModeId(3);
			if(cfg)
			{
				nWingAppear = cfg->nDisplay;	
			}
		}
	}
}

//刷外观
VOID CUserEquipment::RefreshAppear()
{
	//CEquipVessel *pEquips;
	
	bool isWarChariot = false;
	int nVocation =1,nSex =0;

	INT_PTR nEntityType = m_pEntity->GetType();
	if(nEntityType ==enActor)
	{
		//showFation =NeedShowFashion() ;
		nVocation =m_pEntity->GetProperty<int>(PROP_ACTOR_VOCATION);
		nSex = m_pEntity->GetProperty<int>(PROP_ACTOR_SEX);
		//showWeaponExtend = NeedShowWeaponExtend(m_pEntity);
	}
	else if(nEntityType == enHero) 
	{
		const CHeroSystem::HERODATA * pHero =((CHero*)m_pEntity)->GetHeroPtr();
		if(pHero)
		{
			nVocation=pHero->data.bVocation;
			nSex=pHero->data.bSex;

			//showWeaponExtend = NeedShowWeaponExtend(m_pEntity, &(pHero->data));
		}
	}

	int nModelId = 0, nWeaponAppear = 0, nWingAppear = 0,nSwingId = 0,nMaxInner=0;
	int nFashionAppear = 0;
	int nSoldierSoulAppear = 0;//兵魂
	
	//获取模型、翅膀、武器、足迹、兵魂的外观
	GetAppear(this,nVocation,nSex, nModelId,nWeaponAppear,nWingAppear,nSwingId,nSoldierSoulAppear, (int)nEntityType, m_pEntity);

	if (nModelId)
	{
		m_pEntity->SetProperty<int>(PROP_ENTITY_MODELID,nModelId);
	}
	
	m_pEntity->SetProperty<int>(PROP_ACTOR_WEAPONAPPEARANCE,nWeaponAppear);
	m_pEntity->SetProperty<int>(PROP_ACTOR_SWINGAPPEARANCE, nWingAppear); 
	m_pEntity->SetProperty<int>(PROP_ACTOR_SOLDIERSOULAPPEARANCE, nSoldierSoulAppear);
 	if(nEntityType == enActor)
	{
		int nSwingLevel = 0;
		const CStdItem *pStdItem;
		const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
		pStdItem = stdItemProvider.GetStdItem(nSwingId);
		if(pStdItem)
		{
			((CActor*)m_pEntity)->SetSwingLevel(pStdItem->m_wSuitID);
			((CActor*)m_pEntity)->SetSwingId(nSwingId);
		}
		else
		{
			((CActor*)m_pEntity)->SetSwingLevel(0);
			((CActor*)m_pEntity)->SetSwingId(0);
		}
	}

	//这里支持一下是否显示神器的外观
	CUserItem * pUserItem = m_Items[itWeaponPos];
	bool bShowRedWeaponEffect =false;
	if(pUserItem && pUserItem->btQuality == CStdItem::iqItemOrange) //是否身上有神器
	{
		bShowRedWeaponEffect =true;
	}

	if(nEntityType == enActor)
	{
		unsigned int nPrp = m_pEntity->GetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK);
		unsigned int nMask = 1<< smShowRedWeaponEffect ;
		unsigned int nNewValue = nPrp & nMask;
		if(bShowRedWeaponEffect)
		{
			nPrp |= nMask;
		}
		else
		{
			nPrp &= ~(nMask);
		}

		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_SOCIALMASK,nPrp);
	}
	

}

//攻击别人，要处理装备的耐久
void CUserEquipment::OnAttackOther(CAnimal *pEntity, bool bChgDura)
{
}

INT_PTR CUserEquipment::GetRandomInRange(INT_PTR nUpper, INT_PTR nLower)
{
	INT_PTR nValue = 0;
	INT_PTR nDif = nUpper - nLower;
	if (nDif > 0)
		nValue = wrandvalue() % (nDif + 1) + nLower;
	return nValue;
}

INT_PTR CUserEquipment::CalcWeaponEndureConsume()
{
	INT_PTR nValue = 0;
	
	return nValue;
}

INT_PTR CUserEquipment::CalcDressEndureConsume()
{
	INT_PTR nValue = 0;
	
	return nValue;
}

INT_PTR CUserEquipment::CalcArmorEndureConsume()
{
	INT_PTR nValue = 0;
	
	return nValue;
}

//角色每受到一次攻击，帽子、裤子、腰带、鞋子、护腕、项链、饰品、
//戒指均随机掉落1-2点耐久， 衣服随机掉落2-4点耐久。
void CUserEquipment::OnAttacked(CAnimal *pEntity)
{
}



//角色死亡时，穿在身上的所有装备都会掉落耐久值，掉落其该装备耐久值上限的10%。
//若当前耐久不足耐久值上限10%，则装备耐久掉为0。
//当耐久为0时，装备将失去效果，无任何属性加成。
void CUserEquipment::OnDeath()
{
}

bool CUserEquipment::CheckSetFashion()
{
	if (m_pEntity->GetType() == enActor)
	{
		CActor * pActor = (CActor *)m_pEntity;
		CAttrCalc cal;
		CalcFashionAttr(cal);
		CAttrEval * eqipEval =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetEval(pActor->GetProperty<int>(PROP_ACTOR_VOCATION));
		pActor->SetFashionBattle((int)eqipEval->GetAttrSetScore(cal));
		return true;
	}
	return false;
}
bool CUserEquipment::TakeOff(const CUserItem::ItemSeries &series, bool boCheck)
{
	bool boResult = EquipBaseClass::TakeOffEquip( ((CActor*)m_pEntity), series, boCheck);
	if (boResult)
	{
		//CheckSetFashion();
	}
	return boResult;
}

bool CUserEquipment::DeleteEquip(int nIdx, LPCSTR pSender, INT_PTR nLogIdent,bool bNeedFreeMemory)
{
	if ((nIdx < 0) || (nIdx >= EquipmentCount)) return false;
	CUserItem *pItem = m_Items[nIdx];
	if (DeleteEquip(pItem, pSender, nLogIdent,bNeedFreeMemory))
	{
		if(((CActor*)m_pEntity))
			((CActor*)m_pEntity)->GetBagSystem().SendItemChangeLog(2, pItem->wCount, pItem->wItemId ,nLogIdent);
		m_Items[nIdx] = 0;
		return true;
	}

	return false;
}


bool CUserEquipment::DeleteEquip(CUserItem *pItem, LPCSTR pSender, INT_PTR nLogIdent, bool boDeedFreeMem)
{
	if (!pItem) return false;

	if( GetEquipByGuid(pItem->series) ==NULL)
	{
		OutputMsg(rmTip,"CUserEquipment::DeleteEquip,nLogIdent=%d fail,equip not exist ",(int)nLogIdent);
		return false;
	}

	AfterEquipTakenOff(m_pEntity,pItem);

	//销毁物品
	if (boDeedFreeMem)
	{
		GetGlobalLogicEngine()->DestroyUserItem(pItem);
	}	
	return true;
}

void CUserEquipment::AfterEquipTakenOff(CActor * pOnwer,CUserItem *pItem)
{
	
	//这里要向客户端发一个消息，告诉玩家装备上这件物品,如果是宠物可能需要告诉他的主人
	if(pItem ==NULL || pOnwer ==NULL) return ;

	CActorPacket pack;
	CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
	data <<(BYTE)enEuipSystemID<<  (BYTE)sTakeOffEquip;// <<  *pItem;
	(CUserItem) *pItem >> data;
	pack.flush();
	
	//通知脚本事件

	/*
	CScriptValueList va;
	((CActor*)m_pEntity)->InitEventScriptVaueList(va,aeDelEquiment );
	va << pItem->wItemId;
	va << pItem->btCount;
	((CActor*)m_pEntity)->OnEvent(va,va);
	*/


	//m_Items[nPosition] = NULL;//置空装备位置，这里不执行
	//设置角色刷新属性的标记
	((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	//设置角色刷新外形的标记
	((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefFeature);
	return ;
}

//在穿上装备的时候检查一下
void CUserEquipment::OnTakeOnCheckSuit()
{
}

/*此方法好像没有调用
*/
void CUserEquipment::OnCheckEquipStrong()
{
}

//检查宝石的等级
void CUserEquipment::OnCheckDiamondLevel()
{
}




INT_PTR CUserEquipment::Clear(LPCSTR lpSender /* = NULL */, const INT_PTR nLogIdent /* = 0 */)
{
	
	INT_PTR i, nDeleted = 0, nCount;
	const CStdItem *pStdItem;
	const CStdItemProvider &stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	const CUserBag& bag = m_pEntity->GetBagSystem();

	for (i=EquipmentCount-1; i>-1; --i)
	{
		if ( m_Items[i] )
		{
			//记录日志并向角色发送数据
			if ( nLogIdent )
			{
				nCount = m_Items[i]->wCount;
				m_Items[i]->wCount = 0;
				pStdItem = stdItemProvider.GetStdItem(m_Items[i]->wItemId);
				if ( pStdItem )
					bag.LogItemCountChange(m_Items[i], pStdItem, -nCount, lpSender, nLogIdent);
				bag.SendDeleteItem(m_Items[i],nLogIdent);
			}
			//释放物品的内存
			GetLogicServer()->GetLogicEngine()->DestroyUserItem(m_Items[i]);
			nDeleted++;
			//置空装备位置的物品指针
			m_Items[i] = NULL;
		}
	}
	return nDeleted;
}

void CUserEquipment::OnEnterScene()
{
	if (m_pEntity->GetType() != enActor) return;
	CActor *pActor = (CActor *)m_pEntity;
	CFuBen *pFb = pActor->GetFuBen();
	if (!pFb) return;
	CScene *pScene = pActor->GetScene();
	if (!pScene) return;

	int fbId	= pFb->GetFbId();
	int sceneId = pScene->GetSceneId();	

	const CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();	
	for (INT_PTR i=0; i<EquipmentCount; ++i)
	{
		CUserItem *pItem = m_Items[i];		
		if (pItem)
		{
			const CStdItem *pStdItem = itemProvider.GetStdItem(pItem->wItemId);
			if (pStdItem)
			{
				if (pStdItem->needDelWhenExitScene(sceneId))
				{					
					DeleteEquip((int)i, "scene destroy", GameLog::clLeftSceneRemoveItem);
				}		
			}
		}
	}
}

INT_PTR CUserEquipment::RemoveItemByType(int nType, const char * sComment, int nLogId)
{
	INT_PTR nRemovedCnt = 0;
	const CStdItemProvider &itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();	
	for (INT_PTR i = 0; i < EquipmentCount; ++i)
	{
		CUserItem *pItem = m_Items[i];		
		if (pItem)
		{
			const CStdItem *pStdItem = itemProvider.GetStdItem(pItem->wItemId);
			if (pStdItem->m_btType == nType)
			{
				DeleteEquip((int)i, sComment, nLogId);
			}
		}
	}

	return 0;
}


INT_PTR CUserEquipment::DeleteEquip(const CUserItemContainer::ItemOPParam& param, LPCSTR lpSender, const INT_PTR nLogIdent, const int nDuraOdds)
{
	const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(param.wItemId);
	if (!pStdItem)
		return 0;
	INT_PTR Result = 0, nCount = param.wCount;
	CUserItem *pUserItem = NULL;
	for (INT_PTR i = 0; i < EquipmentCount; ++i)
	{
		pUserItem = m_Items[i];
		if(pUserItem == NULL ) continue;

		if ( pUserItem->wItemId == param.wItemId )
		{
			if ( pUserItem->btQuality != param.btQuality && param.btQuality != (BYTE)-1)
				continue;
			if ( pUserItem->btStrong != param.btStrong && param.btStrong != (BYTE)-1 )
				continue;

			if(param.btBindFlag != -1 && param.btBindFlag != 255)
			{
				if( (param.btBindFlag && 0==pUserItem->btFlag) || (param.btBindFlag==0 && pUserItem->btFlag) ) 
				{
					continue;
				}
			}

			// if ((pUserItem->wDuraMax - pUserItem->wDura) > nDuraOdds && nDuraOdds != -1)
			// 	continue;

			if (DeleteEquip((int)i, lpSender, nLogIdent))
			{
				Result++;
				nCount--;
			}
			if (nCount <=0) //已经全部删除完毕了
			{
				break;
			}
		}
	}

	return Result;
}

void CUserEquipment::OnEnterGame()
{
	if (((CActor*)m_pEntity)->m_isFirstLogin)
	{
		// 首次登录，让角色穿上默认装备
		std::vector<int>& dressLiss = GetLogicServer()->GetDataProvider()->GetFirstLoginDress();
		for (size_t i = 0; i < dressLiss.size(); i++)
		{
			int itemid = dressLiss[i];
			CUserItem *pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
			if(pUserItem ==NULL) continue;
			{
				const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(itemid);
				pUserItem->wItemId = pItem->m_nIndex;
				pUserItem->wCount = 1;
				pUserItem->btQuality = pItem->b_showQuality;
				pUserItem->wPackageType = pItem->m_nPackageType;
				pUserItem->setSource(CStdItem::iqOther,GetLogicServer()->GetLogicEngine()->getMiniDateTime());
				TakeOn(pUserItem);
			}
		}
	}
}

bool CUserEquipment::IsStrongLevel( INT_PTR nStrong )
{
	for(INT_PTR i=0; i< EquipmentCount; i ++)
	{
		if(m_Items[i] !=NULL && m_Items[i]->btStrong >= nStrong)
		{
			return true;
		}
	}
	return false;
}

bool CUserEquipment::IsItemInEquip( CUserItem * pItem )
{
	for (INT_PTR i = 0; i < EquipmentCount; i++)
	{
		if (m_Items[i] != NULL && m_Items[i] == pItem)
		{
			return true;
		}
	}
	return false;
}

void CUserEquipment::ClearEquipSharp(bool boCast)
{
	CUserBag & bag = m_pEntity->GetBagSystem();
	for (INT_PTR i = 0; i < EquipmentCount; i++)
	{
		if (m_Items[i] != NULL && m_Items[i]->btSharp > 0)
		{
			m_Items[i]->btSharp = 0;
			if (boCast)
			{
				bag.NotifyItemChange(m_Items[i]);
			}			
		}
	}
}

void CUserEquipment::HandUseSenderRing( CDataPacketReader & packet )
{
	// WORD nX = 0, nY = 0;
	// BYTE btType = 0; //为0则为点击，为1则为自动传送功能
	// packet >> nX >> nY >> btType;
	// static int nSdRingCd = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSenderRingCd;
	// static int nSdRingId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSenderRingId;
	// CUserItem * pRingItem = FindItemByID(nSdRingId);
	// if (pRingItem == NULL)
	// {
	// 	return;
	// }
	
	// if (m_nSenderRingCd > GetLogicCurrTickCount())
	// {
	// 	m_pEntity->SendOldTipmsgWithId(tpSenderRingCding, ttFlyTip);
	// 	return;
	// }
	// CScene * pScene = m_pEntity->GetScene();
	// if (!pScene) return;
	
	// if(((CActor *)m_pEntity)->CanTelport(pScene->GetSceneId(), pScene->GetSceneName(), nX, nY))
	// {
	// 	if(pScene->EnterScene(m_pEntity, nX, nY))
	// 	{
	// 		m_nSenderRingCd = GetLogicCurrTickCount() + nSdRingCd;
	// 		CActorPacket pack;
	// 		CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// 		netPack << (byte)enEuipSystemID << (byte)sSendRingCd;
	// 		netPack << (WORD)(nSdRingCd/1000);
	// 		pack.flush();
	// 		if ( btType == 0 )
	// 		{
	// 			m_pEntity->SendOldTipmsgWithId(tpSendRingSucc, ttFlyTip);
	// 		}
	// 	}
	// }
	// else
	// {
	// 	if ( btType == 0 )
	// 	{
	// 		m_pEntity->SendOldTipmsgWithId(tpSendRingFail, ttFlyTip);
	// 	}
	// }
}

void CUserEquipment::HandGetSenderRingCd( CDataPacketReader & packet )
{
	// static int nGetSdRingCd = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nSenderRingCd;
	// int64 nRetCd = (int64)(m_nSenderRingCd - GetLogicCurrTickCount());
	// if ( nRetCd < 0)
	// {
	// 	nRetCd = 0;
	// }
	// CActorPacket pack;
	// CDataPacket & netPack = m_pEntity->AllocPacket(pack);
	// netPack << (byte)enEuipSystemID << (byte)sSendRingCd;
	// netPack << (WORD)(nRetCd/1000);
	// pack.flush();
}

void CUserEquipment::HandMoveEquipProp( CDataPacketReader & packet )
{
}

/*
装备强化（升星）
*/
void CUserEquipment::HandUpgradeStar( CDataPacketReader & packet )
{
}

//装备分解
void CUserEquipment::HandRecover(CDataPacketReader & packet)
{
	if ( !m_pEntity )
		return;
	BYTE  nAuto = 0;
	BYTE nType = 0;
	BYTE nNum = 0;
	packet >> nAuto >> nType >> nNum;
	//CScriptValueList paramList, retList;
	CActor* pActor = (CActor*)m_pEntity;
	//pActor ->InitEventScriptVaueList(paramList,aeEquipRecover);
	CUserBag & bag = m_pEntity->GetBagSystem();

	//paramList << nAuto;
	//paramList << nType;
	for(int i=0;i < nNum;i++)
	{
		CUserItem::ItemSeries series;
		packet >> series.llId;
		CUserItem *pItem = GetEquipByGuid(series);
		if(pItem == NULL)
		{
			if(nType == 0)
			{
			}
			else
			{
				pItem = bag.FindItemByGuid(series);
			}
		}
		if (pItem == NULL)
		{
			return;
		}
		//paramList << pItem;
	}
	//pActor ->OnEvent(aeEquipRecover,paramList,paramList);
}

//装备合成
void CUserEquipment::HandComposite( CDataPacketReader & packet )
{
	if ( !m_pEntity )
		return;

	BYTE nType		=	0;
	WORD nDesItemID	=	0;
	CUserItem::ItemSeries series1;
	CUserItem::ItemSeries series2;
	CUserItem::ItemSeries series3;
	packet >> nType >> nDesItemID >> series1.llId >> series2.llId >> series3.llId;

	CUserBag& bag = m_pEntity->GetBagSystem();
	CUserItem* pItem1  = bag.FindItemByGuid(series1);
	CUserItem* pItem2  = bag.FindItemByGuid(series2);
	CUserItem* pItem3  = bag.FindItemByGuid(series3);

	if( pItem1 == NULL || pItem2 == NULL || pItem3 == NULL )
	{
		return;
	}

	//CScriptValueList paramList, retList;
	//CActor* pActor = (CActor*)m_pEntity;
	//pActor ->InitEventScriptVaueList(paramList,aeEquip);
	//paramList << (int)gEquipComposite;		//铭刻转换
	//paramList << nType;
	//paramList << nDesItemID;
	//paramList << pItem1;
	//paramList << pItem2;
	//paramList << pItem3;
	//pActor ->OnEvent(aeEquip,paramList,paramList);
}

//[幻武玄甲]设置幻武玄甲的外观是否显示
void CUserEquipment::HandSetExtraEquipShowFlag( CDataPacketReader & packet )
{
	if( !m_pEntity )
		return;

	CActor *pActor = (CActor*)m_pEntity;
	
	BYTE nExtraEquipType = 0;		//1-幻武，2-玄甲
	BYTE nIsShowAppear   = 0;		//1-显示，0-隐藏
	packet >> nExtraEquipType >> nIsShowAppear;

	if( nExtraEquipType == 1 )		//幻武
	{
		if( nIsShowAppear > 0 )
		{
			pActor->SetSocialMask(smHideExtraWeapon, false);		//显示幻武外观
		}
		else
		{
			pActor->SetSocialMask(smHideExtraWeapon, true);			//隐藏幻武外观
		}
	}
	else if( nExtraEquipType ==2 )		//玄甲
	{
		if( nIsShowAppear > 0 )
		{
			pActor->SetSocialMask(smHideExtraDress, false);			//显示玄甲外观
		}
		else
		{
			pActor->SetSocialMask(smHideExtraDress, true);			//隐藏玄甲外观
		}
	}
	else
	{
		return;
	}
	m_pEntity->CollectOperate(CEntityOPCollector::coRefFeature); //需要刷一下外观
}

/*
恢复装备强化失败损失的星级
*/
void CUserEquipment::HandRecoverStar(CDataPacketReader & packet)
{
}

/*
装备强化星级转移
*/
void CUserEquipment::HandTransferStar(CDataPacketReader & packet)
{
}

/*
装备铭刻
*/
void CUserEquipment::HandInscript( CDataPacketReader & packet )
{
}


/*
装备强化星级转移
*/
void CUserEquipment::HandTransferInscript(CDataPacketReader & packet)
{
}

/*
装备鉴定槽解锁
*/
void CUserEquipment::HandIdentifySlotUnlock( CDataPacketReader & packet )
{
}

/*
装备鉴定
*/
void CUserEquipment::HandIdentify( CDataPacketReader & packet )
{
}

/*
装备鉴定
*/
void CUserEquipment::HandTransferIdentify( CDataPacketReader & packet )
{
}

/*
装备吸收经验
*/
INT_PTR CUserEquipment::OnAbsorbExp(INT_PTR nExp)
{
	return 0;
}


void CUserEquipment::DealEquipmentAchieve()
{
	//num count
	// std::map<int, std::> eqList;
	// eqList.clear();
	std::vector<int> eqList;
	eqList.clear();
	for (INT_PTR i=0; i<EquipmentCount; ++i)
	{
		CUserItem * pUserItem = m_Items[i];
		if(pUserItem != NULL)
		{
			const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
			if (pStdItem != NULL)
			{
				// if(eqList.find(pStdItem->nItemlevel) != eqList.end())
				// 	eqList[pStdItem->nItemlevel]++;
				// else
				// 	eqList[pStdItem->nItemlevel] = 1;
				if( i < itEquipDiamondPos || i > itShieldPos)
					eqList.push_back(pStdItem->nItemlevel);
				
				if( i == itEquipDiamondPos)
				{
					m_pEntity->GetAchieveSystem().ActorAchievementComplete(nAchieveSoulJadeLv,1, pStdItem->nItemlevel);
				}
				m_pEntity->GetQuestSystem()->OnQuestEvent(CQuestData::qtEquipItem,1, pUserItem->wItemId);
			}
		}
	}
	while(eqList.size() > 0) {
		int nMinLv =  *(std::min_element(eqList.begin(), eqList.end()));
		std::vector<int>::iterator iter = eqList.begin();
		
		m_pEntity->GetAchieveSystem().ActorAchievementComplete(nAchieveEquipment, eqList.size(), nMinLv);
		m_pEntity->GetQuestSystem()->OnQuestEvent(CQuestData::qtEquip, eqList.size(), nMinLv);
		for(;iter != eqList.end();)
		{
			if(*iter == nMinLv) iter = eqList.erase(iter);
			else iter++;
		}
	}
}

void CUserEquipment::SuitAttrCal(CAttrCalc &cal)
{
	
	std::map<int, std::vector<int> > suitCal;//套装
	const CStdItemProvider& stdItemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	//遍历容器中的物品列表
	for (int i=0 ; i<EquipmentCount; i++)
	{
		CUserItem *pUserItem = m_Items[i];
		if(pUserItem ==NULL) continue;
		const CStdItem* pStdItem = stdItemProvider.GetStdItem(pUserItem->wItemId);
		if(pStdItem)
		{
			int nSuitType = (pStdItem->m_wSuitID/100) * 100;
			if(nSuitType <= 0)
				continue;
			suitCal[nSuitType].push_back(pStdItem->m_wSuitID);
		}
	}

	//身上存在多种套装加成
	std::vector<SuitAttr*> pSuitAttrs; //套装加成
	pSuitAttrs.clear();
	std::map<int, int> tempSuit;
	tempSuit.clear();
	tempSuit = m_nSuitList;
	m_nSuitList.clear();
	if(suitCal.size() > 0)
	{
		std::map<int, std::vector<int> >::iterator it = suitCal.begin();
		for(;it != suitCal.end(); it++)
		{
			std::vector<int> suits = it->second;
			while(suits.size() > 0)
			{
				int minSuitId = *(std::min_element(suits.begin(), suits.end()));
				if(minSuitId == 0)
					break;
				SuitAttr* pSuitAttr = GetLogicServer()->GetDataProvider()->GetStdItemProvider().getSuitPtrDataByKey(minSuitId, suits.size());
				if(pSuitAttr) {
					pSuitAttrs.push_back(pSuitAttr);
					int nkey = minSuitId * 100 +suits.size();
					m_nSuitList[nkey] = 1;
					std::map<int, int>::iterator it = tempSuit.find(nkey);
					if(it == tempSuit.end())
					{
						m_pEntity->SendTipmsgFormatWithId(tmActivateSuit, tstUI, pSuitAttr->name);
					}
				}
				std::vector<int>::iterator iter = suits.begin();
				for(;iter != suits.end();)
				{
					if(*iter == minSuitId) iter = suits.erase(iter);
					else iter++;
				}
			}
		}
	}

	for(std::map<int, int>::iterator it = tempSuit.begin(); it != tempSuit.end(); it++)
	{
		std::map<int, int>::iterator iter = m_nSuitList.find(it->first);
		if(iter == m_nSuitList.end())
		{
			int minSuitId = it->first/100;
			int num = it->first%100;
			SuitAttr* pSuitAttr = GetLogicServer()->GetDataProvider()->GetStdItemProvider().getSuitPtrDataByKey(minSuitId, num);
			if(pSuitAttr) {
				m_pEntity->SendTipmsgFormatWithId(tmInvalidSuit, tstUI, pSuitAttr->name);
			}
		}
	}

	if(pSuitAttrs.size() > 0)
	{
		for(int k = 0; k < pSuitAttrs.size(); k++)
		{
			SuitAttr* data = pSuitAttrs[k];
			if(data)
			{
				for(int j = 0; j < data->attri.nCount; j++)
				{
					if(data->nPercent > 0) {
						int i = data->attri.pAttrs[j].type;
						if(i < 0 || i >= GameAttributeCount) {
							continue;
						}
						if(AttrNeedCalc[i] == false) {
							continue;
						}

						switch(AttrDataTypes[i])
						{
						case adSmall://有符号1字节类型
							cal.m_AttrValues[i].nValue += (int)( cal.m_AttrValues[i].nValue * (data->nPercent/100.0) ); 
							break;
						case adUSmall://无符号1字节类型
							cal.m_AttrValues[i].nValue += (int)( cal.m_AttrValues[i].nValue * ( data->nPercent/100.0) ); 
							break;
						case adShort://有符号2字节类型
							cal.m_AttrValues[i].nValue += (int)( cal.m_AttrValues[i].nValue * (data->nPercent/100.0) ); 
							break;
						case adUShort://无符号2字节类型
							cal.m_AttrValues[i].uValue += (unsigned int)( cal.m_AttrValues[i].nValue * ( data->nPercent/100.0) ); 
							break;
						case adInt://有符号4字节类型
							cal.m_AttrValues[i].nValue += (int)( cal.m_AttrValues[i].nValue * (data->nPercent/100.0) );
							break;
						case adUInt://无符号4字节类型
							cal.m_AttrValues[i].uValue += (unsigned int)( cal.m_AttrValues[i].nValue * (data->nPercent/100.0) );
							break;
						case adFloat://单精度浮点类型值
							cal.m_AttrValues[i].fValue += (float)( cal.m_AttrValues[i].nValue * (data->nPercent/100.0) );
							break; 
						}	
					}else{
						cal << data->attri.pAttrs[j];
					}
				}
			}
		}
	}
}
//跨服数据
void CUserEquipment::SendMsg2CrossServer(int nType)
{
	if(m_pEntity ==NULL) return;
	
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = m_pEntity->GetId();
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	packet << m_pEntity->GetCrossActorId();
	packet << ((CActor*)m_pEntity)->GetAccountID();
	//数据
	size_t count = GetEquipCount();
	int pos = packet.getPosition();
	int nCount = 0;
	packet << (int)nCount  ; //玩家物品的数量
	if(count > 0)
	{
		for (INT_PTR i=0; i < EquipmentCount; i ++ )
		{
			if(m_Items[i])
			{
				nCount++;
				(CUserItem) *m_Items[i] >> packet;
			}
		}
		int* pCount = (int*)packet.getPositionPtr(pos);
        *pCount = nCount;
	}
	pCrossClient->flushProtoPacket(packet);
}


VOID CUserEquipment::OnCrossInitData(std::vector<CUserItem>& Equips)
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("装载玩家装备的数据出错,实体指针为MULL"));
		return;
	}
	//装载物品数据
	//装载装备信息
	{
		int nCount =Equips.size();
		// reader >> nCount;
		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();
		
		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem = Equips[i];
			CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
			
			if(pUerItem)
			{
				INT_PTR nPos =-1;
				const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUerItem->wItemId);
				
				if ( !pStdItem )
				{

					OutputMsg(rmError,_T("actorID=%d的玩家item=%d,装备非法"),m_pEntity->GetId(),pUerItem->wItemId);
					pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
					continue;
				}
				else
				{
					nPos = GetItemTakeOnPosition((Item::ItemType)pStdItem->m_btType,pUerItem->btHandPos);
					
					if(nPos <0)
					{
						OutputMsg(rmError,_T("actorID=%d的玩家nPos=%d,location=%d,itemId=%d,guid=%lld,装备位置非法"),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),nPos,pUerItem->btHandPos,pUerItem->wItemId,pUerItem->series);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
						continue;
					}
				

					if(nPos >=0 && m_Items[nPos] ==NULL)
					{
						OutputMsg(rmTip,_T("OnCrossInitData CUserEquipment nActorID:%d wItemId:%d,btQuality:%d,wStar:%d,cBestAttr:%s"),
						m_pEntity->GetId(),pUerItem->wItemId,pUerItem->btQuality,pUerItem->wStar,pUerItem->cBestAttr);
						m_Items[nPos] = pUerItem;
						EquipTakenInitTimer(nPos,pStdItem);
					}
					else
					{
						OutputMsg(rmError,_T("actorID=%u的玩家nPos=%d已经有装备了"),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID),nPos);
						pEngineer->DestroyUserItem(pUerItem);	//出错则删除内存防止泄露
						pUerItem = NULL;
					}
				}
			}
		}
		((CActor*)m_pEntity)->OnCrossFinishOneInitStep(MSG_CEQUIP); //完成一个步骤
	}
}