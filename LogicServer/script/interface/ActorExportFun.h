/*
	玩家的一些脚本接口的定义，在脚本里使用前加Actor.xx函数，比如：
	Actor.hasQuest(entityPtr,任务id)来获取是否完成了这个任务
*/


//tolua_begin
namespace Actor
{
	
	/*
	* Comments:通过handle获取实体的指针
	* Param Uint64 handle: 实体的句柄
	* @Return void *: 实体指针
	*/
	void* getEntity(double handle);

	/*
	* Comments: 根据角色名查找角色实体
	* Param const char * szActorName: 角色名称
	* @Return void*: 如果角色在线，返回角色对象指针；否则返回NULL
	* @Remark:
	*/
	void* getActorByName(const char* szActorName);

	/*
	* Comments: 根据角色id查找角色实体
	* @param nActorID 角色id
	* @Return void*: 如果角色在线，返回角色对象指针；否则返回NULL,离线玩家是找不到的
	* @Remark:
	*/
	void* getActorById(unsigned int nActorID);

	/*
	* Comments:通过实体的指针返回实体的名字
	* Param void * pEnity: 实体的指针
	* @Return char *:实体的名字
	*/
	const char * getName(void * pEnity);

	//获取账号名
	const char * getAccount(void* pEntity);

	//根据玩家id在帮派成员列表查找名字
	char* findGuildMemberName(void* pEntity,unsigned int nActorid);

	//根据排名找行会的名称
	char* getGuildNameByRank(void* pEntity,int nRank);

	//根据排名找行会的首领名称
	const char* getGuildLeaderNameByRank(void* pEntity,int nRank);

	//根据排名找行会的人数
	int getGuildMemNumByRank(void* pEntity,int nRank);

	//根据行会排名找行会的资金
	int getGuildCoinByRank(void* pEntity,int nRank);

	//根据排名判断行会是否存在
	bool hasGuildByRank(void* pEntity,int nRank);

	void sendWelcomeToNewMember(void* pEntity,char* nMsg);
	/*
	* Comments:获取实体的handle
	* Param void * pEntity:
	* @Return unsigned long long:
	*/
	double getHandle(void* pEntity);

	/*
	* Comments: 获取实体类型
	* Param void * pEntity: 实体对象指针
	* @Return int: 对应于tagEntityType结构。玩家为0， 怪物为1， npc为2， 宠物为4
	* @Remark:
	*/
	int getEntityType(void* pEntity);

	//获取杀人着的句柄
	double getKillHandle(void* pEntity);


	/*
	* Comments: 玩家是否有一个任务
	* Param void * pEnity:实体的指针
	* Param int nQuestID:任务id
	* @Return BOOL:有任务返回TRUE,否则返回FALSE
	*/
	bool hasQuest(void * pEnity,int nQuestID);
	/*
	* Comments: 触发任务
	* Param void * pEnity:实体的指针
	* Param int nQuestID:任务类型
	* @Param int nValue:进度
	* Param int nId: 完成id 默认为0
	*/
	void ExOnQuestEvent(void * pEntity,int nQuestType, int nValue, int nId = 0);

	/*
	* Comments:关闭客户端的NPC的对话框
	* Param void * pEntity:
	* @Return void:
	*/
	void closeNPCDialog(void * pEntity);

	/*
	* Comments:在客户端打开角色的某个属性窗口，如技能窗口，背包窗口等
	* Param void * pEntity:
	* Param int nType:窗口的类型定义，见tagOpenDialogID
	* Param const char*:打开窗口参数
	* @Return void:
	*/
	void openDialogs(void * pEntity,int nType,const char* sParam = "");

	/*
	* Comments:关闭客户端的一个窗口，如技能窗口，背包窗口等
	* Param void * pEntity:
	* Param int nType:窗口的类型定义，见tagOpenDialogID
	* @Return void:
	*/
	void closeDialogs(void * pEntity,int nType);


	/*
	* Comments:获取角色的门派信息
	* Param void * pEntity:
	* @Return int:
	*/
	int getMenPai(void * pEntity);

	/*
	* Comments:获取角色上次下线那天获取的经验值
	* Param void * pEntity: 角色指针
	* @Return int:
	*/
	int getExpLastLogout(void *pEntity);

	/*
	* Comments:获取角色上次下线那天获取的阵营贡献
	* Param void * pEntity: 角色指针
	* @Return int:
	*/
	int getZyContriLogout(void *pEntity);

	/*
	* Comments: 设置社会关系的位
	* Param INT_PTR nMaskID: maskID，同上
	* Param bool flag:如果设置是就传入true，否则传入false
	* @Return void:
	*/
	bool setSocialMask(void *pEntity,int nMaskID,bool flag);

	/*
	* Comments: 获取一个社会关系的位
	* Param INT_PTR nMaskID: maskID,定义在SystemParamDef.h里，如
		enum tagSocialMaskDef
		{
		smGuildCommon=0,    //帮会普通成员
		smGuildTangzhu=1,    //堂主
		smGuildAssistLeader=2, //副帮主
		smGuildLeader=3,     //帮主
		};
	* @Return bool: 如果设置了就返回true,否则返回false
	*/
	bool getSocialMask(void *pEntity,int nMaskID);

	/*
	* Comments:角色进入副本，
	* Param void * pEntity:
	* Param unsigned int hFbHandle: 副本的handle
	* Param int nScenceID: 场景的ID
	* Param int nPosX: 位置x
	* Param int nPosY: 位置y
	* @Return bool:
	*/
	bool enterFuBen(void * pEntity,unsigned int hFb,int nSceneId,int x,int y);

	//进入活动场景
	bool EnterActivityScene(void* pEntity,int nSceneId,int x, int y, int nWeight = 0, int nHeight = 0,int nNpcId=0 );





	/*
	* Comments:进入场景 半径宽与高大于0则选择随机点传送
	* Param void * pEntity:
	* Param nSceneId:
	* Param int x:
	* Param int y:
	* Param int nWeight: 半径宽
	* Param int nHeight: 半径高
	* @Return bool:
	*/
	bool enterScene(void* pEntity,int nSceneId,int x, int y, int nWeight = 0, int nHeight = 0, int nValue = 0, int nParam = 0);


	/*
	* Comments:学习一个技能已经学会，这个主要给怪物使用
	* Param void * pEntity:
	* Param INT_PTR nSkillID: 技能的ID
	* Param nLevelID: 技能的等级
	* @Return int:返回学习技能的错误码，如果没有错误，返回0
	*/
	int learnSkill(void * pEntity,int nId,int nLevel);

	/*
	* Comments:开始学习一个技能
	* Param void * pEntity:玩家指针
	* Param int nId:技能id
	* Param int nLevel:技能等级
	* @Return bool:成功返回true
	*/
	bool startLearnSkill( void * pEntity, int nId, int nLevel);

	//获取技能的等级
	int getSkillLevel(void * pEntity,int nId);

	/*
	* Comments:删除某个技能
	* Param void * pEntity:
	* Param int nId:技能id
	* @Return void:
	*/
	void delSkill(void * pEntity, int nId);

	/*
	* Comments:删除某个物品
	* Param void * pEntity:
	* Param int wItemId:物品id
	* Param int btQuality:品质
	* Param int btStrong:强化值
	* Param int btCount:数量
	* Param int btBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param sComment: 添加物品的运营日志
	* Param nLogID: 添加物品的日志号
	* nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品符合条件
	* Param bIncEquipBar: 是否删除装备
	* @param bFullCount 一定足够数量才扣
	* @Return int:返回删除的物品的数量
	*/
	int removeItem(void * pEntity,int wItemId,int btCount,int btQuality=-1,int btStrong=-1,int btBindFlag = -1,
		char * sComment=NULL,int nLogID=2,const int nDuraOdds = -1,bool bIncEquipBar = false/*, bool bFullCount = false*/);

	/*
	* Comments:增加一个物品
	* Param void * pEntity:
	* Param int wItemId:物品id
	* Param int btQuality:品质
	* Param int btStrong:强化值
	* Param int btCount:数量
	* Param int btBindFlag:是否绑定
	* Param int nLeftTime:物品剩余时间，秒为单位。0表示不限时间
	* Param sComment: 添加物品的运营日志
	* Param nLogID: 添加物品的日志号
	* Param nQualityDataIndex:当需要产生极品属性时，传一个极品属性的库编号列表索引
	* @Return int:返回实际增加的物品的数量
	*/
	int addItem(void * pEntity,int wItemId,int btQuality,int btStrong,int btCount,int btBindFlag, int nLeftTime = 0, char * sComment=NULL,int nLogID=1, int nQualityDataIndex = -1);

	/*
	* Comments:获取实体的整形属性值
	* Param void * pEntity:
	* Param int pid:属性id，属性id见PropertyDef.h里的定义
	* @Return int:属性值
	*/
	unsigned int getIntProperty(void * pEntity,int pid);

	/*
	* Comments:获取实体的整形属性值
	* Param void * pEntity:
	* Param int pid:属性id，属性id见PropertyDef.h里的定义
	* @Return int:属性值
	*/
	int getIntProperty2(void * pEntity,int pid);

    //获取数值巨大的属性值(如经验)
	double getUInt64Property(void * pEntity,int pid);

	//获取战力
	int getActorBattlePower(void * pEntity);

	/// 设置汇总处理标记
	void collectOperate(void * pEntity,int nType);

	/*
	* Comments:获取实体的无符号整形属性值，用于战魂值,pk值(杀戮值) 等少数几个属性用
	* Param void * pEntity:实体的指针 
	* Param int nPropID:属性的ID
	* Param int nValue: 属性的值
	* @Return bool: 成功返回true，否则返回false
	*/
	bool setUIntProperty(void * pEntity,int nPropID,unsigned int nValue);
	bool setUInt64Property(void * pEntity,int nPropID, double dValue);

	//设置怪物的属性
	void setMonsterProperty(void * pEntity,int nPropID,unsigned int nValue);

	/*
	* Comments:通过一个物品的指针和数量来删除
	* Param void * pEntity:玩家的的指针
	* Param void * pUserItem:物品的指针
	* Param int nCount:要删除物品的数量
	* Param bool bNeedFreeMemory :是否需要释放内存
	* @Return int:返回删除的数量
	*/
	int removeItemByPtr(void * pEntity,void * pUserItem, int nCount,bool bNeedFreeMemory,char * sComment,int nWayLogID);

	/*
	* Comments:玩家的血是否满了
	* Param void * pEntity: 实体的指针
	* @Return bool:如果玩家的血满了，就返回true，否则返回false
	*/
	bool isHpFull(void * pEntity);

