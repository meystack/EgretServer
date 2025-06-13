#pragma once

//tolua_begin

namespace ScoreRecorder
{
	//创建一个计分器，创建计分器后不必调用addRef，以及自动调用了。当不再使用这个计分器的时候，必须调用release解除引用。
	void * create(const char *sName);
	//增加引用计数
	//注意：调用Create函数创建的计分器后不必调用addRef，除非有特殊的需求。
	int addRef(void* pRecorder);
	//减少引用计数，对象引用计数为0的时候将自动销毁对象
	//****脚本必须在不使用计分器对象的时候调用release解除对计分器的引用，否则将造成内存泄露。
	int release(void* pRecorder);

	//将一个队伍加入到计分器通知列表中
	void joinTeam(void *pRecorder, void *pTarget);
	//将一个帮会加入到计分器通知列表中
	void joinGuild(void *pRecorder, void *pTarget);
	//将一个阵营加入到计分器通知列表中
	void joinCamp(void *pRecorder, void *pTarget);
	//将一个副本加入到计分器通知列表中
	void joinFuBen(void *pRecorder, void *pTarget);

	//将一个通知对象从计分器通知列表中移除
	void leave(void *pRecorder, void *pTarget);

	//停止一个计分器对象
	void stop(void* pRecorder);

	//为计分项增加分值。如果计分项不存在则会创建计分项并设置值为要增加的值，否则会增加原有计分项的值。
	int incScore(void* pRecorder, const char* sName, int nValueToAdd);

	//改变计分项分值。如果计分项不存在则会创建计分项并设置值，否则会调整原有计分项的值。
	int setScore(void* pRecorder, const char* sName, int nValue);

	//获取一个计分项的值
	int getScore(void* pRecorder, const char* sName);

	//判断指定名称的计分项是否存在
	bool itemExists(void* pRecorder, const char* sName);

	//获取完整的计分器数据，数据表的格式为{ { name = "计分项名称", value = 分值 } .. }
	int getScoreData(lua_State *L);
}

//tolua_end