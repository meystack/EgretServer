#pragma once
//数据读取的时候的一些公共的数据结构
//这个结构体存游戏中任务数据的一些数组
template <class T> 
struct DataList
{
	DataList<T>()
	{
		pData=NULL;
		count =0;
	}
	T *  pData;
	INT_PTR count;
	inline operator T* () const { return pData; }
	inline T *GetDataPtr(INT_PTR nIndex)
	{
		if(pData && nIndex>=0 && nIndex < count )
		{
			return &pData[nIndex];
		}
		else
		{
			return NULL;
		}
	}
};