	/*
	* Comments: 玩家的蓝是否是满了
	* Param void * pEntity:  实体的指针
	* @Return bool:如果玩家的蓝满了，就返回true，否则返回false
	*/
	bool isMpFull(void * pEntity);

	/*
	* Comments: 返回是否在某个区域的标志
	* Param void * pEntity:  实体的指针
	* @Return bool:
	*/
	int getSceneAreaFlag(void * pEntity);

	//通过buffID添加buff
	bool addBuffById(void * pEntity, int nBuffId);
	void addBuffValueById(void * pEntity, int nBuffId, int nValue = 0);

	//添加行会buff
	void addGuildBuff(void * pEntity,int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay =false, void *pGiver=0, int param = 0);

	/*
	* Comments:删除一个buff
	* Param void * pEntity: 实体的指针
	* Param int nBuffType: buff的类型
	* Param int nGroupID:buff的组
	* @Return void:
	*/
	void delBuff(void * pEntity,int nBuffType,int nGroupID);

	/*
	* Comments:通过id删除一个buff
	* Param void * pEntity:
	* Param int nBuffId:
	* @Return void:
	*/
	void delBuffById( void * pEntity, int nBuffId);
	/*
	* Comments:把实体身上group属于[nGroupStart,nGroupEnd]的全部删除
	* Param INT_PTR nGroupStart: 起始 的组id
	* Param INT_PTR nGroupEnd: 结束的组id
	* @Return INT_PTR:返回删除的个数
	*/
	void RemoveGroupBuff(void * pEntity, int nGroupStart,int  nGroupEnd) ;

	/*
	* Comments:玩家身上是否存在某种buff
	* Param void * pEntity:实体的类型
	* Param int nBuffType:buff的类型
	* Param int nGroup:buff的组，如果任何组的话，使用-1
	* @Return bool:如果存在buff返回true，否则返回false
	*/
	bool hasBuff(void * pEntity,int nBuffType, int nGroup=-1);

	/*
	* Comments:是否有这类型的BUFF
	* Param void * pEntity:玩家指针
	* Param int nBuffId:buffID
	* @Return bool:
	*/
	bool hasBuffById( void * pEntity, int nBuffId);

	/*
	* Comments: 玩家的活力是否满了
	* Param void * pEntity: 玩家的指针
	* @Return bool: 如果玩家的活力满了，就返回true，否则返回false
	*/
	//bool isEnergeFull(void * pEntity);

	/*
	* Comments: 改变HP
	* Param void * pEntity: 玩家的指针,增加怪物类型也可以减血
	* Param int nValue: 改变的HP值，比如加100，这里就是100，减100就是-100
	* @Return void:
	*/
	void changeHp(void * pEntity,int nValue);

	/*
	* Comments:修改玩家的MP
	* Param void * pEntity: 玩家的指针
	* Param int nValue: 改变的MP值，比如加100，这里就是100，减100就是-100
	* @Return void:
	*/
	void changeMp(void * pEntity,int nValue);

	/* 统计指定物品的数量
	* Param void * pEntity: 玩家的指针
	 * wItemId	物品ID
	 * nQuality	物品品质，-1表示匹配所有品质；
	 * nQuality	物品强化等级，-1表示匹配所有强化等级;
	 * nBind 是否绑定，如果-1表示不判断是否绑定
	 * nDuraOdds 耐力差 = 最大耐力-当前耐力 -1表示不检测 检测少于此耐力差的物品符合条件
	 * pNoItem:如果存在指针，不计算该物品
	 * Param bool bIncEquipBar: 是否在装备栏搜索。如果为true，在背包中没找到的话就会在装备栏中搜索；否则不在装备栏搜索
	 * @return	返回匹配的物品总数量
	 */
	int getItemCount(void* pEntity, const int wItemId, 
		const int nQuality=0, const int nStrong=0,const int nBind =-1,const int nDuraOdds = -1,void * pNoItem = NULL,bool bIncEquipBar = false);

	/* 
	* Comments:获取仓库指定物品数量
	* Param void * pEntity:玩家指针
	* Param int wItemId:物品id
	* @Return int:
	*/
	int getDepotItemCount(void * pEntity, int wItemId );

	/*
	* Comments: 能否添加物品
	* Param void * pEntity:实体的指针
	* Param int nItemID:物品的id
	* Param int nCount: 物品的数量
	* Param int nQuality: 物品的品质
	* Param int nStrong:物品的强化等级
	* Param bool isBinded:物品是否绑定
	* @Return bool:
	*/
	bool canAddItem(void * pEntity, int nItemID,int nCount,int nQuality=0, int nStrong =0, bool isBinded =false);

	/*
	* Comments: 根据系统提示ID发送系统提示
	* Param void * pEntity: 玩家的指针
	* Param int nTipmsgID:系统提示ID
	* Param int nTipmsgType:系统提示的类型
	* @Return void:
	*/
	void sendTipmsgWithId(void* pEntity,int	nTipmsgID,int nTipmsgType=ttTipmsgWindow );



	/*
	* Comments:带参数发送tipmsg,在配置里需要用%s标示参数，比如 %s 将装备强化到%s级
	* Param void * pEntity: 实体的指针
	* Param int nTipmsgID:tipmsgID
	* Param int nTipmsgType:tipmsg的类型,ttTipmsgWindow等
	* Param char * sParam1:参数1 
	* Param char * sParam2:参数2
	* Param char * sParam3:参数3
	* @Return void:
	*/
	void sendTipmsgWithParams(void * pEntity, int	nTipmsgID,int nTipmsgType=ttTipmsgWindow,char * sParam1=NULL,char *sParam2=NULL,char * sParam3 =NULL );

	/*
	* Comments:发送系统提示消息,自己组装好了，不需要组装就调用这个接口
	* Param LPCTSTR sTipmsg:组装好的字符串
	* Param INT_PTR nType:系统提示显示的类型
	* @Return void:
	*/
	void sendTipmsg(void* pEntity, const char* sTipmsg,int nType);
	
	/*
	* Comments:获取玩家的金钱的数量
	* Param int nMoneyType:金钱的类型,eMoneyType 里定义的
	*/
	unsigned int getMoneyCount(void * pEntity,int nMoneyType);
	
	/*
	* Comments:修改玩家的金钱的数量
	* Param int nMoneyType:金钱的类型,eMoneyType 里定义的
	* Param double nValue:金钱的数值
	* Param int nLogid:日志id 后台行为id
	* Param const char* sComment:备注
	* @Return bool:成功返回true，否则返回false
	*/
	bool changeMoney(void * pEntity,int nMoneyType,double nValue,int nLogid,const char* sComment);

	/*
	* Comments:判断玩家身上有没有这个装备
	* Param void * pEntity:
	* Param const int wItemId:装备的id
	* @Return bool:有则返回true
	*/
	bool hasEquiment(void* pEntity,const int wItemId);
	
	void* getEquiment(void* pEntity,const int wItemId);

	//删掉玩家身上这个装备
	bool DelEquiment(void* pEntity,const int wItemId);

	/*
	* Comments:获取这个玩家是否已经加入了某个队伍
	* Param void * pEntity:
	* @Return bool:有则返回true
	*/
	bool isInTeam(void* pEntity);

	/*
	* Comments:使玩家退出当前队伍，如果有加入的话
	* Param void * pEntity:
	* @Return void:
	*/
	void exitTeam(void* pEntity);


	/*
	* Comments:把玩家加入到一个队伍中，主要是副本队伍中使用
	* Param void * pEntity:
	* Param unsigned int nTeamId:
	* @Return void:
	*/
	void addToTeam(void* pEntity,unsigned int nTeamId);

	/*
	* Comments:获取玩家当前所在的副本的id
	* Param void * pEntity:
	* @Return int:
	*/
	int getFubenId(void* pEntity);

	/*
	* Comments:获取玩家所在的场景id
	* Param void * pEntity:
	* @Return int:
	*/
	int getSceneId(void* pEntity);

	/*
	* Comments:获取所在的场景名称，和坐标
	* Param void * pEntity:
	* Param int & x:
	* Param int & y:
	* @Return char*:
	*/
	char* getSceneName(void* pEntity,int& x,int& y);

	/*
	* Comments:玩家退出副本，如果玩家当前正在普通场景，则函数不处理，如果正在副本，则退回普通场景
	* Param void * pEntity:
	* @Return void:
	*/
	void exitFuben(void* pEntity, int nSceneId=-1, int nPosX=-1, int nPosY=-1);

	/*
	* Comments:玩家退出副本并回到回城点，如果玩家当前正在普通场景，则函数不处理；
										 如果玩家正在副本，则退回普通场景并回城
	* Param void * pEntity:
	* @Return void:
	*/
	void exitFubenAndBackCity( void* pEntity );
	
	/*
	* Comments: 获取技能的属性
	* Param void * pEntity:玩家的指针
	* Param int nSkillID:技能的ID
	* Param int nPropID: 属性的ID,1表示等级，2表示经验
	* @Return int: 返回技能的属性
	*/
	int getSkillProperty(void* pEntity,int nSkillID,int nPropID);

	
	/*
	* Comments: 设置玩家的技能的属性
	* Param void * pEntity: 玩家的等级
	* Param int nSkillID:技能的ID
	* Param int nPropID: 属性的ID,1表示等级，2表示经验
	* Param int nValue:  新的数值
	* @Return bool: 成功返回true，否则返回false
	*/
	bool setSkillProperty(void * pEntity,int nSkillID,int nPropID,int nValue);

	/*
	* Comments:获取角色当前副本的handle
	* Param void * pEntity:
	* @Return VOID*:
	*/
	unsigned int getFubenHandle(void* pEntity);

	/*
	* Comments:获取角色当前场景的handle
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getSceneHandle(void* pEntity);


	/*
	* Comments:获取角色所在的副本的指针
	* Param void * pEntity:
	* @Return void*:
	*/
	void* getFubenPrt(void* pEntity);

	/*
	* Comments:获取角色所在的场景的指针
	* Param void * pEntity:
	* @Return void*:
	*/
	void* getScenePtr(void* pEntity);

