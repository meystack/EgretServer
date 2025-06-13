
/************************************************************
*          实现网络数据包到逻辑数据包变换
*    为了标示网络数据的长度和开始信息，网络包将添加头部标示tag和长度，以及checkSum等头部信息
*    该类讲从网络包里组装出逻辑数据，用于处理，如果是默认的话
***********************************************************/
#ifdef WIN32
#include <Windows.h>
#endif
#include "crc.h"
#include "Encrypt.h"
#include "NetworkDataHandler.h"


CNetworkDataHandler::CNetworkDataHandler(bool bNeedEncrypt)
{
	m_bNeedEncrypt = bNeedEncrypt;
	m_nTag = DEFAULT_TAG_VALUE;	
}

bool CNetworkDataHandler::DataUnpack(char *pInBuff,SIZE_T nInSize, char *pOutBuff, SIZE_T nOutSize,SIZE_T & nInProcessSize,SIZE_T & nOutProcessSize,bool bWithHeader,Encrypt * pEncrypt)
{
	/*
	nOutProcessSize =0;
	nInProcessSize =0;
	SIZE_T nHeaderSize = sizeof(DATAHEADER); 
	if (nInSize < nHeaderSize ) return false; //输入的数据不够
	if (NULL == pInBuff || NULL == pOutBuff) return false;
	if (m_bNeedEncrypt  && pEncrypt ==NULL ) return false;  //如果需要加密的话，又没有传入加密的指针

	bool tagFlag =false; //tag的寻找标志
	SIZE_T i =0;
	WORD * pTagPos =NULL;
	
	for ( ;i < nInSize -1; i++)
	{
		pTagPos = (WORD *)(pInBuff + i);
		if(*pTagPos == m_nTag) //如果找到了tag
		{
			tagFlag = true; //找到了头
			break;
		} 			 
	}
	//如果没有找到，那么
	if (tagFlag ==false )
	{
		i ++;
	}

	nInProcessSize = i; //前面的几个字节，如果不是头，将说明是垃圾数据，将被干掉

	SIZE_T nLeftCount = nInSize  - i; //还有多少个字节再后面

	//剩下的数据不够头，或者没有找到头
	if (tagFlag ==false || nLeftCount  < nHeaderSize ) 
	{
		if (nInProcessSize >0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	DATAHEADER data; //把数据拷贝一份过来
	memcpy( (void *)&data,(void*) (pInBuff + i),nHeaderSize ); //拷贝一份

	PDATAHEADER pHeader = 	&data; //把后面的转换为数据头的指针

	SIZE_T nTotalPackSize =  pHeader->len + nHeaderSize ;  //一个完整的数据包的需要的长度

	if ( nTotalPackSize  >  nLeftCount ) return false; //数据不齐全，没有len个字节发送过来
	
	bool bEncryptFlag = ! m_bNeedEncrypt ; //取反
	if(m_bNeedEncrypt)
	{
	
		pEncrypt->Decode((char *)(pInBuff + i + sizeof(data.tag) + sizeof(data.len)),sizeof(data.EncodePart) ,(char *)&(data.EncodePart)); //进行解密操作
		
		WORD hdrSum =  data.EncodePart.hdrSum; //把这个checkSum保持起来	
		if(pEncrypt)
		{
			data.EncodePart.hdrSum = pEncrypt->GetCheckSum();
			
		}
		else
		{
			data.EncodePart.hdrSum = DEFAULT_HEADER_CHECK_SUM ; //用默认值替换完以后再计算整个头部的checkSum
		}
		
		
		if ( (0xffff &pEncrypt->CRC16((unsigned char *)&data,nHeaderSize )) == hdrSum ) //头部校验成功
		{
			if ( (0xffff &pEncrypt->CRC16( (unsigned char *)(pInBuff + i +nHeaderSize ),pHeader->len ) ) == data.EncodePart.dataSum) 
			{
				bEncryptFlag  =true; //验证成功
			}
		}	
	}
	
	if (bEncryptFlag )	
	{	
		if ( bWithHeader  ) //需要把数据头带出去
		{
			if( nTotalPackSize   <=  nOutSize )
			{
				//拷贝数据
				memcpy(pOutBuff,(void *)(pInBuff + i +nHeaderSize ),nTotalPackSize)	;
				nInProcessSize = i + nTotalPackSize;
				nOutProcessSize = nTotalPackSize;
				
			}
		}
		else  //网络头去掉了,剩下的就是逻辑数据
		{
			if(  pHeader->len    <=  nOutSize )
			{
				memcpy(pOutBuff,(void *)(pInBuff + i + nHeaderSize  ), pHeader->len  )	;
				nInProcessSize = i + nTotalPackSize;
				nOutProcessSize =  pHeader->len  ;
			}
		}
		if(pEncrypt)
		{
			pEncrypt->IncreaseCheckSum();
		}
	}
	else
	{
		//OutputMsg(r)
		nInProcessSize = i + nTotalPackSize; //这段数据作废
		return true;
	}
	*/
	return true;
}

 void CNetworkDataHandler::FillInHeader(char *pInBuff,SIZE_T nInSize,PDATAHEADER pHeader,Encrypt * pEncrypt)
{
	/*
	SIZE_T nheaderSize = sizeof(DATAHEADER);
	pHeader->tag = DEFAULT_TAG_VALUE;
	pHeader->len = (WORD)nInSize;
	
	//计算数据的crc
	if(pEncrypt)
	{
		pHeader->EncodePart.hdrSum =  pEncrypt->GetCheckSum();
	}
	else
	{
		pHeader->EncodePart.hdrSum = DEFAULT_HEADER_CHECK_SUM;
	}
	
	WORD nDataCrc =   0xffff & pEncrypt->CRC16((unsigned char *)pInBuff,nInSize);
	pHeader->EncodePart.dataSum = nDataCrc;

	//计算头部的crc
	WORD nHeadCrc =  0xffff & pEncrypt->CRC16( (unsigned char *)pHeader,nheaderSize); 
	pHeader->EncodePart.hdrSum = nHeadCrc;

	//进行加密
	DATAHEADER encryptHead;
	pEncrypt->Encode((char *)&pHeader->EncodePart,sizeof(&pHeader->EncodePart),(char *)&encryptHead.EncodePart);
	pHeader->EncodePart = encryptHead.EncodePart;
	if(pEncrypt)
	{
		pEncrypt->IncreaseCheckSum();
	}
	*/

}

