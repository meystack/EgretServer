#pragma once

//保存聊天信息到文本

#ifndef	OUTPUTCALL
#define	OUTPUTCALL __stdcall
#endif

//聊天消息输出函数
INT_PTR	 OUTPUTCALL OutPutChatMsg(LPCTSTR channelName,LPCTSTR userName, LPCTSTR strMsg);

//初始化
INT_PTR	 OUTPUTCALL InitChatMsgOut();

//清除聊天消息
INT_PTR	 OUTPUTCALL ClearChatMsgOut();