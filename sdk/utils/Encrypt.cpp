/*/////////////////////////////////////////////////////////////////////////

文件名: Encrypt.cpp

功能描述: 数据加解密的处理

创建人: xiaoql

创建时间: 2010年06月04日

/////////////////////////////////////////////////////////////////////////*/

//#include <Windows.h>

//#include "CRC.h"
#include "Encrypt.h"
#include <time.h>
#include "wrand.h"


void Encrypt::SetSelfSalt(unsigned int salt)
{ 
	if (salt)
	{
		m_nSelfSalt = salt;
	}
}
unsigned int   Encrypt::GenSalt()
{
	unsigned int salt =0;
	//srand((int) time( NULL ) );
	do
	{
		salt =  (unsigned int) wrandvalue();
	}while (! salt);
	return salt;
}
void Encrypt::SetTargetSalt(unsigned int salt,bool isClient)
{
	m_nTargetSalt =salt;
	if (m_nSelfSalt ==0)
	{
		m_nSelfSalt =GenSalt();
	}
	GenKey(isClient);
} 

//先随便使用一个加密算法进行数据加密
void Encrypt::GenKey(bool isClient)
{
	if (m_nSelfSalt &&  m_nTargetSalt)
	{
		if(isClient)
		{
			m_nKey =( m_nSelfSalt ^  m_nTargetSalt) + m_nSelfSalt + 8654 ;
		}
		else
		{
			m_nKey =( m_nSelfSalt ^  m_nTargetSalt) + m_nTargetSalt + 8654 ;
		}
		
		*((PINT)m_sKeybuff) = m_nKey;
	}
}

bool Encrypt::Encode(char *pInBuff,size_t len,char *pOutBuff)
{ 
	if (m_nSelfSalt && m_nTargetSalt )
	{
		for (size_t i=0;i< len; i++)
		{
			pOutBuff[i] =  pInBuff[i] ^ m_sKeybuff[i&3];
		}
		return true;
	}
	else
	{
		return false;	
	}
}

bool Encrypt::Decode(char *pInBuff,size_t len,char * pOutBuff)
{
	//当前的加密算法和解密算法是一样的,反向操作
	return  Encode(pInBuff,len,pOutBuff);
}
