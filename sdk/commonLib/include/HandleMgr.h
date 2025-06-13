/*
 Handle类通常是只读的无符号整数。虽然可以安全地将其设为空来重置句柄，但句柄一旦创建就不应该再修改。
 注意，Handle是参数化的类，需要一个TAG类型来完整定义。
 模板参数TAG除了区分句柄类型外不起任何作用,TAG类型的对象在系统任何地方都不会被用到。
 这么做的原因是为了保证类型安全。使用参数化的Handle类型可以确保，如果向期望获取某种类型句柄的
 函数传递另外一种类型的句柄，则不会被编译通过。
 因此为了保障类型安全，我们创建一个新的句柄类型，
 作为一个唯一符号并被用作Handle类型的参数。TAG类型可以是任何形式的类型.
*/
 
#ifndef _DEBUG
#include <new>
//#include <time.h>
#endif
//#include <vector>
//#include <cassert>
//实现对象的重用,而且管理实体
//参见: http://hi.baidu.com/zyb_debug/blog/item/eaf6ea8ce55f291bb21bba4c.html 

#pragma once

enum
{
	// sizes to use for bit fields 使用位域(bit fields)的大小
	MAX_BITS_INDEX = 21,
	MAX_BITS_MAGIC = 11,

	// sizes to compare against for asserting dereferences
	MAX_INDEX = ( 1 << MAX_BITS_INDEX) - 1,
	MAX_MAGIC = ( 1 << MAX_BITS_MAGIC) - 1,

	//INVALID_INDEX =( 1 << MAX_BITS_INDEX),   //非法的INDEX
};

//TAG在这里并没有什么用处,
template <typename TAG>
class Handle
{
	union
	{
		struct
		{
			unsigned m_Index : MAX_BITS_INDEX; // index into resource array 资源数组的索引
			unsigned m_Magic : MAX_BITS_MAGIC; // magic number to check     需要检查的魔术数
		};
		unsigned int m_Handle;
	};

public:
	
	// Lifetime.生命期

	Handle( void ) : m_Handle( 0 ) { }
	Handle( unsigned nHandle )  {m_Handle= nHandle ;} //

	void Init( unsigned int index )
	{
		
		DbgAssert( IsNull() );             // don't allow reassignment 不允许重新赋值
		DbgAssert( index <=MAX_INDEX );   // verify range 有限范围验证

		//魔数初始化修改为一个随机数，避免出现一些特殊情况，增加服务器重启后可靠性
		static unsigned int s_AutoMagic = rand();
		IncMagicNumImpl(s_AutoMagic);

		m_Index = index;
		m_Magic = s_AutoMagic;
	}

	inline void updateMagic() {
		unsigned int magic = m_Magic;
		IncMagicNumImpl(magic);
		m_Magic = magic;
	}

	// Query.查询

	inline unsigned int GetIndex ( void ) const { return ( m_Index ); }
	inline unsigned int GetMagic ( void ) const { return ( m_Magic ); }
	inline unsigned int GetHandle( void ) const { return ( m_Handle ); }
	
	bool         IsNull   ( void ) const { return ( !m_Handle ); }
	inline unsigned GetMaxIndex(){return MAX_INDEX;}
	operator unsigned int ( void ) const { return ( m_Handle ); }

protected:
	inline void IncMagicNumImpl(unsigned int &magic)
	{
		if ( ++magic > MAX_MAGIC )
			magic = 1;    // 0 is used for "null handle"
	}
};




template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT =1024>
class HandleMgr
{
public:
	typedef DATA	DataType;
	typedef HANDLE	HandleType;
	//typedef HandleMgrStat Inherited;

	 HandleMgr(LPCTSTR lpszDesc);

	~HandleMgr( ) 
	{
		Empty();
	}
	
	
	//申请一个数据指针
	DATA* Acquire( HANDLE& handle );
	
	//不使用指针,放到缓存池里去
	void Release( HANDLE handle );

	// 重新请求一个新句柄
	bool ReNew( HANDLE &handle);	

