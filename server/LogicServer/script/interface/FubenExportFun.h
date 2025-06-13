//tolua_begin
#ifdef WIN32
#include <WinNT.h>
#endif
namespace Fuben
{
	/*
	* Comments: 通过副本的ID创建一个副本
	* Param int nFbID:副本的ID
	* @Return unsigned int:副本的handle
	*/
	unsigned int createFuBen( int nFbID);

	
	/*
	* Comments：副本有专门给脚本存储数据的空间，一个32个int的数组，这个函数用于取出对应的数值
	* Param void * pFb:副本的指针
	* Param int nIndex:索引值
	* @Return int:
	*/
	int getFbValue(void * pFb,int nIndex);

	/*
	* Comments:设置副本的存储内容
	* Param void * pFb:副本指针
	* Param int nIndex:索引值
	* Param int nValue:设置的值
	* @Return void:
	*/
	void setFbValue(void * pFb, int nIndex, int nValue);

	/*
	* Comments:获取副本队伍的队员数目
	* Param void * pTeam:队伍的指针
	* @Return int:
	*/
	int getTeamOnlineMemberCount(unsigned int nTeamId);

	/*
	* Comments:获取队伍的总人数，包括不在线的
	* Param unsigned int nTeamId:
	* @Return int:
	*/
	int getTeamMemberCount(unsigned int nTeamId);

	/*
	* Comments:获取这个队伍要进入的副本的id
	* Param void * pTeam:
	* @Return int:
	*/
	int getTeamFubenId(unsigned int nTeamId);

	
	/*
	* Comments:获取队伍允许加入的队员的人数，总队员数不能超过，用于加入副本队伍的判断
	* Param unsigned int nTeamId:队伍id
	* @Return int:
	*/
	int getTeamMaxMemberCount(unsigned int nTeamId);

	/*
	* Comments:判断这个角色能否进入这个副本,主要用于结婚系统中，新郎（新娘）拒绝捣乱的人进入结婚副本
	* Param UINT hFbHandle:副本的handle
	* Param UINT nAcotid:角色id
	* @Return bool:如是被拒绝的返回true，否则返回false
	*/
	bool isRefuseActor(unsigned int hFbHandle,unsigned int nAcotid);

	/*
	* Comments:把一个玩家加入到这个副本的拒绝名单中
	* Param UINT hFbHandle:副本的handle
	* Param UINT nAcotid:角色id
	* @Return void:
	*/
	void addRefuseActor(unsigned int hFbHandle,unsigned int nAcotid);

	/*
	* Comments:获取副本ID
	* Param void * pFb:副本指针
	* @Return int:
	*/
	int getFubenIdByPtr(void* pFb);

	/*获取此副本归属的行会ID
	返回值：0-此副本不属于任何行会
	*/
	unsigned int getGuildIdByByPtr(void* pFb);

	void* getFubenPtrByHandle(unsigned int nFubenHandle);

	/*
	* Comments:踢出所有的玩家，玩家将返回到普通场景，如果本副本是常规副本，这函数不起作用,同时这个副本占用的内存也会被释放掉
	* Param UINT hFuben:副本的handle
	* @Return void:
	*/
	void closeFuben(unsigned int hFuben);

	/*
	* Comments:根据副本指针获取handle
	* Param void * pFuben:
	* @Return unsigned int:
	*/
	unsigned int getFubenHandle(void* pFuben);

	/*
	* Comments:获取普通副本的handle
	* @Return unsigned int:
	*/
	unsigned int getStaticFubenHandle();

