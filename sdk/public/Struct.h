#ifndef STRUCT_HEAD_FILE
#define STRUCT_HEAD_FILE
 
//////////////////////////////////////////////////////////////////////////////////
//游戏列表
#pragma pack(1)


struct systemtime {
	word wYear;
	word wMonth;
	word wDayOfWeek;
	word wDay;
	word wHour;
	word wMinute;
	word wSecond;
	word wMilliseconds;
};
  
#define PROPERTY_COUNT			(sizeof(g_PropTypeList)/sizeof(g_PropTypeList[0]))//道具数目

            

#pragma pack()

#endif