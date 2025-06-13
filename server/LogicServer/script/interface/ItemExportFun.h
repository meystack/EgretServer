/*
*装备强化，镶嵌等需要的导出接口
* 和玩家的接口比，这些接口只使用在一些特殊的场合	
*/

#pragma once 
//tolua_begin
namespace Item
{

	/*
	* Comments:设置物品的属性
	* Param void * pEntity:玩家的指针
	* Param void * pUserItem:物品的指针
	* Param int nPropertyID:属性id tagItemProperty 定义
		enum tagItemProperty
		{
		ipItemID , //物品的ID
		ipItemCount, //物品的数量
		ipItemStrong, //强化等级
		ipItemQuality,  //装备的品质
		ipItemActorLevel, //需要玩家的等级
		ipItemBind,    //物品的绑定标记
		ipItemType,    //物品的类型
		ipItemMaxProperty, 
		};
	* Param int nValue:新的数值,
	* @Return bool: 成功返回true，否则返回false
	*/
	bool setItemProperty(void * pEntity,  void* pUserItem,int nPropertyID,int nValue);
	
	/*
	* Comments: 获取物品的属性
	* Param void * pEntity:玩家的指针，这里
	* Param void * pItem:物品的指针
	* Param int propID:属性id  SystemParamDef.h 的tagItemProperty里定义的
		
	* Param int &nAuxRetValue: 辅助返回参数
	* @Return int:返回属性值
	*/
	int getItemProperty(void * pEntity, void * pItem, int propID,int &nAuxRetValue);
	
    /*
    * Comments: 通过物品的ID查找物品的属性
    * Param int nItemID: 物品的ID
    * Param int nPropID:  SystemParamDef.h 的tagItemProperty里定义的
    * @Return int:属性的值
    */
    int getItemPropertyById(int nItemID,int nPropID);

	/*
	* Comments: 获得物品的GUID
	* Param void * pItem:物品的指针
	* @Return long long:返回物品的guid
	*/
	double getItemGuid(void * pItem);


	/*
	* Comments:获取物品id
	* Param void * pItem:物品指针
	* @Return int:返回物品id,如果是0表示失败
	*/
	int getItemId(void * pItem);

	/*
	* Comments: 获取物品Guid字符串描述
	* Param void * pItem:
	* @Return char*:
	* @Remark: 注意此函数不是线程安全！
	*/
	char* getItemGuidStr(void *pItem);

	/*
	* Comments: 设置装备处理的结果（强化，打孔等操作）
	* Param void * pEntity:玩家的指针
	* Param void * pUserItem: 装备的指针
	* Param int nProceccType: 处理的类型
		//装备处理的一些操作的操作类型
		enum tagItemProcessType
		{
		itItemEnhance =1,          //装备强化
		itItemOpenHole =2,         //装备的打孔
		itItemDiamondEnlay =3,     //装备镶嵌宝石
		itItemDiamondRemove =4,    //装备宝石摘除
		itItemDiamondSynthetic =5, //宝石合成
		itItemBreakUp=6,           //装备的分解
		itItemInject=7,            //装备的注入
		};
	* Param bool bResult:成功用true,否则用false
	* Param bool bAutoOperate:是否终止自动操作 终止用true, 不终止用false
	* @Return void:
	*/
	void sendItemProcessResult(void * pEntity,  void* pUserItem,int nProceccType,bool bResult,bool bAutoOperate=false);
	


	/*
	* Comments:通知客户端处理一件装备需要的消耗(比如强化，镶嵌等)
	* Param void * pEntity:玩家的指针
	* Param void * pUserItem:装备的指针
	* Param int nProcessType: 处理的类型 tagItemProcessType里定义
	* Param int nItemID:需要消耗的物品的ID(强化石)
	* Param int nCount:需要消耗的物品的数量
	* Param int nMoneyType: 需要消耗的金钱的种类
	* Param int nMoneyCount:需要消耗的金钱的数量
	* Param int nProtectItemID: 保护物品ID
	* Param int nProtectCount: 保护物品数量
	* @Return void:
	*/
	void sendItemProcessConsume(void * pEntity,void * pUserItem, int nProcessType,int nItemID,int nCount, int nMoneyType,int nMoneyCount, int nProtectItemID=0, int nProtectCount=0);

	
	

