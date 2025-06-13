#pragma once

#define MAX_MSG_COUNT 1024			//这个跟数据库定义的长度相关，不能改
//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)
	//保存这个玩家的邮件消息

	typedef struct tagMsgData
	{
		INT64		Msgid;
		BYTE		Msgtype;
		char		Msg[MAX_MSG_COUNT];//
		WORD		nBufSize;			//数据内容的长度，最大不超过MAX_MSG_COUNT
		/*Msg的定义：
		[标题文字][按钮文字][消息内容]
		前2字段是固定		
		*/
		void GetTxt(LPCSTR& sTitle,LPCSTR& sBtnTxt)
		{
			CDataPacketReader packet(Msg,nBufSize);
			packet >> sTitle;
			packet >> sBtnTxt;
		}
	}MsgData;
#pragma pack(pop)
