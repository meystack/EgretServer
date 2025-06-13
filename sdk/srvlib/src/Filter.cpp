#include "CFilter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SETBIT(a, n) (a[n >> 3] |= (1<<( n & 0x7)))
#define GETBIT(a, n) (a[n >> 3] & (1<<( n & 0x7)))
#define toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) ^ 0x20 : (c))



int CFilter::bloom_destroy(BLOOM *bloom)
{
	free(bloom->a);
	free(bloom);

	return 0;
}

BLOOM * CFilter::bloom_create(size_t size, size_t nfuncs)
{
	BLOOM *bloom;
	

	if(!(bloom=(BLOOM *)malloc(sizeof(BLOOM)))) return NULL;

	size_t nSize = (size+CHAR_BIT-1)/CHAR_BIT;
	bloom->a= (unsigned char*)calloc(nSize, sizeof(char));

	if( bloom->a ==NULL ) 
	{
		free(bloom);
		return NULL;
	}
	bloom->nfuncs=nfuncs;
	bloom->asize=size;

	memset(&m_Flags,0,sizeof(m_Flags));
	/*
	m_funcs[0]= &CFilter::BzHash1;
	m_funcs[1]= &CFilter::BzHash2;
	m_funcs[2]= &CFilter::SaxHash  ;
	m_funcs[3]= &CFilter::JSHash;
	m_funcs[4]= &CFilter::PJWHash;
	m_funcs[5]= &CFilter::ELFHash;
	m_funcs[6]= &CFilter::BKDRHash;
	m_funcs[7]= &CFilter::SDBMHash;
	m_funcs[8]= &CFilter::DJBHash;
	m_funcs[9]= &CFilter::DEKHash;
	m_funcs[10]= &CFilter::BPHash;
	m_funcs[11]= &CFilter::FNVHash;
	m_funcs[12]= &CFilter::RSHash;
	m_funcs[13]= &CFilter::APHash;
	m_funcs[14]= &CFilter::SdbmHash;
	*/

	return bloom;
}

