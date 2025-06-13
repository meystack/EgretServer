#include "StdAfx.h"
#include "Npc.h"
#include "../LogicEngine.h"
#include <iostream>  
#include <fstream>  
using namespace std;
using namespace wylib::stream;
// 此宏开关用于控制是否使用缓存的Lua二进制字节码加载以提高NPC创建速度
// 关闭此宏就直接加载脚本文件
#define USE_LUA_BIN_CODE

CNpc::CNpc()
	:Inherited()
{
	m_pConfig = NULL;
	m_bScriptInited = false;
	m_Script.SetEntity(this);
	m_idleTime = 0;
}

void CNpc::InitNPCAi()
{
	//if (m_pConfig && m_pConfig->nAITypeId > 0)
	//{
	//	m_pAI = GetGlobalLogicEngine()->GetAiMgr().CreateAi(m_pConfig->nAITypeId, (CAnimal*)this);		
	//}
}

void CNpc::LogicRun(TICKCOUNT nCurrentTime)
{
	if(IsInited() == false) return ;
	//DECLARE_FUN_TIME_PROF()
	Inherited::LogicRun(nCurrentTime); // (NPC不执行怪物的这些逻辑检测)	
	
	//检查NPC调用idle函数	
	if (m_RunIdleTimer.Check(nCurrentTime))
	{
		DECLARE_TIME_PROF("NPC::Call_Idle")
		if (m_idleTime == 0)
		{
			//设置下次调用idle函数的时间为90秒到180之内。
			int nInterval = 120000;
			if (m_pConfig)
			{
				nInterval = m_pConfig->nIdleInterval;
				nInterval = __max(nInterval, 15000); // 最小不能低于15s，避免执行太频繁			
			}			
			m_RunIdleTimer.SetNextHitTimeFromNow(nInterval + (nCurrentTime%10) * 1000);
			//m_RunIdleTimer.SetNextHitTimeFromNow(90000 + (nCurrentTime%90) * 1000);
		}
		else
		{
			m_RunIdleTimer.SetNextHitTimeFromNow(m_idleTime);
		}
		//if (m_pConfig)
		//{
		//	Say(mssNear, m_pConfig->sSay);
		//}
		
		/*static LPCSTR sIdleFnName = "idleTalk";
		if (m_Script.FunctionExists(sIdleFnName))
		{
			CScriptValueList vl;
			m_Script.Call(sIdleFnName, vl, vl, 0);
		}*/
		
		//检查脚本内存回收时间
		if (m_ScriptGCTimer.Check(nCurrentTime))
		{
			//设置下次进行脚本内存回收的时间为180秒到270秒之内。
			//在一个随机的时间范围内进行垃圾回收可以避免所有NPC在同一个主循环中进行脚本垃圾回收而导致的性能集中损失，
			//对脚本进行垃圾回收是十分低效的。
			DECLARE_TIME_PROF("NPC::scriptGC")
			m_ScriptGCTimer.SetNextHitTimeFromNow(180000 + (nCurrentTime%90) * 1000);
			m_Script.gc();
		}
	}
}

void CNpc::ProcessEntityMsg(const CEntityMsg &msg)
{
	CEntity *pSender;

	if(msg.bIsUsed ==false)
	{
		OutputMsg(rmError,"name=%s,msgid=%d,ProcessEntityMsg is invalid",GetEntityName(),(int)msg.nMsg);
		return ;
	}

	switch(msg.nMsg)
	{
	case CEntityMsg::emGetQuestState:
		if (msg.nSender.GetType() == enActor)
		{
			pSender = GetEntityFromHandle(msg.nSender);
			// if (pSender && pSender->IsInited()) ((CActor*)pSender)->GetQuestSystem()->SendNpcQuestState(this);
		}
		break;
	default:
		Inherited::ProcessEntityMsg(msg);
		break;
	}
}

void CNpc::ResetScript()
{	
#ifdef USE_LUA_BIN_CODE
	// 采取先编译的方式				
	//m_Script.resetBinScript(m_pConfig->packet);
#else
	//m_Script.setScript(m_pConfig->pNpcScriptText, &m_pConfig->LRList);
#endif

	// 清空所有的实体消息和实体回调
	ClearAllEntityMsg();
	ClearScriptCallback();
}

