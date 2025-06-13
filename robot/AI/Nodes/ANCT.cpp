#include "stdafx.h"
#include <iostream>
#include <sstream>

using namespace std;
 
//是否死亡
AINodeRegister<AINodeIsDeath> AINodeIsDeath::reg(ANCT_IsDeath, "AINodeIsDeath");
NodeExecState AINodeIsDeath::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetActorData().IsDeath())
        return NodeExec_Success;
    return NodeExec_Fail;
}

//是否有可接任务
AINodeRegister<AINodeHasTask> AINodeHasTask::reg(ANCT_HasTask, "AINodeHasTask");
NodeExecState AINodeHasTask::Execute(CRobotClient* pActor, int level, bool isLog)
{
    return NodeExec_Fail;
}

//是否能进行任务
AINodeRegister<AINodeCanTask> AINodeCanTask::reg(ANCT_CanTask, "AINodeCanTask");
NodeExecState AINodeCanTask::Execute(CRobotClient* pActor, int level, bool isLog)
{
    return NodeExec_Fail;
}

//是否还有小飞鞋
AINodeRegister<AINodeHasFlyShoes> AINodeHasFlyShoes::reg(ANCT_HasFlyShoes, "AINodeHasFlyShoes");
NodeExecState AINodeHasFlyShoes::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetActorData().GetProperty<int>(PROP_ACTOR_FLYSHOES) > 0)
        return NodeExec_Success;
    return NodeExec_Fail;
}

//周围是否有怪物
AINodeRegister<AINodeHasMonster> AINodeHasMonster::reg(ANCT_HasMonster, "AINodeHasMonster");
NodeExecState AINodeHasMonster::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetLogicAgent().GetDefaultSystem().GetMonsterCount() > 0)
        return NodeExec_Success;
    return NodeExec_Fail;
}

//能否攻击目标
AINodeRegister<AINodeCanAttack> AINodeCanAttack::reg(ANCT_CanAttack, "AINodeCanAttack");
NodeExecState AINodeCanAttack::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetActorData().IsDeath())
        return NodeExec_Fail;
    CClientEntity* pTarget = pActor->GetLogicAgent().GetTarget();
    if (pTarget == NULL) return NodeExec_Fail;
    if (pTarget->IsDeath()) return NodeExec_Fail;
    EntityHandle handle = pActor->GetLogicAgent().GetTargetHandle();
    int x,y;
    pActor->GetLogicAgent().GetDefaultSystem().GetEntityPos(handle,x,y);
    if (pActor->GetLogicAgent().GetDefaultSystem().GetDistance(x,y) <= 1)
    {
        return NodeExec_Success;
    }
    return NodeExec_Fail;
}

//目标在可攻击范围内
AINodeRegister<AINodeTarInAttkRange> AINodeTarInAttkRange::reg(ANCT_TarInAttkRange, "AINodeTarInAttkRange");
NodeExecState AINodeTarInAttkRange::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTarget = pActor->GetLogicAgent().GetTarget();
    if (pTarget == NULL) return NodeExec_Fail;
    EntityHandle handle = pActor->GetLogicAgent().GetTargetHandle();
    int x,y;
    pActor->GetLogicAgent().GetDefaultSystem().GetEntityPos(handle,x,y);
    if (pActor->GetLogicAgent().GetDefaultSystem().GetMaxDistance(x,y) <= 1)
    {
        return NodeExec_Success;
    }
    return NodeExec_Fail;
}

//(移动)坐标点为目标旁边
AINodeRegister<AINodeMovePointIsNearTar> AINodeMovePointIsNearTar::reg(ANCT_MovePointIsNearTar, "AINodeMovePointIsNearTar");
NodeExecState AINodeMovePointIsNearTar::Execute(CRobotClient* pActor, int level, bool isLog)
{
    CClientEntity* pTarget = pActor->GetLogicAgent().GetTarget();
    if (pTarget == NULL) return NodeExec_Fail;
    EntityHandle handle = pActor->GetLogicAgent().GetTargetHandle();
    int x,y;
    pActor->GetLogicAgent().GetDefaultSystem().GetEntityPos(handle,x,y);
    
    SMALLCPOINT& movePoint = pActor->GetLogicAgent().GetMovePoint();
    
	x -= movePoint.x;
	y -= movePoint.y;
	int nMaxDis = std::max(std::abs(x) , std::abs(y));
    if (nMaxDis <= 1)
    {
        return NodeExec_Success;
    }
    return NodeExec_Fail;
}

//(移动)坐标点为当前位置
AINodeRegister<AINodeMovePointIsPosi> AINodeMovePointIsPosi::reg(ANCT_MovePointIsPosi, "AINodeMovePointIsPosi");
NodeExecState AINodeMovePointIsPosi::Execute(CRobotClient* pActor, int level, bool isLog)
{
    int x,y;
    pActor->GetLogicAgent().GetDefaultSystem().GetPosition(x,y);
    SMALLCPOINT& movePoint = pActor->GetLogicAgent().GetMovePoint();
    
    if ((x == movePoint.x) && (y == movePoint.y))
    {
        return NodeExec_Success;
    }
    return NodeExec_Fail;
}

//(移动)坐标点为可到达的
AINodeRegister<AINodePiCanArrive> AINodePiCanArrive::reg(ANCT_PiCanArrive, "AINodePiCanArrive");
NodeExecState AINodePiCanArrive::Execute(CRobotClient* pActor, int level, bool isLog)
{
    return NodeExec_Success;
}

//能进行下一个动作
AINodeRegister<AINodeCanNextAction> AINodeCanNextAction::reg(ANCT_CanNextAction, "AINodeCanNextAction");
NodeExecState AINodeCanNextAction::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if (pActor->GetLogicAgent().CanAction())
        return NodeExec_Success;
    return NodeExec_Running;
}


