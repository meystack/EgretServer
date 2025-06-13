#include <new>
#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <Windows.h>
#endif
#include <stdarg.h>
#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <Lock.h>
#include <QueueList.h>
#include <Thread.h>
#include <Stream.h>
#include <RefClass.hpp>
#include "ShareUtil.h"
#include <BufferAllocator.h>
#include "DataPacket.hpp"
#include "FileLogger.h"
#include <time.h>
#include <FDOP.h>

using namespace wylib::sync::lock;
using namespace wylib::container;
using namespace wylib::stream;
class COPMToFile : 
	protected wylib::thread::CBaseThread
{
public:
	COPMToFile(LPCTSTR lpLogFilePath):
	  wylib::thread::CBaseThread("COPMToFile")
	{
		char dirName[512];
		FDOP::ExtractFileDirectory(lpLogFilePath,dirName,sizeof(dirName));
		FDOP::DeepCreateDirectory(dirName);
		m_fFile =new wylib::stream::CFileStream(lpLogFilePath,CFileStream::faWrite | 
			CFileStream::faShareRead ,  CFileStream::AlwaysCreate);
		m_nRefer = 1;
		m_WriteMsgList.setLock(&m_WriteMsgLock);
		m_FreeMsgList.setLock(&m_FreeMsgLock);
		//将写指针调整到文件末尾
		m_fFile->seek(0, CFileStream::soEnd);
		StartWorkThread();
		m_bNeedDump = false;
#ifdef WIN32
		m_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}
	//增加引用计数，函数返回增加引用后的引用计数值
	inline LONG addRef()
	{
		return InterlockedIncrement(&m_nRefer);
	}
	//减少引用计数，函数返回减少引用后的引用计数值
	//当减少引用后引用计数为0则自动销毁对象自身。
	inline LONG release()
	{
		LONG n = InterlockedDecrement(&m_nRefer);
		if ( n == 0 )
			delete this;
		return n;
	}
	~COPMToFile()
	{
		INT_PTR i;

		//终止数据写入线程
		/*resume();
		terminate();
		waitFor();*/
		StopWorkThread();
		//销毁写入列表中的数据包
		m_WriteMsgList.flush();		
		for (i=m_WriteMsgList.count()-1; i>-1; --i)
		{
			m_WriteMsgList[i]->~CDataPacket();
		}
		m_WriteMsgList.empty();
		//销毁空闲列表中的数据包
		for (i=m_FreeMsgList.count()-1; i>-1; --i)
		{
			m_FreeMsgList[i]->~CDataPacket();
		}
		m_FreeMsgList.empty();
		//释放数据包内存
		for (i=m_MsgPackMemList.count()-1; i>-1; --i)
		{
			m_Allocator.FreeBuffer(m_MsgPackMemList[i]);
		}
		m_MsgPackMemList.empty();
	}

	void addMsg(SHAREOUTPUTMSGTYPE eMsgType, LPCTSTR sText, INT_PTR nTextLen)
	{
		static const int PackBlockSize = 256;
		CDataPacket *pPacket;
		SYSTEMTIME sysTime;

		m_FreeMsgList.lock();
		INT_PTR nCount = m_FreeMsgList.count();
		//如果空闲数据包队列没有数据了则申请PackBlockSize个数据包
		if ( nCount <= 0 )
		{
			pPacket = (CDataPacket*)m_Allocator.AllocBuffer(sizeof(*pPacket) * PackBlockSize);
			m_MsgPackMemList.add(pPacket);
			for (int i=0; i<PackBlockSize; ++i )
			{
				new(pPacket)CDataPacket(&m_Allocator);
				m_FreeMsgList.add(pPacket);
				pPacket++;
			}
			nCount = PackBlockSize;
		}
		//从空闲数据包队列中取出最后一个
		nCount--;
		pPacket = m_FreeMsgList[nCount];
		m_FreeMsgList.trunc(nCount);
		m_FreeMsgList.unlock();

		//清空数据
		pPacket->setPosition(0);
		//写入消息类型
		*pPacket << eMsgType;
		//写入时间
		GetLocalTime(&sysTime);
		pPacket->writeBuf(&sysTime, sizeof(sysTime));
		//写入消息内容
		*pPacket << (WORD)nTextLen;
		pPacket->writeBuf(sText, nTextLen * sizeof(*sText));
		*pPacket << (TCHAR)0;//写入终止符
		//数据包追加到写数据列表中
		m_WriteMsgList.append(pPacket);
	}

	void Dump()
	{
		m_bNeedDump = true;
	}
	
protected:
	VOID OnRountine()
	{
		while ( !terminated() )
		{

			if (m_bNeedDump)
			{
				DumpImpl();
				m_bNeedDump = false;
			}
			ProcessMessages();
			Sleep(16);
		}
	}
	//处理所有日志记录
	VOID ProcessMessages()
	{
		INT_PTR i, nCount;
		SHAREOUTPUTMSGTYPE msgType;
		size_t nMsgLen, nTmsLen;
		LPCTSTR sMsg;
		SYSTEMTIME sysTime;
		TCHAR sTimeStr[64];
		char sMsgBuf[4096];

		m_WriteMsgList.flush();

		nCount = m_WriteMsgList.count();
		for (i=0; i<nCount; ++i)
		{
			CDataPacket &pack = *(m_WriteMsgList[i]);
			pack.setPosition(0);
			pack >> msgType;
			pack >> sysTime;
#ifdef UNICODE
			nMsgLen = pack.readWideString(NULL, 0);
#else
			nMsgLen = pack.readString(NULL, 0);
#endif
			pack >> sMsg;
			if (sMsg)
			{
				nTmsLen = _stprintf( sTimeStr, _T("[%02d-%02d-%02d %02d:%02d:%02d]"), sysTime.wYear - 2000, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
				//转换为ASC编码
#ifdef WIN32
				convertToACP(sMsg, nMsgLen, sMsgBuf, ArrayCount(sMsgBuf));
				//显示此消息
				displayMessage(msgType, sTimeStr, sMsgBuf, nMsgLen);
				//转换为UTF-8编码
				nMsgLen = convertToUTF8(sMsg, nMsgLen, sMsgBuf, ArrayCount(sMsgBuf));
				writeMessage(msgType, sTimeStr, nTmsLen, sMsgBuf, nMsgLen);
#else
				displayMessage(msgType, sTimeStr, sMsg, nMsgLen);
				writeMessage(msgType, sTimeStr, nTmsLen, sMsg, nMsgLen);
#endif
				//向文件写入此消息
				
			}
			else if (IsDebuggerPresent())
			{
					DebugBreak();
			}
		}
		//将写入列表的数据包全部还原回空闲列表中
		m_FreeMsgList.lock();
		m_FreeMsgList.addArray(m_WriteMsgList, nCount);
		m_WriteMsgList.trunc(0);
		m_FreeMsgList.unlock();
	}
	//向文件写入消息记录
	void writeMessage(SHAREOUTPUTMSGTYPE msgType, const char* sTimeStr, const size_t nTmsLen, const char* sMsgBuf, const size_t nMsgLen)
	{
		static struct 
		{
			LPCTSTR sBegin, sEnd;
			size_t sizeBegin, sizeEnd;
		}
		FormatTags[] = 
		{
			{ _T(""),	   _T("\n"), _tcslen(FormatTags[0].sBegin) * sizeof(FormatTags[0].sBegin[0]), _tcslen(FormatTags[0].sEnd) * sizeof(FormatTags[0].sEnd[0]), },
			{ _T("[WRN]"), _T("\n"), _tcslen(FormatTags[1].sBegin) * sizeof(FormatTags[1].sBegin[0]), _tcslen(FormatTags[1].sEnd) * sizeof(FormatTags[1].sEnd[0]), },
			{ _T("[TIP]"), _T("\n"), _tcslen(FormatTags[2].sBegin) * sizeof(FormatTags[2].sBegin[0]), _tcslen(FormatTags[2].sEnd) * sizeof(FormatTags[2].sEnd[0]), },
			{ _T("[ERR]"), _T("\n"), _tcslen(FormatTags[3].sBegin) * sizeof(FormatTags[3].sBegin[0]), _tcslen(FormatTags[3].sEnd) * sizeof(FormatTags[3].sEnd[0]), },
		};

		m_fFile->write(sTimeStr, nTmsLen * sizeof(*sTimeStr));
		if ( FormatTags[msgType].sizeBegin > 0 )
			m_fFile->write(FormatTags[msgType].sBegin, FormatTags[msgType].sizeBegin);
		m_fFile->write(sMsgBuf, nMsgLen * sizeof(*sMsgBuf));
		if ( FormatTags[msgType].sizeEnd > 0 )
			m_fFile->write(FormatTags[msgType].sEnd, FormatTags[msgType].sizeEnd);
	}
	//显示日志消息
	void displayMessage(SHAREOUTPUTMSGTYPE msgType, const char* sTimeStr, const char* sMsgBuf, const size_t nMsgLen)
	{
#ifdef WIN32
		WORD wColorAttrs;
		switch ( msgType )
		{
		case rmWaning: 
			wColorAttrs = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			break;
		case rmError: 
			wColorAttrs = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		//case rmSystem:
		//	wColorAttrs = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		//	break;
		default:
			wColorAttrs = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		}
		SetConsoleTextAttribute(m_hStdOut, wColorAttrs);
		printf(sTimeStr);
		switch ( msgType )
		{
		case rmWaning: printf( ("[WRN]") ); break;
		case rmTip: printf( ("[TIP]") ); break;
		case rmError: printf( ("[ERR]") ); break;
		}

		puts(sMsgBuf);
#else
		switch ( msgType )
		{
		case rmWaning: 
			printf("\033[0;34m%s[WRN]%s\033[0m\r\n",sTimeStr,sMsgBuf);
			break;
		case rmError: 
			printf("\033[0;31m%s[ERR]%s\033[0m\r\n",sTimeStr,sMsgBuf);
			break;
		case rmTip:
			printf("%s[TIP]%s\r\n",sTimeStr,sMsgBuf);
			break;
		default:
			printf("%s%s\r\n",sTimeStr,sMsgBuf);
			break;
		}
#endif
		
	}
#ifdef WIN32
	//转换文字编码为UTF-8
	size_t convertToUTF8(const char* lpMsg, const size_t nMsgLen, char *pBuffer, const size_t BufferLen)
	{
		WCHAR sWCBuf[4096];
		DWORD cch = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
		if ( cch <= 0 )
			cch = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
		if ( cch > 0 )
		{
			cch = (DWORD)WideCharToMultiByte(CP_UTF8, 0, sWCBuf, cch, pBuffer, (int)(BufferLen-1), NULL, NULL);
			if ( cch > 0 )
			{
				pBuffer[cch] = 0;
				return cch;
			}
		}
		return 0;
	}
	//转换文字编码为ASC
	size_t convertToACP(const char* lpMsg, const size_t nMsgLen, char *pBuffer, const size_t BufferLen)
	{
		WCHAR sWCBuf[4096];
		DWORD cch = (DWORD)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
		if ( cch > 0 )
		{
			cch = (DWORD)WideCharToMultiByte(CP_ACP, 0, sWCBuf, cch, pBuffer, int(BufferLen-1), NULL, NULL);
			if ( cch > 0 )
			{
				pBuffer[cch] = 0;
				return cch;
			}
		}
		//本身就是ACP
		else 
		{
			cch = (DWORD)__min(BufferLen-1, nMsgLen);
			memcpy(pBuffer, lpMsg, cch);
			pBuffer[cch] = 0;
			return cch;
		}
		return 0;
	}
#endif
	void DumpImpl()
	{		
		using namespace wylib::stream;
		using namespace wylib::container;
		CFileStream fs(_T("FileLoggerStat.log"), CFileStream::faWrite, CFileStream::AlwaysOpen);
		fs.setPosition(fs.getSize());
		const TCHAR szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");
		time_t      szClock;	
		time(&szClock);
		struct tm curTime;
		localtime_r(&szClock,&curTime);
		TCHAR szDateTime[250] = {0};	
		_tcsftime(szDateTime, _tcslen(szDateTime)-1, _T("%Y-%m-%d %H:%M:%S\r\n"), &curTime);	
		fs.write(szSep, _tcslen(szSep)*sizeof(TCHAR));
		fs.write(szDateTime, _tcslen(szDateTime)*sizeof(TCHAR));
		TCHAR szDesc[256] = {0};
		//_stprintf(szDesc, _T("%-40s%-25s%-25s%-25s%-25s\r\n"), _T("HandleMgrName"), _T("UsedHandleCount"), _T("MaxUsedHdlCount"), _T("AllocedBlockNum"), _T("HandlePerBlock"));
		_stprintf(szDesc, _T("总分配的数据包数目 = %d\r\n"), m_MsgPackMemList.count());
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));

		
		CBufferAllocator::ALLOCATOR_MEMORY_INFO mi;
		INT_PTR nFreeMsgCount = 0, nPrepareWriteMsgCount = 0;
		// 获取空闲可用消息数量以及总共内存分配情况
		
		
		m_FreeMsgList.lock();
		nFreeMsgCount = m_FreeMsgList.count();
		m_Allocator.GetMemoryInfo(&mi);
		m_FreeMsgList.unlock();		

		_stprintf(szDesc, _T("小块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.SmallBuffer.dwAllocSize, mi.SmallBuffer.dwFreeSize);
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));
		_stprintf(szDesc, _T("中块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.MiddleBuffer.dwAllocSize, mi.MiddleBuffer.dwFreeSize);
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));
		_stprintf(szDesc, _T("大块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.LargeBuffer.dwAllocSize, mi.LargeBuffer.dwFreeSize);
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));
		_stprintf(szDesc, _T("超大块内存总分配字节数 = %-20d, 空闲字节数=%d\r\n"), mi.SuperBuffer.dwAllocSize, mi.SuperBuffer.dwFreeSize);
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));		
		// 获取带写入的消息数量
		m_WriteMsgList.lock();
		nPrepareWriteMsgCount = m_WriteMsgList.count() + m_WriteMsgList.appendCount();
		m_WriteMsgList.unlock();
		_stprintf(szDesc, _T("空闲日志消息数量 = %-20d, 待写入消息数量=%d\r\n"), nFreeMsgCount, nPrepareWriteMsgCount);
		fs.write(szDesc, _tcslen(szDesc)*sizeof(TCHAR));
		fs.write(szSep, _tcslen(szSep)*sizeof(TCHAR));
		

	}

