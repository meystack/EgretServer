#pragma once
#include "util/Utility.h"


template<typename TPropType>
class CPropertySetOp
{
public:
	template < class T> 
	inline  T GetProperty(int nPropID) const
	{
		T temp=0;
		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return temp;
		return *(T*)set->GetValuePtr(nPropID);
	}

	template < class T> 
	inline VOID SetProperty(int nPropID, const T& value)
	{
		T oldValue = GetProperty<T>(nPropID);
		if(oldValue == value )return;
		CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return ;
		set->SetValue(nPropID,value);
		OnPropertyChange(nPropID,oldValue,value);
	}

	virtual  CPropertySet* GetPropertyPtr()= 0;
	virtual  const CPropertySet* GetPropertyPtr() const = 0;	
	INT_PTR GetPropertySize() const
	{
		return sizeof(TPropType) - sizeof(CPropertySet);
	}

	inline const char*  GetPropertyDataPtr() const
	{
		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return NULL;
		return set->GetValuePtr(0);
	}



	template < class T>
	void  OnPropertyChange(int nPropID, const T& oldValue,const T& value)
	{
	}
};
template<class PropType>
class EntityT : CPropertySetOp<PropType>
{

};

class CClientEntity
{
public:
	CClientEntity()
	{
		m_handle = 0;
		m_szShowName[0] = '\0';
		m_State = 0;
	}

	template < class T> 
	inline  T GetProperty(int nPropID) const
	{
		T temp=0;
		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return temp;
		return *(T*)set->GetValuePtr(nPropID);
	}

	template < class T> 
	inline VOID SetProperty(int nPropID, const T& value)
	{
		T oldValue = GetProperty<T>(nPropID);
		if(oldValue == value )return;
		CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return ;
		set->SetValue(nPropID,value);
		OnPropertyChange(nPropID,oldValue,value);
	}
	inline INT_PTR GetPropertySize() const
	{
		return GetPropertyTotalSize() - sizeof(CPropertySet);
	}
	
	inline const char*  GetPropertyDataPtr() const
	{
		const CPropertySet* set = GetPropertyPtr();
		if(set ==NULL) return NULL;
		return set->GetValuePtr(0);
	}

	template < class T>
	void  OnPropertyChange(int nPropID, const T& oldValue,const T& value)
	{
	}

	inline void SetHandle(EntityHandle hdl) { m_handle = hdl; }
	inline EntityHandle GetHandle() const { return m_handle; }
	inline const char* GetName() const { return m_szShowName; };
	inline size_t GetNameBuffLen() const { return ArrayCount(m_szShowName); }
	inline void SetName(const char* name)
	{		
		CUtility::CopyCharArrayS(m_szShowName, name);
	}
	inline void GetPosition(int &x, int &y) const 
	{
		x = GetProperty<int>(PROP_ENTITY_POSX);
		y = GetProperty<int>(PROP_ENTITY_POSY);
	}
	bool IsMonster(INT_PTR nEntityType)
	{
		switch (nEntityType)
		{
		//case enNpc:
		//case enMovingNPC:
		case enMonster:
		case enGatherMonster:
			return true;
		}

		return false;
	}
	static INT_PTR GetDir(INT_PTR nSourceX, INT_PTR nSourceY, INT_PTR nTargetX, INT_PTR nTargetY)
	{
		INT_PTR nDir = DIR_UP;
		INT_PTR nSelfPosx = nTargetX - nSourceX;
		INT_PTR nSelfPosy = nTargetY - nSourceY;

		if(nSelfPosx >0)
		{
			if( nSelfPosy >0 )
			{
				nDir =DIR_DOWN_RIGHT  ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_RIGHT;
			}
			else
			{
				nDir =DIR_UP_RIGHT ;
			}
		}
		else if(nSelfPosx ==0)
		{
			if( nSelfPosy >0 )
			{
				nDir =DIR_DOWN  ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_STOP;
			}
			else
			{
				nDir =DIR_UP ;
			}
		}
		else
		{
			if( nSelfPosy >0 )
			{
				nDir = DIR_DOWN_LEFT ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_LEFT;
			}
			else
			{
				nDir = DIR_UP_LEFT;
			}
		}
		return nDir;
	}

	inline bool IsDeath()
	{
		if (m_State & 0x01) return true;
		
		return !!(GetProperty<unsigned int>(PROP_CREATURE_STATE) & (0x1 << esStateDeath) );
	}