	/*
	* Comments: 向客户端发送一件装备的处理的参数(这里要改为vector)
	* Param void * pEntity:实体ID
	* Param int nGetConfigParamID:获取ID,tagGetItemConfigParam 里定义的
	* Param int nParam1:参数
	* Param int nParam2:可选参数,可以不填
	* Param int nParam3:可选参数,可以不填
	* Param int nParam4:可选参数,可以不填
	* Param int nParam5:可选参数,可以不填
	* Param int nParam6:可选参数,可以不填
	* Param int nParam7:可选参数,可以不填
	* Param int nParam8:可选参数,可以不填
	* @Return void:
	*/
	//void sendItemConfig(void * pEntity,int nGetConfigParamID,int nParam1, int nParam2=-1,int nParam3=-1, int nParam4=-1,int nParam5=-1,int nParam6=-1,
	//	int nParam7=-1,int nParam8=-1);


	/*
	* Comments: 创建一个物品
	* Param int nItemID: 物品的ID
	* Param int nCount: 数量
	* Param int nStar: 强化
	* Param int nLostStar: 强化损失的星级
	* Param int nQuality: 品质
	* Param int nBind:绑定标记
	* Param int ntime: 物品创建时间，秒为单位。如果为0表示不限时间
    * Param int nQualityDataIndex: QualityDataIndex
	* @Return void *: 如果是空表示失败
	*/
	void * createItem(int nItemID,int nCount=1,int nStar=0, int nLostStar=0, int nInscriptLevel=0, int nAreaId=0, int nBind=0, int ntime=0);
	

	/*
	* Comments: 删除玩家背包里的一个物品
	* Param void * pEntity:玩家的指针
	* Param void * pItem: 物品的指针
	* Param int nCount:删除物品数量
	* Param char * sComment:  备注
	* Param int nLogID: 物品的日志号
	* Param bool bNeedFreeMemory:  是否需要释放内存
	* @Return int: 删除的数量
	*/
	int removeItem(void *pEntity,void *pItem,int nCount, char * sComment,int nLogID,bool bNeedFreeMemory =true);

	/*
	* Comments: 减少物品使用的次数(耐久)
	* Param void * pEntity:玩家的指针
	* Param void * pItem: 物品的指针
	* Param int nCount:删除物品耐久数量级 nCount*m_dwUseDurDrop
	* Param char * sComment:  备注
	* Param int nLogID: 物品的日志号
	* Param bool bNeedFreeMemory:  是否需要释放内存
	* @Return int: 减少的次数
	*/
	int removeItemDura(void *pEntity,void *pItem,int nCount = 1, char * sComment = "Script",int nLogID = 164,bool bNeedFreeMemory =true);

	/*
	* Comments: 删除玩家背包里指定指针同状态的其他物品
	* Param void * pEntity:玩家的指针
	* Param void * pItem: 物品的指针
	* Param int nCount:删除物品数量
	* Param char * sComment:  备注
	* Param int nLogID: 物品的日志号
	* Param bool bNeedFreeMemory:  是否需要释放内存
	* Param bool bReqSB:是否要判断强化与绑定条件
	* @Return int: 删除的数量
	*/
	int removeOtherItem(void *pEntity,void *pItem,int nCount, char * sComment,int nLogID,bool bNeedFreeMemory =true,bool bReqSB = false);
	
	/*
	* Comments: 删除玩家背包或者装备栏上的物品
	* Param void * pEntity:玩家对象指针
	* Param int nType: 物品类型
	* Param bool bIncEquipBar: 为true表明删除装备栏中的制定类型物品；否则只删除背包中的指定类型物品
	* Param char * sComment:  备注
	* Param int nLogID: 物品的日志号
	* @Return int:返回删除的物品数量
	* @Remark:
	*/
	int removeItemByType(void* pEntity, int nType, bool bIncEquipBar, char * sComment,int nLogID);