	/*
	* Comments:增加一个消息框并且发送到客户端
	* Param EntityHandle hNpc:将执行这些脚本的npc,如果为0，则使用全局的npc
	* Param UINT nActorId:接收这个消息的角色id，如果是0，则表示发送给自己
	* Param char* title:对话框的标题
	* Param char * * sFnName:脚本（按钮）的内容，文本内容为包含了按钮要显示的文字，以及点击后要执行的函数名，中间用 “/”隔开，比如"确定/commonAcceptMissions,1"
	//其中“确定”是客户端要显示的按钮的文字，commonAcceptMissions,1是要执行的脚本函数和参数
	* Param INT_PTR nButtonCount:按钮的数量
	* Param unsigned int nTimeOut:对话框存在的时间,单位：毫秒
	* Param int msgType:0:直接弹出	1：停在屏幕右侧，用户点击才弹出
	* Param int nTimeOutBtn:对话框倒计时结束执行按钮ID 第1个按钮 1,第2个按钮 2....
	* Param char*sTip:鼠标移上去的tip，弹出类型是1才有用
	* Param WORD wIcon:图标ID
	* Param int nTimeOutBtn:倒数时间结束默认执行的按钮ID
	* Param int msgId:消息ID。一般为角色ID。用于msgType=1时叠加同个消息 为0时客户端自动分配 
	* Param int showId:是否弹出提示框系统ID 在SystemParamDef.h(enum tagMsgBoxShowId)定义
	* @Return int:返回错误码
	*/
	int messageBox(void* pEntity,double hNpc,unsigned int nActorId,const char* sTitle,const char* sBtn1,const char* sBtn2,const char* sBtn3,
		unsigned int nTimeOut=0,int msgType=0,const char* sTip=NULL,int nIcon=0,int nTimeOutBtn=0, int msgId = 0, int showId=mbsDefault);
	

	/*
	* Comments:获取玩家当前正在聊天的npc的handle
	* Param void * pEntity:
	* @Return unsigned long long:如果没有，则返回0
	*/
	double getNpc(void* pEntity);

	/*
	* Comments:判断两个玩家是否在同一屏，比如结婚，需要两个人在同一屏幕内（不允许走远）
	* Param void * pEntity:
	* Param void *  other:
	* @Return bool:在同一屏返回true
	*/
	bool isInSameScreen(void* pEntity,void *  other);


	/*
	* Comments: 获取队友的数量,包含自己，比如自己加另外一个队友就是2个人
	* Param void * pEntity: 玩家的指针
	* @Return int:返回队友的数量
	*/
	int getTeamMemberCount(void  * pEntity);

	/*
	* Comments: 获得玩家在线的队友的数量(包括自己)
	* Param void * pEntity:玩家的指针
	* @Return int:返回队伍里在线的队员的数量，包含玩家自己
	*/
	int getOnlineTeamMemberCount(void  * pEntity);


	/*
	* Comments: 获取第nIndex个队友的信息(不包含自己)[0-3有效]
	   注意：队友包含不在线的，如果不在线的话nActorID不为0，但是返回的指针为NULL
	* Param int nIndex:0-3的数字
	* Param int & nActorID: 队友的actorID
	* @Return void*: 返回队友的指针
	*/
	void* getTeammemberByIndex(void  * pEntity,int nIndex,unsigned int &nActorID);

	/*
	* Comments:根据角色的名称获取指针，如果不在线或没有这个名称的，则返回NULL
	* Param char * sName:
	* @Return void*:
	*/
	void* getActorPtrByName(char* sName);

	void* getActorPtrById(unsigned int nActorId);
	
	/*
	* Comments:创建一个怪物,创建出来后，怪物的拥有者是玩家
	* Param void * pEntity:玩家的指针
	* Param unsigned int nMonsterid:怪物id，对应怪物配置表中的id
	* Param int posX:怪物出现的x坐标，注意：怪物出现的场景和副本和玩家是一样的。
	* Param int posY:怪物出现的y坐标
	* Param unsigned int nLiveTime:怪物存活的时间
	* Param bool addSelfName:是否在怪物名字后面加上自己的名字
	* @Return void:如果成功创建，返回怪物的指针，失败返回NULL，一般失败的原因是 xy坐标不正确
	*/
	void* createMonster(void * pEntity,unsigned int nMonsterid,int posX,int posY,unsigned int nLiveTime,bool addSelfName= false);

	/*
	* Comments:设置怪物的归属者成自己
	* Param void * pEntity:
	* Param void * pMonster:怪物的指针
	* @Return void:
	*/
	void setMonsterOwer(void* pEntity, void* pMonster);

	/*
	* Comments: 获取怪物归属者
	* Param void * pEntity:怪物指针
	* @Return void:
	*/
	void* getMonsterOwner(void *pEntity);

	//设置怪物所属玩家角色name
	bool setMonsterOwnerName(void* pMonster, int nIndex, char *sName);

	//获取怪物所属玩家角色name
	//const char * getMonsterOwnerName(void* pMonster, int nIndex);

	//设置怪物的名字
	void setMonsterName(void* pEntity,char* sName);

	//怪物掉落物品
	void monsterRealDropItemExp(void *pEntity,void* pMonster);

	//设置怪物是否可以移动
	void setMonsterCanMove(void *pEntity,bool bState);


	/*
	* Comments:判断复活时间是否超时（用户超过5分钟没有选择回城复活还是原地复活），如果到达，脚本处理，送回主城复活
	* Param void * pEntity:玩家指针
	* Param unsigned int nTimeOut:超时时间，策划文档写是5分钟，即 5x60 = 300 .这个单位是秒
	* @Return void:
	*/
	void setReliveTimeOut(void* pEntity,unsigned int nTimeOut);

	/*
	* Comments:把复活超时时间变成0
	* Param void * pEntity:
	* @Return void:
	*/
	void clearReliveTimeOut(void* pEntity);

	/*
	* Comments:获取玩家复活的剩余超时时间
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getReliveTimeOut(void* pEntity);

	bool reqEnterFuben(void* pEntity, int nFubenId);
	bool reqExitFuben(void* pEntity, int nFubenId);

	/*
	* Comments:回城！你懂的
	* Param void * pEntity:
	* @Return void:
	*/
	void returnCity(void* pEntity);

	/*
	* Comments:复活！如果现在在普通场景，则复活到上一个普通场景的复活点，否则复活在副本里
	* Param void * pEntity:
	* @Return void:
	*/
	void relive(void* pEntity, bool isReturn = false);

	/*
	* Comments:获取角色上次下线的时间
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getLastLogoutTime(void* pEntity);

	//获取本次的登录时间
	unsigned int getLoginTime(void *pEntity);
	
	/*
	* Comments:获取队伍id，如果不在队伍，返回0
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getTeamId(void* pEntity);

	/*;
	* Comments:发送打开行会集结的面板
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	bool sendOpenBuildFormToClient(void* pEntity);

	/*;
	* Comments:获取在帮派的在职时间，给脚本保存数据用
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getGuildZzTime(void* pEntity);

	/*
	* Comments:设置帮派的在职时间，如果上次没保存的数据，直接设置成0
	* Param void * pEntity:
	* Param unsigned int nTime:
	* @Return void:
	*/
	void setGuildZzTime(void *pEntity,unsigned int nTime);

	/*
	* Comments:获取在帮派的获取福利时间，给脚本保存数据用
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	unsigned int getGuildWealTime(void* pEntity);

	/*
	* Comments:设置帮派的获取福利时间，如果上次没保存的数据，直接设置成0
	* Param void * pEntity:
	* Param unsigned int nTime:
	* @Return void:
	*/
	void setGuildWealTime(void *pEntity,unsigned int nTime);

	/*
	* Comments:清除召集令，召集令在每日凌晨定期清理，因此用户登陆时，如果上次下线时间是昨天，则调用这个函数
	* Param void * pEntity:
	* @Return void:
	*/
	void clearGuildZjItem(void *pEntity);

	/*
	* Comments:获取玩家的pk模式
	* Param void * pEntity:
	* @Return int:
	*/
	int getPkMode(void* pEntity);

	/*
	* Comments:设置玩家的pk模式
	* Param void * pEntity:
	* Param int nMode:
	* @Return void:
	*/
	void setPkMode(void* pEntity, int nMode);

	/*
	* Comments:测试玩家所在的场景所有怪物是否被杀死
	* Param void * pEntity:
	* Param int nMonsterId:0表示所有怪物，否则表示指定的怪物id
	* @Return bool:
	*/
	bool isKillAllMonster(void* pEntity,int nMonsterId = 0);

	/*
	* Comments:获取玩家所在场景某种怪物的剩余数量
	* Param void * pEntity:
	* Param int nMonsterId:怪物id
	* @Return int:数量
	*/
	int getLiveMonsterCount(void* pEntity,int nMonsterId);

	/*
	* Comments:获取玩家所在场景某种怪物属于本人的剩余数量
	* Param void * pEntity:
	* Param int nMonsterId:怪物id，如果是0，表示计算所有怪物
	* @Return int:
	*/
	int getMyMonsterCount(void* pEntity,int nMonsterId);

	/*
	* Comments:给一个玩家增加经验
	* Param void * pEntity:
	* Param int nValue:增加经验的值,减少就填负数
	* Param int nWay:经验的来源,比如任务，排名，工会等,在GameLog里定义
	* Param int nParam:如果是任务，这个就填写任务的ID，其他的话填关键的有意义的参数，如果没有就填写0
	* Param  int nType:广播类型 1为周围玩家广播 2自己广播
	* @Return void:
	*/
	void addExp(void* pEntity, int nValue, int nWay,int nParam, int nType = 0);

	//获取动态变量，动态变量仅在运行时有效，玩家下线将清空且不会保存
	int getDyanmicVar(lua_State *L);

	//获取静态变量，静态变量在玩家下线时会保存且在玩家上线时会加载
	int getStaticVar(lua_State *L);

	/*
	* Comments:判断玩家所在的地图区域是否含有指定的属性
	* Param void * pEntity:
	* Param int nAttriValue:见tagMapAreaAttribute的定义
	* @Return bool:
	*/
	bool hasMapAreaAttri(void* pEntity, int nAttriValue);

	/*
	* Comments:判断玩家所在的地图区域是否含有指定的属性
	* Param void * pEntity:
	* Param int nAttriValue:见tagMapAreaAttribute的定义
	* @Return bool:
	*/
	bool hasMapAreaAttriValue(void* pEntity, int nAttriValue,int nValue);


	/* 
	* Comments: 获取区域属性的属性值，只适用于只有一个参数的区域属性，比如经验加成、修为加成
	* Param void * pEntity: 
	* Param int nAttriValue: 见tagMapAreaAttribute的定义
	* @Return int:  
	*/
	int getMapAttriValue(void* pEntity, int nAttriValue);


	/*
	* Comments: 触发成就事件,比如玩家强化装备等，用于玩家获得成就
	* Param int nEventID:事件的ID成就事件的定义在AchieveEvent.h里定义
	   如果带参数的表示成就事件里会对这些参数进行条件过滤，比如强化一个物品到5级
	* Param void * pEntity:实体的指针
	* Param int nValue： 完成度
	* Param int nSubType: 子成就类型 默认为1 
	* @Return void:
	*/
	void triggerAchieveEvent(void * pEntity, int nEventID,int nValue, int nSubType = 1);

