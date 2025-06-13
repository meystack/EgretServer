#include "StdAfx.h"
#include "ScriptTimeCall.h"
#include "../../base/Container.hpp"
//定义BootCall文件头标志
const FileHeaders::FILEIDENT CScriptTimeCallManager::ScriptBootCallFileHeader::FileIdent = 
	{ MAKEFOURCC('S', 'B', 'C', 0) };
//定义BootCall文件版本号
const FileHeaders::FILEVERSION CScriptTimeCallManager::ScriptBootCallFileHeader::FileVersion = 
	{ MAKEFOURCC(1, 11, 3, 8) };


using namespace wylib::stream;
using namespace FDOP;

CScriptTimeCallManager::CScriptTimeCallManager()
	:Inherited()
{
	m_nMarkedRemoveCount = 0;
}

CScriptTimeCallManager::~CScriptTimeCallManager()
{
	RemoveAll(FALSE);
	ClearBootCallList();
}

VOID STDCALL CScriptTimeCallManager::ScriptTimeCallDispatcher(CScriptTimeCallManager *lpManager, CList<ScriptCallInfo>::NodeType *pCallInfoNode, INT_PTR nCount)
{
	ScriptCallInfo &sc = *pCallInfoNode;
	sc.pNPC->GetScript().Call(sc.sFn, sc.args, lpManager->m_SRetList, 0);
}

static ULONGLONG GetNextRunTime(LPCSTR sNextRunDateTime, UINT dwSecInterval)
{
	INT y, m, d, h, n, s;
	y = m = d = h = n = s = 0;

	sscanf(sNextRunDateTime, "%d-%d-%d %d:%d:%d", &y, &m, &d, &h, &n, &s);
	CMiniDateTime tRun, tNow = CMiniDateTime::now();

	tRun.encode(y, m, d, h, n, s);
	while (tNow.tv > tRun.tv)
	{
		tRun += dwSecInterval;
	}
	return tRun;
}

HANDLE CScriptTimeCallManager::RegisterTimeCall(CNpc *pNPC, LPCSTR sFnName, LPCSTR sNextCall, 
	UINT dwSecInterval, bool boCallOnNextBoot,const CScriptValueList &args)
{
	//如果函数已经被注册，则返回空 
	if (GetTimeCall(pNPC, sFnName) || !dwSecInterval)
		return NULL;

	//添加注册信息到自身记录列表
	ScriptCallInfo sc1;
	CList<ScriptCallInfo>::NodeType *pNode = m_CallList.linkAtLast(sc1);

	//为定时调用记录对象赋值
	ScriptCallInfo &sc = *pNode;
	ZeroMemory(&sc, sizeof(sc));
	sc.pNPC = pNPC;
	_asncpytA(sc.sFn, sFnName);
	_asncpytA(sc.sNPCName, pNPC->GetEntityName());
	_asncpytA(sc.sSceneName, pNPC->GetScene()->GetSceneName());
	sc.boBootCall = boCallOnNextBoot;
	sc.boMarkedRemove = FALSE;
	sc.args = args;
	ULONGLONG lNextCall = GetNextRunTime(sNextCall, dwSecInterval);
	
	//向定时调用父类中注册调用
	sc.hCall = AddTimeCall(this, (CTimeCaller::TimedCallBack)ScriptTimeCallDispatcher, pNode, dwSecInterval, lNextCall);
	if (!sc.hCall)
	{
		sc.~ScriptCallInfo();
		m_CallList.remove(pNode);
		pNode = NULL;
	}
	return pNode;
}

bool CScriptTimeCallManager::UnregisterTimeCall(HANDLE hTimeCall)
{
	CList<ScriptCallInfo>::Iterator it(m_CallList);
	CList<ScriptCallInfo>::NodeType *pNode;

	for (pNode = it.first(); pNode; pNode = it.next())
	{
		if (pNode == hTimeCall)
		{
			ScriptCallInfo &sc = *pNode;
			//如果需要在下次启动的时候检查调用则存储此数据
			if (sc.boBootCall)
				AddToBootCall(sc);
			//移除定时调用对象
			RemoveTimeCall(sc.hCall);
			//标记为移除
			sc.boMarkedRemove = TRUE;
			//增加移除标记数
			m_nMarkedRemoveCount++;
			return true;
		}
	}
	return false;
}

HANDLE CScriptTimeCallManager::GetTimeCall(CNpc *pNPC, LPCSTR sFnName)
{
	CList<ScriptCallInfo>::Iterator it(m_CallList);
	CList<ScriptCallInfo>::NodeType *pNode;

	for (pNode = it.first(); pNode; pNode = it.next())
	{
		ScriptCallInfo &sc = *pNode;
		if (!sc.boMarkedRemove)
		{
			if (sc.pNPC == pNPC || !pNPC)
			{
				if (strcmp(sc.sFn, sFnName) == 0)
				{
					return pNode;
				}
			}
		}	
	}
	return NULL;
}

