#pragma  once 
/*
* 机器人管理器，主要的接口放在这个类里
*/
#include "RobotDef.h"
class CRobotManager
{
	
public:
	

	CRobotManager();
	~CRobotManager();

	BOOL Startup();
	VOID Shutdown();
	
	
	inline CBaseList<SERVERCONFIG> &    GetServerConfig() {return m_configs; }   //获取服务器的配置列表,端口,ip
	inline CBaseList<LOGINACCOUNT> &    GetAccountList() {return m_accounts; }  //获取配置的账户的列表
	inline CBaseList<MapScenePoint>&			GetMapPointList() { return m_mapPoints; } // 获取地图分布点配置信息
	
	/*
	* Comments: 获取随机场景Id
	* Param int& nRecommLevel:推荐等级
	* @Return int:
	* @Remark:
	*/
	MapScenePoint* GetRandomSceneId();

	SERVERCONFIG* GetServerPtr();  //获取服务器的配置，如果是一个就返回当前的那个，如果有多个，就从多个配置里选一个
	
	inline int GetServerIndex(){return m_nServerIndex;}  //获取要连接的服务器的编号
	

	//查看各机器人的状态
	void ShowServerAgentCount();

	// 获取版本以及运营商定义对象
	inline CVSPDefinition& GetVSPDefine(){ return m_VSPDefines; }
	// 获取语言包
	inline CLanguageTextProvider& GetTextProvider(){ return m_LanguageText; }
	// 获取全局配置
	inline CConfigLoader& GetGlobalConfig() { return m_configLoader; }
private:
	// 加载配置数据
	bool LoadConfig();
	

public:
	CBaseList<SERVERCONFIG> m_configs;         //服务器的ip,端口的配置列表，支持多个，将从中间随机选一个连接
	CBaseList<LOGINACCOUNT> m_accounts;        //测试账户的列表
	CBaseList<CRobotGroup>  m_robotGroups;     //测试机器人的分组列表,每64个机器人开一个线程
	int						m_groupLoginInterval;//每组登录间隔时间
	CBaseList<MapScenePoint>		m_mapPoints;	   // 地图分布
	
	static CRobotManager * s_pRobotManager;    //全局唯一的机器人管理器的指针
	int                     m_maxAccountCount; //最大的登陆的角色的列表，如果<=0表示不限制
	bool                    m_hasStart;        //是否已经启动成功了
	int                     m_nServerIndex;     //要连接的服务器的编号
	CVSPDefinition			m_VSPDefines;		// 版本和运营商常量定义
	CLanguageTextProvider	m_LanguageText;		// 语言包
	CConfigLoader			m_configLoader;		// 配置加载器
	CHAR					m_sConsoleName[32]; //机器人程序名字
};

//全局唯一的机器人管理器
inline CRobotManager * GetRobotMgr(){ return CRobotManager::s_pRobotManager;}