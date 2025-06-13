#pragma once
//////////////////////////////////////////////////////////////////////////
// 宝箱掉落类，负责加载掉落配置和执行掉落
// 
class CBoxDropMgr;
class CBoxDrop : public CBaseScriptExportObject, public CCustomLogicLuaConfig
{
public:	
	friend class CBoxDropMgr;

	typedef CBaseScriptExportObject Inherit;		

	CBoxDrop();

	/*
	* Comments: 加载掉落配置脚本数据
	* Param const char * pFile:脚本文件名
	* @Return bool: 成功返回true；失败返回false
	* @Remark: 配置文件格式如下：每一行一个掉落项。
	drop = {
	{ id=522 ,count=1,strong =0,quality =-1,propability=10800, group=0},
	{ id=523 ,count=1,strong =0,quality =-1,propability=10800, group=0},
	{ id=524 ,count=1,strong =0,quality =-1,propability=10800, group=0},
	}
	*/
	bool load(const char *pFile);

	/*
	* Comments: 执行一次掉落
	* @Return DropItemList: 掉落物品列表
	*/
	DropItemList drop(void * pEntity =0);

	/*
	* Comments:按概率的方式掉落物品
	* @Return DropItemList: 返回掉落的列表
	*/
	DropItemList proabilityDrop(void * pEntity =0);

	/*
	* Comments: 负责对象的销毁处理。脚本对象的release会减少引用计数，当引用计数为0会调用destroy销毁对象。
	* @Return void:
	*/
	void destroy();

	inline virtual const char* getClassName() const 
	{ 
		return CBoxDrop::m_sClassName; 
	}	


	
protected:
	~CBoxDrop(){}

	/*
	* Comments: 加载掉落配置数据
	* @Return bool:
	*/
	bool loadData();

	/*
	* Comments: 释放掉落相关配置数据
	* @Return void:
	*/
	void releaseData();
	
	//设置物品的属性
	void SetItemInfo(DropItemList& pInfo,ONEDROPDATA * pData );

	/*
	* Comments:判断一件物品的性别和职业的条件能否满足
	* Param ONEDROPDATA * pData:物品的指针
	* Param int sex:性别
	* Param int job:职业
	* @Return bool:能够掉落返回 true，否则返回false
	*/
	inline bool CanDrop(ONEDROPDATA *pData, int sex, int job)
	{
		if(pData->bSex >=0 && sex >=0 && pData->bSex != sex)
		{
			return false;
		}
		if(pData->bJob > 0 && job > 0 && pData->bJob != job)
		{
			return false;
		}
		return true;
	}
private:		
	DROPGROUP		m_dropGroup;	
	static  TCHAR	m_sClassName[32];	
	DECLARE_OBJECT_COUNTER(CBoxDrop)
};



//////////////////////////////////////////////////////////////////////////
// 宝箱掉落管理器；负责管理宝箱管理对象
class CBoxDropMgr
{
public:
	CBoxDropMgr() : m_dataAllocator("CBoxDropMgr"){}
	virtual ~CBoxDropMgr();	
		
	/*
	* Comments: 获取宝箱掉落管理器
	* @Return CBoxDropMgr&:
	*/
	static CBoxDropMgr& getSingleton();


	/*
	* Comments: 初始化宝箱管理器
	* @Return void:
	*/
	static void initialize();

	/*
	* Comments: 销毁宝箱管理器
	* @Return void:
	*/
	static void release();

	/*
	* Comments:创建宝箱
	* Param const char * pKey:
	* Param bool bAddToGOMgr: 是否添加到全局对象管理器
	* @Return CBoxDrop*: 返回创建的宝箱对象
	*/
	CBoxDrop* createBoxDrop(const char *pKey);


	/*
	* Comments: 销毁宝箱对象
	* Param CBoxDrop * bd: 宝箱对象指针	
	* @Return void: 
	* @注意：不要直接调用此函数是否。应该调用CBoxDrop::release()释放对象。
	*/
	void destroyBoxDrop(CBoxDrop *bd);

	
	static CBoxDropMgr *s_BoxDropMgr;
	static LONG s_boxDropMgrCnt;

protected:
	/*
	* Comments: 从列表中移除
	* Param CBoxDrop * bd:
	* @Return void:
	* @Remark:
	*/
	void removeFromList(CBoxDrop *bd);

	void destroyBoxDropImpl(CBoxDrop *bd);
private:
	CSingleObjectAllocator<CBoxDrop>	m_dataAllocator;
	CVector<CBoxDrop*>					m_boxObjectList;
};

