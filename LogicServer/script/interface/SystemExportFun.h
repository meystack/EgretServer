#pragma once
union RedPacketId
{
	int nId;
	struct 
	{
		BYTE nHour;
		BYTE nMin;
		BYTE nSec;
		BYTE nSeries;
	}t;
};
//tolua_begin
/*
	这个文件包含系统的一些调用，比如获取服务器名称，返回系统的时间等
	在使用函数前加入System.xx函数即可获取
	比如获取当前的时间可以: nyear, nMonth,nDay= System.getDate()
*/



static int print(lua_State *L) 
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));		
		OutputMsg(rmNormal, s);
		lua_pop(L, 1);  /* pop result */
	}	
	return 0;
}

namespace System
{	
	/*
	* Comments: 获取服务器的名字
	* @Return char *:返回服务器的名字
	*/
	char* getServerName();
	
	// 获取平台号
	char* getPfId();

	// 获取YY的key
	char* getYYKey();
	
	// 获取游戏代号
	char* getGameFlag();

	/*
	* Comments: 获取服务器的ID
	* @Return int: 返回服务器的id
	*/
	int getServerId();
	
	/*
	* Comments: 获取当前的年月日
	* Param int & nYear: 年
	* Param int & nMonth:月
	* Param int & nDay:日
	* @Return void:
	*/
	void getDate(int & nYear, int &nMonth, int &nDay);
	
	/*
	* Comments: 获取当前的小时 ，分钟，秒数
	* Param int & nHour:小时
	* Param int & nMinute:分钟
	* Param int & nSecond:秒
	* Param int & nMiliSecond:毫秒
	* @Return void:
	*/
	void getTime(int & nHour, int &nMinute, int &nSecond, int &nMiliSecond);
	
	/*
	* Comments:通过场景的名字获取场景的id
	* Param char * sName:场景的名字
	* @Return int:场景的ID,如果场景为空，那么返回一个-1
	*/
	int getSceneIdByName(char *sName);
	

	/*
	* Comments:获取当前是一周的第几天
	* @Return int:返回一个星期的第几天
	*/
	int getDayOfWeek();

	/*
	* Comments:获取当前是本月的第几天
	* @Return int:返回一个月的第几天
	*/
	int getDayOfMonth();

	/*
	* Comments: 获取当前月份
	* @Return int : 返回当前月份
	*/
	int getMonthOfNow();
	
	/*
	* Comments:获取当前的时间滴答（开机以来）
	* @Return Uint64:返回当前的时间滴答
	*/
	unsigned long long getTick();
	
	/*
	* Comments:获取一个取模nModule的随机数
	* Param int nModule: 模数
	* @Return int: 返回取模后的随机数
	*/
	unsigned int getRandomNumber( int nModule);

	/*
	* Comments:获取当前的mini时间，单位秒
	* @Return UINT:
	*/
	unsigned int getCurrMiniTime();

	/*
	* Comments: 判断两个时间是否是同一周
	* Param unsigned int t1: CMiniDateTime格式
	* Param unsigned t2: CMiniDateTime格式
	* @Return bool: 如果t1和t2是同一周，返回true；否则返回false
	* @Remark:
	*/
	bool isSameWeek(unsigned int t1, unsigned int t2);

	/*
	* Comments: 判断两个时间是否是同一日
	* Param unsigned int t1: CMiniDateTime格式
	* Param unsigned t2: CMiniDateTime格式
	* @Return bool: 如果t1和t2是同一周，返回true；否则返回false
	* @Remark:
	*/
	bool isSameDay(unsigned int t1, unsigned int t2);

	/*
	* Comments: 按场景的名字和位置传送实体
	* Param CEntity * pEntity:实体的指针
	* Param char * sSceneName:  场景的名字
	* Param int nPosX:坐标x
	* Param int nPosY:坐标y
	* Param int nType:传送类型
	* @Return bool: 成功返回true，否则返回false
	*/
	bool telportSceneByName(void * pEntity, char * sSceneName, int nPosX, int nPosY, int nType = 0);

	/*
	* Comments:传送到场景的一个点去
	* Param void * pEntity:玩家的指针
	* Param int nSceneId:场景的id
	* Param int nPosX:位置x
	* Param int nPosY:位置y
	* @Return bool:成功返回true，否则返回false
	*/
	bool telportScene(void * pEntity, int nSceneId, int nPosX, int nPosY);