int CFilter::bloom_add(BLOOM *bloom,  char *s,unsigned int len)
{
	size_t n;
	unsigned long a , b, i,h ;
	
	for(n=0; n<bloom->nfuncs; ++n) 
	{
		//这里为了效率牺牲了可读性，避免频繁调用函数，如果要修改，需要与下面的bloom_check函数一起修改
		char * str = s;
		
		
		////////////////////////////////////////分割线////////////////////////////
		////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
		switch( n)
		{

		case 0:  //SaxHash
			h=0;
			for(i=0; i < len; i++, str++)
			{
				h^=(h<<5)+(h>>2)+(unsigned char)(*str); 
			}
			break;

		case 1:  //SdbmHash
			h=0;
			for(i=0; i < len; i++,str ++)
			{
				h=(unsigned char)(*str) + (h<<6) + (h<<16) - h;
			}
			break;

		case 2:  //RSHash
			b    = 378551;
			a    = 63689;
			h = 0;
			for(i=0; i < len; str++, i++)
			{
				h = h * a + (*str);
				a    = a * b;
			}
			break;

		case 3:  //JSHash
			h = 1315423911;
			for(i = 0; i < len; str++, i++)
			{
				h ^= ((h << 5) + (*str) + (h >> 2));
			}
			break;



		case 4:  //ELFHash 
			h= 0;
			a= 0;
			for(i=0; i < len; str++, i++)
			{
				h = (h << 4) + (*str);
				if((a = h & 0xF0000000L) != 0)
				{
					h ^= (a >> 24);
				}
				h &= ~a;
			}
			break;



		case 5:  //BKDRHash
			a = 131; /* 31 131 1313 13131 131313 etc.. */
			h = 0;
			for(i=0; i < len; str++, i++)
			{
				h = (h * a) + (*str);
			}
			break;

		case 6:  //SDBMHash
			h = 0;
			for(i=0;i < len; str++, i++)
			{
				h = (*str) + (h << 6) + (h << 16) - h;
			}
			break;

		case 7:  //DJBHash
			h = 5381;
			for(i=0; i < len; str++, i++)
			{
				h = ((h << 5) + h) + (*str);
			}
			break;

		case 8:  //DEKHash
			h = len;
			for(i=0; i < len; str++, i++)
			{
				h = ((h << 5) ^ (h >> 27)) ^ (*str);
			}
			break;

		case 9:  //BPHash
			h = 0;
			for(i = 0; i < len; str++, i++)
			{
				h = h << 7 ^ (*str);
			}
			break;

		case 10:  //FNVHash
			a = 0x811C9DC5;
			h = 0;
			for(i=0; i < len; str++, i++)
			{
				h *= a;
				h ^= (*str);
			}
			break;

		case 11:  //APHash
			h = 0xAAAAAAAA;
			for(i=0;i < len; str++, i++)
			{
				h ^= ((i & 1) == 0) ? (  (h <<  7) ^ (*str) * (h >> 3)) :
					(~((h << 11) + ((*str) ^ (h >> 5))));
			}
			break;

		default:
			h=0;
			break;
		}
		////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
		////////////////////////////////////////分割线////////////////////////////



		h %= m_pBloom->asize;
		SETBIT(bloom->a,h );
	}

	unsigned char ch;
	unsigned int nTotalLen =0,charLen;
	unsigned int value;
	

	for ( char * sptr = s; *sptr && nTotalLen < len; )
	{
		ch = (unsigned char)*sptr;
		value =0;
		
		if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
		{
			charLen =3;
		}
		else if( (ch & 0x80) == 0x0) //1个字节的0xxxx
		{
			charLen =1;
		}
		else if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			charLen =5;
		}
		else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			charLen =4;
		}
		else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
		{
			charLen =2;
		}
		else
		{
			charLen =1;
		}

		if(charLen >=3)
		{
			value =  (((unsigned char)*(sptr )) << 16) +  (((unsigned char)*(sptr + 1)) << 8) +  ((unsigned char)*(sptr + 2));
		}
		else if(charLen ==2)
		{
			value = ( ((unsigned char)*(sptr )) << 8 )+  ((unsigned char)*(sptr + 1));
			
		}
		else if(charLen ==1)
		{
			value = (unsigned char)*(sptr );
		}

		sptr += charLen; //往后几个字符
		nTotalLen += charLen; //utf8的个数

		//这个词出现过
		if(value < MAX_BIT_VALUE && value >=0)
		{
			SETBIT(m_Flags,value );
		}
		
	}

	return 0;
}


bool CFilter::Init(int nSize ,int nHashFunCount)
{
	if(m_pBloom != NULL)
	{
		bloom_destroy(m_pBloom);
		m_pBloom =NULL;
	}
	if(nSize <=0  )return false;

	//这hash函数的个数错误
	if(nHashFunCount <1 ) 
	{
		nHashFunCount =1;
	}
	else if(nHashFunCount > MAX_HASH_FUC_COUNT ) //超过了最大数目了
	{
		nHashFunCount = MAX_HASH_FUC_COUNT; 
	}

	m_pBloom = bloom_create(nSize,nHashFunCount);
	return m_pBloom != NULL;
}

int CFilter::Load(char *sFileName,int nSize ,int nHashFunCount)
{
	if(Init(nSize,nHashFunCount) ==false) return -1;
	
	int nStrCount = 0;
	size_t nLen = 0;
	char szText[256] = {0};
	char* pStr = 0;
	FILE* fl = fopen(sFileName,"r");
	if(fl ==NULL) return -3;

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
		if(nLen > 0 )
		{
			++nStrCount;
			bloom_add(m_pBloom,szText,nLen);
		}
	}

	fclose(fl);

	return nStrCount;
}



void  CFilter::Destroy()
{
	if(m_pBloom)
	{
		bloom_destroy(m_pBloom);
		m_pBloom =NULL;
	}
}

bool CFilter::AddFilterWord(char *sStr)
{
	if(sStr ==NULL || m_pBloom ==NULL) return false;
	bloom_add(m_pBloom,sStr,strlen(sStr));
	return true;
}

