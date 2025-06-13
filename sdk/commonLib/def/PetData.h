#pragma once
/****************************************************************************************/
/* 宠物的数据结构                                              
/****************************************************************************************/
#pragma  pack(push,4)
typedef struct tagPetDbData
{
	unsigned int nBaseAttack; // 基础的攻击资质
	unsigned int nBaseDefence;// 基础的防御资质
	unsigned int nBaseAgility; //基础的敏捷资质
	unsigned int nBasePhysique; //基础的体魄资质

	unsigned int nSmithAttack; //洗出来的攻击资质
	unsigned int nSmithDefence; //洗出来的防御资质
	unsigned int nSmithAgility; //洗出来的敏捷资质
	unsigned int nSmithPhysique; //洗出来的体魄资质
	unsigned int nExp;     //经验

	unsigned int nHp;      //当前的血  
	unsigned int nMp;      //当前的蓝


	BYTE         bQuality;   //宠物的品质
	BYTE         bStrong;    //强化
	BYTE         bCircle ;   //转数 
	BYTE         bLevel;     //等级

	BYTE         bAattackType; //攻击类型
	BYTE         bID;        //在玩家身上的标记ID，标记玩家的宠物
	WORD         wConfigId;  //配置表里的ID

	BYTE         bState;    //该宠物的状态，休息的,出战的，合体的
	BYTE         bIconId;   //图标的ID，用于宠物换皮
	WORD         wLoyalty ;  //忠诚度
	
	WORD         wSmithCount;		//洗资质的数量
	WORD         wWashMergeRate;     //洗出来的附体资质
	
	char         name[32];   //名字
	unsigned int nScore;               //宠物的评分
	
	BYTE         bSkillOpenSlot;       //技能使用道具开孔的个数
	BYTE         bReserver;   //保留
	WORD         wReserver;  //保留
	
	tagPetDbData()
	{
		memset(this,0,sizeof(*this));
	}
}PETDBDATA,*PPETDBDATA;
	
//宠物的技能的数据
typedef struct tagPetSkillData
{
	int nPetID;  //宠物在玩家身上的编号
	int nSkillID; //技能的id
	int nSlotId;  //技能的槽位的ID
	int nLevel;   //技能的等级
}PETSKILLDATA,*PPETSKILLDATA;

#pragma  pack (pop)