	/*
	* Comments:获取正在对话的npc的名字，注意：没有则会返回null
	* Param void * pEntity:
	* @Return char*:
	*/
	const char* getTalkNpcName(void* pEntity);

	/*
	* Comment：检查是否可以传送 false：已达到限制  1： 可以传送
	*    每日清零，当前npc活动在用，使用静态计数器
	* 	Param void *pActor 
	*	int staticType :静态计数器key
	*	int dayLimit   :每日次数限制
	*	@Return bool 
	*/
	bool checkNpcTranLimit(void *pActor,int staticType, int dayLimit);
	/*
	* Comment：传送后对计数器累加
	*    每日清零，当前npc活动在用，使用静态计数器
	* 	Param void *pActor 
	*	int staticType :静态计数器key
	*	int dayLimit   :累加步长，默认1
	*	@Return bool 
	*/
	bool addNpcTranTimes(void *pActor,int staticType, int num= 1) ;

		
	/*
	* Comments: 获取玩家好友数量
	* Param void * pActor:玩家指针
	* @Return int:
	*/
	int getActorFriendsCount(void *pActor);

	/*
	* Comments: 获取玩家第idx个好友actorid
	* Param void * pActor:玩家指针
	* Param int idx:好友索引
	* @Return int:
	*/
	int getActorFriend(void *pActor, int idx);

	/*
	* Comments: 往实体身上投递一个延迟回调消息。延迟时间到了之后，会在此实体身上触发事件
	* Param void * pEntity: 实体对象	
	* Param int nDelayTime: 延迟时间(ms)
	* Param bool & result: 返回操作结果。成功返回true，失败返回false	
	* Param int param1: 消息参数，回在回调中传出此参数
	* Param int param2: 消息参数，回在回调中传出此参数
	* Param int param3: 消息参数，回在回调中传出此参数
	* Param int param4: 消息参数，回在回调中传出此参数
	* Param int param5: 消息参数，回在回调中传出此参数
	* Param bool bForceCallWhenDestroy: 是否实体销毁时强制执行。实体可能在某些消息还没执行就被销毁，此标记用于决定是否实体销毁前强制执行此消息
	* @Return int: 返回消息编号唯一标识（采用GetTickCount）
	@ @Note: 调用者需先检测操作成功与否。
	*/
	int postEntityScriptDelayMsg(void *pEntity, int nDelayTime, bool &result, int param1 = 0, int param2 = 0, int param3 = 0, int param4 = 0, int param5 = 0, bool bForceCallWhenDestroy = false);

	/*
	* Comments: 从实体消息队列中移除一个脚本延迟消息
	* Param void * pEntity: 实体对象
	* Param unsigned int nMsgIndex: 消息编号
	* @Return bool:
	*/
	bool removeEntityScriptDelayMsg(void *pEntity, unsigned int nMsgIndex);

	/*
	* Comments:判断某个物品能否使用，通常是判断角色的区域是否有这个限制
	* Param int nItemId:物品id
	* @Return bool:
	*/
	bool canUseItem(void *pEntity,int nItemId);

	/*
	* Comments: 玩家的成就是否完成了
	* Param void * pEntity:玩家的指针
	* Param int nAchieveID:成就的id
	* @Return bool: 已经完成了返回true,否则返回false
	*/
	bool isAchieveFinished(void * pEntity,int nAchieveID);

	/* 
	* Comments:是否领过该成就奖励
	* Param void * pEntity:玩家指针
	* Param int nAchieveID:成就id
	* @Return bool:
	*/
	bool isAchieveGiveAwards(void * pEntity, int nAchieveID);
	
	
	/*
	* Comments:获取行会捐献奖励是标记
	* Param void * pEntity:
	* @Return int:
	*/
	//void getGuildCoinAwardFlag(void * pEntity, int & nFlag, int & nTodayCoin, int & nTenDay);

	
	/*
	* Comments:设置行会捐献奖励是标记
	* Param void * pEntity:
	* Param char nFlag:
	* @Return void:
	*/
	//void setGuildCoinAwardFlag(void * pEntity, char nFlag, int nTodayCoin, int nTenDay);

	//获取当天行会捐献金币数量
	int getGuildTodayDonateCoin(void * pEntity);

	//设置当天行会捐献金币数量
	void setGuildTodayDonateCoin(void * pEntity, unsigned int nTodayCoin);

	//获取当天行会捐献道具数量
	int getGuildTodayDonateItem(void * pEntity);

	//设置当天行会捐献道具数量
	void setGuildTodayDonateItem(void * pEntity, unsigned int nTodayItem);

	//发送行会捐献数据
	void sendGuildDonateData(void * pEntity);

	//发送行会升级道具
	void sendGuildUpgradeItem(void * pEntity);

	//发送行会详细信息
	void sendGuildInfo(void * pEntity);

	void sendGuildTreeData(void * pEntity);
	void sendGuildFruitData(void * pEntity);

	int getGuildTreeTodayChargeNum(void * pEntity);
	void setGuildTreeTodayChargeNum(void * pEntity, int nTodayChargeNum);

	int getGuildFruitTodayPickNum(void * pEntity);
	void setGuildFruitTodayPickNum(void * pEntity,  int nTodayPickNum);
	void sendGuildFruitPickData(void * pEntity);

	/*
	* Comments:根据实体的朝向，
	* Param void * pEntity:
	radio:半径范围，多少个网格
	* Param int & nX:获取到的位置x,返回值
	* Param int & nY:
	* @Return bool:如果返回false，表示无可放的点
	*/
	bool getPositionByDir(void* pEntity, int radio,int& nX, int& nY);
		
	/*
	* Comments: 怪物发言
	* Param void * pEntity: 发言者实体指针
	* Param const char * pMsg: 发言内容
	* Param int nBCType: 发言广播类型。参见MonsterSayBCType类型定义
	* @Return void:
	*/
	void monsterSay(void *pEntity, const char *pMsg, int nBCType);


	/*
	* Comments: 跟随某个玩家
	* Param void * pEntity: 跟随者
	* Param void * pActor: 被跟随的玩家
	* Param unsigned int nMinDist: 跟随保持最小距离
	* Param unsigned int nMaxDist: 跟随最大距离。超过这个距离就停止跟随，并且通知玩家跟随失败
	* Param bool bStopWhenFailed: 是否跟随失败后停止跟随。true表示跟随失败后停止跟随
	* @Return void:
	*/
	//void followActor(void *pEntity, void *pActor, unsigned int nMinDist = 2, unsigned int nMaxDist=50, bool bStopWhenFailed = false);

	/*
	* Comments:判断可视范围内是否有指定的怪物存在
	* Param void * pEntity:
	* Param int nMonsterId:怪物id
	* Param int nCount:怪物的数量，比如只要有1只就行，传1
	* @Return bool:存在指定的怪物，并且数量大于nCount则返回true
	*/
	bool hasMonsterNear(void* pEntity, int nMonsterId,int nCount);

	/*
	* Comments: 注册脚本回调函数
	* Param void * pEntity: 注册者实体指针
	* Param void * pNPC:	npc对象指针。如果指针为空，执行的是全局脚本函数；否则是执行NPC的脚本函数
	* Param int nDelay: 第一次执行的延迟时间(ms)
	* Param int nInterval: 执行间隔时间(ms)，最低间隔为5s，小于5s自动调整为5s
	* Param int nCount:执行次数,如果是-1表示无限次
	* Param const char * pszFuncName:脚本函数名称
	* Param ...  不定长脚本参数，用于执行脚本回调函数时回传的参数列表，最多15个。
	* @Return unsigned int: 返回脚本回调的句柄
	*/	
	/// 已废，2013/09/17
	int regScriptCallback(lua_State *L);


	/*
	* Comments:注册定时器
	* Param void * pEntity: 注册者实体指针
	* Param int nNPCId:	npc的Id。如果Id为0，执行的是全局脚本函数；否则是执行NPC的脚本函数
	* Param int nDelay: 第一次执行的延迟时间(ms)
	* Param int nInterval: 执行间隔时间(ms)，最低间隔为5s，小于5s自动调整为5s
	* Param int nCount:执行次数,如果是-1表示无限次
	* Param const char * pszFuncName:脚本函数名称
	* Param ...  不定长脚本参数，用于执行脚本回调函数时回传的参数列表，最多15个。
	* @Return unsigned int: 返回脚本回调的句柄
	* @Return int:
	*/
	int regScriptTimer(lua_State *L);

	/*
	* Comments: 注销脚本回调函数
	* Param void * pEntity: 实体指针
	* Param unsigned int handle: 脚本回调句柄
	* @Return unsigned int:
	*/
	void unregScriptCallbackByHandle(void *pEntity, unsigned int handle);

	/*
	* Comments: 注销脚本回调函数
	* Param void * pEntity: 实体指针
	* Param const char * pFnName: 脚本回调函数名称
	* @Return void:
	*/
	void unregScriptCallback(void *pEntity, const char *pFnName);

	/*
	* Comments: 玩家查询增值卡
	* Param void * pEntity:实体对象
	* Param const char *pSN:增值卡序列号字符串		
	* Param bool bQueryOrUse: 查询还是使用。true表示查询，false表示使用
	* @Return void:
	*/
	void queryServicesAddCard(void *pEntity, const char *pSN, bool bQueryOrUse);

	/*
	* Comments:
	* Param void * pEntity: 玩家使用增值卡
	* Param double seriesNo:增值卡序列号（这个是查询结果返回的卡号）
	* @Return void:
	*/
	void userServicesAddCard(void *pEntity, double seriesNo);

	/*
	* Comments:判断是否怪物
	* Param void * pEntity:
	* @Return bool:是返回TURE
	*/
	bool isMonster(void* pEntity);

	/*
	* Comments:获取实体所在的xy坐标
	* Param void * pEntity:实体指针，可以是玩家、怪物、npc等
	* Param int & x:返回值x坐标
	* Param int & y:返回值y坐标
	* @Return void:这类型的函数调用方法是：local x,y = getEntityPosition(npcPointer,0,0) 参数0，0不重要
	*/
	void getEntityPosition(void* pEntity,int& x, int& y);

	//获得角色曾经最高的等级和转数
	void getEntityMaxCircleAndLevel(void* pEntity, int &nCircle, int &nLevel);

	/*
	* Comments: 设置实体是否跟着归属者传送
	* Param void * pEntity: 实体指针
	* Param bool bSet: true表示跟着归属者一起传送；否则不跟着归属者传送
	* @Return void:
	*/
	void setEntityTelByOwnderAttr(void *pEntity, bool bSet);

	/*
	* Comments: 设置玩家不是通过传送门传送时是否，怪物下属是否跟着一起传送
	* Param void * pEntity: 怪物对象指针
	* Param bool bSet: 为true表示跟着玩家一起传送；
	* @Return void:
	* @Remark:
	*/
	void setEntityTelByOwnerWhenNonePortal(void *pEntity, bool bSet);

