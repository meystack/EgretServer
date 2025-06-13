

// #include "stdafx.h"

// using namespace wylib::stream;

// CItemConfigLoader::CItemConfigLoader()
// {

// }

// CItemConfigLoader::~CItemConfigLoader()
// {

// }

// bool CItemConfigLoader::LoadFile(LPCTSTR sFilePath)
// {
// 	bool bRet		= false;
// 	LPCTSTR sResult = NULL;
// 	CMemoryStream ms;
// 	CCustomLuaPreProcessor pp;
// 	try
// 	{
// 		GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);
// 		if (ms.loadFromFile(sFilePath) <= 0)
// 		{
// 			showErrorFormat(_T("unable to load config file from %s"), sFilePath);
// 			return false;
// 		}

// 		sResult = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
// 		if (!sResult)
// 		{
// 			OutputMsg(rmError, _T("预处理文件%s失败"), sFilePath);
// 			return false;
// 		}
// 		if (!setScript(sResult))
// 		{
// 			OutputMsg(rmError,_T("读取预处理后的配置文件%s失败"), sFilePath);
// 			return false;
// 		}
// 		bRet = ReadItemConfig();
// 	}
// 	catch (RefString& s)
// 	{
// 		OutputMsg(rmError, (LPCTSTR)s);
// 	}
// 	catch (...)
// 	{
// 		OutputMsg(rmError, _T("unexcepted error on load config: %s"), sFilePath);
// 	}
// 	setScript(NULL);

// 	return bRet;
// }

// bool CItemConfigLoader::ReadItemConfig()
// {


// 	if (!openGlobalTable("StdItems"))
// 		return false;
	
// 	//获取配置中的物品数量
// 	const INT_PTR nItemCount = lua_objlen(m_pLua, -1) + 1;
// 	//如果物品数量为0则输出未配置任何物品的错误
// 	if (nItemCount <= 0)
// 		showError(_T("no item data on StdItem config"));
// 	//申请出物品数据并将物品数据内存块清空
// 	m_itemList.reserve(nItemCount);
// 	m_itemList.trunc(nItemCount);
// 	CStdItem *pStdItems = m_itemList;
// 	ZeroMemory(pStdItems, sizeof(CStdItem) * nItemCount);

// 	//循环读取物品配置数据
// 	if (enumTableFirst()) {
// 		INT_PTR nIdx = 1;

// 		do {
// 			//读取物品配置数据
// 			CStdItem &item = pStdItems[nIdx];
// 			readItemData(item);
// 			//如果该物品的ID不是期望的值，则输出错误并终止读取
// 			if (item.m_nIndex != nIdx) {
// 				showErrorFormat(_T("unexpected itemindex %d, request %d"),
// 					item.m_nIndex, nIdx);
// 				endTableEnum();
// 				break;
// 			}
// 			nIdx++;
// 		} while (enumTableNext());

		

// 	}

// 	closeTable();
// 	return true;
// }
// bool CItemConfigLoader::readItemData(CStdItem &item)
// {
// 	int nDefVal = 0;

// 	item.m_nIndex = getFieldInt("id");
// 	getFieldStringBuffer("name", item.m_sName, ArrayCount(item.m_sName));
// 	item.m_btType = (BYTE) getFieldInt("type");
// 	item.m_wIcon = (WORD) getFieldInt("icon");
// 	item.m_wShape = (WORD) getFieldInt("shape");
// 	item.m_dwDura = (UINT) getFieldNumber("dura");
// 	double nDwDefVal = 0;
// 	item.m_dwUseDurDrop = (UINT) getFieldNumber("useDurDrop", &nDwDefVal);
// 	item.m_nCDTime = getFieldInt("cdTime", &nDefVal);
// 	item.m_wDupCount = (WORD) getFieldInt("dup");
// 	item.m_btColGroup = (BYTE) getFieldInt("colGroup");
// 	item.m_btDealType = (BYTE) getFieldInt("dealType");
// 	item.m_nPrice = getFieldInt("dealPrice");
// 	item.m_UseTime = getFieldInt("time");
// 	item.m_btSmithId = (BYTE) getFieldInt("smithId", &nDefVal);
// 	item.b_btBreakId = (BYTE) getFieldInt("breakId", &nDefVal);
// 	item.m_wSuitID = getFieldInt("suitId", &nDefVal); //套装的ID
// 	item.b_showQuality = getFieldInt("showQuality", &nDefVal); //显示的品质
// 	item.m_nDropBroadcast = getFieldInt("dropBroadcast", &nDefVal);
// 	item.w_candidateIconCount = (WORD) getFieldInt("candidateIconCount",
// 		&nDefVal); //获取图标
// 	item.b_specRing = (BYTE) getFieldInt("specRing", &nDefVal);
// 	item.m_btBatchType = (BYTE) getFieldInt("batchType", &nDefVal);
// 	item.m_nBatchValue = getFieldInt("batchValue", &nDefVal);
// 	item.m_nSillId = getFieldInt("skillID2", &nDefVal);
// 	item.m_nConsignType = getFieldInt("consignType", &nDefVal);
// 	item.m_DropCount = 0;

