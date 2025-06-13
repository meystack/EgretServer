#include "StdAfx.h"
#include "SingleObjectAllocator.hpp"
#include "DynamicBuff.h"
#include "BuffSystem.h"

CSingleObjectAllocator<CDynamicBuff> *CBuffSystem::m_pAllocator = NULL;
float CBuffSystem::s_FireValRate = 1.0;

//能否添加一个buff
bool CBuffSystem::CanAppendBuff(INT_PTR nBuffType)
{
	switch (nBuffType)
	{
	case aDizzy:
		if(m_pEntity->GetAttriFlag().DenyMoveForbidBuff)
		{
			return false;
		}
		break;
	case aFixed:
		if(m_pEntity->GetAttriFlag().DenyMoveForbidBuff)
		{
			return false;
		}
		break;
	case aRealFreeze:
		if(m_pEntity->GetAttriFlag().DenyMoveForbidBuff)
		{
			return false;
		}
		break;
	case aMoveSpeedAdd: 
		if(m_pEntity->GetAttriFlag().DenyMoveSpeedSlowBuff) //移动速度的提升
		{
			return false;
		}
		break;
	}
	return true;
}

const CDynamicBuff * CBuffSystem::Append( int nBuffId, CDynamicBuff * pSrcBuff, bool is_notice )
{
	if(m_pEntity ==NULL) return NULL;
	CBuffProvider::BUFFCONFIG * pStdBuff = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
	if (pStdBuff == NULL)
	{
		return NULL;
	}
	if(pStdBuff->nType < 0 || pStdBuff->nType >= GameAttributeCount)
	{
		OutputMsg(rmTip,_T("buff Append by id error,buffType=%d, buffId=%d "), pStdBuff->nType, nBuffId);
		return NULL;
	}
	if(pStdBuff->nGroup < CDynamicBuff::AnyBuffGroup || pStdBuff->nGroup >= CDynamicBuff::MaxBuffGroup)
	{
		OutputMsg(rmTip,_T("buff Append by id error,nGroup=%d,buffType=%d,buffId=%d"),pStdBuff->nGroup,pStdBuff->nType,nBuffId);
		return NULL;
	}
		
	//如果目标不能添加buff的话
	if(! CanAppendBuff(pStdBuff->nType) )
	{
		return NULL;
	}
	int dwTwice = pStdBuff->nTimes;
	if (dwTwice == 0)
		dwTwice = 1;
	else if (dwTwice == -1)						// 调整为无线次数
		dwTwice = (int)CDynamicBuff::Infinite;

	CDynamicBuff * pBuff = GetBuff((GAMEATTRTYPE)pStdBuff->nType, pStdBuff->nGroup);
	GAMEATTRVALUE gameValue;
	if( AttrDataTypes[pStdBuff->nType] == adFloat)
	{
		gameValue.fValue = (float)pStdBuff->dValue;
	}
	else if(AttrDataTypes[pStdBuff->nType] == adUInt) 
	{
		gameValue.uValue = (unsigned int)pStdBuff->dValue;
	}
	else
	{
		gameValue.nValue = (int)pStdBuff->dValue;
	}
	bool boRepace = true;
	if (pSrcBuff) // 从外表拷贝
	{
		if (pBuff != NULL)
		{
			if(pStdBuff->boTimeOverlay) //如果时间需要叠加的话
			{
				//即使是叠加，如果值不相同，也只能顶替前面的
				if (pBuff->value.nValue == pSrcBuff->value.nValue )
				{
					if (dwTwice != CDynamicBuff::Infinite )
					{
						pBuff->dwTwice += dwTwice;
					}					
					boRepace = false;
				}
			}
			if (boRepace)
			{
				RemoveById(pBuff->pConfig->nId);
			}
		}
		if (boRepace)
		{
			m_buffList.add(*pSrcBuff);
			pBuff = &m_buffList[m_buffList.count()-1];
		}
		//OutputMsg(rmTip,_T("add wBuffId=%d, count=%u"),pSrcBuff->wBuffId,m_buffList.count());
		// for (INT_PTR i = 0; i < m_buffList.count(); i++)
		// {
		// 	CDynamicBuff* pBuff = &m_buffList[i];
		// 	//OutputMsg(rmTip,_T("wBuffId=%d"),pBuff->wBuffId);
		// }
		
	}
	else // 从标配新建
	{
		if (pBuff != NULL )
		{
			if (pStdBuff->boTimeOverlay && pBuff->value.nValue == gameValue.nValue )
			{
				if (dwTwice != CDynamicBuff::Infinite)
				{
					pBuff->dwTwice += dwTwice;
				}				
				boRepace = false;
			}
			if (boRepace)
			{
				RemoveById(pBuff->pConfig->nId);
			}
		}
		if (boRepace)
		{
			CDynamicBuff newBuff;
			newBuff.wBuffId = nBuffId;
			newBuff.dwTwice = dwTwice;
			newBuff.value = gameValue;
			newBuff.wActWait = pStdBuff->nInterval;
			newBuff.pConfig = pStdBuff;
			m_buffList.add(newBuff);
			pBuff = &m_buffList[m_buffList.count()-1];
		}
	}
	OnBuffAppend(pBuff,is_notice);

	if(pStdBuff->nColor > 0 ) //如果设置了颜色的话
	{
		AddBuffColor(pStdBuff->nType,pStdBuff->nGroup, pStdBuff->nPropColor, pStdBuff->nColor); //添加颜色
	}
	return pBuff;
}


INT_PTR CBuffSystem::RemoveGroupBuff(INT_PTR nGroupStart,INT_PTR nGroupEnd)
{
	/*DECLARE_FUN_TIME_PROF()*/
	INT_PTR Result = 0;
	INT_PTR nCount = Count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff && pBuff->pConfig->nGroup >= nGroupStart && pBuff->pConfig->nGroup <= nGroupEnd)
		{
			Result += Remove((GAMEATTRTYPE)pBuff->pConfig->nType, pBuff->pConfig->nGroup);
		}
	}
	return Result;
}

INT_PTR CBuffSystem::Remove(const GAMEATTRTYPE buffType, const INT_PTR nGroup)
{
	if(buffType <= aUndefined ) return 0;
	if(buffType < 0 || buffType >= GameAttributeCount)
	{
		OutputMsg(rmTip,_T("buff Remove error,buffType=%d "),buffType);
		return 0;
	}

	if(nGroup < CDynamicBuff::AnyBuffGroup || buffType >= CDynamicBuff::MaxBuffGroup)
	{
		OutputMsg(rmTip,_T("buff Remove error,nGroup=%d "),nGroup);
		return 0;
	}

	INT_PTR Result = 0;
	INT_PTR nCount = Count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff && pBuff->pConfig->nType == buffType)
		{
			if (nGroup == CDynamicBuff::AnyBuffGroup)
			{
				pBuff->wActWait = 100;
				pBuff->dwTwice = 0;
				Result ++;
			}
			else
			{
				if(pBuff->pConfig->nGroup == nGroup)
				{
					pBuff->wActWait = 100;
					pBuff->dwTwice = 0;
					Result ++;
				}
			}
		}
	}
	return Result;
}

