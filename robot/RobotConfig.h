#pragma once

class CRobotManager;

/*
*读取机器人的配置的文件,负责从配置里读取机器人的各种配置，比如账户列表，服务器的ip地址等东西
*/
class CRobotConfig
	: public CCustomLuaConfig
{
public:
	/*
	* Comments:读取配置
	* Param CRobotManager * lpSSManager: 管理器
	* Param const char * sConfigFile:指定配置文件
	* @Return bool:成功返回true，否则返回false
	*/
	bool loadConfig(CRobotManager *lpSSManager, const char * sConfigFile);


protected:
	void showError(LPCTSTR sError);

	/*
	* Comments:真正读取配置文件
	* Param CRobotManager * lpSSManager:管理器的指针
	* @Return bool:成功返回true，否则返回false
	*/
	bool readConfig(CRobotManager *lpSSManager);

	/*
	* Comments: 读取地图分布点配置
	* Param CRobotManager * lpSSManager:管理器的指针
	* @Return bool:
	* @Remark:
	*/
	bool readMapPoints(CRobotManager *lpSSManager);
};
