
#include <string.h>
#include "EDCode.h"
#define	BUFFERSIZE		20480
char	EncBuf[BUFFERSIZE], TempBuf[BUFFERSIZE];
//
//void Encode6BitBuf(const char *src, char *dest, int srclen, int destlen)
//{
//	int	restcount, destpos;
//	unsigned char made, ch, rest;
//	restcount	= 0;
//	rest 		= 0;
//	destpos		= 0;
//	for (int I=0; I<srclen;++I)
//	{
//		if (destpos >= destlen)
//			break;
//		ch		= (unsigned char)src[I];
//		made	= (unsigned char)((rest | (ch >> (2+restcount))) & 0x3F);
//		rest	= (unsigned char)(((ch << (8 - (2+restcount))) >> 2) & 0x3F);
//		restcount	+= 2;
//		if (restcount < 6)
//		{
//      		dest[destpos] = made + 0x3C;
//			destpos++;
//		}
//		else
//		{
//      		if (destpos < destlen-1)
//			{
//				dest[destpos]   = made + 0x3C;
//				dest[destpos+1] = rest + 0x3C;
//				destpos	+=	2;
//			}
//			else
//			{
//				dest[destpos]   = made + 0x3C;
//				destpos++;
//			}
//			restcount	= 0;
//			rest		= 0;
//		}
//	}
//	if (restcount)
//	{
//   		dest[destpos] = rest + 0x3C;
//		destpos++;
//	}
//	dest[destpos]	= 0;
//}
//
//void Decode6BitBuf(const char	*source, char	*buf, int	buflen)
//{
//	const
//		unsigned char Masks[5] = {0xFC, 0xF8, 0xF0, 0xE0, 0xC0};
//	int	len, bitpos, madebit, bufpos;
//	int	ch, tmp, _unsigned char;
//	len		= (int)strlen(source);
//	bitpos	= 2;
//	madebit	= bufpos = tmp = ch	= 0;
//	for (int I=0;I<len;++I)
//	{
//   		if ((source[I] - 0x3C) >= 0)
//			ch	= (unsigned char)(source[I] - 0x3C);
//		else
//		{
//			bufpos	= 0;
//      		break;
//		}
//		if (bufpos >= buflen)
//			break;
//		if ((madebit+6) >= 8)
//		{
//			_unsigned char			= (unsigned char)(tmp | ((ch & 0x3F) >> (6-bitpos)));
//        	buf[bufpos]		= _unsigned char;
//			bufpos++;
//			madebit	= 0;
//			if (bitpos < 6)
//				bitpos	+=	2;
//			else
//			{
//         		bitpos = 2;
//				continue;
//			}
//		}
//		tmp		= (unsigned char)((unsigned char)(ch << bitpos) & Masks[bitpos-2]);   // #### ##--
//		madebit	+= 8-bitpos;
//	}
//	buf[bufpos]	= 0;
//}

const char EncryptMinChar = 0x3C;    //编码后的最小值
  //3编4方式的，如果值需要保持在0..7F范围内，则此值的范围是3C..40



