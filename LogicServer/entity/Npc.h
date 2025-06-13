
#pragma once

/***************************************************************/
/*
/*                     NPC实体，集成怪物实体
/*
/***************************************************************/
#define MAXFUNCTIONLENGTH 1024
class CNpc:
	public CEntity
{
public:
	typedef CEntity Inherited;

public:

	static int GETTYPE() {return enNpc;}

	CNpc();

	void InitNPCAi();

	//初始化
	bool Init(void * data, size_t size){ SetInitFlag(true); return Inherited::Init(data,size); } 
	//删除的一些内存消耗操作
	inline void Destroy()
	{ 
		m_Script.setScript(NULL, NULL); 
		Inherited::Destroy(); 
	}
	//处理LogicRun，实现对脚本的内存定时回收
	void LogicRun(TICKCOUNT nCurrentTime);
	//处理实体消息
	void ProcessEntityMsg(const CEntityMsg &msg);

	//获取属性集的指针
	virtual CPropertySet * GetPropertyPtr() {return &m_property ;}
	virtual const CPropertySet * GetPropertyPtr() const {return &m_property ;}
	virtual  INT_PTR GetPropertySize() const {return sizeof(CNPCProperty) -sizeof(CPropertySet);}
	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}
	
public:

	inline VOID SetConfig(NPCCONFIG* pConifg) 
	{
		m_pConfig = pConifg;
	}
	inline NPCCONFIG* GetConfig() { return m_pConfig;}
	//获取脚本对象
	inline CLogicScript& GetScript(){ return m_Script; }
	/*
	* Comments: 加载/重新加载NPC脚本
	* Param LPCSTR sPath: 脚本文件路径，可以是绝对路径或基于程序运行目录的相对路径
	* Param bool boReload:是否重新读取
	* @Return bool: 返回加载脚本是否成功
	*/
	bool LoadScript(LPCSTR sPath =NULL,bool boReload=false);
	// /*
	// * Comments: 用户与NPC对话并执行指定的函数，如果脚本函数返回字符串，那么脚本返回值将发给客户端呈现
	// * Param CActor* pActor: 对话的玩家对象
	// * Param LPCSTR sFnName: 函数名称
	// * @Return void:
	// */
	// void Talk(CActor *pActor, LPCSTR sFnName);

	// void Talk(CActor * pActor,LPCSTR sFnName,CScriptValueList& va);

	//用户点击NPC的处理函数，默认是调用main函数
	//inline void Click(CActor *pActor){ Talk(pActor, "main"); }
	
	//发送消息，关闭npc的对话窗口
	VOID CloseDialog(CActor *pActor);

	/*
	* Comments: NPC发言
	* @Return void:
	*/
	void NPCTalk();

	/*
	* Comments: 重新刷一遍脚本
	* @Return void:
	* @Remark: 增加此接口用于重新刷一次NPC脚本。主要针对NPC复用。考虑到当前已经处于内测阶段，保证稳定性为主。以后可以考虑
	*		   不用reloadScript，而是call Uninitialize和Initialize接口。	
	*/
	void ResetScript();

	/*
	* Comments:设置npc的执行idle的间隔时间
	* Param int nTime:间隔时间，最小是30000（30秒），单位毫秒
	* @Return void:
	*/
	inline void SetIdleTime(int nTime)
	{
		m_idleTime = ((nTime != 0 && nTime < 30000)?30000:nTime);
	}

	//调用npc脚本的一个函数
	bool Call( LPCSTR sFnName);

	//调用脚本某个模块的一个函数
	bool CallModule(LPCSTR sModuleName,LPCSTR sFnName);

	/*
	* Comments: 通过场景和NPC的名字获取NPC的指针
	* Param char * sSceneName:场景的名字
	* Param char * sNpcName:npc的名字
	* @Return CNpc *:返回NPC的指针
	*/
	static CNpc * GetNpcPtr(char * sSceneName,char * sNpcName);


	
	/**
	* Comments:函数的参数处理，约定的格式为函数名字,参数1，参数2，处理完以后va 中包含参数1，参数2
	* Param LPCSTR  str:输出的函数的名字 格式为函数名字,参数1，参数2
	* Param CScriptValueList & va: 参数列表
	* Param char *  & pFunc:函数的名字
	* Param nOutBuffSize: pFunc这个字符串缓冲的长度
	* @param boAddFuncParam 是否压入函数名
	* @Return void:
	*/
	void FuncParamProcess(LPCSTR  str, CHAR * pFunc, INT_PTR nOutBuffSize,CScriptValueList & va, bool boAddFuncParam = false );

	//调用npc脚本，包含玩家指针
	//void CallScript(CActor * pActor, LPCSTR sFnName);

	//调用npc脚本，不包含玩家指针
	void OnNpcCaller(LPCSTR sFnName, int nNpcId = -1);

	bool LoadScriptBuff(LPCTSTR sFilePath)
	{
		return m_Script.LoadBuff(sFilePath);
	}
	//重新加载npc脚本
	void ReloadAllNpc();
	//检查脚本错误
	void CheckAllScript(char* sParam);
protected:
	CNPCProperty	m_property;  			// 属性集
	CLogicScript	m_Script;			//脚本对象
	NPCCONFIG*		m_pConfig;			//npc的配置信息
	CTimer<60000>	m_ScriptGCTimer;	//脚本垃圾收集定时器（动态时间）
	CTimer<10000>	m_RunIdleTimer;		//NPC调用Idle函数定时器（动态时间）	
	bool			m_bScriptInited;	// 脚本初始化完毕
	int				m_idleTime;			//执行idle的间隔时间,默认是0
};
