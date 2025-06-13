#include "StdAfx.h"
#include "MovementSystem.h"


CAdvanceAstar * CMovementSystem::s_pAStar = NULL;
bool CMovementSystem::s_boCheckIdleLowPly = false;
byte CMovementSystem::s_btMinIdlePlyLevel = 2;

template<>
const CMovementSystem::Inherid::OnHandleSockPacket CMovementSystem::Inherid::Handlers[]=
{
	&CMovementSystem::HandError,    //0,移动
	&CMovementSystem::HandMove,     //1,走
	&CMovementSystem::HandRun,      //2,跑
	&CMovementSystem::HandJump,     //3,跳跃
	&CMovementSystem::HandTurn,		//4,转向
	&CMovementSystem::HandFlyShoes,	//5，用小飞鞋
};

//处理网络消息
void  CMovementSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;
	if(nCmd >=0 && nCmd <ArrayCount(CMovementSystem::Inherid::Handlers))
	{
		(this->*Handlers[nCmd] )(packet);
		m_tmIdleLowPly.SetNextHitTimeFromNow(MOVECHECKTIMER);
	}
}



void CMovementSystem::SendHandFlyShoesRelust()
{
	if(m_pEntity && nFlyShoeNpcId > 0 ) {
		CActorPacket pack;
		CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
		data << (BYTE)enMoveSystemID << (BYTE)sFlyShoes ;
		data << (WORD) nFlyShoeNpcId;
		pack.flush();
		nFlyShoeNpcId = 0;
	}
}

//收到飞鞋包，进行处理
void  CMovementSystem::HandFlyShoes(CDataPacketReader &packet)
{
	
	WORD dest1;
	WORD dest2;
	if(!m_pEntity) return;
	
	packet  >> dest1 >>dest2 ;

	if(m_pEntity->GetType() ==enActor)
	{	//获取配置
		CFlyShoes & flyshoes = GetLogicServer()->GetDataProvider()->GetFlyShoesConfig();

       if( !( ((CActor*)m_pEntity)->CheckLevel( flyshoes.m_ButtonTable[dest2].nOpenLevel, flyshoes.m_ButtonTable[dest2].nOpenCircle)\
	    && (GetLogicServer()->GetDaysSinceOpenServer()>=flyshoes.m_ButtonTable[dest2].nOpenDay)))  
		{
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmFlyShoesError, tstUI);
			return ;
       }//等级，转身，开服天数不够
        
		int nSourceValue = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_FLYSHOES);
		if(nSourceValue < flyshoes.m_Index[dest1].nCost )
		{
		
				((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmflyshoesNotEnough, tstUI);
			return ;
		}//小飞鞋不够

		//充值元宝限制
		int needYuanBao = flyshoes.m_ButtonTable[dest2].nMoney ;
		int myYuanBao = ((CActor*)m_pEntity)->GetProperty<unsigned int>(PROP_ACTOR_DRAW_YB_COUNT);
		if(needYuanBao > myYuanBao  )
		{
			((CActor*)m_pEntity)->SendTipmsgFormatWithId(tmFXTransportYuanBaoNotEnough, tstUI);
			return ;
		}
		
		//LPCSTR sFlyshoes = "flyshoes_transmit";
		//((CActor *)pActor)->ChangeFlyShoes(-contition.nValue,GameLog::clUserItem,0,sFlyshoes,true);

		// int point_x = flyshoes.m_ButtonTable[dest2].nPointRange[0]+flyshoes.m_ButtonTable[dest2].nPointRange[2]+\
		// 		flyshoes.m_ButtonTable[dest2].nPointRange[4]+flyshoes.m_ButtonTable[dest2].nPointRange[6];
		// int point_y = flyshoes.m_ButtonTable[dest2].nPointRange[1]+flyshoes.m_ButtonTable[dest2].nPointRange[3]+\
		// 		flyshoes.m_ButtonTable[dest2].nPointRange[5]+flyshoes.m_ButtonTable[dest2].nPointRange[7];
		// INT_PTR nWeight = flyshoes.m_ButtonTable[dest2].nPointRange[2] -flyshoes.m_ButtonTable[dest2].nPointRange[0] ;
		// INT_PTR nHeight =  flyshoes.m_ButtonTable[dest2].nPointRange[5] - flyshoes.m_ButtonTable[dest2].nPointRange[1] ;

		int point_x = flyshoes.m_ButtonTable[dest2].nPointRange[0] ;
		int point_y = flyshoes.m_ButtonTable[dest2].nPointRange[1];
		INT_PTR nWeight = flyshoes.m_ButtonTable[dest2].nPointRange[2] ;
		INT_PTR nHeight =  flyshoes.m_ButtonTable[dest2].nPointRange[3];


		//处理传送 传送到所需中心点
		//if(((CActor*)m_pEntity)->FlyShoseToStaticScene(flyshoes.m_ButtonTable[dest2].nDestination,point_x/4,point_y/4 , nWeight/2, nHeight/2)){
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		if(((CActor*)m_pEntity)->FlyShoseToStaticScene(flyshoes.m_ButtonTable[dest2].nDestination,point_x,point_y , nWeight, nHeight,enDefaultTelePort,nEffId)){
			//消费小飞鞋
			//((CActor*)m_pEntity)->ChangeFlyShoesNum(-flyshoes.m_Index[dest1].nCost) ;
			((CActor*)m_pEntity)->ChangeFlyShoes(-flyshoes.m_Index[dest1].nCost, GameLog::Log_FlyShoestran, 0, "FlyShoes_Transport") ; 
			nFlyShoeNpcId = flyshoes.m_ButtonTable[dest2].nNpcId;
		}
	}
}
//收到转向包，进行处理
void  CMovementSystem::HandTurn(CDataPacketReader &packet)
{
	//广播 修复转向问题
	//unsigned int nDir = m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
	//nDir = (nDir + 4 ) % DIR_STOP;
	BYTE nDir;
	packet >> nDir;
	nDir = (nDir + 8) % DIR_STOP;
	char buff[128];
	CDataPacket data(buff,sizeof(buff));
	data << (BYTE) enDefaultEntitySystemID << (BYTE) sTurnAround
	<<	(Uint64)m_pEntity->GetHandle()  << (BYTE) nDir;
	unsigned int dir = nDir;
	m_pEntity->SetProperty<unsigned int>(PROP_ENTITY_DIR, dir);
	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
	if(pSystem)
	{
		pSystem->BroadCast(data.getMemoryPtr(),data.getPosition(),false);
	}
}

//处理移动的网络包
void CMovementSystem::HandMove(CDataPacketReader &packet)
{
	WORD nDir;
	WORD nPosX;
	WORD nPosY;
	unsigned int nSendPacketTime = 0;
	packet  >> nPosX >>nPosY >>nDir >> nSendPacketTime;
	//OutputMsg(rmTip, _T("Walk x=%d,y=%d, dir=%d,step=%d"),nPosX,nPosY,nDir,1);
	if (Move(nDir, nPosX, nPosY, 1, nSendPacketTime)) //移动1格
		((CActor *)m_pEntity)->SendOperateResult(true,(unsigned int)1,0,false);
	else
	{ 
		((CActor *)m_pEntity)->SendOperateResult(false,(unsigned int)1,0,false);
		
		WORD nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		WORD nCurrentPosy = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		ResertActorPos(nCurrentPosx, nCurrentPosy);
		//OutputMsg(rmTip, _T("Name:%s Walk stuck x=%d,y=%d, dir=%d,step=%d"), m_pEntity->GetEntityName(), nPosX, nPosY, nDir, 1);
	}

#ifdef _DEBUG
	const char * tname = m_pEntity->GetEntityName();
	INT_PTR nCurrentPosx =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nCurrentDir =  m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	//OutputMsg(rmTip, _T("[%s] 服务器坐标方向(%d,%d)(%d) 客户端坐标方向(%d,%d)(%d)"), tname, nCurrentPosx, nCurrentPosy, nCurrentDir, nPosX, nPosY, nDir);
#endif
}

void CMovementSystem::HandRun(CDataPacketReader &packet)
{
	WORD nDir;
	WORD nPosX;
	WORD nPosY;
	unsigned int nSendPacketTime = 0;
	packet  >> nPosX >>nPosY >>nDir >> nSendPacketTime;
	int nStep = 2;
	//OutputMsg(rmTip, _T("Run x=%d,y=%d, dir=%d,step=%d"),nPosX,nPosY,nDir,nStep);
 
	if(Move(nDir, nPosX, nPosY, nStep, nSendPacketTime))
		((CActor *)m_pEntity)->SendOperateResult(true, (unsigned int)nStep, 0, false);
	else
	{ 
		((CActor *)m_pEntity)->SendOperateResult(false, (unsigned int)nStep, 0, false);
		
		WORD nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		WORD nCurrentPosy = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		ResertActorPos(nCurrentPosx, nCurrentPosy); 
		//OutputMsg(rmTip, _T("Name:%s Walk stuck x=%d,y=%d, dir=%d,step=%d"), m_pEntity->GetEntityName(), nPosX, nPosY, nDir, 1);
	}

#ifdef _DEBUG
	const char * tname = m_pEntity->GetEntityName();
	INT_PTR nCurrentPosx =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nCurrentDir =  m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
	//OutputMsg(rmTip, _T("[%s] 服务器坐标方向(%d,%d)(%d) 客户端坐标方向(%d,%d)(%d)"), tname, nCurrentPosx, nCurrentPosy, nCurrentDir, nPosX, nPosY, nDir);
#endif
}

void CMovementSystem::HandJump(CDataPacketReader &packet)
{
	WORD nPosX;
	WORD nPosY;
	packet  >> nPosX >> nPosY;
	Jump(nPosX,nPosY);
}

