#ifndef			_LOG_TYPE_H_
#define			_LOG_TYPE_H_

using namespace jxSrvDef;

//以下4个定义用于命令分发，表明本数据包是什么类型的日志数据包
#define LOG_LOGIN_CMD	1		//登陆日志的数据包
#define LOG_CONSUME_CMD 2		//消费日志的数据包
#define LOG_ONLINE_CMD	3		//在线日志的数据包
#define LOG_COMMON_CMD  4		//公共操作日志的数据包
#define LOG_SUGGEST_CMD  5		//建议操作日志的数据包
#define LOG_CREATETABLE_CMD 6	//创建公共表

#define LOG_LOOKCHATMSG_CMD 7	//发送给逻辑服查看日志的标记
#define LOG_SENDSERVERINDEX_CMD 8  //发送逻辑服id
#define LOG_SENDCHATRECORD_CMD 9	//发送聊天记录给查看器

#define LOG_SEND_CURRENCY_CMD 10 //发送资产日志
#define LOG_SEND_SHOP_CMD 11 //发送商城日志
#define LOG_SEND_ITEM_CMD 12 //发送道具日志
#define LOG_SEND_TASK_CMD 13 //发送任务日志
#define LOG_SEND_ACTIVITY_CMD 14 //发送活动日志
#define LOG_DEAL_CMD 15 //交易日志
#define LOG_ITEAM_CMD 16 //道具日志
#define LOG_KILLDROP_CMD 17 //击杀掉落日志
#define LOG_JOINATV_CMD 18 //参与活动进度
// 这里定义所有登陆日志的各个类型值
// 注意，类型值没定义完整，后续添加
typedef  enum  tagLogTypeLogin
{
		ltLogin = 1,		//登陆
		ltLogout = 2,		//下线
		ltQueryActor = 3,		//查询角色列表
		ltCreateActor = 4,		//创建角色
		ltCreateActorResult = 5,//创建角色的结果
		ltDelActor = 6,			//删除角色
		ltEntryGame = 7,		//进入游戏(进入逻辑服务器)
		ltEntryGameByTools = 8,	//登陆器登陆
		ltHotIp =100,           //登陆繁忙的IP
		itSealAccount,          //封账户
		ltSealIp,               //封IP
} LOG_TYPE_LOGIN;


typedef  enum  tagLogTypeSuggestType
{
	stBug = 1,		    //提交bug
	stComplaint = 2,    //投诉	
	stSuggest = 3,		//建议
	stOther = 4,		//其他建议
	stChat = 5,			//聊天
	stQuickKick = 6,		//使用外挂踢下线
	stQuickSeal = 7,		//使用外挂封角色
	
} LOG_SUGGEDST_TYPE;




typedef  struct  tagLogDataLogin{
	WORD			nCmdId;			//日志大类标识
	int 			nLogid;			//日志标识，每种日志类型对应唯一值
	unsigned int 			nUserid;		//和本条日志相关的用户的id号，可以为空
	ACCOUNT 		szAccount;	//用户帐号
	int				nServerIndex;	//发送本条日志的服务器的id号
	int				nLevel;//等级
	int				nYb;//元宝
	int				nOnlineTime;//在线时间（秒）
	char			szLoginIp[20];	//登录的客户端ip地址
	int				nServerType;
	char			szLoginDescr[32];	//日志描述
	char			szSpid[32];
	int			nSrvId;	//原serverid
}LOGDATALOGIN,*PLOGDATALOGIN;

typedef  struct  tagLogDataConsume{
	WORD		nCmdId;			//日志大类标识
	int			nLogid;			//日志标识，每种日志类型对应唯一值
	int			nActorid;		//和本条日志相关的角色的id号，可以为空
	ACCOUNT		szAccount;		//用户帐号
	int			nServerIndex;	//发送本条日志的服务器的id号
	ACTORNAME	szCharName;		//角色名
	BYTE		cMoneytype;		//金钱类型
	int			nBalance;		//余额
	int         nConsumecount;	//消费个数
	int			nPaymentCount;	//消费产出数目
	char		szConsumeDescr[32];	//描述
	int         nConsumeLevel;	//玩家的消费的等级
	char		szSpid[32];
	int			nSrvId;	//原serverid

}LOGDATACONSUME,*PLOGDATACONSUME;


typedef  struct  tagLogDataItem{
	WORD		nCmdId;			//日志大类标识
	BYTE		nType;		//日志类型 1 获得 2 删除
	int			nLogid;			//日志标识，每种日志类型对应唯一值
	int			nActorid;		//和本条日志相关的角色的id号，可以为空
	ACCOUNT		szAccount;		//用户帐号
	int			nServerIndex;	//发送本条日志的服务器的id号
	ACTORNAME	szCharName;		//角色名
	WORD		wItemId;		//道具id
	int			nCount;		// 数量
	char		szConsumeDescr[32];	//描述
	char		szSpid[32];
	int			nSrvId;	//原serverid

}LOGDATAITEM,*PLOGDATAITEM;

//全局日志 交易获得
typedef struct tagActorDealLog
{
	int				nLogId;
	unsigned int	nMiniTime;	//时间
	int				nActorid;	//流入角色id
	int				nTarActorid;//流出角色id

	INT 			nItemId;	//物品id
	WORD 			nItemNum;
	WORD 			nMoneyType;
	INT64 			nMoneyNum;
	INT 			nDues;
	int				nServerIndex;	//发送本条日志的服务器的id号
	char			szSpid[32];
	int			nSrvId;	//原serverid

} LOGACTORDEALSAVE,*PLOGACTORDEALSAVE;

typedef  struct  tagLogDataOnlineCount{
	WORD			nCmdId;			//日志大类标识
	int			nServerIndex;//发送本条日志的服务器的id号
	int			nOnlineCount;//本时刻的在线人数
	char		szSpid[32];
}LOGDATAONLINECOUNT,*PLOGDATAONLINECOUNT;

