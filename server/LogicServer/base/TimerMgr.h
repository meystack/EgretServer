#pragma once

/*************************************
	定时器管理,每个类都可以添加
**************************************/
/*
typedef struct tagTickMsg
{
	INT_PTR nTimerID;	//定时器的ID
	TICKCOUNT nNextTick;	//下次回调的时间嘀嗒
	TICKCOUNT nFristTick; //第1次的定时器回调的时间
	INT_PTR  nInterval; //间隔，单位ms
}TICKMSG,*PTICKMSG;

class CTickAAA
{
public:
	TICKCOUNT m_dwNextTick;

	inline bool timeRiched(TICKCOUNT dwTick)
	{
		if ( dwTick >= m_dwNextTick )
		{
			m_dwNextTick += m_dwInterval;
			return true;
		}
		return false;
	}
};

class CAAA
{
	CTickAAA m_SecondTick;

	void run(TICKCOUNT dwCurrTick)
	{
		if ( m_SecondTick.timeRiched(dwCurrTick) )
		{
			foo1();
		}

	}
};

//内存管理器


class CTimerMgr
{
public:
	CTimerMgr(){}
	//每个继承这个类的执行一下这个函数,定时调用
	VOID OnTimerRun()
	{
		//时间没有到
		
		CLinkedNode<TICKMSG>* pNode = m_TimerList.enumFirst();
		TICKCOUNT nCurrentTick = _getTickCount();
		while(pNode )
		{
			if(pNode->m_Data.nFristTick && pNode->m_Data.nFristTick <= nCurrentTick)
			{
				OnTimer(pNode->m_Data.nTimerID); //调用
				pNode->m_Data.nFristTick =0; //第1次调用结束
			}
			else if(pNode->m_Data.nNextTick <= nCurrentTick)
			{
				OnTimer(pNode->m_Data.nTimerID); //调用
				pNode->m_Data.nNextTick = pNode->m_Data.nInterval + nCurrentTick;
			}
			
			pNode =m_TimerList.enumNext();
		}
	
	}
	


	VOID AddTimer(INT_PTR nTimerID, INT_PTR nTickInterval,INT_PTR nFirstTickTime =0)
	{
		
		CLinkedNode<TICKMSG>* pNode = m_TimerList.enumFirst();
		while(pNode )
		{
			if(pNode->m_Data.nTimerID == nTimerID) //已经有了
			{
				return;
			}
			pNode =m_TimerList.enumNext();
		}
		TICKMSG data;
		data.nInterval = nTickInterval;
		data.nNextTick =0;
		data.nTimerID = nTimerID;
		if(nFirstTickTime)
		{
			data.nFristTick = GetLogicServer()->GetLogicEngine()->getTickCount() + nFirstTickTime; //如果设置了第1次的时间，那么就
		}
		else
		{
			data.nFristTick  =0;
		}
		m_TimerList.linkAtLast(data);
		

	}
	//删除定时器
	VOID DelTimer(INT_PTR nTimerID)
	{
		
		CLinkedNode<TICKMSG>* pNode = m_TimerList.enumFirst();
		while(pNode )
		{
			if(pNode->m_Data.nTimerID == nTimerID) //已经有了
			{
				m_TimerList.remove(pNode);
				return;
			}
			pNode =m_TimerList.enumNext();
		}
		
	}

	//定时器回调
	virtual VOID OnTimer(INT_PTR nTimerID) =0;
	
	//清除定时器
	VOID ClearTimer()
	{
		m_TimerList.clear(); 
	}
private:
	CList<TICKMSG> m_TimerList; //定时器的
};
*/

extern TICKCOUNT GetLogicCurrTickCount() ;

template<int INTERVAL>
class CTimer
{

public:
	CTimer()
	{
//如果是逻辑服务器直接取当前的tick，其他服务器自己去取当前的时间，加快构造函数的速度
#ifdef __LOGIC_SERVER__
		if (GetLogicCurrTickCount() == 0)
		{
			m_dwNextTick =_getTickCount() + INTERVAL; //说明CLogicEngine还没初始化
		}
		else
		{ 
			m_dwNextTick = GetLogicCurrTickCount() + INTERVAL;
		}
		
		
#else
		m_dwNextTick=_getTickCount() + INTERVAL;
#endif
	}
	
	
	//判断时间是否到达，如果到达则设置新的时间并返回true
	// Param bool bIgnore: 是否忽略中间累积的处理。
	inline bool CheckAndSet(TICKCOUNT dwCurrentTick, bool bIgnore = false)
	{
		if ( dwCurrentTick >= m_dwNextTick )
		{
			if (bIgnore)
				m_dwNextTick  = dwCurrentTick + INTERVAL;
			else
				m_dwNextTick  += INTERVAL;
			return true;
		}
		return false;
	}
	//判断时间是否到达
	inline bool Check(TICKCOUNT dwCurrentTick)
	{
		return dwCurrentTick >= m_dwNextTick;
	}
	/*
		增加一个接口，设置现在到下一次触发的间隔时间，这个主要用于跨天等特殊场合
		* nNextTime 单位ms，表示下次触发是从现在开始的多少ms
	*/
	inline VOID SetNextHitTimeFromNow(INT_PTR nNextTime)
	{
#ifdef __LOGIC_SERVER__
		m_dwNextTick =GetLogicCurrTickCount() + nNextTime; 
#else
		m_dwNextTick = _getTickCount() + nNextTime;
#endif 

	}
	inline void SetNextHitTime(TICKCOUNT nNextTime)
	{
		m_dwNextTick = nNextTime;
	}
	inline TICKCOUNT GetInterVal(TICKCOUNT dwCurrentTick)
	{
		return m_dwNextTick-dwCurrentTick;
	}
	inline TICKCOUNT GetNextTime()
	{
		return m_dwNextTick;
	}
	inline TICKCOUNT GetINTERVAL()
	{
		return INTERVAL;
	}
	//以当前时间为准重设下次时间
	inline VOID Reset()
	{
		m_dwNextTick = GetLogicCurrTickCount() + INTERVAL;
	}
	//把定时器往后推一定的时间
	inline void Delay(INT_PTR nTick)
	{
		m_dwNextTick += nTick;
	}
private:
	TICKCOUNT m_dwNextTick ;

	//DECLARE_OBJECT_COUNTER(CTimer<INTERVAL>)
};
