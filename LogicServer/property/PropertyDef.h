/*这里是各种实体的属性的定义
*定义规则是:
*   为了客户端的解析麻烦,不要使用word,byte等1,2个字节的长度
*	整数(有符号,无符号)都用n开头,比如nSex 
*   长整形用l开头,比如lExp
*   buff用 a开头(array),比如int aUserQuest[64];
*/

#pragma once
#pragma  pack(4)



typedef struct tagEntityData  
{
	unsigned int    nID;   		// 玩家的actorid或者怪物ID
	int       		nPosX;      // 位置x
	int       		nPosY;      // 位置y
	int  			nModelId;  	// 模型ID
	unsigned int  	nIcon;  	// 头像ID
	int       		nDir;		// 实体的朝向
}ENTITYDATA, *PENTITYDATA;


typedef struct tagCreatureData
{ 
	unsigned int nLevel; 	//等级
	unsigned int nHp;  		//当前血
	unsigned int nMp; 		//当前蓝
	unsigned int nState;    //实体当前的状态
	unsigned int nColor;  	//实体的颜色

	unsigned int nMaxHp; 	//最大血(生命)
	unsigned int nMaxMp; 	//最大蓝(内力)

	int nPhysicalAttackMin; //最小物理攻击
	int nPhysicalAttackMax; //最大物理攻击

	int nMagicAttackMin;	//最小魔法攻击
	int nMagicAttackMax;	//最大魔法攻击

	int nWizardAttackMin;	//最小道术攻击
	int nWizardAttackMax;	//最大道术攻击

	int nHysicalDefenceMin; //最小物理防御
	int nHysicalDefenceMax; //最大物理防御

	int nMagicDefenceMin; 	//最小魔法防御
	int nMagicDefenceMax; 	//最大魔法防御

	int    nHitrate;		//物理命中
	int    nDogerate;		//物理闪避
	
	int   nMagicHitRate;	//魔法命中
	int   nMagicDogerate; 	//魔法闪避

	unsigned int nHpRateRenew;	//生命万分比恢复
	unsigned int nMpRateRenew;	//魔法万分比恢复

	unsigned int nMoveOneSlotTime;	//移动1格需要的时间，也就是移动速度,如果是0表示不能移动
	unsigned int nAttackSpeed;		//增加了攻击速度

	int     nLuck;			//幸运

	int		nHpRenew;		//生命恢复
	int 	nMpRenew;		//魔法恢复

	int   	n34;
	
}CREATUREDATA,*PCREATUREDATA;

//怪物的基本数据
typedef struct tagMonsterData
{
	int nBornPointID;			// 怪物的出生点
	int nLevelDifference;		// 小队平均等级与副本推荐的等级差（用在副本根据组队玩家等级来设置怪物攻击、防御相关属性）
	int nHpRenewValue;			// 怪物出生的血量比例
	int nLiveTime;				// 怪物生命到期时间（0-无生命期限）	
	int nCircle;				// 转生
	unsigned int nMonsterBeLongId;		// 怪物归属Id
} MONSTERDATA, *PMONSTERDATA;

//这些都是玩家的数据，但是不存盘
typedef struct tagActorHeadPartData
{
	unsigned int nMonsterModleId;	//怪物模型id

	int   nTeamOutput; 				//团队副本输出伤害
	int   nTeamID; 					//团队副本(团队ID)
	int   nTeamFBID; 				//团队副本(副本ID)

	float	fDecuctDamage;		//伤害减免
	float	fAbsorbDamage;		//伤害吸收

	int  nWeaponAppearance; 				//武器的外观
	int  nSwingAppearce;					//翅膀的外观
	unsigned int nSoldierSoulAppearance;  	//兵魂外观
	unsigned int nWeaponId;			//武器物品id
}ACTORHEADPARTDATA,*PACTORHEADPARTDATA;


//人和英雄的共有的数据
typedef struct tagActorHeroCommonData
{
	unsigned int nSex;		//男女
	unsigned int nVocation; //  职业
	unsigned int nBattlePower;   //玩家的战力	
	unsigned int nCircle;			//转生次数
}ACTORHEROCOMMONDATA,*PACTORHEROCOMMONDATA;