INT_PTR CNetworkDataHandler::DataPack( char *pInBuff,SIZE_T nInSize,  char *pOutBuff, SIZE_T nOutSize,Encrypt * pEncrypt)
{
	/*
	SIZE_T nheaderSize = sizeof(DATAHEADER);
	if ( nOutSize <  nheaderSize + nInSize ) return 0; //输出缓存长度不够
	if (nInSize >65535) return 0;  //最大只支持65535个字节的发送
	if (m_bNeedEncrypt && pEncrypt ==NULL) return 0; //没有加密器指针

	DATAHEADER head; //网络数据头
	head.tag = m_nTag;
	head.len = (WORD)nInSize;
	head.EncodePart.hdrSum =0;
	head.EncodePart.dataSum =0;
	if (m_bNeedEncrypt) //如果需要加密的话。那么进行填充
	{
		//计算数据的crc
		head.EncodePart.hdrSum = DEFAULT_HEADER_CHECK_SUM;
		WORD nDataCrc =   0xffff & pEncrypt->CRC16((unsigned char *)pInBuff,nInSize);
		head.EncodePart.dataSum = nDataCrc;
		
		//计算头部的crc
		WORD nHeadCrc =  0xffff & pEncrypt->CRC16( (unsigned char *)&head,nheaderSize); 
		head.EncodePart.hdrSum = nHeadCrc;

		//进行加密
		DATAHEADER encryptHead;
		pEncrypt->Encode((char *)&head.EncodePart,sizeof(head.EncodePart),(char *)&encryptHead.EncodePart);
		head.EncodePart = encryptHead.EncodePart;
		 
	}
	memcpy((void *)pOutBuff,(void*)&head,nheaderSize); //头放进去
	memcpy((void *)(pOutBuff+nheaderSize),(void*)pInBuff,nInSize); //数据放进去
	return nInSize + nheaderSize;
	*/
	return 0;

}
