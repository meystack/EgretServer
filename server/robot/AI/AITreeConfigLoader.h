
class CAITreeConfigLoader : public CCustomLogicLuaConfig
{
public:

	CAITreeConfigLoader();
    void Run(CRobotClient* pActor, TICKCOUNT);

	bool Load(LPCTSTR);
	
protected:

	void showError(LPCTSTR sError);

	bool LoadAITreeConfig();
	bool LoadSubTree(int pid, std::vector<AINodeDescribe>&);
	AINodeBase * m_AIRootNode;
};