	// 通过handle返回数据的指针
	DATA*       GetDataPtr( HANDLE handle );
	const DATA* GetDataPtr( HANDLE handle ) const;

	// other query 其他查询

	inline unsigned int GetUsedHandleCount( void ) const
	{ 
		return ( m_nUsedCount ); 
	}
	inline UINT GetTotalHandleCount(void) 
	{
		return    (UINT)m_MagicNumbers.count() * ONE_CHUNK_COUNT;
	}

	inline int GetUserDataSize(void)
	{
		return m_nUsedCount * (sizeof(DATA) + ONE_CHUNK_COUNT);
	}

	inline bool HasUsedHandles( void ) const
	{ 
		return ( (GetTotalHandleCount() -GetUsedHandleCount()) >0 ); 
	}
	inline VOID GetChunkIdPos(UINT index, UINT &nChunkId, UINT  & nPos)
	{
		nPos = index % ONE_CHUNK_COUNT; //取在chunk里的位置; //取在chunk里的位置
		nChunkId = index / ONE_CHUNK_COUNT; //在第几个chunk
		
	}
	//通过数据指针释放
	void  ReleaseDataPtr( DATA * pData );
	
	//获取第1个有效的数据指针，结果的句柄要放在 hHandle,数据指针放在DATA*
	DATA * First(HANDLE & hHandle); 

	DATA * Next(HANDLE & hHandle); //hHandle 句柄 下一个有效数据的指针，DATA 为该数据指针
	
	//清空内存
	void Empty(); 
	
	// 输出统计信息到文件
	void dumpToFile()
	{

	}

private:
	// private types
	//
	typedef wylib::container::CBaseList<  wylib::container::CBaseList<DATA> >      UserVec; //数据
	typedef wylib::container::CBaseList<  wylib::container::CBaseList<HANDLE>  > MagicVec; //handle
	typedef wylib::container::CBaseList< unsigned int> ChunkFreeCountVec; //每个块还有多少个空闲的句柄

	UINT m_nFirstFreeIndex ; //第1个能用的index,这个用于指向释放了的第1个可用index
	UINT m_nLastChunkFirstFreeIndex;  //新申请的空闲列表的一个第1个可用的index
	UINT m_nUsedCount;         //
	
	// private data
	UserVec m_UserData;				// 数据
	MagicVec  m_MagicNumbers;		 // 存的是每个数据当前使用的句柄
	ChunkFreeCountVec m_FreeCounts; //每个块有多少个空闲的元素,这个保证每次查找可用的指针最多查找1024个对象
	const static UINT INVALID_INDEX = ~0 ; //非法的指针
	//const static  int  ONE_CHUNK_COUNT =1024 ;			//每次申请1024个空间,这个要是2的n次幂
	//const static  int CHUNK_SHIFT_BIT_COUNT =10;          //移位ONE_CHUNK_COUNT 需要移位运送多少位
	//const static  int  CHUNK_MASK=0x3ff  ;                  //取在1024块里的位置
};




template <typename TAG>
inline bool operator != ( Handle <TAG> l, Handle <TAG> r )
{ 
	return ( l.GetHandle() != r.GetHandle() ); 
}

template <typename TAG>
inline bool operator == ( Handle <TAG> l, Handle <TAG> r )
{ 
	return ( l.GetHandle() == r.GetHandle() ); 
}

template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> :: HandleMgr(LPCTSTR lpszDesc = NULL)
{ 
	m_nFirstFreeIndex = INVALID_INDEX;
	m_nLastChunkFirstFreeIndex   = INVALID_INDEX;
	m_nUsedCount =0;
}
template <typename DATA, typename HANDLE, int ONE_CHUNK_COUNT>
void HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> ::Empty()
{
	for(INT_PTR i=0; i<m_UserData.count(); i++)
	{
		for(INT_PTR j=0; j< ONE_CHUNK_COUNT ; j ++)
		{
			HANDLE hd = m_MagicNumbers[i][j];
			if(! hd.IsNull() )
			{
				DATA *pdata =&( m_UserData[i][j]);
				if(pdata)
				{
					pdata->~DATA();
				}
				TRACE(_T("Node not releaseed on HanderMgr destroy,DataSize =%d, hander=%d,index=%d\n"),sizeof(DATA),hd.GetHandle(),hd.GetIndex());
				//OutputMsg(rmError,_T("Node not releaseed on HanderMgr destroy,DataSize =%d, hander=%d,index=%d"),sizeof(DATA),hd.GetHandle(),hd.GetIndex());
			}
		}
		m_UserData[i].empty();	
	}

	for(INT_PTR i=0; i<m_UserData.count(); i++)
	{
		m_MagicNumbers[i].empty();
	}
	m_UserData.empty();
	m_MagicNumbers.empty();
}


