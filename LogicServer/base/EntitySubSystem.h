#pragma once
/***************************************************************/
/*
/*                     实体的子系统基类
/* 实体的功能模块,比如组队子系统,技能子系统，继承这个基类
/* 实体的子系统实现一个特定的功能，并有网络消息的分发功能
/* 游戏中的技能,buff，任务等特定的功能都可以搞一个子系统，给实体挂一个
/***************************************************************/
class CEntity;


/*
	SUBSYSTEMID 子系统的ID
	SonClass    子类
	EntityClass	该子系统所挂载的实体类型
*/
template<int SUBSYSTEMID,class SonClass, class EntityClass>
class CEntitySubSystem
{
public:
	typedef void (SonClass::*OnHandleSockPacket)(CDataPacketReader &packet); //网络消息处理函数，为子类定义的

public:
	CEntitySubSystem()
	{
		m_pEntity =NULL;
		m_hasDbInitData = false;
		m_hasDataModify = false;
		m_hasRequestData =false;
	}

	/*
	 *子系统的初始化，
	 *lpEntity 子系统所属的实体指针
	 *data 初始化的数据，如果没有用 NULL
     *size 子系统初始化数据的长度，如果没有数据用0	 
	*/
	inline bool Create(EntityClass *lpEntity,void *data,SIZE_T size)
	{
		m_pEntity = lpEntity;
		return Initialize(data,size);
	}
	
	//子系统初始化,如果是玩家需要输入玩家的存盘数据
	virtual bool Initialize(void *data,SIZE_T size){return true;}
	

	/*
	* Comments: 数据库返回数据
	* Param INT_PTR nCmd: 命令码，如下面的命令
		typedef enum tagDBRecvCmd
		{
			dcQuery=1,			//查询数据
			dcSave,				//保存数据
			dcLoadActor = 3,	//加载用户认证信息
			dcLoadQuest,         //装置任务数据
			dcSaveQuest,		//保存用户的任务数据
			dcLoadSkill,        //装置技能数据
			dcSaveSkill,        //保持技能数据
		};
	* Param INT_PTR nErrorCode: 错误码
		typedef enum tagResultError
		{
			reSucc = 0,
			reDbErr,		//数据库内部错误
			reNoActor,		//没有该角色的属性信息
			reSaveErr,		//保存属性失败
			reNoLoad,		//还没有读入数据
		};
	* Param CDataPacketReader reader &: 读写器
	* @Return VOID:
	*/
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader ){}

	/* Param CDataPacketReader reader &: 读写器
	* @Return VOID:
	*/
	virtual VOID OnCrossInitData(CDataPacketReader &reader ){}
	/*
	*存盘接口，传的是存盘的结构指针,单独存盘的系统就不关住这个数据指针
	*data 玩家存盘数据指针
	*/
	virtual void Save(PACTORDBDATA  pData){ }
	
	//清空内存
	virtual void  Destroy(){} ;								

	//获取子系统的ID
	inline INT_PTR GetSystemID() const {return SUBSYSTEMID;}

	/*
	 *处理网络数据包
	 *nCmd ，分派到该系统里命令，从0开始
	 *packet，网络数据
	*/
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet ){};

	//获取自身的实体句柄handle
	inline EntityHandle GetHandle() const
	{
		if (m_pEntity)
		{
			return m_pEntity->GetHandle(); //返回实体的句柄
		}
		else
		{ 
			return INVALID_ENTITY_HANDLE;	//返回0
		}
	} 
	
	//定时处理
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount){}; 
	
	//角色各子系统初始化完毕，向客户端发送创建主角的数据包后，调用该接口
	virtual void OnEnterGame(){};   

	//DB初始化子系统的数据完成
	inline void OnDbInitData()
	{
		m_hasDbInitData =true;
	}
	
	//DB的数据是否初始化完毕
	inline bool HasDbDataInit()
	{
		return m_hasDbInitData;
	}
	//当前子系统发送数据到跨服逻辑
	/*
	byte cmd
	int nsrvid; 跨服开启的srvid
	unsigned int :nActorId 玩家ID
	*/
	virtual void SendMsg2CrossServer(int nType) {}
	
	/*
	* Comments:设置数据的修改的标记，如果db没有初始化数据，设置修改也没有效果
	* Param bool flag:有数据修改过，设置为true,没有设置为false
	* @Return void:
	*/
	inline void SetDataModifyFlag(bool flag)
	{
		if(!m_hasDbInitData) return ;
		if(m_hasDataModify !=flag)
		{
			m_hasDataModify =flag;
		}
	}
	
	//数据是否修改过，在存盘的时候调用，用于优化存盘,如果没有修改，则不需要存盘
	inline bool HasDataModified()
	{
		return m_hasDataModify;
	}

	//是否向DB请求过数据
	inline bool HasRequestData()
	{
		return m_hasRequestData;
	}
	
	//当向DB请求数据的时候调用
	inline void OnRequestData()
	{
		m_hasRequestData =true;
	}
	

	//CDataPacket& AllocPacket(CActorPacket &pack, BYTE nMsgId){
	//	CDataPacket& dp = m_pEntity->AllocPacket(pack);
	//	dp << SUBSYSTEMID << nMsgId;
	//	return dp;
	//}
	
protected:
	EntityClass*  m_pEntity ; //这个子系统是属于那个实体的,保持的是实体的指针

protected:
	static const OnHandleSockPacket		Handlers[];	//网络数据包处理函数列表 
	bool   m_hasDbInitData;                         //DB是否初始化了改子系统的数据
	bool   m_hasDataModify;                         //数据是否修改过，如果修改过，存盘的时候需要存盘，存盘完了调用一下
	bool   m_hasRequestData;                        //是否向DB请求过数据
	
	//DECLARE_OBJECT_COUNTER(CEntitySubSystem)
};
