#include "StdAfx.h"
#include "TreasureSystem.h"

bool CTreasureSystem::Initialize(void* data, size_t size)
{
    return true;
}

void CTreasureSystem::OnEnterGame()
{
    
}

void CTreasureSystem::ProcessNetData(INT_PTR nCmd, CDataPacketReader& packet)
{
    if(!m_pEntity) return;
    switch (nCmd)
    {
        case encBless:
        {
            ActorBless(packet);
            break;
        }
    default:
        break;
    }
}





void CTreasureSystem::ActorBless(CDataPacketReader& pack)
{
    if(!m_pEntity) return;

    BYTE nCount= 0;
    pack >> nCount;
    int nAddRelust = 0;
    
    CBlessCfg& cfg = GetLogicServer()->GetDataProvider()->GetBlessCfg();
    if(!(m_pEntity->CheckActorSource(0, cfg.m_nItemId, nCount, tstUI)))
    {
        return;
    }

    int nBlessValue = m_pEntity->GetProperty<int>(PROP_ACTOR_Bless);

    for(int i = 0; i < nCount; i++)
    {

        BlessCfg* pBlessCfg = cfg.GetBlessCfg(nBlessValue);
        if(pBlessCfg)
        {
            nAddRelust = pBlessCfg->m_nRate.GetValue();
        }
        if(m_pEntity->RemoveConsume(0, cfg.m_nItemId, 1, -1, -1, -1, 0,GameLog::Log_Bless))
            nBlessValue += nAddRelust;
    }

    if(nBlessValue < 0)
        nBlessValue = 0;
    int nNowValue = m_pEntity->GetProperty<int>(PROP_ACTOR_Bless);
    int nSubValue = nBlessValue - nNowValue;
    std::string nTipMsg = intToString(nSubValue);
    if(nSubValue > 0)
    {
        nTipMsg = intToString(nSubValue, "+");
        ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmBlessAddValue, tstFigthing, nTipMsg.c_str());
    }
    else if(nSubValue < 0)
    {
        ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmBlessSubValue, tstFigthing, nTipMsg.c_str());
    }
    if(nSubValue != 0)
    {
        //m_pEntity->SetProperty<int>(PROP_ACTOR_Bless, nBlessValue);
        m_pEntity->SetActorBless(nBlessValue);
        // ((CActor*)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);	
    }
    else
    {
         ((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmBlessNoAddValue, tstFigthing);   
    }

    CActorPacket ap;
    CDataPacket& data = m_pEntity->AllocPacket(ap);
    data << (BYTE)enTreasureSystem <<(byte)ensBless;
    data <<(int)nBlessValue;
    ap.flush();
}

void CTreasureSystem::RefreshBless(int nDay)
{
    if(!m_pEntity) return;

    CBlessCfg& cfg = GetLogicServer()->GetDataProvider()->GetBlessCfg();
    int nBlessValue = m_pEntity->GetProperty<int>(PROP_ACTOR_Bless);
    BlessCfg* pBlessCfg = cfg.GetBlessCfg(nBlessValue);
    if(pBlessCfg && pBlessCfg->m_nDailyCost > 0)
    {
       nBlessValue -= nDay * pBlessCfg->m_nDailyCost;
        if(nBlessValue < 0)
            nBlessValue = 0;
        
        m_pEntity->SetProperty<int>(PROP_ACTOR_Bless, nBlessValue);
        ((CMonster *)m_pEntity)->CollectOperate(CEntityOPCollector::coRefAbility);
    }
    
}