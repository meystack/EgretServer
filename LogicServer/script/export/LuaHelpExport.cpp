#include "StdAfx.h"
#include "../interface/LuaHelpExportFun.h"
using namespace wylib;
using namespace wylib::stream;


//Note向LuaHelp导出的一些辅助性的函数在这里要放一份
//不然脚本无法访问
const luaL_Reg  LuaHelpExpot[]=
{
	{"saveTable",LuaHelp::saveTable},
	{"getTestTable",LuaHelp::getTestTable},
	{"getTeamMemberList",LuaHelp::getTeamMemberList},
	{"getTeamMemberListByPtr",LuaHelp::getTeamMemberListByPtr},
	{"getFbFromId",LuaHelp::getFbFromId},
	{"getReloadMapPos",LuaHelp::getReloadMapPos},
	{"getNearEntityList",LuaHelp::getNearEntityList},
	{"getAllActorList",LuaHelp::getAllActorList},
	{"getSceneActorListById",LuaHelp::getSceneActorListById},
	{"getSceneAliveActorListById",LuaHelp::getSceneAliveActorListById},
	{"getNearEntityListFromXY",LuaHelp::getNearEntityListFromXY},
	{"getNearEntityListFromRange",LuaHelp::getNearEntityListFromRange},
	{"getRankingItemList",LuaHelp::getRankingItemList},
	{"getFbActorList",LuaHelp::getFbActorList},
	{"getVisiActorList",LuaHelp::getVisiActorList},

	{ NULL, NULL }, //这行不要删除，删除服务器将无法启动
};


namespace LuaHelp
{
	int saveTable(lua_State *L)
	{
		if (lua_istable(L, 1) ==false)
		{
			OutputMsg(rmError,_T("saveTable要作为参数传过来"));
			return 0;
		}
		const char * sFileName = lua_tostring(L,2);
		if(sFileName ==NULL)
		{
			OutputMsg(rmWaning,_T("saveTable表的名字为空，保存为./demo.txt"));
			sFileName ="./demo.txt";
		}
		const char * sTableName = lua_tostring(L,3);
		CFileStream stm(sFileName, CFileStream::faWrite | CFileStream::faCreate,CFileStream::AlwaysCreate );
		if(NULL !=sTableName)
		{
			stm.write(sTableName,strlen(sTableName));
			stm.write(" =\r\n",4);
		}
		
		FormatTableA(L,1,stm);
		return 0;
		
	}

	int getTestTable(lua_State *L)
	{
		float a[10];
		for(int i=0;i< 10;i++)
		{
			a[i]= (float)1221.3443;
		}
		PushNumberVector(L,(float*)(&a[0]),10);
		return 1; //返回1个参数
	}

	int getTeamMemberList(lua_State *L)
	{
		void* member[MAX_TEAM_MEMBER_COUNT];

		void* pEntity = ((void*)  tolua_touserdata(L,1,0));
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;
		CTeam* pTeam = ((CActor*)pEntity)->GetTeam();
		if (!pTeam) return 0;

		int nCount = 0;
		const TEAMMEMBER* pMember=  pTeam->GetMemberList();

		for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
		{
			if(pMember[i].pActor)
			{
				member[nCount++] = pMember[i].pActor;
			}
		}

		LuaHelp::PushDataPointerToTable(L,member,nCount);

		return 1;
	}

	int getTeamMemberListByPtr( lua_State *L )
	{
		void* member[MAX_TEAM_MEMBER_COUNT];

		void* pTeam = ((void*)  tolua_touserdata(L,1,0));
		if (!pTeam) return 0;

		int nCount = 0;
		const TEAMMEMBER* pMember=  ((CTeam*)pTeam)->GetMemberList();

		for(INT_PTR i=0; i< MAX_TEAM_MEMBER_COUNT; i++)
		{
			if(pMember[i].pActor)
			{
				member[nCount++] = pMember[i].pActor;
			}
		}

		LuaHelp::PushDataPointerToTable(L,member,nCount);

		return 1;
	}



