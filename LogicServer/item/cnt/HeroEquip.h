#pragma once


/*
 宠物的装备，基于装备容器写的，重写了2个函数
*/

class CHeroEquip
	:public CEquipVessel
{
public:
	typedef CEquipVessel Inherited;

	CHeroEquip()
	{ 
		m_nHeroId=0;
	};
	~CHeroEquip(){};

	//设置宠物的ID
	void SetHeroId(INT_PTR nHeroId)
	{
		m_nHeroId = nHeroId;
	}

	//当装备穿戴到身上的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOn(CActor * pOnwer, INT_PTR nPos,CUserItem *pUserItem,INT_PTR nLocation=0);

	//当装备从身上卸载的时候，做一些发数据包到客户端
	virtual void AfterEquipTakenOff(CActor * pOnwer,CUserItem *pUserItem);

	//装备被直接爆掉
	virtual void AfterEquipDirectRemove(CActor * pOnwer,CUserItem *pUserItem);

private:
	INT_PTR m_nHeroId; //宠物的ID

};