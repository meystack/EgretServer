#pragma once

/************************************************************************/
/*                           角色BUFF子系统
/*
/*          实现对角色各种buff的添加、删除、属性计算、buff作用和buff管理。
/*
/************************************************************************/
class CAnimal;

class CBuffSystem:
		public CEntitySubSystem<enBuffSystemID,CBuffSystem,CAnimal>
{
public:
	typedef CEntitySubSystem<enBuffSystemID,CBuffSystem,CAnimal> Inherited;

public:
	static CSingleObjectAllocator<CDynamicBuff>	*m_pAllocator;		//buff对象内存申请器
	static float s_FireValRate;

	//buff的颜色数据
	typedef struct tagBuffColorData
	{
		int nBuffType; //buff的类型
		int nGroup;   //buff的group
		int priority;  //优先级
		int nColor;    //设置的颜色
	}BUFFCOLORDATA,*PBUFFCOLORDATA;

public:	
	//根据buff组以及buff类型获取一个buff	
	inline CDynamicBuff* GetBuff(const GAMEATTRTYPE buffType, const INT_PTR nGroup)
	{
		if (m_pEntity == NULL) return NULL;
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		DbgAssert(nGroup >= CDynamicBuff::AnyBuffGroup && buffType < CDynamicBuff::MaxBuffGroup);
		INT_PTR nCount = m_buffList.count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if ( !pBuff)
			{
				continue;
			}
			if (pBuff->pConfig->nType == buffType 
				&& (nGroup == CDynamicBuff::AnyBuffGroup || pBuff->pConfig->nGroup == nGroup)
				)
			{
				return pBuff;
			}
		}
		return NULL;
	}
	
	inline CVector<CDynamicBuff>& GetAllBuff()
	{
		return m_buffList;
	}

	inline CDynamicBuff * GetBuffById( int nBuffId )
	{
		if (m_pEntity == NULL)
		{
			return NULL;
		}
		INT_PTR nCount = m_buffList.count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff * pDBuff = &m_buffList[i];
			if (pDBuff && pDBuff->wBuffId == nBuffId)
			{
				return pDBuff;
			}
		}
		return NULL;
	}
	/*
	* Comments: 获取一个buff类型的第1个buff
	* Param const GAMEATTRTYPE buffType: buff的类型
	* @Return CDynamicBuff *: 获取buff的类型
	*/
	inline CDynamicBuff * GetFirstBuff(const GAMEATTRTYPE buffType)
	{
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if (pBuff && pBuff->pConfig->nType == buffType)
			{
				return pBuff;
			}
		}
		return NULL;
	}

	inline int GetBuffRemainTimeById(int nBuffId)
	{
		CDynamicBuff * pDBuff = GetBuffById(nBuffId);
		return pDBuff ? pDBuff->GetRemainTime() : 0;
	}

	//获取一种buff的的剩余时 间，单位秒
	inline INT_PTR GetBuffRemainTime(const INT_PTR  buffType, const INT_PTR nGroup = -1)
	{
		if(buffType >0 && buffType < GameAttributeCount)
		{
			CDynamicBuff *pBuff =  (nGroup == -1) ? GetFirstBuff((const GAMEATTRTYPE)buffType): GetBuff((const GAMEATTRTYPE)buffType, nGroup);
					
			return pBuff ? pBuff->GetRemainTime() : 0;
		}

		return 0;
	}

	/**
	 * 发送自己的Buff数据
	 */
	void SendBuffData();

	/**
	 * 获取Buff数据
	 */
	void AppendBuffData(CDataPacket &data);
	
	/* TODO: DELETE
	* Comments: 将实体的buff数据写到读写器里
	* Param CDataPacket & pack:读写器
	* @Return void:
	*/
	void WriteBuffData(CDataPacket & pack);

	/*
	* Comments:通过ID添加一个BUFF
	* Param int nBuffId:BUFF的ID,存在BUFF管理器中
	* @Return const CDynamicBuff *:成功返回buff的指针
	*/
	const CDynamicBuff * Append(int nBuffId, CDynamicBuff * pSrcBuff = nullptr, bool is_notice = true );

	/*
	 * Comments: 按分组以及类型删除buff，如果nGroup值为CDynamicBuff::AnyBuffGroup则删除此类型所有组中的buff
	 * const GAMEATTRTYPE buffType:函数返回删除的buff数量
	 * const INT_PTR nGroup:组的id
	 * @Return INT_PTR: 
	 */
	INT_PTR Remove(const GAMEATTRTYPE buffType, const INT_PTR nGroup);


	/*
	* Comments:通过id删除buff
	* Param int nBuffId:buff的id
	* @Return bool *:删除成功buff与否
	*/
	bool RemoveById(int nBuffId,bool boIsExpire=true);

	/*
	* Comments:通过id更改buff值
	* Param int nBuffId:buff的id
	* @Return bool *:成功与否
	*/
	bool ChangeValueById(int nBuffId, GAMEATTRVALUE value);

	/*
	* Comments:把实体身上group属于[nGroupStart,nGroupEnd]的全部删除
	* Param INT_PTR nGroupStart: 起始 的组id
	* Param INT_PTR nGroupEnd: 结束的组id
	* @Return INT_PTR:返回删除的个数
	*/
	INT_PTR RemoveGroupBuff(INT_PTR nGroupStart,INT_PTR nGroupEnd);


	//删除所有的buff，boBoardCast参数表示是否进行广播
	INT_PTR Clear(const bool boBoardCast);

	//例行run，应当至少每秒调用一次
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);

	//计算属性BUFF所提升的属性
	VOID CalcAttributes(CAttrCalc &calc);
	
	//判断指定类型的buff是否存在


	/*
	* Comments:降低buff的时间
	* Param CDynamicBuff * buffTime: buff的指针
	* Param int nSecond:降低的时间
	* @Return VOID:
	*/
	void ReduceBuffTime(CDynamicBuff *pBuff,int nSecond);


	inline bool Exists(const GAMEATTRTYPE buffType)
	{
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if (pBuff != NULL && pBuff->dwTwice > 0 && pBuff->pConfig != NULL && pBuff->pConfig->nType == buffType)
			{
				return true;
			}
		}
		return false;
	}
	
	//通过id查询是否存在该类型的buff
	bool Exists(int nBuffId);
	
	//判断指定状态组中指定类型的buff是否存在
	inline bool Exists(const GAMEATTRTYPE buffType, const INT_PTR nGroup)
	{
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		DbgAssert(nGroup >= CDynamicBuff::AnyBuffGroup && buffType < CDynamicBuff::MaxBuffGroup);
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if ( pBuff && pBuff->pConfig->nType == buffType && pBuff->dwTwice > 0)
			{
				if (nGroup == CDynamicBuff::AnyBuffGroup || pBuff->pConfig->nGroup == nGroup)
				{
					return true;
				}
			}
		}
		return false;
	}

	//判断指定状态组中指定类型的buff是否存在,不包括此buff自己
	inline bool Exists(WORD buffType, const INT_PTR nGroup, const INT_PTR nBuffId)
	{
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		DbgAssert(nGroup >= CDynamicBuff::AnyBuffGroup && buffType < CDynamicBuff::MaxBuffGroup);
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if ( pBuff && pBuff->pConfig->nType == buffType && pBuff->dwTwice > 0)
			{
				if (nGroup == CDynamicBuff::AnyBuffGroup || ( pBuff->pConfig->nGroup == nGroup && pBuff->pConfig->nId !=  nBuffId) )
				{
					return true;
				}
			}
		}
		return false;
	}
	
	//获取指定类型的buff数量
	inline INT_PTR BuffCount(const GAMEATTRTYPE buffType)
	{
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		INT_PTR Result = 0;
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if (pBuff && pBuff->pConfig->nType == buffType)
			{
				Result++;
			}
		}
		return Result;
	}
	
	//获取指定状态组中指定buff类型的数量
	inline INT_PTR BuffCount(const GAMEATTRTYPE buffType, const INT_PTR nGroup)
	{
		DbgAssert(buffType >= 0 && buffType < GameAttributeCount);
		DbgAssert(nGroup >= CDynamicBuff::AnyBuffGroup && buffType < CDynamicBuff::MaxBuffGroup);
		INT_PTR Result = 0;
		INT_PTR nCount = Count();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CDynamicBuff* pBuff = &m_buffList[i];
			if (pBuff && pBuff->pConfig->nType == buffType && pBuff->pConfig->nGroup == nGroup)
			{
				Result++;
			}
		}
		return Result;
	}
	
	//获取buff管理器中的buff数量
	inline INT_PTR Count()
	{
		return m_buffList.count();
	}

	/*
	* Comments: 保存Buff数据到脚本数据中
	* @Return void:
	*/
	void SaveToScriptData();

	/*
	* Comments: 从脚本数据中加载buff数据
	* @Return void:
	*/
	void LoadFromScriptData();

	/*
	* Comments:能否挂上一个buff
	* Param INT_PTR nBuffType:buff的类型
	* @Return bool: 能够添加返回true,否则返回false
	*/
	bool CanAppendBuff(INT_PTR nBuffType);

	// 根据剩余次数，计算出失效时间，用于下线计时
	unsigned int CalcBuffStopTime(const CDynamicBuff *pBuff);

	//计算距离下次生效的时常
	int CalcBuffNextEffectInterval(CDynamicBuff &buff, unsigned int dwStopTime);

	//死亡后删除buff
	void OnDeadRemoveBuff();

	void RemoveBuffGiver();

	bool IsHaveChangeModelBuff();
