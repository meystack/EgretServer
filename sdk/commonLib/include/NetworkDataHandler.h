

#ifndef	_DATA_NETWORD_DATA_HANDLE_H_
#define	_DATA_NETWORD_DATA_HANDLE_H_

#include "CommonDef.h"
#include "Define.h"

//网络数据处理器
class CNetworkDataHandler
{
public:
	/*
	* 构造函数,初始化一些参数
	* 一个数据包的头由tab,len,headCheckSum,dataCheckSum,data组成的
	* param bNeedEncrypt: 是否需要加密处理，如果不需要的话，那么将不进行加密和checksum的校验
	*/
	CNetworkDataHandler(bool bNeedEncrypt=false);

	/*
	* 解包数据
	* param pInBuff 输入的数据包
	* param nInSize 输入的数据包的长度
	* param pOutBuff 输出的数据包指针
	* param nOutSize 输出的数据包的长度
	* param nInProcessSize  输入的数据包处理了的长度，如果大于0，那么前面的这个字节的数据包可以丢弃了
	* param nOutProcessSize 处理好的数据长度，处理好的数据放在pOutBuff里
	* param bWithHeader 是否还需要网络数据头，如果不需要，那么将把8个字节的网络字节头丢弃，pOutBuff中的数据将是无头的逻辑数据
	* param pEncrypt 加密器的指针
	* ret  如果进行了数据处理返回true，否则返回false。数据处理包括发现前面有无效数字，拷贝了一个完整的数据包，丢弃一些字节等
	*/
	bool DataUnpack(char *pInBuff,SIZE_T nInSize, char *pOutBuff, SIZE_T nOutSize,SIZE_T & nInProcessSize,SIZE_T & nOutProcessSize,bool bWithHeader=false,Encrypt * pEncrypt=NULL);
	
	/*
	* 逻辑数据包组装成网络数据包
	* param pInBuff 输入的数据包
	* param nInSize 输入的数据包的长度
	* param pOutBuff 输出的数据包指针
	* param nOutSize 输出的数据包的长度
	* param pEncrypt 加密器的指针
	* ret  返回组装包以后pOutBuff的数据长度，如果没有组装成功，返回0
	*/
	INT_PTR DataPack(char *pInBuff,SIZE_T nInSize, char *pOutBuff, SIZE_T nOutSize,Encrypt * pEncrypt=NULL);

	/*
	* Comments:填充加密头
	* Param char * pInBuff: 数据部分的指针
	* Param SIZE_T nInSize: 数据部分的长度
	* Param PDATAHEADER pHeader: 头的指针
	* Param Encrypt * pEncrypt: 加密器的指针
	* @Return void:
	*/
	static void FillInHeader(char *pInBuff,SIZE_T nInSize,PDATAHEADER pHeader,Encrypt * pEncrypt);

	//设置tag的值
	void SetTagValue(WORD nValue){m_nTag=nValue;}

	void SetNeedEncrypt(bool flag){m_bNeedEncrypt = flag;}
private:
	bool m_bNeedEncrypt; //是否需要加密处理，如果不需要的话，那么将不进行加密和checksum的校验
	WORD m_nTag;    //Tag的值
};
#endif