#include "stdafx.h"
#include <iostream>
#include <sstream>

using namespace std;
 
//请求回城复活
AINodeRegister<AINodeReturnRelive> AINodeReturnRelive::reg(ANFT_ReturnRelive, "AINodeReturnRelive");
NodeExecState AINodeReturnRelive::Execute(CRobotClient* pActor, int level, bool isLog)
{
    pActor->GetLogicAgent().GetDefaultSystem().ReqRelive();
    return NodeExec_Success;
}

//请求元宝复活
AINodeRegister<AINodeYBRelive> AINodeYBRelive::reg(ANFT_YBRelive, "AINodeYBRelive");
NodeExecState AINodeYBRelive::Execute(CRobotClient* pActor, int level, bool isLog)
{
    return NodeExec_Fail;
}

//往 (移动)坐标点 移动一步
AINodeRegister<AINodeMovePoint> AINodeMovePoint::reg(ANFT_MovePoint, "AINodeMovePoint");
NodeExecState AINodeMovePoint::Execute(CRobotClient* pActor, int level, bool isLog)
{
    int x,y,dir;
    if (int nStep = pActor->GetLogicAgent().GetAndPopNextMovePosi(x,y,dir))
    {
        pActor->GetLogicAgent().GetDefaultSystem().Move(x,y,nStep,dir);
        pActor->GetLogicAgent().SetNextAction(650);
        pActor->GetLogicAgent().SetActionMove();
    }
    return NodeExec_Fail;
}

//设置目标 - 怪物
AINodeRegister<AINodeSetTarMonster> AINodeSetTarMonster::reg(ANFT_SetTarMonster, "AINodeSetTarMonster");
NodeExecState AINodeSetTarMonster::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTar = pActor->GetLogicAgent().GetTarget();
    if (pTar == NULL)
    {
        CClientMonster* pMon = pActor->GetLogicAgent().GetDefaultSystem().GetNearestMonster();
        if (pMon)
        {
            pActor->GetLogicAgent().SetTargetHandle(pMon->GetHandle());
        }
        else return NodeExec_Fail;
        
    } else if ( pTar->IsDeath() )
    {
        EntityHandle oldhandle = pTar->GetHandle();
        CClientMonster* pMon = pActor->GetLogicAgent().GetDefaultSystem().GetNearestMonsterNot(oldhandle);
        if (pMon)
        {
            pActor->GetLogicAgent().SetTargetHandle(pMon->GetHandle());
        }
        else return NodeExec_Fail;
    }
    return NodeExec_Success;
}

//设置目标 - 物品
AINodeRegister<AINodeSetTarItem> AINodeSetTarItem::reg(ANFT_SetTarItem, "AINodeSetTarItem");
NodeExecState AINodeSetTarItem::Execute(CRobotClient* pActor, int level, bool isLog)
{
    return NodeExec_Fail;
}

//设置(移动)坐标点 - 挂机点（并寻路）
AINodeRegister<AINodeSetMovePointAsHook> AINodeSetMovePointAsHook::reg(ANFT_SetMovePointAsHook, "AINodeSetMovePointAsHook");
NodeExecState AINodeSetMovePointAsHook::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetLogicAgent().GetHookPoint().isInvalid()) return NodeExec_Fail;
    pActor->GetLogicAgent().SetMovePoint(pActor->GetLogicAgent().GetHookPoint().x,pActor->GetLogicAgent().GetHookPoint().y);
    if (pActor->GetLogicAgent().CalcPath(false))
        return NodeExec_Success;
    return NodeExec_Fail;
}

//设置(移动)坐标点 - 目标旁（并寻路） 
AINodeRegister<AINodeSetMovePointAsTar> AINodeSetMovePointAsTar::reg(ANFT_SetMovePointAsTar, "AINodeSetMovePointAsTar");
NodeExecState AINodeSetMovePointAsTar::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTar = pActor->GetLogicAgent().GetTarget();
    if ( pTar == NULL ) return NodeExec_Fail;
    if ( pTar->IsDeath() ) return NodeExec_Fail;
    int tarX,tarY;
    EntityHandle handle = pActor->GetLogicAgent().GetTargetHandle();
    pActor->GetLogicAgent().GetDefaultSystem().GetEntityPos(handle,tarX,tarY);
    
    pActor->GetLogicAgent().SetMovePoint(tarX,tarY);
    if (pActor->GetLogicAgent().CalcPath())
        return NodeExec_Success;
    return NodeExec_Fail;
}

//攻击目标
AINodeRegister<AINodeAttackTarget> AINodeAttackTarget::reg(ANFT_AttackTarget, "AINodeAttackTarget");
NodeExecState AINodeAttackTarget::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTar = pActor->GetLogicAgent().GetTarget();
    if ( pTar == NULL ) return NodeExec_Fail;
    if ( pTar->IsDeath() ) return NodeExec_Fail;
    EntityHandle handle = pActor->GetLogicAgent().GetTargetHandle();
    pActor->GetLogicAgent().GetSkillSystem().NearAttack(handle);
    pActor->GetLogicAgent().SetNextAction(1000);
    pActor->GetLogicAgent().SetActionAttack();
    return NodeExec_Success;
}

//改变目标 - 怪物（非本目标）
AINodeRegister<AINodeChangeTarMonster> AINodeChangeTarMonster::reg(ANFT_ChangeTarMonster, "AINodeChangeTarMonster");
NodeExecState AINodeChangeTarMonster::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTar = pActor->GetLogicAgent().GetTarget();
    EntityHandle oldhandle;
    if (pTar) oldhandle = pTar->GetHandle(); 
    if (CClientMonster* pMon = pActor->GetLogicAgent().GetDefaultSystem().GetNearestMonsterNot(oldhandle))
    {
        pActor->GetLogicAgent().SetTargetHandle(pMon->GetHandle());
        return NodeExec_Success;
    }
    else 
    return NodeExec_Fail;
}

//设置挂机点
AINodeRegister<AINodeSetHook> AINodeSetHook::reg(ANFT_SetHook, "AINodeSetHook");
NodeExecState AINodeSetHook::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetLogicAgent().GetHookPoint().isInvalid())
    {
        int nSceneId = pActor->GetActorData().GetSceneId();
        SCENEMAPCONF * pSc = GetRobotMgr()->GetGlobalConfig().GetSceneConfig().GetSceneConfig(nSceneId);
        if (pSc)
        {
            int count = pSc->hookList.nCount;
            if (count > 0)
            {
                int idx = rand()%count;
                auto& pos = pSc->hookList.pPosList[idx];
                pActor->GetLogicAgent().SetHookPoint(pos.nPosX,pos.nPosY);
                return NodeExec_Success;
            }
        }
        return NodeExec_Fail;
    }
    return NodeExec_Success;
}
