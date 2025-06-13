#ifndef _MBCSDEF_H_
#define _MBCSDEF_H_

#define MBCSARRAY_SIZE 255

typedef struct tagFT_mbcs_char
{
	char	boWordEnd;										//到此字符是否构成一个完整的屏蔽词
    char    btReserve[3];									//保留
	struct tagFT_mbcs_char* pNextChars[MBCSARRAY_SIZE];		//当前字符的后续字符表，访问索引为字符值-1
}FT_MBCS_CHAR, *PFT_MBCS_CHAR;

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef STDCALL
#if (defined(WIN32) || defined(WINDOWS))
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

/*
创建一个MBCS屏蔽词表
*/
PFT_MBCS_CHAR STDCALL CreateMBCSFilter(void* lpReserved);
/*
销毁一个MBCS屏蔽词表，释放表使用的所有内存
*/
void STDCALL FreeMBCSFilter(PFT_MBCS_CHAR lpFilter);
/*
统计屏蔽词表使用的内存量
*/
size_t STDCALL MemoryUsageOfMBCSFilter(const PFT_MBCS_CHAR lpFilter);
//
int STDCALL LoadMBCSFilterWords(PFT_MBCS_CHAR lpFilter, const char *sFileName);
int STDCALL SaveMBCSFilterWords(PFT_MBCS_CHAR lpFilter, const char *sFileName);
char* STDCALL MatchMBCSFilterWord(const PFT_MBCS_CHAR lpFilter, const char *sInput, int *pMatchLen);
//把一个字符串加入到屏蔽词表中
int STDCALL AddMBCSFilterStrToTable(PFT_MBCS_CHAR lpFilter,const char* pStrFilter);

#ifdef __cplusplus
}
#endif

#endif

