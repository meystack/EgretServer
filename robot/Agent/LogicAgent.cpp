#pragma once

#include "stdafx.h"
/*
* 与逻辑服务器通信的类，处理逻辑服务器的各种消息
* ！！！请注意，发送的包一定要在OnEnterGame 以后调用，因为有一个校验密钥的过程，如果前面发数据，可能导致校验密钥失败
*/

CLogicAgent::CLogicAgent()
{
	m_bHasTrans = false;
	m_NextActionTick = 0;
}

CLogicAgent::~CLogicAgent()
{

}

void CLogicAgent::OnConnected()
{
	SendCreateActor();
	SendLogin();
}

void CLogicAgent::OnDisConnected()
{
	//m_pClient->
	OutputMsg(rmNormal, "[%s]disconnect from logic ",m_pClient->GetAccount().name);
	DiscardUnHandledMsg();
	m_hasEnterGame =false; 
}

void CLogicAgent::OnTimeRun(TICKCOUNT tick)
{
	// 处理来自服务器的消息
	ProcessNetMsg();

	if( m_hasEnterGame ==false) return ;  //如果没有登录游戏，不执行定时调用

	// 子系统更新处理
	m_defaultSystem.OnTimeRun(tick); //定期的调用每个子系统
	m_skillSystem.OnTimeRun(tick);
	GetRobotMgr()->GetGlobalConfig().GetAIBehavierTree().Run(m_pClient,tick);
}

void CLogicAgent::OnEvent(INT_PTR nEvtId, INT_PTR nParam1, INT_PTR nParam2, INT_PTR nParam3, INT_PTR nParam4)
{
	//m_ActionMgr.OnEvent(nEvtId, nParam1, nParam2, nParam3, nParam4);
}

bool CLogicAgent::ProcessNetMsg()
{
	m_netMsgList.flush();
	INT_PTR nMsgCount = m_netMsgList.count();
	for (INT_PTR i = 0; i < nMsgCount; i++)
	{
		BYTE ucSysId = 0, ucCmd = 0;
		CDataPacket& curPacket = *m_netMsgList[0];
		curPacket >> ucSysId >> ucCmd;
		DispatchMsg(ucSysId, ucCmd, curPacket);
		m_netMsgList.remove(0);
		FreePacket(&curPacket);
	}
	return true;
}

//发送数据包进入游戏
void CLogicAgent::SendLogin()
{
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) enLoginSystemId << (BYTE)cCheckUserPasswd;
	LOGINACCOUNT & account = m_pClient->GetAccount(); //获取账户
	pack.writeString(account.name);

	MD5_CTX   ctx; 
	unsigned char md5[33];

	//使用 MD5加密玩家的密码
	MD5Init(&ctx); 
	MD5Update(&ctx,  (unsigned char *)account.pass,   (unsigned int)strlen(account.pass) );
	MD5Final(md5,&ctx);
	md5[32]=0;

	pack.writeString((const  char*)md5);
	pack <<  GetRobotMgr()->GetServerIndex();
	pack <<  GetRobotMgr()->GetServerIndex();

	m_pClient->FlushProtoPacket(pack);
}

//发送数据包创建一个角色
void CLogicAgent::SendCreateActor()
{
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack <<  (BYTE) enLoginSystemId  << (BYTE) cCreateAccount; 

	LOGINACCOUNT & account = m_pClient->GetAccount(); //获取账户

	pack.writeString(account.name);

	MD5_CTX   ctx; 
	unsigned char md5[33];
	//使用 MD5加密玩家的密码
	MD5Init(&ctx); 
	MD5Update(&ctx,  (unsigned char *)account.pass,   (unsigned int)strlen(account.pass) );
	MD5Final(md5,&ctx);
	md5[32]=0;
	pack.writeString((const  char*)md5);
	pack.writeString("430481188112113256"); //乱设一个身份证号码进去
	m_pClient->FlushProtoPacket(pack);
}

void CLogicAgent::DiscardUnHandledMsg()
{
	FreePacket(m_netMsgList);
	m_netMsgList.clear();
}

void CLogicAgent::DispatchMsg(BYTE ucSysId, BYTE ucMsgId, CDataPacketReader& packet)
{
	switch (ucSysId)
	{
	case enLoginSystemId:
		OnRecvLoginMsg(ucMsgId, packet);
		break;
	case enDefaultEntitySystemID:
		m_defaultSystem.OnRecvData(ucMsgId, packet);
		break;
	case enMoveSystemID:
		break;
	case enSkillSystemID:
		m_skillSystem.OnRecvData(ucMsgId, packet);
		break;
	case enBagSystemID:
		//m_itemSystem.OnRecvData(ucMsgId, packet);
		break;
	case enEuipSystemID:
		//m_equipSystem.OnRecvData(ucMsgId, packet);
		break;

	default:
		break;
	}
}