template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
DATA* HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> :: Acquire( HANDLE& handle )
{
	// if free list is empty, add a new one otherwise use first one found 如果空闲列表为空，则新增一个，否则使用第一个可用表项
	UINT index =INVALID_INDEX;
	UINT nChunkId =0,nPos=0;

	//如果前面有空闲的位置,优先使用空闲的对象
	if (m_nFirstFreeIndex != INVALID_INDEX) 
	{
		index = m_nFirstFreeIndex;
		GetChunkIdPos(m_nFirstFreeIndex,nChunkId,nPos);	// 获取当前空闲句柄的位置（在第几块的第几个槽）
		m_nFirstFreeIndex = INVALID_INDEX;				// 设置当前空闲有效句柄为INVALID_INDEX

		//查找一个空闲的
		for(INT i =nChunkId ; i < (INT)m_MagicNumbers.count(); i++)	// m_MagicNumbers.count() 是数据块的数目
		{
			//只有在这个块有空闲的才去查找,避免了无效的查询,最多1024个
			bool bFindNextFreeItem = false;
			INT nFreeCount =  m_FreeCounts[i];
			if( nFreeCount > 0 )
			{
				INT j =0;
				if( nChunkId == i ) //如果是本块,那么从后一个查起
				{
					j = nPos +1;
					if(nFreeCount ==1) //本块只有1块,那肯定用过了阿
					{
						continue;
					}
				}
				CBaseList<HANDLE> & pHandChunk = m_MagicNumbers[i];	// 第i块的句柄数据
				for (; j < ONE_CHUNK_COUNT; j++)
				{
					//HANDLE hTemp = ;
					if( pHandChunk[j].GetMagic() ==0)
					{
						m_nFirstFreeIndex = (i * ONE_CHUNK_COUNT) + j;// 下一个空闲句柄位置
						bFindNextFreeItem = true;
						break;
					}
				}
			}

			if (bFindNextFreeItem)
				break;
		}
		//如果其已经到了空闲列表的最后一个了,那么说明前面已经没有可用使用的了
		if (m_nFirstFreeIndex == m_nLastChunkFirstFreeIndex)
		{
			m_nFirstFreeIndex = INVALID_INDEX;
		}
	}
	else
	{
		if(m_nLastChunkFirstFreeIndex  == INVALID_INDEX ) //if no chunk
		{
			//已经无法申请了
			UINT nTotalCount =  GetTotalHandleCount();
			Assert(nTotalCount == GetUsedHandleCount());
			if ( (unsigned int) nTotalCount >= handle.GetMaxIndex()) return NULL; //已经无法申请了,超过了最大数量
	
			CBaseList<DATA> dataChunk;
			CBaseList<HANDLE> handleChunk;
			m_UserData.addArray(&dataChunk,1); //添加到列表
			m_MagicNumbers.addArray(&handleChunk,1);
			INT_PTR nBackIndex = m_UserData.count();	// m_UserData.count() 是当前数据块的块数目
			if (nBackIndex <1) return NULL;
			nBackIndex --;	// 最后一个块的索引			
			m_UserData[nBackIndex].reserve(ONE_CHUNK_COUNT);	// 保证每个块都是同样长度
			m_UserData[nBackIndex].trunc(ONE_CHUNK_COUNT);
		
			m_MagicNumbers[nBackIndex].reserve(ONE_CHUNK_COUNT);
			m_MagicNumbers[nBackIndex].trunc(ONE_CHUNK_COUNT);
			HANDLE *pHandle = m_MagicNumbers[nBackIndex];		// 最后一个句柄块
			
			memset(pHandle,0,sizeof(HANDLE) *ONE_CHUNK_COUNT );	// 初始化最后一个句柄块
			//使用placement new 调用构造函数
		
			m_nLastChunkFirstFreeIndex = nTotalCount; //当前的最大数量,指向下一个
		
			m_FreeCounts.add(ONE_CHUNK_COUNT); //这个块还有多少个没有使用
			//m_nAllocedChunkCount++;
		}
		if(m_nLastChunkFirstFreeIndex != INVALID_INDEX)
		{
			index = m_nLastChunkFirstFreeIndex;
			m_nLastChunkFirstFreeIndex ++; //指向下一个

			if(m_nLastChunkFirstFreeIndex >= (unsigned) GetTotalHandleCount()) //如果这个块使用完了,就需要重新申请空间了
			{
				m_nLastChunkFirstFreeIndex = INVALID_INDEX;
			}
		}
	}
	
	
	//实在是找不到可以使用的了
	if ( index == INVALID_INDEX)	return NULL;
	handle.Init( index );
	GetChunkIdPos(index,nChunkId,nPos);
	unsigned int nFreeCount =  m_FreeCounts[nChunkId];
	if (nChunkId < (UINT)m_MagicNumbers.count() && nFreeCount >0 )
	{
		m_MagicNumbers[nChunkId][nPos] =handle;
	}
	else
	{		
		return NULL;
	}

	m_FreeCounts[nChunkId]--; //这个列表空闲的数量 	

	DATA *pData = &(m_UserData[nChunkId][nPos]);
	new(pData)DATA();//placement new
	m_nUsedCount++;
	//m_nUsedHdlCount++;
	//if (m_nUsedHdlCount > m_nMaxUsedHandleCount)
	//	m_nMaxUsedHandleCount = m_nUsedHdlCount;
	return pData;
}

