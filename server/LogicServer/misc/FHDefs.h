﻿#pragma once

/************************************************************************/
/*                                                                      
/*                   各种二进制文件头定义文件                               
/*                                                                      
/************************************************************************/

namespace FileHeaders
{
	/***
	* 定义文件标志结构
	* 文件标识由4个字节组成，使用3个字符保存标志字符，最后一个字符始终为0
	* 这样就使得文件标识既可以作为32位整形使用，也可以作为一个包含3个字符
	* 以及一个0终止字符的字符串。
	**********************************************************/
	typedef union tagFileIdent
	{
		UINT	uIdent;
		CHAR    sIdent[sizeof(UINT)];
		struct 
		{
			BYTE a, b, c;
			BYTE zero;
		}p;
	}FILEIDENT, *PFILEIDENT;

	/***
	* 定义文件版本结构
	* 文件版本由4个字节组成，使用4个字节表示版本具体值
	* 第一个字节表示主版本号，例如Beta版本可以为0，第一个正式版为1，第二个正式版为2等；
	* 第二个字节表示版本年份，具体值应当为年减2000；
	* 第三个字节表示版本月份；
	* 第四个字节表示版本日；
	* 使用这样的版本标志可以直观的通过各个字节看出版本号的主版本以及发布的年、月、日。
	**********************************************************************/
	typedef union tagFileVersion
	{
		UINT	uVersion;
		struct 
		{
			BYTE v, y, m, d;
		}p;
	}FILEVERSION, *PFILEVERSION;
}
