#ifndef _DB_PROTO_H_
#define _DB_PROTO_H_


//数据引擎版本号
//#define DBEGN_KN_VERION			MAKEFOURCC(2, 10, 11, 20)
//数据引擎的数据结构版本
//#define	DBSDATATYPE_VERSION		0x010A1B0A

//这里是定义返回给客户端的错误码，均要小于0,1字节
#define	NOERR					0		//正确
#define	ERR_SQL					101		//sql错误
#define	ERR_SESS				102		//用户没登陆
#define ERR_GAMESER				103		//游戏服务没准备好
#define	ERR_DATASAVE			104		//角色上一次保存数据是否出现异常
#define ERR_SELACTOR			105		//客户端选择角色的常规错误
#define ERR_NOGATE				106		//客户端无匹配的路由数据的错误
#define ERR_NOUSER				107		//角色不存在
#define ERR_SEX					108		//错误的性别
#define ERR_NORANDOMNAME		109		//随机生成的名字已经分配完
#define ERR_ZY					110		//客户端上传的角色阵营参数错误
#define ERR_JOB					111		//客户端上传的角色职业参数错误
#define ERR_GUILD				112		//存在帮派不能删除角色
#define ERR_SELSERVER			113		// 选择服务器错误
#define ERR_SQL_NOT_CONNECT		114		// sql没有准备好


//角色状态标志位
#define	CHARSTATE_DISABLED		0x80000000	//角色已被禁用
#define	CHARSTATE_USERDELETED	0x40000000	//角色已被删除
#define	CHARSTATE_SELECTED		0x00000001	//角色被选中

/* 定义数据服务器关于角色创建、删除、查询等的数据包格式
*************************************/
//以下是包的类型定义
typedef	enum ActorCMD{
	CREATEACTORDATAREQ_CMD	= 16,
	CREATEACTORDATARESP_CMD =17,
	DELETEACTORDATAREQ_CMD = 18,
	DELETEACTORDATARESP_CMD = 19,
	QUERYACTORLISTREQ_CMD = 20,
	QUERYACTORLISTRESP_CMD = 21,
	ENTRYGAMEREQ_CMD = 22,
	ENTRYGAMERESP_CMD = 23,
	RANDNAMEREQ_CMD = 24,	//自动生成名字
	RANDNAMERESP_CMD = 25,
	LESSJOBREQ_CMD = 26,		//查询最少人使用的职业
	LESSJOBRESP_CMD = 27,
	LESSZYREQ_CMD = 28,		//查询最少人使用的阵营
	LESSZYRESP_CMD = 29,	
}ACTORCMD;

//查询角色列表的请求包
typedef  struct  tagQueryActorListReq{
	WORD		CmdId;			//数据包的类型
	int			AccountId;		//帐户id	
}QUERYACTORLISTREQ,*PQUERYACTORLISTREQ;

typedef struct
{
	UINT 	ActorId;//角色id
	//以下是该角色的属性值，暂定
	char	ActorName[jxSrvDef::ACTOR_NAME_BUFF_LENGTH];
	char	GuildName[jxSrvDef::MAX_GUILD_NAME_LENGTH];
	int		Icon;
	int		Sex;				//性别
	short	Level;				//等级
	short	Circle;				//转身
	int 	Job;				//职业的编码
	int		Zy;					//阵营编码
}Actors;

//查询角色列表的回应包包
typedef  struct  tagQueryActorListResp{
	WORD  CmdId;			//数据包的类型
	unsigned int		AccountId;		//帐户id
	char 	Count;             //角色数量,小于0表示查询失败
	// 后面是Count个Actors包
	//Actors[Count];
}QUERYACTORLISTRESP,*PQUERYACTORLISTRESP;

//创建角色的请求包
typedef  struct  tagCreateActorDataReq{
	WORD	CmdId;			//数据包的类型   WORD
	char	Namelen;
	char	ActorName[jxSrvDef::ACTOR_NAME_BUFF_LENGTH];		//角色名称
	char	Sex;				//性别
	char 	Job;			//职业的编码
    char	Icon;              //头像
	BYTE	Zy;
	//其他属性不加到数据包中，采用默认值
}CREATEACTORDATAREQ,*PCREATEACTORDATAREQ;

//创建角色的回应包
typedef  struct  tagCreateActorDataResp{
	WORD			CmdId;			//数据包的类型
	UINT 			ActorId;	// 如果ret 为0,则该字段表示新创建的角色id，否则本字段结果没意义
	char			Ret;//创建角色的结果，0 表示成功，否则为错误码
}CREATEACTORDATARESP,*PCREATEACTORDATARESP;

//删除角色的请求包
typedef  struct  tagDeleteActorDataReq{
	WORD	CmdId;			//数据包的类型
	UINT 	Actoid;//角色id
} DELETEACTORDATAREQ,*PDELETEACTORDATAREQ;

//删除角色的回应包
typedef  struct  tagDeleteActorDataResp{
	WORD		CmdId;			//数据包的类型
	UINT 		Actorid;//角色id
	char		Ret;//删除角色的结果，0表示删除成功，否则为错误码
} DELETEACTORDATARESP,*PDELETEACTORDATARESP;

//	进入游戏通知包
typedef  struct  tagEnteyGameReq{
	WORD	CmdId;					//数据包的类型, ENTRYGAMEREQ_CMD
	UINT	Actorid;			//进入的角色id
} ENTRYGAMEREQ,*PENTRYGAMEREQ;

//逻辑服务器信息包
typedef  struct  tagEnteyGameResq{
	WORD	CmdId;			//数据包的类型, ENTRYGAMERESP_CMD
	char 	szIP[128];		//0 结尾的字符串
	int 	Port;
	char	Ret;				//回应的结果，是否可以进入游戏，0表示正确
} ENTRYGAMERESP,*PENTRYGAMERESP;

