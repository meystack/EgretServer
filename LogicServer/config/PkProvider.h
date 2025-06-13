#pragma once

enum tagPkType
{
	//敌对阵营
	ptEnemyZY = 0,
	//中立阵营（同盟）
	ptNeutralZY,
	//同阵营玩家	1
	ptSameZY ,
	//同帮派玩家	1.2
	ptSameGuild,
	//好友	1.2
	ptFriends,		
	//结拜、夫妻	1.5
	ptMarry,
	ptBrother ,
	//师傅	2
	ptMaster,

	ptPkTypeCOunt,

};

enum tagZyType
{
	//敌对阵营玩家
	ztEnemyZY = 0,
	//中立阵营（同盟）
	ztNeutralZY ,
	//同阵营玩家
	ztSameZY ,

	ztZyTypeCount,
};

struct tagOtherPK
{
	INT_PTR					nStart;
	INT_PTR					nEnd;
	double					fValue;
};

typedef  struct tagOtherPKList
{
	INT_PTR					nCount;
	tagOtherPK*				pList;
}OtherPKList;

typedef  struct tagExploitTitle
{
	int						nExploitValue;//需要的战绩值
	char					titleName[64];//称号名称
	bool					isBroadMsg;	//是否播放系统信息
	tagExploitTitle()
	{
		memset(this, 0, sizeof(*this));
	}
}ExploitTitle;

class CPkProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CPkProvider();
	~CPkProvider();

	//从文件加载配置
	bool LoadPkConfig(LPCTSTR sFilePath);

protected:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);
	bool readConfigs();
public:
	INT_PTR					pkSubTime;////配置需要多少秒减少一点杀戮值，默认3*60 = 3 min
	INT_PTR					redName;
	int						m_pkValue;			//杀死一个增加的pk值
	int						m_nSubPkValue;		//求情一次减多少PK值
	int						m_nCountPkValue;	//一天最多求情多少次
	int						m_ClearTime;		//褐名清除掉的时间
	int						m_nPrisonSceneId;	//监狱场景id
	int						m_nPrisonPosX;
	int						m_nPrisonPosY;
	int						m_nPrisonWeight;
	int						m_nPrisonHeight;
private:
	CDataAllocator			m_DataAllocator;	//对象申请器
};


