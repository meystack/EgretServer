#pragma once

/***************************************************************/
/*
/*                     实体技能子系统
/* 实体释放技能
/* 技能冷却
/* 伤害计算
/***************************************************************/
#pragma once
class CAnimal;
//class CSkillCondition;

class CSkillSubSystem:
	public CEntitySubSystem<enSkillSystemID,CSkillSubSystem,CAnimal>
{
public:
	typedef CEntitySubSystem<enSkillSystemID,CSkillSubSystem,CAnimal> Inherid;
	CSkillSubSystem()
	{
		m_currentSingSkillID =0; //
		m_bDamage2HpFlag =false;
		m_attackMusicRate =-1; //攻击的声音的概率
		m_attackMusicID =-1;   //攻击的声音的ID
	};

	const static int GATHER_SKILL_ID =999; //采集的技能ID，用一个特殊的，用于采集

	//这里除了需要存盘的数据以外还加了技能精确到ms的cd，以后可能还添加一些新的东西
	typedef struct tagSkillData:
		public ONESKILLDBDATA
	{
		TICKCOUNT tick;									//技能的cd 时间
		BYTE      bEvent;								//技能在什么事件下调用，用于怪物的需求
		BYTE      bMjMask;								//秘籍的掩码，表示哪个效果是开启的
		BYTE      bIsClose;								//技能是否已关闭
		BYTE	  bIsPassive;							//是否为被动技能
		BYTE      bPriority;							//优先级
		int       nSingTime;							//吟唱时间
		int       nCoolDownTime;						//技能的冷却时间
		unsigned int       nLastDbCd;					//DB里存储的CD时间，用于存储
		unsigned int       nSkillType;					//技能类型
		tagSkillData(){memset(this,0,sizeof(*this)); } //清理
	}SKILLDATA,*PSKILLDATA;

	typedef struct tagSkillPointResult
	{
		PSKILLONERANGE pRange;   //技能的指针
		EntityVector& entityList; //实体列表
		INT_PTR nPosX;   //作用中心点的x
		INT_PTR nPosY;   //作用中心点的y
	}SKILLPOINTRESULT,*PSKILLPOINTRESULT;
	
	/*
	* Comments:近身肉搏(普通攻击),不使用技能的时候使用
	* Param EntityHandle targetHandle: 目标的实体
	* Param INT_PTR nActionID: 动作编号
	* Param INT_PTR nEffectID : 效果id
	* @Return INT_PTR: 返回错误码,成功返回0
	*/
	INT_PTR NearAttack(EntityHandle targetHandle, INT_PTR nActionID,INT_PTR nEffectID);

	void NoticeOneSkillData(PSKILLDATA pSkillData);

	/**
	 * 开启技能(攻杀，刺杀，半月，烈火，逐日)
	 */
	void SwitchSkill(INT_PTR nSkillID, bool nFlag);

	void InitNextSkillFlag();

	INT_PTR CheckNextSkillFlag(CEntity* pTargetEntity);

	void UpdateNextSkillFlag();

	INT_PTR OnBuffExpire(CDynamicBuff *pBuff);

	/*
	* Comments:发射技能
	* Param INT_PTR nSkillID:技能ID
	* Param INT_PTR nPosX: 作用点的x
	* Param INT_PTR nPosY:作用点的y
	* bool bNoCd : 是否需要检测cd，默认是需要检测cd的，普通的情况是不需要检测cd
	* @Return INT_PTR:如果使用没有错误就返回0，否则返回使用技能的错误码
	*/
	INT_PTR LaunchSkill(INT_PTR nSkillID,INT_PTR nPosX = 0, INT_PTR nPosY = 0, bool bNoCd = false);
	
	// 实际发射技能（即时/延时执行技能）
	INT_PTR RealLaunchSkill(INT_PTR nSkillID,
		CScene* pScene, INT_PTR nDir, INT_PTR nCurPosX, INT_PTR nCurPosY,
		CEntity* pTargetEntity, INT_PTR nMousesPosX, INT_PTR nMousesPosY);

	// 单范围技能作用（选择目标，并生效技能）
	INT_PTR SkillApplyOneRange(int nSkillID, PSKILLDATA pSkillData,PSKILLONERANGE pRange,
		CEntity* pTargetEntity,CScene* pScene,int nCurPosX,int nCurPosY,int nMousesPosX, int nMousesPosY, int nSpellDir);

	// 单格技能效果
	void SkillApplyOneGrid(PSKILLONERANGE pRange, EntityVector& entityList,
	INT_PTR nPosX,INT_PTR nPosY,INT_PTR nSkillID,PSKILLDATA pSkillData,int nCurPosX,int nCurPosY);

	/*
	* Comments: 存盘
	* Param PACTORDBDATA pData: 这个本系统不用
	* @Return void:
	*/
	void Save(PACTORDBDATA pData);

	//DB返回数据
	VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader & reader );

	/*
	* Comments: 处理网络数据
	* Param INT_PTR nCmd: 命令码
	* Param CDataPacketReader & packet:数据包,去掉了消息头的
	* @Return void:
	*/
	void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	

	//定时检查,用于检测吟唱的结束
	VOID OnTimeCheck(TICKCOUNT nTick);


	/*
	* Comments:初始化玩家的技能数据
	* Param void * data:数据指针
	* Param SIZE_T size:数据长度
	* @Return bool:成功返回true,否则返回false
	*/
	bool Initialize(void *data,SIZE_T size);


	//当等级满足了自动学习可以学习的技能
	//void AutoLearnSkill();

	//转职学习技能
	void AutoLearnVocSkill(int nLevel = 1);
	
	
	/*
	* Comments:打断技能的吟唱
	* Param bool isPositive: 是否是主动打断，如果是主动打断，不需要告诉自己
	     不是主动就需要打算
	* @Return void:
	*/
	void EndSingSpell(bool isPositive =true); 
	

	/*
	* Comments: 学习一个技能已经学会，这个主要给怪物使用
	* Param INT_PTR nSkillID: 技能的ID
	* Param nLevelID: 技能的等级
	* Param bool bIsClose: 技能是否是关闭的
	* Param bool bForce: 是否强制学习技能，不理会条件是否满足
	* Param bool bSort;  根据冷却时间排序
	* @Return INT_PTR: 返回学习技能的错误码，如果没有错误，返回0
	*/
	INT_PTR LearnSkill(INT_PTR nSkillID, INT_PTR nLevelID =1,bool bIsClose=false, bool bForce=false, bool bSort = false);


	//清空内存
	inline void Destroy()
	{
		m_skillList.empty();
//		m_entityList.empty();
	}
	//进入游戏
	virtual void OnEnterGame();

	// 进入场景
	void OnEnterScene()
	{
		// 开启半月弯刀
		SwitchSkill(4,1);
	}

	/*
	* Comments: 获取学习一个技能等级的错误码
	* Param INT_PTR nSkillID: 技能ID
	* Param INT_PTR nLevelID: 技能的等级
	* Param bool bForce: 是否强制学习技能，不理会条件是否满足
	* @Return INT_PTR: 返回错误码，如果能够学习就返回0，否则返回错误码
	*/
	INT_PTR GetLearnSkillErrorCode(INT_PTR nSkillID, INT_PTR nLevelID, bool bForce=false);


	/*
	* Comments: 获取当前的已经学习的改技能的等级
	* Param INT_PTR nSkillID: 技能ID
	* @Return INT_PTR: 返回当前的技能等级，如果没有学习返回0
	*/
	inline INT_PTR GetSkillLevel(INT_PTR nSkillID)
	{
		for (INT_PTR i =0 ; i< m_skillList.count();i ++)
		{
			if(m_skillList[i].nSkillID ==nSkillID )
			{
				return m_skillList[i].nLevel;
			}
		}
		return 0;
	}

	//计算被动技能的属性提升属性
	VOID CalcAttributes(CAttrCalc &calc);

	/*
	* Comments:计算一个技能的属性
	* Param INT_PTR nSkillID:技能的ID
	* Param INT_PTR nSkillLevel:技能的等级
	* Param INT_PTR nMjMask:技能的mask
	* Param CAttrCalc & calc:属性的计算器
	* @Return VOID:
	*/
	VOID CalcOneSkillAttr(INT_PTR nSkillID,INT_PTR nSkillLevel,CAttrCalc &calc);

	//遗忘被动技能时的处理
	void OnForgetPassSkill(const OneSkillData* pSkill, int nSkillLevel);

	//判断是否启用技能CD
	bool GetEnableCD();

	//设置技能CD启用与否
	void SetEnableCD(bool boEnable);

	//计算2个实体的攻击距离= max( abs(x1-x2), abs(y1-y2) )
	inline INT_PTR GetAttackDistance(CEntity * pStrEntity, CEntity * pTargetEntity)
	{
		if(pStrEntity == pTargetEntity) return 0;
		int x1,y1,x2,y2;
		pStrEntity->GetPosition(x1,y1);
		pTargetEntity->GetPosition(x2,y2);
		return __max( abs(x1-x2), abs(y1-y2));
	}
	
	 
	/*
	* Comments:  获取一个技能的数据指针
	* Param INT_PTR nSkillID:技能ID
	* @Return PONESKILLDBDATA: 技能的数据指针
	*/
	inline PSKILLDATA GetSkillInfoPtr(INT_PTR nSkillID)
	{
		for (INT_PTR i =0 ; i< m_skillList.count();i ++)
		{
			if(m_skillList[i].nSkillID ==nSkillID )
			{
				return &m_skillList[i];
			}
		}
		return NULL;
	}

	//获取技能释放的时候和目标的距离，怪物ai使用的,如果没有设定距离，返回-1
	INT_PTR GetSpellTargetDistance( INT_PTR nSkillID, INT_PTR nSkillLevel);


	//获取技能的列表
	inline const CVector<SKILLDATA> & GetSkills(){return m_skillList;} 

	//获取技能的结果
	inline CSkillResult * GetSkillResult(){return &m_skillResult;}


	/*
	* Comments:玩家采集怪
	* Param EntityHandle hTarget: 目标句柄
	* Param INT_PTR nPosX: 目标位置X
	* Param INT_PTR nPosY: 目标位置Y
	* Param BYTE dir: 玩家朝向
	* @Return void:
	*/
	void GatherMonster(EntityHandle hTarget, int nPosX, int nPosY, BYTE nNewDir);

	/*
	* Comments:  正式学习一个技能的一个等级，条件是已经做了技能升级的预判断
	* Param INT_PTR nSkillID: 技能的ID
	* Param INT_PTR nLevelID: 技能的等级
	* Param bool isCloseD: 技能的等级
	* @Return void: 
	*/
	void StartRealLearnSkill(INT_PTR nSkillID, INT_PTR nLevelID,bool isClose =false);


	/*
	* Comments:设置技能关闭
	* Param INT_PTR nSkillID:技能id
	* Param bool isClose:是否是开启
	* @Return void:
	*/
	void SetSkillClose(INT_PTR nSkillID, bool isClose);

	/*
	* Comments: 遗忘一个技能
	* Param INT_PTR nSkillId: 技能的ID
	* @Return bool:成功返回true，否则返回false
	*/
	bool ForgetSkill(INT_PTR nSkillId);

	/*
	* Comments: 遗忘某个分类的技能
	* Param INT_PTR nSkillId: 技能的分类ID
	* @Return bool:成功返回true，否则返回false
	*/
	bool ForgetSkillByClass(INT_PTR nSkillClass);



	/*
	* Comments:在条件变化的时候检测技能是否满足，比如结拜技能，阵营技能等在一些条件下需要遗忘
	* Param INT_PTR nSkillClass:技能的种类，在下面的枚举的
	* @Return void:
	*/
	void CheckSkillOnConditionChange(INT_PTR nSkillClass);


	/*
	* Comments:临时删除一个技能的CD
	* Param INT_PTR nSkill:技能的id
	* @Return void:
	*/
	void DelSkillCdTemporary(INT_PTR nSkill);

	//设置主动攻击别人的标志 myEntity 主动攻击   pEntity 被攻击
	void SetInitiativeAttack(CEntity * myEntity,CEntity * pEntity);

	//当跳转场景时停止采集
	void StopGather();

	//采集对脚本回调
	bool GathMonsterCallBack(CEntity* pMonster);
	//召唤宠物技能的数量发生变化
	void SendPetSkillCountChange(INT_PTR nSkillId, INT_PTR nCount);
