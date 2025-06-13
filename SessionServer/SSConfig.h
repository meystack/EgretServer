#pragma once

class CSSManager;
extern std::string g_ConfigPath;
class CSSConfig
	: public CCustomLuaConfig
{
public:
	bool loadConfig(CSSManager *lpSSManager);

	//读取GM配置
	bool readGMConfig(CSSManager *lpSSManager);

	

protected:
	void showError(LPCTSTR sError);
	bool readConfig(CSSManager *lpSSManager);

	
	void AddAlwayWhiteIp(CBaseList<unsigned long long > & ips);

};