// 	nDefVal = -1;
// 	item.m_nValidFbId = getFieldInt("validFbId", &nDefVal);	// 物品生效的副本ID。默认为-1
// 	item.m_nValidSceneId = getFieldInt("validSceneId", &nDefVal);// 物品生效的场景ID。默认为-1

// 	// 读取物品可存在的场景集合
// 	item.m_existScenes.nCount = 0;
// 	if (feildTableExists("existScenes") && openFieldTable("existScenes"))
// 	{
// 		INT_PTR count = lua_objlen(m_pLua, -1);
// 		item.m_existScenes.nCount = count;
// 		item.m_existScenes.pSceneId = (int *) m_DataAllocator.allocObjects(
// 			count * sizeof(*(item.m_existScenes.pSceneId)));
// 		INT_PTR index = 0;
// 		if (count > 0 && enumTableFirst()) {
// 			do {
// 				int nSceneId = getFieldInt(NULL);
// 				if (index < count) {
// 					item.m_existScenes.pSceneId[index] = nSceneId;
// 					index++;
// 					if (nSceneId == -1)				// -1表示不限制场景
// 					{
// 						endTableEnum();
// 						break;
// 					}
// 				} else {
// 					endTableEnum();
// 					break;
// 				}
// 			} while (enumTableNext());
// 		}
// 		closeTable();
// 	} else {
// 		// 默认不配置表示所有场景都能用
// 		item.m_existScenes.nCount = 1;
// 		item.m_existScenes.pSceneId = (int *) m_DataAllocator.allocObjects(
// 			item.m_existScenes.nCount
// 			* sizeof(*(item.m_existScenes.pSceneId)));
// 		item.m_existScenes.pSceneId[0] = -1;
// 	}

// 	//读取物品静态属性
// 	//if (feildTableExists("staitcAttrs") && openFieldTable("staitcAttrs")) {
// 	//	readItemAttributeTable(dataAllocator, item.m_StaticAttrs);
// 	//	closeTable();
// 	//}
// 	////读取物品品质属性
// 	//if (feildTableExists("qualityAttrs") && openFieldTable("qualityAttrs")) {
// 	//	if (enumTableFirst()) {
// 	//		nLevel = 0;
// 	//		do {
// 	//			readItemAttributeTable(dataAllocator,
// 	//				item.m_QualityAttrs[nLevel]);
// 	//			nLevel++;
// 	//			if (nLevel >= CStdItem::MaxItemQuality) {
// 	//				endTableEnum();
// 	//				break;
// 	//			}
// 	//		} while (enumTableNext());
// 	//	}
// 	//	closeTable();
// 	//}
// 	////读取物品强化属性
// 	//if (feildTableExists("strongAttrs") && openFieldTable("strongAttrs")) {
// 	//	if (enumTableFirst()) {
// 	//		nLevel = 0;
// 	//		do {
// 	//			readItemAttributeTable(dataAllocator,
// 	//				item.m_StrongAttrs[nLevel]);
// 	//			nLevel++;
// 	//			if (nLevel >= CStdItem::MaxItemStrong) {
// 	//				endTableEnum();
// 	//				break;
// 	//			}
// 	//		} while (enumTableNext());
// 	//		item.m_StrongCount = (BYTE) nLevel;
// 	//	}
// 	//	closeTable();
// 	//}


// 	////读取物品标志属性
// 	//if (feildTableExists("flags") && openFieldTable("flags")) {
// 	//	readItemFlags(item);
// 	//	closeTable();
// 	//}
// 	////读取物品使用条件表
// 	if (feildTableExists("conds") && openFieldTable("conds")) {
// 		readItemConditionTable(item.m_Conditions);
// 		closeTable();
// 	}
// 	////读取物品预留配置
// 	//if (feildTableExists("reserves") && openFieldTable("reserves")) {
// 	//	readItemReservesTable(dataAllocator, item.m_Reserves);
// 	//	closeTable();
// 	//}

// 	return true;
// }
// bool CItemConfigLoader::readItemConditionTable(CStdItem::ItemUseCondTable &condTable) 
// {
// 		condTable.nCount = lua_objlen(m_pLua, -1);
// 		if (enumTableFirst()) {
// 			CStdItem::ItemUseCondition *pConds;
// 			condTable.pConds = pConds =
// 				(CStdItem::ItemUseCondition*) m_DataAllocator.allocObjects(
// 				sizeof(*condTable.pConds) * condTable.nCount);
// 			do {
// 				pConds->btCond = getFieldInt("cond");
// 				pConds->nValue = getFieldInt("value");
// 				pConds++;
// 			} while (enumTableNext());
// 		}
// 		return true;
// }

// const CStdItem * CItemConfigLoader::GetStdItemConfig(int nId )
// {
// 	INT_PTR nCount = m_itemList.count();
// 	for (INT_PTR i = 0; i < nCount; i++)
// 	{
// 		CStdItem* pStdItem = &m_itemList[i];
// 		if (pStdItem->m_nIndex == nId)
// 		{
// 			return pStdItem;
// 		}
// 	}
// 	return NULL;
// }
