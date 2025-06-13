#pragma once


/*
 宠物的装备，基于装备容器写的，重写了2个函数
*/

class CPetEquip
	:public CEquipVessel
{
public:
	typedef CEquipVessel Inherited;

	CPetEquip()
	{ 
		m_nPetId=0;
	};
	~CPetEquip(){};

	//设置宠物的ID
	void SetPetId(INT_PTR nPetId)
	{
		m_nPetId = nPetId;
	}

	//当装备穿戴到身上的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOn(CAnimal * pOnwer, INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation=0);

	//当装备从身上卸载的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOff(CAnimal * pOnwer,CUserItem *pUserItem);

private:
	INT_PTR m_nPetId; //宠物的ID

};