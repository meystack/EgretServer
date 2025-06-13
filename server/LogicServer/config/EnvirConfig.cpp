#include "StdAfx.h"
#include "EnvirConfig.h"

CBufferAllocator* CEnvirConfig::m_pAllocator = NULL;

typedef int (*compfn)(const void*, const void*);

int  compare(int* q1, int* q2)
{
	if ( *q1 < *q2)
		return -1;
	else if (*q1 > *q2)
		return 1;
	else
		return 0;
}

CEnvirConfig::CEnvirConfig() : m_DataAllocator(_T("EnvirConfigAlloc"))
{

}
CEnvirConfig::~CEnvirConfig()
{
	
}

bool CEnvirConfig::LoadConfig(LPCTSTR sFileName)
{
	bool result = TRUE;

	LPCTSTR sResult = NULL;
	wylib::stream::CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//OutputMsg(rmTip,_T("开始装载配置"));
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		if ( ms.loadFromFile(sFileName) > 0 )
		{
			sResult = pp.parse((LPCTSTR)ms.getMemory(), sFileName);
		}
		if (sResult ==NULL)
		{
			OutputMsg(rmError,_T("预处理文件%s失败"),sFileName);

			result = FALSE;
		}
		if ( !setScript(sResult) )
		{
			OutputMsg(rmError,_T("读取预处理后的配置文件失败"));
			result = FALSE;
		}
		if (result)
		{
			//OutputMsg(rmTip,_T("装载配置ok"));

			if (LoadSceneConfig())
			{
				//OutputMsg(rmTip,_T("装载地图场景配置ok！"));
			}
			else
			{
				//OutputMsg(rmError,_T("装载地图场景配置失败！"));
				result = FALSE;
			}
			m_pAllocator->CheckFreeBuffers(true); //强制把一些没必要的内存释放掉
		}
		
	}	
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Envir config error: %s"), s.rawStr());
		result = FALSE;
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sResult, fp);
			fclose(fp);
		}
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Envir config"));
		result = FALSE;
	}
	//销毁脚本虚拟机
	setScript(NULL);
	m_pAllocator->CheckFreeBuffers(true);

	return result;
}
	
