#pragma once
#include "../script/interface/BuffType.h"
/************************************************************************/
/*
/*                          游戏属性数据结构定义文件
/*
/************************************************************************/

/**
/* 定义游戏中的属性值数据结构
/* 此数据被定义为一个4字节的联合类型，可分别通过不同的成员范围不同的字节值
*********************************/
typedef union tagGameAttributeValue
{
	int				nValue;
	unsigned int	uValue;
	float			fValue;
	bool            bValue;
	struct 
	{
		short wLo;
		short wHi;
	};
	struct 
	{
		unsigned short uwLo;
		unsigned short uwHi;
	};
	struct  
	{
		char cLoLo;
		char cLoHi;
		char cHiLo;
		char cHiHi;
	};
	struct  
	{
		char btLoLo;
		char btLoHi;
		char btHiLo;
		char btHiHi;
	};

	struct 
	{
		unsigned short     wValueLo;
		struct
		{
			unsigned char btValueHiLo;
			unsigned char btValueHiHi;
		};
	};
}GAMEATTRVALUE, *PGAMEATTRVALUE;

/**
/* 定义游戏中的属性数据结构
*********************************/
typedef struct tagGameAttribute
{
	unsigned char	type;		//属性类型，值为GAMEATTRTYPE的枚举成员，使用BYTE成员用于节省内存空间
	unsigned char   reserve[3];	//保留3个字节，以便使数据结构按8字节对齐并且充分利用内存空间
	GAMEATTRVALUE	value;		//属性值
}GAMEATTR, *PGAMEATTR;

/**
/* 定义游戏中的紧凑属性数据结构
/* ★用于减少数据存储的内存开销★
*********************************/
#pragma pack(push,1)
typedef struct tagPackedGameAttribute
{
	//定义浮点值的基数，值为0.0001（千分数）
	static const double FloatValueBase;
	union 
	{
		//type8位的类型值
		//sign8为表示符号，如果是0表示value是正数，1表示value是负数
		//value16位的数据值，如果数据类型是浮点数，则使用万分数表示，1表示0.0001
		struct 
		{
			unsigned char  type;	
			unsigned char sign;   
			unsigned short value; 
		};
		int nValue;  //整数的值
	};
}PACKEDGAMEATTR, *PPACKEDGAMEATTR;
#pragma pack(pop)


/**
/* 定义与属性类型匹配的数据类型
*********************************/
enum eAttribueTypeDataType
{
	adVoid = 0,	//VOID类型值
	adSmall,	//有符号1字节类型
	adUSmall,		//无符号1字节类型
	adShort,	//有符号2字节类型
	adUShort,		//无符号2字节类型
	adInt,		//有符号4字节类型
	adUInt,		//无符号4字节类型
	adFloat,	//单精度浮点类型值
};

typedef struct 
{
	int			nCount;
	PGAMEATTR	pAttrs;	//属性表
}AttriGroup;

/**
/* 定义属性数据类型表
*********************************/
typedef eAttribueTypeDataType	TAttrDataTypes[GameAttributeCount];
typedef bool					TAttrNeedCalc[GameAttributeCount];
/**
/* 导出数据类型表声明
*********************************/
extern TAttrDataTypes	AttrDataTypes;	//属性的数据类型表
extern TAttrNeedCalc	AttrNeedCalc;	//属性是否需要通过属性计算器计算属性的标志表
