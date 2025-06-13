
// #pragma once

// /***************************************************************/
// //景观，比如安全区的路标，做为场景标记的东西，程序创建
// /***************************************************************/

// class CLandscape:
// 	public CEntity
// {
// public:
// 	typedef CEntity Inherited;

// 	//初始化
// 	bool Init(void * data, size_t size);

// 	//删除的一些内存消耗操作
// 	void Destroy(); //销毁一个实体

// 	//执行逻辑循环
// 	virtual VOID LogicRun(TICKCOUNT nCurrentTime);

// 	inline INT_PTR GetPropertySize() const {return sizeof(CFireProperty) -sizeof(CPropertySet);}


// 	 //获取属性集
// 	 virtual CPropertySet * GetPropertyPtr() {return &m_property ;}

// 	 virtual  const CPropertySet* GetPropertyPtr() const {return &m_property ;}

// 	 virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}

// 	 //设置过期时间，单位ms
// 	 VOID SetExpiredTime(INT_PTR nTime);	

// 	 /* 
// 	 * Comments: 创建一个场景景观
// 	 * Param CScene * pScene:场景的指针 
// 	 * Param int nPosX: 位置x
// 	 * Param int nPosY: 位置y
// 	 * Param int nModelId: 模型的id
// 	 * Param  int nTime: 存活时间，单位ms,如果永久有效就是-1
// 	 * Param LPCTSTR sName  路灯名称 
// 	 * @Return CFire *: 返回指针
// 	 */
// 	 static CLandscape * CreateLandscape( CScene *pScene, int nPosX,int nPosY,int nModelId, int nTime,LPCTSTR sName = "");
	 
// private:
// 	CLandscapeProperty  m_property; //属性集合

// 	TICKCOUNT   m_expireTime;		 //过期的时间，低于这个过期时间，这个箱子就要删除
	
// };