	/*
	* Comments:延迟多少时间后进行传送
	* Param void * pEntity:
	* Param int nSceneId:
	* Param int nPosX:
	* Param int nPosY:
	* Param int nTime:单位毫秒
	* @Return void:
	*/
	void telportSceneDelay(void * pEntity, int nSceneId, int nPosX, int nPosY,int nTime=0);

	/*
	* Comments: 传送到场景的默认点
	* Param CEntity * pEntity: 实体的指针
	* Param char * sSceneName:场景的名字
	* Param int nPointID:默认点的ID,如果没有多个默认点直接使用0
	* @Return bool: 成功返回true，否则返回false
	*/
	bool telportSceneDefaultPoint(void * pEntity, char * sSceneName,int nPointID=0);
	
	/*
	* Comments: 初始化脚本的网络协议分发器
	* @Return void:
	*/
	void InitScriptNetmsgDispatcher();

	/*
	* Comments: 注册协议回调
	* Param int sysId: 主协议id
	* Param int cmdId: 子协议id
	* @Return void:
	*/
	void regScriptNetMsg(int sysId, int cmdId);

	/*
	* Comments: 初始化脚本的事件分发器
	* @Return void:
	*/
	void InitScriptActorEventDispatcher();

	/*
	* Comments: 注册事件回调
	* Param int enType: 实体类型
	* Param int evId: 事件id
	* @Return void:
	*/
	void regScriptEvent(int nType, int evId);

	/*
	* Comments: 打印调试信息
	* Param char * str: 调试信息
	* @Return void:
	*/
	void trace(char * str);

	/* 逐层创建目录
	 * 如果创建目录C:\a\b\c\d，最终目录的父目录不存在则逐级创建父目录并创建最终目录
	 * @如果目录完全创建成功则函数返回true，否则返回false。
	 * %如果在创建某个父目录成功后并创建子目录失败，则函数返回false且已经创建的父目录不会被删除。
	 * %目录路径的总体字符长度，函数要求必须在MAX_PATH个字符长度以内
	 */
	bool deepCreateDirectory(char * sDirName);

	 
	/* 判断文件或目录是否存在
	* 在操作系统中，目录也是一个文件，如果要判断一个目录是否存在则应当使用DirectoryExists，
	* 要判断一个文件是否存在且是一个归档文件则应当使用IsArchive。
	* @如果文件或目录存在则返回true否则返回false
	* %文件路径字符长度不得超过MAX_PATH
	*/
	bool fileExists(char* sFilePath);

	/*
	* Comments:根据玩家的名字获取玩家的指针
	* Param char * sName:角色名字
	* @Return void*:
	*/
	void* getActorPtr(char* sName);

	
	/*
	* Comments: 全服发布系统提示
	* Param char * sTipmsg: tipmsg的指针
	* Param int nTipmsgType: 公告的显示类型，聊天栏，还是弹出框等
	* @Return void:
	*/
	void broadcastTipmsg(char * sTipmsg,int nTipmsgType =ttTipmsgWindow, int  nLevel = 0);

	/*
	* Comments: 跨服发布全服系统提示
	* Param char * sTipmsg: tipmsg的指针
	* Param int nTipmsgType: 公告的显示类型，聊天栏，还是弹出框等
	* Param bool bCsFlag: 是否在跨服服务器发布提示 true发布 false不发布
	* @Return void:
	*/
	void broadcastTipmsgCs(char * sTipmsg,int nTipmsgType =ttTipmsgWindow, bool bCsFlag = true);

	/*
	* Comments: 全服发布系统提示
	* Param char * sTipmsg: tipmsg的指针
	* Param int nTipmsgType: 公告的显示类型，聊天栏，还是弹出框等
	* @Return void:
	*/
	void broadcastTipmsgLimitLev(char * sTipmsg,int nTipmsgType =ttTipmsgWindow,int nLevel = 0);
	
	/*
	* Comments: 通过ID广播tipmsg
	* Param int nTipmsgID: 提示的ID
	* Param int nTipmsgType: 提示的类型
	* @Return void:
	*/
	void broadcastTipmsgWithID(int nTipmsgID, int nTipmsgType =ttTipmsgWindow);

	/*
	* Comments: 带参数全服广播
	* Param int nTipmsgID: 提示的ID
	* Param int nTipmsgType: 提示的显示类型
	* Param char * sParam1:参数1
	* Param char * sParam2:参数2
	* Param char * sParam3:参数3
	* @Return void:
	*/
	void broadTipmsgWithParams(int	nTipmsgID,int nTipmsgType=ttTipmsgWindow,char * sParam1=NULL,char *sParam2=NULL,char * sParam3 =NULL);
	
