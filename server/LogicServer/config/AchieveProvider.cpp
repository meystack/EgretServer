#include "StdAfx.h"

#include "AchieveProvider.h"
#include "AchieveData.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CAchieveProvider::CAchieveProvider() : m_DataAllocator(_T("AchieveDataAlloc")), m_eventAllocator(_T("AchieveEventAlloc"))
{	
	CVector<int> temp;
	for(INT_PTR i=0;i< eMaxAtomEventID;i++)
	{
		m_atomEventVec.add(temp);
	}
}

CAchieveProvider::~CAchieveProvider()
{
	m_achieveData.empty();
	for (int i = 0; i < m_atomEventVec.count(); i++)
	{
		m_atomEventVec[i].empty();
	}
	m_achieveEvents.empty();
}

void CAchieveProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[CAchieveProvider Config Error]");
	s += sError;
	throw s;
}

bool CAchieveProvider::LoadBadgeConfig(LPCSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on Badge config"));
		//读取标准物品配置数据
		Result = ReadBabgeConfig();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Badge config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}

bool CAchieveProvider::ReadBabgeConfig()
{
	if ( !openGlobalTable("BadgeConfig") )
		return false;
	const INT_PTR nCount = lua_objlen(m_pLua, -1);

	if ( nCount <= 0 )
	{
		showError(_T("no data on title config"));
		return true;
	}

	if ( nCount > 0 )
	{
		//循环读取
		if ( enumTableFirst() )
		{
			INT_PTR nIdx = 0;

			do 
			{
				ACHIEVEBADGE achieveBadge;
				achieveBadge.nId = getFieldInt("id");
				achieveBadge.wItemId = getFieldInt("itemId");
				/*	achieveBadge.wCount = getFieldInt("itemCount");
				achieveBadge.btQuality = getFieldInt("itemQuality");
				achieveBadge.btStrong = getFieldInt("itemStrong");
				achieveBadge.btBindFlag = getFieldInt("itemBindFlag");
				achieveBadge.nLeftTime = getFieldInt("itemLeftTime");*/
				achieveBadge.nAchievePoint = getFieldInt("achievePoint");
				m_BadgeData.add(achieveBadge);
				nIdx++;
			}
			while (enumTableNext());
		}
	}

	closeTable();
	return true;
}

/*
* Comments:装载全部怪物的数据
* @Return bool: 失败返回false
*/
bool CAchieveProvider::LoadAchieves(LPCTSTR sFilePath)
{

	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllAchieve();
		
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadAchieves config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}


bool CAchieveProvider::ReadAllTitles()
{
	if ( !openGlobalTable("Titles") )
		return false;


	CBaseList<TITLECONFIG> userDataList;

	const INT_PTR nCount = lua_objlen(m_pLua, -1);

	if ( nCount <= 0 )
	{
		showError(_T("no data on title config"));
		return true;
	}

	userDataList.reserve(nCount);
	TITLECONFIG *pData= userDataList;
	ZeroMemory(pData, sizeof(TITLECONFIG) * nCount);


	//循环读取
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator(_T("TitleDataTmpAlloc"));
		do 
		{
			TITLECONFIG * pOneData= &userDataList[nIdx];
			ReadOneTitle(dataAllocator,pOneData);
			nIdx++;
		}
		while (enumTableNext());

		//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
		m_titles.reserve(nCount);
		m_titles.trunc(0);
		m_titles.addArray(pData, nCount);
		//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
		m_titleAllocator.~CObjectAllocator();
		//将临时的物品属性申请器的内存数据拷贝到自身申请器中
		m_titleAllocator = dataAllocator;
		//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();
	//OutputMsg(rmTip,_T("Load  Finished"));


	return true;
}

