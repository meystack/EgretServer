
//请求回城复活
class AINodeReturnRelive : public AINodeBase
{
private:
    static AINodeRegister<AINodeReturnRelive> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//请求元宝复活
class AINodeYBRelive : public AINodeBase
{
private:
    static AINodeRegister<AINodeYBRelive> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//往 (移动)坐标点 移动一步
class AINodeMovePoint : public AINodeBase
{
private:
    static AINodeRegister<AINodeMovePoint> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//设置目标 - 怪物
class AINodeSetTarMonster : public AINodeBase
{
private:
    static AINodeRegister<AINodeSetTarMonster> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//设置目标 - 物品
class AINodeSetTarItem : public AINodeBase
{
private:
    static AINodeRegister<AINodeSetTarItem> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//设置(移动)坐标点 - 挂机点（并寻路）
class AINodeSetMovePointAsHook : public AINodeBase
{
private:
    static AINodeRegister<AINodeSetMovePointAsHook> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//设置(移动)坐标点 - 目标旁（并寻路） 
class AINodeSetMovePointAsTar : public AINodeBase
{
private:
    static AINodeRegister<AINodeSetMovePointAsTar> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//攻击目标
class AINodeAttackTarget : public AINodeBase
{
private:
    static AINodeRegister<AINodeAttackTarget> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//改变目标 - 怪物（非本目标） 
class AINodeChangeTarMonster : public AINodeBase
{
private:
    static AINodeRegister<AINodeChangeTarMonster> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//设置挂机点
class AINodeSetHook : public AINodeBase
{
private:
    static AINodeRegister<AINodeSetHook> reg;

public:
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};
