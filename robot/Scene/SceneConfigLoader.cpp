#include "stdafx.h"
using namespace wylib::stream;

const LPCTSTR CSceneConfigLoader::szEnvirConfigFileName = _T("data/envir/staticZone.txt");

using jxcomm::gameMap::CAbstractMap;

CSceneConfigLoader::CSceneConfigLoader()
{
}

void CSceneConfigLoader::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Scene Config Error]");
	s += sError;
	throw s;
}

bool CSceneConfigLoader::Load()
{
	bool bRet		= false;
	LPCTSTR sResult = NULL;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;
	try
	{
		GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);
		if (ms.loadFromFile(szEnvirConfigFileName) <= 0)
		{
			showErrorFormat(_T("unable to load config file from %s"), szEnvirConfigFileName);
			return false;
		}

		sResult = pp.parse((LPCTSTR)ms.getMemory(), szEnvirConfigFileName);
		if (!sResult)
		{
			OutputMsg(rmError, _T("预处理文件%s失败"), szEnvirConfigFileName);
			return false;
		}
		if (!setScript(sResult))
		{
			OutputMsg(rmError,_T("读取预处理后的配置文件%s失败"), szEnvirConfigFileName);
			return false;
		}
		bRet = LoadSceneConfig();
	}
	catch (RefString& s)
	{
		OutputMsg(rmError, (LPCTSTR)s);
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexcepted error on load config: %s"), szEnvirConfigFileName);
	}
	setScript(NULL);
	return bRet;
}

