#include "StdAfx.h"
#include "Container.hpp"
LONG CBoxDropMgr::s_boxDropMgrCnt		= 0;
CBoxDropMgr * CBoxDropMgr::s_BoxDropMgr = NULL;
TCHAR CBaseScriptExportObject::m_sClassName[]	= _T("CBaseScriptExportObject");
TCHAR CBoxDrop::m_sClassName[]					= _T("CBoxDrop");
const char *DropTableName						= "drops";

CBoxDrop::CBoxDrop()
{
	ZeroMemory(&m_dropGroup, sizeof(m_dropGroup));
}

bool CBoxDrop::load(const char *sFile)
{	
	if (!sFile) return false;
	wylib::stream::CMemoryStream ms;
	if (ms.loadFromFile(sFile) <= 0)
	{
		OutputMsg(rmError, _T("load script file(%s) failed"), sFile);
		return false;
	}

	CCustomLuaPreProcessor pp;	
	GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
	LPCSTR sScript = pp.parse((LPCSTR)ms.getMemory(), sFile);
	if (!sScript)
	{
		OutputMsg(rmError, _T("script file(%s) preprocess failed"), sFile);
		return false;
	}

	bool bRet = setScript(sScript);
	if (!bRet)
	{
		OutputMsg(rmError, _T("setscript failed(%s)"), sFile);
		return false;
	}
	bRet = loadData();
	setScript(NULL);
	return bRet;
}

bool CBoxDrop::loadData()
{
	releaseData();
	if (!openGlobalTable(DropTableName))
	{
		OutputMsg(rmError, _T("load box drop data failed, can't find table: %s"), DropTableName);
		return false;
	}
	const INT_PTR dropCount = lua_objlen(m_pLua, -1);
	memset(m_dropGroup.groupMemberCount,	0, sizeof(m_dropGroup.groupMemberCount));
	memset(m_dropGroup.groupNextHitIndex,	0, sizeof(m_dropGroup.groupNextHitIndex));
	if (dropCount > 0)
	{
		m_dropGroup.data.count = dropCount;
		m_dropGroup.data.pData = (PONEDROPDATA)malloc(dropCount * sizeof(ONEDROPDATA));
		ZeroMemory(m_dropGroup.data.pData, dropCount * sizeof(ONEDROPDATA));
		int nIdx = 0;
		if (enumTableFirst())
		{
			PONEDROPDATA pBegin =(PONEDROPDATA )m_dropGroup.data.pData;
			do 
			{
				PONEDROPDATA  pData		= pBegin +nIdx;
				int nDef = 0;
				pData->bAwardType		= (char)getFieldInt("type",&nDef);	//奖励类型
				pData->bStrong			= (char)getFieldInt("strong",&nDef); 
				pData->bQuality			= (char)getFieldInt("quality",&nDef); 
				
				pData->bBind				= (char)getFieldInt("bind", &nDef);

				//最小数量
				pData->nMinCount			= (int)getFieldInt("mincount", &nDef);
				//最大数量
				pData->nMaxCount			= (int)getFieldInt("maxcount", &nDef);

				pData->nCount			= (int)getFieldInt("count", &nDef); 
				pData->wItemID			= (WORD)getFieldInt("id",&nDef); 
				pData->nPropability		= getFieldInt("propability",&nDef); 
				pData->bGroupId			= (BYTE)getFieldInt("group",&nDef);
				pData->nTime				= getFieldInt("time", &nDef);
				//最低的强化
				pData->bMinStrong       =(char) getFieldInt("strongMin",&nDef );

				//最低的品质
				pData->bMinQuality       =(char) getFieldInt("qualityMin",&nDef );
				
				//-1表示任何性别
				nDef =-1;
				pData->bSex       =(char) getFieldInt("sex",&nDef );
				
				//0表示任何的职业
				nDef =0;
				pData->bJob       =(char) getFieldInt("job",&nDef );	

				pData->bAuxParam =(char)getFieldInt("broadcast",&nDef);
				pData->nQualityDataIndex = (WORD)getFieldInt("qualityDataIndex",&nDef);

				if (pData->bGroupId > MAX_DROP_GROUP_COUNT)
				{
					OutputMsg(rmError,_T("%s 掉落组的ID=%d过大，最大=%d"), __FUNCTION__, (int)(pData->bGroupId), MAX_DROP_GROUP_COUNT);
					//pData->bGroupId = 0;
					return false;
				}
				if (pData->bGroupId) {
					m_dropGroup.groupMemberCount[pData->bGroupId-1]++;
				}
				if (pData->nPropability) {					
					pData->nSeed = int(wrand(pData->nPropability)+1);
				}
				nIdx++;
			}
			while (enumTableNext());			
		}
	}

	closeTable();
	return true;
}


