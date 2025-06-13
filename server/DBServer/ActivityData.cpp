#include "StdAfx.h"
#include "../LogicDef.h"
#include "ActivityDb.h"
#include <vector>

void CLogicDBReqestHandler::LoadActivityList(CDataPacketReader &inPacket)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::LoadActivityList");
	const int max_guild = 100;
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	CDataPacket& tmp = allocProtoPacket(dcLoadActivityList);
	tmp << nRawServerId << nLoginServerId;
	CDataPacket* out = &tmp;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		(*out) << (BYTE)reDbErr;
	}
	else
	{
        //向数据库查询活动数据
		int nError = m_pSQLConnection->Query(szSQLSP_LoadActivityData);
		if ( !nError )
		{
            (*out) << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
            INT_PTR pos = out->getPosition();
			(*out) << (int)0;
			nCount = 0;
            GlobalActivityData activitydata;
            while (pRow)
			{
                nCount++;
                sscanf(pRow[0], "%u", &activitydata.nId);
                sscanf(pRow[1], "%u", &activitydata.nActivityType);
                sscanf(pRow[2], "%u", &activitydata.nTimeType);
                unsigned int startTime,endTime;
                sscanf(pRow[3], "%u", &startTime);
                sscanf(pRow[4], "%u", &endTime);
                activitydata.nStartTime = startTime;
                activitydata.nEndTime = endTime;
                out->writeBuf(&activitydata,sizeof(activitydata));
				pRow = m_pSQLConnection->NextRow();
            }
            int* pCount = (int*)out->getPositionPtr(pos);
			*pCount = nCount;
			m_pSQLConnection->ResetQuery();
        }
		else
		{
			(*out) << (BYTE)reDbErr;
		}
    }
    flushProtoPacket((*out));
}

void CLogicDBReqestHandler::SaveActivityList(CDataPacketReader &inPacket)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::SaveActivityList");
	int nRawServerId = 0, nLoginServerId = 0;
	inPacket >> nRawServerId >> nLoginServerId;
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		int nErrorCode = m_pSQLConnection->Exec(szSQLSP_DeleteActivityData);
		if (!nErrorCode)
		{
			m_pSQLConnection->ResetQuery();
		}

		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_SaveActivityData); //表头放这里

		int nCount = 0;
		inPacket >> nCount;
        GlobalActivityData activitydata;
		for(int i = 0; i < nCount; ++i)
		{
			inPacket.readBuf(&activitydata,sizeof(activitydata));
			OutputMsg(rmTip,"[Global Activity] 活动存储(%d)，活动Type=%d, 活动ID=%d !", i, activitydata.nActivityType, activitydata.nId);
			
			strcat(m_pHugeSQLBuffer, " (");
			sprintf(buff, "%u,%u,%u,%u,%u", activitydata.nId, activitydata.nActivityType, activitydata.nTimeType, (unsigned int)activitydata.nStartTime, (unsigned int)activitydata.nEndTime);
			strcat(m_pHugeSQLBuffer, buff);
			strcat(m_pHugeSQLBuffer, " )");	

			if(i < nCount-1)
			{
				strcat(m_pHugeSQLBuffer,",");	
			}
		}

		nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
		if(!nErrorCode)
		{
			m_pSQLConnection->ResetQuery(); 
		}
	}
}