bool CMovementSystem::CanJumpTo(CScene * pScene, INT_PTR nPosX,INT_PTR nPosY,bool bFailSendTipmsg)
{
	
	if( !pScene->IsTelePort(nPosX,nPosY)  && pScene->CanMove(nPosX,nPosY))
	{
		return true;
	}
	else
	{
		if(bFailSendTipmsg)
		{
			if(m_pEntity->GetType() ==enActor)
			{
				((CActor*)m_pEntity)->SendOldTipmsgWithId(tpJumpTargetCanNotJump);
			}
		}
		return false;
	}
	
}

//冲锋
bool CMovementSystem::RapidMove(int nStep)
{
	if(nStep ==0) return false;
	int nPosX,nPosY; //玩家当前的位置
	
	m_pEntity->GetPosition(nPosX,nPosY); 

	INT_PTR nDir = m_pEntity->GetProperty< int>(PROP_ENTITY_DIR);
	
	
	if(nStep <0)
	{
		nDir = (nDir +4 )%DIR_STOP; //反向
		nStep =-nStep;
	}
	
	CScene* pScene;
	pScene = m_pEntity->GetPosInfo().pScene;
	if( NULL == pScene)
	{
		return false;
	}

	INT_PTR nFinishStep = 0; 
	//INT_PTR nLeftStep   = nStep;
	INT_PTR nNewPosX    =  nPosX;
	INT_PTR nNewPosY    =  nPosY;

	while( nFinishStep < nStep) //如果还没完成
	{ 
		INT_PTR nOneStep =1;
		if( GetFanAvailablePos(nNewPosX,nNewPosY,nDir,nOneStep))
		{
			
			//取新坐标
			INT_PTR nCheckPosX    =  nNewPosX;
			INT_PTR nCheckPosY    =  nNewPosY;

			CEntityGrid::NewPosition(nCheckPosX,nCheckPosY,nDir,nOneStep);  
			if(! CanJumpTo(pScene,nCheckPosX,nCheckPosY) ) //遇到了一个传说门就不过去了
			{
				break;
			}
			else
			{
				nNewPosX = nCheckPosX;
				nNewPosY = nCheckPosY;
			}
			nFinishStep ++;		 //已经完成的步子
		}
		else
		{
			break;
		}
	}
	
	//不能运动
	if( nFinishStep <=0 )
	{
		return false;
	}
	
	if(pScene->MoveTo(m_pEntity,nNewPosX,nNewPosY)  )
	{
		INT_PTR nNewDir = CEntity::GetDir(nPosX,nPosY,nNewPosX,nNewPosY);
		if(nNewDir != DIR_STOP)
		{
			m_pEntity->SetDir(nNewDir);
		}
		if(m_pEntity->GetPosInfo().pScene ==pScene)
		{
			char broadcasrBuff[24];
			CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
			//广播玩家的c移动
			broadData << (BYTE) enDefaultEntitySystemID << (BYTE)sRapidMove;
			broadData << (Uint64)m_pEntity->GetHandle() <<(WORD) nPosX << (WORD) nPosY;
			broadData << (WORD) nNewPosX << (WORD) nNewPosY;
			broadData << (BYTE) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
			CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
			if(pSystem)
			{
				pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),true);
			}
		}
		return  true;
	}
	

	return false;
}

//冲锋
bool CMovementSystem::TargetRapidMove(int nTargetPosX, int nTargetPosY, int nMaxStep, int nSpeed)
{
	int nPosX,nPosY; //玩家当前的位置
	m_pEntity->GetPosition(nPosX,nPosY); 

	INT_PTR nDir = m_pEntity->GetProperty< int>(PROP_ENTITY_DIR);

	if (nMaxStep <= 0)
	{
		nMaxStep = 30;
	}

	CScene *pScene = m_pEntity->GetPosInfo().pScene;
	if (pScene) return false;

	INT_PTR nFinishStep = 0, nOneStep = 1; 
	INT_PTR nNewPosX = nPosX;
	INT_PTR nNewPosY = nPosY;

	static INT_PTR nDirTbl[3] = {0, -1, 1};

	while (nFinishStep < nMaxStep) //如果还没完成
	{
		nDir = CEntity::GetDir(nNewPosX,nNewPosY,nTargetPosX,nTargetPosY);
		
		//取新坐标
		INT_PTR nCheckPosX = nNewPosX;
		INT_PTR nCheckPosY = nNewPosY;

		bool bCanMove = false;
		for (INT_PTR i = 0; i < 1; i++)	//判断3方向 现暂判断1方向
		{
			nDir = (nDir + nDirTbl[i]) % DIR_STOP;

			CEntityGrid::NewPosition(nCheckPosX,nCheckPosY,nDir,nOneStep);  
			if (CanJumpTo(pScene,nCheckPosX,nCheckPosY)) //遇到了一个传说门就不过去了
			{
				nNewPosX = nCheckPosX;
				nNewPosY = nCheckPosY;
				bCanMove = true;
				break;
			}
		}

		if (!bCanMove)	//无法通过
		{
			break;
		}
		
		nFinishStep ++;		 //已经完成的步子
	}

	//不能运动
	if (nFinishStep <= 0)
	{
		return false;
	}

	if (pScene->MoveTo(m_pEntity,nNewPosX,nNewPosY))
	{
		INT_PTR nNewDir = CEntity::GetDir(nPosX,nPosY,nNewPosX,nNewPosY);
		if (nNewDir != DIR_STOP)
		{
			m_pEntity->SetDir(nNewDir);
		}
		if(m_pEntity->GetPosInfo().pScene ==pScene)
		{
			char broadcasrBuff[30];
			CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
			//广播玩家的c移动
			broadData << (BYTE) enDefaultEntitySystemID << (BYTE)sTargetRapidMove;
			broadData << (Uint64)m_pEntity->GetHandle() <<(WORD) nPosX << (WORD) nPosY;
			broadData << (WORD) nNewPosX << (WORD) nNewPosY;
			broadData << (BYTE) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
			broadData << (int)nSpeed;
			CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
			if(pSystem)
			{
				pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),true);
			}
		}

		return  true;
	}


	return false;
}





bool CMovementSystem::BeatBack(INT_PTR nDir,INT_PTR nStep,bool isBeatback)
{
	if(nDir <0) return false;
	
	CScene* pScene;
	pScene = m_pEntity->GetPosInfo().pScene;
	if(pScene ==NULL)
	{
		return false;
	}
	
	//当前的坐标
	INT_PTR nCurrentPosx =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nNewPosX    =  nCurrentPosx;
	INT_PTR nNewPosY    =  nCurrentPosy;
	CEntityGrid::NewPosition(nNewPosX,nNewPosY,nDir,1);
	if (!pScene->CanCross(m_pEntity->GetType(),nNewPosX,nNewPosY))
	{
		return false;
	}
	

	if(nStep <0)
	{
		nStep = - nStep;
	}
	
	//当前时间是否能够移动
	
	if(m_pEntity->IsDeath() ==false)
	{
		bool bCanMove  = m_pEntity->CanEnterState(esStateMove); //能否进入移动状态
		if(bCanMove == false) return false;
	}
	
	
	//自身不能移动
	if(m_pEntity->GetAttriFlag().DenyMove)
	{
		return false;
	}

	if(m_pEntity->HasState(esStateSing)) //如果在吟唱状态，那么通知技能系统打断吟唱
	{
		m_pEntity->GetSkillSystem().EndSingSpell(true); //自己打断的
	}

	INT_PTR nFinishStep = 0; 
	
	int nOldDir = nDir;
	while( nFinishStep < nStep) //如果还没完成
	{ 
		INT_PTR nOneStep =1;
		if( GetFanAvailablePos(nNewPosX,nNewPosY,nDir,nOneStep))
		{

			//取新坐标
			INT_PTR nCheckPosX    =  nNewPosX;
			INT_PTR nCheckPosY    =  nNewPosY;

			if (nOldDir != nDir)
			{
				break;
			}

			CEntityGrid::NewPosition(nCheckPosX,nCheckPosY,nDir,nOneStep);  
			if(! CanJumpTo(pScene,nCheckPosX,nCheckPosY) ) //遇到了一个传说门就不过去了
			{
				break;
			}
			else
			{
				nNewPosX = nCheckPosX;
				nNewPosY = nCheckPosY;
			}
			nFinishStep ++;		 //已经完成的步子
		}
		else
		{
			break;
		}
	}
	
	//不能运动
	if( nFinishStep <=0 )
	{
		return false;
	}

	if(pScene->MoveTo(m_pEntity,nNewPosX,nNewPosY)  )
	{
		

		if(m_pEntity->GetPosInfo().pScene == pScene)
		{
		
			unsigned int nNextTime ;
			if(m_pEntity->GetType() == enActor)
			{
				//玩家的话允许有20%的误差
				nNextTime = (unsigned int)(  m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME) * nFinishStep);
			}
			else
			{
				nNextTime = (int)(m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME)/2*nFinishStep);
			}

			((CAnimal*)m_pEntity)->SetCommonOpNextTime(nNextTime,true,true,1,0,false);

			
			char broadcasrBuff[24];
			CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
			//广播玩家的c移动
			BYTE  nCmd;
			if(isBeatback) //击退
			{
				nCmd= sRapidMove;
			}
			else      //击飞
			{
				nCmd = sBeatFly;
			}
			
			broadData << (BYTE) enDefaultEntitySystemID << (BYTE)nCmd;
			broadData << (Uint64)m_pEntity->GetHandle() <<(WORD) nCurrentPosx << (WORD) nCurrentPosy;
			broadData << (WORD) nNewPosX << (WORD) nNewPosY;
			broadData << (BYTE) m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
			CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
			if(pSystem)
			{
				pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),true);
			}
		}
		return  true;
	}


	return true;
}