char* CFilter::Filter( char *pInput,int *pMatchLen)
{
	//char *lpMatchStart = pInput;	//被匹配的输入字符串的起始字符指针

	if(pInput ==NULL || pInput[0] ==0   ) return NULL;
	if(m_pBloom==NULL) return NULL;
	
	size_t  nStrLen = strlen(pInput);
	
	const static int MAX_BUFF_LEN =512; //最大处理的字符的长度

	char  lenArray[MAX_BUFF_LEN];    //每个utf8字符的长度

	char  isFilter[MAX_BUFF_LEN];    //单字是否在屏蔽字库里出现过 

	if(nStrLen >= MAX_BUFF_LEN)
	{
		return NULL;
	}

	int nUtf8Len =0; //utf8字符的个数
	int nTotalLen =0;
	//先找出有多少个utf8字符，每个字符占用的长度
	unsigned char ch;
	unsigned char charLen;
	unsigned int value;
	for (register char * sptr = pInput; *sptr && nTotalLen < nStrLen; )
	{
		value =0;
		
		ch = (unsigned char)*sptr;
		if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
		{
			charLen =3;
		}
		else if( (ch & 0x80) == 0x0) //1个字节的0xxxx
		{
			charLen =1;
		}
		else if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			charLen =5;
		}
		else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		{
			charLen =4;
		}
		else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
		{
			charLen =2;
		}
		else
		{
			charLen =1;
		}

		//这里判断这个词在屏蔽字库里是否出现过，如果出现过，则是1.否则是0
		
		if(charLen >=3)
		{
			value =  (((unsigned char)*(sptr )) << 16) +  (((unsigned char)*(sptr + 1)) << 8) +  ((unsigned char)*(sptr + 2));
		}
		else if(charLen ==2)
		{
			value = ( ((unsigned char)*(sptr )) << 8 )+  ((unsigned char)*(sptr + 1));

		}
		else if(charLen ==1)
		{
			value = (unsigned char)*(sptr );
		}

		if( GETBIT(m_Flags,value) )
		{
			isFilter[nUtf8Len] =1;
		}
		else
		{
			isFilter[nUtf8Len] =0;
		}
		
		sptr += charLen; //往后几个字符
		nTotalLen += charLen; //utf8的个数
		lenArray[nUtf8Len] = (char)charLen;
		
		nUtf8Len ++;
	}
	
	
	//对utf-8序列进行处理
	
	//这些是计算hash函数的时候用到的
	unsigned long a , b, i,len,h ;

	int headPos =0,nHeadLen=0;

	for( headPos=0; headPos< nUtf8Len; nHeadLen += lenArray[headPos],headPos++)
	{
		if(isFilter[headPos] ==0)  //如果这个词没有出现过，直接pass
		{
			continue;
		}
		
		int nTailLen =0; //尾部的长度
		
		for(int j= headPos ; j < nUtf8Len; j++ )
		{
			nTailLen  += lenArray[j]; //该字符的长度

			//如果这个词没有出现的话
			
			if(isFilter[j] ==0)  //这个字符没出现
			{
				for(int k=headPos ; k< j; k++ )
				{
					nHeadLen += lenArray[k];
					headPos++;
				}
				break;
			}
			
			//这里减少函数调用，提高执行速度
			bool flag =true;
			char *sStartPtr =  pInput+nHeadLen ;
			len = (unsigned int)nTailLen;
			

			for( size_t n=0; n< m_pBloom->nfuncs; ++n) 
			{
				char * str = sStartPtr;
				
				////////////////////////////////////////分割线////////////////////////////
				////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
				switch( n)
				{

				case 0:  //SaxHash
					h=0;
					for(i=0; i < len; i++, str++)
					{
						h^=(h<<5)+(h>>2)+(unsigned char)(*str); 
					}
					break;

				case 1:  //SdbmHash
					h=0;
					for(i=0; i < len; i++,str ++)
					{
						h=(unsigned char)(*str) + (h<<6) + (h<<16) - h;
					}
					break;

				case 2:  //RSHash
					b    = 378551;
					a    = 63689;
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h = h * a + (*str);
						a    = a * b;
					}
					break;

				case 3:  //JSHash
					h = 1315423911;
					for(i = 0; i < len; str++, i++)
					{
						h ^= ((h << 5) + (*str) + (h >> 2));
					}
					break;



				case 4:  //ELFHash 
					h= 0;
					a= 0;
					for(i=0; i < len; str++, i++)
					{
						h = (h << 4) + (*str);
						if((a = h & 0xF0000000L) != 0)
						{
							h ^= (a >> 24);
						}
						h &= ~a;
					}
					break;



				case 5:  //BKDRHash
					a = 131; /* 31 131 1313 13131 131313 etc.. */
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h = (h * a) + (*str);
					}
					break;

				case 6:  //SDBMHash
					h = 0;
					for(i=0;i < len; str++, i++)
					{
						h = (*str) + (h << 6) + (h << 16) - h;
					}
					break;

				case 7:  //DJBHash
					h = 5381;
					for(i=0; i < len; str++, i++)
					{
						h = ((h << 5) + h) + (*str);
					}
					break;

				case 8:  //DEKHash
					h = len;
					for(i=0; i < len; str++, i++)
					{
						h = ((h << 5) ^ (h >> 27)) ^ (*str);
					}
					break;

				case 9:  //BPHash
					h = 0;
					for(i = 0; i < len; str++, i++)
					{
						h = h << 7 ^ (*str);
					}
					break;

				case 10:  //FNVHash
					a = 0x811C9DC5;
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h *= a;
						h ^= (*str);
					}
					break;

				case 11:  //APHash
					h = 0xAAAAAAAA;
					for(i=0;i < len; str++, i++)
					{
						h ^= ((i & 1) == 0) ? (  (h <<  7) ^ (*str) * (h >> 3)) :
							(~((h << 11) + ((*str) ^ (h >> 5))));
					}
					break;

				default:
					h=0;
					break;
				}
				////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
				////////////////////////////////////////分割线////////////////////////////

				h %= m_pBloom->asize;

				if(!(GETBIT(m_pBloom->a, h)))
				{
					flag =false;
					break;
				}
			}
			if(flag)
			{
				*pMatchLen = len;
				return pInput +nHeadLen ;
			}
		}
	}
	
	return NULL;
}

