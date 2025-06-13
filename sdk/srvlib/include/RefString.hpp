#pragma once

#include <wyString.h>
#include "RefClass.hpp"
#include <stdarg.h>

namespace wylib
{
	namespace string
	{
		template <class TS, typename TC>
		class CTRefString
			: public wylib::misc::CRefObject<TS>
		{
			typedef wylib::misc::CRefObject<TS> Inherited;
			typedef wylib::misc::CRefObjectImpl<TS> CRefTS;
		public:
			/* 构造函数列表 */
			CTRefString()
				:Inherited()
			{
			}
			CTRefString(const size_t len)
			{
				Inherited::m_ptr = new CRefTS(len);
				Inherited::m_ptr->addRef();
			}
			CTRefString(const TC* str)
			{
				operator = (str);
			}
			CTRefString(const TS& str)
			{
				Inherited::m_ptr = new CRefTS(str);
				Inherited::m_ptr->addRef();
			}
			CTRefString(const CTRefString<TS, TC>& rStr)
			{
				operator = (rStr);
			}
		public:
			/*  类型转换函数，转换为(const TC*)。
				注意： 如果当前字符串为NULL，则会返回""而不是NULL。
			*/
			inline operator const TC* () const
			{
				return Inherited::m_ptr ? Inherited::m_ptr->rawStr() : NULL;
			}
			/*  类型转换函数，转换为(const TC*)。
				注意： 如果当前字符串为NULL，则会返回""而不是NULL。
			*/
			inline operator TC* ()
			{
				return Inherited::m_ptr ? Inherited::m_ptr->rawStr() : NULL;
			}
			/*	赋值函数，赋予(const TC*)类型的值	*/
			inline void operator = (const TC* data)
			{
				CRefTS *newPtr = new CRefTS();
				*((TS*)newPtr) = data;
				newPtr->addRef();
				if (Inherited::m_ptr) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;
			}
			/*	赋值函数，赋予TS类型的值	*/
			inline void operator = (const TS& str)
			{
				CRefTS *newPtr = new CRefTS();
				*((TS*)newPtr) = str;
				newPtr->addRef();
				if (Inherited::m_ptr) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;
			}
			/*	与(const TC*)类型的字符串连接  */
			inline void operator += (const TC* data)
			{
				CRefTS *newPtr = new CRefTS();
				newPtr->addRef();
				if (Inherited::m_ptr) *((TS*)newPtr) = *Inherited::m_ptr;
				*newPtr += data;
				if (Inherited::m_ptr) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;
			}
			/*	与自己或其他字符串连接	*/
			inline void operator += (const TS& str)
			{
				CRefTS *newPtr = new CRefTS();
				newPtr->addRef();
				if (Inherited::m_ptr) *((TS*)newPtr) = *Inherited::m_ptr;
				*newPtr += str;
				if (Inherited::m_ptr) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;
			}
			/*  判断与(const TC*)类型的字符串指针中的字符串数据是否相同（区分大小小写）  */
			inline bool operator == (const TC* data) const
			{
				return compare( data ) == 0;
			}
			/*  判断与另一个字符串的数据是否相同（区分大小小写）  */
			inline bool operator == (const TS& str) const
			{
				return compare( str ) == 0;
			}
			/*  判断与另一个引用字符串的数据是否相同（区分大小小写）  */
			inline bool operator == (const CTRefString<TS, TC>& str) const
			{
				return compare( str ) == 0;
			}
			/*  判断与(const TC*)类型的字符串指针中的字符串数据是否不相同（区分大小小写）  */
			inline bool operator != (const TC* data) const
			{
				return compare( data ) != 0;
			}
			/*  判断与另一个字符串的数据是否不相同（区分大小小写）  */
			inline bool operator != (const TS& str) const
			{
				return compare( str.m_sStr ) != 0;
			}
			/*  判断与另一个引用字符串的数据是否不相同（区分大小小写）  */
			inline bool operator != (const CTRefString<TS, TC>& str) const
			{
				return compare( str ) != 0;
			}

		public:
			/*	取得字符串指针	*/
			inline const TC* rawStr() const
			{
				return Inherited::m_ptr ? Inherited::m_ptr->rawStr() : NULL;
			}
			/*	取得字符串指针	*/
			inline TC* rawStr()
			{
				return Inherited::m_ptr ? Inherited::m_ptr->rawStr() : NULL;
			}
			/*	取得字符串长度	*/
			inline size_t length() const
			{
				return Inherited::m_ptr ? Inherited::m_ptr->length() : 0;
			}
			/*	设置字符串长度(保留空间)	*/
			inline void setLength(const size_t len)
			{
				//CRefObjectImpl<TS> *newPtr = new CRefObjectImpl<TS>(len);
				CRefTS *newPtr = new CRefTS(len);
				newPtr->addRef();
				if (Inherited::m_ptr) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;
			}
			/* 与另一个字符串指针对比  */
			inline int compare(const TC* data) const
			{
				if ( !Inherited::m_ptr )
					return !data ? 0: -1;
				if ( data == Inherited::m_ptr->rawStr() )
					return 0;
				return Inherited::m_ptr->compare(data);
			}
			/* 与另一个字符串类进行对比  */
			inline int compare(const TS& str) const
			{
				if ( !Inherited::m_ptr )
					return str.isEmpty() ? 0 : -1;
				if ( Inherited::m_ptr == &str )
					return 0;
				return Inherited::m_ptr->compare(str);
			}
			/* 与另一个引用字符串类进行对比  */
			inline int compare(const CTRefString<TS, TC>& str) const
			{
				if ( Inherited::m_ptr == str.Inherited::m_ptr )
					return 0;
				if ( ! Inherited::m_ptr ) return -1;
				return Inherited::m_ptr->compare(str->raw_ptr());
			}
			/* 格式化字符串  */
			inline size_t format(const TC* fmt, ...)
			{
				va_list	args;
				size_t Result = 0;

				va_start(args, fmt);
				Result = format_args(fmt, args);
				va_end(args);

				return Result;
			}
			inline size_t format_args(const TC* fmt, va_list _Args)
			{
				size_t Result = 0;

				wylib::misc::CRefObjectImpl<TS> *newPtr = new wylib::misc::CRefObjectImpl<TS>();
				newPtr->addRef();
				Result = newPtr->format_args(fmt, _Args);
				if ( Inherited::m_ptr ) Inherited::m_ptr->release();
				Inherited::m_ptr = newPtr;

				return Result;
			}
		};

		typedef CTRefString<CWideString, wchar_t>	CRefWideString;
		typedef CTRefString<CAnsiString, char>		CRefAnsiString;
	};
};

#ifdef UNICODE
typedef wylib::string::CRefWideString	RefString;
#else
typedef wylib::string::CRefAnsiString	RefString;
#endif
