#pragma once
///下面这里定义的是实体的属性的ID，脚本里需要用到,通过实体的id可以获取实体的属性

//tolua_begin

#define  ACHIEVE_ATTR_VALUE 1000

//下面这些是属性的偏移
enum enPropEntity
{
	PROP_ENTITY_ID= 								0, //实体的id
	PROP_ENTITY_POSX= 								1, //位置 posx
	PROP_ENTITY_POSY= 								2, //位置pos y
	PROP_ENTITY_MODELID= 							3, //实体的模型ID
	PROP_ENTITY_ICON=								4, //头像ID
	PROP_ENTITY_DIR=								5, //实体的朝向
	PROP_MAX_ENTITY=								6
};

enum enPropCreature
{
	PROP_CREATURE_LEVEL = 			PROP_MAX_ENTITY , //等级
	PROP_CREATURE_HP=								7, //血
	PROP_CREATURE_MP= 								8, //蓝
	PROP_CREATURE_STATE=							9, //实体的状态
	PROP_CREATURE_COLOR=							10, //实体的颜色
	//下面这些是战斗属性
	PROP_CREATURE_MAXHP=							11, //最大血，也就是生命
	PROP_CREATURE_MAXMP=							12,	//最大蓝，也就是内力
	PROP_CREATURE_PHYSICAL_ATTACK_MIN=				13,	//最小物理攻击
	PROP_CREATURE_PHYSICAL_ATTACK_MAX=				14,	//最大物理攻击
	PROP_CREATURE_MAGIC_ATTACK_MIN=					15,	//最小魔法攻击
	PROP_CREATURE_MAGIC_ATTACK_MAX=					16,	//最大魔法攻击
	PROP_CREATURE_WIZARD_ATTACK_MIN=				17,	//最小道术攻击
	PROP_CREATURE_WIZARD_ATTACK_MAX=				18,	//最大道术攻击
	PROP_CREATURE_PYSICAL_DEFENCE_MIN=				19,	//最小物理防御
	PROP_CREATURE_PYSICAL_DEFENCE_MAX=				20,	//最大物理防御
	PROP_CREATURE_MAGIC_DEFENCE_MIN=				21,	//最小魔法防御
	PROP_CREATURE_MAGIC_DEFENCE_MAX=				22,	//最大魔法防御
	PROP_CREATURE_HITVALUE=							23,	//物理命中
	PROP_CREATURE_DODVALUE=							24,	//物理闪避
	PROP_CREATURE_MAGIC_HITRATE=					25,	//魔法命中
	PROP_CREATURE_MAGIC_DOGERATE=					26,	//魔法闪避
	PROP_CREATURE_HP_RATE_RENEW=					27,	//HP万分比恢复
	PROP_CREATURE_MP_RATE_RENEW=					28,	//MP万分比恢复
	PROP_CREATURE_MOVEONESLOTTIME=					29,	//移动1格需要的时间，单位ms
	PROP_CREATURE_ATTACK_SPEED=						30,	//攻击速度
	PROP_CREATURE_LUCK=								31,	//幸运
	PROP_CREATURE_HP_RENEW=							32,	//HP值恢复
	PROP_CREATURE_MP_RENEW=							33,	//MP值恢复
	PROP_CREATURE_DIZZY_STATUS=						34,	//麻痹
	PROP_MAX_CREATURE=								35
};

//怪物的
enum enPropMonster
{
	PROP_MONSTER_BORNPOINT = PROP_MAX_CREATURE	, 	// 怪物的出生点
	PROP_MONSTER_LEVEL_DIFF=					36,	// 小队平均等级与副本推荐的等级差
	PROP_MONSTER_INITHPPERCENT=					37, // 怪物出生的血量比例
	PROP_MONSTER_LIVE_TIMEOUT=					38,	// 怪物生命到期时间（0-无生命期限）
	PROP_MONSTER_CIRCLE=						39, // 转生次数
	PROP_MONSTER_BELONG_ID=						40,	// 怪物归属Id
	PROP_MAX_MONSTER,
};