public:
	 /*  计算坐标旋转
	 *  cX 旋转中心X
	 *  cY 旋转中心Y
	 *  nX 基于中心位置的X相对坐标
	 *  nY 基于中心位置的Y相对坐标
	 *  nDir 旋转方向(0到7)
	 *  &nNewX 输出旋转后的绝对坐标X
	 *  &nNewY 输出旋转后的绝对坐标Y
	 */
	static void PositionRotation(INT_PTR cX, INT_PTR cY, INT_PTR nX, INT_PTR nY, INT_PTR nDir, INT_PTR &nNewX, INT_PTR &nNewY);

	//获取坐标
	static void GetPosition(INT_PTR nX, INT_PTR nY, INT_PTR nDir, INT_PTR nStep, INT_PTR &nNewX, INT_PTR &nNewY);
	//获取方向 
	static INT_PTR GetDirection(INT_PTR nX, INT_PTR nY, INT_PTR nTargetX, INT_PTR nTargetY);




	/*
	* Comments:精确旋转,(nX,nY)旋转到(nRotateTargetX,nRotateTargetY)方向以后得到新的位置（nNewX，nNewY）
	* Param int nRotateTargetX:旋转的目标点的x
	* Param int nRotateTargetY:旋转的目标点的y
	* Param int nX:要旋转的点x
	* Param int nY:要旋转的点y
	* Param int & nNewX:新坐标x
	* Param int & nNewY:新坐标y
	* @Return void:
	*/
	static void  AccurateRotation( INT_PTR nRotateTargetX,INT_PTR nRotateTargetY,INT_PTR nX,INT_PTR nY,INT_PTR &nNewX,INT_PTR &nNewY);

	//发送已经学习的技能列表
	void SendLearnSkill();

	void OnLevelUp(int nLevel);

