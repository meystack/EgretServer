#pragma once
#include <map>

class CDBDataCache;
class CActorCacheData;

//////////////////////////////////////////////////////////////////////////
// 角色Cache数据句柄管理器类型定义
//////////////////////////////////////////////////////////////////////////
typedef Handle<UINT> CActorCacheDataHandle;
typedef HandleMgr<CActorCacheData, CActorCacheDataHandle> CActorCacheDataHandleMgr;

class CActorCacheData
{
public:
	CActorCacheDataHandle		m_handle;							// 句柄
	CBaseLinkedList<CActorCacheData*>::NodeType* m_node;						// 对应于在list中的节点，用于删除时提高效率 
	unsigned int							m_nActorId;							// 角色Id
	TICKCOUNT					m_nNextSaveTick;					// 下一次存盘时间
	CDataPacket					m_sActorCacheData[enCT_Max];		// 角色数据Cache
	Uint64						m_sACDDirtyFlag;					// 角色数据更新标记。支持最多64中类型
	//bool						m_sACDDirty[enCT_Max];				// 角色数据Cache脏标记，对每个类型的数据都区分
	//bool						m_bDirty;							// 数据是否需要更新
	bool						m_bExpired;							// 是否过期，如果过期下次RunOne会删除
	static const TICKCOUNT		s_nExpireTime;						// 过期时间
	CActorCacheData() : m_handle(0), m_node(0), m_nActorId(0), m_bExpired(false),m_sACDDirtyFlag(0){}

	inline void Init(CBufferAllocator &allocator, unsigned int nActorId)
	{
		for (INT_PTR i = 0; i < enCT_Max; i++)
		{
			m_sActorCacheData[i].setAllocator(&allocator);
			m_sActorCacheData[i].setPosition(0);
		}		

		m_nActorId = nActorId;
		m_nNextSaveTick = _getTickCount();
	}

	//追踪内存情况
	INT_PTR Trace()
	{
		INT_PTR nTotalSize=0;//总内存
		for (INT_PTR i = 0; i < enCT_Max; i++)
		{
			INT_PTR nSize =m_sActorCacheData[i].getMemorySize();
			/*
			if(nSize >4096)
			{
				OutputMsg(rmNormal,"Actorid=%d,msgid=%d,size=%d",m_nActorId,(int)i,(int)nSize);
			}
			*/
			nTotalSize += nSize;
		}	
		//OutputMsg(rmNormal,"Actorid=%d,memory=%d",m_nActorId,(int)nTotalSize);
		return nTotalSize;
	}
	
	//// 判断数据是否脏
	inline bool IsDirty() const	{
		return m_sACDDirtyFlag != 0;
	}

	inline bool IsACDDirty(int tp) const {
		return (m_sACDDirtyFlag & (Uint64)((1 << tp))) != 0 ? true : false;
	}

	inline void SetDirty(int tp, bool bDirty)
	{
		if (bDirty)
		{
			m_sACDDirtyFlag |= (Uint64)((1 << tp));
		}
		else
		{
			m_sACDDirtyFlag &= ~(Uint64)(1 << tp);
		}
	}

	inline void ResetDirty()
	{
		m_sACDDirtyFlag = 0;
	}


	//inline bool IsDirty() const { 
	//	return m_bDirty; 
	//}
	//// 设置数据脏属性
	//inline void SetDirty(bool bDirty) { 
	//	m_bDirty = bDirty; 		
	//	if (bDirty && IsExpired())
	//		SetExpired(false);
	//}

	// 获取句柄
	inline CActorCacheDataHandle GetHandle() const { return m_handle; }
	// 设置句柄
	inline void SetHandle(CActorCacheDataHandle& handle) 
	{ 
		m_handle = handle; 
	}
	

	// 过期相关的操作
	inline bool IsExpired() const  { return m_bExpired; }

	inline void SetExpired(bool bExpired) { 
		if (m_bExpired != bExpired) 
			m_bExpired = bExpired; 
	}

	inline bool CheckAndSet(TICKCOUNT nCurrTick)
	{
		if (nCurrTick >= m_nNextSaveTick)
		{
			PostponeNextSaveTick();
			return true;
		}

		return false;
	}

	void PostponeNextSaveTick();	