DropItemList CBoxDrop::drop(void * pEntity  )
{	
	int sex=-1, job=0;
	if(pEntity )
	{
		CEntity * pActor = (CEntity*) pEntity;
		if( pActor->GetType() == enActor)
		{
			//玩家的职业和等级
			sex = pActor->GetProperty<int>(PROP_ACTOR_SEX);
			job = pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
		}
	}
	DropItemList dropList;
	INT_PTR nDropCount= m_dropGroup.data.count;
	if (nDropCount <=0 ) return dropList;
	const CStdItemProvider& itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	int nLastGroupID=-1;  //遍历的上1个组的ID
	int nLastGroupIndex =-1;
	bool bGroupHasDrop =false; //这个组是1否已经掉落过了
	bool bBagFullFlag[MAX_TEAM_MEMBER_COUNT] = {0};	// 小队玩家任务背包已满标记，用于给用户提示用
	int nGroupCurrentPos =0; //组内开始的位置
	int nCurrentGroupHeadPos =0; //当前遍历的组的第一个元素的位置
	for(INT_PTR i =0 ; i< nDropCount; i++)
	{
		bool flag =true;  //掉落的标记
		ONEDROPDATA * pData = &m_dropGroup.data[i];
		//这个物品无法掉落
		
		

		if(pData->bGroupId ) 
		{
			if( pData->bGroupId != nLastGroupID )
			{
				nCurrentGroupHeadPos = (int)i;
				nLastGroupID = pData->bGroupId;
				nGroupCurrentPos = m_dropGroup.groupNextHitIndex [pData->bGroupId -1];
				bGroupHasDrop = false;
			}

			pData = &m_dropGroup.data[nCurrentGroupHeadPos + nGroupCurrentPos]; //实际数据是取后面的那一个
			nGroupCurrentPos ++;
			nGroupCurrentPos %=   m_dropGroup.groupMemberCount[pData->bGroupId -1]; //组内的下一
		}
		if(CanDrop(pData,sex,job) == false) continue;
		
		pData->nDeadTimes ++; //死亡次数++

		if(pData->nDeadTimes != pData->nSeed ) 
		{
			flag =false;
		}

		if(pData->bGroupId )
		{
			if( flag ) 
			{
				if(!bGroupHasDrop )
				{
					bGroupHasDrop = true ; //这个组里已经掉落了
					//如果已经掉落了，那么下次要从下一个开始遍历
					m_dropGroup.groupNextHitIndex [pData->bGroupId -1]= nGroupCurrentPos;

				}
				else
				{
					flag =false; //如果已经掉落了，那么就不掉了了
				}
			}
		}
		if(pData->nPropability >0 && pData->nDeadTimes >= pData->nPropability) //已经死了很多次了，需要循环
		{
			pData->nDeadTimes =0;
			//第几次死亡能够掉落
			pData->nSeed = GetGlobalLogicEngine()->GetRandValue() %pData->nPropability +1;
		}
		if(!flag) continue; 

		SetItemInfo(dropList,pData); //添加
	}

	return dropList;
}

