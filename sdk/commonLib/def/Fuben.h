#pragma once

//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)
	//保存这个玩家的副本的记录数据
	typedef struct tagFubenData
	{
		WORD			wFubenId; //副本的id，对应配置表
		WORD			wDaycount;	//本副本今日进入的次数
		BYTE			bProgress;	//当前的进度值
	}FubenData;
#pragma pack(pop)