	/*
	* Comments: 获得实体的指针返回类型
	* Param void * pEntity: 实体的指针
	* @Return int: 如果指针是正确的返回实体类型(玩家,npc等)，否则返回-1
	*/
	int getEntityByPtr(void* pEntity);

	
	/*
	* Comments:通过实体的handle返回类型
	* Param unsigned long long entityHandle: 实体的handle
	* @Return int: 如果指针是正确的返回实体类型(玩家,npc等)，否则返回-1
	*/
	int getEntityTypeByHandle(unsigned long long entityHandle);

	/*
	* Comments: 通过玩家的actorID获取玩家的指针
	* Param unsigned int nActorID:actorID
	* @Return void *:玩家的指针
	*/
	void * getEntityPtrByActorID(unsigned int nActorID);

	/*
	* Comments: 注册一个定时回调函数
	* Param void * pNPC:				NPC对象
	* Param char * sFnName:				函数名称
	* Param char * sNextCall:			下次调用日期和时间，使用标准时间书写格式，如：2011-03-04 20:19:00
	* Param unsigned int dwSecInterval: 调用周期，单位是秒，每隔这么多秒调用一次
	* Param bool boCallOnNextBoot:		在下次启动的时候是否恢复调用
	* Param ...							最多16个参数，用于传递给被调函数（请参见注意事项）
	* @Return void*: 如果注册定时回调成功，则函数返回定时回调句柄，否则返回nil
	* ★注意★	
	*
	*  1、同一个函数是不允许被多次注册为自动调用函数的；
	*
	*  2、sNextCall表示的日期和时间为一个已经过去的时间，则会自动根据函数调用周期值机选下次执
	*  行时间为现在之后的最近的一次时间。例如：一1天为周期注册函数在2010-01-01 12:30执行，而
	*  当前时间是2011-03-04 17:53，那么次函数的下次执行时间为2011-03-05 12:30；
	*
	*  3、参数boCallOnNextBoot决定是否在下次启动中的时候恢复调用此函数。提供这个参数
	*  的意义在于解决某些回调函数需要执行的时间可能恰巧在服务器维护期间，由于程序关闭而
	*  导致函数没有被调用，继而使得部分重要数据没有处理的问题。
	*     例如，注册了一个即将在0:30执行的函数，维护发生在0：0到1:0则程序启动时会检查
	*  调用到此函数在维护期间需要执行，因此会立刻调用一次此函数。如果维护在0:0到0:20则
	*  由于函数没有被漏执行，则不会调用。
	*
	*  4、函数调用参数最多支持CScriptValueList::MaxValueCount(16)个，请不要在参数表
	*  中传递指针型参数，除非这个函数不会再下次启动时被调用(boCallOnNextBoot = false)。
	*
	*/
	int registerTimeCall(lua_State* L);

	/*
	* Comments: 查找某个NPC脚本中注册的定时函数句柄
	* Param void * pNPC: NPC对象，可以为nil，为nil是表示在任何NPC注册的定时回调中查找
	* Param char * sFnName: 函数名称
	* @Return void*: 如果找到此函数，则函数返回定时调用对象句柄，否则返回空
	*/
	void* getTimeCall(void *pNPC, char *sFnName);

	/*
	* Comments: 移除一个定时调用函数
	* Param void * hCall: 通过registerTimeCall注册的定时调用对象句柄
	* Param bool boCallOnNextBoot: 在下次启动的时候是否恢复调用
	* @Return bool: 如果移除成功则函数返回true否则返回false。
	* ★注意★	
	*     参数boCallOnNextBoot决定是否在下次启动的时候恢复调用此函数。提供这个参数
	*  的意义在于解决某些回调函数需要执行的时间可能恰巧在服务器维护期间，由于程序关闭而
	*  导致函数没有被调用，继而使得部分重要数据没有处理的问题。
	*     例如，注册了一个即将在0:30执行的函数，维护发生在0：0到1:0则程序启动时会检查
	*  调用到此函数在维护期间需要执行，因此会立刻调用一次此函数。如果维护在0:0到0:20则
	*  由于函数没有被漏执行，则不会调用。
	*/
	bool unregisterTimeCall(void *hCall);

	//用于测试对tolua++修改为支持直接传递lua_State*指针类型的函数导出支持
	inline int rawCallExport(lua_State* L){ return 0; }

	/*
	* Comments:判断当前时间是否在这个时间段内
	* Param int nStartHour:时间段开始的小时
	* Param int nStartMin:分
	* Param int nStartSec:秒
	* Param int nEndHour:时间段结束的小时
	* Param int nEndMin:分
	* Param int nEndSec:秒
	* @Return bool:如果在这个时间段内，返回true，否则false
	*/
	bool isInTimeRange(int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec);

