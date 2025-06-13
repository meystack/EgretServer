
#pragma once
#include "CFilter.h" //屏蔽字库
#include "AcAuto.h"  
/***************************************************************/
/*
/* 聊天管理器
/* 管理屏蔽字符库，管理踢人屏蔽玩家发言的功能
/***************************************************************/

 
class CNoticeManager:
	public CComponent
{

public:
	/*
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
 	*/

	CNoticeManager()
#ifndef USE_HASK_FILTER
		: m_pForbitWords(nullptr)
		, m_pFilterWords(nullptr)
#endif
	{
		m_nPos = 0;
	}
	virtual	bool  Initialize(); //初始化组件

	VOID  Destroy();

	//定期发送全服公告
	VOID RunOne(TICKCOUNT nTickCount);
	 
	/*
	* Comments:重新装载内存管理器
	* Param int nHashFunCount:hash函数的格式
	* Param int nMemory:内存的数目，单位byte
	* @Return bool:成功返回true，否则返回false
	*/
	bool ReloadFilterDb(int nHashFunCount=4,int nMemoryBytes=2500000); //重新装载屏蔽字符库

	//void ParseNotice( LPCTSTR sPtr );


	//************************************
	// Method:    AddNotice 
	//************************************
	bool AddNotice(NSNNOTICE &notice/*int nId, unsigned int nStartTime, unsigned int nEndTime, int nMins, int nDisplayPos, LPCSTR sMemo*/);
	//获取公告
	NSNNOTICE* GetNotice(int nId);
	//根据索引删除notice，该函数时在m_GameNotice正序循环中调用时需注意索引问题
	void RemoveNoticeByIndex(INT_PTR index);
	//void SendNoticeToBackMgr();

	
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
	//void SaveNotice();

	//后台给玩家直接发送离线消息  nActorName 玩家名称 nMsg 消息类容
	void GmSendOffMsgToActor(LPCSTR nActorName,LPCSTR nMsg);
 
	static bool AppendLine2File(const char* pFileName, const char* pLine);
	//获取公告列表
	inline std::vector<NSNNOTICE> & GetNoticeList() { return m_GameNotice;}
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
 
private: 
	
	std::vector<NSNNOTICE>		m_GameNotice;		//全服的公告列表
	int							m_nPos;				//当前播放到第几条
	CTimer<30000>				m_timer;	//半分钟 
	//static int nMax_ChatRecord;
};
