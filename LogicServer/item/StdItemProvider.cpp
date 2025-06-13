#include "StdAfx.h"
#include "StdItemProvider.h"
#include "../misc/caches/StdItemConfigCacher.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

LPCTSTR CStdItemProvider::StdItemCacheFile = _T(
		"./data/runtime/cache/StdItems.cch");

CStdItemProvider::CStdItemProvider() :
		Inherited(), Inherited2(), m_DataAllocator(_T("ItemDataAlloc")), m_DeriveDataAllocator(
				_T("EquipDeriveDataAlloc")), m_SmithDataAllocator(
				_T("SmithDataAllocator")), m_FiveAttrDataAllocator(
				_T("FiveAttrDataAllocator")), m_FiveAttrListDataAllocator(
				_T("FiveAttrListDataAllocator")), m_SuitAttrDataAllocator(_T("SuitAttrDataAllocator")){
	//m_pStdItemsBack = 0;
	m_nItemCount = 0;
	memset(m_orangeEquipAttr, 0, sizeof(m_orangeEquipAttr));

}

CStdItemProvider::~CStdItemProvider() {
}

bool CStdItemProvider::LoadStdItems(LPCTSTR sFilePath, bool bBackLoad) {
	bool Result = false;
	DWORD dwSrcCRC;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try {
		if (!bBackLoad) {
#ifdef _DEBUG
			if (IsDebuggerPresent())
			{
				if (readCacheData(-1))
				{
					OutputMsg(rmTip, "[StdItemProvider] StdItem data force loaded from cache. you may delete cache file to cause load from source!");
					return true;
				}
			}
#endif
		}

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);
		// //计算脚本文本CRC值
		// dwSrcCRC = ~CRC32Update(0xFFFFFFFF, sText, (int)strlen(sText));
		// //尝试从缓存文件快速读取配置数据，如果无法从缓存中读取或数据已经变更则重新加载源数据
		// Result = readCacheData(dwSrcCRC, bBackLoad);
		// if (Result) {
		// 	OutputMsg(rmTip,
		// 			"[StdItemProvider] StdItem config data loaded from cache");
		// } else {
			//设置脚本内容
		if (!setScript(sText))
			showError(_T("syntax error on StdItem config"));
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sText, fp);
			fclose(fp);
		}
		//读取标准物品配置数据
		Result = readStdItems(bBackLoad);
		//保存物品配置数据缓存
		if (Result)
			saveCacheData(dwSrcCRC);
		// }
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load StdItem config error: %s"), s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on load StdItem config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

void CStdItemProvider::completeRead(CStdItem *pStdItems,
		const INT_PTR nItemCount, CDataAllocator &dataAllocator,
		bool bBackLoad) {
	if (bBackLoad) {
		m_Lock.Lock();
		//m_pStdItemsBack = pStdItems;
		m_ItemBack.reserve(0);
		m_ItemBack.trunc(0);
		m_ItemBack.addArray(pStdItems, nItemCount);
		m_nItemCount = nItemCount;
		m_DataAllocatorBack.~CObjectAllocator();
		m_DataAllocatorBack = dataAllocator;
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
		m_Lock.Unlock();
		return;
	}

	//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
	reserve(nItemCount);
	trunc(0);
	addArray(pStdItems, nItemCount);
	//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
	m_DataAllocator.~CObjectAllocator();
	//将临时的物品属性申请器的内存数据拷贝到自身申请器中
	m_DataAllocator = dataAllocator;
	//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));
}

void CStdItemProvider::UpdateItemConfig() {
	reserve(m_nItemCount);
	trunc(0);
	//addArray(m_pStdItemsBack, m_nItemCount);
	addList(m_ItemBack);
	m_DataAllocator.~CObjectAllocator();
	m_DataAllocator = m_DataAllocatorBack;

	//m_pStdItemsBack = 0;
	m_nItemCount = 0;
	ZeroMemory(&m_DataAllocatorBack, sizeof(m_DataAllocatorBack));
}

bool CStdItemProvider::readStdItems(bool bBackLoad) {

	if (!openGlobalTable("StdItems"))
		return false;
	CBaseList<CStdItem> itemList;
	//获取配置中的物品数量
	const INT_PTR nItemCount = lua_objlen(m_pLua, -1) + 1;
	//如果物品数量为0则输出未配置任何物品的错误
	if (nItemCount <= 0)
		showError(_T("no item data on StdItem config"));
	//申请出物品数据并将物品数据内存块清空
	itemList.reserve(nItemCount);
	CStdItem *pStdItems = itemList;
	ZeroMemory(pStdItems, sizeof(CStdItem) * nItemCount);

	//循环读取物品配置数据
	if (enumTableFirst()) {
		INT_PTR nIdx = 1;
		CDataAllocator dataAllocator;
		do {
			//读取物品配置数据
			//CStdItem item = pStdItems[nIdx];
			int index = getFieldInt("id");
			CStdItem& item = pStdItems[index];
			if(index > nItemCount)
			{
				showErrorFormat(_T("readItemData itemindex %d, maxCount %d"),index, nItemCount);
						endTableEnum();
				return false;
			}
			if(!readItemData(dataAllocator, item))
				break;
			item.m_nIndex = index;
			//如果该物品的ID不是期望的值，则输出错误并终止读取
			// if (item.m_nIndex != nIdx) {
			// 	showErrorFormat(_T("unexpected itemindex %d, request %d"),
			// 			item.m_nIndex, nIdx);
			// 	endTableEnum();
			// 	break;
			// }
			nIdx++;
		} while (enumTableNext());
		//完成数据读取，将读取的数据应用到自身
		completeRead(pStdItems, nItemCount, dataAllocator, bBackLoad);

	}

	closeTable();

	return true;
}