char * CLogicAgent::GetLoginErrorStr(INT_PTR nErrorCode)
{
	switch (nErrorCode)
	{
	case ERR_SQL: //sql错误
		return "sql Error";
		break;
	case ERR_SESS: //用户没登陆
		return "No session Data send from session server";
		break;
	case ERR_GAMESER: //游戏服务没准备好
		return "server not ready";
		break;
	case ERR_DATASAVE: //角色上一次保存数据是否出现异常
		return "User last save data error,forbid login";
		break;
	case ERR_SELACTOR://客户端选择角色的常规错误
		return "client select error";
		break;
	case ERR_NOGATE://客户端无匹配的路由数据的错误
		return "No gate found";
		break;
	case ERR_NOUSER: //角色不存在
		return "No user exist";
		break;
	case ERR_SEX://错误的性别
		return "sex error";
		break;
	case ERR_NORANDOMNAME://随机生成的名字已经分配完
		return "Rand name database is empty";
		break;
	case ERR_ZY://客户端上传的角色阵营参数错误
		return "client camp error";
		break;
	case ERR_JOB: //客户端上传的角色职业参数错误
		return "select job error";
		break;
	default:
		return " Unkown error";
	}
}

//发送数据包去查询角色
void CLogicAgent::SendQueryActorList()
{
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) enLoginSystemId <<(BYTE )cGetUserList; //进入游戏的请求
	pack << (int) m_pClient->GetAccountId(); //获取账户的ID
	m_pClient->FlushProtoPacket(pack);
	
}

//选择角色进入游戏
void CLogicAgent::SelActorEnterGame(unsigned int nActorID,char * sName )
{
	
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) enDefaultEntitySystemID <<(BYTE )cLogin; //进入游戏的请求
	pack <<(unsigned int)m_pClient->GetAccountId();
	pack << (unsigned int) nActorID; //玩家的角色
	m_pClient->FlushProtoPacket(pack); //发送消息进入游戏

	//设置名字和actorid
	m_pClient->SetActorId(nActorID);
	if(sName != NULL)
	{
		m_pClient->SetActorName(sName);
	}
}

//随机一个名字
void CLogicAgent::RandActorName()
{
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) enLoginSystemId <<(BYTE )cRandName; //进入游戏的请求
	char sex =rand() %2; //性别

	pack <<  sex; //玩家的角色
	m_pClient->FlushProtoPacket(pack); //发送消息进入游戏
}

void CLogicAgent::CreateActor(char * sName,int nSex,int nJob, int nCamp )
{
	CDataPacket &pack= m_pClient->AllocProtoPacket();
	pack << (BYTE) enLoginSystemId <<(BYTE )cCreateActor; //进入游戏的请求
	pack.writeString(sName);
	pack << (BYTE) nSex << (BYTE) nJob << (BYTE) 0 << (BYTE)nCamp;
	m_pClient->FlushProtoPacket(pack); //发送消息进入游戏

}


//收到登陆的消息包
void CLogicAgent::OnRecvLoginMsg(BYTE ucMsgId, CDataPacketReader & pack)
{
	unsigned int nAccountId=0,nActorId =0;
	if(sLoginErrorCode ==ucMsgId)
	{
		char bResult =0;
	
		pack >> bResult >> nAccountId;
		if(bResult !=0)
		{
			OutputMsg(rmError, "[%s]login  on session fail, errorid=%d",m_pClient->GetAccount().name,int(bResult));
			return;
		}
		else
		{
			//OutputMsg(rmNormal, "[%s]login  on session ",m_pClient->GetAccount().name);
		}
		
	}
	else if(sGetUserList ==ucMsgId )
	{
		
		BYTE nErrorCode =0;
		int nOpenDay = 0;
		pack >> nOpenDay;
		BYTE canCreate = 0;
		pack >> canCreate;
		pack >> nAccountId >> nErrorCode;

		if(nErrorCode >=100)
		{
			OutputMsg(rmError, "[%slogin  on get user List, errorid=%d",m_pClient->GetAccount().name, GetLoginErrorStr(nErrorCode));
			return ;
		}
		m_pClient->SetAccountId(nAccountId);//设置账户的ID
		if(nErrorCode ==0) //如果只有一个角色，则需要创建一个角色
		{
			RandActorName(); //随机一个名字
		}
		else
		{

			pack >> nActorId; //进入游戏
			char name[32];
			pack.readString(name,sizeof(name));
			SelActorEnterGame(nActorId,name);
		}
	}
	else if(sCreateActor == ucMsgId)
	{

		BYTE bResult =0;
		pack >> nActorId >> bResult; //创建角色的结构
		if(bResult ==0)
		{
			SelActorEnterGame(nActorId);
		}
		else
		{
			OutputMsg(rmWaning,_T("[%s] Create actor fail, errorcode=%s"), m_pClient->GetAccount().name,GetLoginErrorStr((int)bResult));
			//RandActorName(); //重新随一个账户
		}
	}
	else if(sRandNameActor == ucMsgId)
	{
		char bRet =0, bSex=0;
		pack >> bRet >> bSex;
		char sName[32]; //自动产生的名字
		pack.readString(sName,sizeof(sName)); //读取名字
		//随机阵营
		int nCamp = 0;  //随机阵营
		int nJob ;

		//随机一个职业
		nJob =  rand()% 3+1 ;
		CreateActor(sName,bSex,nJob,nCamp); //随机一个阵营选择进入游戏
		m_pClient->SetActorName(sName);
	}
}

