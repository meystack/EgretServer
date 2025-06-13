#pragma once
class CAnimal;
/***************************************************************/
/*为了使代码可维护性更高，代码结构更加清晰，把技能的结果部分放本类计算
/***************************************************************/


//技能释放的结果的参数，使用一个结构体，避免参数越来越多
typedef struct tagSkillResultParam
{
	CAnimal * pSrcEntity; //释放者的指针
	CAnimal *pTargetEntity; //目标的指针
	PSKILLRESULT pSkillResult; //技能的作用id
	INT_PTR nSkillID;  //技能的id
	WORD nTargetPosX;     //技能的作用点的坐标x
	WORD nTargetPosY;   //技能的作用点的坐标y
	BYTE   btIgnoreDefence;    //忽略对方的防御，技能里使用到的
	BYTE   bReserver;
	BYTE   btIgnoreMaxDropHp;	//忽略每次最大伤害限制
	BYTE   btReserver;        //保留
	tagSkillResultParam()
	{
		memset(this,0,sizeof(*this));
	}
}SKILLRESULTPARAM,*PSKILLRESULTPARAM;

/**
 * 内功技能ID
 * 		技能ID 4：半月弯刀
 * 		技能ID 6：烈火剑法
 * 		技能ID 8：逐日剑法
 * 		技能ID 12：雷电术
 * 		技能ID 18：冰咆哮
 * 		技能ID 21：流星火雨
 * 		技能ID 24：灵魂火符
 * 		技能ID 33：嗜血术
 */
enum enInternalWorkSkillsId
{
	enInternalWorkSkillsId_HalfMoon			= 4,	//半月弯刀
	enInternalWorkSkillsId_Fire				= 6,	//烈火剑法
	enInternalWorkSkillsId_DayByDay			= 8,	//逐日剑法
	enInternalWorkSkillsId_Thunder			= 12,	//雷电术
	enInternalWorkSkillsId_IceStorm			= 18,	//冰咆哮
	enInternalWorkSkillsId_RainFlowSpark	= 21,	//流星火雨
	enInternalWorkSkillsId_SoulFireSign		= 24,	//灵魂火符
	enInternalWorkSkillsId_BloodBite		= 33,	//嗜血术
};

//内功(技能Id -- 玩家属性：技能增伤Id)
std::map<unsigned short, unsigned short> InternalWorkIncreaseDamageMap {
	{enInternalWorkSkillsId_HalfMoon, 		PROP_ACTOR_HALFMONTHS_INCREASEDAMAGE},
	{enInternalWorkSkillsId_Fire, 			PROP_ACTOR_FIRE_INCREASEDAMAGE},
	{enInternalWorkSkillsId_DayByDay, 		PROP_ACTOR_DAYBYDAY_INCREASEDAMAGE},
	{enInternalWorkSkillsId_Thunder, 		PROP_ACTOR_THUNDER_INCREASEDAMAGE},
	{enInternalWorkSkillsId_IceStorm, 		PROP_ACTOR_ICESTORM_INCREASEDAMAGE},
	{enInternalWorkSkillsId_RainFlowSpark, 	PROP_ACTOR_FIRERAIN_INCREASEDAMAGE},
	{enInternalWorkSkillsId_SoulFireSign, 	PROP_ACTOR_FIRESIGN_INCREASEDAMAGE},
	{enInternalWorkSkillsId_BloodBite, 		PROP_ACTOR_BLOODBITE_INCREASEDAMAGE},
};

//内功(技能Id -- 玩家属性：技能减伤Id)
std::map<unsigned short, unsigned short> InternalWorkReduceDamageMap {
	{enInternalWorkSkillsId_HalfMoon, 		PROP_ACTOR_HALFMONTHS_REDUCEDAMAGE},
	{enInternalWorkSkillsId_Fire, 			PROP_ACTOR_FIRE_REDUCEDAMAGE},
	{enInternalWorkSkillsId_DayByDay, 		PROP_ACTOR_DAYBYDAY_REDUCEDAMAGE},
	{enInternalWorkSkillsId_Thunder, 		PROP_ACTOR_THUNDER_REDUCEDAMAGE},
	{enInternalWorkSkillsId_IceStorm, 		PROP_ACTOR_ICESTORM_REDUCEDAMAGE},
	{enInternalWorkSkillsId_RainFlowSpark, 	PROP_ACTOR_FIRERAIN_REDUCEDAMAGE},
	{enInternalWorkSkillsId_SoulFireSign, 	PROP_ACTOR_FIRESIGN_REDUCEDAMAGE},
	{enInternalWorkSkillsId_BloodBite, 		PROP_ACTOR_BLOODBITE_REDUCEDAMAGE},
};

