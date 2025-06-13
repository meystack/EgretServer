#pragma once

//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)
typedef struct tagConsignmentData
{
	CUserItem		item;	//标准的物品结构
	uint64_t        nSellIndex; //交易标识
	// unsigned int    nItemId;    ///道具id
	// unsigned int    nCount;     //道具数量
	unsigned int	nSellTime;	//寄卖的时间
	unsigned int	nPrice;		//价钱
	unsigned int	nActorId;	//所有者的id
	BYTE			nState;		//状态: 0正常寄卖 1过期
	int             nAutoId;//自动id
	// WORD        	wStar;	//升星
	// char            nBestAttr[200];  //极品属性
	//ACTORNAME	szName;		//所有者名字

	//BYTE		moneyType;	//元宝或者银两
	//BYTE		bJob;		//适用的职业，0是通用，见职业定义
	//BYTE		bType;		//类型，
	//BYTE		bLevel;		//物品使用所需的等级
	//BYTE		bTimeOut;	//是否过期，0表示没过期，否则表示过期
	tagConsignmentData()
	{
		ZeroMemory(this,sizeof(*this));
	}
}ConsignmentData;

//收益
typedef struct tagConsignmentIncome
{
	unsigned int nActorId;			//玩家id
	unsigned int nIncome;			//收入
	tagConsignmentIncome()
	{
		ZeroMemory(this,sizeof(*this));
	}
}ConsignmentIncome;


#pragma pack(pop)

enum ConsignmentDataState
{
	eConsignmentOnShelf = 0,	//正常寄卖
	eConsignmentOffShelf = 1,	//下架
	eConsigmentActorBuy  = 2,   //已经被购买了
	eConsigmentIsSell    = 3,   //已经出售
};
enum ConsignBuyResult
{
	eConsignBuyCountPriceErr = 0,//不关购买窗口刷新价格数量
	eConsignBuyOk = 1,//成功
	eConsignBuyCloseWnd = 2,//关闭窗口(物品不存在)

};


class ConsignmentSystem:
	public CEntitySubSystem<enConsignmentSystemID,ConsignmentSystem,CActor>
{
public:
	ConsignmentSystem();
	~ConsignmentSystem();
public:

	virtual bool Initialize(void *data,SIZE_T size);

	//数据返回
	VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );
	/*
	* Comments:处理客户端的数据包
	* Param INT_PTR nCmd:消息id
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );
	void OnEnterGame();
public:
	//领取寄卖收益
	void GetConsignIncome(CDataPacketReader &packet);
	//搜索物品
	void SendSearchConsignItem(CDataPacketReader &packet);
	//寄卖物品
	void AddSellItem(CDataPacketReader &packet);
	//购买物品
	void BuyItem(uint64_t nIndex, int nCount, unsigned int nBuyPrice);
	//下发购买物品结果
	void SendBuyItemResult();
	//取消寄卖/取回过期物品
	void CancelSellItem(CDataPacketReader &packet);
	//获取自己的寄卖物品
	//itemGuid:寄卖物品的Guid
	ConsignmentData* GetSellItem(uint64_t nIndex);
	//下发自己的寄卖物品
	void SendMyConsignItem();

	void SendAllConsignItem();
	//吆喝寄卖物品
	void ShoutingItem(CUserItem::ItemSeries itemGuid);
	//
	void SendConsignIncome();

	//红点提示
	void SenDClientRedPoint(bool bRedPoint);

	bool returnConsignProto(CDataPacket & dataPacket, ConsignmentData* pData, uint32_t nActorId);
public:
};