	// 获取当天时间段的长度
	unsigned int getTimeRange(int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec);
	/*
	* Comments:判断当前时间是否在这个日期段内
	* Param int nStartYear:时间段开始的年
	* Param int nStartMonth:月
	* Param int nStartDay:日
	* Param int nStartHour:时间段开始的小时
	* Param int nStartMin:分
	* Param int nStartSec:秒
	* Param int nEndYear:时间段结束的年
	* Param int nEndMonth:月
	* Param int nEndHour:日
	* Param int nEndHour:时间段结束的小时
	* Param int nEndMin:分
	* Param int nEndSec:秒
	* @Return bool:如果在这个日期段内，返回true，否则false
	*/
	bool isInDateRange(int nStartYear,int nStartMonth,int nStartDay,int nStartHour,int nStartMin,int nStartSec,int nEndYear,int nEndMonth,int nEndDay,int nEndHour,int nEndMin,int nEndSec);

	/*
	* Comments:判断当前时间是否在一段偏移时间内 （范围 = 开始时间 + 偏移时间段 + 误差)
	* Param const char * sStarTime:开始时间，格式必须是：20:19:00 这种类型，否则会出错
	* Param unsigned int nOffset:偏移时间段 （秒）
	* Param unsigned int nError:误差（秒）
	* @Return bool:如果在这个时间段内，返回true，否则false
	*/
	bool isInOffsetTimeRange(const char* sStartTime, unsigned int nOffset, unsigned int nError);

	/*
	* Comments:在两个时间段中产生一个随机的时间,起始时间必须小于结束时间，否则会返回nil
	* Param const char * sStarTime:开始时间，格式必须是：2011-03-04 20:19:00 这种类型，否则会出错
	* Param const char * sEndTime:结束时间，格式同上
	* @Return char*:返回的随机时间，格式同上
	*/
	char* getRandDateTime(const char* sStarTime,const char* sEndTime);

	//将时间格式 转换为 短时间格式
	unsigned int encodeNumTime(int nYear,int nMonth,int nDay,int nHour,int nMin,int nSec);
	unsigned int encodeTime(char* sTime);
	
	void decodeTime(unsigned int nMiniTime, int& nYear, int& nMonth, int& nDay);

	/*
	* Comments:获取今日凌晨零点的时间秒数
	* @Return unsigned int:
	*/
	unsigned int getToday();
	/*
	* Comments:获取开服凌晨零点的时间秒数
	* @Return unsigned int:
	*/
	unsigned int getOpenServerToday();

	/*
	* Comments:获取开服凌晨零点的时间秒数
	* @Return unsigned int:
	*/
	unsigned int getOpenServerRelToday();

	/*
	* Comments:获取全局npc的指针
	* @Return void*:
	*/
	void* getGlobalNpc();

	/* 
	* Comments:获取物品npc的指针
	* @Return void *:
	*/
	void * getItemNpc();

	/*
	* Comments: 获取帮派的名字
	* Param int nGuidID:帮派的名字
	* @Return char*:帮派的名字
	*/
	char* getGuildName(unsigned int nGuidID);

	//修改行会资金
	void changeGuildCoin(unsigned int nGuidID,int nValue , int nLogId = 0,const char* pStr = "");

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
	* Comments:获取明日凌晨的时间
	* @Return unsigned int:
	*/
	unsigned int getTomorrow();


	/*
	* Comments:注册怪物死亡后是否要触发脚本
	* Param int nMonsterID:怪物的id
	* Param flag: 是否死亡触发脚本,是表示要触发脚本，否则不触发脚本
	* @Return void:
	*/
	void registerMonsterDeath(int nMonsterID,bool flag =true);
	
	/*
	* Comments: 注册怪物刷新后是否需要调用脚本
	* Param int nMonsterID: 怪物的id
	* Param bool flag:如果是表示
	* @Return void:
	*/
	void registerMonsterMob(int nMonsterID, bool flag =true);

	/*
	* Comments: 注册怪物生命周期到后是否需要调用脚本
	* Param int nMonsterID: 怪物的id
	* Param bool flag:如果是表示
	* @Return void:
	*/
	void registerMonsterLiveTimeOut(int nMonsterID, bool flag =true);

