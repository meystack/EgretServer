#pragma once

typedef struct 
{
	int			nCount;
	PGAMEATTR	pAttrs;	//属性表
}Attris;

//英雄阶配置
typedef struct tagHeroStage
{ 
     int nModel;        //模型ID
	 int nWeapon;     //武器模型
	 int nMaxLevel;   //最大等级
	 int nMaxBless;   //最大祝福值
	 int nGiveForce;  //升阶时系统自动给予体力值
	 bool boEveryDayClearBless; //是否跨天清除祝福值
}HEROSTAGE,*PHEROSTAGE;

//英雄等级配置
typedef struct   tagHeroLevel
{
	unsigned int   nLevelUpExp; //升级经验
	Attris		attri;//属性
}HEROLEVEL,*PHEROLEVEL;

//英雄配置
typedef struct tagHeroConfig
{
	int  nId;                 //ID
	char name[32];     //名字
	int  nOpenLevel;  //开放等级
	int  nMaxLevel;    //最大等级
	int  nMaxForce;    //最大体力值
	int  nAddForce;    //定时恢复体力值
	int	 nOfflineAddForce;	//离线定时恢复体力值
	int  nBattleForce;  //出战体力值
	float fExpRate;     //获得人物经验比例
	DataList<HEROSTAGE> stages; //阶列表
	DataList<HEROLEVEL> levels;  //等级列表
	tagHeroConfig()
	{
		memset(this,0,sizeof(*this));
	}
}HEROCONFIG,*PHEROCONFIG;


class CHeroProvider :
	public CCustomLogicLuaConfig
	//public CFileReader<CHeroProvider>
{
public:
	typedef CCustomLogicLuaConfig	Inherited;
	//typedef CFileReader<CHeroProvider>	InheritedReader;
	typedef CObjectAllocator<char>	CDataAllocator;

	inline int  GetHeroMaxCount(){return m_nMaxHeroCount;}
	inline int  GetHeroAddTime(){return m_nAddTime;}
	inline int  GetHeroAI() { return m_nHeroAI;}
	inline int  GetHeroAttackSpeed (){return  m_nDefaultAttackSpeed;}
	inline int  GetBuffGroup(){return m_nBuffGroup;}
	//获取英雄配置
	inline const PHEROCONFIG GetHeroData(const int  nHeroId) const
	{
	     if(nHeroId <= 0 ||  nHeroId >  m_herosData.count())
		      return NULL;
	     else return &m_herosData[nHeroId -1];
	}

	//获取英雄等级配置
	inline  const PHEROLEVEL GetHeroLevel(const int nHeroId, const int nLevel)
	{
	      PHEROCONFIG  pHero = GetHeroData(nHeroId);
		  if(pHero)
		  {
		       if(nLevel <= 0 ||  nLevel >  pHero->levels.count )
			   {  
			        return NULL;
			   }
			   return  &pHero->levels[nLevel-1];
		  }else
		  {
		      return NULL;
		  }
	}

	//获取英雄阶配置
	inline  const PHEROSTAGE   GetHeroStage(const int nHeroId, const int nStage)
	{
		PHEROCONFIG  pHero = GetHeroData(nHeroId);
		if(pHero)
		{
			if(nStage  <= 0 ||  nStage  >  pHero->stages.count )
			{  
				return NULL;
			}
			return  &pHero->stages[nStage -1];
		}else
		{
			return NULL;
		}
	}

public:
	~CHeroProvider();
	CHeroProvider(); 
	bool LoadConfig(LPCTSTR sFilePath);
protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
   	bool ReadAllHero();
	bool ReadOneHero(CDataAllocator &dataAllocator,PHEROCONFIG  pHero, int nHeroId); 
	bool LoadAttri( PGAMEATTR pAttr);
private:
    CVector<HEROCONFIG>  m_herosData; 
	int  m_nHeroAI; //英雄AI
	int  m_nMaxHeroCount;
	int  m_nAddTime;  //离线体力计算时长
    CDataAllocator m_dataAllocator; //内存分配器
	int  m_nDefaultAttackSpeed;  //默认攻击速度
	int  m_nBuffGroup; //英雄Buff组
};