void CStdItemProvider::ReadItemForge()
{
	if (!openGlobalTable("ForgeConfig"))
		return;
	int nDef = 0;
	if(enumTableFirst())
	{
		do
		{
			ItemForgeCfg forge;
			forge.nId = getFieldInt("id", &nDef);
			forge.nDropId = getFieldInt("dropid", &nDef);
			forge.nLevel = getFieldInt("level", &nDef);
			m_forgeList.push_back(forge);
		}while(enumTableNext());
	}
	closeTable();

	if (!openGlobalTable("ForgeBaseConfig"))
		return;
	nForgeItemId = getFieldInt("itemID",&nDef);
	nOnceForgeCost = getFieldInt("one",&nDef);
	nTenForgeCost = getFieldInt("ten",&nDef);
	nBagone = getFieldInt("bagone",&nDef);
	nBagten = getFieldInt("bagten",&nDef);
	nEquipment = getFieldInt("equipment",&nDef);
	closeTable();
}

void CStdItemProvider::ReadItemCompose()
{
	if (!openGlobalTable("ItemMergeConfig"))
		return;
	
	int nDef = 0;
	if(enumTableFirst())
	{
		do
		{
			if(enumTableFirst())
			{
				do
				{
					if(enumTableFirst())
					{
						do
						{
							ItemComposeCfg compose;
							compose.nId = getFieldInt("Eid", &nDef);
							compose.nLevelLimt = getFieldInt("level", &nDef);
							
							compose.nOpenServerDay = getFieldInt("openserverday", &nDef);
							compose.nArea = getFieldInt("area", &nDef);
							if(feildTableExists("table") && openFieldTable("table"))
							{
								if(enumTableFirst())
								{
									do
									{
										ComposeTableCfg tf;
										tf.nType = getFieldInt("type");
										tf.nCount = getFieldInt("count");
										tf.nId = getFieldInt("id");
										compose.table.push_back(tf);
									}while(enumTableNext());
								}
								closeTable();
							}

							if(feildTableExists("compose") && openFieldTable("compose"))
							{
								compose.composeItem.nType = getFieldInt("type");
								compose.composeItem.nCount = getFieldInt("count");
								compose.composeItem.nId = getFieldInt("id");
								closeTable();
							}
							if(feildTableExists("mergelimit") && openFieldTable("mergelimit"))
							{
								compose.nCircle = getFieldInt("zsLevel", &nDef);
								compose.nOfficeId = getFieldInt("office", &nDef);
								compose.nVipLimit = getFieldInt("vip", &nDef);
								compose.nGhostLevelLimit = getFieldInt("smLevel", &nDef);
								closeTable();
							}
							
							m_composeList[compose.nId] = compose;
						}while(enumTableNext());
					}
				}while(enumTableNext());
			}
		}while(enumTableNext());
	}
	closeTable();
}

