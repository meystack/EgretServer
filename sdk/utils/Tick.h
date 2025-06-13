#ifndef _MTICK_H_
#define _MTICK_H_


/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 增强GetTickCount函数库 $
 *  
 *  - 主要功能 - 
 *
 *	可使用64位的TickCount。内部解决了TickCount在超过49天后重新开始的问题。
 *  同时，提供了一个在整个进程中使用同步的TickCount的机制，即通过SetTickCountEx
 *  函数来设置同步的时间值，从而避免在多处理器中不同处理器之间的频率差异带来的问题
 *
 *****************************************************************/
#include "_osdef.h"
namespace wylib
{
	namespace time
	{
		namespace tick64
		{
			typedef LONGLONG	TICKCOUNT64;
			typedef DWORD		TICKCOUNT32;

			//取机器运行时间的函数，函数返回的是毫秒值，
			//如果硬件支持查询PerformanceCounter则可避免49天的整数溢出的问题
			/***
			  实际测试后此函数性能较差，因为查询CPU中断会导致进入内核并可能挂起
			  r3级的线程。此外在多处理器中如果各个处理器之间频率有误差，则可能
			  导致由不同线程的连续的两次调用，后者取得的时间值比前者要晚的问题。
			***/
			TICKCOUNT64 GetTickCount64();
			/***
			*  提供在整个进程中使用同步的TickCount的机制，即通过SetTickCountEx
			*  函数来设置同步的时间值，从而避免在多处理器中不同处理器之间的频率差异
			*  带来的问题。因此一旦调用者调用一次SetTickCountEx则GetTickCountEx
			*  也随之进入同步模式，调用者必须持续更新TickCount值或设置TickCount值
			*  为0从而还原默认的GetTickCountEx的取值规则。
			***/
			TICKCOUNT64 SetTickCount64(TICKCOUNT64);

			TICKCOUNT32 GetTickCount32();
			TICKCOUNT32 SetTickCount32(TICKCOUNT32);

			#ifdef _MTICK64
			typedef TICKCOUNT64	TICKCOUNT;
			#	define GetTickCountEx		GetTickCount64
			#	define SetTickCountEx		SetTickCount64
			#else
			typedef TICKCOUNT32	TICKCOUNT;	
			#	define GetTickCountEx		GetTickCount32
			#	define SetTickCountEx		SetTickCount32
			#endif

		};
	};
};

#endif
