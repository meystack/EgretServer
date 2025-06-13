#pragma once
class CProduct
{
public:
	CProduct(PlatForm * config);
	~CProduct(void){}

	int Pay(const char* sOPID, const char* sAccount,UINT nMoney,double dRmb,UINT ServerIndex,UINT nType,UINT &uUserId,UINT nActorId, const char * actorname, int level, const char* channel);
	int QueryAmount(UINT uUserId, UINT& nActorId);
	int Consume(UINT uUserId,UINT nAmount,char* sCharName,UINT nServerId, UINT nActorId, BYTE nLevel);

	void SetConfig(PlatForm* config);
	inline char* GetGName() { return m_code;}
	inline char* GetSpid() { return m_spid;}
	inline LPCSTR GetAuthorizationIP() { return m_Config->db;}
	inline char* GetPayKey() { return m_Config->paykey;}
	inline char* GetAwdKey() { return m_Config->awdkey;}
	inline int GetIPListSize() { return m_Config->hostcount;}
	inline long long * GetIPList() { return m_Config->hosts;}
	int GetContractID(const char * sContractID );
private:
	BOOL CheckConnection();
	BOOL SetDBConnection();

	CSQLConenction	m_Mysql;
	CMiniDateTime	m_ConnectMiniDateTime;
	CCSLock			m_UserDBLock;
	char			m_spid[8];
    char            m_code[8];
	PlatForm *   	m_Config;
};

static LPCTSTR sPay_SQL				= "call UserPayment(\"%s\", \"%s\", %d, %f, %d, \"%s\", %d, %u, \"%s\", %d, \"%s\")";
static LPCTSTR sQuery_SQL			= "call QueryAmount(%u,%u,%u)";
static LPCTSTR sConsume_SQL			= "call Consume(%u,%u,%u,\"%s\", %u,%d)";

static LPCTSTR sError_Pay_SQL				= "Error happen when call UserPayment(\"%s\", \"%s\", %d, %f, %d, \"%s\", %d)";
static LPCTSTR sError_Query_SQL			= "Error happen when call QueryAmount(%u,%u)";
static LPCTSTR sError_Consume_SQL			= "Error happen when call Consume(%u,%u,\"%s\", %u)";