public:
	/*** 覆盖父类的函数集 ***/
	//初始化
	virtual bool Initialize(void *data,SIZE_T size)
	{
		//当前生效的buff
		m_nCurColorBuffIndex =-1;
		//m_nFireAreaNotice = 0;
		m_buffList.clear();
		return true;	
	}
	//析构
	virtual VOID Destroy()
	{
		if(m_pEntity ==NULL) return;
		Clear(false);
		m_buffColor.clear();
		m_buffList.clear();
		Inherited::Destroy();
	}

	//设置颜色
	void AddBuffColor(int nBuffType,int nGroup, int nPriority, unsigned int nColor);

	//删除一个buff的颜色
	void RemoveBuffColor(int nBuffType,int nGroup);

	void ClearAllColor(); //清掉所有的颜色

	/* 
	* Comments:更新buff存量值
	* Param CDynamicBuff * pBuff:
	* @Return void:
	*/
	void OnBuffChangeValue(CDynamicBuff * pBuff);

	/* 
	* Comments:更新buff存量值
	* Param CDynamicBuff * pBuff:
	* @Return void:
	*/
	void OnBuffChangeCdTime(CDynamicBuff * pBuff);
protected:
	//buff生效的函数
	VOID TakeEffect(CDynamicBuff *pBuff);
	//buff被添加或替换后的调用函数，参数boIsNewBuff用于表示是否是新增的buff，如果值为FALSE则表示是替换了之前存在的buff
	VOID OnBuffAppend(CDynamicBuff *pBuff, bool is_notice = true);
	//单个buff被删除的调用函数
	VOID OnBuffRemoved(CDynamicBuff *pBuff);
	//指定类型的buff被全部删除的调用函数
	VOID OnTypedBuffRemoved(GAMEATTRTYPE buffType);
	// 保存指定Buff数据到脚本数据中
	bool SaveBuffToScriptData(CDynamicBuff *pBuff, int index);
private:
	bool CanDoThisFunction();
protected:
	CTimer<1000>							m_timer;//1s执行一次
	CVector<BUFFCOLORDATA>                  m_buffColor;  //所有的buff添加的颜色

	//当前这个颜色是由哪个buff产生的,是一个下标
	int                                     m_nCurColorBuffIndex ;
	//int										m_nFireAreaNotice;
	CVector<CDynamicBuff>					m_buffList;
};
