#pragma once

//tolua_begin
//地图区域属性的定义
//完全搬战将的过来，有些未必用得到,先定义
//注意：在配置文件中，attri字段里的type对应下面的值，如aaSaft，而value根据type的值不同，会需要配置不同的值，有些是配一个整数，有些是整数列表（多个整数），有些
//有时不需要配置value
enum tagMapAreaAttribute
{
	aaNoAttri				= 0,			//无意义
	aaSaft					= 1,			//安全区
	aaAddBuff				= 2,			//增加buff				参数一个或多个：buffId [1,2,3...n]
	aaWar					= 3,			//战斗区：杀人不犯罪
	aaReloadMap				= 4,			//重配地图：玩家在此属性地图上线，会被传送到之前未配置重配地图属性的场景中。
	aaCrossMan				= 5,			//允许穿人
	aaCrossMonster			= 6,			//允许穿怪
	aaNotTransfer			= 7,			//禁止定点传送
	aaNotSkillId			= 8,			//禁止用特定技能		参数一个或多个：技能id[1.2.3,...n]
	aaNotItemId				= 9,			//不能使用物品			参数一个或多个：物品id[1,2,3,...n]
	aaSceneLevel			= 10,			//地图最低等级			参数一个或两个：参数1：等级数 参数2：转生等级
	aaCity					= 11,			//回城点				参数两个：参数1：X坐标 参数2：Y坐标
	aaNotLevelProtect		= 12,			//新手保护失效
	aaZY					= 13,			//阵营战争区域
	aaSaftRelive			= 14,			//复活区                参数多个或者无  //无参数：代表是复活区域。
	aaForcePkMode			= 15,			//强制pk模式			参数一个：pk模式 [0和平模式，1队伍模式，2行会模式，4红名模式]
	aaNotSkillAttri			= 16,			//禁止使用任何技能
	aaNotDeal				= 17,			//禁止交易
	aaCannotViewOther		= 18,			//禁止查看他人装备
	aaCannotSeeName			= 19,			//看不到周围人名字
	aaLeaveDelBuf			= 20,			//删除buff				参数多个：buffId [1,2,3...n]
	aaSceneMaxLevel			= 21,			//地图最高等级			参数一个：最高等级数
	asDealArea				= 22,			//私人交易区域
	asSceneAreaMode			= 23,			//区域玩家属性			参数一个或无：对应tagSceneAreaFlag
	asNoDropEquip			= 24,			//场景不爆装备
	aaNewPlayerProtect		= 25,			//新手保护
	aaNoFire				= 26,			//禁止召唤火墙
	aaChangeModel			= 27,			//切换模型				参数两个：1.男模型 2.女模型 [1,2]
	asNoDropBagCross		= 28,			//只掉落身上穿戴的装备
	aaNotHereRelive			= 29,			//禁止原地复活， 无参数
	aaNotDeathPunish		= 30,			//无死亡惩罚，无参数
	aaDigArea				= 31,           //挖矿区域
	aaWorshipMonarch		= 32,			//膜拜君主区域
	aaNoticeBossIcon		= 33,			//允许提示BOSS头像的区域
	aaAutoFight				= 34,			//自动挂机属性
	aaNotCreateTeam			= 35,			//禁止组队
	aaSafeResurrection		= 36,			//复活10秒限制
	aaNotCallHero           = 37,           //禁止召唤英雄
	aaSceneNotTransfer		= 38,			//禁止转送 1行会传送或者召唤
	aaChangeWeapon			= 39,			//切换武器外观			参数两个：1.男模型 2.女模型 [1,2]
	aaChangeName			= 40,			//改变名字 				参数两个：1.男名字id 2.女名字id
	aaActivity				= 41,			//活动区域				参数多个：活动id....
	aaForbidSetpkMode       = 42,			//禁止修改pk模式
	aaNotCallPet      		= 43,			//禁止召唤宠物
	aaCampWar			    = 44,           //阵营战地图
	aaPaodianExp      		= 45,			//泡点地图
	aaCostCoin      		= 46,			//扣金币
	aaOverTimeTick      	= 47,			//超时踢出
	aaAttriCount,							//属性类型的数量
	aaMaxAttriCount			= 64,
};
//tolua_end
