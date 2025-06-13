#pragma once

extern "C"
{
	UINT64 WINAPI UInt64Add(const UINT64, const UINT64 add);
	INT64 WINAPI Int64Add(const INT64, const INT64 add);
	UINT WINAPI UIntAdd(const UINT_PTR, const UINT_PTR add);
	INT WINAPI IntAdd(const INT_PTR, const INT_PTR add);
	WORD WINAPI UShortAdd(const UINT_PTR, const UINT_PTR add);
	SHORT WINAPI ShortAdd(const INT_PTR, const INT_PTR add);
	UCHAR WINAPI USmallAdd(const UINT_PTR, const UINT_PTR add);
	CHAR WINAPI SmallAdd(const INT_PTR, const INT_PTR add);
};