bool CStdItemProvider::readItemData(CDataAllocator &dataAllocator, CStdItem &item) {
	INT_PTR nLevel;
	int nDefVal = 0;

	getFieldStringBuffer("name", item.m_sName, sizeof(item.m_sName));
	item.m_btType = (BYTE) getFieldInt("type");
	item.m_wIcon = (WORD) getFieldInt("icon");
	item.m_wShape = (WORD) getFieldInt("shape", &nDefVal);
	item.m_wBack = (WORD) getFieldInt("back", &nDefVal);
	//item.m_dwDura = (UINT) getFieldInt("dura", &nDefVal);
	double nDwDefVal = 0;
	//item.m_dwUseDurDrop = (UINT) getFieldInt("useDurDrop", &nDefVal);
	item.m_nCDTime = getFieldInt("cdTime", &nDefVal);
	item.m_wDupCount = (WORD) getFieldInt("dup");
	item.m_btColGroup = (BYTE) getFieldInt("colGroup");
	if (item.m_btColGroup >= CUserBag::s_nMaxItemCDGroupCount)
		return false;
	
	item.m_btDealType = (BYTE) getFieldInt("dealType");
	item.m_nPrice = getFieldInt("dealPrice");
	item.m_UseTime = getFieldInt("time");
	//item.m_btSmithId = (BYTE) getFieldInt("smithId", &nDefVal);
	item.m_nRecoverId = getFieldInt("recoverid", &nDefVal);
	item.m_wSuitID = getFieldInt("suitId", &nDefVal); //套装的ID
	item.m_wResonanceId = getFieldInt("resonanceId", &nDefVal); //共鸣ID
	item.b_showQuality = getFieldInt("showQuality", &nDefVal); //显示的品质
	item.m_nDropBroadcast = getFieldInt("dropBroadcast", &nDefVal);
	//item.w_candidateIconCount = (WORD) getFieldInt("candidateIconCount",
	//		&nDefVal); //获取图标
	//item.b_specRing = (BYTE) getFieldInt("specRing", &nDefVal);
	item.m_btBatchType = (BYTE) getFieldInt("batchType", &nDefVal);
	item.m_nBatchValue = getFieldInt("batchValue", &nDefVal);
	item.m_nSillId = 0;//getFieldInt("skillID2", &nDefVal);
	//item.m_nConsignType = getFieldInt("consignType", &nDefVal);
	item.m_DropCount = 0;

	//nDefVal = -1;
	//item.m_nValidFbId = getFieldInt("validFbId", &nDefVal);	// 物品生效的副本ID。默认为-1
	//item.m_nValidSceneId = getFieldInt("validSceneId", &nDefVal);// 物品生效的场景ID。默认为-1

	item.m_nPackageType = getFieldInt("packageType");
	item.m_ndropGroupid = getFieldInt("dropGroupid", &nDefVal);
	item.m_nsuggVocation = getFieldInt("suggVocation",&nDefVal);

	item.m_nItemlvl = getFieldInt("itemlvl",&nDefVal);
	item.nRecycling = getFieldInt("recycling",&nDefVal);
	item.nItemlevel = getFieldInt("itemlevel",&nDefVal);
	item.nJpdrop = getFieldInt("jpdrop",&nDefVal);	
	item.nTips = getFieldInt("tipsid",&nDefVal);
	
	item.m_nOpenDaylimit = getFieldInt("openDaylimit", &nDefVal); 

	bool boDefVal = false;
	item.m_Flags.canMoveKb = (bool)getFieldInt("canMoveKb", &nDefVal);
	item.m_Flags.denyDeal = (bool)getFieldInt("denyDeal", &nDefVal);
	item.m_Flags.denySell = (bool)getFieldInt("denySell", &nDefVal);
	item.m_Flags.denyDestroy = (bool)getFieldInt("denyDestroy", &nDefVal);
	item.m_Flags.recordLog = (bool)getFieldInt("recordLog", &nDefVal);
	
	item.m_CanUseType = 0;
	item.m_CanUseCount = 0;
	if (feildTableExists("UseLimit") && openFieldTable("UseLimit")) 
	{ 
		item.m_CanUseType = getFieldInt("CanUseType", &nDefVal);
		item.m_CanUseCount = getFieldInt("CanUseCount", &nDefVal);   
		closeTable();
	}
	
	if (feildTableExists("deathsplit") && openFieldTable("deathsplit")) {
		if(enumTableFirst())
		{
			do
			{
				ItemDeathSplit splt;
				splt.nType = getFieldInt("type",&nDefVal);
				splt.nItemId = getFieldInt("id",&nDefVal);
				splt.nCount = getFieldInt("count",&nDefVal);
				item.m_nDeathsplit.push_back(splt);
			}while(enumTableNext());
		}
		closeTable();
	}

	if (feildTableExists("select") && openFieldTable("select")) {
		if(enumTableFirst())
		{
			do
			{
				SelectItemInfo info;
				info.nIndex = getFieldInt("index",&nDefVal);
				info.m_ndropGroupid = getFieldInt("dorpid",&nDefVal);
				if (feildTableExists("item") && openFieldTable("item")) {
					ItemDeathSplit splt;
					splt.nType = getFieldInt("type",&nDefVal);
					splt.nItemId = getFieldInt("id",&nDefVal);
					splt.nCount = getFieldInt("count",&nDefVal);
					info.items.push_back(splt);
					closeTable();
				}
				item.m_SelectItems.push_back(info);
			}while(enumTableNext());
		}
		closeTable();
	}
	// 读取物品可存在的场景集合
	item.m_existScenes.nCount = 0;
	if (feildTableExists("existScenes") && openFieldTable("existScenes")) {
		INT_PTR count = lua_objlen(m_pLua, -1);
		item.m_existScenes.nCount = count;
		item.m_existScenes.pSceneId = (int *) dataAllocator.allocObjects(
				count * sizeof(*(item.m_existScenes.pSceneId)));
		INT_PTR index = 0;
		if (count > 0 && enumTableFirst()) {
			do {
				int nSceneId = getFieldInt(NULL);
				if (index < count) {
					item.m_existScenes.pSceneId[index] = nSceneId;
					index++;
					if (nSceneId == -1)				// -1表示不限制场景
					{
						endTableEnum();
						break;
					}
				} else {
					endTableEnum();
					break;
				}
			} while (enumTableNext());
		}
		closeTable();
	} else {
		// 默认不配置表示所有场景都能用
		item.m_existScenes.nCount = 1;
		item.m_existScenes.pSceneId = (int *) dataAllocator.allocObjects(
				item.m_existScenes.nCount
						* sizeof(*(item.m_existScenes.pSceneId)));
		item.m_existScenes.pSceneId[0] = -1;
	}

	//读取物品静态属性
	if (feildTableExists("staitcAttrs") && openFieldTable("staitcAttrs")) {
		readItemAttributeTable(dataAllocator, item.m_StaticAttrs);
		closeTable();
	}
	//读取物品品质属性
	if (feildTableExists("qualityAttrs") && openFieldTable("qualityAttrs")) {
		if (enumTableFirst()) {
			nLevel = 0;
			do {
				readItemAttributeTable(dataAllocator,
						item.m_QualityAttrs[nLevel]);
				nLevel++;
				if (nLevel >= CStdItem::MaxItemQuality) {
					endTableEnum();
					break;
				}
			} while (enumTableNext());
		}
		closeTable();
	}
	//读取物品强化属性
	if (feildTableExists("strongAttrs") && openFieldTable("strongAttrs")) {
		if (enumTableFirst()) {
			nLevel = 0;
			do {
				readItemAttributeTable(dataAllocator,
						item.m_StrongAttrs[nLevel]);
				nLevel++;
				if (nLevel >= CStdItem::MaxItemStrong) {
					endTableEnum();
					break;
				}
			} while (enumTableNext());
			//item.m_StrongCount = (BYTE) nLevel;
		}
		closeTable();
	}
	//读取物品联合属性
	/*
	 if ( feildTableExists("unitedAttrs") && openFieldTable("unitedAttrs") )
	 {
	 item.m_UnitedAttrs.nCount = lua_objlen(m_pLua, -1);
	 if ( enumTableFirst() )
	 {
	 CStdItem::UnitedAttributeGroup *pUnitedGroups;
	 item.m_UnitedAttrs.pGroups = pUnitedGroups = (CStdItem::UnitedAttributeGroup*)dataAllocator.allocObjects(
	 sizeof(*pUnitedGroups) * item.m_UnitedAttrs.nCount);
	 do
	 {
	 //读取联合属性的联合物品表
	 if ( feildTableExists("items") && openFieldTable("items") )
	 {
	 readUnitedItemTable(dataAllocator, *pUnitedGroups);
	 closeTable();
	 }
	 //读取联合属性表
	 if ( feildTableExists("attrs") && openFieldTable("attrs") )
	 {
	 readItemAttributeTable(dataAllocator, *pUnitedGroups);
	 closeTable();
	 }
	 pUnitedGroups++;
	 }
	 while (enumTableNext());
	 }
	 closeTable();
	 }
	 */

	//读取精锻随机属性
	/*
	 if ( feildTableExists("smithAttrs") && openFieldTable("smithAttrs") )
	 {
	 item.m_SmithAttrs.nCount = lua_objlen(m_pLua, -1);
	 item.m_SmithAttrs.pAttrs = (PRANDOMATTRTERM)dataAllocator.allocObjects(
	 sizeof(*item.m_SmithAttrs.pAttrs) * item.m_SmithAttrs.nCount);
	 CRandAttrReader randReader;
	 randReader.readConfig(m_pLua, item.m_SmithAttrs.pAttrs, item.m_SmithAttrs.nCount);
	 closeTable();
	 }
	 */

	//读取物品标志属性
	if (feildTableExists("flags") && openFieldTable("flags")) {
		readItemFlags(item);
		closeTable();
	}
	//读取物品使用条件表
	if (feildTableExists("conds") && openFieldTable("conds")) {
		readItemConditionTable(dataAllocator, item.m_Conditions);
		closeTable();
	}
	//读取物品预留配置
	if (feildTableExists("reserves") && openFieldTable("reserves")) {
		readItemReservesTable(dataAllocator, item.m_Reserves);
		closeTable();
	}

	return true;
}