	/*
	* Comments: 设置实体是否惰性跟随
	* Param void * pEntity: 实体指针
	* Param bool bSet: true表示惰性跟随；否则为非惰性跟随
	* @Return void:
	*/
	void setEntityLazyFollow(void *pEntity, bool bSet);

	/*
	* Comments: 设置实体能攻击怪物属性
	* Param void * pEntity: 实体指针
	* Param bool bSet: true表示可以攻击怪物；否则为不可攻击怪物
	* @Return void:
	*/
	void setEntityCanAttackMonster(void *pEntity, bool bSet);

	/*
	* Comments: 设置实体是否可被玩家攻击
	* Param void * pEntity: 实体指针
	* Param bool bSet: true表示不可被玩家攻击；否则为可被玩家攻击
	* @Return void:
	*/
	void setDenyAttackedByActor(void *pEntity, bool bSet);

	/*
	* Comments: 设置实体是否可被攻击
	* Param void * pEntity: 实体指针
	* Param bool bSet: true表示可被攻击；否则不可被攻击
	* @Return void:
	*/
	void setDenyBeAttack(void *pEntity, bool bSet);

	/*
	* Comments:当复活时候触发
	* Param void * pEntity:
	* @Return void:
	*/
	void onRelive(void* pEntity);

	/*
	* Comments:给客户端发送倒计时时间，客户端收到这个消息，显示一个倒计时的特效
	* Param void * pEntity:
	* Param int nTime:倒计时时间，单位：秒，不超过10s
	* @Return void:
	*/
	void sendCountdown(void* pEntity, int nTime, int  nFlag = 0 , int nType = 1);

	

	/*
	* Comments: 给玩家奖励
	* Param void * pEntity: 玩家的指针
	* Param int nType:奖励的类型id，通用的
	* Param int nId:奖励的id，一般用于物品ID
	* Param double nValue: 奖励的数值，比如经验的话就是经验的数值
	* Param int nQuality:品质
	* Param int nStrong:强化值
	* Param int nBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param int auxParam:辅助参数，比如学习技能的话就是技能的等级
	* Param int nLogId: 记录日志的id
	* Param char * pLogStr: 记录日志的字符串
	* Param int nQualityDataIndex:极品属性-库编号索引,1开始
	* @Return bool:成功返回true，否则false
	*/
	bool giveAward(void* pEntity,int nType, int nId, double nValue,
		int nQuality = 1, int nStrong = 1, int nBindFlag = 0,int auxParam=0,int nLogId=0,char* pLogStr="script", int nQualityDataIndex = -1);
	
	/*
	* Comments:判断消耗值
	* Param INT_PTR nType:消耗的类型
	* Param int nId: 附加ID 如果是物品 则为物品ID 
	* Param int nValue:检测值
	* Param int nQuality:品质
	* Param int nStrong:强化值 -1表示不限
	* Param int nBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param int nParam:辅助参数，比如学习技能的话就是技能的等级
	* @Return unsigned int:满足则返回true
	*/
	bool checkConsume(void* pEntity, int nType, int nId, double nValue, int nQuality = -1, int nStrong = -1, int nBindFlag = -1, int nParam = 0);

	//检测玩家资源，带tips的
	bool CheckActorSource(void* pEntity, int nType, int nId, int nCount, int nTipmsgType = tstUI);

	unsigned long long getConsume(void* pEntity, int nType, int nId, int nValue=0,int nQuality = -1, int nStrong = -1, int nBindFlag = -1, int nParam = 0);
	/*
	* Comments:消耗
	* Param INT_PTR nType:消耗的类型
	* Param int nId: 附加ID 如果是物品 则为物品ID 
	* Param double nValue: 消耗值
	* Param int nQuality:品质
	* Param int nStrong:强化值
	* Param int nBindFlag:是否绑定,如果是-1表示绑定和非绑定都可以
	* Param int nParam:辅助参数，比如学习技能的话就是技能的等级
	* Param INT_PTR nLogId: 计入日志的ID
	* Param LPCTSTR sComment:如果需要存日志的话，用于存盘用的描述
	* @Return unsigned int:返回已消耗的值
	*/
	int removeConsume(void* pEntity, int nType, int nId, double nValue, 
		int nQuality = -1, int nStrong = -1, int nBindFlag = -1,int nParam = 0,int nLogId = 0,char* pLogStr = "script consume");


	/*
	* Comments:使用技能
	* Param INT_PTR nSkillID:技能ID
	* Param INT_PTR nPosX: 作用点的x
	* Param INT_PTR nPosY:作用点的y
	* Param bool needLearn: 是否需要学习才能 使用，特殊的技能不学习也能使用的
	    比如那种队伍光环
	* INT_PTR nSkillLevel: 技能的等级，这个一般可以不配置，特殊的技能使用
	* @Return INT_PTR:如果使用没有错误就返回0，否则返回使用技能的错误码
	*/
	INT_PTR useSkill(void* pEntity,int nSkillID, int nPosX =0,int nPosY=0,bool needLearn =true,int nSkillLevel=0, bool boCd = false);

	/*
	* Comments:获取阵营的id
	* Param void * pEntity:实体的指针
	* @Return int: 返回阵营的id
	*/
	int getCampId(void* pEntity); 
	
	/*
	* Comments:设置阵营的id
	* Param void * pEntity: 实体的指针
	* Param int nCampId:阵营的id
	* @Return void:
	*/
	void setCampId(void* pEntity,int nCampId);

	/*
	* Comments:设置怪物的行会id
	* Param void * pEntity: 实体的指针
	* Param int nCampId:行会的id
	* @Return void:
	*/
	void SetNpcGuildId(void* pEntity,unsigned int nGuildId);

	//死亡以后立刻回复的HP的比例
	void setDieRefreshHpPro(void* pEntity,int nValue);

	//获取怪物的行会id
	unsigned int getNpcGuildId(void* pEntity);

	//改变名称
	void changeEntityName(void* pEntity,char* sName);

	/*
	* Comments:获取帮派的地位
	smGuildCommon=0,    //帮会普通成员
	smGuildTangzhu=1,    //堂主
	smGuildAssistLeader=2, //副帮主
	smGuildLeader=3,     //帮主
	* Param void * pEntity:
	* @Return int:
	*/
	int getGuildPos(void* pEntity);
	//行会职位名称
	char* getGuildTitleName(void* pEntity);

	//获取皇城职位(攻城战)
	int getGuilCityPos(void* pEntity);

	//查看行会名片
	void sendGuildCardByRank(void* pEntity,int nRank);

	//申请加入
	void joinToGuild(void* pEntity,int nJoinGuildId);

	//竞价抢位
	void onStartBidGuildRank(void* pEntity);

	//判断是否报名行会攻城
	bool isSignUpGuildSiege(void* pEntity);
	//自己行会是否是沙巴克
	int MyGuildIsSbk(void* pEntity);

	/*
	* Comments:获取玩家所在帮派的帮主的角色id
	* Param void * pEntity:
	* @Return int:
	*/
	unsigned int getGuildLeaderId(void* pEntity);

	/*
	* Comments:获取帮派等级
	* Param void * pEntity:
	* @Return int:
	*/
	int getGuildLevel(void* pEntity);


	//设置参加帮派斗魂副本的标志
	void setGuldBossFlag(void* pEntity,bool bFlag);

	bool getGuildBossFlag(void* pEntity);
	//发送神装boss次数
	void SendSzBossTimes( void* pEntity);

	void setGuildDartFlag(void* pEntity,bool bFlag);

	/*
	* Comments:增加帮派战死亡的数目
	* Param int nTime:发生的时间
	* Param char* nRecord:发生的事件
	* @Return VOID:
	*/
	void addEventRecord(void* pEntity,char* nRecord,int nEventId,int nParam1,int nParam2,int nParam3,char* nParam4,char* nParam5);

	/*
	* Comments:设置帮派玥石
	* Param void * pEntity:
	* @Return void:
	*/
	void setGuildYs(void* pEntity,int value);

	/*
	* Comments:获取帮派贡献
	* Param void * pEntity:
	* @Return int:
	*/
	int getGuildGx(void* pEntity);

	/*
	* Comments:设置帮派贡献
	* Param void * pEntity:
	* @Return int:
	*/void setGuildGx(void* pEntity,int value);

	//获得在本行会的累计贡献
	int GetGuildTotalGx(void* pEntity);

	/*
	* Comments:获取帮派斗魂是否正在被召唤
	* Param void * pEntity:
	* @Return int:
	*/
	bool getGuildBossCallFlag(void* pEntity);

	void setCallGuildBossFlag(void* pEntity,bool value);

	bool getCallGuildBossFlag(void* pEntity);


	//设置行会镖车的句柄
	void setGuildDartHandle(void* pEntity,double bHandle);

	//获得行会镖车的句柄
	double getGuildDartHandle(void* pEntity);

	void setGuildFr(void* pEntity,int value);

	/*
	* Comments:获取帮派资金
	* Param void * pEntity:
	* @Return int:
	*/
	int getGuildCoin(void* pEntity);

	/*
	* Comments:获取当前帮派的繁荣度
	* Param void * pEntity:
	* @Return int:
	*/
	int getGuildFr(void* pEntity);

	/*
	* Comments: 获取帮派的名字
	* Param int nGuidID:帮派的名字
	* @Return char*:帮派的名字
	*/
	char* getGuildName(void* pEntity);

	/*
	* Comments: 获取帮派的id
	* Param void * pEntity: 玩家指针
	* @Return char*:帮派的id
	*/
	unsigned int getGuildId(void* pEntity);

	/* 
	* Comments: 获取行会指针
	* Param void * pEntity: 玩家指针
	* @Return void *:  行会指针
	*/
	void * getGuildPtr(void* pEntity);
	/*
	* Comments: 判断是否在帮派中
	* Param int nGuidID:帮派的名字
	* @Return char*:false 在帮派中
	*/
	bool bInGuild(void* pEntity);

	/*
	* Comments:
	* Param void * pEntity:
	* @Return void:
	*/
	void upGuildLevel(void* pEntity);

	/*
	* Comments:给所在的帮派发送消息
	* Param void * pEntity:
	guildPos:大于等于这个职位的人才可以收到 tagSocialMaskDef
	* Param const char * sMsg:
	* @Return void:
	*/
	void sendGuildMsg(void* pEntity,int guildPos,const char* sMsg,int nType = ttTipmsgWindow);

	/*
	* Comments:提醒帮派成员的客户端更新帮派信息
	* Param void * pEntity:
	* @Return void:
	*/
	void notifyUpdateGuildInfo(void* pEntity);

