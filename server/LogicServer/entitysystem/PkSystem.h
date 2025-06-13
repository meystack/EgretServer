#pragma once
/**************************************
PK子系统
**************************************/

static const char *szIntercedeCount		= "IntercedeCount";				//今日求情次数

class CPkSystem:
	public CEntitySubSystem<enPkSystemID,CPkSystem,CActor>
{
public:

	/* 
	* Comments: 角色进入游戏
	* @Return void:  
	*/
	virtual void OnEnterGame();

	/*
	* Comments:判断能否给其他玩家攻击
	* 在函数内会同时判断自己能否被攻击（比如10级以下、有buff保护、和平模式等不能被攻击）、以及攻击方有否攻击的能力（比如设置了和平模式不能攻击、帮派模式不能攻击本帮派等）
	* Param CActor * pActor:攻击方玩家的指针
	* @Return bool:返回true表示能被估计,false表示不可以
	*/
	bool _CanAttack(CActor* pActor, CAnimal *pPet=NULL, bool boAttackNotice = true);
	
	bool IsUnLimited(CEntity* pActor);

	bool CanAttack(CActor* pActor, CAnimal *pPet=NULL, bool boAttackNotice = true);

	/*
	* Comments:增加角色的PK值
	* Param INT_PTR nValue:增加的值，可以是负数，表示减少
	* @Return void:
	*/
	void AddPkValue(INT_PTR nValue, CActor * pActor = NULL);

	//获取和设置pk模式
	int GetPkMode();

	//获取Pk模式的名称
	void SendPkModeStr();

	/*
	* Comments: 设置pk模式
	* Param int mode:模式的id
	* Param bool boForce:是否是强制设置的
	* Param bool bNeedBroadCast:是否需要向附近广播，一般情况需要广播，但是在进入游戏的时候设置是不需要广播的
	* @Return void:
	*/
	void SetPkMode( int mode,bool boForce=false,bool bNeedBroadCast =true );

	/*
	* Comments:判断自己的pk模式能否攻击对方
	* Param CActor * pActor:
	* @Return bool:
	*/
	bool CanAttackByMode(CActor* pActor);

	INT_PTR GetZyType(CActor* pActor);
	/*
	* Comments:当PK结束时的计算，包括杀戮值和战魂值。注意哪方胜利是根据HP计算的。HP为0表示失败
	* Param CActor * pActor:pK的对手
	* @Return void:
	*/
	void OnEndPk( CActor* pActor );

	//进入战斗状态（被玩家打通知前端）
	void EnterPK(CActor* pEntity);

	void SendPkToClient();

protected:
	/*
	* Comments:设置自由pk的模式
	* Param CDataPacketReader & reader:
	* @Return void:
	*/
	void SetFreePkMode(CDataPacketReader& reader);

	/*
	* Comments:判断是否有buff保护
	* @Return bool:有的话返回true
	*/
	bool IsProtected();

	/*
	* Comments:删除保护自己的buff,如果有的话
	* @Return void:
	*/
	void DeleteProtectBuff();

	void DoFriendIntercede(CDataPacketReader &reader );

public:
	virtual bool Initialize(void *data,SIZE_T size);
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );


public:
	CPkSystem();
	~CPkSystem();
private:
	UINT						m_SubNext;	//每3分钟减少一点杀戮值
	bool						m_boCanPked;	//能否给别人pk，默认关，40级以上默认开，可以使用物品使其关闭(同时也必须是和平模式）
public:
	BYTE						m_bLastMode;	//上一次的模式，用于区域属性强制改成其他模式后可恢复
};