private:
	LONG						m_nRefer;	//引用计数
	CBufferAllocator			m_Allocator;	//内存管理器
	CCSLock						m_WriteMsgLock;	//待写入到文件的消息列表锁
	CCSLock						m_FreeMsgLock;	//空闲消息包列表锁
	CQueueList<CDataPacket*>	m_WriteMsgList;	//待写入到文件的消息列表
	CLockList<CDataPacket*>		m_FreeMsgList;	//空闲消息包列表
	CBaseList<CDataPacket*>		m_MsgPackMemList;//消息包列表内存头指针列表
	bool						m_bNeedDump;
	wylib::stream::CFileStream*  m_fFile;
#ifdef WIN32
	HANDLE						m_hStdOut;
#endif
};


COPMToFile		*g_pOPMToFile;
SHAREOUTPUTMSGFN lpOldFnBeforeSet;


INT_PTR STDCALL OutputMsgToFileFn(SHAREOUTPUTMSGTYPE MsgType, LPCTSTR lpMsg, INT_PTR nMsgLen)
{
	g_pOPMToFile->addMsg(MsgType, lpMsg, nMsgLen);
	return nMsgLen;
}

void DumpFileLogger()
{
	g_pOPMToFile->Dump();
}

const TCHAR* getCurrentTimeDesc()
{
	static TCHAR szDataTime[256] = {0};
	//time_t szClock;	
	//time(&szClock);
	//struct tm *curTime = localtime(&szClock);	
	//_tcsftime(szDataTime, sizeof(szDataTime)-1, _T("%Y-%m-%d %H-%M-%S"), curTime);	

	SystemTime sysTime;
	GetSystemTime(sysTime);
	SNPRINTFA(szDataTime, sizeof(szDataTime) - 1, "%s%d%d%d.txt", szDataTime, sysTime.year_, sysTime.mon_, sysTime.mday_);
	return szDataTime;
}

CFileLogger::CFileLogger(LPCTSTR sLogFileNamePattern, ...)
{
	if ( !g_pOPMToFile )
	{
		TCHAR* pszFileName = new TCHAR[MAX_PATH];
		va_list ap;
		va_start(ap, sLogFileNamePattern);
		_vsntprintf(pszFileName, MAX_PATH-1, sLogFileNamePattern, ap);
		va_end(ap);
		g_pOPMToFile = new COPMToFile(pszFileName);
		lpOldFnBeforeSet = SetOutputMsgFn(OutputMsgToFileFn);
		delete []pszFileName;
	}
	else g_pOPMToFile->addRef();
}

CFileLogger::~CFileLogger()
{
	if ( g_pOPMToFile )
	{
		if ( g_pOPMToFile->release() == 0 )
		{
			SetOutputMsgFn(lpOldFnBeforeSet);
			g_pOPMToFile = NULL;
		}
	}
}


void CFileLogger::Dump()
{
	if (g_pOPMToFile)
		g_pOPMToFile->Dump();
}

void CFileLogger::DumpToFile()
{
	if (g_pOPMToFile)
		g_pOPMToFile->Dump();
}
