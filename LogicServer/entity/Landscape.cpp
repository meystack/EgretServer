// #include "StdAfx.h"
// #include "Landscape.h"

// VOID CLandscape::LogicRun(TICKCOUNT nCurrentTime)
// {
// 	//延迟删除这个实体
// 	if(m_expireTime <0) return;
// 	if(nCurrentTime >= m_expireTime)
// 	{
// 		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle(),true);
// 	}
// }




// void CLandscape::Destroy()
// {
// 	Inherited::Destroy();
// }

// bool CLandscape::Init(void * data, size_t size)
// {
// 	if( Inherited::Init(data,size) ==false ) return false; 
// 	return true;
// } 

//  VOID CLandscape::SetExpiredTime(INT_PTR nTime)
//  {
// 	 if(nTime <=0)
// 	 {
// 		 m_expireTime = -1;
// 	 }
// 	 else
// 	 {
// 		 m_expireTime = GetLogicCurrTickCount() + nTime;
// 	 }
//  }

//  //创建
//  CLandscape * CLandscape::CreateLandscape( CScene *pScene, int nPosX,int nPosY,int nModelId, int nTime,LPCTSTR sName)
//  {
// 	 if(pScene ==NULL ) return NULL;

// 	 if(!pScene->CanMove(nPosX,nPosY)) return NULL; //不能移动的地方不能召唤出火
// 	 const MapEntityGrid *pEntityGrid = pScene->GetMapEntityGrid(nPosX);
// 	 if(pEntityGrid ==NULL ) return NULL;
// 	 CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
// 	 const EntityPosVector & entitys = pEntityGrid->m_entitys;
// 	 CLandscape* pEntity  =NULL;
// 	 INT_PTR nEntityCount = entitys.count();
// 	 for(INT_PTR i= nEntityCount -1; i >=0  ;i --)
// 	 {
// 		 int nPos = (int)entitys[i].pos;
// 		 if(nPos != nPosY) continue;  //判断同一个坐标的
// 		 EntityHandle hd = entitys[i].hd;

// 		 if( hd.GetType() == enLandscape)
// 		 {
// 			 pEntity =(CLandscape*) pMgr->GetEntity(hd);
// 			 if(pEntity && pEntity->GetProperty<int>(PROP_ENTITY_MODELID) == nModelId) //如果已经有同样的路灯就不生成，避免出现一堆一样的
// 			 {
// 				 return NULL; //已经有了就返回
// 			 }
// 		 }
// 	 }


// 	pEntity =  (CLandscape*)(pScene->CreateEntityAndEnterScene(0, enLandscape, nPosX, nPosY));
	
// 	if(pEntity)
// 	{
// 		pEntity->SetProperty<int>(PROP_ENTITY_MODELID,nModelId); //设置模型的id
// 		pEntity->SetExpiredTime(nTime);
// 		pEntity->SetEntityName(sName);
// 	}
	
	
// 	 return pEntity;
//  }