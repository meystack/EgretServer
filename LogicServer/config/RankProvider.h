#pragma once
#include<algorithm>

class CRankProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

	CRankProvider();
	~CRankProvider();
public:
	//从文件加载标准任务配置
	bool LoadRank(LPCTSTR sFilePath);
protected:
	//以下函数为覆盖父类的相关数据处理函数
	bool readItemData(QuestInfoCfg& item );
	bool ReadRankItem();

private:
	CDataAllocator m_DataAllocator;	//物品属性对象申请器
public:
 
	std::map<int, int> m_RankConfigs;//所有任务
    int nMinRankId;//
};
