#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "UCSDef.h"
#define toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) ^ 0x20 : (c))

//
//此函数用于统计表中共有多少个类似的结构
size_t STDCALL CountUCSFilter(const PFT_UCS_CHAR lpFilter)
{
	size_t szRet = 0;
	PFT_UCS_CHAR pNextChar = 0;
	int i = 0;

	if(!lpFilter)
		return 0;

	for(i = 0; i < UCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			szRet += CountUCSFilter(pNextChar);
	}

	szRet++;	//这里是加一是因为自己也算一个
	return szRet;
}
//把一个字符串加入到屏蔽词表中
int STDCALL AddUCSFilterStrToTable(PFT_UCS_CHAR lpFilter,const wchar_t* pStrFilter)
{
	int nLoop = 0;
	unsigned short nIndex = 0;
	int nStrLen = (int)wcslen(pStrFilter);
	PFT_UCS_CHAR pPrevFilter = lpFilter;
	PFT_UCS_CHAR pNextFilter = 0;

	if(!lpFilter)
		return 0;
	
	for(nLoop = 0; nLoop < nStrLen; nLoop++)
	{					
		nIndex = pStrFilter[nLoop];
		nIndex = toupper(nIndex) - 1;
		pNextFilter = pPrevFilter->pNextChars[nIndex];
		if(!pNextFilter)
		{
			PFT_UCS_CHAR pNewFilter = CreateUCSFilter(0);
			pPrevFilter->pNextChars[nIndex] = pNewFilter;
			pNextFilter = pPrevFilter->pNextChars[nIndex];
		}
		if(nLoop == (nStrLen - 1))
			pNextFilter->boWordEnd = 1;
		pPrevFilter = pNextFilter;
	}

	return 1;
}
//把一个屏蔽字符串保存到文件中
//pFilterStr是个256大小的字符数组(包括结束符)
int SaveUCSFilterStrToFile(PFT_UCS_CHAR lpFilter,int nFilterIndex,FILE* fl,int* pnStrIndex,wchar_t* pFilterStr,int* pnFilterCount)
{	
	wchar_t TempStr[256];
	PFT_UCS_CHAR pNextChar = 0;
	int bHaveChild = 0;
	int i = 0;

	if(!lpFilter)
		return 0;

	memset(TempStr,0,sizeof(TempStr));

	pFilterStr[*pnStrIndex] = (unsigned short)(nFilterIndex + 1);
	pFilterStr[(*pnStrIndex) + 1] = L'\0';
	(*pnStrIndex)++;
	
	for(i = 0; i < UCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
		{
			SaveUCSFilterStrToFile(pNextChar,i,fl,pnStrIndex,pFilterStr,pnFilterCount);
			bHaveChild = 1;
		}
	}

	if(lpFilter->boWordEnd)
	{
		wcscpy(TempStr,pFilterStr);		
		wcscat(TempStr,L"\r\n");			
		fputws(TempStr,fl);
		(*pnFilterCount)++;	

		(*pnStrIndex) -= 2;
		if((*pnStrIndex) < 0)	//这个时候已经回退到第一级子项
			(*pnStrIndex) = 0;
		if((*pnStrIndex) == 0)
			pFilterStr[0] = '\0';
		else
			pFilterStr[(*pnStrIndex) + 1] = L'\0';
	}
	else if(bHaveChild)				//这种情况，出现在A,ABC都是屏蔽词，而AB不是屏蔽词的情况,回退到上一级
	{		
		(*pnStrIndex) -= 2;
		if((*pnStrIndex) < 0)	//这个时候已经回退到第一级子项
			(*pnStrIndex) = 0;
		if((*pnStrIndex) == 0)
			pFilterStr[0] = '\0';
		else
			pFilterStr[(*pnStrIndex) + 1] = L'\0';
	}
	else	//这种情况，即没有子项，自身的boWordEnd又为FALSE值，应该是出现错误
	{
	}

	return 1;
}
//-----------------------------------------------------------------------
PFT_UCS_CHAR STDCALL CreateUCSFilter(void* lpReserved)
{
	PFT_UCS_CHAR pUCSSChar = (PFT_UCS_CHAR)malloc(sizeof(PFT_UCS_CHAR));
	memset(pUCSSChar,0,sizeof(PFT_UCS_CHAR));
	return pUCSSChar;
}