INT_PTR CBuffSystem::Clear(const bool boBoardCast)
{
	/*DECLARE_FUN_TIME_PROF()*/

	INT_PTR Result = 0;
	INT_PTR nCount = Count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CDynamicBuff * pBuff = &m_buffList[i];
		if ( pBuff )
		{
			if (RemoveById(pBuff->wBuffId))
			{
				Result++;
			}
		}
	}
	return Result;
}


void CBuffSystem::ReduceBuffTime(CDynamicBuff *pBuff,int nSecond)
{
	if(pBuff == NULL) return;

	int nLoopTime = 0; //防止死循环

	//减去秒数，如果剩余时间够减的话就减去剩余时间，否则要减去剩余的次数

	if( pBuff->dwTwice == CDynamicBuff::Infinite) return;//如果是无限的buff，那么就不需要减
	
	while( nLoopTime <1000)
	{
		if(pBuff->wActWait >= nSecond )
		{
			pBuff->wActWait -= nSecond;
			break;
		}
		else
		{
			if(pBuff->dwTwice > 0)
			{
				pBuff->wActWait += pBuff->wInterval;
				pBuff->dwTwice--;
			}
			else
			{
				//次数已经减到0,这个buff要干掉了
				pBuff->wActWait = 0;
				pBuff->dwTwice  = 0;
				break;
			}
		}
		nLoopTime ++;
	}
}

//buff操作太频繁，这个地方应该直接设置为0，在统一的地方去移除
//这里不改原因是会影响到其他的功能，暂时先不改，新项目这个地方要修改一下。 by：glj.20140513
bool CBuffSystem::RemoveById(int nBuffId,bool boIsExpire)
{
	if (!CanDoThisFunction())
	{
		return false;
	}
	INT_PTR nBuffCount = m_buffList.count();
	for (INT_PTR i = nBuffCount-1; i > -1 ; i--)
	{
		CDynamicBuff * pDBuff = &m_buffList[i];
		if (pDBuff != NULL && pDBuff->wBuffId == nBuffId)
		{
			OnBuffRemoved(pDBuff);
			m_buffList.remove(i);
			
			if (boIsExpire && (pDBuff->pConfig->nType == aNextSkillFlag))
			{
				m_pEntity->GetSkillSystem().OnBuffExpire(pDBuff);
			}
			//pDBuff->wActWait = 0;
			//pDBuff->dwTwice = 0;
			return true;
		}
	}
	return false;
}

bool CBuffSystem::ChangeValueById(int nBuffId, GAMEATTRVALUE value)
{
	if (!CanDoThisFunction())
	{
		return false;
	}
	INT_PTR nBuffCount = m_buffList.count();
	for (INT_PTR i = nBuffCount-1; i > -1 ; i--)
	{
		CDynamicBuff * pDBuff = &m_buffList[i];
		if (pDBuff != NULL && pDBuff->wBuffId == nBuffId)
		{
			pDBuff->value = value;
			OnBuffChangeValue(pDBuff);
			return true;
		}
	}
	return false;
}

VOID CBuffSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if (!m_pEntity)
	{
		return;
	}
	INT_PTR nBuffCount = m_buffList.count();
	if (nBuffCount <= 0)
	{
		return;
	}
	int  nHeroBuffGroup  = GetLogicServer()->GetDataProvider()->GetHeroConfig().GetBuffGroup();
	for (INT_PTR i = nBuffCount-1; i > -1 ; i--)
	{
		CDynamicBuff * pDBuff = &m_buffList[i];
		if (!pDBuff || !pDBuff->pConfig)
		{
			continue;
		}
		int nBufId = pDBuff->GetId();
		byte bDelEvent = pDBuff->pConfig->btDelEvent;
		if (pDBuff->dwTwice == CDynamicBuff::Infinite || pDBuff->dwTwice > 0)
		{
			if (pDBuff->wActWait != CDynamicBuff::Infinite && pDBuff->wActWait > 0)
			{
				pDBuff->wActWait--;
			}
			if (pDBuff->wActWait == 0)
			{
				if(pDBuff->dwTwice != CDynamicBuff::Infinite && pDBuff->dwTwice > 0)
				{
					pDBuff->dwTwice--;
				}
				pDBuff->wActWait = pDBuff->pConfig->nInterval;
				TakeEffect(pDBuff);
			}
		}
		if (pDBuff->dwTwice == 0)
		{
			OnBuffRemoved(pDBuff);
			if (pDBuff->pConfig->nType == aNextSkillFlag)
			{
				m_pEntity->GetSkillSystem().OnBuffExpire(pDBuff);
			}
			m_buffList.remove(i);
			// buff 删除事件
			if(m_pEntity->GetType() == enActor && bDelEvent == 1)
			{
				((CActor*)m_pEntity)->TriggerBuffRemovedEvent(nBufId);
			}
		}else
		{
			//判断是否英雄Buff,如果没有出战英雄，则删除BUFF		   
			// if(pDBuff->pConfig->nGroup ==nHeroBuffGroup && m_pEntity->GetType() == enActor)
			// {
			//     if(!((CActor*)m_pEntity)->GetHeroSystem().GetBattleHeroPtr())
			//     {
			// 		OnBuffRemoved(pDBuff);
			// 		m_buffList.remove(i);
			// 	    // buff 删除事件
			// 	    if(bDelEvent == 1)
			// 		{
			// 		    ((CActor*)m_pEntity)->TriggerBuffRemovedEvent(nBufId);
			// 		}
			// 	}
			// }
		}
	}
}

void CBuffSystem::SendBuffData()
{
	CActorPacket AP;
	CDataPacket & netPack = ((CActor *)m_pEntity)->AllocPacket(AP);
	netPack << (BYTE )enBuffSystemID << (BYTE)sInitMainActorBuffData;
	INT_PTR nPos = netPack.getPosition();
	netPack << (BYTE)0;
	INT_PTR nBuffCount = Count();
	BYTE btCount = 0;
	for (INT_PTR i = 0; i < nBuffCount; i++)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff)
		{
			DWORD dwRestTime = pBuff->GetRemainTime();	//剩余时间
			netPack << (WORD)pBuff->pConfig->nId;
			netPack << (int)dwRestTime;
			netPack << (BYTE) AttrDataTypes[pBuff->pConfig->nType];
			netPack << pBuff->value;

			//buff CD结束时间
			int nEndTime = 0;
			if ( pBuff->pConfig->nDuration > 0 )
			{
				nEndTime = GetGlobalLogicEngine()->getMiniDateTime() + pBuff->pConfig->nDuration;
			}
			netPack << (int)nEndTime;

			btCount++;
		}
	}
	BYTE *pCount = (BYTE *)netPack.getPositionPtr(nPos);
	*pCount = (BYTE)btCount;
	AP.flush();
}

