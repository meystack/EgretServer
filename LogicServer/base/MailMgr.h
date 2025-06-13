#pragma once
#include <vector>
typedef struct tagServerMailAttach
{
	int			nServerMailId;		//全服邮件id
	ACTORAWARD	actorAward;//奖励配置
	tagServerMailAttach()
	{
		memset(this, 0, sizeof(*this));
	}
}ServerMailAttach;
typedef struct tagServerMail
{
	// int			nServerMailId;		//全服邮件id
	// unsigned int nActorId;			//角色id
	// char			sTitle[200];			//邮件标题
	// char			sContent[600];		//邮件内容
	// int			nMinVipLevel;			//最低vip等级
	// int			nMaxVipLevel;			//最高vip等级
	// int			nMinLevel;			//最低人物等级
	// int			nMaxLevel;			//最高人物等级
	// unsigned int	nMinRegisterTime;		//最早注册时间
	// unsigned int	nMaxRegisterTime;		//最晚注册时间	
	// unsigned int	nMinLoginTime	;		//最早登录时间
	// unsigned int	nMaxLoginTime;		//最晚登录时间
	// int			btOnline;			//在线	0全部，1在线，2不在线
	// int			btSex;               //性别 -1全部，0男， 1女
	// int    nWay;				//用途标识,0系统正常邮件，1后台福利
	ServerMailDef serverData;
	ServerMailAttach mailAttach[MAILATTACHCOUNT];

	tagServerMail()
	{
		memset(this, 0, sizeof(*this));
	}
}ServerMail;
enum eServerMailOnlineType
{
	eMo_All,
	eMo_Online,
	eMo_OffLine
};

class CMailMgr:
	public CComponent
{

public:

	CMailMgr();
	~CMailMgr();
public:
	void RunOne(CMiniDateTime& minidate,TICKCOUNT tick);
	void OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);
	void Load();
	//获取全服邮件
	ServerMail* GetServerMail(int nServerMailId);
	void OnEnterGame(CActor* pActor);
	//删除全服邮件，即设成已完成，不再处理
	void DelServerMail(int nServerMailId);

	//用于测试
	bool TestAddServerMail(ServerMail& serverMail);

	void AddserverMailToDb(ServerMail& serverMail);
private:
	//加载全服邮件
	void LoadNewServerMail();
	//发给指定玩家邮件
	void SendServerMailById(unsigned int nActorId, ServerMail& mail);
	//处理全服邮件
	void HandleAllServerMail();
	//设置已领取标记
	void SetActorGetServerMail(CActor* pActor, int nServerMailId);
	//是否发送过
	bool IsActorGetServerMail(CActor* pActor,int nServerMailId);
	//全服邮件筛选条件
	bool CheckServerMailCond(CActor* pActor, ServerMailDef& mail, bool isLogin = false);
	//设置完成标记
	void SendServerMailComplete(int nServerMailId);

	void SendServerMail();

	
	// unsigned int nActorId;			//角色id
	// char			sTitle[200];			//邮件标题
	// char			sContent[600];		//邮件内容
	// int			nMinVipLevel;			//最低vip等级
	// int			nMaxVipLevel;			//最高vip等级
	// int			nMinLevel;			//最低人物等级
	// int			nMaxLevel;			//最高人物等级
	//  int	nMinRegisterTime;		//最早注册时间
	//  int	nMaxRegisterTime;		//最晚注册时间	
	//  int	nMinLoginTime	;		//最早登录时间
	//  int	nMaxLoginTime;		//最晚登录时间
	// BYTE			btOnline;			//在线	0全部，1在线，2不在线
	// char			btSex;               //性别 -1全部，0男， 1女
	// unsigned int    nWay;				//用途标识,0系统正常邮件，1后台福利
	void AddserverMailToDb(const char * sTitle, const char * sContent, unsigned int nActorId, std::vector<ACTORAWARD> vAwards,int nMinVipLevel = -1, int nMaxVipLevel = -1,
	int nMinLevel = -1, int nMaxLevel = -1,int nMinRegisterTime = -1,int nMaxRegisterTime = -1,int nMinLoginTime = -1,int nMaxLoginTime = -1,int btOnline = 0,int btSex = -1,
	int nWay = 0) ;
private:
	CTimer<60000>           m_1minuteTimer;     //1分钟的定时器
	int					   m_nMaxServerMailId; //最大的服务器邮件id
	CVector<ServerMail>		m_serverMailList;
	unsigned int                     nToDayTime; //当前开服时间
	bool                    m_Init;
};