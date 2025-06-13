#pragma once
//翅膀

//星级的配置
typedef struct tagLevelConfig
{
	int		nLevel;					//等级
	int		nCount;					//属性数量
	int     nLuck;					//升级幸运值
	PGAMEATTR PropList;				//加的基础属性
	tagLevelConfig()
	{
		memset(this,0,sizeof(*this));
	}
}STARCONFIG;

//总配置
typedef struct tagWingConfig
{
	int nCount;							//一共多少级
	tagLevelConfig *nWing;				//各级的信息
	tagWingConfig()
	{
		memset(this,0,sizeof(*this));
	}
}WINGCONFIG;

class CWingsAngel :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;
public:
	CWingsAngel();
	~CWingsAngel();
	//加载配置
	bool LoadConfig(LPCTSTR sFilePath);
	bool readConfigs();
public:
	WINGCONFIG		m_Config;					//翅膀各级的属性
private:
	CDataAllocator			m_NewDataAllocator;	//对象申请器
};