	/*
	* Comments: 设置全局对象。
	* Param const char * pszKey: 对象名称
	* Param CBaseScriptExportObject * object:对象直指
	* @Return bool:
	* @Remark: 用于脚本创建一个C++对象，然后根据Key(变量名称)建立和对象的关联，方便下次使用不用重新创建，直接通过Key查询出对象。
	*/
	bool setObjectVar(const char *pszKey, CBaseScriptExportObject *object);

	/*
	* Comments:根据名称查找对象
	* Param const char * pszKey: 对象名称
	* @Return int:
	*/
	int getObjectVar(lua_State *L);
	
	/*
	* Comments: 移除指定名称的对象
	* Param const char * pszKey: 对象名称
	* @Return bool: 成功返回true；失败返回false
	* @Remark:
	*/
	bool removeObjectVar(const char *pszKey);

	/*
	* Comments: 设置全局指针对象。
		1、如果对象是一个引用计数对象，则调用此函数前调用者必须显示调用addRef。
		2、如果传递空对象则形同于移除对象，如果此对象是一个引用计数对象，则调用此
			函数后调用者必须显示调用release。
	* Param const char * sKey:
	* Param void * ptr:
	* @Return bool:
	*/
	bool setObjectVarPtr(const char* sKey, void* ptr);
	/*
	* Comments: 获取全局指针对象，当对象不再有用时必须调用release函数释放对象
	* Param const char * sKey:
	* @Return void*:
	*/
	void* getObjectVarPtr(const char* sKey);


	//获取全局动态变量，动态变量仅在运行时有效,游戏服务器程序关闭时不会保存到db
	int getDyanmicVar(lua_State *L);

	//获取全局静态变量，静态变量在会保存到数据库
	int getStaticVar(lua_State *L);

	//获取排行榜的第几名的角色id
	unsigned int GetActorIdByCsRank(INT_PTR nRankIndex,int nRank);

	//行会每周维护
	void maintainGuildWeekly();

	//排行榜每天
	void CallmaintainTopTitle();
	
	/*
	* Comments:获取系统的一些配置
	* Param char * pDefName:配置的字符串，比如获取运营商的id，这里输入SPID
	* @Return char *:
	*/
	char * getSystemDef(char * pDefName);

	/*
	* Comments:获取服务器的开区时间
	* @Return unsigned int:返回时间，minitime格式的
	*/
	unsigned int getServerOpenTime();

	/*
	* Comments:获取服务器的合区时间
	* @Return unsigned int:返回时间，minitime格式的
	*/
	unsigned int getServerCombineTime();

	/*
	* Comments:获取当前时间是合服以来的第几天，0表示还没到合服时间，1表示合服当天，2表示合服的第2天，比如1号1合服，2号返回2
	* @Return int:合服以来的第几天
	*/
	int getDaysSinceCombineServer();  

	/*
	* Comments:全服播放全屏特效
	* Param int nEffId::特效id
	* Param int nSec:持续时间
	* @Return void:
	*/
	void playScrEffect(int nEffId,int nSec);

	/*
	* Comments:播放全屏特效，与消息18的区别是，这个是代码实现的特效
	* Param int nEffId:特效id
	* Param int nSec:持续时间
	* @Return void:
	*/
	void playScrEffectCode(int nEffId,int nSec,int x = 0,int y = 0);

	void playSceneScrEffectCode(int nEffId, int nSec, int nScendId = 0, int x = 0,int y = 0);

	/*
	* Comments:给玩家发送离线的文字信息
	* Param int nActorid:
	* Param char * sMsg:
	* Param int nSrc:发出这条消息的角色id，比如GM，当添加消息不成功，比如角色名称错误，需要通知GM,如果是0，表示系统发送
	* @Return void:
	*/
	void sendOfflineMsg(unsigned int nActorid,char* sMsg,int nSrc);

	/* 
	* Comments: 把32位整形数第nBitPos位置nNum (bBit为true时置1，否则置0)
	* Param unsigned int nValue: 32位整形数
	* Param unsigned int nBitPos: 置位位数
	* Param bool bBit: 置位数
	* @Return int:  成功则 返回置位后的整形数，否则返回-1
	*/
	int setIntBit(unsigned int nValue, unsigned int nBitPos, bool bBit);

	/* 
	* Comments: 判断32位整形数nBitPos位是为0/1
	* Param unsigned int nValue: 整形数
	* Param unsigned int nBitPos: 位数
	* @Return int:  
	*/
	int getIntBit(unsigned int nValue, unsigned int nBitPos);

