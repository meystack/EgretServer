#pragma once

class CSkillProvider :
	public CVector<OneSkillData>,
	protected CCustomLogicLuaConfig
{
public:
	typedef CVector<OneSkillData>	Inherited;
	typedef CCustomLogicLuaConfig	Inherited2;
	typedef CObjectAllocator<char>	CDataAllocator;

public:
	CSkillProvider();
	~CSkillProvider();

	//转换为OneSkillData数组的类型转换函数
	inline operator const OneSkillData* () const
	{
		CSkillProvider *pProvider = (CSkillProvider*)this;
		return pProvider->Inherited::operator OneSkillData*();
	}


	/*
	* Comments:通过技能ID返回技能数据
	* Param const INT_PTR nSkillId:技能ID
	* @Return const OneSkillData*: 技能指针
	*/
	inline const OneSkillData* GetSkillData(const INT_PTR nSkillId) const
	{
		//ID为0的
		if ( nSkillId <= 0 || nSkillId >= Inherited::count() )
			return NULL;
		else return &(this->operator const OneSkillData*()[nSkillId]);
	}
	
	inline LPCSTR GetSkillNameById(const INT_PTR nSkillId) const
	{
		const OneSkillData* pSkill = GetSkillData(nSkillId);
		return pSkill ? pSkill->sSkillName:"";
	}
	/*
	* Comments: 获取一个技能一个等级的数据
	* Param const INT_PTR nSkillId:技能ID
	* Param const INT_PTR nLevel: 技能的等级
	* @Return const SKILLONELEVEL *:返回技能等级的数据
	*/
	inline   SKILLONELEVEL * GetSkillLevelData( INT_PTR nSkillId, INT_PTR nLevel)
	{
		const OneSkillData * pSkill = GetSkillData(nSkillId);
		if(pSkill ==NULL) return NULL;
		
		if ( nLevel <= 0 || nLevel >=  pSkill->levels.count )
			return NULL;
		else return  &(pSkill->levels.pData[nLevel]);
	}
	
	/*
	* Comments:获取技能的数量
	* @Return INT_PTR: 技能的数量
	*/
	inline INT_PTR count() const{ return Inherited::count(); }

	/*
	* Comments:通过技能的名字返回id
	* Param char * pSkillName:技能的名字
	* @Return INT_PTR:返回技能的id，如果没有返回-1
	*/
	inline INT_PTR GetSkillIdByName(char * pSkillName)
	{
		for(INT_PTR i=0 ;i <Inherited::count(); i++ )
		{
			const OneSkillData * pData = &(this->operator const OneSkillData*()[i]);
			if( 0== strcmp( pData->sSkillName, pSkillName) )
			{
				return pData->nSkillID ;
			};
		}
		return -1;
	}
	
	/*
	* Comments: 从文件里装载技能数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadSkills(LPCTSTR sFilePath);

	std::vector<OneSkillData*>& GetVocationSKills(int nVocation)
	{
		switch (nVocation)
		{
		case enVocWarrior:
			return m_Warrior;
		case enVocMagician:
			return m_Magician;
		case enVocWizard:
			return m_Wizard;
		default:
			return m_None;
		}
	}

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);

private:
	
	/*
	* Comments:装载全部技能的数据
	* @Return bool: 失败返回false
	*/
	bool ReadAllSkills();

private:
	CDataAllocator m_DataAllocator;
	std::vector<OneSkillData*> m_None;// 无职业技能
	std::vector<OneSkillData*> m_Warrior;// 道士技能
	std::vector<OneSkillData*> m_Magician;// 法师技能
	std::vector<OneSkillData*> m_Wizard; //道士技能
};