bool CStdItemProvider::readItemAttributeTable(CDataAllocator &dataAllocator,
		CStdItem::AttributeGroup &AttrGroup) {
	PGAMEATTR pAttr;
	AttrGroup.nCount = lua_objlen(m_pLua, -1);

	if (enumTableFirst()) {
		pAttr = AttrGroup.pAttrs = (PGAMEATTR) dataAllocator.allocObjects(
				sizeof(*AttrGroup.pAttrs) * AttrGroup.nCount);
		do {
			// read type
			pAttr->type = getFieldInt("type");
			//判断物品属性类型是否有效
			if (pAttr->type < aUndefined || pAttr->type >= GameAttributeCount) {
				showErrorFormat(_T("item type config error %d"), pAttr->type);
			}
			// read value
			switch (AttrDataTypes[pAttr->type]) {
			case adSmall:
			case adShort:
			case adInt:
				pAttr->value.nValue = getFieldInt("value");
				break;
			case adUSmall:
			case adUShort:
			case adUInt:
				pAttr->value.uValue = (UINT) getFieldInt64("value");
				break;
			case adFloat:
				pAttr->value.fValue = (float) getFieldNumber("value");
				break;
			}
			pAttr++;
		} while (enumTableNext());
	}
	return true;
}

bool CStdItemProvider::readUnitedItemTable(CDataAllocator &dataAllocator,
		CStdItem::UnitedAttributeGroup &unitedGroup) {
	unitedGroup.nUnitedItemCount = lua_objlen(m_pLua, -1);
	if (enumTableFirst()) {
		PWORD pItemIds;
		unitedGroup.pUnitedItemIds = pItemIds =
				(PWORD) dataAllocator.allocObjects(
						sizeof(*unitedGroup.pUnitedItemIds)
								* unitedGroup.nUnitedItemCount);
		do {
			*pItemIds = getFieldInt(NULL);
			pItemIds++;
		} while (enumTableNext());
	}
	return true;
}


