#pragma once

/****************************************
	提供给脚本保存全局数据的组件
*****************************************/


class CGlobalVarMgr:
	public CComponent
{
public:

	/*
	* Comments:从db读取数据
	* @Return VOID:
	*/
	VOID Load();

	/*
	* Comments:定期检查
	* @Return VOID:
	*/
	VOID RunOne();

	//处理DB数据
	void OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);

	void DelDbRetData(INT_PTR nCmd, void* data, size_t len);
	//获取小于等于当前转生的人数
	int GetCountByCircleLevel(int nCircleLevel);
	//当前转生人数+1
	void AddCountByCircleLevel(int nCirclrLevel, int nCount = 1);
	//当玩家转生阶数+1  上一转人数 就需要-1
	void SubCountByCircleLevel(int nCircleLevel);
	//存盘
	void Save();

	inline void Destroy() 
	{ 
		if(m_hasModify)
		{
			Save();
		}
		m_Var.clear(); 
	}

	//获取变量对象
	//调用过一次就默认这个修改过
	inline CCLVariant& GetVar() 
	{ 
		m_hasModify =true;
		return m_Var; 
	}  

public:
	CGlobalVarMgr();
	~CGlobalVarMgr();
	std::map<int, int>          m_CircleMap;  //转生人数次数
	
private:
	CTimer<3600000>				m_timer;	//
	CCLVariant					m_Var;
	bool						m_boInited;	//是否已经读入数据
	CDataPacket                 m_saveDataPack;
	bool                        m_hasModify ;  //是否修改过，如果没有修改过就不存盘了
};