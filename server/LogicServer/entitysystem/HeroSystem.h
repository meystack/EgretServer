#pragma once
/**************************************
英雄系统
**************************************/

class CHero;
class CHeroSystem:
	public CEntitySubSystem<enHeroSystemId,CHeroSystem,CActor>
{
public:
	typedef CEntitySubSystem<enHeroSystemId,CHeroSystem,CActor> Inherid;
	friend class CEntitySubSystem<enHeroSystemId,CHeroSystem,CActor>;

	//英雄状态
	enum eHeroState
	{
		psStateSleep,  //休息状态
		psStateNormal, //出战状态
		psStateMerge,  //合体状态
	};

	//英雄的数据修改的标记
	enum eHeroData
	{
		edHeroBasicData,     //基本数据
		edHeroSkillData,      //技能数据
		edHeroEquipData,    //装备数据
	};
	
	//英雄数据结构
	typedef struct tagHeroData
	{
		HERODBDATA  data;//DB里的数据
		//英雄自身属性
		CAttrCalc  cal; //属性计算器
		CAttrCalc  basicCal; //基本的属性:  基础属性 + 装备属性
		//加成到人物身上的属性
		CAttrCalc  mergeCal; //附体属性加成
		CAttrCalc  battleCal; //出战属性加成 
		float nMergeMaxAttackRate; //附体最大攻击比率
		float nMergeMaxDefenceRate;	//附体最大防御比率
		CVector<HEROSKILLDATA> skills; //技能列表
		CHeroEquip equips; //装备
		EntityHandle  handle; //句柄	
		unsigned long long nMaxExp; //最大的经验
		unsigned int nBaseMaxHp; //基础的血 ,用于技能，暂时保持在这里
		unsigned int nBaseMaxMp; //基础的蓝 ,用于技能，暂时保持在这里
		unsigned int nFootAppear; //足迹外观
		float  fExpRate; //获得人物经验比例
		tagHeroData()
		{ 
			equips.clear();
			skills.clear();
		}
	}HERODATA,*PHERODATA;

	CHeroSystem();
	
	~CHeroSystem();
	
	void  Destroy() ;

	virtual void OnEnterGame();   
	
	virtual bool Initialize(void *data,SIZE_T size);
	
	virtual void OnTimeCheck(TICKCOUNT nTickCount);
	
	void OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader & reader);
	
	void Save(PACTORDBDATA pData);

	//初始化的时候检测英雄
	static void OnInitCheckHero();

	void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	//英雄属性附加到人物
	CAttrCalc & CalcAttributes(CAttrCalc &cal);
	 
	//查看英雄
	void ViewHeros(CActor * pActor);

	//进入场景
	void OnEnterScene();

	//获取英雄列表
	inline CVector<HERODATA> &GetHeroList() {return m_heros;}

	//是否能增加英雄
	bool CanAddHero(bool bWithTipmsg=false);

	//添加英雄
    int AddHero(int  nHeroId ,  int  nStage = 1, int   nLevel =1);

	//通过英雄的ID获取英雄的指针
	inline  HERODATA * GetHeroData(INT_PTR nHeroId)
	{
		BYTE  bHeroId= (BYTE) nHeroId;
		for(INT_PTR i=0; i< m_heros.count(); i++)
		{
			if(m_heros[i].data.bID == bHeroId)
			{
			    return &m_heros[i];
			}
		}
		return NULL;
	}

	//获取英雄数量
	inline INT_PTR GetHeroCount()
	{
		return m_heros.count();
	}

	//获取战斗中英雄指针
	CHero * GetBattleHeroPtr();

	//获取战斗中的英雄ID
	int  GetBattleHeroId();

	//设置英雄的数据改变
	inline void SetHeroDataModifyFlag(INT_PTR nDataType,bool flag)
	{	
		if(HasDbDataInit() ==false) return; //如果数据还没有装载，不能设置标记
		switch(nDataType)
		{
		case edHeroBasicData:
			m_HeroBasicDataHasModifed= flag;
			break;
		case edHeroSkillData:
			m_heroskillHasModified =flag;
			break;
		default:
			break;
		}
	}

	//获取英雄存储位置 如果不存返回-1
	INT_PTR GetHeroPos(INT_PTR nHeroId) ;

	//重设所有英雄属性
	void ResetAllHeroProperty(bool bNewHero = false,bool bRefeshAppear =false);

	//英雄学习技能
	bool LearnSkill(int nHeroID, int nSkillID);

	//获取英雄技能等级
	int GetSkillLevel(int nHeroId, int nSkillId);

	//升级英雄技能
	bool SkillLevelUp(int nHeroId,int nSkillId);

	//英雄升阶
	bool StageUp(int nHeroId, int nStage, int nBless);

	//英雄出战
	bool SetHeroBattle(INT_PTR nHeroId,bool boClient =true );

	//收回英雄
	bool SetHeroSleep(INT_PTR nHeroId,bool bClient =true);

	//英雄被攻击
	 void OnAttacked(CAnimal *pEntity);

	//英雄攻击
	void OnAttackOther(CAnimal * pEntity);

	//英雄跟随
	void  FollowEntity(CAnimal *pActor);

	//玩家Death时英雄的处理
	void OnDeath();

	//添加经验
	void OnAddExp(unsigned int nExp, bool boRate = true, int  nHeroId = 0);

	//重置英雄属性
    void ResetProperty(INT_PTR nHeroPos,bool boNewHero =false,bool boRefAppear =false);

	//收回出战英雄
	void  CallbackBattleHero(bool boClient);

	//跨天清除祝福值
	void  EveryDayClearBless();
	
	//改变英雄经验
	bool RealChangeExp(INT_PTR nPos,unsigned int nAddExp);

	//下发英雄数据(创建&&更新)
	void SendHeroData(HERODATA &Hero);
private:	
	void HandError(CDataPacketReader &packet){}
	void HandGetHeroList(CDataPacketReader &packet); //获取英雄列表
	void HandChangeHeroName(CDataPacketReader &packet); //英雄改名
    void HandHeroSkillLevelUp(CDataPacketReader &packet); //技能升级
	void HandHeroStageUp(CDataPacketReader &packet);//英雄升阶
	void HandSetHeroState(CDataPacketReader &packet); //英雄出战或收回
	void HandViewHero(CDataPacketReader &packet); //查看英雄
	void HandHeroLevelUp(CDataPacketReader &packet); //英雄升级

	//计算英雄的属性
	void CalHeroProp(HERODATA & Hero);

	//下发英雄技能
	void SendAllSkills();

	//记录日志
	void LogHero(const INT_PTR nLogIdent, const INT_PTR nHeroId, const INT_PTR nParam1, const INT_PTR nParam2);

	//英雄改名
    void ChangeHeroName(INT_PTR nPos,char * name);

	//英雄事件
	void TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1=-1,INT_PTR nParam2=-1,INT_PTR nParam3=-1,INT_PTR nParam4=-1);

	//英雄合体
	bool SetHeroMerge(INT_PTR nHeroId,bool bClient =true);

	//下发英雄的经验
	void SetClientHeroExp(HERODATA &Hero);

	//下发英雄状态
    void SendHeroStateChange(INT_PTR nPos);

public:
	float  m_fExpRate;     //英雄的经验倍率
private:
	CVector<HERODATA>   m_heros;				       //玩家的英雄的数据
	bool                m_heroskillHasModified;        //英雄的技能数据是否发生改变
	bool                m_HeroBasicDataHasModifed;     //英雄的基本数据是否发生改变
};
