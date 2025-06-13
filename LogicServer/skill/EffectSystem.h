#pragma once

/************************************************************************/
/*
/* 实体特效子系统
/*
/************************************************************************/

//实体的特效
struct CEntityEffect
{
	TICKCOUNT nExpiredTick; //过期时间
	WORD wEffectID;   //特效的类型
	BYTE bEffectType; //特效的类型
	BYTE bReServer;
};

class CEffectSystem:
	public CEntitySubSystem<enEffectSystemID,CEffectSystem,CAnimal>
{

public:
	/*
	* Comments: 添加一个特效
	* Param INT_PTR nType: 特效的类型
	* Param INT_PTR nId:特效的id
	* Param INT_PTR nDurTime:特效的持续时间,如果小于0，表示无限时间
	* @Return void:
	*/
	void AddEffect(INT_PTR nType,INT_PTR nId,INT_PTR nDurTime);
	
	
	/*
	* Comments:删除特效
	* Param INT_PTR nType:特效的类型
	* Param INT_PTR nId:特效的ID
	* @Return bool:成功删除返回true，否则返回false
	*/
	INT_PTR DelEffect(INT_PTR nType,INT_PTR nId);


	/*
	* Comments: 删除所有的特效
	* Param bool nNeedBroadCast: 是否需要广播
	* @Return INT_PTR:返回删除的个数
	*/
	INT_PTR DelAllEffect(bool nNeedBroadCast);
	

	//定时检测
	VOID OnTimeCheck(TICKCOUNT nTick);

	//追加特效的数据
	void AppendEffectData(CDataPacket &pack);

	void Destroy()
	{
		DelAllEffect(false);
	}

	/*
	* Comments:是否存在一个特效
	* Param int nEffectType:特效的类型
	* Param int nEffectId:特效的id
	* @Return bool:存在返回true，否则返回false
	*/
	bool IsExist(int nEffectType ,int nEffectId);

private:
	//删除玩家特效的回调
	void OnDeleteEffect(const CEntityEffect * pEffect);
	
private:
	CList<CEntityEffect *> m_effects; //实体的特效
};