void CBuffSystem::AppendBuffData(CDataPacket &data)
{
	INT_PTR nPos = data.getPosition();
	data << (BYTE)0;
	INT_PTR nBuffCount = Count();
	BYTE btCount = 0;
	for (INT_PTR i = 0; i < nBuffCount; i++)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff)
		{
			DWORD dwRestTime = pBuff->GetRemainTime();	//剩余时间
			data << (WORD)pBuff->pConfig->nId;
			data << (int)dwRestTime;
			data << (BYTE) AttrDataTypes[pBuff->pConfig->nType];
			data << pBuff->value;
			btCount++;
		}
	}
	BYTE *pCount = (BYTE *)data.getPositionPtr(nPos);
	*pCount = (BYTE)btCount;
}

void CBuffSystem::WriteBuffData(CDataPacket & pack)
{
	if(m_pEntity ==NULL )
	{
		pack << (BYTE) 0;
		return ;
	}
	INT_PTR nType = m_pEntity->GetType();

	if(nType != enActor && nType != enMonster && nType != enPet && nType != enHero)
	{
		pack << (BYTE) 0;
		return ;
	}
	INT_PTR nPos = pack.getPosition();
	pack << (BYTE)0;
	INT_PTR nBuffCount = Count();
	//pack << (BYTE) nCount;
	BYTE btCount = 0;
	for (INT_PTR i = 0; i < nBuffCount; i++)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff)
		{
			DWORD dwRestTime = pBuff->GetRemainTime();	//剩余时间
			pack << (WORD)pBuff->pConfig->nId;
			pack << (BYTE)pBuff->pConfig->nType <<(BYTE) pBuff->pConfig->nGroup << (int)dwRestTime;
			pack.writeString(pBuff->pConfig->sName);
			pack << (BYTE) AttrDataTypes[pBuff->pConfig->nType];
			pack << pBuff->value;
			pack << (WORD) pBuff->pConfig->nInterval;
			pack << (BYTE)pBuff->pConfig->btIcon;
			pack << (BYTE)pBuff->pConfig->btDisplay;
			byte nContainerFlag = 0;
			if (pBuff->pConfig->nParam > 0 && (pBuff->pConfig->nType == aHpAdd || pBuff->pConfig->nType == aMpAdd))
			{
				nContainerFlag = pBuff->pConfig->nParam;
			}
			pack << (BYTE)nContainerFlag;
			pack << (BYTE)pBuff->pConfig->nDebuff;
			btCount++;
		}
	}
	BYTE *pCount = (BYTE *)pack.getPositionPtr(nPos);
	*pCount = (BYTE)btCount;
}

VOID CBuffSystem::CalcAttributes(CAttrCalc &calc)
{
	if(m_pEntity ==NULL) return ;
	GAMEATTR attr;
	INT_PTR nCount = m_buffList.count();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CDynamicBuff * pBuff = &m_buffList[i];
		if ( pBuff && AttrNeedCalc[pBuff->pConfig->nType])
		{
			attr.type = (byte)pBuff->pConfig->nType;
			attr.value = pBuff->value;
			calc << attr;
		}
	}
}

