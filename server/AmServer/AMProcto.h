#pragma once

namespace AMProcto
{
#pragma pack(push, 1)
	typedef struct tagAMMsg
	{
		//unsigned short nCmd;    //消息码
		INT nServerId;//服务器Id
		unsigned int nUserId; //UserID
		INT nResult; //返回值
		INT64 lOPPtr;//操作对象（客户端穿透服务器的数据）
		BYTE nLevel;     //提取时等级
	}AMMSG, *PAMMSG;
#pragma pack(pop)

#pragma pack(push, 1)
	typedef struct tagTASKMsg
	{
		//unsigned short nCmd;    //消息码
		INT nServerId;//服务器Id
		unsigned int nUserId; //UserID
		INT nCmd;
		INT nContractid;
		INT nStep;
		/*
		nResult
		0: 步骤已完成 或 奖励发放成功
		1: 用户尚未在应用内创建角色
		2：用户尚未完成本步骤
		3：该步骤奖励已发放过
		4：发货失败
		*/
		INT nResult; //返回值
		INT64 lOPPtr;//操作对象（客户端穿透服务器的数据）
	}TASKMSG, *PTASKMSG;
#pragma pack(pop)

	enum eAMCmdCode
	{
		/* 客户端发送 */
		AMC_REGIST_CLIENT = 100,   //客户端注册
		//AMC_KEEP_ALIVE = 101,      //客户端发送KeepAlive
		AMC_QUERY_AMOUNT = 102,    //客户端查询用户余额
		AMC_COMSUME = 103,         //客户端发送用户消费请求
		AMC_TASK_SITUATION = 104,   //客户端返回任务状态

		/* 服务器回应 */
		AMS_REGIST_CLIENT = 30100,  //返回客户端注册成功（仅成功时返回）  
		//AMS_KEEP_ALIVE = 30101,     //服务器发送KeepAlive
		AMS_QUERY_AMOUNT = 30102,   //服务器返回用户余额（TAMOPMsgData.nAmount为余额）
		AMS_COMSUME = 30103,        //服务器返回用户消费结果（TAMOPMsgData.nAmount为消费结果，0表示成功，-1表示余额不足，其他值表示其他错误）
		AMS_PAYSUCCESS = 30104,    //服务器返回充值成功
		AMS_TASK_SITUATION = 30105,   //查询任务完成状态
		AMS_INVALID_CMD = 65534,    //无效的CMD
	};
}