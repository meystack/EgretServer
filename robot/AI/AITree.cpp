#include "stdafx.h"
#include "AITree.h"
 
#include <iostream>
#include <sstream>
 
using namespace std;
 
AINodeHelper * AINodeHelper::m_nodeHlper(NULL);
 
AINodeHelper * AINodeHelper::SharedHelper()
{
    if(m_nodeHlper == NULL)
        m_nodeHlper = new AINodeHelper();
     
    return m_nodeHlper;
}
 
void AINodeHelper::RegisterNodeCreate(int type, BaseNodeCreate create)
{
    m_type2Create[type] = create;
}
 
void AINodeHelper::RegisterNodeName(int type, std::string name)
{
    m_type2Name[type] = name;
}
 
AINodeBase * AINodeHelper::CreateNode(int type)
{
    AINodeBase * nodeBase = NULL;
     
    do
    {
        map<int, BaseNodeCreate>::iterator iter = m_type2Create.find(type);
         
        if(iter == m_type2Create.end())
            break;
         
        nodeBase = (*iter).second();
         
        if(nodeBase == NULL)
            break;
         
        map<int, string>::iterator iter_name = m_type2Name.find(type);
        if(iter_name != m_type2Name.end())
        {
            string & name = (*iter_name).second;
            nodeBase->SetName(name);
        }
    }while(0);
     
    return nodeBase;
}
 
AINodeBase * AINodeHelper::CreateNodeTree(vector<AINodeDescribe> des)
{
    if(des.size() == 0)
        return NULL;
     
    OutputMsg(rmTip, "CreateNodeTree all count = %d", (int)des.size());
     
    map<int, AINodeBase *> m_type2Create;
     
    AINodeBase * rootNode = NULL;
     
    for(vector<AINodeDescribe>::iterator iter = des.begin(); iter != des.end(); ++iter)
    {
        AINodeDescribe &item = (*iter);
        AINodeBase * node = CreateNode(item.AINodeTypeId);
         
        OutputMsg(rmTip, "CreateNodeTree %d", item.AINodeTypeId);
         
        if(node == NULL)
            continue;
         
        if(strlen(item.Describe) != 0)
        {
            node->SetDescribe(item.Describe);
        }
         
        m_type2Create[item.Id] = node;
         
        if(item.ParentId == 0)
        {
            rootNode = node;
        }
        else
        {
            do
            {
                AINodeBase * parentNode = m_type2Create[item.ParentId];
                if(parentNode == NULL)
                    break;
                 
                AIListNode * listParentNode = dynamic_cast<AIListNode *>(parentNode);
                if(listParentNode != NULL)
                {
                    listParentNode->AddChildNode(node);
                    break;
                }
                 
                AISingleNode * singleNode = dynamic_cast<AISingleNode *>(parentNode);
                if(singleNode != NULL)
                {
                    singleNode->SetChildNode(node);
                    break;
                }
                 
            } while (0);
        }
    }
     
    return rootNode;
}
 
void AINodeHelper::printAITree(AINodeBase * node, int level)
{
    ostringstream oss;
     
    for (int i = 0; i < level; ++i)
    {
        oss << "\t";
    }
     
    oss << node->GetDescribe() << " " << node->GetDescribe() << " " << node;
     
    OutputMsg(rmTip, oss.str().c_str());
     
    do
    {
        AIListNode * listNode = dynamic_cast<AIListNode *>(node);
        if(listNode != NULL)
        {
            vector<AINodeBase *> & childs = listNode->GetChildNodes();
            if(childs.size() > 0)
            {
                for (std::vector<AINodeBase *>::iterator i = childs.begin(); i != childs.end(); ++i)
                {
                    printAITree(*i, level + 1);
                }
            }
             
            break;
        }
         
        AISingleNode * singleNode = dynamic_cast<AISingleNode *>(node);
        if(singleNode != NULL)
        {
            AINodeBase * child = singleNode->GetChildNode();
            if(child != NULL)
            {
                printAITree(child, level + 1);
            }
        }
    } while (0);
     
     
}

//最根层节点
void AINodeBase::Destroy()
{
    OutputMsg(rmTip, "Destroy %s %p", GetDescribe().c_str(), this);
    delete this;
}
 
void AINodeBase::SetDescribe(std::string describe)
{
    m_nodeDescribe = describe;
}
 
string AINodeBase::GetDescribe()
{
    return m_nodeDescribe;
}
 
void AINodeBase::SetName(string name)
{
    m_nodeName = name;
}
 
string AINodeBase::GetName()
{
    return m_nodeName;
}
 
//列表节点
void AIListNode::AddChildNode(AINodeBase * node)
{
    m_childNodes.push_back(node);
}
 
std::vector<AINodeBase *> & AIListNode::GetChildNodes()
{
    return m_childNodes;
}
 
void AIListNode::Destroy()
{
    if(m_childNodes.size() > 0)
    {
        for(vector<AINodeBase *>::iterator iter = m_childNodes.begin(); iter != m_childNodes.end(); ++iter)
        {
            (*iter)->Destroy();
        }
    }
     
    AINodeBase::Destroy();
}
 