	inline bool operator < (const CActorCacheData& data)
	{
		return m_nActorId < data.m_nActorId ? true : false;
	}

private:
	CActorCacheData(const CActorCacheData& rhs)
	{
		m_nActorId	= rhs.m_nActorId;
		m_bExpired	= false;
		ResetDirty();
	}
};


class CDBDataClientHandler;

class CDBDataCache
{
	friend class CActorCacheData;

public:
	CDBDataCache();
	virtual ~CDBDataCache(){}	
public:
	/*
	* Comments: 设置DBClient
	* Param ILogicDBRequestHost *pHost:
	* Param CDBDataClientHandler* handler: DB请求处理器
	* @Return void:
	* @Remark:
	*/
	void SetDBClient(ILogicDBRequestHost* pHost, CDBDataClientHandler* handler);

	/*
	* Comments: 缓存角色数据
	* Param enActorCacheDataType cdType: 缓存数据类型
	* Param CDataPacketReader & packet: 缓存数据
	* @Return bool: 成功返回true；失败返回false
	* @Remark:
	*/
	bool CacheData(enActorCacheDataType cdType, CDataPacketReader& packet);
	
	/*
	* Comments: 保存Cache数据
	* Param bCheckCD: 指定是否检测CD
	* Param bResp: 是否回应消息给逻辑服	
	* Param INT_PTR nTimeLmt: 时间限制
	* @Return bool: 成功返回true；失败返回false	
	* @Remark:
	*/
	bool SaveData(bool bCheckCD, bool bResp, INT_PTR nTimeLmt = INT_MAX);

	/*
	* Comments: 例行处理
	* Param nCurrentTick: 当前的Tickount
	* @Return void:
	* @Remark:
	*/
	void RunOne(TICKCOUNT nCurrTick);

	/*
	* Comments: 角色登录
	* Param CDataPacketReader & packet:
	* @Return void:
	* @Remark:
	*/
	void OnActorLogin(CDataPacketReader& packet);

	/*
	* Comments: 角色退出
	* Param 
	* @Return void:
	* @Remark:
	*/
	void OnActorLogout(CDataPacketReader& packet);

	void SaveAllActorDataImmediately();

	/*
	* Comments:追踪内存的使用情况
	* @Return void:
	*/
	void Trace();

protected:	
	/*
	* Comments: 获取角色Id
	* Param CDataPacketReader & packet:
	* Param INT_PTR nOffset: 角色Id在消息包中的偏移
	* @Return unsigned int: 成功返回角色id，失败返回0
	* @Remark: 读取数据中的角色id字段，并且不修改packet内容
	*/
	unsigned int GetActorId(CDataPacketReader& packet, INT_PTR nOffset = 0);

	/*
	* Comments:立刻保存角色数据
	* Param int nActorId:
	* Param bool bResp: 是否给逻辑服回应答消息
	* @Return bool:
	* @Remark:
	*/
	bool SaveActorDataImmediately(unsigned int nActorId, bool bResp = true);
	
	/*
	* Comments: 移除过期CacheData
	* @Return void:
	* @Remark:
	*/
	void RemoveExpireCache();

	/*
	* Comments: 分配一个角色Cache数据节点	
	* Param int nActorId: 角色Id
	* Param CActorCacheDataHandle & handle: 返回分配的ACD节点句柄
	* @Return CActorCacheData*: 返回可存储ACD对象节点指针
	* @Remark:
	*/
	CActorCacheData* AllocActorCacheData(unsigned int nActorId, CActorCacheDataHandle &handle);

	/*
	* Comments: 根据句柄查询指定ACD节点对象
	* Param CActorCacheDataHandle & handle: ACD节点句柄
	* @Return CActorCacheData*: 返回对应的ACD对象指针
	* @Remark:
	*/
	CActorCacheData* GetActorCacheData(const CActorCacheDataHandle& handle);

	/*
	* Comments: 根据角色Id查找ACD节点对象
	* Param int nActorId:
	* @Return CActorCacheData*:
	* @Remark:
	*/
	CActorCacheData* GetActorCacheDataByActorId(unsigned int nActorId);

