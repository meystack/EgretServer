#ifdef WIN32

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <tchar.h>
#include <Windows.h>
#include <_ast.h>
#include <_memchk.h>
#include <QueueList.h>
#include <Tick.h>
#include <wyString.h>
#include "ShareUtil.h"
#include "../include/FDOP.h"
#include "../include/ObjectAllocator.hpp"
#include "../include/CustomSortList.h"
//#include "FileDB.h"
#include "../include/FileDB_test.h"


namespace FileDBTest
{
	/*
	static const TCHAR* szTestBuffers[] = {
		_T("“DBEngine.exe”: 已加载“F:\\tcgp\\trunk\\ZhanJiangII\\server\\build\\DBEngine\\Win32\\DBEngine.exe”，已加载符号。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\ntdll.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\kernel32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\KernelBase.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“F:\\tcgp\\trunk\\ZhanJiangII\\server\\build\\DBEngine\\Win32\\libmySQL.dll”，未使用调试信息生成二进制文件。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\advapi32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\msvcrt.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\sechost.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\rpcrt4.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\sspicli.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\cryptbase.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\wsock32.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\ws2_32.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\nsi.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\ole32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\gdi32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\user32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\lpk.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\usp10.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\oleaut32.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\dbghelp.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\winmm.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\imm32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\msctf.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\uxtheme.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Program Files (x86)\\Common Files\\Kingsoft\\kiscommon\\kwsui.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\psapi.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\shell32.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\shlwapi.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\version.dll”，已加载符号(去除源信息)。"),
		_T("“DBEngine.exe”: 已加载“C:\\Program Files (x86)\\Common Files\\Kingsoft\\kiscommon\\kswebshield.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\wininet.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\normaliz.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\iertutil.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Windows\\SysWOW64\\urlmon.dll”，Cannot find or open the PDB file"),
		_T("“DBEngine.exe”: 已加载“C:\\Program Files (x86)\\Kingsoft\\KSafe\\ksfmon.dll”，Cannot find or open the PDB file"),
		_T("线程 'Win32 线程' (0x1440) 已退出，返回值为 0 (0x0)。"),
		_T("线程 'Win32 线程' (0x1080) 已退出，返回值为 0 (0x0)。"),
		_T("程序“[0x1158] DBEngine.exe: 本机”已退出，返回值为 0 (0x0)。"),
	};*/
	