int GetUtf8Length(unsigned char ch)
{
	if( (ch & 0xE0) == 0xE0)//3个字节1110xxxx 10xxxxxx 10xxxxxx
	{
		return 3;
	}
	else if( (ch & 0x80) == 0x0) //1个字节的0xxxx
	{
		return 1;
	}
	else if((ch & 0xF8) == 0xF8) //5个字节111110xx 10xxxxxx 10xxxxxx 10xxxxxx
	{
		return 5;
	}
	else if( (ch & 0xF0) == 0xF0 )//4个字节11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	{
		return 4;
	}
	else if( (ch & 0xC0) == 0xC0 )//2个字节110xxxxx 10xxxxxx
	{
		return 2;
	}

	return 1;
}

char* CFilter::Filter2( char *pInput,int *pMatchLen)
{
	if( pInput == NULL 
		|| pInput[0] == 0) 
	{ 
		return NULL;
	}
	if(m_pBloom == NULL)
	{
		return NULL;
	}
	
	const static int MAX_BUFF_LEN = 512; //最大处理的字符的长度
	char  lenArray[MAX_BUFF_LEN];    //每个utf8字符的长度
	char  isFilter[MAX_BUFF_LEN];    //单字是否在屏蔽字库里出现过 

	memset(lenArray, 0, sizeof(lenArray));
	memset(isFilter, 0, sizeof(isFilter));

	char pInput2[MAX_BUFF_LEN];
	memset(pInput2, 0, sizeof(pInput2));
	int  nStrLen2 = 0;
	int  nchecklen2 = 0;
	char  lenArray2[MAX_BUFF_LEN];
	char  isFilter2[MAX_BUFF_LEN];
	char  lenMap[MAX_BUFF_LEN];
	memset(lenArray2, 0, sizeof(lenArray2));
	memset(isFilter2, 0, sizeof(isFilter2));
	memset(lenMap, 0, sizeof(lenMap));

	size_t nStrLen = strlen(pInput);
	if(nStrLen >= MAX_BUFF_LEN) 
	{
		return NULL;
	}

	int nUtf8Len = 0; //utf8字符的个数
	int nTotalLen = 0;

	int nUtf8Len2 = 0; //utf8字符的个数
	int nTotalLen2 = 0;

	//先找出有多少个utf8字符，每个字符占用的长度
	unsigned char ch;
	unsigned char charLen;
	unsigned int value;
	bool iniflag = false;
	lenMap[0] = 0;
	int checkout = 0;//防止死循环
	for (register char * sptr = pInput; *sptr && nTotalLen < nStrLen;)
	{
		value = 0;
		ch = (unsigned char)*sptr;
		charLen = GetUtf8Length(ch);
 
		if (charLen > (nStrLen - (sptr - pInput)))
		{
			charLen = (nStrLen - (sptr - pInput));//如果字符串超过了
		}
		
		if (charLen > 1)
		{
			memcpy(pInput2 + nStrLen2, sptr, charLen);
			nStrLen2 += charLen;
			iniflag = true;
			lenMap[nUtf8Len2+1] = lenMap[nUtf8Len2];
		}
		else if (!iniflag)
		{
			lenMap[0] += 1;
		}

		//这里判断这个词在屏蔽字库里是否出现过，如果出现过，则是1.否则是0
		if(charLen >= 3)
		{
			value =  (((unsigned char)*(sptr )) << 16) +  (((unsigned char)*(sptr + 1)) << 8) +  ((unsigned char)*(sptr + 2));
		}
		else if(charLen ==2)
		{
			value = ( ((unsigned char)*(sptr )) << 8 )+  ((unsigned char)*(sptr + 1));
		}
		else if(charLen ==1)
		{
			value = (unsigned char)*(sptr );
		}

		if( GETBIT(m_Flags,value) )
		{
			isFilter[nUtf8Len] =1;
		}
		else
		{
			isFilter[nUtf8Len] =0;
		}
		
		sptr += charLen; //往后几个字符
		nTotalLen += charLen; //utf8的个数
		lenArray[nUtf8Len] = (char)charLen;
		nUtf8Len ++;

		if (charLen > 1)
		{
			if( GETBIT(m_Flags,value) ) isFilter2[nUtf8Len2] = 1;
			else isFilter2[nUtf8Len2] = 0;
			
			lenArray2[nUtf8Len2] = (char)charLen;
			nTotalLen2 += charLen;
			nUtf8Len2 ++;
		}

		lenMap[nUtf8Len2] += (char)charLen;

		checkout++;
		if (checkout >= nStrLen)
		{
			break;
		} 
	}
	
	//对utf-8序列进行处理
	unsigned long a , b, i, len, h ;//这些是计算hash函数的时候用到的
	
	checkout = 0;//防止死循环
	int headPos = 0, nHeadLen = 0;
	for( headPos = 0; headPos < nUtf8Len2; nHeadLen += lenArray2[headPos], headPos++)
	{
		checkout++;
		if (checkout > nStrLen)
		{
			break;
		} 
		if(isFilter2[headPos] == 0)  //如果这个词没有出现过，直接pass
		{
			continue;
		}
		
		char *sStartPtr = pInput2+nHeadLen;
		int nTailLen = 0; //尾部的长度
		int checkout2 = 0;//防止死循环
		for(int j = headPos; j < nUtf8Len2; j++ )
		{
			checkout2++;
			if (checkout2 > nStrLen)
			{
				break;
			} 
			nTailLen  += lenArray2[j]; //该字符的长度

			//如果这个词没有出现的话
			if(isFilter2[j] == 0)
			{
				for(int k = headPos ; k< j; k++ )
				{
					nHeadLen += lenArray2[k];
					headPos++;
				}
				break;
			}
			
			//这里减少函数调用，提高执行速度
			bool flag = true;
			len = (unsigned int)nTailLen;
			for( size_t n = 0; n < m_pBloom->nfuncs; ++n) 
			{
				char * str = sStartPtr;
				
				////////////////////////////////////////分割线////////////////////////////
				////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
				switch( n)
				{

				case 0:  //SaxHash
					h=0;
					for(i=0; i < len; i++, str++)
					{
						h^=(h<<5)+(h>>2)+(unsigned char)(*str); 
					}
					break;

				case 1:  //SdbmHash
					h=0;
					for(i=0; i < len; i++,str ++)
					{
						h=(unsigned char)(*str) + (h<<6) + (h<<16) - h;
					}
					break;

				case 2:  //RSHash
					b    = 378551;
					a    = 63689;
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h = h * a + (*str);
						a    = a * b;
					}
					break;

				case 3:  //JSHash
					h = 1315423911;
					for(i = 0; i < len; str++, i++)
					{
						h ^= ((h << 5) + (*str) + (h >> 2));
					}
					break;

				case 4:  //ELFHash 
					h= 0;
					a= 0;
					for(i=0; i < len; str++, i++)
					{
						h = (h << 4) + (*str);
						if((a = h & 0xF0000000L) != 0)
						{
							h ^= (a >> 24);
						}
						h &= ~a;
					}
					break;

				case 5:  //BKDRHash
					a = 131; /* 31 131 1313 13131 131313 etc.. */
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h = (h * a) + (*str);
					}
					break;

				case 6:  //SDBMHash
					h = 0;
					for(i=0;i < len; str++, i++)
					{
						h = (*str) + (h << 6) + (h << 16) - h;
					}
					break;

				case 7:  //DJBHash
					h = 5381;
					for(i=0; i < len; str++, i++)
					{
						h = ((h << 5) + h) + (*str);
					}
					break;

				case 8:  //DEKHash
					h = len;
					for(i=0; i < len; str++, i++)
					{
						h = ((h << 5) ^ (h >> 27)) ^ (*str);
					}
					break;

				case 9:  //BPHash
					h = 0;
					for(i = 0; i < len; str++, i++)
					{
						h = h << 7 ^ (*str);
					}
					break;

				case 10:  //FNVHash
					a = 0x811C9DC5;
					h = 0;
					for(i=0; i < len; str++, i++)
					{
						h *= a;
						h ^= (*str);
					}
					break;

				case 11:  //APHash
					h = 0xAAAAAAAA;
					for(i=0;i < len; str++, i++)
					{
						h ^= ((i & 1) == 0) ? (  (h <<  7) ^ (*str) * (h >> 3)) :
							(~((h << 11) + ((*str) ^ (h >> 5))));
					}
					break;

				default:
					h=0;
					break;
				}
				////////！！！！Hash函数执行部分，check和add要保持一致，为了提升执行效率这么弄的，改了一起拷贝 ///
				////////////////////////////////////////分割线////////////////////////////

				h %= m_pBloom->asize;

				if(!(GETBIT(m_pBloom->a, h)))
				{
					flag =false;
					break;
				}
			}
			
			if(flag)
			{
				*pMatchLen = lenMap[j+1] - lenMap[headPos];
				return pInput + lenMap[headPos];
			}
		}
	}

	return NULL;
}