void CMovementSystem::BroadcastMove(INT_PTR nPosX, INT_PTR nPosY,INT_PTR nCmd,unsigned nType,unsigned nValue)
{
	if(m_pEntity == NULL) return;

	char broadcasrBuff[256];
	CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
	//广播玩家的瞬间移动
	broadData << (BYTE) enDefaultEntitySystemID << (BYTE)nCmd ;
	broadData << (Uint64)m_pEntity->GetHandle() <<(WORD) nPosX << (WORD) nPosY;
	broadData << (BYTE) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_DIR);
	broadData << (unsigned int) nType;
	broadData << (unsigned int) nValue;
	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
	if(pSystem)
	{
		pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),true);
	}
}

bool CMovementSystem::GetMoveablePoint(CScene *pScene,INT_PTR& nPosX,INT_PTR& nPosY,INT_PTR nRange)
{
	if(!CanJumpTo(pScene,nPosX,nPosY))
	{
		bool bResult =false;
		INT_PTR newPosX,newPosY; //位置


		for(INT_PTR i=1; i<= nRange; i++ ) //在旁边取几个点,这里还是按1格来，测试发现2格会跳到擂台上
		{
			//下边缘
			newPosY  = nPosY+i;
			for(INT_PTR j= -i;j <i; j ++)
			{
				if(CanJumpTo(pScene,nPosX +j,newPosY))
				{
					bResult = true;
					nPosX += (int)j;
					nPosY =(int)newPosY;
					break;
				}
			}
			if(bResult)
			{
				break;
			}
			//左边缘
			newPosX  = nPosX -i;
			for(INT_PTR j= -i;j <i; j ++)
			{
				if(CanJumpTo(pScene,newPosX,nPosY+j))
				{
					bResult = true;
					nPosX = (int)newPosX;
					nPosY += (int)j;
					break;
				}
			}
			if(bResult)
			{
				break;
			}
			//上边缘
			newPosY  = nPosY-i;
			for(INT_PTR j= -i;j <i; j ++)
			{
				if(CanJumpTo(pScene,nPosX +j,newPosY))
				{
					bResult = true;
					nPosX += (int)j;
					nPosY = (int)newPosY;
					break;
				}
			}
			if(bResult)
			{
				break;
			}
			//右边缘
			newPosX  = nPosX +i;
			for(INT_PTR j= -i;j <i; j ++)
			{
				if(CanJumpTo(pScene,newPosX,nPosY+j))
				{
					bResult = true;
					nPosX =(int)newPosX;
					nPosY += (int)j;
					break;
				}
			}
		}
		return bResult;
	}
	else
	{
		return true;
	}
}
//瞬间移动到一个点去
bool CMovementSystem::InstantMove(int nPosX,int nPosY, int isUseRand,unsigned nType,unsigned nValue)
{
	//当前的坐标
	INT_PTR nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	//当前时间是否能够移动
	bool bCanMove  = m_pEntity->CanEnterState(esStateMove); //能否进入移动状态
	if(bCanMove == false) return false;
	//瞬间移动到一个位置
	if(nCurrentPosx == nPosX && nCurrentPosy == nPosY)
	{
		return false;
	}
	
	if(m_pEntity->HasState(esStateSing)) //如果在吟唱状态，那么通知技能系统打断吟唱
	{
		m_pEntity->GetSkillSystem().EndSingSpell(true); //自己打断的
	}
	CScene* pScene;
	pScene = m_pEntity->GetPosInfo().pScene;
	if(pScene)
	{
		bool bResult =true;
		/*
			这里按一个回字从里到外查找，直到查找到一个点
		     --------
			 ||目标点 ||
			 ||-------||
		*/
		if(!CanJumpTo(pScene,nPosX,nPosY))
		{
			bResult =false;
			if (isUseRand == 1)	//使用查找
			{
				INT_PTR newPosX,newPosY; //位置


				for(INT_PTR i=1; i<=1; i++ ) //在旁边取几个点,这里还是按1格来，测试发现2格会跳到擂台上
				{
					//下边缘
					newPosY  = nPosY+i;
					for(INT_PTR j= -i;j <i; j ++)
					{
						if(CanJumpTo(pScene,nPosX +j,newPosY))
						{
							bResult = true;
							nPosX += (int)j;
							nPosY =(int)newPosY;
							break;
						}
					}
					if(bResult)
					{
						break;
					}
					//左边缘
					newPosX  = nPosX -i;
					for(INT_PTR j= -i;j <i; j ++)
					{
						if(CanJumpTo(pScene,newPosX,nPosY+j))
						{
							bResult = true;
							nPosX = (int)newPosX;
							nPosY += (int)j;
							break;
						}
					}
					if(bResult)
					{
						break;
					}
					//上边缘
					newPosY  = nPosY-i;
					for(INT_PTR j= -i;j <i; j ++)
					{
						if(CanJumpTo(pScene,nPosX +j,newPosY))
						{
							bResult = true;
							nPosX += (int)j;
							nPosY = (int)newPosY;
							break;
						}
					}
					if(bResult)
					{
						break;
					}
					//右边缘
					newPosX  = nPosX +i;
					for(INT_PTR j= -i;j <i; j ++)
					{
						if(CanJumpTo(pScene,newPosX,nPosY+j))
						{
							bResult = true;
							nPosX =(int)newPosX;
							nPosY += (int)j;
							break;
						}
					}
				}
			}
			
		}
		
		if(bResult && pScene->MoveTo(m_pEntity,nPosX,nPosY,true)  )
		{
			if(m_pEntity->GetPosInfo().pScene ==pScene)
			{
				BroadcastMove(nPosX,nPosY,sInstatnceMove,nType,nValue);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

//实体移动实际处理部分
bool CMovementSystem::Move(INT_PTR nDir,INT_PTR nPosX, INT_PTR nPosY,INT_PTR nStep, unsigned int nSendPkgTime,
	bool telSelf,INT_PTR nMoveSpeed,INT_PTR nCmd, INT_PTR nClientFrame,INT_PTR nSimulateReboundSpeed ) 
{
	if(m_pEntity ==NULL) return false;
	
	bool bMoveResult =false; //移动是否成功
	
	INT_PTR nEntityType = m_pEntity->GetType();

	if (nEntityType == enNpc)
	{
		OutputMsg(rmError, _T("Npc[%s]试图移动，现已阻止"), m_pEntity->GetEntityName());
		return false;
	}

	//当前的坐标
	INT_PTR nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	INT_PTR nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);

	if(m_pEntity->HasState(esStateSing)) //如果在吟唱状态，那么通知技能系统打断吟唱
	{
		m_pEntity->GetSkillSystem().EndSingSpell(true); //自己打断的
	}
	
	//自身不能移动,如果使用野蛮冲撞这种技能，该标记不生效
	if(nMoveSpeed ==0)
	{
		if(m_pEntity->GetAttriFlag().DenyMove)
		{
			return false;
		}
	}
	else
	{
		if(m_pEntity->GetAttriFlag().DenyBeMove)
		{
			return false;
		}
	}
	
	//当前时间是否能够移动
	bool bCanMove = true;
	
	if(nMoveSpeed<= 0)
	{
		//if(m_pEntity->GetType() == enActor)
		//	bCanMove = m_pEntity->CheckCommonOpTick( GetLogicCurrTickCount(), true, true);
		//else
		//玩家放到后面去检测他的速度，拉回去
		if(nEntityType != enActor)
		{
			bCanMove = m_pEntity->CheckCommonOpTick(GetLogicCurrTickCount());
		}
	}

	

	if( !bCanMove ) 
	{	
		//OutputMsg(rmTip,_T("can not move"));
		return false; //这里会发包的通知客户端失败
	}

	//检测特殊的buff是否存在
	CBuffSystem *pBuff = m_pEntity->GetBuffSystem();
	INT_PTR ndizzy = m_pEntity->GetProperty<int>(PROP_CREATURE_DIZZY_STATUS);
	if(pBuff && (pBuff->Exists( aDizzy ) || pBuff->Exists(aSlient)) || ndizzy) 
	{
		// return tpSkillTrainWithBuff;
		bCanMove = false;
	}

	if(bCanMove)
	{
		bCanMove = m_pEntity->CanEnterState(esStateMove); //能否进入移动状态
	}
	
	bool isSameScene =true; //是否在同一个场景里
	
	if(bCanMove)  
	{
		//只有位置相等才准予移动
		if(nPosX == nCurrentPosx && nPosY == nCurrentPosy)
		{
			CScene* pScene;
			pScene = m_pEntity->GetPosInfo().pScene;
			if(pScene)
			{
				bool hasEntityBlock =false;
				if(nMoveSpeed>0)
				{
					INT_PTR nNewPox=nCurrentPosx ;
					INT_PTR nNewPoy=nCurrentPosy;
					CEntityGrid::NewPosition(nNewPox,nNewPoy,nDir,nStep);

					const MapEntityGrid *pGrid=pScene->GetMapEntityGrid((int)nNewPox);
					CEntityManager* pEntityMgr = GetGlobalLogicEngine()->GetEntityMgr();
					if(pGrid ==NULL) return false;
					if(pEntityMgr ==NULL) return false;

					if(pScene->IsTelePort(nNewPox,nNewPoy))
					{
						hasEntityBlock =true;
					}
					else
					{
#ifndef GRID_USE_STL_MAP
						for(INT_PTR i=0; i< pGrid->m_entitys.count(); i++)
						{
							if(pGrid->m_entitys[i].pos != nNewPoy) continue;
							EntityHandle& hHandle = pGrid->m_entitys[i].hd;
#else
						const EntityPosVector& entitys = pGrid->m_entitys;
						typedef typename EntityPosVector::const_iterator Iter;
						Iter it = entitys.begin(), end = entitys.end();
						for (; it != end; it++)
						{
							if((*it).second != nNewPoy) continue;
							EntityHandle hHandle = (*it).first;
#endif
							switch(hHandle.GetType())
							{
							case enActor:
							case enMonster:
							case enNpc:
							case enPet:
							case enHero:
								CEntity* pEntity = pEntityMgr->GetEntity(hHandle);
								if (pEntity && !pEntity->IsDeath())
									hasEntityBlock =true;
								break;
							}
						}
					}
				}

				if(!hasEntityBlock && nDir !=DIR_STOP && pScene->Move(m_pEntity,nDir,nStep) ) //这里可能做了传送
				{
					//OutputMsg(rmTip,"移动nDir=%d,nStep=%d",nDir,nStep);
					if (pScene !=  m_pEntity->GetPosInfo().pScene)
					{
						isSameScene =false;
					}
					//移动成功并设置下一次移动的时间
					bMoveResult = true;
					
					//这里可以不设置朝向
					if(nCmd ==0)
					{
						m_pEntity->SetDir(nDir); //设置朝向
					}
					
					 //通知附近的
					if(nEntityType ==enActor || nEntityType == enPet)
					{
						m_pEntity->GetObserverSystem()->OnEvent(neEventMove,nCurrentPosx,nCurrentPosy);
					}
					//这里2格是正常的移动速度，也就是配置的速度，1格的话时间是1半
					unsigned int nNextTime ;
					bool bNeedCheckCd =true; //是否需要检测cd
					if(nEntityType ==enActor)
					{
						//玩家的话允许有20%的误差
						if(nMoveSpeed >0)
						{
							nNextTime = (unsigned int)( nMoveSpeed * nStep);
							bNeedCheckCd =false;
						}
						else
						{
							nNextTime = ( int)(m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME));
						}
					}
					else
					{
						if(nMoveSpeed >0) //特殊情况，野蛮冲撞这种
						{
							nNextTime = (int)(nMoveSpeed*nStep);
						}
						else
						{
							nNextTime = (int)(m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME)/2*nStep);
						}
					}
					
					((CAnimal*)m_pEntity)->SetCommonOpNextTime(nNextTime,false,true, (unsigned int)nStep, nSendPkgTime,true,bNeedCheckCd);
				}
				else
				{
					OutputMsg(rmTip,_T("当前位置不能移动,x=%d,y=%d,ndir=%d,nstep=%d"),nCurrentPosx,nCurrentPosy,nDir,nStep);
				}
				
			}
		}
	}
	
	//char buff[24];
	//CDataPacket data(buff,sizeof(buff));
	//bool bNeedBroadcast = false;

	if(bMoveResult)
	{
		//像附近广播移动消息
		if(isSameScene) //如果在同一个场景里才广播
		{
			char broadcasrBuff[24];
			CDataPacket broadData(broadcasrBuff,sizeof(broadcasrBuff)); //广播的数据
			nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
			nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
			INT_PTR nCodeID = sOtherEntityMove;
			//OutputMsg(rmTip,"succ move,x=%d,y=%d",nCurrentPosx,nCurrentPosy);
			if(nCmd >0)
			{
				//往前面跑

				nCodeID = nCmd;
				broadData <<(BYTE)enDefaultEntitySystemID  << (BYTE) nCodeID 
					<<Uint64(m_pEntity->GetHandle()) << WORD(nCurrentPosx) << WORD(nCurrentPosy)  << (WORD)nMoveSpeed <<(BYTE) m_pEntity->GetProperty<int>(PROP_ENTITY_DIR) <<(BYTE) nClientFrame;
			}
			else
			{
				if(nStep >1)
				{
					nCodeID = sOtherEntityRun;
				}
				broadData <<(BYTE)enDefaultEntitySystemID  << (BYTE) nCodeID 
					<<Uint64(m_pEntity->GetHandle()) << WORD(nCurrentPosx) << WORD(nCurrentPosy) << (BYTE)nDir;

			}

			CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
			if(pSystem)
			{
				pSystem->BroadCast(broadData.getMemoryPtr(),broadData.getPosition(),telSelf);
				//pSystem->SendPosChangeToOwner();
			}
		}

		// 隐身状态
		if(m_pEntity->GetBuffSystem()->Exists(aHide) )
		{
			m_pEntity->GetBuffSystem()->Remove(aHide,-1);
		}
	}
	else //if(!bMoveResult ) //移动失败
	{	
		if( nSimulateReboundSpeed >0 ) //如果移动失败的话，在某些情况下是需要广播的
		{
			CObserverSystem * pSystem = m_pEntity->GetObserverSystem();

			//往前面跑
			//CEntityGrid::NewPosition(nCurrentPosx,nCurrentPosy,nDir,nStep); //设置坐标

			char buffData[24];
			CDataPacket buffDataPack(buffData,sizeof(buffData)); //广播的数据
			buffDataPack <<(BYTE)enDefaultEntitySystemID  << (BYTE) nCmd 
				<<Uint64(m_pEntity->GetHandle()) << WORD(nCurrentPosx) << WORD(nCurrentPosy)  << (WORD)nSimulateReboundSpeed  <<(BYTE)  nDir << (BYTE) nClientFrame;
			if(pSystem)
			{
				pSystem->BroadCast(buffDataPack.getMemoryPtr(),buffDataPack.getPosition(),telSelf);
			}
		}
		else //如果是玩家移动失败的话，要重新设置下坐标
		{
			if(m_pEntity->GetType() ==enActor)
			{
				nCurrentPosx = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
				nCurrentPosy =  m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
				//OutputMsg(rmTip,"fail move,x=%d,y=%d",nCurrentPosx,nCurrentPosy);
				((CActor*)m_pEntity)->SendOperateResult(false);
				ResertActorPos(nCurrentPosx,nCurrentPosy);
			}
		}
	}
	
	return bMoveResult;
}

void CMovementSystem::PostponeMoveTime(INT_PTR nMs)
{
	//m_canMoveTick = GetLogicCurrTickCount() + nMs;
}

void CMovementSystem::ResertActorPos(INT_PTR nPosx,INT_PTR nPosy)
{
	OutputMsg(rmWaning, _T("[%s] ResertActorPos(%d, %d)"), m_pEntity->GetEntityName(), nPosx, nPosy);
	CActorPacket pack;
	CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)enDefaultEntitySystemID << (BYTE)sResetMainActorPos ;
	data << (WORD) nPosx << (WORD )nPosy;
	pack.flush();
}
bool CMovementSystem::Jump(int nPosX, int nPosY)
{
	/*
	char sBuf[128];
	int nCurX, nCurY;
	CDataPacket pack(sBuf, sizeof(sBuf));

	((CEntity*)m_pEntity)->GetPosition(nCurX, nCurY);

	//判断目标位置是否可以移动
	CScene *pScene = ((CEntity*)m_pEntity)->GetScene();
	
	//当前位置能否跳跃
	if (pScene->HasMapAttribute(nCurX,nCurY,aaNotJump)) goto lb_act_fail;
	
	if(! CanJumpTo(pScene,nPosX,nPosY,true )) goto lb_act_fail;

	//能否进入移动状态
	if(! m_pEntity->CanEnterState(esStateMove) ) goto lb_act_fail;
	if(m_pEntity->HasState(esStateSing)) //如果在吟唱状态，那么通知技能系统打断吟唱
	{
		m_pEntity->GetSkillSystem().EndSingSpell(true); //自己打断的
	}

	int nJumpLength =max( nCurY -nPosY, abs( nCurX -nPosX ) ) ;//跳跃的长度
	int nJumpDownHeight = 0;    //向下跳的高度
	int nJumpPowerConsume =0;    //需要消耗的体力的值
	int nJumpHpDropRate =0 ;     //玩家跳跃掉血的比例,百分之x
	
	//跳跃消耗的体力
	nJumpPowerConsume =GetLogicServer()->GetDataProvider()->GetJumpLengthConsume().GetJumpConsume(nJumpLength);
	if( nPosY > nCurY ) //向下跳跃的高度
	{
		nJumpDownHeight = nPosY - nCurY; //向下需要掉血
		nJumpHpDropRate =  GetLogicServer()->GetDataProvider()->GetJumpDamageTable().GetJumpDamage(nJumpDownHeight); //需要掉落%x的血
	}
	

	//计算跳跃的方向
	INT_PTR nDir = CEntity::GetDir(nCurX, nCurY,nPosX, nPosY);
	
	//计算跳跃所需时间
	INT_PTR nTime     =   abs (nPosX - nCurX) ;
	INT_PTR nMaxYTime =   abs(nPosY - nCurY);
	if (nTime < nMaxYTime) nTime = nMaxYTime; //取X,Y方向跳跃的距离的最大的数值

	//跳跃时间必须控制在1秒内。如果时间太长，则客户端表现跳跃抛物的Y轴定点会太高
	if (nTime > 10) nTime = 10;
	nTime *= 100;

	
	//移动到目的地
	if (!pScene->MoveTo(m_pEntity, nPosX, nPosY)) goto lb_act_fail;


	
	//广播跳跃消息
	pack.setLength(0);
	pack << (BYTE)enDefaultEntitySystemID << (BYTE)sEntityJump 
		<< Uint64(m_pEntity->GetHandle()) << (WORD)nPosX << (WORD)nPosY << (BYTE)nDir;
	CObserverSystem * pSystem = m_pEntity->GetObserverSystem();
	pSystem->BroadCast(pack.getMemoryPtr(), pack.getPosition(), true, true);
	//推迟观察者列表的刷新时间
	m_pEntity->PostponeRefViewTime(nTime);
	
	//需要掉血
	if(nJumpHpDropRate >0)
	{
		int nDropValue= int(m_pEntity->GetProperty<unsigned int>(PROP_CREATURE_MAXHP) * (nJumpHpDropRate/(float)100));
		if(nDropValue >0) //需要掉血了
		{
			CEntityMsg msg(CEntityMsg::emChangeHp);
			msg.nParam1 = -nDropValue; //要减的血
			msg.dwDelay = nTime;//延迟掉血
			m_pEntity->PostEntityMsg(msg); 
		}
	}

	//这里会发包通知客户端
	((CActor*)m_pEntity)->SetCommonOpNextTime(nTime,true,true);
	
	return true;

lb_act_fail:
	//返回移动失败
	if (m_pEntity->GetHandle().GetType() == enActor)
	{
		ResertActorPos(nCurX,nCurY);
		((CActor*)m_pEntity)->SendOperateResult(false);
	}
	*/
	return false;
}
void CMovementSystem::OnMotionFinish(INT_PTR nMotionType)
{
	//回位完成了,要去掉这个怪物的回归状态
	if(nMotionType == mtMotionTypeReturnHome )
	{
		if(m_pEntity->HasState(esStateReturnHome))
		{
			m_pEntity->RemoveState(esStateReturnHome);
		}
	}
	else if(nMotionType == mtMotionTypeChase) //追逐结束了
	{
		
	}
	else if (nMotionType == mtMotionTypePoint) // 移动到目标点
	{
		//if (enMonster == m_pEntity->GetType())
		//{					
		//	CAnimalAI *pAI = ((CMonster *)m_pEntity)->GetAI();
		//}
		if (enPet == m_pEntity->GetType())
		{
			m_pEntity->RemoveState(mtMotionTypePoint);
		}
	}
}
VOID CMovementSystem::OnTimeCheck(TICKCOUNT nTick)
{
	if ( !m_pEntity )
	{
		return;
	}
	if (m_pEntity->GetType() == enActor )
	{
		if(s_boCheckIdleLowPly && m_tmIdleLowPly.CheckAndSet(nTick, true))
		{
			int nMyLevel = m_pEntity->GetProperty<int>(PROP_CREATURE_LEVEL);
			if (nMyLevel <= s_btMinIdlePlyLevel)
			{
				((CActor *)m_pEntity)->CloseActor(lwiMoveSysIdle, false);
			}
		}
		return;
	}

	if (m_pEntity->GetType() == enPet)
	{
		if (((CPet*)m_pEntity)->GetBattleStatus() == CPet::PET_BATTLE_STATUE_STAY)
		{
			return;
		}
	}

	bool bHasMove =false;
	CAnimalAI *pAi =m_pEntity->GetAI();
	if( m_timer.Check(nTick) && m_pEntity->CheckCommonOpTick(nTick, false)) 
	{
		if(! m_pEntity->CanEnterState(esStateMove)) return; //不能移动的时候不移动
		bool flag =false; //是否移动成功了
		do
		{
			MotionData &data = m_motionStack.GetCurrentMotionState(); //获取当前的状态
			if(mtMotionTypeIdle == data.bMotionType) break; //不需要运动
						
			// 战斗状态不巡逻。解决怪物在攻击过程中因为巡逻导致走动，继而updateai又让怪物走回来，这样就不断的左右走动而不会攻击
			if (((CAnimal *)m_pEntity)->GetType() != enPet && ((CAnimal *)m_pEntity)->HasState(esStateBattle))
			{
				if (mtMotionTypePatrol == data.bMotionType)
					break;
				else if (mtMotionTypeFollow == data.bMotionType)
				{
					if (pAi)  // 当前跟随，并且有AI，不让他跟随（防止一边跟随一边追怪）。否则，跟随
						break;
				}
				else if (mtMotionTypePoint == data.bMotionType)
				{
					if (pAi && !m_pEntity->GetAttriFlag().noAttackBack)
						break; // 朝目标点移动过程中如果进入战斗了，不移动
				}
			}
				
			INT_PTR nCurrentPosX = ((CEntity*)m_pEntity)->GetProperty<int>(PROP_ENTITY_POSX);
			INT_PTR nCurrentPosY = ((CEntity*)m_pEntity)->GetProperty<int>(PROP_ENTITY_POSY);
			INT_PTR nDir = DIR_STOP;
			INT_PTR nStep =0;
			int nTelportX = 0, nTelportY = 0;
			switch (data.bMotionType)
			{
			case mtMotionTypePatrol: //巡逻
				flag = OnPatrolMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				//OutputMsg(rmTip,_T("move x=%d,y=%d,dir=%d,step=%d"),nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeChase: //追逐
				flag = OnChaseMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeFleeing://逃避
				flag = OnFleeMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeConfused://迷糊状态,乱走
				flag = OnConfusedMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeFollow://跟随（主要是宠物）
				flag = OnFollowMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypePoint:			//往一个点移动
				flag = OnPointMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeReturnHome:	//回归
				flag = OnPointMove(data,nCurrentPosX,nCurrentPosY,nDir,nStep);
				break;
			case mtMotionTypeRandomFlee:
				flag = OnRandomFlee(data, nCurrentPosX, nCurrentPosY, nDir, nStep);
				break;
			default:
				break;
			}
			
			if(!flag)
			{
				OnMotionFinish(data.bMotionType);
					// 跟随运动状态一直保持，不会弹出.  向某个点移动状态也不弹出状态
				if (data.bMotionType != mtMotionTypeFollow && data.bMotionType != mtMotionTypePoint)
					m_motionStack.PopState(); //删除一个状态
			}
			else
			{
				if(nDir !=DIR_STOP )	
				{
						bHasMove = Move(nDir,nCurrentPosX,nCurrentPosY,nStep); //往前面移动一步
						if (data.bMotionType == mtMotionTypeRandomFlee)
						{
							data.wAuxInfo2 = (WORD)nCurrentPosX;
							data.wAuxInfo3 = (WORD)nCurrentPosY;
						}
				}
			}

			if (data.bMotionType == mtMotionTypeFollow || data.bMotionType == mtMotionTypePoint)  // 跟随执行完成（防止跟随者丢失了这里一直执行）
				flag = true;
	
		}while(!flag);

		if(!bHasMove)
		{
			
			m_timer.SetNextHitTimeFromNow ( m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME) );			
		}
		else
		{
			if(pAi)
			{
				m_timer.Delay ( pAi->GetMoveStopInterval() );
			}
		}

	}
}

