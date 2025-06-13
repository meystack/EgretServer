#include "StdAfx.h"
#include "Fire.h"

VOID CFire::LogicRun(TICKCOUNT nCurrentTime)
{
	//延迟删除这个实体
	bool bNeedDestroy =false; //是否需要删除

	if(nCurrentTime >= m_expireTime)
	{
		
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle(),true);
		return;

	}

	CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CAnimal * pMaster =(CAnimal*)pMgr->GetEntity(m_ownerHandle); //如果已经下线的话，要删除
	if(pMaster ==NULL || pMaster->IsDeath())
	{
		bNeedDestroy =true;
	}
	else
	{
		CScene *pScene = pMaster->GetScene();
		if(pScene == NULL)
		{
			bNeedDestroy =true;
		}
		else
		{
			int nPosX, nPosY;
			pMaster->GetPosition(nPosX,nPosY);

			SCENEAREA* pArea = pScene->GetAreaAttri(nPosX,nPosY);	

			if (pScene->HasMapAttribute(nPosX,nPosY,aaNoFire,pArea))
			{
				bNeedDestroy =true;
			}
		}
	}

	if(bNeedDestroy)
	{
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle(),true);
		return;
	}
	

	if (m_attackTimer.CheckAndSet(nCurrentTime,true) )  
	{
		OnTimeDamage(pMaster);
	}
	
	
}

 void CFire::OnFireDamage(CAnimal * pMaster ,CAnimal *pAnimal)
 {
	 SKILLRESULT  result; //技能的结果
	 result.nDelay = 0;
	 result.nValue = m_nAttackValueAdd; 
	 result.nId =(int)m_nAttackRate; //都是100攻击
	 result.nResultType =srSkillResultMagicAttack;
	 result.btHitType = htFire;

	 SKILLRESULTPARAM param;

	 param.pSrcEntity = pMaster;
	 param.pTargetEntity = (CAnimal*)pAnimal;
	 param.pSkillResult =& result;
	 pMaster->GetSkillSystem().GetSkillResult()->DoResult(&param);//释放肉搏技能
 }