	inline void SetStateDeath()
	{
		m_State |= 0x01;
	}


protected:
	virtual  CPropertySet* GetPropertyPtr()= 0;
	virtual  const CPropertySet* GetPropertyPtr() const = 0;	
	virtual INT_PTR GetPropertyTotalSize() const = 0;	

private:
	EntityHandle			m_handle;							// 主玩家句柄	
	char					m_szShowName[256];				// 主玩家显示名称（玩家名称+帮派名称+结拜名称等）
	int						m_State;//0x00:Idle 0x01:死亡
};


// 玩家数据
class CClientActor : public CClientEntity
{
public:	
	CClientActor()
	{
	}
	inline int GetSceneId() const { return m_nSceneId; }
	inline byte GetFubenId() const { return m_nFubenId;}
	inline const char* GetSceneName() const { return m_szSceneName; }
	inline const char* GetMapName() const { return m_szMapName; }

	inline void SetSceneId(const int nId)
	{
		m_nSceneId = nId;
	}

	inline void SetFubenId( const int nFbId)
	{
		m_nFubenId = nFbId;
	}

	inline void SetTeamId( int nTeamId )
	{

	}

	inline void SetSceneName(const char* sceneName)
	{
		CUtility::CopyCharArrayS(m_szSceneName, sceneName);
	}

	inline void SetMapName(const char* mapName)
	{
		CUtility::CopyCharArrayS(m_szMapName, mapName);
	}


protected:
	CPropertySet* GetPropertyPtr()
	{
		return &propertySet;
	}
	const CPropertySet* GetPropertyPtr() const
	{
		return &propertySet;
	}
	INT_PTR GetPropertyTotalSize() const
	{
		return sizeof(propertySet);
	}
private:
	CActorProperty			propertySet;					// 主玩家属性集	
	int						m_nSceneId;						// 玩家所在的场景Id
	char					m_szSceneName[32];				// 玩家所在场景名称
	char					m_szMapName[32];				// 场景所在地图名字
	int						m_nOldPosX;						// 备份移动之前的坐标位置
	int						m_nOldPosY;
	BYTE					m_nFubenId;						//是否在副本上
	bool					m_boTeam;						//是否带有队伍标记
};

// 怪物数据
class CClientMonster : public CClientEntity
{	
public:
	CClientMonster()
	{
	}

	void SetMonsterType(BYTE type) { nMonsterType = type; }

	//是否为守卫
	bool IsGuard() const { return nMonsterType == 7; }
	
protected:
	CPropertySet* GetPropertyPtr()
	{
		return &propertySet;
	}
	const CPropertySet* GetPropertyPtr() const
	{
		return &propertySet;
	}
	INT_PTR GetPropertyTotalSize() const
	{
		return sizeof(propertySet);
	}

private:
	CMonsterProperty		propertySet;					// 怪物属性集
	BYTE					nMonsterType;	//怪物类型（1普通2精英3神boss4boss6练功7人形8世界boss）
};

// NPC数据
class CClientNPC : public CClientEntity
{	
public:
	CClientNPC()
	{
	}

protected:
	CPropertySet* GetPropertyPtr()
	{
		return &propertySet;
	}
	const CPropertySet* GetPropertyPtr() const
	{
		return &propertySet;
	}
	INT_PTR GetPropertyTotalSize() const
	{
		return sizeof(propertySet);
	}

private:
	CNPCProperty			propertySet;					// NPC属性集
};

// 宠物数据
class CClientPet : public CClientEntity
{
public:
	CClientPet()
	{
	}

protected:
	CPropertySet* GetPropertyPtr()
	{
		return &propertySet;
	}
	const CPropertySet* GetPropertyPtr() const
	{
		return &propertySet;
	}
	INT_PTR GetPropertyTotalSize() const
	{
		return sizeof(propertySet);
	}

private:
	CPetProperty		propertySet;					// 宠物属性集	
};

// 传送门数据
class CClientTransfer : public CClientEntity
{
public:
	CClientTransfer()
	{
	}

protected:
	CPropertySet* GetPropertyPtr()
	{
		return &propertySet;
	}
	const CPropertySet* GetPropertyPtr() const
	{
		return &propertySet;
	}
	INT_PTR GetPropertyTotalSize() const
	{
		return sizeof(propertySet);
	}

private:
	CTransferProperty		propertySet;					// 传送门属性集	
};
