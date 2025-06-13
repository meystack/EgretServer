#pragma once
/*
玩家仓库的背包的处理类，玩家的仓库包括了多个背包
*/


//玩家的仓库背包
class CDeportBag:
	public CUserItemContainer
{
public:
	typedef CUserItemContainer Inherited;
	CDeportBag()
	{
		m_hasLoadDbData =false; //是否装载过DB的数据
	}
public:
	/*** 覆盖CUserItemContainer父类的函数集 ***/
	virtual VOID SendDeleteItem(const CUserItem *pUserItem,INT_PTR nLogIdent) const ;
	virtual VOID LogNewItem(const CUserItem *pUserItem, const CStdItem *pStdItem, LPCSTR lpSender, const INT_PTR nLogIdent) const ;
	virtual VOID LogItemCountChange(const CUserItem *pUserItem, const CStdItem *pStdItem, const INT_PTR nCountChg, LPCSTR lpSender, const INT_PTR nLogIdent) const ;
	virtual VOID SendAddItem(const CUserItem *pUserItem,INT_PTR nLogIdent, BYTE bNoBatchUse = 0,BYTE bNotice = 1) const ;
	//isGetNewCount true获得新的物品
	virtual VOID SendItemCountChange(const CUserItem *pUserItem, bool isGetNewCount = false, BYTE bNoBatchUse = 0) const ;
	//当物品添加成功到里边调用，主要用来发系统提示的

	//添加物品的提示
	virtual VOID OnAddItem(const CStdItem * pItem,INT_PTR nCount,INT_PTR nLogIdent,const CUserItem *pUserItem=NULL);
	//删除物品的提示
	virtual VOID OnDeleteItem(const CStdItem * pItem, INT_PTR nItemCount,INT_PTR nLogIdent, int nMsgId = 0,const CUserItem *pUserItem=NULL);
	
	//设置数据是否修改过了
	virtual VOID OnDataModified() 
	{
		SetDataModifyFlag(true);
	}

	//设置数据的修改的标记
	inline void SetDataModifyFlag(bool flag)
	{
		if(!m_hasLoadDbData ) return ;//没有初始化DB的数据是不需要保存的
		if(m_hasDataModify != flag)
		{
			m_hasDataModify = flag;
		} 
	}
	
	//数据是否修改过
	inline bool IsDataModified()
	{
		return m_hasDataModify;
	}

	//已经装载过DB的数据了
	inline void OnInitDbData()
	{
		m_hasLoadDbData = true;
	}

	//是否装载过DB的数据
	inline bool HasInitDbData()
	{
		return m_hasLoadDbData;
	}
	
public:
	CActor *m_pEntity; //玩家的指针
	BYTE    m_bReserver;
	WORD    m_wReserver;
	bool    m_hasLoadDbData;  //是否装载过DB的数据
};