INT_PTR CScriptTimeCallManager::Run(ULONGLONG lRunTickLimit /* = 0 */)
{
	SF_TIME_CHECK();
	DECLARE_TIME_PROF("CScriptTimeCallManager::Run");
	INT_PTR nRet = Inherited::Run(lRunTickLimit);

	if (m_nMarkedRemoveCount > 0)
	{
		RemoveAll(TRUE);
	}
	return nRet;
}

VOID CScriptTimeCallManager::RemoveAll(BOOL boJustRemoveMarked)
{
	CList<ScriptCallInfo>::Iterator it(m_CallList);
	CList<ScriptCallInfo>::NodeType *pNode;

	for (pNode = it.first(); pNode; pNode = it.next())
	{
		ScriptCallInfo &sc = *pNode;
		if (boJustRemoveMarked && sc.boMarkedRemove)
		{
			sc.~ScriptCallInfo();
			it.remove(pNode);
		}
	}
	m_nMarkedRemoveCount = 0;
}

VOID CScriptTimeCallManager::AddToBootCall(const ScriptCallInfo& callInfo)
{
	ScriptBootCallData bc1;
	CList<ScriptBootCallData>::NodeType *pNode = m_BootCallList.linkAtLast(bc1);

	ScriptBootCallData &bc = *pNode;	//拷贝场景名称、NPC名称、函数名称信息
	ZeroMemory(&bc, sizeof(bc));
	_asncpytA(bc.sSceneName, callInfo.sSceneName);
	_asncpytA(bc.sNPCName, callInfo.sNPCName);
	_asncpytA(bc.sFn, callInfo.sFn);

	//拷贝参数表
	bc.args = callInfo.args;

	//计算下次调用的CMiniDateTime类型的时间以及调用周期
	ULONGLONG lInterval, lNextCallTick;
	lInterval = GetCallInterval(callInfo.hCall, &lNextCallTick);
	bc.dwSecInterval = (DWORD)lInterval;
	bc.nNextCall = (int)lNextCallTick;
}

VOID CScriptTimeCallManager::ClearBootCallList()
{
	/*CList<ScriptBootCallData>::NodeType *pNode;
	CList<ScriptBootCallData>::Iterator it(m_BootCallList);*/

	// 此地方会造成多次析构。m_BootCallList.clear()自身会析构对象
	////循环调用析构函数
	//for (pNode = it.first(); pNode; pNode = it.next())
	//{
	//	ScriptBootCallData &bc = *pNode;
	//	bc.~ScriptBootCallData();
	//}

	m_BootCallList.clear();
}

INT_PTR CScriptTimeCallManager::RunBootCalls()
{
	CNpc *pNpc;
	CScene *pScene;
	CFuBen *pFb;
	INT_PTR Result = 0;
	CMiniDateTime tNextCall;
	CMiniDateTime tNow;
	CFuBenManager *pFbMgr = GetLogicServer()->GetLogicEngine()->GetFuBenMgr();
	CList<ScriptBootCallData>::NodeType *pNode;
	CList<ScriptBootCallData>::Iterator it(m_BootCallList);

	tNow = CMiniDateTime::now();
	//必须按顺序检查和调用BootCall，否则可能违背脚本中带有先后关系的调用逻辑顺序
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		ScriptBootCallData &bc = *pNode;
		if (tNow.tv >= bc.nNextCall.tv)
		{
			//查找NPC所在场景对象
			if (!pFbMgr->GetFbPtrBySceneName(bc.sSceneName, pFb, pScene))
			{
				OutputMsg(rmWaning, _T("can not call BootCall[%s-%s:%s] scene was removed"),
					bc.sSceneName, bc.sNPCName, bc.sFn);
				continue;
			}
			//在场景中查找NPC对象
			if ( !(pNpc = pScene->GetNpc(bc.sNPCName)) )
			{
				OutputMsg(rmWaning, _T("can not call BootCall[%s-%s:%s] NPC was removed"),
					bc.sSceneName, bc.sNPCName, bc.sFn);
				continue;
			}
			//检查NPC脚本中是否还存在此函数
			if ( !pNpc->GetScript().FunctionExists(bc.sFn) )
			{
				OutputMsg(rmWaning, _T("can not call BootCall[%s-%s:%s] function does not exists any more"),
					bc.sSceneName, bc.sNPCName, bc.sFn);
				continue;
			}
			//循环调用脚本
			tNextCall = bc.nNextCall;
			do 
			{
				tNextCall.tv += bc.dwSecInterval;
				pNpc->GetScript().Call(bc.sFn, bc.args, m_SRetList, 0);
			}
			while (tNow.tv >= tNextCall.tv);
			Result++;
		}
	}
	ClearBootCallList();//清空列表以防止重复调用
	return Result;
}

