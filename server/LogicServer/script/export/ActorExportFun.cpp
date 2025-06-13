#include "StdAfx.h"
#include "../interface/ActorExportFun.h"

namespace Actor
{
	
	/*
	* Comments:是否是第1次登陆游戏
	* Param void * pEntity:玩家的指针
	* @Return bool:如果是第1次登陆返回true，否则返回false
	*/
	bool isFirstLogin(void * pEntity)
	{
		if(!pEntity) return false;
		
		if(((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->m_isFirstLogin;
	}

	int getActorCreateTime(void* pEntity)
	{
		if(!pEntity) return 0;
		
		if(((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetCreateTime();
	}


	void sendLearnSkill(void * pEntity)
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->GetSkillSystem().SendLearnSkill();
	}
	void * getEntity(double handle)
	{
		unsigned long long d;
		memcpy(&d,&handle ,sizeof(handle));

		EntityHandle hHandle(d);
		return GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(hHandle);
	}

	void* getActorByName(const char* szActorName)
	{		
		return GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(szActorName);
	}
	void* getActorById(unsigned int nActorID)
	{		
		return GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorID);
	}
	const char * getAccount(void* pEntity)
	{
		if (pEntity == NULL) return NULL;
		if(((CEntity *)pEntity)->GetType() != enActor) return NULL;
		return ((CActor*)pEntity)->GetAccount();
	}
	const char * getName(void * pEntity)
	{
		if(pEntity ==NULL)return NULL;
		
		return ((CEntity*)pEntity)->GetEntityName();
	}

	double getHandle( void* pEntity )
	{
		if (pEntity == NULL ) return 0;
		double d;
		memcpy(&d, &(((CEntity*)pEntity)->GetHandle()),sizeof(d));
		return d;
	}

	double getKillHandle(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->isAnimal() ==false) return 0;
		double d;
		memcpy(&d, &(((CAnimal*)pEntity)->GetKillHandle()),sizeof(d));
		return d;
	}

	int getEntityType(void* pEntity)
	{
		if (!pEntity) return -1;
		return ((CEntity *)pEntity)->GetHandle().GetType();
	}

	void showNpcShop(void* pEntity, int nType, int nTab)
	{
		if(pEntity ==NULL)return;
		if (((CEntity*)pEntity)->GetType() == enActor)
		{
			((CActor*)pEntity)->GetStoreSystem().ShowNpcShop(nType,nTab);
		}
	}

	bool hasQuest(void * pEntity,int nQuestID)
	{
		if(!pEntity) return false;
		if(((CEntity *)pEntity)->GetType() != enActor) return false;
		if (((CActor*)pEntity)->GetQuestSystem()->HasQuest(nQuestID) >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}


	void ExOnQuestEvent(void * pEntity,int nQuestType, int nValue, int nId)
	{
		if(!pEntity) return;
		if(((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetQuestSystem()->OnQuestEvent(nQuestType, nValue, nId);
	}

	void closeNPCDialog(void * pEntity)
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		CNpc* pNpc = ((CActor*)pEntity)->GetTalkNpc();
		if (pNpc)
		{
			pNpc->CloseDialog(((CActor*)pEntity));
		}
	}

	void openDialogs(void * pEntity,int nType,const char* sParam)
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->OpenDialog(nType,sParam);
	}


	void closeDialogs( void * pEntity,int nType )
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->CloseDialog(nType);
	}

	int getMenPai(void * pEntity)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return  (int)(((CActor*)pEntity)->GetMenPai());
	}

	int getExpLastLogout(void *pEntity)
	{
		if(!pEntity) return 0;
		if (((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetExpLastLogout();
	}

	int getZyContriLogout(void *pEntity)
	{
		if(!pEntity) return 0;
		if (((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetZyContriLogout();
	}

	bool setSocialMask(void *pEntity,int nMaskID,bool flag)
	{
		if(!pEntity) return 0;
		if (((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->SetSocialMask(nMaskID,flag);
	}

	bool getSocialMask(void *pEntity,int nMaskID)
	{
		if(!pEntity) return 0;
		if (((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetSocialMask(nMaskID);
	}

	bool enterFuBen(void * pEntity,unsigned int hFb,int nSceneId,int x,int y)
	{
		if(pEntity ==NULL ) return FALSE;
		if(( (CEntity *)pEntity)->GetType() != enActor) return FALSE;
		CFuBenManager* pFbMgr = GetGlobalLogicEngine()->GetFuBenMgr();
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		return pFbMgr->EnterFuBen( (CEntity *)pEntity,hFb,nSceneId,x,y);
	}

	bool EnterActivityScene(void* pEntity,int nSceneId,int x, int y, int nWeight, int nHeight,int nNpcId)
	{
		if(pEntity ==NULL ) return FALSE;
		//if(((CEntity *)pEntity)->GetType() != enActor) return FALSE;
		CFuBen* pFb = ((CEntity *)pEntity)->GetFuBen();
		const int nActivityTransmit = 2;	//活动传送
		if (pFb)
		{
			return pFb->Enter((CEntity *)pEntity,nSceneId,x,y,nWeight,nHeight,nActivityTransmit,nNpcId);
		}
		return FALSE;
	}

	bool enterScene( void* pEntity,int nSceneId,int x, int y, int nWeight, int nHeight,int nValue, int nParam)
	{
		if(pEntity ==NULL ) return FALSE;
		//if(((CEntity *)pEntity)->GetType() != enActor) return FALSE;
		CFuBen* pFb = ((CEntity *)pEntity)->GetFuBen();
		if (pFb)
		{
			CScene * pLastScene = ((CEntity *)pEntity)->GetScene();
			CFuBen * pLasfb = ((CEntity *)pEntity)->GetFuBen();

			if (nValue == 0)
			{
				nValue = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
			}
			
			if( pFb->Enter((CEntity *)pEntity,nSceneId,x,y,nWeight,nHeight,enDefaultTelePort,nValue,nParam,pLastScene,pLasfb)){
				return true ;
			}

		}
		return false;
	}



	int learnSkill(void * pEntity,int nId,int nLevel)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor*)pEntity)->GetSkillSystem().LearnSkill(nId,nLevel));
	}

	int getSkillLevel(void * pEntity,int nId)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return (int)(((CActor*)pEntity)->GetSkillSystem().GetSkillLevel(nId));
	}


	void delSkill( void * pEntity, int nId )
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetSkillSystem().ForgetSkill(nId);
	}


	int removeItem(void * pEntity,int wItemId,int btCount,
		int btQuality,int btStrong,int btBindFlag,char * sComment,int nLogID,const int nDuraOdds,bool bIncEquipBar/*, bool bFullCount*/)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		CActor* pActor = (CActor*)pEntity;
		if (pActor->GetDealSystem().IsDealing())
		{
			return 0;
		}
		CUserItemContainer::ItemOPParam ItemPara;
		ItemPara.wItemId = wItemId;
		ItemPara.btQuality  =btQuality ;
		ItemPara.btStrong = btStrong;
		ItemPara.wCount =(WORD)btCount;
		ItemPara.btBindFlag = btBindFlag;
		LPCSTR sLogStr = sComment ? sComment : _T("Script");
		
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(wItemId);
		if(pStdItem ==NULL) return 0;

		int nCount = 0;
		if (bIncEquipBar && pStdItem->m_btType < Item::itEquipMax)
		{
			nCount += (int)(pActor->GetEquipmentSystem().DeleteEquip(ItemPara,sLogStr,nLogID,nDuraOdds));
		}
		ItemPara.wCount -= (WORD)nCount;
		if (ItemPara.wCount > 0)
		{
			nCount += (int)(pActor->GetBagSystem().DeleteItem(ItemPara,sLogStr,nLogID,nDuraOdds/*, bFullCount ? pActor :nullptr*/));
		}
		
		return nCount;
		
	}

	int addItem(void * pEntity,int wItemId,int btQuality,int btStrong,int btCount,int btBindFlag, int nLeftTime, char * sComment,int nLogID, int nQualityDataIndex)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;

		CUserItemContainer::ItemOPParam ItemPara;
		ItemPara.wItemId = wItemId;
		ItemPara.btQuality  =btQuality ;
		ItemPara.wStar = (WORD)btStrong;
		ItemPara.wCount =btCount;
		ItemPara.btBindFlag = btBindFlag;
		ItemPara.nLeftTime = nLeftTime;
		//RandAttrSelector::InitSmithById(ItemPara.wItemId, ItemPara.btQuality, nQualityDataIndex, ItemPara.nSmith);	
		
		return (int)(((CActor*)pEntity)->GetBagSystem().AddItem(ItemPara, sComment ? sComment : _T("Script"), nLogID));				
	}

	bool hasEquiment(void* pEntity,const int wItemId)
	{
		if(pEntity ==NULL ) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetEquipmentSystem().FindItemByID(wItemId)?true:false;
	}

	void* getEquiment(void* pEntity,const int wItemId)
	{
		if(pEntity ==NULL ) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetEquipmentSystem().FindItemByID(wItemId);
	}

	bool DelEquiment(void* pEntity,const int wItemId)
	{
		if(pEntity ==NULL ) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetEquipmentSystem().DelItemByID(wItemId);
	}

	unsigned int getIntProperty(void * pEntity,int pid)
	{
		if(pEntity ==NULL || NULL== (CEntity*)pEntity )  return 0;
		//if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CEntity*)pEntity)->GetProperty<unsigned int>(pid);
	}

	int getIntProperty2(void * pEntity,int pid)
	{
		if(pEntity ==NULL || NULL== (CEntity*)pEntity )  return 0;
		//if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CEntity*)pEntity)->GetProperty<int>(pid);
	}

     double  getUInt64Property(void * pEntity,int pid)
	  {
		  if(pEntity ==NULL || NULL== (CEntity*)pEntity )  return 0;
		  return   (double)((CEntity*)pEntity)->GetProperty<UINT64>(pid);
	  }

	void collectOperate(void * pEntity,int nType)
	{
		if(pEntity ==NULL) return;
		if( ((CEntity *)pEntity)->GetType() == enActor)
		{
			((CActor*)pEntity)->GetPropertySystem().ResertProperty();
		}
		if(( (CEntity *)pEntity)->GetType() != enActor && ((CEntity *)pEntity)->GetType() != enMonster) return;
				
		((CAnimal*)pEntity)->CollectOperate(CEntityOPCollector::CollecteOPType(nType));
	}

	int getActorBattlePower(void * pEntity)
	{
		if(pEntity ==NULL || NULL== (CEntity*)pEntity )  return 0;
		//if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CEntity*)pEntity)->GetProperty<int>(PROP_ACTOR_BATTLE_POWER);
	}

	bool setUIntProperty(void * pEntity,int nPropID,unsigned int nValue)
	{
		if(pEntity ==NULL) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		if(nPropID == PROP_CREATURE_HP || nPropID == PROP_CREATURE_MP || nPropID == PROP_CREATURE_LEVEL|| (nPropID >=  PROP_MAX_CREATURE && nPropID <  PROP_MAX_ACTOR) )
		{			
			( (CActor*)pEntity)->SetProperty<unsigned int>(nPropID,nValue);
		}
		return true;
	}
	bool  setUInt64Property(void * pEntity,int nPropID, double dValue)
	{
		if(pEntity ==NULL) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		if(nPropID == PROP_CREATURE_HP || (nPropID >=  PROP_MAX_CREATURE && nPropID <  PROP_MAX_ACTOR) )
		{			
			UINT64  nValue= (UINT64)dValue;
			( (CActor*)pEntity)->SetProperty<UINT64>(nPropID,nValue);
		}
		return true;
	}

	void setMonsterProperty(void * pEntity,int nPropID,unsigned int nValue)
	{
		if(pEntity ==NULL) return;
		if(( (CEntity *)pEntity)->GetType() != enMonster) return;
		if(nPropID >=  PROP_ENTITY_ID && nPropID <= PROP_MONSTER_LEVEL_DIFF )
		{
			( (CEntity*)pEntity)->SetProperty<int>(nPropID,nValue);
		}
	}

	int removeItemByPtr(void * pEntity,void * pUserItem, int nCount,bool bNeedFreeMemory,char * sComment,int nWayLogID)
	{
		if(pEntity ==NULL ) return 0;
		if(( (CEntity *)pEntity)->GetType() != enActor) return 0;
		if (((CActor *)pEntity)->GetDealSystem().IsDealing())
		{
			return 0;
		}
		return (int)(((CActor*)pEntity)->GetBagSystem().DeleteItem(
			(CUserItem*)pUserItem,nCount,sComment,nWayLogID,bNeedFreeMemory));
	}

	bool isHpFull(void * pEntity)
	{
		if(pEntity ==NULL ) return true;
		if(( (CEntity *)pEntity)->GetType() != enActor) return true;
		return ( ((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_CREATURE_HP) >=
			((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) );
	}

	
	bool isMpFull(void * pEntity)
	{
		if(pEntity ==NULL ) return true;
		if(( (CEntity *)pEntity)->GetType() != enActor) return true;
		return ( ((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_CREATURE_MP) >=
			((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_CREATURE_MAXMP) );
	}

	int getSceneAreaFlag(void * pEntity)
	{
		if(pEntity ==NULL ) return true;
		if(( (CEntity *)pEntity)->GetType() != enActor) return true;

		return ((CActor*)pEntity)->GetSceneAreaFlag();
	}
	/*
	bool isEnergeFull(void * pEntity)
	{
		if(pEntity ==NULL ) return true;
		if(( (CEntity *)pEntity)->GetType() != enActor) return true;
		return ( ((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_ACTOR_ENERGY) >=
			((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_ACTOR_MAX_ENERGY) );
	}
	*/

	bool addBuffById(void * pEntity, int nBuffId)
	{
		if (pEntity == NULL || (((CEntity *)pEntity)->GetType() != enActor 
			&& ((CEntity *)pEntity)->GetType() != enMonster
			&& ((CEntity *)pEntity)->GetType() != enGatherMonster)
			)
		{
			return false;
		}

		return ((CActor *)pEntity)->GetBuffSystem()->Append(nBuffId) ? true:false;
	}


	void addBuffValueById(void * pEntity, int nBuffId, int nValue)
	{
		if (pEntity == NULL || (((CEntity *)pEntity)->GetType() != enActor 
			&& ((CEntity *)pEntity)->GetType() != enMonster
			&& ((CEntity *)pEntity)->GetType() != enGatherMonster)
			)
		{
			return;
		}

		CDynamicBuff * pBuff =  const_cast<CDynamicBuff*>( ((CActor *)pEntity)->GetBuffSystem()->Append(nBuffId));
		if(pBuff && nValue != 0)
		{

			CBuffProvider::BUFFCONFIG * pStdBuff = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
			if (pStdBuff == NULL)
				return ;
			if(pStdBuff->nType < 0 || pStdBuff->nType >= GameAttributeCount)
				return;
			if(pStdBuff->nGroup < CDynamicBuff::AnyBuffGroup || pStdBuff->nGroup >= CDynamicBuff::MaxBuffGroup)
				return;
	
			GAMEATTRVALUE gameValue;
			if( AttrDataTypes[pStdBuff->nType] == adFloat)
			{
				gameValue.fValue = (float)(nValue/100.0);
			}
			else if(AttrDataTypes[pStdBuff->nType] == adUInt) 
			{
				gameValue.uValue = (unsigned int)nValue;
			}
			else
			{
				gameValue.nValue = (int)nValue;
			}
			pBuff->value = gameValue;
		}
	}

	

	void addGuildBuff(void * pEntity,int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay, void *pGiver, int param)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			pGuild->AddGuildBuff(nBuffType,nGroupID,dValue,nTimes,nInterval,buffName,timeOverlay,pGiver,param);
		}
	}

	void delBuff(void * pEntity,int nBuffType,int nGroupID)
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->GetBuffSystem()->Remove( (GAMEATTRTYPE) nBuffType,nGroupID);
	}

	void delBuffById(void * pEntity, int nBuffId)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetBuffSystem()->RemoveById(nBuffId,false);
	}
	void RemoveGroupBuff(void * pEntity, int  nGroupStart,int nGroupEnd) 
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetBuffSystem()->RemoveGroupBuff(nGroupStart,nGroupEnd);

	}

	bool hasBuff(void * pEntity,int nType, int nGroup)
	{
		if(pEntity ==NULL ) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return  false;
		GAMEATTRTYPE buffType =(GAMEATTRTYPE) nType;
		
		return ((CActor *)pEntity)->GetBuffSystem()->Exists(buffType,nGroup);
	}

	bool hasBuffById(void * pEntity, int nBuffId)
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		if(((CActor *)pEntity)->GetBuffSystem()->GetBuffById(nBuffId) != NULL)
		{
			return true;
		}
		return false;
	}

	void changeHp(void * pEntity,int nValue)
	{
		if(pEntity ==NULL ) return ;
		//if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		//修改，支持对玩家和怪物的减血
		if (( (CEntity *)pEntity)->GetType() == enActor)
		{
			((CActor *)pEntity)->ChangeHP(nValue);
		}
		else if (( (CEntity *)pEntity)->GetType() == enMonster)
		{
			((CMonster *)pEntity)->ChangeHP(nValue);
		}
		else if (( (CEntity *)pEntity)->GetType() == enPet)
		{
			((CPet *)pEntity)->ChangeHP(nValue);
		}
	}

	
	void changeMp(void * pEntity,int nValue)
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->ChangeMP(nValue);
	}

	bool canAddItem(void * pEntity, int nItemID,int nCount,int nQuality, int nStrong ,bool isBinded)
	{
		if(pEntity ==NULL) return false;
		CUserItemContainer::ItemOPParam params;
		params.wItemId = (WORD)nItemID;
		params.wCount = (WORD)nCount;
		params.btQuality = (BYTE)nQuality;
		params.btStrong = (BYTE)nStrong;
		params.btBindFlag = (BYTE)(isBinded ? 1:0);
		return ((CActor *)pEntity)->GetBagSystem().CanAddItem(params)  ;
	}

	int getItemCount(void* pEntity, const int wItemId, 
		const int nQuality, const int nStrong,const int nBind,const int nDuraOdds, void * pNoItem,bool bIncEquipBar )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if (((CActor *)pEntity)->GetDealSystem().IsDealing())
		{
			return 0;
		}
		const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(wItemId);
		if(pStdItem ==NULL) return 0;

		int nCount = 0;
		if (bIncEquipBar && pStdItem->m_btType < Item::itEquipMax)
		{
			nCount += (int)(((CActor *)pEntity)->GetEquipmentSystem().GetEquipCountByID(wItemId,nQuality,nStrong,nBind,nDuraOdds, pNoItem));
		}
		nCount += (int)(((CActor *)pEntity)->GetBagSystem().GetItemCount(wItemId,nQuality,nStrong,nBind,nDuraOdds, pNoItem));
		
		return nCount; 
	}

	void sendTipmsg( void* pEntity , const char* sTipmsg,int nType )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->SendTipmsg(sTipmsg,nType);
	}

	void sendTipmsgWithId(void* pEntity,int	nTipmsgID,int nTipmsgType)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->SendTipmsgWithId(nTipmsgID,nTipmsgType);
	}
	void sendTipmsgWithParams(void * pEntity, int	nTipmsgID,int nTipmsgType,char * sParam1,char *sParam2,char * sParam3  )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		if(sParam1 ==NULL)
		{
			((CActor *)pEntity)->SendTipmsgWithId(nTipmsgID,nTipmsgType);
		}
		else
		{
			if(sParam2 ==NULL)
			{
				((CActor * )pEntity)->SendTipmsgFormatWithId(nTipmsgID,nTipmsgType,sParam1);
			}
			else
			{
				if(sParam3 ==NULL)
				{
					((CActor * )pEntity)->SendTipmsgFormatWithId(nTipmsgID,nTipmsgType,sParam1,sParam2);
				}
				else
				{
					((CActor * )pEntity)->SendTipmsgFormatWithId(nTipmsgID,nTipmsgType,sParam1,sParam2,sParam3);
				}
				
			}
		}
		
	}
	
