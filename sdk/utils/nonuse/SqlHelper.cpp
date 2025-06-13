//#include "StdAfx.h"
//
//CSqlHelper::CSqlHelper(CSQLConenction* conn) : m_pSQLConnection(conn), m_bNeedReset(false)
//{
//}
//
//CSqlHelper::~CSqlHelper()
//{
//	Reset();
//}
//
//void CSqlHelper::Reset()
//{
//	if (m_bNeedReset)
//	{
//		m_pSQLConnection->ResetQuery();
//		m_bNeedReset = false;
//	}
//}
//
//int CSqlHelper::Query(const char* sQueryFormat, ...)
//{
//	int	Result;
//	va_list	args;	
//	va_start(args, sQueryFormat);
//	char szBuff[40960] = {0};
//	Result = vsprintf(szBuff, sQueryFormat, args);
//	va_end(args);
//	int nResult = RealQuery(szBuff, Result);
//	if (nResult == 0)
//		m_bNeedReset = true;
//	return nResult;	
//}
//
//int CSqlHelper::RealQuery(const char* sQueryText, const size_t nTextLen)
//{
//	int nResult = m_pSQLConnection->RealQuery(sQueryText, nTextLen);
//	if (nResult == 0)
//		m_bNeedReset = true;
//	return nResult;
//}
//
//int CSqlHelper::Exec(const char* sQueryFormat, ...)
//{
//	int	Result;
//	va_list	args;	
//	va_start(args, sQueryFormat);
//	char szBuff[40960] = {0};
//	Result = vsprintf(szBuff, sQueryFormat, args);
//	va_end(args);
//	int nResult = RealExec(szBuff, Result);
//	if (nResult == 0)
//		m_bNeedReset = true;
//	return nResult;
//}
//
//int CSqlHelper::RealExec(const char* sExecText, const size_t nTextLen)
//{
//	int nResult = m_pSQLConnection->RealExec(sExecText, nTextLen);
//	if (nResult == 0)
//		m_bNeedReset = true;
//	return nResult;
//}