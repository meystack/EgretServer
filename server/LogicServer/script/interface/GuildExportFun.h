#pragma once

//tolua_begin
/*
	本文件导出帮派的相关操作的函数
*/
namespace Guild
{
	/* 
	* Comments: 获取行会的在线玩家列表
	* Param  guildPtr 行会指针
	* @Return int:  
	*/
	int getGuildOnlineMemberList(lua_State *L);

	/* 
	* Comments: 获取行会的所有玩家id列表
	* Param guildPtr 行会指针
	* @Return int:  
	*/
	int getGuildMemberIdList(lua_State *L);

	void getGuildMemberBasicData(void* pGuild, unsigned int nActorId, int &nSex, int &nJob);
	

	/* 
	* Comments: 获取行会指针
	* Param unsigned int nGuildId: 行会id
	* @Return void*:  
	*/
	void* getGuildPtr(unsigned int nGuildId);
	/*
	* Comments: 获取帮派的名字
	* Param int nGuidID:帮派的名字
	* @Return char*:帮派的名字
	*/
	char* getGuildName(unsigned int nGuidID);

	/*
	* Comments:根据帮派id获取帮主指针，如果不在线，返回NULL
	* Param int nGuildID:
	* @Return void*:
	*/
	void* getGuildLeader(unsigned int nGuildID);

	/*
	* Comments:根据帮派id返回帮主的角色id
	* Param int nGuildID:
	* @Return int:
	*/
	unsigned int getGuildLeaderId(unsigned int nGuildID);

	/*
	* Comments:修改帮派繁荣
	* Param int nGuildId:帮派id
	* Param int nFrVar:繁荣增量
	* @Return bool:成功返回true
	*/
	bool changeGuildFr( unsigned int nGuildId, int nFrVar);

	/*
	* Comments:修改帮派资金
	* Param int nGuildId:帮派id
	* Param int nCoinVar:资金增量
	* @Return bool:成功返回true
	*/
	bool changeGuildCoin(unsigned  int nGuildId, int nCoinVar, int nLogId = 0,const char* pStr = "");

	/*
	* Comments:获取帮主的名字
	* Param int nGuildId:帮派id
	* @Return const char*:返回帮主名字，null表示没这个帮派
	*/
	const char* getGuildLeaderName(unsigned  int nGuildId );

	//获取行会成员人数
	int getGuildMemberNum(unsigned int nGuildId);

	/*
	* Comments: 行会广播消息
	* Param void * packet: 数据包CActorPacket对象指针
	* Param void* guildPtr 行会指针
	* Param int nLevel: 广播的玩家最低限制等级
	* Param int nCircle: 广播的玩家最低限制转数
	* @Return void:
	* @Remark:
	*/
	void guildBroadCastMsg(void* packet, void* guildPtr, int nLevel=0, int nCircl=0);
	//获取皇城君主的id
	unsigned int getCityOwnMasterId();

	
	/// 震天龙弹填充进度
	unsigned int getThunderPower(unsigned int nGuildId);
	void setThunderPower(unsigned int nGuildId, unsigned int nValue);
	/// 震天龙弹填充个数
	unsigned int getFillThunderTimes(unsigned int nGuildId);
	void setFillThunderTimes(unsigned int nGuildId, unsigned int nValue);

	//是否攻城战开始了
	bool isGuildSiegeStart();

	//下发行会战指挥面板到全体参战的成员
	void sendGuildSiegeCmdPanel(unsigned char nFlag);

	//会长下发指令
	void setGuildSiegeCommand(void* pEntity, int nGuildId, int nCmdIdx, int nCmdMsgIdx);

	unsigned int getGuildSiegeCommandTime(int nGuildId);

	//自动报名攻城战
	void autoSignGuildSiege();
	//攻城当天重置下报名
	void resetSignGuildsiege();

	void clearGuildSiegeCommand();

	//今天是否开启攻城
	bool isGuildSiegeTodayOpen();
	//开始攻城
	void startGuildSiege();
	//结束攻城
	void guildSiegeEnd(unsigned int nGuildId);
	//获取职位人的姓名
	char* getCityPostionInfo(int nPos,int &nJob,int &nSex);
	//设置占领皇城的行会id
	void setCityOwnGuildId(unsigned int nGuildId);
	unsigned int getPalaceOccoupyTime();
	//获取占领皇城的行会id
	unsigned int getCityOwnGuildId();
	//攻城战举行当天（19:00），清除攻城数据
	void clearGuildSiegeData();
	//是否限制金币使用(只实现了建筑升级)
	bool isLimitGuildCoin(unsigned int nGuildId);
	//获取皇城职位
	unsigned int getCityPostionById(unsigned int nActorId);



	//在行会频道发送消息
	void sendGuildMsg(unsigned int nGuildId, char* szMsg,int nLevel = 0, int nCircle = 0);

	void getUpgradeItemCount(unsigned int nGuildId, int& nItemCount1,int& nItemCount2, int& nItemCount3);
	void setUpgradeItemCount(unsigned int nGuildId, int nItemCount1,int nItemCount2, int nItemCount3);

	void getGuildTreeData(unsigned int nGuildId, int &nTreeLevel, int &nTreeDegree);
	void setGuildTreeData(unsigned int nGuildId, int nTreeLevel, int nTreeDegree);

	unsigned int getGuildTreeFruitTime(unsigned int nGuildId);
	void setGuildTreeFruitTime(unsigned int nGuildId, unsigned int nFruitTime);

	void getGuildFruitData(unsigned int nGuildId, int &nGuildFruitLevel, int &nGuildFruitDegree, int &nGuildFruitNum);
	void setGuildFruitData(unsigned int nGuildId, int nGuildFruitLevel, int nGuildFruitDegree, int nGuildFruitNum);

	//将行会神树从成熟期回归到幼苗期
	void procGuildTreeToSapling();

	void procGuildDailyData();

	void getGuildTaskData(unsigned int nGuildId, int &nTaskId, int &nTaskSche);

	//当日成员捐献的行会资金总值
	int getDailyGuildCoinDonated(unsigned int nGuildId);
	void addDailyGuildCoinDonated(unsigned int nGuildId, int nAddGuildCoin);

	void sendMemberList(void* pEntity, unsigned int nActorId=0);

	void setSbkGuildId(unsigned int nGuildId);
};

//tolua_end