bool CEnvirConfig::LoadSceneConfig()
{
	//场景配置表
	int			 s_nSceneCount = 0;
	SCENECONFIG* s_pSceneConfig = NULL;
	if (openGlobalTable("Scenes"))
	{
		s_nSceneCount = (int)lua_objlen(m_pLua, -1);

		if (s_nSceneCount > 0)
		{
			int nSize = sizeof(SCENECONFIG) * s_nSceneCount;
			s_pSceneConfig = (SCENECONFIG*)m_DataAllocator.allocObjects(nSize);
			ZeroMemory(s_pSceneConfig,nSize);
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
				
				if (nFuBenScenePos >= s_nSceneCount || nFuBenScenePos < 0)
				{
					OutputMsg(rmError,_T("load scene id error!nSceneId=%d"),nSceneId);
					return false;
				}
				
				SCENECONFIG* sceneConf = s_pSceneConfig + nFuBenScenePos;
				sceneConf->nScenceId = nSceneId;

				int nDef =0;
				bool boDef = false;
				bool bMaxAnger = false;
				bool bBroadcast = false;
				sceneConf->nDefaultX = getFieldInt("defaultX",&nDef);
				sceneConf->nDefaultY = getFieldInt("defaultY",&nDef);
				sceneConf->boCanCall = getFieldInt("iscall", &nDef);

				//场景的类型
				sceneConf->bSceneType = getFieldInt("sceneType",&nDef);
				sceneConf->nMapGgroup = getFieldInt("mapGgroup",&nDef);
				sceneConf->bIsNoPickUp = getFieldInt("isNoPickUp",&nDef);
				getFieldStringBuffer("scencename",sceneConf->szScenceName,ArrayCount(sceneConf->szScenceName));
				getFieldStringBuffer("mapfilename",sceneConf->szMapFileName,ArrayCount(sceneConf->szMapFileName));			

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

			} while (enumTableNext());
		}
	} else return FALSE;

	//GOODS_TAG DefConsumeList[3]={ {-1,-1,-1},{-1,-1,-1},{-1,-1,-1} };

	//副本配置表
	if (openGlobalTable("StaticFubens"))
	{
		int nDef_0 = 0;
		int nDef_1 = 1;
		int nDef_f1 = -1;
		bool bDef_true = false;
		int nCount = (int)lua_objlen(m_pLua,-1);
		//读取每个副本的配置
		for (size_t i = 0; i <= nCount; i++)
		{
			if (!getFieldIndexTable(i)) return false;

			COMMONFUBENCONFIG* fuben = (COMMONFUBENCONFIG*)m_DataAllocator.allocObjects(sizeof(COMMONFUBENCONFIG));

			fuben->nFbId = getFieldInt("fbid");															//副本ID
			fuben->nFbConfType  =  getFieldInt("fbConfType",&nDef_0);									//副本类型
			memset(fuben->fbName, 0, sizeof(fuben->fbName));
			getFieldStringBuffer(("fbname"),fuben->fbName,sizeof(fuben->fbName));
			fuben->nFbEnterType  =  getFieldInt("enterType",&nDef_0);									//副本进入类型
			fuben->nRecommandLevel = getFieldInt("recommandLevel",&nDef_1);								//推荐等级
			fuben->bDisableMonsterPropAdjust = getFieldBoolean("disableMonsterPropAdjust", &bDef_true); //是否禁止动态更新怪物属性
			fuben->nDestoryTime = getFieldInt("destoryTime",&nDef_0);									//当副本没人时销毁时间
			fuben->nDefSceneId = getFieldInt("defSceneID",&nDef_0);										//进入默认场景id
			memset(fuben->ConsumeList, -1, sizeof(fuben->ConsumeList));
			if (feildTableExists("consumeList") && openFieldTable("consumeList"))
			{
				if (enumTableFirst())
				{
					int i = 0;
					do 
					{
						if (i >= sizeof(fuben->ConsumeList)/sizeof(GOODS_TAG)) break;
						
						fuben->ConsumeList[i].type = getFieldInt("type", &nDef_f1);
						fuben->ConsumeList[i].id = getFieldInt("id", &nDef_f1);
						fuben->ConsumeList[i].count = getFieldInt("count", &nDef_f1);
						i++;
					} while (enumTableNext());
				}
				closeTable();
			}
			
			CFuBenManager* fbMgr = GetGlobalLogicEngine()->GetFuBenMgr();
			CFuBenHandle hHandle;
			CFuBen* pFuBen = CFuBenManager::m_FuBenMgr->Acquire(hHandle);
			fbMgr->AddStaticFuBen(pFuBen);
			pFuBen->SetFbId(fuben->nFbId);
			pFuBen->SetHandle(hHandle);//
			pFuBen->SetConfig(fuben);

			//读入场景信息
			pFuBen->m_vSceneConfig = (SceneConfigList*)m_DataAllocator.allocObjects(sizeof(SceneConfigList));
			ZeroMemory(pFuBen->m_vSceneConfig,sizeof(SceneConfigList));
			
			//第一个副本，含括所有场景
			if (fuben->nFbId == 0)
			{
				pFuBen->m_vSceneConfig->nCount = s_nSceneCount;
				pFuBen->Init();
				if (pFuBen->m_vSceneConfig->nCount > 0)
				{
					int nSize = sizeof(pSCENECONFIG) * pFuBen->m_vSceneConfig->nCount;
					pFuBen->m_vSceneConfig->ppList = (SCENECONFIG**)m_DataAllocator.allocObjects(nSize);
					ZeroMemory(pFuBen->m_vSceneConfig->ppList, nSize);
				}
				
				for (size_t i = 0; i < pFuBen->m_vSceneConfig->nCount; i++)
				{
					//设置指针
					SCENECONFIG* sceneConf = s_pSceneConfig + i;
					*(pFuBen->m_vSceneConfig->ppList + i) = sceneConf;

					//创建场景实例
					if (!pFuBen->AddStaticScene(sceneConf,fbMgr,m_DataAllocator))
					{
						OutputMsg(rmError, _T("load fuben error!fubenid=%d"), fuben->nFbId);
						return FALSE;
					}
				}
			}
			//其他副本
			else
			{
				if (openFieldTable("scenes"))
				{
					pFuBen->m_vSceneConfig->nCount = (int)lua_objlen(m_pLua, -1);
					pFuBen->Init();
					if (pFuBen->m_vSceneConfig->nCount > 0)
					{
						int nSize = sizeof(pSCENECONFIG) * pFuBen->m_vSceneConfig->nCount;
						pFuBen->m_vSceneConfig->ppList = (SCENECONFIG**)m_DataAllocator.allocObjects(nSize);
						ZeroMemory(pFuBen->m_vSceneConfig->ppList, nSize);
					}

					if (enumTableFirst())
					{
						int i = 0;
						do {
							int sceneid = getFieldInt(NULL);
							int nFuBenScenePos = sceneid - 1;
							if (nFuBenScenePos >= s_nSceneCount || nFuBenScenePos < 0)
							{
								OutputMsg(rmError, _T("load fuben error!fubenid=%d,scene=%d"), fuben->nFbId, sceneid);
								return FALSE;
							}

							if (fuben->nDefSceneId == 0)
							{
								fuben->nDefSceneId = sceneid;
							}
							
							//设置指针
							SCENECONFIG* sceneConf = s_pSceneConfig + nFuBenScenePos;
							*(pFuBen->m_vSceneConfig->ppList + i) = sceneConf;

							//重复场景检测
							if (pFuBen->HasScene(sceneConf->nScenceId))
							{
								OutputMsg(rmError,_T("It is the Same Scene id!sceneid=%d"),sceneConf->nScenceId);
								return FALSE;
							}
							else
							{
								OutputMsg(rmTip,"load scene,id=%d",sceneConf->nScenceId);
							}

							//创建场景实例
							if (!pFuBen->AddStaticScene(sceneConf,fbMgr,m_DataAllocator))
							{
								OutputMsg(rmError, _T("load fuben error!fubenid=%d"), fuben->nFbId);
								return FALSE;
							}
							i++;
						}while (enumTableNext());
					}
					closeTable();//close scene
				} else return FALSE;
			}
			closeTable();//end StaticFubens[i]
		}

		closeTable();//close FuBen
	} else return FALSE;

	GetGlobalLogicEngine()->GetFuBenMgr()->CreateFreeFb();

	return true;
}

