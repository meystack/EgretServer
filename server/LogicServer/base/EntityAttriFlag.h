#pragma once


//实体的属性标志位
struct EntityFlags
{
	union
	{
		Uint64 nValue;
		struct
		{
			bool DenyMove	:1;							//不能移动(实现)
			bool DenyBeMove:1;							//不能被移动，比如牢门(实现)
			bool DenyAttackActor:1;						//不攻击玩家(实现)
			bool DenyBeAttack:1;						//不能被攻击(实现)
			bool DenyAttackedByActor:1;					//不能被玩家攻击(实现)
			bool CanAttackMonster:1 ;					//能够攻击怪物(实现)
			bool DenyAutoAddHp:1;						//不能自动添加HP(实现)
			bool DenyAutoAddMp:1;						//不能自动添加MP(实现)
			bool CanSceneTelport:1;						//能否跨场景传送(实现)   10
			bool CanSeeHideActor:1;						//能否看到隐身的玩家(实现)
			bool DenyDieSubExp:1;						//不进行经验衰减(实现)
			bool TelByOwnerEntity:1;					//传送的时候跟着归属者传送
			bool MonsterLazyFollow:1;					//懒惰跟随属性。惰性跟随距离远了会自动拉到被跟随者；非惰性跟随会检测跟随距离，距离符合会传送。（部分实现）
			bool DenyUseNearAttack:1;					//不使用肉搏
			bool DenyDizzyBuff:1;						//不能添加晕眩的buff
			bool DenyMoveForbidBuff:1;					//不能添加定身的Buff
			bool DenyInnnerSkillForbidBuff:1;			//不能添加封的buff
			bool DenyOutSkillForbidBuff:1;				//不能添加断的buff
			bool DenyMoveSpeedSlowBuff:1;				//禁止移动速度降低的buff 20
			bool DenySee:1;								//不能被看到
			bool DenyInitiativeAttackActor:1;			//不主动攻击玩家（针对主动怪）
			bool DenyShowMasterName:1;					//拒绝显示主人的名字
			bool TelByOwnerEntityWhenNonePortal:1;		//指定怪物下属是否在主人通过非传送门传送的时候也跟随传送，默认为不传送
			bool CanGrowUp:1;							//boss是否成长
			bool AttackKiller:1;						//只主动攻击红名玩家
			bool ShowNameByNumber:1;					//怪物最后不能显示为数字，默认可以
			bool DenyBeCatch:1;							//禁止被法师的诱惑技能抓取
			bool CanAlwaysEnter:1;						//怪物不管什么情况，都能进入场景
			bool BeAttackNeedUseSkill:1;				//被攻击需要释放技能
			bool IdleUseSkill:1;						//平时没什么事情也使用技能，主要用于一些特殊的怪物，这些怪物讲收到怪物的攻击
			bool NeedSaveToLog:1;						//是否需要保存到统计后台
			bool PetNoAttact:1;							//宠物不攻击
			bool DenyAddNumber:1;						//人数超过1千的时候不自动增加个数
			bool AttackPet:1;							//主动攻击道士宠物  40
			bool DenyShowName:1;						//不显示名称
			bool CanReuse:1;							//是否重用，这里不读取配置，如果怪物的刷新时间为0，设置为可重用
			bool MoveTelport:1;							//移动就传送
			bool NoUseAStar:1;							//不使用A*算法
			bool AttackSeeKill:1;						//死亡见证（攻击），至少攻击1次，且在视野范围之内，才算参与击杀
			bool noBeCrossed:1;							//是否不能被穿，0-可穿怪（）默认，1-不可穿怪
			bool noAttackBack:1;						//被攻击了不还手
			bool noReturnHome:1;						//不回巢，默认都是回巢的
			bool attackToScript:1;						//被攻击了需要通知脚本
			bool DenySetDir :1;							//怪物生成后不能改变朝向.默认朝向用stditem->dir
			bool bDeathTriggerScript: 1;				//死亡触发脚本（注：一般此处是通过脚本registerMonsterDeath（）设置为true）
	        bool bMobTriggerScript :1;					//刷新的时候触发脚本（注：一般此处是通过脚本registerMonsterMob（）设置为true）
			bool bLiveTimeOutTriggerScript :1;			//生命期到的时候触发脚本（注：一般此处是通过脚本registerMonsterLiveTimeOut（）设置为true）
			bool boShowVestEntityName:1;				//显示怪物归属人物名字
		};
	};
	EntityFlags()
	{
		nValue =0;
	}
};
