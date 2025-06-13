﻿#ifndef _FDOP_H_
#define _FDOP_H_

/************************************************************************

                    常用文件与目录操作函数库

************************************************************************/

#include "_osdef.h"
#include "Define.h"

namespace FDOP
{
	/* 判断文件或目录是否存在
	* 在操作系统中，目录也是一个文件，如果要判断一个目录是否存在则应当使用DirectoryExists，
	* 要判断一个文件是否存在且是一个归档文件则应当使用IsArchive。
	* @如果文件或目录存在则返回true否则返回false
	* %文件路径字符长度不得超过MAX_PATH
	*/
	bool FileExists(LPCTSTR sFilePath);
	
	/* 判断文件是否存在切是一个可直接读取的归档文件
	 * %文件路径字符长度不得超过MAX_PATH
	 */
	bool IsArchive(LPCTSTR sFilePath);
	
	/* 判断目录是否存在，文件存在且文件是一个目录则返回true否则返回false
	 * %文件路径字符长度不得超过MAX_PATH
	 */
	bool IsDirectory(LPCTSTR sDirPath);
	
	//Is COmmon File,Not Direcroty
	bool IsCommonFile(LPCTSTR sDirPath);

	/* 获取文件或目录名称
	 * c:\abc\123.txt --> 123.txt
	 * c:\abc\efg\ --> efg
	 * 参数sDirBuf用于存储文件名称字符串
	 * 参数dwBufLen为sNameBuf参数的缓冲区字符（非字节）长度，其中含需要保留的终止字符，
		    如果dwBufLen值为0则函数不会将文件名拷贝到sNameBuf中；
		    如果dwBufLen值非0则函数会将文件名拷贝到sNameBuf中并会在sNameBuf中写入终止符;
		    如果缓冲区不够则只拷贝dwBufLen-1个字符并会在sNameBuf中写入终止符。
		@函数返回拷贝文件名所需的字符长度（含终止符）
	*/
	SIZE_T ExtractFileName(LPCTSTR sFilePath, LPTSTR sNameBuf, SIZE_T dwBufLen);
	
	/* 获取文件路径中的文件名部分，不包含文件后缀部分
	 * c:\abc.txt --> abc
	 * 参数sNameBuf用于存储文件名称字符串
	 * 参数dwBufLen为sNameBuf参数的缓冲区字符（非字节）长度，其中含需要保留的终止字符，
		    如果dwBufLen值为0则函数不会将文件名拷贝到sNameBuf中；
		    如果dwBufLen值非0则函数会将文件名拷贝到sNameBuf中并会在sNameBuf中写入终止符;
		    如果缓冲区不够则只拷贝dwBufLen-1个字符并会在sNameBuf中写入终止符。
		@函数返回拷贝文件名所需的字符长度（含终止符）
	*/
	SIZE_T ExtractFileNameOnly(LPCTSTR sFileName, LPTSTR sNameBuf, SIZE_T dwBufLen);
	
	/* 获取文件名或文件路径中的文件后缀部分
	 * abc.txt --> .txt
	 * 返回值包含后缀符号'.'
	 */
	LPCTSTR ExtractFileExt(LPCTSTR sFileName);
	
	/* 获取文件所在目录路径
     * c:\abc\efg\123.txt --> c:\abc\efg\
	 * c:\abc\efg\ --> c:\abc\
	 * 参数sDirBuf用于存储目录字符串
	 * 参数dwBufLen为sDirName参数的缓冲区字符（非字节）长度，其中含需要保留的终止字符，
	       如果dwBufLen值为0则函数不会将目录路径拷贝到sDirBuf中；
		   如果dwBufLen值非0则函数会将目录路径拷贝到sDirBuf中并会在sDirBuf中写入终止符;
		   如果缓冲区不够则只拷贝dwBufLen-1个字符并会在sDirBuf中写入终止符。
		@函数返回拷贝目录路径所需的字符长度（含终止符）
	 */
	SIZE_T ExtractFileDirectory(LPCTSTR sFilePath, LPTSTR sDirBuf, SIZE_T dwBufLen);

	/* 获取顶层目录名称
	 * (abc\efg\ --> abc)
	 * 参数ppChildDirPath用于存顶层目录之后的目录路径，参数可以为空
	 * 参数sDirName用于存储目录字符串
	 * 参数dwBufLen为sDirName参数的缓冲区字符（非字节）长度，其中含需要保留的终止字符，
	       如果dwBufLen值为0则函数不会将目录名拷贝到sDirName中；
		   如果dwBufLen值非0则函数会将目录名拷贝到sDirName中并会在sDirName中写入终止符;
		   如果缓冲区不够则只拷贝dwBufLen-1个字符并会在sDirName中写入终止符。
		@函数返回拷贝目录名所需的字符长度（含终止符）
	 */
	SIZE_T ExtractTopDirectoryName(LPCTSTR sDirPath, OUT LPCTSTR *ppChildDirPath, LPTSTR sDirName, SIZE_T dwBufLen);
	/* 逐层创建目录
	 * 如果创建目录C:\a\b\c\d，最终目录的父目录不存在则逐级创建父目录并创建最终目录
	 * @如果目录完全创建成功则函数返回true，否则返回false。
	 * %如果在创建某个父目录成功后并创建子目录失败，则函数返回false且已经创建的父目录不会被删除。
	 * %目录路径的总体字符长度，函数要求必须在MAX_PATH个字符长度以内
	 */
	bool DeepCreateDirectory(LPCTSTR sDirPath);

}

#endif
