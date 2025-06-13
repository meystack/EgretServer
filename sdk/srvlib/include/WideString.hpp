#ifndef _WYLIB_WIDESTRING_H_
#define _WYLIB_WIDESTRING_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ UCS2字符串类 $
 *  
 *  - 主要功能 - 
 *
 *	实现UCS字符串的基本操作，包括与UCS2字符指针的相互操作
 *
 *****************************************************************/

#include <string.h>
#include <wchar.h>
#include <stdlib.h>

namespace wylib
{
	namespace string
	{
		class CAnsiString;

		class CWideString
		{
		public:
			typedef wchar_t			CharType;
			typedef wchar_t*		RawStrType;

		private:
			wchar_t*	m_sStr;

		private:
			static const wchar_t* EMPTY_STR;
		
		protected:
			/*
				realloc 内存申请、释放函数，如果需要关心内存的申请和释放，子类可以继承此函数
					ptr: 申请、释放的指针
					newsize: 申请内存字节数，若为0则表示释放ptr指针
					若ptr和newsize都不为0则表示对ptr内存块重新申请大小
			*/
			virtual void* alloc(void* ptr, size_t newsize)
			{
				if ( newsize == 0 )
				{
					if ( ptr ) free( ptr );
					return NULL;
				}
				return realloc( ptr, newsize );
			}
			/*
				setData	设定字符串数据
					data: 指向原始字符串的指针
			*/
			inline void setData(const wchar_t* data)
			{
				if ( data )
				{
					size_t size = wcslen(data) + 1;
					size *= sizeof(*data);
					m_sStr = (wchar_t*)alloc( m_sStr, size );
					memcpy( m_sStr, data, size );
				}
				else m_sStr = (wchar_t*)alloc( m_sStr, 0 );
			}
			/*
				catData 与现有字符串内容连接
					data: 指向原始字符串的指针，字符串内容会被连接到当前字符串之后
			*/
			inline void catData(const wchar_t* data)
			{
				if ( data )
				{
					size_t cat_size = wcslen(data);
					size_t cur_size = length();
					m_sStr = (wchar_t*)alloc( m_sStr, (cat_size + cur_size + 1) * sizeof(*data) );
					memcpy( &m_sStr[cur_size], data, (cat_size + 1) * sizeof(*data) );
				}
			}
		public:
			/*
				?构造和析构函数
			*/
			CWideString()
			{
				m_sStr = NULL;
			}
			CWideString(const wchar_t* data)
			{
				m_sStr = NULL;
				setData( data );
			}
			CWideString(const CWideString &str)
			{
				m_sStr = NULL;
				setData( str.m_sStr );
			}
			virtual ~CWideString()
			{
				setData( NULL );
			}

		public:
			/*  类型转换函数，转换为(const wchar_t*)。
				注意： 如果当前字符串为NULL，则会返回""而不是NULL。
			*/
			inline operator const wchar_t* () const
			{
				return m_sStr ? m_sStr : EMPTY_STR;
			}
			/*  类型转换函数，转换为(const wchar_t*)。
				注意： 如果当前字符串为NULL，则会返回""而不是NULL。
			*/
			inline operator wchar_t* ()
			{
				return m_sStr ? m_sStr : NULL;
			}
			/*	赋值函数，赋予(const wchar_t*)类型的值	*/
			inline void operator = (const wchar_t* data)
			{
				if ( data != m_sStr )
				{
					setData( data );
				}
			}
			/*	赋值函数，赋予相同类型的值	*/
			inline void operator = (const CWideString& str)
			{
				if ( &str != this )
				{
					setData( str.m_sStr );
				}
			}
			/*	与(const wchar_t*)类型的字符串连接  */
			inline void operator += (const wchar_t* data)
			{
				if ( data != m_sStr )
				{
					catData( data );
				}
				else
				{
					CWideString wstr(m_sStr);
					catData(wstr.m_sStr);
				}
			}
			/*	与自己或其他字符串连接	*/
			inline void operator += (const CWideString& str)
			{
				if ( &str != this )
				{
					catData( str.m_sStr );
				}
				else
				{
					CWideString wstr(m_sStr);
					catData(wstr.m_sStr);
				}
			}
			/*	与(const wchar_t*)类型进行加法运算并返回连接后的CWideString类	*/
			inline const CWideString operator + (const wchar_t* data)
			{
				CWideString str( m_sStr );
				str.catData( data );
				return str;
			}
			/*	与另一个字符串进行加法运算并返回连接后的CWideString类	*/
			inline const CWideString operator + (const CWideString& str)
			{
				CWideString _str( m_sStr );
				_str.catData( str.m_sStr );
				return _str;
			}
			/*  判断与(const wchar_t*)类型的字符串指针中的字符串数据是否相同（区分大小小写）  */
			inline bool operator == (const wchar_t* data) const
			{
				return compare( data ) == 0;
			}
			/*  判断与另一个字符串的数据是否相同（区分大小小写）  */
			inline bool operator == (const CWideString& str) const
			{
				return compare( str.m_sStr ) == 0;
			}
			/*  判断与(const wchar_t*)类型的字符串指针中的字符串数据是否不相同（区分大小小写）  */
			inline bool operator != (const wchar_t* data) const
			{
				return compare( data ) != 0;
			}
			/*  判断与另一个字符串的数据是否不相同（区分大小小写）  */
			inline bool operator != (const CWideString& str) const
			{
				return compare( str.m_sStr ) != 0;
			}

		public:
			/*	取得字符串指针	*/
			inline const wchar_t* rawStr() const
			{
				return m_sStr;
			}
			/*	取得字符串指针	*/
			inline wchar_t* rawStr()
			{
				return m_sStr;
			}
			/*	取得字符串长度	*/
			inline size_t length() const
			{
				return m_sStr ? wcslen(m_sStr) : 0;
			}
			/*	设置字符串长度(保留空间)	*/
			inline void setLength(const size_t len)
			{
				if ( len > 0 )
				{
					m_sStr = (wchar_t*)alloc( m_sStr, sizeof(m_sStr[0]) * (len + 1) );
					if ( m_sStr ) m_sStr[len] = 0;
				}
				else m_sStr = (wchar_t*)alloc( m_sStr, 0 );
			}
			/* 与另一个字符串指针对比  */
			inline int compare(const wchar_t* data) const
			{
				if ( !m_sStr || !m_sStr[0] )
				{
					if ( !data || !data[0] )
						return 0;
					else return 1;
				}
				return wcscmp( m_sStr, data );
			}
			/* 与另一个字符串类进行对比  */
			inline int compare(const CWideString& str) const
			{
				if ( !m_sStr || !m_sStr[0] )
				{
					if ( !str.m_sStr || !str.m_sStr[0] )
						return 0;
					else return 1;
				}
				return wcscmp( m_sStr, str.m_sStr );
			}
			/* 判断是否为空字符串 */
			inline bool isEmpty() const
			{
				return !m_sStr || !m_sStr[0];
			}
			/*	转换字符串编码为本地多字节(local multibytes)格式。
				如果sOutput参数不为空且编码转换成功则函数将转换结果赋值给sOutput参数并返回sOutput，
				若转换失败则返回null且sOutput的内容可能仍然会改变；
				如果sOutput参数为空且函数转换编码成功则返回新的CAnsiString对象，此时调用者需要再合适的时候delete返回值，
				转换失败则函数返回NULL
			*/
			CAnsiString* toAStr(CAnsiString* sOutput = NULL) const;
			/* 格式化字符串 */
			size_t format(const wchar_t* fmt, ...);
			size_t format_args(const wchar_t* fmt, va_list _Args);
		};

	};
};

#endif