	int getValueMAKELONG(unsigned int param1, unsigned int param2,unsigned int param3);
	/*
	* Comments:刷新各种商城的动态道具
	* Param int nLabel:分类的ID,1表示限时抢购  2表示0元特价 表示1元特价
	* @Return void:
	*/
	void refreshStore( int nLabel);

	/*
	* Comments:获取现在是开服的第几天
	* @Return int:0表示还没开服（比如开服前的测试），1表示今天是开服当前，2表示开服第2天，
	*/
	int getDaysSinceOpenServer();

	
	/*
	* Comments: 对玩家禁言或者解禁
	* Param void * pEntity: 要操作的玩家对象指针
	* Param bool bForbid: 为true表示禁言，否则为解禁。 如果是禁言，nDuration表示禁言时间
	* Param int nDuration: 禁言时间，以秒为单位
	* @Return void:
	* @Remark:
	*/
	void SetShutup(void *pEntity, bool bForbid, int nDuration);

	unsigned int findGuildIdByHandle(unsigned int nHandle);


	/*
	* Comments:给所在的帮派发送消息
	* Param void * pEntity:
	guildPos:大于等于这个职位的人才可以收到
	* Param const char * sMsg:
	* @Return void:
	*/
	void sendGuildMsg(unsigned int nGuildId,int guildPos,const char* sMsg,int nType = ttTipmsgWindow);

	/*
	* Comments: 改变帮派繁荣
	* Param int nGuildId:帮派id
	* Param int value:差值
	* @Return void:
	*/
	void changeGuildFr( unsigned int nGuildId, int value);
	
	/*
	* Comments:通过技能ID返回技能数据
	* Param const INT_PTR nSkillId:技能ID
	* @Return const OneSkillData*: 技能指针
	*/
	char *getSkillNameById(int nSkillId);

	/*
	* Comments: 获取逻辑服务器ID
	* @Return int: 
	* @Remark:
	*/
	int getLocalServerId();

	/*
	* Comments: 获取公共服务器Id
	* @Return int:
	* @Remark:
	*/
	int getCommonServerId();

	//设置传送点是否可见
	void setTeleportVisible(bool bVisible,unsigned int hScene,int x,int y);
	
	//传送点是否可视(可用)
	bool isTeleportVisible(unsigned int hScene,int x,int y);

	/*
	* Comments:本场景随机传送
	* Param void * pEntity:
	* @Return bool:
	*/
	bool telportRandPos(void * pEntity );

	bool telportLastSceneRandPos(void * pEntity);

	//刷新元宝消费排行
	void rfeshConsumeRank();


	/*
	* Comments:更新排行榜（更新头衔，离线数据）
	* @Return void:
	*/
	//void updateRankData();
	
	/*
	* Comments:更新本服雕像
	* @Return void:
	*/
	void updateVocNpcStatue(int nType);

	//发送到SS保存跨服排行
	void saveCsRank();
	//加载跨服排行数据
	void loadCsRank();

	//在m_CsRankList获取serverId
	int getServerIdByCsRank(unsigned int nActorId);

	//装载跨服玩家的离线消息
	void loadUserOfflineDataCs(int nServerId, unsigned int nActorId);

	//设置行会镖车到期时间
	void setGuildExipredTime(unsigned int nGuildId,int nValue);

	//获取行会镖车的句柄
	double getGuildDartHandle(unsigned int nGuildId);

	//设置行会镖车的句柄
	void setGuildDartHandle(unsigned int nGuildId,int nValue);
	

	/*
	* Comments:通过id取怪的名称
	* Param int nMonsterId:怪id
	* @Return char *:返回怪名称，如返回null表示没有这个怪id
	*/
	char * getMonsterNameById( int nMonsterId);

	//通过怪物id取怪物的刷新时间
	int getMonsterTimeById(int nSceneId, int nMonsterId, bool bDie = false);

	//获取怪的等级
	int getMonsterLevel( int nMonsterId );

	//获取是否开启活动勇闯毒潭状态
	int getToxicFreshTimes(unsigned int nGuildId);

	//设置开启勇闯毒潭状态
	void setToxicFreshTimes(unsigned int nGuildId,int nTimes);

	//设置在启勇闯毒潭中击杀的个数
	void setkillToxicNum(unsigned int nGuildId,int nValue);

	//获取在启勇闯毒潭中击杀的个数
	int getkillToxicNum(unsigned int nGuildId);