//单条节点
void AISingleNode::SetChildNode(AINodeBase * node)
{
    if(m_childNode != node)
    {
        if(m_childNode != NULL)
            m_childNode->Destroy();
         
        m_childNode = node;
    }
}
 
AINodeBase * AISingleNode::GetChildNode()
{
    return m_childNode;
}
 
void AISingleNode::Destroy()
{
    if(m_childNode != NULL)
    {
        m_childNode->Destroy();
    }
     
    AINodeBase::Destroy();
}
 
//选择节点
AINodeRegister<AISelectNode> AISelectNode::reg(ANBT_SELECT, "AISelectNode");
 
NodeExecState AISelectNode::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if(isLog)
    {
        ostringstream oss;
        for (int i = 0; i < level; ++i) {
            oss << " ";
        }
        oss << GetDescribe();
        OutputMsg(rmTip, "%s", oss.str().c_str());
    }

    for(vector<AINodeBase *>::iterator iter = m_childNodes.begin(); iter != m_childNodes.end(); ++iter)
    {
        AINodeBase * node = (*iter);
        NodeExecState nRet = node->Execute(pActor, level + 1, isLog);
            
        if(isLog && !node->IsMiddle())
        {
            ostringstream oss;
            for (int i = 0; i < level + 1; ++i) {
                oss << " ";
            }
            oss << node->GetDescribe() << " Execute " << nRet;
            OutputMsg(rmTip, "%s", oss.str().c_str());
        }

        switch (nRet)
        {
        case NodeExec_Success:
        case NodeExec_Running:
            return nRet;
        }
    }
    return NodeExec_Fail;
}
 
//顺序节点
AINodeRegister<AISequenceNode> AISequenceNode::reg(ANBT_SEQUENCE, "AISequenceNode");
 
NodeExecState AISequenceNode::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if(isLog)
    {
        ostringstream oss;
        for (int i = 0; i < level; ++i) {
            oss << " ";
        }
        oss << GetDescribe();
        OutputMsg(rmTip, "%s", oss.str().c_str());
    }

    for(vector<AINodeBase *>::iterator iter = m_childNodes.begin(); iter != m_childNodes.end(); ++iter)
    {
        AINodeBase * node = (*iter);
        NodeExecState nRet = node->Execute(pActor, level + 1, isLog);
            
        if(isLog && !node->IsMiddle())
        {
            ostringstream oss;
            for (int i = 0; i < level + 1; ++i) {
                oss << " ";
            }
            oss << node->GetDescribe() << " Execute " << nRet;
            OutputMsg(rmTip, "%s", oss.str().c_str());
        }

        switch (nRet)
        {
        case NodeExec_Fail:
        case NodeExec_Running:
            return nRet;
        }
    }
    return NodeExec_Success;
}
 
//并列节点
AINodeRegister<AIParallelNode> AIParallelNode::reg(ANBT_PARALLEL, "AIParallelNode");

NodeExecState AIParallelNode::Execute(CRobotClient* pActor, int level, bool isLog)
{
    if(isLog)
    {
        ostringstream oss;
        for (int i = 0; i < level; ++i) {
            oss << " ";
        }
        oss << GetDescribe();
        OutputMsg(rmTip, "%s", oss.str().c_str());
    }

    int nRetCnt[3] = {0};
    for(vector<AINodeBase *>::iterator iter = m_childNodes.begin(); iter != m_childNodes.end(); ++iter)
    {
        AINodeBase * node = (*iter);
        NodeExecState nRet = node->Execute(pActor, level + 1, isLog);
            
        if(isLog && !node->IsMiddle())
        {
            ostringstream oss;
            for (int i = 0; i < level + 1; ++i) {
                oss << " ";
            }
            oss << node->GetDescribe() << " Execute " << nRet;
            OutputMsg(rmTip, "%s", oss.str().c_str());
        }

        if (nRet == NodeExec_Running)
				return NodeExec_Running;
		++ nRetCnt[nRet];
    }

    //全部succ，返回succ，否则fail
    return (nRetCnt[NodeExec_Fail] == 0) ? NodeExec_Success : NodeExec_Fail;
}

//取反节点
AINodeRegister<AINotNode> AINotNode::reg(ANBT_NOT, "AINotNode");
 
NodeExecState AINotNode::Execute(CRobotClient* pActor, int level, bool isLog)
{
    NodeExecState nRet = (m_childNode->Execute(pActor, level + 1, isLog));
    
    if(isLog)
    {
        ostringstream oss;
        for (int i = 0; i < level + 1; ++i) {
            oss << " ";
        }
        if(m_childNode != NULL)
        {
            oss << m_childNode->GetDescribe() << " Execute " << !nRet;
        }
        else
        {
            oss << "no child";
        }
        OutputMsg(rmTip, "%s", oss.str().c_str());
    }

    switch (nRet)
    {
    case NodeExec_Fail:
        return NodeExec_Success;
    case NodeExec_Success:
        return NodeExec_Fail;
    case NodeExec_Running:
        return NodeExec_Running;
    }
    assert(false);
    return NodeExec_Fail;
}