	/*
	* Comments: 删除玩家身上装备
	* Param void * pEntity: 玩家对象指针
	* Param void * pItem: 物品对象指针
	* Param const char * sComment: 备注
	* Param int nLogId: 日志Id
	* @Return int: 返回删除的物品数量
	* @Remark:
	*/
	int removeEquip(void* pEntity, void* pItem, const char* sComment, int nLogId, bool bIncHero=false);

	/*
	* Comments: 获取背包或者装备中的指定类型的物品
	* Param void * pEntity: 玩家对象指针
	* Param int nType: 装备类型
	* Param bool bIncEquipBar: 是否在装备栏搜索。如果为true，在背包中没找到的话就会在装备栏中搜索；否则不在装备栏搜索
	* Param bool bInEquipBar: 返回搜索到的位置。如果找到了，返回true表明是在装备栏，否则在背包栏
	* @Return void*: 返回搜索到的物品
	* @Remark:
	*/
	void* getItemByType(void* pEntity, int nType, bool bIncEquipBar, bool &bInEquipBar);

	/*
	* Comments: 添加物品
	* Param void * pEntity:玩家的指针
	* Param void * pItem:物品的指针
	* Param char * comment:记录日志用的描述
	* int nLogWay: 物品的获得/失去途径,在LogDef.h里定义的
	* int nItemCount: 物品的数量，为了支持商店里的回购功能，这里支持重新设定数量
	    如果使用-1,那么就不设置数量，使用原来的数量
	* @Return int: 返回添加好的个数
	*/
	int addItem(void *pEntity,void *pItem,char * comment,int nLogWay,int nItemCount =-1, int bNotice = 1);



	/*
	* Comments: 获取物品的链接
	* Param int nItemID: 物品的ID
	* Param void * pUserItem: 物品的指针，非装备=NULL就行；装备并需要显示动态数据这里不能为空
	* @Return char *:返回物品的链接
	*/
	const char* getItemLinkMsg( int nItemID,void* pUserItem = NULL);

	/*
	* Comments:获得奖励的描述，比如金币，道具将返回道具的名字（可带连接）
	* Param void * pEntity:玩家指针，这里没用
	* Param int nAwardType:奖励类型
	* Param int nAwardId: 奖励的id,只有道具有效
	* Param bool useChatLink:是否带连接，只有道具有效
	* Param void * pUserItem:道具的指针，
	* @Return char *
	*/
	const char * getAwardDesc(int nAwardType,int nAwardId=0, bool useChatLink=false,void *pUserItem =NULL);

	/*
	* Comments: 返回玩家当前身上有几件装备
	* @Return int:装备的数量
	*/
	int getEquipCount(void * pEntity);

	/*
	* Comments: 通过装备的排序返回身上装备的指针
	* Param int nSortID:排序号，比如玩家身上有6件装备，使用nSortID在[1,6]的范围里有效
	* @Return void*:返回装备的指针
	*/
	void* getEquipBySortID(void * pEntity,int nSortID);

	/*
	* Comments: 玩家背包里的物品的数量
	* @Return int:返回物品的数量
	*/
	int getBagItemCount(void * pEntity);


	/*
	* Comments:获取背包里的物品的指针
	* Param int nSortID:排序号，比如玩家背包里有6件装备，那么[1,6有效]
	* @Return void *: 返回背包里的物品的指针
	*/
	void * getBagItemBySortID(void * pEntity,int nSortID);

	/*
	* Comments: 通过背包物品的guid返回物品的指针
	* Param void * pEntity: 玩家的指针
	* Param long long lItemGuid:物品的guid
	* @Return void *:物品的指针
	*/
	void * getBagItemPtrByGuid(void * pEntity,double lItemGuid);

	/*
	* Comments: 通过物品id返回背包物品的指针
	* Param void * pEntity: 玩家的指针
	* Param long long lItemGuid:物品的guid
	* @Return void *:物品的指针
	*/
	void * getBagItemPtrById(void * pEntity,int lItemId, int nQuality /* = -1 */, int nStrong /* = -1 */);

	/*
	* Comments: 通过身上装备物品的guid返回装备的指针
	* Param void * pEntity: 玩家的指针
	* Param long long lItemGuid:身上装备的guid
	* @Return void *:身上装备的指针
	*/
	void * getEquipPtrByGuid(void *pEntity, double lItemGuid);


