#pragma once

/************************************************************************/
/* 
/*                           角色动态变量子系统
/* 
/*   角色动态变量主要用于向脚本提供存放角色数据。这些数据也可以直接被C++读取和修改
/* 
/* 
/************************************************************************/

//*** 角色变量基础类 ***/
template<int SUBSYSTEMID,class SonClass, class EntityClass>
class CCustomEntityVarSubSystem :
	public CEntitySubSystem<SUBSYSTEMID, SonClass, EntityClass>
{
public:
	typedef CEntitySubSystem<SUBSYSTEMID, SonClass, EntityClass> Inherited;

public:
	//获取变量对象
	inline CCLVariant& GetVar() { return m_Var; }

public:
	//初始化
	inline bool Initialize(void *data,SIZE_T size)
	{
		if (!Inherited::Initialize(data, size))
			return false;
		m_Var.clear();
		return true;
	}
	//析构
	inline void Destroy() { m_Var.clear(); }

protected:
	CCLVariant	m_Var;//根变量
};

//*** 玩家变量类，存DB ***/
class CActorVarSubSystem :
	public CCustomEntityVarSubSystem<enActorVarSystemID, CActorVarSubSystem, CActor>
{
public:
	typedef CCustomEntityVarSubSystem<enActorVarSystemID, CActorVarSubSystem, CActor> Inherited;

public:
	//处理DB数据
	void OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);
	//存盘
	void Save(PACTORDBDATA  pData);
};