void CLogicDBReqestHandler::LoadActorActivity(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
	CDataPacket& out = allocProtoPacket(dcLoadPersonActivity);
	out << nRawServerId << nLoginServerId;
	out << nActorID;

	static std::vector<PersonStopedActivity> arrStopActivity;
	arrStopActivity.clear();

	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		out << (BYTE)reDbErr;
	}
	else
	{
		int nError = m_pSQLConnection->Query(szSQLSP_LoadActorActivityData, nActorID);
		if ( !nError )
		{
            out << (BYTE)reSucc;
			MYSQL_ROW pRow = m_pSQLConnection->CurrentRow();
			int nCount = m_pSQLConnection->GetRowCount();
            INT_PTR pos = out.getPosition();
			out << (int)0;
			nCount = 0;
			PersonActivtyData activitydata;
            PersonStopedActivity stopdata;
            while (pRow)
			{
                sscanf(pRow[0], "%u", &activitydata.nId);
                sscanf(pRow[1], "%u", &activitydata.nActivityType);
                int nExpiredTime;
                sscanf(pRow[2], "%d", &nExpiredTime);
                activitydata.nExpiredTime = (unsigned int)nExpiredTime;
				if (nExpiredTime == -1)//活动已结束
				{
					stopdata.nId = activitydata.nId;
					stopdata.nActivityType = activitydata.nActivityType;
					arrStopActivity.push_back(stopdata);
				}
				else
				{
					nCount++;
					out.writeBuf(&activitydata,sizeof(activitydata));
				}
				pRow = m_pSQLConnection->NextRow();
            }
            int* pCount = (int*)out.getPositionPtr(pos);
			*pCount = nCount;
			m_pSQLConnection->ResetQuery();

			// 把结束的活动丢入
			nCount = (int)arrStopActivity.size();
			out << nCount;
			for (int i = 0; i < nCount; i++)
			{
				out.writeBuf(&arrStopActivity[i],sizeof(stopdata));
			}
        }
		else
		{
			out << (BYTE)reDbErr;
		}
	}
    flushProtoPacket(out);
}

void CLogicDBReqestHandler::SaveActorActivity(CDataPacketReader &inPacket, unsigned int nActorID, int nRawServerId, int nLoginServerId)
{
    DECLARE_TIME_PROF("CLogicDBReqestHandler::SaveActorActivity");
	
	//数据库连接是否就绪
	if ( !m_pSQLConnection->Connected() )
	{
		return;
	}
	else
	{
		char buff[1024];
		strcpy(m_pHugeSQLBuffer, szSQLSP_SaveActorActivityData); //表头放这里

		PersonActivtyData activitydata;
		PersonStopedActivity stopdata;
		bool hasActivity = false;
		
		// 保存运行中的
		int nCount = 0;
		inPacket >> nCount;
		for(int i = 0; i < nCount; ++i)
		{
			inPacket.readBuf(&activitydata,sizeof(activitydata));
			
			strcat(m_pHugeSQLBuffer, " (");
			sprintf(buff, "%u,%u,%u,%u", nActorID, activitydata.nId, activitydata.nActivityType, (unsigned int)activitydata.nExpiredTime);
			strcat(m_pHugeSQLBuffer, buff);
			strcat(m_pHugeSQLBuffer, " )");	

			if(i < nCount-1)
			{
				strcat(m_pHugeSQLBuffer,",");	
			}
			hasActivity = true;
		}
		// printf("%s\n",m_pHugeSQLBuffer);
		// 保存结束的
		nCount = 0;
		if (inPacket.getAvaliableLength() > 4)
			inPacket >> nCount;
		unsigned int temp = (unsigned int)-1;
		int nId = 0;
		int nActivityType = 0;
		for(int i = 0; i < nCount; ++i)
		{
			inPacket >> nId ;   //id
            inPacket >> nActivityType;  //type
			// inPacket.readBuf(&stopdata,sizeof(stopdata));
			if(hasActivity)
				strcat(m_pHugeSQLBuffer, ",");

			strcat(m_pHugeSQLBuffer, "(");
			sprintf(buff, "%u,%u,%u,%d", nActorID, nId, nActivityType, -1);
			strcat(m_pHugeSQLBuffer, buff);
			strcat(m_pHugeSQLBuffer, " )");	

			// if(i < nCount-1)
			// {
			// 	strcat(m_pHugeSQLBuffer,",");	
			// }
			hasActivity = true;
		}
		// printf("%s\n",m_pHugeSQLBuffer);
		if (hasActivity)
		{
			int nErrorCode = m_pSQLConnection->Exec(m_pHugeSQLBuffer);
			if(!nErrorCode)
			{
				m_pSQLConnection->ResetQuery(); 
			}
		}
	}
}
