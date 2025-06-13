#pragma once
/****************************************************************************************/
/* 英雄的数据结构                                              
/****************************************************************************************/
#pragma  pack(push,4)
typedef struct tagHeroDbData
{
	BYTE         bStage;          //阶
	BYTE		 bReserve3;		  //保留字3
	WORD		 wReserve4;		  //保留字4
	int          nBodyforce;      //体力
	int          nBless;          //祝福
	unsigned long long  nExp;     //经验
	unsigned int nHp;             //当前的血  
	unsigned int nMp;             //当前的蓝
	BYTE         bCircle ;        //转数 
	BYTE         bLevel;          //等级
	BYTE         bVocation;       //职业
	BYTE         bSex;            //性别
	BYTE         bID;             //英雄ID
	BYTE         bState;          //休息的,出战的，合体的
	WORD         wIconId;         //图标的ID，用于换头像
	unsigned int nScore;          //评分
	unsigned int nModel;          //模型
	unsigned int nWeaponAppear;   //武器的外观
	unsigned int nSwingAppear;    //翅膀的外观
	int          nReserver1;
	int          nReserver2;
	char         name[32];        //名字
	tagHeroDbData()
	{
		memset(this,0,sizeof(*this));
	}
}HERODBDATA,*PHERODBDATA;
	
//英雄技能结构
typedef struct tagHeroSkillData
{
	int nHeroID;   //英雄ID
	int nSkillID;    //技能的id
	int nSlotId;     
	int nLevel;     //技能的等级
	BYTE bIsClose;   
	BYTE bReserver;
	WORD wReserver;
	unsigned int nExp;		
	unsigned int nNextLevelExp;	
	tagHeroSkillData()
	{
	    memset(this,0,sizeof(*this));
	}
}HEROSKILLDATA,*PHEROSKILLDATA;

#pragma  pack (pop)