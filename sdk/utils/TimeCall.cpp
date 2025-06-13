#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <tchar.h>
#include <windows.h>
#endif

#include <_ast.h>
#include <_memchk.h>
#include <Tick.h>
#include <LinkedList.h>
#include "ShareUtil.h"
#include "TimeCall.h"

#include <string.h>

using namespace wylib::container;
CTimeCaller::CTimeCaller()
{
	m_nRmCallCount = 0;
	m_pRunIter.setList(m_CallList);
}

HANDLE CTimeCaller::AddTimeCall(LPVOID lpThis, TimedCallBack lpCall, LPVOID lpData, ULONGLONG lInterval, ULONGLONG lNextCall /* = 0 */)
{
	TIMECALLINFO tc;

	ZeroMemory(&tc, sizeof(tc));
	tc.lpCallback = lpCall;
	tc.lpThis = lpThis;
	tc.lpData = lpData;
	tc.lNextCallTick = lNextCall;
	tc.lInterval = lInterval;
	tc.boMarkedRemove = FALSE;
	m_CallList.linkAtLast(tc);

	return m_CallList.last();
}

BOOL CTimeCaller::RemoveTimeCall(HANDLE hTimedCall)
{
	CLinkedNode<TIMECALLINFO>* pNode;
	CLinkedListIterator<TIMECALLINFO> it(m_CallList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (hTimedCall == pNode)
		{
			TIMECALLINFO &tc = *pNode;
			//标记为移除
			tc.boMarkedRemove = TRUE;
			m_nRmCallCount++;
			return TRUE;
		}
	}
	return FALSE;
}

ULONGLONG CTimeCaller::GetCallInterval(HANDLE hTimedCall, PULONGLONG lpNextCall)
{
	CLinkedNode<TIMECALLINFO>* pNode;
	CLinkedListIterator<TIMECALLINFO> it(m_CallList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (hTimedCall == pNode)
		{
			TIMECALLINFO &tc = *pNode;
			if (lpNextCall) *lpNextCall = tc.lNextCallTick;
			return tc.lInterval;
		}
	}
	return 0;
}

bool CTimeCaller::SetCallInterval(HANDLE hTimedCall, ULONGLONG lInterval, ULONGLONG lNextCall)
{
	CLinkedNode<TIMECALLINFO>* pNode;
	CLinkedListIterator<TIMECALLINFO> it(m_CallList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (hTimedCall == pNode)
		{
			TIMECALLINFO &tc = *pNode;
			tc.lInterval = lInterval;
			if (lNextCall) tc.lNextCallTick = lNextCall;
			return true;
		}
	}
	return false;
}

INT_PTR CTimeCaller::Run(ULONGLONG lRunTickLimit /* = MAXULONGLONG */)
{
	INT_PTR Result = 0;
	CLinkedNode<TIMECALLINFO>* pNode =NULL;
	ULONGLONG lCurTick = GetCurrentTick();
	ULONGLONG lFinishTick = lRunTickLimit ? lRunTickLimit + _getTickCount() : 0;
	
	//继续回调
	while (pNode = m_pRunIter.next())
	{
		TIMECALLINFO &tc = *pNode;
		if (lCurTick >= tc.lNextCallTick)
		{
			tc.nCallCount++;
			tc.lNextCallTick += tc.lInterval;
			tc.lpCallback(tc.lpThis, tc.lpData, tc.nCallCount);
		}
		Result++;
		if (lFinishTick && (ULONGLONG)_getTickCount() >= lFinishTick)
			break;
	}
	
	//移除标记为移除的回调
	if (m_nRmCallCount > 0)
	{
		CLinkedNode<TIMECALLINFO>* pNode;
		CLinkedListIterator<TIMECALLINFO> it(m_CallList);
		for (pNode = it.first(); pNode; pNode = it.next())
		{
			TIMECALLINFO &tc = *pNode;
			if (tc.boMarkedRemove)
				it.remove(pNode);
		}
		m_nRmCallCount = 0;
	}

	return Result;
}

ULONGLONG CTimeCaller::GetCurrentTick()
{
	return _getTickCount();
}
