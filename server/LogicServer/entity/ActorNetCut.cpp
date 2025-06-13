#include "StdAfx.h"
#include "Actor.h"
#include "../script/interface/LogDef.h" 
#include<vector>
  
#define FUBEN_ID_KFSL 22 //跨服首领特殊处理 
#define CAN_DISPLAY_NUM_KFSL 30 //同屏数量暂定20个 后续看看是否优化
#define BOSS_ID_KFSL 385     //bossID

int CActor::CanNetworkCutDownDisplayCount()
{
    return (int)CAN_DISPLAY_NUM_KFSL; 
}
bool CActor::CanNetworkCutDown()
{ 
    return false; 
    //return true; 
    //return m_CanNetworkCutDown; 
}
bool CActor::CheckNetworkCutDown(const int fuId, const int SceneId)
{ 
    if (!GetLogicServer()->IsCrossServer())
    {
        return false; 
    } 

    if(fuId == FUBEN_ID_KFSL)
    {   
        return true; 
    }
    return false; 
}
void CActor::InitCanDisplay()
{ 
    m_CanDisplayList.clear();
    m_PrecedenceDisplayList.clear(); 
}
void CActor::OnAddCanDisplay(EntityHandleTag& tag)
{ 
    m_CanDisplayList.push_back(tag);
} 
void CActor::OnCalculateDisplay()
{  
	CEntityManager* em = GetGlobalLogicEngine()->GetEntityMgr();
    std::vector<EntityHandleTag>::iterator it = m_CanDisplayList.begin();   
    for (;it != m_CanDisplayList.end();)
    {
        CEntity *pEntity = em->GetEntity((*it).m_handle);
        if(!pEntity)
        {
            it = m_CanDisplayList.erase(it); 
        }
        else
        {
            ++it;
        }
    }
    if(m_CanDisplayList.size() > CAN_DISPLAY_NUM_KFSL)
    { 
        auto iter = m_CanDisplayList.erase(std::begin(m_CanDisplayList) + CAN_DISPLAY_NUM_KFSL, std::end(m_CanDisplayList));
    } 
}

//增加可视玩家
bool CActor::OnAddCanSeeEntity(EntityHandle & hEntity)
{  
    unsigned long long handle = (unsigned long long)hEntity.GetHandle(); 
    if (m_CanSeeEntityList.insert(std::make_pair(handle, hEntity)).second)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//删除可视玩家
bool CActor::OnClearCanSeeEntity(EntityHandle & hEntity)
{
    if(m_CanSeeEntityList.empty())
    {
        return false;
    }
    unsigned long long handle = (unsigned long long)hEntity.GetHandle(); 
    std::map<unsigned long long, EntityHandle>::iterator it = m_CanSeeEntityList.find(handle);
    if (it != m_CanSeeEntityList.end())
    {
        m_CanSeeEntityList.erase(it);
        return true;
    }
    else
    {
        return false;
    }
}
  
bool CActor::OnCheckCanSeeEntity(EntityHandle & hEntity)
{
    if(m_CanSeeEntityList.empty())
    {
        return false;
    }
    unsigned long long handle = (unsigned long long)hEntity.GetHandle(); 

    if (m_CanSeeEntityList.find(handle) == m_CanSeeEntityList.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}
