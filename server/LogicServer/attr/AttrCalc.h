#pragma once

/************************************************************************/
/*
/*                          游戏属性计算器类
/*
/*   用于在角色、物品、BUFF等系统中计算属性值。通过使用重载的+运算符增加属性或属性值。
/* 计算器能够处理属性值运算过程中的溢出错误并在溢出发生时为属性值设定为指定数据类型的最小
/* 值或最大值。
/*
/************************************************************************/

class CAttrCalc
{
public:
	CAttrCalc();
	/* 重置属性计算器 */
	inline void reset(){ memset(m_AttrValues, 0, sizeof(m_AttrValues)); }
	
	//************************************
	// Qualifier 自身的立即属性 += (自身的倍率属性)(1+baseAttr的倍率属性)
	// 同时自身的倍率属性清0
	// Parameter:  CAttrCalc & baseAttr，基础计算器，比如以装备计算倍率的话就是装备的计算器
	//返回自己
	//************************************
	CAttrCalc&  applyPowerAttributes(const CAttrCalc& baseAttr);

	/* 向属性计算器中增加一个属性，相同属性的值将会叠加到一起 */
	CAttrCalc& operator << (const tagGameAttribute &attr);
	/* 向属性计算器中增加一个紧凑属性，相同属性的值将会叠加到一起 */
	CAttrCalc& operator << (const tagPackedGameAttribute &attr);
	/* 向属性计算器中增加另一个属性计算器中的所有属性的值*/
	CAttrCalc& operator << (const CAttrCalc &attrCalc);

	//所有的属性都乘以一定比例
	CAttrCalc& operator *= (float rate);
		
	GAMEATTRVALUE	m_AttrValues[GameAttributeCount];	//属性集定义
};
