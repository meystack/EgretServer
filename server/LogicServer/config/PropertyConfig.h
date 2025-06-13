#pragma once
#include<vector>
#include<string>
#include<map>
/// 宠物的等级
#define MAX_PET_LEVEL_COUNT 10

//玩家的职业配置

#define  MAX_LEVEL1_PROPERTY_COUNT	4	///< 1级属性的数量
#define  MAX_LEVEL2_PROPERTY_COUNT	15	///< 2级属性的数量
#define  ACTOR_MAX_LEVEL			120	///< 玩家的最大的等级
#define  MAX_DROP_COIN_ICON_COUNT	10	///< 最大的掉落的金币的图标的数目
#define	 MAX_SUIT_COUNT				10	///< 最大套装装备个数
#define  MAX_WEAPONEXTENDBUFF_COUNT 2
#define  MAX_LUCK_CURSE_LEVEL		20
#define  THINGS_BUF_NUM				4  //秘宝buf加经验的配置数量	
#define  CHAT_HELP					2  //求救消费	

typedef struct tagLevel1PropertyTransfer
{	
	/// 1级属性到2级属性的转换,一个5*15的矩阵
	float m_level1PropertyTransTable[MAX_LEVEL1_PROPERTY_COUNT][MAX_LEVEL2_PROPERTY_COUNT];

	tagLevel1PropertyTransfer()
	{
		memset(this,0,sizeof(*this) );
	}
} LEVEL1PROPERTYTRANSFER, *PLEVEL1PROPERTYTRANSFER;


 // 玩家的升级经验表
typedef struct tagLevelUpExpTable 
{
	CVector<LONGLONG> expTable;	///< 经验升级表
	CVector<LONGLONG> expRateTable;	///< 多倍经验倍率
	/*
	* Comments: 获取升级的经验
	* Param INT_PTR nLevel:如果输入2，表示1级到2级的升级经验 
	* @Return LONGLONG:返回升级经验
	*/
	LONGLONG GetLevelExp(INT_PTR nLevel)
	{
		if(nLevel <=0 || nLevel > expTable.count() )
		{
			return 100000000000000000; //如果参数出现错误，搞一个很大的经验
		}
		else
		{
			return expTable[nLevel -1];
		}
	}
	LONGLONG GetRatebyExp(INT_PTR nLevel)
	{
		if(nLevel <=0 || nLevel > expRateTable.count() )
		{
			return 0; //如果参数出现错误
		}
		else
		{
			return expRateTable[nLevel];
		}
	}
}LEVELUPEXPTABLE,*PLEVELUPEXPTABLE;

/// 杀怪经验衰减表
typedef struct tagKillMonsterExpTable
{
	CVector<WORD> killMonsterAboveExpTable; ///< 杀>=自身等级怪经验表
	CVector<WORD> killMonsterBelowExpTable; ///< 杀 < 自身等级怪经验表
	std::map<int, int>killMonsterExpMap; 	//转生等级差-杀怪经验衰减
	int  nExpMapSize = 0;

	/*
	* Comments:获取杀怪的经验
	* Param INT_PTR nLevelDis: 等级差
	* @Return float: 返回经验的衰减，百分数的
	*/
	float GetKillMonsterExp(INT_PTR nLevelDis)
	{
		if(nLevelDis >=0  )
		{
			/*if(nLevelDis < killMonsterAboveExpTable.count())
			{
				//return (float)killMonsterAboveExpTable[nLevelDis] / 10000; // 杀死
			}	*/
			return 1.0 ;//怪物经验高于玩家不衰减	
		}
		else 
		{
			nLevelDis *= -1;
			//nLevelDis --;
			/*if(nLevelDis >= 0  && nLevelDis < killMonsterBelowExpTable.count())
			{
				return (float)killMonsterBelowExpTable[nLevelDis] /10000;
			}*/
			if(nLevelDis >= 0  )
			{
				if(nLevelDis <=20 ){
					return 1.0;//玩家等级高于怪物20以内，不衰减
				}
				else {
					return (float)(1- (float)(nLevelDis -20 )/30.0)  ;
					//这里当返回值<=0时，获得最小经验，调用时候单独处理一下小于零
				}
			}

		}
		return 0.0;
	}

	/*
	* Comments:获取杀怪的经验
	* Param int nLevelDis: 转生等级差
	* @Return float: 返回经验的衰减，百分数的
	*/
	float GetCircleKillMonsterExp(int nLevelDis)
	{
		if(nLevelDis <= 2)//(人转生等级 - 怪物转生等级 <= 2)均不进行经验衰减
		{
			return 	(float)killMonsterExpMap[2] / 100; 
		} 

		auto it = killMonsterExpMap.find(nLevelDis);

		if(it != killMonsterExpMap.end()) {
			return (float) it->second / 100;
		} else {
			auto it2 = killMonsterExpMap.crbegin();
			return (float) it2->second / 100;
		}
	}
}KILLMONSTEREXPTABLE,*PKILLMONSTEREXPTABLE; //杀怪的经验衰减