bool CStdItemProvider::readItemFlags(CStdItem &item) {
	bool boDefVal = false;
	//item.m_Flags.recordLog = getFieldBoolean("recordLog", &boDefVal);
	item.m_Flags.denyStorage = getFieldBoolean("denyStorage", &boDefVal);
	item.m_Flags.denyGuildDepot = getFieldBoolean("denyGuildDepot", &boDefVal);
	item.m_Flags.autoBindOnTake = getFieldBoolean("autoBindOnTake", &boDefVal);
	item.m_Flags.autoStartTime = getFieldBoolean("autoStartTime", &boDefVal);
	//item.m_Flags.denyDeal = getFieldBoolean("denyDeal", &boDefVal);
	//item.m_Flags.denySell = getFieldBoolean("denySell", &boDefVal);
	//item.m_Flags.denyDestroy = getFieldBoolean("denyDestroy", &boDefVal);
	item.m_Flags.destroyOnOffline = getFieldBoolean("destroyOnOffline",&boDefVal);

	item.m_Flags.destroyOnDie = getFieldBoolean("destroyOnDie", &boDefVal);
	item.m_Flags.denyDropdown = getFieldBoolean("denyDropdown", &boDefVal);
	item.m_Flags.dieDropdown = getFieldBoolean("dieDropdown", &boDefVal);
	item.m_Flags.offlineDropdown = getFieldBoolean("offlineDropdown",
	 		&boDefVal);
	// //item.m_Flags.inlayable = getFieldBoolean("inlayable", &boDefVal);
	// //item.m_Flags.hideDura = getFieldBoolean("hideDura", &boDefVal);
	// item.m_Flags.denySplite = getFieldBoolean("denySplite", &boDefVal);
	item.m_Flags.asQuestItem = getFieldBoolean("asQuestItem", &boDefVal);
	// item.m_Flags.monAlwaysDropdown = getFieldBoolean("monAlwaysDropdown",
	// 		&boDefVal);
	// item.m_Flags.hideQualityName = getFieldBoolean("hideQualityName",
	// 		&boDefVal);
	// item.m_Flags.denyTipsAutoLine = getFieldBoolean("denyTipsAutoLine",
	// 		&boDefVal);
	item.m_Flags.showLootTips = getFieldBoolean("showLootTips", &boDefVal);
	// item.m_Flags.denyDropDua = getFieldBoolean("denyDropDua", &boDefVal);
	//item.m_Flags.denyRepair = getFieldBoolean("denyRepair", &boDefVal);
	// item.m_Flags.canDig = getFieldBoolean("canDig", &boDefVal);
	// item.m_Flags.fullDel = getFieldBoolean("fullDel", &boDefVal);
	item.m_Flags.denyBuffOverlay = getFieldBoolean("denyBuffOverlay", &boDefVal);
	// item.m_Flags.skillRemoveItem = getFieldBoolean("skillRemoveItem",
	// 		&boDefVal);
	// item.m_Flags.denyHeroUse = getFieldBoolean("denyHeroUse", &boDefVal);
	item.m_Flags.matchAllSuit = getFieldBoolean("matchAllSuit", &boDefVal);
	//item.m_Flags.canMoveKb = getFieldBoolean("canMoveKb", &boDefVal);
	// item.m_Flags.notConsumeForCircleForge = getFieldBoolean(
	// 		"notConsumeForCircleForge", &boDefVal);
	// item.m_Flags.notShowAppear = getFieldBoolean("notShowAppear", &boDefVal);
	// item.m_Flags.boDelete = getFieldBoolean("isDelete", &boDefVal);
	// item.m_Flags.showdura = getFieldBoolean("showdura", &boDefVal);
	// item.m_Flags.bMeltingFlag = getFieldBoolean("MeltingFlag", &boDefVal);
	// item.m_Flags.bCanIdentify = getFieldBoolean("canIdentify", &boDefVal);//是否可以被鉴定，默认不可鉴定
	return true;
}

bool CStdItemProvider::readItemConditionTable(CDataAllocator &dataAllocator,
		CStdItem::ItemUseCondTable &condTable) {
	condTable.nCount = lua_objlen(m_pLua, -1);
	if (enumTableFirst()) {
		CStdItem::ItemUseCondition *pConds;
		condTable.pConds = pConds =
				(CStdItem::ItemUseCondition*) dataAllocator.allocObjects(
						sizeof(*condTable.pConds) * condTable.nCount);
		do {
			pConds->btCond = getFieldInt("cond");
			pConds->nValue = getFieldInt("value");
			pConds++;
		} while (enumTableNext());
	}
	return true;
}

bool CStdItemProvider::readItemReservesTable(CDataAllocator &dataAllocator,
		CStdItem::ItemReservesTable &table) {
	table.nCount = lua_objlen(m_pLua, -1);
	if (enumTableFirst()) {
		CStdItem::ItemReserve *pConfigs;
		table.pConfigs = pConfigs =
				(CStdItem::ItemReserve*) dataAllocator.allocObjects(
						sizeof(*table.pConfigs) * table.nCount);
		do {
			pConfigs->nValue1 = getFieldInt("value1");
			pConfigs->nValue2 = getFieldInt("value2");
			pConfigs++;
		} while (enumTableNext());
	}
	return true;
}

bool CStdItemProvider::readCacheData(DWORD dwSrcCRC32, bool bBackLoad) {
#ifdef _DEBUG
	CStdItem *pStdItems;
	INT_PTR nItemCount;
	CStdItemConfigCacher cache;
	CDataAllocator allocator;

	if ( !cache.LoadFromCache(StdItemCacheFile, dwSrcCRC32, allocator, (void**)&pStdItems, nItemCount) )
	return false;
	completeRead(pStdItems, nItemCount, allocator, bBackLoad);
	return true;
#else
	return false;
#endif
}

bool CStdItemProvider::saveCacheData(DWORD dwSrcCRC32) {
#ifdef _DEBUG	
	bool result = false;
	const CStdItem *pStdItems = *this;
	CStdItemConfigCacher cache;

	try
	{
		result = cache.SaveToCache(StdItemCacheFile, dwSrcCRC32, m_DataAllocator, pStdItems, count());
		if ( !result )
		{
			showError(_T("save StdItem config data cache failure!"));
		}
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on save StdItem config cache!"));
	}
	return result;
#else
	return false;
#endif
}

void CStdItemProvider::showError(LPCTSTR sError) {
	m_sLastErrDesc = sError;
	RefString s = _T("[StdItemProvider]");
	s += sError;
	throw s;
}

bool CStdItemProvider::LoadSmithData(LPCTSTR sFilePath) {

	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);

		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			CDataAllocator smithDataAllocator;
			Result = ReadSmithData(smithDataAllocator);
			m_SmithDataAllocator.~CObjectAllocator();
			m_SmithDataAllocator = smithDataAllocator;
			ZeroMemory(&smithDataAllocator, sizeof(smithDataAllocator));
			if (!Result) {
				return false;
			}
		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load Item smith config error: %s"), s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on mith config config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return true;
}