	int getFbFromId( lua_State *L )
	{
		int nFbid = (int)tolua_tonumber(L,1,0);
		unsigned int Buf[256];//最多返回256个副本
		int nCount = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbCountFromId(nFbid,Buf,256);
		LuaHelp::PushNumberVector(L,Buf,nCount);
		return 1;
	}


	int getReloadMapPos( lua_State *L )
	{
		void* pEntity = ((void*)  tolua_touserdata(L,1,0));
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return 0;

		int pos[3];
		((CActor*)pEntity)->GetNotReloadMapPos(pos[0],pos[1],pos[2]);
		LuaHelp::PushNumberVector(L,pos,3);
		return 1;
	}


	int getNearEntityList( lua_State *L )
	{
		void* pEntity = ((void*)  tolua_touserdata(L,1,0));
		if(pEntity == NULL) return 0;
		int nRadio = (int)tolua_tonumber(L,2,0);
		int nType = (int)tolua_tonumber(L, 3, -1);
		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		CScene* pScene = ((CEntity*)pEntity)->GetScene();
		if (pScene)
		{
			pScene->GetVisibleList(((CEntity*)pEntity),vec,-nRadio,nRadio,-nRadio,nRadio);
			if (vec.count() > 0)
			{
				CVector<void*> pEntityList;
				pEntityList.clear();
				CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
				for (int i = 0; i < vec.count(); i++)
				{
					CEntity* pe = pMgr->GetEntity(vec[i]);
					if (pe && (-1 == nType || pe->GetType() == nType))
					{
						pEntityList.add(pe);
					}
				}
				LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
			}else
			{
				LuaHelp::PushDataPointerToTable(L,NULL,0);
			}
		}
		else
		{
			LuaHelp::PushDataPointerToTable(L,NULL,0);
		}
		return 1;
	}


	int getNearEntityListFromXY( lua_State *L )
	{
		void* pScene = ((void*)  tolua_touserdata(L,1,0));
		if(pScene == NULL) return 0;
		int x = (int)tolua_tonumber(L,2,0);
		int y = (int)tolua_tonumber(L,3,0);
		int nRadio = (int)tolua_tonumber(L,4,0);

		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();
		
		((CScene*)pScene)->GetVisibleList(x,y,vec,-nRadio,nRadio,-nRadio,nRadio);
		if (vec.count() > 0)
		{
			CVector<void*> pEntityList;
			pEntityList.clear();
			CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
			for (int i = 0; i < vec.count(); i++)
			{
				CEntity* pe = pMgr->GetEntity(vec[i]);
				if (pe)
				{
					pEntityList.add(pe);
				}
			}
			LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		}else
		{
			LuaHelp::PushDataPointerToTable(L,NULL,0);
		}
		
		return 1;
	}


	int getNearEntityListFromRange( lua_State *L )
	{
		void* pScene = ((void*)  tolua_touserdata(L,1,0));
		if(pScene == NULL) return 0;
		int x = (int)tolua_tonumber(L,2,0);
		int y = (int)tolua_tonumber(L,3,0);
		int width = (int)tolua_tonumber(L,4,0);
		int height = (int)tolua_tonumber(L,5,0);
		int type = (int)tolua_tonumber(L,6,0);

		EntityVector& vec = *(CFuBenManager::m_pVisibleList);
		vec.clear();

		((CScene*)pScene)->GetVisibleList(x,y,vec,-width,width,-height,height);
		if (vec.count() > 0)
		{
			CVector<void*> pEntityList;
			pEntityList.clear();
			CEntityManager* pMgr = GetGlobalLogicEngine()->GetEntityMgr();
			for (int i = 0; i < vec.count(); i++)
			{
				CEntity* pe = pMgr->GetEntity(vec[i]);
				if (pe && pe->GetType() == type)
				{
					pEntityList.add(pe);
				}
			}
			LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		}else
		{
			LuaHelp::PushDataPointerToTable(L,NULL,0);
		}

		return 1;
	}