private:
	
	/*
	* Comments:设置一个技能是否开启
	* Param INT_PTR nSkillId:技能的ID
	* Param INT_PTR nFlag:开启的标记
	* @Return void:
	*/
	void HandSetSkillOpen(INT_PTR nSkillId,INT_PTR nFlag);

	//客户端同步一个技能的CD
	void OnGetCd(INT_PTR nSkillID);

	//设置默认技能
	void SetDefaultSkill(INT_PTR nSkillID);

	 /*
	 * Comments: 广播使用技能
	 * Param INT_PTR nSkillID: 技能ID
	 * Param INT_PTR nLevel:技能的等级
	 * Param EntityHandle hHandle: 目标实体
	 * Param INT_PTR nDir:方向
	 * INT_PTR INT_PTR nAuxParam :辅助参数
	 * @Return : 
	 */
	 inline void BroadSpellSkill(INT_PTR nSkillID,INT_PTR nLevel,INT_PTR nDir,INT_PTR nAuxParam, unsigned short nX = 0, unsigned short nY = 0);


	//释放技能的时候是否需要使用客户端上发的方向

	//使用技能，以及开始吟唱技能的客户端消息中，增加了一个dir:Byte数据，
	//用于表示角色的施法面向。仅在（目标handle为0 或 为角色自己的handle）或（鼠标位置为角色自己的位置）时有意义
	INT_PTR CheckSpellTarget(INT_PTR nSkillID,EntityHandle nTarget, WORD &nDestX, WORD &nDestY, INT_PTR nDir);
	
	//检测释放当前的技能能否通过特殊的buff的判断，如果能通过返回0，否则返回错误的ID
	inline INT_PTR CheckSpecialBuff(const OneSkillData * pSkill);

	bool CheckGather(class CMonster *pTarget, int nPosX, int nPosY, BYTE nDir);

	void BroadGather();

	//重设技能的CD时间
	void ResetSkillCd( PSKILLDATA  pSkill,const SKILLONELEVEL * pLevel);

	//存盘的时候是否有长CD的技能，如果都是短cd的技能，考虑技能的cd不存盘
	bool HasLongCdSkill();

	//开始一个技能的CD
	void HandStartSkillCD(INT_PTR nSkillId);

public:
	CTimer<1000> m_singTimer;   //吟唱定时器

private:
	CVector<SKILLDATA> m_skillList ;//已经学习的技能列表

	TICKCOUNT m_CommonCd; //公共的CD
	
	CSkillCondition m_targetCondition; //目标筛选条件

	CSkillResult  m_skillResult;
	
	WORD       m_currentSingSkillID; //当前吟唱的技能的ID
	WORD       m_wReserver; //保留
	WORD	   m_wMousePosX; //鼠标点中场景的点的x坐标
	WORD	   m_wMousePosY; //鼠标点中场景的点的Y坐标
	EntityHandle  m_hSingTarget; //吟唱的目标

	bool       m_bDamage2HpFlag;  //伤害输出给自己加血的开关w，开启了造成了伤害才能给自己加血，防止aoe一个技能加很多次血
	BYTE       m_attackMusicRate; //攻击的声音的概率
	int       m_attackMusicID;   //攻击的声音的ID

public:
	//跨服数据
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(std::vector<CSkillSubSystem::SKILLDATA>& skillData);
};