void CMovementSystem::GetCalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep)
{
	CalculateMoveParam(nStartX,nStartY,nEndX,nEndY,nDir,nStep);
}

bool CMovementSystem::CalculateMoveParam(INT_PTR nStartX,INT_PTR nStartY,
	INT_PTR nEndX, INT_PTR nEndY, INT_PTR & nDir,INT_PTR &nStep,INT_PTR nMinDisSquare,INT_PTR nMaxDisSquare)
{
	DECLARE_TIME_PROF("CMovementSystem::Astar");
	//距离的平方
	INT_PTR nDisSquare = GetDisSqare(nStartX,nStartY, nEndX,nEndY);
	
	 //距离已经到达了这么个间隔，不需要再移动了
	if(nDisSquare <= nMinDisSquare )
	{
		m_searchNodes.clear();
		return false;
	}

	//超过了最大的距离就不再移动了
	if( nMaxDisSquare && nDisSquare >= nMaxDisSquare )
	{
		return false; 
	}
	//已经运动到了终点
	if(nDisSquare <=0)
	{
		return false; //停止移动
	}
	else
	{
		
		//在一个扇形区域里找一个能运动到的点，可以是1步到，也可以是2步到,优先2步到
		CScene *pScene = m_pEntity->GetScene();
		if(pScene==NULL) return false;
		 
		INT_PTR nEntityType =m_pEntity->GetType();

		nDir = CEntity::GetDir(nStartX, nStartY,nEndX,nEndY);
		

#ifdef __USE_ASTAR__SEARCH_
		INT_PTR nNextX=nStartX,nNextY=nStartY;

		bool bCanAlwaysEnter =false;
		if(nEntityType == enMonster)			//如果是可以穿人穿怪的
		{
			if(m_pEntity->GetAttriFlag().CanAlwaysEnter == true)
			{
				bCanAlwaysEnter = true;
			}
			//if (m_pEntity->GetAttriFlag().NoUseAStar)
			//{
				return GetFanAvailablePos(nStartX, nStartY, nDir, nStep);
			//}
		}
		else if (nEntityType == enPet && (m_searchNodes.count()<=1))
		{
			if ( ((CPet*)m_pEntity)->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeFollow )
			{
				//bool result = GetFanAvailablePos(nStartX, nStartY, nDir, nStep);
				INT_PTR nCheckPosX=nStartX,nCheckPosY=nStartY;
				CEntityGrid::NewPosition(nCheckPosX,nCheckPosY,nDir,1);
				if (pScene->CanCross(enPet,nCheckPosX,nCheckPosY))
				{
					//return result;
					nStep = 1;
					return true;
				}
			}
		}

		//如果下一步能够走的话，就不需要寻路
		//一格的话根本就不需要寻路，就在前面了
		INT_PTR nAbsX = abs(nStartX - nEndX);
		INT_PTR nAbsY=  abs(nStartY - nEndY);
		if(nAbsX <=1 && nAbsY <=1)
		{
			CEntityGrid::NewPosition(nNextX,nNextY,nDir,1); //获取新的坐标
			nStep =1;
			return pScene->CanCross(nEntityType,nNextX,nNextY,bCanAlwaysEnter,false);
		}

		//这里进行了一下优化，如果目标寻路点还是当前的目标点，那么记录下来路径，如果这条路径还能走的话，则不重新寻路
		INT_PTR nOldNodeCount = m_searchNodes.count();
		int nSonX, nSonY;
		if(nOldNodeCount>1)
		{
			bool alwaysMove = false;
			if (nEntityType == enPet)
			{
				alwaysMove = ((CPet*)m_pEntity)->GetAlwaysMoveFlag();
				if(!alwaysMove)
				{
					((CPet*)m_pEntity)->SetAlwaysMoveFlag(true);
				}
			}
			
			if(alwaysMove || (nEndX == m_searchNodes[0].x && nEndY == m_searchNodes[0].y)) //上次的寻路点还在这
			{
				//找到上一个
				if(nStartX == m_searchNodes[nOldNodeCount -1].x && nStartY == m_searchNodes[nOldNodeCount -1].y)
				{
					m_searchNodes.pop(); //把当前的推出去
					nOldNodeCount --;
					if(nOldNodeCount >=1)
					{
						nSonX = m_searchNodes[nOldNodeCount -1].x;
						nSonY = m_searchNodes[nOldNodeCount -1].y;
						if(pScene->CanCross(nEntityType,nSonX,nSonY,bCanAlwaysEnter,false))
						{
							INT_PTR firstDir = CEntity::GetDir(nStartX,nStartY, nSonX,nSonY);

							if(nEntityType ==enHero)
							{
                                  nDir = firstDir;
								  nStep =2;
								  return true;
							}
							nDir = firstDir;
							nStep =1;
							return true;
						}
						
					}
				}
			}
		}
		
		//重新寻路
		m_searchNodes.clear();
		bool result = pScene->GetPath(m_pEntity, m_searchNodes, nEndX, nEndY);
		if ((!result) && nEntityType == enPet)
		{
			CPetAi * pAi = (CPetAi*)(((CAnimal*)m_pEntity)->GetAI());
			pAi->AddAvoidMap(GetEntityFromHandle(((CAnimal*)m_pEntity)->GetTarget()));
		}
		
		if(int nNodesCount = m_searchNodes.count())
		{
			SMALLCPOINT node = m_searchNodes[nNodesCount -1];
			nDir = CEntity::GetDir(nStartX,nStartY, node.x,node.y);
			nStep = 1;
		}
#else
		return GetFanAvailablePos(nStartX,nStartY, nDir,nStep);
#endif
		
	}
	

	return false;
}