//读取
bool CStdItemProvider::ReadEquipDerive() {
	if (!openGlobalTable("EquipDeriveConfig"))
		return false;
	INT_PTR nIndex = 0;
	CDataAllocator deriveDataAllocator;
	if (enumTableFirst()) {
		do {
			INT_PTR levelIndex = 0;			//配置索引
			if (enumTableFirst()) {
				do {
					int nLevel = getFieldInt("level"); //强化效果所需星星数
					if (nIndex == 0)	//装备强化衍生光效配置
							{
						int nDefVal = 0;
						STARDERIVE star;
						star.wStarCount = (WORD) nLevel;
						star.wAppear = (WORD) getFieldInt("appear", &nDefVal);
						m_starderive.add(star);
					}
					if (feildTableExists("prop") && openFieldTable("prop")) {
						if (nIndex == 0)     //装备升星的加成
								{
							readItemAttributeTable(deriveDataAllocator,
									m_starderive[levelIndex++].attriGroup);
						} else if (nIndex == 1)
								{

						} else if (nIndex == 2) //橙装的属性加成
								{
							readItemAttributeTable(deriveDataAllocator,
									m_orangeEquipAttr[levelIndex++]);
						}
						closeTable();
					}

				} while (enumTableNext());
			}

			nIndex++;
		} while (enumTableNext());
	}
	m_DeriveDataAllocator.~CObjectAllocator();
	m_DeriveDataAllocator = deriveDataAllocator;
	ZeroMemory(&deriveDataAllocator, sizeof(deriveDataAllocator));
	closeTable();
	return true;
}

