#ifndef TRANSMIT_MGR_H_
#define TRANSMIT_MGR_H_

/*
		传送管理器。用于管理玩家从一个服务器传送到另外一个服务器的操作。包括玩家发起传送的数据打包、
	传送到目的地的数据解包、公共服务器发给普通服务器的数据定时存盘数据处理等等

	源逻辑服务器：											目标服务器					
		发送请求传送消息（角色Id、场景名称、坐标）			
												接收到请求传送消息，检测场景是否可进入
												如果可以传送，给出可以进入的应答消息。
		发送开始传送消息，带上角色Id					
												为此用户设置会话状态，并且创建此角色的缓存数据对象。
		打包并发送角色基本数据和子系统数据				接收到用户数据，先缓存起来。如果客户端连接上
												就创建角色，并且初始化角色数据。如果客户端超时还
												没有连接上，就关闭用户会话，删除角色缓存数据对象
*/

// 角色传送缓存数据。传送目的方接收到数据先缓存起来
namespace TRANSMIT
{
	class CActorTransmitCacheData
	{
	public:
		CActorTransmitCacheData() : m_nDataType(-1), m_packet(0){}
		CActorTransmitCacheData(int nType, CDataPacket *packet){
			m_nDataType = nType;
			m_packet = packet;
		}

		int GetDataType() const { return m_nDataType; }
	private:
		int				m_nDataType;		// 角色数据类型
		CDataPacket		*m_packet;			// 数据包	
	};

	class CActorTransmitCache
	{
	public:
		enum
		{
			TRANSMIT_TIMEOUT = 2 * 60 * 1000,		// 传送超时
		};

		CActorTransmitCache()
		{
			m_nAccountId	= 0;
			m_bAllDataReady = false;
		}

		unsigned int GetAccountId() const
		{
			return m_nAccountId;
		}

		void SetAccountId(unsigned int nAccountId)
		{
			m_nAccountId = nAccountId;
		}
				
		unsigned int GetStartTransTime() const 
		{
			return m_nStartTransTime;
		}
				
		void SetStartTransTime(unsigned int nTime)
		{
			m_nStartTransTime = nTime;
		}

		/*
		* Comments: 缓存角色传送数据
		* Param int nType: 数据类型。例如技能数据、背包物品数据等等
		* Param CDataPacket * packet: 数据包内容
		* @Return void:
		* @Remark:
		*/
		void CacheData(int nType, CDataPacket *packet)
		{
			CActorTransmitCacheData *pCacheData = GetCacheData(nType);
			if (pCacheData)
			{
				// TODO 覆盖
			}
			else
			{
				// 添加一个新的
				CActorTransmitCacheData cacheData(nType, packet);				
				m_cache.add(cacheData);
			}
		}

		/*
		* Comments: 获取指定类型的数据
		* Param const int nType:
		* @Return CActorTransmitCacheData*:
		* @Remark:
		*/
		CActorTransmitCacheData* GetCacheData(const int nType)
		{
			for (INT_PTR i = 0; i < m_cache.count(); i++)
			{
				if (nType == m_cache[i].GetDataType())
					return &m_cache[i];
			}

			return NULL;
		}

		/*
		* Comments: 设置全部数据缓存完毕标记
		* Param bool flag:
		* @Return void:
		* @Remark:
		*/
		void SetAllDataReady(bool flag)
		{
			m_bAllDataReady = flag;
		}

	private:
		unsigned int						m_nAccountId;		// 该角色的登录账号ID
		unsigned int						m_nStartTransTime;	// 开始传送时间
		bool								m_bAllDataReady;	// 所有的传送数据都缓存OK
		CVector<CActorTransmitCacheData>	m_cache;
	};

	// 同一个服务器内的所有角色数据
	class CServerActorInfo
	{
	public:
		/*
		* Comments: 获取指定角色ID的Cache数据对象
		* Param int nActorId:
		* @Return CActorTransmitCache*:
		* @Remark:
		*/
		CActorTransmitCache* GetActorCache(unsigned int nActorId)
		{
			std::map<int, CActorTransmitCache*>::iterator iter = m_actorDBData.find(nActorId);
			if (iter != m_actorDBData.end())
				return iter->second;

			return NULL;
		}