// 往一点的移
bool CMovementSystem::OnPointMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{
	bool boResult =  CalculateMoveParam( nCurrentPosX,nCurrentPosY,data.values[0].wLo,data.values[0].wHi,nDir,nStep);
	if (boResult)
	{
		EntityHandle  hTarget = m_pEntity->GetTarget();
		if (hTarget.IsNull())
		{
			if (!m_pEntity->HasState(esStateBattle))
			{
				//((CMonster *)m_pEntity)->GetBornPoint((INT_PTR &)x, (INT_PTR &)y);
			}
		}
	}
	return boResult;	
}

//巡逻
bool CMovementSystem::OnPatrolMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{

	if(data.bAuxInfo ==0 &&  data.wAuxInfo ) //是静止的，什么都不做，就静止着
	{
		nDir = DIR_STOP;
		data.wAuxInfo --;
		return true;
	}
	else
	{
		INT_PTR nNewDir = data.values[1].btValueHiHi; //上次的移动方向
		nStep =1;
		INT_PTR nDistance = GetDisSqare(nCurrentPosX,nCurrentPosY,data.values[0].wLo,data.values[0].wHi);
		if(nDistance >= data.values[1].wLo) //如果距离超过了最大的距离，需要转向移动
		{
			//朝出生点走
			nNewDir  = CEntity::GetDir(nCurrentPosX,nCurrentPosY,data.values[0].wLo,data.values[0].wHi); //获取方向
		}
		if( GetFanAvailablePos(nCurrentPosX,nCurrentPosY,nNewDir,nStep) )
		{
			nDir = nNewDir;
			if(data.values[1].btValueHiHi != nDir )
			{
				if(data.values[1].btValueHiHi != DIR_STOP && abs(data.values[1].btValueHiHi -nDir ) >=3) //这里是转向了，在这里停留几秒
				{
					 //停留几秒再走
					if( data.wAuxInfo4  >  data.wAuxInfo3 )
					{
						data.wAuxInfo = data.wAuxInfo3 + (WORD)wrand( data.wAuxInfo4  -data.wAuxInfo3 );
					}
					else
					{
						data.wAuxInfo = data.wAuxInfo3;
					}
					
					data.values[1].btValueHiHi = (BYTE)nDir;
					data.bAuxInfo =0;
					nDir = DIR_STOP;
					return true;
				}
				else
				{
					data.values[1].btValueHiHi = (BYTE)nDir;
				}
				
			}
			return true;

		}
		else
		{
			nDir = DIR_STOP;
			return true;
		}

	}

}


