#pragma once

/************************************************************************/
/*
/*                         游戏角色BUFF对象类
/*
/*    类中包含了BUFF的数据定义以及BUFF的类型定义。buff由类型、分组、剩余作用次数以及
/* 作用值构成。
/*
/*    buff类型：决定buff对角色产生的作用的类型，例如体力上限提升、攻击力提升等；
/*    buff分组：相同类型的buff允许存在多个，单个不同buff实例时间具有不同的分组，对一个
/*             buff的定位是通过buff的类型以及分组进行的。因此不允许存在相同类型以及相
/*             同分组的buff；
/*    作用次数：一个buff产生后，将在一定的时间后自动消失。buff存在的时间也是buff作用的
/*            次数，buff每秒钟对角色产生一次作用；
/*    作用值：不同类型的buff可能具有不同的值，值将表示此类型的buff对角色的作用影响的度，
/*          例如同为体力上限提升的buff，值为100则表示增加100体力上限，值为-300则表示
/*          减少体力上限300。对于Add类型的属性buff，可以使用负数来产生有损buff，对于
/*          Power类型的属性buff，0.1表示提升10%，也可以使用负数来产生有损buff；
/*
/************************************************************************/

class CDynamicBuff
{
	friend class CBuffSystem;
	friend class CCampBuffMgr;
public:
	/** 定义buff的分组归类 **/
	enum BuffGroup
	{
		bgSystemBuff = 0,		//系统默认的BUFF组
		
		bgUserBuffMin = 32,		//可以由开发者自定义的BUFF组的起始值
		bgSkillBuffMin =bgUserBuffMin, //技能的最小的group
		bgSkillBuffMax=80,            //技能的最大的group

		bgItemBuffMin ,       //物品的最小的组
		bgUserBuffMax = 127,	//可以由开发者自定义的BUFF组的结束值
		bgItemBuffMax =bgUserBuffMax,       //物品组的最大值

		bgGuildBuffMin,			//行会buff最小的组
		bgGuildBuffMax = 200,	//行会buff结束值

		/*如果添加新的buff组请在AnyBuffGroup枚举值前增加*/
		AnyBuffGroup = -1,		//用于匹配任何组中的buff
		MaxBuffGroup = 300,		//buff分组最大值
	};
	/** 定义buff的最长作用时间（单位是秒），也是无时限buff的作用次数值 **/
	static const INT_PTR Infinite = 0xFFFFFFFF;
	static const INT_PTR MaxBuffNameLength = 48; //Buff的名字

	//获取上一个BUFF
	inline CDynamicBuff* GetPrevBuff()
	{
		return pPrev;
	}
	/// 获取buff的的剩余时 间，单位秒
	inline int GetRemainTime(){
		return ( dwTwice -1 ) *  pConfig->nInterval  +   wActWait;
	}

	WORD GetId(){return wBuffId;};
public:
	WORD			wBuffId;	//BUFFID;
	WORD			wActWait;	//下次作用剩余时间，单位是秒
	DWORD			dwTwice;	//剩余作用次数，每秒btInterval次，每作用一次此值减少1。值为0的时候buff应当消失。值为CDynamicBuff::Infinite的时候buff永远有效
	GAMEATTRVALUE	value;		//buff的值
	UINT32			mCreateTime; // Buff添加时间MiniDateTime格式
	EntityHandle	mGiver;		// Buffer施法者
	CBuffProvider::BUFFCONFIG * pConfig;  //buff的标准配置
	bool			bSetVest;	//是否设置攻击归属
	BYTE			btType;		//buff类型，值为CDynamicBuff::BuffType中的枚举值
	BYTE			btGroup;	//buff分组，分组的意义在于可以同时拥有多个相同类型的buff
	WORD            wAuxParam;   //辅助的参数,用于一些特殊场合
	WORD			wInterval;	//buff作用周期，单位是秒	
	CHAR            sBuffName[MaxBuffNameLength]; //Buff的名字
	int             m_nBuffCd = 0;//buff下次生效的cd 短时间戳

private:
	CDynamicBuff*	pPrev;		//上一个buff。buff是作为链表的结构进行管理的

};
