#pragma once

using namespace FileHeaders;

class CCustomConfigCacher
{
public:
	/****
	*
	* 配置数据缓存文件头定义
	* ★配置数据缓存文件的设计目的在于优化程序启动速度，减少配置文件读取时间。
	* ★缓存文件不能对32位环与64位环境进行自动兼容！32位环境下生成的缓存数据不
	*  能在64位环境下读取，反之依然。
	* ★缓存文件读取程序能够自动完成对数据的读取，但由于前后缓存内容中的指针地址
	*  可能发生变化，子类必须正确的完成对数据中指针的偏移修改！
	*
	*+-----+--------+----------+-----------+--------------+
	*|文件头|对象数据段|内存块描述段|内存块1数据段|内存块N数据段... |
	*+-----+--------+----------+-----------+--------------+
	*******************************************************************/
	typedef struct tagConfigDataCacheFileHeader
	{
		FILEIDENT	Ident;			//文件标志
		FILEVERSION	Version;		//文件版本号
		DWORD		dwMemBlockCount;//内存块的数量
		DWORD		dwElementCount;	//数据记录数量
		DWORD		dwSizeElement;	//数据记录对象的大小
		DWORD		dwDataCRC32;	//整个文件数据段的CRC32效验值
		DWORD       dwSourceCRC32;	//生成缓存是的原始数据CRC32效验值
		BYTE		btPlatFormIdent;//用于保存文件生成时的处理器架构标志（32位、64位）
		BYTE		btReseve0[3];	//保留字节
		BYTE		btReseve[32];	//保留字节，促使结构体大小为64字节
	}CONF_DATA_CACHE_HDR, *PCONF_DATA_CACHE_HDR;

	/***
	* 缓存文件内存块描述结构
	****************************************/
	typedef struct tagMemoryBlockRecord
	{
		char*	lpMemoryBase;		//内存起始地址
		size_t	dwMemorySize;		//内存块字节大小
	}MEM_BLOCK_REC, *PMEM_BLOCK_REC;

private:
	//缓存文件读取环境结构
	struct CacheReadEnvir
	{
		PCONF_DATA_CACHE_HDR	pHdr;			//文件头
		void*					pElements;		//对象指针记录数组
		PMEM_BLOCK_REC			pMemBlocks;		//内存块描述段
		char*					pDataSegment;	//内存数据段
		char*					pNewMemory;		//读取缓存的整合新内存段指针
	};

public:
	CCustomConfigCacher();
	virtual ~CCustomConfigCacher();

public:
	/*
	* Comments: 从文件加载并读取配置缓存数据
	* Param LPCTSTR sCacheFile: 文件路径，可以为相对路径或绝对路径
	* Param DWORD dwSourceCRC: 当前配置源文件的CRC32效验值
	* Param CObjectAllocator<char> & allocator: 用于提供配置一级对象的内存申请器
	* Param OUT void * *  pElements: 用于存储缓存中配置数据的对象列表的指针变量
	* Param INT_PTR & nElementCount: 用于存储缓存中配置数据的对象列表中对象数量的变量
	* @Return bool: 如果缓存文件有效且源文件的CRC经对比后没有变化则读取缓存文件并将结果输
	*               出到pElementList以及nElementCount中，建立对象的内存会通过allocator
	*               申请。如果源文件已经变化则不会读取缓存数据并返回false。
	*/
	bool LoadFromCache(LPCTSTR sCacheFile, DWORD dwSourceCRC, 
		CObjectAllocator<char> &allocator, OUT void** pElements, INT_PTR &nElementCount);
	/*
	* Comments: 将配置缓存数据保存到文件中
	* Param LPCTSTR sCacheFile: 文件路径，可以为相对路径或绝对路径
	* Param DWORD dwSourceCRC: 当前配置源文件的CRC32效验值
	* Param const CObjectAllocator<char> & allocator: 配置数据的内存申请器对象
	* Param const void *  pElements: 配置数据的对象列表指针
	* Param INT_PTR nElementCount: 配置数据的对象列表中对象数量
	* @Return bool: 如果保存成功则函数返回true否则返回false。
	*/
	bool SaveToCache(LPCTSTR sCacheFile, DWORD dwSourceCRC, 
		const CObjectAllocator<char> &allocator, const void *pElements, INT_PTR nElementCount);

protected:
	/**** ★★★★★★子类必须覆盖处理的函数集★★★★★★ ****/
	/*
	* Comments: 验证缓存文件头，判断缓存是否有效（判断文件头标志、判断版本号、判断dwSizeElement值）
	* Param const CONF_DATA_CACHE_HDR & hdr: 文件头对象
	* @Return bool: 如果验证成功则返回true，否则返回false。如果返回false会导致放弃对缓存文件的读取。
	*/
	virtual bool ValidateHeader(const CONF_DATA_CACHE_HDR& hdr) = 0;
	/*
	* Comments: 调整单个一级数据对象的成员指针
	* Param LPVOID lpElement: 数据对象
	* @Return bool: 如果所有成员指针调整完成，则返回true，否则返回false。返回false会导致终止对缓存文件的读取。
	* ★remarks: 之所以要调整对象的成员指针，是因为配置数据中可能包含指针成员且在保存的时候保存的是之前的指针地址，
	*           当缓存文件重新读取并分配内存后，在新的内存中，这些指针地址可能会发生变化，因此必要对这些已经变化
	*           的指针进行调整，以便保证读取后获得正确的结果。
	*
	* ★计算一个指针成员的新的指针地址，请调用GetNewMemoryPtr函数传递旧的指针地址进行计算★
	*/
	virtual bool AdjustElementPointers(LPVOID lpElement) = 0;
	/*
	* Comments: 填充缓存文件头数据，只需填充（文件头标志、版本号、以及dwSizeElement）其他的会自动填充和处理
	* Param CONF_DATA_CACHE_HDR & hdr:
	* @Return void:
	*/
	virtual void FillHeaderData(CONF_DATA_CACHE_HDR& hdr) = 0;

protected:
	/**** ★★★★★★以下为内部处理函数★★★★★★ ****/
	/*
	* Comments: 读取并处理缓存文件为可用数据
	* Param CObjectAllocator<char> & allocator:
	* Param OUT void * *  pElements:
	* Param INT_PTR & nElementCount:
	* @Return bool:
	*/
	bool ReadCacheFile(CObjectAllocator<char> &allocator, OUT void** pElements, INT_PTR &nElementCount);
	/*
	* Comments: 验证数据文件的CRC有效性以及与新的源文件的CRC做对比
	* Param DWORD dwSourceCRC:
	* @Return bool: 如果缓存数据验证成功且CRC一致，则函数返回true，否则返回false。
	*/
	bool ValidateCacheDataCRC(DWORD dwSourceCRC);
	/*
	* Comments: 基于缓存中一个旧的指针，计算对应的新内存地址中的新指针地址
	* Param LPCVOID lpAddress:
	* @Return void*:
	*/
	void* GetNewMemoryPtr(LPCVOID lpAddress);
	/*
	* Comments: 计算缓存中所有内存块的总字节大小
	* @Return size_t:
	*/
	size_t CalcDataSize();
	/*
	* Comments: 将缓存数据中的内存块数据拷贝到新的内存空间中
	* Param char * lpNewMemory:
	* @Return void:
	*/
	void CopyBlockMemorys(char* lpNewMemory);

protected:
	wylib::stream::CMemoryStream m_CacheStream;		//缓存数据读写流
	CacheReadEnvir*				 m_pCacheReadEnvir;	//缓存文件读取环境，仅在读取缓存数据函数调用期间按有意义
};