typedef struct tagActorDbPardData
{
	unsigned int nPkMod;		 		//玩家的pk模式
	unsigned int nSex;					//性别
	unsigned int nVocation; 			//职业
	unsigned long long lExp;    		//经验
	int nPKValue;						//玩家的pk值(杀戮值)
	unsigned int nBagGridCount;			//背包的格子数量
	unsigned int nMonthCardTime; 		//月卡到期时间
	unsigned int nMedicineCardTime;		//大药月卡到期时间
	unsigned int nBindCoin;				//绑定金钱
	unsigned int nNonBindCoin; 			//非绑定金钱 --金币
	unsigned int nBindYuanbao; 			//绑定元宝
	unsigned int nNonBindYuanbao; 		//非绑定元宝 -银两
	unsigned int nEvilPkStatus;			//恶意PK状态
	unsigned int nGuildID;				//帮派的ID
	unsigned int nTeamID;				//队伍的ID
	unsigned int nSocialMask;			//社会关系的mask，是一些bit位合用的
	unsigned int nGuildExp;				//玩家个人当前的贡献度
	unsigned int nGmLevel;				//gm等级
	unsigned int nDefaultSkillID;		//玩家的默认技能的ID
	LONGLONG lMultiExpUnused;  			//未使用的多倍经验，64位的
	int achievepoint;					//累计功勋(玩家的成就点)	
	unsigned int nCurCustomTitle;		//当前自定义称号
	unsigned int nMeritoriousDay; 		//每日功勋
	unsigned int nForeverCardFlag;		//免费特权(第0位)及永久卡(第1位)的开启标志
	unsigned int nActivity;      		//玩家的活跃度
	unsigned nDrawYbCount;   			//提取元宝数目
	unsigned int nBattlePower;   		//玩家的战力	
	unsigned int nRecoverState;		//转生阶
	unsigned int  nLootPetId;		//宠物id
	unsigned int nLastLoginOpendayNo_; 	//上次登录是开服的第几天
	unsigned int nPersonBossJifen;		//个人BOSS积分(开服活动)
	unsigned int nDeportGridCount ; 	//仓库的格子数目（原始大小+ 购买大小）
	unsigned int nCircle;				//转生次数
	unsigned int nCircleSoul;			//转生灵魄
	unsigned int nAnger;				//怒气		低两位表示当前怒气值 高两位表示最大怒气值
	unsigned int nCheckIns;				//签到次数
	unsigned int nPrestige;				//声望
	unsigned int nBless;				//当前祝福值
	unsigned int nCardAwardFlag;		//免费特权(第0位)/月卡(第1位)/大药(第2位)/永久卡(第3位)的当日领取标记
	unsigned int nTotalOnlineMin;		//总在线时间，单位：分
	unsigned int nWingPoint;			//翅膀幸运值  nWingPoint
	unsigned int nCurnewTitel;			//当前头衔
	unsigned long long nDimensionalKey;	//次元钥匙
	unsigned long long nDefaultLootPetDis;//默认捡取拾取宠物的捡取距离
	unsigned long long nRebateDrawYbCount;// 返利卡提取元宝数目
	unsigned int nCSActorId;				//跨服玩家id
	unsigned int nOffice;				//仓库元宝
	unsigned int nDepotCoin;			//仓库金币
	unsigned int nSupperPlayLvl;		//超玩VIP等级
	unsigned int nFrenzy;				//狂暴状态（0/1）
	unsigned int nBeKilledCount;		//被杀总次数
	unsigned int nKillMonsterCount;		//杀怪总数
	unsigned int nFlyShoes; 			//飞鞋
	unsigned int nBroatNum; 			//喇叭
	unsigned int nRecyclepoints; 		//回收积分
	unsigned int nMeridianLv; 			//经脉等级
	unsigned int n_106;
}ACTORDBPARTDATA,*PACTORDBPARTDATA;