bool CNpc::LoadScript(LPCSTR sPath,bool boReload)
{
	DECLARE_TIME_PROF("CNpc::LoadScript");
	LPCSTR sScript = NULL;	
	if(sPath ==NULL)
	{
		OutputMsg(rmError, _T("unable to load Script file npc=%s"), GetEntityName());
		return false;
	}
	if (strcmp(sPath, "") == 0)
	{
		return true;
	}
	//从文件加载脚本
	CMemoryStream ms;
	if (ms.loadFromFile(sPath) <= 0)
	{
		OutputMsg(rmError, _T("unable to load Script file %s"), sPath);
		return false;
	}

	//对脚本进行预处理
	CCustomLuaPreProcessor pp;
	GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
	//增加语言包已被加载的标记，语言包已经作为独立的配置读取了，这里注册宏后可以加快脚本读取
	if (!pp.getMacro("_LANGUAGE_INCLUDED_"))
		pp.addMacro("_LANGUAGE_INCLUDED_");
	try
	{
		sScript = pp.parse((LPCSTR)ms.getMemory(),sPath);
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("%s config parse error: %s\r\n"),sPath, s.rawStr());
		return false;
	}

	if ( !sScript )
	{
		OutputMsg(rmError, _T("unable to pre-process Script file,npc %s"), GetEntityName() );
		return false;
	}
	wylib::misc::CRefObject<CCustomLuaPreProcessor::CLineRangeList> LRList;
	INT_PTR nLRCount = pp.getLineRangeData(NULL, 0);
	if (nLRCount > 0)
	{
		LRList = new wylib::misc::CRefObjectImpl<CCustomLuaPreProcessor::CLineRangeList>();
		LRList.raw_ptr()->reserve(nLRCount);
		pp.getLineRangeData(*LRList.raw_ptr(), nLRCount);
		LRList.raw_ptr()->trunc(nLRCount);
	}
	if(!m_Script.setScript(sScript, &LRList))
	{
		FILE* fp = fopen("scripterror.txt", "wb");
		if (fp)
		{
			fputs(sScript, fp);
			fclose(fp);
		}
		return false;
	}
	return true;
}

bool CNpc::Call( LPCSTR sFunName)
{
	if (sFunName == NULL || sFunName[0] == 0) return false;
	CScriptValueList va;
	
	CHAR sFnArgs[1024];
	sFnArgs[0]=0;

	FuncParamProcess(sFunName,sFnArgs,sizeof(sFnArgs),va);
	//调用脚本函数，期待返回1个值
	return ( m_Script.Call(sFnArgs, va, va, 1) );
}

bool CNpc::CallModule(LPCSTR sModuleName,LPCSTR sFunName)
{
	if (sFunName == NULL || sFunName[0] == 0) return false;
	CScriptValueList va;
	
	CHAR sFnArgs[1024];
	sFnArgs[0]=0;

	FuncParamProcess(sFunName,sFnArgs,sizeof(sFnArgs),va);
	//调用脚本函数，期待返回1个值
	return ( m_Script.CallModule(sModuleName,sFnArgs, va, va, 1) );
}

CNpc * CNpc::GetNpcPtr(char * sSceneName,char * sNpcName)
{
	if(sSceneName ==NULL || sNpcName ==NULL) return NULL;
	CNpc *pNpc =NULL;
	//刷新系统NPC
	if (!_stricmp(sSceneName, "SYS"))
	{

		//刷新全局功能NPC
		if (!_stricmp(sNpcName, "FUNCTION"))
		{
			pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		}
		else if (!_stricmp(sNpcName, "MONSTER"))
		{
			pNpc = GetGlobalLogicEngine()->GetMonFuncNpc();
		}
	}
	else
	{
		//场景是否存在

		CFuBen * pFb =NULL;
		CScene * pScene =NULL;
		if(GetGlobalLogicEngine()->GetFuBenMgr()->GetFbPtrBySceneName(sSceneName,pFb,pScene) ==false || pScene ==NULL) 
		{
			OutputMsg(rmError,_T("func [%s] scene=%s not exist"),__FUNCTION__,sSceneName);
			return NULL;
		}
		pNpc = pScene->GetNpc(sNpcName);
	}
	if(pNpc ==NULL)
	{
		OutputMsg(rmError,_T("func [%s] scene=%s NPC=%s not exist"),__FUNCTION__,sSceneName,sNpcName);
		
	}
	return pNpc;
}

