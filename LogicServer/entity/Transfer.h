
#pragma once

/***************************************************************/
/*
/*                     非生物,包括传送门,建筑,特效等等
/*
/***************************************************************/


class CTransfer:
	public CNolive
{
public:
	typedef CNolive Inherited;

	static int GETTYPE() {return enTransfer;}

	//初始化
	bool Init(void * data, size_t size)
	{
		if( Inherited::Init(data,size) ==false) return false;
		return true;
	}

	//删除的一些内存消耗操作
	inline void Destroy(){ Inherited::Destroy(); } //销毁一个实体

	//获取属性集的大小
	inline INT_PTR GetPropertySize() const
	{
		return sizeof(CTransferProperty) - sizeof(CPropertySet);
	}
	//获取属性集的指针
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}
	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}

	//获取怪物更新的mask
	//inline CUpdateMask * GetBroadCastMask() {return s_monsterBroadcastMask;}

	/*
	* Comments: 设置传送门的参数
	* Param INT_PTR nScenceID: 目标场景的ID
	* Param INT_PTR nPosX:目标场景的x
	* Param INT_PTR nPosY: 目标场景的y
	* @Return void:
	*/
	inline void SetParams(int nScenceID,int nPosX,int nPosY)
	{
		m_nTargetX = nPosX;
		m_nTargetY = nPosY;
		m_nTargetSceneID = nScenceID;
	}
	/*
	* Comments: 获取传送门的ID
	* Param INT_PTR & nScenceID:场景ID
	* Param INT_PTR & nPosX: X
	* Param INT_PTR & nPosY:Y
	* @Return void:
	*/
	inline void GetParams(int &nScenceID,int &nPosX,int &nPosY)
	{
		nPosX  = m_nTargetX;
		nPosY  =m_nTargetY ;
		nScenceID  = m_nTargetSceneID;
	}
private:
	CTransferProperty m_property;

	int m_nTargetX; //目标场景的x
	int m_nTargetY; //目标场景的y
	int m_nTargetSceneID; //目标场景的场景ID
};