typedef struct tagCreateActorData
{
	char	actorName[jxSrvDef::ACTOR_NAME_BUFF_LENGTH];	  //角色名称
	char	accountName[jxSrvDef::ACCOUNT_NAME_BUFF_LENGTH];  //账户名称
	BYTE	sex;				//性别
	BYTE 	job;			    //职业的编码
    BYTE	icon;               //头像
	BYTE	zy;                 //阵营
	int     nServerIndex;       //服务器编号
	unsigned int nAccountId;    //账户的id
	int     nGateIndex;         //网关的index
	Uint64  lKey;               //key
	unsigned long long lIp;     //客户端登陆的ip
	int      nLastRandNameIndex;//上次随机名字的index
	int      nAdId;				//广告id
	int      nRawServerindex;   //原始的服务器的id
	char strToken[jxSrvDef::MAX_TOKEN_LENGTH];		//用于运营商的渠道统计
	tagCreateActorData()
	{
		memset(this, 0, sizeof(*this));
	}
}CREATEACTORDATA,*PCREATEACTORDATA;


/************************************/

/*  SQL查询语句定义
 *******************************************/
//从数据库中加载角色名称过滤表
static LPCSTR NSSQL_LoadFilterNames				= "CALL loadFilterNames()";

static LPCSTR szSQLSP_GetFilterNames					= "call getdenycharnames()";//查询名称屏蔽此列表
static LPCSTR szSQLSP_LoadGameSrvRouteTable				= "select * from gameserveraddress";//查询游戏网关路由表

static LPCSTR szSQLSP_QueryLessJob						= "call getlessjob(%d)";//查询最少人选择的职业
static LPCSTR szSQLSP_QueryLessZy						= "select zy from zycount where serverindex=%d order by usercount asc limit 1";//查询最少人选择的阵营
static LPCSTR szSQLSP_QueryActorCount					= "select count(*) from actors where accountid=%u and serverindex=%d";//查询本账户所有的角色数量，包括已删号或者被屏蔽的
static LPCSTR szSQLSP_QueryZyList						= "call queryzylist(%d)";//查询可选的阵营列表

static LPCSTR szSQLSP_LoadZyDistributeData				= "call loadzydata()"; // 查询阵营分布数据
static LPCSTR szSQLSP_LoadJobDistributeData				= "call loadjobdata()"; // 查询职业分布数据
static LPCSTR szSQLSP_CleanZyDistributeData				= "call cleanzydata(%d)"; // 清除阵营分布数据
static LPCSTR szSQLSP_CleanJobDistributeData			= "call cleanjobdata(%d)"; // 清除职业分布数据
static LPCSTR szSQLSP_AddZyDistributeData				= "insert into zycount values"; // 插入阵营分布数据
static LPCSTR szSQLSP_AddJobDistributeData				= "insert into jobcount values"; // 插入职业分布数据

static LPCSTR szSQLSP_LoadActorNameInitData				= "select actorname,serverindex from actors";	//查询玩家名称数据

static LPCSTR szSQLSP_LoadActorBattleData				= "call loadbattlepowerrank(%d,%d)";	 //查询玩家战力数据
static LPCSTR szSQLSP_LoadActorCombatGamData			= "call loadcombatgameinfo(%u)";	 //查询玩家战力竞技信息
static LPCSTR szSQLSP_DeleteActorCombatGamData			= "call deletecombatgame(%u)";		//删除玩家战力竞技信息
static LPCSTR szSQLSP_AddActorCombatGamData				= "call addcombatgameinfo(%u,%d,%d,%d,%d,%d,%d,%d)";		//添加玩家战力竞技信息
static LPCSTR szSQLSP_LoadActorCombatRecordData			= "call loadcombatrecord(%u)";	 //加载玩家战力竞技日志
static LPCSTR szSQLSP_DeleteActorCombatRecordData		= "call deletecombatrecord(%u)";		//删除玩家战力竞技日志
static LPCSTR szSQLSP_AddActorCombatRecordData			= "call addcombatrecord(%u,%u,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d)";		//添加玩家战力竞技日志
static LPCSTR szSQLSP_UpdateActorCombatBaseInfo			= "call updatecombatgame(%u,%d)";		//更新

static LPCSTR szSQLSP_LoadActorCombatBaseInfo			= "select currank,maxrank,lastrank,combatscore,challegenum,picknum,cleartime,cdflag from combatinfo where actorid=%d";
static LPCSTR szSQLSP_AddActorCombatBaseInfo			= "insert into combatinfo(actorid) values(%d)";
static LPCSTR szSQLSP_SaveActorCombatBaseInfo			= "update combatinfo set currank=%d,maxrank=%d,lastrank=%d,combatscore=%d,challegenum=%d,picknum=%d,cleartime=%d,cdflag=%d where actorid=%d";
static LPCSTR szSQLSP_LoadActorCombatLogInfo			= "select type,result,targetid,targetname,rank from combatlog where actorid=%d";
static LPCSTR szSQLSP_SaveActorCombatLogInfo			= "insert into combatlog(actorid,type,result,targetid,targetname,rank) values";
static LPCSTR szSQLSP_DeleteActorCombatLogInfo			= "delete from combatlog where actorid=%d;";

//客户端相关数据的SQL查询
//static LPCSTR szSQLSP_CheckCreateCharCountryFlag		= "call checkcreatecharcountryflag(%d)";//查询允许创建的国家列表（标志位作为允许创建的国家表）
//static LPCSTR szSQLSP_ClientQueryChar					= "call clientquerycharactors(%d,\"%s\")";//查询角色在特定服务器的账号列表
//static LPCSTR szSQLSP_GetAvailableCharactorCountInServer= "call getavailablecharactorcountinserver(%d, \"%s\")";//查询角色在特定服务器的角色数量
static LPCSTR szSQLSP_GetCharactorIdByName				= "call getcharactoridbyname(\"%s\",%d)";//通过角色名称查询角色ID
static LPCSTR szSQLSP_ClientCreateNewCharactor			= "call clientcreatenewcharactor(%u,\"%s\",%llu,%u,\"%s\",%d,%d,%d,%d,%d,\"%s\",%d)";//用户创建角色
static LPCSTR szSQLSP_ClientDeleteCharactor				= "call clientdeletecharactor(%u,\"%s\")";//用户删除角色(EXEC)
static LPCSTR szSQLSP_ClientStartPlay					= "call clientstartplay(%d,%u,\"%s\",%u,%llu)";//开始游戏
static LPCSTR szSQLSP_QueryActorLastLoginServerId		= "call getactorlastloginserverid(%u, %u, %d)"; // 获取角色上次登录的服务器ID
static LPCSTR szSQLSP_SelecteGuildData					= "call selectguildid(%u)"; // 查询帮派id
static LPCSTR szSQLSP_GetValidActorIdByName				= "call getvalidactoridbyname(\"%s\", %d)";		//查询角色名是否有效