unsigned int Encode6BitBuf(const char *pSrc, char *pDest, unsigned int dwSrcSize, unsigned int dwMaxSize)
{
  //前端掩码
	static const unsigned char btFrontMasks[4]	= { 0xFF, 0xF8, 0xE0, 0x80 };
	static const unsigned char btBehindMasks[4]	= { 0x00, 0x01, 0x07, 0x1F };
	static const unsigned char btBitMasks[4]		= { 0x00, 0x06, 0x18, 0x60 };

	char	btSrc, btNew, btIdx, btFront, btBehind;
	unsigned int	dwSizeLeft;

	dwSizeLeft = dwMaxSize;

	if ( dwSizeLeft )
	{
		btIdx  = 3;
		btNew  = 0;

		while (dwSrcSize)
		{
			btSrc = *pSrc;
			btNew	= btNew | (btSrc & btBitMasks[btIdx]);  //取第 btIdx 位
			btFront	= (btSrc & btFrontMasks[btIdx]) >> 2; //保存 btIdx 前的位
			btBehind= btSrc & btBehindMasks[btIdx];        //保存 btIdx 后的位

			*pDest = (btFront | btBehind ) + EncryptMinChar;
			dwSizeLeft--;
			if (!dwSizeLeft)
				break;
			pDest++;
			btIdx--;

			if ( !btIdx )
			{
				*pDest = (btNew >> 1) + EncryptMinChar;
				dwSizeLeft--;
				if (!dwSizeLeft)
					break;
				pDest++;
				btIdx = 3;
				btNew = 0;
			}
			            
			pSrc++;
			dwSrcSize--;
		}
		
		if ( !dwSrcSize && (btIdx != 3) )//remaind
		{
			*pDest = (btNew >> 1) + EncryptMinChar;;
			dwSizeLeft--;
			pDest++;
		}
		
		if ( dwSizeLeft )
			pDest[0] = 0;
	}


	return dwMaxSize - dwSizeLeft;
}

unsigned int Decode6BitBuf(const char *pSrc, char *pDest, unsigned int dwSrcSize, unsigned int dwMaxSize)
{
  //前端掩码                                           
	static const unsigned char btFrontMasks[4] = { 0xFF, 0xFE, 0xF8, 0xE0 };
	static const unsigned char btBehindMasks[4]= { 0x00, 0x01, 0x07, 0x1F };
	static const unsigned char btBitMasks[4]   = { 0x00, 0x06, 0x18, 0x60 };
	
	char	btSrc, btFront, btBehind, btBits, btIdx;
	unsigned int	dwSizeLeft;

	dwSizeLeft = dwMaxSize;
	
	if ( dwSizeLeft )
	{
		btIdx = 3;

		while ( dwSrcSize > 1 )
		{
			if ( btIdx == 3 )
			{
				btBits = (dwSrcSize > 3) ? pSrc[3] : pSrc[dwSrcSize-1];
				btBits = (btBits - EncryptMinChar) << 1;
			}

			btSrc	= *pSrc - EncryptMinChar;
			btFront = (btSrc & btFrontMasks[btIdx]) << 2;
			btBehind= btSrc & btBehindMasks[btIdx];
			*pDest  = btFront | btBehind | (btBits & btBitMasks[btIdx]);

			dwSizeLeft--;
			if ( !dwSizeLeft )
				break;
			pDest++;
			btIdx--;

			if ( !btIdx )
			{
				dwSrcSize--;
				if ( !dwSrcSize )
					break;
				pSrc++;
				btIdx = 3;
			}

			dwSrcSize--;
			pSrc++;
		}

		if ( dwSizeLeft )
			pDest[0] = 0;
	}

	return dwMaxSize - dwSizeLeft;
}

char	*		EnCodeBuffer(const void	*InBuf, size_t BufSize)
{
	if ( BufSize >= BUFFERSIZE )
		BufSize = BUFFERSIZE - 1;
	Encode6BitBuf((char*)InBuf, (char*)&EncBuf, (unsigned int)BufSize, BUFFERSIZE);
	return EncBuf;
}


void	DeCodeBuffer(const void	*InBuf, void *OutBuf, size_t SrcSize, size_t MaxSize)
{
	Decode6BitBuf ((char*)InBuf, (char*)EncBuf, (unsigned int)SrcSize, BUFFERSIZE);
	memcpy(OutBuf, &EncBuf[0], MaxSize);
}

unsigned int GetEncryptedSize(unsigned int dwSize)
{
	unsigned int	Result = dwSize + dwSize / 3;
	if ( dwSize % 3 )
		Result++;
	return Result;
}

unsigned int GetDecryptedSize(unsigned int dwSize)
{
	unsigned int	Result = dwSize / 4 * 3;
	if ( dwSize & 3 )
		Result += (dwSize & 3) - 1;
	return Result;
}