class CSkillResult
{
public:
	
	typedef  void  (CSkillResult::*SkillResultFunc)(PSKILLRESULTPARAM pParam); 
	/*
	* Comments: 计算技能的结果
	* Param PSKILLRESULTPARAM pParam:技能的指针
	* Param bool bNoDelay: 如果为true的话表示无论参数是什么，都不需延迟
	* @Return void:
	*/
	void DoResult( PSKILLRESULTPARAM pParam,bool bNoDelay =false);

	
	//错误的id
	inline void IsError(PSKILLRESULTPARAM pParam){}
	

	//添加buff
	inline void AddBuff(PSKILLRESULTPARAM pParam);
	
	 //删除buff
	inline void DefBuff(PSKILLRESULTPARAM pParam);
	
	 //内功攻击
	inline void PhysicAttack(PSKILLRESULTPARAM pParam);

	//神圣攻击
	inline void HolyAttack(PSKILLRESULTPARAM pParam);

	//瞬移到目标旁
	inline void MoveToTarget(PSKILLRESULTPARAM pParam);

	//瞬移回巢
	inline void MoveBackHome(PSKILLRESULTPARAM pParam);

	//外功攻击
	inline void MagicAttack(PSKILLRESULTPARAM pParam);

	//修改属性
	inline void ChangeProperty(PSKILLRESULTPARAM pParam);

	//把目标抓到自己身边
	inline void CatchTarget(PSKILLRESULTPARAM pParam);

	 //设定攻击目标
	inline void SetAttackTarget(PSKILLRESULTPARAM pParam);

	//治疗
	inline void Cure(PSKILLRESULTPARAM pParam);
	

	//冲锋
	inline void RapidMove(PSKILLRESULTPARAM pParam);
	
	//丢弃目标
	inline void LostTarget(PSKILLRESULTPARAM pParam);
	
	//删除特效,还没编码
	inline void DeleteEffect(PSKILLRESULTPARAM pParam);
	
	//刷怪
	inline void MobEntity(PSKILLRESULTPARAM pParam);
	
	//振屏
	inline void ScreenShake(PSKILLRESULTPARAM pParam);

	//自身的血的%d治疗给目标加血
	inline void SelfHpCureTarget(PSKILLRESULTPARAM pParam);

	//删除特定技能的CD时间
	inline void  SelfDelSkillCd(PSKILLRESULTPARAM pParam);

	//击飞
	inline void  BeatFly(PSKILLRESULTPARAM pParam);

	//击退
	inline void  BeatBack(PSKILLRESULTPARAM pParam);


	inline void CatchMonster(PSKILLRESULTPARAM pParam);//抓怪
	
	inline void WizardAttack(PSKILLRESULTPARAM pParam);//道术攻击
	
	inline void IgnoreDefence(PSKILLRESULTPARAM pParam);  //忽略目标的防御
	
	inline void IgnoreMaxDropHp(PSKILLRESULTPARAM pParam);  //忽略每次最大伤害限制



	inline void  MobFire(PSKILLRESULTPARAM pParam); //召唤出火墙

	inline void  MobPet(PSKILLRESULTPARAM pParam); //召唤出宠物

	//删除自身宠物
	inline void RemovePet(PSKILLRESULTPARAM pParam);

	 //往前走若干格
	inline void  GoAhead(PSKILLRESULTPARAM pParam);

	//后退若干格
	inline void  GoBack(PSKILLRESULTPARAM pParam); 

	
	
	//在一个场景里随机移动
	static void RandTransfer(CFuBen *pFb,CScene *pScene,CEntity *pEntity);

	static void FixTransfer(CFuBen *pFb,CScene *pScene,CEntity *pEntity, int nPosX,int nPosY);

	//最后一个安全区域随机传送
	void  LastSafeAreaRTransfer(PSKILLRESULTPARAM pParam); 

	//指定场景地点传送
	void  Transfer(PSKILLRESULTPARAM pParam); 

	//本场景随机传送
	void  SceneTransfer(PSKILLRESULTPARAM pParam); 

	//上一个安全区传送
	void  LastSafeAreaTransfer(PSKILLRESULTPARAM pParam); 


	//推迟移动时间
	void PostponeMoveTime(PSKILLRESULTPARAM pParam);

	//降低特定部位的装备的耐久
	void  ChangEquipDua(PSKILLRESULTPARAM pParam);

	//按怪物的掉率掉落东西
	void DropMonsterItem(PSKILLRESULTPARAM pParam);

	//改变模型
	void ChangeModel(PSKILLRESULTPARAM pParam);