		CActorTransmitCache* GetActorCacheByAccountId(unsigned int nAccountId)
		{
			std::map<int, CActorTransmitCache*>::iterator iter = m_actorDBData.begin();
			std::map<int, CActorTransmitCache*>::iterator iter_end = m_actorDBData.end();
			for (; iter != iter_end; ++iter)
			{
				if (iter->second->GetAccountId() == nAccountId)
					return iter->second;
			}

			return NULL;
		}

		/*
		* Comments: 添加一个角色Cache数据
		* Param int nActorId:
		* @Return CActorTransmitCache*: 返回此角色的CaChe对象指针
		* @Remark:
		*/
		CActorTransmitCache* AddActorCache(unsigned int nActorId)
		{
			CActorTransmitCache *pCache = GetActorCache(nActorId);
			if (pCache != NULL)
				return pCache;

			pCache = new CActorTransmitCache();
			m_actorDBData[nActorId] = pCache;
			return pCache;
		}
				
		/*
		* Comments: 删除一个角色Cache数据
		* Param int nActorId:
		* @Return void:
		* @Remark:
		*/
		void RemoveActorCache(unsigned int nActorId)
		{
			std::map<int, CActorTransmitCache*>::iterator iter = m_actorDBData.find(nActorId);
			if (iter != m_actorDBData.end())
			{
				delete iter->second;
				m_actorDBData.erase(iter);
			}
		}

		/*
		* Comments: 踢出所有的玩家
		* @Return void:
		* @Remark:
		*/
		void CloseAllActor();

		/*
		* Comments: 例行更新角色传送缓存对象
		* Param TICKCOUNT nCurrTick:
		* @Return void:
		* @Remark:
		*/
		void RunOne(TICKCOUNT nCurrTick);

	private:
		std::map<int, CActorTransmitCache*>		m_actorDBData;	// 角色Id --> 传送缓存数据
	};

	// 记录角色传送到公共服务器的一些信息
	struct CTransedActorRecord
	{
		unsigned int		m_nAccountId;				// 账号ID
		unsigned int		m_nActorId;					// 角色ID
		unsigned int		m_nLoginTime;				// 角色登录普通逻辑服务器的时间
	};
}


class CTransmitMgr
{
public:
	enum
	{
		SERVER_INDEX_MAX = 1024,				// 最大的ServerIndex
	};

	CTransmitMgr(){}
	~CTransmitMgr(){}
		
	/*
	* Comments: 请求将玩家传送到远程逻辑服务器
	* Param CActor * pActor: 角色指针
	* Param const char * pSceneName: 场景名称
	* Param const int nPosX: 场景坐标X
	* Param const int nPosY: 场景坐标Y
	* @Return bool:
	* @Remark: 此函数发送消息给远程逻辑服务器检测玩家是否可以传送到目标服务器的指定场景
	*/
	bool ReqTransmitTo(CActor *pActor, const char *pSceneName, const int nPosX, const int nPosY);
	
	/*
	* Comments: 从远程逻辑服务器传送到本服务器
	* Param INT_PTR nCmd: 消息ID
	* Param CDataPacketReader & reader: 消息数据
	* @Return void:
	* @Remark: 调用此函数接收处理角色传送各个模块的数据，这里接收到数据处理类似于角色登录从DB中加载数据处理
	*/
	void TransmitFrom(INT_PTR nCmd, CDataPacketReader &reader);
	
	/*
	* Comments: 是否存在指定服务器的指定角色的传送会话信息
	* Param int nServerIdx: 角色传送前的ServerIndex
	* Param int nActorId: 角色ID
	* @Return bool: 存在角色传送会话信息，返回true；否则返回false
	* @Remark: 收到源服务器的开始传送消息后就为此角色建立传送会话数据结构，并且设置超时。在超时时间内角色连接到本逻辑服务器不需要
			   查询DB和Session会话状态，直接创建角色。
	*/
	bool ExistActorTransSessionData(int nServerIdx, unsigned int nActorId);

	/*
	* Comments: 判断是否存在指定账号的传送会话信息
	* Param int nServerIdx: 角色传送前的ServerIndex
	* Param int nAccount:
	* @Return bool:
	* @Remark:
	*/
	bool ExistAccountTransSessionData(int nServerIdx, unsigned int nAccount);