void CMovementSystem::MoveRandomFlee(INT_PTR nMaxDist, int nMinTime, int nMaxTime, bool bStartImmediately)
{
	ClearMotionStack();
	int nPosX, nPosY;
	CScene *pScene = m_pEntity->GetScene();
	if(!pScene) return;
	m_pEntity->GetPosition(nPosX, nPosY);
	MotionData md;
	md.bMotionType = mtMotionTypeRandomFlee;
	md.values[1].wValueLo		= (WORD)(nMaxDist * nMaxDist);
	md.bAuxInfo					= (BYTE) DIR_STOP;	
	md.wAuxInfo4				= MAKEWORD(nMinTime, nMaxTime);
	if (!bStartImmediately)
	{
		unsigned int range = nMaxTime - nMinTime + 1;
		if (range <= 0) range = 1;
		md.wAuxInfo = (nMinTime + range) * 2;
	}	
	else
		md.wAuxInfo = 0;
	md.values[0].wLo			= (WORD)nPosX;
	md.values[0].wHi			= (WORD)nPosY;
	md.wAuxInfo2				= -1;
	md.wAuxInfo3				= -1;

	m_motionStack.PushState(md);
}

//巡逻移动
void CMovementSystem::MovePatrol(INT_PTR nRadius ,INT_PTR nMinSleepTime , INT_PTR nMaxSleepTime )
{
	INT_PTR nPosX, nPosY;
	CScene * pScene = m_pEntity->GetScene();
	if(pScene ==NULL) return;
	
	if(m_pEntity->GetType() != enMonster) return;
	 
	((CMonster*)m_pEntity)->GetBornPoint(nPosX,nPosY); //获得出生点

	MotionData data;
	data.bMotionType =mtMotionTypePatrol; //巡逻移动
	data.values[0].wLo  = (WORD) nPosX; //表示巡逻中心点
	data.values[0].wHi =  (WORD) nPosY; 
	
	data.values[1].wValueLo = (WORD)(nRadius * nRadius); //移动的最大的距离的平方，记录这个用于降低运算量

	data.values[1].btValueHiLo =(BYTE)nRadius;    //移动最大的半径
	
	if(nMaxSleepTime >nMinSleepTime )
	{
		data.wAuxInfo = (WORD)( (int)nMinSleepTime + (int)wrand((int)(nMaxSleepTime-nMinSleepTime)) ); //停止的秒数
	}
	else
	{
		data.wAuxInfo = (WORD) nMinSleepTime;
	}
	
	data.bAuxInfo =0;  //开始的运动状态为停止
	data.values[1].btValueHiHi=(BYTE) DIR_STOP;  //开始不设方向

	data.wAuxInfo3 = (WORD)nMinSleepTime;
	data.wAuxInfo4 = (WORD)nMaxSleepTime;

	int nCurrentPosX = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int nCurrentPosY = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	
	m_motionStack.PushState(data);

	if( nCurrentPosX != nPosX ||  nCurrentPosY != nPosY)
	{
		MoveTargetedHome(); //如果和老窝位置不对，先归巢再巡逻
	}
}

//逃跑
bool CMovementSystem::OnFleeMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{
	CScene * pSence = m_pEntity->GetScene();
	if(pSence ==NULL) return false;

	CEntity * pEntity = GetEntityFromHandle(EntityHandle(data.entityHandle));
	if(pEntity ==NULL) return false; //追逐目标已经下线
	if(pEntity->GetSceneID() != m_pEntity->GetSceneID())  return false; //和自己已经不是一个场景了
	//目标的坐标
	int nTargetPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int nTargetPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	
	//往相反的方向移动
	nDir = CEntity::GetDir(nTargetPosX,nTargetPosY, nCurrentPosX, nCurrentPosY);
	nStep =2;
	
	if( GetFanAvailablePos(nCurrentPosX,nCurrentPosY,nDir,nStep) )
	{
		return true;
	}
	else
	{
		//已经无路可退了，就结束逃跑的模式
		nDir = DIR_STOP;
		return false;
	}
}

bool CMovementSystem::OnRandomFlee(MotionData &data, INT_PTR nCurrPosX, INT_PTR nCurrPosY, INT_PTR &nDir, INT_PTR &nStep)
{
	CScene *pSence = m_pEntity->GetScene();
	if(!pSence) return false;

	// 每次走的距离配置在ai，借用'巡逻最大半径'配置项.
	// data.values[1].wValueLo 巡逻半径的平方;	data.values[1].btValueHiLo 巡逻最大半径	
	// data.wAuxInfo：巡逻停止间隔，用间隔次数来控制。
	// data.values[0].wLo：最近这次逃跑位置X
	// data.values[0].wHi：最近这次逃跑位置Y
	// data.bAuxInfo：记录上一次的运动方向
	if (data.wAuxInfo > 0)					// 当前处于Idle状态
	{
		data.wAuxInfo--;
		data.values[0].wLo = (WORD)nCurrPosX;
		data.values[0].wHi = (WORD)nCurrPosY;
		data.wAuxInfo2 = -1;
		data.wAuxInfo3 = -1;
		nDir = DIR_STOP;
		return true;
	}
	
	// 如果距离没到最大距离。如果是逃跑的开始，需要记录开始逃跑的位置	
	INT_PTR nDistance = GetDisSqare(nCurrPosX, nCurrPosY, data.values[0].wLo,data.values[0].wHi);
	if (nDistance >= data.values[1].wLo)		
	{		
		int nMinStopTime = LOBYTE(data.wAuxInfo4);
		int nMaxStopTime = HIBYTE(data.wAuxInfo4);
		int range = nMaxStopTime - nMinStopTime + 1;
		if (range <= 0) range = 1;
		data.wAuxInfo = (WORD)((wrand(range) + nMinStopTime) * 2);
		nDir = DIR_STOP;
		return true;
	}

	nDir = data.bAuxInfo;
	nStep = 1;
	INT_PTR nNewDir = nDir;	
	//if (GetFanAvailablePosEx(nCurrPosX, nCurrPosY, nNewDir, nStep))
	if (GetNexAvailablePos(nCurrPosX, nCurrPosY, nNewDir, data.wAuxInfo2, data.wAuxInfo3))
	{		
		data.bAuxInfo = (BYTE)nNewDir;
		nDir = nNewDir;
		return true;
	}
	else
	{
		int nMinStopTime = LOBYTE(data.wAuxInfo4);
		int nMaxStopTime = HIBYTE(data.wAuxInfo4);
		int range = nMaxStopTime - nMinStopTime + 1;
		if (range <= 0) range = 1;
		data.wAuxInfo = (WORD)(wrand(range) + nMinStopTime);		
		nDir = DIR_STOP;		
		return true;
	}

	return true;
}