typedef  struct  tagLogDataCommon{
		WORD			nCmdId;			//日志大类标识
		int 			nLogid;			//日志标识，每种日志类型对应唯一值
		unsigned int 	nActorid;		//和本条日志相关的角色的id号，可以为空
		ACTORNAME		szName;			//角色名字
		ACCOUNT 		szAccount;		//用户帐号
		int				nServerIndex;	//发送本条日志的服务器的id号
//以下为非固定字段	
/*
公共操作日志的类型会比较多，为使日志服务器具有通用性和易扩展性，服务器接收的数据包不明确定义每个字段的详细含义，
而是预先定义足够用的字段，每个字段的含义由每个写入日志的服务器决定。
*/
		int			nPara0;	
		int			nPara1;	
		int			nPara2;	
		char		szShortStr0[10];
		char		szShortStr1[10];
		char		szMidStr0 [20];
		char		szMidStr1 [20];
		char		szMidStr2[20];
		char		szLongStr0[32];
		char		szLongStr1[32];
		char		szLongStr2[32];
		char		szSpid[32];
		int			nSrvId;	//原serverid
}LOGGERDATACOMMON,*PLOGGERDATACOMMON;

//建议的内容
typedef  struct  tagLogDataSuggest{
	WORD			nCmdId;			//日志大类标识
	ACCOUNT 		szAccount;		//用户帐号
	char			szCharName[ACTOR_NAME_BUFF_LENGTH];
	WORD			nServerIndex;	//发送本条日志的服务器的id号
	WORD            nType;          //类型

	char        szTitle[128];			//标题
	char        szSuggest[512];			//建议
	int        nActorLevel;        //玩家的等级
}LOGGERDATASUGGEST,*PLOGGERDATASUGGEST;

#pragma pack(push, 4)

//资产日志表
typedef struct tagCurrencyLog{
	WORD	nLogId;			//日志ID（消费点ID）
	WORD	nLogType;		//日志类型（1产出 2消费）
	WORD	nCurrencyType;	//货币类型（1金币 2绑金 3银两 4元宝 ）
	WORD	nIsShop;		//是否商城消费（1是 2否）
	INT64	nAmount;		//消费金额
	INT64	nBalance;		//当前余额

	unsigned int 	nActorid;		//角色id
	ACTORNAME		szName;			//角色名字
	ACCOUNT 		szAccount;		//用户帐号
} LOGCURRENCY,*PLOGCURRENCY;

//商城日志表
typedef struct tagShopLog{
	INT		nItemId;		//道具ID
	WORD	nShopType;		//商城类型
	WORD	nItemType;		//道具类型ID
	WORD	nItemNum;		//购买道具数
	WORD	nCurrencyType;	//货币类型（1金币 2绑金 3银两 4元宝 ）
	INT64	nConsumeNum;	//消耗货币总额

	unsigned int 	nActorid;		//角色id
	ACTORNAME		szName;			//角色名字
	ACCOUNT 		szAccount;		//用户帐号
} LOGSHOP,*PLOGSHOP;

//道具日志表
typedef struct tagItemLog{
	INT		nItemId;		//道具ID
	WORD	nItemType;		//道具类型ID
	WORD	nItemNum;		//获得/消耗道具数
	WORD	nOperType;		//操作类型（1获得，2消耗）
	char	sReason[64];	//获得/消耗道具的原因

	unsigned int 	nActorid;		//角色id
	ACTORNAME		szName;			//角色名字
	ACCOUNT 		szAccount;		//用户帐号
} LOGITEM,*PLOGITEM;

//任务日志表
typedef struct tagTaskLog{
	INT		nTaskId;		//任务ID
	WORD	nTaskType;		//任务类型
	WORD	nStatu;			//状态（1接收，2完成）

	unsigned int 	nActorid;		//角色id
	ACTORNAME		szName;			//角色名字
	ACCOUNT 		szAccount;		//用户帐号
} LOGTASK,*PLOGTASK;

//活动日志表
typedef struct tagActivityLog{
	INT		nAtvId;			//活动ID
	WORD	nAtvType;		//活动类型
	WORD	nStatu;			//状态（1接收，2完成）

	unsigned int 	nActorid;		//角色id
	ACTORNAME		szName;			//角色名字
	ACCOUNT 		szAccount;		//用户帐号
} LOGACTIVITY,*PLOGACTIVITY;

//击杀掉落日志
typedef struct tagLogKillDrop
{
	WORD			nCmdId;			//日志大类标识
	int			nActorid; 		//角色id
	ACCOUNT		srtAccount;		//账号
	ACTORNAME	strName;		//角色昵称
	ACTORNAME	strMonsetName;	//怪物昵称
	ACTORNAME	strSceneName;	//场景名称
	ACTORNAME	strItemName;	//道具名称
	int			nServerIndex;	//发送本条日志的服务器的id号
	int			nCount;			//数量
	int         nKilltime;		//掉落时间
	int         nPosx;			//x坐标
	int         nPosy;          //y坐标
	int			nSrvId;	//原serverid
}LOGKILLDROP,*PLOGKILLDROP;


typedef struct tagAtvSchedule
{
	WORD			nCmdId;			//日志大类标识
	int			nServerIndex;	//发送本条日志的服务器的id号
	int			nActorid; //角色id
	ACCOUNT		srtAccount;  //角色昵称
	ACTORNAME	strName;//角色昵称
	int			nAtvID;	//活动id
	int         nIndex;	//活动进度
	int         nJoinTime; //参与时间
	int			nSrvId;	//原serverid
}LOGATVSCHEDULE;

#pragma pack(pop)

#endif