VOID CBuffSystem::TakeEffect(CDynamicBuff *pBuff)
{
	if( pBuff ==NULL || m_pEntity ==NULL) return; //如果是
	if( AttrNeedCalc[ pBuff->pConfig->nType] ) //如果是需要计算属性的，那么肯定是配置问题
	{
		//OutputMsg(rmTip,_T("buff error,type=%d take effect"),pBuff->btType);
		return;
	}
	if(m_pEntity->IsDeath() || m_pEntity->IsDestory()) return; //死亡了不生效
	int nValue =0;
	float fValue =0.0;
	bool bDamaged = false;
	bool bNeedCheckPropVal = false;
	bool bNeedDel =false;  //是否需要删除

	CEntity *pGiver =NULL;
	
	
	switch(pBuff->pConfig->nType)
	{
	//#pragma __CPMSG__(实现按不同的buff在周期性生效的时候产生作用的实现)
	//case 产生作用的属性类型: 对角色产生作用(); break;
	//红增加
	case aHpAdd:
		if( !pBuff->mGiver.IsNull() )
		{
			pGiver= GetEntityFromHandle(pBuff->mGiver);
		}
		if (pBuff->pConfig->nParam > 0)
		{
			int nDiffHp = m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP) - m_pEntity->GetProperty<int>(PROP_CREATURE_HP);
			if (nDiffHp > 0)
			{
				if (nDiffHp > pBuff->pConfig->nParam2)
				{
					nDiffHp = pBuff->pConfig->nParam2;
				}
				pBuff->value.nValue -= nDiffHp;
				float fRate = 0;//((CAnimal *)m_pEntity)->GetFastMedicamentRenew();
				int nRealValue = (int)(nDiffHp * (1+fRate));
				if (nRealValue > 0)
				{
					CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
					if (pDeductBuff != NULL)
					{
						int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
						if(nDeductVal <= 0)
						{
							nRealValue = 0;
						}
						else if(nDeductVal <= nRealValue)
						{
							nRealValue = nDeductVal;
						}
					}
				}
				((CAnimal *)m_pEntity)->ChangeHP(nRealValue, pGiver);
				if (pBuff->value.nValue <= 0)
				{
					bNeedDel =true;
				}
				else
				{
					OnBuffChangeValue(pBuff);
				}
			}
			
		}
		else
		{
			if (pBuff->value.nValue < 0 && m_pEntity->GetType() == enActor && ((CActor *)m_pEntity)->HasMapAttribute(aaSaft))
			{
				break;
			}
			float fRate = 0;// ((CAnimal *)m_pEntity)->GetFastMedicamentRenew();
			int nRealValue = (int)(pBuff->value.nValue * (1+fRate));
			if (nRealValue > 0)
			{
				CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
				if (pDeductBuff != NULL)
				{
					int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
					if(nDeductVal <= 0)
					{
						nRealValue = 0;
					}
					else if(nDeductVal <= nRealValue)
					{
						nRealValue = nDeductVal;
					}
				}
			}
			((CAnimal *)m_pEntity)->ChangeHP(nRealValue, pGiver);
			if(pBuff->pConfig->boFullDel) //满了要删除
			{
				if( m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP) <= m_pEntity->GetProperty<int>(PROP_CREATURE_HP))
				{
					bNeedDel =true;
				}
			}
			bNeedCheckPropVal = true;
		}
		break;
	//红倍率增加
	case aHpPower:
	{
		if( !pBuff->mGiver.IsNull() )
		{
			pGiver= GetEntityFromHandle(pBuff->mGiver);
		}
		int nRealValue = (int)(pBuff->value.fValue * m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP));
		if (nRealValue > 0)
		{
			CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
			if (pDeductBuff != NULL)
			{
				int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
				if(nDeductVal <= 0)
				{
					nRealValue = 0;
				}
				else if(nDeductVal <= nRealValue)
				{
					nRealValue = nDeductVal;
				}
			}
		}

		((CAnimal *)m_pEntity)->ChangeHP(nRealValue,pGiver);
		if(pBuff->pConfig->boFullDel) //满了要删除
		{
			if( m_pEntity->GetProperty<int>(PROP_CREATURE_MAXHP) <= m_pEntity->GetProperty<int>(PROP_CREATURE_HP))
			{
				bNeedDel =true;
			}
		}
		bNeedCheckPropVal = true;
		break;
	}
	//魔增加
	case aMpAdd:
		if( !pBuff->mGiver.IsNull() )
		{
			pGiver= GetEntityFromHandle(pBuff->mGiver);
		}
		if (pBuff->pConfig->nParam > 0)
		{
			int nDiffMp = m_pEntity->GetProperty<int>(PROP_CREATURE_MAXMP) - m_pEntity->GetProperty<int>(PROP_CREATURE_MP);
			if (nDiffMp > 0)
			{
				if (nDiffMp > pBuff->pConfig->nParam2)
				{
					nDiffMp = pBuff->pConfig->nParam2;
				}
				pBuff->value.nValue -= nDiffMp;

				float fRate = 0;// ((CAnimal *)m_pEntity)->GetFastMedicamentRenew();
				int nRealValue = (int)(nDiffMp * (1+fRate));
				if (nRealValue > 0)
				{
					CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
					if (pDeductBuff != NULL)
					{
						int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
						if(nDeductVal <= 0)
						{
							nRealValue = 0;
						}
						else if(nDeductVal <= nRealValue)
						{
							nRealValue = nDeductVal;
						}
					}
				}

				((CAnimal *)m_pEntity)->ChangeMP(nRealValue);
				if (pBuff->value.nValue <= 0)
				{
					bNeedDel =true;
				}
				else
					OnBuffChangeValue(pBuff);
			}
			
		}
		else
		{
			if (pBuff->value.nValue < 0 && m_pEntity->GetType() == enActor && ((CActor *)m_pEntity)->HasMapAttribute(aaSaft))
			{
				break;
			}
			float fRate = 0;//((CAnimal *)m_pEntity)->GetFastMedicamentRenew();
			int nRealValue = (int)(pBuff->value.nValue * (1+fRate));
			if (nRealValue > 0)
			{
				CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
				if (pDeductBuff != NULL)
				{
					int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
					if(nDeductVal <= 0)
					{
						nRealValue = 0;
					}
					else if(nDeductVal <= nRealValue)
					{
						nRealValue = nDeductVal;
					}
				}
			}

			((CAnimal *)m_pEntity)->ChangeMP(nRealValue);
			bNeedCheckPropVal = true;
			if(pBuff->pConfig->boFullDel) //满了要删除
			{
				if( m_pEntity->GetProperty<int>(PROP_CREATURE_MAXMP) <= m_pEntity->GetProperty<int>(PROP_CREATURE_MP))
				{
					bNeedDel =true;
				}
			}
		}
		break;
		//魔倍率增加
	case aMpPower:
		{
			if( !pBuff->mGiver.IsNull() )
			{
				pGiver= GetEntityFromHandle(pBuff->mGiver);
			}
			int nRealValue = (int)(pBuff->value.fValue * m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP));
			if (nRealValue > 0)
			{
				CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
				if (pDeductBuff != NULL)
				{
					int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
					if(nDeductVal <= 0)
					{
						nRealValue = 0;
					}
					else if(nDeductVal <= nRealValue)
					{
						nRealValue = nDeductVal;
					}
				}
			}

			((CAnimal *)m_pEntity)->ChangeMP(nRealValue);
			// bNeedCheckPropVal = true;
			if(pBuff->pConfig->boFullDel) //满了要删除
			{
				if( m_pEntity->GetProperty<int>(PROP_CREATURE_MAXMP) <= m_pEntity->GetProperty<int>(PROP_CREATURE_MP))
				{
					bNeedDel =true;
				}
			}
		}
		break;
	case aHpMpAdd:
		{
			unsigned int nOldValue = pBuff->value.uValue;
			for (int propId = PROP_CREATURE_HP,propMaxId = PROP_CREATURE_MAXHP; propId<= PROP_CREATURE_MP; propId++,propMaxId++)
			{
			    unsigned int nDiffHpMp = m_pEntity->GetProperty<int>(propMaxId) - m_pEntity->GetProperty<int>(propId);
				if (nDiffHpMp > 0)
				{
					unsigned int DiffParam = propId == PROP_CREATURE_HP? pBuff->pConfig->nParam:pBuff->pConfig->nParam2;
					if (nDiffHpMp > DiffParam)
					{
						nDiffHpMp = DiffParam;
					}

					float fRate = 0;
					unsigned int nRealValue = (unsigned int)(nDiffHpMp * (1+fRate));
					if (nRealValue > 0)
					{
						CDynamicBuff * pDeductBuff = GetBuff(aHPMPDeductPower, CDynamicBuff::AnyBuffGroup);
						if (pDeductBuff != NULL)
						{
							int nDeductVal = (int)(nRealValue * (1.0 + pDeductBuff->value.fValue));
							if(nDeductVal <= 0)
							{
								nRealValue = 0;
							}
							else if((unsigned int )nDeductVal <= nRealValue)
							{
								nRealValue = nDeductVal;
							}
						}
					}
					if (pBuff->value.uValue < nRealValue)
					{
						nRealValue = pBuff->value.uValue;
					}
					pBuff->value.uValue -= nRealValue;
					if (propId == PROP_CREATURE_HP)
					{
						((CAnimal *)m_pEntity)->ChangeHP(nRealValue);
					}
					else if (propId == PROP_CREATURE_MP)
					{
						((CAnimal *)m_pEntity)->ChangeMP(nRealValue);
					}
				}		
			}	
			if (pBuff->value.uValue == 0)
			{
				bNeedDel =true;
			}
			else if (nOldValue != pBuff->value.uValue)
			{
				OnBuffChangeValue(pBuff);
			}
			break;
		}
	// 红持续增加
	case aHpCure:
		{
			if( !pBuff->mGiver.IsNull() )
			{
				pGiver= GetEntityFromHandle(pBuff->mGiver);
			}
			// 血差
			int nDiffHp = (int)(m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP)) - m_pEntity->GetProperty<int>(PROP_CREATURE_HP);
			if (pBuff->value.nValue > 0 && nDiffHp > 0)
			{
				//是否有禁止回血的BUff状态
				CDynamicBuff * pForbidHpRenewBuff = GetBuff(aForbidHpRenew, CDynamicBuff::AnyBuffGroup);
				if (pForbidHpRenewBuff == NULL)
				{
					CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(pBuff->wBuffId);
					if(pBuffConf != NULL)
					{
						int nHpAdd = pBuffConf->nParam;
						nHpAdd = __min(nHpAdd, pBuff->pConfig->nParam);
						nHpAdd = __min(nHpAdd, nDiffHp);
						pBuff->value.nValue -= nHpAdd;
						((CAnimal *)m_pEntity)->ChangeHP(nHpAdd);
						if (pBuff->value.nValue <= 0)
						{
							bNeedDel = true;
						}
						else
						{
							OnBuffChangeValue(pBuff);
						}
					}
				}
			}
			else if (pBuff->value.nValue < 0)
			{
				int nCurHp = m_pEntity->GetProperty<int>(PROP_CREATURE_HP);
				CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(pBuff->wBuffId);
				if(pBuffConf != NULL)
				{
					int nHpSub = pBuffConf->nParam;
					nHpSub = __min(nHpSub, nCurHp);
					((CAnimal *)m_pEntity)->ChangeHP(nHpSub);

					if (m_pEntity->GetType() == enMonster || m_pEntity->GetType() == enPet)
					{
						if(pBuff->mGiver !=0 )
						{
							if(CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pBuff->mGiver))
							{
								((CAnimal*)m_pEntity)->OnAttacked((CAnimal*)pEntity, pBuff->bSetVest);
								((CAnimal*)m_pEntity)->GetAI()->AttackedBy((CAnimal*)pEntity);
							}
						}
					}
				}
			}
			break;
		}
	// 蓝持续增加
	case aMpCure:
		{
			// 魔差
			int nDiffMp = (int)(m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXMP)) - m_pEntity->GetProperty<int>(PROP_CREATURE_MP);
			if (pBuff->value.nValue > 0 && nDiffMp > 0)
			{
				CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(pBuff->wBuffId);
				if(pBuffConf != NULL)
				{
					int nMpAdd = pBuffConf->nParam;
					nMpAdd = __min(nMpAdd, pBuff->value.nValue);
					nMpAdd = __min(nMpAdd, nDiffMp);
					pBuff->value.nValue -= nMpAdd;
					((CAnimal *)m_pEntity)->ChangeMP(nMpAdd);
					if (pBuff->value.nValue <= 0)
					{
						bNeedDel = true;
					}
					else
					{
						OnBuffChangeValue(pBuff);
					}
				}
			}
			else if (pBuff->value.nValue < 0)
			{
				int nCurMp = m_pEntity->GetProperty<int>(PROP_CREATURE_HP);
				CBuffProvider::BUFFCONFIG* pBuffConf = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(pBuff->wBuffId);
				if(pBuffConf != NULL)
				{
					int nHpSub = abs(pBuffConf->nParam);
					nHpSub = __min(nHpSub, nCurMp);
					((CAnimal *)m_pEntity)->ChangeHP(-nHpSub);
				}
			}
			break;
		}
	default:
		break;
	}
	
	if (bNeedCheckPropVal && pBuff->value.nValue <= 0)
	{
		if (pGiver )
		{
			if (m_pEntity->GetTarget().IsNull())
				m_pEntity->SetTarget(pGiver->GetHandle());
			if (m_pEntity->GetType() == enMonster)
			{
				((CAnimal*)m_pEntity)->OnAttacked((CAnimal*)pGiver, pBuff->bSetVest);
				((CAnimal*)pGiver)->OnAttackOther((CAnimal*)m_pEntity, pBuff->bSetVest);
				pBuff->bSetVest = false;
				if( ((CAnimal *)m_pEntity)->GetAI() )
				{				
					((CAnimal *) m_pEntity)->GetAI()->AttackedBy((CAnimal *)pGiver);
				}
			}
		}
	}
	if (m_pEntity->IsDeath() && pGiver)
	{
		((CAnimal*)m_pEntity)->OnKilledByEntity(pGiver);
	}
		
	if(bNeedDel)
	{
		pBuff->dwTwice =0;
		pBuff->wActWait = 0;
	}
	//((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
}

