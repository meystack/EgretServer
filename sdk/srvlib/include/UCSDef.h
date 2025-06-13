#ifndef _UCSDEF_H_
#define _UCSDEF_H_

#define UCSARRAY_SIZE 65535

typedef struct tagFT_ucs_char
{
    char   boWordEnd;// 到此字符是否构成一个完整的屏蔽词
	char    btReserve[3];//保留
	struct tagFT_ucs_char* pNextChars[UCSARRAY_SIZE]; //当前字符的后续字符表，访问索引为字符值-1
}FT_UCS_CHAR, *PFT_UCS_CHAR;

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

//UCS字符集函数
PFT_UCS_CHAR STDCALL CreateUCSFilter(void* lpReserved);
void STDCALL FreeUCSFilter(PFT_UCS_CHAR lpFilter);
size_t STDCALL MemoryUsageOfUCSFilter(const PFT_UCS_CHAR lpFilter);
int STDCALL LoadUCSFilterWords(PFT_UCS_CHAR lpFilter, const wchar_t *sFileName);
int STDCALL SaveUCSFilterWords(PFT_UCS_CHAR lpFilter, const wchar_t *sFileName);
wchar_t* STDCALL MatchUCSFilterWord(const PFT_UCS_CHAR lpFilter, const wchar_t *pInput, int *pMatchLen);
//把一个字符串加入到屏蔽词表中
int STDCALL AddUCSFilterStrToTable(PFT_UCS_CHAR lpFilter,const wchar_t* pStrFilter);

#ifdef __cplusplus
}
#endif

#endif

