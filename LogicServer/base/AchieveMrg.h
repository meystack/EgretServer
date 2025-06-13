#pragma once
#include <vector>


//处理离线完成的成就
class CAchieveMrg:
	public CComponent
{

public:

	CAchieveMrg();
	~CAchieveMrg();
public:
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);
	void Load();
	//获取
	void OnEnterGame(CActor* pActor);
    void Save();
    void dealAchieve(CActor* pActor);
    void addOfflineAchieve(unsigned int nActorId, int nAchieveType,int nValue, int nAchieveSubType, int nWay);
private:
	//加载全服邮件
	void LoadAllOffineAchieve();
	
private:
	bool isDatatify; //数据变化
	std::map<unsigned int, std::vector< OffLineAchieveData> >		m_offlineAchieve;
    typedef std::map<unsigned int, std::vector< OffLineAchieveData> >::iterator m_Inter;
};