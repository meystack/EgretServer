#pragma once


// 返利活动 领取奖励后保存时间
#define REBATEDATASAVETIME 8 * 24 * 3600

struct RebateData
{
	int nActorId;						// 玩家Id
	int nId;							// 返利序号
	int nTimeType;						// 时间类型
	CMiniDateTime nStartTime;			// 开始时间
	CMiniDateTime nEndTime;				// 结束时间
	CMiniDateTime nClearTime;			// 从数据库中删除的时间(从发放奖励的下一天起，保存一周)
	int nOfflineType;					// 活动类型(1：单日累充；2：多日累充；3：单日累消；4：多日累消)
	int nAtonce;						// 0：次日发放；1：立刻发放奖励
	int nOverlay;						// 是否可叠加(0：不可叠加；1：可叠加)
	int nYBValue;						// 累计充值元宝数 
	int nStatus;						// 领取状态 （0：不可领取；1：可领取；2：已领取）
	RebateData()
	{
		memset(this, 0 ,sizeof(*this));
	}
};

enum YBChangeType //元宝变化类型
{   
	eRebateYBChangeType_MIN,
    eRebateYBChangeType_Recharge = 1,    // 充值 
    eRebateYBChangeType_Consume  = 2,    // 消耗 
    eRebateYBChangeType_MAX, 
};

class CRebateSystem:
	public CEntitySubSystem<enRebateSystemID, CRebateSystem, CActor> 
{
	public:
		typedef CEntitySubSystem<enRebateSystemID, CRebateSystem, CActor> Inherited;

	public:
		CRebateSystem();
		~CRebateSystem();

	public:
		virtual bool Initialize(void *data,SIZE_T size);
		virtual void Destroy();
		virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);
		virtual VOID OnTimeCheck(TICKCOUNT nTickCount);

		//角色各子系统初始化完毕，向客户端发送创建主角的数据包后，调用该接口
		virtual void OnEnterGame();  
	public:
		void LoadFromDB();
		void SaveToDB();
		void OnEnterScene();

		/*
		* Comments:更新 返利系统 数据
		* Param int nYBValue:充值 或 消耗 元宝数
		* Param int nType   :1：充值，2：消耗
		* @Return void:
		*/
		void UpdateRebateData(int nYBValue, int nType);

		// 清理 单日累充 或者 单日消耗 元宝数
		void ClearOneDayRechargeAndOneDayConsumeYBNum();

		// 发放 已结束、次日发放、未发放 的返利活动
		void SendNextDayAward();
		
		// 检查未发放的 次日方法奖励
		void CheckNextDayAward();

		// 玩家登录时，发放未发放的奖励
		void SendAward();
	private:
		// 发放 已结束、次日发放、未发放 的返利活动
		void SendImmediatelyAward();
		//  清理 领取奖励后保存时间 超时的返利活动
		void ClearTimeOutRebates();
	private:

		struct RebateCmpter
		{
			bool operator()(const RebateData* _x, const RebateData* _y) const
			{ return (unsigned int)(_x->nStartTime) == (unsigned int)(_y->nStartTime)? _x < _y : (unsigned int)(_x->nStartTime) < (unsigned int)(_y->nStartTime); }
		};

		std::map<int, RebateData> m_RunningRebates;			// <id,RebateData> 进行中的活动数据
		std::map<int, RebateData> m_WaitingRebates;			// <id,RebateData> 等待中的活动数据
		std::set<RebateData*,RebateCmpter> m_RunningSeq;   // 排序以结束时间小的为优先
    	std::set<RebateData*,RebateCmpter> m_WaitingSeq;   // 排序以开始时间小的为优先

		std::map<int, RebateData> m_SendNextDayAwardRebates;// <id,RebateData> 已结束、次日发放、未发放 的返利活动
		std::map<int, RebateData> m_SendImmediatelyAwardRebates;// <id,RebateData> 已结束、立即发放、未发放 的返利活动
		std::map<int, RebateData> m_ClearTimeOutRebates; // <id,RebateData>领取奖励后保存时间 超时的返利活动
	public:	
	
		typedef std::set<RebateData*,RebateCmpter>::iterator RebateSetIterator;
		typedef std::map<int, RebateData> ::iterator RebateMapIterator;
};