// 加载场景配置
bool CSceneConfigLoader::LoadSceneConfig()
{
	m_sceneConfigList.pList = NULL;
	m_sceneConfigList.nCount = 0;
	if (openGlobalTable("Scenes"))
	{
		m_sceneConfigList.nCount = (int)lua_objlen(m_pLua, -1);
		if (m_sceneConfigList.nCount > 0)
		{
			int nSize = sizeof(SCENEMAPCONF) * m_sceneConfigList.nCount;
			m_sceneConfigList.pList = (SCENEMAPCONF*)m_DataAllocator.allocObjects(nSize);
			ZeroMemory(m_sceneConfigList.pList,nSize);
		}

		int nFuBenScenePos = 0;
		int nSceneId = 0;
		if (enumTableFirst())
		{
			do 
			{
				nSceneId = getFieldInt("sceneid");
				nFuBenScenePos = nSceneId - 1;
				if (nSceneId == 0)
				{
					continue;
				}
				
				if (nFuBenScenePos >= m_sceneConfigList.nCount || nFuBenScenePos < 0)
				{
					OutputMsg(rmError,_T("load scene id error!nSceneId=%d"),nSceneId);
					return false;
				}
				
				SCENEMAPCONF* sceneConf = &m_sceneConfigList.pList[nFuBenScenePos];
				new (sceneConf) tagSceneMapConf();
				sceneConf->nScenceId = nSceneId;

				int nDef =0;
				bool boDef = false;
				bool bMaxAnger = false;
				bool bBroadcast = false;
				sceneConf->nDefaultX = getFieldInt("defaultX",&nDef);
				sceneConf->nDefaultY = getFieldInt("defaultY",&nDef);

				//场景的类型
				sceneConf->bSceneType = getFieldInt("sceneType",&nDef);
				getFieldStringBuffer("scencename",sceneConf->szScenceName,ArrayCount(sceneConf->szScenceName));
				
				//读取场景map数据
				getFieldStringBuffer("mapfilename",sceneConf->szMapFileName,ArrayCount(sceneConf->szMapFileName));
				if (!LoadSceneMap(sceneConf))
				{
					OutputMsg(rmError,_T("load scene Map error!sceneid=%d,scenename=%s"),sceneConf->nScenceId,sceneConf->szScenceName);
					return false;
				}
				
				//读取刷怪配置
				if(!LoadRefreshConfig(sceneConf))
				{
					OutputMsg(rmError,_T("load scene monster error!sceneid=%d,scenename=%s"),sceneConf->nScenceId,sceneConf->szScenceName);
					return false;
				}
				
				//读取区域配置
				if (!LoadAreasConfig(sceneConf))
				{
					OutputMsg(rmError,_T("load scene area error!sceneid=%d,scenename=%s"),sceneConf->nScenceId,sceneConf->szScenceName);
					return false;
				}

				//读取npc
				if(!LoadNpcConfig(sceneConf))
				{
					OutputMsg(rmError,_T("load scene Npc error!sceneid=%d,scenename=%s"),sceneConf->nScenceId,sceneConf->szScenceName);
					return false;
				}

				//teleport传送点
				if (!LoadTelePortConfig(sceneConf))
				{
					OutputMsg(rmError,_T("load scene Teleport error!sceneid=%d,scenename=%s"),sceneConf->nScenceId,sceneConf->szScenceName);
					return false;
				}

				LoadHookConfig(sceneConf);

			} while (enumTableNext());
		}
	} else return FALSE;

	closeTable();
	return true;
}
bool CSceneConfigLoader::LoadSceneMap(SCENEMAPCONF* sceneConf)
{
	char sMapFullBuff[40]; //地图文件的全名
	sprintf(sMapFullBuff,"data/map/%s",sceneConf->szMapFileName);
	return sceneConf->m_MapData.LoadFromFile(sMapFullBuff);
}
bool CSceneConfigLoader::LoadRefreshConfig(SCENECONFIG* sceneConf) 
{
	if (!openFieldTable("refresh"))
	{
		return FALSE;
	}
	sceneConf->vRefreshList.nCount = (int)lua_objlen(m_pLua, -1);
	if (sceneConf->vRefreshList.nCount > 0)
	{
		int nSize = sizeof(REFRESHCONFIG)*sceneConf->vRefreshList.nCount;
		sceneConf->vRefreshList.pList = (REFRESHCONFIG*)m_DataAllocator.allocObjects(nSize);
		ZeroMemory(sceneConf->vRefreshList.pList,nSize);
	}
	int refreshPos = 0;
	if (enumTableFirst())
	{
		do
		{
			REFRESHCONFIG* refreshData = sceneConf->vRefreshList.pList + refreshPos;
			refreshData->nCount = getFieldInt("count");
			refreshData->nNextRefreshTime = getFieldInt("time");	//每次刷怪的时间
			if (refreshData->nNextRefreshTime < 0)
			{
				refreshData->nNextRefreshTime = MAXINT;//只刷一次
			}
			refreshData->nFirstTime = getFieldInt("firstTime");
			refreshData->nNextRefreshTime = refreshData->nNextRefreshTime * 1000;//这个单位是秒
			refreshData->nFirstTime = refreshData->nFirstTime * 1000;//这个单位是秒
			refreshData->nEntityId = getFieldInt("entityid");
			refreshData->nMobX = getFieldInt("x");
			refreshData->nMobY = getFieldInt("y");
			refreshData->nMobRange = getFieldInt("range");
			int nDef = 0;
			refreshData->nLiveTime = getFieldInt("livetime",&nDef);
			refreshPos++;
		} while (enumTableNext());
	}
	closeTable();//close Refresh
	return true;
}
bool CSceneConfigLoader::LoadAreasConfig(SCENECONFIG* sceneConf)
{
	//读取特殊区域列表
	if (!openFieldTable("area"))
	{				
		OutputMsg(rmError,_T("Scene Config Open area table error!sceneid=%d"),sceneConf->nScenceId);
		return FALSE;
	}
	
	sceneConf->vAreaList.nCount = (int)lua_objlen(m_pLua, -1) + 1;

	if (sceneConf->vAreaList.nCount > 0)
	{
		int nSize = sizeof(SCENEAREA)*sceneConf->vAreaList.nCount;
		sceneConf->vAreaList.pList = (SCENEAREA*)m_DataAllocator.allocObjects(nSize);
		ZeroMemory(sceneConf->vAreaList.pList,nSize);

		//第0个默认属性
		SCENEAREA* area = sceneConf->vAreaList.pList;
		ZeroMemory(area,sizeof(SCENEAREA));
		area->boInherit = false;
		area->szName[0] = 0;
		area->NoTips = (byte) 1;

		// 默认全局区域
		{
			CAbstractMap* pMap = &((SCENEMAPCONF*)sceneConf)->m_MapData;
			
			area->nPointCount = 4;
			area->lpPoints= (LPPOINT)m_DataAllocator.allocObjects(sizeof(area->lpPoints[0])*4);
			area->lpPoints[0].x = 0;
			area->lpPoints[0].y = 0;
			area->lpPoints[1].x = pMap->m_dwWidth;
			area->lpPoints[1].y = 0;
			area->lpPoints[2].x = pMap->m_dwWidth;
			area->lpPoints[2].y = pMap->m_dwHeight;
			area->lpPoints[3].x = 0;
			area->lpPoints[3].y = pMap->m_dwHeight;
		}

		// 默认全局区域中心点
		{
			area->Center[0] = sceneConf->nDefaultX;
			area->Center[1] = sceneConf->nDefaultY;
		}

		// 默认全局区域无属性
		area->attri[0].bType = 0;
		area->attri[0].nCount = 0;
	}
	else
	{
		OutputMsg(rmTip,_T("Scene area config is NULL!sceneid=%d"),sceneConf->nScenceId);
		sceneConf->vAreaList.pList = NULL;
		closeTable();
		return TRUE;
	}
	int nAreaPos = 1;
	if (enumTableFirst())
	{
		do 
		{		
			SCENEAREA* area = sceneConf->vAreaList.pList + nAreaPos;
			ZeroMemory(area,sizeof(SCENEAREA));
			nAreaPos++;

			area->szName[0] = 0;
			getFieldStringBuffer("name",area->szName,ArrayCount(area->szName));
			int nDef = 0;
			area->NoTips = (byte)getFieldInt("notips", &nDef);
			if (area->szName[0] == 0)
			{
				OutputMsg(rmError,_T("Scene area name is NULL!sceneid=%d,areapos=%d"),sceneConf->nScenceId,nAreaPos);
				return false;
			}
			if (nAreaPos == 0)
			{
				area->boInherit = false;
			}
			else
			{
				bool boDefault = true;
				area->boInherit = getFieldBoolean("inherit",&boDefault);
			}
			
			if(! LoadAreaRangeConfig(sceneConf, area))
			{
				return false;
			}

			if (!LoadAreaCenterConfig(sceneConf, area))
			{
				return false;
			}

		} while (enumTableNext());
	}
	closeTable();

	if (sceneConf->vAreaList.nCount <= 0)
	{
		OutputMsg(rmError,"load scene config error!range is NULL!sceneid=%d",sceneConf->nScenceId);
		return false;
	}
	return true;
}
bool CSceneConfigLoader::LoadAreaRangeConfig(SCENECONFIG* sceneConf, SCENEAREA* area)
{
	if ( !openFieldTable("range") )
	{
		OutputMsg(rmError,_T("Scene Config Open range table error!sceneid=%d"),sceneConf->nScenceId);
		return false;
	}
	area->nPointCount = (int)lua_objlen(m_pLua, -1);
	if (area->nPointCount < 0 || area->nPointCount % 2 != 0)
	{
		OutputMsg(rmError,_T("scene config area error!Range Point Count Error!sceneid=%d,count=%d"),sceneConf->nScenceId,area->nPointCount);
		return false;
	}

	if (area->nPointCount == 0)
	{
		CAbstractMap* pMap = &((SCENEMAPCONF*)sceneConf)->m_MapData;
		area->nPointCount = 4;
		area->lpPoints= (LPPOINT)m_DataAllocator.allocObjects(sizeof(area->lpPoints[0])*4);
		area->lpPoints[0].x = 0;
		area->lpPoints[0].y = 0;
		area->lpPoints[1].x = pMap->m_dwWidth;
		area->lpPoints[1].y = 0;
		area->lpPoints[2].x = pMap->m_dwWidth;
		area->lpPoints[2].y = pMap->m_dwHeight;
		area->lpPoints[3].x = 0;
		area->lpPoints[3].y = pMap->m_dwHeight;
	}
	else
	{
		area->nPointCount = area->nPointCount / 2;
		if (area->nPointCount > 0)
		{
			area->lpPoints= (LPPOINT)m_DataAllocator.allocObjects(sizeof(area->lpPoints[0])*area->nPointCount);
		}
		int index = 0;
		if (enumTableFirst())
		{
			do 
			{
				area->lpPoints[index].x = getFieldInt(NULL);
				enumTableNext();
				area->lpPoints[index].y = getFieldInt(NULL);
				index++;
			} while (enumTableNext());
		}
	}
	
	closeTable();
	return true;
}
bool CSceneConfigLoader::LoadAreaCenterConfig(SCENECONFIG* sceneConf, SCENEAREA* area)
{
	if ( !openFieldTable("center") )//获取场景列表
	{
		OutputMsg(rmError,_T("Scene Config Open center table error!sceneid=%d"),sceneConf->nScenceId);
		return false;
	}
	size_t count = lua_objlen(m_pLua, -1);
	if (count != 2)
	{
		OutputMsg(rmError,"load scene config error!range center config error!sceneid=%d",sceneConf->nScenceId);
		return false;
	}
	count = 0;
	if (enumTableFirst())
	{
		do
		{
			area->Center[count++] = getFieldInt(NULL);
		} while (enumTableNext());
	}
	closeTable();
	return true;
}
bool CSceneConfigLoader::LoadNpcConfig( SCENECONFIG * sceneConf )
{
	if (!openFieldTable("npc"))
	{
		return FALSE;
	}
	size_t nItemCount = lua_objlen(m_pLua, -1);
	sceneConf->NpcList.nCount = (int)nItemCount;
	if (nItemCount > 0)
	{
		INT_PTR nSize = sizeof(NPCPOS)*nItemCount;
		sceneConf->NpcList.pPosList = (NPCPOS*)m_DataAllocator.allocObjects(nSize);
		ZeroMemory(sceneConf->NpcList.pPosList,nSize);
		
		if (enumTableFirst())
		{
			int idx = 0;
			int nDef = 0;
			do 
			{
				NPCPOS* pNpc = sceneConf->NpcList.pPosList + idx;
				pNpc->nId = getFieldInt("id", &nDef);
				if (pNpc->nId == 0)
				{
					OutputMsg(rmError, _T("场景%d配置的npcID为0"), sceneConf->nScenceId);
				}
				pNpc->nPosX = getFieldInt("posx");
				pNpc->nPosY = getFieldInt("posy");
				idx++;
			} while (enumTableNext());
		}
	}
	closeTable();
	return true;
}
bool CSceneConfigLoader::LoadTelePortConfig(SCENECONFIG* sceneConf)
{
	if (!openFieldTable("teleport"))
	{
		return FALSE;
	}
	size_t nItemCount = lua_objlen(m_pLua, -1);
	sceneConf->TeleportList.nCount = (int)nItemCount;
	if (nItemCount > 0)
	{
		INT_PTR nSize = sizeof(TeleportConfig)*nItemCount;
		sceneConf->TeleportList.pList = (TeleportConfig*)m_DataAllocator.allocObjects(nSize);
		ZeroMemory(sceneConf->TeleportList.pList,nSize);

		if (enumTableFirst())
		{
			int idx = 0;
			do 
			{
				TeleportConfig* pTele = sceneConf->TeleportList.pList + idx;
				pTele->nSceneId = getFieldInt("toSceneid");//toSceneid
				pTele->nPosX = getFieldInt("posx");
				pTele->nPosY = getFieldInt("posy");
				pTele->nToPosX = getFieldInt("toPosx");
				pTele->nToPosY = getFieldInt("toPosy");
				pTele->nModelId = getFieldInt("modelid");
				
				bool boUsed = true;
				pTele->bUse = getFieldBoolean("used", &boUsed);			//默认是可用的
				int nValue = 0;
				pTele->nPassId = getFieldInt("passid",&nValue);
				pTele->nOpendayLimit = getFieldInt("opendayLimit",&nValue);
				pTele->nToPosRadius = getFieldInt("toPosRadius", &nValue);//
				nValue = 1;
				pTele->nDist = getFieldInt("dist",&nValue);
				getFieldStringBuffer("name",pTele->szName,ArrayCount(pTele->szName));
				idx++;
			} while (enumTableNext());
		}
	}
	closeTable();
	return true;
}
bool CSceneConfigLoader::LoadHookConfig(SCENECONFIG* sceneConf)
{
	if (feildTableExists("hook") && openFieldTable("hook"))
	{
		size_t nItemCount = lua_objlen(m_pLua, -1);
		sceneConf->hookList.nCount = (int)nItemCount;
		if (nItemCount > 0)
		{
			INT_PTR nSize = sizeof(NPCPOS)*nItemCount;
			sceneConf->hookList.pPosList = (NPCPOS*)m_DataAllocator.allocObjects(nSize);
			ZeroMemory(sceneConf->hookList.pPosList,nSize);

			if (enumTableFirst())
			{
				int idx = 0;
				do 
				{
					NPCPOS* pHookPos = sceneConf->hookList.pPosList + idx;
					pHookPos->nId = idx;
					pHookPos->nPosX = getFieldInt("x");
					pHookPos->nPosY = getFieldInt("y");
					idx++;
				} while (enumTableNext());
			}
		}
		closeTable();
	}
	return true;
}

