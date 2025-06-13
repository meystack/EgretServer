#pragma once

/************************************************************************
 
                            定时回调函数类

                   支持按间隔毫秒数以及指定日期时间点的周期回调。

************************************************************************/

#include "_osdef.h"

class CTimeCaller
{
public:
	/*
	* 定时回调函数类型定义
	* lpThis 回调对象（用于支持OOP，存储this指针）
	* lpData 回调数据，数据由注册时传递
	* nCount 当前是第几次被调用
	*/
	typedef VOID (STDCALL *TimedCallBack) (LPVOID lpThis, LPVOID lpData, INT_PTR nCount);

public:
	CTimeCaller();

	/*
	* Comments: 注册定时回调对象
	* Param LPVOID lpThis: 回调对象（用于支持OOP，存储this指针），会在回调函数中传递
	* Param TimedCallBack lpCall: 回调函数指针，回调函数形如CTimeCaller::TimedCallBack
	* Param LPVOID lpData: 回调数据，会在回调函数中传递
	* Param ULONGLONG lInterval: 调用周期，单位是毫秒
	* Param ULONGLONG lNextCall: 下次调用时间（单位是TickCount），如果传递0则表示从当前时间起经过一次调用周期后调用
	* @Return HANDLE: 回调对象句柄
	*/
	HANDLE AddTimeCall(LPVOID lpThis, TimedCallBack lpCall, LPVOID lpData, ULONGLONG lInterval, ULONGLONG lNextCall = 0);
	/*
	* Comments: 移除回调函数
	* Param HANDLE hTimedCall: 回调对象句柄
	* @Return BOOL: 如果回调对象存在且成功移除则函数返回TRUE否则返回FALSE
	*/
	BOOL RemoveTimeCall(HANDLE hTimedCall);
	/*
	* Comments: 通过定时调用对象句柄获取调用周期
	* Param HANDLE hTimedCall: 定时调用对象句柄
	* Param PULONGLONG lpNextCall: 指向用于存储下次调用发生的TickCount值的数据缓冲，参数为空则不向其中存储数据
	* @Return ULONGLONG: 函数返回此回调的调用周期，单位是毫秒
	*/
	ULONGLONG GetCallInterval(HANDLE hTimedCall, PULONGLONG lpNextCall);
	/*
	* Comments: 通过定时调用对象句柄设置调用周期
	* Param HANDLE hTimedCall: 定时调用对象句柄
	* Param ULONGLONG lInterval: 新的调用周期，单位为毫秒
	* Param ULONGLONG lNextCall: 下次调用此函数的TickCount值
	* @Return bool: 更新调用周期成功则返回true
	* ★注意★
	*    lNextCall参数表示下次调用此函数的TickCount值，传递0表示不改变下次
	*  调用的时间；传递比当前时间小的值将导致在下次例行监察时调用此函数，鉴于此
	*  特性，调用者可以通过传递一个比当前时间小调用周期倍的值来实现立刻对此函数
	*  进行N次调用的功能；如果传递的值比当前时间大则会在设定的时间中回调此函数。
	*/
	bool SetCallInterval(HANDLE hTimedCall, ULONGLONG lInterval, ULONGLONG lNextCall = 0);
	/*
	* Comments: 检查并调用已经到达运行时间的回调对象
	* Param ULONGLONG lRunTickLimit: 对Run函数运行的时间限制，单位是毫秒，0表示不限制
	* @Return INT_PTR: 返回检查了多少个回调对象
	*/
	INT_PTR Run(ULONGLONG lRunTickLimit = 0);

protected:
	/*
	* Comments: 获取当前运行时间的函数，默认的实现是返回当前的TickCount，
	*			子类可以通过覆盖此函数来实现返回基于日期时间的值
	* @Return ULONGLONG:
	*/
	virtual ULONGLONG GetCurrentTick();

protected:
	/**
	 * 定义回调函数注册信息
	 ***********************/
	typedef struct tagTimeCallInfo
	{
		TimedCallBack	lpCallback;	//回调函数指针
		LPVOID			lpThis;		//回调对象（用于支持OOP，存储this指针）
		LPVOID			lpData;		//回调数据，数据由注册时传递
		INT_PTR			nCallCount;	//调用次数
		ULONGLONG		lNextCallTick;//下次调用的时间(TickCount)
		ULONGLONG		lInterval;	//回调周期
		BOOL			boMarkedRemove;//是否标记为移除
	}TIMECALLINFO, *PTIMECALLINFO;

protected:
	wylib::container::CBaseLinkedList<TIMECALLINFO>		m_CallList;	//回调对象列表
	wylib::container::CLinkedListIterator<TIMECALLINFO>	m_pRunIter;	//下此继续检查的回调对象节点
	INT_PTR												m_nRmCallCount;//标记移除的数量
};
