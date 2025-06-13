#include "StdAfx.h"
/*
#include "../common/db/Fuben.h"

int CDBDataClient::SaveFubenData(int nActorid,int Fbid,int Daycount,int Progress)
{
	int ret = reSucc;
	if ( !m_pSQLConnection->Connected() )
	{
		ret = reDbErr;
	}
	else
	{
		//insert
		int nError = m_pSQLConnection->Exec(szSQLSP_AddCharFubenData, nActorid,Fbid,Daycount,Progress);
		if ( !nError )
		{
			ret = reSucc;
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			ret = reDbErr;
		}
	}
	if (ret != reSucc)
	{
		OutputMsg(rmError,_T("save fuben data error!id=%d,Fbid=%d,Daycount=%d,Progress=%d"),
			nActorid,Fbid,Daycount,Progress);
	}
	return ret;
}

VOID CDBDataClient::SaveFuben(CDataPacketReader &inPacket)
{
	int nActorId = 0;
	inPacket >> nActorId;
	CDataPacket& retPack = allocProtoPacket(dcSaveFuben);
	retPack << nActorId;

	BYTE ret = CleanFuben(nActorId);
	
	if (ret != reSucc)
	{
		retPack << (BYTE)ret;
		flushProtoPacket(retPack);
		return;
	}

	WORD nCount = 0;
	inPacket >> nCount;
	for (int i = 0; i < nCount; i++)
	{
		FubenData data;
		inPacket >> data;
		ret = SaveFubenData(nActorId,data.wFubenId,data.wDaycount,data.bProgress);
		if (ret != reSucc)
		{			
			break;
		}
	}
	retPack << (BYTE)ret;
	
	flushProtoPacket(retPack);
}

int CDBDataClient::CleanFuben(INT_PTR nActorid)
{
	int ret = reSucc;
	if ( !m_pSQLConnection->Connected() )
	{
		ret = reDbErr;
	}
	else
	{
		//先删除
		int nError = m_pSQLConnection->Exec(szSQLSP_CleanCharFuben, nActorid);
		if ( !nError )
		{
			m_pSQLConnection->ResetQuery();		
		}else
		{
			ret = reDbErr;
		}
	}
	if (ret != reSucc)
	{
		OutputMsg(rmError,_T("cleanfubendata error!id=%d"),
			nActorid);
	}
	return ret;
}

VOID CDBDataClient::LoadFubenData(CDataPacketReader &inPacket)
{
	int nActorId;
	inPacket >> nActorId;
	LoadFubenData(nActorId);

}

VOID CDBDataClient::LoadFubenData(INT_PTR nActorId)
{
	CDataPacket& retPack = allocProtoPacket(dcLoadFuben);
	retPack << nActorId;

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		retPack << (BYTE)reDbErr;
	}
	else
	{
		//向数据库查询进度任务数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadCharFubenData, nActorId);
		if ( !nError )
		{
			retPack << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			WORD nCount = m_pSQLConnection->GetRowCount();
			retPack << nCount;
			while ( pRow)
			{
				FubenData data;
				sscanf(pRow[0], "%d", &data.wFubenId);
				sscanf(pRow[1], "%d", &data.wDaycount);
				sscanf(pRow[2], "%d", &data.bProgress);
				retPack << data;
				pRow = m_pSQLConnection->NextRow();
			}
			m_pSQLConnection->ResetQuery();
		}
		else
		{
			retPack << (BYTE)reDbErr;
		}
	}

	flushProtoPacket(retPack);
}
*/