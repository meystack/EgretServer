#pragma once

/************************************************************************
 
                使用32位的原子数据描述日期和时间的数据类型

   由于使用32位的原子数据，导致数据的存储值范围有限，因此时间值的最小单位是秒。对于一个
 时间值，本类型实际存储的就是自2010年1月1日0点到当前时间所经过的秒数！
   取得两个时间对象的时间差，可以直接使用减法运算。将一个日期值增加一定的时间，可以直接
 增加时间的秒数。

   ★实际上，数据存储的时间值使用0到30位，第31位表示此时间是否开始计时的记录标志位，例
 如一个装备在产生的时候肯能带有一个使用时限，时间值表示的是该装备可以使用的时长，例如1天，
 30天。而装备仅在第一次被玩家穿戴的时候才开始计时。当计时开始的时候，时间值被修改为使用
 时长加当前时间值，且最高位被置位来表示这个物品的时间已经开始了计时。

   ★短时间类型能够描述的最大日期为2079年1月1日。

************************************************************************/

#pragma pack (push, 1)

#include <string.h>
#include "_osdef.h"
struct CMiniDateTime
{
public:
	//定义短时间类型开始的年份
	static const int YearBase = 2010;	
	//定义时间是否已经开始计时的标志位
	static const unsigned int RecordFlag = 0x80000000;
	//定义各种单位的时间的秒数
	static const unsigned int SecOfMin  = 60;
	static const unsigned int SecOfHour = SecOfMin * 60;
	static const unsigned int SecOfDay  = SecOfHour * 24;
	static const unsigned int SecOfYear[2];
	static const unsigned int SecOfMonth[2][13];//以月为第二数组的下标，免去月份-1的操作！
	static const unsigned int MonthDays[2][13];//以月为第二数组的下标，免去月份-1的操作！

public:
	/* 从规定的日期和时间中设置短日期时间类型值
	 * year		公元年份，例如2012；
	 * month	月份，注意：1表示1月，而非0；
	 * day		日份，注意：1表示1号，而非0；
	 * hour		小时，0表示0点，23表示晚上11点；
	 * min		分钟，0表示0分；
	 * sec		秒钟，0表示0秒；
	 * ★★★运算函数不会改变时间的记录标志位
	 */
	inline CMiniDateTime& encode(const unsigned int year, const unsigned int mon, const unsigned int day, 
		const unsigned int hour, const unsigned int min, const unsigned int sec)
	{
		int i;
		unsigned int v = 0;

		//计算年份经过的秒数
		for (i=year - 1; i>=YearBase; --i)
		{
			v += SecOfYear[isLeapYear(i)];
		}

		//计算当年1月到本月的秒数
		bool boLeepYear = isLeapYear(year);
		for (i=mon -1; i>=1; --i)
		{
			v += SecOfMonth[boLeepYear][i];
		}

		//计算当年1号到今天的秒数
		v += (day - 1) * SecOfDay;

		//计算今天0点到现在的秒数
		v += hour * SecOfHour;

		//计算当前小时的0分到现在的秒数
		v += min * SecOfMin;

		v += sec;

		tv = (tv & RecordFlag) | (v & (~RecordFlag));
		return *this;
	}

	// 从MiniDataTime转化到系统时间。暂时只精确到天。
	inline void decode(SYSTEMTIME &sysTime)
	{		
		unsigned int v = tv & (~RecordFlag);
		
		// 年
		int year = YearBase;
		while (true)
		{
			unsigned int nSecOfCurrYear = SecOfYear[isLeapYear(year)];
			if (v >= nSecOfCurrYear)
			{
				v -= nSecOfCurrYear;
				year++;
			}
			else
				break;
		}

		// 月
		bool bIsLeepYear = isLeapYear(year);
		int month = 1;										// index based 1 <<<<
		while (true)
		{
			unsigned int nSecOfCurrMonth = SecOfMonth[bIsLeepYear][month];
			if (v >= nSecOfCurrMonth)
			{
				v -= nSecOfCurrMonth;
				month++;
			}
			else
				break;
		}

		// 日
		int day = v / SecOfDay + 1;								// index based 1 <<<<		
		
		memset(&sysTime, 0, sizeof sysTime);
		sysTime.wYear	= year;
		sysTime.wMonth	= month;
		sysTime.wDay	= day;
		//小时、分、秒
		UINT SecOfToday = v % (24 * 3600);
		sysTime.wHour = SecOfToday / 3600;
		sysTime.wMinute = (SecOfToday % 3600) / 60;
		sysTime.wSecond = SecOfToday % 60;
	}

	inline bool isSameDay(const CMiniDateTime &rhs)
	{		
		return  ((tv & (~RecordFlag)) / SecOfDay) == ((rhs.tv & (~RecordFlag)) / SecOfDay) ? true : false;
	}

