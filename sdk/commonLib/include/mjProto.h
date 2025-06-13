#ifndef	_MJPROTO_H_
#define	_MJPROTO_H_

/************************************************************************/
/* 
/*                    《战将传奇》游戏通信协议头定义
/* 
/************************************************************************/

/******
* 游戏通信协议头
* 
****************************************************/
#ifndef DefaultMessageDef
#define DefaultMessageDef

#pragma pack(push, 1)
typedef struct tagDefaultMessage
{
	int		Recog;
	WORD	Ident;
	WORD	Param;
	WORD	Tag;
	WORD	Series;
}DEFAULTMESSAGE, *PDEFAULTMESSAGE;
#pragma pack(pop)

//构造游戏协议头的宏
#define	MAKEDEFMSG(m, i, r, p, t, s)	{(m)->Recog = r; (m)->Ident = (int)i; (m)->Param = (int)p; (m)->Tag = (int)t; (m)->Series = (int)s;}

//协议头编码大小
#define	DEFBLOCKSIZE	16

//定义协议头起始字节
#define MJP_PACK_START (char)'#'
//定义协议头结束字节
#define MJP_PACK_END   (char)'!'

//数据编码宏
#define	EncodeMessage(buf, msg)	EncodeBuffer(msg, buf, sizeof(DEFAULTMESSAGE), DEFBLOCKSIZE)
//数据解码宏
#define	DecodeMessage(msg, buf)	DecodeBuffer(buf, msg, DEFBLOCKSIZE, sizeof(DEFAULTMESSAGE))

#endif


#endif
