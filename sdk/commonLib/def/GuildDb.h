#pragma once
#define		MAX_EVENTCOUNT		50				//200  行会日志最大条数
#define     MAX_GUILDDEPOTBAG   24
#define     MAX_GUILDDEPOTRECOUD 100
#define MAX_GUILDTITLENUM 2000

#define     GUILDDEPOT_PAGE_GRIDS			30		//行会仓库每页有42个格子
#define     GUILDDEPOT_PAGE_GRIDS_CHECK		12		//需要审核行会仓库每页有12个格子


class EntityHandle;
#include "UserItem.h"

//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)
typedef struct tagActorNodeSimple
{
	unsigned int		nActorId;	
	INT64				nGuildGX;	//个人的贡献值
	unsigned int				nVipGrade;
	BYTE				nSex;
	BYTE				nLevel;
	BYTE                nCircle; //转数
	BYTE				nJob;
	BYTE				nMenPai;
	BYTE				nType;	//地位
	BYTE				nTK;	//堂口
	UINT				nZzNext;	//在职时间
	UINT				nZjNext;	
	int					nGuildTiTle;		//封号序号
	int					nTime;		//最近登陆的时间
	int					nTotalGx;	//在本行会的贡献值
	int					nIcon;		//头像id
	int					nBattleValue;		//战力
	int					nGuildCityPos;		//皇城职位(攻城)(已经不用)
	int					nModelId;		//模型id
	int					nWeaponId;		//武器外观
	int					nSwingId;		//翅膀外观
	int					nJoinTime;			//加入行会的时间
}ActorNodeSimple;

typedef struct tagActorNode
{
	tagActorNodeSimple	BasicData;
	ACTORNAME			sActorName;
	EntityHandle		hActorHandle;
	//bool				boStatus;//这个角色是否已删号
	tagActorNode()
	{
		ZeroMemory(this,sizeof(*this));
		//boStatus = TRUE;
	}
	//DECLARE_OBJECT_COUNTER(tagActorNode)
}ActorCommonNode;

typedef struct tagWarGuildHistory
{
	int					nGid;	//对方帮派的id
	int					nPkCount;		//杀敌数
	int					nDieCount;		//己方被杀数量
	char				sEndTime[20];	//下次变化状态的时间
	char				sGuildName[32];	//对方帮派的名称

	tagWarGuildHistory()
	{
		sGuildName[0] = 0;
	}
	//DECLARE_OBJECT_COUNTER(tagWarGuildHistory)
}WarGuildHistory;

//帮派技能
typedef struct tagGuildSkill
{
	int nSkillId;	//帮派技能的id
	int nLevel;		//帮派技能的等级
	tagGuildSkill()
	{
		memset(this,0,sizeof(*this));
	}
}GUILDSKILLS,*PGUILDSKILLS;

typedef struct tagEventMsg
{
	int aTime;							//事件发生的时间
	char nEventMsg[512];				//发生的事件
	int mEventId;						//事件id
	int mParam1;						//参数1
	int mParam2;						//参数2
	int mParam3;						//参数3
	char mParam4[32];					//参数4
	char mParam5[32];					//参数5
	tagEventMsg()
	{
		memset(this,0,sizeof(*this));
	}
}GUILDEVENTRECOUD;

class CGuild;

typedef struct tagGuildTitle
{
	char nTitleName[32];					//行会封号
}GUIlDTITLES;

//行会沙巴克战是的命令
typedef struct tagGuildCmdStr
{
	BYTE  bCmdId;						//1 集合 2 进攻  3 收兵
	char strCmd[32];					//行会封号
}GUILDCMDSTR;

//请求成为联盟
typedef struct tagInterRelationMsg
{
	unsigned int nGuildId;					//行会id
}INTERRELATIONMSG;

//行会仓库
typedef struct tagGuildDepotItem
{
	unsigned int		nActorId;		//玩家的id
	int		nCoinType;					//存入的物品，需要取出消耗的钱的类型 1 金币 2 元宝
	int		nCoinNum;					//取出物品需要的钱的数量

	int     nExitFlag;					//0存在，1被取出了(似乎已经不使用了)
	unsigned int nExchangeActorId;		//0-未被申请兑换，>0,有人申请兑换
	ACTORNAME 	szExchangeActorName;	//申请兑换的玩家Name
	unsigned int nExchangeTime;			//申请兑换时间

	CUserItem* nUserItem;		
	tagGuildDepotItem()
	{
		memset(this,0,sizeof(*this));
	}
}GUILDDEPOTITEM;

//行会仓库操作记录
typedef struct tagGuildDepotMsg
{
	BYTE	nType;					//类型 1 存入 2 取出 3 收回
	int		nTime;					//操作的时间
	char	nActorName[32];			//操作人的姓名
	char    nItemName[32];			//操作的物品名字
	int		nNum;					//物品数量
	int     nCoinType;				//钱的类型 1 金币 2 元宝
	int     nCoinNum;				//钱的数量 
	tagGuildDepotMsg()
	{
		memset(this,0,sizeof(*this));
	}
}GUILDDEPOTMSG;

typedef struct tagWarDbData
{
	int				nRelationShip;	//0 无 1 联盟 2 敌对
	int				nState;			//1 宣战状态 0 正常
	int				nPkCount;		//杀敌数
	int				nDieCount;		//己方被杀数量
	int				nNext;			//下次可以宣战的时间

	bool			boSaveHistory;//是否需要保存到历史记录表中，默认是false，只有当状态从敌对到和平的时候，才改为true，保存后，又改成false

	tagWarDbData()
	{
		memset(this,0,sizeof(*this));
	}
} WarDbData;

//行会之间的关系、状态处理
typedef struct tagWarRelationData
{
	unsigned int nGuildId;					//行会id
	WarDbData			m_WarData;
}WarRelationData;

typedef struct tagWarGuild
{
	enum tagWarStatus
	{
		wsDeclareWar = 0,		//宣战
		wsOnWar = 1,			
		wsPeaceful = 2,
	};
	CGuild*				m_pGuild;
	WarDbData			m_WarData;

	tagWarGuild()
	{
		m_WarData.boSaveHistory = false;
		m_pGuild = NULL;
	}	
	/*
	* Comments:检查当前时间，并且返回当前的状态
	* UINT nWarTime:敌对的持续时间
	* UINT nDecTime:宣战的持续时间
	* bool boChange:是否有状态改变
	* @Return int:
	*/
	int checkStatus(UINT nNow,UINT nWarTime,UINT nDecTime,bool& boChange) 
	{		
		//if (nNow > m_WarData.nNext) //可以改变状态了
		// {
		// 	m_WarData.nRelationShip = wsPeaceful;//可以宣战
		// 	boChange = true;
		// }
		return m_WarData.nRelationShip;
	}

	VOID Declare(CGuild* pWarGuild,UINT nNow,UINT nDecTime)
	{
		if (pWarGuild == NULL) return;
		m_pGuild = pWarGuild;
		m_WarData.nPkCount = 0;
		m_WarData.nDieCount = 0;
		m_WarData.nNext = nNow + nDecTime;
		m_WarData.nRelationShip = wsDeclareWar;
		m_WarData.boSaveHistory = false;
	}
}WarGuild;

//行会贡献排名列表
typedef struct tagGuildMemGxList
{
	unsigned int			nActorId;				//成员的id
	int			nGx;					//行会贡献
}GUILDMEMGXLIST;

#pragma pack(pop)