typedef struct  tagWorldLevelExpRate
{
      int  nMin;
	  int  nMax;
	  float  fValue;
}WORLDLEVELEXPRATE;

typedef struct tagOnLineAwardYb//活跃度奖励需要玩家等级跟世界等级的差距值
{
	int nMin;		//达到分钟数目
	int nYb;		//元宝数量
	tagOnLineAwardYb()
	{
		nMin = 0;
		nYb  = 0;
	}

}ONLINEAWARDYB;

typedef struct tagLevelLimit	//活跃度奖励需要玩家等级跟世界等级的差距值
{
	int nCircle;		//达到分钟数目
	int nLevel;			//元宝数量

	tagLevelLimit()
	{
		nCircle = 0;
		nLevel  = 0;
	}

}LEVELLIMIT;

//ExpLmtOpenday的elmemet
typedef struct basiclmt{
		LONGLONG num ;
		int damprate;
		basiclmt(LONGLONG _num = 1000000000000, int _damprate = 10000) 
		: num(_num)
		, damprate(_damprate)
		{ }

		basiclmt(const basiclmt& tmp) 
		: num(tmp.num)
		, damprate(tmp.damprate)
		{ }

}BASICLMT;

typedef struct elemTypeOpenday{
	LONGLONG multilmt;//多倍经验每日上限
	std::map<int, BASICLMT> m_basiclmt ;  //基础经验每日上限以及衰减率
	elemTypeOpenday(LONGLONG  param = 0)
	:multilmt(param)
	{ }
}ELEMTYPEOPENDAY;

typedef struct ExpLmtOpenday	//开服天数相关的等级限制
{

	std::map<int, ELEMTYPEOPENDAY> m_expLmtOpenday ;

}EXPLMTOPENDAY;



