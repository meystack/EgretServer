#include "StdAfx.h"


CActorDataPacketStat::~CActorDataPacketStat()
{
	Clear();
	m_Records.empty();
	m_AllRecords.empty();
	m_AllRecords.clear();
}

void CActorDataPacketStat::AddRecord(const CDataPacketRecord &record)
{
	CDataPacketRecord *pRecord = CActorDataPacketStatAlloc::m_spADPSAllocator->AllocRecord();
	if (pRecord)
	{
		pRecord->SetData(record);
		m_Records.add(pRecord);
	}
}

void CActorDataPacketStat::AddRecord(BYTE ucSysId, BYTE ucMsgId, WORD wLength, TICKCOUNT tickCount)
{
	AddRecord(CDataPacketRecord(ucSysId, ucMsgId, wLength, tickCount));
}

void CActorDataPacketStat::Clear()
{
	for (INT_PTR i = 0; i < m_Records.count(); i++)
	{
		CActorDataPacketStatAlloc::m_spADPSAllocator->FreeRecord(m_Records[i]);
	}

	m_Records.clear();
}

void CActorDataPacketStat::WriteToFile(CActor* pActor)
{
	static LPCTSTR szFileName = _T("ActorPacket.txt");
	using namespace wylib::stream;
	CFileStream fs(szFileName, CFileStream::faWrite, CFileStream::AlwaysOpen);
	fs.setPosition(fs.getSize());
	WriteToStream(fs, pActor);
}

void CActorDataPacketStat::WriteToStream(wylib::stream::CBaseStream &stream, CActor* pActor)
{
	const char* pActorName = NULL;
	if (pActor)
	{
		pActorName = pActor->GetEntityName();
	}

	const TCHAR szSep[] = _T("-------------------------------------------------------------------------------------------\r\n");
	TCHAR szDateTime[256];
	CUtility::GetDateTimeDesc(szDateTime);
	stream.write(szSep, _tcslen(szSep));
	stream.write(szDateTime, _tcslen(szDateTime));
	TCHAR szDesc[256] = {0};
	_stprintf(szDesc, _T("\r\n%-20s%-25s%-25s%-25s%-25s\r\n"), _T("消息号"), _T("系统号"), _T("消息号"), _T("消息体长度"), _T("发送时间"));
	stream.write(szDesc, _tcslen(szDesc));

	TCHAR szContent[256] = {0};
	TCHAR szContentFmt[256] = {0};
	_tcscpy(szContentFmt, _T("%-20d%-25d%-25d%-25d%-25lld\r\n"));
	for (INT_PTR i = 0; i < m_Records.count(); i++)
	{
		CDataPacketRecord* pRecord = m_Records[i];
		_stprintf(szContent, szContentFmt, (int)(i+1), (int)pRecord->m_nSystemId, 
			(int)pRecord->m_nMsgId, (int)pRecord->m_nLenth, (INT64)pRecord->m_nRecordTime);
		stream.write(szContent, _tcslen(szContent));
	}
}

void CActorDataPacketStat::ClearRecords()
{
	m_AllRecords.empty();
	m_AllRecords.clear();
}

void CActorDataPacketStat::AddAllRecord(BYTE ucSysId, BYTE ucMsgId, WORD wLength, TICKCOUNT tickCount,char* sMapName,int nPosX,int nPosY)
{
	TDATAPACKETRECORD tmpRecord;
	tmpRecord.m_nSystemId = ucSysId;
	tmpRecord.m_nMsgId = ucMsgId;
	tmpRecord.m_nLenth = wLength;
	tmpRecord.m_nRecordTime = tickCount;
	tmpRecord.m_TotalSzie = wLength;
	if(sMapName)
	{
		_asncpytA(tmpRecord.m_MapName,sMapName);
	}
	else
	{
		_asncpytA(tmpRecord.m_MapName,"");
	}
	tmpRecord.m_PosX = nPosX;
	tmpRecord.m_PosY = nPosY;
	tmpRecord.m_Count = 1;

	int nCount = (int)m_AllRecords.count();
	for(int i=0;i<nCount;i++)
	{
		TDATAPACKETRECORD& temp = m_AllRecords[i];
		if(temp.m_nSystemId == ucSysId && temp.m_nMsgId == ucMsgId)
		{
			temp.m_Count++;
			temp.m_TotalSzie += wLength;
			return;
		}
	}

	m_AllRecords.add(tmpRecord);
}

void CActorDataPacketStat::WriteRecordToFile()
{
	if(m_Count < 0)
		m_Count = 0;
	static LPCTSTR szFileName = _T("data/log/PacketRecord.txt");
	using namespace wylib::stream;
	CFileStream fs(szFileName, CFileStream::faWrite, CFileStream::AlwaysOpen);
	fs.setPosition(fs.getSize());

	m_Count++;
	WriteRecordToStream(fs);
}

void CActorDataPacketStat::WriteRecordToStream(wylib::stream::CBaseStream &stream)
{
	const TCHAR szSep[] = _T("服务器下发的数据包-------------------------------------------------------------------------------------------\r\n");
	TCHAR szDateTime[256];
	CUtility::GetDateTimeDesc(szDateTime);
	stream.write(szSep, _tcslen(szSep));
	stream.write(szDateTime, _tcslen(szDateTime));
	TCHAR szCount[256] = {0};
	CVector<void *> actorList;
	GetGlobalLogicEngine()->GetEntityMgr()->GetOnlineAcotrPtrList(actorList);
	int nCount = (int)actorList.count();

	_stprintf(szCount,_T("%-20s%-2d%"),_T("玩家人数："),nCount);
	stream.write(szCount, _tcslen(szCount));

	TCHAR szDesc[256] = {0};
	_stprintf(szDesc, _T("\r\n%-20s%-25s%-25s%-25s%-25s%-25s\r\n"), _T("系统号"), _T("消息号"), _T("消息数量"), _T("单个消息体长度"),_T("总消息体长度"), _T("发送时间"));
	stream.write(szDesc, _tcslen(szDesc));

	TCHAR szContent[256] = {0};
	TCHAR szContentFmt[256] = {0};
	_tcscpy(szContentFmt, _T("%-20d%-25d%-25d%-25d%-25lld%-25lld\r\n"));
	INT64 nTotalSize = 0;
	for (INT_PTR i = 0; i < m_AllRecords.count(); i++)
	{
		TDATAPACKETRECORD& tempRecord = m_AllRecords[i];
		if(tempRecord.m_Count == 0)  
			tempRecord.m_Count = 1;
		int tmpSize = (int)(tempRecord.m_TotalSzie/tempRecord.m_Count);
		_stprintf(szContent, szContentFmt, (int)tempRecord.m_nSystemId, 
			(int)tempRecord.m_nMsgId, (int)tempRecord.m_Count,(int)tmpSize,(INT64)(tempRecord.m_TotalSzie), (INT64)tempRecord.m_nRecordTime);
		stream.write(szContent, _tcslen(szContent));
		nTotalSize += tempRecord.m_TotalSzie;
	}

	TCHAR szTotal[256] = {0};
	if(m_Count <= 0)
		m_Count = 1;

	_stprintf(szTotal,_T("%-2s%-20lld%-2s%-0.2f%mbps\r\n"),_T("数据总大小："),nTotalSize,_T("平均带宽："),nTotalSize * 8/(60*m_Count*1024*1024));
	stream.write(szTotal, _tcslen(szTotal));

	//ClearRecords();
}