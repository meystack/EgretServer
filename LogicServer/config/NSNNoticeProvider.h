#pragma once

//全局公告
typedef struct tagNSNNotice
{
	int nAfterSrvDay;	// 开服N天后开公告
	int nId;			// 公告id
	int nBeforeSrvDay;	// 开服前N天开公告
    int nInterval;      // 播报间隔（秒）
    bool boIsTrundle;   // 是否走马灯公告
    bool boIsChat;      // 是否发聊天框信息
    //short nTimeType;    // 时间类型
	//CMiniDateTime       nStartTime;	// 开始公告时间
	//CMiniDateTime       nEndTime;	// 结束公告时间
	//CMiniDateTime       nNextNotice;// 下次标记时间
	unsigned int        nNextTime;// 下次标记时间
	short nTimeCount;
	short nTimeIdx; 
    char sContent[300];//公告内容
} NSNNOTICE,*PNSNNOTICE;

class CNSNNoticeProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CNSNNoticeProvider();
	~CNSNNoticeProvider();

	bool LoadAll(LPCTSTR sFilePath);
	bool ReadAllNotices();

public:// 功能接口

	//bool UpdateOpenTime(lua_State * pLuaState, NSNNOTICE& notice, bool nNextTime = false );
    std::map<int,NSNNOTICE>& getNoticeConf() { return m_Notices; }

private: 
    //static 	CBufferAllocator* m_pAllocator; 
	//bool ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out);

	std::map<int, NSNNOTICE> m_Notices;// 公告id：公告配置
};