	/*
	* Comments: 与连接的对端逻辑服务器断开时调用此函数。
	* Param int nServerIdx:
	* @Return void:
	* @Remark:对于普通逻辑服务器而言，在断开与公共服务器的连接时，如果有玩家已经传送到公共服务器，需要通知
			SessionServer断开此玩家的会话；对于公共服务器而言，如果与某个普通逻辑服务器断开，那么需要将
			所有此逻辑服务器的玩家踢下线。
	*/
	void OnLogicServerDisconnected(int nServerIdx);

	/*
	* Comments: 接收逻辑服务器之间的传送相关消息
	* Param int nDir: 标记消息方向，0表示是普通逻辑服务器到公共逻辑服务器，反之为1
	* Param int nMsgId: 消息Id
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark:
	*/
	void OnRecvMessage(int nDir, int nMsgId, CDataPacketReader &reader);

	/*
	* Comments: 传送管理器例行处理
	* Param TICKCOUNT nCurrTick:
	* @Return void:
	* @Remark:
	*/
	void RunOne(TICKCOUNT nCurrTick);

protected:	
	/*
	* Comments: 开始传送。
	* Param CActor * pActor:
	* @Return void:
	* @Remark: 发开始传送消息给远程服务器，用于标记传送的开始。消息携带角色Id、ServerIndex等信息。			   
			   目标服务器收到此消息后就创建此角色传送数据缓存对象，并且为此角色设置会话状态，等待客户端连接。
	*/
	void StartTransmitTo(CActor *pActor);

	/*
	* Comments: 源服务器发送给目标服务器的传送完毕通知
	* Param CActor * pActor:
	* @Return void:
	* @Remark: 
	*/
	void EndTransmitTo(CActor *pActor);

	/*
	* Comments: 接收到请求传送消息。
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark: 检测目标场景是否可以传送进入，并且把结果通知给连接逻辑服务器
	*/
	void OnRecvReqTransmitTo(CDataPacketReader &reader);

	/*
	* Comments: 接收到请求传送ACK
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark:
	*/
	void OnRecvReqTransmitToAck(CDataPacketReader &reader);

	/*
	* Comments: 接收到玩家开始传送消息。
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark: 为此玩家创建角色数据缓存对象，设置角色session状态等等
	*/
	void OnRecvStartTransmitTo(CDataPacketReader &reader);

	/*
	* Comments: 接收到传送结束消息
	* Param CDataPacketReader & reader:
	* @Return void:
	* @Remark: 目标服务器收到此消息就开始角色数据的初始化
	*/
	void OnRecvEndTransmitTo(CDataPacketReader &reader);

	/*
	* Comments: 发送消息给连接的逻辑服务器
	* Param void * pData:
	* Param SIZE_T size:
	* Param int nServerIdx: 目标逻辑服务器的ServerIndex。对于公共服务器有意义。
	* @Return void:
	* @Remark:
	*/
	void SendMessage(void *pData, SIZE_T size, int nServerIdx);

	/*
	* Comments: 获取角色所在的原生服务器ServerIndex
	* Param CActor * pActor:
	* @Return int:
	* @Remark:
	*/
	int GetActorRawServerIndex(CActor *pActor);


protected:
	// 最多支持1024个服务器。用于缓存传送到本服务器的玩家的打包数据
	TRANSMIT::CServerActorInfo	m_ServerActorInfo[SERVER_INDEX_MAX];	
	// 对于普通逻辑服务器，还需要记录传送到公共服务器的玩家的信息，包括账号ID、角色ID、玩家上线时间，用于
	// 定时数据存盘的检测以及与公共服务器断开连接时关闭此部分玩家会话用

};

// 普通逻辑服务器传送管理器
class CNSTransmitMgr : public CTransmitMgr
{
public:
	/*
	* Comments: 被普通逻辑服务器调用，用于将当前传送到公共服务器的玩家会话关闭。	
	* @Return void:
	* @Remark:
	*/
	void CloseTransedActorSession();

private:
	CVector<TRANSMIT::CTransedActorRecord>			m_transedActorList;		// 已经传送到公共服务器的玩家列表
};

// 公共服务器的传送管理器
class CCSTransmitMgr : public CTransmitMgr
{
public:
	/*
	* Comments: 被公共逻辑服务器调用，用于将来自指定逻辑服务器的玩家踢下线
	* Param int nServerIdx:
	* @Return void:
	* @Remark: 
	*/
	void KillActorByServerIdx(int nServerIdx);
};

#endif