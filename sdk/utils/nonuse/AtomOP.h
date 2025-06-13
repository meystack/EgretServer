#pragma once

/************************************************************************/
/*
/*                具有防止运算溢出的原子数据加减法运算函数库
/*
/*  运算函数会捕获运算结果的数据类型溢出错误从而修正返回值为数据类型的最小或最大值。
/*
/************************************************************************/
namespace wylib
{
	namespace arithmetic
	{
		namespace atomSafe
		{
			/* 64位原子类型加减法运算，会捕获运算结果的数据类型溢出错误从而修正返回值为数据类型的最小或最大值 */
			inline UINT64 uint64Add(UINT64 a, UINT64 add) { return UInt64Add(a, add); }
			inline INT64 int64Add(INT64 a, INT64 add) { return Int64Add(a, add); }
			/* 32位原子类型加减法运算，会捕获运算结果的数据类型溢出错误从而修正返回值为数据类型的最小或最大值 */
			inline UINT uintAdd(UINT_PTR a, UINT_PTR add){ return UIntAdd(a, add); }
			inline INT intAdd(INT_PTR a, INT_PTR add){ return IntAdd(a, add); }
			/* 16位原子类型加减法运算，会捕获运算结果的数据类型溢出错误从而修正返回值为数据类型的最小或最大值 */
			inline WORD ushortAdd(UINT_PTR a, UINT_PTR add){ return UShortAdd(a, add); }
			inline INT shortAdd(INT_PTR a, INT_PTR add){ return ShortAdd(a, add); }
			/* 8位原子类型加减法运算，会捕获运算结果的数据类型溢出错误从而修正返回值为数据类型的最小或最大值 */
			inline UCHAR usmallAdd(UINT_PTR a, UINT_PTR add){ return USmallAdd(a, add); }
			inline CHAR smallAdd(INT_PTR a, INT_PTR add){ return SmallAdd(a, add); }
		}
	}
}