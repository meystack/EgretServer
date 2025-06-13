#pragma once
class CAnimal;

//这个类是技能的目标选择条件的类，为了不把技能的类扩大很多，把目标选择方面的
//代码放在这里，主要是判断技能是否生效

class CSkillCondition
{
public:
	//目标选择的条件
	typedef   bool  (CSkillCondition::*TargetSelCondition)(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value ); 

	//技能的训练和使用的条件
	typedef   INT_PTR  (CSkillCondition::*SkillUseTrainCondition)(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 

	
	/*
	* Comments: 检测条件是否满足
	* Param CAnimal * pSrcEntity: 施法者
	* Param CAnimal * pTargetEntity:被施法者
	* Param TARGETSELCONDITION & value: 技能的配置值
	* bool needConsume: 是否要消耗，如果是，那么就消耗，否则不消耗
	* @Return bool: 如果能够选中就返回true,否则返回false
	*/
	bool Check( CAnimal * pSrcEntity,CAnimal *pTargetEntity,TARGETSELCONDITION &value);


	/*
	* Comments:检测玩家的使用和训练的条件
	* Param CActor * pActor:玩家的指针
	* Param SKILLTRAINSPELLCONDITION & contition: 条件
	* @Return INT_PTR:返回错误码
	*/
	INT_PTR CheckUseTrainContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

	/*
	* Comments:检测玩家的技能施法条件
	* Param CActor * pActor:玩家的指针
	* Param SKILLTRAINSPELLCONDITION & contition: 条件
	* @Return INT_PTR:返回错误码
	*/
	INT_PTR CheckSpellContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	/*
	* Comments:检测玩家的技能升级条件
	* Param CActor * pActor:玩家的指针
	* Param SKILLTRAINSPELLCONDITION & contition: 条件
	* @Return INT_PTR:返回错误码
	*/
	INT_PTR CheckUpGradeContion(CAnimal * pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//错误的id
	inline bool IsError(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value)
	{
		return false;
	}

	
	//目标为敌人
	inline bool IsEnermy(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标为友方
	inline bool IsFriendly(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标为自己
	inline bool IsSelf(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标为队伍成员
	inline bool IsTeammate(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标为怪物
	inline bool IsMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标为玩家
	inline bool IsActor(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//标等级 >= value,
	inline bool LevelAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标等级 <= value
	inline bool LevelBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//pk值低于数值,整数
	inline bool PkBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//pk值高于数值 ,整数
	inline bool PkAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//随机概率,5000表示百分之50概率，单位万分之1
	inline bool Probalility(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//死亡
	inline bool IsDeath(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//可攻击
	inline bool CanAttack(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//就是自己的等级减目标的等级小于等于这个数值
	
	inline bool LevelDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
		
	bool CircleDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	bool RevertCircleDisBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//就是目标的等级减自己的等级小于等于这个数值
	inline bool RevertLevelDisBelow (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//和目标的距离大于这个值
	inline bool DistanceLarge(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//目标存在buff
	// value1=buff类型 , value2 =buff的组
	inline bool WithBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//目标不存在buff
	inline bool WithoutBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	// 驯服成功
	inline bool TrainOk(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//体力高于百分比,整数，用万分数表示,6000表示60%
	inline bool HpAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//体力低于百分比,整数，用万分数表示,6000表示60%	
	inline bool HpBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//活力高于百分比,整数，用万分数表示,6000表示60%
	inline bool MpAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	// 活力低于百分比 ,整数，用万分数表示,6000表示60%
	inline bool MpBelow (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	// 帮派相同,value as bool
	inline bool SameGuild (CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);
	
	//需要面向目标
	inline bool FaceTarget(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//和目标的方向一致
	inline bool TargetOneDir(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//是特定id的怪物
	inline bool IsSpecialMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	
	//自身的hp比率高于一定的比率
	inline bool SelfHpRateAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自身的hp比率低于一定的比例
	inline bool SelfHpRateBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//阵营相同
	inline bool SelSameCamp(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//和自己是结拜的关系
	inline bool SelfIsBrother(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//是自己的主人
	inline bool IsMaster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//是特定种类的怪物
	inline bool IsSpecialTypeMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	
	



	//最大魔法攻击>= 一个值
	inline bool IsMagicAttackMaxAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//最大魔法攻击< 一个值
	inline bool IsMagicAttackMaxBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//最大道术攻击>= 一个值
	inline bool IsWizardAttackMaxAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//最大道术攻击< 一个值
	inline bool IsWizardAttackMaxBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自身的材料的id
	inline bool IsSelfMetiralItemId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//是否装备了指定的武器
	inline bool IsSpeciaWeapon(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//目标是否是沙巴克成员
	inline bool IsSbkMember(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//在副本里，并且副本id为
	inline bool FbId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自身是怪物，并且怪物的id=value
	inline bool SelfSpecialMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//模型的id=value
	inline bool IsModelId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//场景里是否存在怪物
	inline bool SceneHasMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//场景不存在怪物
	inline bool SceneWithoutMonster(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//自己的等级 <= value
	inline bool SelfLevelBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//己的等级 > value
	inline bool SelfLevelAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//自身的场景的id
	inline bool SelfSceneId(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//场景怪物数目<=value
	inline bool SceneMonsterCount(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//已经召唤出来的宠物的数目
	inline bool MobPetCount(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);


	//实体类型
	inline bool EntityType(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//实体类型
	inline bool EntityVocation(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	
	//是否有特定的装备
	inline bool HasSpecialEquip(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//目标转数 > value,
	inline bool CircleAbove(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//目标转数 <= value
	inline bool CircleBelow(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自己身上有buff
	inline bool SelfWithBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自己身上没有buff
	inline bool SelfWithoutBuff(CAnimal * pSrcEntity,CAnimal *pTargetEntity,GAMEATTRVALUE &value);

	//自己的宠物
	inline bool SelfJobPet(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

	//自己的转数比目标转数
	inline bool SelfCircleBelow(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

	//目标转数比自己的转数
	inline bool RevertCircleBelow(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

	//判断怪物升级的等级数
	inline bool MonsterLevelUpCheck(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);
	
	//判断场景是否有归属于自己的宠物
	inline bool  ScenceHasSelfPet(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

	//判断自己技能等级是否大于等于目标人物等级
	inline bool  SelfSkillLevelAndTargetLevelAbove(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

	//判断自己技能等级是否小于目标人物等级
	inline bool  SelfSkillLevelAndTargetLevelBelow(CAnimal * pSrcEntity, CAnimal * pTargetEntity, GAMEATTRVALUE & value);

////////////////////////////////技能的升级和训练的条件


	//没有条件
	inline   INT_PTR  UtConditionNone(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume); 


	//等级必须大于等于一定的级别
	inline   INT_PTR  UtConditionLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 

	//金钱
	inline   INT_PTR  UtConditionCoin(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 

	inline   INT_PTR  UtConditionFlyShoes(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition ,bool needConsume);

	//物品
	inline   INT_PTR  UtConditionItem(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume); 

	//经验
	inline   INT_PTR  UtConditionExp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//内功等级
	inline   INT_PTR  UtConditionInternalWorkLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume); 

	//修为
	inline   INT_PTR  UtConditionXiuwei(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 


	//职业
	inline   INT_PTR  UtConditionVocation(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 

	//HP是否足够
	inline   INT_PTR  UtConditionHp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume ); 

	//MP
	inline   INT_PTR  UtConditionMp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );  

	//目标存在
	inline   INT_PTR  UtConditionTargetExist(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );  

	//宠物id=value的数量<param
	inline   INT_PTR  UtConditionPetCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );  

	//存在buff
	inline   INT_PTR  UtConditionWithBuff(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume); 

	//不存在buff
	inline   INT_PTR  UtConditionWithoutBuff(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );


	//和目标的距离必须小于
	inline   INT_PTR  UtConditionTargetDistance(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

	//和目标能否重合
	inline   INT_PTR  UtConditionTargetSamePosition(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );


	//需要面向目标
	inline   INT_PTR  UtConditionFaceTarget(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume); 

	//需要和目标的方向
	inline   INT_PTR  UtConditionOneDirTarget(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

	//自身的血的百分比必须低于一定的值
	inline   INT_PTR  UtConditionHpRateBelow(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

	//自身的血的百分比必须高于一定的值
	inline	 INT_PTR  UtConditionHpRateAbove(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

    //需要特定的目标
	inline   INT_PTR   UtConditionSpecialTarget (CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);
	
	//目标可攻击
	inline   INT_PTR   UtConditionTargetCanAttack (CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//体力
	inline INT_PTR   UtConditionJumpPower(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//绑定银两
	inline INT_PTR  UtConditionBindCoin(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

		//阵营的称号
	inline INT_PTR UtConditionCampTitle(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//结拜的数目
	inline INT_PTR UtConditionBrotherCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//与目标Y轴的绝对的距离
	inline INT_PTR UtConditionYAbsDis(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//队伍里的在线的结拜好友的数量 >= value，不包括自己
	inline INT_PTR UtConditionTeamBrotherCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//消耗基础蓝的万分比  value表示基础蓝的万分比，比如8500表示85%，玩家的基础蓝表示等级带来的MaxMp,不包括buff，装备等附加的
	inline INT_PTR UtConditionBaseMaxMpRate(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);
	
	//消耗基础血的万分比  value表示基础血的万分比，比如8500表示85%，玩家的基础血表示等级带来的MaxHp,不包括buff，装备等附加的
	inline INT_PTR UtConditionBaseMaxHpRate(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);


	//在与玩家的战斗状态,value 1表示是 0表示否
	inline INT_PTR UtConditionBattleState(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//需要帮派贡献
	inline INT_PTR  UtConditionGuildContr(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//需要帮派对应的技能等级达到
	inline INT_PTR  UtConditionGuildSkillLevel(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//Xp值的消耗
	inline INT_PTR  UtConditionXpValue(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//材料的耐久
	inline INT_PTR  UtConditionMatiralDura(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//消耗怒气值
	inline INT_PTR  UtConditionAngerValue(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//需要装备有挖矿工具
	inline INT_PTR  UtConditionDigTool(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);

	//需要在特定区域才可以使用
	inline INT_PTR  UtConditionAreaProp(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume);
	
	//至少需要转生才可学习
	inline INT_PTR UtConditionCircle(CAnimal * pActor, INT_PTR nSkillID, SKILLTRAINSPELLCONDITION &condition, bool needConsume);

	//与目标线距离
	inline INT_PTR  UtConditionLineDistance(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );


	//有目标并且目标的距离>=value
	inline INT_PTR  UtConditionTargetDistanceAbove(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );

	//需要当前职业宠物数量>=value
	inline INT_PTR  UtConditionHasJobPetCount(CAnimal *pActor,INT_PTR nSkillID,SKILLTRAINSPELLCONDITION &contition,bool needConsume );  

	//需要行会主殿等级
	inline INT_PTR UtConditionGuildMainHall(CAnimal * pActor, INT_PTR nSkillId,SKILLTRAINSPELLCONDITION &contition ,bool needConsume);
	
	//需要行会科技等级
	inline INT_PTR UtConditionGuildTech(CAnimal * pActor, INT_PTR nSkillId,SKILLTRAINSPELLCONDITION &contition ,bool needConsume);
protected:
	static const TargetSelCondition Handlers[]; //目标的选择条件

	static const SkillUseTrainCondition UseTrainHandlers[]; //使用和升级的条件
	
	static const SkillUseTrainCondition SpellHandlers[]; //施法条件
	static const SkillUseTrainCondition UpgradeHandlers[]; //升级条件

	
};
