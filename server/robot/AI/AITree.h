#pragma once
 
#include <stdio.h>
#include <vector>
#include <map>
#include <string>
 
#pragma mark =============== Helper ==================
 
class CRobotClient;
class AINodeBase;

typedef AINodeBase * (* BaseNodeCreate)();

struct AINodeDescribe
{
    AINodeDescribe()
    {
        memset(this, 0, sizeof(AINodeDescribe));
    }

    AINodeDescribe(int id, int pId, int typeId, char * describe = NULL)
    :Id(id)
    ,ParentId(pId)
    ,AINodeTypeId(typeId)
    {
        memset(Describe, 0, sizeof(Describe));
         
        if(describe != NULL && strlen(describe) < sizeof(Describe) / sizeof(char))
        {
            strcpy(Describe, describe);
        }
    }
     
    int Id;             //当期节点Id
    int ParentId;       //父节点Id
    int AINodeTypeId;   //智能节点类型Id
    char Describe[256];   //节点名称
};

class AINodeHelper
{
private:
    static AINodeHelper * m_nodeHlper;
     
    std::map<int, BaseNodeCreate> m_type2Create;
    std::map<int, std::string> m_type2Name;
     
public:
    static AINodeHelper * SharedHelper();
     
    void RegisterNodeCreate(int type, BaseNodeCreate create);
    void RegisterNodeName(int type, std::string name);
     
    AINodeBase * CreateNode(int type);          //创建节点
     
    AINodeBase * CreateNodeTree(std::vector<AINodeDescribe> des);
     
    void printAITree(AINodeBase * node, int level = 0);
};

template <class T>
class AINodeRegister
{
public:
    static AINodeBase * CreateT()
    {
        return new T();
    }
     
    AINodeRegister(int type, std::string name = "")
    {
        AINodeHelper * helper = AINodeHelper::SharedHelper();
         
        helper->RegisterNodeCreate(type, &AINodeRegister::CreateT);
         
        if(name != "")
            helper->RegisterNodeName(type, name);
    }
};

#pragma mark ================== 枚举 =================

enum AINodeExecResult
{
    NodeExec_Success,
    NodeExec_Fail,
    NodeExec_Running
};

typedef int NodeExecState;

// 注意：以下枚举，必须要同步到 AITree.lua 配置中

enum AINodeBaseType
{
    ANBT_SELECT=1,        //选择节点
    ANBT_SEQUENCE,      //顺序节点
    ANBT_PARALLEL,      //并列节点
    ANBT_NOT,           //取反节点
    ANBT_FINAL = ANBT_NOT
};

enum AINodeCondType
{
    ANCT_IsDeath = ANBT_FINAL + 1,       //是否死亡
    ANCT_HasTask = ANBT_FINAL + 2,       //是否有可接任务
    ANCT_CanTask = ANBT_FINAL + 3,       //是否能进行任务
    ANCT_HasFlyShoes = ANBT_FINAL + 4,   //是否还有小飞鞋
    ANCT_HasMonster = ANBT_FINAL + 5,    //周围是否有怪物
    ANCT_CanAttack = ANBT_FINAL+6,       //能否攻击目标
    ANCT_TarInAttkRange = ANBT_FINAL+7,  //目标在可攻击范围内
    ANCT_MovePointIsNearTar = ANBT_FINAL+8,      //(移动)坐标点为目标旁边
    ANCT_MovePointIsPosi = ANBT_FINAL+9,         //(移动)坐标点为当前位置
    ANCT_PiCanArrive = ANBT_FINAL+10,     //(移动)坐标点为可到达的
    ANCT_CanNextAction = ANBT_FINAL+11,   //能进行下一个动作
    ANCT_FINAL = ANCT_CanNextAction
};

enum AINodeFuncType
{
    ANFT_ReturnRelive = ANCT_FINAL + 1,  //请求回城复活
    ANFT_YBRelive = ANCT_FINAL + 2,      //请求元宝复活
    ANFT_MovePoint = ANCT_FINAL + 3,     //往 (移动)坐标点 移动一步
    ANFT_SetTarMonster = ANCT_FINAL + 4, //设置目标 - 怪物（如果目标死亡或不存在了）
    ANFT_SetTarItem = ANCT_FINAL + 5,    //设置目标 - 物品
    ANFT_SetMovePointAsHook = ANCT_FINAL + 6,     //设置(移动)坐标点 - 挂机点（并寻路）
    ANFT_SetMovePointAsTar = ANCT_FINAL + 7,      //设置(移动)坐标点 - 目标旁（并寻路）
    ANFT_AttackTarget = ANCT_FINAL + 8,  //攻击目标
    ANFT_ChangeTarMonster = ANCT_FINAL+9, //改变目标 - 怪物（非本目标）
    ANFT_SetHook = ANCT_FINAL+10,          //设置挂机点
};

class AINodeBase
{
protected:
    std::string m_nodeName;
    std::string m_nodeDescribe;
     
public:
    AINodeBase()
    :m_nodeName("AINodeBase")
    ,m_nodeDescribe("")
    {
    }
     
    virtual ~AINodeBase() { }
     
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false) { return NodeExec_Fail; }
     
    virtual void Destroy();

    virtual bool IsMiddle(){ return false; }
     
    virtual void SetDescribe(std::string describe);
    virtual std::string GetDescribe();
     
    virtual void SetName(std::string name);
    virtual std::string GetName();
};

#pragma mark ================== 复合节点 ==================

//多条节点
class AIListNode : public AINodeBase
{
protected:
    std::vector<AINodeBase *> m_childNodes;
     
public:
    virtual void AddChildNode(AINodeBase * node);
    virtual std::vector<AINodeBase *> & GetChildNodes();
    virtual void Destroy();
};

//选择节点
class AISelectNode : public AIListNode
{
private:
    static AINodeRegister<AISelectNode> reg;
     
public:
    virtual bool IsMiddle(){ return true; }
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//顺序节点
class AISequenceNode : public AIListNode
{
private:
    static AINodeRegister<AISequenceNode> reg;
     
public:
    virtual bool IsMiddle(){ return true; }
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

//并列节点
class AIParallelNode : public AIListNode
{
private:
    static AINodeRegister<AIParallelNode> reg;
     
public:
    virtual bool IsMiddle(){ return true; }
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

#pragma mark ================== 修饰节点 ==================

//单条节点
class AISingleNode : public AINodeBase
{
protected:
    AINodeBase * m_childNode;
     
public:
    AISingleNode()
    :m_childNode(NULL)
    { }
     
    virtual void SetChildNode(AINodeBase * node);
    virtual AINodeBase * GetChildNode();
    virtual void Destroy();
};

//取反节点
class AINotNode : public AISingleNode
{
private:
    static AINodeRegister<AINotNode> reg;
     
public:
    virtual bool IsMiddle(){ return true; }
    virtual NodeExecState Execute(CRobotClient* pActor, int level = 0, bool isLog = false);
};

#pragma mark ================== 叶子节点 ==================

#include "Nodes/ANCT.h"
#include "Nodes/ANFT.h"