INT_PTR CScriptTimeCallManager::LoadBootCalls(LPCTSTR sFilePath)
{
	if (!FileExists(sFilePath))
		return 0;

	ScriptBootCallFileHeader* pHdr;
	CMemoryStream ms;
	if (ms.loadFromFile(sFilePath) < sizeof(pHdr))
		return -1;

	pHdr = (ScriptBootCallFileHeader*)ms.getMemory();
	if(!pHdr)
	{
		return 0;
	}
	//验证文件是否有效
	if (pHdr->ident.uIdent != ScriptBootCallFileHeader::FileIdent.uIdent)
	{
		OutputMsg(rmError, _T("invalid BootCall file"));
		return -2;
	}
	if (pHdr->version.uVersion != ScriptBootCallFileHeader::FileVersion.uVersion)
	{
		OutputMsg(rmError, _T("invalid BootCall file vesion %d.%d.%d.%d"), 
			pHdr->version.p.v, pHdr->version.p.y, pHdr->version.p.m, pHdr->version.p.d);
		return -3;
	}
	if (pHdr->dwSizeData != sizeof(ScriptBootCallData))
	{
		OutputMsg(rmError, _T("BootCall data structure has been modified"));
		return -4;
	}
	if (pHdr->dwDataCRC32 != ~CRC32Update(0xFFFFFFFF, pHdr + 1, pHdr->dwDataSize))
	{
		OutputMsg(rmError, _T("BootCall data CRC check failure"));
		return -5;
	}

	ClearBootCallList();

	//读取BootCall数据
	if (pHdr->dwNumCalls > 0)
	{
		ScriptBootCallData bcd;

		ms.setPosition(sizeof(*pHdr));
		for (INT_PTR i=pHdr->dwNumCalls-1; i>-1; --i)
		{
			CLinkedNode<ScriptBootCallData> *pNode = m_BootCallList.linkAtLast(bcd);
			ScriptBootCallData& bc = *pNode;
			ZeroMemory(&bc, sizeof(bc));
			bc.loadFromStream(ms);
		}
	}

	return pHdr->dwNumCalls;
}

INT_PTR CScriptTimeCallManager::SaveBootCalls(LPCTSTR sFilePath)
{
	TCHAR sSavePath[1024];

	//获取保存文件的目录路径，如果目录路径字符长度超出缓存长度则报错
	if ( ExtractFileDirectory(sFilePath, sSavePath, ArrayCount(sSavePath)) >= ArrayCount(sSavePath) )
	{
		OutputMsg(rmError, _T("unable to save BootCall Data to %s, path to long"), sFilePath);
		return -1;
	}
	//逐层判断目录是否存在，如果不存在则创建
	if ( !DeepCreateDirectory(sSavePath) )
	{
		OutputError(GetLastError(), _T("unable to create BootCall directory %s "), sSavePath);
		return -2;
	}

	ScriptBootCallFileHeader hdr;
	CMemoryStream ms;

	ZeroMemory(&hdr, sizeof(hdr));
	hdr.ident = ScriptBootCallFileHeader::FileIdent;
	hdr.version = ScriptBootCallFileHeader::FileVersion;
	hdr.dwNumCalls = (DWORD)m_BootCallList.count();
	hdr.dwSizeData = sizeof(ScriptBootCallData);
	ms.setSize(sizeof(hdr));
	ms.setPosition(sizeof(hdr));

	//写入数据
	CList<ScriptBootCallData>::NodeType *pNode;
	CList<ScriptBootCallData>::Iterator it(m_BootCallList);
	for (pNode = it.first(); pNode; pNode = it.next())
	{
		ScriptBootCallData &bc = *pNode;
		bc.saveToStream(ms);
	}
	//计算数据断CRC
	hdr.dwDataSize = (DWORD)ms.getSize() - sizeof(hdr);
	if (hdr.dwDataSize == 0)
	{
		hdr.dwDataSize = 0;
	}
	else
	{
		hdr.dwDataCRC32 = ~CRC32Update(0xFFFFFFFF, (LPCSTR)ms.getMemory() + sizeof(hdr), hdr.dwDataSize);
	}
	//写入文件头
	memcpy(ms.getMemory(), &hdr, sizeof(hdr));

	ms.saveToFile(sFilePath);
	return hdr.dwNumCalls;
}

ULONGLONG CScriptTimeCallManager::GetCurrentTick()
{
	return GetGlobalLogicEngine()->getMiniDateTime();
}

bool CScriptTimeCallManager::ScriptBootCallData::loadFromStream(wylib::stream::CBaseStream &stm)
{
	stm.read(sSceneName, sizeof(sSceneName));
	stm.read(sNPCName, sizeof(sNPCName));
	stm.read(sFn, sizeof(sFn));
	stm.read(&nNextCall, sizeof(nNextCall));
	stm.read(&dwSecInterval, sizeof(dwSecInterval));
	args.loadFromStream(stm);
	return true;
}

void CScriptTimeCallManager::ScriptBootCallData::saveToStream(wylib::stream::CBaseStream &stm)
{
	stm.write(sSceneName, sizeof(sSceneName));
	stm.write(sNPCName, sizeof(sNPCName));
	stm.write(sFn, sizeof(sFn));
	stm.write(&nNextCall, sizeof(nNextCall));
	stm.write(&dwSecInterval, sizeof(dwSecInterval));
	args.saveToStream(stm);
}

