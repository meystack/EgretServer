#pragma once

/************************************************************************

                        简易二进制K-V数据库系统

  FileDB是一个简易的面向二进制的KV数据存储系统。设计目的在于以最高效和简单的方式提供
一种可变长度二进制数据的存储方式。数据的存储介质是磁盘而不是内存，实时的将数据写入到磁
 盘可以在系统宕机时将损失降到最小，这满足最大化降低系统故障影响的设计需求。

   每个数据库由两个文件组成，一个是数据文件另一个是索引文件。数据文件中存储在该数据库中
 数据存储的所有数据。我们将数据文件划分为一系列的“块”（Chunk），块是存储数据的基本单元。
 如果一个FileDB数据库中存储了100份数据，那么这个数据库的数据文件中至少会有100个块，每
 个块负责存储一份数据，每添加一份新数据，就有一个新的块产生，块在产生的时候，其大小由需要
 存储的数据的大小决定，但每个块的大小都会进行“块单元大小对齐”。块单元大小对齐值是在创建数
 据库后指定的，其意义在于使每个块的长度对齐到一个值的整倍数，视不同的应用需求指定将合理的
 块单元大小将有利于优化已存储的数据内容增长和减少时的存储效率以及整个数据文件的空间利用率。
   每个块的起始24字节为一个DataHeader结构，结构中描述了块的数据ID以及块存储的数据长度，
 这被称之为块描述头。块描述头的设计目的在于当丢失索引文件时可以通过数据文件中的块描述头来
重建索引文件。
 
   在索引文件中，存储了数据文件中的所有块的描述信息。每个描述信息包括块存储的数据的ID、块
 在数据文件中的偏移位置，块存储的数据长度以及块的大小。这些信息用于在读写数据库的时候快速的
 定位要在数据文件中读和写的位置等一切针对块进行操作的功能。

   向FileDB中存储数据以及获取数据的一句是64位的整数ID。对于不同的数据内容，ID必须是唯一
 的。如果以1为ID向数据库存储了100字节的数据，然后再存储50字节的数据，那么最终通过ID值1获
 取的数据是最后存储的50字节的内容。值为0的ID是不允许使用的，他在FileDB的实现中有着特殊的
 意义。 

 ★★★该数据库只能在本机系统运行，目前没有提供基于网络的支持，但这依然是可以扩展实现的。
 ★★★该数据库没有提供权限管理功能，对存储数据的安全管理是使用本数据库系统的软件的责任。

************************************************************************/

#ifdef WIN32
class CCustomFileDB 
{
public:
	CCustomFileDB();
	~CCustomFileDB();

