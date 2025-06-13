
#pragma once

#include <string.h>
#include "_osdef.h"
#include "CRC16.h"

class  Encrypt
{
public:
	Encrypt()
	{
		Reset();
	}

	//重置
	inline void Reset()
	{
		memset(this,0,sizeof(*this));
	}
    void  SetSelfSalt(unsigned int salt); //设置自己的随机数

	/*
	* Comments:  设置目标的key
	* Parameter: unsigned int salt:目标的salt
	* Parameter: bool isClient:自身是作为客户端还是服务器，使用了不对称的key生成算法
	* @Return  void:
	*/
	void SetTargetSalt(unsigned int salt,bool isClient=false);  //设置通信方的随机数,同时会生成key

	unsigned int GetSelfSalt(){return m_nSelfSalt;}     //获取自身的随机数

	unsigned int GetTargetSalt() {return m_nTargetSalt; } //获取目标的随机数
	
	bool Encode( char *pInBuff,size_t len,char *pOutBuff ); //加密数据

	bool Decode( char *pInBuff,size_t len,char *pOutBuff ); //解密数据

	inline bool IsKeyCrcCorrect( WORD nKeyCRC) // 
	{
		return  (GetKeyCRC()== nKeyCRC);
	}

	inline static UINT_PTR CRC16(LPCVOID lpBuffer, size_t len)  //计算CRC-16的Checksum
	{
		return CRC16Calc((unsigned char *)lpBuffer,len);
	}
	inline unsigned int GetKey()
	{
		return m_nKey;
	}
	WORD GetKeyCRC() //获取key的crc值
	{
		return (CRC16(m_sKeybuff,4) & 0xffff);
	}
	unsigned int   GenSalt(); // 产生salt

	//增长checksum
	inline void IncreaseCheckSum()
	{
		m_wCheckSum += 1;
	}

	inline WORD GetCheckSum()
	{
		return m_wCheckSum;
	}

private:
	void  GenKey(bool isClient=false);      //产生密钥
	
	
private:
	unsigned int m_nSelfSalt;    //自己产生的随机数
	unsigned int m_nTargetSalt;  //通信方的随机数
	unsigned int m_nKey;         //密钥
	unsigned char m_sKeybuff[4] ;  //密钥的buff，添加这个为了方便计算
	WORD m_wCheckSum;       //一个数，自增长的,用于防止重复发包
};