//下面这些都是hash函数
/*
inline unsigned int BzHash(char *str, unsigned int len);
inline unsigned int SaxHash (char* str, unsigned int len);
inline unsigned int SdbmHash(char* str, unsigned int len);
inline unsigned int RSHash  (char* str, unsigned int len);
inline unsigned int JSHash  (char* str, unsigned int len);
inline unsigned int PJWHash (char* str, unsigned int len);
inline unsigned int ELFHash (char* str, unsigned int len);
inline unsigned int BKDRHash(char* str, unsigned int len);
inline unsigned int SDBMHash(char* str, unsigned int len);
inline unsigned int DJBHash (char* str, unsigned int len);
inline unsigned int DEKHash (char* str, unsigned int len);
inline unsigned int BPHash  (char* str, unsigned int len);
inline unsigned int FNVHash (char* str, unsigned int len);
inline unsigned int APHash  (char* str, unsigned int len);

unsigned int CFilter::BzHash(char *str, unsigned int len)   
{
	unsigned char *key = (unsigned char *)str;   
	unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;   
	int ch;  

	for(register unsigned int i=0; i < len; i++)
	{
		ch = *key++;
		ch = toupper(ch);   
		seed1 = m_cryptTable[(2 << 8) + ch] ^ (seed1 + seed2);   
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;    
	}   
	return seed1;    
}   



unsigned int CFilter::SaxHash( char *key,unsigned int len)
{ 

	unsigned int h=0;
	for(register unsigned int i=0; i < len; i++, key++)
	{
		h^=(h<<5)+(h>>2)+(unsigned char)(*key); 
	}
	return h;
}


unsigned int CFilter::SdbmHash( char *key,unsigned int len)
{
	unsigned int h=0;
	for(register unsigned int i=0; i < len; i++,key ++)
	{
		h=(unsigned char)(*key) + (h<<6) + (h<<16) - h;
	}
	return h;
}



unsigned int CFilter::RSHash(char* str, unsigned int len)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	

	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}



unsigned int CFilter::JSHash(char* str, unsigned int len)
{
	unsigned int hash = 1315423911;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash ^= ((hash << 5) + (*str) + (hash >> 2));
	}

	return hash;
}



unsigned int CFilter::PJWHash(char* str, unsigned int len)
{
	const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
	const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
	const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
	const unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
	unsigned int hash              = 0;
	unsigned int test              = 0;
	

	for(register unsigned int i=0;  i < len; str++, i++)
	{
		hash = (hash << OneEighth) + (*str);

		if((test = hash & HighBits)  != 0)
		{
			hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}

	return hash;
}



unsigned int CFilter::ELFHash(char* str, unsigned int len)
{
	unsigned int hash = 0;
	unsigned int x    = 0;
	
	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash = (hash << 4) + (*str);
		if((x = hash & 0xF0000000L) != 0)
		{
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return hash;
}



unsigned int CFilter::BKDRHash(char* str, unsigned int len)
{
	unsigned int hash = 0;
	
	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash = (hash * seed) + (*str);
	}

	return hash;
}



unsigned int CFilter::SDBMHash(char* str, unsigned int len)
{
	unsigned int hash = 0;

	for(register unsigned int i=0;i < len; str++, i++)
	{
		hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}



unsigned int CFilter::DJBHash(char* str, unsigned int len)
{
	unsigned int hash = 5381;
	

	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash = ((hash << 5) + hash) + (*str);
	}

	return hash;
}



unsigned int CFilter::DEKHash(char* str, unsigned int len)
{
	unsigned int hash = len;

	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
	}
	return hash;
}



unsigned int CFilter::BPHash(char* str, unsigned int len)
{
	unsigned int hash = 0;
	unsigned int i    = 0;
	for(i = 0; i < len; str++, i++)
	{
		hash = hash << 7 ^ (*str);
	}

	return hash;
}



unsigned int CFilter::FNVHash(char* str, unsigned int len)
{
	const unsigned int fnv_prime = 0x811C9DC5;
	unsigned int hash      = 0;
	

	for(register unsigned int i=0; i < len; str++, i++)
	{
		hash *= fnv_prime;
		hash ^= (*str);
	}

	return hash;
}



unsigned int CFilter::APHash(char* str, unsigned int len)
{
	unsigned int hash = 0xAAAAAAAA;
	

	for(register unsigned int i=0;i < len; str++, i++)
	{
		hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) * (hash >> 3)) :
			(~((hash << 11) + ((*str) ^ (hash >> 5))));
	}

	return hash;
}

*/
