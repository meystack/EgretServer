#ifndef _INTERNAL_MESSAGE_H_
#define _INTERNAL_MESSAGE_H_

/************************************************************************/
/*						                                                */
/* 程序内部通信消息结构                                                 */
/*						                                                */
/************************************************************************/
#include "ObjectCounter.h"
typedef struct tagAppInternalMessage : public Counter<tagAppInternalMessage>
{
	UINT	uMsg;	//消息号
	UINT64	uParam1;	//消息参数1
	UINT64	uParam2;//消息参数2
	UINT64	uParam3;//消息参数3
	UINT64	uParam4;//消息参数4

}APPINTERNALMSG, *PAPPINTERNALMSG;

#endif