	/*
	* Comments: 释放指定句柄的ACD节点对象
	* Param CActorCacheDataHandle & handle: ACD节点句柄
	* Param , bool updateIndex: 为true表示在释放ACD节点同时更新索引表
	* @Return void:
	* @Remark:
	*/
	void FreeActorCacheData(const CActorCacheDataHandle& handle, bool updateIndex = true);

	/*
	* Comments: 根据角色缓存数据类型查找对应的消息号
	* Param enActorCacheDataType tp:
	* @Return jxSrvDef::INTERSRVCMD:
	* @Remark:
	*/
	jxSrvDef::INTERSRVCMD GetCmdIdByCDT(enActorCacheDataType tp);

	/*
	* Comments: 保存角色数据
	* Param CActorCacheData & data: 角色数据
	* Param bool bCheckCD: 是否检测CD
	* Param TICKCOUNT nCurrTick: 当前的TickCount
	* Param bool bResp: 是否给逻辑服务器会操作结果应答消息
	* @Return bool: 成功返回true；失败返回false
	* @Remark:
	*/
	bool SaveActorDataImpl(CActorCacheData& data, bool bCheckCD, TICKCOUNT nCurrTick, bool bResp);		
	bool SaveActorBasicData(CDataPacketReader& packet,		bool bResp = true);
	bool SaveBagItemData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveEquipItemData(CDataPacketReader& packet,		bool bResp = true);
	bool SaveDepotItemData(CDataPacketReader& packet,		bool bResp = true);
	bool SavePetItemData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveOtherGameSetsData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveSkillData(CDataPacketReader& packet,			bool bResp = true);	
	bool SaveQuestData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveGuildData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveMiscData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveAchieveData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveScriptData(CDataPacketReader& packet,			bool bResp = true);
	bool SavePetData(CDataPacketReader& packet,				bool bResp = true);
	bool SavePetSkillData(CDataPacketReader& packet,		bool bResp = true);
	bool SavCombatGameInfoData( CDataPacketReader& packet,	bool bResp = true);
	bool SavCombatRecordData( CDataPacketReader& packet,	bool bResp = true);
	bool SaveFriendData( CDataPacketReader& packet,			bool bResp = true);
	bool SavGamesetData(CDataPacketReader& packet,			bool bResp = true);
	bool SaveActorMailData(CDataPacketReader& packet,		bool bResp = true);
	bool SaveAlmirahItemData(CDataPacketReader& packet,		bool bResp = true); // 衣橱系统
	bool SaveDeathData( CDataPacketReader& packet,			bool bResp = true);
	bool SaveDeathDropData( CDataPacketReader& packet,		bool bResp = true);
	bool SaveConsignData( CDataPacketReader& packet,		bool bResp = true);//寄卖
	bool SaveStaticCounts( CDataPacketReader& packet,       bool bResp = true);//计数器
	CLogicDBReqestHandler& GetLogicDBRequestHandler();

private:
	typedef bool (CDBDataCache::*SaveHandler)(CDataPacketReader&, bool);

	CBufferAllocator				m_allocator;						// BuffAllocator
	ILogicDBRequestHost				*m_pHost;
	CDBDataClientHandler*			m_pClientHandler;					// DB请求处理器
	CActorCacheDataHandleMgr		m_sACDHandleMgr;					// 角色Cache数据句柄管理器
	std::map<unsigned int, CActorCacheData*>	m_sACDIdxTbl;						// 角色Cache数据索引表 actorid --> CActorCacheData*
	CBaseLinkedList<CActorCacheData*>	m_list;								// 用于遍历
	wylib::container::CLinkedListIterator<CActorCacheData*> m_listIter; // 用于保存上次遍历位置的迭代器

	TICKCOUNT						m_nNextCheckTime;					// 检测更新角色数据定时器
	bool							m_bHasExpireCache;					// 标记是否删除Cache数据
	static const TICKCOUNT			s_nCheckInterval;					// 检测更新角色数据定时器间隔，默认5s检测一次
	static const SaveHandler		s_aActorCacheHandler[];				// 角色数据Cache处理器表	
	static const jxSrvDef::INTERSRVCMD s_aCDT2RespSrvCmdID[];				// 角色缓存数据类型到应答消息命令ID映射表
public:
	static TICKCOUNT			s_nActorCacheSaveInterval;			// 角色数据存盘间隔
};

