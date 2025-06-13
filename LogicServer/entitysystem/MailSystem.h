

#pragma  once
//邮件保存分批保存
#define MAIL_DATA_SAVE_COUNT_EACH_TIME  30//每次保存的邮件数
#define MAIL_DATA_SEND_COUNT_EACH_TIME  40//每次下发的邮件数
//邮件保存类型
enum eMailSaveType
{
	eMailSaveType_Mail=0,
	eMailSaveType_Attach =1
};
enum eMailSaveDelType
{
	eMailSaveDelType_Normal=0,
	eMailSaveDelType_Del=1
};
#define MAILATTACHCOUNT 6
class CMailSystem:
	public CEntitySubSystem<enMailSystemID, CMailSystem, CActor>
{
public:
	//邮件附件
	enum eMailState
	{
		eMailState_No =0,
		eMailState_Read =1,//已读
		eMailState_Get=2//已领取
	};
	enum eMailSource
	{
		eMailSource_System=0,//游戏正常功能发送
		eMailSource_BackStageWelFare=1,//后台福利发送
	};
	//分批下发到客户端
	enum eMailBatchSendType
	{
		eMailBatchSendType_DelOld =0,//删除旧的并开始添加
		eMailBatchSendType_Add =1//直接添加
	};
	//附件说明：
	//建议直接使用脚本函数发邮件，直接配置Award即可
	//SendMail,SendMailByAward接口,设置item里的参数，然后giveaward生成奖励。
	//SendMailFromUserItem接口，直接拷贝到item，发整个具体物品。
	//当nType>0，nResourceCount用来存资源数量,保存时暂用(wDura+wDuraMax)来存。
	//当nType == qatAddExp, 不使用nResourceCount 而用id 和count
	typedef struct tagMailAttach
	{
		byte nType;
		CUserItem item;
		//下面的不存数据库
		int nResourceCount;			
		WORD	wQualityDataIndex;	//用来标记发邮件前是否生成极品属性(0不生成);

		tagMailAttach & operator = (const tagMailAttach & oneAttrach)
		{
			memcpy(this, &oneAttrach, sizeof(*this));
		}
		tagMailAttach()
		{
			memset(this, 0, sizeof(*this));
		}
	}MAILATTACH, *PMAILATTACH;

	typedef struct tagMailInfo
	{
		// 暂时废弃这个id ---客户端跟服务器对不上
		// CUserItem::ItemSeries nMailId; //


		uint64_t nMailId; //邮件id
		unsigned int nSrcId;//来源
		char sTitle[200];
		char sContent[600];
		MAILATTACH mailAttach[MAILATTACHCOUNT];
		unsigned int nCreateDt;
		byte nState;
		byte nBind;
		byte nIsDel;//是否删除了,1删除，0否（现在删除了存数据库供后台查询）

		//下面的不存数据库
		byte nAttachCount;//未领取附件数量
		bool isSendToClient;//是否已经发送到客户端
		tagMailInfo()
		{
			memset(this, 0, sizeof(*this));
		}
	}MAILINFO;

	enum enMailOpt
	{
		moReadFlag = 1,
	};
	CMailSystem();
	~CMailSystem();

	bool Initialize(void *data,SIZE_T size);

	void OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	void ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	void Save(PACTORDBDATA pData);
	void SaveMailToDb(std::vector<MAILINFO>& mailList, BYTE nDelType);

	void LoadMailDataFromDb();

	void SendAllMail();
	static void SetUseItemToMailAttach(CUserItem& userItem, MAILATTACH& mailAttach);
	static void SetMailAttachToUseItem(CUserItem& userItem, MAILATTACH& mailAttach);
	void AddOneMailToPacket(CDataPacket& pack, MAILINFO& oneMail);

	void AddAttachToItemParam(MAILATTACH& attach, CUserItemContainer::ItemOPParam& itemPara);
	/*
	* Comments:邮件操作
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void MailOpt(CDataPacketReader & packet );


	bool isSameIdMail(MAILINFO& mailInfo);


	/* 
	* Comments:发送邮件的静态函数
	* Param unsigned int nActorId:发送的玩家IDtpSkillTrainCoinLimited
	* Param LPCSTR sTitle:邮件标题
	* Param LPCSTR sContent:邮件内容
	* Param MAILATTACH pAttach[MAILATTACHCOUNT]: 附件
	* @Return bool:
	*/
	//发邮件,可以不带附件
	static bool SendMail( unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent ,MAILATTACH pAttach[MAILATTACHCOUNT] = NULL, int nSrcId = eMailSource_System);
	//发邮件，用奖励配置生成物品(资源用nResourceCount)
	static void SendMailByAward(unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent,CVector<ACTORAWARD>& awardList, int nSrcId = eMailSource_System);
	//发邮件，直接发useritem,不重新生成物品,注意管理pUserItem内存
	//暂没需求发多个CUserItem，有就自行在现有接口上扩充
	static bool SendMailFromUserItem( unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent, CUserItem* pUserItem);
	//添加附近
	static void AddMailAttach(MAILATTACH& attach, int nType, int nId, int nQuality, int nStrong, bool bind, int smith[CUserItem::MaxSmithAttrCount]);
	/* 
	* Comments:保存离线邮件
	* Param MAILINFO & mail:邮件结构引用
	* @Return void:
	*/
	static void SaveOfflineMail(unsigned int nActorId, MAILINFO & mail);

	//给客户端下发一封新的邮件
	void SendNewMail(MAILINFO& oneMail);
	void DeleteMails(CDataPacketReader & packet);
	void DeleteMailsAll();

	void DeleteSomeMails(CDataPacketReader & packet);
	
	/*
	* Comments:批量收取邮件中的附近
	* Param CDataPacketReader & packet:
	* @Return void:
	*/
	void GetMailItems(CDataPacketReader & packet);
	void GetMailItemsAll();

	/*
	* Comments:标记已读邮件
	* Param INT64 nMailId:
	* @Return void:
	*/
	void SetMailReaded(INT64 nMailId);
	
	/*
	* Comments:删除邮件
	* Param INT64 nMailId:
	* @Return bool:
	*/
	bool RemoveMail(INT64 nMailId);

	/*
	* Comments:通过邮件ID， 获取邮件数据
	* Param INT64 nMailId:
	* @Return MAILINFO *:
	*/
	inline MAILINFO * GetMail(INT64 nMailId)
	{
		INT_PTR nCount = m_mailList.size();
		for (INT_PTR i = 0; i < nCount; i++)
		{
			MAILINFO & mail = m_mailList[i];
			if (mail.nMailId == nMailId)
			{
				return &mail;
			}
		}
		return NULL;
	}

	//加载邮件内容
	void OnLoadMails(CDataPacketReader & packet);
	INT_PTR GetMailCount(){return m_mailList.size();}
private:
		/*
	* Comments:发送到我角色的邮件
	* Param MAILINFO & oneMail:
	* @Return bool:
	*/
	bool SendMail(MAILINFO & oneMail);
	//提取一封邮件的附件
	void GetOneMailAttach(MAILINFO& oneMail);
	//日志记录获取附件
	void LogGetMailAttach(MAILINFO& mailInfo);
	void LogDelMail(MAILINFO& mailInfo);
private:
	std::vector<MAILINFO> m_mailList;
	std::vector<MAILINFO> m_mailDelList;//已删除邮件列表
	uint32_t          m_nMailIndex = 0; //邮件初始index
	bool              nInitOnce = false;
//跨服相关
public:
	//发邮件，用奖励配置生成物品(资源用nResourceCount)
	static void SendCrossServerMail(unsigned int nActorId, LPCSTR sTitle, LPCSTR sContent,std::vector<ACTORAWARD>& awardList, int nSrcId = eMailSource_System);
};