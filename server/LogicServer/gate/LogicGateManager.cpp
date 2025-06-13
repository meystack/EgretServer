#include "StdAfx.h"
#include "LogicGateManager.h"

CLogicGateManager::CLogicGateManager(CLogicServer* lpLogicServer)
	:Inherited()
{
	m_pLogicServer = lpLogicServer;
}

CLogicGateManager::~CLogicGateManager()
{

}

VOID CLogicGateManager::Initialize()
{
	int i;
	for ( i=0; i<ArrayCount(m_Gates); ++i )
	{
		m_Gates[i].m_pLogicServer = m_pLogicServer;

		//将网关添加到列表
		m_GateList[i] = &m_Gates[i];
		m_Gates[i].SetGateIndex(i); //设置网关编号，避免重复查找

	}
}

VOID CLogicGateManager::Uninitialize()
{
	int i;
	for ( i=0; i<ArrayCount(m_GateList); ++i )
	{
		//将网关从列表移除
		m_GateList[i] = NULL;
	}
}

BOOL CLogicGateManager::Startup()
{
	return Inherited::Startup();
}

VOID CLogicGateManager::Stop()
{
	Inherited::Stop();
}

VOID CLogicGateManager::OnResultGateUserExists(int nSessionId, const BOOL boOnline)
{
	CLogicSSClient * pSessionClient = m_pLogicServer->GetSessionClient();
	if (pSessionClient)
	{
		pSessionClient->PostQuerySessionExistsResult(nSessionId,boOnline?1:0);
	}
}

void CLogicGateManager::CloseAllGates()
{
	for (INT_PTR i=0;i <MaxGateCount; i++ )
	{
		if( m_Gates[i].connected()) //如果这个数据生效的
		{
			//m_Gates[i].PostInternalMessage(SSM_KICK_USER,uParam1,uParam2,0,0);
			m_Gates[i].PostCloseGateServer();
		}
	}
}

void CLogicGateManager::KickUser(ACCOUNTDATATYPE accountId,UINT64 lKey )
{
	PostInternalMessage(SGIM_POST_KICK_USER,accountId,lKey,0,0);
}


//刷一个车
VOID CLogicGateManager::DispatchInternalMessage(UINT uMsg, UINT64 uParam1, UINT64 uParam2, UINT64 uParam3,UINT64 uParam4 )
{	
	//是使用buff数据进来的，需要处理数据包
	if(uMsg ==SGIM_RECV_LOGIN_DATA)
	{
		int nGateIndex=0;
		
		CDataPacket* pDataPacket = (CDataPacket*)uParam2;
		if ( pDataPacket )
		{			
			//会话服务器发来的消息
			if(uParam1 == jxInterSrvComm::SessionServerProto::sCheckPasswdResult) //session发来的检测密码的
			{
				int  nGateIndex= (int)uParam3; //先读取网关的编号
				if(nGateIndex >=0 && nGateIndex < MaxGateCount ) //判断是否是合法的网关
				{
					if( m_Gates[nGateIndex].connected()) //如果这个数据生效的
					{
						m_Gates[nGateIndex].PostInternalMessage(SSM_RECV_LOGIN_DATA,uParam1,uParam2,0,0);
					}
					else
					{
						GetLogicServer()->GetSessionClient()->FreeBackUserDataPacket(pDataPacket);
					}
				}
				
			}	//数据服务器发来的消息
			else if(uParam1 == jxInterSrvComm::DbServerProto::dcGetActorList 
				|| uParam1 == jxInterSrvComm::DbServerProto::dcCreateActor
				|| uParam1 == jxInterSrvComm::DbServerProto::dcDeleteActor
				|| uParam1 == jxInterSrvComm::DbServerProto::dcGetRandName
				|| uParam1 == jxInterSrvComm::DbServerProto::dcCheckSecondPsw
				|| uParam1 == jxInterSrvComm::DbServerProto::dcCreateSecondPsw
				  )
			{
				int  nGateIndex= (int)uParam3; //先读取网关的编号
				if(nGateIndex >=0 && nGateIndex < MaxGateCount ) //判断是否是合法的网关
				{
					if( m_Gates[nGateIndex].connected()) //如果这个数据生效的
					{
						m_Gates[nGateIndex].PostInternalMessage(SSM_RECV_LOGIN_DATA,uParam1,uParam2,0,0);
					}
					else
					{
						GetLogicServer()->GetDbClient()->FreeBackUserDataPacket(pDataPacket);
					}
				}
			}
			//会话服务器发来的消息
			else if(uParam1 == jxInterSrvComm::SessionServerProto::sSimulateCheckPasswdResult) //session发来的检测密码的
			{
				int  nGateIndex= (int)uParam3; //先读取网关的编号
				if(nGateIndex >=0 && nGateIndex < MaxGateCount ) //判断是否是合法的网关
				{
					if( m_Gates[nGateIndex].connected()) //如果这个数据生效的
					{
						m_Gates[nGateIndex].PostInternalMessage(SSM_RECV_LOGIN_DATA,uParam1,uParam2,0,uParam4);
					}

				}
				
			}
		}
		
	}
	else if(uMsg ==SGIM_RECV_LOGIN_CMD) //是使用原始的数据进来的
	{
		//登陆的结构
		if(uParam1 == jxInterSrvComm::SessionServerProto::sCheckPasswdResult) 
		{
			int nGateIndex = (int)uParam2;
			if(nGateIndex >=0 && nGateIndex < MaxGateCount ) //判断是否是合法的网关
			{
				//使用内部消息传过去
				if( m_Gates[nGateIndex].connected()) //如果这个数据生效的
				{
					m_Gates[nGateIndex].PostInternalMessage(SSM_RECV_LOGIN_CMD,uParam1,uParam3,uParam4,0);
				}
			}
		}
	}
	else if(uMsg ==SGIM_POST_KICK_USER) //需要踢人了
	{
		for (INT_PTR i=0;i <MaxGateCount; i++ )
		{
			if( m_Gates[i].connected()) //如果这个数据生效的
			{
				//m_Gates[i].PostInternalMessage(SSM_KICK_USER,uParam1,uParam2,0,0);
				m_Gates[i].PostKickUser((unsigned int)uParam1,uParam2);
			}
		}
	}
	else	
	{
		Inherited::DispatchInternalMessage( uMsg,  uParam1,  uParam2, uParam3,uParam4 );
	}
}