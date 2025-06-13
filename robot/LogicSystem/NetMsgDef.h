#pragma once
/*
** 本文件主要用于定义客户端同服务器之间的网络消息协议数据定义。
*/

#pragma pack(push, 1)

/*
enum ScriptSubSystemID
{
enScriptMiscSystem = 139,
enScriptTestSystem = 150,
};
*/

enum ScriptMiscSystemMsgID
{
	enMisc_QuickTel = 7,				// 速传
};

// 测试子系统消息定义
enum TestSystemMsgID
{
	enTestSys_ReliveInPlace = 1,			// 原地复活
	enTestSys_EnterFuben = 2,				// 进入副本
	enTestSys_RequestEquips=3,				// 请求装备
};

// 其他非玩家实体数据( TODELETE )
typedef struct tagNonActorPropData
{
	EntityHandle			handle;							// 实体句柄
	char					szDecorateName[256];			// 修饰名称
	WORD					wPosX;							// 实体坐标X
	WORD					wPosY;							// 实体坐标Y
	unsigned int					wModelId;						// 实体模型Id
	BYTE					ucDir;							// 朝向
	// 以下是怪物、宠物特有的数据
	BYTE					ucLevel;						// 等级
	unsigned int			nHP;							// HP
	unsigned int			nMP;							// MP
	unsigned int			nMaxHP;							// 最大HP
	unsigned int			nMaxMP;							// 最大MP
	WORD					wMoveSpeed;						// 移动速度，实际上是移动一个格子需要的时间（ms）
	WORD					wAttackSpeed;					// 攻击速度，实际上是攻击时间间隔（ms）
	unsigned int			nStatus;						// 实体状态
	unsigned int            nCreatureColor;
	unsigned int			nNameColor;						// 名字颜色
	BYTE					ucAttackType;					// 攻击类型和怪物类型。攻击类型占低4位，怪物类型占高4位
	// 以下还有Buff数据、效果Effect数据、怪物拥有者句柄、NPC功能分类Id等，暂时未用到，不做定义
}NonActorPropData;

// 其他玩家数据
typedef struct tagOtherActorPropData
{
	EntityHandle			handle;							// 实体句柄
	char					szDecorateName[256];			// 修饰名称
	WORD					wPosX;							// 实体坐标X
	WORD					wPosY;							// 实体坐标Y
	int						nModelId;						// 实体模型Id
	unsigned int			nHP;							// HP
	unsigned int			nMP;							// MP
	unsigned int			nMaxHP;							// MaxHP
	unsigned int			nMaxMP;							// MaxMP
	WORD					wMoveSpeed;						// 移动速度
	BYTE					ucSex;							// 性别
	BYTE					ucVocation;						// 职业
	WORD					ucLevel;						// 等级
	unsigned int			nCircle;						//转数
	int						nWeaponAppear;					// 武器外观
	int						nSwingAppear;					//翅膀外观
	int						nSocialMask;					//社会关系
	WORD					wIcon;							// 图标
	WORD					nAttackSpeed;					// 攻击速度
	BYTE					ucDir;							// 朝向
	unsigned int			nStatus;						// 状态掩码
	unsigned int			nRes1;							
	int						nTeamId;						// 队伍Id
	BYTE					ucCamp;							// 阵营
	unsigned int			nHeadTitle;						// 头衔
	unsigned int			nNameColor;						// 名字颜色ARGB值	
	unsigned int			nSupLevel;						// 超玩等级
	unsigned int			nSoldierSoulAppear;				// 兵魂外观
	WORD					nWeaponId;						// 正在装备的武器id
	unsigned int			nGuildID;						// 行会id
	unsigned int			nMonsterID;						// 怪物模型id
	unsigned int			nMeritoriousPoint;				// 累计功勋
	unsigned int			nNextSkillFlag;					// 战士下一次技能标记
	unsigned int			nEvilPkStatus;					// 恶意PK状态
	unsigned int			nPkValue;						// PK值
	
	// 下面还有Buff数据、效果数据。暂时没用到先不处理
}OtherActorPropData;

// 怪物数据
typedef struct tagMonsterPropData
{
	EntityHandle			handle;							// 实体句柄
	char					szDecorateName[256];			// 修饰名称
	WORD					wPosX;							// 实体坐标X
	WORD					wPosY;							// 实体坐标Y
	int						nModelId;						// 实体模型Id
	BYTE					ucDir;							// 朝向
	WORD					ucLevel;						// 等级
	unsigned int			nHP;							// HP
	unsigned int			nMP;							// MP
	unsigned int			nMaxHP;							// MaxHP
	unsigned int			nMaxMP;							// MaxMP
	WORD					wMoveSpeed;						// 移动速度
	WORD					wAttkSpeed;						// 攻击速度
	unsigned int			nStatus;						// 状态掩码
	unsigned int			nNameColor;						// 名字颜色ARGB值	
	WORD					wMonsterId;						// 怪物id
	unsigned int			nLiveTimeOut;					// 生命到期时间
}MonsterPropData;

// 宠物数据
typedef struct tagPetPropData
{
	EntityHandle			handle;							// 实体句柄
	char					szDecorateName[256];			// 修饰名称
	WORD					wPosX;							// 实体坐标X
	WORD					wPosY;							// 实体坐标Y
	int						nModelId;						// 实体模型Id
	BYTE					ucDir;							// 朝向
	WORD					ucLevel;						// 等级
	unsigned int			nHP;							// HP
	unsigned int			nMP;							// MP
	unsigned int			nMaxHP;							// MaxHP
	unsigned int			nMaxMP;							// MaxMP
	WORD					wMoveSpeed;						// 移动速度
	WORD					wAttkSpeed;						// 攻击速度
	unsigned int			nStatus;						// 状态掩码
	unsigned int			nNameColor;						// 名字颜色ARGB值	
	WORD					wEntityId;						// 宠物id

}PetPropData;

typedef struct tagClientSkillData
{
	WORD					wSkillId;
	BYTE					ucSkillLevel;
	WORD					wSkillMiji;
	int						nSkillCD;				// 技能CD
	int						nSkillExp;				// 技能经验？
	unsigned int			nMijiExpireTick;		// 秘籍过期时刻
	BYTE					nSkillInvalid;			// 技能失效。1表示失效，1表示正常
}ClientSkillData;


#pragma pack(pop)