	unsigned int getMoneyCount(void * pEntity, int nMoneyType)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor *)pEntity)->GetMoneyCount(eMoneyType(nMoneyType));
	}
	
	bool changeMoney(void * pEntity,int nMoneyType,double nValue,int nLogid,const char* sComment)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		if (nValue ==0) return false;
		//if (nMoneyType == mtYuanbao && nValue > 0) return false;//禁止脚本发放元宝，只允许扣除，安全性考虑  测试阶段新手礼包， 开放给脚本用！//已经不禁止了
		
		return  ((CActor*)pEntity)->ChangeMoney(nMoneyType,(INT_PTR)nValue,nLogid,0,sComment);
	}

	bool isInTeam(void* pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetTeam() != NULL;
	}

	void exitTeam(void* pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		CTeam* pMyTeam = ((CActor*)pEntity)->GetTeam();
		if (pMyTeam)
		{
			pMyTeam->DelMember(((CActor*)pEntity)->GetId());
		}
	}
	

	

	unsigned int getActorId(void* pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetId();
	}

	void addToTeam(void* pEntity,unsigned int nTeamId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		CTeam* pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamId);
		if (!pTeam) return;
		((CTeam*)pTeam)->AddMember(((CActor*)pEntity));
	}

	int getFubenId(void* pEntity)
	{
		if(pEntity == NULL) return 0;

		CFuBen* pFb = ((CEntity*)pEntity)->GetFuBen();
		if (pFb)
		{
			return pFb->GetConfig()->nFbId;
		}
		return 0;
	}


	int getSceneId( void* pEntity )
	{
		if(pEntity == NULL) return 0;

		return ((CEntity*)pEntity)->GetSceneID();
	}

	char* getSceneName( void* pEntity,int& x,int& y )
	{
		x = 0;
		y = 0;
		if(pEntity == NULL) return "";
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (pScene)
		{
			((CEntity*)pEntity)->GetPosition(x,y);
			return (char*)(pScene->GetSceneName());
		}
		return "";
	}
	
	/*
		在副本中，要退出副本，调用此方法
		如果指定 nSceneId, nPosX, nPosY，则返回到指定的场景+位置，因为如果退回场景与指定场景相同，
			则当角色先退回到原先位置，紧接着enterScene()到指定位置后，会导致角色跳跃现象（即角色处于原位置，
			点击鼠标后，闪跳到指定位置），所以需要直接返回到指定场景+位置
		如果没有指定，则返回到进入副本之前的原来位置
	*/
	void exitFuben(void* pEntity, int nSceneId, int nPosX, int nPosY)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		CFuBen* pFb =  ((CActor*)pEntity)->GetFuBen();
		if (pFb && pFb->IsFb())
		{
			// LPCSTR sSceneName = ((CActor *)pEntity)->GetScene()->GetSceneName();
			// if(CLocalSender * pSender = GetLogicServer()->GetLocalClient())
			// {
			// }
			((CActor*)pEntity)->ReturnToStaticScene(nSceneId, nPosX, nPosY);
		}else
		{
			if(nSceneId > 0 && nPosX >= 0 && nPosY >= 0)
			{
				((CActor*)pEntity)->ReturnToStaticScene(nSceneId, nPosX, nPosY);
			}
		}

		
	}

	/*
	退出副本，并回城的默认点
	*/
	void exitFubenAndBackCity( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		// CFuBen* pFb =  ((CActor*)pEntity)->GetFuBen();
		// if (pFb && pFb->IsFb())
		// {
		// 	// LPCSTR sSceneName = ((CActor *)pEntity)->GetScene()->GetSceneName();
		// 	// if(CLocalSender * pSender = GetLogicServer()->GetLocalClient())
		// 	// {
		// 	// }
		// }

		CActor *pActor = ( CActor* )pEntity;
		WORD nCityPointSceneId	= 0;
		WORD nCityPointPosX		= 0;
		WORD nCityPointPosY		= 0;
		pActor->GetCityPoint(nCityPointSceneId, nCityPointPosX, nCityPointPosY );
		((CActor*)pEntity)->ReturnToStaticScene(nCityPointSceneId, nCityPointPosX, nCityPointPosY);
	}
	/*
	* Comments: 获取技能的属性
	* Param void * pEntity:玩家的指针
	* Param int nSkillID:技能的ID
	* Param int nPropID: 属性的ID,1表示等级，2表示经验
	* @Return int: 返回技能的属性
	*/
	int getSkillProperty(void* pEntity,int nSkillID,int nPropID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(nPropID == 1)//1表示等级
		{
			((CActor *)pEntity)->GetSkillSystem().GetSkillLevel(nSkillID);
		}
		else
		{
			return 0;
		}
		return 0;
	}

	
	/*
	* Comments: 设置玩家的技能的属性
	* Param void * pEntity: 玩家的等级
	* Param int nSkillID:技能的ID
	* Param int nPropID: 属性的ID,1表示等级，2表示经验
	* Param int nValue:  新的数值
	* @Return bool: 成功返回true，否则返回false
	*/
	bool setSkillProperty(void * pEntity,int nSkillID,int nPropID,int nValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		if(nPropID == 1)//1表示等级
		{
			if (nValue ==((CActor *)pEntity)->GetSkillSystem().GetSkillLevel(nSkillID))
			{
				return false;
			}
			else
			{
				if( ((CActor *)pEntity)->GetSkillSystem().LearnSkill(nSkillID,nValue) )
				{
					return false;
				}
				else
				{
					return true;
				}
				
			}
		}
		else
		{
			return false;
		}
	}

	unsigned int getFubenHandle( void* pEntity )
	{
		if(!pEntity) return 0;
		CFuBen* pFb = ((CActor*)pEntity)->GetFuBen();
		if (pFb)
		{
			return pFb->GetHandle();
		}
		return 0;
	}


	unsigned int getSceneHandle( void* pEntity )
	{
		if(!pEntity) return 0;
		CScene* scene = ((CActor*)pEntity)->GetScene();
		if (scene)
		{
			return scene->GetHandle();
		}
		return 0;
	}

	int messageBox( void* pEntity,double hNpc,unsigned int nActorId,const char* sTitle,
		const char* sBtn1,const char* sBtn2,const char* sBtn3 ,unsigned int nTimeOut,int msgType,const char* sTip,int nIcon,int nTimeOutBtn, int msgId,int showId )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		char sFnName[MAX_BUTTON_COUNT][MAX_MSG_COUNT];	//对应的按钮
		int nButtonCount = 0;
		if (sBtn1 && *sBtn1)
		{
			_asncpytA(sFnName[nButtonCount],sBtn1);
			nButtonCount++;
		}
		if (sBtn2 && *sBtn2)
		{
			_asncpytA(sFnName[nButtonCount],sBtn2);
			nButtonCount++;
		}
		if (sBtn3 && *sBtn3)
		{
			_asncpytA(sFnName[nButtonCount],sBtn3);
			nButtonCount++;
		}
		EntityHandle handle;
		memcpy(&handle,&hNpc,sizeof(hNpc));
		return ((CActor*)pEntity)->AddAndSendMessageBox(handle,nActorId,sTitle,sFnName,nButtonCount,nTimeOut,msgType,sTip,(WORD)nIcon,nTimeOutBtn,msgId,showId);
	}

	double getNpc( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CNpc* pNpc = ((CActor*)pEntity)->GetTalkNpc();
		unsigned long long lhd= 0;
		if (pNpc) lhd = pNpc->GetHandle();
		double hd;
		memcpy(&hd,&lhd,sizeof(double));
		return hd;
	}

	bool isInSameScreen( void* pEntity,void * other )
	{
		if(pEntity == NULL) return false;
		if(other == NULL) return false;
		return ((CEntity*)pEntity)->IsInSameScreen((CEntity*)other);
	}

	unsigned int getTeamId( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CTeam * pTeam = ((CActor*)pEntity)->GetTeam();
		if(pTeam ==NULL) return 0;

		return pTeam->GetTeamID();
	}

		
	
	int getTeamMemberCount(void  * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CTeam * pTeam = ((CActor*)pEntity)->GetTeam();
		if(pTeam ==NULL) return 0;
		
		return (int)(pTeam->GetMemberCount());
	}
	int getOnlineTeamMemberCount(void  * pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CTeam * pTeam = ((CActor*)pEntity)->GetTeam();
		if(pTeam ==NULL) return 0;
		return (int)(pTeam->GetOnlineUserCount());
	}



	void* getTeammemberByIndex(void  * pEntity,int nIndex,unsigned int &nActorID)
	{
		nActorID =0;
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		unsigned int nSelfActorID= ((CEntity *)pEntity)->GetProperty<unsigned int>(PROP_ENTITY_ID);
		CTeam * pTeam = ((CActor*)pEntity)->GetTeam();
		if(pTeam ==NULL) return NULL;
		const TEAMMEMBER* pMember=  pTeam->GetMemberList();
		int count=0;
		for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT ; i++)
		{
			if(pMember[i].nActorID &&nSelfActorID != pMember[i].nActorID )
			{
				if(count == nIndex )
				{
					nActorID = pMember[i].nActorID;
					return  pMember[i].pActor;
				}
				count ++;
			}
			
		}
		return NULL;
	}

	void* getFubenPrt( void* pEntity )
	{
		if(!pEntity) return NULL;
		return ((CEntity*)pEntity)->GetFuBen();
	}

	void* getScenePtr( void* pEntity )
	{
		if(!pEntity) return NULL;
		return ((CEntity*)pEntity)->GetScene();
	}


	void* getActorPtrByName( char* sName )
	{
		if (sName == NULL || sName[0] == 0) return NULL;
		return GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}

	void* getActorPtrById(unsigned int nActorId)
	{
		if( nActorId == 0 ) 
		{
			return NULL;
		}
		return GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}

	void* createMonster( void * pEntity,unsigned int nMonsterid,int posX,int posY,unsigned int nLiveTime, bool addSelfName)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		CActor *pActor = (CActor*)pEntity;

		if (pScene)
		{
			PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterid);
			if(pConfig ==NULL) return NULL;

			int nEntityType = pConfig->btEntityType;
			if(nEntityType ==enActor) //配置错误
			{
				return NULL;
			}

			CEntity* pMonster = 
				pScene->CreateEntityAndEnterScene(nMonsterid,nEntityType,posX,posY,-1,pActor,nLiveTime);
			if (pMonster != NULL)
			{
				//pScene->SetMonsterConfig(pMonster,-1,nMonsterid);
				//((CMonster*)pMonster)->SetLiveTIme(nLiveTime);				
				//((CMonster*)pMonster)->SetOwner(((CEntity*)pEntity)->GetHandle());
				if(pMonster->IsMonster() && addSelfName)
				{
					((CMonster*)pMonster)->SetOwnerName(((CEntity*)pEntity)->GetEntityName());

				}
				//pMonster->GetAttriFlag().CanAttackMonster = true;
				return pMonster;
			}
		}
		return NULL;
	}




	unsigned int getReliveTimeOut( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;		
		unsigned int nTimeOut = ((CActor*)pEntity)->GetDeathTime();
		unsigned int nNow = GetGlobalLogicEngine()->getMiniDateTime();
		if (nNow < nTimeOut)
		{
			return nTimeOut-nNow;
		}
		return 0;
	}

	void setReliveTimeOut( void* pEntity,unsigned int nTimeOut )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;		
		((CActor*)pEntity)->SetDestoryTime(nTimeOut);
	}

	void clearReliveTimeOut( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;		
		((CActor*)pEntity)->ClearDestoryTime();
	}

	bool reqEnterFuben(void* pEntity, int nFubenId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		if (((CEntity*)pEntity)->GetType() == enActor )
		{
			return ((CActor*)pEntity)->GetFubenSystem().ReqEnterFuben(nFubenId);
		}
		return false;
	}

	bool reqExitFuben(void* pEntity, int nFubenId)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		if (((CEntity*)pEntity)->GetType() == enActor )
		{
			return ((CActor*)pEntity)->GetFubenSystem().ReqExitFuben(nFubenId);
		}
		return false;
	}

	void returnCity( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return;
		if (((CEntity*)pEntity)->GetType() == enActor )
		{
			CFuBen * pFb = ((CActor*)pEntity)->GetFuBen();
			if( pFb != NULL && pFb->IsFb())
			{
				((CActor*)pEntity)->GetFubenSystem().ReqExitFuben(pFb->GetFbId());
				return;
			}
		}
		((CActor*)pEntity)->ReturnCity();
	}

	void relive( void* pEntity, bool isReturn)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->Relive(isReturn);
	}

	unsigned int getLastLogoutTime( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetLastLogoutTime();
	}

	unsigned int getLoginTime( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetLoginTime();
	}

	bool sendOpenBuildFormToClient(void* pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->SendOpenBuildFormToClient(((CActor*)pEntity));
		}

		return false;
	}

	unsigned int getGuildZzTime( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetGuildSystem()->nZzNext;

	}

	void setGuildZzTime( void *pEntity,unsigned int nTime )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;

		UINT ctime = ((CActor*)pEntity)->GetLastLogoutTime() +
			GetLogicServer()->GetDataProvider()->GetGuildConfig().nLeftTimeLimit;

		if((!((CActor*)pEntity)->GetGuildSystem()->IsFree()) || (nTime<=ctime))
			((CActor*)pEntity)->GetGuildSystem()->nZzNext = nTime;
		else
			((CActor*)pEntity)->GetGuildSystem()->nZzNext = 0;
	}

	unsigned int getGuildWealTime( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetGuildSystem()->nWealNext;

	}

	void setGuildWealTime( void *pEntity,unsigned int nTime )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->GetGuildSystem()->nWealNext = nTime;
	}

	int getPkMode( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetPkSystem().GetPkMode();
	}

	void setPkMode( void* pEntity, int nMode )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->GetPkSystem().SetPkMode(nMode,true);
	}

	void clearGuildZjItem( void *pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->GetGuildSystem()->ClearZJ();
	}

	bool isKillAllMonster( void* pEntity,int nMonsterId )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (pScene)
		{
			return pScene->IsKillAllMonster(nMonsterId);
		}
		return false;
	}

	int getLiveMonsterCount( void* pEntity,int nMonsterId )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (pScene)
		{
			return pScene->GetMonsterCount(nMonsterId,0);
		}
		return 0;
	}


	int getMyMonsterCount( void* pEntity,int nMonsterId )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (pScene)
		{
			return pScene->GetMonsterCount(nMonsterId,((CEntity*)pEntity)->GetHandle());
		}
		return 0;
	}

	void addExp( void* pEntity, int nValue, int nWay,int nParam, int nType )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->AddExp(nValue,nWay,nParam, nType);
	}

	int getDyanmicVar(lua_State *L)
	{
		CEntity *pEntity = (CEntity*)lua_touserdata(L, 1);
		if (!pEntity)
			return 0;
		return LuaCLVariant::returnValue(L, pEntity->GetDynamicVar());
	}

	int getStaticVar(lua_State *L)
	{
		CActor *pActor = (CActor*)lua_touserdata(L, 1);
		if (!pActor || pActor->GetType() != enActor)
			return 0;
		return LuaCLVariant::returnValue(L, pActor->GetActorVar());
	}

	bool hasMapAreaAttri( void* pEntity, int nAttriValue )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		int x,y;
		((CEntity*)pEntity)->GetPosition(x,y);
		if (pScene && pScene->HasMapAttribute(x,y,nAttriValue))
		{
			return true;
		}
		return false;
	}

	bool hasMapAreaAttriValue(void* pEntity, int nAttriValue,int nValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		int x,y;
		((CEntity*)pEntity)->GetPosition(x,y);
		if (pScene && pScene->HasMapAttribute(x,y,nAttriValue,nValue))
		{
			return true;
		}
		return false;
	}

	int getMapAttriValue(void* pEntity, int nAttriValue)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (!pScene)
		{
			return 0;
		}
		int x,y;
		((CEntity*)pEntity)->GetPosition(x,y);
		SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
		if(!pArea) return 0;
		int nValue = 0;
		if (pScene && pScene->GetMapAttriValue(x,y,nAttriValue,nValue,pArea))
		{
			return nValue;
		}
		return 0;
	}
	void triggerAchieveEvent(void * pEntity, int nEventID,int nValue, int nSubType)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return ;
		((CActor *)pEntity)->GetAchieveSystem().ActorAchievementComplete(nEventID,nValue,nSubType);
	}

	const char* getTalkNpcName( void* pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CNpc* pNpc = ((CActor*)pEntity)->GetTalkNpc();
		
		return pNpc? pNpc->GetEntityName():NULL;
	}

	int getActorFriendsCount(void *pActor)
	{
		if (!pActor || ((CEntity*)pActor)->GetType() != enActor)
			return false;

		return (int)((CActor *)pActor)->GetFriendList().count();
	}

	int getActorFriend(void *pActor, int idx)
	{
		if (!pActor || ((CEntity*)pActor)->GetType() != enActor)
			return false;

		CVector<int> &friendList = ((CActor *)pActor)->GetFriendList();
		if (idx < friendList.count())
			return friendList[idx];

		return 0;
	}

	int postEntityScriptDelayMsg(void *pEntity, int nDelayTime, bool &result, int param1, int param2, int param3, int param4, int param5, bool bForceCallWhenDestroy)
	{
		result = false;
		if (!pEntity) return 0;

		TICKCOUNT nCurrTick = _getTickCount();
		CEntityMsg msg(CEntityMsg::emEntityScriptDelay);
		msg.dwDelay = nDelayTime;		
		msg.nParam1 = nCurrTick;
		msg.nForceCallWhenDestroy = bForceCallWhenDestroy ? 1 : 0;
		msg.nParam2 = param1;
		msg.nParam3 = param2;
		msg.nParam4 = param3;
		msg.nParam5 = param4;
		msg.nParam6 = param5;
		((CEntity *)pEntity)->PostEntityMsg(msg);
		result = true;
		return (int)nCurrTick;
	}

	bool removeEntityScriptDelayMsg(void *pEntity, unsigned int nMsgIndex)
	{		
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;

		CActor *pActor = (CActor *)pEntity;
		return pActor->removeEntityScriptDelayMsg(nMsgIndex) >= 0 ? true : false;
	}

	bool canUseItem( void *pEntity,int nItemId )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CScene* pScene = ((CEntity *)pEntity)->GetScene();
		int x,y;
		((CEntity *)pEntity)->GetPosition(x,y);
		if (pScene && pScene->HasMapAttribute(x,y,aaNotItemId,nItemId))
		{
			((CActor*)pEntity)->SendTipmsgWithId(tmItemCanNotUseNow,tstUI);
			return false;
		}
		if (((CActor*)pEntity)->HasState(esStateStall))
		{
			return false;
		}
		return true;
	}

	/*
	* Comments: 玩家的成就是否完成了
	* Param void * pEntity:玩家的指针
	* Param int nAchieveID:成就的id
	* @Return bool: 已经完成了返回true,否则返回false
	*/
	bool isAchieveFinished(void * pEntity,int nAchieveID)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetAchieveSystem().IsAchieveFinished(nAchieveID);
	}

	bool isAchieveGiveAwards(void * pEntity,int nAchieveID)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetAchieveSystem().IsAchieveGiveAwards(nAchieveID);
	}

	bool getPositionByDir( void* pEntity, int radio,int& nX, int& nY )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CActor* pActor = (CActor*)pEntity;
		pActor->GetPosition(nX,nY);
		CScene* pScene = pActor->GetScene();
		INT_PTR j, nDir, nDX, nDY;
		nDir = pActor->GetProperty<unsigned int>(PROP_ENTITY_DIR);
		if (!pScene) return false;
		bool result = false;
		for (j=0; j<8; ++j)
		{
			CSkillSubSystem::GetPosition(nX, nY, nDir, radio, nDX, nDY);
			if (pScene->CanMove(nDX, nDY))
			{
				result = true;
				break;
			}
			nDir = (nDir + 1) & 7; 
		}
		if (result)
		{
			nX = (int)nDX;
			nY = (int)nDY;
		}
		return result;
	}

	
	void monsterSay(void *pEntity, const char *pMsg, int nBCType)
	{
		if (!pEntity || !pMsg) return;	
		if (((CEntity *)pEntity)->IsMonster() || ((CEntity *)pEntity)->isNPC())
		{
			CMonster *pMonster = (CMonster *)pEntity;
			pMonster->Say((MonsterSayBCType)nBCType, pMsg);
		}
	}

	// void followActor(void *pEntity, void *pActor, unsigned int nMinDist, unsigned int nMaxDist, bool bStopWhenFailed)
	// {
	// 	if (!pEntity || !pActor) return;		
	// 	if (((CEntity *)pEntity)->GetType() == enMonster)
	// 	{
	// 		CMonster *pMonster = (CMonster *)pEntity;
	// 		CMovementSystem *pMoveSystem = pMonster->GetMoveSystem();
	// 		if (pMoveSystem)
	// 		{
	// 			pMonster->SetOwner(((CEntity *)pActor)->GetHandle());	
	// 			LPCTSTR  sName = pMonster->GetEntityName();
	// 			pMonster->SetShowName(sName);
	// 			pMonster->GetAttriFlag().CanAttackMonster = true;
	// 			pMoveSystem->ClearMovement();
	// 			pMonster->SetMaxFollDist(nMaxDist);
	// 			pMoveSystem->MoveFollow((CAnimal *)pActor, nMinDist, nMaxDist, bStopWhenFailed);				
	// 			EntityHandle handle = ((CEntity *)pEntity)->GetHandle();
	// 			((CEntity *)pActor)->AddOwnedBaBy(handle);				
	// 		}
	// 	}		
	// }

	bool hasMonsterNear( void* pEntity, int nMonsterId,int nCount )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		int nResult = 0;
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		const CVector<EntityHandleTag>& list = ((CActor*)pEntity)->GetObserverSystem()->GetVisibleList();
		for (INT_PTR i = 0; i < list.count(); i++)
		{
			EntityHandle handle = list[i].m_handle;
			CEntity *pEntity = pEntityMgr->GetEntity(handle);
			if (pEntity && pEntity->GetType() == enMonster && pEntity->GetProperty<int>(PROP_ENTITY_ID) == nMonsterId)
			{
				nResult++;
				if (nResult >= nCount)
				{
					return true;
				}
			}
		}
		return false;
	}
		
	int regScriptCallback(lua_State *L)
	{	
		CEntity *pEntity	= (CEntity *)lua_touserdata(L, 1);
		if (!pEntity) return 0;
		CNpc *pNpc			= (CNpc *)lua_touserdata(L, 2);
		UINT nDelay			= (UINT)lua_tointeger(L, 3);
		UINT nInterval		= (UINT)lua_tointeger(L ,4);
		int nCount			= (int)lua_tointeger(L, 5);
		LPCSTR pszFnName	= lua_tostring(L, 6);
		CScriptValueList paramList;
		paramList.getArguments(L, 7);
		unsigned int evt_hdl = 1;//pEntity->RegScriptCallback(pNpc, nDelay, nInterval, nCount, pszFnName, paramList);
		lua_pushnumber(L, evt_hdl);
		return 1;
	}

	int regScriptTimer(lua_State *L)
	{	
		CEntity *pEntity	= (CEntity *)lua_touserdata(L, 1);
		if (!pEntity) return 0;
		UINT nNpcId			= (UINT)lua_tointeger(L,2);
		UINT nDelay			= (UINT)lua_tointeger(L, 3);
		UINT nInterval		= (UINT)lua_tointeger(L ,4);
		int nCount			= (int)lua_tointeger(L, 5);
		LPCSTR pszFnName	= lua_tostring(L, 6);
		CScriptValueList paramList;
		paramList.getArguments(L, 7);
		unsigned int evt_hdl = pEntity->RegScriptCallback(nNpcId, nDelay, nInterval, nCount, pszFnName, paramList);
		lua_pushnumber(L, evt_hdl);
		return 1;
	}

	void unregScriptCallbackByHandle(void *pEntity, unsigned int handle)
	{
		if (!pEntity)  return;
		return ((CEntity *)pEntity)->UnregScriptCallback(handle);
	}

	void unregScriptCallback(void *pEntity, const char *pFnName)
	{
		if (!pEntity)  return;
		return ((CEntity *)pEntity)->UnregScriptCallback(pFnName);
	}

	void queryServicesAddCard(void *pEntity, const char *pSN, bool bQueryOrUse)
	{
		if (!pSN || !pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		const int nSnLen = 20;
		char szBuf[nSnLen+1] = {0};
		INT_PTR nLen = strlen(pSN);
		if (nLen >= nSnLen)
			nLen = nSnLen;
		strncpy(szBuf, pSN, nLen);
		szBuf[nLen] = '\0';		
		char *pEnd = 0;		
#ifdef WIN32
		unsigned long long sn = _strtoui64(szBuf, &pEnd, 10);
#else
		unsigned long long sn = strtoll(szBuf, &pEnd, 10);
#endif
		CActor *pActor = (CActor *)pEntity;
		pActor->GetBagSystem().UseServicesAddedCard(sn, bQueryOrUse);
	}

	void userServicesAddCard(void *pEntity, double seriesNo)
	{
		UINT64 sn;
		memcpy(&sn, &seriesNo, sizeof(double));
		CActor *pActor = (CActor *)pEntity;
		pActor->GetBagSystem().UseServicesAddedCard(sn, false);
	}

	void setMonsterOwer( void* pEntity, void* pMonster )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		if (!pMonster || ((CEntity *)pMonster)->GetType() != enMonster && ((CEntity *)pMonster)->GetType() != enGatherMonster) return;
		((CMonster*)pMonster)->SetOwnerName(((CActor*)pEntity)->GetEntityName());
	}

	void* getMonsterOwner(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enMonster) return NULL;
		EntityHandle handle = ((CMonster *)pEntity)->GetOwner();
		CEntity *pOwner = GetEntityFromHandle(handle);
		return pOwner;
	}

	bool setMonsterOwnerName(void* pMonster, int nIndex, char *sName)
	{
		if (!pMonster || !(((CEntity *)pMonster)->IsMonster())) return false;
		return ((CMonster*)pMonster)->SetOwnerName( sName);
	}



	void setMonsterName(void* pEntity,char* sName)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enMonster) return;
		((CEntity *)pEntity)->SetEntityName(sName);
	}

	void monsterRealDropItemExp(void *pEntity,void* pMonster)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		if (!pMonster || ((CEntity *)pMonster)->GetType() != enMonster) return;

		unsigned int nID = ((CMonster*)pMonster)->GetProperty<unsigned int>(PROP_ENTITY_ID);
		int nPosX,nPosY; //位置
		((CMonster*)pMonster)->GetPosition(nPosX,nPosY); //获取坐标

		CMonster::RealDropItemExp(nID,((CActor*)pEntity),((CActor*)pEntity)->GetScene(),nPosX,nPosY,(CMonster*)pMonster);
	}

	void setMonsterCanMove(void *pEntity,bool bState)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().DenyMove = bState;
	}

	bool isMonster( void* pEntity )
	{
		if (pEntity)
		{
			return ((CEntity*)pEntity)->GetType() == enMonster;
		}
		return false;
	}

	void getEntityPosition( void* pEntity,int& x, int& y )
	{
		if (pEntity)
		{
			((CEntity*)pEntity)->GetPosition(x,y);
		}
	}

	void getEntityMaxCircleAndLevel(void* pEntity, int &nCircle, int &nLevel)
	{
		if (pEntity && ((CEntity*)pEntity)->GetType() == enActor) 
		{
			((CActor*)pEntity)->GetMaxCircleAndLevel(nCircle,nLevel);
		}
	}

	void setEntityTelByOwnderAttr(void *pEntity, bool bSet)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().TelByOwnerEntity = bSet;
	}

	void setEntityTelByOwnerWhenNonePortal(void *pEntity, bool bSet)
	{
		if (!pEntity)
			return;

		((CEntity *)pEntity)->GetAttriFlag().TelByOwnerEntityWhenNonePortal = bSet;
	}

	void setEntityLazyFollow(void *pEntity, bool bSet)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().MonsterLazyFollow = bSet;
	}

	void setEntityCanAttackMonster(void *pEntity, bool bSet)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().CanAttackMonster = bSet;
	}

	void setDenyAttackedByActor(void *pEntity, bool bSet)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().DenyAttackedByActor = bSet;
		((CEntity *)pEntity)->GetAttriFlag().DenyBeAttack = bSet;
	}

	void setDenyBeAttack(void *pEntity, bool bSet)
	{
		if (!pEntity) return;
		((CEntity *)pEntity)->GetAttriFlag().DenyBeAttack = bSet;
	}

	void onRelive( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->OnRelive();
	}

	void sendCountdown( void* pEntity, int nTime,int  nFlag, int nType)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetMiscSystem().SendCountdown(nTime, nFlag, nType);
	}

	bool giveAward(void* pEntity,int nType, int nId, double nValue,
		int nQuality, int nStrong, int nBindFlag, int auxParam,int nLogId,char* pLogStr, int nQualityDataIndex)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GiveAward(nType,nId,(INT_PTR)nValue,nQuality,nStrong,nBindFlag,auxParam,nLogId,pLogStr, nQualityDataIndex); 
	}

	bool checkConsume(void* pEntity, int nType, int nId, double nValue, int nQuality, int nStrong, int nBindFlag, int nParam)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->CheckConsume(nType, nId, (INT_PTR)nValue, nQuality, nStrong, nBindFlag, nParam);
	}

	bool CheckActorSource(void* pEntity, int nType, int nId, int nCount, int nTipmsgType )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->CheckActorSource(nType, nId, nCount, nTipmsgType);
	}


	unsigned long long getConsume(void* pEntity, int nType, int nId,int nValue, int nQuality, int nStrong, int nBindFlag, int nParam)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetConsume(nType, nId, nValue,nQuality, nStrong, nBindFlag, nParam);
	}
	int removeConsume(void* pEntity, int nType, int nId, double nValue, int nQuality, int nStrong, int nBindFlag,int nParam,int nLogId,char* pLogStr)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return (int)((CActor*)pEntity)->RemoveConsume(nType,nId,(INT_PTR)nValue,nQuality,nStrong,nBindFlag,nParam,nLogId,pLogStr); 
	}

	INT_PTR useSkill( void* pEntity,int nSkillID, int nPosX /*=0*/,int nPosY/*=0*/,bool needLearn /*=true*/,int nSkillLevel/*=0*/, bool boCd )
	{
		if (!pEntity || ((CEntity *)pEntity)->isAnimal() ==false) return 0;
		return ((CActor*)pEntity)->GetSkillSystem().LaunchSkill(nSkillID,nPosX,nPosY,boCd); 
	}
	
	int getCampId(void* pEntity)
	{
		if(pEntity ==NULL )return 0;
		if( ((CEntity*)pEntity)->isAnimal()  ==false) return 0;

		return ((CAnimal*)pEntity)->GetCampId();
	}
	
	void setCampId(void* pEntity,int nCampId)
	{
		if(pEntity ==NULL )return ;
		if(  ((CEntity*)pEntity)->isAnimal()  ==false) return ;
		((CAnimal*)pEntity)->SetCamp(nCampId);
	}

	void SetNpcGuildId(void* pEntity,unsigned int nGuildId)
	{
		if(pEntity ==NULL )return ;
		if(  ((CEntity*)pEntity)->isAnimal()  ==false) return ;

		((CAnimal*)pEntity)->SetNpcGuildId(nGuildId);
	}

	void setDieRefreshHpPro(void* pEntity,int nValue)
	{
		if(pEntity ==NULL )return ;
		if(((CEntity*)pEntity)->isAnimal() == false) return ;
		((CAnimal*)pEntity)->SetDieRefreshHpPro(nValue);
	}

	unsigned int getNpcGuildId(void* pEntity)
	{
		if(pEntity ==NULL )return 0;

		if(  ((CEntity*)pEntity)->isAnimal()  ==false) return 0;

		return ((CAnimal*)pEntity)->GetNpcGuildId();
	}


	void changeEntityName(void* pEntity,char* sName)
	{
		if(pEntity ==NULL )return ;
		if(  ((CEntity*)pEntity)->isAnimal()  ==false) return ;
		((CAnimal*)pEntity)->ChangeName(sName);
	}

	int getGuildPos( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetGuildSystem()->GetGuildPos(); 
	}
	char* getGuildTitleName(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return "";
		int nPos = getGuildPos(pEntity);
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetGuildTitleName(nPos);
		}
		return "";
	}
	int getGuilCityPos(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetGuildSystem()->GetGuildCityPos();
	}

	void setGuldBossFlag( void* pEntity,bool bFlag)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SetGuldBossFlag(bFlag); 
	}

	void SendSzBossTimes( void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetBossSystem().SendSZBossTimes(); 
	}

	void setGuildDartFlag(void* pEntity,bool bFlag)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SetGuildDartFlag(bFlag); 

	}

	bool getGuildBossFlag(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetGuildSystem()->GetGuildBossFlag(); 
	}

	unsigned int getGuildLeaderId( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild && pGuild->m_nLeaderId)
		{
			return pGuild->m_nLeaderId;
		}
		return 0;
	}

	void sendWelcomeToNewMember(void* pEntity,char* nMsg)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SendWelcomeToNewMember(nMsg);
	}

	char* findGuildMemberName(void* pEntity,unsigned int nActorid)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return NULL;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->FindGuildMemberName(nActorid);
		}
		return NULL;

	}

	char* getGuildNameByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return NULL;

		return GetGlobalLogicEngine()->GetGuildMgr().GetGuildNameByRank(nRank);
	}

	const char* getGuildLeaderNameByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return NULL;

		return GetGlobalLogicEngine()->GetGuildMgr().GetGuildLeaderNameByRank(nRank);
	}

	int getGuildMemNumByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		return GetGlobalLogicEngine()->GetGuildMgr().GetGuildMemNumByRank(nRank);
	}

	void sendGuildCardByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;

		((CActor*)pEntity)->GetGuildSystem()->SendGuildCardByRank(nRank);
	}

	void onStartBidGuildRank(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;

		((CActor*)pEntity)->GetGuildSystem()->OnStartBidGuildRank();
	}

	bool isSignUpGuildSiege(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(((CActor*)pEntity)->GetGuildSystem())
		{
			CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
			if(pGuild)
			{
				return pGuild->m_SignUpFlag != 0;
			}
		}

		return false;
	}

	int MyGuildIsSbk(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		if(((CActor*)pEntity)->GetGuildSystem())
		{
			CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
			if(pGuild)
			{
				if(pGuild->GetGuildId() == GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId())
					return 1;
			}
		}

		return 0;
	}

	void joinToGuild(void* pEntity, int nJoinGuildId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->JoinToGuildByRank(nJoinGuildId);
	}

	int getGuildCoinByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		return GetGlobalLogicEngine()->GetGuildMgr().GetGuildCoinByRank(nRank);
	}

	bool hasGuildByRank(void* pEntity,int nRank)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;

		return GetGlobalLogicEngine()->GetGuildMgr().HasGuildByRank(nRank);
	}

	void addEventRecord(void* pEntity,char* nRecord,int nEventId,int nParam1,int nParam2,int nParam3,char* nParam4,char* nParam5)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			pGuild->AddEventRecord(nRecord,nEventId,nParam1,nParam2,nParam3,nParam4,nParam5);
		}

	}

	int getGuildLevel( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetLevel();
		}
		return 0;
	}

	void setGuildYs(void* pEntity,int value)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			pGuild->SetGuildYs(value);
		}

	}



	int getGuildCoin(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetCoin();
		}

		return 0;
	}

	void changeGuildCoin(void *pEntity,int value, int nLogId,const char* pStr) 
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			pGuild->ChangeGuildCoin(value, nLogId, pStr);
		}
	}

	void sendGuildDonateData(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SendGuildDonateData();
	}

	void sendGuildUpgradeItem(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SendGuildUpgradeItem();
	}

	void sendGuildInfo(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SendGuildInfoResult();
	}

	void sendGuildTreeData(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			((CActor*)pEntity)->GetGuildSystem()->SendGuildTreeData();
		}
	}

	void sendGuildFruitData(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			((CActor*)pEntity)->GetGuildSystem()->SendGuildFruitData();
		}
	}

	int getGuildTreeTodayChargeNum(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return ((CActor*)pEntity)->GetGuildSystem()->GetTodayChargeTreeNum();
		}
		return 0;
	}

	void setGuildTreeTodayChargeNum(void * pEntity, int nTodayChargeNum)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			((CActor*)pEntity)->GetGuildSystem()->SetTodayChargeTreeNum(nTodayChargeNum);
		}
	}

	int getGuildFruitTodayPickNum(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return ((CActor*)pEntity)->GetGuildSystem()->GetTodayPickFruitNum();
		}
		return 0;
	}

	void setGuildFruitTodayPickNum(void * pEntity,  int nTodayPickNum)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			((CActor*)pEntity)->GetGuildSystem()->SetTodayPickFruitNum(nTodayPickNum);
		}
	}

	void sendGuildFruitPickData(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			((CActor*)pEntity)->GetGuildSystem()->SendGuildFruitPickData();
		}
	}


	bool getGuildBossCallFlag(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetUseGuildBoss();
		}

		return false;
	}

	void setCallGuildBossFlag(void* pEntity,bool value)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SetCallGuildBossFlag(value);
	}

	bool getCallGuildBossFlag(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetGuildSystem()->GetCallGuildBossFlag();
	}

	int getGuildGx( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		return (int)((CActor*)pEntity)->GetGuildSystem()->GetGx();
	}

	int GetGuildTotalGx(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		return (int)((CActor*)pEntity)->GetGuildSystem()->GetTotalGuildGx();
	}

	void setGuildGx( void* pEntity,int value )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;

		((CActor*)pEntity)->GetGuildSystem()->SetGx(value);
	}

	bool bInGuild(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;

		return ((CActor*)pEntity)->GetGuildSystem()->IsFree() == TRUE ?  true : false;
	}

	int getGuildSkillLevel(void* pEntity,int skillId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		return ((CActor*)pEntity)->GetGuildSystem()->GetGuildSkillLevel(skillId); 
	}

	void setGuildSkillLevel(void* pEntity,int skillId,int skillLevel)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetGuildSystem()->SetGuildSkillLevel(skillId,skillLevel);
	}

	bool canUpGuildSkillLevel(void* pEntity,int sType,int sCount,bool sConsume)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;

		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			return pGuild->BcanUpGuildSkillLevel((CActor*)pEntity,sType,sCount,sConsume);
		}

		return false;
	}

	void setGuildDartHandle(void* pEntity,double bHandle)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			pGuild->SetGuildDartHandle(bHandle);
		}
	}



	void setGuildFr(void* pEntity,int value)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			pGuild->SetGuildFr(value);
		}

	}

	double getGuildDartHandle(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			return pGuild->GetGuildDartHandle();
		}

		return 0;
	}

	

	bool decountUpGuildSkill(void* pEntity,int sType,int sCount,bool sConsume)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;

		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			return pGuild->DecountUpGuildSkill(sType,sCount,sConsume);
		}

		return false;
	}

	void saveDbGuildSkill(void* pEntity,int skillId,int skillLevel)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;

		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			pGuild->SaveDbGuildSkill((CActor*)pEntity,skillId,skillLevel);
		}

	}

	void sendGuildSkillToAllMember(void* pEntity,int skillId,int skillLevel)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;

		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();

		if (pGuild)
		{
			pGuild->SendGuildSkillToAllMember(skillId,skillLevel);
		}
	}

	int getGuildFr( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->GetFr();
		}
		return 0;
	}

	char* getGuildName( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return "";
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->m_sGuildname;
		}
		return "";
	}

	unsigned int getGuildId(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return 0;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->m_nGid;
		}
		return 0;
	}
	void * getGuildPtr(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return NULL;
		return ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
	}
	void upGuildLevel( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return ;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->SetGuildLevel(pGuild->GetLevel()+1,TRUE,true,true);
		}
		return ;
	}

	void sendGuildMsg( void* pEntity,int guildPos,const char* sMsg,int nType )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return ;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->SendGuildMsg(guildPos,sMsg,nType);
		}
		return ;
	}

	void notifyUpdateGuildInfo( void* pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return ;
		CGuild* pGuild = ((CActor*)pEntity)->GetGuildSystem()->GetGuildPtr();
		if (pGuild)
		{
			return pGuild->NotifyUpdateGuildInfo();
		}
		return ;
	}

	bool refreshActorObserveList(void* pEntity)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) 
			return false;

		((CActor*)pEntity)->GetObserverSystem()->OnTimeCheck(0);
		return true;
	}
	
	void sendSceneTime( void* pEntity, int nTime )
	{
		// if (!pEntity) return;
		// if (((CEntity *)pEntity)->GetType() != enActor) return;
		// CActor* pActor = (CActor *)pEntity;
		// CActorPacket AP;
		// CDataPacket& DataPacket = pActor->AllocPacket(AP);
		// AP << (BYTE)enFubenSystemID << (BYTE)sFubenRestTime << (int)nTime;
		// AP.flush();
	}

	bool enlargeBag(void *pEntity, int nGridCount)
	{
		if (!pEntity) return false;
		if (((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetBagSystem().EnlargeBag(nGridCount);
	}

	void sendPrivateChatMsg( void *pEntity,void* pActor,const char* sMsg )
	{
		// if (!pEntity || !pActor) return;
		// if (((CEntity *)pEntity)->GetType() != enActor || ((CEntity *)pActor)->GetType() != enActor) return;
		// CActorPacket pack;
		// CDataPacket & data = ((CActor*)pActor)->AllocPacket(pack);
		// data << (BYTE) enChatSystemID << (BYTE) sPrivateChate;
		// data.writeString(((CActor*)pEntity)->GetEntityName());
		// data.writeString(sMsg);
		// data << (BYTE) ((CActor*)pEntity)->GetProperty<unsigned int>(PROP_ACTOR_SEX);
		// data << (BYTE)(((CActor*)pEntity)->GetChatSystem()->IsFreeTalk() ?1:0);	
		// data << (BYTE)1; // 阵营Id
		// data << (BYTE)0; // 阵营职位Id
		// pack.flush();
	}

	void playScrEffect( void* pEntity, int nEffId,int nSec, bool toWorld, int nLevel)
	{
		if (!pEntity) return;
		if (((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->GetMiscSystem().PlaySrcEffect(nEffId, nSec, toWorld, nLevel);
	}


	void playScrEffectCode( void* pEntity, int nEffId,int nSec,int x,int y)
	{
		if (!pEntity) return;
		if (((CEntity *)pEntity)->GetType() != enActor) return;
		CActorPacket ap;
		CDataPacket& data = ((CActor*)pEntity)->AllocPacket(ap);
		data << (BYTE)enMiscSystemID << (BYTE)sScrEffectCode << (WORD)nEffId <<  (int)nSec << (int)x << (int)y;
		ap.flush();
	}

	void playScrSceneEffectCode(void* pEntity, int nEffId,int nSec,int nSceneId,int x,int y)
	{
		if (!pEntity) return;
		if (((CEntity *)pEntity)->GetType() != enActor) return;
		CActorPacket ap;
		CDataPacket& data = ((CActor*)pEntity)->AllocPacket(ap);
		data << (BYTE)enMiscSystemID << (BYTE)sScrEffectCode << (WORD)nEffId <<  (int)nSec << (int)x << (int)y;
		ap.flush();

		((CActor*)pEntity)->AddSceneEffectToList(nEffId,nSceneId,nSec);
	}

	void delEffectCode(void * pEntity, int nEffId)
	{
		if ( !pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->DeleteSceneEffect(nEffId);
	}
	void addState( void* pEntity,int state )
	{
		if (!pEntity) return;
		if (((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->AddState(state);
	}

	void removeState( void* pEntity,int state )
	{
		if (!pEntity) return;
		if (((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->RemoveState(state);
	}

	bool  consumeCoin(void* pEntity,double nCount,int nlogId, char* comment,bool needLog,bool bindFirst )
	{
		if (!pEntity) return false;
		if (((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->ConsumeCoin((INT_PTR)nCount,nlogId,(LPCTSTR)comment,needLog,bindFirst);
	}
	void worldBroadCastMsg(void* packet, int nLevel, int nCircle)
	{
		if (packet)
		{
			CActorPacket* pPack = (CActorPacket *)packet;
			GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(pPack->packet->getMemoryPtr(), pPack->packet->getPosition(), nLevel, nCircle);
		}
	}

	void battleGroupBroadCastMsgCs(void* packet, int nLevel, bool bCsFlag)
	{
		if (!packet) return;
		CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
		CDataPacket &data = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cSendGroupMessage);
		data << (WORD)jxInterSrvComm::SessionServerProto::fcWorldBroadCastMsg;
		data << (int)nLevel;
		CActorPacket *pPacket = (CActorPacket *)packet;
		data << (int)(pPacket->packet->getLength());
		data.writeBuf(pPacket->packet->getMemoryPtr(), pPacket->packet->getLength());
		pSSClient->flushProtoPacket(data);
		
		if (bCsFlag)
			worldBroadCastMsg(packet, nLevel);
	}

	int getMiscData(void* pEntity, enMiscDataType mdType)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CActor* pActor = (CActor *)pEntity;
		return pActor->GetMiscSystem().GetMiscData(mdType);
	}

	void setMiscData(void* pEntity, int mdType, int nVal)
	{		
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		CActor* pActor = (CActor *)pEntity;
		pActor->GetMiscSystem().SetMiscData((enMiscDataType)mdType, nVal);
	}

	void setNpcIdleTime( void* pEntity,int nTime )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enNpc) return;
		((CNpc*)pEntity)->SetIdleTime(nTime);
	}

	bool setEntityVisibleFlag(void * pEntity, bool canSee)
	{
		if (!pEntity ) return false;
		if(!canSee)
		{
			OutputMsg(rmNormal, _T("[%s] npc hide now!"), ((CEntity *)pEntity)->GetEntityName());
		}
		EntityFlags & flag= ((CEntity*)pEntity)->GetAttriFlag();
		flag.DenySee = !canSee;
		return true;
	}
	
	unsigned int getItemLeftTime(void* pEntity, void* pUserItem)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor || !pUserItem) 
			return false;
		CActor* pActor = (CActor *)pEntity;
		CUserItem* pItem = (CUserItem *)pUserItem;

		return pActor->GetBagSystem().GetItemLeftTime(pItem);
	}

	void inviteJoinTeam( void* pEntity,void* pActor )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 	return;
		if (!pActor || ((CEntity *)pActor)->GetType() != enActor) 	return;
		((CActor*)pEntity)->GetTeamSystem().InviteJoinTeam((CActor*)pActor);
	}

	void setDir( void* pEntity,int nDir )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 	return ;
		((CEntity*)pEntity)->SetDir(nDir);
		char buff[128];
		CDataPacket data(buff,sizeof(buff));
		data << (BYTE) enDefaultEntitySystemID << (BYTE) sTurnAround
			<<	(Uint64)((CEntity*)pEntity)->GetHandle()  << (BYTE) nDir;
		CObserverSystem * pSystem = ((CActor*)pEntity)->GetObserverSystem();
		if(pSystem)
		{
			pSystem->BroadCast(data.getMemoryPtr(),data.getPosition(),true);
		}
	}

	void addEffect( void* pEntity,int nEffId,int nEffType,int nTime ,int nDelay,int nTotalCount,int nDirCount)
	{
		if (!pEntity) 	return ;
		if (((CEntity *)pEntity)->GetType() != enActor && ((CEntity *)pEntity)->GetType() != enNpc)
		{
			return;
		}
		CEntityMsg msg(CEntityMsg::emAppendEntityEffect, ((CEntity*)pEntity)->GetHandle());
		msg.dwDelay = nDelay;
		msg.nParam1 = nEffType;
		msg.nParam2 = nEffId;
		msg.nParam3 = nTime;
		msg.nParam4 = nTotalCount;
		msg.nParam5 = nDirCount;
		((CEntity*)pEntity)->PostEntityMsg(msg); //向目标发送消息，稍候由目标进行广播
	}

	void addSceneEffect(void* pEntity,int nEffId,int nEffType,int nTime ,int nDelay,int nRelX,int nRelY,int nTotalCount,int nDirCount)
	{
		if (!pEntity) 	return ;
		if (((CEntity *)pEntity)->GetType() != enActor )
		{
			return;
		}
		CEntityMsg msg(CEntityMsg::emAppendSceneEffect);
		msg.dwDelay = nDelay;
		msg.nParam1 = nEffType;
		msg.nParam2 = nEffId;
		msg.nParam3 = nTime;
		CActor * pActor = (CActor*) pEntity;
		msg.nParam4 = pActor->GetProperty<int>(PROP_ENTITY_POSX) + nRelX;
		msg.nParam5 = pActor->GetProperty<int>(PROP_ENTITY_POSY) + nRelY;
		msg.nParam6 = nTotalCount;
		msg.nParam7 = nDirCount;
		pActor->PostEntityMsg(msg); //施法者自己发送消息，稍候广播
	}

	void broadSceneEffect(void* pEntity,int nEffId,int nEffType,int nTime ,int nDelay,int nRelX,int nRelY,int nTotalCount, int nDirCount)
	{
		if (!pEntity) 	return ;
		if (((CEntity *)pEntity)->GetType() != enActor )
		{
			return;
		}
		CEntityMsg msg(CEntityMsg::emAppendSceneEffect);
		msg.dwDelay = nDelay;
		msg.nParam1 = nEffType;
		msg.nParam2 =nEffId;
		msg.nParam3 = nTime;
		CActor * pActor = (CActor*) pEntity;
		msg.nParam4 = nRelX;
		msg.nParam5 = nRelY;
		msg.nParam6 = nTotalCount;
		msg.nParam7 = nDirCount;

		CActorPacket pack;
		CDataPacket & data = pActor->AllocPacket(pack);
		data << (BYTE)enDefaultEntitySystemID <<(BYTE) sAddSceneEffect ;
		data <<  (Uint64)msg.nSender;
		data << (BYTE) msg.nParam1 << (WORD) msg.nParam2 ; //特效的类型以及ID
		data <<(WORD) msg.nParam4  << (WORD) msg.nParam5;  //x,y 
		data << (DWORD)msg.nParam3; //持续时间

		data <<(BYTE)(msg.nParam6); 
		data <<(BYTE)msg.nParam7;

		pack.flush();
	}

	void delAllEffect( void* pEntity )
	{
		if (!pEntity) 	return ;
		if (((CEntity *)pEntity)->GetType() != enActor && ((CEntity *)pEntity)->GetType() != enNpc)
		{
			return;
		}
		((CAnimal*)pEntity)->GetEffectSystem().DelAllEffect(true);
	}

	bool isDeath( void* pEntity )
	{
		if (!pEntity) return false;
		return ((CEntity*)pEntity)->IsDeath();
	}

	// void npcTalk( void* pEntity,void* pNpc,const char* sFunc )
	// {
	// 	if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) 
	// 		return;
	// 	if (!pNpc || ((CEntity*)pNpc)->GetType() != enNpc) 
	// 		return;
	// 	EntityHandle handle= ((CNpc*)pNpc)->GetHandle();
	// 	((CActor*)pEntity)->NpcTalk(handle,sFunc);
	// }

	// void npcTalkByName( void* pEntity,char* szName,const char* sFunc/*="" */ )
	// {
	// 	if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) 
	// 		return;
	// 	CScene* pScene = ((CActor*)pEntity)->GetScene();
	// 	if (pScene)
	// 	{
	// 		CNpc* pNpc = pScene->GetNpc(szName);
	// 		if (pNpc)
	// 		{
	// 			EntityHandle handle= ((CNpc*)pNpc)->GetHandle();
	// 			((CActor*)pEntity)->NpcTalk(handle,sFunc);
	// 		}
	// 	}
	// }

	bool hasState(void* pEntity, int state)
	{
		if (!pEntity) 
			return false;

		CAnimal* entity = (CAnimal *)pEntity;
		return entity->HasState(state);
	}

	int getGMLevel(void* pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return 0;
		CActor* pActor = (CActor *)pEntity;
		return pActor->GetGmLevel();
	}

	double getTargetHandle(void * pEntity)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		double d;
		memcpy(&d, &(((CActor *)pEntity)->GetTarget()),sizeof(d));
		return d;
	}

	unsigned int getTargetId( void* pEntity )
	{
		int nTargetId = 0;
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return nTargetId;
		CEntity* pTarget = GetEntityFromHandle(((CEntity*)pEntity)->GetTarget());
		if (pTarget)
		{
			nTargetId = pTarget->GetProperty<unsigned int>(PROP_ENTITY_ID);
		}
		return nTargetId;
	}


	bool moveToEntity( void *pEntity, void* targetEntity )
	{
		if(!pEntity || !targetEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return false;
		if(((CEntity*)targetEntity)->GetType() == enActor)
		{
			return ((CActor*)pEntity)->MoveToEntity((CEntity*)targetEntity);
		}
		return false;
	}

	bool setTopTitle(void *pEntity, int nTitleId, bool bFlag)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return false;
		if (nTitleId == 0) 
			return false;
		return ((CActor*)pEntity)->SetHeadTitle(nTitleId, bFlag);
	}

	void reqTransmitTo(void *pEntity, int nDestServerId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return;
		if(((CActor *)pEntity)->CanTransMit(nDestServerId))
			((CActor *)pEntity)->RequestTransmitTo(nDestServerId);
	}

	void reqTransmitToCommonServer(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return;
		CLogicServer *lpLogicServer = GetLogicServer();
		int nId = lpLogicServer->GetCommonServerId();
		if(((CActor *)pEntity)->CanTransMit(nId))
			((CActor *)pEntity)->RequestTransmitTo(nId);
	}

	void reqTransmitToRawServer(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return;
		if (((CActor *)pEntity)->IsInRawServer())
			return;
		int nDestServerId = ((CActor *)pEntity)->GetRawServerIndex();
		if(((CActor *)pEntity)->CanTransMit(nDestServerId))
			((CActor *)pEntity)->RequestTransmitTo(nDestServerId);
	}

	bool isInRawServer(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return false;
		return ((CActor *)pEntity)->IsInRawServer();
	}
	//原区服id
	int getActorRawServerId(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
			return false;

		return ((CActor *)pEntity)->GetRawServerIndex();
	}
	
	bool setActorCircle(void* pEntity, int nCircle)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;
		return ((CActor *)pEntity)->SetActorCircle(nCircle);
	}

	void viewOffLineActor(void *pEntity, unsigned nActorId, char *sName, bool loadWhenNotExist, unsigned int nShowType)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		CActor* pActor = (CActor *)pEntity;

		GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActor(pActor, nActorId);
	}

	void viewOffLineActorHero(void *pEntity, unsigned nActorId, unsigned nHeroId, bool loadWhenNotExist, unsigned int nShowType)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		CActor* pActor = (CActor *)pEntity;

		GetGlobalLogicEngine()->GetOfflineUserMgr().ViewOffLineActorHero(pActor,nActorId, nHeroId, loadWhenNotExist, nShowType);
	}

	// void sendCanAddFriendList(void *pEntity, int nLevel1, int nLevel2,int nMaxNum)
	// {
	// 	if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
	// 		return;
	// 	CActor* pActor = (CActor *)pEntity;

	// 	pActor->GetFriendSystem().SendCanAddFriendList(nLevel1, nLevel2, nMaxNum);
	// }

	void changeEntityModel( void * pEntity, int nRadius, int nModelId)
	{
		if (!pEntity) 
			return;
		((CAnimal *)pEntity)->ChangeModel(nRadius, nModelId);
	}

	void changeShowName( void * pEntity, int nRadius, char *sName)
	{
		if (!pEntity) 
			return;
		((CAnimal *)pEntity)->ChangeShowName(nRadius, sName);
	}

	void resetShowName( void * pEntity )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		((CActor*)pEntity)->ResetShowName();
	}

	bool checkActorLevel(void *pEntity, int nLevel, int nCircle, bool bCircleOnly)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;
		return ((CActor*)pEntity)->CheckLevel(nLevel, nCircle, bCircleOnly);
	}

	bool checkActorMaxCircleAndLevel(void *pEntity, int nLevel, int nCircle)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;
		return ((CActor*)pEntity)->CheckMaxCircleAndLevel(nLevel, nCircle);
	}

	bool checkOpenLevel(void *pEntity, int nLevelConfigId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;
		return ((CActor*)pEntity)->CheckOpenLevel(nLevelConfigId);
	}

	void updateActorName(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;

		UpdateNameClr_OnPkModeChanged(((CActor *)pEntity)->GetHandle());
	}

	bool startLearnSkill( void * pEntity, int nId, int nLevel )
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return false;
		}
		((CActor*)pEntity)->GetSkillSystem().StartRealLearnSkill(nId, nLevel);
		return true;
	}

	int getBuffRemainTime( void * pEntity, int nBuffType, int nBuffGroup)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		return (int)((CActor*)pEntity)->GetBuffSystem()->GetBuffRemainTime(nBuffType, nBuffGroup);
	}

	int getBuffRemainTimeById( void * pEntity, int nBuffId )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		return (int)((CActor*)pEntity)->GetBuffSystem()->GetBuffRemainTimeById(nBuffId);
	}

	bool canGiveAward( void *pEntity, int nType, int nId, double nCount, int nQuality,int nStrong,int nBindFlag,bool boNotice /*= true*/ )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		return ((CActor*)pEntity)->CanGiveAward(nType,nId,(INT_PTR)nCount,nQuality,nStrong,nBindFlag,boNotice);
	}

	bool createTeam( void *pEntity )
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return false;
		}
		CActor * pLeader = (CActor*)pEntity;
		return !!pLeader->GetTeamSystem().CreateTeam();		
	}

	int autoFindPathToNpc( void * pEntity, int nSceneId, char * sNpcName)
	{
		if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return -1;
		}
		return ((CActor*)pEntity)->GetMoveSystem()->AutoFindPathToNpc(nSceneId, sNpcName);
	}

	unsigned int getSocialRelationId(void *pEntity, int nType)
	{
		if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		
		return ((CActor*)pEntity)->GetFriendSystem().GetSocialRelationId(nType);
	}

	int getSocialCurNum(void *pEntity, int nType)
	{
		if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		return ((CActor*)pEntity)->GetFriendSystem().GetSocialCurnum(nType);
	}

	char* getSocialActorName(void *pEntity, unsigned int nActorId)
	{
		if (pEntity != NULL && ((CEntity*)pEntity)->GetType() == enActor)
		{
			return ((CActor*)pEntity)->GetFriendSystem().GetSocialActorName(nActorId);
		}

		return NULL;
	}

	// int getMasterIntimacy(void *pEntity, unsigned int nActorId,int nType)
	// {
	// 	if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
	// 	{
	// 		return 0;
	// 	}

	// 	return ((CActor*)pEntity)->GetFriendSystem().GetMasterIntimacy(nActorId,nType);
	// }

	// void graduationGiveMasterWard(void *pEntity,unsigned int nActorId)
	// {
	// 	if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
	// 	{
	// 		return;
	// 	}

	// 	return ((CActor*)pEntity)->GetFriendSystem().GraduationGiveMasterWard(nActorId);
	// }

	// void giveUpLevelAwardTips(void *pEntity,unsigned int nActorId,int nIndex)
	// {
	// 	if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
	// 	{
	// 		return;
	// 	}

	// 	((CActor*)pEntity)->GetFriendSystem().GiveUpLevelAwardTips(nActorId,nIndex);
	// }
	
	// void sendGrauateToMate(void *pEntity,unsigned int nActorId,int nLevel)
	// {
	// 	if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
	// 	{
	// 		return;
	// 	}

	// 	((CActor*)pEntity)->GetFriendSystem().SendGrauateToMate(nActorId,nLevel);

	// }

	void sendMoveToMsg(void *pEntity, char *sSceneName, int nX, int nY, char *sTitle, char *sTips)
	{
		if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetMsgSystem().SendMoveToMsg(sSceneName, nX, nY, sTitle, sTips);
	}

	/*
	void donateGuildCoin(void *pEntity,int nItemId,int nItemNum)
	{
		if (pEntity == NULL || ((CEntity*)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->DonateGuildCoin(nItemNum);
	}
	*/

	void sendChat(void *pEntity, int nChannleID, char *pMsg, bool boSave)
	{
		if (!pEntity || !pMsg || ((CEntity*)pEntity)->GetType() != enActor) return;

		((CActor *)pEntity)->GetChatSystem()->SendChat(nChannleID, pMsg, boSave);
	}

	void changeVocation(void *pEntity, int nVocation)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;

		((CActor *)pEntity)->ChangeVocation(nVocation);
	}

	void changeSex(void *pEntity, int nSex)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;

		((CActor *)pEntity)->ChangeSex(nSex);
	}

	bool setFullAnger( void * pEntity )
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return false;
		((CActor*)pEntity)->AddAnger(0, true);
		return true;
	}


	// void AddSocialRelation(void * pEntity, int nIndex, int nType, unsigned int nActorId, char* sName)
	// {
	// 	((CActor*)pEntity)->GetFriendSystem().DoAddSocialRelation((BYTE)nIndex, (BYTE)nType, nActorId, sName);
	// }

	bool HasSocialRelation(void* pEntity1, void* pEntity2, int nType)
	{
		if ( nType == SOCIAL_FRIEND )
		{
			return ((CActor*)pEntity1)->GetFriendSystem().GetSocialFlag(((CActor*)pEntity2)->GetProperty<unsigned int>(PROP_ENTITY_ID),SOCIAL_FRIEND);
		}
		// else if ( nType == SOCIAL_ANIMAL )
		// {
		// 	return ((CActor*)pEntity1)->GetFriendSystem().GetSocialFlag(((CActor*)pEntity2)->GetProperty<unsigned int>(PROP_ENTITY_ID),SOCIAL_ANIMAL);
		// }
		return false;
	}

	int GetNameColorData(void * pEntity)
	{
		return ((CActor*)pEntity)->GetNameColorData();
	}

	bool canTelport( void * pEntity, int nSceneId, char * sSceneName /*= NULL*/ )
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		return ((CActor *)pEntity)->CanTelport(nSceneId, sSceneName);
	}

	void clearEquipSharp( void *pEntity, bool boCast )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType()!= enActor)
		{
			return;
		}
		CActor * pActor = (CActor *)pEntity;
		pActor->GetEquipmentSystem().ClearEquipSharp(boCast);
		pActor->GetBagSystem().ClearEquipSharp(boCast);
	}
	
	int setSalary(void *pEntity, int nNowValue, int nLastValue)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CActor * pActor = (CActor *)pEntity;
		return pActor->SetSalary(nNowValue, nLastValue);
	}

	int getSalary(void *pEntity, int &nNowValue, int &nLastValue)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CActor * pActor = (CActor *)pEntity;
		return pActor->GetSalary(nNowValue, nLastValue);
	}


	bool isDealing(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return false;
		return ((CActor *)pEntity)->GetDealSystem().IsDealing();
	}

	void queryActorName( void * pEntity, const char * sName )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		// ((CActor *)pEntity)->GetStoreSystem().DoQueryActorExists(sName);
	}



	void changeGuildGx(void * pEntity,int nGx)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetGuildSystem()->ChangeGuildGx(nGx);
	}

	void setShxDoneTimes(void * pEntity,int nTimes)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->SetShxDoneTimes(nTimes);
	}

	int getShxDoneTimes(void *pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetShxDoneTimes();
	}

	int getShxAddTimes(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetShxAddTimes();
	}

	void changeShxAddTimes(void * pEntity,int nTimes)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetGuildSystem()->ChangeShxAddTimes(nTimes);
	}

	int getTodayGxValue(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetTodayGxValue();
	}

	void changeExploreTimes(void * pEntity,int nTimes)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->ChangeExploreTimes(nTimes);
	}

	int getExploreTimes(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetExploreTimes();
	}

	int getExploreItemId(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0 ;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetExploreItemId();
	}

	void setExploreItemId(void * pEntity,int nItemId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->SetExploreItemId(nItemId);
	}

	int getChallengeTimes(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetChallengeTimes();
	}

	void setChallengeTimes(void * pEntity,int nTimes)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->SetChallengeTimes(nTimes);
	}

	void setAddChallengeTimes(void * pEntity,int nTimes)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->SetAddChallengeTimes(nTimes);
	}

	int getAddChallengeTimes(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}

		return ((CActor *)pEntity)->GetGuildSystem()->GetAddChallengeTimes();
	}

	void changeTodayGx(void * pEntity,int nValue)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}

		((CActor *)pEntity)->GetGuildSystem()->ChangeTodayGx(nValue);
	}

	int getGuildTodayDonateCoin(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return ((CActor *)pEntity)->GetGuildSystem()->GetTodayDonateCoin();
	}

	void setGuildTodayDonateCoin(void * pEntity, unsigned int nTodayCoin)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->GetGuildSystem()->SetTodayDonateCoin(nTodayCoin);
	}

	int getGuildTodayDonateItem(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return -1;
		return ((CActor *)pEntity)->GetGuildSystem()->GetTodayDonateItem();
	}

	void setGuildTodayDonateItem(void * pEntity, unsigned int nTodayItem)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return;
		((CActor *)pEntity)->GetGuildSystem()->SetTodayDonateItem(nTodayItem);
	}

	void getNpcPos( int nSceneId, int nNpcId, int &x, int &y )
	{
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if (pScene != NULL)
			{
				CNpc * pNpc = pScene->GetNpc(nNpcId);
				if (pNpc != NULL)
				{
					pNpc->GetPosition(x, y);
				}
			}
		}
	}

	// const char * GetLastSocialActorInfo(void * pEntity, int nType, unsigned int & nActorId){
	// //int GetLastSocialActorInfo(lua_State *L){
	// 	//void* pEntity = ((void*)  tolua_touserdata(L,1,0));
	// 	nActorId = 0;
	// 	if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) return 0;
	// 	//int nType(tolua_tonumber(L,2,0)); // 
	// 	FRIENDRELATION* pFriend(((CActor*)pEntity)->GetFriendSystem().GetLastSocialActorInfo(nType)) ;
	// 	if (pFriend){
	// 		//tolua_pushnumber(L,pFriend->nActorId);
	// 		//tolua_pushstring(L,pFriend->sName);
	// 		//OutputMsg(rmTip,"pFriend->nActorId = %d, pFriend->sName = %s", pFriend->nActorId, pFriend->sName );
	// 		nActorId = pFriend->nActorId;
	// 		return pFriend->sName;
	// 		//return 2;
	// 	}
	// 	//else{ // 测试数据
	// 	//	tolua_pushnumber(L, 900);
	// 	//	tolua_pushstring(L, "xu yao chou ren  xi tong  zhi chi");
	// 	//	return 2;
	// 	//}
	// 	//OutputMsg(rmTip,"pFriend not found");
	// 	return 0;
	// }

	bool isTeamLeader( void * pEntity )
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		CActor * pActor = (CActor *)pEntity;
		CTeam * pTeam = pActor->GetTeam();
		if (pTeam != NULL && pTeam->GetTeamID() > 0)
		{
			return pActor == pTeam->GetCaptin();
		}
		return false;
	}

	void backMain( void * pEntity, const char * sFunName )
	{
		// if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		// {
		// 	return;
		// }
		// CActor * pActor = (CActor *)pEntity;
		// pActor->OnNpcTalk(pActor->GetTarget(), (!sFunName || !strcmp(sFunName, "main"))? "" : sFunName); // 避免返回按钮NPC少了一些内容
		// pActor->m_boCallScript = false;
	}

	bool openBuyItemDialog( void * pEntity, double hNpc, int nItemId, int nCount, const char * sNotice, const char * sDesc,const char * sCallFunc )
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		EntityHandle hNpcHandle;
		memcpy(&hNpcHandle,&hNpc,sizeof(hNpc));
		return ((CActor *)pEntity)->AddBuyItemDialog(hNpcHandle,nItemId,nCount,sNotice,sDesc,sCallFunc);
	}

	void sendCustomEffect( void * pEntity, unsigned char nType, int nParam )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetMiscSystem().SendCustomEffect(nType, nParam);
	}

	void changeActivity(void * pEntity, int nValue)
	{
		if(pEntity ==NULL ) return ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;

		((CActor*)pEntity)->ChangeActivity(nValue);
	}

	
	void setAchieveUnFinished(void * pEntity, int nAchieveId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetAchieveSystem().SetAchieveUnFinished(nAchieveId);
	}
	void setAchieveInstantFinish(void* pEntity,int nAchieveId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetAchieveSystem().SetAchieveInstantFinish(nAchieveId);
	}

	int getDepotItemCount( void * pEntity, int wItemId )
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}
		return ((CActor *)pEntity)->GetDeportSystem().GetDepotItemCount(wItemId);
	}

	void RemoveBattlePet(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->GetPetSystem().RemoveBattlePet();
	}

	bool addNewTitle(void * pEntity, int nId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		return ((CActor*)pEntity)->GetNewTitleSystem().addNewTitle( nId );
	}
	bool delNewTitle(void * pEntity, int nId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		return ((CActor*)pEntity)->GetNewTitleSystem().delNewTitle( nId );
	}
	bool isHaveNewTitle(void * pEntity, int nId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return false;
		}
		return ((CActor*)pEntity)->GetNewTitleSystem().IsHaveNewTitle( nId ) >= 0;
	}

	void setCurNewTitle(void * pEntity, int nId)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor*)pEntity)->GetNewTitleSystem().SetCurNewTitle(nId);
	}

	void BroadCastNameClrScene(void * pEntity)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		BroadCastNameClr_Scene((CActor*)pEntity);
	}

	void SendShortCutMsg(void * pEntity, const char* szMsg)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		((CActor *)pEntity)->SendShortCutMsg(szMsg);
	}

	void setExploitRate( void * pEntity, double nVal )
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return;
		}
		CActor * pActor = (CActor *)pEntity;
		pActor->m_nExploitRate  = nVal;
	}


	void setMonsterGrowLevel( void* pEntity, int nValue)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enMonster)
		{
			return;
		}
		CMonster* pMonster = (CMonster*)pEntity;
		unsigned int nID = pMonster->GetProperty<unsigned int>(PROP_ENTITY_ID);
		PMONSTERCONFIG pConfig = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nID);
		if ( !pConfig )
		{
			return;
		}
		int nDestValue = pConfig->nLevel + nValue;
		pMonster->SetProperty<unsigned int>(PROP_CREATURE_LEVEL,nDestValue);
		pMonster->OnLevelUp(nDestValue);
	}

	
    int  GetGameSetting(void* pEntity, int nType, int nIdx) //获取游戏设置 
	{
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return 0;
		}		
        return ((CActor*)pEntity)->GetMiscSystem().GetGameSetting(nType,nIdx);
	}

    void setPosition(void *pEntity, int nSceneID,  int nPosx, int nPosy)// 设置坐标
	{
		if (pEntity == NULL)
		{
			return ;
		}
		if(nSceneID > 0) //如果 nSceneID = 0 ，则不需要设置场景
		{
			((CEntity *)pEntity)->SetSceneID(nSceneID);
		}
		((CEntity *)pEntity)->SetPosition(nPosx,nPosy);
	}

	int getAppearByteType( void *pEntity,int nType )
	{
		int nResult = 0;
		if (pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor)
		{
			return nResult;
		}
		CActor* pActor = (CActor*)pEntity;
		return 0;   ///pActor->GetAlmirahSystem().GetAppearByType(nType);
	}

	void SetEnterFuBenMapPos(void *pEntity,int nSceneId,int x,int y)
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		((CActor*)pEntity)->SetEnterFuBenMapPos(nSceneId,x,y);
	}

	void sendNotEnoughMoney(void* pEntity, int nType, int nCount)
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		((CActor*)pEntity)->SendMoneyNotEnough((BYTE)nType, nCount);
	}
	void sendAwardNotEnough(void* pEntity, unsigned char nType, unsigned short wItemId, int nCount)
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;
		((CActor*)pEntity)->SendAwardNotEnough(nType, wItemId, nCount);
	}

	//激活翅膀
	void setActivation(void* pEntity,int nModleId)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return ;
		}
		((CActor*)pEntity)->GetAlmirahSystem().SetActivation(nModleId);
	}
	//某个翅膀是否激活
	bool getActivation(void* pEntity,int nModleId)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return false;
		}
		return ((CActor*)pEntity)->GetAlmirahSystem().GetActivation(nModleId);
	}


	//发送行会频道消息
	void sendGuildChannelMsg(void* pEntity, char* szMsg)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return ;
		}
		((CActor *)pEntity)->GetChatSystem()->SendChat(ciChannelGuild, szMsg);
	}
	void SetRelivePoint(void* pEntity, int nPosX,int nPosY,int nSceneId, bool isFuben)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return ;
		}
		((CActor *)pEntity)->SetRelivePoint(nPosX, nPosY, nSceneId, isFuben);
	}
	const char* getTitle(void* pEntity)
	{
		if (pEntity)
		{
			return ((CEntity*)pEntity)->GetTitle();
		}
		return NULL;
	}
	void setTitle(void* pEntity, char* sTitle)
	{
		if (pEntity && sTitle)
		{
			((CEntity*)pEntity)->SetTitle(sTitle);
			char sAllName[128];
			if (strcmp(sTitle, "")== 0)
			{
				sprintf_s(sAllName,sizeof(sAllName),"%s",((CEntity*)pEntity)->GetEntityName());
			}
			else
			{
				sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",((CEntity*)pEntity)->GetEntityName(),((CEntity*)pEntity)->GetTitle());
			}
			if (((CEntity*)pEntity)->isAnimal() || ((CEntity*)pEntity)->isNPC())
			{
				((CAnimal*)pEntity)->ChangeShowName(20, sAllName);
			}
		}
	}


	bool isNearBySceneNpc(void *pEntity, int nSceneId, int nNpcId )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return false;
		}
		CActor* pActor = (CActor*)pEntity;
		return GetGlobalLogicEngine()->GetMiscMgr().IsNearBySceneNpc( pActor, nSceneId, nNpcId );
	}

	//刷新角色外观
	void refreshFeature(void *pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return;
		}
		CActor* pActor = (CActor*)pEntity;
		pActor->CollectOperate(CEntityOPCollector::coRefFeature);
	}

	/*获取最近一次死亡的时间
	0：没有死亡
	*/
	unsigned int getDeadTimeRecent( void *pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return 0;
		}
		// CActor* pActor = (CActor*)pEntity;
		// return pActor->GetFriendSystem().GetDeadTimeRecent();
	}

	int getLoginDays( void *pEntity )
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return 0;
		}
		CActor* pActor = (CActor*)pEntity;
		return pActor->GetLoginDaysValue();
	}

	unsigned int addExpToExpBox( void *pEntity, unsigned int nAddExp)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return 0;
		}
		CActor* pActor = (CActor*)pEntity;
		return (unsigned int)pActor->GetEquipmentSystem().OnAbsorbExp( nAddExp );
	}

	int  AddExpReachLevel(void *pEntity, unsigned int  nAddExp)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return 0;
		}
		CActor* pActor = (CActor*)pEntity;
		return  pActor->AddExpReachLevel(nAddExp);
	}

	double GetLevelExp(void *pEntity, int nLevel)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return 0;
		}
		return (double)GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nLevel); 	   
	}

	void updateActorEntityProp(void* pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return;
		}
		CActor* pActor = (CActor*)pEntity;
		pActor->GetObserverSystem()->UpdateActorEntityProp();
		
	}
	unsigned int  GetMonAttackedTime(void *pEntity)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enMonster) 
		{
			return 0;
		}
		return   ((CMonster*)pEntity)->GetAttackedTime();
	}

	void setAchieveGiveAwards(void *pEntity,int nAchieveID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return ;
		}
		CActor* pActor = (CActor*)pEntity;
		pActor->GetAchieveSystem().SetAchieveGiveAwards(nAchieveID);
	}
	void setAchieveFinished(void *pEntity,int nAchieveID)
	{
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) 
		{
			return ;
		}
		CActor* pActor = (CActor*)pEntity;
		pActor->GetAchieveSystem().SetAchieveFinished(nAchieveID);
	}

	void swapCombatRank(void *pEntity,int nActorId,int nTagetId)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetCombatSystem().SwapCombatRank(nActorId,nTagetId);
	}

	void ChallegeOverOp(void *pEntity, int nResult, int nActorId, char* nName, int nIsReal)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetCombatSystem().ChallegeOverOp(nResult, nActorId, nName, nIsReal);
	}

	int getMyCombatRank(void *pEntity)
	{
	     if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return 0;
		 return ((CActor*)pEntity)->GetCombatSystem().GetMyCombatRank();
	}

	int getTotalStar(void* pEntity)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return 0;
		CActor* pActor = (CActor *)pEntity;
		CUserEquipment &equipSys = pActor->GetEquipmentSystem();
		return equipSys.GetTotalStar();
	}


	void OnNewDayCombatClear(void *pEntity)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		((CActor*)pEntity)->GetCombatSystem().OnNewDayClear();
	}

	unsigned int getMaxAttack(void *pEntity)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetActorAttackValueMax();
	}

	unsigned int getMaxDefence(void *pEntity)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return 0;
		return ((CActor*)pEntity)->GetActorDefenceValueMax();
	}

	void SetForceVesterId(void *pEntity, unsigned int nForceVesterId)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enMonster) return;
		((CMonster*)pEntity)->SetForceVesterId(nForceVesterId);
	}

	int getRunningActivityId(lua_State* L)
	{
		void* pEntity = ((void*)  lua_touserdata(L,1));
  		int nAtvType = ((int)  lua_tonumber(L,2));

		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return 0;

		bool isGlobal = false;
		if (GetLogicServer()->GetDataProvider()->GetActivityConfigs().IsGlobalAtvType(nAtvType))
		{
			isGlobal = true;
		}
		else if(GetLogicServer()->GetDataProvider()->GetActivityConfigs().IsPersonAtvType(nAtvType))
		{
			isGlobal = false;
		}
		else
		{
			lua_pushnil( L );
			return 0;
		}
		
		std::vector<int>& AtvIdList = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetActivityIdList(nAtvType);

		lua_newtable(L);
		int idx = 0;
		for (size_t i = 0; i < AtvIdList.size(); i++)
		{
			if (isGlobal)
			{
				if(!GetGlobalLogicEngine()->GetActivityMgr().IsRunning(AtvIdList[i]))
					continue;
			}
			else
			{
				if(!((CActor*)pEntity)->GetActivitySystem().IsActivityRunning(AtvIdList[i]))
					continue;
			}
			//tolua_pushfieldvalue(L, -3, idx++, AtvIdList[i]);
			lua_pushinteger(L, (int)++idx);
			lua_pushinteger(L, (int)AtvIdList[i]);
			lua_settable(L, -3);
		}
		return 1;
	}

	bool isActivityRunning(void *pEntity, int nAtvId)
	{
		if (GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
		{
			return true;
		}
		 
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return false;
		if (((CActor*)pEntity)->GetActivitySystem().IsActivityRunning(nAtvId))
		{
			return true;
		}
		return false;
	}

	bool isActivityTypeRunning(void *pEntity, int nAtvType)
	{
		if (GetLogicServer()->GetDataProvider()->GetActivityConfigs().IsGlobalAtvType(nAtvType))
		{
			return GetGlobalLogicEngine()->GetActivityMgr().IsTypeRunning(nAtvType);
		}
		else if(GetLogicServer()->GetDataProvider()->GetActivityConfigs().IsPersonAtvType(nAtvType))
		{
			if (pEntity == NULL) return false;
			if (((CEntity*)pEntity)->GetType() != enActor) return false;
			return ((CActor*)pEntity)->GetActivitySystem().IsTypeRunning(nAtvType);
		}
		return false;
	}


	bool isMaxLevel(void *pEntity)
	{
		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;

		unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(((CActor*)pEntity)->GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));	
		unsigned int nLevel = ((CActor*)pEntity)->GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
		if(nLevel < nMaxPlayerLevel)
			return false;
		return true;
	}

	bool checkNpcTranLimit(void *pActor,int staticType, int dayLimit){
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;

		if(-1==dayLimit) return true ;
		int timesNow = ((CActor*)pActor)->GetStaticCountSystem().GetStaticCount(staticType);
		if(timesNow < dayLimit) return true;
			else return false ;
	}
	bool addNpcTranTimes(void *pActor,int staticType, int num) {
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;

		int timesNow = ((CActor*)pActor)->GetStaticCountSystem().GetStaticCount(staticType);
		((CActor*)pActor)->GetStaticCountSystem().SetStaticCount(staticType ,timesNow+num);
		return true ;
	}

	//30+level*0.1 + day*0.1
	void addPaodianExp(void* pEntity, int paramA , int paramB, int paramC, int nLog){
		if (pEntity == NULL) return;
		int num = 0;
		if(!paramA && !paramB && !paramC)
		{
			num = (((CActor*)pEntity)->GetLevel())*0.1 +(GetLogicServer()->GetDaysSinceOpenServer())*0.1 + 30 ;
		}	
		else
		{
			num = paramA +((((CActor*)pEntity)->GetLevel())*paramB)/100 + ((GetLogicServer()->GetDaysSinceOpenServer())*paramC)/100 ;
		}
		//首充卡&&色卡会员 泡点plus
		// int nPlusNum = ((((CActor*)pEntity)->GetMiscSystem()).GetSuperRightAtvPaoDianPlus()) ; 
		// if(nPlusNum> 0 )  
		// 	num =num * (100+nPlusNum)/100.0 ;
		
		((CActor*)pEntity)->AddExp(num,GameLog::Log_PaoDian,nLog);
	}

	void addTypePaodianExp(void* pEntity, int nType, int nLogId){
		if (pEntity == NULL) return;
		
		((CActor*)pEntity)->AddPaoDianExp(nType, nLogId);
	}

	int ChouJiangByGroupId(lua_State* L)
	{

		void* pEntity = ((void*)  lua_touserdata(L,1));
  		int nDropGroupId = ((int)  lua_tonumber(L,2));
		int nLogId = ((int)  lua_tonumber(L,3));
		LPCTSTR logstr = ((LPCTSTR)lua_tostring(L, 4));

		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return 0;

		std::vector<DROPGOODS> RetList ;
		if (!((CActor*)pEntity)->GiveDropAwardWithRet(nDropGroupId, nLogId,logstr, RetList ) || (RetList.size() <=0) )
		{
			lua_pushnil( L );
			return 0;
		}
		else
		{
			//dropInfos[i].info.nType, dropInfos[i].info.nId, dropInfos[i].info.nCount
			lua_newtable(L);
			int placeOut = lua_gettop(L);
			int idx = 0;
			// lua_pushinteger(L, ((int)++idx));
			// lua_newtable(L);
			// int t = lua_gettop(L);
			// lua_pushstring(L, "key");
			// lua_pushinteger(L, ((int)++idx));
			// lua_settable(L, t);
			// lua_settable(L, t1);

			for (size_t i = 0; i < RetList.size(); i++)
			{
				lua_pushinteger(L, ((int)++idx));				
				lua_newtable(L);
				int placeInner = lua_gettop(L);
				lua_pushstring(L,"Type");
				lua_pushinteger(L,(int)RetList[i].info.nType);
				lua_settable(L, -3);
				lua_pushstring(L,"Id");
				lua_pushinteger(L,((int)RetList[i].info.nId));
				lua_settable(L, -3);
				lua_pushstring(L,"Count");
				lua_pushinteger(L,(int)RetList[i].info.nCount);
				lua_settable(L, -3);

				//lua_settable(L, -5);

			
			// 	//tolua_pushfieldvalue(L, -3, idx++, RetList[i]);
			// 	//lua_pushinteger(L, (int)++idx);
			// 	//lua_pushinteger(L, (int)RetList[i]);
			// 	//lua_settable(L, -3);
			}
			lua_settable(L,placeOut);
			return 1;
		}

	}



	void sendActivityData(void *pEntity,int nAtvId)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		CActor* pActor = ((CActor*)pEntity);

		if (GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
		{
			GetGlobalLogicEngine()->GetActivityMgr().SendOneActivity(pActor, nAtvId);
		}
		else if (pActor->GetActivitySystem().IsActivityRunning(nAtvId))
		{
			pActor->GetActivitySystem().SendOneActivity(nAtvId);
		}
	}

	
	void closeOneActivity(void *pEntity,int nAtvId)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		CActor* pActor = ((CActor*)pEntity);

		if (pActor->GetActivitySystem().IsActivityRunning(nAtvId))
		{
			pActor->GetActivitySystem().CloseActivity(nAtvId);
		}
	}
	

	//移除宠物
	void removePet(void *pEntity)
	{
		if(!pEntity || ((CEntity*)pEntity)->GetType() != enActor) return;
		CActor* pActor = ((CActor*)pEntity);
		pActor->GetPetSystem().RemoveAllPets();
	}

	void setChatForbit(void* pEntity, int nChannalId, bool value)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return;
		}
		((CActor *)pEntity)->GetChatSystem()->SetForbit(nChannalId, value);
	}
	
	void* getMaster(void *pEntity)
	{
		if (pEntity == NULL)
		{
			return NULL;
		}

		if (((CEntity*)pEntity)->GetType() == enActor)
		{
			return pEntity;
		}
		else if (((CEntity*)pEntity)->GetType() == enPet)
		{
			return ((CPet*)pEntity)->GetMaster();
		}
		return NULL;
	}

	void SetBeLongBoss(void* pEntity, int nBossId, int nSceneId)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return;
		}
		((CActor *)pEntity)->SetBeLongBoss(nBossId, nSceneId);
	}

	void CancelBeLongBoss(void* pEntity)
	{
		if ( !pEntity || ((CEntity*)pEntity)->GetType() != enActor )
		{
			return;
		}
		((CActor *)pEntity)->SelfCancelBossBeLong();
	}


	int getStaticCount(void *pActor,int staticType){
		if (pActor == NULL) return -1;
		if (((CEntity*)pActor)->GetType() != enActor) return -1;

		return ((CActor*)pActor)->GetStaticCountSystem().GetStaticCount(staticType);
	}
	void addStaticCount(void *pActor,int staticType, int num) {
		if (pActor == NULL) return;
		if (((CEntity*)pActor)->GetType() != enActor) return;

		((CActor*)pActor)->GetStaticCountSystem().AddStaticCount(staticType ,num);
	}

	void setStaticCount(void *pActor,int staticType, int num) {
		if (pActor == NULL) return;
		if (((CEntity*)pActor)->GetType() != enActor) return;

		((CActor*)pActor)->GetStaticCountSystem().SetStaticCount(staticType ,num);
	}

	bool IsHasFreePrivilege(void *pActor)
	{
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;
		return ((CActor*)pActor)->GetMiscSystem().IsHasFreePrivilege();
	}

	bool IsHasMonthCard(void *pActor)
	{
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;
		return ((CActor*)pActor)->GetMiscSystem().IsHasMonthCard();
	}

	bool IsHasMedicineCard(void *pActor)
	{
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;
		return ((CActor*)pActor)->GetMiscSystem().IsHasMedicineCard();
	}

	bool IsHasForverCard(void *pActor)
	{
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;
		return ((CActor*)pActor)->GetMiscSystem().IsHasForverCard();
	}

	void SendActivityLog(void *pActor,int nAtvId,int nAtvType,int nStatu)
	{
		if (pActor)
		{
			if (((CEntity*)pActor)->GetType() != enActor) return;
			GetLogicServer()->SendActivityLocalLog(nAtvId, ((CActor*)pActor), nAtvType, nStatu);
		}
	}
	void SendJoinActivityLog(void *pActor,int nAtvId,int nIndex)
	{
		if (pActor)
		{
			if (((CEntity*)pActor)->GetType() != enActor) return;
			GetGlobalLogicEngine()->AddJoinAtvToLog((int)(((CActor*)pActor)->GetId()), ((CActor*)pActor)->GetAccount(), 
			((CActor*)pActor)->GetEntityName(),nAtvId,nIndex,((CActor*)pActor)->getOldSrvId());
		}
	}

	bool CheckDistanceByHandle(void *pActor,double handle, int distance )
	{
		if (pActor == NULL) return false;
		if (((CEntity*)pActor)->GetType() != enActor) return false;

		//查找目标玩家
		CActor *pTarget = NULL;
		if(handle > 0)
		{
			unsigned long long d;
			memcpy(&d,&handle ,sizeof(handle));

			EntityHandle hHandle(d);
			pTarget = (CActor*)GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(hHandle);
		}
		if(!pTarget)
		{
			return false ;
		}

		//判断双方距离
		if (!((CActor*)pActor)->CheckTargetDistance(pTarget,distance ))
		{
			//pActor->SendTipmsgFormatWithId(tmTagetDistanceTooFar, tstUI);
			return false ;
		}
		return true ;

	}

	int GetMaxColorCardLevel(void* pEntity)
	{
		if (pEntity == NULL) return 0;
		if (((CEntity*)pEntity)->GetType() != enActor) return 0;
		int nLv = ((CActor*)pEntity)->GetMiscSystem().GetMaxColorCardLevel() ; 
		return nLv;
	}

	bool IsHasWhiteCard(void* pEntity)
	{

		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetMiscSystem().IsHasWhiteCard() ; 
		
	}

	bool IsHasGreenCard(void* pEntity)
	{

		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetMiscSystem().IsHasGreenCard() ; 
		
	}

	bool IsHasBlueCard(void* pEntity)
	{

		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetMiscSystem().IsHasBlueCard() ; 
		
	}

	bool IsHasPurpleCard(void* pEntity)
	{

		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetMiscSystem().IsHasPurpleCard() ; 
		
	}

	bool IsHasOrangeCard(void* pEntity)
	{

		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		return ((CActor*)pEntity)->GetMiscSystem().IsHasOrangeCard() ; 
		
	}
	void SetNpcTeleport(void* pEntity, int nSceneId, int nJoinTime, int nMapGroup) 
	{
		if (pEntity == NULL) return ;
		if (((CEntity*)pEntity)->GetType() != enActor) return ;
		((CActor*)pEntity)->SetJDJoinMap(nSceneId, nJoinTime, nMapGroup);
	}

	bool checkKuangbao(void* pEntity, int nState) 
	{
		if (pEntity == NULL) return false;
		if (((CEntity*)pEntity)->GetType() != enActor) return false;
		int nIsFrenzy = ((CActor*)pEntity)->GetProperty<int>(PROP_ACTOR_FRENZY);
		if (nIsFrenzy != nState)
		{
			return false;
		}
		return true;
	}

	int getGhostSumLv(void* pEntity) 
	{
		if (pEntity == NULL) return 0 ;
		if (((CEntity*)pEntity)->GetType() != enActor) return 0 ;
		return ((CActor*)pEntity)->GetGhostSystem().GetSumLv();
	}

	bool checkCommonLimit(void *pEntity, int nLevel, int nCircle, int nVip, int nOffice)
	{
		if (!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return false;

		if (((CActor*)pEntity)->GetMiscSystem().GetMaxColorCardLevel()  < nVip)
			return false;
		 
		if (!(((CActor*)pEntity)->CheckLevel(nLevel, nCircle)))
			return false;
		
		if (((CActor*)pEntity)->GetOfficeLv() < nOffice)
			return false;
		return true;
	}
	int getTotalOnlineTime(void* pEntity)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return 0;
		CActor* pActor = (CActor *)pEntity;
		
		return pActor->getTotalOnlineTime();
	}
	void KickUserAccount(void* pEntity)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return ;
		CActor* pActor = (CActor *)pEntity;
		if(pActor){
			pActor->KickUserAccount();
		}
	}

	void setRechargeStatus(void* pEntity, int nStatus)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return ;
		CActor* pActor = (CActor *)pEntity;
		if(pActor){
			pActor->SetRechargeStatus(RechargeStatus, nStatus);
		}
	}

	int  getRechargeStatus(void* pEntity)
	{
		if (!pEntity || ((CEntity*)pEntity)->GetType() != enActor)
			return 0;
		CActor* pActor = (CActor *)pEntity;
		if(pActor){
			return pActor->GetRechargeStatus(RechargeStatus);
		}
	}
}

