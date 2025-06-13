#include "StdAfx.h"
#include "Container.hpp"
#include "AchieveMrg.h"

using namespace jxInterSrvComm::DbServerProto;
CAchieveMrg::CAchieveMrg()
{
    m_offlineAchieve.clear();
    isDatatify = false;
}
CAchieveMrg::~CAchieveMrg()
{

}
void CAchieveMrg::Load()
{
	LoadAllOffineAchieve();
}
void CAchieveMrg::LoadAllOffineAchieve()
{
	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcLoadofflineAchieve);
		int nServerId = GetLogicServer()->GetServerIndex();
		DataPacket << nServerId << nServerId;
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CAchieveMrg::OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size)
{
	int nRawServerId, nLoginServerId;
	CDataPacketReader inPacket(data,size);
	int nErrorCode;
	inPacket >> nRawServerId >> nLoginServerId >>  nErrorCode;

	if (nCmd == dcLoadofflineAchieve && nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
	{
		int nCount = 0;
        inPacket >>nCount;
        for (int i =0; i < nCount; i++)
        {
            OffLineAchieveData data;
            unsigned int nActorId = 0;
            inPacket >> nActorId;
            inPacket >> data.nType;
            inPacket >> data.nSubType;
            inPacket >> data.nValue;
            inPacket >> data.nWay;
            m_offlineAchieve[nActorId].push_back(data);
		}
	}
}

void CAchieveMrg::OnEnterGame(CActor* pActor)
{
	dealAchieve(pActor);
}

void CAchieveMrg::dealAchieve(CActor* pActor)
{
	if (!pActor)
	{
		return ;
	}
    int nActorId = pActor->GetId();
    m_Inter it = m_offlineAchieve.find(nActorId);
    if(it != m_offlineAchieve.end())
    {
        for (INT_PTR i= it->second.size()-1; i>=0;i--)
        {	
            OffLineAchieveData& data = it->second[i];
            pActor->GetAchieveSystem().ActorAchievementComplete(data.nType, data.nValue, data.nSubType, data.nWay);
        }
        m_offlineAchieve.erase(it);
        isDatatify = true;
        return;
    }
}



void CAchieveMrg::addOfflineAchieve(unsigned int nActorId, int nAchieveType,int nValue, int nAchieveSubType, int nWay)
{
    OffLineAchieveData data;
    data.nType = nAchieveType;
    data.nSubType = nAchieveSubType;
    data.nValue = nValue;
    data.nWay = nWay;
    m_offlineAchieve[nActorId].push_back(data);
    isDatatify = true;
}


// 推送数据到DBServer进行存储
VOID CAchieveMrg::Save()
{
    CDataClient* pDbClient = GetLogicServer()->GetDbClient();
    if (pDbClient->connected())
    {
        CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveofflineAchieve);
        int nServerId = GetLogicServer()->GetServerIndex();
        dataPacket << nServerId << nServerId;

        int nCount = 0;
        INT_PTR pos = dataPacket.getPosition();
        dataPacket << nCount;
        m_Inter runIter = m_offlineAchieve.begin();
        for (;runIter != m_offlineAchieve.end(); ++runIter)
        {
            for(int i = 0; i < (runIter->second).size(); i++)
            {
                OffLineAchieveData& data = runIter->second[i];
                dataPacket<<(unsigned int)(runIter->first);
                dataPacket.writeBuf(&data,sizeof(data));
                nCount++;
            }
           
        }
        int* pCount = (int*)dataPacket.getPositionPtr(pos);
        *pCount = nCount;

        GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
    }
}