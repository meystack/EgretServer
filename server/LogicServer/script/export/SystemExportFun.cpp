#include "StdAfx.h"
#include "../interface/SystemExportFun.h"
#include <time.h>
#include "md5.h"

namespace System
{
	static char	toLuaString[1024];	//用于返回给lua的字符串

	/*
	* Comments: 获取服务器的名字
	* @Return char *:返回服务器的名字
	*/
	char* getServerName()
	{
		return (char *)GetLogicServer()->getServerName();
	}

	char* getPfId()
	{
		static char *sSpid = (char *)GetLogicServer()->GetVSPDefine().GetDefinition("SPID");
		return sSpid;
	}

	char* getYYKey()
	{
		static char *YYKey = (char *)GetLogicServer()->GetVSPDefine().GetDefinition("YYKey");
		return YYKey;
	}
	
	char* getGameFlag()
	{
		static char *GameFlag = (char *)GetLogicServer()->GetVSPDefine().GetDefinition("GameFlag");
		return GameFlag;
	}

	/*
	* Comments: 获取服务器的ID
	* @Return int: 返回服务器的id
	*/
	int getServerId()
	{
		return GetLogicServer()->GetServerIndex();
	}
	
	/*
	* Comments: 获取当前的年月日
	  调用方法: nYear,nMonth,nDay = System.getTime( nYear,nMonth,nDay  )
	* Param int & nYear: 年
	* Param int & nMonth:月
	* Param int & nDay:日
	* @Return void:
	*/
	void getDate(int & nYear, int &nMonth, int &nDay)
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		nYear = sysTime.wYear;
		nMonth = sysTime.wMonth;
		nDay = sysTime.wDay;
	}
	
	/*
	* Comments: 获取当前的小时 ，分钟，秒数
	调用方法: nHour,nMinute,nSecond = System.getTime(nHour,nMinute,nSecond )
	* Param int & nHour:小时
	* Param int & nMinute:分钟
	* Param int & nSecond:秒
	* @Return void:
	*/
	void getTime(int & nHour, int &nMinute, int &nSecond, int &nMiliSecond)
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		nHour = sysTime.wHour;
		nMinute = sysTime.wMinute;
		nSecond = sysTime.wSecond;
		nMiliSecond = sysTime.wMilliseconds;
	}
	
	/*
	* Comments:获取当前是一周的第几天
	* @Return int:返回一个星期的第几天
	*/
	int getDayOfWeek()
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		return sysTime.wDayOfWeek;
	}

	/*
	* Comments:获取当前是本月的第几天
	* @Return int:返回一个月的第几天
	*/
	int getDayOfMonth()
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		return sysTime.wDay;
	}

	/*
	* Comments:获取当前是第几个月
	* @Return int:返回第几个月
	*/
	int getMonthOfNow()
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		return sysTime.wMonth;
	}

	
	/*
	* Comments:获取当前的时间滴答（开机以来）
	* @Return Uint64:返回当前的时间滴答
	*/
	unsigned long long getTick( )
	{
		return GetLogicServer()->GetLogicEngine()->getTickCount();
	}
	
	
	/*
	* Comments:获取一个取模nModule的随机数
	* Param int nModule: 模数
	* @Return int: 返回取模后的随机数
	*/
	unsigned int getRandomNumber( int nModule)
	{
		if( nModule <=0 )return 0;
		return   ( (unsigned int) wrandvalue() + (unsigned int)_getTickCount() ) % (unsigned int) nModule ;
	}
	

	
	/*
	* Comments: 按场景的名字和位置传送实体
	* Param CEntity * pEntity:实体的指针
	* Param char * sSceneName:  场景的名字
	* Param int nPosX:坐标x
	* Param int nPosY:坐标y
	* @Return bool: 成功返回true，否则返回false
	*/
	bool telportSceneByName(void * pEntity, char * sSceneName, int nPosX, int nPosY, int nType)
	{
		if(pEntity ==NULL || sSceneName ==NULL )return false;
		return ( (CEntity*)pEntity)->TelportSceneByName(sSceneName,nPosX,nPosY, nType);
	}

	int getSceneIdByName(char *sName)
	{
		 if(sName ==NULL) return -1;
		 CFuBen * pRetFb =NULL;
		 CScene * pRetScene =NULL;
		 GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneName(sName,pRetFb,pRetScene);
		 if(pRetScene ==NULL ) return -1;
		 return pRetScene->GetSceneId();
	}

	bool telportScene(void * pEntity, int nSceneId, int nPosX, int nPosY)
	{
		if(pEntity ==NULL || nSceneId <0 ) return false;
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		if (((CEntity*)pEntity)->GetType() == enActor )
		{
			CFuBen * pFb = ((CActor*)pEntity)->GetFuBen();
			if( pFb != NULL && pFb->IsFb())
			{
				((CActor*)pEntity)->GetFubenSystem().ReqExitFuben(pFb->GetFbId());
				return true;
			}
		}
		return GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen((CEntity*)pEntity,(CFuBen *)NULL,nSceneId,nPosX,nPosY,enDefaultTelePort,nEffId);
	}

	void telportSceneDelay( void * pEntity, int nSceneId, int nPosX, int nPosY,int nTime/*=0*/ )
	{
		if(pEntity ==NULL || nSceneId <0 ) return ;
		CEntityMsg msg(CEntityMsg::emRealTransfer);
		msg.nParam1 = nSceneId; //要减的血
		msg.nParam2 = nPosX;
		msg.nParam3 = nPosY;
		msg.dwDelay = nTime;//延迟掉血
		((CEntity*)pEntity)->PostEntityMsg(msg); 
	}

	/*
	* Comments: 传送到场景的默认点
	* Param CEntity * pEntity: 实体的指针
	* Param char * sSceneName:场景的名字
	* Param int nPointID:默认点的ID
	* @Return bool: 成功返回true，否则返回false
	*/
	bool telportSceneDefaultPoint(void * pEntity, char * sSceneName,int nPointID)
	{
		if(pEntity ==NULL || sSceneName ==NULL) return false;
		return	((CEntity*)pEntity)->TelportSceneDefaultPoint(sSceneName,nPointID);
	}
	
	/*
	* Comments: 初始化脚本的网络协议分发器
	* @Return void:
	*/
	void InitScriptNetmsgDispatcher()
	{
		NetMsgDispatcher::Init();
	}

	/*
	* Comments: 注册协议回调
	* Param int sysId: 主协议id
	* Param int cmdId: 子协议id
	* @Return void:
	*/
	void regScriptNetMsg(int sysId, int cmdId)
	{
		//static GameEngine* ge = GetGameEngine();
		NetMsgDispatcher::RegisterScriptMsg(sysId, cmdId);
	}

	/*
	* Comments: 初始化脚本的事件分发器
	* @Return void:
	*/
	void InitScriptActorEventDispatcher()
	{
		EventDispatcher::Init();
	}

	/*
	* Comments: 注册事件回调
	* Param int enType: 实体类型
	* Param int evId: 事件id
	* @Return void:
	*/
	void regScriptEvent(int nType, int evId)
	{
		if (nType == enActor)
		{
			EventDispatcher::RegistActorEvent(evId);
		}
	}

	/*
	* Comments: 打印调试信息
	* Param char * str: 调试信息
	* @Return void:
	*/
	void trace(char * str)
	{
		if(str ==NULL) return;
		OutputMsg(rmTip, "[Script Trace]%s", str);
	}

	bool deepCreateDirectory(char * sDirName)
	{
		return FDOP::DeepCreateDirectory(LPCTSTR(sDirName));
	}

	bool fileExists(char* sFilePath)
	{
		return FDOP::FileExists(LPCTSTR(sFilePath));
	}

	unsigned int getCurrMiniTime()
	{
		return GetGlobalLogicEngine()->getMiniDateTime();
	}

	bool isSameWeek(unsigned int t1, unsigned int t2)
	{
		return GetGlobalLogicEngine()->IsSameWeek(t1,t2);
	}

	bool isSameDay(unsigned int t1, unsigned int t2)
	{
		CMiniDateTime ta;
		ta.tv = t1;
		CMiniDateTime tb;
		tb.tv = t2;
		if (ta.isSameDay(tb))
			return true;
		else
			return false;
	}

	void* getActorPtr( char* sName )
	{
		if (sName == NULL || *sName == 0) return NULL;
		return GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
	}

	void broadcastTipmsg(char * sTipmsg,int nTipmsgType,int nLevel)
	{

		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(sTipmsg,nTipmsgType,nLevel);

	}

	void broadcastTipmsgCs(char * sTipmsg, int nTipmsgType, bool bCsFlag)
	{
		char szData[1024] = {0};
		CDataPacket packet(szData, sizeof(szData));
		packet << (WORD)jxInterSrvComm::SessionServerProto::fcBroadcastTipmsg;
		packet << (BYTE)nTipmsgType;
		size_t nLen = strlen(sTipmsg);
		packet.writeString(sTipmsg, nLen);
		CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
		pSSClient->BroadMsgToCommonLogicClient(packet.getMemoryPtr(), packet.getLength());
		if (bCsFlag)
			broadcastTipmsg(sTipmsg, nTipmsgType);
	}

	void broadcastTipmsgLimitLev(char * sTipmsg,int nTipmsgType,int nLevel)
	{
		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsg(sTipmsg,nTipmsgType,nLevel);
	}
	
	void broadcastTipmsgWithID(int nTipmsgID, int nTipmsgType)
	{
		GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsgWithID(nTipmsgID,nTipmsgType);
	}

	void broadTipmsgWithParams(int	nTipmsgID,int nTipmsgType,char * sParam1,char *sParam2,char * sParam3)
	{
		
		if(sParam1 ==NULL)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->BroadcastTipmsgWithID(nTipmsgID,
				nTipmsgType);
		}
		else if(sParam2 ==NULL)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(nTipmsgID,
				nTipmsgType,sParam1);
		}
		else if(sParam3 ==NULL)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(nTipmsgID,
				nTipmsgType,sParam1,sParam2);
		}
		else
		{
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(nTipmsgID,
				nTipmsgType,sParam1,sParam2,sParam3);
		}
	
		
	}
	
	int getEntityByPtr(void* pEntity)
	{
		CEntity * ptr= (CEntity * )pEntity;
		if(NULL == ptr) return -1;
		return (int)(ptr->GetType());
	}

	
	int getEntityTypeByHandle(unsigned long long entityHandle)
	{
		if(entityHandle ==0) return -1;
		EntityHandle hd(entityHandle);
		CEntity * ptr = GetEntityFromHandle(hd);
		if(ptr ==NULL ) return  -1;
		
		return (int)(ptr->GetType());
	}
	/*
	* Comments: 通过玩家的actorID获取玩家的指针
	* Param unsigned int nActorID:actorID
	* @Return void *:玩家的指针
	*/
	void * getEntityPtrByActorID(unsigned int nActorID)
	{
		return (void*)(GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorID));
	}

	int registerTimeCall(lua_State *L)
	{
		CNpc *pNPC = (CNpc*)lua_touserdata(L, 1);
		LPCTSTR sFnName = lua_tostring(L, 2);
		LPCTSTR sNextCall = lua_tostring(L, 3);
		UINT dwSecInterval = (UINT)lua_tointeger(L, 4);
		bool boBootCall = lua_toboolean(L, 5) != FALSE;

		//为了保证逻辑性能，禁止脚本注册周期低于1分钟的周期函数
		if (dwSecInterval < 60)
			return 0;

		CScriptValueList args;
		args.getArguments(L, 6);
		
		HANDLE hCall = GetLogicServer()->GetLogicEngine()->GetScriptTimeCallManager().RegisterTimeCall(pNPC,
			sFnName, sNextCall, dwSecInterval, boBootCall, args);
		hCall ? lua_pushlightuserdata(L, hCall) : lua_pushnil(L);
		return 1;
	}

	void* getTimeCall(void *pNPC, char *sFnName)
	{
		return GetLogicServer()->GetLogicEngine()->GetScriptTimeCallManager().GetTimeCall((CNpc*)pNPC, sFnName);
	}

	bool unregisterTimeCall(void *hCall)
	{
		return GetLogicServer()->GetLogicEngine()->GetScriptTimeCallManager().UnregisterTimeCall(hCall);
	}

	unsigned int getToday()
	{
		return CMiniDateTime::today();
	}

	unsigned int getOpenServerToday()
	{
		CMiniDateTime openServerTime     = GetLogicServer()->GetServerOpenTime(); //获取开服的时间
		return  openServerTime.today();
	}

	unsigned int getOpenServerRelToday()
	{
		CMiniDateTime openServerTime     = GetLogicServer()->GetServerOpenTime(); //获取开服的时间
		return  openServerTime.rel_today();
	}

	bool isInTimeRange( int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec )
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		int year = sysTime.wYear;
		int month = sysTime.wMonth;
		int day = sysTime.wDay;
		CMiniDateTime start; 
		start.encode(year,month,day,nStartHour,nStartMin,nStartSec);
		CMiniDateTime end ;
		end.encode(year,month,day,nEndHour,nEndMin,nEndSec);
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		if ( ( nNow >= start && nNow <= end ) || ( nNow >= end && nNow <= start))
		{
			return true;
		}
		else return false;
	}
	
	unsigned int getTimeRange( int nStartHour,int nStartMin,int nStartSec,int nEndHour,int nEndMin,int nEndSec )
	{
		SYSTEMTIME &sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		int year = sysTime.wYear;
		int month = sysTime.wMonth;
		int day = sysTime.wDay;
		CMiniDateTime start; 
		start.encode(year,month,day,nStartHour,nStartMin,nStartSec);
		CMiniDateTime end ;
		end.encode(year,month,day,nEndHour,nEndMin,nEndSec);
		return start < end ? (end - start) : (start - end);
	}

	bool isInDateRange(int nStartYear,int nStartMonth,int nStartDay,int nStartHour,int nStartMin,int nStartSec,int nEndYear,int nEndMonth,int nEndDay,int nEndHour,int nEndMin,int nEndSec)
	{
		CMiniDateTime start; 
		start.encode(nStartYear,nStartMonth,nStartDay,nStartHour,nStartMin,nStartSec);
		CMiniDateTime end ;
		end.encode(nEndYear,nEndMonth,nEndDay,nEndHour,nEndMin,nEndSec);
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		if ( ( nNow >= start && nNow <= end ) || ( nNow >= end && nNow <= start))
		{
			return true;
		} 
		return false;
	}

	bool isInOffsetTimeRange(const char* sStartTime, unsigned int nOffset, unsigned int nError)
	{
		SYSTEMTIME sysTime = GetLogicServer()->GetLogicEngine()->getSysTime();
		int year = sysTime.wYear;
		int month = sysTime.wMonth;
		int day = sysTime.wDay;
		sscanf(sStartTime, "%d:%d:%d", &sysTime.wHour, &sysTime.wMinute, &sysTime.wSecond);
		CMiniDateTime nStart;
		nStart.encode(sysTime);
		CMiniDateTime nEnd;
		nEnd = nStart + nOffset + nError;
		UINT nNow = GetGlobalLogicEngine()->getMiniDateTime();
		if ((nNow >= nStart && nNow <= nEnd) || (nNow >= nEnd && nNow <= nStart))
		{
			return true;
		}
		else 
			return false;
	}

	char* getRandDateTime( const char* sStarTime,const char* sEndTime )
	{
		SYSTEMTIME startSysTime;
		SYSTEMTIME endSysTime;
		sscanf(sStarTime, "%d-%d-%d %d:%d:%d", &startSysTime.wYear, &startSysTime.wMonth, &startSysTime.wDay, &startSysTime.wHour, &startSysTime.wMinute, &startSysTime.wSecond);
		sscanf(sEndTime, "%d-%d-%d %d:%d:%d", &endSysTime.wYear, &endSysTime.wMonth, &endSysTime.wDay, &endSysTime.wHour, &endSysTime.wMinute, &endSysTime.wSecond);
		CMiniDateTime nStart,nEnd;
		nStart.encode(startSysTime);
		nEnd.encode(endSysTime);
		if (nStart > nEnd)
		{
			return NULL;
		}
		UINT seed = nEnd - nStart;
		seed = wrand(seed);
		nStart = nStart + seed;
		nStart.decode(startSysTime);
		sprintf_s(toLuaString,sizeof(toLuaString),"%d-%02d-%02d %02d:%02d:%02d",startSysTime.wYear, startSysTime.wMonth, startSysTime.wDay, startSysTime.wHour, startSysTime.wMinute, startSysTime.wSecond);
		return toLuaString;
	}

	unsigned int encodeNumTime(int nYear,int nMonth,int nDay,int nHour,int nMin,int nSec)
	{
		CMiniDateTime nTime; 
		nTime.encode(nYear,nMonth,nDay,nHour,nMin,nSec);

		return nTime;
	}
	unsigned int encodeTime(char* sTime)
	{
		CMiniDateTime nTime; 
		//nTime.encode(sTime);
		CBackStageSender::StdTimeStr2MiniTime(sTime, nTime);
		return nTime;
	}
	void* getGlobalNpc()
	{
		return GetGlobalLogicEngine()->GetGlobalNpc();
	}
	void decodeTime(unsigned int nMiniTime, int& nYear, int& nMonth, int& nDay)
	{
		CMiniDateTime nTime = nMiniTime;
		SYSTEMTIME sysTime;
		ZeroMemory(&sysTime,sizeof(sysTime));
		nTime.decode(sysTime);
		nYear = sysTime.wYear;
		nMonth = sysTime.wMonth;
		nDay = sysTime.wDay;
	}

	char* getGuildName(unsigned int nGuidID)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuidID);
		if(NULL ==pGuild )
		{
			return NULL;
		}
		else
		{
			return pGuild->m_sGuildname;
		}
	}

	void changeGuildCoin(unsigned int nGuidID,int nValue, int nLogId, const char*  pStr)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuidID);
		if(NULL ==pGuild )
		{
			return;
		}
		else
		{
			pGuild->ChangeGuildCoin(nValue, nLogId, pStr);
		}
	}

	void* getGuildLeader( unsigned int nGuildID )
	{
		// CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildID);
		// if(NULL ==pGuild )
		// {
		// 	return NULL;
		// }
		// else
		// {
		// 	if (!pGuild->m_pLeader) return NULL;
		// 	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(pGuild->m_pLeader->hActorHandle);
		// 	if (pEntity && pEntity->GetType() == enActor)
		// 	{
		// 		return pEntity;
		// 	}
		// }
		return NULL;
	}

	unsigned int getGuildLeaderId( unsigned int nGuildID )
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildID);
		if(NULL ==pGuild )
		{
			return 0;
		}
		else
		{
			return pGuild->m_nLeaderId;

		}
		return 0;
	}
	unsigned int getTomorrow()
	{
		return CMiniDateTime::tomorrow();
	}

	void registerMonsterDeath(int nMonsterID,bool flag)
	{
		PMONSTERCONFIG  pConfig= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID );
		if(pConfig)
		{
			pConfig->pflags->bDeathTriggerScript =flag;
		}
	}

	void registerMonsterMob(int nMonsterID, bool flag )
	{
		PMONSTERCONFIG  pConfig= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID );
		if(pConfig)
		{
			pConfig->pflags->bMobTriggerScript =flag;
		}
	}

	void registerMonsterLiveTimeOut(int nMonsterID, bool flag )
	{
		PMONSTERCONFIG  pConfig= GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterID );
		if(pConfig)
		{
			pConfig->pflags->bLiveTimeOutTriggerScript =flag;
		}
	}

	bool setObjectVar(const char *pszKey, CBaseScriptExportObject *object)
	{
		return GlobalObjectMgr::getSingleton().SetGlobalObject(pszKey, object);
	}

	int getObjectVar(lua_State *L)
	{
		if (lua_gettop(L) < 1 || !lua_isstring(L, 1))
		{
			lua_pushnil(L);
			return 1;
		}

		const char *pKey = ((const char *) lua_tostring(L, 1));	
		CBaseScriptExportObject *obj = (CBaseScriptExportObject*)GlobalObjectMgr::getSingleton().GetGlobalObject(pKey);
		if (obj)
			tolua_pushusertype(L, (void*)obj, obj->getClassName());	
		else lua_pushnil(L);
		return 1;
	}

	bool removeObjectVar(const char *pszKey)
	{	
		return GlobalObjectMgr::getSingleton().RemoveObject(pszKey);
	}
	
	
	/*
	* Comments: 设置全局指针对象。如果对象是一个引用计数对象，则调用此函数前调用者必须显示调用addRef。
	            如果传递空对象则形同于removeObjectVarPtr。
	* Param const char * sKey:
	* Param void * ptr:
	* @Return bool:
	*/
	bool setObjectVarPtr(const char* sKey, void* ptr)
	{
		GlobalObjectMgr &mgr = GlobalObjectMgr::getSingleton();
		if (mgr.GetGlobalObject(sKey) && ptr != NULL)
			return false;
		mgr.SetGlobalObject(sKey, ptr);
		return true;
	}
	/*
	* Comments: 获取全局指针对象，当对象不再有用时必须调用release函数释放对象
	* Param const char * sKey:
	* @Return void*:
	*/
	void* getObjectVarPtr(const char* sKey)
	{
		return GlobalObjectMgr::getSingleton().GetGlobalObject(sKey);
	}

	int getDyanmicVar( lua_State *L )
	{
		return LuaCLVariant::returnValue(L, GetGlobalLogicEngine()->GetDyanmicVar());
	}

	int getStaticVar( lua_State *L )
	{
		return LuaCLVariant::returnValue(L, GetGlobalLogicEngine()->GetGlobalVarMgr().GetVar());
	}

	//获取排行榜的第几名的角色id
	unsigned int GetActorIdByCsRank(INT_PTR nRankIndex,int nRank)
	{
		return GetGlobalLogicEngine()->GetTopTitleMgr().GetActorIdByCsRank(nRankIndex,nRank);
	}

	char * getSystemDef(char * pDefName)
	{
		if(pDefName ==NULL )return NULL;
		return (char *)GetLogicServer()->GetVSPDefine().GetDefinition(pDefName);
	}

	void maintainGuildWeekly()
	{
		GetGlobalLogicEngine()->GetGuildMgr().MaintainGuildWeekly();
	}

	void CallmaintainTopTitle()
	{
		GetGlobalLogicEngine()->GetTopTitleMgr().maintainTopTitle();
	}

	unsigned int getServerOpenTime()
	{
		return GetLogicServer()->GetServerOpenTime();
	}

	unsigned int getServerCombineTime()
	{
		return GetLogicServer()->GetServerCombineTime();
	}

	int getDaysSinceCombineServer()
	{
		return GetLogicServer()->GetDaysSinceCombineServer();
	}

	void playScrEffect( int nEffId,int nSec )
	{
		char data[128];
		CDataPacket pack(data,sizeof(data));
		pack << (BYTE)enMiscSystemID << (BYTE)sScrEffect << (WORD)nEffId << (int)nSec;
		GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(data,pack.getPosition());
	}

	void playScrEffectCode( int nEffId,int nSec,int x ,int y )
	{
		char data[128];
		CDataPacket pack(data,sizeof(data));
		pack << (BYTE)enMiscSystemID << (BYTE)sScrEffectCode << (WORD)nEffId << (int)nSec << (int)x << (int)y;
		GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(data,pack.getPosition());
	}

	void playSceneScrEffectCode(int nEffId, int nSec, int nScendId, int x,int y)
	{
		char data[128];
		CDataPacket pack(data,sizeof(data));
		pack << (BYTE)enMiscSystemID << (BYTE)sScrEffectCode << (WORD)nEffId << (int)nSec << (int)x << (int)y;
		GetGlobalLogicEngine()->GetEntityMgr()->BroadCastScene(data, pack.getPosition(), nScendId);
	}

	void sendOfflineMsg( unsigned int nActorid,char* sMsg,int nSrc )
	{
		CActor::AddTxtMsg(nActorid,sMsg,nSrc);
	}

	int setIntBit(unsigned int nValue, unsigned int nBitPos, bool bBit)
	{
		if (nBitPos >= (32)) return -1;		
		if (bBit == true)
		{
			nValue |= (0x1 << nBitPos);
		}
		else
		{
			nValue &= ~(0x1 << nBitPos);
		}
		
		return nValue;
	}

	int getIntBit(unsigned int nValue, unsigned int nBitPos)
	{
		if (nBitPos >= (32)) return -1;
		//return nValue & ((0x1) << nBitPos);
		return (nValue >> nBitPos) & 0x1;

	}

	int getValueMAKELONG(unsigned int param1, unsigned int param2,unsigned int param3)
	{
		int nValue = 0;
		// printf("getValueMAKELONG param1:%d, param2:%d, param3:%d\n",param1,param2,param3);
		nValue = MAKELONG( param1, MAKEWORD( param2, param3));
		return nValue;
	}

	void refreshStore( int nLabel)
	{
		// GetLogicServer()->GetDataProvider()->GetGameStore().ResetDynamicMerchadise(nLabel);
	}

	int getDaysSinceOpenServer()
	{
		return GetLogicServer()->GetDaysSinceOpenServer();
	}

	void SetShutup(void *pEntity, bool bForbid, int nDuration)
	{
		if(!pEntity || ((CEntity *)pEntity)->GetType() != enActor) 
			return;

		CActor *pActor = (CActor *)pEntity;
		pActor->GetChatSystem()->SetShutup(bForbid, nDuration);
	}



	unsigned int findGuildIdByHandle(unsigned int nHandle)
	{
		if(nHandle<=0) return 0;

		return GetGlobalLogicEngine()->GetGuildMgr().FindGuildIdByHandle(nHandle);
	}




	void sendGuildMsg(unsigned int nGuildId,int guildPos,const char* sMsg,int nType)
	{
		CGuild *pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild)
		{
			pGuild->SendGuildMsg(guildPos,sMsg,nType);
		}
	}

	void changeGuildFr( unsigned int nGuildId, int value )
	{
		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if (pGuild)
		{
			pGuild->SetGuildFr(value);
		}
	}

	char *getSkillNameById(int nSkillId)
	{
		return (char *)GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillId);
	}

	int getLocalServerId()
	{
		return GetLogicServer()->GetServerIndex();
	}

	int getCommonServerId()
	{
		return GetLogicServer()->GetCommonServerId();
	}

	void setTeleportVisible(bool bVisible,unsigned int hScene,int x,int y)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			if(pScene->m_pTelePortTable ==NULL) return;

			INT_PTR nIndex = pScene->m_pTelePortTable[y*pScene->GetSceneWidth() + x];

			if (nIndex != 0 && pScene->GetSceneData() && nIndex <= pScene->GetSceneData()->TeleportList.nCount)
			{
				TeleportConfig& config = pScene->GetSceneData()->TeleportList.pList[nIndex-1];

				CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(config.nHandle);

				if(pEntity)
				{
					pEntity->GetAttriFlag().DenySee = !bVisible;	
					config.bUse = bVisible;	
				}
			}
		}
	}
	bool isTeleportVisible(unsigned int hScene,int x,int y)
	{
		CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		if (pScene)
		{
			if(pScene->m_pTelePortTable ==NULL) return false;

			INT_PTR nIndex = pScene->m_pTelePortTable[y*pScene->GetSceneWidth() + x];

			if (nIndex != 0 && pScene->GetSceneData() && nIndex <= pScene->GetSceneData()->TeleportList.nCount)
			{
				TeleportConfig& config = pScene->GetSceneData()->TeleportList.pList[nIndex-1];

				CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(config.nHandle);

				if(pEntity)
				{
					return config.bUse;
				}
			}
		}
		return false;
	}


	bool telportRandPos( void * pEntity )
	{
		if(pEntity ==NULL) return false;
		CEntity * pMoveEntity = (CEntity*)pEntity;
		if (pMoveEntity->GetType() != enActor)
		{
			return false;
		}
		CScene * pScene = pMoveEntity->GetScene();
		if (!pScene)
		{
			return false;
		}
		CSkillResult::RandTransfer(pMoveEntity->GetFuBen(), pScene, pMoveEntity);
		return true;
	}

	bool telportLastSceneRandPos( void * pEntity )
	{
		if(pEntity ==NULL) return false;
		if(( (CEntity *)pEntity)->GetType() != enActor) return false;
		CActor * pActor = (CActor *)pEntity;
		int nSceneId = pActor->GetCitySceneId();
		CFuBen * pFb=GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if(pFb ==NULL) return false;

		CScene *pScene= pFb->GetScene(nSceneId);

		CSkillResult::RandTransfer(NULL,pScene,pActor);
		return true;
	}

	void rfeshConsumeRank()
	{
		CGameStoreRank &rank = GetLogicServer()->GetDataProvider()->GetGameStore().GetConsumeRank();
		rank.RfeshConsumeRank();
	}

	// void updateRankData()
	// {
	// 	GetGlobalLogicEngine()->GetTopTitleMgr().UpdateRankData();
	// }

	void updateVocNpcStatue(int nType)
	{
		GetGlobalLogicEngine()->GetTopTitleMgr().UpdateVocNpcStatue(nType);
	}

	void saveCsRank()
	{
		GetGlobalLogicEngine()->GetTopTitleMgr().SaveCsRank();
	}
	
	void loadCsRank()
	{
		GetGlobalLogicEngine()->GetTopTitleMgr().LoadCsRank();
	}


	int getServerIdByCsRank(unsigned int nActorId)
	{
		return GetGlobalLogicEngine()->GetTopTitleMgr().GetServerIdByCsRank(nActorId);
	}

	void loadUserOfflineDataCs(int nServerId, unsigned int nActorId)
	{
		GetGlobalLogicEngine()->GetTopTitleMgr().LoadUserOfflineData(nServerId, nActorId);
	}
	
	void delServerMail(int nServerMailId)
	{
		GetGlobalLogicEngine()->GetMailMgr().DelServerMail(nServerMailId);
	}

	void setGuildExipredTime(unsigned int nGuildId,int nValue)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild )
		{
			pGuild->SetGuildExipredTime(nValue);
		}
	}

	double getGuildDartHandle(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild )
		{
			return pGuild->GetGuildDartHandle();
		}
		else
		{
			return 0;
		}
	}

	void setGuildDartHandle(unsigned int nGuildId,int nValue)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(pGuild )
		{
			pGuild->SetGuildDartHandle(nValue);
		}
	}

	

	

	char * getMonsterNameById(int nMonsterId)
	{		
		return GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterNameById(nMonsterId);
	}


	int getMonsterTimeById(int nSceneId, int nMonsterId,bool bDie)
	{
		int nFlushTime = 0;
		CFuBen * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if(NULL == pFb)
			return 0;
		CScene * pScene = pFb->GetScene(nSceneId);
		if (pScene != NULL)
		{
			TICKCOUNT nCurTick = GetLogicCurrTickCount();
			nFlushTime = pScene->GetFlushTime(nMonsterId, nCurTick,true,bDie);
		}
		return nFlushTime;
	}

	int getToxicFreshTimes(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(NULL ==pGuild )
		{
			return 0;
		}
		else
		{
			return pGuild->GetToxicFreshTimes();
		}
	}

	void setToxicFreshTimes(unsigned int nGuildId,int nTimes)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(NULL ==pGuild )
		{
			return;
		}
		else
		{
			pGuild->SetToxicFreshTimes(nTimes);
		}
	}

	void setkillToxicNum(unsigned int nGuildId,int nValue)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(NULL ==pGuild )
		{
			return;
		}
		else
		{
			pGuild->SetkilToxicNum(nValue);
		}
	}

	int getkillToxicNum(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(NULL ==pGuild )
		{
			return 0;
		}
		else
		{
			return pGuild->GetkillToxicNum();
		}
	}

	void onSbkMasterOpenBuff(unsigned int nGuildId,int nBuffType,int nGroupID,double dValue,int nTimes, int nInterval,char * buffName,bool timeOverlay, void *pGiver, int param)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(NULL ==pGuild )
		{
			return;
		}
		else
		{
			pGuild->SbkMasterOpenBuff(nBuffType,nGroupID,dValue,nTimes, nInterval,buffName,timeOverlay, pGiver,param);
			pGuild->AddBuffToUnion(nBuffType,nGroupID,dValue,nTimes, nInterval,buffName,timeOverlay, pGiver,param);
		}
	}

	int filterWords( char * sMsg )
	{
		return GetGlobalLogicEngine()->GetChatMgr().Filter(sMsg);
	}

	bool sendPataMasterMsg( char * actorname, char * sMsg, char * sTitle, int bType, int nFubenId, int nDeadLine )
	{
		return CActor::AddPaTaAwardMsgByName(actorname, sMsg, sTitle, (byte)bType, nFubenId, nDeadLine);
	}
	
	bool addItemMsgById(unsigned int nActorId, int nSrcActorId, int nItemId, int nCount, int bBind, 
		char * sTitle, char * sBtn, int nWay, char * sComment, bool needLog, int nMsgType, int nParam)
	{
		CUserItemContainer::ItemOPParam param;
		param.wItemId = (WORD)nItemId;
		param.wCount = (WORD)nCount;
		param.btQuality = (BYTE)0;//默认0
		param.btStrong = (BYTE)0;//默认0
		param.btBindFlag = (BYTE)bBind;//默认0

		return CActor::AddItemMsg(nActorId,nSrcActorId,param,sTitle,sBtn,nWay,sComment,needLog,nMsgType,nParam);
	}

	bool addItemMsgByName(char * sActorName, int nSrcActorId, int nItemId, int nCount, int bBind, char * sTitle, char * sBtn, int nWay)
	{
		CUserItemContainer::ItemOPParam param;
		param.wItemId = (WORD)nItemId;
		param.wCount = (WORD)nCount;
		param.btQuality = (BYTE)0;//默认0
		param.btStrong = (BYTE)0;//默认0
		param.btBindFlag = (BYTE)bBind;//默认0

		return CActor::AddItemMsgByName(sActorName,nSrcActorId,param,sTitle,sBtn,nWay);
	}

	int getMonsterGrowInfo( int nMonsterId, int & nGrowLv, int & nCurExp)
	{
		return CBossSystem::GetMonsGrowInfo(nMonsterId, nGrowLv, nGrowLv);
	}

	void sendItemOutPutToLog()
	{
		GetLogicServer()->GetDataProvider()->GetStdItemProvider().SendItemOutPutToLog();
	}

	int getMerchandisePrice(unsigned int nGoodsId, int &btDealGoldType, int &nPrice, int &nQuality, int &nStrong, int &nBind)
	{
		CGameStore& store= GetLogicServer()->GetDataProvider()->GetGameStore();
		CGameStore::PMERCHANDISE pMerchandise = store.GetMerchandise(nGoodsId);

		if (pMerchandise)
		{
			btDealGoldType = pMerchandise->btDealGoldType;
			nPrice = pMerchandise->dwPrice;
			nQuality = pMerchandise->bQuality;
			nStrong = pMerchandise->bStrong;
			nBind = (pMerchandise->boBind ? 1 : 0);
			return pMerchandise->wItemId;
		}

		return 0;
	}

	bool isStartCommonServer()
	{
		return GetLogicServer()->IsStartCommonServer();
	}

	bool isCommonServer()
	{
		return GetLogicServer()->IsCommonServer();
	}

	unsigned int getPlayerMaxLevel(int nCircle)
	{
		return GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(nCircle);
	}

	void saveFiles()
	{
		GetLogicServer()->GetDataProvider()->SaveRunTimeData();
	}
	
	unsigned int getRealtimeMiniTime()
	{
		return CMiniDateTime::now();
	}





	bool sendCommonMsgByName(int nMsgId, char * sName, char * sMsg, int nSrcActorId, char * sParam1,char * sParam2,int nParam1,int nParam2)
	{/*
		if (nMsgId == CMsgSystem::mtSendFire)
		{
			CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(sName);
			if (pActor != NULL)
			{
				return pActor->GetMsgSystem().SendFireMsgInfo(0, nParam1, nParam2, sParam1, sParam2);
			}
			return false;
		}
		else*/
			return CActor::AddCommonMsgByActorName(nMsgId, sName, sMsg, nSrcActorId, nParam1, nParam2,sParam1);
	}

	bool getchangeNameFlag()
	{
		return GetLogicServer()->GetchangeNameFlag();
	}

	void sendGuildListToDestServer()
	{
		//GetGlobalLogicEngine()->GetGuildMgr().SendGuildListToDestServer();
	}

	int getGuildChallengeTimes(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;

		return pGuild->GetGuildChallengeTimes();
	}

	void setGuildChallengeTimes(unsigned int nGuildId,int nTimes)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;

		pGuild->SetGuildChallengeTimes(nTimes);
	}

	void setGuildNextChallengeTime(unsigned int nGuildId,int nTime)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;

		pGuild->SetGuildNextChallengeTime(nTime);
	}

	int getGuildNextChallengeTime(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;

		return pGuild->GetGuildNextChallengeTime();
	}

	void setGuildBossLevel(unsigned int nGuildId,int nLevel)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;

		pGuild->SetGuildBossLevel(nLevel);
	}

	int getGuildBossLevel(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;
		
		return pGuild->GetGuildBossLevel();
	}


	//设置挑战副本的句柄
	void setBossChanllengeHandle(unsigned int nGuildId,unsigned int bHandle)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;

		pGuild->SetBossChanllengeHandle(bHandle);
	}

	//获得帮派副本句柄
	unsigned int geBossChanllengeHandle(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;

		return pGuild->GeBossChanllengeHandle();
	}

	int	getGuildbuildingLev(unsigned int nGuildId,int nType)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;
		return pGuild->GetGuildbuildingLev(nType);
	}

	void changeGuildBuildingLev(unsigned int nGuildId,int nType,int nValue)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;
		pGuild->ChangeGuildBuildingLev(nType,nValue);
	}

	int getGuildBuildingCdTime(unsigned int nGuildId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return 0;
		return pGuild->GetGuildBuildingCdTime();
	}

	void setGuildBuildingCdTime(unsigned int nGuildId,int nTime)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;
		pGuild->SetGuildBuildingCdTime(nTime);
	}

	void sendGuildChanllengeResult(unsigned int nGuildId,unsigned int nActorId,int nType,int nResult)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;
		pGuild->SendGuildChanllengeResult(nActorId,nType,nResult);
	}

	//沙巴克奖励
	void sendGuildSBKRank(unsigned int nGuildId,int nRankId)
	{
		CGuild * pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
		if(!pGuild) return;
		pGuild->SendSbkGuildRankAward(nRankId);
	}

	//沙巴克奖励
	void sendSBKGuild()
	{
		GetGlobalLogicEngine()->GetGuildMgr().SendSbkGuildAward();
	}
	
	/// 暂时屏蔽该功能
	void CrossBattlefieldSignup(void* sysarg, bool bAdd){
		//CActor* pActor((CActor*)sysarg); 

		//if (pActor)
		//{
		//	GetGlobalLogicEngine()->GetCrossManager().CrossBattlefieldSignup(pActor, bAdd);
		//}
	}
	int GetCurrentCrossBattlefieldPlayerNum(){
		return 0;
		//return GetGlobalLogicEngine()->GetCrossManager().GetCurrentCrossBattlefieldPlayerNum();
	}

	bool HadCrossBattlefieldSignup(void* sysarg){
		//CActor* pActor((CActor*)sysarg); 
		//if (pActor)
		//{
		//	CCrossComponet::CrossBattlefieldData& cbData(GetGlobalLogicEngine()->GetCrossManager().cbData);
		//	//auto it(crossMgr.cbData.SignupPlayers.find(pActor->GetActorId()));
		//	auto it(find(begin(cbData.SignupPlayers), end(cbData.SignupPlayers), pActor->GetActorId()));
		//	return it != end(cbData.SignupPlayers);
		//}
		return false;
	}

	void initCrossBattlefieldData(int nTriggerNewActivity, int nNewActivityPlayers){
		//CCrossComponet& crossMgr(GetGlobalLogicEngine()->GetCrossManager());
		//crossMgr.cbData = CCrossComponet::CrossBattlefieldData(nTriggerNewActivity, nNewActivityPlayers);
	}

	const char * getNpcName( int nNpcId )
	{
		PNPCCONFIG pNpcCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetNpcData(nNpcId);
		if (pNpcCfg != NULL)
		{
			return pNpcCfg->sName;
		}
		return NULL;
	}
	void setNpcTitle(unsigned int hScene, int nNpcId, char* sTitle)
	{
		// CScene* pScene = CFuBenManager::m_SceneMgr->GetDataPtr(hScene);	
		// if (pScene)
		// {
		// 	CNpc * pNpc = pScene->GetNpc(nNpcId);
		// 	if (pNpc)
		// 	{
		// 		pNpc->SetTitle(sTitle);
		// 		char sAllName[128];
		// 		sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",pNpc->GetEntityName(),pNpc->GetTitle());
		// 		pNpc->ChangeShowName(20, sAllName);
		// 	}
		// }
	}
	

	unsigned long long getEntityHandle(void* pEntity)
	{
		if (pEntity == NULL ) return 0;
		long long d;
		memcpy(&d, &(((CEntity*)pEntity)->GetHandle()),sizeof(d));
		return d;
	}
	void setFireValRate( float fVal )
	{
		CBuffSystem::s_FireValRate = fVal;
	}

	int getMonsterLevel( int nMonsterId )
	{
		MONSTERCONFIG * pMonsterCfg = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonsterId);
		if (pMonsterCfg != NULL)
		{
			return pMonsterCfg->nLevel;
		}
		return 0;
	}

	void * getItemNpc()
	{
		return GetGlobalLogicEngine()->GetItemNpc();
	}
	int getMailAttachValue(lua_State* L , int nTableIndex, const char* sKey)
	{
		lua_getfield(L,nTableIndex,sKey);
		int value = 0;
		if (lua_isnumber(L,-1))
		{
			value = (int)lua_tonumber(L, -1);
		}
		lua_pop(L,1);
		return value;
	}

	int sendSysMail(lua_State *L)
	{
		CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
		unsigned int nActorId = (unsigned int)lua_tonumber(L,1);
		const char * sTitle =  lua_tostring(L,2);
		const char * sContent =  lua_tostring(L,3);
		int mailAttachIndex = 0;
		if (lua_istable(L,4))
		{
			if (lua_objlen(L, -1) > MAILATTACHCOUNT)
			{
				return 0;
			}
			lua_pushnil( L );  
			while( 0 != lua_next( L, 4 ) ) 
			{ 
				if (lua_istable(L,-1))
				{
					int nAwardTableIndex = lua_gettop(L);
					mailAttach[mailAttachIndex].nType = getMailAttachValue(L, nAwardTableIndex , "type");
					mailAttach[mailAttachIndex].item.wItemId = getMailAttachValue(L, nAwardTableIndex, "id");
					if(mailAttach[mailAttachIndex].nType != qatEquipment)
					{
						mailAttach[mailAttachIndex].item.wItemId = mailAttach[mailAttachIndex].nType;
					}
					int nCount = getMailAttachValue(L, nAwardTableIndex, "count");
					// if (mailAttach[mailAttachIndex].nType > qatEquipment)
					// {
					// 	mailAttach[mailAttachIndex].nResourceCount = nCount;
					// 	if (mailAttach[mailAttachIndex].nType == qatAddExp)
					// 	{
					// 		mailAttach[mailAttachIndex].item.wCount = nCount;//经验库要保留count
					// 	}
					// }
					// else
					{
						mailAttach[mailAttachIndex].item.wCount = nCount;
					}
					// mailAttach[mailAttachIndex].item.btQuality = getMailAttachValue(L, nAwardTableIndex, "quality");
					// mailAttach[mailAttachIndex].item.wStar = getMailAttachValue(L, nAwardTableIndex, "strong");
					// mailAttach[mailAttachIndex].item.btFlag = getMailAttachValue(L, nAwardTableIndex, "bind");
					// mailAttach[mailAttachIndex].wQualityDataIndex = getMailAttachValue(L, nAwardTableIndex, "qualityDataIndex");
					// mailAttach[mailAttachIndex].item.bInSourceType = getMailAttachValue(L, nAwardTableIndex, "bInSourceType");
					// mailAttach[mailAttachIndex].item.nAreaId = getMailAttachValue(L, nAwardTableIndex, "nAreaId");
					mailAttachIndex++;
				}
				lua_pop( L, 1 );  
				
			} 
		}
		CMailSystem::SendMail(nActorId, sTitle, sContent, mailAttach);
		return 1;	
	}

	unsigned int getBossLastKillTime(int nBossId)
	{
		CMonsterProvider & monsterProvider = GetLogicServer()->GetDataProvider()->GetMonsterConfig();
		return monsterProvider.GetBossLastKillTime( nBossId );
	}

	
	// 判断这个活动id对应的活动是否运行中
	bool isActivityRunning(int nAtvId)
	{
		return GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId);
	}
	//结束活动
	void closeActivityRunning(int nAtvId, bool nNextTime)
	{
		GetGlobalLogicEngine()->GetActivityMgr().CloseActivity(nAtvId,nNextTime);
	}

	//  判断是否到达活动结束前的n秒
	bool isReachSecondBeforeActivityEnd(int activity_id, int second_before_end){
		unsigned int second_now = CMiniDateTime::now();
		CActivityProvider& activitycfg= GetLogicServer()->GetDataProvider()->GetActivityConfigs();
		if(activitycfg.IsGlobalActivity(activity_id)){
			unsigned int second_end = GetGlobalLogicEngine()->GetActivityMgr().GetEndTime(activity_id);
			if(second_now > (second_end - second_before_end)) return true ;
		}
		
		
		return false ;
	}


	//获取活动结束时间
	unsigned int getActivityEndMiniSecond(int activity_id){
		return  GetGlobalLogicEngine()->GetActivityMgr().GetEndTime(activity_id);
	}



	void setSysRateExp( int nRate, int nTime )
	{
		GetGlobalLogicEngine()->SetSysExpRate(nRate, nTime);
	}

	bool getSceneCenterPos( int nSceneId, int & nPosX, int & nPosY )
	{
		CFuBen  * pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
		if (pFb != NULL)
		{
			CScene * pScene = pFb->GetScene(nSceneId);
			if (pScene != NULL)
			{
				INT_PTR nX, nY;
				pScene->GetSceneAreaCenter(nX, nY);
				nPosX = (int)nX;
				nPosY = (int)nY;
				return true;
			}
		}
		return false;
	}

	bool createDropItems(void* pEntity, int nWardType, int nItemId, int nCount, int nBind)
	{
		if ( !pEntity ) 
		{
			return false;
		}
		int nPosX = 0;
		int nPosY = 0;
		CEntity* pEntityEx = (CEntity*)pEntity;
		pEntityEx->GetPosition(nPosX,nPosY);
		int nNowtime = GetGlobalLogicEngine()->getMiniDateTime();
		if (nWardType == qatEquipment)
		{
			CUserItem* pUserItem = GetLogicServer()->GetLogicEngine()->AllocUserItem(true);
			if( !pUserItem )
			{
				return false;
			}
			pUserItem->wItemId  = nItemId;
			pUserItem->wCount = nCount;
			pUserItem->btFlag = (BYTE)nBind;
			CDropItemEntity *pDropItem= CDropItemEntity::CreateDropItem(pEntityEx->GetScene(),nPosX,nPosY,GameLog::clScriptCreateDropItem,pEntityEx->GetEntityName());
			if(pDropItem != NULL)
			{
				pDropItem->SetItem(pUserItem);
				pUserItem->setSource(CStdItem::iqOther,nNowtime);
			}
			else
			{
				GetGlobalLogicEngine()->DestroyUserItem(pUserItem);
			}
		}
		return true;
	}
	
	bool createDropMoney(void* pEntity, int nWardType, int nMoneyType, int nDropCount, int nBind)
	{
		if ( !pEntity ) 
		{
			return false;
		}
		int nPosX = 0;
		int nPosY = 0;
		CEntity* pEntityEx = (CEntity*)pEntity;
		pEntityEx->GetPosition(nPosX,nPosY);
		CDropItemEntity *pDropItem= CDropItemEntity::CreateDropItem(pEntityEx->GetScene(),nPosX,nPosY,GameLog::clScriptCreateDropItem,pEntityEx->GetEntityName());
		if(pDropItem != NULL)
		{
			pDropItem->SetMoneyCount((int)nDropCount, (int)nMoneyType);
		}
		return true;
	}

	bool isOpenWuYiGoldEgg()
	{
		return false;//GetGlobalLogicEngine()->GetMiscMgr().IsOpenWuYiGoldEgg();
	}

	void setQuickBuffId( int nBuffId )
	{
		CActor::s_nQuickBuffId = nBuffId;
	}
	void sendWorldLevel(void* pEntity)
	{
		if ( !pEntity ) 
		{
			return;
		}
		GetGlobalLogicEngine()->GetWorldLevelMgr().SendWorldLevel((CEntity*)pEntity);
	}

	int getWorldLevel()
	{
		return GetGlobalLogicEngine()->GetWorldLevelMgr().GetWorldLevel();
	}
	int NewRedPacketId()
	{
		static BYTE nRedPacketSeries = 1;

		RedPacketId rId;
		
		SYSTEMTIME sysTime = GetGlobalLogicEngine()->getSysTime();
		rId.t.nHour = (BYTE)sysTime.wHour;
		rId.t.nMin = (BYTE)sysTime.wMinute;
		rId.t.nSec = (BYTE)sysTime.wSecond;
		rId.t.nSeries = (BYTE)nRedPacketSeries++;
		return rId.nId;
	}
	//转换为当前时间格式串
	const char* miniDateTime2String(unsigned int tv)
	{
		CMiniDateTime t(tv);
		SYSTEMTIME sysTime;
		t.decode(sysTime);
		static char buf[20]={0};
		sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",sysTime.wYear,sysTime.wMonth,sysTime.wDay,sysTime.wHour,sysTime.wMinute,sysTime.wSecond);
		return buf;
	}
	void SendConsumeLog()
	{
		GetGlobalLogicEngine()->SendActorConsumeLog();
	}

	void RefeshWorldLevel()
	{
		GetGlobalLogicEngine()->GetWorldLevelMgr().ReloadWorldLevel();
	}

	void reloadItem()
	{
		GetGlobalLogicEngine()->reloadItemConfig();
	}

	void reloadLang()
	{
		GetLogicServer()->GetTextProvider().LoadFromFile(_T("data/language/LangCode.txt"));
	}
	void reloadMonster()
	{
		GetGlobalLogicEngine()->GetBackResLoadModule().LoadMonsterConfig();
	}
	void reloadVSPDef()
	{
		GetLogicServer()->GetVSPDefine().LoadDefinitions(_T("data/VSPDef.txt"));
	}
	void reloadFw()
	{
		int nHashFun =4, nMemoryBytes= 2500000;
		GetGlobalLogicEngine()->GetChatMgr().ReloadFilterDb(nHashFun,nMemoryBytes);
	}
	void reloadConfig(int nId)
	{
		GetLogicServer()->GetDataProvider()->ReloadConfig(nId);
	}
	int getSpid()
	{
		return (int)GetLogicServer()->GetSpid();
	}

	const char* getStrSpid()
	{
		return GetLogicServer()->GetVSPDefine().GetDefinition(CVSPDefinition::SPID);
	}

	
	int getRunningActivityId(lua_State* L)
	{
  		int nAtvType = ((int)  lua_tonumber(L,1));

		bool isGlobal = false;
		if (GetLogicServer()->GetDataProvider()->GetActivityConfigs().IsGlobalAtvType(nAtvType))
		{
			isGlobal = true;
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
			if(!GetGlobalLogicEngine()->GetActivityMgr().IsRunning(AtvIdList[i]))
				continue;
			//tolua_pushfieldvalue(L, -3, idx++, AtvIdList[i]);
			lua_pushinteger(L, (int)++idx);
			lua_pushinteger(L, (int)AtvIdList[i]);
			lua_settable(L, -3);
		}
		
		return 1;
	}


	int getRunningActivityStartTime(int nAtvId)
	{
  		return GetGlobalLogicEngine()->GetActivityMgr().GetStartTime(nAtvId);
	}
	int getRunningActivityStartTimeRelToday(int nAtvId)
	{
		CMiniDateTime nStartTime  = GetGlobalLogicEngine()->GetActivityMgr().GetStartTime(nAtvId); //
		return  nStartTime.rel_today();
	}

	// int GetRankTopAcotrInfo()
	// {
	// 	std::vector<int>& AtvIdList = GetLogicServer()->GetDataProvider()->GetActivityConfigs().GetActivityIdList(nAtvType);

	// 	lua_newtable(L);
	// 	int idx = 0;
	// 	for (size_t i = 0; i < AtvIdList.size(); i++)
	// 	{
	// 		if (isGlobal)
	// 		{
	// 			if(!GetGlobalLogicEngine()->GetActivityMgr().IsRunning(AtvIdList[i]))
	// 				continue;
	// 		}
	// 		else
	// 		{
	// 			if(!((CActor*)pEntity)->GetActivitySystem().IsActivityRunning(AtvIdList[i]))
	// 				continue;
	// 		}
	// 		//tolua_pushfieldvalue(L, -3, idx++, AtvIdList[i]);
	// 		lua_pushinteger(L, (int)++idx);
	// 		lua_pushinteger(L, (int)AtvIdList[i]);
	// 		lua_settable(L, -3);
	// 	}
	// 	return 1;
	// }

	void AsyncWorkLuaCallback(void* ptr)
	{
		AsyncWorker::CBData* cb = (AsyncWorker::CBData*)ptr;
		AsyncWorker::LuaParams* params = (AsyncWorker::LuaParams*)cb->params_;

		CScriptValueList* arg = NULL;

		if (params)
		{
			arg = &params->result_;
		}

		static CLogicScript& lua = GetGlobalLogicEngine()->GetGlobalNpc()->GetScript();
		lua.CallModule("AsyncWorkDispatcher","OnWorkFinish", *arg, *arg, 0);
	}

	int addAsyncWorker(lua_State* L)
	{
		if (!lua_istable(L, 1)) return 0;

		CScriptValueList args;
		int cnt = (int)lua_objlen(L, 1);
		cnt = __min(cnt, CScriptValueList::MaxValueCount);

		if (cnt > 0)
		{
			lua_pushnil(L);
			int qcnt = 0;

			while (lua_next(L, 1) && qcnt < cnt)
			{
				int type = lua_type(L, -1);

				// 只支持数字和字符串
				if (type == LUA_TNUMBER)
				{
					args << (int)(int)lua_tointeger(L, -1);
				}
				else if (type == LUA_TSTRING)
				{
					args << (const char*)lua_tostring(L, -1);
				}

				lua_pop(L, 1);

				qcnt++;
			}
		}

		unsigned int guid = GetLogicServer()->GetAsyncWorker()->SendLuaMessage(args, &AsyncWorkLuaCallback);
		lua_pushnumber(L, guid);
		return 1;
	}

	void SendChatMsg(char* msg, int nChatchannelId, void* pEntity, int nitemId)
	{
		if(!msg ) return;
		char buff[1024];
		CDataPacket outPack(buff, sizeof(buff)); //下发的数据包

		outPack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)nChatchannelId;
		std::string actorName = "";
		int nLevel = 0;
		Uint64 handle = 0;
		CActor * pActor = NULL;
		unsigned int nVip  = 0;
		if(pEntity)
		{
			pActor = (CActor*)pEntity;
			actorName = pActor->GetEntityName();
			nLevel = pActor->GetLevel();
			handle = Uint64(pActor->GetHandle());
			nVip = (unsigned int)(pActor->GetProperty<unsigned int>(PROP_ACTOR_SUPPER_PLAY_LVL));
		}
		char* message = (char*)msg;
		outPack.writeString(actorName.c_str());
		outPack.writeString(message);
		outPack << (int)nLevel;
		outPack <<(Uint64)handle;
		outPack<<(unsigned int)nVip;
		outPack << (BYTE)(nitemId != 0? 1:0);
		
		switch(nChatchannelId)
		{
			case ciChannelSecret: //私聊
			{
				if(pActor) {
					CActorPacket pack;
					CDataPacket & data = pActor->AllocPacket(pack);
					data.writeBuf(outPack.getMemoryPtr(),outPack.getPosition());
					pack.flush();
				}
			}
			break;
			case ciChannelNear: //附近
				{
					if(pActor) {
						pActor->GetObserverSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(),true); //附近广播
					}
				}
				break;
		
			case ciChannelGuild: //工会
				{
					if(pActor) {
						pActor->GetGuildSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
					}	
				}
				break;
			case ciChannelTeam: //队伍聊天
				{
					if(pActor) {
						pActor->GetTeamSystem().BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
					}	
				}
				break;
			case ciChannelWorld: //世界频道
				GetGlobalLogicEngine()->GetChatMgr().AddChatRecord(outPack.getMemoryPtr(), outPack.getPosition());
				GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition());
				break;
		}
	}


	int sendSysDropMail(lua_State* L)
	{
		CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
		memset(mailAttach, 0, sizeof(CMailSystem::MAILATTACH)*MAILATTACHCOUNT);
		unsigned int nActorId = (unsigned int)lua_tonumber(L,1);
		const char * sTitle =  lua_tostring(L,2);
		const char * sContent =  lua_tostring(L,3);
		int nDropId = (int)lua_tonumber(L,4);
		if (nDropId <= 0)
			return 0;
		
		int mailAttachIndex = 0;
		std::vector< CUserItem > dropItems;
		GetLogicServer()->GetDataProvider()->GetDropCfg().getDropItemOPParam(nDropId, dropItems);
		if(dropItems.size() == 0)
			return 0;
		
		for(int i = 0; i < dropItems.size(); i++)
		{
			if (mailAttachIndex >= MAILATTACHCOUNT)
			{
				CMailSystem::SendMail(nActorId, sTitle, sContent, mailAttach);
				mailAttachIndex = 0;
				memset(mailAttach, 0, sizeof(CMailSystem::MAILATTACH)*MAILATTACHCOUNT);
			}
			CUserItem & item = dropItems[i];
			mailAttach[mailAttachIndex].nType = item.nType;
			mailAttach[mailAttachIndex].item.wItemId = item.wItemId;

			if(item.nType != qatEquipment)
				mailAttach[mailAttachIndex].item.wItemId = item.nType;

			mailAttach[mailAttachIndex].item.wCount = item.wCount;
			memcpy(mailAttach[mailAttachIndex].item.cBestAttr, item.cBestAttr, sizeof(mailAttach[mailAttachIndex].item.cBestAttr));
			mailAttachIndex++;
		}					

		// CMailSystem::SendMail(nActorId, sTitle, sContent, mailAttach);
		return 1;	
	}


	void sendAllActorOneActivityData(int nAtvId)
	{
		CActor *pActor;
		CVector<void*> actorList;
		GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
		for (INT_PTR i = 0; i < actorList.count(); i++)
		{
			CActor *pActor = (CActor *)actorList[i];

			if (pActor->OnGetIsTestSimulator()
				|| pActor->OnGetIsSimulator() )
			{
				continue;
			}
			if (GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
			{
				GetGlobalLogicEngine()->GetActivityMgr().SendOneActivity(pActor, nAtvId);
			}
			else if (pActor && pActor->GetActivitySystem().IsActivityRunning(nAtvId))
			{
				pActor->GetActivitySystem().SendOneActivity(nAtvId);
			}
		}
	}

	void  CheckGPActivityStartEnd(void* pEntity, int atvId, unsigned int ScriptEndTime)
	{
		if (!pEntity) return  ;
		if(( (CEntity *)pEntity)->GetType() != enActor) return ;
		GetGlobalLogicEngine()->GetActivityMgr().CheckGPStartEnd((CActor*)pEntity, atvId, ScriptEndTime);
	}


	int getPActivityOpenDay(int nAtvId)
	{
		return GetLogicServer()->GetDataProvider()->GetActivityConfigs().getPActivityOpenDay(nAtvId);
	}

	const char* MD5(lua_State *L)
	{
		unsigned char signCheck[2048];
		unsigned char tmp[512];
		int n = lua_gettop(L);
		int i = 0;
		lua_getglobal(L, "tostring");
		signCheck[0]=0;
		for (i = 1; i <= n; i++) {
			const char *s;
			lua_pushvalue(L, -1);
			lua_pushvalue(L, i);
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);
			if (s == NULL)
			{
				luaL_error(L, " MD5 call error ");
				return NULL;
			}
			sprintf_s((char*)tmp, sizeof(tmp), _T("%s%s"), (char*)signCheck, s);
			sprintf_s((char*)signCheck, sizeof(signCheck), _T("%s"), (char*)tmp);
			lua_pop(L, 1);
		}

		static char md5string[64];
		MD5_CTX   ctx;
		MD5Init(&ctx); 
		MD5Update(&ctx,(unsigned char*)signCheck, strlen((char*)signCheck) );
		MD5Final((unsigned char*)md5string,&ctx);
		return md5string;
	}


	int sendCrossServerMail(lua_State *L)
	{
		CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
		unsigned int nActorId = (unsigned int)lua_tonumber(L,1);
		const char * sTitle =  lua_tostring(L,2);
		const char * sContent =  lua_tostring(L,3);
		int mailAttachIndex = 0;
		std::vector<ACTORAWARD> awardList;
		if (lua_istable(L,4))
		{
			if (lua_objlen(L, -1) > MAILATTACHCOUNT)
			{
				return 0;
			}
			lua_pushnil( L );  
			while( 0 != lua_next( L, 4 ) ) 
			{ 
				if (lua_istable(L,-1))
				{
					int nAwardTableIndex = lua_gettop(L);
					ACTORAWARD award;
					award.btType = getMailAttachValue(L, nAwardTableIndex , "type");
					award.wId = getMailAttachValue(L, nAwardTableIndex, "id");
					award.wCount = getMailAttachValue(L, nAwardTableIndex, "count");
					awardList.emplace_back(award);
				}
				lua_pop( L, 1 );  
				
			} 
		}
		CMailSystem::SendCrossServerMail(nActorId, sTitle, sContent, awardList);
		return 1;	
	}
	void KickAllCrossServerActor()
	{
		GetGlobalLogicEngine()->GetCrossMgr().KickAllCrossServerActor();
	}

};
