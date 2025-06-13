#pragma once

//导出给脚本使用的对象的基类
//引用计数生命期自动管理类，禁止对此类调用delete，不用时应当调用release。
class CBaseScriptExportObject
{
public:	
	CBaseScriptExportObject() { 
		m_nRefCount = 0;
		m_sObjectName[0] = 0; 
	}
	//增加引用计数，函数返回增加后的引用计数。
	virtual int addRef(){ m_nRefCount++;return m_nRefCount;}

	//减少引用计数，函数返回减少后的引用计数。如果计数减少为0则函数销毁对象自己
	virtual int release() { 	
		int nRet = --m_nRefCount;
		if (nRet <= 0)
			destroy();
		return nRet;
	}

	/*
	* Comments: 销毁实体
	* @Return void:
	*/
	virtual void destroy() = 0;
		
	inline int getRef() const { return m_nRefCount; }
	inline virtual const char* getClassName() const { return CBaseScriptExportObject::m_sClassName; }	

	inline void SetObjectName(LPCTSTR name)
	{
		if (name)
		{
			size_t nLen = __min(_tcslen(name), 31);
			memcpy(m_sObjectName, name, nLen * sizeof(TCHAR));
			m_sObjectName[nLen] = 0;
		}
		else
			m_sObjectName[0] = 0;
	}

	inline const TCHAR* GetObjectName() const { return m_sObjectName; }

protected:
	//~CBaseScriptExportObject(){}

private:
	int m_nRefCount;						// 引用计数，构造时必须为1
	static TCHAR m_sClassName[32];			// 该类的名称，用于脚本导出
	TCHAR m_sObjectName[32];				// 脚本对象名称，用于析构时从全局对象管理器中清除此对象
};



class GlobalObjectMgr
{
public:	
	static GlobalObjectMgr& getSingleton()
	{
		return *sGlobalObjectMgr;
	}

	static void initialize();

	static void uninitialize();
	

	/*
	* Comments:设置全局对象，
	* Param const char * pName: 变量名称
	* Param void * object: 对象指针，如果为空则表示移除此变量
	* @Return bool:成功返回true；失败返回false。如果有重名的变量，返回失败。
	*/
	bool SetGlobalObject(const char *pName, void *object)
	{
		if (!pName) return false;

		if (!object)
		{
			m_Objects.remove(pName);
			return true;
		}
		void **pItem = m_Objects.get(pName);
		if (pItem) return false;

		pItem = m_Objects.put(pName);
		if (pItem)
		{
			*pItem = object;		
			return true;
		}

		return false;
	}

	/*
	* Comments: 获取全局对象
	* Param const char * pName: 对象名称
	* @Return void*: 如果对象存在，返回对象指针。如果对象不存在，返回NULL
	*/
	void* GetGlobalObject(const char *pName)
	{		
		if (!pName || strlen(pName) == 0) return NULL;
		void **pItem = m_Objects.get(pName);
		if (pItem)
			return *pItem;
		return NULL;
	}

	bool RemoveObject(const char *pName)
	{
		if (!pName || strlen(pName) == 0) return false;

		return m_Objects.remove(pName) >= 0 ? true : false;
	}

	void ClearGlobalObject(const char *pName)
	{
		if (pName)
			m_Objects.remove(pName);
	}

	static GlobalObjectMgr *sGlobalObjectMgr;
	static LONG sGlobalObjectMgrCount;

private:	
	CCustomHashTable<void*>	m_Objects;
};