void CEnvirConfig::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

//读取刷怪配置
bool CEnvirConfig::LoadRefreshConfig( SCENECONFIG* sceneConf ) 
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
			/*
			int nPosX1 = getFieldInt("mapx1");
			int nPosX2 = getFieldInt("mapx2",&nPosX1);

			int nPosY1 = getFieldInt("mapy1");
			int nPosY2 = getFieldInt("mapy2",&nPosY1);

			if (nPosX1 > nPosX2 || nPosY1 > nPosY2)
			{
				OutputMsg(rmError,",sceneid=%d,point error,mapx1 > mapx2 or mapy1> mapy2, mapx1=%d,mapx2=%d,mapy1=%d,mapy2=%d",
					sceneConf->nScenceId,nPosX1,nPosX2,nPosY1,nPosY2);
				return false;
			}
			*/
			//refreshData->nEntityType = getFieldInt("entitytype");
			refreshData->nEntityId = getFieldInt("entityid");
			refreshData->nMobX = getFieldInt("x");
			refreshData->nMobY = getFieldInt("y");
			refreshData->nMobRange = getFieldInt("range");
			int nDef = 0;
			refreshData->nLiveTime = getFieldInt("livetime",&nDef);

			char sMapFullBuff[40]; //地图文件的全名
			sprintf(sMapFullBuff,"data/map/%s",sceneConf->szMapFileName);
			CAbstractMap* pMap = GetGlobalLogicEngine()->GetFuBenMgr()->GetMapData(sMapFullBuff);
			if (!pMap)
			{
				OutputMsg(rmError,"Map File Is NULL!filename=%s",sceneConf->szMapFileName);
				return false;
			}
			/*
			refreshData->nPointsCount = 0;
			
			refreshData->Points = (MAPPOINT*)m_DataAllocator.allocObjects(sizeof(MAPPOINT)*(nPosX2-nPosX1+1) * (nPosY2-nPosY1+1) );
			for (int x = nPosX1; x <= nPosX2; x++)
			{
				for(int y=nPosY1; y <= nPosY2; y ++ )
				if (pMap->canMove((DWORD)x, (DWORD)y))
				{
					MAPPOINT p;
					p.x = (int)x;
					p.y = (int)y;
					refreshData->Points[refreshData->nPointsCount] = p;
					refreshData->nPointsCount++;
				}
			}
			if (refreshData->nPointsCount <= 0)
			{
				//全部刷怪点都无效
				OutputMsg(rmError,"Monster Refresh Config Error!filename=%s,sceneid=%d,refresh Point=%d,x=%d,y=%d",sceneConf->szMapFileName,sceneConf->nScenceId,refreshPos,(int)nPosX1,(int)nPosX2);
				return false;
			}
			*/
			refreshPos++;
		} while (enumTableNext());
	}
	closeTable();//close Refresh
	return true;
}

