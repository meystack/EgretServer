#ifndef _MBASE_LIST_H_
#define _MBASE_LIST_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 数组列表类 $
 *  
 *  - 主要功能 - 
 *
 *	可按索引访问并可动态插入和删除的数组类
 *
 *****************************************************************/
#include "_osdef.h"
//#include <string.h>
#include <stdlib.h>

namespace wylib
{
	namespace container
	{
		template <typename T>
		class CBaseList
		{
		public:
			typedef CBaseList<T> ListClass;

			protected:
			T*		m_pData;
			INT_PTR m_tMaxCount;
			INT_PTR m_tCount;

		public:
			CBaseList()
			{
				m_pData = NULL;
				m_tMaxCount = 0;
				m_tCount = 0;
			}
		
			virtual ~CBaseList()
			{
				empty();
			}
			inline INT_PTR count() const { return m_tCount; }
			inline INT_PTR maxCount() const { return m_tMaxCount; }
			void insert(const INT_PTR index, const T& data)
			{
				#ifdef WIN32
				Assert( index > -1 && index <= m_tCount );
				#endif
				if ( m_tCount >= m_tMaxCount )
					reserve( (m_tMaxCount > 0) ? m_tMaxCount * 2 : 16 );

				if ( index < m_tCount )
				{
					memmove( &m_pData[index + 1], &m_pData[index], sizeof(T) * (m_tCount - index) );
				}

				m_pData[index] = data;
				m_tCount++;
			}
			INT_PTR add(const T& data)
			{
				if ( m_tCount >= m_tMaxCount )
					reserve( (m_tMaxCount > 0) ? m_tMaxCount * 2 : 16 );

				memcpy(&m_pData[m_tCount], &data, sizeof(data));
				m_tCount++;
                return m_tCount-1;
			}
			inline CONST T& get(const INT_PTR index) const
			{
				#ifdef WIN32
				Assert( index > -1 && index < m_tCount );
				#endif
				return m_pData[index];
			}
			
			inline void set(const INT_PTR index, const T &item)
			{
				#ifdef WIN32
				Assert( index > -1 && index < m_tCount );
				#endif
				m_pData[index] = item;
			}
			INT_PTR index(const T& data) const
			{
				INT_PTR i;

				for ( i=m_tCount-1; i>-1; --i )
				{
					if ( m_pData[i] == data )
					{
						return i;
					}
				}

				return -1;
			}
			void remove(const INT_PTR index)
			{
				#ifdef WIN32
				Assert( index > -1 && index < m_tCount );
				#endif
				remove( index, 1 );
			}
			void remove(const INT_PTR index, const INT_PTR count)
			{
				#ifdef WIN32
				Assert( index + count <= m_tCount );
				#endif
				if ( count > 0 )
				{
					//memcpy( &m_pData[index], &m_pData[index + count], sizeof(m_pData[0]) * (m_tCount - index - count) );
					memmove( &m_pData[index], &m_pData[index + count], sizeof(m_pData[0]) * (m_tCount - index - count) );
					// printf("%d, %d, %d\n",index,m_tCount,m_tCount - index - count);
					m_tCount -= count;
				}
			}
			virtual void empty()
			{
				clear();
				m_tMaxCount = 0;
				if (m_pData)
				{
					free( m_pData );
					m_pData = NULL;
				}
			}
			inline void clear()
			{
				m_tCount = 0;
			}
			inline void trunc(const INT_PTR count)
			{
				#ifdef WIN32
				Assert( count > -1 && count <= m_tMaxCount );
				#endif
				m_tCount = count;
			}
			virtual void reserve(INT_PTR count)
			{
				if ( count > m_tCount && count != m_tMaxCount )
				{
					m_tMaxCount = count;
					m_pData = (T*)realloc( m_pData, sizeof(T) * count );
				}
			}
			inline void addList(const CBaseList<T> &list)
			{
				addArray((T*)list, list.m_tCount);
			}
			inline void addArray(T* data, INT_PTR length)
			{
				if ( m_tCount + length > m_tMaxCount )
					reserve(m_tCount + length);
				memcpy(&m_pData[m_tCount], data, length * sizeof(T));
				m_tCount += length;
			}
			inline INT_PTR push(const T& data)
			{
				return add(data);
			}
			inline T pop()
			{
				if ( m_tCount > 0 )
				{
					m_tCount--;
					return m_pData[m_tCount];
				}
				throw "stack was empty";
			}
			inline operator T* () const { return m_pData; }
			inline void swap(CBaseList<T> &targetList)
			{
				T*		pTmp;
				INT_PTR nTmp;

				pTmp = this->m_pData;
				this->m_pData = targetList.m_pData;
				targetList.m_pData = pTmp;

				nTmp = this->m_tMaxCount;
				this->m_tMaxCount = targetList.m_tMaxCount;
				targetList.m_tMaxCount = nTmp;

				nTmp = this->m_tCount;
				this->m_tCount = targetList.m_tCount;
				targetList.m_tCount = nTmp;
			}
		};
	};
};


#endif