	//打开数据库
	bool open(LPCTSTR sDBName);
	//关闭索引文件
	void close();
	//创建数据库
	bool create(LPCTSTR sDBName);
	//向数据库更新数据，如果数据不存在则添加为新数据
	bool put(INT64 nDataId, LPCVOID lpData, INT64 dwSize);
	//从数据库中读取数据
	//如果dwBufferSize为0则进行指定ID数据的长度查询操作，此时函数返回此ID的数据长度。
	//如果dwBufferSize为正数值，则函数向lpBuffer中拷贝数据库中存储的数据，拷贝的长度
	//受dwBufferSize限制。如果dwBufferSize大于数据的实际长度，则仅向lpBuffer中拷贝
	//数据的实际长度字节数。函数返回值表示最终向lpBuffer拷贝了多少字节。
	INT64 get(INT64 nDataId, LPVOID lpBuffer, INT64 dwBufferSize) const;
	//从数据库移除数据
	//如果数据存在且删除成功，则返回true。
	//改操作可能引发索引列表的内存大量拷贝。
	bool remove(INT64 nDataId);
	//获取存储在数据库中的第nIndex个数据的ID
	inline INT64 getId(INT_PTR nIndex){ return m_DataList[nIndex].pIndex->chunk.nId; }
	//获取数据块单位长度
	inline DWORD getChunkSize() const{ return m_Header.dwChunkSize; }
	//设置数据块单位长度
	inline void setChunkSize(DWORD dwNewSize){ m_Header.dwChunkSize = dwNewSize; };
	//是否在向文件写入数据后立即提交数据，如果开启则每当发生一次完整的数据和索引文件的写操作时，
	//都会等待文件被写入到硬盘后才返回。这个可以有效的提高数据存储的安全性。但由于同步文件写操作
	//比较耗时，因此可能因为存储效率被降低而造成性能瓶颈。
	inline bool getFlushFileDataImmediately() const{ return m_boFlushFileDataImmediately != 0; }
	inline void setFlushFileDataImmediately(bool boImmediately){ m_boFlushFileDataImmediately = (boImmediately ? TRUE : FALSE); }
	//获取数据记录数量
	inline INT_PTR getDataCount() const{ return m_DataList.count(); }
	//获取空闲数据块数量
	inline INT_PTR getFreeChunkCount() const{ return m_FreeDataOffsetList.count(); }
	//获取空索引数量
	inline INT_PTR getNullIndexCount() const{ return m_NullIndexList.count(); }
	//计算总存储数据大小
	//通过计算总存储数据大小与数据文件大小，可以进行数据文件利用率的计算：利用率 = 总存储数据大小 / 数据文件大小
	INT64 getTotalDataSize() const;
	//计算总空闲数据块大小
	//通过计算总空闲数据块大小与数据文件大小，可以进行数据空闲率的计算：空闲率 = 总空闲数据块大小 / 数据文件大小
	INT64 getTotalFreeChunkSize() const;
	//获取数据文件大小
	INT64 getDataFileSize() const;
	//获取数据ID列表
	//函数从第nIndex个ID开始填充pIdList列表并且最多填充nMaxCount个。
	//函数返回值表示实际向pIdList中填充的ID数量，返回值可能比nMaxCount小。
	INT64 getIdList(INT64 nIndex, PINT64 pIdList, INT64 nMaxCount);
public:
	static const TCHAR DataFileExt[];//数据文件后缀
	static const TCHAR IndexFileExt[];//索引文件后缀