	/*
	* Comments:创建一个怪物
	* Param unsigned int hScene:场景的handle
	* Param unsigned int nMonsterid:怪物id，对应怪物配置表中的id
	* Param int posX:怪物出现的x坐标，注意：怪物出现的场景和副本和玩家是一样的。
	* Param int posY:怪物出现的y坐标
	* Param unsigned int nLiveTime:怪物存活的时间
	* @Return void*:如果成功创建，返回怪物的指针，失败返回NULL，一般失败的原因是 xy坐标不正确
	*/
	void* createMonster(unsigned int hScene,unsigned int nMonsterid,int posX,int posY,unsigned int nLiveTime, unsigned int nBornLevel = 0);
	//获取可生成怪物坐标
	int getreateMonsterPosXY(lua_State* L);

	void setMonsterIndex(void* pEntity, int nIdx);

	void ResetMonsterShowName(void* pEntity, const char*  sName);

	bool AddMonsterBuffById(void* pEntity, int nBuffId);

	void DelMonsterBuffById(void* pEntity, int nBuffId);

	/*
	* Comments:在指定的坐标范围内生成N个怪物
	* Param unsigned int hScene:场景的handle
	* Param unsigned int nMonsterid:怪物id，对应怪物配置表中的id
	* Param int posX1:坐标范围的x起始坐标
	* Param int posX2:坐标范围的x终止坐标,posx2必须大于等于posx1，否则产生不了正确的怪物
	* Param int posY:y坐标， 这3个值表示在x1-x2的水平线上随机位置产生N个怪物
	* Param int nCount:需要生成的怪物数量
	* Param unsigned int nLiveTime:怪物存活的时间
	* Param bool bForcePos:随机第一个点，就强制选定这个点
	* Sample: Fuben.createMonstersAndSetPathpoint(hScene, 13, 79, 90, 31, 3, 0, 96, 29, 99, 26)
	* @Return void:
	*/
	void createMonsters(unsigned int hScene,unsigned int nMonsterid,int posX1,int posX2,int posY,int nCount,unsigned int nLiveTime, unsigned int nBornLevel = 0);

	int createMonstersInRange(unsigned int hScene,unsigned int nMonsterid,int posX1,int posY1,int posX2,int posY2,int nCount,
		unsigned int nLiveTime, unsigned int nBornLevel = 0, char* szMonsterName=NULL, unsigned int nForceVesterId=0,
		bool bForcePos=false);

	//在指定的坐标生成1个怪物
	void* createOneMonsters(unsigned int hScene,unsigned int nMonsterid,int posX,int posY,int nCount,unsigned int nLiveTime, unsigned int nBornLevel = 0,
		char* szMonsterName = NULL,
		int nForceVesterId = 0,
		int nInitMonsterPercent = 100);
	/*
	* Comments: 在制定的坐标范围内生产N个怪物。如果是护送怪，设置怪物的路径点	
	* Param unsigned int hScene: 场景句柄
	* Param unsigned int nMonsterId: 怪物Id
	* Param int nPosX1: 坐标范围的x起始坐标
	* Param int nPosX2: 坐标范围的x终止坐标,posx2必须大于等于posx1，否则产生不了正确的怪物
	* Param int nPosY: y坐标， 这3个值表示在x1-x2的水平线上随机位置产生N个怪物
	* Param int nCount: 刷怪的数量
	* Param unsigned int nLiveTime: 怪物存活时间
	* Param int ...(vararg) 位置坐标。每一个点包含x,y两个坐标，数目不限制。
	* @Return int:	
	*/
	int createMonstersAndSetPathpoint(lua_State *L);

	int getMonsterCount(void* pFuben, int nMonsterId);

	int getSceneMonsterCount(void* pFuben, int nScenId, int nMonsterId);

	/*
	* Comments:根据名字获取场景的handle，没有找到则返回0
	* Param const char * sSceneName:场景名字
	* Param unsigned int hFuben:指定的副本handle，如果是普通场景的，则写0
	* @Return unsigned int:没有找到则返回0
	*/
	unsigned int getSceneHandleByName(const char* sSceneName,unsigned int hFuben);