//读区域配置
bool CEnvirConfig::LoadAreasConfig( SCENECONFIG* sceneConf )
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
			char sMapFullBuff[40]; //地图文件的全名
			sprintf(sMapFullBuff,"data/map/%s",sceneConf->szMapFileName);
			CAbstractMap* pMap = GetGlobalLogicEngine()->GetFuBenMgr()->GetMapData(sMapFullBuff);
			if (!pMap)
			{
				OutputMsg(rmError,"Map File Is NULL!filename=%s",sceneConf->szMapFileName);
				return false;
			}
			
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
		sceneConf->vAreaList.pList = nullptr;
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
			if(!LoadAreaAttriConfig(sceneConf, area))
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

bool CEnvirConfig::LoadAreaRangeConfig( SCENECONFIG* sceneConf, SCENEAREA* area )
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
		char sMapFullBuff[40]; //地图文件的全名
		sprintf(sMapFullBuff,"data/map/%s",sceneConf->szMapFileName);
		CAbstractMap* pMap = GetGlobalLogicEngine()->GetFuBenMgr()->GetMapData(sMapFullBuff);
		if (!pMap)
		{
			OutputMsg(rmError,"Map File Is NULL!filename=%s",sceneConf->szMapFileName);
			return false;
		}
		
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
				if (index >= area->nPointCount) break;
				
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