	static const TCHAR* szTestBuffers[] = {
		_T("1.	务必保证所参与工作中涉及的工作机密、成果以及工具的保密性，若未经许有意或无意泄露者，视问题严重程度进行处罚；"),
		_T("2.	提倡学习新技术，凡因创新、引进技术对工作、项目产生推动作用皆给予嘉奖；"),
		_T("3.	务必保管好主管交予的相关密码（包括但不限于计算机用户登陆密码、版本控制系统密码、资源密码），同事之间不得相互询问转告密码，亦不得向非部门主管之外的任何人提供密码；（D类错误）"),
		_T("4.	务必于主管规定时间内完成任务，若当月连续2次因无合理理由而未在规定时间内完成任务则（D类错误），如对项目整体进度产生严重延误则（E类错误）；"),
		_T("5.	每日工作前，务必将所有项目相关文件通过版本控制系统更新一次，下班前，务必将所做更改的文件(无论更改完成与否)通过版本控制系统提交。凡因未按时间或提交项目文件且对其他同事工作产生影响者按照（B类错误）进行处罚，在仙海程序部群里群共享里有一个SVNBat，通过批处理一键式的提交、更新所有位置的代码，避免代码更新漏、提交漏的情况。请需要的同事进行下载使用。下载后请阅读说明进行配置； "),
		_T("6.	若自己所参与、负责项目或模块出现问题（无论是否工作时间）必须在主管规定时间内解决、缓解或避开问题，若因问题未及时解决而产生后严重果者，视问题严重程度（D类错误）-（E类错误）；"),
		_T("7.	认真做好所参与项目的修改日志，详细记录新增与改动的项目，对于可能产生的问题之处或重大修改之处做出醒目标记（B类错误）；"),
		_T("8.	务必确认编译警告的严重性，如却无实际影响则以合理的方法消除编译警告（A类错误）；"),
		_T("9.	提交的最终代码中不的包含未是用的局部变量或用于调试的冗余代码（A类错误）；"),
		_T("10.	务必注意OA公共文件柜中各文档的变动，一旦收到OA中文档变动的通知，务必及时查看并决定是否需要修改文档所涉工程代码（B类错误）。"),
		_T("11.	凡通过版本控制系统提交代码，务必在注释中写明所有修改的内容（B类错误）。"),
		_T("12.	凡对项目中需涉及第三方库或代码，尽量选择具有全部源代码且符合行业标准的库或代码，若无代码或代码不全面则应当由技术总监决定是否采用。"),
		_T("13.	当所涉及的工程或功能在实际开发中遇有与策划或设计文档有冲突之处应及时与策划部或设计人沟通并商讨冲突的解决方法。"),
		_T("14.	底层模块的设计应当注意复用性。"),
		_T("15.	功能或模块开发完成后，参与开发和设计人员应当首先进行逻辑性测试，严禁递交未经过逻辑测试的代码编译出的最终文件（C类错误）。"),
		_T("16.	未经过项目负责人批准，不得在任何非公司提供的存储介质中存储、或贝代码、文档、以及设计项目保密性质的数据文件（F类错误）。"),
		_T("17.	凡对数据库的update或delete操作，能够使用limit限制的必须要使用limit限制，防止语句错误修改或删除了不应当被操作的数据。"),
		_T("18.	每周最后一个工作日下班后务必OA书写当周工作总结，总结中包括当周工作内容的描述、总结以及下周的计划（A类错误）。"),
		_T("19.	对所有软件的修改，务必交由测试部测试，未经测试部测试的服务端或客户端软件禁止向服务器或玩家更新（C类错误）。"),
		_T("20.	在与其他公司协同进行技术开发或测试时，需态度友好，悉心听取对方意见和建议，不得做出有违公司形象的举措（A类错误）。"),
		_T("21.	c/c++/as3代码中{以及}必须单独占一行，每个标识符和运算符之间一定且只有一个空格（但函数参数后面的逗号紧随参数之后）；每个代码域必须以一个制表符的宽度进行缩进（A类错误）。"),
		_T("22.	pascal代码中if后的begin必须单独占一行，end单独占一行，else begin单独占一行；每个代码域必须以一个制表符的宽度进行缩进；每个标识符和运算符之间一定且只有一个空格（但函数参数后面的逗号或分号紧随参数之后）（A类错误）。"),
		_T("23.	新功能的开发或现有功能的开发，若有说明文档则必须严格按照说明文档进行开发。若开发中对说明文档中的介绍有疑义，务必及时联系相关负责人；若负责人不在场，则可结合项目之前的设计理念和编写方法做适当变通的做法。反若因文档确切表达有无，追究文档发布人(C类错误)；若文档表达无误而开发人员未按开发文档的主题设计思想进行开发或修改的，追究B类错误。"),
		_T("24.	所有经OA发布的工单，参与工作人员在收到工单后必须点击“主办”按钮而非工单标题，以便工作发布人能够看到工单的接受情况，工作办理完毕之后应当将工作转交回工作发布人，禁止非工作发布人点击“办理结束”按钮（A类错误）。"),
		_T("25.	数据库中某个表若需要增加字段且数据表格式已经投入使用，所有新加字段必须加在数据表的末尾，禁止向数据表现有字段前插入（B类错误）。"),
		_T("26.	必须在规定时间内完成上级分配任务，若因故不能按时完成则必须在进度时间过完70%前向上级汇报。（A类错误）"),
		_T("27.	必须真领会设计需求文档，凡对文档有歧义或不解之处必须立即与文档编写者沟通，若不能立即沟通，则应先将有歧义的部分暂缓并优先完成清晰的可立刻执行的部分。（A类错误）"),
		_T("28.	在规定时间内完成的任务，可在半小时内测试完成对，及时下班时间已到也必须在测试部测试完成并通过后才可下班，若个人有有事需按时下班，则必须提前安排参与一起开发、修改此功能或模块的同事或具有修改能力的同时协助测试并修改测试未通过的部分。（A类错误）"),
		_T("29.	工作时间内，工作流系统（OA）、BUG管理系统、测试系统必须时刻处于开启状态，所有通过系统分配的工作、BUG、测试流程必须在2分配后2小时内接取，不得出现超过工作限定时间仍未接取工作的情况。（A类错误）"),
		_T("30.	对于产品中的问题，必须时刻保持警惕，凡发现问题无论是理解错误还是真正有问题，必须及时通过BUG管理系统提交BUG。 （A类错误）"),
		_T("31.	对开发工具、工作系统不熟悉的新同事或对新启用的工作流程系统尚未完全掌握的，必须自行安排工作以外时间，上岗一月后必须熟练掌握所有开发工具和工作系统软件的用法。"),
		_T("32.	开发、测试和更新，必须严格遵守部门工作制度中的流程，不得跳过流程中的任何步骤。"),
		_T("33.	每个类、数据结构中的成员、函数，必须书写注释，注释中简明详细的阐明成员的用途，函数要阐明函数的作用、每个参数的意义以及返回值的说明。（A类错误）"),
		_T("34.	当使用版本控制同步代码并出现冲突时，必须使用其他人提交的文件覆盖自己的文件并自己重新修改代码提交。（A类错误）"),
		_T("35.	进行数据库设计与开发时，推荐直接手工书写创建数据库、表以及存储过程的SQL语句；"),
		_T("36.	所有对游戏数据表、存储过程创建、修改的SQL语句必须保存为sql文件，并亲自通过mysql命令行工具使用的测试。（B类错误）"),
		_T("37.	创建数据库、数据表、字段、存储过程时，其名称必须全部使用小写。（B类错误）"),
		_T("38.	SQL查询、更新语句中，涉及到的数据库、表、字段以及存储过程名，必须全部使用小写。（B类错误）"),
		_T("39.	所有基于MySQL的数据表，必须全部使用MyISAM引擎，这样才能进行数据库的backup指令备份，在创建表的SQL语句中必须明确书写type = MyISAM。（B类错误）"),
		_T("40.	创建数据库表的SQL语句中、为已经存在的表增加字段的语句中，除非字段具有自增属性或者字段的数据类型是datetime或datasnap，否则必须指定字段的默认值。（B类错误）"),
		_T("41.	创建数据表、字段、存储过程的SQL代码中必须包含对数据库、字段以及存储过程用途以及存储过程参数说明的注释。（A类错误）"),
		_T("42.	能使用int的地方绝不使用string。（A类错误）"),
		_T("43.	AS3中禁止在显示对象子类的构造函数中在子成员创建完成后添加对其的事件监听，应当通过监听自己的相关事件并判断事件的target的不同而进行不同的操作，如果有必要单独设置对子成员的事件监听，那么则在ADDED_TO_STAGE中注册监听并在REMOVED_FROM_STAGE中移除监听。（A类错误）"),
		_T("44.	在数据库中曾加一个表，如果该表与原有表具有主键行对应数据关系，那么必须在添加表后主动向表中增加内容！（B类错误）"),
		_T("45.	按策划文档、技术文档进行开发时，如果对文档设计有异议或有更好的实现方法时，请及时联系设计人沟通。禁止擅自修改实现方案或不履行设计文档的规则进行开发（B类错误）"),
		_T("46.	AS3中书写一个类时，必须从上到下按照如下顺序来书写常量、成员、函数，禁止不按次序胡乱书写！（A类错误）"),
	};