	/*
	* Comments:
	* Param void * pEntity:
	* @Return void:
	*/
	int getGuildSkillLevel(void* pEntity,int skillId);

	/*
	* Comments:设置帮派技能的等级 
	* @Return void:
	*/
	void setGuildSkillLevel(void* pEntity,int skillId,int skillLevel);

		/*
	* Comments:提醒帮派成员的客户端更新帮派信息
	* Param INT_PTR sType: 类型
	* Param INT_PTR sCount: 需要的量
	* Param bool sConsume: 是否需要扣除
	* @Return void:
	*/
	bool canUpGuildSkillLevel(void* pEntity,int sType,int sCount,bool sConsume);
	
	/*
	* Comments:扣除帮派升级所需要的
	* Param INT_PTR sType: 类型
	* Param INT_PTR sCount: 需要的量
	* Param bool sConsume: 是否需要扣除
	* @Return void:
	*/
	bool decountUpGuildSkill(void* pEntity,int sType,int sCount,bool sConsume);

	/*
	* Comments:扣除帮派升级所需要的
	* Param int skillId: 技能id
	* Param int skillLevel: 技能等级
	* @Return void:
	*/
	void saveDbGuildSkill(void* pEntity,int skillId,int skillLevel);

	/*
	* Comments:发送帮派更改的升级的帮派技能给所有成员
	* @Return VOID:
	*/
	void sendGuildSkillToAllMember(void* pEntity,int skillId,int skillLevel);

	/*
	* Comments: 刷新玩家的观察列表
	* @Param void* pEntity: 玩家对象指针
	* @Return bool: 成功返回true；失败返回false
	* @Remark:
	*/
	bool refreshActorObserveList(void* pEntity);

	/*
	* Comments:是否是第1次登陆游戏
	* Param void * pEntity:玩家的指针
	* @Return bool:如果是第1次登陆返回true，否则返回false
	*/
	bool isFirstLogin(void * pEntity);

	/*
	* Comments:下发玩家已经学习的技能
	* Param void * pEntity:玩家的指针
	* @Return void:
	*/
	void sendLearnSkill(void * pEntity);

	/*
	* Comments:使客户端可以显示一个剩余时间的倒计时
	* Param void * pEntity:
	* Param int nTime:单位是秒
	* @Return void:
	*/
	void sendSceneTime(void* pEntity, int nTime);

	/*
	* Comments:扩展玩家的背包
	* Param void * pEntity:玩家的指针
	* Param int nGridCount:扩展的格子的数目
	* @Return bool:如果成功的扩展就返回true，否则返回false,tipmsg由引擎负责发送
	*/
	bool enlargeBag(void *pEntity, int nGridCount);

	/*
	* Comments:发送私聊信息
	* Param void * pEntity:本人
	* Param void * pActor:对方玩家指针
	* Param const char * sMsg:信息内容
	* @Return void:
	*/
	void sendPrivateChatMsg(void *pEntity,void* pActor,const char* sMsg);

	/*
	* Comments:播放全屏特效
	* Param void * pEntity:
	* Param int nEffId:特效id
	* Param int nSec:持续时间
	* @Return void:
	*/
	void playScrEffect(void* pEntity, int nEffId,int nSec, bool toWorld = false, int nLevel = 0);

	/*
	* Comments:播放全屏特效，与消息18的区别是，这个是代码实现的特效
	* Param void * pEntity:
	* Param int nEffId:特效id
	* Param int nSec:持续时间
	* @Return void:
	*/
	void playScrEffectCode(void* pEntity, int nEffId,int nSec,int x=0,int y=0);

	/* 
	* Comments:删除特效
	* Param void * pEntity:
	* Param int nEffId:
	* @Return void:
	*/
	void delEffectCode(void * pEntity, int nEffId);

	/*
	* Comments:播放全屏场景特效
	* Param void * pEntity:
	* Param int nEffId:特效id
	* Param int nSec:持续时间
	* Param int nSceneId:场景id
	* @Return void:
	*/
	void playScrSceneEffectCode(void* pEntity, int nEffId,int nSec,int nSceneId,int x=0,int y=0);

	/*
	* Comments:设置玩家某个状态
	* Param void * pEntity:
	* Param int state:
	* @Return void:
	*/
	void addState(void* pEntity,int state);

	/*
	* Comments:去掉某个状态
	* Param void * pEntity:
	* Param int state:
	* @Return void:
	*/
	void removeState(void* pEntity,int state);

	/*
	* Comments:消耗银两(绑定的，非绑定的都可以)
	* Param double nCount:消耗的数目，是正数，比如消耗100银两就是传入100
	* Param int nlogId:日志的od
	* Param LPCTSTR comment:日志里的备注
	* Param bool needLog: 是否需要日志
	* Param bool bindFirst: true表示绑定的优先消耗，否则表示非绑定的优先消耗
	* @Return bool:成功消耗返回true，否则不够的话不会扣除并返回false
	*/
	bool  consumeCoin(void* pEntity,double nCount,int nlogId, char* comment="script",bool needLog=true,bool bindFirst =true);


	/*
	* Comments: 世界广播消息
	* Param void * packet: 数据包CActorPacket对象指针
	* Param int nLevel: 广播的玩家最低限制等级
	* @Return void:
	* @Remark:
	*/
	void worldBroadCastMsg(void* packet, int nLevel = 0, int nCircle = 0);

	/*
	* Comments: 本战区广播
	* Param void * packet: 数据包CActorPacket对象指针
	* Param int nLevel: 广播的玩家最低限制等级
	* Param bool bCsFlag: 是否在跨服服务器广播 true广播 false不广播
	* @Return void:
	* @Remark:
	*/
	void battleGroupBroadCastMsgCs(void* packet, int nLevel = 0, bool bCsFlag = true);


	/*
	* Comments: 获取玩家的一些杂项数据
	* Param void * pEntity: 玩家对象指针
	* Param enMiscDataType mdType:杂项数据类型
	* @Return int: 返回对应类型的数据值
	* @Remark:
	*/
	int getMiscData(void* pEntity, enMiscDataType mdType);


	/*
	* Comments: 设置玩家的一些杂项数据
	* Param void * pEntity: 玩家对象指针
	* Param enMiscDataType mdType: 杂项数据类型
	* Param int nVal: 值
	* @Return void:
	* @Remark:
	*/
	void setMiscData(void* pEntity, int mdType, int nVal);

	/*
	* Comments:设置npc的执行idle的间隔时间
	* Param void * pEntity:实体指针，必须是npc
	* Param int nTime:间隔时间，最小是30000（30秒），单位毫秒
	* @Return void:
	*/
	void setNpcIdleTime(void* pEntity,int nTime);

	/*
	* Comments:设置一个实体是否可见，通过添加实体的标记来实现
	* Param void * pEntity:实体的指针，可以是一切实体
	* Param bool canSee: 能看到的话是true，否则是false
	* @Return bool: 成功返回true，否则返回false
	*/
	bool setEntityVisibleFlag(void * pEntity, bool canSee =true);
	
	/* 
	* Comments: 获取物品过期时间
	* Param void * pEntity: 玩家对象指针
	* Param void * pUserItem: 用户物品指针
	* @Return unsigned int:  返回物品过期短时间
	*/
	unsigned int getItemLeftTime(void* pEntity, void* pUserItem);

	/*
	* Comments:邀请某人组队
	* Param void * pEntity:
	* Param void * pActor:
	* @Return void:
	*/
	void inviteJoinTeam(void* pEntity,void* pActor);

	/*
	* Comments:设置朝向
	* Param void * pEntity:
	* Param int nDir:
	* @Return void:
	*/
	void setDir(void* pEntity,int nDir);

	/*
	* Comments:给实体增加一个特效
	* Param void * pEntity:可以是玩家或者npc
	* Param int nEffId:特效id
	* Param int nEffType:特效类型
	* Param int nTime:持续时间，如果是负数，表示无限
	* @Return void:
	*/
	void addEffect(void* pEntity,int nEffId,int nEffType,int nTime,int nDelay=0,int nTotalCount = 0,int nDirCount = 0);

	
	/*
	* Comments:删除所有特效
	* Param void * pEntity:
	* @Return void:
	*/
	void delAllEffect(void* pEntity);

	/*
	* Comments:实体是否死亡
	* Param void * pEntity:
	* @Return bool:
	*/
	bool isDeath(void* pEntity);

	/*
	* Comments:模拟点击某个npc，激活对话
	* Param void * pEntity:玩家指针
	* Param void * pNpc:npc指针
	* Param const char * sFunc:函数名
	* @Return void:
	*/
	//void npcTalk(void* pEntity,void* pNpc,const char* sFunc="");

	/*
	* Comments:根据玩家所在的场景查找指定的npc对话
	* Param void * pEntity:
	* Param char * szName:
	* @Return void*:
	*/
	//void npcTalkByName( void* pEntity,char* szName,const char* sFunc="" );

	void showNpcShop(void* pEntity, int nType, int nTab);

	/*
	* Comments: 添加场景特效
	* Param void * pEntity:玩家的指针
	* Param int nEffId:特效的id
	* Param int nEffType:特效的类型
	* Param int nTime:特效的持续时间，单位毫秒
	* Param int nDelay:延迟生效时间，单位毫秒
	* Param int nRelX:相对于玩家的坐标x
	* Param int nRelY:相对于玩家的坐标y
	* @Return void:
	*/
	void addSceneEffect(void* pEntity,int nEffId,int nEffType,int nTime ,int nDelay=0,int nRelX=0,int nRelY=0,int nTotalCount = 0, int nDirCount = 0);

	/*
	* Comments: 添加场景特效
	* Param void * pEntity:玩家的指针
	* Param int nEffId:特效的id
	* Param int nEffType:特效的类型
	* Param int nTime:特效的持续时间，单位毫秒
	* Param int nDelay:延迟生效时间，单位毫秒
	* Param int nRelX:特效坐标x
	* Param int nRelY:特效坐标y
	* @Return void:
	*/
	void broadSceneEffect(void* pEntity,int nEffId,int nEffType,int nTime ,int nDelay=0,int nRelX=0,int nRelY=0,int nTotalCount = 0, int nDirCount = 0);

	/*
	* Comments: 判断玩家是否存在某个状态
	* Param void* pEntity:
	* Param int state:
	* @Return bool:
	* @Remark:
	*/
	bool hasState(void* pEntity, int state);

	/*
	* Comments: 获取GM等级
	* Param void * pEntity: 玩家对象指针
	* @Return int:
	* @Remark:
	*/
	int getGMLevel(void* pEntity);

	/// 根据角色指针获取角色id
	unsigned int getActorId(void* pEntity);
	/*
	* Comments:获取玩家目标对象ID
	* Param void * pEntity:玩家对象指针
	* @Return unsigned int:
	*/
	unsigned int getTargetId( void* pEntity);

