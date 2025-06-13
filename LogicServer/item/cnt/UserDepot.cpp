#include "StdAfx.h"
#include "UserDepot.h"
#include "DepotBag.h"
#include "../../entitysystem/MiscSystem.h"
#include "FvMask.h"

using namespace jxInterSrvComm;

template<>
const CUserDepot::InheritedSybSystem::OnHandleSockPacket CUserDepot::InheritedSybSystem::Handlers[]=
{
	&CUserDepot::HandError,    //错误处理
	&CUserDepot::HandGetItems, //获取仓库的物品列表
	&CUserDepot::HandItemBag2Deport, //把一个物品从背包拖放到仓库
	&CUserDepot::HandItemDepot2Bag, //把一个物品从仓库拖放到背包
	&CUserDepot::HandRemoveItem, //删除仓库物品
	&CUserDepot::HandDepotBuy,		//购买仓库页
	&CUserDepot::HandArrangeItemList,		//整理
};

VOID CUserDepot::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;

	if(nCmd >=1 && nCmd < cMaxDepotCodeID)
	{
		(this->*Handlers[nCmd] )(packet);
	}
}

void CUserDepot::HandRemoveItem(CDataPacketReader &packet)
{
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}
	CUserItem::ItemSeries series ;
	packet >> series.llId;
	CUserItem *pUserItem =  m_depotBag.FindItemByGuid(series);
	if(pUserItem ==NULL){
		m_pEntity->SendTipmsgFormatWithId(tmItemNotExist, tstUI);
		return ;
	}
	m_depotBag.DeleteItem(pUserItem,pUserItem->wCount,"deport_remove_item",GameLog::Log_DeportRemoveItem,true);
}

void CUserDepot::SendClientDepotData()
{
	CActorPacket pack;
	CDataPacket &data =  ((CActor *)m_pEntity)->AllocPacket(pack);
	INT_PTR nCount =m_depotBag.count();
	data <<(BYTE)enDepotSystemID << (BYTE)cDeportGetItems <<   (int) nCount;
	// CUserItem ** pItem =m_depotBag.operator CUserItem**();
	for(INT_PTR i=0; i< nCount; i++)
	{
		if(m_depotBag.m_pUserItemList[i])
		{
			(CUserItem) *m_depotBag.m_pUserItemList[i] >> data;
		}
	}
	pack.flush();
}

bool CUserDepot::CanOpenDepot(CActor *pEntity)
{
	if(!pEntity) 
		return false;
	unsigned int val = pEntity->GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
	
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_Violet)))
	{ 
		return true;//紫卡
	}
	if (FvMask::HasAny(val, _MASK_(eMembershipLv_White)))
	{
		int nSceneId = pEntity->GetSceneID();  
		if (nSceneId == 3)
		{
			return true; //在盟重
		} 
	}
	
	OutputMsg(rmWaning, "[WG] checkDepot 玩家 = %d Actorname=%s, errer!", ((CActor*)m_pEntity)->GetId(), m_pEntity->GetEntityName());

	return false;
}

//获取仓库的物品列表
//改为用到的时候再去db服务器请求
void CUserDepot::HandGetItems(CDataPacketReader &packet)
{
	if(!m_pEntity) 
		return;
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}
	
	WarehouseCfg& cfg = GetLogicServer()->GetDataProvider()->GetStdItemProvider().m_WarehouseCfg;
	if(!m_pEntity->CheckLevel(cfg.nOpenLevel, 0))
	{
		m_pEntity->SendTipmsgFormatWithId(tmdepotLvLimit, tstUI);
		return;
	}
	if(((CActor*)m_pEntity)->GetMiscSystem().GetMaxColorCardLevel() < cfg.nOpenCardLv)
	{
		m_pEntity->SendTipmsgFormatWithId(tmdepotCardLvLimit, tstUI);
		return;
	}
	
	SendClientDepotData();
}


void CUserDepot::HandArrangeItemList(CDataPacketReader & packet)
{
	if (!m_pEntity || m_pEntity->GetType() != enActor)
	{
		return;
	}
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}
	m_depotBag.ArrangeItemList(GameLog::Log_DepotMerge);
	CActorPacket pack;
	CDataPacket &data= ((CActor *)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enDepotSystemID << (BYTE)sDeportArrangeBagOver;
	pack.flush();
}

