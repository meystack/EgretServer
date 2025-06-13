
#pragma once

/***************************************************************/
/*  掉落的道具,在场景里，因为需要所有的人都看到，创建一个实体
/*  该实体有过期时间，可以设置归属和存活时间，在存活时间过期以后删除
/*  为了节省内存，一些属性的定义已经重新定义了，比如PROP_ENTITY_DIR，
/*   PROP_ENTITY_MODELID,PROP_ENTITY_ID
/***************************************************************/

class CDropItemEntity:
	public CEntity
{
public:
	typedef CEntity Inherited;

	static int GETTYPE() {return enDropItem;}

	//初始化
	bool Init(void * data, size_t size);

	//删除的一些内存消耗操作
	void Destroy(); //销毁一个实体

	//执行逻辑循环
	virtual VOID LogicRun(TICKCOUNT nCurrentTime);

	inline INT_PTR GetPropertySize() const {return sizeof(CDropItemProperty) -sizeof(CPropertySet);}
	virtual int GetPropertySetTotalSize()const { return sizeof(m_property);}
	//设置掉落物品的物品指针
	void SetItem( CUserItem *pUserItem);

	//获取这个道具的指针
	inline CUserItem * GetItem (){return m_pItem;}

	//设置金钱的数目
	void SetMoneyCount( int nMoneyCount, int nMoneyType=mtCoin);

	//获取金钱类型
	int GetMoneyType() {return m_btMoneyType;}
	
	//获取金钱的数目
	 unsigned int GetMoneyCount();

	 //获取属性集
	 virtual CPropertySet * GetPropertyPtr() {return &m_property ;}

	 virtual  const CPropertySet* GetPropertyPtr() const {return &m_property ;}

	 /*
	 * Comments:设置属于的队伍的id
	 * Param unsigned int nTeamId:队伍的id
	 * @Return void:
	 */
	 void SetTeamId(unsigned int nTeamId); //设置属于的队伍
	 
	 //获取所属的队伍的id
	 inline unsigned int GetTeamId() { return GetProperty<unsigned int>(PROP_ENTITY_DIR); }

	 //设置可以拾取的时间
	 void SetCanPickUpTime(int nTime);

	 //获取一个玩家pActor前来拾取这个道具的错误码，如果是0表示可以拾取，否则返回错误码，比如归属不正确
	 int GetLootErrorCode(CActor * pActor, bool boObserVer = false);
	 
	int GetPetLootErrorCode(CActor * pActor,bool boObserVer = false);
	 
	/*
	* Comments:设置这个道具的归属
	* Param CActor * pActor:玩家的指针
	* @Return void:
	*/
	 void SetMaster(CActor *pActor);


	 /*
	 * Comments:玩家丢物品时，设置主要是谁，防再次拾起
	 * Param CActor * pActor:主人的指针
	 * @Return void:
	 */
	 void SetDropMasterId(CActor * pActor);

	/*
	* Comments:设置一个掉落物品不归属于任何人
	 * @Return void:
	 */
	 void SetDropNoMasterId();

	 //获取这个道具是属于哪个玩家的
	 inline unsigned int GetMasterActorId() { return GetProperty<unsigned int>(PROP_ENTITY_ID);}
 
	 
	 /*
	 * Comments:创建一个掉落的物品
	 * Param CScene * pScene:场景的指针
	 * Param int nPosX:坐标的x
	 * Param int nPosY:坐标的y
	 * Param int nLogId:创建这个掉落的ID
	 * Param LPCTSTR sLogDesc:这个掉落的描述
	 * Param LPCTSTR nExitTime:掉落配置里的存活时间
	 * Param flag : 默认为0 ，1表示启用当前点不掉落
	 * @Return CDropItemEntity *:  返回创建好的实体
	 */
	 static CDropItemEntity * CreateDropItem( CScene *pScene, int nPosX,int nPosY,int nLogId =0,LPCTSTR sLogDesc=NULL,int nExitTime = 0,bool flag = 0 );
	 
	 // 生成1个掉落物品（自动外圈寻找空位）
	 static CDropItemEntity * CreateOne( CScene *pScene, int nPosX, int nPosY,int nLogId = 0,LPCTSTR sLogDesc = NULL,int nExitTime = 0 );
	 
	 //获取过期时间，超过过期时间了就要删除
	 inline TICKCOUNT GetExpireTick (){return m_expireTime;}

	 //获取过期时间，超过过期时间了就要删除
	 inline TICKCOUNT SetExpireTick (TICKCOUNT tk){m_expireTime = tk;}

	 //判断是否前5分钟不能拾取，后面才能拾取
	 TICKCOUNT GetCanPickUpTime() {return m_CanPickUpTime;}

	 /*
	* Comments:设置属于主人的时间
	* Param int nMinSecond:单位毫秒
	* @Return void:
	*/
	void SetMasterOwnerTime(int nMinSecond );

	//处理掉落物品时间
	//type==666 时， 可手动设置捡取时间
	void DealDropItemTimeInfo(CActor * pActor, int type, int pick_time = 60);

public:
	int         m_nLogId;             //掉落的logid
	char        m_sLogDesc[32];       //掉落的描述
	bool        m_showLootTips;     //是否提示客户端拾取
private:
	
	

private:
	CUserItem * m_pItem;  //物品的指针
	
	CDropItemProperty m_property; //属性集合

	TICKCOUNT   m_expireTime;		 //过期的时间，低于这个过期时间，这个箱子就要删除

	TICKCOUNT   m_ownerExpireTime;    //属于他的主人的过期时间

	TICKCOUNT	m_CanPickUpTime;		//能拾取的时间

	BYTE		m_btMoneyType;			//掉落金钱类型
};