	/*
	* Comments:移动到实体附近
	* Param void * pEntity:玩家
	* Param void * targetEntity:目标实体
	* @Return boo:
	*/
	bool moveToEntity(void *pEntity, void* targetEntity);


	/* 
	* Comments: 设置角色头衔
	* Param void *: 角色指针
	* Param int: 头衔ID
	* Param bool: true为添加头衔 false为删除头衔
	* @Return bool:  成功则返回true
	*/
	bool setTopTitle(void *pEntity, int nTitleId, bool bFlag = true);
	/*
	* Comments: 请求传送到目标服务器
	* Param void * pEntity:
	* Param int nDestServerId:
	* @Return void:
	* @Remark:
	*/
	void reqTransmitTo(void *pEntity, int nDestServerId);

	/*
	* Comments: 请求传送到公共服务器
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	void reqTransmitToCommonServer(void *pEntity);

	/*
	* Comments: 请求传送到原来服务器
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	void reqTransmitToRawServer(void *pEntity);

	/*
	* Comments: 判断玩家是否在原始服务器上
	* Param void * pEntity:
	* @Return bool: 如果玩家在原始服务器上，返回true；否则返回false
	* @Remark:
	*/
	bool isInRawServer(void *pEntity);

	/*
	* Comments: 获取角色的原始服务器ID
	* Param void * pEntity:
	* @Return int:
	* @Remark:
	*/
	int getActorRawServerId(void *pEntity);

	/*
	* Comments:更改行会资金
	* Param void * pEntity:玩家指针
	* Param int value:行金资金的增量/减量
	* @Return void:
	*/
	void changeGuildCoin(void *pEntity,int value, int nLogId = 0,const char* pStr = "");
	
	/*
	* Comments:设置玩家转数
	* Param void * pEntity:
	* Param int nCircle:转数
	* @Return bool:
	*/
	bool setActorCircle(void* pEntity, int nCircle);

	/*
	* Comments:查看离线玩家的信息 优先使用actorid 如果actorid==0 则使用name
	* Param char * name:玩家的名字
	* Param unsigned int nActorId:玩家的actorid 
	* Param bool loadWhenNotExist: 不存在是从DB装载
	* Param unsigned int nShowType:窗口类型 0 排行榜窗口消息 1 独立窗口消息
	* @Return int:
	*/
	void viewOffLineActor(void *pEntity, unsigned nActorId, char *sName = "", bool loadWhenNotExist = true, unsigned int nShowType = 1);

	//查看离线玩家英雄的信息
	void viewOffLineActorHero(void *pEntity, unsigned nActorId, unsigned nHeroId, bool loadWhenNotExist = true, unsigned int nShowType = 1);

	//下发推荐好友列表 nValue 等级差 nMaxNum 最大人数
	//void sendCanAddFriendList(void *pEntity, int nLevel1, int nLevel2,int nMaxNum);

	/*
	* Comments:改变自己视野中实体的模型
	* Param void * pEntity:实体指针
	* Param int: nRadius:半径范围
	* Param int nModelId:新的模型id
	* @Return void:
	*/
	void changeEntityModel( void * pEntity, int nRadius, int nModelId);

	/*
	* Comments:改变自己视野中实体的SHOWNAME
	* Param void * pEntity:实体指针
	* Param int: nRadius:半径范围
	* Param char *sName:新的NAME
	* @Return void:
	*/
	void changeShowName( void * pEntity, int nRadius, char *sName);

	void resetShowName( void * pEntity );
	/*
	* Comments:检测角色等级 转数
	* Param void * pEntity:实体指针
	* Param int nLevel:限制等级
	* Param int nCircle:限制转数
	* Param int bCircleOnly	:	当大于转生限制时，是否只判断转生
	* @Return void:
	*/
	bool checkActorLevel(void *pEntity, int nLevel, int nCircle = 0, bool bCircleOnly=false);
	
	/*
	* Comments:检测角色增加达到的最大等级+转数
	*/
	bool checkActorMaxCircleAndLevel(void *pEntity, int nLevel, int nCircle);
	/*
	* Comments: 更新玩家名字颜色给附近玩家
	* @Return void:
	* @Remark: 
	*/
	void updateActorName(void *pEntity);
	//检查系统开启等级(对应levelconfig)
	bool checkOpenLevel(void *pEntity, int nLevelConfigId);


	/*
	* Comments:获取Buff剩余时间
	* Param void * pEntity:玩家指针
	* Param int nBuffType:Buff类型
	* @Return int:返回剩余时间 
	*/
	int getBuffRemainTime( void * pEntity, int nBuffType, int nBuffGroup = -1);

	//通过id取buff剩余时间
	int getBuffRemainTimeById( void * pEntity, int nBuffId );

	/*
	* Comments:能否添加奖励
	* Param void * pEntity:玩家指针
	* Param int nTpe:奖励类型
	* Param int nId:id
	* Param int nCount:count
	* Param int nQuality:
	* Param int nStrong:
	* Param int nBindFlag:
	* Param bool boNotice:出错时是否提示信息，默认为提示
	* @Return bool:成功返回true
	*/
	bool canGiveAward(void *pEntity, int nTpe, int nId, double nCount, int nQuality,int nStrong,int nBindFlag,bool boNotice = true);

	/*
	* Comments:创建单人难伍
	* Param VOID * pEntity:玩家指针
	* @Return bool:成功返回true
	*/
	bool createTeam(void *pEntity);

	/*
	* Comments:自动 寻路到npc
	* Param void * pEntity:玩家的指针
	* Param int nSceneId:场景id
	* Param char * sNpcName:npc名字
	* @Return int:成功返回0，角色不存在或不是人物返回-1
	*/
	int autoFindPathToNpc( void * pEntity, int nSceneId, char * sNpcName);

	/*
	* Comments:获取存在某关系的一个玩家id
	* Param void * pEntity:玩家指针
	* Param int nType:任务类型
	* @Return bool:成功返回true
	*/
	unsigned int getSocialRelationId(void *pEntity, int nType);

	/*
	* Comments:获取当前关系的人数
	* Param void * pEntity:玩家指针
	* Param int nType:任务类型
	* @Return bool:成功返回true
	*/
	int getSocialCurNum(void *pEntity, int nType);

	/*
	* Comments:获取当前关系玩家名称
	* Param void * pEntity:玩家指针
	* Param int nType:nActorId 对方的id
	* @Return bool:成功返回true
	*/
	char* getSocialActorName(void *pEntity, unsigned int nActorId);

	/*
	* Comments:获取师徒的亲密度
	* Param void * pEntity:玩家指针
	* Param int nType:nActorId 对方的id
	* @Return bool:成功返回true
	*/
	//int getMasterIntimacy(void *pEntity, unsigned int nActorId,int nType);

	/*
	* Comments:师父领取徒弟出师奖励
	* Param void * pEntity:玩家指针
	* Param int nType:nActorId 对方的id
	* @Return bool:成功返回true
	*/
	//void graduationGiveMasterWard(void *pEntity,unsigned int nActorId);

	/*
	* Comments:升级给奖励提示	nActorId师父的id nIndex第一个奖励
	* Param void * pEntity:玩家指针
	* Param int nType:nActorId 对方的id
	* @Return bool:成功返回true
	*/
	//void giveUpLevelAwardTips(void *pEntity,unsigned int nActorId,int nIndex);

	/*
	* Comments: 发送给他的同门 出师提示
	* @Return bool: 如果设置了就返回true,否则返回false
	*/
	//void sendGrauateToMate(void *pEntity,unsigned int nActorId,int nLevel);

	//寻径消息 
	void sendMoveToMsg(void *pEntity, char *sSceneName, int nX, int nY, char *sTitle, char *sTips);


	//捐献帮派资金
	//void donateGuildCoin(void *pEntity,int nItemId,int nItemNum);

	/*
	* Comments: 发言
	* Param void * pEntity: 发言者实体指针
	* Param int nChannleID: 聊天频道的ID定义 enum tagChannelID
	* Param const char * pMsg: 发言内容
	* Param bool boSave:是否保存到日志
	* @Return void:
	*/
	void sendChat(void *pEntity, int nChannleID, char *pMsg, bool boSave);

	/*
	* Comments: 转换职业
	* Param int nVocation: 职业ID
	* @Return void:
	*/
	void changeVocation(void *pEntity, int nVocation);

	/*
	* Comments: 转换职业
	* Param int nVocation: 职业ID
	* @Return void:
	*/
	void changeSex(void *pEntity, int nSex);

	/*
	* Comments:设置玩家满怒气
	* Param void * pEntity:玩家指针
	* @Return bool:成功返回true
	*/
	bool setFullAnger( void * pEntity);

	/* 
	* Comments: 增加、删除玩家的社会关系
	* Param void * pEntity: 
	* Param int bIndex: 1-好友，4-仇人，5-师傅，6-徒弟
	* Param int bType: 0-增加，1-删除
	* Param int nActorId: 对方玩家ID
	* Param char * sName: 对方玩家Name
	* @Return void:  
	*/
	//void AddSocialRelation(void* pEntity, int nIndex, int nType, unsigned int nActorId, char* sName);

	/* 
	* Comments: 判断两个玩家之间的社会关系
	* 即 pEntity2 是 pEntity1的XXX
	* Param void * pEntity1: 
	* Param void * pEntity2: 
	* Param int nType: 0-好友，1-仇敌，2-师傅
	* @Return bool:  
	*/
	bool HasSocialRelation(void* pEntity1, void* pEntity2, int nType);

	/* 
	* Comments:	获取名称颜色
	* Param void * pEntity: 
	* @Return void:   0-白名，1-黄名，2-褐名，3-红名
	*/
	int GetNameColorData(void * pEntity);

	/*
	* Comments:能否传送到目标场景，主要判断自身状态能否传送
	* Param void * pEntity:玩家指针
	* Param int nSceneId:场景id，
	* Param LPCSTR sSceneName:场景名字，二选 一
	* @Return bool:传送成功返回true
	*/
	bool canTelport(void * pEntity, int nSceneId, char * sSceneName = NULL);



	/*
	* Comments:清空装备的锋利值
	* Param void * pEntity:
	* Param bool boCast:是否下发到客户端
	* @Return void:
	*/
	void clearEquipSharp(void *pEntity, bool boCast = true);



	//设置工资
	int setSalary(void *pEntity, int nNowValue, int nLastValue); //TO DELETE

	//获取工资
	int getSalary(void *pEntity, int &nNowValue, int &nLastValue);

	//判断是否正在交易
	bool isDealing(void *pEntity);

	//查询玩家是否存在
	void queryActorName(void * pEntity, const char * sName);

	

