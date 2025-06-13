#pragma once

class CAiProvider :
	protected CVector<AICONFIG>,
	protected CCustomLogicLuaConfig
{
public:
	typedef CVector<AICONFIG>		Inherited;
	typedef CCustomLogicLuaConfig	Inherited2;
	typedef CObjectAllocator<char>	CDataAllocator;

public:
	CAiProvider();
	~CAiProvider();

	//转换函数
	inline operator const AICONFIG* () const
	{
		CAiProvider *pProvider = (CAiProvider*)this;
		return pProvider->Inherited::operator AICONFIG*();
	}


	/*
	* Comments:通过怪物ID返回怪物数据
	* Param const INT_PTR nId:怪物ID
	* @Return const MONSTERCONFIG*: 怪物的数据指针
	*/
	inline const PAICONFIG GetAiData(const INT_PTR nId) const
	{
		//ID为0的
		if ( nId <=0 || nId > Inherited::count() )
			return NULL;
		else return (PAICONFIG)&(this->operator const AICONFIG*()[nId-1]);
	}
	

	
	/*
	* Comments:获取数量
	* @Return INT_PTR: 
	*/
	inline INT_PTR count() const{ return Inherited::count(); }

	
	/*
	* Comments: 从文件里装载怪物的数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadAI(LPCTSTR sFilePath);

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	
	/*
	* Comments:装载全部怪物的数据
	* @Return bool: 失败返回false
	*/
	bool ReadAllAI();


	/*
	* Comments: 读取一个怪物的数据
	* Param CDataAllocator & dataAllocator: 内存分配器
	* Param AICONFIG * oneAi:  ai指针
	* @Return bool:
	*/
	bool ReadOneAI(CDataAllocator &dataAllocator,AICONFIG *oneAi);
	
	/*
	* Comments: 读取AI中的发言配置
	* Param CDataAllocator & allocator:
	* Param AICONFIG * pAIConfig:
	* @Return bool:
	*/
	bool ReadMonsterTalk(CDataAllocator &allocator, AICONFIG *pAIConfig);

	/*
	* Comments: 读取发言配置
	* Param MonsterTalkConfig & cfg:
	* @Return void:
	*/
	void ReadTalkConfigImpl(MonsterTalkConfig &cfg);

	/*
	* Comments: 读取AI中的怪物移动路径点
	* Param CDataAllocator & allocator:
	* Param AICONFIG * pAIConfig:
	* @Return bool:
	*/
	bool ReadPathPoints( CDataAllocator &dataAllocator,AICONFIG *pAiConfig);

private:
	CDataAllocator m_DataAllocator; //内存分配器

	//玩家的宠物洗资质的数据
	DataList< DataList<RANDOMATTRTERM> > m_smithData;
};
