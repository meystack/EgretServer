#ifndef _WYLIB_PRINT_MSG_H_
#define _WYLIB_PRINT_MSG_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 通用输出消息重定向函数库 $
 *  
 *  - 主要功能 - 
 *
 *	实现常用的消息输出重定向，在不同项目中设置新的消息输出处
 *	理函数即可，默认的消息输出处理函数是libc的printf函数。
 *	输出消息函数的临时数据是基于栈的，所以是多线程安全的。
 *
 *****************************************************************/

#include "TypeDef.h"

namespace wylib
{
	namespace printmsg
	{
		/*	输出消息的类型定义	*/
		typedef enum ePrintMsgType
		{
			pmNormal = 0,	//普通消息
			pmTip,			//提示信息
			pmAlert,		//警告信息
			pmError,		//错误信息
		}PRINTMSGTYPE;

		/*	输出消息的重要程度定义	*/
		typedef enum eMsgLevel
		{
			mlLowest	= 0,	//无关紧要的信息
			mlNormal	= 5,	//正常等级的信息
			mlHighest	= 10,	//非常重要的信息
		}MSGLEVEL;

		#ifndef EXTEN_CALL
		#define EXTEN_CALL __stdcall
		#endif

		/*	输出消息处理函数的函数类型	*/

#ifdef WIN32
		typedef int (EXTEN_CALL *MSGHANDLEPROC)	(ePrintMsgType, eMsgLevel, LPCTSTR sMsg, int nMsgLen);
#else
		typedef int ( * MSGHANDLEPROC)	(ePrintMsgType, eMsgLevel, LPCTSTR sMsg, int nMsgLen);
#endif

		//设定消息处理函数，函数返回之前的消息处理函数
		MSGHANDLEPROC SetMsgHandler(MSGHANDLEPROC lpProc);
		//获取当前的消息处理函数
		MSGHANDLEPROC GetMsgHandler();
		//输出消息的函数，内部会对传递的相关参数进行格式化。
		//注意：格式化后输出的内容的长度必须小于4K
		int PrintMsg(ePrintMsgType msgType, eMsgLevel msgLevel, LPCTSTR format, ...);
	};
};

#endif

