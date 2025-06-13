#pragma once

class CNpc;

//一件套装的属性
typedef struct tagScriptTimer
{
	DataList<int> months; //月
	DataList<int> days;  //日
 
	DataList<int> weeks;//星期，-1表示匹配所有
	DataList<int> hours ;   //小时，-1表示匹配所有
	DataList<int> minutes;  //分，-1表示匹配所有
	DataList<int> serverday;	//开服第几天
	DataList<int> notopenserverdays;		//表示开服的第几天不能用，其他时间都可以用
	DataList<int> combineserverday;			//合服第几天
	DataList<int> notCombineserverday;		//表示合服的第几天不能用，其他时间都可以用
	DataList<int> beforeCombineday;			//合服前第几天
	DataList<int> notBeforeCombineday;		//表示合服的前第几天不能用，其他时间都可以用
	
	//char         sSceneName[32]; //场景的名字
	//char         sNpcName  [32]; //NPC的名字
	char         sFuncName [128]; //脚本的名字
	TICKCOUNT    tick; //上次调用的tick
	int         nNpcId;   //NPC的Id，如果为0表示全局执行，否则执行npc下的函数

}SCRIPTTIMER,*PSCRIPTTIMER;

class CScriptTimerConfig :
	public CCustomLogicLuaConfig,
	public CVector<SCRIPTTIMER>
{
public:
	typedef CVector<SCRIPTTIMER>		Inherited;
	typedef CCustomLogicLuaConfig	Inherited2;
	typedef CObjectAllocator<char>	CDataAllocator;
public:
	CScriptTimerConfig(){}
	~CScriptTimerConfig()
	{
		Inherited::empty();
	}

	//转换函数
	/*
	inline operator const SCRIPTTIMER* () const
	{
		CScriptTimerConfig *pProvider = (CScriptTimerConfig*)this;
		return pProvider->Inherited::operator SCRIPTTIMER*();
	}
	*/


	/*
	* Comments:获取数量
	* @Return INT_PTR: 
	*/
	inline INT_PTR count() const{ return Inherited::count(); }

	
	/*
	* Comments: 从文件里装载定时执行的数据
	* Param LPCTSTR sFilePath: 路径的名称
	* @Return bool: 失败返回false
	*/
	bool LoadScriptTimerData(LPCTSTR sFilePath);


protected:
	//以下函数为覆盖父类的相关数据处理函数
	//void showError(LPCTSTR sError);
	void ReadTable(DataList<int> &intList, char * tableName);

private:
	
	/*
	* Comments:装载数据
	* @Return bool: 失败返回false
	*/
	bool ReadAllScriptData();


private:
	CDataAllocator m_DataAllocator; //内存分配器
	//CVector<SCRIPTTIMER> m_data; // 数据
};