#define GLOBAL_TIME_LEN 32
/// 全局配置表
typedef struct tagGlobalConfig
{
	int 			nBlinkEnterEffect;					// 瞬移技能进入场景的特效id
	int				nScrollEnterEffect;					// 回城卷进入场景的技能特效id


	int				nMonsterItemRefreshTime;							// 怪物掉落物品的有效时间，单位毫秒
	int				teamLootMaxDistanceSquare;							// 最大的拾取距离的平方 
	int				nTeamOpenLevel;										//组队开启等级
	int				nInitSceneId;										// 初始化场景ID，当地图销毁或者改变，这默认登陆到洛阳城
	int				nInitScenePosX;										// 场景坐标x
	int				nInitScenePosY;										// 场景坐标y
	int				nOnlineCountBase;									// 在线人数超过nOnlineCountBase人 怪物翻倍
	int				nSceneRefreshMonsterTime;							// 场景中刷怪的时间限制，单位：毫秒
	int				nFubenDestoryTime;									///< 副本没人后的保留时间

	bool            bIsFcmOpen;											// 防沉迷是否开启
	unsigned int    nFcmHalfExpMoneyTime;								// 防沉迷开始经验、金钱减半的时间(单位秒)
	unsigned int    nFcmZeroExpMoneyTime ;								// 防沉迷开始经验、金钱没有的时间(单位秒)
	
	//以下是盛趣的防沉迷
	bool            bIsSqKidsFcmOpen;									// 盛趣儿童防沉迷是否开启
	unsigned int    nKidsFcmHolidayTime;								// 儿童防沉迷节假日
	unsigned int    nKidsFcmWorkDayTime ;								// 儿童防沉迷非节假日 
	char strKidsFcmHolidayStartTime[GLOBAL_TIME_LEN];
    char strKidsFcmHolidayEndTime[GLOBAL_TIME_LEN]; 
	char strKidsFcmWorkDayStartTime[GLOBAL_TIME_LEN];
    char strKidsFcmWorkDayEndTime[GLOBAL_TIME_LEN]; 

	int				nBindYuanbaoLimit;									// 绑定元宝的上限，默认值5000
	int				nMoneylimit;									// 金币的上限，默认值5000
	int				nBindmoneylimit;									// 绑定金币的上限，默认值5000
	int				nYuanbaolimit;									// 元宝的上限，默认值5000
	int             nPetLevelAboveOwner;								// 宠物最多等级高出玩家的等级
	
	float           fPetDeathHpRate;									// 宠物复活的时候恢复多少血nMaxCalCurseValue


	int				nMonsterVectResetInterval;							///怪物归属重置间隔(单位ms)(5s没有打怪，清除怪物原来归属)
	int				nMonsterHatredKeepTime;								//boss怪物仇恨值记录时间（秒）（超过重置伤害值）
	int				QuestEquipData[CHAT_HELP];							// 穿戴装备任务

	float			fWriteNameBagDropRate;								// 白名背包掉落系数
	float			fWriteNameEquipDropRate;							// 白名装备掉落系数
	float			fRedNameBagDropRate;								// 红名背包掉落系数
	float			fRedNameEquipDropRate;								// 红名装备掉落系数
	int				equipDropRates[itMaxEquipPos];						// 装备各部位系数

	int				nWriteNameBagDropNum;								// 白名掉落数量
	int				nRedNameBagDropNum;									// 全部掉出
	int				nWriteNameEquipDropNum;								// 白名装备掉落数量
	int				nRedNameEquipDropNum;								// 红名装备掉落数量

	float			fHeroWriteNameEquipDropRate;						// 英雄白名装备掉落系数
	float			fHeroRedNameEquipDropRate;							// 英雄红名装备掉落系数
	int				nHeroWriteNameEquipDropNum;							// 英雄白名装备掉落数量
	int				nHeroRedNameEquipDropNum;							// 英雄红名装备掉落数量
	int				heroEquipDropRates[itMaxEquipPos];					// 英雄装备各部位系数

	int				nDeathDropExpTotalRate;								// 经验掉落总系数,0为不开启,大于0表示掉落总系数
	int				nWriteNameExpDropRate;								// 白名经验掉落系数 
	int				nWriteNameExpDropMax;								// 白名经验掉落上限
	int				nRedNameExpDropRate;								// 红名经验掉落系数
	int				nRedNameExpDropMax;									// 红名经验掉落上限
	
	int				nSuckBloodRate;										//吸血属性(怪物)
	int				nSuckBloodRatePlayer;								//吸血属性 (玩家)万分比

	float			fBodyDamageRate;

	int				nMonsterGropDropRangeMax;							// 掉落组(group>0)掉落区间最大值		
	std::vector<int>    nDizzyBuffTateJob;                                  // 战士，师法，道士触发麻痹系数
	int             nDizzyBuffMaxRate;									// 麻痹最大概率
	int             nDizzyBuffMaxTime;                                 // 麻痹最大时间

	int				nFriendChatLimit;									// 好友聊天最低等级
	//int				nChatLogFlag;										// 聊天内容是否存盘

	int				n2StepSkillHitRate;									// 技能攻击的时候，距离2格命中的概率，1表示万分之1
	int				n3StepSkillHitRate;									// 技能攻击的时候，距离3格命中的概率，1表示万分之1	
	int				n4StepSkillHitRate;									// 技能攻击的时候，距离4格命中的概率，1表示万分之1


	int				nMailExpireDays;									// 邮件保存多少天
	int				nBossDigNeedStoreCount;								//	挖boss需要寻宝空间
	int				nDoLevelMailGift;									// 是否等级礼包发邮件（1-是，0-否）
	int				nCanActivityFiveAttrLevel;
	int				nCanActivityFiveAttrStrongLevel;
	int				nHammerItemId;										// 副本采集的锤子物品ID 
	int				nMoveSpeedLimit;									// 人物移动速度限制，当人物速度移动速度小于这个值，则会强制变为默认速度
	int				nMailMaxCount;										//最多保存邮件数
	std::vector<std::string>    nMessageColor;                                      //发送消息的颜色
	int 			nGuardEffId  ;  //守护特效
	int 			nParalyEffId  ;  //麻痹
	int 			nCritiEffId  ;  //暴击
	std::vector<int>    nFirstChargeIds;                                      //首充对应的活动id
	int             nMaxDoubleExpValue;                                       //多倍经验上限
	int             nMaxRankTipId;
	std::vector<std::tuple<int,int,int>> vecTeamCallConsum;	//团队召唤道具消耗
	std::vector<std::tuple<int,int,int>> vecGuildCallConsum;	//行会召唤道具消耗
	int             nMergeTimes = 0;//合服次数

	int             nCrossMsgDay = 0;//跨服消息限制
	
	int 			nCrippleEffectId;			//致残特效Id
	int 			nAdvancedCrippleEffectId;	//高级致残特效Id

	//当前job 必须已经是玩家job 减1
	int GetJobDizzyRate(int nJob) {
		int nRate = 10000;
		if(nJob < 0 || nJob >= nDizzyBuffTateJob.size()) {
			return nRate;
		}
		return nDizzyBuffTateJob[nJob];
	}
}GLOBALCONFIG,*PGLOBALCONFIG;