//被蛊惑乱移动
void CMovementSystem::MoveConfused(INT_PTR nSeconds)
{
	MotionData data;
	data.bMotionType =mtMotionTypeConfused;
	
	data.nExpiredTime = GetGlobalLogicEngine()->getTickCount() + nSeconds *1000; //过期时间
	m_motionStack.PushState(data);
}


//迷惑
bool CMovementSystem::OnConfusedMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{
	if(GetGlobalLogicEngine()->getTickCount() >= data.nExpiredTime) //已经过期了
	{
		return false;
	}
	data.nExpiredTime -= m_pEntity->GetProperty<unsigned>(PROP_CREATURE_MOVEONESLOTTIME); //迷惑的时间减去一个周期
	
	INT_PTR nSeed= wrandvalue() %3;
	if(nSeed == 0)
	{
		nDir = DIR_STOP;
	}
	else
	{
	
		INT_PTR nPosX = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
		INT_PTR nPosY = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);
		nDir = m_pEntity->GetProperty<int>(PROP_ENTITY_DIR);
		if(nSeed ==1)
		{
			nDir = (nDir +4)%8; //取反
		}
		nStep =2;
		if(! GetFanAvailablePos(nPosX,nPosY,nDir,nStep))
		{
			nDir = DIR_STOP;
		}
	}
	return true;
}


//不移动
void CMovementSystem::MoveIdle()
{
	MotionData data;
	data.bMotionType =mtMotionTypeIdle;

	m_motionStack.PushState(data);
}

//归位
void CMovementSystem::MoveTargetedHome()
{
	//这里要取怪物的出生点,注意修改
	INT_PTR nPosX, nPosY;
	CScene * pScene = m_pEntity->GetScene();
	if(pScene ==NULL) return;

	if(m_pEntity->GetType() != enMonster) return; //只有怪物才有回归点
	((CMonster*)m_pEntity)->GetBornPoint(nPosX,nPosY);
	MotionData data;
	data.bMotionType =mtMotionTypeReturnHome;
	data.values[0].wLo  = (WORD) nPosX;
	data.values[0].wHi =  (WORD) nPosY;
	m_motionStack.PushState(data);
	
}

void CMovementSystem::MoveNoReturnHome(INT_PTR nRadius,INT_PTR nMinSleepTime, INT_PTR nMaxSleepTime)
{
	int nPosX, nPosY;
	CScene * pScene = m_pEntity->GetScene();
	if(pScene ==NULL) return;

	if(m_pEntity->GetType() != enMonster) return;

	m_pEntity->GetPosition(nPosX,nPosY);

	MotionData data;
	data.bMotionType =mtMotionTypePatrol; //巡逻移动
	data.values[0].wLo  = (WORD) nPosX; //表示巡逻中心点
	data.values[0].wHi =  (WORD) nPosY; 

	data.values[1].wValueLo = (WORD)(nRadius * nRadius); //移动的最大的距离的平方，记录这个用于降低运算量

	data.values[1].btValueHiLo =(BYTE)nRadius;    //移动最大的半径

	if(nMaxSleepTime >nMinSleepTime )
	{
		data.wAuxInfo = (WORD)( (int)nMinSleepTime + (int)wrand((int)(nMaxSleepTime-nMinSleepTime)) ); //停止的秒数
	}
	else
	{
		data.wAuxInfo = (WORD) nMinSleepTime;
	}

	data.bAuxInfo =0;  //开始的运动状态为停止
	data.values[1].btValueHiHi=(BYTE) DIR_STOP;  //开始不设方向

	data.wAuxInfo3 = (WORD)nMinSleepTime;
	data.wAuxInfo4 = (WORD)nMaxSleepTime;

	int nCurrentPosX = m_pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int nCurrentPosY = m_pEntity->GetProperty<int>(PROP_ENTITY_POSY);

	m_motionStack.PushState(data);
}

//跟随
void CMovementSystem::MoveFollow(CAnimal* pTarget,INT_PTR nMinDistance, unsigned int nMaxDist, bool bStopWhenFailed)
{
	if(pTarget ==NULL) return;
	
	CAnimalAI *pAI = m_pEntity->GetAI();
	if (pAI)
		pAI->SetFollowStatus(true);

	MotionData data;
	data.bAuxInfo = bStopWhenFailed ? 1 : 0;				// 是否跟随失败停止跟随标记。为1表示为跟随失败停止跟随
	data.wAuxInfo = (BYTE)(nMinDistance * nMinDistance);	// 保持的距离
	data.wAuxInfo2 =(WORD) (nMaxDist * nMaxDist);			// 跟随最大距离距离
	data.wAuxInfo3 = 0;										// 是否跟随。如果为1表明是不跟随；否则跟随
	data.wAuxInfo4 = 0;										// 如果当前不跟随（跟随失败）定期检测自己和目标的距离，如果达到距离重新跟随
	data.entityHandle = pTarget->GetHandle();
	data.bMotionType = mtMotionTypeFollow;
	m_motionStack.PushState(data);
}

//跟随
bool CMovementSystem::OnFollowMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{	
	CEntity * pEntity = GetEntityFromHandle(EntityHandle(data.entityHandle)); //目标的实体
	m_useAStar =true;//跟随型的全部使用a*算法
	
	if(pEntity ==NULL) return false; //追逐目标已经下线
	if (data.wAuxInfo3 == 1)   // 设定了不跟随标记
	{
		if (data.wAuxInfo4 > 0)
		{
			data.wAuxInfo4--;
			return false;
		}
		else
		{
			// 检测被跟随者和自己的距离
			bool bInFollowDist = false;
			int nTargetPosX, nTargetPosY;
			if (pEntity->GetSceneID() == m_pEntity->GetSceneID())
			{
				nTargetPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
				nTargetPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
				INT_PTR nDistance = GetDisSqare(nCurrentPosX, nCurrentPosY, nTargetPosX, nTargetPosY);
				if (nDistance < data.wAuxInfo2)
				{
					bInFollowDist = true;
				}
			}
			
			if (bInFollowDist)
			{
				data.wAuxInfo4 = 0;
				data.wAuxInfo3 = 0;  // 继续跟随
				//OutputMsg(rmNormal, _T("------重新开始跟随"));
				CalculateMoveParam( nCurrentPosX,nCurrentPosY,nTargetPosX,nTargetPosY,nDir,nStep,data.wAuxInfo);
				return true;
			}
			else
			{
				data.wAuxInfo4 = 5;
				//OutputMsg(rmNormal, _T(" 当前仍然不在一个跟随距离内，继续等待"));
				return false;
			}
		}
	}
	if(pEntity->GetSceneID() != m_pEntity->GetSceneID() && data.bAuxInfo == 1)  //和自己已经不是一个场景了
	{
		data.wAuxInfo3 = 1;
		data.wAuxInfo4 = 5;		// 倒计时，在不跟随状态下，每次更新都减1，直到为0，就检测一下和目标是否在相同的场景和规定的跟随距离
		return false;		
	}

	//目标的坐标
	int nTargetPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int nTargetPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	INT_PTR nTargetDir = (pEntity->GetProperty<int>(PROP_ENTITY_DIR) + 4) & 7; //玩家反方向
	INT_PTR nNewPox = nTargetPosX, nNewPoy = nTargetPosY;
	CEntityGrid::NewPosition(nNewPox,nNewPoy,nTargetDir,1);
	INT_PTR nDistance = GetDisSqare(nCurrentPosX, nCurrentPosY, nNewPox, nNewPoy);
	if (nDistance > data.wAuxInfo2 && data.bAuxInfo == 1)
	{	
		data.wAuxInfo3 = 1;
		return false;
	}	

	CalculateMoveParam( nCurrentPosX,nCurrentPosY,nNewPox,nNewPoy,nDir,nStep,data.wAuxInfo);
	return true;
}


//追逐
void CMovementSystem::MoveChase(CAnimal * pTarget,INT_PTR nMinDistance,INT_PTR nMaxDistance  )
{
	if(pTarget ==NULL) return;
	if (m_pEntity->GetType() == enMonster && ((CMonster *)m_pEntity)->GetAttriFlag().MoveTelport)
	{
		int nMyPosX = 0, nMyPosY = 0;
		int nTargetPosX = 0, nTargetPosY = 0;
		pTarget->GetPosition(nTargetPosX, nTargetPosY);
		INT_PTR nDis = GetDisSqare(nMyPosX, nMyPosY, nTargetPosX, nTargetPosY);
		if (nDis > 10)
		{
			InstantMove(nTargetPosX, nTargetPosY-1);
			return;	
		}	
			
	}

	MotionData data;
	data.entityHandle = pTarget->GetHandle();
	data.bMotionType =mtMotionTypeChase;
	data.bAuxInfo = (BYTE) (nMinDistance * nMinDistance);   //最小追逐半径
	data.wAuxInfo = (WORD) (nMaxDistance * nMaxDistance);  //最大的追逐半径
	m_motionStack.PushState(data);
}