//实体碰撞了火
void CFire::OnCollideFire(CAnimal *pAnimal)
{
	CAnimal * pMaster =GetMasterPtr();
	if(pMaster ==NULL)return;
	int nPosX,nPosY;
	GetPosition(nPosX,nPosY);
	CScene * pScene = GetScene();
	if(pScene ==NULL) return;
	
	if (pScene->HasMapAttribute(nPosX,nPosY,aaSaft)) return;

	if(pMaster->CanAttack(pAnimal))
	{
		OnFireDamage(pMaster,pAnimal);
	}
}
//定时伤害
void CFire::OnTimeDamage(CAnimal *pMaster)
{
	//主人下线了就不造成伤害了
	
	int nPosX,nPosY;
	GetPosition(nPosX,nPosY);
	CScene * pScene = GetScene();
	if(pScene ==NULL) return;

	const MapEntityGrid *pEntityGrid = pScene->GetMapEntityGrid(nPosX);
	if(pEntityGrid ==NULL ) return ;

	if (pScene->HasMapAttribute(nPosX,nPosY,aaSaft)) return;

	const EntityPosVector & entitys = pEntityGrid->m_entitys;
	
#ifndef GRID_USE_STL_MAP
	if(entitys ==NULL) return;
#else
	if(entitys.empty()) return;
#endif

	CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	CAnimal * pAnimal;

#ifndef GRID_USE_STL_MAP
	INT_PTR nEntityCount = entitys.count();
	for(INT_PTR i=nEntityCount-1; i>=0 ;i --)
	{
		if(entitys[i].pos != nPosY) continue;

		EntityHandle had = entitys[i].hd;
#else
	typedef typename EntityPosVector::const_iterator Iter;
	Iter it = entitys.begin(),end = entitys.end();
	for (; it != end; it++)
	{
		if((*it).second != nPosY) continue;
		EntityHandle had = (*it).first;
#endif
		switch(had.GetType())
		{
		case enActor:
		case enMonster:
		case enPet:
			pAnimal =(CAnimal*) pMgr->GetEntity(had);
			if(pAnimal ==NULL) continue;
			
			if(pAnimal->IsInited() && pMaster->CanAttack(pAnimal)) //如果能够攻击的话
			{
				OnFireDamage(pMaster,pAnimal);
			}
			break;
		default:
			break;
		}
	}
}

inline CAnimal * CFire::GetMasterPtr()
{
	return (CAnimal*)GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(m_ownerHandle);
}

void CFire::Destroy()
{
	Inherited::Destroy();
}
bool CFire::Init(void * data, size_t size)
{
	if( Inherited::Init(data,size) ==false ) return false; 
	m_expireTime =0;
	m_nAttackRate =0;
	m_nAttackValueAdd =0;
	//m_attackTime =0;

	m_attackTimer.SetNextHitTime(GetLogicCurrTickCount());//这里要设置下，一进入游戏就发起一次攻击
	return true;
} 

//设置过期的时间
 void  CFire::SetExpireTime(unsigned int nMiSecond)
 {
	 m_expireTime =GetGlobalLogicEngine()->getTickCount() + nMiSecond; //过期的时间设置一下
	 //m_attackTimer.SetNextHitTimeFromNow((INT_PTR)m_attackTimer.GetINTERVAL()); //
 }

 //创建一团火
 CFire * CFire::CreateFire( CScene *pScene, int nPosX,int nPosY,CCreature *pMaster,int nModelId,unsigned int nTime,int nAttackRate,int nAttackValue)
 {
	 if(pScene ==NULL || pMaster ==NULL) return NULL;
	 if(!pScene->CanMove(nPosX,nPosY)) return NULL; //不能移动的地方不能召唤出火
	 SCENEAREA* pArea = pScene->GetAreaAttri(nPosX,nPosY);	
	 if(pScene->HasMapAttribute(nPosX,nPosY,aaNoFire,pArea))
	 {
		 return NULL;
	 }
	 const MapEntityGrid *pEntityGrid = pScene->GetMapEntityGrid(nPosX);
	 if(pEntityGrid ==NULL ) return NULL;
	 CFire* pEntity  =NULL;
	 //如果已经有了一个实体，那么把这个实体设置为自己的，并且把参数重新设置一下
	 CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	 const EntityPosVector & entitys = pEntityGrid->m_entitys;
	 
#ifndef GRID_USE_STL_MAP
	 INT_PTR nEntityCount = entitys.count();
	 for(INT_PTR i=nEntityCount -1; i >=0  ;i --)
	 {
		 int nPos = (int)entitys[i].pos;
		 if(nPos != nPosY ) continue;
		 EntityHandle hd = entitys[i].hd;
#else
	typedef typename EntityPosVector::const_iterator Iter;
	Iter it = entitys.begin(),end = entitys.end();
	for (; it != end; it++)
	{
		if((*it).second != nPosY) continue;
		const EntityHandle& hd = (*it).first;
#endif
		 if(hd.GetType() == enFire)
		 {
			pEntity =(CFire*) pMgr->GetEntity(hd);
			if(pEntity)
			{
				return NULL; //已经有了就返回
			}
		 }
	 }

	pEntity =  (CFire*)(pScene->CreateEntityAndEnterScene(0, enFire, nPosX, nPosY));
	 if(pEntity)
	 {
		 pEntity->SetInitFlag(true);
		 pEntity->SetProperty<int>(PROP_ENTITY_MODELID,nModelId); //设置模型的id
		 pEntity->SetExpireTime(nTime);	 
		 pEntity->SetMasterHandle(pMaster->GetHandle()); //设置主人的handle
		 pEntity->SetAttackParam(nAttackRate,nAttackValue);
	 }
	 return pEntity;
 }