	/*
	* Comments: 添加buff
	*Param int nGuildId:行会id
	* Param int nBuffType: buff的类型
	* Param int nGroupID: buff的分组 
	* Param double dValue: buff的值
	* Param int nTimes: 作用的次数
	* Param int nInterval:作用的间隔，单位秒
	* Param char * buffName:buff名字
	* Param timeOverlay: 时间是否叠加,如果是true表示叠加，否则表示不叠加
	* Param pGiver: Buff给予者
	* Param int param: 辅助参数
	* @Return void:
	*/
	void onSbkMasterOpenBuff(unsigned int nGuildId,int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay =false, void *pGiver=0, int param = 0);
	
	/*
	* Comments:对文字过滤关键字
	* Param char * sMsg:文字
	* @Return int:返回过滤掉的字数
	*/
	int filterWords(char * sMsg);

	

	//发送爬塔楼主奖励消息
	bool sendPataMasterMsg(char * actorname, char * sMsg, char * title, int bType,int nFubenId, int nDeadLine);

	//通过ID给玩家物品
	bool addItemMsgById(unsigned int nActorId, int nSrcActorId, int nItemId, int nCount, int bBind, char * sTitle, 
		char * sBtn, int nWay, char * sComment="", bool needLog=true, int nMsgType=0, int nParam=0);

	//通过名字给玩家物品
	bool addItemMsgByName(char * sActorName, int nSrcActorId, int nItemId, int nCount, int bBind, char * sTitle, char * sBtn, int nWay);

	/*
	* Comments:获取怪成长信息
	* Param int nMonsterId:
	* Param int & nGrowLv:
	* Param int & nCurExp:
	* @Return int:
	*/
	int getMonsterGrowInfo(int nMonsterId, int & nGrowLv, int & nCurExp);

	//发送物品产出统计
	void sendItemOutPutToLog();


	//获取商城物品价格数据
	int getMerchandisePrice(unsigned int nGoodsId, int &btDealGoldType, int &nPrice, int &nQuality, int &nStrong, int &nBind);

	//是否连接到跨服服务器
	bool isStartCommonServer();

	//是否为公共服
	bool isCommonServer();

	//获取角色最大等级
	unsigned int getPlayerMaxLevel(int nCircle);

	//执行文件存盘操作
	void saveFiles();

	//取得当前短时间，时时获取的，在开服前用到
	unsigned int getRealtimeMiniTime();

	//通过id给玩家发送消息
	bool sendCommonMsgByName(int nMsgId, char * sName, char * sMsg, int nSrcActorId, char * sParam1 = NULL,char * sParam2 = NULL,int nParam = 0, int nParam2 = 0);

	//获取开启改名功能的状态
	bool getchangeNameFlag();

	//发送行会列表到公共服
	void sendGuildListToDestServer();

	//获取行会已挑战的次数
	int getGuildChallengeTimes(unsigned int nGuildId);

	//设置行会挑战的次数
	void setGuildChallengeTimes(unsigned int nGuildId,int nTimes);

	//设置行会下次挑战的时间
	void setGuildNextChallengeTime(unsigned int nGuildId,int nTime);

	//获取行会下次挑战的时间
	int getGuildNextChallengeTime(unsigned int nGuildId);

	//设置行会封印兽碑的等级
	void setGuildBossLevel(unsigned int nGuildId,int nLevel);

	//获得行会封印兽碑
	int getGuildBossLevel(unsigned int nGuildId);

	//设置挑战副本的句柄
	void setBossChanllengeHandle(unsigned int nGuildId,unsigned int bHandle);

	//获得帮派副本句柄
	unsigned int geBossChanllengeHandle(unsigned int nGuildId);

	//获取建筑的等级
	int	getGuildbuildingLev(unsigned int nGuildId,int nType);

	//设置建筑的等级
	void changeGuildBuildingLev(unsigned int nGuildId,int nType,int nValue);

	//获取建筑的剩余cd
	int getGuildBuildingCdTime(unsigned int nGuildId);

	//设置建筑的剩余cd
	void setGuildBuildingCdTime(unsigned int nGuildId,int nTime);

	//发送行会挑战的结果
	void sendGuildChanllengeResult(unsigned int nGuildId,unsigned int nActorId,int nType,int nResult);
	void sendGuildSBKRank(unsigned int nGuildId,int nRankId);
	void sendSBKGuild();
	
	//通过npcID获取npc的名字
	const char * getNpcName(int nNpcId);
	//设置npc 称号
	void setNpcTitle(unsigned int hScene, int nNpcId, char* sTitle);

	
	/* 
	* Comments:获取实体handle,返回uint64，double getHandle(void* pEntity)在ActorExportFun.h
	* Param void * pEntity: 
	* @Return Uint64:  
	*/
	unsigned long long getEntityHandle(void* pEntity);

