#include "StdAfx.h"
#include "MonsterConfigCacher.h"

const FILEIDENT CMonsterConfigCacher::CacheFileIdent = { MAKEFOURCC('M', 'S', 'C', 0) };
const FILEVERSION CMonsterConfigCacher::CacheVersion = { MAKEFOURCC(1, 11, 3, 5) };

bool CMonsterConfigCacher::ValidateHeader(const CONF_DATA_CACHE_HDR& hdr)
{
	//验证文件标识
	if (hdr.Ident.uIdent != CacheFileIdent.uIdent)
		return false;
	//验证文件版本
	if (hdr.Version.uVersion != CacheVersion.uVersion)
		return false;
	//验证MonsterConfig结构大小
	if (sizeof(MONSTERCONFIG) != hdr.dwSizeElement)
		return false;
	return true;
}

bool CMonsterConfigCacher::AdjustElementPointers(LPVOID lpElement)
{
	MONSTERCONFIG *pMonConf = (PMONSTERCONFIG)lpElement;
#define AdjustPtr(ptr, T) if (ptr && !(ptr = (T)GetNewMemoryPtr(ptr))) return false;
#define AdjustPtrArray(a, name, n, T) for (i=n-1; i>-1; --i) if ( (a)[i].name && !((a)[i].name = (T)GetNewMemoryPtr((a)[i].name)) ) return false;

	AdjustPtr(pMonConf->drops.data.pData, PONEDROPDATA);	
	/*for (INT_PTR i = 0; i < mstMax; i++)
	{
		MonsterSayInfo &info = pMonConf->monsterSayList.vecSayInfo[i];
		if (info.nCount > 0)
		{
			AdjustPtr(info.pMsgList, LPSTR*);
			for (INT_PTR j = 0; j < info.nCount; j++)
			{				
				AdjustPtr(info.pMsgList[j], LPSTR);
			}
		}
	}*/
	
	for (INT_PTR i = 0; i < mstMax; i++)
	{
		MonsterSayTypeConfig& sayTypeCfg = pMonConf->monsterSayList.vecSayTypeInfo[i];
		if (sayTypeCfg.nCount > 0)
		{
			AdjustPtr(sayTypeCfg.pMonsterSayCfg, MonsterSayInfo*);
			MonsterSayInfo* pMonsterSayCfg = sayTypeCfg.pMonsterSayCfg;
			for (INT_PTR j = 0; j < sayTypeCfg.nCount; j++)
			{
				MonsterSayInfo* pInfo = &pMonsterSayCfg[j];
				AdjustPtr(pInfo->pMsgList, LPSTR*);
				LPSTR* pMsgList = pInfo->pMsgList;
				for (INT_PTR k = 0; k < pInfo->nCount; k++)
				{
					AdjustPtr(pMsgList[k], LPSTR);
				}
			}
		}
	}

	PriorAttackTarget &targetInfo = pMonConf->priorAttackTarget;
	if (targetInfo.nCount > 0)
	{
		AdjustPtr(targetInfo.pTargetIDList, int *);
	}
	

#undef AdjustPtr
#undef AdjustPtrArray
	return true;
}

void CMonsterConfigCacher::FillHeaderData(CONF_DATA_CACHE_HDR& hdr)
{
	hdr.Ident.uIdent = CacheFileIdent.uIdent;
	hdr.Version.uVersion = CacheVersion.uVersion;
	hdr.dwSizeElement = sizeof(MONSTERCONFIG);
}
