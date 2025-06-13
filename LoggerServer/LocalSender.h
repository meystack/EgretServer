#ifndef			_LOCALLOG_SENDER_H_
#define			_LOCALLOG_SENDER_H_

#ifndef WIN32

#include <string.h>
#include <curl/curl.h>

enum eRequestType
{
	enGet = 1,
	enPost,
};

class CCurlHttpRequest
{
public:
	typedef void (*SelfCallBack)(int para1, int para2, int para3, int para4);
public:
	CCurlHttpRequest();
	~CCurlHttpRequest();

	bool SetUrl(const char * url);
	bool SetRequestType(eRequestType rtype);
	bool SetCallBackParam(int para1, int para2, int para3, int para4);
	bool SetResponseCallback(SelfCallBack selfCallBack, void * curlHttpRequest);
	bool SetRequestData(char * pData);
	bool Run();
	
private:
	CURL *m_Curl;
	eRequestType m_RequestType;
	int m_para1, m_para2, m_para3, m_para4;
	SelfCallBack m_SelfCallBack;
	void * m_CurlHttpRequest;
	std::string m_Context;
	std::string m_Header;
};


class CLocalSender
{
public:
	CLocalSender();
	~CLocalSender(void);
	bool IsInUse(){return m_bUseFlag;}

	void SendHttpPostRequest(int nType, char *sUrl, char* buffer);

	void SendMsgToSP(int nServerIndex,int nChannleID,int nActorId,char* sActorName,char* msg);

protected:
	bool Connect();
	void RealSend(char * str );
	void Send(char * str);

	void ConvertMyTo360(int sidx, int cid, const char * uid, const char * sUser, const char *sMsg);
	void ConvertMyToYyg(int sidx, int cid,  const char * sUser, const char *sMsg);
	void ConvertMyTo4399(int sidx, const char * uid, const char * sUser, const char *sMsg, const char *sIp);
	void ConvertMyTo91w(int sidx, int cid, const char * uid, const char * sUser, const char *sMsg, const char *sIP);
	void ConvertMyTo37w(int sidx, const char * uid, const char * sUser, int userid, const char *sMsg, const char *sIP);
private:
	CLogConfig  m_config;
	SOCKET m_nSocket;
	unsigned short	m_nPort;
	unsigned int	m_nIP;
	bool m_bConnect;
	bool m_bUseFlag;
};

#endif

#endif
