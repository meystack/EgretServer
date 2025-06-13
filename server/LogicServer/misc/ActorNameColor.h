#pragma once

// 默认的名字颜色
#define DEFAULT_NAME_COLOR	0xFFFFFFFF

struct ACTOR_NAME_CLR_TABLE				// 角色名称颜色配置表
{	
	unsigned int clr_pk_otherside;		// 擂台、pk看对方的颜色
	unsigned int clr_pkval_over300;		// 杀戮值超过300其他玩家看到的名称颜色
	unsigned int clr_pkval_over100;		// 杀戮值超过100其他玩家看到的名称颜色
	unsigned int clr_pkval_over60;		// 杀戮值超过60其他玩家看到的名称颜色
	unsigned int clr_force_pk;			// 强制pk对方名称颜色
	unsigned int clr_teammates;			// 蓝色 队友名称颜色
	unsigned int clr_normal;			// 普通名称颜色
	unsigned int clr_pkmode;			// 橘红色 pk模式
	unsigned int clr_slave;				// 绿色 自己下属的名称显示颜色
	
	unsigned int clr_quality_white;				// 白色
	unsigned int clr_quality_green;				// 绿色
	unsigned int clr_quality_blue;				// 蓝色
	unsigned int clr_quality_puple;				// 紫色
	unsigned int clr_quality_orange;			// 橙色
	unsigned int clr_quality_red;				// 红色

};


/*
* Comments:根据品质获得
* Param int nQuality:品质，宠物或者装备的品质
* @Return unsigned int:颜色，白，绿，蓝，紫等
*/
unsigned int GetColorByQuality(int nQuality);

/*
* Comments: 获取其他观察者看玩家的名称颜色
* Param EntityHandle selfHandle:  玩家句柄
* Param EntityHandle otherHandle: 观察者句柄。如果other和self相同，就是自己观察自己
* Param bool bIncChallenge: 是否考虑擂台战。（设置此标记的目的是擂台退出的更新）
* @Return unsigned int: 颜色值
*/
unsigned int GetNameColorLookedByOther(EntityHandle selfHandle, EntityHandle otherHandle, bool bIncChallenge = true);


/*
* Comments: 获取怪物名字颜色
* Param EntityHandle monsterHandle: 怪物句柄
* Param EntityHandle otherHandle: 观察者句柄。可以是怪物的Master，也可以是其他玩家
* @Return unsigned int: 怪物名称颜色值
* @Note：
*/
unsigned int GetMonsterNameColorByOther(EntityHandle monsterHandle, EntityHandle otherHandle);

//////////////////////////////////////////////////////////////////////////
/// 引起玩家名称颜色改变的各种Handler
//////////////////////////////////////////////////////////////////////////
/*
* Comments: 玩家登陆时下发
* Param EntityHandle selfHandle:
* @Return void:
*/
void UpdateNameClr_OnActorLogin(EntityHandle selfHandle);

/*
* Comments:  玩家杀戮值变化引起的玩家名称颜色改变处理
* Param EntityHandle selfHandle: 玩家句柄
* Param unsigned int oldVal: 老的杀戮值
* Param unsigned int oldVal: 新的杀戮值
* @Return void:
* @Note: 需判断是否需要改变颜色，如果需要，广播给周围玩家（包括自己）
*/
void UpdateNameClr_OnPkValChanged(EntityHandle selfHandle, unsigned int oldVal, unsigned int newVal);


/*
* Comments: 玩家加入或者离开队伍引起的玩家名称颜色改变处理
* Param EntityHandle selfHandle: 玩家句柄
* Param int nTeamId: 队伍ID
* Param bool bJoin: 为true表明是加入队伍，否则是离开队伍
* @Return void:
* @Note: 加入队伍或者离开队伍之后调用
*/
void UpdateNameClr_OnJoinOrLeaveTeam(EntityHandle selfHandle, int nTeamId, bool bJoin = true);


/*
* Comments: 玩家进入PK或者退出PK引起的玩家名称颜色改变处理
* Param EntityHandle selfHandle: 玩家句柄
* Param EntityHandle otherSideHandle: 切磋对方句柄（切磋完无法从切磋子系统中获取对方句柄，这里传参数进来）
* @Return void:
*/
void UpdateNameClr_OnEnterOrLeavePK(EntityHandle selfHandle, EntityHandle otherSideHandle);

/*
* Comments: 修改pk模式时需要下发名字颜色
* Param EntityHandle selfHandle: 玩家句柄
* @Return void:
*/
void UpdateNameClr_OnPkModeChanged(EntityHandle selfHandle);

/*
* Comments: 更新周围
* Param CActor* pActor: 
* @Return void:
*/
void UpdateOtherNameClr_GiveSelf(EntityHandle selfHandle);

void BroadCastNameClr_Scene(CActor* pActor);

void SendNameClr_ToSelf(CActor* pActor);

//通知周围其他玩家更新自己的名称颜色
void NotifyOtherNameClr_GiveThemSelf(EntityHandle selfHandle);

