#pragma once

/***************************************************************/
/*
/*                     组件
/*
/*             是游戏的功能模块
/*
/***************************************************************/

class CComponent
{
public:
	CComponent()
	{
		m_maxLoopTick =0;
		m_minLoopTick =0;
		m_lastTickCount =0;
		m_totalTickCount =0;
		m_tickLimit    =5; //默认5毫秒 
		m_times =0;
		m_componentName[0]=0; //组件的名字
	}
	
	virtual	bool  Initialize(){return true;}	 //组件的初始化函数,如果初始化失败返回False
	
	virtual VOID  Destroy(){}   ; 	 //组件的销毁函数，这里要消耗一些S分配的内存

	virtual ~CComponent(){}

	//设置本次循环的时间
	VOID SetLoopTickCount(TICKCOUNT nTick)
	{
		if(nTick <=0) return;
		if(m_maxLoopTick < nTick)
		{
			m_maxLoopTick = nTick; 
		}
		if (  nTick < m_minLoopTick  ) 
		{
			m_minLoopTick = nTick;
		}
		m_times ++;
		m_totalTickCount += nTick; 
		m_lastTickCount = nTick;
	}

	//设置每次循环的最大的时间
	VOID SetLoopTickLimit(TICKCOUNT tick)
	{
		m_tickLimit = tick;
	}

	//获取每次最大的时间限制
	TICKCOUNT GetLoopTickLimit()
	{
		return m_tickLimit;
	}

	//获取一次循环最少使用了多少时间
	inline TICKCOUNT GetMinTick()
	{
		return m_minLoopTick;
	}

	//获得一次循环最大使用了多少时间
	inline TICKCOUNT GetMaxTick()
	{
		return m_maxLoopTick;
	}
	//获取一次循环最后一次使用了多少时间
	inline TICKCOUNT GetLastTick()
	{
		return m_lastTickCount;
	}

	//获取每次循环平均使用了多少的时间
	inline TICKCOUNT GetAveTick()
	{
		if(m_times >0)
		{
			return m_totalTickCount/m_times;
		}
		else
		{
			return 0;
		}
	}
	//设置组件的名字
	VOID SetConponentName(LPCTSTR sName)
	{
		if(sName ==NULL) return ;
		strcpy(m_componentName,sName); //设置组件的名字
	}
	//获取组件的名字
	char * GetComponentName()
	{
		return m_componentName;
	}
	
protected:
	TICKCOUNT m_maxLoopTick;         //最久得一次循环占有了多长的时间
	TICKCOUNT m_minLoopTick;        // 最小的一次循环占有的时间
	TICKCOUNT m_lastTickCount;      //最新一次的时间
	TICKCOUNT m_totalTickCount;     //存起来的总时间
	TICKCOUNT m_times;              //累计起来调用了多少次
	TICKCOUNT m_tickLimit;          //每次循环最大的时间
	char      m_componentName[32];  //组件的名字
};