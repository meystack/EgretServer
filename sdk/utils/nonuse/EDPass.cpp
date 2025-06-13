#include "aes.h"
#include <stdlib.h>
#include <string.h>
#include "base64.h"

static int boInited = 0;
static const int Bits128 = 16;

const char* EncryptPassword(char* sDest, int nDestSize, const char* sPassWord, const char* sKey128Bit)
{
	struct aes_ctx ctx;
	int nInSize;
	char* sBuffer;
	char* sBase64Buffer;
	int nSize;

	if ( !boInited )
	{
		boInited = 1;
		gen_tabs();
	}
	aes_set_key( &ctx, (const u8 *)sKey128Bit, Bits128 );

	nInSize = (int)strlen(sPassWord) * sizeof(sPassWord[0]);
	sBuffer = (char*)malloc(nInSize * 8 + 4);
	sBase64Buffer = sBuffer + nInSize * 2;

	nSize = (int)_AESEncrypt( &ctx, sBuffer, sPassWord, nInSize );      
	nSize = base64_encode(sBase64Buffer, sBuffer, nSize);
	sBase64Buffer[nSize] = 0;

	strncpy(sDest, sBase64Buffer, nDestSize-1);
	sDest[nDestSize-1] = 0;

	free(sBuffer);
	return sDest;
}


const char* DecryptPassword(char* sPassword, int nPasswordLen, const char* sEncrypted, const char* sKey128Bit)
{
	struct aes_ctx ctx;
	int nInSize;
	char* sAESBuffer;
	char* sBuffer;
	int nSize;

	if ( !boInited )
	{
		boInited = 1;
		gen_tabs();
	}
	aes_set_key( &ctx, (const u8 *)sKey128Bit, Bits128 );
		

	nInSize = (int)strlen(sEncrypted) * sizeof(sEncrypted[0]);
	sAESBuffer = (char*)malloc(nInSize * 8 + 4);
	sBuffer = sAESBuffer + nInSize * 2;

	nSize = base64_decode(sAESBuffer, sEncrypted, nInSize);
	_AESDecrypt( &ctx, sBuffer, sAESBuffer, nSize );      
	sBuffer[nSize] = 0;

	strncpy(sPassword, sBuffer, nPasswordLen-1);
	sPassword[nPasswordLen-1] = 0;

	free(sAESBuffer);
	return sPassword;
}


