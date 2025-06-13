#ifndef WIN32

#include "stdafx.h"
#include "LocalSender.h"
#include "md5.h"
#include <time.h>
//#include <sys/timeb.h>

using namespace jxSrvDef;
using namespace wylib::stream;

CCurlHttpRequest::CCurlHttpRequest()
{
	m_Curl = NULL;
	m_SelfCallBack = NULL;
	m_para1=m_para2=m_para3=m_para4 = 0;
	m_CurlHttpRequest = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	m_Curl = curl_easy_init();
	if( ! m_Curl )
	{
		OutputMsg(rmError, _T("Init CURL fail !!!"));
	}
}

CCurlHttpRequest::~CCurlHttpRequest()
{
	if( m_Curl )
		curl_easy_cleanup(m_Curl);
	curl_global_cleanup();
}

bool CCurlHttpRequest::SetUrl(const char * url)
{
	if( m_Curl ){
		curl_easy_setopt(m_Curl, CURLOPT_URL, url);
		return true;
	}
	else 
		return false;
}

bool CCurlHttpRequest::SetRequestType(eRequestType rtype)
{
	m_RequestType = rtype;
	return true;
}

bool CCurlHttpRequest::SetCallBackParam(int para1, int para2, int para3, int para4)
{
	m_para1 = para1, m_para2 = para2, m_para3 = para3, m_para4 = para4;
	return true;
}

bool CCurlHttpRequest::SetResponseCallback(SelfCallBack selfCallBack, void * curlHttpRequest)
{
	m_SelfCallBack = selfCallBack;
	m_CurlHttpRequest = curlHttpRequest;
	return true;
}

bool CCurlHttpRequest::SetRequestData(char * pData)
{
	if(m_RequestType == enPost )
	{
		curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, pData);
		return true;
	}
	else
		OutputMsg(rmError, _T("还没设置发送类型 !!!"));
	return false;
}
void OnRecvData(const char *pData)
{
	if (!strstr(pData,"status\":1"))
	{
		OutputMsg(rmError,"chat push error %s",pData);
	}
}
static size_t CallWirteFunc(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len  = size * nmemb;
	stream->append(ptr, len);
	OnRecvData(ptr);
	return len;
}

static size_t HeaderCallBack(const char  *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len  = size * nmemb;
	stream->append(ptr, len);
	return len;
}

bool CCurlHttpRequest::Run()
{
	curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, CallWirteFunc);
	curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &m_Context);
	//抓取头信息，回调函数
	curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, HeaderCallBack );
	curl_easy_setopt(m_Curl, CURLOPT_HEADERDATA, &m_Header);

	CURLcode res;
	res = curl_easy_perform(m_Curl);
	if( res == CURLE_OK )
	{
		if(m_SelfCallBack)
			m_SelfCallBack(m_para1, m_para2, m_para3, m_para4);
		m_Context.clear();
		m_Header.clear();
	}
	else
	{
		if( res == CURLE_UNSUPPORTED_PROTOCOL )
			OutputMsg(rmError, _T("不支持的协议，由URL的头部指定"));
		else if( res == CURLE_COULDNT_CONNECT )
			OutputMsg(rmError, _T("不能连接到remote 主机或者代理"));
		else if( res == CURLE_REMOTE_ACCESS_DENIED )
			OutputMsg(rmError, _T("访问被拒绝"));
		else if( res == CURLE_HTTP_RETURNED_ERROR )
			OutputMsg(rmError, _T("Http返回错误"));
		else if( res == CURLE_READ_ERROR )
			OutputMsg(rmError, _T("读本地文件错误"));
		else if( res == CURLE_SSL_CACERT )
			OutputMsg(rmError, _T("访问HTTPS时需要CA证书路径"));
		else 
			OutputMsg(rmError,curl_easy_strerror(res));
		return false;
	}
	return true;
}