void CNpc::FuncParamProcess(LPCSTR sFnArgs, OUT CHAR * pFunc, INT_PTR nOutBuffSize,CScriptValueList & va, bool boAddFuncParam )
{
	//_asncpytA(pFunc, sPtr);
	char * sArgPos, *sPtr ;

	strncpy(pFunc,sFnArgs,nOutBuffSize);
	sArgPos= (char *)strchr(pFunc, ',');
	sPtr = pFunc;
	
	if (NULL == sArgPos)
	{// 没参数
		if (boAddFuncParam)
		{
			va << (LPCTSTR)pFunc;
		}
		return;
	}

	sPtr = sArgPos + 1;
	*sArgPos = 0;//讲第一个','位置的字符改写为0，使得sFnArgs再第一个','处终止，sFnArgs即是函数名称
	if (boAddFuncParam)
	{
		va << (LPCTSTR)pFunc;
	}
	//解出所有以','分隔的参数
	while (va.count() < CScriptValueList::MaxValueCount)
	{
		sArgPos = strchr(sPtr, ',');
		if (!sArgPos)
		{
			if (*sPtr)
				va << sPtr;
			break;
		}
		*sArgPos = 0;//
		va << sPtr;
		//将处理指针调整到次','的后面继续搜索
		sPtr = sArgPos + 1;
	}
}

// void CNpc::Talk(CActor *pActor, LPCSTR sFnName)
// {
// 	if (pActor == NULL) return;
// 	if (sFnName == NULL || sFnName[0] == 0)
// 		return;
// 	//OutputMsg(rmTip,_T("NPC Talk:%s"),sFnName);
// 	CScriptValueList va;
// 	CHAR sFnArgs[1024];
// 	sFnArgs[0]=0;


// 	//向参数表中写入玩家对象
// 	va << pActor;

// 	FuncParamProcess(sFnName,sFnArgs,sizeof(sFnArgs),va);

// 	//尝试将函数名称与参数进行分割，"function,arg1,arg2"
// 	/*
// 	_asncpytA(sFnArgs, sFnName);
// 	sPtr = sFnArgs;
// 	sArgPos = strchr(sPtr, ',');
// 	if (sArgPos)
// 	{
// 		sPtr = sArgPos + 1;
// 		*sArgPos = 0;//讲第一个','位置的字符改写为0，使得sFnArgs再第一个','处终止，sFnArgs即是函数名称
// 		//解出所有以','分隔的参数
// 		while (va.count() < CScriptValueList::MaxValueCount)
// 		{
// 			sArgPos = strchr(sPtr, ',');
// 			if (!sArgPos)
// 			{
// 				if (*sPtr)
// 					va << sPtr;
// 				break;
// 			}
// 			*sArgPos = 0;//再相爱
// 			va << sPtr;
// 			//将处理指针调整到次','的后面继续搜索
// 			sPtr = sArgPos + 1;
// 		}
// 	}
// 	*/

// 	Talk(pActor,sFnArgs,va);
// }

void CNpc::OnNpcCaller(LPCSTR sFnName, int nNpcId)
{
	CScriptValueList paramList, retParamList;
	CHAR sFnArgs[1024];
	sFnArgs[0]=0;
	if (nNpcId < 0)
	{
		nNpcId = GetId();
	}
	paramList << nNpcId;
	FuncParamProcess(sFnName, sFnArgs, sizeof(sFnArgs), paramList, true);
	if(!GetScript().Call("OnNpcTimer", paramList, retParamList,0))
	{
		OutputMsg(rmError,_T("npc[%d,%d] func [%s] function =%s,call error"),GetId(), nNpcId,__FUNCTION__, sFnName);
	}
}

// void CNpc::CallScript(CActor * pActor, LPCSTR sFnName)
// {
// 	CScriptValueList paramList, retList;
// 	CHAR sFnArgs[1024];
// 	sFnArgs[0]=0;
// 	paramList << pActor << GetId();
// 	FuncParamProcess(sFnName, sFnArgs, sizeof(sFnArgs), paramList, true);
// 	if(!GetGlobalLogicEngine()->GetScriptNpc()->GetScript().Call("OnNpcEvent", paramList, retList, 0))
// 	{
// 		const RefString &s = GetGlobalLogicEngine()->GetScriptNpc()->GetScript().getLastErrorDesc();
// 		if (paramList.count() >= 3)
// 		{
// 			CScriptValue& sValue = paramList[2];
// 			if (sValue.getType() == CScriptValue::vString)
// 			{
// 				pActor->SendOldTipmsgFormatWithId(tpOnNpcEventMsg,ttDialog,(LPCTSTR)s,(LPCTSTR)sValue);
// 			}
// 		}
// 		else
// 		{
// 			pActor->SendTipmsg((LPCTSTR)s,ttDialog);
// 		}
// 	}
// }

// void CNpc::Talk(CActor * pActor,LPCSTR sFnName,CScriptValueList& va)
// {
// 	if (pActor == NULL) return;
// 	INT_PTR nCampId = GetCampId(); //自身的阵营ID
	
// 	if(GetAttriFlag().DenySee)
// 	{
// 		CloseDialog(pActor);
// 		return ;
// 	}