	void showDBInfo(LPCTSTR szFnName, LPCTSTR szDBName, const CFileDB &db)
	{
		_tprintf(_T("%s %s 数据量：%d 数据利用率:%d%% 索引利用率：%d%%\r\n"), szFnName, szDBName, 
			db.getDataCount(), 
			int((double)db.getTotalDataSize() / db.getDataFileSize() * 100),
			int((double)db.getDataCount() / (db.getDataCount() + db.getFreeChunkCount() + db.getNullIndexCount()) * 100)
			);
	}

	/** 测试打开和创建数据以及添加、读取数据 **/
	void TestPutGet(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);
		//添加和更新数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//读取数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	/** 测试打开和读取 **/
	void TestOpenGet(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);

		//读取数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	/** 测试添加、读取和更新数据 **/
	void TestUpdate(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);
		//添加数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//更新数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[ArrayCount(szTestBuffers) - 1 - i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//读取数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[ArrayCount(szTestBuffers) - 1 - i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	/** 测试打开和读取（相反的数据顺序） **/
	void TestOpenGet2(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);

		//读取数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[ArrayCount(szTestBuffers) - 1 - i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	/** 测试合并所有块的情况 **/
	void TestMergeAllChunks(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);
		//添加数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, "", 1);
		}
		//重新添加数据
		memset(sBuffer, 0xCC, sizeof(sBuffer));
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[ArrayCount(szTestBuffers) - 1 - i];
			db.put(i + 1, sBuffer, 17);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	/** 测试分割一个大数据块为两个小数据块的功能 **/
	void TestSplitLargeChunk(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);
		//添加数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//将一个大的数据改得比原始快大小少一半以上则会引发数据块拆分功能
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			db.put(i + 1, "", 1);
		}
		//再还原数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//读取和检测数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}
	/** 测试重新申请一个大块时从大的空闲块中分离一部分用于空间扩展 **/
	void TestReallocAndSplitLargeChunk(LPCTSTR szDBName)
	{
		INT_PTR i;
		LPCTSTR sData;
		TCHAR sBuffer[1024];
		INT64 nSize;
		CFileDB db;

		if (!db.open(szDBName) && !db.create(szDBName))
			return;
		db.setChunkSize(16);
		db.setFlushFileDartaImmediately(false);
		//添加数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			db.put(i + 1, sBuffer, sizeof(sBuffer));
		}
		//将所有数据修改为1字节，这也会引发数据块拆分功能
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			db.put(i + 1, "", 1);
		}
		//再将所有数据修改为块单元大小+1字节以上，即可引发从大的空闲块中分离一部分用于空间扩展
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			sData = szTestBuffers[i];
			db.put(i + 1, sData, _tcslen(sData));
		}
		//读取和检测数据
		for (i=0; i<ArrayCount(szTestBuffers); ++i)
		{
			nSize = db.get(i + 1, sBuffer, sizeof(sBuffer) - 1);
			sBuffer[nSize] = 0;

			sData = szTestBuffers[i];
			Assert(_tcscmp(sData, sBuffer) == 0);
		}
		showDBInfo(_T(__FUNCTION__), szDBName, db);
	}

	void RunFileDBTestRoutines()
	{
		_tprintf(_T("test begin...\r\n"));

		//测试添加、读取数据
		TestPutGet(_T("FDBTest/test1"));
		//测试打开和读取
		TestOpenGet(_T("FDBTest/test1"));
		//测试添加、读取和更新数据
		TestUpdate(_T("FDBTest/test2"));
		//测试打开和读取（相反的数据顺序）
		TestOpenGet2(_T("FDBTest/test2"));
		//测试合并所有块的情况
		TestMergeAllChunks(_T("FDBTest/test3"));
		//测试分割一个大数据块为两个小数据块的功能
		TestSplitLargeChunk(_T("FDBTest/test4"));
		//测试重新申请一个大块时从大的空闲块中分离一部分用于空间扩展
		TestReallocAndSplitLargeChunk(_T("FDBTest/test5"));

		_tprintf(_T("test pass success!\r\n"));
	}
}
#endif
