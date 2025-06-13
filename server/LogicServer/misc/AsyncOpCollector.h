#pragma once


enum tagAsyncOPType
{
	asop_none,						// 无效操作
	asop_withdrawYuanBao = 1,		// 提取元宝
	asop_useSACard,					// 使用增值业务卡
	asop_createGuild,				// 创建工会
};

enum tagOPResult
{
	op_Succ,						// 操作成功
	op_Failed,						// 操作失败
	op_Expired,						// 超时
	op_ParamError,					// 参数错误
};

// 增值卡相关信息
typedef struct tagSACardInfo
{		
	UINT64	m_nSN;
	int		m_nType;
	int		m_nSubType;
	int		m_nErrorCode;
	bool	m_bQuery;				// true为查询；false为使用
}SACardInfo;

// 元宝相关信息
struct tagWithdrawYuanBaoInfo : public Counter<tagWithdrawYuanBaoInfo>
{
	int m_nCount;
};

class AsyncOpItem : public Counter<AsyncOpItem>
{
public:
	tagAsyncOPType	m_nType;				// 类型
	bool			m_bActive;				// 用户主动发起的操作
	TICKCOUNT		m_nExpireTime;			// 过期时间(ms)
	void			*m_pData;				// 操作类型对应的数据
	AsyncOpItem() : m_nType(asop_none), m_pData(NULL){}
};

class AsyncOpCollector
{
public:
	AsyncOpCollector() : m_curAsyncOpType(asop_none){}
	void SetActor(CActor *pActor);

	/*
	* Comments: 能否进行某个类型异步操作
	* Param tagAsyncOPType nType: 异步操作类型
	* @Return bool:
	*/
	bool CanAsyncOp(tagAsyncOPType nType);

	/*
	* Comments: 开始一个异步操作
	* Param tagAsyncOPType nType: 异步操作类型
	* Param unsigned int nExpireTime: 超时时间
	* Param void * data: 操作相关的数据
	* Param bool bActive: 是否是主动操作，主动操作同时只能执行一个
	* @Return bool: 如果当前正在进行某个异步操作，并且当前操作是主动操作，返回false。其它情况返回true
	*/
	bool StartAsyncOp(tagAsyncOPType nType, unsigned int nExpireTime, void *data = 0, bool bActive = true);
	
	/*
	* Comments: 通知某个异步操作完成
	* Param tagAsyncOPType nType: 操作类型
	* Param int nResult: 操作结果
	* Param void *pData: 操作相关数据
	* @Return void:
	*/
	void NotifyAsyncOpResult(tagAsyncOPType nType, int nResult, void *pData);

	/*
	* Comments: 更新异步操作，执行超时处理
	* Param TICKCOUNT nCurrentTime: 当前的TickCount
	* @Return void:
	*/
	void Update(TICKCOUNT nCurrentTime);

protected:
	/*
	* Comments: 操作超时
	* Param AsyncOpItem& item: 超时操作数据
	* @Return void:
	*/
	void OnAsyncOpExpired(AsyncOpItem &item);

	void OnResultUseSACard(int nResult, void *pData);

	void OnResultWidthdrawYuanBao(int nResult);
		
	/*
	* Comments:	使用增值卡超时
	* @Return void:
	*/
	void OnUseSACardExpired();

private:
	CActor						*m_pActor;			// 玩家对象
	CVector<AsyncOpItem>		m_asyncOpList;		// 异步操作列表
	tagAsyncOPType				m_curAsyncOpType;	// 当前异步操作类型
};