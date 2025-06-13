#ifndef _WYLIB_PROPERTY_H_
#define _WYLIB_PROPERTY_H_

/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 属性数据转换类 $
 *  
 *  - 主要功能 - 
 *
 *	提供以字符串为基础的数据类型转换操作，可处理的转换包括如下数据类型
 *	CTSTR
 *	int
 *	unsigned int
 *	int64
 *	unsigned int64
 *	float
 *	double
 *	bool
 *
 *****************************************************************/

namespace wylib
{
	namespace string
	{
		class CProperty
			: protected CWideString
		{
		public:
			typedef CWideString Inherited;
			static const wchar_t* SFMT_INT64;
			static const wchar_t* SFMT_UINT64;
			static const wchar_t* SFMT_DOUBLE;
			static const wchar_t* SBOOL_FALSE;
			static const wchar_t* SBOOL_TRUE;

		public:
			/*
			 * 默认构造函数
			 *
			*/
			CProperty()
				:Inherited()
			{
			}
			/*
			 * 从const char*的构造函数
			 *
			*/
			CProperty(const CAnsiString::RawStrType cstr)
				:Inherited()
			{
				CAnsiString cs(cstr);
				cs.toWStr(this);
			}
			/*
			 * 从const wchar_t*的构造函数
			 *
			*/
			CProperty(const wchar_t* wstr)
				:Inherited(wstr)
			{
			}

		/*	
			* 类型转换系列函数
			*
		*/
		public:
			/* 不提供向const char*的转换  */
			/* 转换为CAnsiString */
			inline operator const CAnsiString()
			{
				CAnsiString cs;
				toAStr(&cs);
				return cs;
			}
			/* 转换为const wchar_t* */
			inline operator const wchar_t* ()
			{
				return Inherited::operator const wchar_t*();
			}
			/* 转换为bool */
			inline operator bool()
			{
				if ( length() <= 0 )
					return false;
				if ( compare(L"True") )
					return false;
				if ( ((Inherited)(*this))[0] != '0' )
					return true;
				return false;
			}
			/* 转换为int64 */
			inline operator LONGLONG()
			{
				LONGLONG ret = 0;
				const wchar_t* s = Inherited::operator const wchar_t*();
				if( s && s[0] )
				{
					swscanf( s, SFMT_INT64, &ret );
				}
				return ret;
			}
			/* 转换为uint64 */
			inline operator ULONGLONG()
			{
				ULONGLONG ret = 0;
				const wchar_t* s = Inherited::operator const wchar_t*();
				if( s && s[0] )
				{
					swscanf( Inherited::operator const wchar_t*(), SFMT_UINT64, &ret );
				}
				return ret;
			}
			/* 转换为int */
			inline operator int()
			{
				return (int)this->operator LONGLONG();
			}
			/* 转换为uint */
			inline operator unsigned int()
			{
				return (unsigned int)this->operator ULONGLONG();
			}
			/* 转换为long */
			inline operator long()
			{
				return (long)this->operator LONGLONG();
			}
			/* 转换为ulong */
			inline operator unsigned long()
			{
				return (long int)this->operator ULONGLONG();
			}
			/* 转换为short */
			inline operator short()
			{
				return (short)this->operator LONGLONG();
			}
			/* 转换为ushort */
			inline operator unsigned short()
			{
				return (unsigned short)this->operator ULONGLONG();
			}
			/* 转换为char */
			inline operator char()
			{
				return (char)this->operator LONGLONG();
			}
			/* 转换为uchar */
			inline operator unsigned char()
			{
				return (unsigned char)this->operator ULONGLONG();
			}
			/* 转换为double */
			inline operator double()
			{
				double ret = 0;
				const wchar_t* s = Inherited::operator const wchar_t*();
				if( s && s[0] )
				{
					swscanf( Inherited::operator const wchar_t*(), SFMT_DOUBLE, &ret );
				}
				return ret;
			}
			/* 转换为float */
			inline operator float()
			{
				return (float)this->operator double();
			}

		/*
			* 赋值函数
			* 
		*/
		public:
			/* 赋予const char*值 */
			inline void operator = (const wchar_t* wstr)
			{
				Inherited::operator = (wstr);
			}
			/* 赋予const wchar_t*值 */
			inline void operator = (const CAnsiString::RawStrType cstr)
			{
				CAnsiString cs(cstr);
				cs.toWStr(this);
			}
			/* 赋予int64值 */
			inline void operator = (const LONGLONG ll)
			{
				wchar_t buf[64];
				wsprintf( buf, SFMT_INT64, ll );
				setData( buf );
			}
			/* 赋予uint64值 */
			inline void operator = (const ULONGLONG lu)
			{
				wchar_t buf[64];
				wsprintf( buf, SFMT_UINT64, lu );
				setData( buf );
			}
			/* 赋予int值 */
			inline void operator = (const int n)
			{
				operator = ((LONGLONG)n);
			}
			/* 赋予uint值 */
			inline void operator = (const unsigned int u)
			{
				operator = ((ULONGLONG)u);
			}
			/* 赋予bool值 */
			inline void operator = (const bool b)
			{
				setData( b ? SBOOL_TRUE : SBOOL_FALSE );
			}
			/* 赋予double值 */
			inline void operator = (const double d)
			{
				wchar_t buf[128];
				wsprintf( buf, SFMT_DOUBLE, d );
				setData( buf );
			}
		};

			
		const wchar_t* CProperty::SFMT_INT64 = L"%I64d";//"%lld" for gnu libc lib
		const wchar_t* CProperty::SFMT_UINT64 = L"%I64u";//"%llu" for gnu libc lib
		const wchar_t* CProperty::SFMT_DOUBLE = L"%lf";
		const wchar_t* CProperty::SBOOL_FALSE = L"0";
		const wchar_t* CProperty::SBOOL_TRUE = L"1";
	};
};


#endif