VOID CBuffSystem::OnBuffAppend(CDynamicBuff *pBuff, bool is_notice)
{
//#pragma __CPMSG__(实现添加buff或更新buff的广播消息)
	if (!m_pEntity) return;
	Assert(pBuff);
	char buff[1024];
	if (pBuff->dwTwice < 1) return;
	DWORD dwRestTime = -1;
	if (pBuff->dwTwice != CDynamicBuff::Infinite)
		dwRestTime = (pBuff->dwTwice - 1) * pBuff->pConfig->nInterval + pBuff->wActWait; /*pBuff->dwTwice * pBuff->wInterval;*///剩余时间
	BYTE btEntityType = (BYTE)m_pEntity->GetType();
	if (btEntityType == enMonster && !pBuff->mGiver.IsNull())	//加上BUFF时设置归属
	{
		pBuff->bSetVest = true;
	}
	else
	{
		pBuff->bSetVest = false;
	}
	byte btType = (BYTE)pBuff->pConfig->nType;
	if ((btType < 0)||(btType >= GameAttributeCount)) return;
	
	if(is_notice || (btEntityType == enActor || btEntityType == enHero))
	{
		CDataPacket packet(buff,sizeof(buff));	
		packet << (BYTE )enBuffSystemID << (BYTE)sAddBuff;
		packet << (Uint64)m_pEntity->GetHandle();
		packet << (WORD)pBuff->wBuffId;
		packet << (int)dwRestTime ;
		packet << (BYTE) AttrDataTypes[btType];
		packet << pBuff->value;
		
		//buff CD结束时间
		int nEndTime = 0;
		if ( pBuff->pConfig->nDuration > 0 )
		{
			nEndTime = GetGlobalLogicEngine()->getMiniDateTime() + pBuff->pConfig->nDuration;
		}
		packet << (int)nEndTime;

		//Assert(packet.getPosition() == sizeof(buff));//数据包格式定义的时候提醒我改改buff的大小

		CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
		//if ( btEntityType == enActor)
		//{
			pSystem->BroadCast(packet.getMemoryPtr(),packet.getPosition(),true);
			//((CActor *)m_pEntity)->SendData(packet.getMemoryPtr(),packet.getPosition());
		//}
		//else if ( btEntityType == enHero)
		//{
		//	CActor *pOwner =NULL;
		//	pOwner =   ((CHero*)m_pEntity)->GetMaster();
		//	if(pOwner)
		//	{
		//		((CActor *)pOwner)->SendData(packet.getMemoryPtr(),packet.getPosition());
		//	}
		//}
	}
	//如果需要计算属性的话，就重新要设置一下
	if(AttrNeedCalc[btType ])
	{
		//设置角色刷新属性的标记
		((CAnimal *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	}
	if (btEntityType == enActor && pBuff->pConfig->nType == aChangeMonsterModle)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_MONSTER_MODELID, (unsigned int)pBuff->pConfig->dValue);
	}
	if ( btEntityType == enActor && pBuff->pConfig->nType == aHide )
	{
		m_pEntity->AddState(esStateHide);
	}
	if(btType ==aDizzy || btType == aFixed || btType == aRealFreeze)
	{
		m_pEntity->AddState(esStateDizzy);//这个是禁止移动状态
	}
	if (btType == aSlient)
	{
		m_pEntity->AddState(esStateSlient);
	}
	// if(pBuff->pConfig->nEffectId >0) //有特效，调用特效子系统
	// {
	// 	//如果不存在特效的话就添加一个时间很长的特效
	// 	if( !m_pEntity->GetEffectSystem().IsExist(pBuff->pConfig->nEffectType,pBuff->pConfig->nEffectId))
	// 	{
	// 		 int nTime =20000000  ; //添加一个24个小时的特效，这个基本上只是随buff生效而生效了
	//
	// 		CDataPacket data(buff,sizeof(buff));
	// 		data << (BYTE)enDefaultEntitySystemID <<(BYTE) sAddEffect ;
	// 		data << (Uint64)pBuff->mGiver << (Uint64)m_pEntity->GetHandle();
	// 		data << (BYTE) pBuff->pConfig->nEffectType << (WORD)pBuff->pConfig->nEffectId; //特效的类型以及ID
	// 		data << (DWORD)(nTime); //持续时间
	// 		data << (BYTE)pBuff->pConfig->nEffectTotalCount;
	// 		data << (BYTE)pBuff->pConfig->nEffectDirCount;
	// 		m_pEntity->GetObserverSystem()->BroadCast(data.getMemoryPtr(), data.getPosition(),true); 
	//	
	// 		m_pEntity->GetEffectSystem().AddEffect(pBuff->pConfig->nEffectType,pBuff->pConfig->nEffectId,nTime);
	// 	}
	// }
}