typedef struct tagStateConfig
{
	bool allowTable[esMaxStateCount][esMaxStateCount]; //一个2维数组
}STATECONFIG,*PSTATECONFIG;

typedef struct tagTeamKillMonsterExp
{
	float teamExp[9];	//队伍的经验加成
	int   teamExpBuf[4];//队伍的经验加成buf图标,TeamKillMonsterExpbufId

	/*
	* Comments:获取队伍的经验加成的比率
	* Param INT_PTR nTeamMember:队伍的人数数量
	* @Return int:返回
	*/
	float GetTeamKillMonsterExp(INT_PTR nTeamMember)
	{
		if(nTeamMember > ArrayCount(teamExp) || nTeamMember < 1) return 0;
		//return teamExp[nTeamMember -1];
		
	}

	int GetTeamKillMonsterExpBufId(INT_PTR nCount)
	{
		if(nCount > ArrayCount(teamExpBuf) || nCount < 1) return 0;
		return teamExpBuf[nCount -1];
	}
}TEAMKILLMONSTEREXP,*PTEAMKILLMONSTEREXP;

/// 玩家的跳跃距离的消耗
typedef struct tagJumpLengthConsume
{
	CVector<int> jumpConsumes; 

	int GetJumpConsume(int nLength)
	{
		if(nLength <= 0) return 0;
		INT_PTR nCount = jumpConsumes.count();
		if(nCount == 0) return 0;
		return jumpConsumes[__min(nLength, nCount) - 1]; 
	}
}JUMPLENGTHCONSUME, * PJUMPLENGTHCONSUME;

struct tagChatChannelLimit
{
	BYTE 	btChannelId;		// 聊天频道Id
	WORD 	wLevel;				// 聊天频道 等级限制
	DWORD 	nRechargeAmount;	// 聊天频道 充值金额限制(目前以元宝充值为准)
};

/// 聊天配置
typedef struct tagChatSystemConfig
{
	int				nChatChannelCD[ciChannelMax];						// 聊天频道CD配置	
	struct tagChatChannelLimit ChatLimit[ciChannelMax];				// 频道聊天等级限制配置
	int				nRechargeopen[ciChannelMax];						// 充值频道聊天等级限制配置
	size_t			nAutoSilencing;										// 自动禁言，玩家连续发多少次相同内容就禁言
	size_t			nAutoSilencingSecond;								// 自动禁言时长，单位是秒
	size_t			nAutoSilenceLevel;									// 自动禁言限制等级， <= nAutoSilenceLevel,禁言范围
	int				nSameChatSliencingCount;							//同一内容连续发N次禁言
	int				nSameChatSliencingSecond;							//同一内容连续发N次禁言时间
	int				nAutoSilenceSameIpShutup;							//同一ip禁言X次则整个ip禁言
	int				nChatLogFlag;										// 聊天内容是否存盘
	int             nShowItemSize;                                       //聊天最多展示的物品数量
	std::vector<int>				nVipChats;						//聊天vip限制
	int            nCounter;//
	int            nDisplayLeftTimesLimit;                               //聊天vip限制 剩余次数提示限制
}CHATSYSTEMCONFIG,*PCHATSYSTEMCONFIG;

