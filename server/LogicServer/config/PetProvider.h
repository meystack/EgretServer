#pragma once




typedef struct tagPetLevelConfig
{
	unsigned int color;
	CAttrCalc cal;

}PETLEVELCONFIG,*PPETLEVELCONFIG;

//宠物的配置
typedef struct tagPetConfig
{
	int id;         //宠物的ID
	char name[32];    //宠物的名字
	int icon;         //图标
	int nModel; //模型的id

	PETLEVELCONFIG levels [MAX_PET_LEVEL_COUNT];

	//int defaultSkill;  //默认的技能
	int  aiId;         //宠物的ai的iD
	tagPetConfig()
	{
		memset(this,0,sizeof(*this));
	}

	//获取一个等级的数据
	PPETLEVELCONFIG GetLevelData(int nLevel)
	{
		if(nLevel <=0 || nLevel > MAX_PET_LEVEL_COUNT) return NULL;
		return &levels[nLevel -1];
	}
}PETCONFIG,*PPETCONFIG;

typedef struct tagPetMergePropRate
{
	WORD	wType;
	float	fRate;
}PetMergePropRate;


class CPetProvider :
	public CCustomLogicLuaConfig
{
public:
	
	typedef CCustomLogicLuaConfig	Inherited;
	typedef CObjectAllocator<char>	CDataAllocator;

public:
	
	~CPetProvider();
	CPetProvider(); 

	/*
	* Comments:通过ID返回宠物数据
	* Param const INT_PTR nId:宠物ID
	* @Return const PPETCONFIG*: 宠物的数据指针
	*/
	inline const PPETCONFIG GetPetData(const INT_PTR nId) const
	{
		//ID为0的
		if ( nId <0 || nId >= m_petData.count() )
			return NULL;
		else return &m_petData[nId];
	}

	
	/*
	* Comments:获取宠物数量
	* @Return INT_PTR: 
	*/
	inline INT_PTR GetPetCount() const{ return m_petData.count(); }

	
	/*
	* Comments: 从文件里装载宠物的数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadPets(LPCTSTR sFilePath);



	
	//装载宠物的经验的数据
	bool LoadPetExp(LPCTSTR sFilePath);


	/*
	* Comments:通过宠物的洗资质的id获取洗资质的数据指针
	* Param INT_PTR nSmithId: 洗资质的ID
	* @Return DataList<RANDOMATTRTERM> *: 洗资质的配置的指针
	*/
	DataList<RANDOMATTRTERM> * GetSmithData(INT_PTR nSmithId);


	/*
	* Comments: 通过宠物的名字返回宠物的ID
	* Param LPCTSTR name:宠物的名字
	* @Return INT_PTR: 成功返回宠物的ID，否则返回-1
	*/
	inline INT_PTR GetPetIdByName(LPCTSTR name)
	{
		for(INT_PTR i=1; i< m_petData.count(); i++)
		{
			if(strcmp(m_petData[i].name,name) ==0)
			{
				return m_petData[i].id;
			}
		}
		return -1;
	}

	inline unsigned int GetMaxExp(INT_PTR nLevel,INT_PTR nCircle)
	{
		if(nCircle >=0 && nCircle < m_petExp.count)
		{
			if(nLevel >=1 && nLevel <=  m_petExp[nCircle].count)
			{
				return *(m_petExp[nCircle].pData + (nLevel -1));
			}
			 
		}
		return 100000000; //返回一个很大的数，表示出错了
	}

	inline DataList<PetMergePropRate> *GetPetMergePropRate(const INT_PTR nLevel) 
	{
		//nLevel不为0的
		if (nLevel <= 0 || nLevel > m_petMergePropRate.count)
			return NULL;
		else return m_petMergePropRate.GetDataPtr(nLevel-1);
	}

	
protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	
	//读取所有的宠物的配置
	bool ReadAllPet();


	/*
	* Comments:读取一个宠物的数据
	* Param CDataAllocator & dataAllocator:内存分配器
	* Param PPETCONFIG pPet: 宠物的指针
	* Param INT_PTR nPetId:宠物的id
	* @Return bool:成功返回true，否则false
	*/
	bool ReadOnePet(CDataAllocator &dataAllocator,PPETCONFIG pPet,INT_PTR nPetId);
		
	bool ReadTable(char *sName, int *pAddr,int nMaxCount);
	
	//读取宠物的升级经验
	bool ReadPetExp();

	//读取宠物附身属性
	bool ReadPetMergePropRate();

private:
	CVector<PETCONFIG>  m_petData;     
	CDataAllocator m_dataAllocator; //内存分配器
	
	DataList< DataList<unsigned int> > m_petExp; 

	DataList<DataList<PetMergePropRate> > m_petMergePropRate;

};