/*机器人组
* 若干个机器人共有一个线程连接服务器
* 暂定为64个机器人共有一个线程连接到服务器
*/
#pragma  once 

#define  MAX_THREAD_CLIENT_COUNT 36  //单个的线程最多承载多少个用户
class CRobotGroup
{
public:
	CRobotGroup()
	{
		m_hThread =INVALID_HANDLE_VALUE;
		m_clientCount=0;
		m_clints = NULL;
		m_hasStart = false;
		m_nId = 0;
		m_IntervalTime = 0;
	};

	~CRobotGroup()
	{
		if(m_clints)
		{
			delete []m_clints;
		}
		
	};

	void SetID(int nId)
	{
		m_nId = nId;
	}


	/*
	* Comments:设置一些参数，比如玩家列表等信息
	* Param int nStartAccountIndex:本组负责的开始的玩家的index
	* Param int nEndAccountIndex:本组负责的结束的玩家的index
	* @Return bool:
	*/
	bool Init(int nStartAccountIndex, int nEndAccountIndex);

	//处理网络的数据和服务器进行连接
	//static VOID	RecvDataRun(CRobotGroup *pThread); 

	//开现场执行
	static VOID ThreadRun(void *lpThread);

	/*
	* Comments:启动线程测试，
	* Param INT_PTR nStartUserIndex:开始的用于的
	* Param INT_PTR nEndUserIndex:结束的用于的index
	* @Return void:
	*/
	bool Start(); 

	void Stop();  //停止测试

	//一次逻辑循环调用
	void SingleRun();
	
	//获取一个连接的指针
	inline CRobotClient * GetClient(INT_PTR nIndex)
	{
		if(m_clints ==NULL || nIndex <0 || nIndex >= m_clientCount) return NULL;
		return &(m_clints[nIndex]);
	}

	//获取有多少个连接
	inline int GetClientCount(){return m_clientCount;}
	
private:
	CRobotClient *m_clints ;  //客户端的对象的列表
	int          		m_clientCount;  //实际使用的客户端的数目
	THREAD_HANDLE		 m_hThread;  //线程的handle
	bool         m_hasStart;    //是否启动成功
	int			 m_nId;	

	int			m_IntervalTime;		//发言时间间隔

};