//掉落道具配置
typedef struct tagDropItemConfig
{
	int				nDropItemExpireTime;								// 掉落在地上的物品的存活时间,单位秒
	int				nDropItemMasterProtectTime;							// 掉落在地上的物品属于他的主人的时间,单位秒
	int				dropCoinCounts[MAX_DROP_COIN_ICON_COUNT];			// 掉落在地上的金币，在各范围里图标不同，这里标记不同范围的金币的范围
	int				dropCoinIcons[MAX_DROP_COIN_ICON_COUNT];			// 掉落在地上的金币，在dropCoinCount范围里的图标的id
	// int				dropYuanBaoCounts[MAX_DROP_COIN_ICON_COUNT];		// 掉落在地上的元宝，在各范围里图标不同，这里标记不同范围的元宝的范围
	// int				dropYuanBaoIcons[MAX_DROP_COIN_ICON_COUNT];			// 掉落在地上的元宝，在dropYuanBaoCount范围里的图标的id
	int				dropBindCoinCounts[MAX_DROP_COIN_ICON_COUNT];		// 掉落在地上的绑定金币，在各范围里图标不同，这里标记不同范围的绑定金币的范围
	int				dropBindCoinIcons[MAX_DROP_COIN_ICON_COUNT];		// 掉落在地上的绑定金币，在dropBindCoinCounts范围里的图标的id
	int				nMonsterMaxCoinOneDrop;								// 怪物掉落的金币，每堆的最大值
    int				nMonsterMaxBindCoinOneDrop;					        // 怪物掉落的绑定金币，每堆的最大值
	int				nDropItemNeedTime;					        // 掉落在地上的物品需要时间才能拾取
	int 	        nDropPetLootDistance;//宠物捡取最大格数
}DROPITEMCONFIG, *PDROPITEMCONFIG;


typedef struct  tagDeathDropConfig
{
	int nWriteNameBagDropRate = 0;//白名背包掉落
	int nWriteNameEquipDropRate = 0;//白名装备掉落
	int nRedNameBagDropRate = 0;//红名背包掉落
	int nRedNameEqiupRate = 0;//红名装备掉落
	int nMaxEquipDropNum = 1;//红名装备掉落
} DEATHDROPCONFIG, *PDEATHDROPCONFIG;

typedef struct  tagBagRemainConfig
{
	//背包格数判定
	std::map<int, std::vector<int> > m_nBagRemain;
} BAGREMAINCONFIG, *PBAGREMAINCONFIG;

typedef struct  tagHookExp
{
	int nBasePdValue = 0;//固定经验
	int nPdPercent = 0; //加成系数 万分比
} tagHookExp, *PtagHookExp;

//泡点
typedef struct  tagHookExpConfig
{
	std::map<int, tagHookExp> m_HookMaps;
} HookExpConfig, *PHookExpConfig;

typedef struct  tagPropertyConfig
{
	LEVELUPEXPTABLE			expTable;						///< 经验升级表
	KILLMONSTEREXPTABLE		killMonsterExpTable;			///< 杀怪经验衰减表

	TEAMKILLMONSTEREXP		teamKillMonsterExp;				///< 队伍杀怪的经验
	GLOBALCONFIG			globalConfig;
	CHATSYSTEMCONFIG        chatConfig;						///聊天const配置
	DROPITEMCONFIG          dropItemConfig;					///掉落道具const配置
	DEATHDROPCONFIG         deathDropCfg;                   //死亡掉落概率
	BAGREMAINCONFIG         bagRemain;                      //背包格数判定
	HookExpConfig           hookExpCfg;//泡点数据
} PROPERTYCONFIG, *PPROPERTYCONFIG;


/// 各等级能够获得的最大的修为
typedef struct tagRenownLimits
{
	int remowns[ACTOR_MAX_LEVEL]; 
	int nMaxConfigCount;
	int GetMaxRemown(int nLevel)
	{
		if(nLevel > nMaxConfigCount || nLevel > ACTOR_MAX_LEVEL )
		{
			nLevel = nMaxConfigCount;
		}
		if(nLevel <= 0) 
		{
			nLevel = 1;
		}
		return remowns[nLevel - 1];
	}
	tagRenownLimits()
	{
		memset(this,0,sizeof(*this));
	}
}RENOWNLIMIT,*PRENOWNLIMIT;
