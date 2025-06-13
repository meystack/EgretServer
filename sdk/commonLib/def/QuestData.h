#pragma once
#pragma  pack(4)
struct QuestItem
{
	int         nQid;//任务id
	int         nType;//任务类型
	int         nValue;//任务进度
	int         nState;//任务状态
	int         nComplete;//完成
	int         nUseFsTimes;//使用飞鞋次数
	int         nStartId;//环形 任务开始id
	QuestItem()
	{
		memset(this, 0, sizeof(*this));
	}
};
#pragma  pack()