static LPCSTR szSQLSP_SelectActorNameCount				= "select count(*) form actors where actorname=\"%s\" and serverindex=%d";			//查询角色名是否存在
static LPCSTR szSQLSP_ChangeActorName					= "update actors set actorname=\"%s\" where actorid=%u and serverindex=%d";							//玩家改名字

//数据客户端帮会相关数据的SQL查询
//static LPCSTR szSQLSP_GetServerGuildCount				= "call getserverguildcount(%d)";//查询所属服务器的帮会数量
//static LPCSTR szSQL_LoadGuildsIndex						= "select guildid from guildbase where serverindex=%d limit %d,%d";//查询所属服务器的帮会ID列表
static LPCSTR szSQLSP_LoadGuildBasicData				= "call loadguildbasicdata(%d)";//查询帮会基本数据
static LPCSTR szSQLSP_LoadGuildMembers					= "call loadguildmembers(%u)"; //查询帮会成员数据
static LPCSTR szSQLSP_LoadGuildApplyLists				= "call loadguildapplylists(%u)"; //查询行会申请列表
static LPCSTR szSQLSP_LoadGuildApplyResult				= "select actorid, guildid from actorapplyguildresult"; //查询行会申请列表
static LPCSTR szSQLSP_DeleteActorGuildApplyResult		= "truncate actorapplyguildresult"; //清空
static LPCSTR szSQLSP_InsertGuildApplyResult			= "insert into actorapplyguildresult(actorid, guildid) values"; //插入
// static LPCSTR szSQLSP_GuildDealApply					= "call dealguildapply(%u, %u, %d)"; //处理
static LPCSTR szSQLSP_GuildInsertApply					= "insert into guildapplylist(guildid,actorid, applytime) values(%u,%u, %d)"; //处理
static LPCSTR szSQLSP_GuildDeleteApply					= "delete from guildapplylist where guildid = %u and actorid = %u;"; //处理
static LPCSTR szSQLSP_AddOfflineGuildMember				= "call addofflineguildmember(%u,%u,%d)"; //增加一个成员，并返回这个成员的详细数据
static LPCSTR szSQLSP_LoadGuildMemberBasic				= "call loadguildmemberbasic(%u)";//查询帮会成员的详细数据
static LPCSTR szSQLSP_LoadGuildWar						= "call loadguildwar(%u)"; //查询帮派战数据
//static LPCSTR szSQLSP_LoadGuildWarHistory				= "call loadguildwarhistory(%d)"; //查询帮派战历史数据
//static LPCSTR szSQLSP_AddGuildWarHistory				= "call addguildwarhistory(%d,%d,%d,%d)"; //增加帮派战历史数据
static LPCSTR szSQLSP_LoadGuildSKILL					= "call loadguildskill(%u)"; //查询帮派技能数据
static LPCSTR szSQLSP_LoadGuildEvent					= "call loadguildevent(%u)"; //查询帮派事件数据
static LPCSTR szSQLSP_LoadActorBuildingData				= "call loadactorguildbuilding(%u)"; //查询帮派事件数据
//boss
static LPCSTR szSQLSP_LoadBossData					= "call loadbossdata()";	//加载boss
static LPCSTR szSQLSP_DeleteBossData				= "truncate bossmgr;";		//清空
static LPCSTR szSQLSP_SaveBossData					= "insert into bossmgr(bossid, referTime, leftCount, bosslv) values"; //保存boss


// 活动 BEGIN
static LPCSTR szSQLSP_LoadActivityData					= "call loadactivitydata()";	//加载活动
static LPCSTR szSQLSP_DeleteActivityData				= "truncate activitydata;";		//清空
static LPCSTR szSQLSP_SaveActivityData					= "insert into activitydata(nId, nActivityType, nTimeType, nStartTime, nEndTime) values"; //保存活动数据

static LPCSTR szSQLSP_LoadActorActivityData				= "call loadactoractivity(%u)";	//加载个人活动
static LPCSTR szSQLSP_SaveActorActivityData				= "REPLACE INTO actoractivity(actorid, nId, nActivityType, nExpiredTime) VALUES";	//更新或插入个人活动
// 活动 END