void CLogicAgent::OnRecv(CDataPacketReader & pack)
{	
	CDataPacket *packet = AllocPacket();
	packet->writeBuf(pack.getOffsetPtr(), pack.getAvaliableLength());
	packet->setPosition(0);
	m_netMsgList.append(packet);	
}

CDataPacket* CLogicAgent::AllocPacket()
{
	static const INT_PTR SINGLE_ALLOC_COUNT = 100;
	CDataPacket* packet;
	if (m_freeMsgList.count() > 0)
	{
		packet = m_freeMsgList.pop();
	}
	else
	{
		m_freeMsgList.flush();
		if (m_freeMsgList.count() <= 0)
		{
			CDataPacket* pkg = (CDataPacket*)m_allocator.AllocBuffer(sizeof(CDataPacket) * SINGLE_ALLOC_COUNT);
			for (INT_PTR i = 0; i < SINGLE_ALLOC_COUNT; i++)
			{
				CDataPacket* curPkg = pkg + i;
				new(curPkg) CDataPacket(&m_allocator);
				m_freeMsgList.add(curPkg);
			}
			
		}
		packet = m_freeMsgList.pop();
	}

	packet->setPosition(0);
	return packet;
}

void CLogicAgent::FreePacket(CDataPacket* packet)
{
	m_freeMsgList.append(packet);
}

void CLogicAgent::FreePacket(CBaseList<CDataPacket*> &list)
{
	m_freeMsgList.appendList(list);
}

CClientEntity* CLogicAgent::GetTarget()
{
	INT_PTR nType = m_TargetHandle.GetType();
	if (enMonster == nType)
		return m_defaultSystem.GetMonster(m_TargetHandle);
	else if (enActor == nType)
		return m_defaultSystem.GetOtherData(m_TargetHandle);
	else if (enPet == nType)
		return m_defaultSystem.GetPet(m_TargetHandle);
	return NULL;
}

bool CLogicAgent::CanCrossCheck(INT_PTR x, INT_PTR y)
{
	return !m_defaultSystem.HasEntity(x,y);
}

bool CLogicAgent::CalcPath(bool isLoopLimit)
{
    int curX,curY;
    GetDefaultSystem().GetPosition(curX,curY);
	
	int nSceneId = m_pClient->GetActorData().GetSceneId();
	SCENEMAPCONF * pSc = GetRobotMgr()->GetGlobalConfig().GetSceneConfig().GetSceneConfig(nSceneId);
	if (pSc != NULL)
	{
		typedef typename AStar::CrossCheckFuncType FuncType;
		FuncType checkproxy = std::bind(&CLogicAgent::CanCrossCheck,this,std::placeholders::_1,std::placeholders::_2);
        return m_Astar.getPath(curX,curY,m_MovePoint.x,m_MovePoint.y,m_searchNodes,checkproxy,isLoopLimit);
    }
}

void CLogicAgent::SetNextAction(int millsec)
{
	m_NextActionTick = m_pClient->GetCurTick() + millsec;
}

bool CLogicAgent::CanAction()
{
	return m_NextActionTick <= m_pClient->GetCurTick();
}

int CLogicAgent::GetAndPopNextMovePosi(int& x,int& y,int& dir)
{
	int count = m_searchNodes.size();
	if (count < 2) return 0;

	SMALLCPOINT pos = m_searchNodes[count - 1];
	x = pos.x;
	y = pos.y;
	pos = m_searchNodes[count - 2];
	dir = pos.dir;

	if (count > 2)
	{
		CClientEntity* pentity = GetTarget();//第一个点 自己， 最后一个点 目标
		int tarX,tarY;
		GetDefaultSystem().GetEntityPos(m_TargetHandle, tarX,tarY);
		SMALLCPOINT pos2 = m_searchNodes[count - 3];
		if (pos2.dir == pos.dir && ((pos2.x != tarX) || (pos2.y != tarY)))
		{
			m_searchNodes.pop_back();
			m_searchNodes.pop_back();
			return 2;
		}
	}
	m_searchNodes.pop_back();
	return 1;
}
