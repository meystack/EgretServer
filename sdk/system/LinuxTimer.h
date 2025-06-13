 
#ifndef __SYSTEM_LINUXTIMER_H__
#define __SYSTEM_LINUXTIMER_H__

#include <sys/time.h>
//主要是系统的一些API的集成 
#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct _SYSTEMTIME 
{
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

void GetLocalTime(PSYSTEMTIME pTime)
{
	time_t now;
	time(&now);
	struct tm timenow;
	localtime_r(&now,&timenow);
	pTime->wYear = (unsigned short)timenow.tm_year + 1900;
	pTime->wMonth = (unsigned short)timenow.tm_mon +1;
	
	pTime->wDayOfWeek =(unsigned short) timenow.tm_wday;
	pTime->wDay =(unsigned short)timenow.tm_mday;
	pTime->wHour = (unsigned short)timenow.tm_hour;
	pTime->wMinute= (unsigned short)timenow.tm_min;
	pTime->wSecond=(unsigned short)timenow.tm_sec;
	pTime->wMilliseconds= 0;
}

void GetLocalTimeFull(PSYSTEMTIME pTime)
{
	time_t now;
	time(&now);
	struct tm timenow;
	localtime_r(&now,&timenow);
	pTime->wYear = (unsigned short)timenow.tm_year + 1900;
	pTime->wMonth = (unsigned short)timenow.tm_mon +1;
	
	pTime->wDayOfWeek =(unsigned short) timenow.tm_wday;
	pTime->wDay =(unsigned short)timenow.tm_mday;
	pTime->wHour = (unsigned short)timenow.tm_hour;
	pTime->wMinute= (unsigned short)timenow.tm_min;
	pTime->wSecond=(unsigned short)timenow.tm_sec;
	pTime->wMilliseconds= (unsigned short)timenow.tm_sec;
	
	struct timeval tv;
	gettimeofday(&tv, NULL);     // NULL is only legal value
	pTime->wMilliseconds = tv.tv_usec / 1000;     // Get Millisecond;
}
  
// 高精度计时器 
class CGameTimer
{
public:
	CGameTimer()
	{
		m_bInit = false;
		m_dLastTime = 0.0;
	}
	
	~CGameTimer()
	{
	}

	// 初始化
	void Initialize()
	{
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		
		m_dLastTime = ToSeconds(&tv);
		m_bInit = true;
	}
	
	// 高精度计时器是否可用
	bool CanUseQPF() const
	{
		return true;
	}
	
	// 是否使用高精度计时器
	void SetUseQPF(bool flag)
	{
	}
	
	// 当前时间数值
	double GetCurrentTime()
	{
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		
		return ToSeconds(&tv);
	}
	
	// 获得逝去的秒数
	double GetElapseTime(double expect = 0.0)
	{
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		
		double now = ToSeconds(&tv);
		double elapse = now - m_dLastTime;
		
		if (elapse >= expect)
		{
			m_dLastTime = now;
		}
		
		return elapse;
	}
	
	// 获得逝去的毫秒数
	int GetElapseMillisec(int expect = 0)
	{
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		
		double now = ToSeconds(&tv);
		double elapse = now - m_dLastTime;
		
		if (elapse >= expect)
		{
			m_dLastTime = now;
		}
		
		return (int)(elapse * 1000.0);
	}
	
private:
	double ToSeconds(struct timeval* tv)
	{
		return (double)tv->tv_sec + (double)tv->tv_usec * 0.000001;
	}
	
private:
	bool m_bInit;
	double m_dLastTime;
};

#ifdef __cplusplus
}
#endif
 
#endif