	/*
	* Comments:根据id获取场景的handle，没有找到则返回0
	* Param int nSceneID:场景id
	* Param unsigned int hFuben:指定的副本handle，如果是普通场景的，则写0
	* @Return unsigned int:没有找到则返回0
	*/
	unsigned int getSceneHandleById(int nSceneId,unsigned int hFuben);

	
	/*
	* Comments:获取场景某种怪物属于的剩余数量
	* Param unsigned int hScene:场景handle
	* Param int nMonsterId:怪物id，如果是0，表示计算所有怪物
	* @Return int:
	*/
	int getMyMonsterCount(unsigned int hScene,int nMonsterId);

	/*
	* Comments:让指定的怪物全部退出当前场景
	* Param unsigned int hScene:
	* Param int nMonsterId: =0清除场景所有怪物
	* @Return void:
	*/
	void clearMonster(unsigned int hScene,int nMonsterId = 0);

	/*
	* Comments: 让指定怪退出场景（消失）
	* Param double handle: 怪物句柄
	* Param bool bDelayDestroy: 是否延迟删除
	* @Return void:
	*/
	void clearMonster(double handle, bool bDelayDestroy = false);


	/* 
	* Comments: 清除场景实体
	* Param unsigned int hScene: 场景handle
	* Param int nType: 类型
	* @Return void:  
	*/
	void clearSceneEntity(unsigned int hScene, int nType);
	/* 
	* Comments: 强制杀死怪物
	* Param void * pMonster: 怪物指针
	* Param void * pEntity: 人物指针
	* @Return void:  
	*/
	void killMonsterByActor(void* pMonster, void* pEntity);
	/*
	* Comments:给场景内的所有玩家发信息
	* Param unsigned int hScene:场景handle
	* Param char * sMsg:信息内容
	type:系统提示显示的类型
	* @Return void:
	*/
	void sendMsgToScene(unsigned int hScene, char* sMsg,int nType);

	/*
	* Comments:给全场景的人发个倒计时
	* Param unsigned int hScene:
	* Param int nTime:
	* @Return void:
	*/
	void sendCountDown(unsigned int hScene,int nTime, int nFlag);

	/*
	* Comments:给副本内所有玩家发信息
	* Param unsigned hFb:副本handle
	* Param char * sMsg:信息内容
	type:系统提示显示的类型
	* @Return void:
	*/
	void sendMsgToFb(unsigned hFb,char* sMsg,int nType);

	/*
	* Comments:根据npc的名字获取指针
	* Param unsigned int hScene:
	* Param char * szName:npc的名字
	* @Return void*:
	*/
	void* getNpcPtrByName(unsigned int hScene,char* szName);

	/*
	* Comments:获取副本内其中一个玩家的指针
	* Param unsigned int hFb:副本handle
	* @Return void*:
	*/
	void* getFbActorPtr(unsigned int hFb);

	/*
	* Comments:根据怪物id获取场景内的怪物指针，只返回一个
	* Param unsigned int hScene:
	* Param int nMonsterId:
	* @Return void*:
	*/
	void* getMonsterPtrById(unsigned int hScene,int nMonsterId);

	/*
	* Comments:根据怪物id和idx获取场景内的怪物指针，只返回一个
	* Param unsigned int hScene:
	* Param int nMonsterId:
	* @Return void*:
	*/
	void* getMonsterPtrByIdx( unsigned int hScene,int nMonsterId, int nMonsterIdx);
	/* 
	* Comments: 获取攻击过该怪物的玩家id列表
	* Param lua_State * L: 怪物指针
	* @Return int:  
	*/
	int getMonsterBeAttackActorList(lua_State *L);

	/*
	* Comments:测试玩家所在的场景所有怪物是否被杀死
	* Param unsigned int hScene:
	* Param int nMonsterId:0表示所有怪物，否则表示指定的怪物id
	* @Return bool:
	*/
	bool isKillAllMonster(unsigned int hScene,int nMonsterId = 0);

