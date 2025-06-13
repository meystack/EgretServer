/*
	宠物相关的一些脚本的导出
*/


//tolua_begin
namespace Pet
{
	/*
	//宠物的属性的枚举
	enum tagPetProperty
	{
		enPetLevel =1,  //宠物的等级
		enPetStrong =2, //宠物的强化 
		enPetCircle =3, //宠物的转数
		enPetConfigId =4,  //宠物的配置表里的ID
		enPetExp =5,      //宠物的经验
		enPetLoyalty =6,  //宠物的忠诚度
		enPetMaxLoyalty =7, //宠物最大的忠诚度
		enBaseAttack =8,  //基础的攻击资质
		enBaseDefence =9, //基础的防御资质
		enBaseAgility =10,//基础的敏捷资质
		enBasePhysique =11, //基础的体魄资质
		enSmithAttack =12,   //洗出来的攻击资质
		enSmithDefence =13, //洗出来的防御资质
		enSmithAgility =14,//洗出来的敏捷资质
		enSmithPhysique =15,  //洗出来的体魄资质
		enPetScore =16,     //宠物的评分
		enPetAttackType =17, //宠物的攻击类型,1表示内功攻击，2表示外功攻击
		enPetIconId =18,    //宠物的图标ID
		enPetWashMergeRate=19,   //洗练的宠物的附体比例
		enPetQuality =20 ,   //宠物的品质
		enPetTotalMergeRate =21, //当前全部的附体资质
		enPetMaxMergeRate =22,  //当前最大的附体资质比例
		enPetSkillSlotCount = 23, //宠物技能的槽位的数目
		
	};
	*/

	//宠物增强的类型
	/*
	enum tagPetEnhanceType
	{
		etPetEnhanceStrong =1, //强化
		etPetEnhanceSmith =2, //洗资质
		etPetEnhanceCircle =3, //转生
		
	};
	*/

	/*
	* Comments: 添加宠物
	* Param void * pEntity:玩家的指针
	* Param int nPetConfigId:宠物配置表里的ID
	* Param int nCircle:宠物的转数，也就是品质
	* Param int nStrong:宠物的强化等级
	* Param int nLevel:宠物的初始化的等级
	* Param int nAttackType:攻击类型，1为内功，2为外功，其他的就随机
	* Param int nQuality:品质，如果-1表示使用默认的宠物配置里的品质
	* @Return int:成功返回大于0的宠物id
	*/
	int addPet(void* pEntity,int nPetConfigId, int nLevel =1, int nAiId = 0,int nLiveSecs = 0);

	/*
	* Comments:获取指定宠物的数量
	* Param void * pEntity:玩家指针
	* Param int nPetId:宠物id
	* @Return int:返回的数量
	*/
	int getPetCountByConfigId(void * pEntity, int nPetId);

	/*
	* Comments:通过宠物的ID获取宠物的名字
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* @Return char *:返回宠物的名字
	*/
	char * getPetName(void * pEntity ,int nPetId);

	/*
	* Comments:删除一个宠物
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的id
	* @Return bool:成功返回true,否则返回false
	*/
	bool delPetById(void * pEntity, int nPetId);
	
	/*
	* Comments:宠物的强化
	* Param void * pEntity: 玩家的指针
	* Param int nPertId:宠物的id
	* Param int nStrong:宠物的
	* @Return bool:成功返回true，否则返回false
	*/
	//bool setPetStrong(void *pEntity, int nPertId,int nStrong);

	/*
	* Comments:宠物洗资质
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* @Return bool:成功返回true，否则返回false
	*/
	//bool petSmith(void *pEntity, int nPetId);


	/*
	* Comments:宠物添加经验
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的id
	* Param int nExp:经验的数值，比如添加100经验就是要100
	* @Return int:成功添加的经验，如果没有添加进去会返回0
	*/
	//bool addExp(void *pEntity,int nPetId, int nExp);

	/*
	* Comments:宠物增加忠诚度
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* Param int nValue:忠诚度的数值
	* @Return bool:成功返回true，否则返回false
	*/
	//bool addLoyalty(void *pEntity,int nPetId, int nValue);

	/*
	* Comments:获取宠物的属性
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* Param int nPropId:宠物的属性的ID，在tagPetProperty里定义
	* @Return int:
	*/
	//int getPetProperty(void *pEntity,int nPetId, int nPropId);

	/*
	* Comments:判断一个玩家能否添加一个宠物
	* Param void * pEntity:玩家的指针
	* Param int nPetConfigId:宠物的配置ID
	* @Return bool:能够添加返回true，否则返回false
	*/
	//bool  canAddPet(void *pEntity, int nPetConfigId);

