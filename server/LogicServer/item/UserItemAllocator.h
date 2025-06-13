#pragma once

/************************************************************************/
/*
/*                             角色物品对象申请器
/*
/*   用于在角色获得物品的时候申请一个物品对象，销毁物品的时候释放一个物品对象。
/*
/************************************************************************/

//为了便于追踪物品的申请和是否
struct CItemData:
	public CUserItem
{
	bool    btUsed;   //是否在使用的标记
	LPCSTR  file;    //申请的文件的位置
	INT_PTR line;    //申请的内存的行数
};

//物品来自
struct ItemLogInfo
{
	LPCSTR  file;    //申请的文件的位置
	INT_PTR line;    //申请的物品的行数
	INT_PTR nCount;  //申请物品的数目
	CUserItem* pItem;//申请的其中一个物品
};

class CUserItemAllocator : 
	public CSingleObjectAllocator<CItemData>
{
public:
	typedef CSingleObjectAllocator<CItemData> Inherited;

public:


	inline void CheckItemAllocThread(LPCSTR file, INT_PTR line )
	{
		UINT nCurThreadId= GetCurrentThreadId(); //当前的线程ID
		if(m_itemAllocThreadId != 0)
		{
			if(m_itemAllocThreadId != nCurThreadId)
			{
				//在不同线程操作物品分配，可能引起不可预知的问题
				OutputMsg(rmError,"[ItemTrace],AllocUserItem in different thread,old=[%d],curr=[%d],file=%s,line=%d",m_itemAllocThreadId,nCurThreadId,
					file,line);
				m_itemAllocThreadId = nCurThreadId;
			}
		}
		else
		{
			m_itemAllocThreadId = nCurThreadId;
		}
	}

	
	/* 
	* Comments: 增加物品的申请激励
	* Param LPCSTR file: 文件名
	* Param INT_PTR line: 行数
	* @Return void: 
	*/
	inline void AddItemLog( LPCSTR file, INT_PTR line, CUserItem* pItem)
	{
		for(INT_PTR i=0; i< m_itemLog.count() ;i++)
		{
			if(m_itemLog[i].file == file &&m_itemLog[i].line == line  )
			{
				m_itemLog[i].nCount ++;
				return;
			}
		}
		ItemLogInfo log;
		log.file= file;
		log.line =line;
		log.nCount =1;
		log.pItem = pItem;
		m_itemLog.add(log);
	}

	inline void DecItemLog( LPCSTR file, INT_PTR line)
	{
		for(INT_PTR i=0; i< m_itemLog.count() ;i++)
		{
			if(m_itemLog[i].file == file &&m_itemLog[i].line == line  )
			{
				if(m_itemLog[i].nCount>0)
				{
					m_itemLog[i].nCount --;
				}
				return;
			}
		}
		OutputMsg(rmError,"DecItemLog not found,file=%s,line=%d",file,(int)line);
	}

	inline CUserItem::ItemSeries BuildMailSn()
	{
		CUserItem::ItemSeries series;
		series.t.time	= time(NULL);
		series.t.btServer = (WORD)m_nServerIndex;
		series.t.wSeries = (WORD)m_nMailSeries;
		//单位时间内的系列号值递增
		m_nMailSeries++;
		return series;
	}

	inline CUserItem::ItemSeries BuildNeedBuySn()
	{
		CUserItem::ItemSeries series;
		series.t.time	= m_ItemTime;
		series.t.btServer = (WORD)m_nServerIndex;
		series.t.wSeries = (WORD)m_nNeedBuySeries;
		// series.t.btSpid = (BYTE)(m_nSpId >0x7F? 0x7F:m_nSpId);
		//单位时间内的系列号值递增
		m_nNeedBuySeries++;
		return series;
	}

	inline CUserItem* allocItem(const bool boNewSeries, LPCSTR file, INT_PTR line)
	{
		CheckItemAllocThread(file,line);
		CItemData* pItem = allocObject();

		if(pItem ==NULL)
		{
			OutputMsg(rmError,"alloc Item Fail");
			return NULL;
		}
		if(pItem->btUsed)
		{
			OutputMsg(rmError,"[ItemTrace],allocItem item reused item,oldfile=%s,oldline=%d,itemId=%d,count=%d,curFile=%s,curLine=%d",
				pItem->file?pItem->file:"" ,(int)pItem->line,(int)pItem->wItemId,(int)pItem->wCount,file,(int)line);

			OutputMsg(rmTip,"realloc a new item");
			return allocItem(boNewSeries,file,line);
		}
		
		if(boNewSeries)
		{
			memset(pItem, 0, sizeof(*pItem));
			//产生物品系列号
			pItem->series.t.time	= m_ItemTime;
			pItem->series.t.btServer = (BYTE)m_nServerIndex;
			pItem->series.t.wSeries = (WORD)m_nItemSeries;
			//单位时间内的系列号值递增
			m_nItemSeries++;
		}
		pItem->file = file;
		pItem->line =line;
		pItem->btUsed =true;
		m_nTotalAllocItemCount ++;
		AddItemLog(file,line, (CUserItem*)pItem);
		return (CUserItem*)pItem;
	}

	void DestroyUserItem(CUserItem *pUserItem, LPCSTR file, INT_PTR line)
	{
		CheckItemAllocThread(file,line);
		CItemData *pUserData = (CItemData *)pUserItem;
		if(pUserData->btUsed ==false)
		{
			OutputMsg(rmError,"[ItemTrace],DestroyUserItem item again,oldfile=%s,oldline=%d,itemId=%d,count=%d,curFile=%s,curLine=%d",
				pUserData->file?pUserData->file:"" ,(int)pUserData->line,(int)pUserData->wItemId,(int)pUserData->wCount,file,(int)line);
			return ;
		}
		pUserData->btUsed= false;
		if(m_nTotalAllocItemCount >0)
		{
			m_nTotalAllocItemCount --;
		}
		DecItemLog(pUserData->file,pUserData->line);
		freeObject(pUserData); 
	}


	
	/* 更新物品系列号产生所必须的时间值
	 * 当时间值被更新后，序列值会原为1
	 * ★至少应当在1秒内更新一次时间值！
	 */
	inline VOID UpdateDateTime(const unsigned int time)
	{
		if ( time != m_ItemTime )
		{
			m_ItemTime = time;
			m_nItemSeries = 1;
			m_nMailSeries = 1;
			m_nNeedBuySeries = 1;
		}
	}
	//设置服务器ID，服务器ID将参与物品系列号的生成
	inline VOID SetServerIndex(const INT_PTR nServerIndex)
	{
		m_nServerIndex = nServerIndex;
	}

	//设置运营商的ID
	inline VOID SetSpid(const INT_PTR nSpid)
	{
		m_nSpId = nSpid;
	}

	//获取服务器ID
	inline INT_PTR GetSpid()
	{
		return m_nSpId;
	}


	//获取服务器ID
	inline INT_PTR GetServerIndex()
	{
		return m_nServerIndex;
	}

	CUserItemAllocator()
		:Inherited(_T("CUserItemAllocator"))
		, m_nSpId(0)
	{
		m_nServerIndex = 0;
		UpdateDateTime(CMiniDateTime::now());
		m_itemAllocThreadId =0;
		m_nTotalAllocItemCount =0;
	}

	void TraceItem()
	{
		if(m_nTotalAllocItemCount >0)
		{
			OutputMsg(rmError,"Not free itemcount=%d",(int)m_nTotalAllocItemCount);
			for(INT_PTR i=0; i< m_itemLog.count(); i++)
			{
				if(m_itemLog[i].nCount >0)
				{
					if(m_itemLog[i].pItem)
					{
						OutputMsg(rmError,"Item not free,file=%s,line=%d,count=%d,itemguid:%lld,itemid:%d",m_itemLog[i].file,(int)m_itemLog[i].line,(int)m_itemLog[i].nCount,m_itemLog[i].pItem->series,m_itemLog[i].pItem->wItemId);
					}
					else
					{
						OutputMsg(rmError,"Item not free,file=%s,line=%d,count=%d",m_itemLog[i].file,(int)m_itemLog[i].line,(int)m_itemLog[i].nCount);
					}
				
				}

			}
		}
	}

	~CUserItemAllocator()
	{
		TraceItem();
		m_itemLog.empty();
	}
	
protected:
	CMiniDateTime	m_ItemTime;		//物品系列时间
	INT_PTR			m_nItemSeries;	//物品系列值
	INT_PTR			m_nMailSeries;	//邮件系列值
	INT_PTR			m_nNeedBuySeries;	//求购单系列值
	INT_PTR			m_nServerIndex;	//服务器ID
	INT_PTR         m_nSpId;        //运营商的编号
	UINT            m_itemAllocThreadId;  //线程的id 
	INT_PTR         m_nTotalAllocItemCount; //总共申请的物品的数目
	wylib::container::CBaseList<ItemLogInfo> m_itemLog;

};