//static LPCSTR szSQL_LoadGuildAllys						= "select a.objectid,b.guildname from guildobjectinfo a,guildbase b where a.guildid=%d and a.objecttype=1 and a.objectid=b.guildid limit %d,%d";//查询帮会联盟数据
//static LPCSTR szSQL_LoadGuildHostiles					= "select a.objectid,b.guildname from guildobjectinfo a,guildbase b where a.guildid=%d and a.objecttype=2 and a.objectid=b.guildid limit %d,%d";//查询帮会敌对数据
//static LPCSTR szSQL_LoadGuildWanteds					= "select a.objectid,b.charname,c.charlevel,c.guildid from guildobjectinfo a,chardescr b,chardata c where a.guildid=%d and a.objecttype=3 and a.objectid=b.charid and a.objectid=c.charid limit %d,%d";//查询帮会通缉数据
//static LPCSTR szSQL_LoadGuildJoinrequests				= "select a.objectid,b.charname,c.charlevel,c.sex,c.job from guildobjectinfo a,chardescr b,chardata c where a.guildid=%d and a.objecttype=4 and a.objectid=b.charid and a.objectid=c.charid limit %d,%d";//查询帮会入帮请求数据
static LPCSTR szSQLSP_AddGuildMember					= "call addguildmember(%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%u)";//添加帮会成员(EXEC)
static LPCSTR szSQLSP_DelGuildMember					= "call delguildmember(%u)";//删除帮会成员(EXEC)
static LPCSTR szSQLSP_UpdateGuildMember					= "call updateguildmember(%u,%u,%d,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%u)";//更新帮会成员(EXEC)
static LPCSTR szSQLSP_CreateGuild						= "call createguild(%u,%u,\"%s\",\"%s\",%d)";//创建帮会(QUERY)
static LPCSTR szSQLSP_DeleteGuild						= "call deleteguild(%u)";//删除帮会(EXEC)
//static LPCSTR szSQLSP_AddGuildObject					= "call addguildobject(%d,%d,%d)";//添加帮会联盟、敌对、通缉以及入帮请求数据(EXEC)
//static LPCSTR szSQLSP_DelGuildObject					= "call delguildobject(%d,%d,%d)";//删除帮会联盟、敌对、通缉以及入帮请求数据(EXEC)
// static LPCSTR szSQLSP_UpdateGuildBasicData				= "call updateguildbasicdata(%u,\"%s\",%d,%d,%d,%d,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%u,%d,%d,%d,%d)";//更新帮会基本数据(EXEC)
static LPCSTR szSQLSP_UpdateGuildBasicData				= "call updateguildbasicdata(%u,\"%s\",%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d,%d,%d)";//更新帮会基本数据(EXEC)

static LPCSTR szSQLSP_DeleteGuildWarData				= "call deleteguildwar(%u)";		//删除帮派战的数据
static LPCSTR szSQLSP_AddGuildWarData					= "call addguildwar(%u,%d,%d,%d,%d,%d,%d)";		//增加帮派战的数据
//static LPCSTR szSQLSP_UpdateGuildSkill					= "call updateguildskill(%d,%d,%d)";			//更新帮派技能等级
static LPCSTR szSQLSP_InitGuildSkill					= "call initguildskill(%u,%d,%d)";			//更新帮派技能等级
static LPCSTR szSQLSP_DeleteGuildSkill					= "call deleteguildskill(%u,%d)";			//更新帮派技能等级
static LPCSTR szSQLSP_DeleteGuildEvent					= "call cleanguildevent(%u)";					//删除帮派事件
static LPCSTR szSQLSP_AddeGuildEvent					= "call addguildenvent(%u,%d,%d,%d,%d,%d,\"%s\",\"%s\")";			//更新帮派事件
static LPCSTR szSQLSP_DeleteGuildMember					= "delete from actorguild where actorid=%u";//添加帮会成员(EXEC)

//数据客户端角色相关数据的SQL查询
// zac 2010-11-23
//根据帐号id查询角色列表，及角色的简单属性
static LPCSTR szSQLSP_LoadActorListByAccountId			= "call loadactorlistbyaccountid(%u,%d)";
/*****end*****/
static LPCSTR szSQLSP_LoadCharBasicData					= "call loadcharbasicdata(%u)";//查询角色基本数据
static LPCSTR szSQLSP_UpdateCharStatus					= "update actors set status = %d where actorid = %u limit 1";	//设置角色状态
static LPCSTR szSQLSP_UpdateCharOnlineStatus			= "call updatecharonlinestatus(%u,%d)";//更新在线标识
static LPCSTR szSQLSP_LoadCharOfflineData				= "call loadcharofflinedata(%u)";//查询角色离线的数据

static LPCSTR szSQLSP_LoadCircleCount                   = "call loadcirclecount()";//加载转生等级人数

static LPCSTR szSQLSP_LoadStaticCounts            		= "call loadstaticcount(%u)";// 加载玩家计数器
static LPCSTR szSQLSP_DeleteStaticCounts                = "delete from actorstaticcount where id = %u";//加载数据
static LPCSTR szSQLSP_InsertStaticCounts                = "insert into actorstaticcount(id,nstatictype,nvalue) values"; //插入数据


static LPCSTR szSQLSP_LoadGhosts           				= "call loadactorghost(%u)";// 
static LPCSTR szSQLSP_DeleteGhosts                		= "delete from actorghost where actorid = %u";//
static LPCSTR szSQLSP_InsertGhosts                		= "insert into actorghost(actorid,id,lv, bless) values"; //插入数据


static LPCSTR szSQLSP_LoadActorStrengthen           	= "call loadstrengthen(%u)";// 加载玩家强化数据
static LPCSTR szSQLSP_DeleteActorStrengthen             = "delete from actorstrengthen where actorid = %u";//delete
static LPCSTR szSQLSP_InsertActorStrengthen             = "insert into actorstrengthen(actorid,ntype,npos,nlv) values"; //插入数据

static LPCSTR szSQLSP_LoadAchieveMent            		= "call loadactorachieve(%u)";// 加载玩家成就数据
static LPCSTR szSQLSP_DeleteAchieveMent                 = "delete from actorachieve where actorid = %u";//加载数据
static LPCSTR szSQLSP_InsertAchieveMent                 = "insert into actorachieve(actorid,achievetype,nValue) values"; //插入数据

static LPCSTR szSQLSP_LoadOfflineAchieveMent            = "call loadofflineachieve()";// 加载成就数据
static LPCSTR szSQLSP_DelOfflineAchieveMent             = "delete from offlineachieve";//加载数据
static LPCSTR szSQLSP_InsertOfflineAchieveMent          = "insert into offlineachieve(actorid,type,subtype,value,way) values"; //插入数据

static LPCSTR szSQLSP_LoadHeroCharOfflineData			= "call loadherocharofflinedata(%u, %d)";//查询英雄离线的数据
static LPCSTR szSQLSP_LoadHeroOfflineEquips				= "call loadheroofflineequips(%u,%d)";//查询英雄装备

static LPCSTR szSQLSP_LoadCharItems						= "call loadcharitems(%u,%d)";//查询角色装备、背包以及仓库物品

static LPCSTR szSQLSP_LoadCharSkills					= "call loadcharskills(%u)";////查询角色技能
//static LPCSTR szSQLSP_LoadMainQuest						= "call loadmainquest(%d)";//查询角色主线任务的完成情况

