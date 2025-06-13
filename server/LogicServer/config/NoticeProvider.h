#pragma once

//全局公告
typedef struct tagGlobalNotice
{
	int nId;			// 公告id
	int nAfterSrvDay;	// 开服N天后开公告
	int nBeforeSrvDay;	// 开服前N天开公告
    int nInterval;      // 播报间隔（秒）
    bool boIsTrundle;   // 是否走马灯公告
    bool boIsChat;      // 是否发聊天框信息
    short nTimeType;    // 时间类型
	CMiniDateTime       nStartTime;	// 开始公告时间
	CMiniDateTime       nEndTime;	// 结束公告时间
	CMiniDateTime       nNextNotice;// 结束公告时间
	short nTimeCount;
	short nTimeIdx;
	OneTimePair* pTimeDetail;	// 时间配置
    char sContent[180];//公告内容
} GLOBALNOTICE,*PGLOBALNOTICE;

class CNoticeProvider :
	protected CCustomLogicLuaConfig
{
public:
	typedef CCustomLogicLuaConfig	Inherited;

public:
	CNoticeProvider();
	~CNoticeProvider();

	bool LoadAll(LPCTSTR sFilePath);
	bool ReadAllNotices();

public:// 功能接口

	bool UpdateOpenTime(lua_State * pLuaState, GLOBALNOTICE& notice, bool nNextTime = false );
    std::map<int,GLOBALNOTICE>& getNoticeConf() { return m_Notices; }

private:

    static 	CBufferAllocator* m_pAllocator;

	bool ReadTime(lua_State * pLuaState, int nTimeType, LPCTSTR sKey, int nKeyLen, CMiniDateTime& out);

	std::map<int,GLOBALNOTICE> m_Notices;// 公告id：公告配置
};
