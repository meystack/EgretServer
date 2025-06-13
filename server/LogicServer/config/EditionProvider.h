struct SimulorMapPosInfo
{ 
	int	nSimulorMapSceneId;	//默认场景
	int	nSimulorMinPosX;
	int	nSimulorMinPosY;
	int	nSimulorMaxPosX;
	int	nSimulorMaxPosY;
	SimulorMapPosInfo()
    {
        memset(this, 0 ,sizeof(*this));
    }
};

struct tagEditionInfo
{
    int          nStart;         //是否启用交易额度
    int 		 nJiaoYiEDuRate; //充值元宝转化交易额度比例
    int          nProportion;    //元宝和交易额度的消耗比例(此处为百分比)
	int			 nPrivilege;	 //达到特定等级后不扣除交易额度
	int          nNoSetOpenDay;	 //老区多少天后限制新账号建立角色
	std::vector<int> nSecondChargeIds;//二充对应的活动id
	std::vector<int> vecCreateRoleAutoAddTitleId;//创角自动携带 称号Id
	std::vector<int> vecServerNameId;//区服冠名 活动Id

	//假人配置
	bool		canSimulor;	 	//虚拟人开启
	int			nSimulorSize;	//虚拟人数量
	std::vector<SimulorMapPosInfo> nSimulorSceneInfo; 

	tagEditionInfo()
    {
        memset(this, 0 ,sizeof(*this));
    }		
};


class CEditionProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig	Inherited;
	 
public:
	CEditionProvider();
	~CEditionProvider();

	bool Load(LPCTSTR sFilePath);
	bool ReadConfig(LPCTSTR sFileName);

public:// 功能接口
	tagEditionInfo &GetEditionInfo() { return m_EditionInfo; };
	
	bool GetCanSimulor() { return m_EditionInfo.canSimulor; };
	int GetSimulorSize() { return m_EditionInfo.nSimulorSize; };
	std::vector<SimulorMapPosInfo>& GetSimulorInfo() { return m_EditionInfo.nSimulorSceneInfo; };

private: 
    static 	CBufferAllocator* m_pAllocator; 

	tagEditionInfo m_EditionInfo;
};