static LPCSTR szSQLSP_LoadActorGameSetData2				= "select setting from actorgameotherSets where actorid=%u"; //获取玩家的二进制数
static LPCSTR szSQLSP_UpdateGameSetData2				= "update actorgameotherSets set `setting`=";//更新设置的2进制数据

static LPCSTR szSQLSP_LoadOtherGameSets				    = "call loadgameothersetsdata(%u)";////查询游戏其他设置---已经有部分设置在其他表里
static LPCSTR szSQLSP_DeleteOtherGameSets    			= "delete from actorgameotherSets  where actorid = %u";//删除游戏其他设置
static LPCSTR szSQLSP_AddOtherGameSets    			    = "insert into actorgameotherSets(actorid,kb) values";//添加游戏其他设置

static LPCSTR szSQLSP_LoadCharRoleItems					= "call loadcharroleitems(%u)";//查询角色任务进度数据
static LPCSTR szSQLSP_LoadCharFinishRoles				= "call loadcharfinishroles(%u)";//查询角色完成任务数据
//static LPCSTR szSQLSP_LoadCharShortCuts					= "call loadcharquickitemdistribute(%d)";//查询角色快捷键数据
//static LPCSTR szSQLSP_LoadCharTalismans                 = "call loadchartalismans(%d)";//查询角色的法宝数据
static LPCSTR szSQLSP_UpdateCharBasicData				= "call updatecharbasicdata(%u,%d,%d,%d,%d,%d,%d,%lld,%d,%d,%d,%d,%d,%d,%u,%u,%u,%d,%d,%d,%u,\"%s\",%d,%d,%d,%lld,%d,%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%d,%d,%d,%d,%d,%d,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%d,%d,%d,%d,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lld,";//更新角色基本数据(EXEC)
static LPCSTR szSQLSP_UpdateActorBinaryData				= "call updatecharbinarydata(%lld,";//更新角色的2进制数据
static LPCSTR szSQLSP_UpdateActorOfflineData			= "call updateactorofflinedata(%u,";
static LPCSTR szSQLSP_UpdateActorCustomInfo				= "call updateactorCustomInfo(%u,";
static LPCSTR szSQLSP_LoadActorCustomInfo				= "call loadactorCustomInfo(%u)";
static LPCSTR szSQLSP_ChangeTradingQuota				= "call changetradingquota(%u,%lld)";// 更改交易额度

static LPCSTR szSQLSP_UpdateActorScriptData				= "update actorvariable set `variable`=";//更新角色的2进制数据
static LPCSTR szSQLSP_LoadActorScriptData				= "select variable from actorvariable where actorid=%u"; //获取玩家的二进制数

static LPCSTR szSQLSP_CleanCharItems					= "call cleancharitems(%u,%d)";//删除角色装备、背包以及仓库物品(EXEC),actorID,nItemtableID(1:背包，2：装备，3：仓库)

//向一个物品表里插入全部的物品数据，可能是背包物品，也可能是装备物品也可能是仓库物品
static LPCSTR szSQLSP_AddCharItems                    ="insert into %s(actorid,itemguid,itemidquastrong,itemduration,itemcountflag,iteminlayhole,itemtime,itemreservs,itemsmith1,itemsmith2,itemsmith3, itemsmith4, itemsmith5, itemreservs2,initsmith,bestAttr,refining,sourcename) values ";   //插入到背包物品表


static LPCSTR szSQLSP_CleanCharSkills					= "call cleancharskills(%u)";//删除角色技能(EXEC)
//static LPCSTR szSQLSP_AddCharNewSkill					= "call addcharnewskill(%d,%d,%d,%d,%d)";//添加角色技能(EXEC)
static LPCSTR szSQLSP_AddCharAllSkills					= "insert into skill(actorid,skillidlvmj,cdtime,reserve,mjexpiretime,miji) values";//添加角色技能(EXEC)

static LPCSTR szSQLSP_CleanCharRoleItems				= "call cleancharroleitems(%u)";//删除角色任务进度(EXEC)
static LPCSTR szSQLSP_AddCharNewRoleItem				= "insert into goingquest(actorid,idtask,type,value,state,complete,usefstimes,nStartId) values ";//添加角色任务进度(EXEC)
// static LPCSTR szSQLSP_CleanCharRoleItems				= "call cleancharroleitems(%u)";//删除角色任务进度(EXEC)
// static LPCSTR szSQLSP_AddCharNewRoleItem				= "call addcharnewroleitem(%u,%d,%d,%d)";//添加角色任务进度(EXEC)
// static LPCSTR szSQLSP_CleanCharFinishRoles				= "call cleancharfinishroles(%u)";//删除角色完成任务(EXEC)
// static LPCSTR szSQLSP_AddCharNewFinishRole				= "call addcharnewfinishrole(%u,%d,%d)";//添加角色完成任务(EXEC)
//static LPCSTR szSQLSP_CleanCharTalismans                = "call cleanchartalismans(%d)";//删除角色法宝数据（EXEC）

//static LPCSTR szSQLSP_AddCharTalismans                  = "call addchartalismans(%d)";//添加角色法宝数据（EXEC）
//数据客户端角色通知相关数据的SQL查询
//static LPCSTR szSQLSP_LoadCharNotify					= "call loadcharnotify(%d)";//查询角色下一个通知数据
//static LPCSTR szSQLSP_AddCharNotify						= "call addcharnotify(%d,\"%s\",\"%s\")";//添加一个角色通知数据(EXEC)
//static LPCSTR szSQL_DelCharNotify						= "delete from charnotify where idx=%d";//删除一个角色通知数据(EXEC)

static LPCSTR szSQLSP_RenameCharactor					= "call renamecharactor(%u,\"%s\")";//执行角色重命名

//static LPCSTR szSQLSP_AddCharFubenData					= "call addcharfubendata(%d,%d,%d,%d)";//添加角色副本数据
//static LPCSTR szSQLSP_LoadCharFubenData					= "call loadcharfubendata(%d)";
//static LPCSTR szSQLSP_CleanCharFuben					= "call cleancharfubendata(%d)";