//把一个物品从背包拖放到仓库
void CUserDepot::HandItemBag2Deport(CDataPacketReader &packet)
{ 
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}	

	if(HasDbDataInit() ==false ) return; //如果db的数据还没装载之前，不能执行
	if(!m_pEntity)  return;
	CUserItem::ItemSeries series ;
	packet >> series.llId;

	CUserBag& bag= m_pEntity->GetBagSystem();
	//从角色的包裹中查找要穿戴的物品

	CUserItem *pUserItem =  bag.FindItemByGuid(series);
	if(pUserItem ==NULL){
		m_pEntity->SendTipmsgFormatWithId(tmItemNotExist, tstUI);
		return ;
	}

	
	if( m_pEntity->GetDealSystem().IsDealing() ) //在交易的时候不能放仓库
	{
		m_pEntity->SendTipmsgFormatWithId(tmFailForDealing,tstUI ); //
		return;
	}

	//这里要处理仓库是否已经满的情况
	if(m_depotBag.availableCount() <= 0)
	{
		m_pEntity->SendTipmsgFormatWithId(tmdepotBagIsFull, tstUI); //仓库满了
		return; 
	}

	const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
	if(pStdItem ==NULL) return;
	if(pStdItem->m_Flags.denyStorage)
	{
		m_pEntity->SendTipmsgFormatWithId(tmItemCanNotPutDepot,tstUI); //禁止放仓库
		return ;
	}
	pUserItem->btDeportId = (BYTE)0; //记录一下是哪个仓库里的
	pUserItem->btSharp = 0;	//放到仓库先把锋利值清空
	if (bag.RemoveItem(series,"bag_2_deport",GameLog::Log_Bag2Deport,false))
	{
		if( m_depotBag.AddItem2depot(pUserItem,"bag_2_deport",GameLog::Log_Bag2Deport,true) > 0 )	//仓库添加成功，重新设置背包中的经验盒子
		{
			// bag.AssignExpItem();		//指定一个经验盒子作为当前吸收经验的盒子
			m_pEntity->SendTipmsgFormatWithId(tmBag2Deport, tstUI,(char*)(pStdItem->m_sName)); //仓库满了
		}
		else
		{
			OutputMsg(rmWaning,_T("ItemBag2Deport:name(%s)itemguid(%lld)"),(char*)m_pEntity->GetEntityName(),pUserItem->series.llId);
		}
	}

}

bool CUserDepot::HandItemDepot2BagLogic(CUserItem::ItemSeries series)	//根据index从仓库取一个物品
{
	CUserBag& bag= m_pEntity->GetBagSystem();
	CUserItem *pUserItem = m_depotBag.FindItemByGuid(series);
	
	if(pUserItem ==NULL) 
		return false;
	if(! bag.CanAddItem(pUserItem) )
	{
		m_pEntity->SendTipmsgFormatWithId(tmBagIsFull,tstUI); //背包已经满
		return false;
	}

	pUserItem->btDeportId = (BYTE)0;
	if (m_depotBag.RemoveItem(series,"deport_2_bag",GameLog::Log_Deport2Bag,false))
	{
		if(bag.AddItem(pUserItem,"deport_2_bag",GameLog::Log_Deport2Bag,true) > 0)
		{
			CMiniDateTime tc = GetLogicServer()->GetLogicEngine()->getMiniDateTime();
			const CStdItem * pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(pUserItem->wItemId);
			if(pStdItem)
			{
				m_pEntity->SendTipmsgFormatWithId(tmDeport2Bag, tstUI,(char*)(pStdItem->m_sName)); //仓库满了
			}
			
		}
		else
		{
			OutputMsg(rmWaning,_T("ItemDepot2Bag:name(%s)itemguid(%lld)"),(char*)m_pEntity->GetEntityName(),pUserItem->series.llId);
		}
	}
	return true;
}

 //把一个物品从仓库拖放到背包
void CUserDepot::HandItemDepot2Bag(CDataPacketReader &packet)
{
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}
	if(HasDbDataInit() ==false ) return; //如果db的数据还没装载之前，不能执行
	CUserItem::ItemSeries series ;
	packet >> series.llId;
	HandItemDepot2BagLogic(series);
}

bool CUserDepot::Initialize(void *data,SIZE_T size)
{
	if ( !InheritedSybSystem::Initialize(data, size) )
	{
		OutputMsg(rmError,_T("bag InheritedSybSystem init fail"));
		return false;
	}
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("m_pEntity ==NULL"));
		return false;
	}
	
	PACTORDBDATA  pActorData = (ACTORDBDATA *)data;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA)) 
	{
		OutputMsg(rmError,_T("data len error len=%d ,correct len=%d"),size, sizeof(ACTORDBDATA));
		return false;
	}
	else
	{
		int nGridNum = ((CActor *)m_pEntity)->GetProperty<int>(PROP_ACTOR_DEPORT_GRID_COUNT);
		if (nGridNum > 0 )
		{
			m_depotBag.setCapacity (nGridNum ); //设定格子的数量
		}
		m_depotBag.m_pEntity = m_pEntity;
	}
	
	return true;
}


