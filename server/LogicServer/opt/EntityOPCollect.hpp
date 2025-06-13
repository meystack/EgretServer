#pragma once

/************************************************************************/
/* 
/*                            实体对象操作汇总收集系统
/*
/*    实体对象操作汇总处理用于优化处理性能，将在一次逻辑循环中需要多次处理的相同事情集中
/* 到例行逻辑函数即将返回前执行。
/* 例如：
/*  1) 刷新属性，在一次例行逻辑中可能需要多次刷新属性，而使用集中标记则可以只将需要刷新属
/*     性的标志置为真，并在例行函数即将返回前判断标记并刷新属性）
/*
/*  2) 刷新外观，在一次例行逻辑中可能需要多次刷新外观，外观包括衣服、物品的形象，名称、名称
/*     颜色，是否组队等标志
/*
/************************************************************************/

class CEntityOPCollector : public Counter<CEntityOPCollector>
{
public:
	typedef enum CollecteOPType
	{
		coRefAbility = 0,	//重新计算属性
		coRefFeature,		//重新计算外观
		coTransport,       //传送标记，传送以后要向新出生地点旁边的玩家发一个消息
		coResetMaxHPMP,		// 重置最大血量和法力值，用于升级后的更新
		coRefMonsterDynProp,	// 刷新怪物动态属性
		//定义标记数量
		CollectedOperationCount
	};
public:
	CEntityOPCollector()
	{
		Reset();
	}
	//重设汇总标记掩码
	inline void Reset()
	{
		memset(m_Operations, 0, sizeof(m_Operations));
	}
	//设置汇总标记掩码
	inline CEntityOPCollector& Collect(const CollecteOPType eOPType)
	{
		DbgAssert(eOPType >= 0 && eOPType < CollectedOperationCount );
		m_Operations[eOPType/32] |= (1 << (eOPType & 31));
		return *this;
	}
	//判断某个标记掩码是否被设置
	inline bool isCollected(const CollecteOPType eOPType)
	{
		return (m_Operations[eOPType/32] & (1 << (eOPType & 31))) != 0;
	}
protected:
	int	m_Operations[CollectedOperationCount/32+1];
};