template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
void HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> :: ReleaseDataPtr( DATA * pData )
{
	UINT_PTR count =m_UserData.count();
	SIZE_T nChunkSize = ONE_CHUNK_COUNT * sizeof(DATA);  //每一块Chunk的大小,每次申请1024个
	UINT_PTR nEndIndex = ONE_CHUNK_COUNT -1; //1023
	for (UINT_PTR i= 0;i< count; i++)
	{
		DATA * pStart = &m_UserData[i][0];
		DATA * pEnd =  &m_UserData[i][nEndIndex];
		
		INT_PTR nDataIndex = pData -pStart;  //第多少个数据
		if(nDataIndex >= 0 && pData <=  pEnd) //找到了这个内存块
		{
			INT_PTR nByteDis = (char *)pData -  (char *)pStart;
			if( nByteDis  % sizeof(DATA) != 0) //内存长度错误
			{
				return; 
			}
			Release( m_MagicNumbers[i][nDataIndex] );
			return;
		}
	}
}


template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
void HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> :: Release( HANDLE handle )
{
	// which one? 哪一个
	if ( handle.IsNull() )return;
	unsigned int index = handle.GetIndex();
	UINT nChunkId =0,nPos=0;
	GetChunkIdPos(index,nChunkId,nPos);

	if( nChunkId  >= (UINT)m_MagicNumbers.count() ) return ; //有问题,已经超过了
	if (m_MagicNumbers[nChunkId][nPos].GetMagic() != handle.GetMagic()) return; //重复释放

	// ok remove it - tag as unused and add to free list 可以删除了--表及其没有使用并加到空闲列表
	
	if (index < m_nFirstFreeIndex )
	{
		m_nFirstFreeIndex = index;
	}
	HANDLE hTemp;
	m_MagicNumbers[nChunkId][nPos] =hTemp;
	

	//调用析构函数
	DATA *pdata =&( m_UserData[nChunkId][nPos]);
	if(pdata)
	{
		pdata->~DATA();
	}

	m_FreeCounts[nChunkId] ++; //这个块的空闲的个数 -1
	m_nUsedCount--;	
	//m_nUsedHdlCount--;
}