	//按百分比扣属性,rate表示扩除的万分比，1000表示10%，id=1表示扣血，1表示扣蓝
	void RateChangeProperty(PSKILLRESULTPARAM pParam);

	//改成怪物是否可被攻击
	void ChangeMonsterAttacked(PSKILLRESULTPARAM pParam);

	//给予奖励
	void GiveAward(PSKILLRESULTPARAM pParam);

	//客户端播放引导性特效
	inline void PlayGuide(PSKILLRESULTPARAM pParam);

	//往1个点移动
	inline void PointMove(PSKILLRESULTPARAM pParam);

	//自己移动到范围内的鼠标指定的点
	inline void SelfPointMove(PSKILLRESULTPARAM pParam);

	//向某目标冲撞
	inline void TargetRapidMove(PSKILLRESULTPARAM pParam);

	//瞬回血
	inline void FastSelfCurHp(PSKILLRESULTPARAM pParam);

	//把目标移动到范围内的鼠标指定的点
	inline  void TargetPointMove(PSKILLRESULTPARAM pParam);

	//抓取符合条件所有对象
	inline  void CatchAll(PSKILLRESULTPARAM pParam);

	//抓取符合条件所有对象
	inline  void MonsterMove(PSKILLRESULTPARAM pParam);

	/////////////////////////下面是一些公共的函数

	//正常的攻击，所有的攻击都调用这个函数，只是攻击类型不同
	inline void NormalAttack(PSKILLRESULTPARAM pParam, int nAttackType);

	
	/*
	* Comments:添加buff的处理，因为buff和debuff在成功率和时间减免上有差别，所以把这个接口统一出来
	* Param PSKILLRESULTPARAM pParam:参数
	* Param float fTimeDesc:时间的减免
	* @Return void:
	*/
	inline void AddBufProcess(PSKILLRESULTPARAM pParam, float fTimeDesc=0.0);


	/*
	* Comments:是否命中
	    目前策划给的命中概率是 ( 实际命中几率（%） = 攻击方命中几率（%） - 防守方闪避几率（%）+ 0.95
	* Param PSKILLRESULTPARAM pParam: 参数
	* Param bool isInnerAttack: 是否是内功攻击，true表示是内功攻击，否则表示外功攻击
	* @Return bool: 如果命中返回true，否则返回false
	*/

	inline bool isHit(PSKILLRESULTPARAM pParam, INT_PTR  nAttackType );
	

	//闪避了
	void OnDodge(CEntity * pSrcEntity,CEntity * pEntity);

	inline unsigned int GetAttackAbility(CEntity * pEntity)
	{
		return 1;
	}

	//广播受击
	inline void BroadCastHit(CAnimal * pEntity, Uint64 hTargetHandle);

		/*
	* Comments: 伤害计算, 现在策划的伤害计算是
		hpAttack = 伤害值* (1 - 防御减免比例);
		防御减免比例= 
						外防 /(1500 + 外防) :外攻攻击
						内防 /(800 + 内防)  : 内攻攻击
		故伤害计算为  伤害值 *  1500/(1500 + 外防)  :外攻攻击
				      伤害值 *  800/ (800 + 内防)   :内攻攻击
	* Param INT_PTR nSkillId: 技能的ID
	* Param CEntity * pTargetEntity: 目标实体的指针
	* Param float fAttackValue: 释放技能者打出的伤害值
	* Param INT_PTR  nAttackType: 攻击类型
	* @Return void:
	*/
	 void CalculateDamage(INT_PTR nSkillId,CAnimal * pSrcEntity,CAnimal *pTargetEntity,
		 PSKILLRESULTPARAM pParam,float fAttackValue,INT_PTR  nAttackType);

	 /*
	 * Comments:广播各神对主人丢弃资产
	 * Param LPCTSTR sName:主人名字
	 * Param bool boItems:是否是丢弃物品
	 * Param int nItemId:物品id
	 * Param int nCount:物品数量或金钱数量
	 * @Return void:
	 */
	 void NoticeRemMasterAsset( LPCTSTR sName, bool boItems, int nItemId, int nCount);

	 /*
	 * Comments:通过类型获取攻击值
	 * Param CActor * pSrcActor:玩家
	 * Param int nType:类型
	 * Param int nParam：参数是否随机，不随机就取最大值
	 * @Return int:
	 */
	 int GetAttackValByType(CAnimal * pSrcActor, int nType, int nParam = 0 );
	
	 
	 //说话
	 inline void   Shout(PSKILLRESULTPARAM pParam);


protected:
	static const SkillResultFunc Handlers[];
	//static SkillResultMgr * s_skillResultMgr; //技能内存结果的管理器
};