	//查找物品 
	//nItemPos:	1背包 2玩家装备 3英雄装备
	//nHeroId:如果是英雄装备则为英雄POS
	//nEntityId: <=0则遍历查找 如果查找到则赋值nItemPos与nHeroId 如果>0用作heroId
	void * getEquipItemPtr(void *pEntity, double lItemGuid, int &nItemPos, int &nHeroId, int nEntityId=-1);

	/*
	* Comments:从装备栏中取得指定类型物品
	* Param void * pEntity:玩家指针
	* Param int nType:装备类型
	* @Return void *:返回装备的指针
	*/
	void * getEquipByType(void * pEntity, int nType);

	/*
	* Comments: 回收装备的内存指针,这个只能用于玩家商店卖东西的回购功能，谨慎使用
	* Param void * pItem: 物品的指针
	* @Return void:
	*/
	void releaseItemPtr(void * pItem);



	/*
	***牵扯到单个物品的判断 只需要判断当前背包最小剩余格子数量
	* Comments: 获得一个玩家背包空余的格子数量
	* Param void * pEntity:玩家的指针
	* Param int type:背包类型 --可以通过配置packageType这个字段获得
	* @Return int:返回背包空余的格子数量
	*/
	int getBagEmptyGridCount(void *pEntity, int type);

	/*
	***牵扯到多物品的判断 需要判断所有背包最小剩余格子数量
	* Comments: 获得玩家所有背包最小空余的格子数量
	* Param void * pEntity:玩家的指针
	* @Return int:返回背包空余的格子数量
	*/
	int getAllBagMinEmptyGridCount(void *pEntity);

	/* 
	*** 根据背包类型检测空余格子是否足够，然后给出不同提示
	*nType: enum BagType 类型
	*/
	bool bagIsEnough(void *pEntity,int nType);

	/*
	* Comments: 添加该物品需要多少个格子
	* Param void * pEntity:玩家的指针
	* Param int nItemID: 物品的id
	* Param int nItemCount: 物品的数量
	* Param int nQuality: 物品的品质
	* Param int nStrong:物品的强化
	* Param int nBind: 物品的绑定标记
	* Param int nParam: 辅助参数 可以用于物品指针
	* @Return int: 返回需要多少个格子才能容纳这个物品
	  如果是添加批量物品，先
	  local nGridCount= Item.getBagEmptyGridCount(args) --获得有多少个格子
	  对每个物品遍历一次，剩余的格子数量少一个，如果总格子没有了，就表示不能添加了
	*/
	int getAddItemNeedGridCount(void *pEntity,int nItemID, int nItemCount,int nQuality=0,
		int nStrong =0, int nBind =-1, int nParam = 0);

	/*
	* Comments:通过物品的id获取物品的静态属性
	* Param int nItemId:物品的id
	* @param int nAttrType:物品的静态属性id，enum tagGameAttributeType
	* @Return -1 没有此属性，否则返回属性值
	*/
	int getItemStaticAttrById(int nItemId, int nAttrType);

	/*
	* Comments:通过物品的id获取物品的名字
	* Param int nItemId:物品的id
	* @Return char *:物品的名字
	*/
	char * getItemName(int nItemId );
	
	/*
	* Comments:通过物品的id获取物品的名字
	* Param int nItemId:物品的id
	* @Return int:物品的背包属性
	*/
	int getItemPackageTyp(int nItemId );

	
	/*
	* Comments:添加一个卖给商店的物品，如果没有回购，玩家下线将回收
	* Param void * pEntity:玩家的指针
	* Param void * itemPtr: 物品的指针
	* Param int nItemCount: 物品的数量(因为物品的数量在删除的时候会被清0，这里把物品的数量放这里保存)
	* @Return unsigned bool:成功返回true，否则返回false
	*/
	bool addSellItem(void * pEntity, void * itemPtr, int nItemCount);

	
	/*
	* Comments:删除一个卖给商店的指针，回购完了调用
	* Param void * pEntity:玩家的指针
	* Param void * pItem:卖给商店的物品的指针，回购以后要把这个删除
	* @Return bool:成功返回true
	*/
	bool delSellItem(void * pEntity, void * pItem);

	
	/*
	* Comments:通过guid获得卖给商店的物品的指针
	* Param void * pEntity:玩家的指针
	* Param  double lItemGuid:卖给商店物品物品的guid
	* @Return void *: 物品的指针
	*/
	void *  getSellItem(void * pEntity, double lItemGuid);

