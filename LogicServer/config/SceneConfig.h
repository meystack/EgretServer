#pragma once

using namespace wylib::container;

enum tagSceneType
{
	enSceneTypeCommon=0,	///< 普通场景
	enSceneTypeActivity=1,	///< 活动
	enSceneTypeFb=2,		///< 副本场景
	enSceneTypePractice=3,	///< 挂机场景(普通挂机)


};
class CNpc;
//FuBenConfig.txt 副本相关，主要就是定义副本对应的场景列表

typedef struct tagCommonFuBenConfig
{
	int				nFbId;
	char            fbName[1024];
	short   		nFbConfType;
	short   		nFbEnterType;
	bool			bDisableMonsterPropAdjust;
	bool			_resv;
	short			nRecommandLevel;
	int				nDestoryTime;
	int				nDefSceneId;
	GOODS_TAG		ConsumeList[3];
}COMMONFUBENCONFIG,*PCOMMONFUBENCONFIG;

typedef struct tagRefreshConfig
{
	int nEntityId;//怪物的id值，根据这个id值读取怪物的属性
	TICKCOUNT nNextRefreshTime;		//刷怪的时间间隔
	int nFirstTime;	//第一次刷怪的延长时间
	int nCount;		//每次刷怪的数量
	int nEntityLevel;
	int nMobX;
	int nMobY;
	int nMobRange;
	int nLiveTime;
	tagRefreshConfig()
	{
		memset(this, 0, sizeof(*this));
	}
	
} REFRESHCONFIG;

typedef struct tagRefreshList
{
	int nCount;
	REFRESHCONFIG* pList;
}REFRESHLIST;

//保存每个刷怪点的数据
typedef struct
{
	TICKCOUNT	nNextTime;	//下次刷怪的时间
	int			nCurrCount;	//当前剩下的怪物数量
} REFRESHVALUE;

typedef struct tagAreaAttri
{
	BYTE		bType;
	WORD		nCount;
	int*		pValues;	//这个int类型不要修改,是每个区域属性的参数，比如减血的属性，这个参数就是血量
	char*		sFunc;		//以下两个属性专门用于进入区域触发脚本的功能，这个指执行NPC的函数名，暂不支持带参数
	Uint64		hNpc;		//NPC的handle
}AreaAttri;

typedef struct tagSceneArea
{
	char		szName[32];//区域的名称
	WORD		nPointCount;
	LPPOINT		lpPoints;//点的集合
	WORD		Center[2];
	byte		NoTips;	//是否不提示
	bool		boInherit;//是否继承第一个区域的属性，默认是true
	AreaAttri	attri[aaAttriCount];
	INT			attrBits[aaAttriCount/32+1];//int类型不能改
}SCENEAREA,*PSCENEAREA;

typedef struct tagAreaList
{
	WORD nCount;
	SCENEAREA*	pList;
}AREALIST;

typedef struct tagNpcConfig
{
	int		nId;
	char	sName[32];
	char	sScript[128];
	int		nModelId;
	int		nIcon;
	int		nIdleInterval;	// NPC 空闲执行间隔
	//BYTE    bCamp;   		//NPC的阵营
	//int		nAITypeId;		// npc ai
	char	sTitle[32];
	char	sTalk[768];
	bool	bCanHideFlag;	//在场景内是否可以在程序内被设置为隐藏（默认为false、true是可以被设置，主要在沙巴克活动隐藏npc用）
	//任务列表
	CVector<int> GiveQuest;
	CVector<int> EndQuest;
	CVector<int> TalkQuest;		//别的任务找该Npc谈话的
	int  nOpenDay;//开服时间

}NPCCONFIG,*PNPCCONFIG;

typedef struct tagTeleportConfig
{	
	int		nPosX;
	int		nPosY;
	int		nSceneId;
	int		nToPosX;
	int		nToPosY;
	int		nToPosRadius;//目标坐标半径= nToPosRadius 随机传送
	int		nModelId;
	int 	nOpendayLimit; //开服天数限制
	bool	bUse;		//是否可用
	EntityHandle nHandle;	//传送门的句柄
	int    nPassId;			//能通过的id 1 表示只有占领沙巴克城的行会才能通过  默认为0所有都能通过
	int    nDist;			//传送门的xy坐标往外nDist距离的点都算传送点
	ACTORNAME		szName;//传送门的名称
}TeleportConfig;

typedef struct tagTelportList
{
	int					nCount;
	TeleportConfig*		pList;
}TelportList;


typedef struct tagNpcPos
{
	int nId;
	int nPosX;
	int nPosY;
}NPCPOS, *PNPCPOS;

typedef struct tagNpcPosList
{
	int				nCount;
	NPCPOS *		pPosList;
}NPCLIST,*PNPCLIST;

typedef struct tagHookLisk
{
	int		nCount;
	NPCPOS*	pPosList;
}HookList;

/// 场景相关的配置 SceneConfig.txt
typedef struct  tagSceneConfig
{
	int		nScenceId;
	short	nDefaultX;
	short	nDefaultY;
	char	szScenceName[32];
	char	szMapFileName[32];
	BYTE    bSceneType; 				//场景类型,0表示普通场景，1表示活动场景，2表示副本场景,3挂机地图
	bool	boCanCall;	//是否允许召唤
	int     nMapGgroup;//地图组
	bool	bIsNoPickUp;//禁止宠物拾取及显示
	AREALIST			vAreaList;
	REFRESHLIST			vRefreshList;
	
	NPCLIST				NpcList;	//npc的列表
	TelportList			TeleportList;		
	HookList			hookList;
	tagSceneConfig()
	{
		memset(this,0,sizeof(*this));
	}
} SCENECONFIG,*pSCENECONFIG;

typedef struct tagSceneConfigList
{
	int			 nCount;
	SCENECONFIG** ppList;
}SceneConfigList;

typedef struct tagFuBenConfig
{
	int			nFbId;
	char		szName[128];
	int			nMinLevel;
	int			nRecommandLevel;
	bool		bDisableMonsterPropAdjust;
	int			nMinCount;
	int			nMaxCount;
	int			nType;
	int			nRestTime;
	int			nGroupId;
	int			nDestoryTime;
	int			nProgress;		//进度是否显示(小于0不显示，1只有一个进度)
	char        szFbTitle1[32];
	char        szFbTitle2[32];
	char        szFbTitle3[32];
	byte        nFbType;
	char		szFbProgressTips[256];
}FUBENCONFIG,*PFUBENCONFIG;