// 对外接口
SCENEMAPCONF* CSceneConfigLoader::GetSceneConfig(int nSceneId)
{
	if (nSceneId < 0)
		return NULL;

	INT_PTR nSceneCount = m_sceneConfigList.nCount;
	if (nSceneId >= nSceneCount) return NULL;

	for (int i =0; i < m_sceneConfigList.nCount; i++)
	{
		if (m_sceneConfigList.pList[i].nScenceId == nSceneId)
		{
			return &m_sceneConfigList.pList[i];
		}
	}
	return NULL;
}
bool CSceneConfigLoader::IsTeleport(const int nSceneId, const int nX, const int nY)
{
	SCENECONFIG* pSC = GetSceneConfig(nSceneId);
	Assert(pSC);
	INT_PTR nTelCount = pSC->TeleportList.nCount;
	for (INT_PTR i = 0; i < nTelCount; i++)
	{
		TeleportConfig& tc = pSC->TeleportList.pList[i];
		if (nX == tc.nPosX && nY == tc.nPosY)
			return true;
	}
	
	return false;
}
bool CSceneConfigLoader::CanMove(const int nSceneId, const int nX, const int nY)
{
	SCENEMAPCONF* pSC = (SCENEMAPCONF*)GetSceneConfig(nSceneId);
	Assert(pSC);
	return pSC->m_MapData.canMove(nX, nY);	
}
void CSceneConfigLoader::GetSceneDefaultPoint(SCENECONFIG* sc, int& nX, int& nY)
{
	nX = nY = -1;
	if (!sc)
		return;

	SCENEAREA* sa = &sc->vAreaList.pList[0];
	nX = sa->Center[0];
	nY = sa->Center[1];
}
void CSceneConfigLoader::GetRandomRefreshPos(const int nSceneId, int& nX, int& nY)
{
	SCENECONFIG* pSceneConfig = GetSceneConfig(nSceneId);
	Assert(pSceneConfig);
	INT_PTR nRefreshPtCount = pSceneConfig->vRefreshList.nCount;
	Assert(nRefreshPtCount > 0);
	unsigned long nRandRefreshPtIdx = wrand((unsigned long)nRefreshPtCount);	
	REFRESHCONFIG* pRefreshCfg = &pSceneConfig->vRefreshList.pList[nRandRefreshPtIdx];
	
	nX = pRefreshCfg->nMobX;
	nY = pRefreshCfg->nMobY;
	//OutputMsg(rmNormal, _T("rand refresh pt:%d, random pt idx=%d"), nRandRefreshPtIdx, nRandPtIdx);
}
bool CSceneConfigLoader::GetMonsterPos(int nSceneId, int nEntityId, int & nX, int & nY)
{
	SCENECONFIG * pSc = GetSceneConfig(nSceneId);
	if (pSc != NULL)
	{
		int nCount = pSc->vRefreshList.nCount;
		for (int i = 0; i < nCount; i++)
		{
			REFRESHCONFIG * pRefreshCfg = &pSc->vRefreshList.pList[i];
			if (pRefreshCfg != NULL && pRefreshCfg->nEntityId == nEntityId)
			{
				nX = pRefreshCfg->nMobX;
				nY = pRefreshCfg->nMobY;
				return true;
			}
		}
		
	}
	return false;
}