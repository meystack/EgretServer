
#pragma once
#include "CFilter.h" //屏蔽字库
#include "AcAuto.h"  
/***************************************************************/
/*
/* 聊天管理器
/* 管理屏蔽字符库，管理踢人屏蔽玩家发言的功能
/***************************************************************/
enum
{
	eCS_CHATRECORDSREQ_NULL = 0,
	eCS_CHATRECORDSREQ_BEGIN = 100,
	eCS_CHATRECORDSREQ_MID = 99,
};
enum
{
	eCS_CHATRCS_TYPE_NULL = 0,
	eCS_CHATRCS_TYPE_SYS = 1,//原系统消息
	eCS_CHATRCS_TYPE_FIGHT = 2,//战斗消息
};


class CChatManager:
	public CComponent
{

public:
	typedef struct NoticeData
	{
		char* sMsg;			 //公告内容
		int   nPos;			 //公告位置
		int	  nId ;			 //公告id
		int		nMins;		  //时间间隔
		unsigned int nStartTime;//开始时间
		unsigned int	nEndTime;//结束时间
		//下面的数据不需保存
		unsigned int nHitTime;//下次触发的时间
		NoticeData()
		{
			memset(this,0,sizeof(*this));
		}
	}NOTICEDATA;

	typedef struct ChatRecord
	{ 
		union
		{
			struct
			{
				LONGLONG llId;
			};

			struct
			{
				unsigned int msgTime;	 //创建时间 
				int intVal;
			};
		}; 
		byte msgType;			 //有系统消息 和 战斗消息
		int msgCount;			 //内容长度
		char msg[1024];			 //公告内容
		ChatRecord()
		{
			memset(this,0,sizeof(*this));
		}
	}CHATRECORD;

	CChatManager()
#ifndef USE_HASK_FILTER
		: m_pForbitWords(nullptr)
		, m_pFilterWords(nullptr)
#endif
	{
		m_nPos = 0;
	}
	virtual	bool  Initialize(); //初始化组件

	static bool ChatCsRecordSort(CHATRECORD tmp1, CHATRECORD tmp2);

	VOID  Destroy();

	//定期发送全服公告
	VOID RunOne(TICKCOUNT nTickCount);
	
	/*
	* Comments:屏蔽一个字符
	* Param INOUT char * pInput:输入的字符串
	* @Return int:返回屏蔽了的字符的个数
	*/
	int  Filter( char *pInput);

	/*
	* Comments: 一个字符串是否和谐
	* Param char * pInput:字符串指针
	* @param nMode 1敏感词库 2 禁言库
	@param pLen 返回匹配的长度，
	* @Return bool:和谐返回true，否则返回false
	*/
	bool IsStrInvalid(char * pInput, UINT_PTR nMode = 1, int* pLen = nullptr);

	/*
	* Comments:重新装载内存管理器
	* Param int nHashFunCount:hash函数的格式
	* Param int nMemory:内存的数目，单位byte
	* @Return bool:成功返回true，否则返回false
	*/
	bool ReloadFilterDb(int nHashFunCount=4,int nMemoryBytes=2500000); //重新装载屏蔽字符库

	void ParseNotice( LPCTSTR sPtr );


	//************************************
	// Method:    AddNotice
	// Comments:  根据id新加一个公告，如果已经存在，则更新
	// Parameter: int nId 公告id
	// Parameter: unsigned int nStartTime 开始时间
	// Parameter: unsigned int nEndTime 结束时间
	// Parameter: int nMins 间隔时间
	// Parameter: INT_PTR nDisplayPos 显示位置
	// Parameter: LPCSTR sMemo 公告
	//************************************
	bool AddNotice(int nId, unsigned int nStartTime, unsigned int nEndTime, int nMins, int nDisplayPos, LPCSTR sMemo);
	//获取公告
	NOTICEDATA* GetNotice(int nId);
	//根据索引删除notice，该函数时在m_GameNotice正序循环中调用时需注意索引问题
	void RemoveNoticeByIndex(INT_PTR index);
	void SendNoticeToBackMgr();

	
	/*
	* Comments:删除指定的公告
	* Param LPCSTR sMemo:必须全文匹配才能成功删除
	* @Return void:
	*/
	bool DeleteNotice(LPCSTR sMemo);
	
	/*
	* Comments:保存公告到文件中
	* @Return void:
	*/
	void SaveNotice();

	//后台给玩家直接发送离线消息  nActorName 玩家名称 nMsg 消息类容
	void GmSendOffMsgToActor(LPCSTR nActorName,LPCSTR nMsg);

	//后台添加屏蔽字  strWords:屏蔽字，nParam：1表示添加，2 表示删除  nTag: 1 表示添加或删除发言的屏蔽字 2 表示添加或删除创建角色的屏蔽字
	void MgrAddFilterWords(char* strWords,int nParam,int nTag,int nRecog);
	bool AddKeyword( char* pKeyword, int mode);
	static bool AppendLine2File(const char* pFileName, const char* pLine);
	//获取公告列表
	inline CVector<NOTICEDATA> & GetNoticeList() { return m_GameNotice;}
	/*
	* Comments:数据服务器返回数据
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/
	VOID OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);	

	/*
	* Comments:后台删除所有公告
	* @Return void:
	*/
	bool DeleteAllNotice();

	/*
	* Comments:根据id删除指定的公告
	* Param id
	* @Return void:
	*/
	bool DeleteNoticeById(int nId);

	//聊天禁言ip次数
	int GetChatShutUpIpCount(unsigned long);
	//ip禁言次数增加
	void AddChatShutUpIpCount(unsigned long nIp);
	void AddChatRecord(const char* msg, int nSize);
	void sendHistoryChatMsg(CActor* pActor);

	void addShowItem(CUserItem* pUserItem);
	void GetShowItem(CUserItem& userItem, LONG64 iid);
	
	//系统聊天信息入包
	void OnMakeChatRecordPacket(CDataPacket &pdata, int nPageNum, UINT nNow);
	void OnAddChatRecordPacketByCs(CDataPacketReader &pdata); 
	bool AddChatCsRecord(CHATRECORD& Record);
	std::vector<CHATRECORD>& GetChatCsRecord(){ return m_nChatCsRecords;};

	//void AddFightRecord(const char* msg, int nSize);	//跨服战斗消息，逻辑服存储 
	void OnBroadTipToBranch(CHATRECORD& reocrd);		//及时发送系统提示

	bool OnCsAddPkMsgRecord(CDataPacketReader &pdata);//收到跨服来的系统消息 
	bool OnCsAddSysMsgRecord(CDataPacketReader &pdata);//收到跨服来的系统消息
	void OnSendHistoryCsSysMsg(CActor* pActor);  
	bool OnSendPkCsChat(CHATRECORD& record);
	bool OnSendCsSysChat(CHATRECORD& record);//系统消息 需要解包
	
	bool OnCheckCanMsg();//是否可以跨服消息
 
	bool OnBackClearAllMsg();//后台清理消息
private:

	//2个屏蔽字管理器，根据defile选取
#ifdef USE_HASK_FILTER
	CFilter                     m_hashFilter;       ///< 屏蔽字
	CFilter						m_forbitWards;		///< 禁言
#else
	PFT_MBCS_CHAR				m_pFilterWords;		///< 屏蔽词列表
	PFT_MBCS_CHAR				m_pForbitWords;		///< 禁言词列表
#endif
	
	CVector<NOTICEDATA>			m_GameNotice;		//全服的公告列表
	int							m_nPos;				//当前播放到第几条
	CTimer<60000>				m_timer;	        //1分钟
	//CTimer<1800000>				m_HalfTimer;	    //半个小时
	CTimer<60000>				m_HalfTimer;	    //半个小时	
	std::map<unsigned long,int>	m_chatShutUpIpCountMap;//ip聊天禁言次数记录
	std::vector<CHATRECORD>     m_nChatRecords;     //
	std::vector<CUserItem>      m_nShowItem;        //展示物品
	static int nMax_ChatRecord;
	
	//新增跨服消息
	std::vector<CHATRECORD>     m_nChatCsRecords;   
	std::map<LONGLONG, unsigned int> 	m_mapChatCs;
	//std::vector<CHATRECORD>     m_nCsFightRecords;   
};