//设置颜色
void CBuffSystem::AddBuffColor(int nBuffType,int nGroup,int nPriority, unsigned int nColor)
{
	
	BUFFCOLORDATA data;
	data.nBuffType = nBuffType;
	data.nGroup = nGroup;
	data.nColor = nColor;
	data.priority = nPriority;

	//当前生效的这个buff
	if(m_nCurColorBuffIndex >=0 && m_nCurColorBuffIndex < m_buffColor.count())
	{
		PBUFFCOLORDATA pData =& m_buffColor[m_nCurColorBuffIndex];
		if ( !pData )
		{
			return;
		}
		if( pData->nGroup == nGroup && pData->nBuffType == nBuffType) //同一个
		{
			return ;
		}
		int nCurPriority = pData->priority; //优先级
		m_buffColor.add(data); //最后一个

		if(nCurPriority < nPriority) //更高优先级的来了
		{
			m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_COLOR,nColor);
			m_nCurColorBuffIndex = (int)m_buffColor.count() -1; //当前的下标
		}
	}
	else //当前没有
	{
		m_buffColor.add(data); //最后一个
		m_nCurColorBuffIndex = (int)m_buffColor.count() -1; //当前的下标
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_COLOR,nColor);
	}
}

//删除一个buff的颜色
void CBuffSystem::RemoveBuffColor(int nBuffType,int nGroup)
{
	/*
	if(m_nCurColorBuffIndex >=0 && m_nCurColorBuffIndex < m_buffColor.count())
	{
		PBUFFCOLORDATA pData = &m_buffColor[m_nCurColorBuffIndex];
		if( (nBuffId<0 || pData->nBuffId == nBuffId) && pData->nBuffType == nBuffType  ) //生效的这个干掉了
		{
			m_buffColor.remove(m_nCurColorBuffIndex); //先把当前的去掉
			m_nCurColorBuffIndex =-1;
		}
	}
	*/

	//同类的全部删除
	for(INT_PTR i= m_buffColor.count() -1; i>-1; i--)
	{
		if(m_buffColor[i].nBuffType ==nBuffType && (nGroup<0 || m_buffColor[i].nGroup == nGroup)   )
		{
			m_buffColor.remove(i);
		}
	}
	//遍历这些buff
	int nPriority =-1;
	m_nCurColorBuffIndex =-1;
	for(INT_PTR i=0 ;i < m_buffColor.count(); i++)
	{
		if(m_buffColor[i].priority > nPriority)
		{
			m_nCurColorBuffIndex = (int)i;
		}
	}
	if(m_nCurColorBuffIndex >= 0) //还有选择的
	{
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_COLOR,m_buffColor[m_nCurColorBuffIndex].nColor); //设置颜色
	}
	else
	{
		m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_COLOR,0);
	}
}

void CBuffSystem::ClearAllColor()
{
	if(m_nCurColorBuffIndex <0) return;
	m_nCurColorBuffIndex =-1;
	m_pEntity->SetProperty<unsigned int>(PROP_CREATURE_COLOR,0);
	m_buffColor.clear(); //内存清除
}