/*
增加定义，需要在tagActorDbPardData增加数据
*/

enum enPropActor
{
	//============不存DB==================
	PROP_ACTOR_MONSTER_MODELID=				PROP_MAX_CREATURE, //怪物模型id
	PROP_ACTOR_TEAMFUBEN_OUTPUT=			36, //团队副本输出伤害
	PROP_ACTOR_TEAMFUBEN_TEAMID=			37, //团队副本(团队ID)
	PROP_ACTOR_TEAMFUBEN_FBID=				38, //团队副本(副本ID)
	PROP_ACTOR_DEDUCT_DAMAGE=				39, //伤害减免
	PROP_ACTOR_ABSORB_DAMAGE=				40, //伤害吸收
	PROP_ACTOR_WEAPONAPPEARANCE=			41, //武器外观
	PROP_ACTOR_SWINGAPPEARANCE=				42, //套装外观 
	PROP_ACTOR_SOLDIERSOULAPPEARANCE=		43, //时装外观(高低16位2个word,存2个外观)
	PROP_ACTOR_WEAPON_ID=					44, //正在装备的武器物品id
	//============存DB==================
	PROP_ACTOR_PK_MOD=						45, //玩家的PK模式
	PROP_ACTOR_SEX=							46, //性别
	PROP_ACTOR_VOCATION=					47, //职业
	PROP_ACTOR_EXP=							48, //经验 这个多4个字节,uint64的
	PROP_ACTOR_EXP_=						49,
	PROP_ACTOR_PK_VALUE =					50, //玩家的pk值(杀戮值)
	PROP_ACTOR_BAG_GRID_COUNT=				51, //背包的格子数量
	PROP_ACTOR_MONTH_CARD=					52, // 月卡到期时间
	PROP_ACTOR_MEDICINE_CARD=				53, // 大药月卡到期时间
	PROP_ACTOR_BIND_COIN=					54, //绑定金钱
	PROP_ACTOR_COIN=						55, //非绑定金钱
	PROP_ACTOR_BIND_YUANBAO=				56, //绑定元宝
	PROP_ACTOR_YUANBAO=						57, //非绑定元宝
	PROP_ACTOR_EVIL_PK_STATUS=				58, //恶意PK状态
	PROP_ACTOR_GUILD_ID =					59, //帮派的ID
	PROP_ACTOR_TEAM_ID=						60, //队伍的ID
	PROP_ACTOR_SOCIALMASK=					61, //社会关系的mask，是一些bit位合用的 //怪物保存虚拟的bossid  用于boss 刷新
	PROP_ACTOR_GUILDEXP=					62, //玩家个人当前的贡献度
	PROP_ACTOR_GM_LEVEL=					63, //gm等级
	PROP_ACTOR_DEFAULT_SKILL=				64, //玩家的默认技能的ID
	PROP_ACTOR_MULTI_EXP_UNUSED=			65, //剩余多倍经验，64位的
	PROP_ACTOR_MULTI_EXP_UNUSED_=			66,
	PROP_ACTOR_MERITORIOUSPOINT	=			67, //累计功勋(玩家的成就点)
	PROP_ACTOR_CURCUSTOMTITLE=				68, //当前自定义称号
	PROP_ACTOR_MERITORIOUS_DAY=				69, //每日功勋
	PROP_ACTOR_FOREVER_CARD_CLAG=			70, // 免费特权(第0位)永久卡(第1位)首充会员(2位) 色卡(3-7)
	PROP_ACTOR_ACTIVITY=					71, //玩家的活跃度
	PROP_ACTOR_DRAW_YB_COUNT=				72, //提取元宝数目
	PROP_ACTOR_BATTLE_POWER=				73, //玩家的战力
	PROP_ACTOR_RECOVERSTATE=				74, //回收状态
	PROP_ACTOR_LOOT_PET_ID=			        75, //当前宠物id
	PROP_ACTOR_LASTLOGIN_OPEN_DAY_NUM=		76, //上次登录时的开服天数
	PROP_ACTOR_PERSONBOSS_JIFEN=			77, //个人BOSS积分(开服活动)
	PROP_ACTOR_DEPORT_GRID_COUNT=			78, //仓库的格子数目
	PROP_ACTOR_CIRCLE=						79, //转生等级		
	PROP_ACTOR_CIRCLE_SOUL=					80, //转生修为
	PROP_ACTOR_ANGER=						81, //怒气		低两位表示当前怒气值 高两位表示最大怒气值
	PROP_ACTOR_CHECKINS=					82, //签到次数
    PROP_ACTOR_Prestige=					83, //声望
	PROP_ACTOR_Bless=						84, //当前祝福值
	PROP_ACTOR_CARD_AWARD_FLAG=				85, // 免费特权(第0位)/月卡(第1位)/大药(第2位)/永久卡(第3位)首充会员(4位) 色卡(5-9)的当日领取标记
	PROP_ACTOR_TOTAL_ONLINE_MIN=			86, //总在线时间，单位：分钟
	PROP_ACTOR_WINGPOINT=					87, //翅膀幸运值  nWingPoint
	PROP_ACTOR_CURNEWTITLE=					88, //当前头衔	
	PROP_ACTOR_DIMENSIONALKEY=				89, //次元钥匙
	PROP_ACTOR_89_=							90,
	PROP_ACTOR_DEFAULTLOOTPETDIS = 			91, //
	PROP_ACTOR_91_ = 						92,
	PROP_ACTOR_REBATE_DRAW_YB_COUNT = 		93, // 返利卡提取元宝数目
	PROP_ACTOR_93_ = 						94,
	PROP_CROSS_SERVER_ACTORID =				95, //跨服玩家id
	PROP_ACTOR_OFFICE=						96, //官职
	PROP_ACTOR_DEPOT_COIN=					97, //仓库金币
	PROP_ACTOR_SUPPER_PLAY_LVL=				98, //超玩VIP等级
	PROP_ACTOR_FRENZY=						99, //狂暴状态
	PROP_ACTOR_BE_KILLED_COUNT=				100, //被杀总次数
	PROP_ACTOR_KILL_MONSTER_COUNT=			101, //杀怪总数 
	PROP_ACTOR_FLYSHOES =                   102, //飞鞋数量
	PROP_ACTOR_BROATNUM =                   103, //喇叭数量
	PROP_ACTOR_RECYCLE_POINTS =             104, //回收积分
	PROP_ACTOR_MERIDIALv    =               105, //经脉等级
	PROP_ACTOR_106_  =              		106, 