	int getAllActorList( lua_State *L )
	{
		CVector<void*> pEntityList;
		pEntityList.clear();
		int nMinLevel = (int)(tolua_tonumber(L, 1, 0));
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(pEntityList, nMinLevel);
		LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		return 1;
	}


	int getSceneActorListById( lua_State *L )
	{
		int sid = (int)tolua_tonumber(L,1,0);
		CScene* pScene = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0)->GetScene(sid);
		if (pScene)
		{
			CVector<void*> pEntityList;
			pEntityList.clear();

			CEntityList& list = pScene->GetPlayList();
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
				if (pEntity && pEntity->GetType() == enActor)
				{
					pEntityList.add(pEntity);
				}		
			}
			LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		}
		else
		{
			LuaHelp::PushDataPointerToTable(L,NULL,0);
		}
		return 1;
	}

	int getSceneAliveActorListById(lua_State *L)
	{
		int sid = (int)tolua_tonumber(L,1,0);
		CScene* pScene = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0)->GetScene(sid);
		if (pScene)
		{
			CVector<void*> pEntityList;
			pEntityList.clear();

			CEntityList& list = pScene->GetPlayList();
			CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
			CLinkedNode<EntityHandle> *pNode;
			CLinkedListIterator<EntityHandle> it(list);
			for (pNode = it.first(); pNode; pNode = it.next())
			{
				CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
				if (pEntity && pEntity->GetType() == enActor)
				{
					if(pEntity->IsDeath() == false)
						pEntityList.add(pEntity);
				}		
			}
			LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		}
		else
		{
			LuaHelp::PushDataPointerToTable(L,NULL,0);
		}
		return 1;
	}

	int getRankingItemList( lua_State* L )
	{
		void* pRank = ((void*)  tolua_touserdata(L,1,0));
		if(pRank == NULL) return 0;
		int nCount = (int)tolua_tonumber(L,2,0);//需要返回前N名

		CVector<CRankingItem*>& itemList = ((CRanking*)pRank)->GetList();
		CVector<void*>* pList = (CVector<void*>*)(&itemList);
		nCount = __min(nCount, (int)itemList.count());
		LuaHelp::PushDataPointerToTable(L,(void**)(*pList),nCount);

		return 1;
	}

	int getFbActorList( lua_State* L )
	{
		void* pFb = ((void*)  tolua_touserdata(L,1,0));
		if(pFb == NULL) return 0;

		CVector<void*> pEntityList;
		pEntityList.clear();

		CFuBen::SceneList& sl = ((CFuBen*)pFb)->GetSceneList();
		CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
		for ( INT_PTR i = 0; i < sl.count(); i++)
		{
			CScene* pScene = sl[i];
			if (pScene)
			{
				CEntityList& list = pScene->GetPlayList();
				CLinkedNode<EntityHandle> *pNode;
				CLinkedListIterator<EntityHandle> it(list);
				for (pNode = it.first(); pNode; pNode = it.next())
				{
					CEntity* pEntity = pEntityMgr->GetEntity(pNode->m_Data);
					if (pEntity && pEntity->GetType() == enActor)
					{
						pEntityList.add(pEntity);
					}		
				}
			}
		}

		LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());

		return 1;
	}


	int getVisiActorList( lua_State* L )
	{
		void* pEntity = ((void*)  tolua_touserdata(L,1,0));
		if(pEntity == NULL) return 0;

		CVector<void*> pEntityList;
		pEntityList.clear();
		const CVector<EntityHandleTag>& entityList = ((CAnimal*)pEntity)->GetObserverSystem()->GetVisibleList();
		for (INT_PTR i = 0; i < entityList.count(); i++)
		{
			CEntity* pEntity = GetEntityFromHandle(entityList[i].m_handle);
			if (pEntity && pEntity->GetType() == enActor)
			{
				pEntityList.add(pEntity);
			}
		}

		LuaHelp::PushDataPointerToTable(L,(void**)(pEntityList),pEntityList.count());
		return 1;
	}




}