VOID CBuffSystem::OnBuffRemoved(CDynamicBuff *pBuff)
{
//#pragma __CPMSG__(实现删除buff的广播消息)
	if (!m_pEntity) return;
	CScene *pScene =m_pEntity->GetScene(); 
	if (pScene == NULL)
	{
		return;
	}
	Assert(pBuff);
	char buff[1024];
	byte btType = (BYTE)pBuff->pConfig->nType;
	if ((btType < 0)||(btType >= GameAttributeCount)) return;

	byte btGroup = (BYTE)pBuff->pConfig->nGroup;
	BYTE btEntityType = (BYTE)m_pEntity->GetType();
	//if(btEntityType == enActor || btEntityType == enHero)
	{
		CDataPacket packet(buff,sizeof(buff));
		packet <<(BYTE) enBuffSystemID << (BYTE)sDelBuff  << m_pEntity->GetHandle();
		packet << btType << btGroup;
		if (CObserverSystem * pSystem = m_pEntity->GetObserverSystem())
		{
			pSystem->BroadCast(packet.getMemoryPtr(),packet.getPosition(),true);
		}
		
		// if (btEntityType == enActor)
		// {
		// 	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
		// 	{
		// 		pSystem->BroadCast(packet.getMemoryPtr(),packet.getPosition(),false);
		// 	}
		// 	((CActor *)m_pEntity)->SendData(packet.getMemoryPtr(),packet.getPosition());
		// }
		// else 
		// {
		// 	CActor *pOwner =NULL;
		// 	pOwner =   ((CHero*)m_pEntity)->GetMaster();
		//
		// 	if(pOwner)
		// 	{
		// 		((CActor *)pOwner)->SendData(packet.getMemoryPtr(),packet.getPosition());
		// 	}
		// }
	}

	//如果需要计算属性的话，就重新要设置一下
	if(AttrNeedCalc[btType])
	{
		//设置角色刷新属性的标记
		((CAnimal *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	}
	if ( btEntityType == enActor && btType == aHide )
	{
		m_pEntity->RemoveState(esStateHide);
	}
	if (btType == aChangeMonsterModle && btEntityType == enActor)
	{
		m_pEntity->SetProperty<unsigned int>(PROP_ACTOR_MONSTER_MODELID, (unsigned int)0);
	}
	if (!Exists(aDizzy)  || !Exists(aFixed) || !Exists(aRealFreeze))
	{
		m_pEntity->RemoveState(esStateDizzy);//这个是禁止移动状态
	}
	if (btType == aSlient)
	{
		m_pEntity->RemoveState(esStateSlient);
	}
	if(btType == aDamageDropTime)
	{
		m_pEntity->GetSkillSystem().LaunchSkill(pBuff->value.nValue,0,0,true); //定时使用技能
	}
	if(pBuff->pConfig->nEffectId >0)
	{
		m_pEntity->GetEffectSystem().DelEffect(pBuff->pConfig->nEffectType,pBuff->pConfig->nEffectId);
	}
	if(pBuff->pConfig->nColor)
	{
		RemoveBuffColor(btType, pBuff->pConfig->nGroup); //需要删除这个颜色
	}

	// buff 删除事件
	if(pBuff->pConfig->btDelEvent == 2)
	{
		((CActor*)m_pEntity)->TriggerBuffRemovedEvent(pBuff->GetId());
	}
}

VOID CBuffSystem::OnTypedBuffRemoved(GAMEATTRTYPE buffType)
{
	if ((buffType < 0)||(buffType >= GameAttributeCount)) return;

//#pragma __CPMSG__(实现按buff类型批量删除buff的广播消息)
	char buff[128];
	CDataPacket packet(buff,sizeof(buff));
	packet <<(BYTE) enBuffSystemID << (BYTE)sDelBuffType << m_pEntity->GetHandle();
	packet <<(BYTE) buffType;

	//Assert(packet.getPosition() == sizeof(buff));//数据包格式定义的时候提醒我改改buff的大小

	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
	if (pSystem)
	{
		pSystem->BroadCast(packet.getMemoryPtr(),packet.getPosition(),true);
	}

	//Assert(packet.getPosition() == sizeof(buff));//数据包格式定义的时候提醒我改改buff的大小
	/*
	if(buffType ==aMoveForbid )
	{
		m_pEntity->RemoveState(esStateMoveForbid);
	}
	*/

	RemoveBuffColor(buffType,-1);

	if( buffType ==aDizzy || buffType == aFixed || buffType == aRealFreeze)
	{
		m_pEntity->RemoveState(esStateDizzy);//这个是禁止移动状态
	}
	//如果需要计算属性的话，就重新要设置一下
	if(AttrNeedCalc[buffType])
	{
		//设置角色刷新属性的标记
		((CAnimal *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
	}
}

void CBuffSystem::SaveToScriptData()
{
	if (m_pEntity->GetType() != enActor) return;		
	int saved_count = 0;
	for (INT_PTR i = 0; i < m_buffList.count(); i++)
	{
		CDynamicBuff * pBuff = &m_buffList[i];
		if (pBuff && pBuff->pConfig->boSave && pBuff->dwTwice > 0)
		{
			saved_count +=  (SaveBuffToScriptData(pBuff, saved_count) ? 1 : 0);
		}
	}

	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();
	var.set("buff_total_count", saved_count);
}

static const char *pszBuffIdPattern		= "buff_%d_id";			// Buff名称
//static const char *pszBuffTypePattern		= "buff_%d_type";			// Buff类型
//static const char *pszBuffGroupPattern		= "buff_%d_group";			// Buff组
//static const char *pszBuffIntervalPattern	= "buff_%d_interval";		// Buff作用间隔
static const char *pszBuffValuePattern		= "buff_%d_value";			// Buff属性值
//static const char *pszBuffAuxParamPattern	= "buff_%d_auxparam";		// Buff辅助参数
static const char *pszBuffStopTimePattern	= "buff_%d_stopttime";		// Buff失效时间
static const char *pszBuffLeftCountPattern	= "buff_%d_leftcount";		// Buff剩余次数（如果为0xffffffff，则一直执行）
static const char *pszBuffCdTimePattern	= "buff_%d_cdttime";		// Buffcd时间

bool CBuffSystem::SaveBuffToScriptData(CDynamicBuff *pBuff, int index)
{
	if (!pBuff) return false;
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();	

	char elem_type_name[1024] = {0};
	
	//if (pBuff->sBuffName)											
	//{
	//	sprintf(elem_type_name, pszBuffNamePattern, index);		// 名称
	//	var.set(elem_type_name, pBuff->sBuffName);
	//}
	sprintf(elem_type_name, pszBuffIdPattern, index);			// ID
	var.set(elem_type_name, pBuff->wBuffId);
	//sprintf(elem_type_name, pszBuffGroupPattern, index);		// 组
	//var.set(elem_type_name, pBuff->btGroup);
	//sprintf(elem_type_name, pszBuffIntervalPattern, index);		// 间隔
	//var.set(elem_type_name, pBuff->wInterval);
	sprintf(elem_type_name, pszBuffValuePattern, index);		// 属性值
	var.set(elem_type_name, pBuff->value.nValue);	
	//sprintf(elem_type_name, pszBuffAuxParamPattern, index);		// Buff辅助参数
	//var.set(elem_type_name, pBuff->wAuxParam);	
	sprintf(elem_type_name, pszBuffLeftCountPattern, index);	// Buff剩余次数
	var.set(elem_type_name, pBuff->dwTwice);

	unsigned int nStopTime = CalcBuffStopTime(pBuff);			// 计算Buff失效时间
	
	sprintf(elem_type_name, pszBuffStopTimePattern, index);		// Buff失效时间
	var.set(elem_type_name, nStopTime);
	sprintf(elem_type_name, pszBuffCdTimePattern, index);		// Buff cd时间
	var.set(elem_type_name, pBuff->m_nBuffCd);
	return true;
}


void CBuffSystem::LoadFromScriptData()
{		
	CActor *pSelf = (CActor *)m_pEntity;
	CCLVariant &var = pSelf->GetActorVar();
	char elem_type_name[1024] = {0};
	CCLVariant *pVar = 0;
	pVar = var.get("buff_total_count");
	if (pVar){

		int buff_total_count = (int)(double)(*pVar);
		CDynamicBuff buff;
		for (INT_PTR index = 0; index < buff_total_count; index++)
		{
			sprintf(elem_type_name, pszBuffIdPattern, index);			// 类型	
			if (pVar = var.get(elem_type_name))
			{
				buff.wBuffId = (WORD)((double)(*pVar));		
				pVar->clear();
			}
			if (buff.wBuffId > 0)
			{
				buff.pConfig = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(buff.wBuffId);
			}
			if (buff.wBuffId == 0 && buff.pConfig == NULL)
			{
				continue;
			}
			//sprintf(elem_type_name, pszBuffGroupPattern, index);		// 组
			//if (pVar = var.get(elem_type_name))
			//{
			//	buff.btGroup = (BYTE)((double)(*pVar));	
			//	pVar->clear();
			//}

			//sprintf(elem_type_name, pszBuffIntervalPattern, index);		// 间隔
			//if (pVar = var.get(elem_type_name))
			//{
			//	buff.wInterval = (WORD)((double)(*pVar));
			//	pVar->clear();
			//}

			sprintf(elem_type_name, pszBuffValuePattern, index);		// 属性值
			if (pVar = var.get(elem_type_name))
			{
				buff.value.nValue = (int)((double)(*pVar));
				pVar->clear();
			}

			//sprintf(elem_type_name, pszBuffAuxParamPattern, index);		// Buff辅助参数
			//if (pVar = var.get(elem_type_name))
			//{
			//	buff.wAuxParam = (WORD)((double)(*pVar));
			//	pVar->clear();
			//}

			sprintf(elem_type_name, pszBuffLeftCountPattern, index);	// Buff剩余次数
			if (pVar = var.get(elem_type_name))
			{
				buff.dwTwice = (unsigned int)((double)(*pVar));
				pVar->clear();
			}

			int nNextEffectInterval = 0;
			sprintf(elem_type_name, pszBuffStopTimePattern, index);		// Buff剩余时间
			if (pVar = var.get(elem_type_name))
			{
				unsigned int dwStopTime = (unsigned int)((double)(*pVar));
				nNextEffectInterval = CalcBuffNextEffectInterval(buff, dwStopTime);
				buff.wActWait = nNextEffectInterval;
				pVar->clear();
			}
			else
				buff.dwTwice = 0;
			
			int nCdTime = 0;
			sprintf(elem_type_name, pszBuffCdTimePattern, index);		// Buff cd时间
			if (pVar = var.get(elem_type_name))
			{
				buff.m_nBuffCd = (unsigned int)((double)(*pVar));
				pVar->clear();
			}

			// Add Buff
			if (buff.dwTwice > 0)
			{
				//buff.wActWait = buff.pConfig->nInterval;
				Append(buff.wBuffId, &buff, false);
			}
		}
	} 


	SendBuffData();
}

// 根据剩余次数，计算出失效时间，用于下线计时
unsigned int CBuffSystem::CalcBuffStopTime(const CDynamicBuff *pBuff)
{
	unsigned int nStopTime = GetGlobalLogicEngine()->getMiniDateTime();
	if (pBuff && pBuff->dwTwice != CDynamicBuff::Infinite)
	{
		if(pBuff->dwTwice >0)
		{
			nStopTime += (pBuff->dwTwice-1) * pBuff->pConfig->nInterval + pBuff->wActWait;
		}
		else
		{
			nStopTime += pBuff->wActWait;
		}
	}

	return nStopTime;
}

//计算距离下次生效的时常
int CBuffSystem::CalcBuffNextEffectInterval(CDynamicBuff &buff, unsigned int dwStopTime)
{
	int nNextEffectInterval = 0; // 距离下次生效的时常
	if (buff.dwTwice != CDynamicBuff::Infinite)
	{
		unsigned int nCurrTime = GetGlobalLogicEngine()->getMiniDateTime();
		if (dwStopTime > nCurrTime && buff.pConfig->nInterval > 0)
		{
			buff.dwTwice = (dwStopTime - nCurrTime) / buff.pConfig->nInterval;
			nNextEffectInterval = (dwStopTime - nCurrTime) % buff.pConfig->nInterval;
		}
		else
		{
			buff.dwTwice = 0;
		}	
		
		if (nNextEffectInterval != 0)
			buff.dwTwice++;

	}

	return nNextEffectInterval;
}

bool CBuffSystem::Exists( int nBuffId )
{
	const CBuffProvider::BUFFCONFIG * pBuffConfig = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
	if (pBuffConfig != NULL)
	{
		GAMEATTRTYPE buffType =(GAMEATTRTYPE)pBuffConfig->nType;
		return Exists(buffType, pBuffConfig->nGroup);
	}
	return false;

}

void CBuffSystem::OnDeadRemoveBuff()
{
	if ( !CanDoThisFunction() )
	{
		return;
	}
	INT_PTR nCount = Count();
	for (INT_PTR i = nCount - 1; i > -1; i--)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff && pBuff->pConfig->boDieRemove)
		{
			OnBuffRemoved(pBuff);
			m_buffList.remove(i);
			//pBuff->wActWait = 0;
			//pBuff->dwTwice = 0;
		}
	}
}

void CBuffSystem::RemoveBuffGiver()
{
	if ( !CanDoThisFunction() )
	{
		return;
	}
	INT_PTR nCount = Count();
	for (INT_PTR i = nCount - 1; i > -1; i--)
	{
		CDynamicBuff* pBuff = &m_buffList[i];
		if (pBuff && pBuff->pConfig && pBuff->pConfig->boDelGiverByLeaveGuild)
		{
			pBuff->mGiver = NULL;
		}
	}
}

void CBuffSystem::OnBuffChangeValue( CDynamicBuff * pBuff )
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor || !pBuff )
	{
		return;
	}
	CActorPacket AP;
	CDataPacket & netPack = ((CActor *)m_pEntity)->AllocPacket(AP);
	netPack << (byte)GetSystemID() << (byte)sChangeBuffVallue;
	netPack << (Uint64)m_pEntity->GetHandle();
	netPack << (WORD)pBuff->wBuffId;
	netPack << pBuff->value;
	AP.flush();
}

bool CBuffSystem::CanDoThisFunction()
{
	if ( !m_pEntity || !m_pEntity->isAnimal() || !m_pEntity->IsInited() || m_pEntity->IsDestory() )
	{
		return false;
	}
	return true;
}

bool CBuffSystem::IsHaveChangeModelBuff()
{
	bool boResult = false;
	if ( Exists(aChangeMonsterModle))
	{
		boResult = true;
	}
	return boResult;
}

void CBuffSystem::OnBuffChangeCdTime( CDynamicBuff * pBuff )
{
	if ( !m_pEntity || m_pEntity->GetType() != enActor || !pBuff )
	{
		return;
	}
	CActorPacket AP;
	CDataPacket & netPack = ((CActor *)m_pEntity)->AllocPacket(AP);
	netPack << (byte)GetSystemID() << (byte)sChangeBuffCdTime;
	// netPack << (Uint64)m_pEntity->GetHandle();
	netPack << (WORD)pBuff->wBuffId;
	netPack << (int)pBuff->m_nBuffCd;
	AP.flush();
}