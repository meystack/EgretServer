#pragma once

/*******************************************************/
/*
/*					逻辑服务器常量定义头
/*
/*******************************************************/

//定义游戏逻辑引擎版本号
#define LOGIC_KN_VERSION	MAKEFOURCC(15,4,9,1)

class CScene;
class CFuBen;

class EntityHandle : public Counter<EntityHandle>
{
public:
	EntityHandle( void ) : m_Handle( 0 ) { }
	EntityHandle(Uint64 handle):m_Handle(handle){}
	inline void init(unsigned nEntityType,unsigned nIndex)
	{
		m_EntityType = nEntityType;
		m_Index = nIndex;
	}
	unsigned int GetIndex ( void ) const { return ( m_Index ); }
	unsigned int GetType ( void ) const { return ( m_EntityType ); }
	Uint64 GetHandle( void ) const { return ( m_Handle ); }
	bool         IsNull   ( void ) const { return ( !m_Handle ); }
	inline operator Uint64 ( void ) const { return ( m_Handle ); }
	inline EntityHandle& operator = (const Uint64 handleValue) { m_Handle = handleValue; return *this; }
	inline EntityHandle& operator = (const EntityHandle handleValue) { m_Handle = handleValue.m_Handle; return *this; }	
private:
	union
	{
		struct
		{
			unsigned int  m_Index : 32; //实体的handle
			unsigned int  m_EntityType : 32; //实体的类型
			
		};
		Uint64  m_Handle;
	};
};
const static   EntityHandle   INVALID_ENTITY_HANDLE  ; //非法的实体的句柄

typedef struct tagEntityPos
{
//	unsigned int nFbHandler; //副本handler
	CFuBen*	pFb;
	CScene* pScene; //scence handler
	WORD nSceneId; //场景id
} ENTITYPOS,*PENTITYPOS;

// 场景位置信息
typedef struct tagScenePosInfo
{
	WORD nSceneId;
	WORD nPosX;
	WORD nPosY;

	void SetData(const int _nSceneId, const int _nPosX, const int _nPosY)
	{
		nSceneId	= _nSceneId;
		nPosX		= _nPosX;
		nPosY		= _nPosY;
	}
}ScenePosInfo;

enum tagLogicInternalMessage
{
	SSM_GATE_CREATE_ACTOR =1, //create actor 
	
	SSM_GATE_USER_LOGOUT,       //USER LOGOUT

	SSM_DB_RET_DATA,			//db返回数据

	SSM_FRIEND_LIST,			// db返回好友列表

	SSM_FRIEND_CHANGED,			// 好友发生变化

	SSM_FRIEND_INFO_UPDATE_SVR_ADDR,	// 好友服务器地址信息  added by caphone 2011.4.11

	SSM_SS_RET_DATA,			//会话服务器返回数据

	SSM_MS_RET_DATA,			//后台服务器的相关信息

	SSM_SET_SIGN,             //设置签名

	SSM_LOGIN_FAIL,           //玩家登陆失败

	SSM_LOGIC_2_COMMONLOGIC_DATA,		// 逻辑服务器->公共逻辑服务器

	SSM_COMMONLOGIC_2_LOGIC_DATA,		// 公共逻辑服务器->逻辑服务器

	SSM_SESSION_KICK_USER,           //Session服务器发消息来踢下线一个玩家

	SSM_RECV_LOGIN_DATA,             //收到的登陆的网络数据

	SSM_RECV_LOGIN_CMD,             //收到的登陆的命令码

	SSM_KICK_USER,             //踢人下线

	SSM_CLOSE_SERVER,        //关闭网关程序

	SSM_NEWBACKMGR_DATA,		//BackStage数据

	SSM_ASYNC_WORK_THREAD,//异步工作线程

	SSM_LOGIC_2_CROSS_DATA,		// 逻辑服务器->跨服逻辑服务器

	SSM_CROSS_2_LOGIC_DATA,		// 跨服逻辑服务器->逻辑服务器

	SSM_CROSS_CEHCK_ACTORDATA,		//验证玩家数据
	SSM_CROSS_SERVER_DB_DATA,		//跨服数据服->跨服逻辑服
};


// 服务器加载玩家数据
enum  
{ 
	eMSG_USR_LOGIN_DATA_NULL = 0,  
	eMSG_USR_LOGIN_DATA_ACTOR_SYSTEM = 1, 	//玩家数据  
	eMSG_USR_LOGIN_DATA_ACHIEVE_SYSTEM = 2, 	//玩家数据  
	eMSG_USR_LOGIN_DATA_ACTIVITY_SYSTEM = 3,		//技能数据
	eMSG_USR_LOGIN_DATA_STATICCOUNT_SYSTEM = 4,	 
	eMSG_USR_LOGIN_DATA_STRENGTHEN_SYSTEM = 5, 
	eMSG_USR_LOGIN_DATA_ACTORVARSUB_SYSTEM = 6,  
	eMSG_USR_LOGIN_DATA_FRIEND_SYSTEM = 7,	 
	eMSG_USR_LOGIN_DATA_HERO_SYSTEM = 8,	 
	eMSG_USR_LOGIN_DATA_MAIL_SYSTEM = 9,	 
	eMSG_USR_LOGIN_DATA_EQUIP_SYSTEM = 10,	
	eMSG_USR_LOGIN_DATA_SKILL_SYSTEM = 11,	
	eMSG_USR_LOGIN_DATA_GHOST_SYSTEM = 12,	
	eMSG_USR_LOGIN_DATA_NEWTITLE_SYSTEM = 13,	
	eMSG_USR_LOGIN_DATA_HALLOWS_SYSTEM = 14,	
	eMSG_USR_LOGIN_DATA_LOOTPET_SYSTEM = 15,	
	eMSG_USR_LOGIN_DATA_REBATE_SYSTEM = 16,	   
	eMSG_USR_LOGIN_DATA_ACTOR_CUSTOM = 17, 	//玩家自定义数据 
	eMSG_USR_LOGIN_DATA_MAX,		//同步结束结束 
};

