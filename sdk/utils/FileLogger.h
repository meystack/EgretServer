#pragma once

/*********************************************************

                    日志文件记录器

                 生成的日志文件是HTML格式的
 将此类型在main函数中定义一份变量即可，请勿在一个程序中多次使用此类型

*********************************************************/

const TCHAR* getCurrentTimeDesc();

class CFileLogger
{
public:
	CFileLogger(LPCTSTR sLogFileNamePattern, ...);
	~CFileLogger();
	void Dump();

	static void DumpToFile();

	//void SetNeedOutput(bool bFlag = true);
};