	//行会贡献有改变
	void changeGuildGx(void * pEntity,int nGx);

	//设置上香剩余次数
	void setShxDoneTimes(void * pEntity,int nTimes);

	//获取上香剩余次数
	int getShxDoneTimes(void * pEntity);

	//获取增加的上香次数
	int getShxAddTimes(void * pEntity);

	//设置增加上香次数
	void changeShxAddTimes(void * pEntity,int nTimes);

	//获取当天的贡献值
	int getTodayGxValue(void * pEntity);

	//设置已探险的次数
	void changeExploreTimes(void * pEntity,int nTimes);

	//获取已探险的次数
	int getExploreTimes(void * pEntity);

	//获取探险获得物品id
	int getExploreItemId(void * pEntity);

	//设置探险获得的物品id
	void setExploreItemId(void * pEntity,int nItemId);

	//获取已挑战的次数
	int getChallengeTimes(void * pEntity);

	//设置已挑战的次数
	void setChallengeTimes(void * pEntity,int nTimes);

	//设置添加的次数
	void setAddChallengeTimes(void * pEntity,int nTimes);

	//获取已添加的次数
	int getAddChallengeTimes(void * pEntity);

	//改变当天上香的贡献值
	void changeTodayGx(void * pEntity,int nValue);

	/*
	* Comments:获取当前对象目标handle
	* Param void * pEntity:
	* @Return unsigned int:
	*/
	double getTargetHandle(void * pEntity);

	//通过场景获取公共场景的npc坐标
	void getNpcPos(int nSceneId, int nNpcId, int &x, int &y);


	/// 获取最后那名玩家的信息，例如最新仇人
	/// @param pEntity 玩家指针
	/// @param nType 关系类型 @see FRIENDRELATION::nState @see enSocialRelation
	/// @return nActorId,sName
	//int GetLastSocialActorInfo(lua_State *L);
	//const char * GetLastSocialActorInfo(void * pEntity,  int nType, unsigned int & nActorId);

	//是否是队长
	bool isTeamLeader(void * pEntity);

	//重新调用main函数
	void backMain(void * pEntity, const char * sFunName);

	/* 
	* Comments:打开购买物品窗口
	* Param void * pEntity:玩家实体
	* Param double hNpc:执行的npc
	* Param int nItemId:
	* Param int nCount:
	* Param const char * sNotice:
	* Param const char * sCallFunc:
	* @Return bool:
	*/
	bool openBuyItemDialog(void * pEntity, double hNpc, int nItemId, int nCount, const char * sNotice, const char * sDesc, const char * sCallFunc);

	/* 
	* Comments:发送客户端个性特效
	* Param void * pEntity:
	* Param unsigned char nType:类型
	* Param int nParam:附加参数
	* @Return void:
	*/
	void sendCustomEffect(void * pEntity, unsigned char nType, int nParam);
	
	// 改变活跃度
	void changeActivity(void * pEntity, int nValue);

	
	//设置成就没完成，在合服中用
	void setAchieveUnFinished(void * pEntity, int nAchieveId);
	//秒完成成就，现用于活跃项
	void setAchieveInstantFinish(void* pEntity,int nAchieveId);
	//把狗收回
	void RemoveBattlePet(void * pEntity);
	
	bool addNewTitle(void * pEntity, int nId);
	bool delNewTitle(void * pEntity, int nId);
	bool isHaveNewTitle(void * pEntity, int nId);
	void setCurNewTitle(void * pEntity, int nId);


	//向场景玩家（包括自己）广播自己的名称颜色
	void BroadCastNameClrScene(void * pEntity);

	/* 
	* Comments:发送快捷栏上的消息（不消失）
	* Param void * pEntity:
	* @Return void:
	*/
	void SendShortCutMsg(void * pEntity, const char* szMsg);

	//设置战绩倍率
    void setExploitRate(void * pEntity, double nVal);

	//改变怪物的成长等级
	void setMonsterGrowLevel( void* pEntity, int nValue);
	
	int  GetGameSetting(void* pEntity, int nType, int nIdx); //获取游戏设置
    void setPosition(void *pEntity, int nSceneID,  int nPosx, int nPosy);// 设置坐标
	int getAppearByteType( void *pEntity, int nType );

	//设置进入副本前的场景地图id和位置
	void SetEnterFuBenMapPos(void *pEntity,int nSceneId,int x,int y);

	//提示货币不足
	//nType：货币类型
	void sendNotEnoughMoney(void* pEntity, int nType, int nCount);
	//提示奖励不足
	//nType:奖励类型
	//nCount:奖励数量（不是数量差）
	void sendAwardNotEnough(void* pEntity, unsigned char nType, unsigned short wItemId, int nCount);
	
	void setTitle(void* pEntity, char* sTitle);
	//设置复活点
	void SetRelivePoint(void* pEntity, int nPosX,int nPosY,int nSceneId, bool isFuben);
	//发送行会消息
	void sendGuildChannelMsg(void* pEntity, char* szMsg);

	//激活某个翅膀
	void setActivation(void* pEntity,int nModleId);	 
	//某个翅膀是否激活
	bool getActivation(void* pEntity,int nModleId);	

	bool isNearBySceneNpc(void *pEntity, int nSceneId, int nNpcId );

	//刷新角色外观
	void refreshFeature(void *pEntity);
	/*
	//接取赏金任务
	bool acceptRewardQuest(void *pEntity, int nQuestId);

	bool justFinishRewardQuest(void *pEntity, int nQuestId);
	//提交赏金任务，并领奖
	bool commitRewardQuest(void *pEntity, int nQuestId);
	*/

	unsigned int getDeadTimeRecent( void *pEntity );

	//获取角色登陆天数
	int getLoginDays( void *pEntity );

	/*
		将经验加给经验盒子
		return 实际增加的经验值
	*/
	unsigned int addExpToExpBox( void *pEntity, unsigned int nAddExp);
	
	//预先检测添加经验后能到多少级
	int  AddExpReachLevel(void *pEntity, unsigned int  nAddExp);

	//获取升级到下一等级经验
	double GetLevelExp(void *pEntity, int nLevel);


	//增加npc活动泡点经验值
	void addPaodianExp(void* pEntity, int paramA = 0, int paramB = 0, int paramC = 0, int nLog = 0);

	void addTypePaodianExp(void* pEntity, int nType, int nLogId);

	void updateActorEntityProp(void *pEntity);
	unsigned int  GetMonAttackedTime(void *pEntity);

	void setAchieveGiveAwards(void *pEntity,int nAchieveID);
	void setAchieveFinished(void *pEntity,int nAchieveID);

	//交换排名
	void swapCombatRank(void *pEntity, int nActorId, int nTagetId);

	//pk后的操作
	void ChallegeOverOp(void *pEntity, int nResult, int nActorId, char* nName, int nIsReal);

	//0点清除职业宗师相关信息
	void OnNewDayCombatClear(void *pEntity);

    //获取我的职业宗师当前排名
	int getMyCombatRank(void *pEntity);

	//获取全套装备总升星数
	int  getTotalStar(void *pEntity);

	//最大攻击
	unsigned int getMaxAttack(void *pEntity);
	//最大防御
	unsigned int getMaxDefence(void *pEntity);

	/*
	* Comments:设置怪物的强制归属
	* Param void *pEntity 怪物实体
	* Param nForceVesterId 强制归属玩家ID
	* @Return void:
	*/
	void SetForceVesterId(void *pEntity, unsigned int nForceVesterId);

	// 根据活动类型，获取运行中的活动id列表
	int getRunningActivityId(lua_State* L);

	//通过掉落组id抽奖，lua返回抽奖结果
	int ChouJiangByGroupId(lua_State* L);

	// 判断这个活动id对应的活动是否运行中
	bool isActivityRunning(void *pEntity, int nAtvId);

	// 判断这个活动类型的活动是否有进行中的
	bool isActivityTypeRunning(void *pEntity, int nAtvType);

	//判断玩家是否到达等级上限 
	bool isMaxLevel(void *pEntity);
	
	//发送一个活动数据
	void sendActivityData(void *pEntity,int nAtvId);

	//结束一个个人活动
	void closeOneActivity(void *pEntity,int nAtvId);

	//移除宠物
	void removePet(void *pEntity);

	//禁止某个频道聊天
	void setChatForbit(void* pEntity, int nChannalId, bool value);

	//获取宠物的主人
	void* getMaster(void *pEntity);

	//设置boss归属
	void SetBeLongBoss(void* pEntity, int nBossId, int nSceneId );
	void CancelBeLongBoss(void* pEntity);
	int getActorCreateTime(void* pEntity);

	//获取计数器
	int getStaticCount(void *pActor,int staticType);
	
	//计数器累加
	void addStaticCount(void *pActor,int staticType, int num) ;
	//计数器累加
	void setStaticCount(void *pActor,int staticType, int num) ;
	
	//是否拥有免费特权
	bool IsHasFreePrivilege(void *pActor);
	//是否拥有月卡
	bool IsHasMonthCard(void *pActor);
	//是否拥有大药月卡
	bool IsHasMedicineCard(void *pActor);
	//是否拥有永久卡
	bool IsHasForverCard(void *pActor);

	//获取色卡最高等级
	int GetMaxColorCardLevel(void* pEntity);
	//是否有橙卡 5 级
	bool IsHasOrangeCard(void *pEntity);
	//是否有紫卡 4 级
	bool IsHasPurpleCard(void *pEntity);
	//是否有蓝卡 3 级
	bool IsHasBlueCard(void *pEntity);
	//是否有绿卡 2 级
	bool IsHasGreenCard(void *pEntity);
	//是否有白卡 1 级
	bool IsHasWhiteCard(void *pEntity);



	//检测实体距离
	bool CheckDistanceByHandle(void *pActor,double handle, int distance );

	void SendActivityLog(void *pActor,int nAtvId,int nAtvType,int nStatu);
	//npc传送
	void SetNpcTeleport(void* pEntity, int nSceneId, int nJoinTime,int nMapGroup = 0);
	//通用限制
	bool checkCommonLimit(void *pEntity, int nLevel, int nCircle = 0, int nVip = 0, int nOffice = 0);

	bool checkKuangbao(void* pEntity, int nState = 0) ;
	int getGhostSumLv(void* pEntity);
	void SendJoinActivityLog(void *pActor,int nAtvId,int nIndex);
	//累计在线时间
	int getTotalOnlineTime(void* pEntity);

	void KickUserAccount(void* pEntity);

	// 设置玩家充值状态 0 ：初始值；1：已完成首充；2：已完成二充
	void setRechargeStatus(void* pEntity, int nStatus);
	int  getRechargeStatus(void* pEntity);
};


//tolua_end