bool CAchieveProvider::ReadOneTitle(CDataAllocator &dataAllocator,TITLECONFIG *pTitle)
{
	getFieldStringBuffer("name",pTitle->name,sizeof(pTitle->name) );
	pTitle->nGroupId = getFieldInt("groupId");
	pTitle->nDurTime = getFieldInt("time");
	bool bDef = false;
	pTitle->isDelete = getFieldBoolean("isDelete",&bDef); //该称号是否删除了
	bDef = false;
	pTitle->isFamous = getFieldBoolean("isFamous",&bDef); //该称号是否是名人称号
	int nDeufalt = 0;
	pTitle->nHeadTitleId = getFieldInt("headTitleId",&nDeufalt);

	if ( feildTableExists("buff") && openFieldTable("buff") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			pTitle->buffs.count =0; //没有技能
		}
		else
		{
			pTitle->buffs.count =nCount; 
			pTitle->buffs.pData =(PTITLEBUFF)dataAllocator.allocObjects(nCount * sizeof(TITLEBUFF));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;

				PTITLEBUFF pBaseLevel =(PTITLEBUFF )pTitle->buffs.pData;
				PTITLEBUFF  pOneCondition  =NULL;
				do 
				{					
					pOneCondition= pBaseLevel + nIdx;
					pOneCondition->bType = (BYTE) getFieldInt("type"); 
					pOneCondition->bGroup = (BYTE) getFieldInt("group"); 
					pOneCondition->value = (float) getFieldNumber("value"); 
					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}
	return true;
}
bool  CAchieveProvider::LoadTitles(LPCSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllTitles();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadTitles config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}

bool CAchieveProvider::LoadTopTitles(LPCSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllTopTitle();

	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load HeadTitle config "));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}

bool CAchieveProvider::ReadAllTopTitle()
{
	if ( !openGlobalTable("HeadTitle") )
		return false;
	CBaseList<TOPTITLECONFIG> userDataList;

	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	if ( nCount <= 0 )
	{
		showError(_T("no data on HeadTitle config"));
		return true;
	}

	userDataList.reserve(nCount);
	TOPTITLECONFIG *pData= userDataList;
	ZeroMemory(pData, sizeof(TOPTITLECONFIG) * nCount);

	//循环读取
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator(_T("TopTitleDataTmpAlloc"));
		do 
		{
			TOPTITLECONFIG * pOneData= &userDataList[nIdx];
			ReadOneTopTitle(dataAllocator,pOneData);
			nIdx++;
		}
		while (enumTableNext());

		//读取头衔配置完成，将临时列表中的头衔数据全部拷贝到自身中
		m_topTitles.reserve(nCount);
		m_topTitles.trunc(0);
		m_topTitles.addArray(pData, nCount);
		//调用自身的申请器的析构函数，以便释放之前的内存块
		m_topTitleAllocator.~CObjectAllocator();
		//将临时的申请器的内存数据拷贝到自身申请器中
		m_topTitleAllocator = dataAllocator;
		//清空临时申请器的内存数据，防止新读取的数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();

	return true;
}

bool CAchieveProvider::ReadOneTopTitle(CDataAllocator &dataAllocator,TOPTITLECONFIG *pTopTitle)
{
	pTopTitle->nRankId = getFieldInt("rankId");
	pTopTitle->nMinRank = getFieldInt("minRank");
	pTopTitle->nMaxRank = getFieldInt("maxRank");
	pTopTitle->nTitleId = getFieldInt("titleId"); 
	//读取静态属性
	if ( feildTableExists("staitcAttrs") && openFieldTable("staitcAttrs") )
	{
		//使用物品类似读取方式
		if (!ReadAttributeTable(dataAllocator, pTopTitle->staticAttrs))
		{
			OutputMsg(rmError, _T("ReadOneTopTitle Attr Error!"));
		}
		closeTable();
	}
	return true;
}

bool CAchieveProvider::ReadAttributeTable(CDataAllocator &dataAllocator, CStdItem::AttributeGroup &AttrGroup)
{
	PGAMEATTR pAttr;
	AttrGroup.nCount = lua_objlen(m_pLua, -1);

	if ( enumTableFirst() )
	{
		pAttr = AttrGroup.pAttrs = (PGAMEATTR)dataAllocator.allocObjects(sizeof(*AttrGroup.pAttrs) * AttrGroup.nCount);
		do 
		{
			pAttr->type = getFieldInt("type");
			//判断物品属性类型是否有效
			if ( pAttr->type < aUndefined || pAttr->type >= GameAttributeCount )
			{
				showErrorFormat(_T("attribute type config error %d"),pAttr->type);
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
			pAttr++;
		}
		while (enumTableNext());
	}
	return true;
}


CAttrCalc& CAchieveProvider::CalcHeadTitleAttributes(INT_PTR nTitleId, CAttrCalc &calc)
{
	PTOPTITLECONFIG pConfig = GetTopTitle(nTitleId);

	if (pConfig != NULL)
	{
		//写入静态属性
		if ( pConfig->staticAttrs.nCount > 0 )
		{

			for (INT_PTR i=pConfig->staticAttrs.nCount-1; i>-1; --i)
			{
				calc << pConfig->staticAttrs.pAttrs[i];
			}
		}
	}

	return calc;
}

bool CAchieveProvider::ReadAllAchieve()
{

	if ( !openGlobalTable("Achieves") )
		return false;


	CBaseList<ACHIEVEDATA> userDataList;

	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	
	if ( nCount <= 0 )
	{
		showError(_T("no data on achieve config"));
		return true;
	}

	userDataList.reserve(nCount);
	ACHIEVEDATA *pData= userDataList;
	ZeroMemory(pData, sizeof(ACHIEVEDATA) * nCount);


	//循环读取
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator(_T("AchieveDataTmpAlloc"));
		do 
		{
			ACHIEVEDATA * pOneData= &userDataList[nIdx];

			//读取
			if(ReadOneAchieve( dataAllocator,pOneData,nIdx) ==false)
			{
				showErrorFormat(_T("achieve config error, id=%d "),nIdx );
				return false;
			}
			nIdx++;
		}
		while (enumTableNext());

		//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
		m_achieveData.reserve(nCount);
		m_achieveData.trunc(0);
		m_achieveData.addArray(pData, nCount);
		//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
		m_DataAllocator.~CObjectAllocator();
		//将临时的物品属性申请器的内存数据拷贝到自身申请器中
		m_DataAllocator = dataAllocator;
		//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();
	OutputMsg(rmTip,_T("Load achieve Finished"));


	return true;
}

//装载成就的分组的数据
bool CAchieveProvider::LoadAchieveGroup(LPCSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on ai config"));

		if ( !openGlobalTable("AchieveGroups") )
			return false;
		m_achieveGroup.clear();
		if ( enumTableFirst() )
		{
			int nIdx = 0;
			do 
			{		
				ACHIEVEGROUP  data;
				int id = getFieldInt("id");
				if(id != nIdx)
				{
					OutputMsg(rmError,_T("%d Data id error,id=%d"),sFilePath,nIdx);
					return false;
				}
				int nDefault=0;

				data.nCamp =  getFieldInt("camp",&nDefault); 
				data.nLevel =  getFieldInt("level",&nDefault); 
				 
				data.nVocation =  getFieldInt("vocation",&nDefault); 
				nDefault=-1;
				data.nSex =  getFieldInt("sex",&nDefault);

				data.titleId = getFieldInt("titleID",0);

				if (!openFieldTable("titleNeedAchieves"))
				{
					OutputMsg(rmError,_T("装载战绩配置失败，找不到militaryRank[].range"));
					return false;
				}

				if (enumTableFirst())		//
				{
					INT_PTR nIdx = 0;
					do 
					{
						if( nIdx < GROUP_ARCHIEVE_MAX )
						{
							data.mTitleNeedAchieves[nIdx] = (int)getFieldInt(NULL);
							nIdx++;
						}
					} while (enumTableNext());
				}
				closeTable();

				m_achieveGroup.add(data);
				nIdx ++;
			}while (enumTableNext());
		}
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadAchieveGroup config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return true;
}

/*
* Comments: 读取一个怪物的数据
* Param CDataAllocator & dataAllocator: 内存分配器
* Param AICONFIG * oneAi:  ai指针
* @Return bool:
*/
bool CAchieveProvider::ReadOneAchieve(CDataAllocator &dataAllocator,ACHIEVEDATA *oneAchieve,INT_PTR nIndex)
{

	getFieldStringBuffer("name",oneAchieve->name,sizeof(oneAchieve->name) );
	int nAchieveID =  getFieldInt("id");
	if((int)nIndex != nAchieveID) //
	{
		OutputMsg(rmTip,_T("成就ID配置错误,index=%d"),nIndex);
		return false; 
	}
	oneAchieve->nCondCount = getFieldInt("needCondCount");
	bool bDefault =false;
	oneAchieve->bNeedBroadcast = getFieldBoolean("broadcast",&bDefault);
	
	int nDef = 0;
	oneAchieve->groupId = getFieldInt("groupId",&nDef);
	oneAchieve->nBadgeId = getFieldInt("badgeId",&nDef);

	//该成就是否已经删除了
	bDefault =false;
	oneAchieve->bIsDelete = getFieldBoolean("isDelete",&bDefault); 
	
	oneAchieve->isDefaultActive = getFieldBoolean("isDefaultActive",&bDefault); //该称号是否删除了

	oneAchieve->isDayRefresh = getFieldBoolean("isDayRefresh",&bDefault);
	oneAchieve->isMonthRefresh = getFieldBoolean("isMonthRefresh", &bDefault);

	oneAchieve->showPos = (BYTE)getFieldInt("showPos",&nDef);

	if ( feildTableExists("conds") && openFieldTable("conds") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			oneAchieve->conditions.count =0; //没有技能
		}
		else
		{
			oneAchieve->conditions.count =nCount; //技能的动作
			oneAchieve->conditions.pData =(PACHIEVECONDITION)dataAllocator.allocObjects(nCount * sizeof(ACHIEVECONDITION));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;
			
				PACHIEVECONDITION pBaseLevel =(PACHIEVECONDITION )oneAchieve->conditions.pData;
				PACHIEVECONDITION  pOneCondition  =NULL;
				do 
				{					
					pOneCondition= pBaseLevel + nIdx;
					pOneCondition->wEventID = (WORD) getFieldInt("eventId"); 
					pOneCondition->nCount = getFieldInt("count"); 
					
					//pOneCondition->wDataPos =(WORD) getFieldInt("dataPos"); 
					//pOneCondition->bDataSize =(BYTE) getFieldInt("dataByteCount"); 
					if(pOneCondition->wEventID < m_achieveEvents.count())
					{
						if(m_achieveEvents[pOneCondition->wEventID].achieveIDList.pData == NULL)
						{
							m_achieveEvents[pOneCondition->wEventID].achieveIDList.pData = (int*)dataAllocator.allocObjects(
								MAX_ACHIEVE_EVENT_ACHIEVE_COUNT * sizeof(int)); //每个成就事件默认有30个成就关注
							m_achieveEvents[pOneCondition->wEventID].achieveIDList.count =0;
						}
						int nCount= (int)m_achieveEvents[pOneCondition->wEventID].achieveIDList.count  ; 
						if( nCount < MAX_ACHIEVE_EVENT_ACHIEVE_COUNT) //避免内存越界
						{
							*(m_achieveEvents[pOneCondition->wEventID].achieveIDList.pData + nCount) = nAchieveID; //将这个成就添加进去
							m_achieveEvents[pOneCondition->wEventID].achieveIDList.count ++;
						}
						else
						{
							OutputMsg(rmError,_T("成就事件%d 已经有超过了%d个成就关注了"),pOneCondition->wEventID,MAX_ACHIEVE_EVENT_ACHIEVE_COUNT);
						}
					}
					else
					{
						OutputMsg(rmError,_T("成就ID=%d,事件id=%d不存在"),nIdx,pOneCondition->wEventID);
						return false;
					}
					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}

	if ( feildTableExists("awards") && openFieldTable("awards") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			oneAchieve->awards.count =0; //没有技能
		}
		else
		{
			oneAchieve->awards.count =nCount; //技能的动作
			oneAchieve->awards.pData =(PACHIEVEAWARD)dataAllocator.allocObjects(nCount * sizeof(ACHIEVEAWARD));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;

				PACHIEVEAWARD pBaseLevel =(PACHIEVEAWARD )oneAchieve->awards.pData;
				PACHIEVEAWARD  pOneAward  =NULL;
				int nDefault=0;
				int nCount=1;
				do 
				{					
					pOneAward= pBaseLevel + nIdx;
					pOneAward->bType = (BYTE) getFieldInt("type"); 
					pOneAward->wID = (WORD) getFieldInt("id"); 
					pOneAward->nCount = (unsigned  int)getFieldInt("count",&nCount); 
					pOneAward->bQuality =(BYTE) getFieldInt("quality",&nDefault); 
					pOneAward->bBind =(BYTE) getFieldInt("bind",&nDefault); 
					pOneAward->bStrong =(BYTE) getFieldInt("strong",&nDefault); 
					
					nDefault =-1; //所有的性别都能用
					pOneAward->nSex =  getFieldInt("sex",&nDefault);

					nDefault =0;
					pOneAward->nVocation =  getFieldInt("job",&nDefault);
					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}

	return true;
}
	
bool CAchieveProvider::LoadAchieveEvents(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{

		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on ai config"));
		//读取标准物品配置数据
		Result = ReadAllAchieveEvent();


	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LoadAchieveEvents config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);

	return Result;
}


//读取成就的事件
bool CAchieveProvider::ReadAllAchieveEvent()
{

	if ( !openGlobalTable("AchieveEvents") )
		return false;


	CBaseList<ACHIEVEEVENT> userDataList;

	const INT_PTR nCount = lua_objlen(m_pLua, -1);

	if ( nCount <= 0 )
	{
		showError(_T("no data on achieve event config"));
		return false;
	}

	userDataList.reserve(nCount);
	ACHIEVEEVENT *pData= userDataList;
	ZeroMemory(pData, sizeof(ACHIEVEEVENT) * nCount);


	//循环读取
	if ( enumTableFirst() )
	{
		INT_PTR nIdx = 0;
		CDataAllocator dataAllocator(_T("AchieveEvtTmpAlloc"));
		do 
		{
			ACHIEVEEVENT * pOneData= &userDataList[nIdx];

			//读取
			if(ReadOneEvent( dataAllocator,pOneData,nIdx) ==false)
			{
				showErrorFormat(_T("achieve event config error, id=%d "),nIdx );
				return false;
			}
			nIdx++;
		}
		while (enumTableNext());

		//读取物品配置完成，将临时列表中的物品数据全部拷贝到自身中
		m_achieveEvents.trunc(0);
		m_achieveEvents.reserve(nCount);
		m_achieveEvents.addArray(pData, nCount);
		//调用自身的物品属性申请器的析构函数，以便释放之前的物品属性的内存块
		m_eventAllocator.~CObjectAllocator();
		//将临时的物品属性申请器的内存数据拷贝到自身申请器中
		m_eventAllocator = dataAllocator;
		//情况临时物品属性申请器的内存数据，防止新读取的物品属性数据被销毁
		ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	}

	closeTable();
	OutputMsg(rmTip,_T("Load achieve event Finished"));


	return true;
}

//读取一个事件
bool CAchieveProvider::ReadOneEvent(CDataAllocator &dataAllocator,ACHIEVEEVENT *pEvent,INT_PTR nAchieveEventID)
{
	pEvent->wAtomEventID = (WORD)getFieldInt("atomEventId");
	int nDef = 0;
	pEvent->bPlusParamId = (BYTE)getFieldInt("plusParamId",&nDef);

	if(pEvent->wAtomEventID >= eMaxAtomEventID || pEvent->wAtomEventID == eAchieveEventIdNone)
	//if( pEvent->wAtomEventID == eAchieveEventIdNone)
	{
		OutputMsg(rmWaning,_T("成就的原子事件ID=%d超过了最大值%d"),pEvent->wAtomEventID,eMaxAtomEventID );
		return false;
	}
	else
	{
		m_atomEventVec[pEvent->wAtomEventID].add((int)nAchieveEventID); //成就事件加入到这个列表
	}
	pEvent->bLogicOp = (BYTE)getFieldInt("logicop");
	if ( feildTableExists("conds") && openFieldTable("conds") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			pEvent->conds.count = 0; //没有技能
		}
		else
		{
			pEvent->conds.count = nCount; //技能的动作
			pEvent->conds.pData =(PACHIEVEEVENTCOND)dataAllocator.allocObjects(nCount * sizeof(ACHIEVEEVENTCOND));
			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;

				PACHIEVEEVENTCOND pBaseLevel =(PACHIEVEEVENTCOND )pEvent->conds.pData;
				PACHIEVEEVENTCOND  pOneCondition  =NULL;
				do 
				{					
					pOneCondition= pBaseLevel + nIdx;
					pOneCondition->bParamID = (BYTE) getFieldInt("paramID"); 
					pOneCondition->bParamType = (BYTE) getFieldInt("paramType"); 
					ReadEventCond(dataAllocator,pOneCondition);
					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
	}
	return true;
}

//读取一个事件的条件
bool CAchieveProvider::ReadEventCond(CDataAllocator &dataAllocator,ACHIEVEEVENTCOND * pCond)
{
	if ( feildTableExists("params") && openFieldTable("params") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		pCond->params.count= (int)nCount; //个数
		if (enumTableFirst())
		{
			pCond->params.pData = (int *)dataAllocator.allocObjects(nCount * sizeof(int));
			INT_PTR nIdx = 0;
			int* pBaseLevel =(int* )pCond->params.pData;
			do 
			{
				pBaseLevel[nIdx] = (int) getFieldInt(NULL); 
				nIdx++;
			}while (enumTableNext());
		}
		closeTable();
	}
	return true;
}