void  CBoxDrop::SetItemInfo(DropItemList& dropList,ONEDROPDATA * pData )
{
// 	INT_PTR nItemID= pData->wItemID;
// 	if(nItemID)
// 	{
	//去掉nItemID判断，修改为通过bAwardType奖励类型 进行奖励
		DropItemInfo dropItem;
		dropItem.btAwardType = pData->bAwardType;
		dropItem.wItemId =  pData->wItemID;
		dropItem.btCount = (int)pData->nCount;
		int nRandCount = 0;
		//随机奖励数量
		if (pData->nMinCount >= 0 && pData->nMaxCount > 0 )
		{
			dropItem.btCount = (pData->nMinCount + wrand(abs((int)(pData->nMaxCount-pData->nMinCount))+1));
		}
		//dropItem.nRandCount = (unsigned int)nRandCount;

		//支持随机属性
		if (pData->bQuality <0)
		{
			int nValue =0;
			if( pData->bMinQuality >0 )
			{
				nValue = -1* pData->bQuality;
				if( nValue >  pData->bMinQuality)
				{
					nValue = pData->bMinQuality + GetGlobalLogicEngine()->GetRandValue() % (nValue - pData->bMinQuality +1);
				}
				else
				{
					nValue = pData->bMinQuality;
				}
			}
			else
			{
				nValue = -1 * pData->bQuality;
				nValue = GetGlobalLogicEngine()->GetRandValue() % (nValue +1);
			}

			dropItem.btQuality = (BYTE)nValue;
		}
		else
		{
			dropItem.btQuality = (BYTE)pData->bQuality;
		}

		if(pData->bStrong <0)
		{
			int nValue =0;
			if( pData->bMinStrong >0 )
			{
				nValue = -1* pData->bStrong;
				if( nValue >  pData->bMinStrong)
				{
					nValue = pData->bMinStrong + GetGlobalLogicEngine()->GetRandValue() % (nValue - pData->bMinStrong +1);
				}
				else
				{
					nValue = pData->bMinStrong;
				}
			}
			else
			{
				nValue = -1 * pData->bStrong;
				nValue = GetGlobalLogicEngine()->GetRandValue() % (nValue +1);
			}

			dropItem.btStrong = (BYTE)nValue;
		}
		else
		{
			dropItem.btStrong = (BYTE)pData->bStrong;
		}
		dropItem.btBind		= (BYTE)pData->bBind;
		dropItem.btAuxParam	= pData->bAuxParam;
		dropItem.nTime		= pData->nTime;
		dropItem.nQualityDataIndex	= pData->nQualityDataIndex;		//附加属性idx
		dropList.addItem(dropItem);		
			
// 	}
// 	else //0表示金钱,金钱直接发给玩家了
// 	{
// 		dropList.addMoney(pData->nCount);
// 	}
}
DropItemList CBoxDrop::proabilityDrop(void * pEntity )
{
	int sex=-1, job=0;
	if(pEntity )
	{
		CEntity * pActor = (CEntity*) pEntity;
		if( pActor->GetType() == enActor)
		{
			//玩家的职业和等级
			sex = pActor->GetProperty<int>(PROP_ACTOR_SEX);
			job = pActor->GetProperty<int>(PROP_ACTOR_VOCATION);
		}
	}

	DropItemList dropList;
	
	int total[MAX_DROP_GROUP_COUNT]; //最大的掉落组
	int rand[MAX_DROP_GROUP_COUNT];		//随机数
	int result[MAX_DROP_GROUP_COUNT]; //结果

	memset(&total,0,sizeof(total));
	memset(&rand,0,sizeof(rand));
	memset(&result,0,sizeof(result));
	
	INT_PTR nDropCount = m_dropGroup.data.count;
	
	//计算概率的总和
	for(INT_PTR i =0 ; i< nDropCount; i++)
	{
		ONEDROPDATA * pData = &m_dropGroup.data[i];
		if(pData->bGroupId && pData->bGroupId <= MAX_DROP_GROUP_COUNT) 
		{
			if(CanDrop(pData,sex,job) == false) continue;
			{
				total[pData->bGroupId -1] += pData->nPropability;
			}
			
		}
	}

	//计算出随机数
	for(INT_PTR i=0; i< MAX_DROP_GROUP_COUNT; i++)
	{
		if(total[i] >0)
		{
			rand[i] =(int)(1+ wrand( total[i])); 
		}
	}
	
	INT_PTR nCurrentGroupId=0;
	INT_PTR nCurrentTotal=0;

	for(INT_PTR i =0 ; i< nDropCount; i++)
	{
		bool isHit =false; //是否命中了
		ONEDROPDATA * pData = &m_dropGroup.data[i];
		INT_PTR nGroupPos= pData->bGroupId -1;

		if(nGroupPos >=0 && nGroupPos < MAX_DROP_GROUP_COUNT) 
		{
			if(result[nGroupPos] >=0)
			{
				if(CanDrop(pData,sex,job) == false) continue;
				result[nGroupPos] += pData->nPropability;
				if (result[nGroupPos] >=rand[nGroupPos] )
				{
					result[nGroupPos] =-1;//一个组里已经有命中的了
					isHit =true;
				}
				
			}
		}
		else
		{
			isHit =CanDrop(pData,sex,job);
			
		} 
		if(!isHit)  continue;
		SetItemInfo(dropList,pData);
	}

	return dropList;
}


