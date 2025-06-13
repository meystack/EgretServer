#ifndef	EDCODE_H
#define	EDCODE_H

//#include <windows.h>

extern "C"
{

/*
#ifndef STDCALL
#define STDCALL __stdcall
#endif
*/

unsigned int  Encode6BitBuf(const char *pSrc, char *pDest, unsigned int dwSrcSize, unsigned int dwMaxSize);
unsigned int  Decode6BitBuf(const char *pSrc, char *pDest, unsigned int dwSrcSize, unsigned int dwMaxSize);

unsigned int  GetEncryptedSize(unsigned int dwSize);
unsigned int  GetDecryptedSize(unsigned int dwSize);

#define	EncodeBuffer( i, o, si, so )	(Encode6BitBuf( (char*)(i), (char*)(o), (DWORD)(si), (DWORD)(so) ))
#define	DecodeBuffer( i, o, si, so )	(Decode6BitBuf( (char*)(i), (char*)(o), (DWORD)(si), (DWORD)(so)))


}

#endif