//static LPCSTR szSQLSP_AddFriend							= "call addcharfriend(%d,%d,%d,'%s',%u)";//添加角色交互系统的数据
//static LPCSTR szSQLSP_LoadCharFriends					= "call loadcharfriends(%d)";
//static LPCSTR szSQLSP_CleanCharFriends					= "call cleancharfriends(%d)";


//消息系统
static LPCSTR szSQLSP_LoadCharMsgList					= "call loadcharmsglist(%u,%lld)";//读取角色的所有消息
static LPCSTR szSQLSP_DeleteCharMsg						= "call deletecharmsg(%u,%lld)";	//删除角色的一条消息
static LPCSTR szSQLSP_AddCharMsg						= "call addcharmsg(%u,%d,";
static LPCSTR szSQLSP_AddCharMsgByName					= "call addcharmsgbyactorname(%d,\"%s\",%d,";	// 根据玩家角色名添加离线消息

static LPCSTR szLoadUserItem							= "call loaduseritem(%u,%u,%d)";//获取角色的活动背包内容,[账户id，角色id,serverindex]
static LPCSTR szGetUserItem								= "call getuseritem(%u,%u,%lld,%d)";//领取物品[账户id，角色id,消息id]
static LPCSTR szDelUserItem								= "call deleteuseritem(%lld)";//删除用户的活动背包

static LPCSTR szSQLSP_LoadAlmirah						= "call loadralmirah(%u)";		//加载衣橱系统
static LPCSTR szSQLSP_DeleteAlmirah						= "delete from actoralmirahitem where actorid = %u"; //删除玩家
static LPCSTR szSQLSP_InsertAlmirah						= "insert into actoralmirahitem(actorid,nId,nlv,ntakeon) values ";


static LPCSTR szSQLSP_AddCombatBaseInfo				   = "insert into combatgame(actorid,winningtimes,chagangetimes,nextchagangetime,refreshtimes,awardrank,addchagneTimes,bshowflag) values";//添加角色技能(EXEC)
static LPCSTR szSQLSP_AddCombatRecordInfo			   = "insert into combatrecord(actorid,tagetId,tagetname,nvictory,ntime,nranking,nawardtype1,nawardtype2,nawardtype3,nawardvalue1,nawardvalue2,nawardvalue3) values";//添加角色技能(EXEC)

static LPCSTR szSQLSP_AddGuildDepotItem				   = "insert into guildstore(guildid,actorid,itemguid,itemidquastrong, itemduration, itemcountflag,iteminlayhole,itemtime,itemreservs,itemsmith1,itemsmith2,itemsmith3,itemreservs2,initsmith,ntype,nmoney,exchangeactorid,exchangeactorname,exchangetime) values";//添加角色技能(EXEC)
static LPCSTR szSQLSP_DeleteDepotItem				   = "delete from guildstore where guildid=%u";		//删除行会仓库物品
static LPCSTR szSQLSP_LoadDepotItem					   = "call loadguildstoredata(%u)";
static LPCSTR szSQLSP_LoadDepotRecord				   = "call loadguilddepotrecord(%u)";			//加载行会仓库操作记录
static LPCSTR szSQLSP_DeleteGuildDepotRecord		   = "call deleteguilddepotrecord(%u)";			//删除仓库操作记录
static LPCSTR szSQLSP_AddGuildDepotRecord			   = "insert into guildstorerecord(guildid,ntype,ntime,itemnum,cointype,coinnum,actorname,itemname) values";	//							

static LPCSTR szSQLSP_DeleteTimeRideData				= "delete from periodride where nactorid=%u";
static LPCSTR szSQLSP_AddTimeRidesData					= "insert into periodride(nactorid,nrideid,ntime) values";

//好友
static LPCSTR szSQLSP_LoadFriendsData					= "call loadfriendsdata(%u)";			//加载好友列表
static LPCSTR szSQLSP_SaveFriendsData					= "call deletefriends(%u)";				//删除好友列表
static LPCSTR szSQLSP_AddFriendsData					= "insert into friends(nactorid,friendid,ntype,nintimacy,warteamid,dotime,color) values";

static LPCSTR szSQLSP_LoadActorDeathData				= "call loadactordeath(%u)";			//加载
static LPCSTR szSQLSP_DelActorDeathData					= "call deleteactordeath(%u)";			//删除
static LPCSTR szSQLSP_AddActorDeathData					= "insert into actordeath(actorid, killerid, deathtime, killername,killsceneid,killposx,killposy, type) values";

static LPCSTR szSQLSP_LoadActorDeathDropData			= "call loadactordeathdrop(%u)";			//加载
static LPCSTR szSQLSP_DelActorDeathDropData				= "call deleteactordeathdrop(%u)";			//删除
static LPCSTR szSQLSP_AddActorDeathDropData				= "insert into actordeathdrop(actorid, killerid, deathtime, dropid, dropcount) values";

static LPCSTR szSQLSP_DeleteFriendsData					= "update friends set ntype=%d,nintimacy=%d,warteamid=%lld where nactorid=%u and friendid=%u";
static LPCSTR szSQLSP_DeleteOtherFriendsData			= "update friends set ntype=ntype&~(1<<1) where nactorid=%u and friendid=%u";
static LPCSTR szSQLSP_AddFriendsChatMsg					= "insert into friendchatmsg(nactorid,tagetid,ntime,chatmsg) values(%u, %u, %d,\"%s\")";	
static LPCSTR szSQLSP_LoadFriendsChatMsgData			= "call loadfriendcharmsg(%u)";			//加载好友列表
static LPCSTR szSQLSP_DeleteFriendsChatMsgData			= "delete from friendchatmsg where nactorid=%u";
static LPCSTR szSQLSP_GetChatMsgCount					= "select count(*) from friendchatmsg where nactorid=%u";
//static LPCSTR szSQLSP_LoadFriendsOfflineData			= "call loadfriendsofflinedata(%d)";			//加载N天未上线的好友


