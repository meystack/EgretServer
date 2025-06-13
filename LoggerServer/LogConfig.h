#ifndef _LOG_CONFIG_H_
#define _LOG_CONFIG_H_

#ifndef WIN32

class CCustomLuaConfig;


#include <vector>
using namespace std;


class CLogConfig:
	public CCustomLuaConfig
{
public:
	/*
	* Comments:  读取服务器的配置
	* @Return  char *:返回错误的字符串
	*/
	char *  ReadConfig();
	
	//获取Url地址
	char * GetUrl() {return m_sUrl;}

	//快捷禁言时间
	int GetGag() {return m_nGag;}

	//获取运营商ID
	int GetSid() {return m_nSid;}
	//获取开放接口类型
	int GetNumPost() {return m_nNumPost;}
	//获取转发Url地址
	char * GetPUrl() {return m_sPUrl;}
	//获取游戏标识
	char * GetgKey() {return m_sgKey;}
	//获取签名
	char * GetSign() {return m_sSign;}

	//获取TCP端口
	int GetTPort() {return m_nPort;}
	//获取服务器的列表
	vector<int> & GetSrvList() {return m_serverList;}

	//设置服务器的列表
	inline void SetSrvList(const vector<int> & sSrvLis)
	{
		m_serverList = sSrvLis;
	}

public:
	CLogConfig();
	~CLogConfig();
public:

protected:
	void ShowError(const LPCTSTR sError);

	char m_sUrl[256]; // 地址
	int  m_nGag;	  // 快捷禁言时间

	int	 m_nSid;	  // 运营商ID 0： 360 1：YY 2：4399
	int	 m_nNumPost;  // 推送类型 0： 不推送 列表显示 1：推送 不显示列表 2：推送 显示列表
	char m_sPUrl[256];// 转发地址
	char m_sgKey[16]; // 游戏标识
	char m_sSign[128];// 签名
	int	 m_nPort;	  // TCP 端口
	//要屏蔽的服务器列表
	vector<int> m_serverList;
};

#endif

#endif

