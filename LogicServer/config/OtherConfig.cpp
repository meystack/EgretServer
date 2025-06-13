#include "StdAfx.h"
#include "LogicDataProvider.h"

bool CLogicDataProvider::LoadScriptBootCall(LPCTSTR sFilePath)
{
	return m_pLogicEngine->GetScriptTimeCallManager().LoadBootCalls(sFilePath) >= 0;
}

bool CLogicDataProvider::SaveScriptBootCall(LPCTSTR sFilePath)
{
	return m_pLogicEngine->GetScriptTimeCallManager().SaveBootCalls(sFilePath) >= 0;
}