static LPCSTR szSQLSP_GetJopTopLevelActor				= "select actorid,actorname,job,circle,level,exp,sex from actors where job=%d and (actors.status & 2) =2 and serverindex=%d order by circle desc,level desc,exp desc limit 1";
static LPCSTR szSQLSP_GetActorConsumeRank				= "select actorid,actorname,(actors.honourlv & 0xffff) from actors where (actors.status & 2) =2 and serverindex=%d and honourlv > 0 order by honourlv desc limit 3";
static LPCSTR szSQLSP_GetActorBattleRank				= "select actorid,actorname,job,battlepower,sex from actors where job=%d and (actors.status & 2) =2 and serverindex=%d order by battlepower desc limit 1";
static LPCSTR szSQLSP_GetActorSwingLevelRank			= "select actorid,actorname,battlepower,fashionbattle,swinglevel,job,sex from actors where fashionbattle>0 and (actors.status & 2) =2 and serverindex=%d order by fashionbattle desc,battlepower desc limit 3";
static LPCSTR szSQLSP_GetActorHeroTopicRank				= "select actors.actorid,actors.actorname,actorpets.circle,actorpets.level,actorpets.exp,actorpets.name from actorpets,actors where (actors.status & 2) =2 and actors.actorid=actorpets.actorid and actors.serverindex=%d order by actorpets.circle desc,actorpets.level desc,actorpets.exp desc limit 3";

static LPCSTR szSQLSP_GetTopLevelActor				    = "select avg(level) from (select level from actors where (actors.status & 2) =2 and serverindex=%d order by level desc limit 10)  as level";

static LPCSTR szSQLSP_GetCombineTopLevelActor				= "select actorid,actorname,job,circle,level,exp from actors where (actors.status & 2) =2 and serverindex=%d and (circle > 0 or level >= 60) order by circle desc,level desc,exp desc";
static LPCSTR szSQLSP_GetCombineBattleRank				= "select actorid,actorname,job,battlepower from actors where (actors.status & 2) =2 and serverindex=%d order by battlepower desc limit 10";

static LPCSTR szSQLSP_GetAllWholeActivityData			= "call loadwholeactivitydata(%d,%d,%d)";			//全民活动

static LPCSTR szSQLSP_LoadActorGameSetData					= "call loadgamesetdata(%u)";			//加载游戏设置数据
static LPCSTR szSQLSP_DeleteActorGameSetData				= "call deletegamesetdata(%d)";				//删除游戏设置数据
static LPCSTR szSQLSP_AddActorGameSetData					= "call savegamesetdata(%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)";				//保存游戏设置数据
static LPCSTR szSQLSP_UpdateGameSetData						= "call updategamesetdata(%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)";
//清除玩家的宠物的数据
static LPCSTR szSQLSP_CleanPetData					= "call cleanactorpets(%u)";//删除角色的宠物数据(EXEC)

static LPCSTR szSQLSP_QueryFilterWordsData			= "select * from filternames where namestr=\"%s\"";
static LPCSTR szSQLSP_AddFilterWordsData			= "insert into filternames(namestr) values(\"%s\")";
static LPCSTR szSQLSP_DelFilterWordsData			= "delete from filternames where namestr=\"%s\"";

//添加宠物
static LPCSTR szSQLSP_AddPets				   = "insert into actorpets(actorid,exp,hp,mp,name,circle,level,ids,modelid,weaponappear,score,vocation,sex,swingappear,reserver1,reserver2,stage,bodyforce,bless) values";//添加角色技能(EXEC)

//装载宠物的数据
static LPCSTR szSQLSP_LoadPetData					= "call loadactorpets(%u)";

//装载宠物的技能数据
static LPCSTR szSQLSP_LoadPetSkillData				= "call loadpetskills(%u)";

static LPCSTR szSQLSP_cleanPetSkillData				= "call cleanpetskills(%u)";

static LPCSTR szSQLSP_addPetSkillData				= "insert into petskills(actorid,petid,skillid,skilllevel,reserver,skillexp) values";

static LPCTSTR szSQL_SaveTopRank					= "call saverank(%d)";
static LPCTSTR szSQL_LoadTopRank					= "call loadrank(%d, %d)";
//static LPCTSTR szSQL_SaveTopRankCS					= "call saverankCS(%d)";//跨服排行榜保存 ，限制日期的
static LPCTSTR szSQL_SaveTopRankCS					= "call saverankCS(%d, \"%s\")";//跨服排行榜保存 ，限制日期的

static LPCTSTR szSQL_InitDB							= "call initdb(%d)";//初始化数据库

//玩家关系数据
static LPCTSTR szSQL_LoadRelation					= "call loadrelation(%u)";
static LPCTSTR szSQL_AddRelation					= "insert into actorrelation(actorid,relationtype,relationid,param) values";
static LPCTSTR szSQL_CleanRelation					= "call cleanrelation(%u)";

// 更新个人擂台数据
static LPCTSTR szSQL_UpdateChallengeData			= "call updatechallengedata(%d)";

//gm删除金钱和物品
static LPCTSTR szSQLSP_DeleteGuidItem				= "call deleteitembyguid(%d, %lld)";   //gm从后台删除离线玩家物品(物品表id,物品GUID)
static LPCTSTR szSQLSP_DeleteActorMoney				= "call deleteactormoney(\"%s\", %d, %d)";	 //GM从后台删除玩家金钱

