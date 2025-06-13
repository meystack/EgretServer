
// class CWalkRide
// 	:public CAnimal
// {
// public:
// 	typedef CAnimal Inherited;

// 	bool Init(void * data, size_t size)
// 	{
// 		m_pOnwer = NULL;
// 		if(Inherited::Init(data, size))
// 			return true;
// 		return false;
// 	}

// 	inline void Destroy()
// 	{

// 	}

// 	void InitAi(int nAiId);

// 	INT_PTR GetPropertySize() const
// 	{
// 		return sizeof(CWalkRideProperty) - sizeof(CPropertySet);
// 	}

// 	virtual VOID LogicRun(TICKCOUNT nCurrentTime);

// 	inline void SetMaster(CActor * pActor)
// 	{
// 		m_pOnwer = pActor;
// 	}

// 	inline CActor * GetMaster()
// 	{
// 		return m_pOnwer;
// 	}

// 	virtual const CPropertySet * GetPropertyPtr() const
// 	{
// 		return &m_property;
// 	}

// 	virtual CPropertySet * GetPropertyPtr()
// 	{
// 		return &m_property;
// 	}
// 	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}
// 	//获取怪物更新的mask
// 	inline CUpdateMask * GetBroadCastMask() {return s_pWalkRideMask;}


// 	/* 
// 	* Comments:初始化mask
// 	* @Return void:
// 	*/
// 	static void InitWalkRideBroadcastMask();


// 	/* 
// 	* Comments:删除mask的指针
// 	* @Return void:
// 	*/
// 	static void DestroyWalkRideBroadcastMask();
// private:
// 	CActor * m_pOnwer;
// 	CWalkRideProperty m_property;
// 	static CUpdateMask * s_pWalkRideMask;
// };