	/*
	* Comments:物品是否被精锻过
	* Param void * itemPtr:
	* @Return bool:
	*/
	bool isForgeItem(void* itemPtr);

	/*
	* Comments:是否有装备强化达到了指定的等级
	* Param void * pEntity:
	* Param int nLevel:
	* @Return bool:
	*/
	bool isStrongLevel(void* pEntity,int nLevel);

	/*
	* Comments:是否有精锻过的物品，一般任务系统使用到
	* @Return bool:
	*/
	//bool hasForgeItem(void* pEntity);

	/*
	* Comments:背包获取耐久度满的指定的物品
	* Param void * pEntity:
	* Param int nItemId:
	* @Return void*:返回null表示没有
	*/
	void* getDuraFullItemFromBag(void* pEntity,int nItemId);

	/*
	* Comments:根据物品id获取所属的套装id
	* Param int nItemId:
	* @Return int:
	*/
	int getSuitIdByItemId(int nItemId);

	/*
	* Comments:判断是否装备齐全某个套装的所有装备
	* Param void * pEntity:
	* Param int nSuit:套装id
	* Param int nCount:要收集的套装的数量
	* @Return bool:
	*/
	bool isGetAllSuitEquip(void* pEntity,int nSuit,int nCount);

	/*
	* Comments: 装备物品
	* Param void * pEntity:
	* Param void * pUserItem:
	* @Return bool:
	* @Remark: 
	*/
	bool equipItem(void* pEntity, void* pUserItem, int nHeroId = 0, bool isBagEquip = true);
	//脱下装备
	bool takeOffItem(void* pEntity, void* pUserItem, int nHeroId, bool checkGrid);

	/*
	* Comments: 清除背包
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	void clearBag(void* pEntity);

	//装备是否禁止修理 --不用了
	//bool isDenyRepair(int nItemId);

	/*
	* Comments: 根据物品指针获取物品在背包的索引
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	int getBagItemIndex(void* pEntity,void * itemPtr);

	/*
	* Comments: 根据在背包的索引获取物品的指针
	* Param void * pEntity:
	* @Return void:
	* @Remark:
	*/
	void* getItemPtrByIndex(void* pEntity,int nIndex);

	/*
	* Comments:初始化装备极品属性
	* Param void * pEntity:玩家指针
	* Param void * pItemPtr:物品指针
	* @Return bool:成功返回true
	*/
	//bool initEquipBaseAttr(void * pEntity, void *pItemPtr, int nQualityDataIndex);

	bool isGuildMeltingItem(void* pItem);


	//掉落组掉落物品，可设置掉落捡取时间，默认2min, nDropTips = 0
	bool drop_item_in_random_area_byGroupID(int sceneId, int nPosX,int nPosY,int dropGroupId,int pick_time = 120, int nDropTips = 0);


	//是否装备
	bool  isEquipment(void *pItemPtr);
	bool isEquipmentById(int nItemId);
    //获取背包里同样ID最高星的装备
	void* getItemPtrHighestStar(void  *pEntity, int  nItemID);

	//从属性库中给装备随机生成属性
	void generateItemAttrsInSmith( void * pEntity,  void *pItemPtr, int nSmithId, int nAttrNum, unsigned char nLockType1, unsigned char nLockType2, 
		unsigned char nLockType3, int nFlag, unsigned char nJob = 0 );

	//从某个属性库中随机抽取一条属性
	void generateOneAttrInSmith( int nSmithId, unsigned char& nAttrType, int& nAttrValue, unsigned char nLockType=enSmithLockType_No);

	bool hasItemAttrs( void * pEntity, void *pItemPtr, int nFlag );
	//转移装备属性
	bool transferItemAttrs( void * pEntity, void *pSrcItemPtr, void *pDesItemPtr, int nFlag );
}
//tolua_end