	/** 数据库文件头 **/
	struct FDBHeader
	{
		static const DWORD IDENT = MAKEFOURCC('F', 'D', 'I', 0);
		static const DWORD VERSION = 0x010B0A0D;
		//文件标识固定为：MAKEFOURCC('F', 'D', 'B', 0)
		DWORD	dwIdent;		
		//数据库文件格式版本号，目前为0x010B0A0D
		DWORD	dwVersion;	
		//数据库中存储的记录数量
		INT		nRecordCount;
		/*数据记录块单位大小
		  记录块大小用于预保留数据记录的空间，以便优化在记录内数据长度变大
		  时的存储效率。数据库存储记录数据时，会保证用于对一个记录的字节长
		  度是dwChunkSize的倍数。例如在创建数据库时指定记录块大小为1024，
		  那么向数据库存储一个长度为800字节的记录时，仍然会给此记录分配长度
		  为1024字节的空间，存储1025字节的记录时，则会分配2048字节的空间。
		  这将有利于在记录的数据长度会不断变化的场合，预先为下次变化保留存储
		  空间，而合理的提供数据记录块大小值，则能充分的体现这一优化效果。
		  dwChunkSize在数据库创建的时候既被指定，并且此后不得再改变。
		*/
		DWORD	dwChunkSize;
		//保留字节
		char	btReseves[48];
	};
#pragma pack(push, 1)
	//数据记录头。每个数据块均以一个数据记录头开始。 
	struct DataHeader
	{
		static const INT64 VALID_IDENT   = 0xFFAADD8800DDAAFF;
		static const INT64 INVALID_IDENT = 0xCCAADD8800DDAACC;
		INT64	dwIdent;	//数据记录头标志，固定为0xFFAADD8800DDAAFF
		INT64	nId;		//数据ID
		INT64	nDataSize;	//数据长度（不含本记录头）
	public:
		DataHeader(){ dwIdent = VALID_IDENT; }
	};
#pragma pack(pop)
	// 数据块索引 
	struct ChunkIndex
	{
#pragma pack(push, 1)
		struct ChunkDesc
		{
			INT64	nId;		//数据记录唯一ID值。如果值为零则表示该记录为一个空闲的数据块，可以被回收利用。
			INT64	nDataOffset;//记录在数据库文件中的绝对偏移值
			INT64	nDataSize;	//数据记录字节数（包含数据块开头的DataHeader的大小）
			INT64	nChunkSize;	//数据记录块大小。如果值为0则表示该记录为没有任何意义，该索引在文件中的位置空间可以被回收利用。
		}chunk;
#pragma pack(pop)
		INT64	nIndexOffset;	//本索引记录在索引文件中的偏移位置
	};
	//有效数据记录项 
	struct AvaliableDataIndex
	{
		ChunkIndex *pIndex;
	public:
		inline operator ChunkIndex* (){ return pIndex; }
		inline INT_PTR compare (const AvaliableDataIndex & another) const 
		{
			if (pIndex == another.pIndex) return 0;
			if (this->pIndex->chunk.nId < another.pIndex->chunk.nId) return -1; 
			else if (this->pIndex->chunk.nId > another.pIndex->chunk.nId) return 1; 
			//如果出现相同ID索引的情况，那么就是发生错误了！
			else { Assert(pIndex == another.pIndex); return 0; }; 
		}
		inline INT_PTR compareKey(const INT64 nID) const 
		{ 
			if (this->pIndex->chunk.nId < nID) return -1; 
			else if (this->pIndex->chunk.nId > nID) return 1; 
			else return 0; 
		}
	};
	//空闲记录块大小排序项 
	struct FreeDataSizeIndex
	{
		ChunkIndex *pIndex;
	public:
		inline operator ChunkIndex* (){ return pIndex; }
		inline bool operator == (const FreeDataSizeIndex & another) const { return pIndex == another.pIndex; }
		inline INT_PTR compare (const FreeDataSizeIndex & another) const 
		{ 
			//if (pRecord == another.pRecord) return 0;
			if (this->pIndex->chunk.nChunkSize < another.pIndex->chunk.nChunkSize) return -1; 
			else if (this->pIndex->chunk.nChunkSize > another.pIndex->chunk.nChunkSize) return 1; 
			else return 0; 
		}
		inline INT_PTR compareKey(const INT64 nSize) const 
		{
			if (this->pIndex->chunk.nChunkSize < nSize) return -1; 
			else if (this->pIndex->chunk.nChunkSize > nSize) return 1; 
			else return 0; 
		}
	};
	// 空闲记录块偏移排序项 
	struct FreeDataOffsetIndex
	{
		ChunkIndex *pIndex;
	public:
		inline operator ChunkIndex* (){ return pIndex; }
		inline bool operator == (const FreeDataOffsetIndex & another) const { return pIndex == another.pIndex; }
		inline INT_PTR compare (const FreeDataOffsetIndex & another) const 
		{
			if (this->pIndex->chunk.nDataOffset < another.pIndex->chunk.nDataOffset) return -1; 
			else if (this->pIndex->chunk.nDataOffset > another.pIndex->chunk.nDataOffset) return 1; 
			//对于存储数据块索引的列表，如果出现相同偏移位置的索引的情况，那么就是发生错误了！
			else { Assert(pIndex == another.pIndex); return 0; }; 
		}
		inline INT_PTR compareKey(const INT64 nOffset) const 
		{ 
			if (this->pIndex->chunk.nDataOffset < nOffset) return -1; 
			else if (this->pIndex->chunk.nDataOffset > nOffset) return 1; 
			else return 0;
		}
	};