//邮件系统
static LPCTSTR szSQLSP_DeleteMail					= "call cleanmail(%u)";
static LPCTSTR szSQLSP_LoadMail						= "call loadmail(%u)";
static LPCTSTR szSQLSP_SaveMail						= "insert into mail(mailid,actorid,srcid,title,content,createdt,state,isdel) values ";
static LPCTSTR szSQLSP_SaveMailAttach				= "insert into mailattach(mailid,actorid,type,itemguid,itemidquastrong,itemduration,itemcountflag,iteminlayhole,itemtime,itemreservs,itemsmith1,itemsmith2,itemsmith3, itemsmith4, itemsmith5, itemreservs2,initsmith) values "; 
static LPCTSTR szSQLSP_LoadNewServerMail			= "call loadservermail(%d)";
static LPCTSTR szSQLSP_LoadNewServerMailAttach		= "call loadservermailAttach(%d)";
static LPCTSTR szSQLSP_SaveNewServerMail			= "call updateservermail(%d)";
static LPCTSTR szSQLSP_SaveServerMail				= "insert into servermail(actorid,title,content,minviplevel,maxviplevel,minlevel,maxlevel,minregistertime,maxregistertime,minlogintime,maxlogintime,online,sex,way) values ";
static LPCTSTR szSQLSP_SaveServerMailAttach			= "insert into servermailattach(servermailid,type,id,count) values "; 
static LPCTSTR szSQLSP_GetMaxMailIdServerMail		= "select max(servermailid) from servermail";
static LPCSTR szSQLSP_LoadActorNewTitle			= "call loadactornewtitle(%u)";
static LPCSTR szSQLSP_DelActorNewTitle			= "call cleanactornewtitle(%u)";
static LPCSTR szSQLSP_AddActorNewTitle			= "insert into actornewtitle(actorid,idmakelong,remaintime) values";
static LPCSTR szSQLSP_LoadActorCustomTitle			= "call loadactorcustomtitle(%u)";
static LPCSTR szSQLSP_DelActorCustomTitle			= "call cleanactorcustomtitle(%u)";
static LPCSTR szSQLSP_AddActorCustomTitle			= "insert into actorcustomtitle(actorid,idmakelong,remaintime) values";

//复活特权
static LPCSTR szSQLSP_LoadReviveDurationData		= "call loadactorreviveduration(%u)";
static LPCSTR szSQLSP_DelReviveDurationData			= "call cleanactorreviveduration(%u)";
static LPCSTR szSQLSP_AddReviveDurationData			= "insert into actorreviveduration(actorid,idmakelong,remaintime) values";


static LPCSTR szSQLSP_LoadNewHundredBattle  	= "call loadnewhundredbattle(%d,%d)";	 //查询玩家战力数据

static LPCSTR szSQLSP_LoadActorWarChariot			= "call loadactorwarchariot(%u)";
static LPCSTR szSQLSP_DelActorWarChariot			= "call cleanactorwarchariot(%u)";
static LPCSTR szSQLSP_AddActorWarChariot			= "insert into actorwarchariot(actorid,curhp,repairtime,parmmakelong) values";


static LPCSTR szSQLSP_LoadAccountPsw				= "select psw from accountpsw where accountid=%u and serverindex=%d ";
static LPCSTR szSQLSP_CreateAccountPsw				= "insert into accountpsw(accountid,serverindex,psw) values (%u,%d,'%s')";

//寄卖
static LPCSTR szSQLSP_LoadConsignmentItem				= "call loadconsignmentitem(%d)";
static LPCSTR szSQLSP_DeleteConsignmentItem				= "call deleteconsignmentitem(%u,%d)";
static LPCSTR szSQLSP_AddCharConsignmentItem			= "insert into consignmentitem(actorid,itemguid,itemidquastrong,itemduration,itemcountflag,iteminlayhole,itemtime,itemreservs,itemsmith1,itemsmith2,itemsmith3, itemsmith4, itemsmith5, itemreservs2,initsmith,bestAttr,sourcename,selltime,price,state,serverindex,autoid) values ";   //增加寄卖物品
// static LPCSTR szSQLSP_AddCharConsignmentItem			= "insert into consignmentitem(actorid,itemid,itemcount,selltime,price,state,serverindex, bestAttr,itemstar) values ";   //增加寄卖物品

static LPCSTR szSQLSP_LoadConsignmentIncome				= "call loadconsignmentincome(%d)";
static LPCSTR szSQLSP_DeleteConsignmentIncome			= "call deleteconsignmentincome(%u,%d)";
static LPCSTR szSQLSP_AddCharConsignmentIncome			= "insert into consignmentincome(actorid,income,serverindex) values ";   //增加寄卖物品

// 圣物
static LPCSTR szSQLSP_UpdateHallowseData = "call updateactorhallow(%u,%d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, \"%s\",\"%s\",\"%s\",\"%s\", \"%s\",\"%s\",\"%s\",\"%s\")";
static LPCSTR szSQLSP_LoadGHallowseData  = "call loadactorhallow(%u)";
//跨服  
static LPCSTR szSQLSP_CrateCrossServerCharactor			= "call crossserverCreate(%u,\"%s\",%llu,%u,\"%s\",%d,%d,%d,%d,%d,\"%s\",%d)";//用户创建角色
static LPCSTR szSQLSP_QueryeCrossServerCharactor			= "select actorid from actors where actorid = %d";//
static LPCSTR szSQLSP_QueryeCrossServerGuildId			= "select guildid,socialmask from actors where actorid = %d";//


static LPCSTR szSQLSP_LoadLootPetData            		= "call loadactorlootpet(%u)";// 加载玩家 宠物系统非 ai宠物 数据
static LPCSTR szSQLSP_DelLootPetData                    = "call delactorlootpet(%u)"; //加载数据
static LPCSTR szSQLSP_InsertLootPetData                 = "insert into actorlootpet(actorid,type,id,continueTime) values"; //插入数据

static LPCSTR szSQLSP_LoadRebateData            		= "call loadactorrebatedata(%u)";// 加载玩家 返利系统 数据
static LPCSTR szSQLSP_LoadNextDayRebateData            	= "call loadactornextdayrebatedata(%u,%d)";// 加载玩家 次日发放 返利系统数据
static LPCSTR szSQLSP_UpdateRebateData                  = "call updateactorrebatedata(%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)"; //更新 返利系统 数据
static LPCSTR szSQLSP_SaveRebateData                 	= "call saveactorrebatedata(%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)"; //保存 返利系统 数据
static LPCSTR szSQLSP_ClearRebateData                 	= "call clearactorrebatedata(%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)"; //清理 返利系统 数据

#endif