	/*
	* Comments:获取玩家所在场景某种怪物的剩余数量
	* Param unsigned int hScene:
	* Param int nMonsterId:怪物id
	* @Return int:数量
	*/
	int getLiveMonsterCount(unsigned int hScene,int nMonsterId);

	/*
	* Comments:设置场景的剩余时间，通常用于限时的活动
	* Param unsigned int hScene:场景handle
	* Param int nTime:剩余时间，单位秒，通常只设置一次
	* @Return void:
	*/
	void setSceneTime(unsigned int hScene, int nTime);

	/*
	* Comments:获取场景的剩余时间，通常用于限时的活动，必须在调用之前使用setSceneTime设置时间
	* Param unsigned int hScene:场景handle
	* @Return int:剩余时间，单位秒
	*/
	int getSceneTime(unsigned int hScene);

	/*
	* Comments: 根据场景id获取名字
	* Param int nSceneId:场景id
	* @Return char*:
	*/
	const char* getSceneNameById(int nSceneId);

	//设置一个场景里的npc是否可见 bset false 可见 true 不可见
	void setSceneNpcCanNotSee(int nSceneId,bool bSet);



	/*
	* Comments:设置区域属性
	unsigned int hScene:场景的handle
	* Param INT_PTR nIndex:区域的索引值，对应场景的配置表中区域索引，不能超过总区域的数量
	* Param INT_PTR nType:区域属性类型，见tagmapattribute定义
	* Param LPCSTR sValues:属性的参数，如"1,2,10"等，函数内会分解成多个整数保存,最多支持256个整数
	* Param LPCSTR sFunc:如果是进入区域执行脚本的类型，这个要写执行的脚本函数,以下两个参数只有触发脚本的属性才有效
	* Param Uint64 hNpc:执行的NPC的handle
	* Param int notips 当前区域是否提示
	* @Return bool:设置是否成功，失败的原因都是nIndex或者nType越界,或者参数有误
	*/
	bool setSceneAreaAttri(unsigned int hScene,int nIndex,int nType,const char* sValues,const char* sFunc=NULL, double hNpc=0, int notips = 0);

	//x y 坐标在场景sceneid 中的区域
	int GetAreaListIndex(int nSceneId,int x, int y, unsigned int fbHandle = 0);

	// /*
	// * Comments:设置区域属性
	// unsigned int SceneId:场景的id
	// * Param INT_PTR x,y:场景x， y坐标
	// * Param INT_PTR nType:区域属性类型，见tagmapattribute定义
	// * Param LPCSTR sValues:属性的参数，如"1,2,10"等，函数内会分解成多个整数保存,最多支持256个整数
	// * Param int notips 当前区域是否提示
	// * @Return bool:设置是否成功，失败的原因都是nIndex或者nType越界,或者参数有误
	// */
	// bool AddSceneAreaAttri( unsigned int SceneId,int x, int y,int nType,const char* sValues, int notips = 0);

	//当前场景是否属于同一个工会
	int GetNowSceneGuildList(unsigned int nSceneId);
	//重置副本场景 区域配置
	void ResetFubenSceneConfig(int nSceneId);
	/*
	* Comments:获取杀死boss的玩家的个数
	* Param void * pfb:副本指针
	* @Return int:
	*/
	int getKillBossCount(void* pfb);

	/*
	* Comments:获取副本中杀死过boss的玩家的名字
	* Param void * pfb:副本指针
	* Param int index:玩家索引
	* @Return const char*:玩家名字
	*/
	const char* getKillBossActorName(void *pfb,int index);

	/*
	* Comments:获取副本的剩余时间
	* Param void * pfb:
	* @Return int:
	*/
	int getFbTime(void* pfb);

	/*
	* Comments:获取副本中，玩家死亡的总次数
	* Param void * pfb:
	* @Return int:
	*/
	int getActorDieCount(void* pfb);