// 跨服服务器到逻辑服的消息定义
enum tagCross2LogicMsg
{
	C2L_Mail = 1,			// 邮件
	C2L_ReqLoginData = 2,			//发送登录数据
	C2L_CloseActor = 3,			//踢下线
};


// 跨服服务器到逻辑服的消息定义
enum //tagMsg2CorssServerEnum
{
	MSG_CACTOR, 	//玩家数据
	MSG_CSKILL,		//技能数据
	MSG_CEQUIP,		//装备数据
	MSG_CGAMESET,	//设置数据
	MSG_CTITLE,		//称号数据
	MSG_CFASH,		//时装数据
	MSG_CHALIDOME,	//圣物数据
	MSG_CSTRENGTH,	//强化数据
	MSG_CGHOST,		//神魔数据
	MSG_CUSEBAG,	//背包数据
	MSG_CRELIVE,	//复活数据
	MSG_LOOTPET,	//宠GLOBLE物数据
	MSG_GLOBAL_DATA,//系统数据
	MSG_CLOADGUILD, //加载行会数据数据
	MSG_CS_END,		//同步结束结束 
};

// 逻辑服务器之间的消息定义
enum tagLogic2LogicMsg
{
	L2L_ReqTransmit,			// 请求传送
	L2L_ReqTransmitAck,			// 请求传送应答
	L2L_StartTransmit,			// 开始传送
	L2L_EndTransmit,			// 传送完毕
};

enum tagLoginErrorCode
{
	eNoSessionData=1,          //会话找不到会话状态
	eSessionStatusIsError=2,   //会话状态不正确
	eDbStatusError=3,          //数据服务器的状态错误  
	eIsLogin =4,               //已经登陆了游戏
};


#define TYPE_PHYSICAL_ATTACK 1  //物理攻击类型
#define TYPE_MAGIC_ATTACK 2     //魔法攻击类型
#define TYPE_WIZARD_ATTACK 3    //道术法攻击类型
#define TYPE_TOXIC_ATTACK 4     //毒物攻击，也就是debuf
#define TYPE_HOLY_ATTACK 5     //神圣攻击

#define INVALID_CAMP_ID 0

//玩家领取奖励的统一配置
typedef struct tagActorAward
{
	BYTE btType;
	WORD wId;
	int wCount;
	// BYTE btQuality;
	// BYTE btStrong;//不使用
	// WORD wStar;
	// BYTE btStarLost;
	// BYTE btBind;
	// BYTE btInscriptLevel;
	// int	 nAreaId;
	// BYTE btLuck;
	// WORD nQualityDataIndex;//极品属性索引
	// int  nSmith[5];
	// WORD wIdentifyslotnum;
	// WORD wIdentifynum;
	// int  nResourceCount;//资源数量（邮件奖励资源用这个存)
	tagActorAward()
	{
		memset(this, 0, sizeof(*this));
	}
}ACTORAWARD, *PACTORAWARD;

enum enLogicWayId
{
	lwiInitActorFail = 1,
	lwiCloseAllUser,
	lwiCloseTransmitAllUser,
	lwiUseQuickTool,
	lwiRecvTransmit,
	lwiAcupointDbFail,
	lwiAlmirahDbFail,
	lwiGmTickActor,
	lwiCheckSpeedSystemTick,
	lwiCheckSpeedConfirmSpeed,  //10
	lwiLoadDiamondTypeFail,
	lwiLoadDiamondPosFail,
	lwiLoadDiamondDbRetFail,
	lwiMagicDbFail,
	lwiFcmExitTick,
	lwiMoveSysIdle,
	lwiEquipDbFail,
	lwiBackStageTickActor,
	lwiBackStageTickAccount,
	lwiBackStageTickAll,		//20
	lwiGateCreateKeyError,
	lwiGateUserLogout,
	lwiSessionKickActor,
	lwiNetMgrKickByActorName,
	lwiNetMgrKickByAccountName,
	lwiBackStageTickActorByLevel,
	lwiSameActorLogin, 
	lwiBackStageForbidActor,
	lwiSimulatorSameActorLogin, 
	lwiSimulatorRankDown, 
}; 
//归属boss
typedef struct tagBeLongBoss
{
	int nBossId; 
	int nSceneId;
}BeLongBoss;
