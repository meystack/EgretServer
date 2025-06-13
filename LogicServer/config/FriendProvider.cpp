#include "StdAfx.h"
#include "FriendProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;

CFriendProvider::CFriendProvider(): Inherited(), m_DataAllocator(_T("FriendDataAlloc"))
{

}

CFriendProvider::~CFriendProvider()
{

}

bool CFriendProvider::LoadFriendConfig(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on Friend config"));
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load Friend config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load Friend config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CFriendProvider::readConfigs()
{
	if ( !openGlobalTable("FriendConfig") )
		return false;

	m_MaxFriendNum = getFieldInt("MaxFriendNum");
	m_MaxStrangerNum = getFieldInt("MaxStrangerNum");
	m_MaxEnemyNum = getFieldInt("MaxEnemyNum");
	m_MaxBlacklist = getFieldInt("MaxBlacklist");

	m_TrackItemId = getFieldInt("trackItemId");
	m_TrackNeedYb = getFieldInt("trackNeedYb");
	m_AlarmItemId = getFieldInt("alarmItemId");
	m_SendNeedCoin = getFieldInt("sendNeedCoin");

	m_MinAttendMaster = getFieldInt("minAttendMaster");
	m_MaxAttendMaster = getFieldInt("maxAttendMaster");
	m_CanBeMaster = getFieldInt("canBeMaster");

	m_MaxMaster = getFieldInt("maxMaster");
	m_maxDisciple = getFieldInt("maxDisciple");

	m_graduateLevel = getFieldInt("graduateLevel");
	m_needIntimacy = getFieldInt("needIntimacy");

	m_publishAttendMaster = getFieldInt("publishAttendMaster");

	m_maxCanMasterNum = getFieldInt("maxCanMasterNum");

	m_desertMasterNeedCoin = getFieldInt("desertMasterNeedCoin");
	m_driveOutMaster = getFieldInt("driveOutMaster");

	m_awardExpRate = getFieldInt("expRate");

	m_awardExpTeacherRate = getFieldInt("expTeacherRate");
	m_yuanbaoPupilRate	= getFieldInt("yuanbaoPupilRate");

	m_changeNamCoin = getFieldInt("changeNamCoin");

	m_CallFriendCoin = getFieldInt("CallRequestCust");
	m_DieListCount = getFieldInt("DieListCount");
	m_TrackActorYuanB = getFieldInt("TrackActorYuanB");
	m_nMaxMoodString = getFieldInt("MaxMoodString");
	
	m_nTrackActorFeeType = getFieldInt("TrackActorFeeType");
	if (feildTableExists("TrackActorFeeNum") && openFieldTable("TrackActorFeeNum"))
	{
		size_t count = lua_objlen(m_pLua, -1);
		if (count > 0 && enumTableFirst())
		{
			INT_PTR idx = 0;
			do 
			{
				if (idx < DAILY_TRACK_FEE_NUM)
				{
					m_nTrackActorFeeNum[idx++] = getFieldInt(NULL);
				}
				else
				{
					endTableEnum();
					break;
				}
			} while (enumTableNext());
		}
		closeTable();
	}

	closeTable();
	return true;
}

void CFriendProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[FriendProvider]");
	s += sError;
	throw s;
}