	/*
	* Comments:扩展宠物的槽位
	* Param void * pEntity:玩家的指针
	* Param int nGridCount:宠物的格子的数目
	* @Return bool:成功返回true，否则返回false
	*/
	//bool  enlargePetSlot(void *pEntity, int nGridCount);

	/*
	* Comments: 设置宠物的转数
	* Param void * pEntity:玩家的指针
	* Param int nPetID:宠物的id
	* Param int nCircle:宠物的转数
	* @Return bool: 成功返回true，返回false
	*/
	//bool setCircle(void *pEntity, int nPetID, int nCircle);

	/*
	* Comments:给宠物学习技能
	* Param void * pEntity:玩家的指针
	* Param int nPetID: 宠物的id
	* Param int nSkillID:技能的id
	* Param int nSlotId:技能的巢位的id,槽位是1,2,3,4,5,6,7
	* @Return bool:成功返回true,否则返回false
	*/
	//bool learnSkill(void *pEntity,int nPetID, int nSkillID,int nSlotId);

	
	/*
	* Comments:宠物的技能升级
	* Param void * pEntity:玩家的指针
	* Param int nPetID:宠物的ID
	* Param int nSkillID:玩家的
	* @Return bool:
	*/
	//bool skillLevelUp(void *pEntity,int nPetID, int nSkillID );
	
	/*
	* Comments: 获取玩家的一个宠物的一个槽位上学习的技能的id
	* Param void * pEntity: 玩家的指针
	* Param int nPet:宠物的id
	* Param int nSkillSlot: 技能的槽位的id，槽位是1,2,3,4,5,6,7
	* @Return int:如果存在技能返回技能的id,否则返回-1
	*/
	//int getLearnSkillId(void* pEntity,int nPet,int nSkillSlot);

	/*
	* Comments:遗忘一个技能
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的id
	* Param int nSkillId:技能的id
	* @Return bool:成功遗忘返回true，否则返回false
	*/
	//bool forgetSkill(void* pEntity,int nPetId,int nSkillId);

	/*
	* Comments:获取宠物技能的等级
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的id
	* Param int nSkillId:技能的ID
	* @Return int:当前宠物的技能列表里该技能的等级
	*/
	//int getSkillLevel(void* pEntity,int nPetId, int nSkillId);


	/*
	* Comments:获取放出来战斗中的宠物的id
	* Param void * pEntity:玩家的指针
	* @Return int:获取放出来的宠物的id，如果没有放出来的宠物就返回-1
	*/
	//int getBattlePetId(void * pEntity);

	/*
	* Comments:获取附体中的宠物的id
	* Param void * pEntity:玩家的指针
	* @Return int:获取附体的宠物的id，如果没有附体的宠物就返回-1
	*/
	//int getMergePetId(void * pEntity);


	/*
	* Comments:设置宠物的结果
	* Param void * pEntity:指针
	* Param int nPetId:宠物的id
	* Param int nOpId:操作码
		etPetEnhanceStrong =1, //强化
		etPetEnhanceSmith =2, //洗资质
		etPetEnhanceCircle =3, //转生
	* Param bool result:结果，成功为true，否则为
	* @Return void:
	*/
	//void sendPetOpResult(void * pEntity,int nPetId,int nOpId, bool result);

	/*
	* Comments:修改C宠物的攻击类型
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* @Return bool:成功返回true
	*/
	//bool changeAttackType(void *pEntity, int nPetId);

	/*
	* Comments: 宠物出战
	* Param void * pEntity:
	* Param int nPetId:
	* @Return void:
	* @Remark:
	*/
	//void setPetBattle(void* pEntity, int nPetId);

	/*
	* Comments:宠物换皮肤
	* Param void * pEntity:玩家的实体ID
	* Param int nPetId:宠物的ID
	* Param int configId:换完后的配置的ID
	* Param int nQuality:宠物的品质
	* @Return bool:成功返回true，否则返回false
	*/
	//bool setPetSkin(void* pEntity, int nPetId,int configId,int nQuality);

	/*
	* Comments:设置宠物的附体比率
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的ID
	* Param int nRate:附体的比率
	* @Return bool:成功返回true，否则返回false
	*/
	//bool setPetMergeRate(void* pEntity, int nPetId,int nRate);

	/*
	* Comments:扩大宠物的技能槽位
	* Param void * pEntity:玩家的指针
	* Param int nPetId:宠物的技能ID
	* @Return bool:成功返回true，否则返回false
	*/
	//bool enlargePetSkillSlot(void * pEntity, int nPetId);

};



//tolua_end