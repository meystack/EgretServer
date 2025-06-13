#pragma once
#include <vector>
#include <map>
//成就

typedef struct tagMailCfg
{
    int nId;
    char cTitle[1024];
    char cContent[1024];
    std::vector<ACTORAWARD> vAwards;
    tagMailCfg()
    {
        nId = 0;
        memset(this->cTitle, 0, sizeof(this->cTitle));
        memset(this->cContent, 0, sizeof(this->cContent));
        vAwards.clear();
    }
}MailCfg;


class CMailCfg :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;
public:
	CMailCfg();
	~CMailCfg();
	//加载配置
	bool LoadConfig(LPCTSTR sFilePath);
	bool readConfigs();
    bool readLvMailConfigs();
    bool readOpenDayMailConfigs();
    bool readCombineMailConfigs();
    std::map<int, std::vector<int> >& getCombineMials() {return m_nCombineMails;}
    MailCfg* getOneMailCfg(int nId);
public:

	std::map<int, MailCfg> nALlMails; //
	std::map<int, std::vector<int> > m_lvMails;               // 等级
	std::map<int, std::vector<int> > m_nOpenDayMails; //开服邮件 
    std::map<int, std::vector<int> > m_nCombineMails; //合服邮件 
private:
	CDataAllocator		m_NewDataAllocator;		//对象申请器
};