//追逐目标
bool CMovementSystem::OnChaseMove(MotionData & data,INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR & nStep)
{

	CEntity * pEntity = GetEntityFromHandle(EntityHandle(data.entityHandle));
	if(pEntity ==NULL) return false; //追逐目标已经下线
	if(pEntity->GetSceneID() != m_pEntity->GetSceneID())  return false; //和自己已经不是一个场景了
	//目标的坐标
	int nTargetPosX = pEntity->GetProperty<int>(PROP_ENTITY_POSX);
	int nTargetPosY = pEntity->GetProperty<int>(PROP_ENTITY_POSY);
	
	INT_PTR nTagetType = pEntity->GetType();
	//计算追逐
	return CalculateMoveParam( nCurrentPosX,nCurrentPosY,nTargetPosX,nTargetPosY,nDir,nStep,data.bAuxInfo,data.wAuxInfo);
}



//逃跑
void CMovementSystem::MoveFleeing(CAnimal * pTarget)
{
	if(pTarget ==NULL) return;
	MotionData data;
	data.entityHandle  = pTarget->GetHandle();
	data.bMotionType =mtMotionTypeFleeing;
	m_motionStack.PushState(data);
}

//往一个点移动
void CMovementSystem::MovePoint(INT_PTR nPosX,INT_PTR nPosY)
{
	MotionData data;
	data.bMotionType =mtMotionTypePoint;
	data.values[0].wLo  = (WORD) nPosX;
	data.values[0].wHi =  (WORD) nPosY;
	m_motionStack.PushState(data);
}

//在一个扇形区域里找一个能运动到的点，可以是1步到，也可以是2步到,优先2步到
bool CMovementSystem::GetFanAvailablePos(INT_PTR nCurrentPosX,INT_PTR nCurrentPosY, INT_PTR &nDir,INT_PTR &nStep)
{
	/*
		----->
		从这个方向开始寻找一个能到的点，算法就是以这个方向出发寻找，按扇形依次向外寻找可用点
		如果寻找到了，就返回，保证是这个方向上的优先
	*/

	CScene * pSence = m_pEntity->GetScene();
	if(pSence ==NULL) return false;

	
	INT_PTR nNewPosX,nNewPosY,nNewDir,nMaxStep =nStep;
	if(nMaxStep ==0) nMaxStep =1;

	if(nDir == DIR_STOP)
	{
		nDir = (++ nDir )% DIR_STOP;
	}
	INT_PTR nEntityType = m_pEntity->GetType();
	INT_PTR nFinalStep =-1; //最终的移动的步伐，如果找到了一个，就返回
	for(INT_PTR i=1 ;i <= nMaxStep; i++)
	{
		//先优先在这个方向找
		nNewPosX = nCurrentPosX;
		nNewPosY=  nCurrentPosY;
		if(nDir == DIR_STOP) continue;
		CEntityGrid::NewPosition(nNewPosX,nNewPosY,nDir,i);  //再取一次坐标
		if( pSence->CanMove(nNewPosX,nNewPosY) ) //如果1格不能移动的话，那么2格也不需要尝试了，中间有间隔
		{
			if (nEntityType == enMonster && !pSence->CanCross(nEntityType,nNewPosX, nNewPosY, false, false))
			{
				break;
			}
			nFinalStep =i;
		}
		else
		{
			break;
		}
	}
	
	//如果在该方向找到了，就去寻找
	if(nFinalStep >=1)
	{
		nStep = nFinalStep;
		return true;
	}

	//扇形查找的话，找到1格就够
	for(INT_PTR i=1; i<= nMaxStep; i++)
	{
		for(INT_PTR j=1; j <4; j++)
		{
			for(INT_PTR k=-1;k<=1; k++)
			{
				if(k ==0 || (j==4 && k==1)) continue; //在扇形的区间查找
				nNewPosX = nCurrentPosX;
				nNewPosY=  nCurrentPosY;
				nNewDir = (nDir + j*k +DIR_STOP)%DIR_STOP; //新的方向
				if(nNewDir ==DIR_STOP) continue;

				CEntityGrid::NewPosition(nNewPosX,nNewPosY,nNewDir,i);  //
				if(pSence->CanMove(nNewPosX,nNewPosY) )
				{
					if(nEntityType == enMonster && !pSence->CanCross(nEntityType,nNewPosX, nNewPosY,false, false))
					{
						continue;
					}
					nDir = nNewDir;
					nStep = i;
					return true;
				}
			}
		}
	}
	
	return false;
}

bool CMovementSystem::GetNexAvailablePos(INT_PTR nCurrentPosX, INT_PTR nCurrentPosY, INT_PTR &nDir, INT_PTR nPrevPosX, INT_PTR nPrevPosY)
{
	CScene *pSence = m_pEntity->GetScene();
	if (!pSence) return false;
		
	INT_PTR nNewPosX, nNewPosY;	
	// 随机找一个位置，并且保证不能和PrevPos一样
	CVector<int> vecFlag;
	vecFlag.reserve(DIR_STOP);
	for (INT_PTR i = 0; i < DIR_STOP; i++)
	{
		nNewPosX = nCurrentPosX;
		nNewPosY = nCurrentPosY;		
		CEntityGrid::NewPosition(nNewPosX, nNewPosY, i, 1);
		if ((nNewPosX != nPrevPosX || nNewPosY != nPrevPosY) && pSence->CanMove(nNewPosX, nNewPosY))
			vecFlag.add((int)i);
	}
	
	if (vecFlag.count() > 0)
	{
		nDir = vecFlag[wrand((unsigned long)(vecFlag.count()))];		
		return true;
	}

	return false;
}

/*
bool CMovementSystem::Get90AngleAvaibleDir(CScene *pScene,INT_PTR nCurretnPosX, INT_PTR nCurrentPosY,INT_PTR nDesPosX,INT_PTR nDesPosY,INT_PTR &nDir,bool bOnlyCheckCrossPoint )
{
	bool isCrossPoint;
	int nDirMask;

	if( pScene->CanMove(nCurretnPosX,nCurrentPosY,isCrossPoint,nDirMask) )
	{
		//如果只是检测交叉点
		if( bOnlyCheckCrossPoint && !isCrossPoint)
		{
			nDir= DIR_STOP;
			return true;
		}
		
		INT_PTR nTargetDir = CEntity::GetDir(nCurretnPosX,nCurrentPosY, nDesPosX, nDesPosY);
		if( (1 << nTargetDir) & nDirMask ) //如果当前点有路可以走，就沿着这个路走
		{
			nDir= nTargetDir;
			return true;
		}
		if(! bOnlyCheckCrossPoint)
		{
			nDir= DIR_STOP;
			return true;
		}
		//在当前的方向和旁边的2个方向查找一个能够移动的电
		
		nDir = ( nTargetDir +1) % DIR_STOP;
		if( (1 << nDir) & nDirMask ) //如果当前点有路可以走，就沿着这个路走
		{
			return true;
		}
		nDir = ( nTargetDir -1) % DIR_STOP;
		if( (1 << nDir) & nDirMask ) //如果当前点有路可以走，就沿着这个路走
		{
			return true;
		}
		

		nDir= DIR_STOP;
		return true;
	}
	else
	{
		return false;
	}
}
*/

/*

INT_PTR  CMovementSystem::GetNextMoveDir(INT_PTR nCurretnPosX, INT_PTR nCurrentPosY,INT_PTR nDesPosX,INT_PTR nDesPosY,INT_PTR nServerchRadius)
{
	CScene *pSence = m_pEntity->GetScene();
	if (!pSence) return DIR_STOP;
	

	bool isCrossPoint;
	int nDirMask;
	
	if( !pSence->CanMove(nCurretnPosX,nCurrentPosY,isCrossPoint,nDirMask) )
	{
		return DIR_STOP ;
	}
	INT_PTR nTargetDir = CEntity::GetDir(nCurretnPosX,nCurrentPosY, nDesPosX, nDesPosY);
	if(nTargetDir ==DIR_STOP)
	{
		return DIR_STOP;
	}
	if( (1 << nTargetDir) & nDirMask ) //如果当前点有路可以走，就沿着这个路走
	{
		return nTargetDir;
	
	}
	
	
	INT_PTR nTestDir; //测试的方向
	for(INT_PTR i=1;i <= 2 ;i ++)
	{
		for(INT_PTR k=-1;k <=1 ; k++)
		{
			if(k==0) continue;
			nTestDir = i*k + 	nTargetDir;

			if(nTestDir >= DIR_STOP)
			{
				nTestDir -= DIR_STOP;
			}
			else if(nTestDir < 0  )
			{
				nTestDir += DIR_STOP;
			}
			if( ((1 << nTestDir) & nDirMask) ==0 ) //这个方向不可以移动
			{
				continue;
			}
			INT_PTR nStepX=0,nStepY=0;
			switch ( nTestDir)
			{
			case DIR_UP:
				nStepY =-1;
				break;
			case DIR_UP_RIGHT:
				nStepX =1;
				nStepY =-1;
				break;
			case DIR_RIGHT:
				nStepX =1;
				break;
			case DIR_DOWN_RIGHT:
				nStepX =1;
				nStepY =1;
				break;
			case DIR_DOWN:
				nStepY =1;
				break;
			case DIR_DOWN_LEFT:
				nStepX =-1;
				nStepY =1;
				break;
			case DIR_LEFT:
				nStepX =-1;
				break;
			case  DIR_UP_LEFT:
				nStepX =-1;
				nStepY = -1;
				break;
			default:
				break;
			}
			int nTestDirMask;
			bool isTestPointCrossPoint;
			for(INT_PTR j=1; j< nServerchRadius; j++)
			{

				if(! pSence->CanMove(nCurretnPosX + j * nStepX,nCurrentPosY + j* nStepY,isTestPointCrossPoint,nTestDirMask) )
				{
					break;
				}
				else
				{
					if(isTestPointCrossPoint )
					{
						return nTestDir;
					}
				}
			}
		}
		
		
	
	}

	return DIR_STOP;
	
}
*/

int CMovementSystem::AutoFindPathToNpc( int nSceneId, char * sNpcName )
{
	CActorPacket pack;
	CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)GetSystemID() << (BYTE)sAutoPath ;
	data << (int) nSceneId;
	data.writeString(sNpcName);
	pack.flush();
	return 0;
}
