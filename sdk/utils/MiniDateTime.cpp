#ifdef WIN32
#include <Windows.h>
#endif

#include "MiniDateTime.h"

const unsigned int CMiniDateTime::SecOfYear[2] = { 365 * SecOfDay, 366 * SecOfDay };

const unsigned int CMiniDateTime::SecOfMonth[2][13] = 
{
	{0, 31 * SecOfDay, 28 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay},
	{0, 31 * SecOfDay, 29 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay, 30 * SecOfDay, 31 * SecOfDay}
};

const unsigned int CMiniDateTime::MonthDays[2][13] =
{
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};
