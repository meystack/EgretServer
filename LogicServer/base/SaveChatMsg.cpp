/*
#include "StdAfx.h"
#include "SaveChatMsg.h"

FILE* g_ChatMsgFp = NULL;
char ChatMsgName[] = "ChatMsg";
RTL_CRITICAL_SECTION	g_OutputChatMsgLock;

//聊天消息输出函数
INT_PTR	 OUTPUTCALL OutPutChatMsg(LPCTSTR channelName,LPCTSTR userName, LPCTSTR strMsg)
{
	INT_PTR	Result = 0;

	if (g_ChatMsgFp)
	{
		EnterCriticalSection( &g_OutputChatMsgLock );

		SYSTEMTIME Systime;
		GetLocalTime( &Systime );

		char tmpStr[256];
		sprintf_s( tmpStr,sizeof(tmpStr),"[%d-%d-%d %d:%d:%d]", Systime.wYear - 2000, Systime.wMonth, Systime.wDay, Systime.wHour, Systime.wMinute, Systime.wSecond );
		fputs(tmpStr,g_ChatMsgFp);
		fputc('\t',g_ChatMsgFp);

		char sBuf[1024];
		sprintf_s(sBuf,sizeof(sBuf),"<%s> %s: %s",channelName,userName,strMsg);
		fputs(sBuf,g_ChatMsgFp);

		fputc('\n',g_ChatMsgFp);
		fflush(g_ChatMsgFp);

		LeaveCriticalSection( &g_OutputChatMsgLock );
	}

	return 0;
}

INT_PTR	 OUTPUTCALL InitChatMsgOut()
{
	InitializeCriticalSection( &g_OutputChatMsgLock );
	//设置CTYPE为本地代码页，防止出现无法打印UNICODE 255之后的字符的问题。
	char sACP[16];
	sprintf(sACP, ".%d", GetACP());
	//setlocale(LC_CTYPE, sACP);

	SYSTEMTIME Systime;
	GetLocalTime( &Systime );

	char tmpStr[256];
	sprintf_s( tmpStr,sizeof(tmpStr),"%d-%d-%d %d-%d-%d", Systime.wYear - 2000, Systime.wMonth, Systime.wDay, Systime.wHour, Systime.wMinute, Systime.wSecond );

	char sPath[256] = "../data/runtime/chat/";
	if (!FDOP::FileExists(sPath))
	{
		FDOP::DeepCreateDirectory(sPath);
	}

	sprintf_s(sPath,sizeof(sPath),"../data/runtime/chat/%s-%s-log.txt",ChatMsgName,tmpStr);

	//strcat(sPath,tmpStr);
	//strcat(sPath,ChatMsgName);

	//OutputMsg( rmError, _T("sPath:%s"),sPath );
	g_ChatMsgFp = fopen(sPath,"a");

	return 0;
}

INT_PTR	 OUTPUTCALL ClearChatMsgOut()
{
	DeleteCriticalSection( &g_OutputChatMsgLock );

	if(g_ChatMsgFp)
		fclose(g_ChatMsgFp);

	return 0;
}
*/