CLocalSender::CLocalSender()
{
	m_bConnect = false;
	m_bUseFlag = false;
	m_nSocket = 0;
	m_nPort = 0;
	m_nIP = 0;
	char* str = m_config.ReadConfig();
	if( str )
	{
		OutputMsg(rmError, _T("preprocess script file Config.txt failed. Error msg =[%s]"), str);
	}
	else
	{
		m_bUseFlag = true;
		if(m_config.GetSid() == 2 )
		{
			if( Connect() )
				m_bConnect = true;
		}
	}
}

CLocalSender::~CLocalSender(void)
{
	if(m_nSocket > 0)
	{
		close(m_nSocket);
		m_nSocket = 0;
	}
}

bool CLocalSender::Connect()
{
	int nRet = (SOCKET) socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if(nRet)
	{
		m_nSocket = nRet;
	}
	else
	{
		OutputMsg(rmError, _T("Create Socket Error"));
		return false;
	}
	m_nIP = ( m_config.GetUrl() != NULL ) ? inet_addr( m_config.GetUrl() ) : htonl( INADDR_ANY );
	m_nPort = m_config.GetTPort();

	if(m_nPort <= 0)
	{
		OutputMsg(rmError, _T("Port Error"));
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons( m_nPort );
	addr.sin_addr.s_addr = m_nIP;

	if( connect( m_nSocket, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
	{
		OutputMsg( rmError, _T("bind failed errno = [%d], IP = [%s], Port = [%d]"), GetLastError(), m_config.GetUrl(), m_nPort );
		close(m_nSocket);
		return false;
	}

	return true;
}

void CLocalSender::RealSend(char * str )
{
	if( m_bConnect )
	{
		int now  = strlen(str);
		int succ = 0;
		int count = 0;
		while( succ < now )
		{
			int size = send( m_nSocket, &str[succ], now - succ, MSG_DONTWAIT );
			if( size == 0 )
			{
				m_bConnect = false;
				close(m_nSocket);
				return ;
			}
			else if ( size < 0 )
			{
				if ( errno == EAGAIN && errno == ENOBUFS )
				{
					count++;
					if(count  > 5)
					{
						if( now - succ > 0)
							OutputMsg( rmWaning, _T("4399端接受缓慢，聊天消息没完全发送成功") );
						break;
					}
					continue;
				}
				break;
			}
			succ += size;
		}
	}
	else
	{
		OutputMsg( rmError, _T("4399端已经关闭，聊天消息没有发送") );
	}
}

void CLocalSender::Send(char * str )
{
	if( m_bConnect )
	{
		RealSend(str);
	}
	else
	{
		if( Connect() )
		{
			m_bConnect = true;
			RealSend(str);
		}
	}
	
}

void CLocalSender::SendHttpPostRequest(int nType, char *sUrl, char* buffer)
{
	CCurlHttpRequest* request = new CCurlHttpRequest();
	request->SetUrl(sUrl);
	request->SetRequestType(enPost);
	request->SetRequestData(buffer);
	request->SetCallBackParam(nType,0,0,0); //设置一些参数
	request->Run();
	delete request;
}

static char * CharIdToStrEx(int ncharid)  
{
	switch(ncharid)
	{
		case 0:	return _T("【私聊】");  break;
		case 1:	return _T("【同屏】");  break;
		case 2:	return _T("【传音】");  break;
		case 3:	return _T("【行会】");  break;
		case 4:	return _T("【组队】");  break;
		case 5:	return _T("【团队】");  break;
		case 6:	return _T("【地图】");  break;
		case 7:	return _T("【提示】");  break;
		case 8:	return _T("【阵营】");  break;
		case 9:	return _T("【系统】");  break;
		case 10: return _T("【世界】"); break;
		case 11: return _T("【呼救】"); break;
		case 12: return _T("【出售】"); break;
		case 13: return _T("【好友】"); break;
		default: return _T("频道");	break;
	}
}

static void ReplaceAllDistinct(string&   str,const   string&   old_value,const   string&   new_value)     
{     
	for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {     
		if(   (pos=str.find(old_value,pos))!=string::npos   )     
			str.replace(pos,old_value.length(),new_value);     
		else   break;     
	}
}    

static void Remove(string & str, char ch)
{
	string::iterator it;
	for (it =str.begin(); it != str.end(); ++it)
	{
		if ( *it == ch)
		{
			str.erase(it);
		}
	}
}

const char HEX2DEC[256] = 
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

	/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

static std::string URLDecode(const std::string & sSrc)
{
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
		*pEnd++ = *pSrc++;

	std::string sResult(pStart, pEnd);
	delete [] pStart;
	return sResult;
}

// Only alphanum is safe.
const char SAFE[256] =
{
	/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
	/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

	/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
	/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
	/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
	/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

	/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

static std::string URLEncode(const std::string & sSrc)
{
	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc)
	{
		if (SAFE[*pSrc]) 
			*pEnd++ = *pSrc;
		else
		{
			// escape this char
			if(isalnum(*pSrc) || '-'==*pSrc || '_'==*pSrc || '.'==*pSrc)
				*pEnd++ = *pSrc;
			else if(isspace(*pSrc))
				*pEnd++ = '+';
			else
			{
				*pEnd++ = '%';
				*pEnd++ = DEC2HEX[*pSrc >> 4];
				*pEnd++ = DEC2HEX[*pSrc & 0x0F];
			}
		}
	}

	std::string sResult((char *)pStart, (char *)pEnd);
	delete [] pStart;
	return sResult;
}

//360
void CLocalSender::ConvertMyTo360(int sidx, int cid, const char * uid, const char * sUser, const char *sMsg)
{  
	time_t tt;
	time(&tt);

	std::string sStr;

	std::string sAccname(uid);
	int nPos = sAccname.find_last_of('_');
	if( nPos > 0 )
		sAccname.erase(nPos, sAccname.length());

	int ncharid = 0;
	switch(cid)
	{
		case  0:  ncharid = 1;	break;
		case  1:  ncharid = 8;	break;
		case  2:  ncharid = 2;	break;
		case  3:  ncharid = 6;	break;
		case  4:  ncharid = 7;	break;
		case  10:  ncharid = 4;	break;
		default:	
			ncharid = 9;	break;
	}

	int i = tt;
	sStr = sMsg;
	ReplaceAllDistinct(sStr, _T("%"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x13"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x10"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\r\n"), _T("*"));
	Remove(sStr, _T('\r'));
	Remove(sStr, _T('\n'));

	char signCheck[512];
	sprintf_s(signCheck, sizeof(signCheck), _T("%sS%d%s%s%d%s%d%s"), m_config.GetgKey(),sidx,sAccname.c_str(),sUser,ncharid,sStr.c_str(),i,m_config.GetSign());
	unsigned char md5string[64];
	MD5_CTX   ctx;
	MD5Init(&ctx); 
	MD5Update(&ctx,(unsigned char *)signCheck, (unsigned int)strlen(signCheck) );
	MD5Final(md5string,&ctx);
	md5string[32]=0;

	// 如果 sUser 和 sMsg 编码有问题转成URLEncode 编码即可。 
	char body[512];
	sprintf_s(body, sizeof(body), _T("&gkey=%s&server_id=S%d&qid=%s&name=%s&type=%d&toqid=&toname=&content=%s&time=%d&ip=&sign=%s"),
	m_config.GetgKey(), sidx, sAccname.c_str(), URLEncode(sUser).c_str(), ncharid, URLEncode(sStr).c_str(), i, (char*)md5string);

	//m_config.GetPUrl(), body, strlen(body), true
	SendHttpPostRequest(enPost, m_config.GetPUrl(), body );
}

//yy
void CLocalSender::ConvertMyToYyg(int sidx, int cid,  const char * sUser, const char *sMsg)
{  
	//CStringA sdata, body, body2, sSay;
	std::string sdata, body2, sSay;
	int ntype = 1;
	if (cid != 0)
	{
		ntype = 2;
	}

	//body2 =  CLogCheckApp::Convert(CharIdToStrEx(cid),CP_ACP, CP_UTF8);
	body2 = CharIdToStrEx(cid);
	body2 += string(sMsg);

	char body[512];
	sprintf_s(body, sizeof(body),"&to_user_role_name=no&message_type=%d&game_scene_id=0&server_id=%d&game_id=%s",
				ntype,sidx,m_config.GetgKey());
	sdata = body;
	sdata += "&from_user_role_name=";
	sdata += URLEncode(sUser);
	sdata += "&chat_content=";
	sdata += URLEncode(body2);
	//theApp.SendHttpPostRequest(m_config.GetPUrl(),sdata.c_str(), sdata.length(), true); 
	SendHttpPostRequest(enPost, m_config.GetPUrl(), body );
}  

//4399
void CLocalSender::ConvertMyTo4399(int sidx, const char * uid, const char * sUser, const char *sMsg, const char *sIp)
{
	if( m_bConnect )
	{
		std::string sAccname(uid);
		int nPos = sAccname.find_last_of('_');
		if( nPos > 0 )
			sAccname.erase(nPos, sAccname.length());

		char body[512];
		sprintf_s(body, sizeof(body),"%s|S%d|%s|%s|%s|%s\r\n", m_config.GetgKey(),sidx,sAccname.c_str(),sUser,sIp,sMsg);

		Send(body);
	}

}

//91wan
void CLocalSender::ConvertMyTo91w(int sidx, int cid, const char * uid, const char * sUser, const char *sMsg, const char *sIP)
{  

	time_t tt;
	time(&tt);

	std::string sStr;

	std::string sAccname(uid);
	int nPos = sAccname.find_last_of('_');
	if( nPos > 0 )
		sAccname.erase(nPos, sAccname.length());

	int ncharid = 0;
	switch(cid)
	{
	case  0:  ncharid = 1;	break;
	case  1:  ncharid = 8;	break;
	case  2:  ncharid = 2;	break;
	case  3:  ncharid = 6;	break;
	case  4:  ncharid = 7;	break;
	case  10:  ncharid = 4;	break;
	default:	
		ncharid = 9;	break;
	}

	int i = tt;
	sStr = sMsg;
	ReplaceAllDistinct(sStr, _T("%"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x13"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x10"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\r\n"), _T("*"));
	Remove(sStr, _T('\r'));
	Remove(sStr, _T('\n'));

	char signCheck[512];
	sprintf_s(signCheck, sizeof(signCheck), "%sS%d%s%s%d%s%d%s%s",m_config.GetgKey(),sidx,sAccname.c_str(),URLEncode(sUser).c_str(),ncharid,URLEncode(sStr).c_str(),i,sIP,m_config.GetSign());
	unsigned char md5string[64];
	MD5_CTX   ctx;
	MD5Init(&ctx); 
	MD5Update(&ctx,(unsigned char *)signCheck, (unsigned int)strlen(signCheck) );
	MD5Final(md5string,&ctx);
	md5string[32]=0;

	// 如果 sUser 和 sMsg 编码有问题转成URLEncode 编码即可。 
	char body[512];
	sprintf_s(body, sizeof(body), "&gkey=%s&tkey=1&server_id=S%d&qid=%s&name=%s&type=%d&content=%s&time=%d&ip=%s&sign=%s",
		m_config.GetgKey(), sidx, uid, URLEncode(sUser).c_str(), ncharid, URLEncode(sStr).c_str(), i, sIP, md5string);
	//m_config.GetPUrl(),body, strlen(body), true
	SendHttpPostRequest(enPost, m_config.GetPUrl(), body );
}

//37wan
void CLocalSender::ConvertMyTo37w(int sidx, const char * uid, const char * sUser, int userid, const char *sMsg, const char *sIP)
{  
	time_t tt;
	time(&tt);

	std::string sStr;

	std::string sAccname(uid);
	int nPos = sAccname.find_last_of('_');
	if( nPos > 0 )
		sAccname.erase(nPos, sAccname.length());

	int i = tt;
	sStr = sMsg;
	ReplaceAllDistinct(sStr, _T("%"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x13"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\x10"), _T("*"));
	ReplaceAllDistinct(sStr, _T("\r\n"), _T("*"));
	Remove(sStr, _T('\r'));
	Remove(sStr, _T('\n'));

	char signCheck[512];
	sprintf_s(signCheck, sizeof(signCheck), _T("%s%d%s%s%d%s%d"),m_config.GetSign(),sidx,sAccname.c_str(),sUser,userid,sStr.c_str(),i);
	unsigned char md5string[64];
	MD5_CTX   ctx;
	MD5Init(&ctx); 
	MD5Update(&ctx,(unsigned char *)signCheck, (unsigned int)strlen(signCheck) );
	MD5Final(md5string,&ctx);
	md5string[32]=0;

	// 如果 sUser 和 sMsg 编码有问题转成URLEncode 编码即可。 
	char body[512];
	sprintf_s(body, sizeof(body), _T("&game_key=%s&server_id=%d&time=%d&login_account=%s&actor=%s&actor_id=%d&to_login_account=&to_actor=&content=%s&ip=%s&sign=%s"),
		m_config.GetgKey(), sidx, i, sAccname.c_str(), URLEncode(sUser).c_str(), userid, URLEncode(sStr).c_str(), sIP,md5string);

	//m_config.GetPUrl(),body, strlen(body), true
	SendHttpPostRequest(enPost, m_config.GetPUrl(), body );
}

//接受到日志服的数据包进行处理
void CLocalSender::SendMsgToSP(int nServerIndex,int nChannleID,int nActorId,char* sActorName,char* msg)
{
	char newMsg[256];
	strncpy(newMsg, msg, sizeof(newMsg));

	char sIp[128];
	char sAname[128];
	char sSay[128];
	char * str = NULL;
	int count = 0;

	str = strtok(newMsg, "\b");
	if(str)
	{
		strncpy(sIp, str, sizeof(sIp));   //IP
		count++;
	}
	str = strtok(NULL, "\b");
	if(str)
	{
		strncpy(sAname, str, sizeof(sAname));   // 角色名称
		count++;
	}
	str = strtok(NULL, "\b");
	if(str)
	{
		strncpy(sSay, str, sizeof(sSay));   //聊天内容
		count++;
	}
	if( m_config.GetNumPost() != 0 &&  count == 3 )
	{
		switch(m_config.GetSid())
		{	
			case 0: 
				{
					ConvertMyTo360(nServerIndex, nChannleID,sAname, sActorName, sSay);
					break;
				}
			case 1: 
				{
					ConvertMyToYyg(nServerIndex, nChannleID, sActorName, sSay);
					break;
				}
			case 2:
				{
					ConvertMyTo4399(nServerIndex, sAname, sActorName, sSay, sIp);
					break;
				} 
			case 3:
				{
					ConvertMyTo91w(nServerIndex, nChannleID,sAname, sActorName, sSay, sIp);
					break;
				} 
			case 4:
				{
					ConvertMyTo37w(nServerIndex, sAname, sActorName, nActorId, sSay, sIp);
					break;
				} 
		default:
			break;
		}
	}
	else
	{
		//OutputMsg(rmError, _T("The Msg doesn't meet the requirement!!! nServerIndex=[%d], nChannleID=[%d], sActorName=[%s], nActorId=[%d], msg=[%s]"), nServerIndex, nChannleID, sActorName, nActorId, msg);
	}
}

#endif
