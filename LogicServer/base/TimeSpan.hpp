
//时间的管理器
class CTimeStatisticMgr
{
public:
	void AddStatictic(LPCTSTR name, TICKCOUNT tick)
	{
		
	}
};

//时间统计的类,
class CTimeSpan
{
public:
	CTimeSpan( LPCTSTR name)
	{
		m_func = name;
		m_tick = _getTickCount();
	}
	~CTimeSpan()
	{
		AddStatictic(m_func,_getTickCount() - m_tick); 
	}
private:
	TICKCOUNT m_tick; //花费的时间
	LPCTSTR   m_func; //函数的名字
};


#define TIME_CHECK()  CTimeSpan(__FUNCTION__) 