	//存db的属性跟tagActorDbPardData的字段按顺序对应,请同步修改！
	//=========不存DB=========
	PROP_ACTOR_NEXT_SKILL_FLAG=				107, //下一次技能标记（标记战士的下一次附加技能：攻杀，刺杀，半月，烈火，逐日）
	PROP_ACTOR_ZY                              , //阵营类型
	PROP_ACTOR_CRIT_DAMAGE, //新版暴击最终造成的伤害值
	PROP_ACTOR_CRIT_RATE, //新版暴击几率(万分比)
	PROP_ACTOR_CRIT_POWER, //新版暴击力(int)
	PROP_ACTOR_DEDUCT_CRIT, //抗暴
	PROP_ACTOR_MEDIC_RENEW, //药品恢复倍率增益（1为100%）
	PROP_ACTOR_AREA_ATTR, //区域属性存储位
	PROP_ACTOR_AREA_ATTR_0, //
	PROP_ACTOR_GOLDEQ_ATTR1, //降低受战士伤害值
	PROP_ACTOR_GOLDEQ_ATTR2, //降低受法师伤害值
	PROP_ACTOR_GOLDEQ_ATTR3, //降低受道士伤害值
	PROP_ACTOR_GOLDEQ_ATTR4, //降低受怪物伤害值	
	PROP_ACTOR_GOLDEQ_ATTR5, //增加对战士伤害值
	PROP_ACTOR_GOLDEQ_ATTR6, //增加对法师伤害值
	PROP_ACTOR_GOLDEQ_ATTR7, //增加对道士伤害值
	PROP_ACTOR_GOLDEQ_ATTR8, //增加对怪物伤害值
	PROP_ACTOR_CRIT_MUTRATE, //新版暴击倍率(万分比)
	PROP_ACTOR_BANCHAT,//禁言
	PROP_ACTOR_PET_STATUS,//宝宝状态
	PROP_ACTOR_DAMAGEBONUS,//伤害加成
	PROP_ACTOR_IGNORDEFENCE,//无视防御
	PROP_ACTOR_SUCKBLOOD,//吸血
	PROP_ACTOR_EXP_POWER,//经验倍率
	PROP_ACTOR_LOOTBINDCOIN,//金币倍率
	PROP_ACTOR_CUT,//切割
	PROP_ACTOR_NORMAL_ATK_ACC,//普攻加速万分比
	PROP_ACTOR_HP_RATE,//HP加成万分比
	PROP_ACTOR_PK_DEDUCT_DAMAGE,//pk伤害减免万分比
	PROP_ACTOR_HP_2_MP_DAMAGE,//护身
	PROP_ACTOR_ZL_MONEY,//战令金币
	PROP_ACTOR_CURREVIVEDURATION,//当前复活特权持续时间
	PROP_ACTOR_HALFMONTHS_INCREASEDAMAGE,		 //半月增伤
	PROP_ACTOR_FIRE_INCREASEDAMAGE,			 	 //烈火增伤
	PROP_ACTOR_DAYBYDAY_INCREASEDAMAGE,			 //逐日增伤
	PROP_ACTOR_ICESTORM_INCREASEDAMAGE,			 //冰咆哮增伤
	PROP_ACTOR_FIRERAIN_INCREASEDAMAGE,			 //火雨增伤
	PROP_ACTOR_THUNDER_INCREASEDAMAGE,			 //雷电增伤
	PROP_ACTOR_BLOODBITE_INCREASEDAMAGE,		 //噬血术增伤
	PROP_ACTOR_FIRESIGN_INCREASEDAMAGE,			 //火符增伤
	PROP_ACTOR_HALFMONTHS_REDUCEDAMAGE,		 	 //半月减伤
	PROP_ACTOR_FIRE_REDUCEDAMAGE,			 	 //烈火减伤
	PROP_ACTOR_DAYBYDAY_REDUCEDAMAGE,			 //逐日减伤
	PROP_ACTOR_ICESTORM_REDUCEDAMAGE,			 //冰咆哮减伤
	PROP_ACTOR_FIRERAIN_REDUCEDAMAGE,			 //火雨减伤
	PROP_ACTOR_THUNDER_REDUCEDAMAGE,			 //雷电减伤
	PROP_ACTOR_BLOODBITE_REDUCEDAMAGE,			 //噬血术减伤
	PROP_ACTOR_FIRESIGN_REDUCEDAMAGE,			 //火符减伤
	
	PROP_ACTOR_CREAT_TIME_MIN,			 		//创建时间155
	PROP_ACTOR_TRADINGQUOTA,			 		//交易额度(unsigned long long)156
	PROP_ACTOR_TRADINGQUOTA_,			 		//交易额度157
	PROP_ACTOR_GUILD_LEVEL,						//行会等级158
	PROP_ACTOR__159,							//目前暂未使用
	//!!!存DB的属性不要放这里
	///这里加了属性， tagACTORTAILDATA 结构需要增加对应的字段，切记!
	PROP_MAX_ACTOR,
};
//tolua_end