void STDCALL FreeUCSFilter(PFT_UCS_CHAR lpFilter)
{
	PFT_UCS_CHAR pNextChar = 0;
	int i = 0;

	if(!lpFilter)
		return;
	
	for(i = 0; i < UCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			FreeUCSFilter(pNextChar);
	}

	free(lpFilter);
	lpFilter = 0;
}

size_t STDCALL MemoryUsageOfUCSFilter(const PFT_UCS_CHAR lpFilter)
{
	size_t szResult = 0;
	size_t szStructCount = 0;
	int nStructSize = sizeof(FT_UCS_CHAR);	//每一个FT_UCS_CHAR结构的大小

	if(!lpFilter)
		return 0;

	//计算内存大小思路：
	//先统计出总共有多少个FT_UCS_CHAR结构
	//然后再用这个结构数量的大小来乘以单个结构大小

	szStructCount = CountUCSFilter(lpFilter);
    szResult = (szStructCount * nStructSize);
	return szResult;
}

int STDCALL LoadUCSFilterWords(PFT_UCS_CHAR lpFilter, const wchar_t *sFileName)
{
#ifdef WIN32
	char szText[256] ={0};	
	char chRet;
	int nLoop = 0;	
	FILE* fl = (FILE*)_wfopen(sFileName,L"rb,ccs=Unicode");


	int nStrCount = 0;
	size_t nLen = 0;	
	do
	{
		chRet = fgetc(fl);
		szText[nLoop] = chRet;		
		if(chRet == '\n')
		{
			szText[nLoop] = '\0';			
			nLen = strlen(szText);
			while ( nLen > 0 && szText[nLen-1] == '\r' )
			{
				szText[nLen-1] = 0;
				nLen--;
			}
			if(nLen > 0 && AddUCSFilterStrToTable(lpFilter,(const wchar_t*)szText))
				++nStrCount;

			memset(szText,0,sizeof(szText));
			nLoop = 0;
		}
		else
			++nLoop;
	}while(chRet != EOF);
	
	return nStrCount;
#else
	//linux mot support
	return 0;
#endif
}

int STDCALL SaveUCSFilterWords(PFT_UCS_CHAR lpFilter, const wchar_t *sFileName)
{
#ifdef WIN32
	wchar_t szText[256] = {0};
	int nFilterCount = 0, nStrIndex = 0;
	FILE* fl = (FILE*)_wfopen(sFileName,L"w+a,ccs=Unicode");	
	PFT_UCS_CHAR pNextChar = 0;
	int i = 0;

	if(!fl)
		return 0;
	
	for(i = 0; i < UCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			SaveUCSFilterStrToFile(pNextChar,i,fl,&nStrIndex,szText,&nFilterCount);
	}

	fclose(fl);
	return nFilterCount;
#else
	return 0;

#endif
}


wchar_t* STDCALL MatchUCSFilterWord(const PFT_UCS_CHAR lpFilter, const wchar_t *pInput, int *pMatchLen)
{
	PFT_UCS_CHAR pCurFTChar;		//当前匹配位置的屏蔽字符指针
	wchar_t *lpMatchStart = NULL;	//被匹配的输入字符串的起始字符指针
	wchar_t *lpMatchEnd = NULL;		//被匹配的字符串的结束字符之后的一个字符的指针,此值减去lpMatchStart即为被匹配字符串的长度
	wchar_t* pResult;
	wchar_t* pTest;
	unsigned short ch;

	while(ch = *pInput)
	{
		ch = toupper(ch);
		pCurFTChar = lpFilter->pNextChars[ch - 1];
		if ( pCurFTChar )
		{
			pTest = lpMatchStart = (wchar_t*)pInput;
			do
			{
				pTest++;
				//到该字符是否处构成了完整的匹配词
				if ( pCurFTChar->boWordEnd )
					lpMatchEnd = pTest;
				//继续向后匹配
				ch = *pTest;
				ch = toupper(ch);
				pCurFTChar = pCurFTChar->pNextChars[ch - 1];
			}
			while ( pCurFTChar && *pTest );
			//出现了完整的匹配词则终止
			if ( lpMatchEnd )
				break;
		}
		pInput++;
	}

	pResult = (lpMatchEnd  ? lpMatchStart : NULL);
	if(pMatchLen)
		*pMatchLen = (int)(size_t)(pResult ? (lpMatchEnd - lpMatchStart) : 0);

	return pResult;
}