void CBoxDrop::releaseData()
{
	if (m_dropGroup.data.pData)
	{
		free(m_dropGroup.data.pData);
		m_dropGroup.data.pData = 0;
	}
}

void CBoxDrop::destroy()
{
	releaseData();
	CBoxDropMgr::getSingleton().destroyBoxDrop(this);
}


CBoxDropMgr& CBoxDropMgr::getSingleton()
{	
	return *s_BoxDropMgr;
}

void CBoxDropMgr::initialize()
{
	if (::InterlockedIncrement(&s_boxDropMgrCnt) == 1)
	{
		s_BoxDropMgr = new CBoxDropMgr();
	}
}

void CBoxDropMgr::release()
{
	if (::InterlockedDecrement(&s_boxDropMgrCnt) == 0)
	{
		SafeDelete(s_BoxDropMgr);
	}
}


CBoxDropMgr::~CBoxDropMgr()
{
	INT_PTR count = m_boxObjectList.count();
	for (INT_PTR i = 0; i < count; i++)
	{	
		// 强制销毁		
		m_boxObjectList[i]->releaseData();
		destroyBoxDropImpl(m_boxObjectList[i]);
	}
	m_boxObjectList.clear();
}

CBoxDrop* CBoxDropMgr::createBoxDrop(const char *pKey)
{
	if (!pKey) return 0;
	
	CBoxDrop *box = m_dataAllocator.allocObject();	
	if (box) {
		new(box)CBoxDrop();
		box->SetObjectName(pKey);
		box->addRef();
		GlobalObjectMgr::getSingleton().SetGlobalObject(pKey, box);
		m_boxObjectList.add(box);
	}

	return box;
}

void CBoxDropMgr::destroyBoxDrop(CBoxDrop *bd)
{
	destroyBoxDropImpl(bd);
	removeFromList(bd);
}

void CBoxDropMgr::removeFromList(CBoxDrop *bd)
{
	for (INT_PTR i = 0; i < m_boxObjectList.count(); i++)
	{
		if (m_boxObjectList[i] == bd)
			m_boxObjectList.remove(i);
	}	
}

void CBoxDropMgr::destroyBoxDropImpl(CBoxDrop *bd)
{
	if (bd)	{
		GlobalObjectMgr::getSingleton().ClearGlobalObject(bd->GetObjectName());
		bd->~CBoxDrop();
		m_dataAllocator.freeObject(bd);		
	}	
}