void CUserDepot::Save(PACTORDBDATA  pData)
{
	if(m_pEntity ==NULL) return;

	
	if(HasDbDataInit() ==false) return;  //如果没有装载过仓库的数据就不需要存储
	
	if(!m_depotBag.IsDataModified()) return; //数据没有修改
	
	CDataPacket& dataPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveDepotItem);
	dataPacket << m_pEntity->GetRawServerIndex() << GetLogicServer()->GetServerIndex();
	dataPacket <<(unsigned int ) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家ID

	size_t totalCount = m_depotBag.Inherited::count();

	dataPacket  << (BYTE )itItemDepot << (int)totalCount  ; //玩家物品的数量
		
	for (INT_PTR j=0; j< m_depotBag.Inherited::count(); j ++ )
	{
		dataPacket <<(CUserItem) *m_depotBag.m_pUserItemList[j]; //把一个物品打包过去
	}

	GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
	m_depotBag.SetDataModifyFlag(false);
	
}
//db返回
VOID CUserDepot::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(m_pEntity ==NULL) return;
	//CUserItem data;
	//装载物品数据
	
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadDepotItem &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回物品的列表
	{
		if(HasDbDataInit()) return; //已经初始化一次了
		OnDbInitData();

		int nDeportInitGridNum = ((CActor *)m_pEntity)->GetProperty<int>(PROP_ACTOR_DEPORT_GRID_COUNT); //仓库格子
		m_depotBag.setCapacity (nDeportInitGridNum ); //设定格子的数量

		int nCount =0;
		reader >> nCount;

		CUserItem userItem;
		CLogicEngine *pEngineer = GetLogicServer()->GetLogicEngine();

		for (INT_PTR i=0 ; i < nCount; i++ )
		{
			userItem << reader;
			CUserItem *pUerItem = pEngineer->CopyAllocUserItem(false,&userItem); //不申请序列号	
			
			if(pUerItem)
			{
				//只有没有装过DB的数据才能装载，防止装载数据重复复制数据
				if(m_depotBag.HasInitDbData() ==false)
				{
					m_depotBag.AddItem2depot(pUerItem,m_pEntity->GetEntityName(),0,false); //不需要记录日志的，不需要通知客户端
				}
			}
		}
		//调用各背包的接口，已经初始化db的数据完毕
		m_depotBag.OnInitDbData();
	}
}


INT_PTR CUserDepot::DeleteItemByGuid( CUserItem::ItemSeries guid, LPCTSTR sLogStr, int nLogId )
{
	for (INT_PTR j=0; j< m_depotBag.Inherited::count(); j ++ )
	{
		if (m_depotBag.m_pUserItemList[j] && (m_depotBag.m_pUserItemList[j]->series == guid))
		{
			return m_depotBag.DeleteItem(m_depotBag.m_pUserItemList[j], m_depotBag.m_pUserItemList[j]->wCount, sLogStr, nLogId);
		}
	}
	return 0;
}

void CUserDepot::AddDepot(WORD nIdx)					//增加仓库格子，成就特权使
{
	if(nIdx <= 0)
	{
		return;
	}
	((CActor *)m_pEntity)->SetProperty<int>(PROP_ACTOR_DEPORT_GRID_COUNT, nIdx); //仓库格子
	m_depotBag.setCapacity(nIdx); //更改格子的数量
}

void CUserDepot::HandDepotBuy( CDataPacketReader & packet )
{
	if (!CanOpenDepot(m_pEntity))
	{
		return;
	}
	byte index = 0;
	packet >> index;
	if(index <= 0)
	{
		return;
	}
	BAGCONFIG &config = GetLogicServer()->GetDataProvider()->GetBagConfig();

	WORD openCount = (WORD)m_depotBag.Inherited::capacity();
	WORD newOpenCount = openCount + index;
	if(newOpenCount > 1)//config.nDeportPageNum * config.nDeportPageTotalGridNum )
	{
		m_pEntity->SendOldTipmsgWithId(tpDepotGridNumLimit, ttFlyTip); //没有那么多格子可以开
		return;
	}

	int nYB = index;//config.nDeportGridPrice * index;		//第一个免费，数组下标0开始，所以减2
	int nMyYb = ((CActor *)m_pEntity)->GetMoneyCount(eMoneyType(mtBindYuanbao), true);
	if(nMyYb < nYB)
	{
		((CActor*)m_pEntity)->SendMoneyNotEnough((BYTE)mtYuanbao, nYB);
		return;
	}

	((CActor *)m_pEntity)->SetProperty<int>(PROP_ACTOR_DEPORT_GRID_COUNT, newOpenCount); //仓库格子

	m_depotBag.setCapacity(newOpenCount); //更改格子的数量

	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpLogDepotBugLog);
	((CActor*)m_pEntity)->DeleteBindYuanbaoAndYuanbao(nYB,GameLog::clEnlargeDepot,0, sFormat);
}

int CUserDepot::GetDepotItemCount( int nItemId )
{
	return (int)m_depotBag.GetItemCount(nItemId);
}
