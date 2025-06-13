#include "StdAfx.h"
#include "HeroEquip.h"

//当装备穿戴到身上的时候，做一些发数据包到客户端
void CHeroEquip::AfterEquipTakenOn(CActor * pOnwer, INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation)
{
	if(m_nHeroId ==0)
	{
		OutputMsg(rmError ,"%s HeroId =0",__FUNCTION__);
		return ;
	}

	if(pOnwer->GetHandle().GetType() ==enActor)
	{
		//CActorPacket pack;
		//CDataPacket &data =  ((CActor *)pOnwer)->AllocPacket(pack);
		//data <<(BYTE)enHeroSystemId<<  (BYTE)sHeroTakenOnEquip << (BYTE)m_nHeroId << pUserItem->series.llId << (BYTE)nLocation;
		//pack.flush();
	}
}

//当装备从身上卸载的时候，做一些发数据包到客户端
void CHeroEquip::AfterEquipTakenOff(CActor * pOnwer,CUserItem *pUserItem)
{
	if(m_nHeroId ==0)
	{
		OutputMsg(rmError ,"%s HeroId =0",__FUNCTION__);
		return ;
	}
	
	if(pOnwer->GetHandle().GetType() ==enActor)
	{
		//CActorPacket pack;
		//CDataPacket &data =  ((CActor *)pOnwer)->AllocPacket(pack);
		//data <<(BYTE)enHeroSystemId<<  (BYTE)sHeroTakenOffEquip << (BYTE)m_nHeroId << pUserItem->series.llId;
		//pack.flush();
	}
}

void CHeroEquip::AfterEquipDirectRemove(CActor * pOnwer,CUserItem *pUserItem)
{
	if(pOnwer ==NULL || pUserItem ==NULL) return ;
	
	//pOnwer->GetHeroSystem().SetHeroEquipChange(); 
}
