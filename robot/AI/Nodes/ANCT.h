
//是否死亡
class AINodeIsDeath : public AINodeBase
{
private:
    static AINodeRegister<AINodeIsDeath> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//是否有可接任务
class AINodeHasTask : public AINodeBase
{
private:
    static AINodeRegister<AINodeHasTask> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//是否能进行任务
class  AINodeCanTask: public AINodeBase
{
private:
    static AINodeRegister<AINodeCanTask> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//是否还有小飞鞋
class AINodeHasFlyShoes : public AINodeBase
{
private:
    static AINodeRegister<AINodeHasFlyShoes> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//周围是否有怪物
class AINodeHasMonster : public AINodeBase
{
private:
    static AINodeRegister<AINodeHasMonster> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//能否攻击目标
class AINodeCanAttack : public AINodeBase
{
private:
    static AINodeRegister<AINodeCanAttack> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//目标在可攻击范围内
class AINodeTarInAttkRange : public AINodeBase
{
private:
    static AINodeRegister<AINodeTarInAttkRange> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//(移动)坐标点为目标旁边
class AINodeMovePointIsNearTar : public AINodeBase
{
private:
    static AINodeRegister<AINodeMovePointIsNearTar> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//(移动)坐标点为当前位置
class AINodeMovePointIsPosi : public AINodeBase
{
private:
    static AINodeRegister<AINodeMovePointIsPosi> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//(移动)坐标点为可到达的
class AINodePiCanArrive : public AINodeBase
{
private:
    static AINodeRegister<AINodePiCanArrive> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//能进行下一个动作
class AINodeCanNextAction : public AINodeBase
{
private:
    static AINodeRegister<AINodeCanNextAction> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};