template<typename DATA, typename HANDLE, int ONE_CHUNK_COUNT>
bool HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT> :: ReNew( HANDLE &handle)
{
	if (handle.IsNull())
		return false;

	// check handle validity
	unsigned int index = handle.GetIndex();
	UINT nChunkId =0,nPos=0;
	GetChunkIdPos(index,nChunkId,nPos);
	if (nChunkId > (UINT)m_MagicNumbers.count() || m_MagicNumbers[nChunkId][nPos].GetMagic() != handle.GetMagic())
		return false;

	handle.updateMagic();
	m_MagicNumbers[nChunkId][nPos] = handle;
	return true;
}


template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
inline DATA* HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT>
	:: GetDataPtr( HANDLE handle )
{
	if ( handle.IsNull() ) return ( 0 );
	unsigned int index = handle.GetIndex();
	UINT nChunkId =0,nPos=0;
	GetChunkIdPos(index,nChunkId,nPos);

	// check handle validity - $ this check can be removed for speed 检查句柄有效性-为提供性能可以去掉这个检查
	// if you can assume all handle references are always valid. 如果你假设索引句柄解引用都总是有效

	if( nChunkId  >=(UINT)m_MagicNumbers.count()) return NULL; //有问题,
	if (m_MagicNumbers[nChunkId][nPos].GetMagic() != handle.GetMagic()) 
		return NULL; //重复释放

	return (& m_UserData[nChunkId][nPos] );
}

template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
inline const DATA* HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT>
	:: GetDataPtr( HANDLE handle ) const
{
	// this lazy cast is ok - non-const version does not modify anything
	typedef HandleMgr <DATA, HANDLE> ThisType;
	return ( const_cast <ThisType*> ( this )->GetDataPtr( handle ) );
}


//获取第1个有效的数据指针，结果的句柄要放在 hHandle,数据指针放在DATA*
template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
 DATA* HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT>
 ::First(HANDLE & hHandle)
{
	
	INT_PTR count=	m_FreeCounts.count();
	for(int i=0; i< count; i++)
	{
		if(ONE_CHUNK_COUNT- m_FreeCounts[i] >0 ) //如果一个块里有数据，那么就在这个块里找
		{
			
			for(INT_PTR j=0; j< ONE_CHUNK_COUNT; j++)
			{
				if(m_MagicNumbers[i][j] != 0)
				{
					hHandle = m_MagicNumbers[i][j];
					return &m_UserData[i][j];
				}
			}
		}
	}
	return NULL;
}

//hHandle 句柄 下一个有效数据的指针，DATA 为该数据指针
 template <typename DATA, typename HANDLE,int ONE_CHUNK_COUNT>
 DATA* HandleMgr <DATA, HANDLE,ONE_CHUNK_COUNT>
:: Next(HANDLE & hHandle)
{
	if(hHandle ==0) return NULL; //压根就没有输入
	UINT index = hHandle.GetIndex();
	UINT nChunkId =0,nPos=0;
	GetChunkIdPos(index,nChunkId,nPos);

	for(INT_PTR i =nChunkId ; i < m_MagicNumbers.count(); i++)
	{
		//只有在这个块有空闲的才去查找,避免了无效的查询,最多1024个
		INT_PTR nFreeCount =  m_FreeCounts[i];
		if( ONE_CHUNK_COUNT- nFreeCount >0   )
		{
			INT_PTR j =0;
			if( nChunkId == i ) //如果是本块,那么从后一个查起
			{
				j = nPos +1;
				if(nFreeCount ==1) //本块只有1块,那肯定用过了阿
				{
					continue;
				}
			}
			CBaseList<HANDLE> & pHandChunk = m_MagicNumbers[i];
			for (; j < ONE_CHUNK_COUNT; j++)
			{
				//HANDLE hTemp = ;
				if( pHandChunk[j].GetMagic() !=0)
				{
					hHandle = pHandChunk[j];
					return  &m_UserData[i][j];
				}
			}
		}
	}
	return NULL;
}