	typedef CObjectAllocator<ChunkIndex>					IndexAllocator;
	typedef wylib::container::CBaseList<ChunkIndex*>		IndexRecordList;
	typedef CCustomSortList<AvaliableDataIndex, INT64>		DataList;
	typedef CCustomSortList<FreeDataSizeIndex, INT64>		FreeDataSizeList;
	typedef CCustomSortList<FreeDataOffsetIndex, INT64>		FreeDataOffsetList;
	typedef wylib::container::CBaseList<ChunkIndex*>		NullIndexList;

	typedef wylib::sync::lock::CCSLock						FileLock;

protected:
	HANDLE				m_hIndexFile;	//索引文件句柄
	HANDLE				m_hDataFile;	//数据文件句柄
	FileLock			m_IndexFileLock;//索引文件锁
	FileLock			m_DataFileLock;	//数据文件锁
	FDBHeader			m_Header;		//文件头
	IndexAllocator		m_IndexAllocator;	//索引记录申请器
	IndexRecordList		m_IndexRecordList;	//索引记录列表
	DataList			m_DataList;			//有效数据索引列表
	FreeDataSizeList	m_FreeDataSizeList;		//空闲数据大小排序表，用于快速找到一个合适的空数据位置
	FreeDataOffsetList	m_FreeDataOffsetList;	//空闲数据偏移排序表，用于合并连续的空数据位置
	NullIndexList		m_NullIndexList;	//无效索引记录列表
	INT64				m_nNextIndexOffset;	//下一个索引记录的偏移位置
	BOOL				m_boFlushFileDataImmediately;//是否在向文件写入数据后立即提交数据

protected:
	//申请一个索引记录。返回的索引中会正确填充所以记录的所有成员。
	//当dwDataSize为0时表示仅申请一个索引对象，不预留空间。

	ChunkIndex* allocChunk(INT64 nId, INT64 dwDataSize);
	//将索引记录写入到索引文件中
	bool flushIndex(ChunkIndex *pIndex);
	//将索引记录添加为空数据索引
	void addFreeChunk(ChunkIndex *pIndex);
	//将一个大的数据块拆分成两个小块，参数dwSplitOutSize为从大数据块中拆分出来的新数据块的大小
	bool splitLargeChunk(ChunkIndex *pIndex, INT64 dwSplitOutSize);
	//重新申请一个更大的数据库块
	ChunkIndex* reallocChunk(ChunkIndex *pIndex, INT64 dwNewDataSize);
	//向数据文件写数据块
	bool writeData(ChunkIndex *pIndex, const DataHeader *pDataHeader, LPCVOID lpDataBuffer);
	//向数据文件中写数据（任意数据）
	bool writeDataFile(INT64 nOffset, LPCVOID lpBuffer, INT64 dwSize);
	//从数据文件中读数据
	bool readDataFile(INT64 nOffset, LPVOID lpBuffer, INT64 dwSize) const;
	//向索引文件中写数据（任意数据）
	bool writeIndexFile(INT64 nOffset, LPCVOID lpBuffer, INT64 dwSize);

protected:
	//验证各个列表的数据数量
	void validateListCount() const;
	//计算一个nChunkSize大小的数据块能够分割出的新数据块的大小，如果返回0则表示不能分割
	//参数dwDataSize为在nChunkSize大小的数据块中存储的数据大小（包含DataHeader的大小，非块大小）
	//将大的数据块拆分为小数据块的条件为：
	//	1、数据库头中规定的数据块大小不低于128字节；
	//	2、数据块写入新数据后剩余空间大于数据库头中的数据块单位大小；
	//	3、新数据长度是数据块长度的一半以内；

	INT64 getChunkSplitOutSize(INT64 nChunkSize, INT64 dwDataSize) const;

	//获取索引记录在空闲大小列表中的索引
	INT_PTR getFreeSizeListIndex(ChunkIndex *pIndex) const;
};

#endif

