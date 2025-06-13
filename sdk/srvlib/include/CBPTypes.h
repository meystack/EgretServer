#pragma once

/************************************************************

   CBP系统（Common Binary Property）通用二进制属性描述系统
					数据结构定义

	 通用二进制属性描述系统用于在多种开发语言中以及服务器、客户端
 之间提供基于一份数据而产生的二进制的通用配置描述数据。避免为不
 同系统以及功能之间的数据配置文件独立设计不同的数据提供文件的工
 作。将所有的配置数据都发布为通用格式的属性描述文件，将使得所有
 系统以及功能之间的数据配置提供统一、规范化。

************************************************************/


/*
* 通用二进制属性系统——数据类型
*
*/
typedef enum tagCBPDataType
{
	//null值。无数据（0字节）
	v_null,
	//bool值。1字节，0为false，其他值为true
	v_boolean,
	//number值。64位浮点数
	v_number,
	//multi-byte string值
	v_string,
	//object值（属性对象，一个对象中包含多个属性。也可以是数组）
	v_object,
}CBPDATATYPE;

/*
 * 通用二进制属性系统——数据描述值
 **/
#pragma pack(push, 1)
typedef union tagCBPDataDesc
{
	//boolean值
	bool b;
	//number值
	double	n;
	//string以及object值
	struct
	{
		//字符串的长度或对象的属性数量
		size_t	len;	
	}s, t;
	//对于字符串、对象类型的值，值的具体内容在此结构的后面。
}CBPDATADESC, *PCBPDATADESC;
#pragma pack(pop)

/*
 * 通用二进制属性系统——数据(包含数据类型和数据值描述）
 **/
#pragma pack(push, 1)
typedef struct tagCBPData
{
	BYTE			type;	//数据值的数据类型,为tagLuaDataType枚举值
	CBPDATADESC		desc;	//数据描述值
}CBPDATA, *PCBPDATA;
#pragma pack(pop)

/*
 * L通用二进制属性系统——属性
 **/
#pragma pack(push, 1)
typedef struct tagComBinProperty
{
	BYTE			namelen;	//属性名称长度
	CBPDATA			data;		//属性数据值
}COMBINPROPERTY, *PCOMBINPROPERTY;
#pragma pack(pop)


/*
 * L通用二进制属性系统——属性文件头
 **/
typedef struct tagCBPFileHeader
{
	DWORD	dwIdent;			//文件标识，固定为0x00504243
	DWORD	dwVersion;			//文件版本，当前版本为0x010A0C0A
	DWORD	dwDataSourceSize;	//未压缩的原始数据字节大小
	DWORD	dwDataCompressed;	//压缩后的数据段字节大小
	char    btReseve[48];		//保留字节
}CBPFILE_HEADER, *PCBPFILE_HEADER;
