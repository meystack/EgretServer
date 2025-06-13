

#ifndef	___COMMON_SERVER_DEFINE___
#define	___COMMON_SERVER_DEFINE___


#define DEFAULT_TAG_VALUE 0xCCEE		   //默认的包的开头的2个字节
#define DEFAULT_HEADER_CHECK_SUM 0x7C58 //默认的头部的CRC的值

#pragma pack(push, 1)
typedef struct
{
	unsigned short tag;
   	unsigned int len;
	/*
   	struct 
   	{
      		unsigned short dataSum;
      		unsigned short hdrSum;
   	} EncodePart;
	union
	{
		struct
		{
			int recvTime;	// 网关接收消耗时间
			int sendTime;	// 网关发送消耗时间
		}s;
		__int64 tp; // 时间点，用于网关接受到客户端数据后打上的时间戳。
	}u;
	*/
}DATAHEADER,*PDATAHEADER;
#pragma pack(pop)

//实体移动的方向
 typedef enum tagDirCode{
	DIR_UP = 0,		//上
	DIR_UP_RIGHT=1,	//右上
	DIR_RIGHT=2,		//右
	DIR_DOWN_RIGHT=3,	//右下
	DIR_DOWN=4,		//下
	DIR_DOWN_LEFT=5,	//左下
	DIR_LEFT=6,		//左
	DIR_UP_LEFT=7,	//左上	
	DIR_STOP		//不动
}DIRCODE ;
	
 //基本的x,y的信息
 typedef struct tagMapPoint
 {
	 union
	 {
		 struct 
		 {
			 unsigned short x;
			 unsigned short y;
		 };
		 int nValue;
	 };
 }MAPPOINT,*PMAPPOINT;

 
#endif
