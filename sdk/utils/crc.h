#ifndef _WYLIB_CRC_H_
#define _WYLIB_CRC_H_

/**
 * wylib 2006 - 2011 快速CRC运算函数库
 * 通过预先计算出CRC码表并使用汇编函数书写CRC计算函数，
 * 将码表通过内联到函数代码中，使用CPU的一级指令缓存快
 * 速访问码表来提高函数效率。
 */

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

	/**
	* Comments: CRC32值计算函数，对data的datalen字节计算crc值
	* Param DWORD crc: 原始crc值
	* Param const void * data: 数据段指针
	* Param UINT_PTR datalen: 要计算数据段中crc值的字节数量
	* @Return DWORD : 对数据进行crc32计算的结果
	*/
	unsigned int CRC32Update(unsigned int crc, const void* data, int datalen);

#ifdef __cplusplus
}
#endif

#endif