	//从SYSTEMTIME结构中设置日期时间值
	inline CMiniDateTime& encode(const SYSTEMTIME& sysTime)
	{
		return encode(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	}
	//开始记录时间，timeOfNow参数表示当前时间的短时间类型值
	inline CMiniDateTime& startRecord(const unsigned int timeOfNow)
	{
		if ( !(tv & RecordFlag) )
			tv = RecordFlag | ((timeOfNow & (~RecordFlag)) + tv);
		return *this;
	}
	//获取当前日期时间的短时间值
	inline static unsigned int now()
	{
		CMiniDateTime tv;
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		tv.encode(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		return tv.tv;
	}
	//获取今天凌晨的时间
	inline static unsigned int today()
	{
		CMiniDateTime tv;
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		tv.encode(sysTime.wYear, sysTime.wMonth, sysTime.wDay, 0, 0, 0);
		return tv.tv;
	}
	//计算明日凌晨的值
	inline static unsigned int tomorrow()
	{
		int dayofMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
		CMiniDateTime tv;
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		sysTime.wDay++;
		if (sysTime.wMonth == 2 && isLeapYear(sysTime.wYear))
		{
			if (sysTime.wDay > 29)
			{
				sysTime.wMonth++;
				sysTime.wDay = 1;
			}
		}
		else if (sysTime.wDay > dayofMonth[sysTime.wMonth-1])
		{
			sysTime.wMonth++;
			sysTime.wDay = 1;
		}
		if (sysTime.wMonth > 12)
		{
			sysTime.wYear++;
			sysTime.wMonth = 1;
		}
		tv.encode(sysTime.wYear, sysTime.wMonth, sysTime.wDay, 0, 0, 0);
		return tv.tv;
	}

	// 计算相对于当前MiniDateTime所指示的时间点的明天
	inline unsigned int rel_tomorrow()
	{
		unsigned v = tv & (~RecordFlag);		
		v = (v + SecOfDay) / SecOfDay * SecOfDay;
		return (tv & RecordFlag) | v;
	}

	// // 计算相对于当前MiniDateTime所指示的时间点的当前开始时间点
	inline unsigned int rel_today()
	{
		unsigned v = tv & (~RecordFlag);		
		v = v / SecOfDay * SecOfDay;
		return (tv & RecordFlag) | v;
	}

	// // 计算相对于当前MiniDateTime所指示的时间点的当前开始时间点
	inline unsigned int rel_today(unsigned int hours, unsigned int mins, unsigned int seconds)
	{
		unsigned v = tv & (~RecordFlag);		
		v = v / SecOfDay * SecOfDay;
		return ((tv & RecordFlag) | v) + SecOfHour*hours + SecOfMin*mins + seconds;
	}

	//判断是否是闰年的函数
	inline static bool isLeapYear(const unsigned int year)
	{
		return ((year & 3) == 0) && ((year % 100 != 0) || (year % 400 == 0));
	}


public:
	//转换为unsigned int的类型转换运算符
	inline operator unsigned int() const
	{
		return tv;
	}
	//从unsigned int复制的运算符函数
	inline CMiniDateTime& operator = (const unsigned int time)
	{
		tv = time;
		return *this;
	}
	//重载相等判断的运算符
	inline bool operator == (const unsigned int time)
	{
		return tv == time;
	}
	//重载不等判断的运算符
	inline bool operator != (const unsigned int time)
	{
		return tv != time;
	}
	inline bool operator > (const unsigned int time)
	{
		return (tv & (~RecordFlag)) > (time & (~RecordFlag));
	}
	inline bool operator >= (const unsigned int time)
	{
		return (tv & (~RecordFlag)) >= (time & (~RecordFlag));
	}
	inline bool operator < (const unsigned int time)
	{
		return (tv & (~RecordFlag)) < (time & (~RecordFlag));
	}
	inline bool operator <= (const unsigned int time)
	{
		return (tv & (~RecordFlag)) <= (time & (~RecordFlag));
	}
	//重载+运算符，避免对是否使用的标志位进行运算
	inline unsigned int operator + (const unsigned int time)
	{
		return ((tv & (~RecordFlag)) + (time & (~RecordFlag))) & (~RecordFlag);
	}
	//重载-运算符，避免对是否使用的标志位进行运算
	inline unsigned int operator - (const unsigned int time)
	{
		return ((tv & (~RecordFlag)) - (time & (~RecordFlag))) & (~RecordFlag);
	}
	//重载+=运算符，避免对是否使用的标志位进行运算
	inline CMiniDateTime& operator += (const unsigned int time)
	{
		tv = (tv & RecordFlag) | (((tv & (~RecordFlag)) + (time & (~RecordFlag))) & (~RecordFlag));
		return *this;
	}
	//重载-=运算符，避免对是否使用的标志位进行运算
	inline CMiniDateTime& operator -= (const unsigned int time)
	{
		tv = (tv & RecordFlag) | (((tv & (~RecordFlag)) - (time & (~RecordFlag))) & (~RecordFlag));
		return *this;
	}
public:
	CMiniDateTime()
	{
		tv = 0;
	}
	CMiniDateTime(const CMiniDateTime &mt)
	{
		tv = mt.tv;
	}
	CMiniDateTime(const unsigned int tva)
	{
		tv = tva;
	}
public:
	unsigned int tv;
};

#pragma pack (pop)