// 	//不同的阵营的NPC别的阵营不能使用
// 	if(nCampId && nCampId != pActor->GetCampId())
// 	{
// 		/*
// 		CActorPacket ap;
// 		CDataPacket &pack = pActor->AllocPacket(ap);
// 		pack << (BYTE)enDefaultEntitySystemID;
// 		pack << (BYTE)sNpcTalk;
// 		pack << (BYTE)1;
// 		pack << m_hEntityHandler;
// 		LPCTSTR pStr= (LPCSTR)GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpCampNpcFail);
// 		if(pStr)
// 		{
// 			pack << pStr;
// 		}
// 		else
// 		{
// 			pack << "";
// 		}
// 		ap.flush();
// 		*/
// 		return;
// 	}
// 	CScriptValueList vr;
// 	//调用脚本函数，期待返回1个值
// 	if ( m_Script.Call(sFnName, va, vr, 1) )
// 	{
// 		//如果有返回值
// 		if ( vr.count() > 0 )
// 		{
// 			//取出第一个返回值并判断是否是字符串，如果是字符串则向客户端发送此数据
// 			CScriptValue &val = vr[0];
// 			if ( val.getType() == CScriptValue::vString )
// 			{
// 				//向客户端发送NPC对话内容的数据包
// 				CActorPacket ap;
// 				CDataPacket &pack = pActor->AllocPacket(ap);
// 				pack << (BYTE)enDefaultEntitySystemID;
// 				pack << (BYTE)sNpcTalk;
// 				pack << (BYTE)1;
// 				pack << m_hEntityHandler;
// 				pack << (LPCSTR)val;
// 				ap.flush();

// 				//OutputMsg(rmTip,_T("Npc Talk result:%s"),(LPCSTR)val);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		//脚本错误，将以模态对话框的形式呈现给客户端
// 		const RefString &s = m_Script.getLastErrorDesc();
// 		pActor->SendTipmsg((LPCSTR)s,ttDialog);
// 		OutputMsg(rmError, _T("NPCTalk(%s), id=%d Failed..."), GetEntityName(),GetId());
// 	}
// }

VOID CNpc::CloseDialog(CActor *pActor)
{
	CActorPacket ap;
	CDataPacket &pack = pActor->AllocPacket(ap);
	pack << (BYTE)enDefaultEntitySystemID;
	pack << (BYTE)sNpcTalk;
	pack << (BYTE)0;
	pack << m_hEntityHandler;
	ap.flush();
}

void CNpc::NPCTalk()
{
	CScriptValueList paramList, retList;
	paramList << this;
	if (!m_Script.Call("NPCTalk", paramList, retList, 0))
	{
		OutputMsg(rmError, _T("NPCTalk(%s) Failed..."), GetEntityName());
	}
}
void CNpc::ReloadAllNpc()
{
	// LPCTSTR scriptFile[] = {CLogicEngine::szGlobalFuncScriptFile,CLogicEngine::szMonsterFuncScriptFile,CLogicEngine::szQuestNpcFile,CLogicEngine::szItemNpcFile};
	// CNpc* pNpc[] = {GetGlobalLogicEngine()->GetGlobalNpc(),GetGlobalLogicEngine()->GetMonFuncNpc(),GetGlobalLogicEngine()->GetScriptNpc(),GetGlobalLogicEngine()->GetItemNpc()};
	// for (int i=0; i <ArrayCount(scriptFile); i++)
	// {
	// 	pNpc[i]->GetScript().LoadScript(scriptFile[i]);
	// }
}
void CNpc::CheckAllScript(char* sParam)
{
	LPCTSTR scriptFile[] = {CLogicEngine::szGlobalFuncScriptFile,CLogicEngine::szMonsterFuncScriptFile,CLogicEngine::szQuestNpcFile,CLogicEngine::szItemNpcFile};
	LPCTSTR scriptName[] = {"function","monster","script","item"};
	char* pErrStr = NULL;
	for (int i = 0; i <ArrayCount(scriptName); i++)
	{
		if (strncmp(sParam,scriptName[i],strlen(scriptName[i]))==0)
		{
			sParam+=strlen(scriptName[i]); *sParam=0; pErrStr = ++sParam;
			m_Script.CheckScript(scriptFile[i], pErrStr);
			return;
		}
	}
	
	int nOkCount = 0;
	for (int i=0; i <ArrayCount(scriptFile); i++)
	{
		if (m_Script.CheckScript(scriptFile[i],pErrStr))
		{
			nOkCount ++;
		}
	}
	if(nOkCount==ArrayCount(scriptFile))
	{
		OutputMsg(rmTip,"脚本正常");
	}
	else
	{
		OutputMsg(rmTip,"注:本命令只作检查，修复错误脚本文件之后需要用rsf命令重新加载或者重启！");
	}
}
