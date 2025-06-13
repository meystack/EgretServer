#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "MBCSDef.h"
#define toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) ^ 0x20 : (c))

//
//此函数用于统计表中共有多少个类似的结构
size_t STDCALL CountMBCSFilter(const PFT_MBCS_CHAR lpFilter)
{
	size_t szRet = 0;
	PFT_MBCS_CHAR pNextChar = 0;
	int i = 0;

	if(!lpFilter)
		return 0;
	
	for(i = 0; i < MBCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			szRet += CountMBCSFilter(pNextChar);
	}

	szRet++;	//这里是加一是因为自己也算一个
	return szRet;
}
//把一个字符串加入到屏蔽词表中
int STDCALL AddMBCSFilterStrToTable(PFT_MBCS_CHAR lpFilter,const char* pStrFilter)
{	
	int nLoop = 0;
	unsigned char nIndex = 0;
	int nStrLen = (int)strlen(pStrFilter);
	PFT_MBCS_CHAR pPrevFilter = lpFilter;
	PFT_MBCS_CHAR pNextFilter = 0;

	if(!lpFilter)
		return 0;

	for(nLoop = 0; nLoop < nStrLen; nLoop++)
	{					
		nIndex = pStrFilter[nLoop];
		nIndex = toupper(nIndex) - 1;
		pNextFilter = pPrevFilter->pNextChars[nIndex];
		if(!pNextFilter)
		{
			PFT_MBCS_CHAR pNewFilter = CreateMBCSFilter(0);
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
int SaveMBCSFilterStrToFile(PFT_MBCS_CHAR lpFilter,int nFilterIndex,FILE* fl,int* pnStrIndex,char* pFilterStr,int* pnFilterCount)
{	
	PFT_MBCS_CHAR pNextChar = 0;
	int bHaveChild = 0;
	int i = 0;
	char TempStr[256];
	memset(TempStr,0,sizeof(TempStr));

	if(!lpFilter)
		return 0;

	pFilterStr[*pnStrIndex] = (unsigned char)(nFilterIndex + 1);
	pFilterStr[(*pnStrIndex) + 1] = '\0';
	(*pnStrIndex)++;
	
	for(i = 0; i < MBCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
		{
			SaveMBCSFilterStrToFile(pNextChar,i,fl,pnStrIndex,pFilterStr,pnFilterCount);
			bHaveChild = 1;
		}
	}

	if(lpFilter->boWordEnd)
	{
		strcpy(TempStr,pFilterStr);		
		strcat(TempStr,"\r\n");	//换行符
		fputs(TempStr,fl);
		(*pnFilterCount)++;	

		(*pnStrIndex) -= 2;
		if((*pnStrIndex) < 0)	//这个时候已经回退到第一级子项
			(*pnStrIndex) = 0;
		if((*pnStrIndex) == 0)
			pFilterStr[0] = '\0';
		else
			pFilterStr[(*pnStrIndex) + 1] = '\0';
	}
	else if(bHaveChild)				//这种情况，出现在A,ABC都是屏蔽词，而AB不是屏蔽词的情况,回退到上一级
	{		
		(*pnStrIndex) -= 2;
		if((*pnStrIndex) < 0)	//这个时候已经回退到第一级子项
			(*pnStrIndex) = 0;
		if((*pnStrIndex) == 0)
			pFilterStr[0] = '\0';
		else
			pFilterStr[(*pnStrIndex) + 1] = '\0';
	}
	else	//这种情况，即没有子项，自身的boWordEnd又为FALSE值，应该是出现错误
	{
	}

	return 1;
}
//-----------------------------------------------------------------------
PFT_MBCS_CHAR STDCALL CreateMBCSFilter(void* lpReserved)
{
	PFT_MBCS_CHAR pMBCSChar = (PFT_MBCS_CHAR)malloc(sizeof(FT_MBCS_CHAR));
	memset(pMBCSChar,0,sizeof(FT_MBCS_CHAR));
	return pMBCSChar;
}

void STDCALL FreeMBCSFilter(PFT_MBCS_CHAR lpFilter)
{
	int i = 0;
	PFT_MBCS_CHAR pNextChar = 0;
	if(!lpFilter)
		return;

	for(i = 0; i < MBCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			FreeMBCSFilter(pNextChar);
	}

	free(lpFilter);
	lpFilter = 0;
}

size_t STDCALL MemoryUsageOfMBCSFilter(const PFT_MBCS_CHAR lpFilter)
{
	size_t szResult = 0;
	int nStructSize = sizeof(FT_MBCS_CHAR);	//每一个FT_MBCS_CHAR结构的大小
	size_t szStructCount = 0;

	if(!lpFilter)
		return 0;

	//计算内存大小思路：
	//先统计出总共有多少个FT_MBCS_CHAR结构
	//然后再用这个结构数量的大小来乘以单个结构大小

	szStructCount = CountMBCSFilter(lpFilter);
    szResult = (szStructCount * nStructSize);
	return szResult;
}

int STDCALL LoadMBCSFilterWords(PFT_MBCS_CHAR lpFilter, const char *sFileName)
{
	int nStrCount = 0;
	size_t nLen = 0;
	char szText[256] = {0};
	char* pStr = 0;
	FILE* fl = fopen(sFileName,"r");
	//检查并跳过UTF-8文件BOM
	fread(szText, 3, 1, fl);
	if ((*(int*)szText & 0x00FFFFFF) != 0xBFBBEF )
		fseek(fl, 0, 0);
	while(pStr = fgets(szText,256,fl))
	{
		nLen = strlen(szText);
		while ( nLen > 0 && ((unsigned int)szText[nLen-1] <= 0x20))
		{
			szText[nLen-1] = 0;
			nLen--;
		}
		if(nLen > 0 && AddMBCSFilterStrToTable(lpFilter,szText))
			++nStrCount;
	}

	fclose(fl);
	
	return nStrCount;
}

int STDCALL SaveMBCSFilterWords(PFT_MBCS_CHAR lpFilter, const char *sFileName)
{
	char szText[256] = {0};
	int nFilterCount = 0, nStrIndex = 0;
	int i = 0;
	PFT_MBCS_CHAR pNextChar = 0;

	FILE* fl = fopen(sFileName,"w+a");
	if(!fl)
		return 0;
	
	for(i = 0; i < MBCSARRAY_SIZE; i++)
	{
		pNextChar = lpFilter->pNextChars[i];
		if(pNextChar)
			SaveMBCSFilterStrToFile(pNextChar,i,fl,&nStrIndex,szText,&nFilterCount);
	}

	fclose(fl);

	return nFilterCount;
}

char* STDCALL MatchMBCSFilterWord(const PFT_MBCS_CHAR lpFilter, const char *pInput, int *pMatchLen)
{
	PFT_MBCS_CHAR pCurFTChar;	//当前匹配位置的屏蔽字符指针
	char *lpMatchStart = NULL;	//被匹配的输入字符串的起始字符指针
	char *lpMatchEnd = NULL;	//被匹配的字符串的结束字符之后的一个字符的指针,此值减去lpMatchStart即为被匹配字符串的长度
	char *pResult;
	char *pTest;
	unsigned char ch;

	while(ch = *pInput)
	{
		ch = toupper(ch);
		pCurFTChar = lpFilter->pNextChars[ch - 1];
		if ( pCurFTChar )
		{
			pTest = lpMatchStart = (char*)pInput;
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
		//对于MBCS，进入下一个字符应当跳过2个字符
		//if ( (unsigned char)(*pInput) >= 0x80 )
		//	pInput += 2;
		//else pInput += 1;
		pInput++;
	}

	pResult = (lpMatchEnd  ? lpMatchStart : NULL);
	if(pMatchLen)
		*pMatchLen = (int)(size_t)(pResult ? (lpMatchEnd - lpMatchStart) : 0);

	return pResult;
}