//这些属性不存盘的，是玩家的动态属性
typedef struct tagACTORTAILDATA
{
	int			 nNextSkillFlag;	//下一次技能标记（标记战士的下一次附加技能：攻杀，刺杀，半月，烈火，逐日）
	int          nZy;               //阵营类型
	unsigned int nCritDamage;		//新版暴击最终造成的伤害值
	unsigned int nCritRate;			//新版暴击几率(万分比)
	int			 nCritPower;		//新版暴击力(int)
	float		 fDamagePower;		//抗暴
	float		 fDMedicanceRenew;	//药品恢复增益（1为100%）
	unsigned int nAreaAttr; 		//区域属性存储位
	unsigned int nAreaAttr_0;
	unsigned int nGoldAttr_1; //新加属性
	unsigned int nGoldAttr_2;
	unsigned int nGoldAttr_3;
	unsigned int nGoldAttr_4;
	unsigned int nGoldAttr_5;
	unsigned int nGoldAttr_6;
	unsigned int nGoldAttr_7;
	unsigned int nGoldAttr_8;
	unsigned int nCritMutRate;			//新版暴击倍率几率(万分比)
	unsigned int nBanChat;//禁言时间
	unsigned int nPetStatus;//宠物状态
	unsigned int nDamageBonus;//伤害加成
	unsigned int nIgnorDefence;//无视防御
	unsigned int nSuckBlood;//吸血
	unsigned int nExpPower;//经验倍率
	unsigned int nLootBindCoin;//金币倍率
	unsigned int nCutting;//切割
	unsigned int nNormalAttackAcc;//普攻加速度万分比
	unsigned int nHpAddRate;//血量加成万分比
	unsigned int nPkDecuctDamage;//pk减免万分比
	unsigned int nHp2MpDamage;//护身
	unsigned int nZLMoney;//战令币
	int 		 nCurReviveDuration;//当前复活特权持续时间
	unsigned int nHalfMonthsIncreaseDamage;		//半月增伤
	unsigned int nFireIncreaseDamage;			//烈火增伤
	unsigned int nDayByDayIncreaseDamage;		//逐日增伤
	unsigned int nIceStormIncreaseDamage;		//冰咆哮增伤
	unsigned int nFireRainIncreaseDamage;		//火雨增伤
	unsigned int nThunderIncreaseDamage;		//雷电增伤
	unsigned int nBloodBiteIncreaseDamage;		//噬血术增伤
	unsigned int nFireSignIncreaseDamage;		//火符增伤
	unsigned int nHalfMonthsReduceDamage;		//半月减伤
	unsigned int nFireReduceDamage;				//烈火减伤
	unsigned int nDayByDayReduceDamage;			//逐日减伤
	unsigned int nIceStormReduceDamage;			//冰咆哮减伤
	unsigned int nFireRainReduceDamage;			//火雨减伤
	unsigned int nThunderReduceDamage;			//雷电减伤
	unsigned int nBloodBiteReduceDamage;		//噬血术减伤
	unsigned int nFireSignReduceDamage;			//火符减伤

	unsigned int nCreateTime;
	unsigned long long nTradingQuota; 		//交易额度
	unsigned int nGuildLevel;				//行会等级
	unsigned int n_159;						//目前暂未使用
}ACTORTAILDATA, PACTORTAILDATA;



//怪物或者宠物的三级战斗属性
typedef struct tagCreatureBattleData :public CREATUREDATA
{ 
	int _53; 	//伤害抵消的BUFF状态
	int _54;  		//伤害减免
	int _55; 		//麻痹，不可移动，不可释放技能
	int _56;    //经验增加一个数值
	int _57;  	//治愈的BUFF状态

	int _58; 	//隐身状态
	int _59; 	//经验的增加倍率，万分比

	int _60; //冻结，只能走，不能跑
	int _61; //pk保护状态

	int _62;	// 真·冻结，不能走不能跑
	int _63;	//持续回蓝的BUFF状态

	int _64;	//按比例将MP抵消HP
	int _65;	//麻痹概率，万分比

	int nDeductDizzyRate; //抗麻痹概率，万分比
	int _67; //麻痹时长增加

	int nDizzyTimeAbsorbAdd; 	//麻痹时长减免 68
	tagCreatureBattleData()
	{
		memset(this, 0, sizeof(*this));
	}
}CREATURBATTLEEDATA,*PCREATURBATTLEEDATA;


#pragma  pack()