	/*
	* Comments:获取副本内出现过的怪物的总数，包括已死亡的
	* Param void * pfb:
	* @Return int:
	*/
	int getFbMonsterTotal(void* pfb);

	/*
	* Comments:获取副本中被杀死的怪物的数量
	* Param void * pfb:
	* @Return int:
	*/
	int getKillMonsterCount(void* pfb);

	//获取动态变量，动态变量仅在运行时有效,参数是副本指针
	int getDyanmicVar(lua_State *L);

	void clearDynamicVar(lua_State *L);

	/*
	* Comments:获取场景中玩家数量
	* Param unsigned int nScene:场景句柄
	* @Return int:返回场景玩家的数量
	*/
	int getScenePlayerCount(unsigned int nScene);

	/*
	* Comments:获取场景的宽度
	* Param unsigned int nScene:场景句柄
	* @Return int:
	*/
	int getSceenWidth(unsigned int nScene);

	/*
	* Comments:获取场景的高度
	* Param unsigned int nScene:场景句柄
	* @Return int:
	*/
	int getScenHeight(unsigned int nScene);

	/*
	* Comments:判断这个点是否可以移动
	* Param unsigned int nScene:场景句柄
	* @Return int:
	*/
	bool onCanMove(unsigned int nScene,int x,int y);

	/*
	* Comments:通过npc名字从所有场景中其指针（不包含副本)
	* Param const char * sNpcName:npc名字
	* @Return void *:返回npc指针
	*/
	void * getNpcPtrFromAllScenes(const char * sNpcName);

	/* 
	* Comments:通过id获取场景指针
	* Param int nSceneId:
	* @Return CScene *:
	*/
	CScene * getScenePtrById( int nSceneId);

	/*
	* Comments:通过名字取得场景指针
	* Param const char * pSceneName:
	* @Return CScene *:
	*/
	CScene * getScenePtrByName(const char * pSceneName);

	/*
	* Comments:通过场景id和npcid获取npc指针
	* Param unsigned int nSceneId:场景id
	* Param unsigned int nNpcId:npcID
	* @Return void *:返回npc的指针
	*/
	void * getNpcPtrById( unsigned int nSceneId, unsigned int nNpcId);

	//通过场景handle获取npcID
	void * getFbNpcPtrById(unsigned int nSceneHandle, unsigned int nNpcId);

	/*
	* Comments:判断场景所在的地图区域是否含有指定的属性
	* Param void * pEntity:
	* Param int nAttriValue:见tagMapAreaAttribute的定义
	* @Return bool:
	*/
	bool hasMapAreaAttri(void* pScene, int nPosX, int nPosY, int nAttriValue);

	//设置副本时间
	void setFbTime(void * pFb, unsigned int nTime);

	//清除地图中的所有怪
	void clearAllMonster(unsigned int hScene);

	int getEntityCountAtXY(unsigned int hScene, int nX, int nY, int nEntityType);

	/* 
	* Comments:场景内全体玩家发送快捷栏上的消息（不消失）
	* Param void * pEntity:
	* @Return void:
	*/
	void SendSecneShortCutMsg(unsigned int hScene, const char* szMsg);

	//获取地图进入需要等级
	void getSceneNeedLevel( void* ptrScene, int x, int y, int& nLevel, int& nCircle);

	//设置某个坐标点不可移动( true 不可移动， false 可移动)
	void setUnableMovePoint( unsigned int hScene, int x, int y,bool boValue);

	int getSceneId(unsigned int hScene);

	//副本中启用默认的场景刷怪方式
	bool useDefaultCreateMonster(unsigned int fbHandle,bool flag= 0 );

	bool isFuben(void* pFb);

	void ExitAllFbActor(void* pFb);
	int getMonsterHp(unsigned int hScene,int nMonsterId );
	int getMonsterMaxHp(unsigned int hScene,int nMonsterId );
};

//tolua_end
