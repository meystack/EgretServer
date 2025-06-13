#include "StdAfx.h"
#include "PetEquip.h"

//当装备穿戴到身上的时候，做一些发数据包到客户端
void CPetEquip::AfterEquipTakenOn(CAnimal * pOnwer, INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation)
{
	if(m_nPetId ==0)
	{
		OutputMsg(rmError ,"%s petId =0",__FUNCTION__);
		return ;
	}
	/*
	if(pOnwer->GetHandle().GetType() ==enActor)
	{
		CActorPacket pack;
		CDataPacket &data =  ((CActor *)pOnwer)->AllocPacket(pack);
		data <<(BYTE)enPetSystemID<<  (BYTE)sPetTakenOnEquip << (BYTE)m_nPetId << pUserItem->series.llId;
		pack.flush();
	}
	*/

}

//当装备从身上卸载的时候，做一些发数据包到客户端
void CPetEquip::AfterEquipTakenOff(CAnimal * pOnwer,CUserItem *pUserItem)
{
	if(m_nPetId ==0)
	{
		OutputMsg(rmError ,"%s petId =0",__FUNCTION__);
		return ;
	}
	/*
	if(pOnwer->GetHandle().GetType() ==enActor)
	{
		CActorPacket pack;
		CDataPacket &data =  ((CActor *)pOnwer)->AllocPacket(pack);
		data <<(BYTE)enPetSystemID<<  (BYTE)sPetTakenOffEquip << (BYTE)m_nPetId << pUserItem->series.llId;
		pack.flush();
	}
	*/

}