bool CEnvirConfig::LoadAreaAttriConfig( SCENECONFIG* sceneConf, SCENEAREA* area  )
{

	if (area->boInherit)//继承第一个区域的属性
	{
		SCENEAREA* pFirst = sceneConf->vAreaList.pList;
		for (INT_PTR i = 0; i < aaAttriCount; i++)
		{
			area->attri[i].bType = pFirst->attri[i].bType;
			INT_PTR nCount = area->attri[i].nCount = pFirst->attri[i].nCount;
			area->attri[i].pValues = pFirst->attri[i].pValues;
			if (nCount > 0)
			{
				INT_PTR nSize = sizeof(int)*nCount;
				area->attri[i].pValues = (int*)m_DataAllocator.allocObjects(nSize);
				memcpy(area->attri[i].pValues,pFirst->attri[i].pValues,nSize);
			}
		}
	}

	if (!openFieldTable("attri"))
	{
		OutputMsg(rmError,_T("Scene Config Open attri table error!sceneid=%d"),sceneConf->nScenceId);
		return true;
	}
	if (enumTableFirst())
	{
		do 
		{
			int type = getFieldInt("type");
			if (type < 0 || type >= aaAttriCount)
			{
				OutputMsg(rmError,_T("scene config error:attri type error!type=%d,sceneid=%d"),type,sceneConf->nScenceId);
				return false;
			}
			area->attri[type].bType = type;
			if (!openFieldTable("value"))
			{
				OutputMsg(rmError,_T("scene config error:attri table error!sceneid=%d"),sceneConf->nScenceId);
				return false;
			}

			area->attri[type].nCount = (int)lua_objlen(m_pLua,-1);
			if (area->attri[type].nCount > 0)
			{
				area->attri[type].pValues = (int*)m_DataAllocator.allocObjects(sizeof(int)*area->attri[type].nCount);
			}
			int index = 0;
			if (enumTableFirst())
			{
				do 
				{
					area->attri[type].pValues[index++] = getFieldInt(NULL);
				} while (enumTableNext());
			}
			closeTable();
			//检查参数个数对不对
			if (!checkParam(area,type))
			{
				OutputMsg(rmError,_T("scene area config error:attri param count error!sceneid=%d,type=%d"),sceneConf->nScenceId,type);
				return false;
			}
			

		} while (enumTableNext());
	}
	closeTable();

	//统计一下区域属性的数量
	ZeroMemory(area->attrBits,sizeof(area->attrBits));
	for (INT_PTR i = 0; i < aaAttriCount; i++)
	{
		if (area->attri[i].bType == i)
		{
			INT_PTR nIndex = i / 32;
			INT_PTR nBit = i % 32;
			area->attrBits[nIndex] |= (1 << nBit);
		}
	}
	return true;
}

bool CEnvirConfig::LoadAreaCenterConfig( SCENECONFIG* sceneConf, SCENEAREA* area )
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

bool CEnvirConfig::checkParam( SCENEAREA* pArea,INT_PTR nType )
{
	if ((nType < 0) || (nType >= aaAttriCount)) return false;
	
	AreaAttri& attri = pArea->attri[nType];
	bool boSort = true;//参数是否需要排序，默认是排序
	switch(nType)
	{
	case aaAddBuff:
	case aaLeaveDelBuf:
		{
			boSort = false;
			if ( attri.nCount <= 0) return false;
			INT_PTR nMaxBuffCount = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuffCount();
			for (INT_PTR i = 0; i < attri.nCount; i++)//增加N个buff
			{
				int nBuffId = attri.pValues[i];
				if (nBuffId <= 0 || nBuffId > (int)nMaxBuffCount)
				{
					return false;
				}
			}
			break;
		}
	case aaForcePkMode:
	case aaSceneMaxLevel:
	case asSceneAreaMode:
		{
			if (attri.nCount != 1) return false;//需要且只需要1个参数
			break;
		}
	case aaSaftRelive:
		{
			boSort = false;
			break;
		}
	case aaSceneLevel://需要多个参数
		{
			if ( attri.nCount < 1 )
			{
				return false;
			}
			boSort = false;
			break;
		}
	case aaChangeModel:
	case aaChangeWeapon:
		{
			if (attri.nCount != 2)
			{
				return false;
			}
			boSort = false;
			break;
		}
	}
	//从小到大排序
	if (boSort && attri.nCount > 0)
		qsort(attri.pValues,attri.nCount,sizeof(*(attri.pValues)),(compfn)compare);
	return true;
}

//读取Npc
bool CEnvirConfig::LoadNpcConfig( SCENECONFIG* sceneConf ) 
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

//读取传送点
bool CEnvirConfig::LoadTelePortConfig( SCENECONFIG* sceneConf )
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
