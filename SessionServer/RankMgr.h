#pragma  once


//排行榜的信息，用于做全服的排行榜,只取最nb的多少名


//排行的依据，根据 nValue
struct CRankItem
{
	unsigned int nActorid;     //角色的actorid
	char         sCharName[32];//角色的名字
	int          nKey;         //关键字，其中1表示战士排行榜，2表示法师排行榜，3表示道士排行榜,其他类似
	
	int          nServerId;    //服务器的id

	int          nValue;       //关键字的值，表示排行榜用于排行依据的值
	unsigned long long			lAuxValue1; //辅助的依据1
	unsigned long long          lAuxValue2; //排行的依据2，用于排行的辅助，比如根据等级

	int          nProp1;        //附加属性1
	int          nProp2;        //附加属性2
	int          nProp3;        //附加属性3
	CRankItem()
	{
		memset(this,0,sizeof(*this)); //初始化设置0
	}
};

class CRankMgr
{

public:
	CRankMgr()
	{
		m_rankItems.setLock(&m_itemListLock);

	}

	~CRankMgr()
	{
		m_rankItems.empty();
	}

	/*
	* Comments:  增加1个排行榜的元素
	* Parameter: CRankItem & rankItem:排行榜元素
	* Parameter: int nRankItemCount:排完排行榜你取多少名
	* @Return  bool:成功返回true,否则返回false
	*/
	bool AddRankItem(CRankItem &rankItem,int nRankItemCount=1);

	//获取排行榜的元素列表
	const void GetRankList(int nKey,wylib::container::CBaseList<CRankItem> & itemList)
	{
		itemList.clear();
		for(INT_PTR i=0; i< m_rankItems.count(); i++)
		{
			if(m_rankItems[i].nKey == nKey || nKey <0)
			{
				itemList.add(m_rankItems[i]);
			}
		}
	}

private:

	/*
	* Comments:  获取某个key的数目
	* Parameter: int nKeyId:key的ID
	* Parameter: unsigned int nActorid:玩家的actorid
	* @Return  int:该玩家在这个排行榜的位置
	*/
	int GetActorPos(int nKeyId,unsigned int nActorid);

private:
	wylib::container::CLockList<CRankItem>		m_rankItems;		 //排行的列表
	wylib::container::CCSLock					m_itemListLock;	 //排行的列表锁
};