bool CStdItemProvider::ReadQualitdataIndex(CDataAllocator& dataAllocator) {
	if (!openGlobalTable("QualityDataIndex")) {
		return false;
	}
	//获取配置中的物品数量
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if (nCount <= 0)
		showError(_T("no QualitdataIndex on QualityDataIndex"));
	INT_PTR nListIndex = 0; //当前遍历的index
	m_qualityDataIndexList.count = nCount;
	m_qualityDataIndexList.pData = (DataList<int>*) dataAllocator.allocObjects(
			nCount * sizeof(DataList<int> ));
	if (enumTableFirst()) {
		do {

			DataList<int>* pIndexList = m_qualityDataIndexList.pData
					+ nListIndex;
			
			if (feildTableExists("index") && openFieldTable("index")) {
				const INT_PTR nDataCount = lua_objlen(m_pLua, -1);
				pIndexList->count = nDataCount;
				pIndexList->pData = (int*) dataAllocator.allocObjects(nDataCount * sizeof(int));
				if (enumTableFirst()) {
					int nDataIndex = 0;
					do{
						int* pIndex = pIndexList->pData + nDataIndex;
						*pIndex = getFieldInt(NULL);
						nDataIndex++;
					} while (enumTableNext());
				}
				closeTable();
			}
			// const INT_PTR nDataCount = lua_objlen(m_pLua, -1);
			// pIndexList->count = nDataCount;
			// pIndexList->pData = (int*) dataAllocator.allocObjects(
			// 		nDataCount * sizeof(int));
			// if (enumTableFirst()) {
			// 	int nDataIndex = 0;
				
			// 	do {
			// 		int* pIndex = pIndexList->pData + nDataIndex;
			// 		*pIndex = getFieldInt(NULL);
			// 		nDataIndex++;
			// 	} while (enumTableNext());
			// }
			nListIndex++;
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

//读取极品属性的数据
bool CStdItemProvider::ReadSmithData(CDataAllocator &dataAllocator) {
	if (!ReadQualitdataIndex(dataAllocator)) {
		return false;
	}
	if (!openGlobalTable("SmithConfig"))
		return false;
	CBaseList<CStdItem> itemList;

	//获取配置中的物品数量
	const INT_PTR nSmithCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if (nSmithCount <= 0)
		showError(_T("no smith data on smith config"));
	//申请出物品数据并将物品数据内存块清空
	m_smithData.count = nSmithCount;
	if (nSmithCount > 0) {
		m_smithData.pData =
				(DataList<RANDOMATTRTERM>*) dataAllocator.allocObjects(
						nSmithCount * sizeof(DataList<RANDOMATTRTERM> ));
	} else {
		return false;
	}
	INT_PTR nIndex = 0; //当前遍历的index
	if (enumTableFirst()) {
		do {
			DataList<RANDOMATTRTERM>* pSmithData = m_smithData.pData + nIndex;
			const INT_PTR nItemCount = lua_objlen(m_pLua, -1);
			pSmithData->count = nItemCount;

			pSmithData->pData = (PRANDOMATTRTERM) dataAllocator.allocObjects(
					nItemCount * sizeof(RANDOMATTRTERM));
			CRandAttrReader randReader;
			if (randReader.readConfig(m_pLua,
					(PRANDOMATTRTERM) pSmithData->pData, dataAllocator,
					nItemCount) != nItemCount) {
				OutputMsg(rmError, "Read Smith%d.txt Error", nIndex);
				return false;
			}
			nIndex++;
		} while (enumTableNext());
	}
	closeTable();
	return true;
}

DataList<RANDOMATTRTERM> * CStdItemProvider::GetSmithData(int nSmithId) {
	if (nSmithId == 0 || nSmithId >= m_smithData.count)
		return NULL;
	return m_smithData.pData + nSmithId;
}
DataList<int>* CStdItemProvider::GetQualityIndexList(int nIndex) {
	if (nIndex <= 0 || nIndex > m_qualityDataIndexList.count) {
		return NULL;
	}
	return m_qualityDataIndexList.pData + nIndex - 1;
}
bool CStdItemProvider::ReadItemEval() {

	if (!openGlobalTable("EquipValuation"))
		return false;
	for (INT_PTR i = 0; i < enMaxVocCount - 1; i++) {
		m_evals[i].reset();
	}

	INT_PTR i = 0;
	if (enumTableFirst()) {
		do {
			if (enumTableFirst()) {
				do {
					tagGameAttribute attr;
					attr.type = (BYTE) getFieldInt("attrId");
					attr.value.fValue = (float) getFieldNumber("unitVal");

					m_evals[i] << attr;

				} while (enumTableNext());
			}

			i++;
		} while (enumTableNext());

	}

	closeTable();

	return true;

}

void CStdItemProvider::AddItemOutPutCount(INT_PTR wItemId, int nCount,
		const INT_PTR nLogIdent) {
	bool bNeedSave = false;

	switch (nLogIdent) {
	case GameLog::clKillMonsterItem:		//杀怪获得物品
	case GameLog::clBuyItem:			//用户从NPC商店购买物品
	case GameLog::clAchieveRewardItem:			//成就奖励物品
	case GameLog::clQuestGiveItem:          //任务奖励物品
	case GameLog::clBuyStoreItem:          //购买商城物品
		bNeedSave = true;
		break;

	default:
		bNeedSave = false;
		break;
	}

	if (!bNeedSave)
		return;

	if (wItemId > 0 && wItemId < Inherited::count()) {
		CStdItem* stdItem = &(this->operator CStdItem*()[wItemId]);
		if (stdItem) {
			stdItem->m_DropCount += nCount;
		}
	}
}

void CStdItemProvider::SendItemOutPutToLog() {
	// for (INT_PTR i = 0; i < Inherited::count(); i++) {
	// 	const CStdItem* pItem = &(this->operator CStdItem*()[i]);
	// 	if (pItem && pItem->m_DropCount > 0) {
	// 	}
	// }
}

bool CStdItemProvider::LoadGodStoveConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			CDataAllocator dataAllocator;
			Result = ReadGodStoveConfig(dataAllocator);
			m_EquipPosStrongAllocator.~CObjectAllocator();
			m_EquipPosStrongAllocator = dataAllocator;
			ZeroMemory(&dataAllocator, sizeof(dataAllocator));

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load GodStoveStrongConfig error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on GodStoveConfig "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}

bool CStdItemProvider::ReadGodStoveConfig(CDataAllocator& dataAllocator) {
	if (!openGlobalTable("GodStoveConfig")) {
		return false;
	}
	//宝石属性衍生属性加成
	if (feildTableExists("GodStoveDeriveConfig")
			&& openFieldTable("GodStoveDeriveConfig")) {
		//const INT_PTR count = lua_objlen(m_pLua, -1);
		if ( enumTableFirst()) {
			do {
				GODSTOVEDERIVE derive;
				derive.nLevel = getFieldInt("level");
				if (feildTableExists("prop") && openFieldTable("prop")) {
					readItemAttributeTable(dataAllocator, derive.attriGroup);
					closeTable();
				}
				m_godStoveDerive.add(derive);
			} while (enumTableNext());
		}
		closeTable();
	}
	closeTable();
	return true;
}

CStdItem::AttributeGroup* CStdItemProvider::GetGodStoveDeriveAttrByLevel(
		INT_PTR nLevel) {
	for (INT_PTR i = m_godStoveDerive.count() - 1; i >= 0; i--) {
		GODSTOVEDERIVE& derive = m_godStoveDerive[i];
		if (nLevel >= derive.nLevel) {
			return &derive.attriGroup;
		}
	}
	return NULL;
}

bool CStdItemProvider::LoadComposeConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			ReadItemCompose();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("LoadComposeConfig error: %s"),
				s.rawStr());
		Result = false;
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on ComposeConfig "));
		Result = false;
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CStdItemProvider::LoadForgeConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			ReadItemForge();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load GodStoveStrongConfig error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on GodStoveConfig "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}

bool CStdItemProvider::LoadAttri(PGAMEATTR pAttr, LPCSTR reserve0Name)
{
	do
	{
		pAttr->type = getFieldInt("type");
		//
		if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
		{
			showErrorFormat(_T("LoadAttri type config error %d"),pAttr->type);
		}
		switch(AttrDataTypes[pAttr->type])
		{
		case adSmall:
		case adShort:
		case adInt:
			pAttr->value.nValue = getFieldInt("value");
			break;
		case adUSmall:
		case adUShort:
		case adUInt:
			pAttr->value.uValue = (UINT)getFieldInt64("value");
			break;
		case adFloat:
			pAttr->value.fValue = (float)getFieldNumber("value");
			break;
		}
		int nDef =0;
		if (reserve0Name)
		{
			pAttr->reserve[0] = getFieldInt(reserve0Name,&nDef);
		}
		++pAttr;
	}while(enumTableNext());
	return true;
}



bool CStdItemProvider::LoadSuitItemConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			ReadSuitItem();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load ReadSuitItem error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on ReadSuitItem "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}


void CStdItemProvider::ReadSuitItem()
{
	if(!openGlobalTable("SuitItemCfg"))
		return;
	int nDef = 0;
	if(enumTableFirst())
	{
		do
		{
			SuitAttr suit;
			suit.nSuitId = getFieldInt("suitid", &nDef);
			suit.nSuitNum = getFieldInt("suitnum", &nDef);
			suit.nPercent = getFieldInt("percent", &nDef);
			getFieldStringBuffer("name", suit.name, sizeof(suit.name));
			int nKey = suit.nSuitId *100 + suit.nSuitNum;
			m_SuitAttrs[nKey] = suit;
			SuitAttr* arrCfg = &m_SuitAttrs[nKey];
			if(arrCfg) {
				if(feildTableExists("attr") && openFieldTable("attr"))
				{
					int  nCount = (int)lua_objlen(m_pLua,-1);
					PGAMEATTR pAttr = NULL;
					arrCfg->attri.nCount = nCount;
					if(nCount > 0)
					{
						arrCfg->attri.pAttrs = (PGAMEATTR)m_SuitAttrDataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
						ZeroMemory(arrCfg->attri.pAttrs, sizeof(GAMEATTR) * nCount);
						pAttr = arrCfg->attri.pAttrs;
					}
					if(enumTableFirst())
					{
						if(!LoadAttri(pAttr))
						{
							return;
						}
						pAttr++;
					}
					closeTable();
				}
			}
		} while (enumTableNext());
	}
	closeTable();
}


bool CStdItemProvider::LoadNumericalConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on StdItem config"));
		} else {
			ReadNumerical();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load ReadNumerical error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on ReadNumerical "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}


void CStdItemProvider::ReadNumerical()
{
	if(!openGlobalTable("NumericalIcon"))
		return;
	int nDef = 0;
	if(enumTableFirst())
	{
		do
		{
			SourceConfig config;
			config.nId = getFieldInt("id", &nDef);
			getFieldStringBuffer("name", config.name, sizeof(config.name));
			m_SourceConfigs[config.nId] = config;
		} while (enumTableNext());
	}
	closeTable();
}


SuitAttr* CStdItemProvider::getSuitPtrDataByKey(int nSuitId, int nSuitNum)
{
	int nKey =nSuitId *100 + nSuitNum;
	std::map<int , SuitAttr>::iterator it = m_SuitAttrs.find(nKey);
	if( it != m_SuitAttrs.end())
	{
		return &(it->second);
	}
	return NULL;
}


bool CStdItemProvider::SuitIsExists(int nKey)
{
	std::map<int , SuitAttr>::iterator it = m_SuitAttrs.find(nKey);
	if( it != m_SuitAttrs.end())
	{
		return true;
	}
	return false;
}

bool CStdItemProvider::LoadRecoverItemConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on RecoverItem config"));
		} else {
			ReadRecoverItemConfig();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load RecoverItemConfig error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on RecoverItemConfig "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}


void CStdItemProvider::ReadRecoverItemConfig()
{
	if (!openGlobalTable("YBrecoverConfig"))
		return;
	
	int nDef = 0;

	if(enumTableFirst())
	{
		do
		{
			ItemRecoverCfg recover;
			recover.nId = getFieldInt("id", &nDef);
			recover.nTimesLimit = getFieldInt("timeslimit", &nDef);
			recover.nTimeLimit = getFieldInt("daylimit", &nDef);
			recover.nRightLimt = getFieldInt("privilegelimit", &nDef);
			if(feildTableExists("item") && openFieldTable("item"))
			{
				if(enumTableFirst())
				{
					do
					{
						ComposeTableCfg tf;
						tf.nType = getFieldInt("type");
						tf.nCount = getFieldInt("count");
						tf.nId = getFieldInt("id");
						recover.cost.push_back(tf);
					}while(enumTableNext());
				}
				closeTable();
			}

			if(feildTableExists("privilegeaward") && openFieldTable("privilegeaward"))
			{
				if(enumTableFirst())
				{
					do
					{
						int nType = getFieldInt("type");
						if(feildTableExists("award") && openFieldTable("award"))
						{
							if(enumTableFirst())
							{
								do
								{
									ComposeTableCfg tf;
									tf.nType = getFieldInt("type");
									tf.nCount = getFieldInt("count");
									tf.nId = getFieldInt("id");
									recover.nAwards[nType].push_back(tf);
								}while(enumTableNext());
							}
							closeTable();
						}
					}while(enumTableNext());
				}
				closeTable();
			}

			if(feildTableExists("recoverlimit") && openFieldTable("recoverlimit"))
			{
				recover.nLevelLimt = getFieldInt("level", &nDef);
				recover.nCircle = getFieldInt("zsLevel", &nDef);
				recover.nOpenServerDay = getFieldInt("openDay", &nDef);
				closeTable();
			}
			
			m_recoverLists[recover.nId] = recover;
		}while(enumTableNext());
	}
	closeTable();
}


bool CStdItemProvider::LoadWarehousemConfig(LPCTSTR sFilePath) {
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	bool Result = true;
	try {

		//从文件加载配置脚本
		if (ms.loadFromFile(sFilePath) <= 0)
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR) ms.getMemory(), sFilePath);

		//设置脚本内容
		if (!setScript(sText)) {
			showError(_T("syntax error on ReadWarehouse config"));
		} else {
			ReadWarehouse();

		}
	} catch (RefString &s) {
		OutputMsg(rmError, _T("load ReadWarehouse error: %s"),
				s.rawStr());
	} catch (...) {
		OutputMsg(rmError, _T("unexpected error on ReadWarehouse "));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return true;
}


//仓库
void CStdItemProvider::ReadWarehouse()
{
	if (!openGlobalTable("WarehouseConfig"))
		return;
	int nDef = 0;
	if(feildTableExists("openprivilege") && openFieldTable("openprivilege"))
	{
		m_WarehouseCfg.nOpenLevel = getFieldInt("openlevel", &nDef);
		m_WarehouseCfg.nOpenCardLv = getFieldInt("vip", &nDef);
		closeTable();
	}
	m_WarehouseCfg.nInitial = getFieldInt("initial", &nDef);
	if(feildTableExists("warehouses") && openFieldTable("warehouses"))
	{
		INT_PTR count = lua_objlen(m_pLua, -1);
		if(count <= 0)
			return;
		count += 1;
		std::vector<int> lists(count, 0);
		if(enumTableFirst())
		{
			do
			{
				int nCount = getFieldInt("count", &nDef);
				int nCardlv = getFieldInt("vip", &nDef);
				lists[nCardlv] = nCount;
			} while (enumTableNext());
			
		}
		m_WarehouseCfg.v_warehouses = lists;
		closeTable();
	}
}
