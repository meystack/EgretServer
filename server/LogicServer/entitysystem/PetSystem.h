#pragma once
/**************************************
玩家的宠物子系统,包含玩家有哪些宠物，处理宠物方面的通信
保存玩家的宠物的技能和装备的数据
**************************************/

#define CHG_PET_BUFF_GROUP 33

class CPet;
class CPetSystem:
	public CEntitySubSystem<enPetSystemID,CPetSystem,CActor>
{
public:
	typedef CEntitySubSystem<enPetSystemID,CPetSystem,CActor> Inherid;
	
	
	typedef struct tagPetData
	{
		unsigned int nConfigId;     //宠物的ID,也就是怪物的id
		unsigned int nId;           //宠物的ID
		int			nSkillId;		//召唤宠物的技能id
		EntityHandle  handle;   //宠物的句柄 
		tagPetData()
		{
			nConfigId =0;
			nId =0;
			nSkillId =0;
		}
	}PETDATA,*PPETDATA;
	
	virtual void Destroy();

	~CPetSystem();
	
	void RemoveAllPets();

	/*
	 * Comments:玩家被攻击的时候，要通知宠物
	 * Param CAnimal * pEntity:攻击者的指针
	 * @Return void:
	 */
	 void OnAttacked(CAnimal *pEntity);

	 /*
	 * Comments:攻击别人
	 * Param CAnimal * pEntity:被攻击者的指针
	 * @Return void:
	 */
	 void OnAttackOther(CAnimal * pEntity);

	 //玩家死亡的时候
	 void OnDeath();

	 void OnActorLoginOut();

	 virtual void OnEnterGame();
		void OnEnterScene();

	 /*
	 * Comments:添加一个宠物
	 * Param INT_PTR  PetId: 宠物的id
	 * Param INT_PTR nLevel:宠物的等级
	 * Param INT_PTR nAiId:宠物的id
	 * Param INT_PTR nLiveSecs:生存时间，默认是无限
	 * Param INT_PTR nInheritPercent 宠物继承主人属性百分比
	 * Param INT_PTR nSkillId 技能Id（用什么技能召唤的）
	 * @Return bool:成功返回true，否则返回false
	 */
	 bool AddPet(INT_PTR  PetId,INT_PTR nLevel, INT_PTR nAiId, INT_PTR nLiveSecs = 0, INT_PTR nInheritPercent = 0, INT_PTR nSkillId = 0,int nTarPosX = 0,int nTarPosY = 0);


	/*
	* Comments:产生一个宠物的ID
	* @Return INT_PTR:宠物的ID
	*/
	unsigned int GenPetId();
	
	//删除一个宠物
	bool RemovePet(int nPetId);

	//删除某个id的全部宠物
	INT_PTR RemoveAllPetByConfigId(int nPetConfigId);

	//删除出战宠物
	void RemoveBattlePet();


	/*
	* Comments:获取一个宠物的存储位置 
	* Param INT_PTR nPetId:宠物的ID
	* @Return BYTE:返回位置，如果不存在就是-1
	*/
	inline INT_PTR  GetPetPos(INT_PTR nPetId) 
	{
		for(INT_PTR i=0; i< m_pets.count();i ++)
		{
			if(m_pets[i].nId == nPetId)
			{
				return i;
			}
		}
		return -1;
	}


	//处理网络消息
	VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	void SendPetCount();
	
	//获取宠物的数目
	INT_PTR GetPetCount()
	{
		return m_pets.count();
	}

	/*
	* Comments:获取道士召唤出来的宠物数量（不包含偶遇宠物）
	* @Return int:返回宠物数据
	*/
	int GetJobPetCount();

	//获取比较高级的道士宠物
	CPet *GetTopJobPet();

	//找到宠物的指针
	inline PETDATA *GetPetData(INT_PTR nPos)
	{
		if(nPos <0 || nPos >= m_pets.count())
		{
			return NULL;
		}
		return &m_pets[nPos];
	}

	/*
	* Comments:获取指定id的数量
	* Param int nPetId:宠物的配置id=怪物id
	* @Return int:返回的数量
	*/
	int GetPetCountByConfigId(int nPetId);

	/*
	* Comments:获取技能召唤出来的宠物数量
	* Param int nSkillId:召唤宠物的技能id
	* @Return int:返回的数量
	*/
	int GetPetCountBySkillId(int nSkillId);

	/*
	* Comments:定时执行
	* Param TICKCOUNT curTick:当前tick
	* @Return VOID:
	*/
	VOID OnTimeCheck(TICKCOUNT curTick);

	/*
	* Comments:召回宠物
	* @Return void:
	*/
	void RecallPet();

	/* 
	* Comments:是否是自己的宠物
	* Param CPet * pPet:
	* @Return bool:
	*/
	bool IsMyJobPet(CPet * pPet);
private:
	//统一的移除宠物接口
	void RemovePetByPos(INT_PTR nPos);
	void SendPetSkillCountChange(int nPetConfigId, int nSkillId);
private:
	CVector<PETDATA>    m_pets;				       //玩家的宠物的数据（召唤所得）
	CTimer<10000>		m_10Sec;
};