	//设置大篝火影响的比例
	void setFireValRate(float fVal);

	//删除全服邮件
	void delServerMail(int nServerMailId);

	int  sendSysMail(lua_State *L);
	//通过掉落发送邮件
	int sendSysDropMail(lua_State *L);

	int getMailAttachValue(lua_State* L , int nTableIndex, const char* sKey);

	unsigned int getBossLastKillTime(int nBossId);		//下发BOSS最近被击杀的时间

	// 判断这个活动id对应的活动是否运行中
	bool isActivityRunning(int nAtvId);

	//结束活动 nNextTime是否开启下一段时间
	void closeActivityRunning(int nAtvId, bool nNextTime = false);
	/*
	* comment ：是否到了活动结束前的n秒	
	* Param：int activityId
	* Param: int second_before_end
	* return bool : 0:当前时间未达到结束前的n秒 1 :当前时间达到结束前的n秒
	*/
	bool isReachSecondBeforeActivityEnd(int activity_id, int second_before_end);

	//获取活动结束时间
	unsigned int getActivityEndMiniSecond(int activity_id);

	//在线玩家发送单个活动数据
	void sendAllActorOneActivityData(int nAtvId);
	//个人活动开启时间（开服天数）
	int getPActivityOpenDay(int nAtvId);

	//登陆时检测全局个人活动的开启结束
	void  CheckGPActivityStartEnd(void* pEntity, int atvId, unsigned int ScriptEndTime);


	//发送消息
	void SendChatMsg(char* msg, int nChatchannelId, void* pEntity = NULL, int nitemId = 0);
	
	int getRunningActivityId(lua_State* L);
	//获取活动开始时间
	int getRunningActivityStartTime(int nAtvId);
	int getRunningActivityStartTimeRelToday(int nAtvId);
	// 添加异步工作
	int addAsyncWorker(lua_State* L);

#pragma region CrossBattlefield
	///  降魔战场报名
	/// sysarg 玩家实体指针
	/// bAdd 1= 报名, 0 = 取消报名
	void CrossBattlefieldSignup(void* sysarg, bool bAdd);
	/// 获取当前已报名人数
	int GetCurrentCrossBattlefieldPlayerNum();
	/// 查询玩家是否已经报名
	bool HadCrossBattlefieldSignup(void* sysarg);
	/// 数据初始化
	void initCrossBattlefieldData(int nTriggerNewActivity, int nNewActivityPlayers);

	/* 
	* Comments:设置系统多倍经验
	* Param int nRate:
	* Param int nTime:
	* @Return void:
	*/
	void setSysRateExp(int nRate, int nTime);

	/* 
	* Comments:获取场景中心点坐标
	* Param int nSceneId:场景
	* Param int & nPosX:返回的x
	* Param int & nPosY:返回的y
	* @Return bool:
	*/
	bool getSceneCenterPos(int nSceneId, int & nPosX, int & nPosY);

	

	//在实体所在坐标创建物品，这里目前只支持道具
	bool createDropItems(void* pEntity, int nWardType, int nItemId, int nCount, int nBind = 1);

	//创建一个掉金钱
	bool createDropMoney(void* pEntity, int nWardType, int nMoneyType, int nDropCount, int nBind = 1);

	void sendWorldLevel(void* pEntity);
	//获取世界等级
	int getWorldLevel();

	/* 
	* Comments:设置加速BUFF的ID
	* Param int nBuffId:BUFF ID
	* @Return void:
	*/
	void setQuickBuffId(int nBuffId);


	int NewRedPacketId();

	/// 短时间转换为字符串
	const char* miniDateTime2String(unsigned int tv);
	/// 报名玩家列表
	//int getCrossBattlefieldSignupList( lua_State *L ); ///< 貌似不行的
	//马上发送消费记录
	void SendConsumeLog();
    //刷新世界等级
	void RefeshWorldLevel();
	//热更新物品
	void reloadItem();
	//热更新语言包
	void reloadLang();
	//热更新怪物
	void reloadMonster();
	//热更新VSPDef
	void reloadVSPDef();
	//热更新Fw();
	void reloadFw();
	//热更新配置
	void reloadConfig(int nId);
	//获取spid
	int getSpid();
	//获取sspid
	const char* getStrSpid();
	//MD5加密（不支持多线程）
	const char* MD5(lua_State *L);

	int sendCrossServerMail(lua_State *L);
	//踢全服玩家
	void KickAllCrossServerActor();
#pragma endregion CrossBattlefield
	//*/
};


//tolua_end
