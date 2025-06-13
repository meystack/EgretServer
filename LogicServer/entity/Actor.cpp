#include "StdAfx.h"
#include "Actor.h"
#include "../script/interface/LogDef.h"
#include "../base/Container.hpp" 
#include "config/HallowsConfig.h"

using namespace jxInterSrvComm;

// 时间波动，用于每天24点定时分散用
#define TIME_FLUCTUATE	60  

#define LEVLMAILVAR   "Level_%d_MailVar"

//静态函数，用于初始化更新的指针
 //那些属性需要发送到客户端
 CUpdateMask*					CActor::s_actorBroadcastMask = NULL;
 char 							CActor::s_actorForbitMask[CUpdateMask::MAX_MASK_BYTE_COUNT];
 CVector<MessageBoxItem>*		CActor::MessageBoxList = NULL;

 CVector<CUserItem*>* CActor::s_itemDropPtrList =NULL;						//掉落装备列表
 CVector<BYTE>* CActor::s_itemDropTypeList =NULL;

 int							CActor::MsgBox_Id = 0;

 int                            CActor::s_nTraceAccountId =0; //追踪的玩家的账户的ID,-1表示所有的都打印,0表示不打打印，大于0表示只打印特定的
 int 	CActor::m_sSaveDBInterval = 180000;
 int    CActor::s_nKickQuickUserCount =3;  //踢玩家下
 float   CActor::s_fKickQuickRate =0.12f;  //阈值
 int	CActor::s_nCheckQuickCountKick = 0;
 bool	CActor::s_boCheckQuickSealActor = false;
 int	CActor::s_nQuickBuffId = 0;		//buffId默认值


 char CUpdateMask::s_forbitUpdateMask[CUpdateMask::MAX_MASK_BYTE_COUNT];//????CUpdateMask中的定义为什么写到这里来？？？


CActor::CActor()
	//:Inherited()
	:CAdvanceAnimal()
{
	m_nUserSocket =0;  //SOCKET
	m_nGateID =0; //LogicGate服务器的编号
	m_nGateSessionIndex=0; //在网关上的编号
	m_nServerSessionIndex =0; //在logicGate服务器上的索引
	m_nAccountID=0;    //帐户的ID
	m_nEnterFbScenceID=0; //进入副本前的位置
	m_nEnterFbPosx=0;    //进入副本前x的坐标
	m_nEnterFbPosy=0;  //进入副本前的y的坐标
	m_dwClientTicksValue =0;
	m_nRecvHeatbeatTickCount =0;
	m_sAccount[0] =0;
	m_wInitLeftStep = eMSG_USR_LOGIN_DATA_MAX - 1;  //初始化的步骤，现在是16步，玩家数据，装备数据，技能数据, 脚本二进制数据(用于判断db数据是否正常装载完毕)  玩家计数器, 成就,玩家好友数据 活动 邮件系统,强化数据,神魔,生物
	m_isSuperman =false;
	m_nFcmTime =-1;
	//m_property.nForeverCardFlag = 0;

	/*
	*
	*	2021-08-20 火墙 暂时性修改

	m_deqLastFireSkillEntity.clear();

	*/
	m_needTraceMsg =false; 
	//ZeroMemory(&m_bBanneretLeftTimeRemind, sizeof(m_bBanneretLeftTimeRemind));
	m_netMsgPool.reserve(32);
	
	m_boCallScript = true;
	
	
	m_nCommOpTimes = 0;
	m_nOccuCommOpTime = 0;

	m_nLastSceneId = -1;
	m_nLastPosX = 0;
	m_nLastPosY = 0;
	m_nWholeAwards = 0;
	
	//m_nZyContriGetToday=0;	        // 角色当天获取的阵营贡献（存db）
	//m_nZyContriGetLastLogout=0;     // 角色上次下线那天杀怪获取的经验值
	
	//m_nExpGetToday =0;              // 角色当天获取的经验值（存db）
	//m_nExpGetLastLogout =0;         // 角色上次下线那天杀怪获取的经验值
	//m_fDrunkExpAdd =0.0;
	//m_fDrunkXiuweiAdd =0.0;
	m_bTracePacket = false;		
	m_nTransmitStatus = enTS_IDLE;
	m_nTransmitDestServerId = 0;

	ZeroMemory(&m_commsrvRelivePointPos,	sizeof(m_commsrvRelivePointPos));
	ZeroMemory(&m_commsrvFBRelivePointPos,	sizeof(m_commsrvFBRelivePointPos));
	ZeroMemory(&m_commsrvCityPointPos,		sizeof(m_commsrvCityPointPos));
	ZeroMemory(&m_commsrvPosBeforeEnterFB,	sizeof(m_commsrvPosBeforeEnterFB));		
	ZeroMemory(&m_rawsrcScenePos,			sizeof(m_rawsrcScenePos));	
	ZeroMemory(m_sIp,						sizeof(m_sIp));
	
	m_nRawServerIndex =0;
	m_nLastLoginServerId =0;
	m_InDartFlag = false;
	m_nSwingid = 0;
	m_nSwingLevel = 0;
	m_LonginDays = 0;
	
	m_AttackOthersTime = 0;
	m_nReincarnationTimes = 0;
	SetAttackLevel(100); //玩家的被攻击优先级为100
	m_SceneEffect.clear();
	m_useQuickSoftTime =0;  //使用加速外挂的次数
	m_nQuickBuffTimes = 0;
	m_nNotReloadMapSceneId = 0;
	m_CreateIndex = 0;
	m_RundomKey = 0;
	m_nUpdateTime = 0;
	m_nAtvTotalOnlineTime = 0;

	m_dwAutoAddExpTick = 0;
	m_nExploitRate  = 0;
	m_NextChangeVocTime = 0;
	m_nOldSrvId = 0;
	blongLists.empty();//归属列表
	m_wCrossInitLeftStep = MSG_CS_END;//同步跨服数据, 加载跨服行会数据（跨服行会数据是是本地加载的）
	m_bRebateCharge = false;
	//网络优化
	m_CanNetworkCutDown = false;
	m_nNewDayArrive = 0;

	m_IsTestSimulator = false;
	m_IsSimulator = false; 
 
}
//这里是玩家需要更新的属性（变化时通知客户端）
void CActor::InitActorBroadcastMask()
{
	memset(CUpdateMask::s_forbitUpdateMask,0,CUpdateMask::MAX_MASK_BYTE_COUNT);
	if (!s_actorBroadcastMask) 
	{
		s_actorBroadcastMask = new CUpdateMask();
	}
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_LEVEL);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MAXHP);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MAXMP);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_HP);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MP);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_WEAPONAPPEARANCE);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_SWINGAPPEARANCE);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ENTITY_MODELID);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ENTITY_ICON);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_STATE);//实体的状态
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_SOCIALMASK);//实体的社会关系状态
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_ATTACK_SPEED);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MOVEONESLOTTIME);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_ZY); //阵营改变下发

	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_FOREVER_CARD_CLAG);//免费特权(第0位)永久卡(第1位)首充会员(2位) 色卡(3-7)
 	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MONTH_CARD);//月卡到期时间
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MEDICINE_CARD);//大药月卡到期时间
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CARD_AWARD_FLAG);//免费特权(第0位)/月卡(第1位)/大药(第2位)/永久卡(第3位)的当日领取标记

	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CURNEWTITLE); 
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_TEAM_ID); 
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_COLOR);				//名字的颜色
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CIRCLE);					//转数
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CIRCLE_SOUL);			//转生灵魂
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MONSTER_MODELID);		//怪物模型
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_SOLDIERSOULAPPEARANCE);	//兵魂外观
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_WEAPON_ID);				//正在装备的武器物品id
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CRIT_RATE);				///暴击几率
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CRIT_POWER);				//暴击伤害
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_DEDUCT_CRIT);			//抗暴
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_DEDUCT_DAMAGE);			//伤害减免
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_ABSORB_DAMAGE);			//伤害吸收
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_TEAMFUBEN_TEAMID);		//团队副本(团队ID)
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_TEAMFUBEN_FBID);			//团队副本(副本ID)
	//s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_LASTLOGIN_OPEN_DAY_NUM);		//上次登录开服天数记录
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GUILD_ID);				//行会ID
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GUILD_LEVEL);			//行会Level
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_PERSONBOSS_JIFEN);		//boss积分
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_DRAW_YB_COUNT);			//玩家总的充值金额
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MERITORIOUSPOINT);		//功勋点
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_SUPPER_PLAY_LVL);		//超玩vip等级
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_NEXT_SKILL_FLAG);		//战士技能标记
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MEDIC_RENEW);			//药品恢复倍率增益（1为100%）
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_FLYSHOES);			//飞鞋
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_BROATNUM);			//喇叭
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_RECYCLE_POINTS);			//回收积分
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_PK_VALUE);			//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_EVIL_PK_STATUS);			//恶意pk状态
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_AREA_ATTR);
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MERIDIALv);              //经脉等级
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_TRADINGQUOTA);           //交易额度
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_HP_RENEW);           //HP值恢复
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_MP_RENEW);           //MP值恢复
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_MULTI_EXP_UNUSED);    	//多倍经验剩余
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GUILDEXP);    	//玩家个人当前的贡献度
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_CREATURE_DIZZY_STATUS);    	//麻痹
	//神装
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR1);    	//降低受战士伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR3);    	//降低受道士伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR2);    	//降低受法师伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR4);    	//降低受怪物伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR5);    	//增加对战士伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR6);    	//增加对法师伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR7);    	//增加对道士伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_GOLDEQ_ATTR8);    	//增加对怪物伤害值
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_HP_2_MP_DAMAGE);    	//护身

	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_DAMAGEBONUS);    	//伤害加成
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_IGNORDEFENCE);    	//无视防御
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_SUCKBLOOD);    	//吸血
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_LOOTBINDCOIN);    	//金币倍率
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_EXP_POWER);    	//经验倍率
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CUT);    	//切割

	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_BANCHAT);    	//禁言时间
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_PET_STATUS);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_NORMAL_ATK_ACC);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_HP_RATE);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_FRENZY);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_PK_DEDUCT_DAMAGE);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_OFFICE);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_DEPORT_GRID_COUNT);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_RECOVERSTATE);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_ZL_MONEY);    	//
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_LOOT_PET_ID);    //
	s_actorBroadcastMask->SetUpdateMaskFlag(PROP_ACTOR_CURCUSTOMTITLE);    //
	
	//玩家不需要更新的属性
	memset(CActor::s_actorForbitMask,0,CUpdateMask::MAX_MASK_BYTE_COUNT);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ENTITY_DIR,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ENTITY_POSX,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ENTITY_POSY,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ACTOR_TOTAL_ONLINE_MIN,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ACTOR_BE_KILLED_COUNT,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ACTOR_KILL_MONSTER_COUNT,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ACTOR_CRIT_DAMAGE,true);
	CUpdateMask::SetForbidUpdate(s_actorForbitMask, PROP_ACTOR_LASTLOGIN_OPEN_DAY_NUM,true);	

	//这些属性不需要更新
	//CUpdateMask::SetForbidUpdate(PROP_ENTITY_DIR,true);
	CUpdateMask::SetForbidUpdate(PROP_ENTITY_POSX,true);
	CUpdateMask::SetForbidUpdate(PROP_ENTITY_POSY,true);

	CUpdateMask::SetForbidUpdate(PROP_ACTOR_TOTAL_ONLINE_MIN,true);
	CUpdateMask::SetForbidUpdate(PROP_ACTOR_BE_KILLED_COUNT,true);
	CUpdateMask::SetForbidUpdate(PROP_ACTOR_KILL_MONSTER_COUNT,true);
	CUpdateMask::SetForbidUpdate(PROP_ACTOR_CRIT_DAMAGE,true);
	CUpdateMask::SetForbidUpdate(PROP_ACTOR_LASTLOGIN_OPEN_DAY_NUM,true);
}


void  CActor::Destroy()
{
	//删除一些没有处理的网络消息	
	CEntityManager * pMgr = GetGlobalLogicEngine()->GetEntityMgr();	
	CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(m_nGateID);
	
	/*CDataPacket *pMsg;
	CList<CDataPacket*>::Iterator it(m_netMsgPool);
	CList<CDataPacket*>::NodeType *pNode;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if(logicgate)
		{
			logicgate->AddLogicProcessedData(pMsg);
		}		
	}
	m_netMsgPool.clear();*/
	if (logicgate)
		logicgate->AddLogicProcessedDataList(m_netMsgPool);
	m_netMsgPool.clear();

	//这里是玩家的一些内存的释放工作
	m_GameStoreBuySystem.Destroy();
	m_DealSystem.Destroy();
	m_teamSystem.Destroy();
	m_guildSystem.Destroy();
	m_varSystem.Destroy();
	m_heroSystem.Destroy();

	m_Bag.Destroy();
	m_Equiptment.Destroy();
	m_deportSystem.Destroy(); //
	m_miscSystem.Destroy();
	//m_MagicItemSystem.Destroy();

	Inherited::Destroy();
	m_sPacketStat.Clear();

}




//uParam1 nSocket
//uParam2 nActorID
//uParam3 nGateID
//uParam4 nGateSessionIndex
//uParam5 nServerSessionIndex

void CActor::Onlogin(UINT_PTR nSocket,UINT_PTR nActorID,UINT_PTR nGateID, 
	UINT_PTR nGateSessionIndex,UINT_PTR nServerSessionIndex,
	UINT_PTR nAccountID,INT_PTR nFcmTime,INT_PTR nGmLevel, int nRawServerId, LONGLONG nClientIpAddr)
{
	m_nUserSocket = nSocket;
	//SetProperty<unsigned>( & m_property.nID, (unsigned)nActorID );
	SetProperty<unsigned>(PROP_ENTITY_ID, (unsigned int)nActorID);
	m_nGateID = (unsigned)nGateID; //所在的网关
	m_nGateSessionIndex  = (unsigned)nGateSessionIndex;  //在网关上的编号
	m_nServerSessionIndex = (unsigned)nServerSessionIndex; //在逻辑服务器上的编号
	m_nAccountID = (unsigned)nAccountID;
	m_nFcmTime = (int) nFcmTime;
	m_nGmLevel  = (int) nGmLevel; //玩家的GM
	m_lLastLoginIp = nClientIpAddr;
	SetRawServerIndex(nRawServerId);
	//GetLogicServer()->GetSessionClient()->PostChangeSessionState((int)nAccountID,jxSrvDef::gsInGame);
	//int nid = (int)nActorID;
	//GetLogicServer()->GetDbClient()->SendDbServerData(jxInterSrvComm::DbServerProto::dcQuery,nid);
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcQuery);
	packet << GetRawServerIndex() << GetLogicServer()->GetServerIndex() << (unsigned int)nActorID;
	if (!m_IsTestSimulator)
	{
		pDBClient->flushProtoPacket(packet);
	}
	GetGlobalLogicEngine()->GetEntityMgr()->OnActorLogin((unsigned int)nAccountID, (unsigned int)nActorID, GetHandle());
	
	//踢下同账号同服的跨服角色
	auto pCrossClient = GetLogicServer()->GetCrossClient();
	if(pCrossClient->connected()) {
		CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCloseActor);
		unsigned int nCrossServerSrvId = GetLogicServer()->GetServerIndex();
		packet << (BYTE)2 << (int)nCrossServerSrvId << nAccountID;
		pCrossClient->flushProtoPacket(packet);
	}
	OutputMsg(rmTip,_T("CActor::Onlogin, actorID=%u,accountID=%u,nGateID=%d,nSocket=%d"),
		nActorID,m_nAccountID,nGateID,nSocket);
}

void CActor::OnServerLogin(
	UINT_PTR nActorID,
	UINT_PTR nAccountID,
	INT_PTR nFcmTime,
	INT_PTR nGmLevel,
	int nRawServerId,
	LONGLONG nClientIpAddr)
{
	m_nUserSocket = -1; //不需要绑定
	//SetProperty<unsigned>( & m_property.nID, (unsigned)nActorID );
	SetProperty<unsigned>(PROP_ENTITY_ID, (unsigned int)nActorID);
	m_nGateID = -1; //所在的网关
	m_nGateSessionIndex  = -1;  //在网关上的编号
	m_nServerSessionIndex = -1;//在逻辑服务器上的编号
	m_nAccountID = (unsigned)nAccountID;
	m_nFcmTime = (int) nFcmTime;
	m_nGmLevel  = (int) nGmLevel; //玩家的GM
	m_lLastLoginIp = nClientIpAddr;
	SetRawServerIndex(nRawServerId);
	//GetLogicServer()->GetSessionClient()->PostChangeSessionState((int)nAccountID,jxSrvDef::gsInGame);
	//int nid = (int)nActorID;
	//GetLogicServer()->GetDbClient()->SendDbServerData(jxInterSrvComm::DbServerProto::dcQuery,nid);
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcQuery);
	packet << GetRawServerIndex() << GetLogicServer()->GetServerIndex() << (unsigned int)nActorID;
	if (!m_IsTestSimulator)
	{
		pDBClient->flushProtoPacket(packet);
	}
	GetGlobalLogicEngine()->GetEntityMgr()->OnActorLogin((unsigned int)nAccountID, (unsigned int)nActorID, GetHandle());
	
	//踢下同账号同服的跨服角色
	auto pCrossClient = GetLogicServer()->GetCrossClient();
	if(pCrossClient->connected()) {
		CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCloseActor);
		unsigned int nCrossServerSrvId = GetLogicServer()->GetServerIndex();
		packet << (BYTE)2 << (int)nCrossServerSrvId << nAccountID;
		pCrossClient->flushProtoPacket(packet);
	}
	OutputMsg(rmTip,_T("CActor::Onlogin, actorID=%u,accountID=%u"),
		nActorID,m_nAccountID);
}

//成功连击
void CActor::OnAttackOther(CAnimal * pEntity, bool bChgDura)
{
	Inherited::OnAttackOther(pEntity);  
	GetEquipmentSystem().OnAttackOther(pEntity, bChgDura);
	m_petSystem.OnAttackOther(pEntity);
	INT_PTR nType =  pEntity->GetType();
	CHero * pHero = GetHeroSystem().GetBattleHeroPtr();
	if(enMonster == nType)
	{
		int nMonId = pEntity->GetId();
		PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonId);
		if(pMonConf  &&  (pMonConf->btMonsterType ==tagMonsterConfig::mtBoss ||  pMonConf->btMonsterType ==tagMonsterConfig::mtElite 
		      ||  pMonConf->btMonsterType ==  tagMonsterConfig::mtToumu) )
		{
		    m_heroSystem.OnAttackOther(pEntity);
		}
	}else if(enActor == nType)
	{
	    m_heroSystem.OnAttackOther(pEntity);
	}
	
}
void CActor::OnAttacked(CAnimal * pEntity, bool bSetVest)
{	
	if(!HasState(esStateBattle))
	{
		AddState(esStateBattle);
		if(pEntity->GetType() == enActor)
		{			
			GetPkSystem().EnterPK((CActor*)pEntity);
		}
	}
	//OutputMsg(rmNormal, "CActor::OnAttacked: nDemage=%d", nDemage);
	Inherited::OnAttacked(pEntity);  
	GetEquipmentSystem().OnAttacked(pEntity);  //要掉装备耐久
	m_petSystem.OnAttacked(pEntity);			//宠物被攻击
	INT_PTR nType =  pEntity->GetType();
	CHero * pHero = GetHeroSystem().GetBattleHeroPtr();
	if(enMonster == nType)
	{
		int nMonId = pEntity->GetId();
		PMONSTERCONFIG pMonConf = GetLogicServer()->GetDataProvider()->GetMonsterConfig().GetMonsterData(nMonId);
		if(pMonConf  &&  (pMonConf->btMonsterType ==tagMonsterConfig::mtBoss ||  pMonConf->btMonsterType ==tagMonsterConfig::mtElite 
			||  pMonConf->btMonsterType ==  tagMonsterConfig::mtToumu) )
		{
			m_heroSystem.OnAttacked(pEntity);
		}
	}else if(enActor == nType)
	{
		m_heroSystem.OnAttacked(pEntity);
	}
}

VOID CActor::CloseActor(int nWayId, bool isGatewayLogout, int nLogoutServerId, bool bDestroyEntiy, bool bNeedDBAck)
{
	//首先要判断handle是否合法，防止网关发多个包来踢玩家下线引起问题
	CEntity* pSelf = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity( GetHandle());
	if(pSelf ==NULL) return;
	
	unsigned int nActorID = m_property.nID;

	//下线
	OutputMsg(rmTip, "[Close] 玩家下线 Actorname=%s :%d ActorId =%s", this->GetEntityName(), (int)isGatewayLogout, this->GetId());

	// 如果玩家在传送状态，那么在传送之前已经进行了存盘	
	if (this->IsInited() && m_nTransmitStatus < enTS_StartTransmit)
	{
		CMiniDateTime nCurrentTime;
		nCurrentTime.tv = GetGlobalLogicEngine()->getMiniDateTime();
		PreCloseActor();
		//删除定时器
		//DelTimer(eEntity500msTimer);
		//DelTimer(eEntityOneSecondTimer); 

		//这里修改一下，即使玩家的实体没有创建好的时候也应该执行下面的退出流程，但是DB不能存储，存储的话将出现存盘错误

		//if(m_isInited ==false) return;
		//SetInitFlag(false);

		//保存脚本数据,和其他的一些处理
		CScriptValueList paramList;
		//InitEventScriptVaueList(paramList,aeUserLogout);
		paramList << GetProperty<unsigned int>(PROP_ENTITY_ID);
		OnEvent(aeUserLogout, paramList,paramList);

		int ntotalTime = nCurrentTime.tv - m_nAtvTotalOnlineTime.tv;
		if(ntotalTime <0)
		{
			ntotalTime =0;
		}
		CScriptValueList paramLists;
		paramLists << this;
		paramLists << ntotalTime;
		GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("UpdateOnlineTime", paramLists, paramLists, 0);
		// 离开副本事件
		CFuBen* pFb = GetFuBen();
		CScene* pScene = GetScene();
		if (pScene && pFb && pFb->IsFb())
		{
			CScriptValueList paramList;
			paramList << (int)CFubenSystem::enOnExit;
			paramList << (int)pFb->GetConfig()->nFbConfType;
			paramList << (int)pFb->GetFbId();
			paramList << (int)pFb->GetConfig()->nFbEnterType;
			paramList << pFb;
			paramList << (int)pScene->GetSceneId();
			paramList << this;
			if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("FubenDispatcher","OnEvent", paramList, paramList, 1))
			{
				OutputMsg(rmError,"[FubenSystem] OnEvent OnExit 错误，副本ID=%d，副本类型=%d !",pFb->GetFbId(),pFb->GetConfig()->nFbConfType);
			}
		}
 
		GetGlobalLogicEngine()->GetSimulatorMgr()->OnRemoveRuningActor(this->GetId());

		m_nLastLogOutTime = nCurrentTime;

		int nOnlineTime =nCurrentTime.tv -  m_nLoginTime.tv;//在线时间
		if(nOnlineTime <0)
		{
			nOnlineTime =0;
		}

		//只有创建好的实体才能够存盘，没有创建好的玩家不能存盘
		if(m_isInited == true)
		{
			//下线存盘
			OutputMsg(rmTip, "[Close] 玩家下线save DB Offline Actorname = %s ", this->GetEntityName());
			
			if (!OnGetIsSimulator() && !OnGetIsTestSimulator())
			{
				SaveDb(true, nLogoutServerId, bNeedDBAck); //存盘
				SaveOfflinePropertyData();
				SaveCustomInfoData(); 
			}
		}
		if(OnGetIsSimulator() || OnGetIsTestSimulator())
		{
			//虚拟玩家下线　
			OutputMsg(rmTip, _T("[simulator], 下线 actorID = %u, Actorname = %s"), this->GetId(), this->GetEntityName());
		
			CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
			pSimuMgr->OnRemoveRuningActor(this->GetId());
		}
		m_friendSystem.FriendLoginOut();
		//摆摊系统
		//m_stallSystem.OnActorLogout(); 
		m_miscSystem.OnActorLoginOut(); //防沉迷
		m_fubenSystem.OnActorLoginOut();
		m_guildSystem.OnActorLoginOut();
		m_petSystem.OnActorLoginOut();
		//收回英雄
		m_heroSystem.CallbackBattleHero(false);
		int x,y;
		GetPosition(x,y);

		if (IsInited())
		{
			int nYb = GetProperty<int>(PROP_ACTOR_YUANBAO);
			int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
			if(GetLogicServer()->GetLogClient())
			{
				GetLogicServer()->GetLogClient()->SendLoginLog(ltLogout,GetAccountID(),GetAccount(),m_sIp,GetEntityName(),nLevel, nYb ,nOnlineTime, GetId(), getOldSrvId());
			}

			//记录日志
			if(GetLogicServer()->GetLocalClient())
			{
			}
			if(m_guildSystem.checkSbkGuildSeniorofficials())
			{
				// GetGlobalLogicEngine()->GetOfflineUserMgr().OnOfflineSynchronizationData(this);
				GetGlobalLogicEngine()->GetOfflineUserMgr().LoadUser(nActorID);
			}
		}
 
		//向好友服务器发送消息
		//GetLogicServer()->GetFriendClient()->PostUserLogonOut(this);
		RemoveMyMessageBox();
		if(IsInTransmitStatus() ==false && IsInRawServer() )
		{
			GetLogicServer()->GetSessionClient()->PostCloseSession(m_nAccountID, m_lKey); //session close
		}
		OutputMsg(rmTip,
				  _T("[%s][%d]CloseActor(bDestroyEntity=%d, actorID=%u, account=%d, onlinetime=%d, socket=%d"), 
				  GetEntityName(), 
				  nWayId,
				  (int)bDestroyEntiy,
				  (unsigned int)nActorID, 
				  m_nAccountID, 
				  nOnlineTime, 
				  (int)m_nUserSocket);
	}

	if (bDestroyEntiy)
	{	 
		//是网关关闭的话，就不需要
		if(isGatewayLogout == false && !m_IsTestSimulator)
		{
			OutputMsg(rmTip,_T("CActor::CloseActor, iskick,PostCloseUser, account=%d, socket=%d"),(int)m_nAccountID,(int)m_nUserSocket); 
			CLogicGate *pGate = (CLogicGate *)(GetLogicServer()->GetGateManager()->getGate(m_nGateID));
			if(pGate )
			{
				pGate->PostKickUser(m_nAccountID, m_lKey); //踢下线		
			}
		}
		else
		{
			//OutputMsg(rmTip,_T("CActor::CloseActor,PostCloseUser, account=%d, socket=%d"),m_nAccountID,(int)m_nUserSocket); 
		}
		OutputMsg(rmTip,_T("[%s]Logout, actorID=%u, account=%d, socket=%d"), GetEntityName(), (unsigned int)nActorID, m_nAccountID, (int)m_nUserSocket);
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(GetHandle()); //回收
	}
	else
	{
		OutputMsg(rmTip,"account=[%d]logout 不删除实体",m_nAccountID);
	}
}

#ifdef _DEBUG
LONG CActor::g_boPacketAlreadyAlloced = FALSE;
LPCSTR CActor::g_sPacketAllocFile = NULL;
INT_PTR CActor::g_nPacketAllocLine = 0;
#endif

#ifdef _DEBUG
CDataPacket&  CActor::_AllocPacket(CActorPacket &pack, LPCSTR file, INT_PTR line)
#else
CDataPacket&  CActor::AllocPacket(CActorPacket &pack)
#endif
{
#ifdef _DEBUG
	Assert(GetCurrentThreadId() == GetLogicServer()->GetLogicEngine()->getThreadId());
	Assert(FALSE == InterlockedCompareExchange(&g_boPacketAlreadyAlloced, TRUE, FALSE));
	g_sPacketAllocFile = file;
	g_nPacketAllocLine = line;
#endif
	//CDataPacket* pSendToGatePacket = NULL;
 
	if(OnGetIsSimulator() || OnGetIsTestSimulator())
	{
		//需要虚拟的数据  
		CDataPacket *SimulaPack = new CDataPacket((char *)&m_cbSimulatorData[0], sizeof(m_cbSimulatorData)); 
		pack.packet = SimulaPack;
		OutputMsg(rmTip, _T("[simulator], 有数据需要屏蔽！") );
	}
	else
	{
		CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(m_nGateID);
		pack.packet = logicgate->GetSendToGatePacket();
	}
	//Assert(&pack.packet);
	//OutputMsg(rmTip,"AllocPacket =%d",pack.packet->getOffsetPtr());
	if(!m_isInited)
	{
#ifdef _DEBUG
	OutputMsg(rmError,"AllocData when actor not inited,file=%s,line=%d",file,(int)line);
#else
	OutputMsg(rmError,"AllocData when actor not inited");
#endif
		//如果是调试版本直接挂掉服务器
	}
	pack.packet->reserve(pack.packet->getPosition()+sizeof(GATEMSGHDR));

	PGATEMSGHDR pHdr = (PGATEMSGHDR)pack.packet->getOffsetPtr();//保留协议头部分

	pack.nHdrPos = pack.packet->getPosition();
	
	pHdr->dwGateCode	= RUNGATECODE;
	pHdr->nSocket	= m_nUserSocket;
	pHdr->wSessionIdx= (WORD)m_nGateSessionIndex;
	pHdr->wIdent		= GM_DATA;
	pHdr->wServerIdx = m_nServerSessionIndex;
	pHdr->nDataSize = 0;
	pHdr->wTemp = RUNTEMPCODE;
	pHdr->tickCount = _getTickCount();
	pack.packet->adjustOffset(sizeof(GATEMSGHDR));
	pack.pActor = this;	
	return *pack.packet;
}
CDataPacket& CActor::AllocPacketEx(CActorPacket &pack, BYTE nSubSysId, BYTE nMsgId){
	CDataPacket& dp = AllocPacket(pack);
	dp << nSubSysId << nMsgId;
	return dp;
}

CDataPacket&  CActor::InitActorPacket(CActorPacket & pack,int nGateId, int nGateSessionIndex, int nServerSessionIndex,unsigned long long sock, LPCSTR file, INT_PTR line)
{
#ifdef _DEBUG
	Assert(GetCurrentThreadId() == GetLogicServer()->GetLogicEngine()->getThreadId());
	Assert(FALSE == InterlockedCompareExchange(&g_boPacketAlreadyAlloced, TRUE, FALSE));
	g_sPacketAllocFile = file;
	g_nPacketAllocLine = line;
#endif
	CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(nGateId);
	pack.packet = logicgate->GetSendToGatePacket();

	pack.packet->reserve(pack.packet->getPosition()+sizeof(GATEMSGHDR));

	PGATEMSGHDR pHdr = (PGATEMSGHDR)pack.packet->getOffsetPtr();//保留协议头部分
	pack.pActor = NULL;
	pack.nHdrPos = pack.packet->getPosition();
	pHdr->dwGateCode	= RUNGATECODE;
	pHdr->nSocket	= sock;
	pHdr->wSessionIdx= (WORD)nGateSessionIndex;
	pHdr->wIdent		= GM_DATA;
	pHdr->wServerIdx = nServerSessionIndex;
	pHdr->nDataSize = 0;
	pHdr->wTemp = RUNTEMPCODE;
	pHdr->tickCount = _getTickCount();
	pack.packet->adjustOffset(sizeof(GATEMSGHDR));
	return *pack.packet;
}

bool CActor::IsAdult()
{
	return m_miscSystem.IsAdult();
};   
void CActor::SendKeepAliveToGate()
{
	if( CActor::s_nTraceAccountId >= 0 && CActor::s_nTraceAccountId != m_nAccountID)
		return;

	BYTE ucSubSysId = enDefaultEntitySystemID;
	BYTE ucMsgId = cAppKeepAlive;
	long long nCurTick = _getTickCount();
	CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(m_nGateID);
	CDataPacket* packet = logicgate->GetSendToGatePacket();
	int nDataLen = sizeof(ucSubSysId)+sizeof(ucMsgId)+sizeof(nCurTick);
	packet->reserve(packet->getPosition()+sizeof(GATEMSGHDR)+nDataLen);
	PGATEMSGHDR pHdr = (PGATEMSGHDR)packet->getOffsetPtr();
	pHdr->dwGateCode	= RUNGATECODE;
	pHdr->nSocket		= m_nUserSocket;
	pHdr->wSessionIdx	= (WORD)m_nGateSessionIndex;
	pHdr->wIdent		= GM_APPKEEPALIVE;
	pHdr->wServerIdx	= m_nServerSessionIndex;
	pHdr->nDataSize		= nDataLen;
	pHdr->wTemp			= RUNTEMPCODE;	
	pHdr->tickCount		= 0;  // 保存逻辑服发送此消息的时间	
	packet->adjustOffset(sizeof(GATEMSGHDR));
	*packet << ucSubSysId << ucMsgId << nCurTick;
}


void CActorPacket::flush()
{
	if(pActor->OnGetIsSimulator() || pActor->OnGetIsTestSimulator())
	{
		//需要虚拟的数据不能发送 
		CDataPacket *SimulaPack = packet; 
		delete SimulaPack;
		return;
	}  

#ifdef _DEBUG
	Assert(TRUE == InterlockedCompareExchange(&CActor::g_boPacketAlreadyAlloced, TRUE, TRUE));
	CActor::g_sPacketAllocFile = NULL;
	CActor::g_nPacketAllocLine = 0;
#endif
	PGATEMSGHDR pHdr = (PGATEMSGHDR)(packet->getMemoryPtr() + nHdrPos);

	pHdr->nDataSize = (int)(packet->getPosition() - nHdrPos - sizeof(*pHdr));// (packet->getMemoryPtr() + packet->getPosition()) - (char*)pHdr - sizeof(GATEMSGHDR);
	if (pActor && pActor->IsInited() && pActor->m_bTracePacket)
	{
		char* pData = (char *)(pHdr+1);
		pActor->m_sPacketStat.AddRecord(*pData, *(pData+1), pHdr->nDataSize, pHdr->tickCount);
	}

	if (pActor && pActor->IsInited() && GetGlobalLogicEngine()->GetPacketRecordState())
	{
		char* pData = (char *)(pHdr+1);
		int posx,posy;
		pActor->GetPosition(posx,posy);
		CScene *pScene	= pActor->GetScene();
		LPCTSTR szSceneName	= "";
		int nLenth = (int)(packet->getPosition() - nHdrPos);
		if(pScene)
		{
			szSceneName	= pScene->GetSceneName();
		}
		GetGlobalLogicEngine()->GetDataPacketStat().AddAllRecord(*pData, *(pData+1), nLenth, pHdr->tickCount,"",posx,posy);
	}
	
	//OutputMsg(rmTip,"向客户端发送%d个字节",pHdr->nDataSize);
#ifdef _DEBUG
	InterlockedCompareExchange(&CActor::g_boPacketAlreadyAlloced, FALSE, TRUE);
	Assert(pHdr->nDataSize > 0 && pHdr->nDataSize <= 0x8000);
#endif
}

void CActorPacket::rollBack()
{
#ifdef _DEBUG
	Assert(TRUE == InterlockedCompareExchange(&CActor::g_boPacketAlreadyAlloced, TRUE, TRUE));
	CActor::g_sPacketAllocFile = NULL;
	CActor::g_nPacketAllocLine = 0;
#endif
	packet->setPosition((size_t)nHdrPos);
#ifdef _DEBUG
	InterlockedCompareExchange(&CActor::g_boPacketAlreadyAlloced, FALSE, TRUE);
#endif
}

//将这些数据缓存起来处理
VOID  CActor::PostNetMsg(CDataPacket * pNetData,INT_PTR nSize)
{
	CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(m_nGateID);
	
	if(  nSize < 10)
	{
		if(logicgate)
		{
			logicgate->AddLogicProcessedData(pNetData); //已经处理过的
		}
		OutputMsg(rmError,_T(" account=%d CActor::PostNetMsg, 数据长度太短"),m_nAccountID);
		return;
	}
	//if( CActor::s_nTraceAccountId <0 ||  CActor::s_nTraceAccountId == m_nAccountID)
	//{
	//	BYTE * pAddr= (BYTE*)pNetData->getOffsetPtr() ;
	//	pAddr += sizeof(Uint64); //要往前面偏移几个字节
	//	OutputMsg(rmTip,_T("2PostNetMsg, account=%d,system=%d,cmd=%d,size=%d"),m_nAccountID,(int)(*pAddr),(int)(*(pAddr+1)),(int)(nSize -10));
	//}

	/*
	CDataPacket *pPack=  GetGlobalLogicEngine()->GetEntityMgr()->AllocDatapack();
	if(pPack ==NULL) 
	{
		OutputMsg(rmError,_T(" CActor::PostNetMsg packet is nil"));
		return ; 
	}

	pPack->writeBuf(pNetData->getOffsetPtr(),pNetData->getLength() - pNetData->getPosition()); //将剩下的数据 
	*/
	//m_netMsgPool.linkAtLast( pNetData); //将这些数据链接在后面
	m_netMsgPool.add(pNetData);
}

//预先检测添加经验后能到多少级
int  CActor::AddExpReachLevel(INT_PTR nExpValue)
{
   LONGLONG nNewValue = m_property.lExp + nExpValue;	
   unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));	
   LONGLONG nExp =0;
   unsigned int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); 
   unsigned int nNext = nLevel;
   do 
   {
	   if (nLevel < nMaxPlayerLevel)	
	   {	
		   nExp= GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nNext + 1); 
		   if (nExp <= 0)
			   break;
		   if(nNewValue >= nExp)
		   {
			   nNewValue -= nExp;
			   nNext +=1;		
		   }
	   }
	   else
	   {	
		   nExp= GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nNext+1); 
		   if (nNewValue > nExp)		
			   nNewValue = nExp;
		   break;
	   }
   } while (nNewValue > nExp);
   return nNext;
}


/*
杀怪经验，如果怪物等级相对于玩家太低，则没有经验
这里只使用了防沉迷，衰减，杀怪的经验倍率
*/
void  CActor::AddExp(INT_PTR nExpValue,INT_PTR nExpWay ,INT_PTR nParam, INT_PTR nType, INT_PTR nParam2)
{
	//OutputMsg(rmNormal,_T("CActor::AddExp, ActorId=%d, nExpValueOld=%d, nExpWay=%d, nType=%d"), GetId(), nExpValue, nExpWay, nType);
		if(nExpValue ==0) return;

	bool hasSend =false;
	bool isLevelUp = false;//是否升级了
	LONGLONG nNewValue = 0;
	int openday = GetLogicServer()->GetDaysSinceOpenServer();
	int colorCardLevel = m_miscSystem.GetMaxColorCardLevel() ;
	
	if (nExpValue > 0)
	{
		int rate =m_miscSystem.GetFcmExpMoneyRate();//获取当前防沉迷的经验降低
		
		nExpValue =INT_PTR(( nExpValue *rate)/100); 
		if( nExpWay == GameLog::Log_PaoDian) {
			int nPlusNum = GetMiscSystem().GetSuperRightAtvPaoDianPlus() ; 
			if(nPlusNum > 0 ) {
				nExpValue =nExpValue * (100+nPlusNum)/100.0 ;
			} 
		}
		//玩家死亡了不能获得经验
		if(IsDeath()) return;

		if(nExpWay == GameLog::Log_KillMonster || GameLog::Log_TeamKillMonster == nExpWay) //这里要用多倍经验
		{
			float fRate = 1.0;			//nMonsterLevel为0表示不进行经验的衰减

			int nMonsterCircleLevel =(int)nParam2; //怪物转生等级
			int nCircleLevelDis = GetProperty<unsigned int>(PROP_ACTOR_CIRCLE) - nMonsterCircleLevel; //获取自己与怪物的转生等级
			fRate = GetLogicServer()->GetDataProvider()->GetKillMonsterExpReduce().GetCircleKillMonsterExp(nCircleLevelDis); 

			// // 2021-08-03 暂时不用			
			// int nMonsterLevel =(int)nParam; //怪物等级
			// if (nMonsterLevel > 0)
			// {
			// 	int nLevelDis = nMonsterLevel - m_property.nLevel; //获取自己的等级
			// 	fRate *= GetLogicServer()->GetDataProvider()->GetKillMonsterExpReduce().GetKillMonsterExp(nLevelDis); 
			// 	//经验的衰减，返回值为百分比float，fRate<=0时经验直接+1(单独处理)
			// }
			
			//英雄增加经验
			//m_heroSystem.OnAddExp((unsigned int)nExpValue);
			nExpValue =(INT_PTR)(nExpValue* fRate);
			if(nExpValue <=0) 
			{
				nExpValue = 1;
			}


			fRate =m_fExpRate;//杀怪的经验倍率
			LEVELUPEXPTABLE &levelRate = GetLogicServer()->GetDataProvider()->GetLevelUpExp() ;
			EXPLMTOPENDAY &expLmt = GetLogicServer()->GetDataProvider()->GetExpLmtOpenday() ;
			
			int lvl = GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
			if (lvl >= levelRate.expRateTable.count())
			{
				lvl = levelRate.expRateTable.count() - 1;
			}
			
			float fmultiRate = (float)levelRate.expRateTable[ lvl ]/10000.0; // 多倍经验倍率
				  fmultiRate *= m_fExpRate ;//和基础经验使用相同的基础倍率
			float fdampRate_1 = 1.0 ;
			float fdampRate_2 = 1.0 ;
			float fdampRate_3 = 1.0 ;

			//需求更改，衰减的逻辑没删，衰减加成同时有效
			float tmpTeQuan = (m_miscSystem.GetSuperRightAtvPaoDianPlus()+100)/100.0;
			fdampRate_1 *= tmpTeQuan ;
			fdampRate_2 *= tmpTeQuan ;
			fdampRate_3 *= tmpTeQuan ;

			if( expLmt.m_expLmtOpenday.find(openday) != expLmt.m_expLmtOpenday.end() )
			{
				if( expLmt.m_expLmtOpenday[openday].m_basiclmt.find(1) != expLmt.m_expLmtOpenday[openday].m_basiclmt.end()){
					fdampRate_1 = (float)expLmt.m_expLmtOpenday[openday].m_basiclmt[1].damprate /10000.0 ;				//触发  1  级衰减
				}else {
					OutputMsg(rmWaning,_T("exp limited config error"));
				}

				if( expLmt.m_expLmtOpenday[openday].m_basiclmt.find(2) != expLmt.m_expLmtOpenday[openday].m_basiclmt.end()){
					fdampRate_2 = (float)expLmt.m_expLmtOpenday[openday].m_basiclmt[2].damprate /10000.0 ;				//触发  2  级衰减
				}else {
					OutputMsg(rmWaning,_T("exp limited config error"));
				}

				if( expLmt.m_expLmtOpenday[openday].m_basiclmt.find(3) != expLmt.m_expLmtOpenday[openday].m_basiclmt.end()){
					fdampRate_3 = (float)expLmt.m_expLmtOpenday[openday].m_basiclmt[3].damprate /10000.0 ;				//触发  3  级衰减
				}else {
					OutputMsg(rmWaning,_T("exp limited config error"));
				}
			}

			if( m_property.lMultiExpUnused>0 )
			{
				LONGLONG multiExp = 0 ; 
				multiExp = nExpValue*fmultiRate ;
				fRate += fmultiRate;
				nExpValue = (INT_PTR)(nExpValue * fRate); //乘以经验倍率
				if((GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) -multiExp)<0){
					nExpValue +=(GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) -multiExp);
					multiExp = GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) ;
				}
				SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED , GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) -multiExp);
				int nTip = tmAddExpBasic;
				if( GameLog::Log_TeamKillMonster == nExpWay) //组队杀怪
					nTip = tmAddExpGeneral;
				SendTipmsgFormatWithId(nTip ,tstProperty,(INT_PTR)(nExpValue -multiExp)); //提醒玩家获得基础经验
				SendTipmsgFormatWithId(tmGetExpMulti ,tstProperty,(INT_PTR)multiExp); //提醒玩家获得多倍经验
				if( m_property.lMultiExpUnused <= 0){
					m_property.lMultiExpUnused = 0; //不然会将负数存入数据库，下次发放多倍经验会出错
					// SendTipmsgFormatWithId(tmMultiExpExhaust, tstUI);
					// SendTipmsgFormatWithId(tmMultiExpExhaust, tstUI);
					SendTipmsgFormatWithId(tmMultiExpExhaust, tstUI);
					/*UI橙色一次性提示3条“多倍经验已用完，打怪只能获得基础经验”*/
				}
			}
			else 
			{
				LONGLONG basicExp = m_StaticCountSystem.GetStaticCount(nDailyStaticBasicExp) ;
					
				if( basicExp >= expLmt.m_expLmtOpenday[openday].m_basiclmt[3].num ) {
					//触发三级衰减，每15min提醒
					// struct timespec ts;
					// clock_gettime(CLOCK_REALTIME, &ts); 
					// if(((ts.tv_sec + ts.tv_nsec/1000000000)-(ts_outputTipmsg15min_3.tv_sec + ts_outputTipmsg15min_3.tv_nsec/1000000000)) > 60*15){
					// 		SendTipmsgFormatWithId(tmTriggerExpLmt_3,tstUI);
					// 		//色卡特权提示加这
					// 		clock_gettime(CLOCK_REALTIME, &ts_outputTipmsg15min_3); 
					// }

					float colorRate = m_miscSystem.GetMaxExpDampColorCard()/100.0 ;
					float fDampRate=__max(colorRate,fdampRate_3) ;
					fRate*= fDampRate ;

				}else if( basicExp >= expLmt.m_expLmtOpenday[openday].m_basiclmt[2].num  ){
					//触发二级衰减，每15min提醒
					// struct timespec ts;
					// clock_gettime(CLOCK_REALTIME, &ts); 
					// if(((ts.tv_sec + ts.tv_nsec/1000000000)-(ts_outputTipmsg15min_2.tv_sec + ts_outputTipmsg15min_2.tv_nsec/1000000000)) > 60*15){
					// 		SendTipmsgFormatWithId(tmTriggerExpLmt_2,tstUI);
					// 		//色卡特权提示加这
					// 		clock_gettime(CLOCK_REALTIME, &ts_outputTipmsg15min_2); 
					// }

					float colorRate = m_miscSystem.GetMaxExpDampColorCard()/100.0 ;
					float fDampRate=__max(colorRate,fdampRate_2) ;
					fRate*= fDampRate ;
					
					
				}else if( basicExp >= expLmt.m_expLmtOpenday[openday].m_basiclmt[1].num ){
					//触发一级衰减，每15min提醒
					// struct timespec ts;
					// clock_gettime(CLOCK_REALTIME, &ts); 
					// if(((ts.tv_sec + ts.tv_nsec/1000000000)-(ts_outputTipmsg15min.tv_sec + ts_outputTipmsg15min.tv_nsec/1000000000)) > 60*15){
					// 		SendTipmsgFormatWithId(tmTriggerExpLmt_1,tstUI);
					// 		//色卡特权提示加这
					// 		clock_gettime(CLOCK_REALTIME, &ts_outputTipmsg15min); 
					// }
					
					float colorRate = m_miscSystem.GetMaxExpDampColorCard()/100.0 ;
					float fDampRate=__max(colorRate,fdampRate_1);
					fRate*= fDampRate ;
				}
				/*基础经验计入每日数值*/
				nExpValue = (INT_PTR)(nExpValue * fRate); //乘以经验倍率
				if( 0 == nExpValue) nExpValue =1;//最少给1点辛苦经验
				m_StaticCountSystem.SetStaticCount(nDailyStaticBasicExp,basicExp + nExpValue);
				int nTip = tmAddExpBasic;
				if( GameLog::Log_TeamKillMonster == nExpWay) //组队杀怪
					nTip = tmAddExpGeneral;
				SendTipmsgFormatWithId(nTip ,tstProperty,nExpValue); //提醒玩家获得基础经验
			}
		//AddExpGetToday((unsigned int)nExpValue);
		} else if(nExpWay == GameLog::Log_PaoDian){
			m_achieveSystem.ActorAchievementComplete(nAchieveActorNum, nExpValue, qatExp, nParam);
			SendTipmsgFormatWithId(tmAddExpbyPaoDian ,tstProperty,nExpValue);
		}else {
			SendTipmsgFormatWithId(tmAddExpBasic ,tstProperty,nExpValue);
		}//非杀怪的经验提示

		//上面未对hasSend进行修改，统一通过这里进行sendtipmsg
		//if(!hasSend)
		//{
		//	SendTipmsgFormatWithId(tmGetExp ,ttTipmsgWindow,nExpValue); //提醒玩家获得经验
		//	}
		
		//这里要处理升级判断
		nNewValue = m_property.lExp + nExpValue;	
		unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));	
		LONGLONG nExp =0;
		if (IsNeedLogExp(nExpWay))
		{
			//记录日志
			if(GetLogicServer()->GetLocalClient())
			{
			}
		}
		do 
		{
			unsigned int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL); // 玩家当前等级
			unsigned int nNext = nLevel + 1;
			if (nLevel < nMaxPlayerLevel)	// 未到最高等级
			{	
				nExp= (LONGLONG)GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nNext); //升到下一级需要多少经验
				if (nExp <= 0)
					break;

				if(nNewValue >= nExp)
				{
					nNewValue -= nExp;
					unsigned int nId(GetId());
					SetLevel(nNext);
					m_BossSystem.SendSZBossTimes(nNext);	
					//等级达到25级时推荐加入行会信息
					m_guildSystem.SendSuggestGuildToActor(nNext);
					//GetFriendSystem().SetSocialNeedFresh(nId);
					isLevelUp = true;
					GetGlobalLogicEngine()->GetTopTitleMgr().SetTopMaxLevel(nNext);
					m_achieveSystem.ActorAchievementComplete(nAchieveLevel, nNext);
					m_questSystem.OnQuestEvent(CQuestData::qtLevel, nNext);
					UpdateUseDeopt();
					//nExp= GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nLevel +1); 
				}
			}
			else							// 已经满级
			{	
				nExp= GetLogicServer()->GetDataProvider()->GetLevelUpExp().GetLevelExp(nNext); 
				if (nNewValue > nExp)		// 经验值满了不继续增加
					nNewValue = nExp;
				break;
			}
		} while (nNewValue > nExp);

	}
	else  //掉经验的代码
	{
		nNewValue = m_property.lExp + nExpValue;
		if (nNewValue < 0)
		{
			nNewValue = 0;
		}
		// if (nExpWay == GameLog::clDeathDropExp)
		// {
		// 	SendOldTipmsgFormatWithId(tpDeathDropExp ,ttTipmsgWindow,-nExpValue);
		// }
	}
	
	SetProperty<Uint64>(PROP_ACTOR_EXP,(Uint64)nNewValue);
	//GetGlobalLogicEngine()->GetWorldLevelMgr().CheckWorldLevel(this, isLevelUp);
	// if ( nExpValue >= GetLogicServer()->GetDataProvider()->GetGlobalConfig().nWaningExp )
	// {
	// 	OutputMsg(rmWaning,_T("%s(%u)-%d-(%lld)"),(char*)GetEntityName(),GetId(),nExpWay,(Uint64)nExpValue);
	// }

	if (nType == 1)
	{
		char buff[512];
		CDataPacket outPack(buff,sizeof(buff)); //下发的数据包
		outPack << (BYTE) enMiscSystemID << (BYTE)sNoticeExpChange;
		outPack << (INT64)GetHandle();
		outPack << (byte)qatExp;
		outPack << (int)nExpValue;
		GetObserverSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(),true); //附近广播
	}
	else if (nType == 2)
	{
		if (nExpValue > 0)
		{
			INT_PTR nTips = tpAddExp;

			if (nTips != tpNoError)
			{
				SendOldTipmsgFormatWithId(nTips, ttFlyTip, (int)nExpValue);
			}
			else
			{
				CActorPacket ap;
				CDataPacket &outPack = AllocPacket(ap); //下发的数据包
				outPack << (BYTE) enMiscSystemID << (BYTE)sNoticeExpChange;
				outPack << (Uint64)GetHandle();
				outPack << (byte)qatExp;
				outPack << (int)nExpValue;
				ap.flush();
			}
			
		}
	}
}

	
bool  CActor::ConsumeCoin(INT_PTR nCount,int nlogId, LPCTSTR comment,bool needLog,bool bindFirst )
{
	if(HasEnoughCoin(nCount) ==false) return false;
	INT_PTR nBindCount =0;       //绑定银两的数目
	INT_PTR nNonBindCount = GetProperty<unsigned int>(PROP_ACTOR_COIN); //非绑定元宝的数目
	bool ret =true;
	if(bindFirst ) //优先消耗绑定的
	{
		if(nBindCount >= nCount) //绑定的数目
		{
			return ChangeMoney(mtBindCoin,-nCount,nlogId,0,comment,needLog);
		}
		else
		{
			if(nBindCount >0)
			{
				ret = ChangeMoney(mtBindCoin,-nBindCount,nlogId,0,comment,needLog);   //先消耗绑定的，再消耗非绑定额
				if(ret)
				{
					ret = ChangeMoney(mtCoin,nBindCount- nCount,nlogId,0,comment,needLog); 
				}
				return ret;
			}
			else
			{
				return  ChangeMoney(mtCoin, - nCount,nlogId,0,comment,needLog);
			}
		}
	}
	else //优先消耗非绑定的
	{
		if(nNonBindCount >= nCount) //绑定的数目
		{
			return ChangeMoney(mtCoin,-nCount,nlogId,0,comment,needLog);
		}
		else
		{
			if(nNonBindCount >0)
			{
				ret &= ChangeMoney(mtCoin ,    -nNonBindCount,nlogId,0,comment,needLog);   //先消耗绑定的，再消耗非绑定额
				ret &= ChangeMoney(mtBindCoin, nNonBindCount- nCount,nlogId,0,comment,needLog); 
				return ret;
			}
			else
			{
				return ChangeMoney(mtBindCoin,- nCount,nlogId,0,comment,needLog); 
			}
		}
	}

}


void CActor::OnFeeCmd(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader)
{
	static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	CScriptValueList paramList,retList;
	switch (nCmd)
	{
		case jxInterSrvComm::DbServerProto::dcLoadFee:
		{
			int nCount = 0;
			reader >> nCount;
			if (nCount > 0)
			{
				CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcClearFee);
				dataPacket << GetRawServerIndex() << GetLogicServer()->GetServerIndex();
				dataPacket << (int)(GetProperty<unsigned int>(PROP_ENTITY_ID));

				int nDelCount = 0;
				INT_PTR pos = dataPacket.getPosition();
				dataPacket << nDelCount;

				FeeData feedata;
				for(int i = 0; i < nCount; ++i)
				{
					reader.readBuf(&feedata,sizeof(feedata));
					OutputMsg(rmTip, _T("[Fee]2 [%s] 进行充值： Id（%d），PfId(%s), ActorId（%d），ProdId（%s）"),
						GetEntityName(), feedata.nId, feedata.sPf, feedata.nActorId, feedata.sProdId
					);

					// 触发消费调用
					paramList.clear();
					retList.clear();
					paramList << this;
					paramList << feedata.sPf;
					paramList << feedata.sProdId;
					paramList << feedata.nNum;
					if (!globalNpc->GetScript().CallModule("Fee", "OnFeeCmd", paramList, retList, 1))
					{
						OutputMsg(rmError,"[Fee]2 [%s] 充值报错！Id（%d），PfId(%s), ActorId（%d），ProdId（%s），Num（%s）!",
							GetEntityName(), feedata.nId, feedata.sPf, feedata.nActorId, feedata.sProdId, feedata.nNum
						);
					}

					// 返回数据库删除
					if(retList.count() >= 0 && (bool)retList[0] == true)
					{
						++nDelCount;
						dataPacket << feedata.nId;
					}
					else
					{
						OutputMsg(rmError,"[Fee]2 [%s] 充值出错！Id（%d），PfId(%s), ActorId（%d），ProdId（%s）!",
							GetEntityName(), feedata.nId, feedata.sPf, feedata.nActorId, feedata.sProdId
						);
					}
				}
				int* pCount = (int*)dataPacket.getPositionPtr(pos);
				*pCount = nDelCount;
				GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket);
			}
			break;
		}
	}
}

/*改变货币
注意：ZGame中，如果扣除绑定元宝，如果绑定元宝不足，则扣除元宝，反正不可逆
在ChangeBindYuanbaoAndYuanbao()中
*/
bool CActor::ChangeMoney(INT_PTR nMentyType,INT_PTR nMoneyValue,INT_PTR nWay ,INT_PTR nCount,LPCSTR sComment ,bool needLog, bool boForceLog,bool bNotice )
{
	if(nMoneyValue ==0) return false;

	//不受防沉迷控制的金钱
	//除了交易或者元宝，其他的钱受到防沉迷的控制
	if(isRechargeYuanbao(nWay)||nWay ==GameLog::clDealTransferMoney || nWay == GameLog::clActorDepotMoney || nWay == GameLog::clStallSellItemMoney || nMentyType == mtYuanbao) 
	{
	}
	else
	{
		if(nMoneyValue >0) //如果是+金钱，才受防沉迷的控制，扣钱不能受防沉迷控制的
		{
			nMoneyValue =(INT_PTR)( (m_miscSystem.GetFcmExpMoneyRate() * nMoneyValue) /100);
		}		
	}

	int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);//获取玩家的等级

	//玩家提取元宝
	if( isRechargeYuanbao(nWay) && nMentyType == mtYuanbao && nMoneyValue >0)
	//if((nWay == GameLog::clWithdrawYuanBao||nWay == GameLog::clChargeYuanbao) && nMentyType == mtYuanbao && nMoneyValue >0)
	{
		//当前提取元宝的数目，当玩家提取元宝的时候，将提取的元宝的记录下来，触发成就
		unsigned int nCurrentDrayYb = GetProperty<unsigned int>(PROP_ACTOR_DRAW_YB_COUNT); //玩家提取的元宝的数目

		if(nCurrentDrayYb == 0)
		{

		}

		// 二充
		if (GetRechargeStatus(RechargeStatus) == 1)
		{
			std::vector<int> &vecSecondRechargIdLists = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo().nSecondChargeIds;
			if(vecSecondRechargIdLists.size() > 0 && !m_bRebateCharge)
			{
				for(auto id :vecSecondRechargIdLists)
				{
					GetActivitySystem().UpdateActivityData(id, nCurrentDrayYb + nMoneyValue);
				}
			}
		}

		// 返利卡充值元宝 首充 不计算
		std::vector<int> & rechargIdLists = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nFirstChargeIds;
		if(rechargIdLists.size() > 0 && !m_bRebateCharge)
		{
			for(auto id :rechargIdLists)
			{
				GetActivitySystem().UpdateActivityData(id, nCurrentDrayYb + nMoneyValue);
			}
		}

		// 区服冠名 活动
		std::vector<int> &vecServerNameId = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo().vecServerNameId;
		if( vecServerNameId.size() > 0 && !m_bRebateCharge )
		{
			for( auto id :vecServerNameId )
			{
				if ( GetGlobalLogicEngine()->GetActivityMgr().IsRunning(id) )
				{
					GetActivitySystem().UpdateActivityData(id, nCurrentDrayYb + nMoneyValue);
				}
			}
		}

		//充值元宝同步增加交易额度
		tagEditionInfo& editionInfo = GetLogicServer()->GetDataProvider()->GetEditionProvider().GetEditionInfo();
		float JiaoyiRate = editionInfo.nJiaoYiEDuRate /100.0 ; //获取比例
		unsigned int  nOldJiaoYiCount= GetMoneyCount(mtJyQuota);	
		unsigned int  nNewJiaoYiCount = /*nOldJiaoYiCount +*/ nMoneyValue*JiaoyiRate ;
		ChangeJyQuota(nNewJiaoYiCount, nWay, nCount, sComment,needLog );
		// SetProperty<unsigned int>(PROP_ACTOR_TRADINGQUOTA, (unsigned int)nNewJiaoYiCount);


		nCurrentDrayYb += (unsigned int)nMoneyValue;
		if ( !m_bRebateCharge )
		{
			SetProperty<unsigned int>(PROP_ACTOR_DRAW_YB_COUNT,nCurrentDrayYb);
		}
		else {
			// 设置 返利卡提取元宝数
			SetProperty<unsigned long long>(PROP_ACTOR_REBATE_DRAW_YB_COUNT, (GetProperty<unsigned long long>(PROP_ACTOR_REBATE_DRAW_YB_COUNT) + (unsigned long long)nMoneyValue) );
		}
		

		//激活首充会员卡
		if ( ! m_miscSystem.IsHasShouChongCard()) 
		{
			unsigned int val = GetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG);
			val |= (1<<2);
			SetProperty<unsigned int>(PROP_ACTOR_FOREVER_CARD_CLAG,(unsigned int)val);
		}
					
		//触发脚本事件
		CScriptValueList paramList;
		//InitEventScriptVaueList(paramList,aeWithDrawYuanBao);
		paramList << nMoneyValue; //提取元宝数
		OnEvent(aeWithDrawYuanBao,paramList,paramList);

		if (!m_bRebateCharge)
		{
			m_activitySystem.OnChargeYuanBao();//提取元宝时刷新活动数据
		}

		int nCombineDays = GetLogicServer()->GetDaysSinceCombineServer();

		//GetAchieveSystem().OnEvent(aAchieveDrawYuBao,nCurrentDrayYb);

		// ku25 每日首充和每日累计充值
		if ( 0 == strcmp(GetLogicServer()->GetVSPDefine().GetDefinition("SPID"), "22") )
		{
			static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if ( globalNpc )
			{
				CScriptValueList paramList;
				paramList << this;
				paramList << nMoneyValue;

				if ( !globalNpc->GetScript().CallModule("ActivityType10027", "ChangeOneDayRechargesYBNum", paramList, paramList, 0) )
				{
					OutputMsg(rmError,"[ku25] ChangeOneDayRechargesYBNum 错误 玩家=%s !", GetEntityName());
				}
			}
		}
	}
	if(nMoneyValue ==0) return false; //数值为0 
	unsigned long long nNewValue =0;
	bool isConsumeYb= false;
	unsigned int  nOldMoneyCount= GetMoneyCount(eMoneyType(nMentyType));	//现有货币（且绑定与非绑定的货币不加在一起）
	if (nMoneyValue < 0 && (-nMoneyValue) > (INT_PTR)nOldMoneyCount)
	{
		OutputMsg(rmWaning,_T("[%s]Money type=%d is not enough,with changmoney=%d way=%d"),GetEntityName(),nMentyType,nMoneyValue,nWay);
		return false;
	}
	
	//消费的元宝在这里处理
	// if( nMentyType == mtYuanbao && nMoneyValue <0)
	// {
	// 	isConsumeYb = isConsumeYuanbao(nWay);
	// 	if(isConsumeYb )
	// 	{
	// 		//消费放进下面的函数
	// 		UpdateConsumeYuanbao(-nMoneyValue,sComment);
	// 	}		
	// }
	//成就或者任务 用于统计消耗数值
	if(nMoneyValue < 0)
	{
		isConsumeYb = isConsumeYuanbao(nWay);
		if(isConsumeYb )
		{
			if(nMentyType == mtYuanbao)
			{
				//消费放进下面的函数
				UpdateConsumeYuanbao(-nMoneyValue,sComment);
			}
			m_achieveSystem.ActorAchievementComplete(nAchieveCost, -nMoneyValue, nMentyType);
		}
	}

	// 返利系统 充值 或者 消耗元宝，返利卡充值，返利系统不计算
	if ( !m_bRebateCharge && ( (isRechargeYuanbao(nWay) && nMentyType == mtYuanbao && nMoneyValue > 0) || ( isConsumeYuanbao(nWay) && nMentyType == mtYuanbao && nMoneyValue < 0) ) )
	{
		int nType = 0;
		if ( isRechargeYuanbao(nWay) )
		{
			nType = 1;
		} 
		else if ( isConsumeYuanbao(nWay) )
		{
			nType = 2;
		}
		
		m_RebateSystem.UpdateRebateData(nMoneyValue, nType);
	}

	//这里要判断是否会溢出4容易
	if(nMoneyValue >0)
	{
		if(GameLog::Log_Recover == nWay) {
			if(m_fBindCoinRate > 1.0f) {
				nMoneyValue *= m_fBindCoinRate ;
			}
		}
		unsigned long long nNew =  (unsigned long long)(nOldMoneyCount + nMoneyValue);
		//做大不能超过
		if(nNew >= 0xFFFFFFFF)
		{
			OutputMsg(rmWaning,_T("Money type=%d is not large,with changmoney=%d"),nMentyType,nMoneyValue);
			nNewValue = 0xFFFFFFFF;
			SendOldTipmsgFormatWithId(tpNoMoreMoney,ttDialog,CActor::GetMoneyTypeDesc(nMentyType));
		}
		else
		{
			nNewValue = (unsigned int)nNew;
		}
		isConsumeYb = isConsumeYuanbao(nWay);
		if(isConsumeYb )
		{
			m_achieveSystem.ActorAchievementComplete(nAchieveActorNum, nMoneyValue, nMentyType);
			if(isRechargeYuanbao(nWay)) {
				m_achieveSystem.ActorAchievementComplete(nAchieveRecharge, nMoneyValue, nMentyType);

				if ( nMentyType == mtYuanbao && !m_bRebateCharge )
				{
					m_achieveSystem.ActorAchievementComplete(nAchieveRealRecharge, nMoneyValue, nMentyType);
				}

				int nLastRechargeTime = m_StaticCountSystem.GetStaticCount(nRechargTime_StaticType);//计数器
				CMiniDateTime todaytime = CMiniDateTime::today();
				// 返利卡充值元宝 累计充值天数 不计算
				if(nLastRechargeTime != todaytime.tv && nMentyType == mtYuanbao && !m_bRebateCharge) {
					m_achieveSystem.ActorAchievementComplete(nAchieveRechargeDay, 1, nMentyType);
					m_StaticCountSystem.SetStaticCount(nRechargTime_StaticType,todaytime.tv);//计数器
				}
				
			}

		}
	}
	else
	{
		nNewValue =(unsigned int)( nOldMoneyCount + nMoneyValue);
	}
	

	if( nNewValue <0)
	{
		OutputMsg(rmWaning,_T("Money type=%d is not enough,with changmoney=%d"),nMentyType,nMoneyValue);
		return false;
	}
	if (nMentyType == mtBindYuanbao)
	{	
		unsigned int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBindYuanbaoLimit;
		if (nNewValue > nMax)
		{
			nMoneyValue = nMax - GetMoneyCount(eMoneyType(nMentyType));
			nNewValue = nMax;
			SendTipmsgFormatWithId(tmBindYuanbaoFull,tstUI);
		}
	}
	else if (nMentyType == mtCoin)
	{	
		unsigned int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMoneylimit;
		if (nNewValue > nMax)
		{
			nMoneyValue = nMax - GetMoneyCount(eMoneyType(nMentyType));
			nNewValue = nMax;
			SendTipmsgFormatWithId(tmCoinFull,tstUI);
		}
	}
	else if (nMentyType == mtBindCoin)
	{	
		unsigned int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBindmoneylimit;
		if (nNewValue > nMax)
		{
			nMoneyValue = nMax - GetMoneyCount(eMoneyType(nMentyType));
			nNewValue = nMax;
			SendTipmsgFormatWithId(tmBindCoinFull,tstUI);
		}
	}
	else if (nMentyType == mtYuanbao)
	{	
		unsigned int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nYuanbaolimit;
		if (nNewValue > nMax)
		{
			nMoneyValue = nMax - GetMoneyCount(eMoneyType(nMentyType));
			nNewValue = nMax;
			SendTipmsgFormatWithId(tmYuanbaoFull,tstUI);
		}
	}

	//int nTipmsgID =0;
	switch(nMentyType)
	{
	case mtBindCoin:
		SetProperty<unsigned int>(PROP_ACTOR_BIND_COIN, (unsigned int)nNewValue); 
		break;
	case mtCoin:
		SetProperty<unsigned int>(PROP_ACTOR_COIN, (unsigned int)nNewValue);
		break;
	case mtBindYuanbao:
		SetProperty<unsigned int>(PROP_ACTOR_BIND_YUANBAO, (unsigned int)nNewValue);
		break;
	case mtYuanbao:
		SetProperty<unsigned int>(PROP_ACTOR_YUANBAO, (unsigned int)nNewValue);
		break;
	case mtWingSoul:
		SetProperty<unsigned int>(PROP_ACTOR_WINGPOINT, (unsigned int)nNewValue);
		break;
	case mtFlyShoes://飞鞋
		SetProperty<unsigned int>(PROP_ACTOR_FLYSHOES, (unsigned int)nNewValue);
		break;
	case mtJyQuota://交易额度
		if ( !m_bRebateCharge )
		{
			// 返利卡充值元宝 交易额度 不计算
			SetProperty<unsigned long long>(PROP_ACTOR_TRADINGQUOTA, (unsigned long long)nNewValue);
		}
		break;
	default: 
		return false;
	}
	
	// 弹出提示（不走TipMsg）
	CActorPacket ap;
	CDataPacket &outPack = AllocPacketEx(ap, enDefaultEntitySystemID, sMoneyChange);
	outPack << (BYTE)nMentyType << (int)nMoneyValue;
	ap.flush();
	
	//元宝必须要记录日志[zgame所有都记录消费日志]
	if(needLog || nMentyType == mtYuanbao || boForceLog || nMentyType==mtCoin)
	{
		// 记录全局日志
		GetGlobalLogicEngine()->AddCosumeToLog((WORD)nWay, 
					(int)GetId(), GetAccount(), GetEntityName(), (BYTE)nMentyType,
					(int)nCount, (int)nMoneyValue, sComment, nLevel, nNewValue, getOldSrvId());
					
		if (getMoneyType(nMentyType))
		{
			// 记录本地日志
			GetLogicServer()->SendCurrencyLocalLog((int)nWay,this,nMoneyValue>0?1:2,(int)nMentyType,nMoneyValue,nNewValue,(nWay == GameLog::Log_ShopBuy ? 1 : 0));
		}
	}

	if (isRechargeYuanbao(nWay) && nMentyType == mtYuanbao)
	{
		SetRebateCharge(false);
	}

	return true;
}

bool CActor::IsNeedLogExp(INT_PTR nLogId)
{
	if (nLogId == GameLog::Log_KillMonster)
	{
		return false;
	}
	return true;
}

void CActor::UpdateConsumeYuanbao(const INT_PTR nConsumerCount,LPCSTR sComment)
{
	if (nConsumerCount <= 0) return;

	int nLevel =GetProperty<int>(PROP_CREATURE_LEVEL);//获取玩家的等级

	if(m_consumeyb == 0)
	{
		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
	}

	m_consumeyb +=(unsigned)(nConsumerCount ); //消耗的元宝的数目

	//添加当天消耗的元宝到排行榜
	// GetStoreSystem().AddConsumeToRank((int)nConsumerCount);

	//m_achieveSystem.OnEvent(aAchieveEventConsumerYuanbao,nConsumerCount);//消费元宝
	//GetFriendSystem().TeacherToPupilYBBind((int)nConsumerCount);		//师傅消费元宝发福利给徒弟

	CScriptValueList paramList;
	//InitEventScriptVaueList(paramList,aeConsumeYb);
	paramList << (int)nConsumerCount; 
	OnEvent(aeConsumeYb, paramList,paramList);
}

void  CActor::SendOldTipmsgWithId(INT_PTR nTipmsgID,INT_PTR nType)
{
	// SendTipmsg(GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgID),nType);
}

void CActor::SendOldTipmsgFormatWithId(INT_PTR nTipmsgID,INT_PTR nType /* = ttTipmsgWindow */, ...)
{
	
	// va_list args;
	// LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgID);
	// if(sFormat ==NULL) return;
	// va_start(args, nType);
	// SendTipmsgWithArgs(sFormat,args,nType);
	// va_end(args);

}

void  CActor::SendTipmsgWithId(INT_PTR nTipmsgID,INT_PTR nType)
{
	if (nTipmsgID != 0)
	{
		SendTipmsg(GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipmsgID),nType);
	}
}

void CActor::SendTipmsgFormatWithId(INT_PTR nTipmsgID,INT_PTR nType /* = ttTipmsgWindow */, ...)
{ 
    if (OnGetIsTestSimulator()
        || OnGetIsSimulator())
    {
        return; 
    }

	va_list args;
	LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(nTipmsgID);
	if(sFormat ==NULL) return;
	va_start(args, nType);
	SendTipmsgWithArgs(sFormat,args,nType);
	va_end(args);
}
void CActor::SendTipmsgWithArgs(LPCTSTR sFormat, va_list &args, INT_PTR nType)
{
	//_vsntprintf
	//这个函数可能导致服务器挂掉，暂时不用
	
	if(NULL ==sFormat || strcmp(sFormat,"")==0)return;

	static const WORD dwMaxMsgSize = 1024;
	TCHAR sBuf[1024];

    if (OnGetIsTestSimulator()
        || OnGetIsSimulator())
    {
        return; 
    }

	if(nType == tstChatSystem)
	{
		CActorPacket ap;
		CDataPacket &pack = AllocPacket(ap);
		pack.reserve(pack.getPosition() + dwMaxMsgSize + 4);
		pack << (BYTE) enChatSystemID << (BYTE)sSendChat <<(BYTE)ciChannelTipmsg;

		_vsntprintf(sBuf, ArrayCount(sBuf) - 1, sFormat, args);
		pack.writeString("");
		pack.writeString(sBuf);
		pack << (int)0;
		pack <<(Uint64)0;
		pack <<(unsigned int)(0);
		pack << (BYTE)(0);
		ap.flush();
	}
	else
	{
		INT_PTR nCount =  _vsntprintf(sBuf, ArrayCount(sBuf) - 1, sFormat, args);
		if (nCount < 0)
		{
			OutputMsg(rmError, _T("格式化Tipmsg失败，nType=%d, sFormat=%s"), nType, sFormat);
			return;
		}

		CActorPacket ap;
		CDataPacket &pack = AllocPacket(ap);
		pack.reserve(pack.getPosition() + dwMaxMsgSize + 4);
		pack <<(BYTE)enChatSystemID << (BYTE)sSendTipmsg << (WORD) nType;
		pack.writeString(sBuf);
		ap.flush();
	}
}



//重设玩家的显示名字
//后面的需要更多的可以在这里添加
//角色名\攻城战职位名\行会名\战绩称号\摊位名\结婚\
//注意要加反斜杠'\\'
void CActor::ResetShowName()
{
	int nGuildPos = GetGuildSystem()->GetGuildPos();	//帮派中的职位
	GetGuildSystem()->SetGuildTitle(nGuildPos);

	char pShowName[320];
	memset(pShowName, 0, sizeof(pShowName));
	strcpy(pShowName,GetEntityName());//拷贝玩家的名字进去
	CGuild * pGuild = m_guildSystem.GetGuildPtr();
	strcat(pShowName,"\\");
	if (pGuild && (pGuild->m_sGuildname != NULL))
	{
	 	strcat(pShowName,pGuild->m_sGuildname);
		if(pGuild->GetGuildId() == GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId())
		{
			strcat(pShowName,"\\");
			LPCTSTR sFormat;
			sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmSbkCityName);
			strcat(pShowName, sFormat);
		}
		
	}
	// strcat(pShowName,"\\");
	// //帮派和帮派的排名
	// if(  NULL != pGuild && GetProperty<unsigned int>(PROP_ACTOR_GUILD_ID) > 0)
	// {
	// 	unsigned int nSbkGuildId = GetGlobalLogicEngine()->GetGuildMgr().GetCityOwnGuildId();
	// 	if(nSbkGuildId > 0 && nSbkGuildId == pGuild->m_nGid)
	// 	{
	// 		LPCTSTR sFormat;
	// 		sFormat = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tpSbkCityName);
	// 		strcat(pShowName,"<");
	// 		strcat(pShowName,sFormat);
	// 		strcat(pShowName,">");
	// 	}
	// 	strcat(pShowName,pGuild->m_sGuildname);
	// }
	
	SetShowName(pShowName);

	//如果已经进入游戏了，需要广播一下
	if(IsInited())
	{
		//ChangeName(GetEntityName());
		ChangeName(GetShowName());
		//OutputMsg(rmWaning,_T("GetShowName:%s"),GetShowName());
	}
	//OutputMsg(rmWaning,_T("ResetShowName: ShowName=%s"),pShowName);
}

void CActor::InitTomorrowDateTime()
{	
	m_tomorrowDateTime.tv = m_nLoginTime.tomorrow();	
}

void CActor::InitActorExpGetToday(unsigned int nExpLastTime)
{	
	m_nExpGetLastLogout = nExpLastTime;	
	if (!m_nLastLogOutTime.isSameDay(m_nLoginTime))
		nExpLastTime = 0;

	m_nExpGetToday = nExpLastTime;		
}

void CActor::InitActorZyContriToday(unsigned int nZyContriLastTime)
{
	m_nZyContriGetLastLogout = nZyContriLastTime;

	if (!m_nLastLogOutTime.isSameDay(m_nLoginTime))
		nZyContriLastTime = 0;

	m_nZyContriGetToday = nZyContriLastTime;	
}

void CActor::AddZyContriToday(unsigned int nZyContri)
{
	if (nZyContri>=0)
		m_nZyContriGetToday += nZyContri;
}

bool CActor::CheckExpGetTodayUpLimit()
{
	int nLevelIdx = GetProperty<int>(PROP_CREATURE_LEVEL) - 1;
	unsigned int nGetExpUpLimitEveryDay = 0;
	CVector<unsigned int> &cfg = GetLogicServer()->GetDataProvider()->GetExpGetUpperLmtCfg();
	if (cfg.count() > 0 && ((nLevelIdx >= 0) || (nLevelIdx < cfg.count())))
		nGetExpUpLimitEveryDay = cfg[nLevelIdx];
	if (m_nExpGetToday >= nGetExpUpLimitEveryDay)
		return true;

	return false;
}

void CActor::AddExpGetToday(unsigned int nExp)
{
	unsigned int oldExpToday = m_nExpGetToday; //今天获取的杀怪经验
	m_nExpGetToday += nExp;
	unsigned int nGetExpUpLimitEveryDay = 0; //每日获得经验的上限
	CVector<unsigned int> &cfg = GetLogicServer()->GetDataProvider()->GetExpGetUpperLmtCfg();
	int nLevelIdx = GetProperty<int>(PROP_CREATURE_LEVEL) - 1;			// 如果是超过了，然后升级了怎么办???
	if ((nLevelIdx >= 0) || (nLevelIdx < cfg.count()))
		nGetExpUpLimitEveryDay = cfg[nLevelIdx];
	if (oldExpToday < nGetExpUpLimitEveryDay && m_nExpGetToday >= nGetExpUpLimitEveryDay)
	{		
		OnExpGetTodayStatusChanged(true);
	}
}

void CActor::PraseRevivePoint(const long long point, WORD &sceneId, WORD &posX, WORD &posY)
{
	LARGE_INTEGER Pos64;
	Pos64.QuadPart = point;
	sceneId = (WORD)Pos64.HighPart;
	posX = HIWORD(Pos64.LowPart);
	posY = LOWORD(Pos64.LowPart);
}

void CActor::InitActorRelivePointData(const ACTORDBDATA *pActorData)
{
	PraseRevivePoint(pActorData->csrevivepoint,		m_nRelivePointSceneId,	m_nRelivePointPosX, m_nRelivePointPosY);
	PraseRevivePoint(pActorData->cityrevivepoint,	m_nCityPointSceneId,	m_nCityPointPosX,	m_nCityPointPosY);
	PraseRevivePoint(pActorData->fbrevivepoint,		m_nFubenReliveSceneId,	m_nFubenRelivePosX, m_nFubenRelivePosY);
}

void CActor::ComposeRevivePoint(long long &point, const int nSceneId, const int nPosX, const int nPosY)
{
	LARGE_INTEGER li;
	LONG pos	= MAKELONG(nPosY, nPosX);
	li.HighPart = nSceneId;
	li.LowPart	= pos;
	point = li.QuadPart;
}

void CActor::SaveActorRelivePointData(ACTORDBDATA &dbData)
{
	bool bIsInRawServer = IsInRawServer();
	ComposeRevivePoint(dbData.csrevivepoint,	m_nRelivePointSceneId,	m_nRelivePointPosX,	m_nRelivePointPosY);
	ComposeRevivePoint(dbData.cityrevivepoint,	m_nCityPointSceneId,	m_nCityPointPosX,	m_nCityPointPosY);
	ComposeRevivePoint(dbData.fbrevivepoint,	m_nFubenReliveSceneId,	m_nFubenRelivePosX,	m_nFubenRelivePosY);

	//if (!bIsInRawServer)
	{
		/*
		OutputMsg(rmTip, _T("保存角色复活点数据, 普通场景复活点(%d, %d_%d)， 副本复活点(%d, %d_%d)， 回城复活点(%d, %d_%d)"),
			m_nRelivePointSceneId, m_nRelivePointPosX,	m_nRelivePointPosY,
			m_nCityPointSceneId,	m_nCityPointPosX,	m_nCityPointPosY,
			m_nFubenReliveSceneId,	m_nFubenRelivePosX,	m_nFubenRelivePosY);
		*/

	}	
}

bool CActor::Init(void * pData, size_t size)
{	
	DECLARE_TIME_PROF("CActor::Init");
	//这里搞了一个
	EntityFlags & flag= GetAttriFlag();
	flag.CanAttackMonster =true;
	flag.CanSceneTelport =true;
	
	PACTORDBDATA  pActorData = (ACTORDBDATA *)pData;
	if(pActorData == NULL || size != sizeof(ACTORDBDATA))
	{
		const int actordbdatasize = sizeof(ACTORDBDATA);
		if(pActorData == NULL )
		{
			OutputMsg(rmWaning, _T("创建角色的数据长度错误，数据指针为空，可能是其他实体创建调用错了接口"));
		}
		else
		{
			OutputMsg(rmWaning, _T("创建角色的数据长度错误，初始化失败"));
		}
		return false;
	}

	if (!m_IsTestSimulator && !m_IsSimulator)
	{ 
		CLogicGateUser *pGateUser = GetLogicServer()->GetGateManager()->GetUserPtr(m_nGateID, m_lKey);
		if (pGateUser && (pGateUser->nCharId != pActorData->nID || pGateUser->nGlobalSessionId != pActorData->nAccountId))
		{
			//2017-1-11
			//登录客户端发来的nAccountID ，nActorID作校验,避免可以登录其他账号的角色
			OutputMsg(rmWaning, _T("登录异常: gateChar:%u,gateAccountid:%u,actorid:%u,accountid:%u"), pGateUser->nCharId, pActorData->nID, pGateUser->nGlobalSessionId, pActorData->nAccountId);
			return false;
		}
	}
	GetGlobalLogicEngine()->GetEntityMgr()->OnActorInit(pActorData->name, GetHandle());
	memcpy((char *)&m_property.nPkMod,(char *)&pActorData->nPkMod,sizeof(ACTORDBPARTDATA)); //全部拷贝到属性集
	
	//拷贝玩家的账户的信息
	memcpy(&m_sAccount, &pActorData->accountName, sizeof(m_sAccount));
	m_sAccount[sizeof(m_sAccount)-1] = 0;

	int nMaxAnger = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationMaxAnger(GetProperty< int>(PROP_ACTOR_VOCATION));
	m_property.nAnger = MAKELONG(m_property.nAnger,nMaxAnger);

	//朝向
	m_property.nDir = LOBYTE(LOWORD(pActorData->nActorDir)); //朝向
	
	m_nLastLogOutTime = pActorData->nLastLogoutTime;  //上次的登录时间

	CSimulatorMgr *pSimuMgr = GetGlobalLogicEngine()->GetSimulatorMgr(); 
	if (m_IsTestSimulator)
	{ 
		m_nAccountID = 0;//pSimuMgr->GetTestActorId();
	}
	else
	{ 
		m_nAccountID = pActorData->nAccountId; //玩家的账户id
	}

	m_nLoginTime.tv =GetGlobalLogicEngine()->getMiniDateTime();
	m_nUpdateTime = m_nLoginTime;
	m_nAtvTotalOnlineTime = m_nLoginTime;
	m_CurLoginTime.tv =GetGlobalLogicEngine()->getMiniDateTime();

	// m_property.nAlmirahLevel = 0x nFootPrintLv nWeaponExLv nSwingLv nFashionLv
	// data.nAlmirahLv			= 0x nFootPrintLv nSwingLv nWeaponExLv nFashionLv
	//db过来[时装，幻武，翅膀，空]
	WORD n12 = 0; //LOWORD(pActorData->nAlmirahLv);
	WORD n34 = 0; //HIWORD(pActorData->nAlmirahLv);
	byte nFashionLv = LOBYTE(n12);
	byte nWeaponExLv = HIBYTE(n12);
	byte nSwingLv = LOBYTE(n34);
	byte nFootPrintLv = HIBYTE(n34);
	//下发到客户端[时装，翅膀，幻武，空]
	//m_property.nAlmirahLevel = 0;  //MAKELONG(MAKEWORD(nFashionLv,nSwingLv),MAKEWORD(nWeaponExLv,nFootPrintLv));
	//m_property.nAlmirahSwingLv = HIWORD(pActorData->nAlmirahLv);

	// 初始化玩家当日打怪获取经验值	
	InitActorExpGetToday(pActorData->nExpToday);

	//初始化角色当日阵营贡献
	InitActorZyContriToday(pActorData->nZyContrToday);

	// 
	InitTomorrowDateTime();
	InitActorRelivePointData(pActorData);

	m_nCeateTime = pActorData->nCreateTime;
	m_consumeyb = pActorData->nConsumeYb;
	m_nLastLoginServerId = pActorData->nLastLoginServerId;
	m_nSwingLevel = pActorData->nSwingLevel;
	m_nSwingid = pActorData->nSwingId;
	m_LonginDays = pActorData->nLoginDays;
	m_nSalary = pActorData->nSalary;
	m_nFashionBattle = pActorData->nFashionBattle;
	m_NextChangeVocTime = pActorData->nNextChangeVocTime;
	m_nOldSrvId = pActorData->nOldSrvId;
	m_nAttackLevel = 100; // 玩家的攻击等级定死！
	if( Inherited::Init(pData, size) == false)
	{
		OutputMsg(rmWaning, _T("玩家的实体的基类初始化失败"));
		return false;
	}
	InitActorCampContr(); // 保证阵营子系统已经创建完毕
	SetEntityName(pActorData->name);
	if(m_chatSystem.Create(this, pData, size) == false)
	{
		OutputMsg(rmWaning, _T("聊天子系统初始化失败"));
		return false;
	}
	if(!m_Equiptment.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("Init fail for equip system "));
		return false; // 属性子系统的初始化
	}

	if(m_questSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning, _T("任务子系统初始化失败"));
		return false;
	}
	if (m_guildSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("帮派子系统初始化失败"));
		return false;
	}
	if(m_GameStoreBuySystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("GameStoreBuySubSystem creation failure..."));
		return false;
	}
	if(m_DealSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("DealSystem creation failure..."));
		return false;
	}
	if(m_lootSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("lootsystem creation failure..."));
		return false;
	}
	
	if(m_teamSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("team create failure..."));
		return false;
	}

	if(m_Bag.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for bag system "));
		return false; // 背包子系统的初始化
	}

	if (m_fubenSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("fuben create failure..."));
		return false;
	}
	if (m_msgSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("msgSystem create failure..."));
		return false;
	}
	if (m_pkSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("PkSystem create failure..."));
		return false;
	}
	if (m_deportSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("deport system create failure..."));
		return false;
	}
	if(m_miscSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("misc system create failure..."));
		return false;
	}
	if (!m_varSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning,_T("actor system create failure..."));
		return false;
	}
	if(!m_achieveSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("achieve system create failure..."));
		return false;
	}

	if (!m_petSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("pet system create failure..."));
		return false;
	}

	if (!m_heroSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("hero system create failure..."));
		return false;
	} 
	// if(!m_stallSystem.Create(this, pData, size))
	// {
	// 	OutputMsg(rmWaning, _T("stallsystem system create failure ..."));
	// 	return false;
	// }

	if(!m_friendSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("friendSystem system create failure ..."));
		return false;
	}

	if (!m_BossSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("BossSystem create failure ..."));
		return false;
	}
	if (!m_MailSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("MailSystem create failure ..."));
		return false;
	}
	if(!m_CkSpeedSystem.Create(this, pData, size))		//加速外挂检测子系统
	{
		OutputMsg(rmWaning, _T("m_CkSpeedSystem system create failure ..."));
		return false;
	}
	if (!m_NewTitleSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_NewTitleSystem create failure ..."));
		return false;
	}

	if (!m_AlmirahSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_AlmirahSystem create failure ..."));
		return false;
	}
	if (!m_ConsignmentSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_ConsignmentSystem create failure ..."));
		return false;
	}
	if (!m_CombatSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_CombatSystem create failure ..."));
		return false;
	}

	if(m_GameSets.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("Gamesetsystem creation failure..."));
		return false;
	}
	if(m_StaticCountSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("m_StaticCountSystem creation failure..."));
		return false;
	}

	if(m_BasicFunctionSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("m_BasicFunctionSystem creation failure"));
		return false;
	}

	if(!m_treasureSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_treasureSystem creation failure"));
		return false;
	}

	if (!m_activitySystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_activitySystem creation failure"));
		return false;
	} 

	if (!m_strengthenSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_strengthenSystem creation failure"));
		return false;
	}
	if (!m_nGhostSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nGhostSystem creation failure"));
		return false;
	}
	if (!m_nHallowsSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nHallowsSystem creation failure"));
		return false;
	}
	if (!m_nCCrossSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nCCrossSystem creation failure"));
		return false;
	}
	if (!m_ReviveDurationSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_ReviveDurationSystem creation failure"));
		return false;
	}
	if (!m_LootPetSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_LootPetSystem creation failure"));
		return false;
	}
	if (!m_RebateSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_RebateSystem creation failure"));
		return false;
	}

	m_asopCollector.SetActor(this);	
	
	const ENTITYPOS& pos = GetPosInfo();
	int nSceneId = pos.nSceneId, posx,posy;
	GetPosition(posx,posy);

	//多倍经验更新
	do
	{
		int openday = GetLogicServer()->GetDaysSinceOpenServer() ;
		int i = m_property.nLastLoginOpendayNo_ ;
		//今天首次登陆
		if(i < openday ){
			//开服天数有关的加多倍经验总量
			EXPLMTOPENDAY & expLmt = GetLogicServer()->GetDataProvider()->GetExpLmtOpenday() ;
			LONGLONG exp_i = 0 ;

			// // 2021-08-03 多倍经验 暂时不用
			// for( ; i<openday ; i++){
			// 	if( expLmt.m_expLmtOpenday.find(i+1) != expLmt.m_expLmtOpenday.end() ){
			// 		exp_i += ( LONGLONG )expLmt.m_expLmtOpenday[i+1].multilmt ;	
			// 	}
			// }
			//SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED , GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) +exp_i);
			//m_property.lMultiExpUnused += exp_i ;


			//合服天数
			int cmbDay = GetLogicServer()->GetDaysSinceCombineServer() ;

			//合服前三天每天多加100w多倍经验
			if(cmbDay>0 && cmbDay<=3)
			{
				SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED , (GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) + 1000000));
			}

			//合服操作
			if(cmbDay>0)
			{
				CMiniDateTime combineServerTime  = GetLogicServer()->GetServerCombineTime(); //获取合服的绝对时间
				if((combineServerTime.tv > 0) && (GetStaticCountSystem().GetStaticCount(nLastCombineSrvMinTime) != combineServerTime.tv ))
				{
					//合服天数差
					int diffDay = openday - m_property.nLastLoginOpendayNo_  ;
					//start  这里的逻辑只会在每次合服后的第一次登陆时执行
						//个人活动脚本处理
						m_activitySystem.OnCombineServer(diffDay);
						

					//end 
					GetStaticCountSystem().SetStaticCount(nLastCombineSrvMinTime,combineServerTime.tv) ;
				}
			}
			
			m_property.nLastLoginOpendayNo_ = openday ;//更新最后一次登录的开服天数
		}
 
	}while(0) ;

	
	if (nSceneId <= 0)
	{
		GLOBALCONFIG & gloCfg = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		nSceneId = gloCfg.nInitSceneId;
		posx = gloCfg.nInitScenePosX;
		posy = gloCfg.nInitScenePosY;
	}
	if( GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(this,pos.pFb,nSceneId,posx,posy) ==false )
	{
		OutputMsg(rmWaning,_T("actorID=%d,Init fail for EnterFuBen,posX=%d,posY=%d,nScenceID=%d"),m_property.nID,m_property.nPosX,m_property.nPosY,GetSceneID());
		return false;
	}
	UpdateUseDeopt();
	OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_ACTOR_SYSTEM);
	StartRequestActorSubSystemData();
	
	//补充玩家数据 ，增加一个消息
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcLoadCustomInfo);
	packet << GetRawServerIndex() << GetLogicServer()->GetServerIndex() << (unsigned int)GetId();

	if (!m_IsTestSimulator)
	{
		pDBClient->flushProtoPacket(packet);
	}
	OutputMsg(rmTip, _T("CActor::LoadDB, actorID=%u,accountID=%u"),	GetId(), m_nAccountID);

	if (m_IsSimulator)
	{
		OutputMsg(rmTip, _T("[simulator], 登录　actorID=%u,accountID=%u, Actorname = %s"),	GetId(), m_nAccountID, GetEntityName());
	}
	//GetGlobalLogicEngine()->GetTopTitleMgr().SendTopRankData(1, this);
	return true;
}

void CActor::StartRequestActorSubSystemData()
{
	bool bReqTrans = true;
	int nCurrServerId = GetLogicServer()->GetServerIndex();
	if (m_nLastLoginServerId == 0 || nCurrServerId == m_nLastLoginServerId)
	{
		bReqTrans = false;
	}
	else
	{
		bReqTrans = false;
	}	
	
	if (!bReqTrans)
	{
		RequestActorSubSystemDataImpl();
	}
	else
	{
		if(CanTransMit(m_nLastLoginServerId))
		{
			OutputMsg(rmTip, _T("上次登录的服务器[serverid=%d]不是原始服务器，请求传送"), m_nLastLoginServerId);
			RequestTransmitTo(m_nLastLoginServerId, enTransmitAtLogin);	
		}
		else
		{
			RequestActorSubSystemDataImpl();
		}
		
		
	}
}

void CActor::RequestActorSubSystemDataImpl()
{
	unsigned int nActorID = GetId();
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcQuerySubSystemData);
	int nServerId = GetLogicServer()->GetServerIndex();
	packet << GetRawServerIndex() << nServerId << nActorID;	
	pDBClient->flushProtoPacket(packet);
}

void CActor::SetCommonServerFlag()
{
	SetSocialMask(smIsInCommonServer, !IsInRawServer());
}

void CActor::OnGmTitle()
{
	// if (m_nGmLevel > 0 && m_nGmLevel <= GetLogicServer()->GetDataProvider()->GetGlobalConfig().nGmFlagLv)
	// {
	// 	SetSocialMask(smGmFlag, true);
	// }
	// else
	// {
	// 	SetSocialMask(smGmFlag, false);
	// }
	SetProperty<unsigned int>(PROP_ACTOR_GM_LEVEL, m_nGmLevel);
}

void CActor::OnFinishOneInitStep(int step)
{
	if(m_wInitLeftStep > 0)
	{
		OutputMsg(rmNormal, _T("CActor::OnFinishOneInitStep step=%d"), step);
		m_wInitLeftStep --;
		if(m_wInitLeftStep == 0)
		{
			OutputMsg(rmNormal, _T("CActor::OnFinishOneInitStep actor[%u] dbdata loaded, prepare send packet to client"), GetId());
			
			OnGmTitle();
			//GetPropertySystem().ResertProperty();
			GetEquipmentSystem().RefreshAppear(); //刷外观
			
			//int nGuildId= GetProperty<int>(PROP_ACTOR_GUILD_ID); //获取玩家的帮派ID
			//if(nGuildId !=0 )
			//{
			//	if( GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId) ==NULL)
			//	{
			//		SetProperty<int>(PROP_ACTOR_GUILD_ID,0);
			//	}
			//}
			
			SetInitFlag(true); //初始化好了
			SetCommonServerFlag();  //设置公共服务器的标记
			
			GetPropertySystem().ResertProperty();//因为最大内劲值是内存的上线要算一下
 
			if (!m_IsTestSimulator 
				&& !m_IsSimulator )
			{ 
				SendCreateData(); //下发创建的包
			}
			GetPropertyPtr()->ClearUpdateMask(); //这里要清除属性改变
			 
			if((int)OnGetCustomInfoByte(ACTORRULE_KICK_USER) == 1)
			{
				unsigned int nCurrTm = GetGlobalLogicEngine()->getMiniDateTime();
				unsigned int nFreeTm = OnGetCustomInfoUInt(ACTORRULE_KICK_USER_FREETIME); 
				if (nFreeTm <= nCurrTm)
				{
					ADD_CUSTOM_PROPERTY_BYTE(ACTORRULE_KICK_USER, 0); 
					ADD_CUSTOM_PROPERTY(ACTORRULE_KICK_USER_FREETIME, nCurrTm);
				}
				else
				{
					CActorPacket pack;
					AllocPacket(pack);
					pack << (BYTE) enDefaultEntitySystemID <<(BYTE)sActorForbid;
					pack.flush();
					//禁止 
					CloseActor(lwiBackStageTickActor, false); 
					OutputMsg(rmError, "玩家被后台禁止 ActorID = %u ", GetId());
					return;
				}
			}

			int nCustomTitleId = OnGetCustomInfoInt(ACTORRULE_CUSTOMTITLE_ID);
			if (  nCustomTitleId != 0 )
			{
				// 后台添加自定义称号
				if ( nCustomTitleId > 0 )
				{
					m_NewTitleSystem.addCustomTitle(nCustomTitleId);
				}
				else // 后台删除自定义称号
				{
					m_NewTitleSystem.delCustomTitle(-nCustomTitleId);
				}
				ADD_CUSTOM_PROPERTY_INT(ACTORRULE_CUSTOMTITLE_ID, 0);
			}

			//向好友服务器发送数据,通知一个玩家上现了
			//GetLogicServer()->GetFriendClient()->PostUserLogonIn(this,GetProperty<unsigned int>(PROP_ENTITY_ID) );
			//--------
			CMiniDateTime todaytime = CMiniDateTime::today();
			bool isNextDay = false;
			if(GetLastLogoutTime() < todaytime) //昨天下线的，已经跨天了
			{
				// GetMiscSystem().ClearCheckSignIn();
				// SetStaticVarValue(szDayHadExchangeExploitCount, 0);
				// GetHeroSystem().EveryDayClearBless();
				// GetStaticCountSystem().DailyRefresh();
				// GetTreasureSystem().RefreshBless();
				// GetAchieveSystem().ActorAchievementComplete(nAchieveLogin, 1);
				SYSTEMTIME sysTime;
				m_nLastLogOutTime.decode(sysTime);
				m_nLastLogOutTime.encode(sysTime.wYear,sysTime.wMonth,sysTime.wDay,0,0,0);
				typedef unsigned int uint;
				int dayDiff = (uint(todaytime) - uint(m_nLastLogOutTime) + 3600*24 - 1)/(3600*24);
				printf("logouttime:%d, today:%d\n",GetLastLogoutTime(), CMiniDateTime::today());
				if ((int)m_nLastLogOutTime == 0)
				{
					dayDiff = 0;
					GetAchieveSystem().ActorAchievementComplete(nAchieveContinueLogin, 1);
				}
				isNextDay = true;
				OnNewDayArrive(dayDiff);

			}
			LoginCloseActor2Center();
			// if (!m_nLastLogOutTime.isSameDay(m_nLoginTime))
			// {
			// 	int nlMaxDay = GetAchieveSystem().GetAchieveCompleteValueByType(nAchieveContinueLogin);
			// 	int nMaxDays = GetStaticCountSystem().GetStaticCount(nContinueLoginMaxTimes)+1;
			// 	if(nMaxDays > nlMaxDay)
			// 	{
			// 		nlMaxDay++;
			// 		int nLoginDay = (CMiniDateTime::today() - m_nLastLogOutTime.tv)/(24*3600);
			// 		if(nLoginDay <= 1)
			// 		{
			// 			nlMaxDay = nMaxDays;
			// 		}else
			// 		{
			// 			nMaxDays = 1;
			// 		}
			// 		GetAchieveSystem().ActorAchievementComplete(nAchieveContinueLogin, nlMaxDay);
					
			// 		GetStaticCountSystem().SetStaticCount(nContinueLoginMaxTimes, nMaxDays);
			// 	}
			// }
			// m_nLastLogOutTime = CMiniDateTime::now();
			////----------每日放在初始化 各个系统之前
			OnCreated();
			
			OnEnterScene();

			ResetShowName(); //重设玩家的showname
			if(isNextDay)
				GetAchieveSystem().ActorAchievementComplete(nAchieveLogin, 1);
			//下发计分器数据
			GetLogicServer()->GetLogicEngine()->GetScoreRecorderManager().ActorLogin(this);

			m_RundomKey = ((unsigned int) wrandvalue() + (unsigned int)_getTickCount() ) % (unsigned int) 26;
			 
			CScriptValueList paramList;
			//InitEventScriptVaueList(paramList,aeUserLogin);
			paramList << (int)(m_isFirstLogin ?1:0); //登录事件里添加一个参数，标明是不是第1次登录
			OnEvent(aeUserLogin,paramList,paramList);	
			// GetAchieveSystem().OnEvent(eAchieveEventLogin,0,m_LonginDays);
			GetAchieveSystem().SendAchieveData();
			paramList.clear();
			paramList << this;
			
			if (!m_IsTestSimulator
				&& !m_IsSimulator )
			{
				GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("SendPcClientState", paramList, paramList, 0);
			}
			m_activitySystem.OnUserLogin();
			GetGlobalLogicEngine()->GetActivityMgr().OnUserLogin(this);

			m_miscSystem.OnUserLogin();

			// 放在角色登录之后，保证脚本变量能正确初始化
			
			if (!CheckExpGetTodayUpLimit())	// 更新每日经验衰减buff
				OnExpGetTodayStatusChanged(false);
			CMiscMgr & miscMgr = GetGlobalLogicEngine()->GetMiscMgr();
			unsigned int nMyId = GetId();
			CRankingMgr & rankMgr = GetGlobalLogicEngine()->GetRankingMgr();

			//记录日志
			int nYb = GetProperty<int>(PROP_ACTOR_YUANBAO);
			int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
			if(GetLogicServer()->GetLogClient() && !m_IsTestSimulator && !m_IsSimulator)
			{
				CLogicGateUser *pGateUser= GetLogicServer()->GetGateManager()->GetUserPtr(m_nGateID,m_lKey);
				if(pGateUser)
				{
					_asncpytA(m_sIp, pGateUser->sIPAddr);
				}
				if (m_nLastLogOutTime == 0)
					GetLogicServer()->GetLogClient()->SendLoginLog(ltCreateActor,GetAccountID(),GetAccount(),m_sIp,GetEntityName(),nLevel, nYb, 0, GetId(), getOldSrvId());
				GetLogicServer()->GetLogClient()->SendLoginLog(ltEntryGame,GetAccountID(),GetAccount(),m_sIp,GetEntityName(),nLevel, nYb, 0, GetId(), getOldSrvId());
			}
			m_miscSystem.DealRecover();
			// 
			if (!m_IsTestSimulator
				&& !m_IsSimulator)
			{
				SendRankTips();
				CheckCombineMail();
			}
			//救主灵刃CD时间设置(针对离线CD重置的处理)
			SetJZLRCDTime(szJZLRCDTime);
			//记录日志
			if(GetLogicServer()->GetLocalClient())
			{
			}
			LoginCheckOfflineMail();

			m_RebateSystem.SendAward();			 
		}
	}
}


void CActor::OnCreated()
{
	DECLARE_TIME_PROF("CActor::OnCreated");
	m_teamSystem.OnEnterGame();
	m_friendSystem.OnEnterGame();
	m_miscSystem.OnEnterGame();
	m_fubenSystem.OnEnterGame();
	m_pkSystem.OnEnterGame();	
	m_achieveSystem.OnEnterGame();
	m_heroSystem.OnEnterGame();//
	GetSkillSystem().OnEnterGame();
	m_Equiptment.OnEnterGame();
	//m_stallSystem.OnEnterGame();
	m_guildSystem.OnEnterGame();
	m_questSystem.OnEnterGame();
	m_GameStoreBuySystem.OnEnterGame();
	m_msgSystem.OnEnterGame();
	m_CkSpeedSystem.OnEnterGame();
	m_AlmirahSystem.OnEnterGame();
	m_ConsignmentSystem.OnEnterGame();
	m_CombatSystem.OnEnterGame();
	m_StaticCountSystem.OnEnterGame();
	m_petSystem.OnEnterGame();
	m_activitySystem.OnEnterGame();
	m_Bag.OnEnterGame();
	m_chatSystem.OnEnterGame();
	m_BossSystem.OnEnterGame();
	m_NewTitleSystem.OnEnterGame();
	GetGlobalLogicEngine()->GetWorldLevelMgr().SendWorldLevel((CEntity*)this);
	GetGlobalLogicEngine()->GetMailMgr().OnEnterGame(this);
	GetGlobalLogicEngine()->GetActivityMgr().OnEnterScene(this);
	ResetShowName();
}

VOID  CActor::SendCreateData()
{
	//if(GetPosInfo().pScene ==NULL)
	//{
	//	OutputMsg(rmError,_T("actorID=%d,Init fail for SendCreateData,pScence=null"),m_property.nID);
	//	return;
	//}

	CActorPacket pack;
	CDataPacket& data = AllocPacket(pack);
	data << BYTE(enDefaultEntitySystemID) << BYTE(sCreateMainActor);
	data << (Uint64) m_hEntityHandler ; //自己的句柄
	INT_PTR nSize = GetPropertySize();
	//data.writeString(GetPosInfo().pScene->GetSceneData()->szScenceName); //场景名字
	//data.writeString(GetPosInfo().pScene->GetSceneData()->szMapFileName); //地图的名字
	data << (WORD)nSize;
	int i = this->GetProperty<unsigned int>(PROP_ENTITY_ID);
	data.writeBuf(GetPropertyDataPtr(), nSize);
	data.writeString(GetShowName());
	data << (unsigned int)m_NextChangeVocTime;

	//下发紫钻接口查询参数
	//unsigned int nTime = GetGlobalLogicEngine()->getMiniDateTime();
	//LPCTSTR key =  GetLogicServer()->GetVSPDefine().GetDefinition("YYZiZuanKey");
	//MD5_CTX md5_context; 
	//MD5Init(&md5_context); 
	//char sFormat[128];
	//char sign[64] = {0};
	//sprintf(sFormat,"&time=%u&account=%s&key=%s",nTime,GetAccount(),key);
	//MD5Update(&md5_context, (unsigned char*)sFormat, (unsigned int)strlen(sFormat));
	//MD5Final((unsigned char*)sign,&md5_context);

	//data << nTime;
	//data.writeString(GetAccount());
	//data.writeString(sign);

	pack.flush();

	if (!GetLogicServer()->IsCommonServer())
	{
		// 下发跨服开启状态
		bool boStartCS = GetLogicServer()->IsStartCommonServer();
		CActorPacket packet;
		CDataPacket &dataPacket = AllocPacket(packet);
		dataPacket << BYTE(enDefaultEntitySystemID) << BYTE(sOpenCommonServer);
		dataPacket << (BYTE)boStartCS;
		packet.flush();
	}
	
	//在角色之前，提前下发服务器的时间下去
	CActorPacket datePack;
	AllocPacket(datePack);
	datePack << (BYTE)enMiscSystemID << (BYTE)sSendServerTime ;
	datePack << (unsigned int)GetGlobalLogicEngine()->getMiniDateTime();
	datePack << (unsigned int)GetLogicServer()->GetServerOpenTime();
	datePack.flush();
	
	//buff的数据给发下去
	/*
	CActorPacket buffPack;
	CDataPacket& buffData = AllocPacket(buffPack);
	buffData<<(BYTE)enBuffSystemID << (BYTE)sInitMainActorBuffData ;
	buffPack.flush();
	*/
	//GetBuffSystem()->WriteBuffData(buffData);
	GetBuffSystem()->LoadFromScriptData();
	GetSkillSystem().InitNextSkillFlag();
	//GetAchieveSystem().LoadFromScriptData(); //装载称号的过期的数据

	UpdateNameClr_OnActorLogin(GetHandle());

	m_ConsignmentSystem.SenDClientRedPoint(false); //红点提示
}

void CActor::ChangePropertyValue(int propId,INT_PTR nValue, INT_PTR nAddTipId, INT_PTR nReduceTipId)
{
	if(nValue == 0)
	{
		return ;
	}
	int nOldValue = GetProperty<int>(propId);
	if (nValue > 0)
	{
		SendOldTipmsgFormatWithId(nAddTipId, ttTipmsgWindow, (int)nValue);
	}
	else
	{
		SendOldTipmsgFormatWithId(nReduceTipId, ttTipmsgWindow, -(int)nValue);
	}
	int nNewValue = (int)(nValue + nOldValue);
	if (nNewValue < 0)
	{
		nNewValue = 0;
	}
	SetProperty<unsigned int>(propId, (unsigned int)nNewValue);
	
}
void CActor::TriggerBeKilledEvent(CEntity* pKiller,int nExploit)
{
	// 触发死亡事件
	CScriptValueList paramList;
	//InitEventScriptVaueList(paramList, aeOnActorBeKilled); //
	paramList << (void*)pKiller << (int)nExploit;
	OnEvent(aeOnActorBeKilled,paramList,paramList);
}

void  CActor::OnKilledByEntity(CEntity * pKillerEntity)
{
	Inherited::OnKilledByEntity(pKillerEntity);
	if (!pKillerEntity ) 
	{
		OutputMsg(rmError, "OnKilledByEntity !pKillerEntity");
		DeathCancelBossBeLong();
		return;
	}
	INT_PTR nEntityType = pKillerEntity->GetType();
	OutputMsg(rmNormal, _T("OnKilledByEntity nKillerEntityType : %d"), nEntityType);
	if (nEntityType==enPet)
	{
		DeathCancelBossBeLong();
		CActor* pActor =  ((CPet*) pKillerEntity)->GetMaster();
		if(pActor)
		{
			OnKilledByEntity(pActor); 
		}
		return;
	}
	else if(nEntityType==enHero)
	{
		DeathCancelBossBeLong();
		CActor* pActor =  ((CHero*) pKillerEntity)->GetMaster();
		if(pActor)
		{
			OnKilledByEntity(pActor); 
		}
		return;
	}
	else
	{
		if(nEntityType == enActor)
		{
			DeathCancelBossBeLong((CActor *)pKillerEntity);
		}
	}

	//是否发各种提示
	bool showKillTips =  HideOtherPlayerInfo() ? false:true;

	if(pKillerEntity == this) return ; //被自己杀死了

	CScene *pScence = pKillerEntity->GetScene();
	char scenceName[32] = "";
	int x=0;
	int y=0;
	GetPosition(x,y);

	if(pScence)
	{
		_asncpytA(scenceName,pScence->GetSceneData()->szScenceName);
	}

	if(nEntityType == enActor && pKillerEntity->IsInited())
	{
		if(showKillTips)
		{
			((CActor*)pKillerEntity)->SendTipmsgFormatWithId(tmSkillKillEntity,tstFigthing,GetEntityName());
		}
		CScene *myScence = GetScene();
		if ( myScence )
		{
			char myScenceName[32] = "";
			_asncpytA(myScenceName,myScence->GetSceneData()->szScenceName);
			m_guildSystem.OnBeKilledByOtherTips((CActor *)pKillerEntity,myScenceName);
		}
		((CActor*)pKillerEntity)->GetFriendSystem().AddDeath(this, 1);
	}

/*
	if(nEntityType == enActor
		&& pKillerEntity->GetType() == enActor )
	{
		CActor* pActor = ((CActor*)pKillerEntity);
		CScriptValueList paramList;
		paramList <<  (void *)this;
		paramList << (void *)pKillerEntity;
		paramList << GetGlobalLogicEngine()->getMiniDateTime();
		pActor->OnEvent(aeOnActorBeKilled, paramList, paramList);
	}
*/
	{
		if(nEntityType == enActor)
		{
			//在特殊的场景里被杀不需要广播的，比如一些特殊的活动地图
			// if(showKillTips )
			// {
			// 	SendTipmsgFormatWithId(tmBeSkilled,tstFigthing,pKillerEntity->GetEntityName());
			// }
			SendTipmsgFormatWithId(tmBeSkilled,tstFigthing,pKillerEntity->GetEntityName());
		}
		// else
		// {
		// 	SendTipmsgFormatWithId(tmBeSkilledByOther,tstFigthing,pKillerEntity->GetEntityName());
		// }
		TriggerBeKilledEvent(pKillerEntity,0);		//通知前端复活面板显示扣了多少
	}
	CFuBen* pFb = GetFuBen();
	CScene* pScene = GetScene();
	if (pScene && pFb && !pFb->IsFb()) {
		LPTSTR sTitle = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeathMailTitle);
		char text[1024];
		LPTSTR sformat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeathMailContent);
		if(sformat) {
			sprintf_s(text, sizeof(text),sformat, (char*)(pScene->GetSceneName()),(char*)(pKillerEntity->GetEntityName()));
			CMailSystem::SendMail(GetId(), sTitle, text);
		}

	}
	
	if (nEntityType == enActor && showKillTips)
	{
		//int nCampTitle = GetProperty<int>(PROP_ACTOR_CAMP_TITLE);
		int nIsSameCampUser =1; //是否是本阵营的玩家
		CActor* pKillerActor = (CActor *)pKillerEntity;
		// if(!nIsSameCampUser)		//如果与被死的玩家不是同阵营，触发杀敌对阵营的任务事件
		// {
		// 	pKillerActor->GetQuestSystem()->OnQuestEvent(CQuestData::qtKillOtherCamp, 1, 1);
		// }
		//pk结束后的处理
		GetPkSystem().OnEndPk((CActor*)pKillerEntity);		//注意入参被杀者的对手的实例

		//取消褐名状态
		SetAttackOthersFlag(false);

		// //本人是对方仇人
		// if (((CActor*)pKillerEntity)->GetFriendSystem().GetSocialFlag(GetId(),SOCIAL_ANIMAL))
		// {
			
		// }

		CEntityManager* em	= GetGlobalLogicEngine()->GetEntityMgr();
		if(em)
		{
			CScene *myScence = GetScene();
			if ( myScence )
			{
				char myScenceName[32] = "";
				_asncpytA(myScenceName,myScence->GetSceneData()->szScenceName);
				std::string s1 = pKillerActor->GetEntityName();
				std::string s2 = GetEntityName();
				CGuild* pGuild = pKillerActor->GetGuildSystem()->GetGuildPtr();
				if (pGuild)
				{
					char killerGuildname[100] = {0};
					LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDescribeStr1);
					if(sFormat)
					{
						sprintf_s(killerGuildname, sizeof(killerGuildname), sFormat,pGuild->GetGuildName());
						s1 += killerGuildname;
					}
					
				}
				CGuild* pMyGuild = GetGuildSystem()->GetGuildPtr();
				if (pMyGuild)
				{
					char MyGuildname[100] = {0};
					LPCTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDescribeStr2);
					if(sFormat)
					{
						sprintf_s(MyGuildname, sizeof(MyGuildname), sFormat,pMyGuild->GetGuildName());
						s2 += MyGuildname;
					}
					
				}
				
				OutputMsg(rmTip, _T("玩家[%s] 在场景 [%s] 击败了 [%s]"), pKillerActor->GetEntityName(), myScenceName, s2.c_str() ); 
				em->BroadTipmsgWithParams(tmKilledByActor,  tstKillDrop ,s1.c_str(),myScenceName,s2.c_str());
				if (GetLogicServer()->IsCrossServer())
				{
					//跨服服务公布消息
					em->BroadTipmsgWithParamsToCs(tmKilledByActorCs,  tstKillDrop ,s1.c_str(),myScenceName,s2.c_str());
					 
					//OutputMsg(rmTip, "errerCode1646:em->BroadTipmsgWithParamsToCs:--------------------");
				} 
			}
			
		}
		
		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
	}
	if ( nEntityType == enMonster)
	{
		DeathCancelBossBeLong();
		CSkillSubSystem & skillSystem =  ((CAnimal*)pKillerEntity)->GetSkillSystem();
		const CVector<CSkillSubSystem::SKILLDATA> & skills= skillSystem.GetSkills();
		//这里加这个是为了一个技能
		for(INT_PTR i=0; i< skills.count();i++)
		{
			if(skills[i].bEvent == mSkilleventkill)
			{
				skillSystem.LaunchSkill(skills[i].nSkillID,x,y,true);
			}
		}

		//记录日志
		if(GetLogicServer()->GetLocalClient())
		{
		}
	}
	CVector<DROPITEM>  listDropItem;	//实际的掉落物品
	OnDeathDropItem();		//掉落
	//GetFriendSystem().OnDeath(pKillerEntity, listDropItem);
	ChangeRecordData(rRecordBeKilled);
	GetFriendSystem().AddDeath(pKillerEntity);
	/*
	//当判断玩家恶意pk其他玩家pk值 >=200, 立刻送其进入监狱,这个是版署服需求写的到时候正式服就去掉该判断处理
	if( GetLogicServer()->GetDataProvider()->GetPkConfig().redName <= ((CActor*)pKillerEntity)->GetProperty<int>(PROP_ACTOR_PK_VALUE) )
	{
		CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
		if (pNpc == NULL) return;
		CScriptValueList paramList, retParamList;
		paramList << pKillerEntity;
		pNpc->GetScript().Call("TranRedActorToRedSceen",paramList,retParamList );
	}
	*/
}

const char * CActor::GetMoneyTypeDesc (INT_PTR nMoneyType)
{
	int nTipmsgId =0;

	switch(nMoneyType)
	{
	case mtCoin:
		nTipmsgId = tpMoneyDescCoin;
		break;
	case mtBindCoin:
		nTipmsgId = tpMoneyDescBindCoin;
		break;
	case mtYuanbao:
		nTipmsgId = tpMoneyDescYuanbao;
		break;
	case mtBindYuanbao:
		nTipmsgId = tpMoneyDescBindYuanbao;
		break;
	default:
		return "";
	}	
	if(nTipmsgId )
	{
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgId);
	}
	return "";
}

void CActor::OnEntityDeath()
{
	if ( IsBeLongBossActor() )
	{
		DeathCancelBossBeLong();
	}
	
	//GetStallSystem().EndStall();	//角色死亡结束摆摊
	GetDealSystem().CancelDeal();
	Inherited::OnEntityDeath();
	SetProperty<int>(PROP_CREATURE_MP, 0);
	CScriptValueList paramList;
	//InitEventScriptVaueList(paramList,aeOnActorDeath);
	OnEvent(aeOnActorDeath,paramList,paramList);

	if (!CanDeathNotPunish())
	{
		GetEquipmentSystem().OnDeath();
	}
	
	CFuBen* pfb = GetFuBen();
	if (pfb)
	{
		pfb->AddActorDieCount();
	}

	int x,y;
	GetPosition(x,y);
	CScene* pScene = GetScene();

	
	m_petSystem.OnDeath();
	m_heroSystem.OnDeath();
	SendReliveRingCd();
	//OnDeathDropExp(); //死亡掉经验
}

void CActor::updateFuBenNpcDynProp(int nLevelUpdateVal)
{
	// 玩家等级变化，如果在副本中需要更新副本怪物动态属性
	//unsigned int nTeamID = GetProperty<unsigned int>(PROP_ACTOR_TEAM_ID);
	//CTeam *pTeam = GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamID);
	//if (pTeam)
	//{
	//	// 在小队中，更新小队所在的所有副本场景怪物动态属性
	//	GetGlobalLogicEngine()->GetTeamMgr().updateTeamMemFubenAveragePlayerLvl(nTeamID);
	//}
	//else
	//{
	//	// 不在小队中，如果在副本，更新副本怪物动态属性
	//	CScene *pScene = GetScene();
	//	if (pScene)
	//	{
	//		CFuBen *pFB = pScene->GetFuBen();
	//		if (pFB && pFB->IsFb())
	//			pFB->updateFubenPlayerAverageLvl(GetProperty<unsigned int>(PROP_CREATURE_LEVEL));		
	//	}
	//}
	CScene *pScene = GetScene();
	if (pScene)
	{
		CFuBen *pFB = pScene->GetFuBen();
		if (pFB && pFB->IsFb())
		{
			pFB->OnFbPlayerLevelChanged(nLevelUpdateVal);
		}
	}
}

void CActor::OnLevelUp(int nUpdateVal)
{
	Inherited::OnLevelUp(nUpdateVal);

	
	int nLevel = (int) GetProperty<unsigned int>(PROP_CREATURE_LEVEL);

	//触发脚本
	CScriptValueList  paramList;
	CScriptValueList  retParamList;
	//((CActor*)this)->InitEventScriptVaueList(paramList,(int)aeLevel);
	paramList <<nLevel;
	((CActor*)this)->OnEvent(aeLevel,paramList,retParamList);

	updateFuBenNpcDynProp(nUpdateVal);
	
	m_achieveSystem.OnLevelUp(); //先升级刷一下事件的订阅
	m_activitySystem.OnLevelUp();
	m_skillSystem.OnLevelUp(nLevel);

	//m_achieveSystem.OnEvent(eAchieveEventLevelUp,nLevel);		
	
	//GetSkillSystem().AutoLearnSkill(); //自动学习技能

	// 更新当日杀怪获取经验值
	//UpdateExpGetTodayWhenLevelUp();

	//通知任务系统
	//GetQuestSystem()->OnQuestEvent(CQuestData::qtActorLevel,0,nLevel,FALSE,this);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	//GetFriendSystem().OnLevelUp();
	if (GetTeam())
	{
		GetTeam()->BroadcastUpdateMember(this,PROP_CREATURE_LEVEL);
	}

	//职业宗师
	m_CombatSystem.OnLevelUp();
}

/*
nType	：传送类型
nValue	：传送值
nParam	：传送参数
*/
void CActor::OnEnterScene(int nType, int nValue, int nParam)
{
	if( IsInited() == false) return; //主角的创建包里会下发玩家的属性
	
	// 上线就删除离线数据
	GetGlobalLogicEngine()->GetOfflineUserMgr().RemoveUser(this->GetId());

	CScene *pScene = GetPosInfo().pScene;
	if(pScene == NULL)
	{
		return; 
	}
	
	CFuBen *pFb= pScene->GetFuBen();
	if (!m_IsTestSimulator
		&& !m_IsSimulator)
	{
		CActorPacket pack;
		CDataPacket & data = AllocPacket(pack);
		data << (BYTE) enDefaultEntitySystemID << (BYTE) sEnterScene ;
		data.writeString(pScene->GetSceneData()->szScenceName); //场景名字
		data.writeString(pScene->GetSceneData()->szMapFileName); //地图的名字
		data << (WORD) pScene->GetSceneId();
		
		if(pFb)
		{
			data << (WORD)pFb->GetFbId();
		}
		else
		{
			data << (WORD)0;
		}

		data << (WORD) GetProperty<unsigned int>( PROP_ENTITY_POSX);
		data << (WORD) GetProperty<unsigned int>( PROP_ENTITY_POSY);
		data << (unsigned int)GetProperty<unsigned int>(PROP_ACTOR_AREA_ATTR);	 //区域属性存储位
		data << (unsigned int)GetProperty<unsigned int>(PROP_ACTOR_AREA_ATTR_0);

		//传送类型&值
		data << (unsigned int)nType;		//传送类型
		data << (unsigned int)nValue;		//传送值
		data << (int)nParam;				//传送参数
		
		pack.flush(); 
	}
	int x,y;
	GetPosition(x,y);
	
	SCENEAREA* pArea = pScene->GetAreaAttri(x,y);	//直接返回区域属性，后面的函数判断直接用这个,避免多次获取
	if(pArea && !m_IsTestSimulator && !m_IsSimulator)
	{
		pScene->GetGrid().SendAreaConfig(pArea,this); 
	}

	//设置传送标记，观察者子系统那边要重置坐标，搞一个特效
	if( IsInited() == true)
	{
		//this->GetMoveSystem()->SetMoveFlag( CMovementSystem::mfTransport, true); //设置为传送标记
		((CMonster *)this)->CollectOperate(CEntityOPCollector::coTransport); 
	}
	if(pScene->GetFuBen())
	{
		// if( pScene->GetFuBen()->IsFb() )
		// {
		// 	GetAchieveSystem().OnEvent(eAchieveEventEnterFb, pScene->GetFuBen()->GetFbId());  //进入场景
		// }
	}

	//显示优化
	m_CanNetworkCutDown = false;
	if(CheckNetworkCutDown((int)pFb->GetFbId(), (int)pScene->GetSceneId()))
	{ 
		m_CanNetworkCutDown = true;
	}
	m_CanSeeEntityList.clear();
	InitCanDisplay();


	// 进入场景更新副本队伍平均等级
	//updateFuBenNpcDynProp(0);  进入场景会更新副本等级
	//GetAchieveSystem().OnEvent(aAchieveEventEnterScene, pScene->GetSceneId());				//触发进入场景的成就
	//GetQuestSystem()->OnQuestEvent(CQuestData::qtSearchScene, pScene->GetSceneId(), 1);		//触发探索场景的任务
	m_heroSystem.OnEnterScene();
	m_GameSets.OnEnterScene();
	m_friendSystem.SendSocialList(SOCIAL_FRIEND);
	m_friendSystem.SendSocialList(SOCIAL_FOLLOW);
	m_MailSystem.SendAllMail();
	m_achieveSystem.SendAchieveRedPoint();
	m_activitySystem.OnEnterScene();
	m_NewTitleSystem.OnEnterScene();
	m_petSystem.OnEnterScene();
	m_skillSystem.OnEnterScene();

	if (IsInited() == true)//在角色创建完前发这些数据包可能导致客户端出错
	{
		GetBagSystem().OnEnterScene();
		GetEquipmentSystem().OnEnterScene();
	}

	//进入场景后将自己名称颜色发给本人客户端（发给其他人是通过CObserverSystem::EntityAppear（）方法发送的）
	
	if (!m_IsTestSimulator
		&& !m_IsSimulator)
	{
		SendNameClr_ToSelf(this);
	}

	if( pArea && pScene->HasMapAttribute(x,y,aaNotCallHero, pArea) )
	{
		GetHeroSystem().CallbackBattleHero(true);
	}

	// 加载充值数据
	if (!m_IsTestSimulator
		&& !m_IsSimulator)
	{
		CDataPacket& dataPacket = GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcLoadFee);
		dataPacket << GetRawServerIndex() << GetLogicServer()->GetServerIndex();
		dataPacket << (int)(GetProperty<unsigned int>(PROP_ENTITY_ID));
		dataPacket.writeString(m_sAccount);
		 
		GetLogicServer()->GetDbClient()->flushProtoPacket(dataPacket); 
	}

	//进入场景的区域非膜拜君主区域
	/*
	if( pArea && !pScene->HasMapAttribute(x,y,aaWorshipMonarch, pArea) )
	{
		GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this, "ExitWorshipMonarchArea");
	}
	*/
	DealActorJDTime();
	int nLeftTime = GetJDJoinTime();
	if(nLeftTime)
	{
		nLeftTime -= time(NULL);
		CActorPacket ap;
		CDataPacket& data = AllocPacket(ap);
		data<<(BYTE)enMoveSystemID << (BYTE)sNpcTransform;
		data<<(int)nLeftTime;
		ap.flush();
	}

	// CheckCombineMail();
}

bool CActor::HasMapAttribute(BYTE bType, SCENEAREA* pArea)
{
	CFuBen * pFb  = GetFuBen();
	if (!pFb) 
		return false;

	CScene* pScene =  GetScene();
	if (!pScene)
	{
		return false;
	}
	int x,y;
	GetPosition(x,y);
	return pScene->HasMapAttribute(x, y, bType, pArea);
}

bool CActor::HasMapAttribute(BYTE bType, int value, SCENEAREA* pArea)
{
	CFuBen * pFb  = GetFuBen();
	if (!pFb) 
		return false;

	CScene* pScene =  GetScene();
	if ( !pScene )
	{
		return false;
	}
	int x,y;
	GetPosition(x,y);
	return pScene->HasMapAttribute(x, y, bType, value, pArea);
}

void CActor::SetEnterFuBenMapPos(int nSceneId,int x,int y)
{
	m_nEnterFbScenceID = nSceneId;
	m_nEnterFbPosx = x;
	m_nEnterFbPosy = y;
}

void CActor::OnPreExitScene()
{
	if(IsInited()==false) return;

	CFuBen * pFb  = GetFuBen();
	if(pFb ==NULL) return;
	CScene* pScene =  GetScene();
	int x,y;
	GetPosition(x,y);
	if( !pFb->IsFb()) //从普通场景里退出，要记录它进入普通场景的位置
	{
		if(pScene)
		{
			bool isInRawServer = IsInRawServer();
			if(!pScene->HasMapAttribute(x, y, aaReloadMap))
			{
				int nSceneId = pScene->GetSceneId();
				if (isInRawServer)
				{
					m_nEnterFbPosx = x;
					m_nEnterFbPosy = y;
					m_nEnterFbScenceID = nSceneId;
				}
				else
				{					
					m_commsrvPosBeforeEnterFB.SetData(nSceneId, x, y);
				}
				//OutputMsg(rmTip, _T("角色准备退出场景，更新进入副本前的位置信息(%d, %d_%d)"), nSceneId, x, y);
			}
			else
			{
				if (isInRawServer)
				{
					m_nEnterFbPosx = m_nNotReloadMapPosX;
					m_nEnterFbPosy = m_nNotReloadMapPosY;
					m_nEnterFbScenceID = m_nNotReloadMapSceneId;
				}
				else
				{					
					m_commsrvPosBeforeEnterFB.SetData(m_nNotReloadMapSceneId, m_nNotReloadMapPosX, m_nNotReloadMapPosY);
				}
				/*
				OutputMsg(rmTip, _T("角色准备退出场景[退出不保存地图]，更新进入副本前的位置信息(%d, %d_%d)"), 
					m_nNotReloadMapSceneId, 
					m_nNotReloadMapPosX, 
					m_nNotReloadMapPosY);
				*/

			}
		}
	}
	
	//删除场景区域添加的buff
	SCENEAREA* pArea = pScene?pScene->GetAreaAttri(x,y):NULL;
	if (pArea)
	{
		RemoveAreaBuff(pArea);

		if (pScene->HasMapAttribute(x,y,aaForcePkMode,pArea))		//恢复上一次的pk模式
		{
			CPkSystem& pk = GetPkSystem();
			pk.SetPkMode(pk.m_bLastMode,true);
		}

		if (pScene->HasMapAttribute(x,y,asSceneAreaMode,pArea))//进入某个区域给玩家设置标记
		{
			AreaAttri& attri = pArea->attri[asSceneAreaMode];
			if (attri.nCount > 0)
			{
				SetSceneAreaFlag(attri.pValues[0]);
			}
		}
		else
		{
			SetSceneAreaFlag(0);
		}

		if (pScene->HasMapAttribute(x, y, aaCannotSeeName, pArea))
		{
			SetSocialMask(smHideHeadMsg, false);
			CollectOperate(CEntityOPCollector::coRefFeature);
			CHero * pHero = GetHeroSystem().GetBattleHeroPtr();
			if(pHero)
			{
				pHero->SetSocialMask(smHideHeadMsg, false);
				pHero->CollectOperate(CEntityOPCollector::coRefFeature);
			}
		}
		if (pScene->HasMapAttribute(x,y,aaForcePkMode,fpUnion))
		{
			GetGuildSystem()->BroadCastGuildPlayerPos(gptDisappear);
		}
	}
}

void CActor::SaveOfflinePropertyData()
{
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcSaveOfflineInfo);
	int nServerId = GetLogicServer()->GetServerIndex();
	unsigned int nActorID = GetId();
	packet << GetRawServerIndex() << nServerId << nActorID;
	INT_PTR nSize = GetPropertySize();
	assert(sizeof(CActorOfflineProperty) == nSize);
	packet.writeBuf(GetPropertyDataPtr(),nSize);
	pDBClient->flushProtoPacket(packet);
}

void CActor::PackActorBasicData(ACTORDBDATA &data, int nLogoutServerId)
{
	memset(&data,0,sizeof(data));
	memcpy(&data,m_property.GetValuePtr(0),sizeof(ENTITYDATA));
	memcpy( ((char*)&data)+ sizeof(ENTITYDATA) ,m_property.GetValuePtr(PROP_ACTOR_PK_MOD),sizeof(ACTORDBPARTDATA));
	data.nPlayerMaxAttack = GetActorAttackValueMax();
	data.nHp = m_property.nHp;
	data.nMp =m_property.nMp;
	data.nLevel = m_property.nLevel;
	data.nLastLogoutTime = m_nLastLogOutTime;
	data.nLastLoginTime = m_nLoginTime;
	data.lLastLoginIp = m_lLastLoginIp;
	
	data.nLastLoginOpendayNo_  = m_property.nLastLoginOpendayNo_;
	data.lMultiExpUnused  = m_property.lMultiExpUnused;
	//PK
	data.nPKValue = m_property.nPKValue;
	data.nMedicineCardTime  = m_property.nMedicineCardTime;
	//金钱
	data.nBagGridCount = m_property.nBagGridCount;
	data.nDeportGridCount = m_property.nDeportGridCount;
	
	data.nBindCoin = m_property.nBindCoin;
	data.nGuildExp = m_property.nGuildExp;
	data.nBindYuanbao = m_property.nBindYuanbao;
	data.nNonBindCoin = m_property.nNonBindCoin;
	data.nNonBindYuanbao = m_property.nNonBindYuanbao;

	data.nDepotCoin = m_property.nDepotCoin;
	data.nOffice = m_property.nOffice;

	//喇叭喇叭和飞鞋
	data.nBroatNum = m_property.nBroatNum;
	data.nFlyShoes = m_property.nFlyShoes;
	data.nMeridianLv = m_property.nMeridianLv;

	data.nRecyclepoints = m_property.nRecyclepoints;
	// data.nHair = m_property.nHair;

	data.nWingPoint = m_property.nWingPoint;	//羽魂
	
	data.nSupperPlayLvl = m_property.nSupperPlayLvl;
	data.nFrenzy = m_property.nFrenzy;

	


	data.nRecoverState  = m_property.nRecoverState;
	data.nLootPetId = m_property.nLootPetId;
	data.nPersonBossJifen = m_property.nPersonBossJifen;

	//经验
	data.lExp = m_property.lExp;
	data.nExpToday = m_nExpGetToday;
	unsigned int nDir = GetProperty<unsigned int>(PROP_ENTITY_DIR);//实体的朝向
 
	//vip
	data.nForeverCardFlag = m_property.nForeverCardFlag;
	data.nMonthCardTime = m_property.nMonthCardTime;

	data.nAnger = LOWORD(m_property.nAnger);
	
	CGuild* pGuild = GetGuildSystem()->GetGuildPtr();
	if (pGuild)
	{
		strcpy(data.sGuildname,pGuild->GetGuildName());
	}

	//朝向存盘
	data.nActorDir= MAKELONG(MAKEWORD((BYTE)nDir,0),0 ); //第1个字节为朝向
	data.nScenceID = GetSceneID();

	data.nLastLogoutTime = GetGlobalLogicEngine()->getMiniDateTime();

	int nOnlineTime = data.nLastLogoutTime -  m_nUpdateTime.tv;//在线时间
	if(nOnlineTime <0)
	{
		nOnlineTime =0;
	}
	m_nUpdateTime.tv = data.nLastLogoutTime;
	data.nTotalOnlineMin = nOnlineTime;
	
	// m_property.nAlmirahLevel = 0x nFootPrintLv nWeaponExLv nSwingLv nFashionLv
	// data.nAlmirahLv			= 0x nFootPrintLv nSwingLv nWeaponExLv nFashionLv
	//WORD n12 = LOWORD(m_property.nAlmirahLevel);
	//WORD n34 = HIWORD(m_property.nAlmirahLevel);
	WORD n12 = LOWORD(0);
	WORD n34 = HIWORD(0);
	byte nFashionLv = LOBYTE(n12);
	byte nSwingLv = HIBYTE(n12);
	byte nWeaponExLv = LOBYTE(n34);
	byte nFootPrintLv = HIBYTE(n34);
	data.nAlmirahLv = MAKELONG(MAKEWORD(nFashionLv,nWeaponExLv), MAKEWORD(nSwingLv,nFootPrintLv)); 

	// 保存角色复活点数据
	SaveActorRelivePointData(data);

	//data.nEquipScore =GetProperty<unsigned int>(PROP_ACTOR_EQUIP_SCORE);; //宠物的装备的得分  
	data.nConsumeYb =  (unsigned int) m_consumeyb;

	data.nZyContrToday = m_nZyContriGetToday; //今天获得的阵营贡献的数值	
	data.nLastLoginServerId = nLogoutServerId > 0 ? nLogoutServerId : GetLogicServer()->GetServerIndex();
	data.nSwingLevel = m_nSwingLevel;
	data.nSwingId = m_nSwingid;
	data.nLoginDays = m_LonginDays;
	data.nSalary = m_nSalary;
	data.nFashionBattle = m_nFashionBattle;
	data.nNextChangeVocTime = m_NextChangeVocTime;

	bool bInRawServer = IsInRawServer();
	CONST ENTITYPOS & pos=  GetPosInfo( );
	if(pos.pFb && pos.pScene)
	{
		if (bInRawServer)
		{
			bool isReloadMap = pos.pScene->HasMapAttribute(m_property.nPosX,m_property.nPosY,aaReloadMap);//是否重配地图，如果是，不保存这个坐标
			data.nFbHandle = pos.pFb->GetHandle(); //获取handle
			if (!pos.pFb->IsFb())//如果本身就在常规场景中
			{
				if (isReloadMap)
				{
					data.nEnterFbPos = MAKELONG(m_nNotReloadMapPosX,m_nNotReloadMapPosY);//
					data.nEnterFbScenceID = m_nNotReloadMapSceneId;
				}else
				{
					data.nEnterFbPos = MAKELONG(m_property.nPosX,m_property.nPosY);//
					data.nEnterFbScenceID = GetSceneID();
				}
			}
			else		//目前在副本中
			{
				//进入副本前的坐标
				if (isReloadMap)
				{
					data.nEnterFbPos = MAKELONG(m_nNotReloadMapPosX,m_nNotReloadMapPosY);//
					data.nEnterFbScenceID = m_nNotReloadMapSceneId;
					data.nFbHandle = 0;
				}else
				{
					data.nEnterFbPos =  MAKELONG(m_nEnterFbPosx,m_nEnterFbPosy);	//这个坐标保证不是重配地图的区域,在设置这个值的时候就判断了
					data.nEnterFbScenceID =m_nEnterFbScenceID;
				}
			}
			if (isReloadMap)
			{
				data.nPosX = m_nNotReloadMapPosX;
				data.nPosY = m_nNotReloadMapPosY;
				data.nScenceID = m_nNotReloadMapSceneId;//这个要修改成非重配地图的区域，避免被保存
			}
		}
		else
		{
			OutputMsg(rmTip, _T("在跨服服务器打包角色[actorId=%d]场景位置相关信息数据"), GetId());
			// 将原始服务器的相关信息存盘，不保存跨服服务器的任何场景信息
			// 跨服之后再回去，自动退出副本
			data.nFbHandle = 0;
			// 进入fb前位置
			data.nEnterFbPos = MAKELONG(m_nEnterFbPosx, m_nEnterFbPosy);
			data.nEnterFbScenceID = m_nEnterFbScenceID;
			OutputMsg(rmTip, _T("角色在原始服务器进入副本前位置信息(%d, %d_%d)"), data.nEnterFbScenceID, m_nEnterFbPosx, m_nEnterFbPosy);
			// 角色存盘时位置
			data.nPosX = m_rawsrcScenePos.nPosX;
			data.nPosY = m_rawsrcScenePos.nPosY;
			data.nScenceID = m_rawsrcScenePos.nSceneId;
			OutputMsg(rmTip, _T("角色在原始服务器位置信息(%d, %d_%d)"), data.nScenceID, data.nPosX, data.nPosY);
		}
	}
}

VOID CActor::SaveDb(bool bLogout, int nLogoutServerId, bool bNeedDBAck)
{
	if (m_nTransmitStatus >= enTS_StartTransmit)
		return;

	//GetDealSystem().CancelDeal();
	//OutputMsg(rmTip,_T("accountID=%d,actorID=%d, Save data"),m_nAccountID,m_property.nID);
	ACTORDBDATA data; //实体的存盘数据
	PackActorBasicData(data, nLogoutServerId);
	GetSkillSystem().Save(&data);
	// GetQuestSystem()->Save(&data);
	GetGameSetsSystem().SaveToDB();//...
	GetBagSystem().Save(&data);
	GetEquipmentSystem().Save(&data);
	GetGuildSystem()->Save(&data);
	GetDeportSystem().Save(&data);
	m_miscSystem.Save(&data); //这里有元宝的种植的数据

	//GetChallengeSystem().Save(&data);
	m_achieveSystem.Save(&data); //称号成就存盘
	GetChatSystem()->SaveToScriptData(&data);	
	GetBuffSystem()->SaveToScriptData();
	//GetAchieveSystem().SaveToScriptData();

	m_varSystem.Save(&data);
	m_heroSystem.Save(&data);
	m_questSystem.Save(&data);
	GetFriendSystem().SaveToDb();
	GetFriendSystem().SaveActorDeathToDb();
	// GetFriendSystem().SaveDeathDropToDb();
	m_activitySystem.Save(&data);

	//GetLogicServer()->GetDbClient()->SendDbServerData(jxInterSrvComm::DbServerProto::dcSave, data);
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcSave);
	int nServerId = GetLogicServer()->GetServerIndex();
	packet << GetRawServerIndex() << nServerId << data << m_property.nTradingQuota;
	pDBClient->flushProtoPacket(packet);

	m_BossSystem.Save(&data);

	m_MailSystem.Save(&data);

	m_NewTitleSystem.SaveToDB();

	m_AlmirahSystem.Save(&data);	// 衣橱系统

	m_CombatSystem.Save(&data);

	m_StaticCountSystem.SaveToDb();
	m_strengthenSystem.SaveToDb();
	m_nGhostSystem.SaveToDb();
	m_nHallowsSystem.OnSaveToDb();
	m_ReviveDurationSystem.SaveToDB();
	m_LootPetSystem.SaveToDB();
	if (bLogout)
	{		
		CDataPacket &logoutPacket = pDBClient->allocProtoPacket(DbServerProto::dcLogout);
		logoutPacket << GetRawServerIndex() << nServerId << GetId() << bNeedDBAck;
		pDBClient->flushProtoPacket(logoutPacket);

	}
}

VOID CActor::SaveBasicData()
{
	if (m_nTransmitStatus >= enTS_StartTransmit)
		return;

	ACTORDBDATA data; //实体的存盘数据
	PackActorBasicData(data, 0);
	GetGuildSystem()->Save(&data);
	m_achieveSystem.Save(&data);
	GetChatSystem()->SaveToScriptData(&data);
	m_questSystem.Save(&data);

	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcSave);
	int nServerId = GetLogicServer()->GetServerIndex();
	packet << GetRawServerIndex() << nServerId << data << m_property.nTradingQuota;
	pDBClient->flushProtoPacket(packet); 
}


void CActor::HandleNetworkData(INT_PTR nSystemID, INT_PTR nCmd,CDataPacket &pack)
{
	DECLARE_TIME_PROF("CActor::HandleNetworkData");
	if (GetGlobalLogicEngine()->GetActorMsgFilter().Apply((const int)nSystemID, (const int)nCmd))
		return;

	// 脚本处理协议（假如有的话）
	if(NetMsgDispatcher::ProcessNetMsg(this, nSystemID, nCmd, pack))
	{
		return;
	}

	if(nSystemID == enCheckSpeedSystemID)	//为了方便断点调试，放外面
	{
		return m_CkSpeedSystem.ProcessNetData(nCmd,pack);
	}

#ifdef _DEBUG_GATEMSG
	OutputMsg(rmTip, _T("CActor::HandleNetworkData:%d"),nSystemID);
#endif
	switch (nSystemID)
	{
		//没有子系统要的都放到这里处理
	case enDefaultEntitySystemID:
		ProcessDefaultNetData(nCmd,pack); //
		break;
		//移动子系统处理
	case enMoveSystemID:
		m_moveSystem.ProcessNetData(nCmd,pack);
		break;
	case enGameSetsSystemID:
		m_GameSets.ProcessNetData(nCmd,pack);
		break;
		//buff子系统处理
	case enBuffSystemID:
		m_buffSystem.ProcessNetData(nCmd,pack);
		break;
		//任务子系统
	case enQuestSystemID:
		m_questSystem.ProcessNetData(nCmd,pack);
		break;
		//既能子系统
	case enSkillSystemID:
		m_skillSystem.ProcessNetData(nCmd,pack);
		break;
	case enChatSystemID:
		m_chatSystem.ProcessNetData(nCmd,pack);
		break;
	case enBagSystemID:
		m_Bag.ProcessNetData(nCmd,pack);
		break;
	case enEuipSystemID:
		m_Equiptment.ProcessNetData(nCmd,pack);
		break;
	case enStoreSystemID:
		m_GameStoreBuySystem.ProcessNetData(nCmd,pack);
		break;
	case enGuildSystemID:
		GetGuildSystem()->ProcessNetData(nCmd,pack);
		break;
	case enDealSystemID:
		m_DealSystem.ProcessNetData(nCmd, pack);
		break;
	case enLootSystemID:
		m_lootSystem.ProcessNetData(nCmd,pack);
		break;
	case enTeamSystemID:
		m_teamSystem.ProcessNetData(nCmd,pack);
		break;
	case enStrengthenSystemID:
		m_strengthenSystem.ProcessNetData(nCmd,pack);
		break;
	case enFubenSystemID:
		m_fubenSystem.ProcessNetData(nCmd,pack);
		break;
	case enPkSystemID:
		m_pkSystem.ProcessNetData(nCmd,pack);
		break;
	case enDepotSystemID:
		m_deportSystem.ProcessNetData(nCmd,pack);
		break;
	case enMiscSystemID:
		m_miscSystem.ProcessNetData(nCmd,pack);
		break;
	case enConsignmentSystemID:
		m_ConsignmentSystem.ProcessNetData(nCmd,pack);
		break;
	case enMsgSystemID:
		m_msgSystem.ProcessNetData(nCmd,pack);
		break;
	case enAchieveSystemID:
		m_achieveSystem.ProcessNetData(nCmd,pack);
		break;
	case enPetSystemID:
		m_petSystem.ProcessNetData(nCmd,pack);
		break;
	// case enStallSystemID:
	// 	m_stallSystem.ProcessNetData(nCmd, pack);
	// 	break;
	case enFriendLogicSystemID:
		m_friendSystem.ProcessNetData(nCmd,pack);
		break;
	case enHeroSystemId:
		m_heroSystem.ProcessNetData(nCmd,pack);
		break;
	case enBossSystemID:
		m_BossSystem.ProcessNetData(nCmd, pack);
		break;
	case enMailSystemID:
		m_MailSystem.ProcessNetData(nCmd, pack);
		break;
	case enCheckSpeedSystemID:			//加速检测外挂子系统
		m_CkSpeedSystem.ProcessNetData(nCmd,pack);
		break;
	case enNewTitleSystemID:
		m_NewTitleSystem.ProcessNetData(nCmd,pack);
		break;
	case enAlmirahSystemID:
		m_AlmirahSystem.ProcessNetData(nCmd,pack);
		break;
	case enCombatSystemID:
		m_CombatSystem.ProcessNetData(nCmd, pack);
		break;
	case enStaticCountSystemID:
		m_StaticCountSystem.ProcessNetData(nCmd, pack);
		break;
	case enBasicFunctionsSystemID:
		m_BasicFunctionSystem.ProcessNetData(nCmd, pack);
		break;
	case enTreasureSystem:
		m_treasureSystem.ProcessNetData(nCmd,pack);
		break;
	case enActivityID:
		m_activitySystem.ProcessNetData(nCmd,pack);
		break;
	case enGhostSystemID:
		m_nGhostSystem.ProcessNetData(nCmd,pack);
		break;
	case enHallowsSystemID:
		m_nHallowsSystem.ProcessNetData(nCmd,pack);
		break;
	case enCrossServerSystemID:
		m_nCCrossSystem.ProcessNetData(nCmd,pack);
		break;
	case enLootPetSystemID:
		m_LootPetSystem.ProcessNetData(nCmd,pack);
		break;
	default:
		break;
	}
}

//定时器的ID
VOID CActor::LogicRun(TICKCOUNT nCurrentTime)
{
	if(m_IsTestSimulator || m_IsSimulator)
	{ 
		return;
	}
	//心跳超时
	if(m_nRecvHeatbeatTickCount && nCurrentTime - m_nRecvHeatbeatTickCount >= 20000)
	{
		#ifndef _DEBUG
		CloseActor(false); //正式运营的时候要开启
		#endif
	}
	
	if (m_nTransmitStatus >= enTS_StartTransmit)
		return;

	if ( !IsInited() || IsDestory() )
	{
		return;
	}

	DECLARE_TIME_PROF("CActor::LogicRun");
	Inherited::LogicRun(nCurrentTime);

	CEntityManager * pMgr = GetGlobalLogicEngine()->GetEntityMgr();
	//这里要进行分时的处理
	TICKCOUNT nStart= _getTickCount();
	TICKCOUNT nLimitTime = pMgr->GetOneEntityTime(); //一个实体的处理时间	
	CLogicGate* logicgate = (CLogicGate*)GetLogicServer()->GetGateManager()->getGate(m_nGateID);	
	
	bool isOutTime= false; //已经超时了

	if(m_HeartBeatTime.CheckAndSet(nCurrentTime))
	{		
		SendHeartBeatPack(); //发送心跳，优先发送，避免超时不发，5分钟踢下线
	}
	while (m_netMsgPool.count() > 0)
	{
		CDataPacket* pMsg = m_netMsgPool[0];		
		BYTE nSystemId = 0, nCmdId = 0;		
		(*pMsg) >> m_lGateTickCount >> m_llRcvPacketTick >> nSystemId >> nCmdId;
		if (logicgate)
			logicgate->AddLogicProcessedData(pMsg);

 		m_netMsgPool.remove(0);		// 删除此消息
		HandleNetworkData(nSystemId, nCmdId, *pMsg);
		if (_getTickCount() - nStart > nLimitTime)
		{
			isOutTime = true;
			break;
		}
	}	


	//如果超时了就不处理
	if(isOutTime)
	{
		return;
	}

	if (m_saveDbTime.Check(nCurrentTime))
	{		
		m_saveDbTime.SetNextHitTimeFromNow(m_sSaveDBInterval);
		SaveDb(false);
	}
	else if (m_saveBasicTime.CheckAndSet(nCurrentTime))
	{ 
		if (!OnGetIsSimulator() && !OnGetIsTestSimulator())
		{
			SaveBasicData();
		}
	}
	
	if(m_t1minute.CheckAndSet(nCurrentTime))
	{
		//这个不需要太频繁
		m_Bag.OnTimeCheck(nCurrentTime);
		m_questSystem.OnTimeCheck(nCurrentTime);
		m_achieveSystem.OnTimeCheck(nCurrentTime); //检测称号的过期
		m_petSystem.OnTimeCheck(nCurrentTime); //检测宠物的忠诚度
		if (m_bTracePacket)
		{
			m_sPacketStat.WriteToFile(this);
			m_sPacketStat.Clear();
		}
		m_miscSystem.OnTimeCheck(nCurrentTime); //检测防沉迷
		ChangeRecordData(rRecordOnLineTime);
		DealActorJDTime(); 
		//更新在线的心跳 
	}

	if (m_t1s.CheckAndSet(nCurrentTime))
	{
		DECLARE_TIME_PROF("Actor::1sTimer")
		//清理超时的任务以及下发		
		m_guildSystem.OnTimeCheck(nCurrentTime);
		m_fubenSystem.OnTimeCheck(nCurrentTime);
		m_pkSystem.OnTimeCheck(nCurrentTime);//要3分钟减少一点杀戮值,还要检查切磋是否超时
		m_BossSystem.OnTimeCheck(nCurrentTime);
		ReliveTimeOut();//处理复活时间是否超时
		//超人,GM
		m_asopCollector.Update(nCurrentTime);

		//检查区域属性并做相应的处理
		m_NewTitleSystem.OnTimeCheck(nCurrentTime);

		//检查血饮狂刀是否触发以及相关Buff的移除
		CheckXYKD();

		//检查区域属性并做相应的处理
		m_ReviveDurationSystem.OnTimeCheck(nCurrentTime);

		//宠物系统非 ai宠物
		m_LootPetSystem.OnTimeCheck(nCurrentTime);

		//返利系统
		m_RebateSystem.OnTimeCheck(nCurrentTime);
	}
	//3秒定时器
	if(m_t3s.CheckAndSet(nCurrentTime))
	{
		DECLARE_TIME_PROF("Actor:3sTimer")
		
		if (!OnGetIsSimulator()
			&& !OnGetIsTestSimulator())//虚拟人不增加泡点经验
		{
			//泡点
			AddPaoDianExp();
		}
	}

	//5秒定时器
	if(m_5s.CheckAndSet(nCurrentTime))
	{
		DECLARE_TIME_PROF("Actor:5sTimer")
		SendKeepAliveToGate();
		SendReliveRingCd();				//发送复活戒指到期的CD时间
		m_activitySystem.RunOne_5s(nCurrentTime);
		AreaCostCoin();
	}
	//10秒定时器
	if(m_t15s.CheckAndSet(nCurrentTime))
	{
		DECLARE_TIME_PROF("Actor:10sTimer")
		SendScriptTotalOnlineTime();
	}

	if (m_EquipChkTimer.CheckAndSet(nCurrentTime))
	{
		m_Equiptment.OnTimeCheck(nCurrentTime);
	} 
	if (m_t5minute.CheckAndSet(nCurrentTime))
	{
		
	}

	if (m_t12minute.CheckAndSet(nCurrentTime))
	{
		m_CombatSystem.OnTimeCheck(nCurrentTime);
	}

	if(m_isSuperman)
	{
		SetProperty<unsigned int>(PROP_CREATURE_HP, GetProperty<unsigned int>(PROP_CREATURE_MAXHP));
		SetProperty<unsigned int>(PROP_CREATURE_MP, GetProperty<unsigned int>(PROP_CREATURE_MAXMP));
		//SetProperty<unsigned int>( PROP_ACTOR_JUMP_POWER,GetProperty<unsigned int>(PROP_ACTOR_MAX_JUMP_POWER));
	}

	if(GetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS))
	{
		int nTime = GetGlobalLogicEngine()->getMiniDateTime();
		if(nTime - m_AttackOthersTime > GetLogicServer()->GetDataProvider()->GetPkConfig().m_ClearTime)
		{
			m_AttackOthersTime = 0;
			SetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS,0);
			SendOldTipmsgWithId(tpOutPkState,ttTipmsgWindow);
			UpdateNameClr_OnPkModeChanged(GetHandle());
		}
	}
	
	if (m_SecTimer.CheckAndSet(nCurrentTime, true))
	{
		//保存实时离线数据，排行榜查询用
		if(m_isInited == true)
		{ 
			//下线存盘
			OutputMsg(rmTip,"[Close] 玩家下线 Offline Actorname=%s ", this->GetEntityName());
			SaveOfflinePropertyData();
			
			SaveCustomInfoData();
		}
	}
 
}

//收到客户端的数据
void CActor::ProcessDefaultNetData( INT_PTR nCmd,CDataPacket &pack )
{
	switch (nCmd)
	{
	case cCheckTime:
		{
			// 用以计算网络延时
			CActorPacket ret_pack;
			CDataPacket & ret_data = AllocPacket(ret_pack);
			ret_data << (BYTE) enDefaultEntitySystemID << (BYTE) sCheckTimeBack;
			ret_pack.flush();			
		}
		break;
	case cHeartbeat:
		{
			TICKCOUNT nCurrent= GetLogicCurrTickCount();
			unsigned int nCurrentTick =0;
			pack >> nCurrentTick;

			bool isValid =true;

			if(m_nRecvHeatbeatTickCount >0)
			{
				int nDisTicks =(int) (nCurrent - m_nRecvHeatbeatTickCount);
				//如果超时了15秒,或者低于1秒，说明说明客户端出现了问题，要踢出 
				if(nDisTicks >= 20000 || nDisTicks < 1000) //
				{
					//OutputMsg(rmWaning,_T("ActorID=%u,Heart Beat Distance =%d ms,not correct"),GetProperty<unsigned>(PROP_ENTITY_ID),nDisTicks);
					isValid =false;
				}
			}

			if( m_dwClientTicksValue >0)
			{
				LONGLONG nDisTicks  = nCurrentTick - m_dwClientTicksValue;
				if( nDisTicks > 80000 )
				{
					isValid =false;
					OutputMsg(rmWaning,_T("ActorID=%u,Client tickCount Distance =%d ms,not correct"),GetProperty<unsigned>(PROP_ENTITY_ID),nDisTicks);
				}
			}
			if(!isValid )
			{
				//CloseActor(false); //正式运营的时候要开启
			}
			m_nRecvHeatbeatTickCount = nCurrent;
			m_dwClientTicksValue = nCurrentTick;
		}
		break;
	case cSetTargetEntity:
		{
			EntityHandle targetHandle;
			pack >> targetHandle; //目标实体
			if(GetEntityFromHandle(targetHandle) ==NULL)
			{
				SetTarget(0);
			}
			else
			{
				SetTarget(targetHandle);
				/*
				CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(targetHandle);
				if (pEntity->isNPC())
				{
					NpcTalk(targetHandle,_T(""));
				}
				*/
			}
		}
		break;
	case cSetMousePos:
		{
			WORD nPosX = -1;
			WORD nPosY = -1;
			pack >> nPosX >> nPosY;
			//SetMousePos(nPosX,nPosY); //设置场景中选择的点
		}
		break;
	case cNpcTalk:
		{
			//与npc对话
			EntityHandle targetHandle;
			short nFuncId;
			pack >> targetHandle;
			pack >> nFuncId;
			//char sBuf[1024];
			//sBuf[0] = 0;
			//pack.readString(sBuf,ArrayCount(sBuf));
			//NpcTalk(targetHandle, sBuf);
			OnNpcTalk(targetHandle,nFuncId,pack);
		}
		break;
	case cAppKeepAlive:
		{
			long long nSendTick = 0;
			pack >> nSendTick;
			TICKCOUNT nCurrTick = _getTickCount();
			OutputMsg(rmNormal, _T("app keepalive ack cur=%lld send=%lld, delay=%lld"), nCurrTick, nSendTick, nCurrTick-nSendTick);
		}
		break;
	case cTestHeartBeat:
		{
			unsigned int nSendTime = 0;
			pack >> nSendTime;
			TICKCOUNT nCurrTick = _getTickCount();

			CActorPacket pack;
			CDataPacket & data = AllocPacket(pack);
			data <<(BYTE)enDefaultEntitySystemID << (BYTE)sTestHeartBeatAck;		
			int nGateDelay	= (int)(m_llRcvPacketTick - m_lGateTickCount);
			int nLogicDelay	= (int)(_getTickCount()- m_llRcvPacketTick);
			data << nSendTime << nGateDelay << nLogicDelay;
			pack.flush();
		}
		break;

	case sSendCultureInfo:
		{
			CActorPacket pack;
			CDataPacket & data = AllocPacket(pack);
			data <<(BYTE)enDefaultEntitySystemID << (BYTE)sSendCultureInfo;		
			SYSTEMTIME sysTime;
			CMiniDateTime openServerTime = GetLogicServer()->GetServerOpenTime(); //获取开服的时间
			openServerTime.decode(sysTime);
			char strOpenTime[32];
			sprintf_s(strOpenTime,sizeof(strOpenTime),"%d-%d-%d",sysTime.wYear,sysTime.wMonth,sysTime.wDay);
			data.writeString(strOpenTime);
			pack.flush();
			
			break;
		}
	case cClientGetLoginDays:
		{
			CActorPacket pack;
			CDataPacket & data = AllocPacket(pack);
			data <<(BYTE)enDefaultEntitySystemID << (BYTE)sSendLoginDays;		
			data << (WORD)GetLoginDaysValue();
			pack.flush();
			break;
		}
	case  cCommonTelePort:		//通用传送 
		{
			unsigned int nType  = 0;
			unsigned int nValue = 0;
			unsigned char nDir  = 0;
			pack >> nType;
			pack >> nValue;
			pack >> nDir;
			CommonTelePort(nType, nValue, nDir);
			break;
		}
	case cReqRankData: //请求排行榜数据
		{
			short nRankId;
			BYTE nCount;
			pack >> nRankId;
			pack >> nCount;

			if (CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankId))
			{
				// if((int nItemCount = pRank->GetList().count())  <= 0) {
				// 	GetGlobalLogicEngine()->GetRankingMgr().LoadCenterRankData(enCSALLLevel);
				// }
				CActorPacket actorPack;
				CDataPacket& netPack = AllocPacket(actorPack);
				netPack << (byte)enDefaultEntitySystemID << (byte)sSendRankData;
				netPack << nRankId;
				pRank->PushToPack(this, nCount, &netPack);
				actorPack.flush();
			}
			break;
		}
	case cClientSendfcmInfo: //发送平台防沉迷信息
		{
			BYTE nFCMInfo; // 0 未填写实名信息， 1 已成年  ，2 未成年
			pack >> nFCMInfo; 
			if(m_miscSystem.ReInitialFCM(nFCMInfo) ==false)
			{
				OutputMsg(rmWaning,_T("misc system ReInitialFCM..."));
			}
			break ;
		}
	case cYYLogin:
		{
    		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
       		CScriptValueList paramList;
			BYTE nUserType;//用户类型，2为大厅用户
			pack >> nUserType;
			paramList << this;
			switch (nUserType)
			{
			case 2:
				globalNpc->GetScript().CallModule("ActivityType10011", "OnYYHallLogin", paramList, paramList, 0);
				paramList.clear();
				paramList << this;
				globalNpc->GetScript().CallModule("ActivityType10012", "OnYYLogin", paramList, paramList, 0);
				paramList.clear();
				paramList << this;
				globalNpc->GetScript().CallModule("ActivityType10013", "OnSupperLogin", paramList, paramList, 0);
				break;
			
			default:
				break;
			}
			break;
		}
	case c360Login:
		{
    		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if ( globalNpc )
			{
				CScriptValueList paramList;
				paramList << this;

				if ( !globalNpc->GetScript().CallModule("ActivityType10022", "On360Login", paramList, paramList, 0) )
				{
					OutputMsg(rmError,"[360] On360Login 错误 玩家=%s !", GetEntityName());
				}
			}
			break;
		}
	case c7GameLogin:
		{
    		static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if ( globalNpc )
			{
				CScriptValueList paramList;
				paramList << this;

				if ( !globalNpc->GetScript().CallModule("ActivityType10023", "On7GameLogin", paramList, paramList, 0) )
				{
					OutputMsg(rmError,"[7游戏] On7GameLogin 错误 玩家=%s !", GetEntityName());
				}
			}
			break;
		}
	case cCallMember:
		{
			BYTE nType = 0;
			pack >> nType;

			int nCallNotEnoughMsgId = 0;

			std::vector<std::tuple<int,int,int>>* pConsum = nullptr;
			switch (nType)
			{
			case 1:
				pConsum = &GetLogicServer()->GetDataProvider()->GetGlobalConfig().vecTeamCallConsum;
				nCallNotEnoughMsgId = tmTeamCallNotEnough;
				if (!GetTeam())
				{
					SendTipmsgFormatWithId(tmCallNoTeam,tstUI);
					return;
				}
				break;
			case 2:
				pConsum = &GetLogicServer()->GetDataProvider()->GetGlobalConfig().vecGuildCallConsum;
				nCallNotEnoughMsgId = tmGuildCallNotEnough;
				if (!GetGuildSystem()->GetGuildPtr())
				{
					SendTipmsgFormatWithId(tmGuildCallLimit,tstUI);
					return;
				}
				{
					int pos = GetGuildSystem()->GetGuildPos();
					if (pos != smGuildLeader && pos != smGuildAssistLeader && pos != smGuildTangzhu)
					{
						SendTipmsgFormatWithId(tmGuildCallLimit,tstUI);
						return;
					}
				}
				break;
			default:
				return;
			}

			auto& vecConsum = *pConsum;

			// 检测场景召唤 tmSceneCallLimit
			if(CScene* pScene = GetScene()) {
				if(SCENECONFIG* pdata = pScene->GetSceneData())
					if (!pdata->boCanCall)
					{
						SendTipmsgFormatWithId(tmSceneCallLimit,tstUI);
						return;
					}
			}

			// 副本不给传送
			if (CFuBen* pFb = GetFuBen())
			{
				if (pFb->GetConfig()->nFbId != 0)
					return;
			}
			

			// 检查消耗
			for(int  i = 0; i < vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = vecConsum[i];
				if( !(CheckConsume(item.type, item.id, item.count)) )
				{
					SendTipmsgFormatWithId(nCallNotEnoughMsgId,tstUI);
					return;
				}
			}
			
			// 消耗
			for(int  i = 0; i < vecConsum.size(); i++)
			{
				GOODS_TAG item;
				std::tie(item.type, item.id, item.count) = vecConsum[i];
				RemoveConsume(item.type, item.id, item.count,-1,-1,-1,0, GameLog::Log_Call, GameLog::LogItemReason[GameLog::Log_Call]);
			}
			
			// 成功召唤提示
			SendTipmsgFormatWithId(tmCallSuccWait,tstUI);

			// 缓存的信息
			unsigned int nActorId = GetProperty<unsigned int>(PROP_ENTITY_ID);
			int nX = GetProperty<int>(PROP_ENTITY_POSX);
			int nY = GetProperty<int>(PROP_ENTITY_POSY);
			int nSceneId = GetScene()->GetSceneId();
			unsigned int nExpire = GetGlobalLogicEngine()->getMiniDateTime() + 60*5;

			// 召唤玩家
			char buff[1024];
			CDataPacket outPack(buff, sizeof(buff));
			outPack << (BYTE)enDefaultEntitySystemID << (BYTE)sMemberCall;
			outPack << (BYTE)nType;
			outPack << nSceneId;
			outPack << nX;
			outPack << nY;
			outPack.writeString(GetEntityName());

			switch (nType)
			{
			case 1:
				{
					if (CTeam * pTeam = GetTeam())
					{
						pTeam->SetCallMemInfo(nActorId, nSceneId, nX, nY, nExpire);
						pTeam->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(), nActorId);
					}
				}
				break;
			case 2:
				{
					if (CGuild * pGuild = GetGuildSystem()->GetGuildPtr())
					{
						pGuild->SetCallMemInfo(nActorId, nSceneId, nX, nY, nExpire);
						pGuild->BroadCastExceptSelf(outPack.getMemoryPtr(), outPack.getPosition(), nActorId);
					}
				}
				break;
			default:
				return;
			}
			break;
		}
	case cAgreeCall:
		{
			BYTE nType = 0;
			pack >> nType;
			unsigned int nActorId,nExpire;
			int nX,nY,nSceneId;

			// 召唤的缓存信息
			switch (nType)
			{
			case 1:
				{
					if (CTeam * pTeam = GetTeam())
					{
						std::tie(nActorId,nSceneId,nX,nY,nExpire)
						= pTeam->GetCallMemInfo();
					}
				}
				break;
			case 2:
				{
					if (CGuild * pGuild = GetGuildSystem()->GetGuildPtr())
					{
						std::tie(nActorId,nSceneId,nX,nY,nExpire)
						= pGuild->GetCallMemInfo();
					}
				}
				break;
			default:
				return;
			}

			// 没有超时，则进行传送
			if (GetGlobalLogicEngine()->getMiniDateTime() < nExpire)
			{
				if (CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0))
				{
					if (CScene *pScene= pFb->GetScene(nSceneId))
					{
						int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;

						pScene->EnterScene(this, nX, nY, enDefaultTelePort, nEffId);
					}
				}
			}
			break;
		}
	case cChangeVoc:
		{
			BYTE nVoc=0,nSex=0;
			pack >> nVoc;
			pack >> nSex;
			UseChangeVocationCard(nVoc,nSex);
		}
		break;
	// case cQueryYYZiZuanResult:
	// 	{
	// 		BYTE nPlatId=0, nViplevel=0, nViptype=0;
	// 		unsigned int nTime;
	// 		pack >>nPlatId >>nViplevel>>nViptype>>nTime;
	// 		char checksign[64] = {0};
	// 		pack.readString(checksign);
	//
	// 		LPCTSTR key =  GetLogicServer()->GetVSPDefine().GetDefinition("YYZiZuanKey");
	// 		char sFormat[128];
	// 		char sSign[64] = {0};
	//
	// 		sprintf(sFormat,"%d%d%d%u%s",nPlatId, nViplevel, nViptype, nTime, key);
	// 		MD5_CTX md5_context; 
	// 		MD5Init(&md5_context);
	// 		MD5Update(&md5_context, (unsigned char*)sFormat, (unsigned int)strlen(sFormat));
	// 		MD5Final((unsigned char*)sSign,&md5_context);
	// 		if (strcmp(sSign, checksign) == 0)
	// 		{
	// 			//共用一个属性,逻辑实现判断下spid
	// 			// viplevel:vip等级
	// 			// viptype(11: 月卡会员奖励 12: 季度卡会员奖励 13: 半年卡会员奖励 14: 年卡会员奖励)
	// 			SetProperty<int>(PROP_ACTOR_YY_ZIZUAN_LEVEL, nViplevel);
	// 			SetProperty<int>(PROP_ACTOR_YY_ZIZUAN_TYPE, nViptype);
	// 			if (nPlatId == 0)//YY平台 spid = "yyg"
	// 			{
	// 				GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this,"YYPurpleDiamondLoginCheck");
	// 			}
	//	
	// 		}
	// 		break;
	// 	}
	// case cQueryYYDttqResult:
	// 	{
	// 		BYTE nLevel;
	// 		unsigned int nTime;
	// 		pack >>nLevel >>nTime;
	// 		char checksign[64] = {0};
	// 		pack.readString(checksign);
	//
	// 		LPCTSTR key =  GetLogicServer()->GetVSPDefine().GetDefinition("YYZiZuanKey");//同紫钻key
	// 		char sFormat[128];
	// 		char sSign[64] = {0};
	//
	// 		sprintf(sFormat,"%d%u%s",nLevel, nTime, key);
	// 		MD5_CTX md5_context; 
	// 		MD5Init(&md5_context);
	// 		MD5Update(&md5_context, (unsigned char*)sFormat, (unsigned int)strlen(sFormat));
	// 		MD5Final((unsigned char*)sSign,&md5_context);
	// 		if (strcmp(sSign, checksign) == 0)
	// 		{
	// 			//注意,有查询结果之后才能执行login等逻辑(不能直接使用HandlerActorEventPlayerLogin处理login逻辑)
	// 			//这里保存结果//nLevel: 0 无级别 ,1 平民级别 ,2 贵族级别 ,3 王室级别
	// 			//非大厅用户登录，客户端也会返回cQueryYYDttqResult结果，来取得大厅等级
	// 			SetProperty<int>(PROP_ACTOR_YY_DT_LEVEL, nLevel); 
	// 			GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this,"SetYYGiftLogin");
	// 		}
	// 		break;
	// 	}
	default:
		break;
	}
}

CNpc* CActor::GetTalkNpc()
{
	EntityHandle hHandle= GetTarget();
	CEntity* pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(hHandle);
	if (pEntity && pEntity->isNPC())
	{
		return (CNpc*)pEntity;
	}
	return NULL;
}


VOID CActor::OnEvent(int nEntityType, CScriptValueList & paramList,CScriptValueList & retParamList)
{
	if(m_IsTestSimulator || m_IsSimulator)
	{
		return;
	}
	DECLARE_TIME_PROF("CActor::OnEvent");
	// CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	// if (pNpc == NULL) return;
	// if( ! pNpc->GetScript().Call("OnActorEvent",paramList,retParamList ) )
	// {
	// 	//脚本错误，将以模态对话框的形式呈现给客户端
	// 	const RefString &s = pNpc->GetScript().getLastErrorDesc();
	// 	SendTipmsg((LPCSTR)s,ttDialog);
	// }
	EventDispatcher::OnActorEvent(this, nEntityType, paramList, retParamList);
}

VOID CActor::OnItemEvent(CScriptValueList & paramList, CScriptValueList & retParamList)
{
	DECLARE_TIME_PROF("CActor::OnItemEvent");
	CNpc * pNpc = GetGlobalLogicEngine()->GetItemNpc();
	if (pNpc != NULL)
	{
		if(!pNpc->GetScript().Call("OnItemEvent", paramList, retParamList))
		{
			const RefString & s = pNpc->GetScript().getLastErrorDesc();
			SendTipmsg((LPCSTR)s, ttDialog);
		}
	}else{
		OutputMsg(rmError,_T("GetGlobalLogicEngine()->GetItemNpc() got null"));
	}
}

// VOID CActor::NpcTalk(EntityHandle& NpcHandle,LPCTSTR sFunctionName)
// {
// 	DECLARE_FUN_TIME_PROF()
	
// 	//取得npc的指针
// 	CEntity* pEntity =NULL;
// 	bool boGlobal = false;
// 	if (NpcHandle == 0)
// 	{
// 		pEntity = GetGlobalLogicEngine()->GetGlobalNpc();
// 		boGlobal = true;
// 	}
// 	else
// 		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(NpcHandle);
// 	if (pEntity)
// 	{
// 		SetTarget(pEntity->GetHandle());
// 		//只有npc才可以对话
// 		if (pEntity->isNPC() )
// 		{
// 			if (!boGlobal)
// 			{
// 				//检验数据和坐标，必须在规定范围以内
// 				if (pEntity->GetFuBen() != GetFuBen() || pEntity->GetScene() != GetScene())
// 				{
// 					return;
// 				}
// 				int actX,actY,npcX,npcY;
// 				GetPosition(actX,actY);
// 				pEntity->GetPosition(npcX,npcY);
// 				if (npcX > (actX+MOVE_GRID_COL_RADIO) || npcX < (actX-MOVE_GRID_COL_RADIO) 
// 					|| npcY > (actY+MOVE_GRID_ROW_RADIO) || npcY < (actY-MOVE_GRID_ROW_RADIO))
// 				{
// 					//OutputMsg(rmTip,_T("incorrect range in NpcTalk"));
// 					return;
// 				}
// 			}

// 			//调用npc或者monster的脚本,如果有的话
// 			if (sFunctionName[0] == 0)
// 			{
// 				((CNpc*)pEntity)->Click(this);
// 			}
// 			else
// 			{
// 				((CNpc*)pEntity)->Talk(this,sFunctionName);
// 			}
// 		}
// 		else	
// 		{
// 			//不正常的网络包
// 			OutputMsg(rmTip,_T("%u,%u,%s,incorrect network data in NpcTalk"), GetAccountID(),GetId(),sFunctionName);
// 		}
// 	}

// }

CTeam * CActor::GetTeam()
{
	unsigned int nTeamID = GetProperty<unsigned int>(PROP_ACTOR_TEAM_ID);
	if(nTeamID == 0) return NULL;
	return GetGlobalLogicEngine()->GetTeamMgr().GetTeam(nTeamID);
}

/*
*飞鞋用，回到特定场景特定位置
*/
bool CActor::FlyShoseToStaticScene(INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight, INT_PTR nHeight, int nType , int nValue, int nParam)
{

	//玩家死亡了不能使用飞鞋传送
	if(IsDeath()) return false ;

	//如果原来在副本中，则不做处理
	CFuBen* pDyFb = GetFuBen();
	CScene* pScene = GetScene();

	if (!(pDyFb && pScene))
	{
		return false;
	}

	//回到普通场景
	CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	return pFb->Enter(this,nScenceID,nPosX,nPosY,nWeight,nHeight,nType,nValue,-1,pScene,pDyFb);
/*	
	//回到之前保存的位置
	if (GetLogicServer()->IsCommonServer())//跨服退出回到跨服场景
	{
		int nCommonSceneId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCommonSrvEntrSceneId;
		CScene* pScene = pFb->GetScene(nCommonSceneId);
		if (pScene)
		{
			//int nPosX, nPoxY;
			pScene->GetAreaCenter(nPosX, nPosY);
			pScene->GetDefaultEnterPos(nPosX, nPosY);
			
			pFb->Enter(this,nCommonSceneId,nPosX,nPosY);
		}
	}
	else
	{
		if( nSceneId >= 0 && nPosX >= 0 && nPosY >= 0 )
		{
			pFb->Enter(this,nSceneId,nPosX,nPosY);
		}
		else
		{
			pFb->Enter(this,m_nEnterFbScenceID,m_nEnterFbPosx,m_nEnterFbPosy);
		}
	}
*/
}

/*
退出副本，
返回到进入副本之前的位置或者指定位置
*/
void CActor::ReturnToStaticScene(int nSceneId, int nPosX, int nPosY)
{
	//如果原来就是普通副本中，则不做处理
	CFuBen* pDyFb = GetFuBen();
	if (pDyFb && !pDyFb->IsFb())
	{
		return;
	}
	CScene* pScene = GetScene();
	//正在副本中或者pFb为NULL
	// if (pScene != NULL)
	// {
	// 	pScene->ExitScene(this);	
	// }
	

	//回到普通场景
	CFuBen* pFb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);

#ifdef _DEBUG
	//CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
	//const char* callstack = pNpc->GetScript().GetCallStack();
#endif

	//回到之前保存的位置
	
	// if (GetLogicServer()->IsCommonServer())//跨服退出回到跨服场景
	// {
	// 	int nCommonSceneId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nCommonSrvEntrSceneId;
	// 	CScene* pNewScene = pFb->GetScene(nCommonSceneId);
	// 	if (pNewScene)
	// 	{
	// 		//int nPosX, nPoxY;
	// 		pNewScene->GetAreaCenter(nPosX, nPosY);
	// 		pNewScene->GetDefaultEnterPos(nPosX, nPosY);
	// 		pFb->Enter(this,nCommonSceneId,nPosX,nPosY,0,0,0,0,-1,pScene,pDyFb);
	// 	}
	// }
	// else
	{
		if( nSceneId >= 0 && nPosX >= 0 && nPosY >= 0 )
		{
			pFb->Enter(this,nSceneId,nPosX,nPosY,0,0,0,0,-1,pScene,pDyFb);
		}
		else
		{
			pFb->Enter(this,m_nEnterFbScenceID,m_nEnterFbPosx,m_nEnterFbPosy,0,0,0,0,-1,pScene,pDyFb);
		}
	}

	//离开副本把狗要招回去
	m_petSystem.RecallPet();
}

bool CActor::AddBuyItemDialog(EntityHandle hNpc, WORD nItemId, byte nCount, LPCSTR sNotice, LPCSTR sDesc, LPCSTR sCallFunc)
{
	CActorPacket pack;
	CDataPacket& netPack = AllocPacket(pack);
	netPack << (byte)enDefaultEntitySystemID << (byte)sNoticeBuyItems;
	netPack << hNpc << nItemId << nCount;
	netPack.writeString(sNotice);
	netPack.writeString(sDesc);
	netPack.writeString(sCallFunc);
	pack.flush();
	 
	return 0;
}

int CActor::AddAndSendMessageBox( EntityHandle hNpc,UINT nActorId,const char* sTitle,char sFnName[][MAX_MSG_COUNT],INT_PTR nButtonCount,
	unsigned int nTimeOut,int msgType,const char* sTip,WORD wIcon,int nTimeOutBtn, int msgId, int showId)
{
	if (sTitle == NULL || sFnName == NULL || nButtonCount == 0 || nButtonCount > MAX_BUTTON_COUNT)
	{
		OutputMsg(rmTip,_T("AddAndSendMessageBox Param Error!"));
		return 1;//参数错误
	}
	if (hNpc == 0)
	{
		hNpc = GetGlobalLogicEngine()->GetGlobalNpc()->GetHandle();
	}
	if (nActorId == 0)
	{
		nActorId = GetProperty<UINT>(PROP_ENTITY_ID);
	}
	//加到队列中
	MessageBoxItem* pItem = NULL;

	
	MessageBoxItem item;		
	MessageBoxList->add(item);
	MessageBoxItem& item1 = (*MessageBoxList)[MessageBoxList->count()-1];
	pItem = &item1;
	
	pItem->msgid = CActor::MsgBox_Id++;
	pItem->hNpc = hNpc;
	pItem->nActorid = nActorId;
	pItem->bButtonCount = (BYTE)nButtonCount;
	for (int j = 0; j < nButtonCount; j++)
	{
		if (sFnName[j] == NULL) return 1;
		//sFnName包含了按钮要显示的文字，以及点击后要执行的函数名，中间用 “/”隔开，比如"确定/commonAcceptMissions,1"
		//其中“确定”是客户端要显示的按钮的文字，commonAcceptMissions,1是要执行的脚本函数和参数
		char* str = sFnName[j];
		while (*str && *str != '/')
		{
			str++;
		}
		if (*str++ == 0) return 1; //错误的参数,没找到“/”
		if (*str == 0) return 1;	//“/”后面没有其他字符串了
		_asncpytA(pItem->sFnName[j],str);
	}
	//发送消息给客户端
	//根据id找到角色指针
	CActor* pActor = NULL;
	if (nActorId != GetProperty<UINT>(PROP_ENTITY_ID))
	{
		pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorId);
	}
	else
	{
		pActor = this;
	}
	if (!pActor) return 2;
	CActorPacket pack;
	CDataPacket & data = pActor->AllocPacket(pack);
	data << (BYTE) enDefaultEntitySystemID << (BYTE) sMessageBox ;
	data << hNpc;
	data.writeString(sTitle);
	data << (BYTE)nButtonCount;
	for (int j = 0; j < nButtonCount; j++)
	{
		char* str = sFnName[j];
		while (*str && *str != '/')//这里不用判断字符串是否合法了，前面已经判断过
		{
			str++;
		}

		data.writeString(sFnName[j],str-sFnName[j]);	
	}
	data << (UINT)nTimeOut;
	data << (int)(pItem->msgid);
	data << (BYTE)msgType;
	data.writeString(sTip?sTip:"");
	data << (WORD)wIcon;
	data << (BYTE)nTimeOutBtn;
	data << (int)msgId;
	data << (WORD)showId;
	pack.flush();
	return 0;
}


bool CActor::AddTxtMsg(unsigned int nActorId,const char* sMsg,unsigned int nSrcActorId)
{
	if (!sMsg) return false;
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	packet.writeString(sMsg);
	packet.writeString("");//无需标题文字
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsg(nActorId,CMsgSystem::mtTxtMsg,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddPaTaAwardMsgByName( const char * pActorName, const char * sMsg,const char *sTitle, byte bType,int nFubenId, int nDeadline )
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	packet.writeString(sMsg);
	packet.writeString(sTitle);
	packet << bType << nFubenId << nDeadline;
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName(pActorName,CMsgSystem::mtPaTaMasterAward,MsgBuff,packet.getPosition(),0);
}

bool CActor::AddCommonMsgByActorName(int nMsgId, const char *pActorName, const char *sMsg,unsigned int nSrcActorId, int nParam, int nParam2,const char * sParam)
{
	if (!pActorName || !sMsg) return false;
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	packet.writeString(sMsg);
	packet.writeString("");//无需标题文字
	packet << nParam << nParam2;
	if (sParam != NULL)
	{
		packet.writeString(sParam);
	}	
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName((char *)pActorName,nMsgId,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddFestivalMsgByActorName(int nMsgId, LPCSTR pActorName, LPCSTR sMsg, unsigned int nSrcId,LPCSTR sParam1,LPCSTR sParam2,int nParam1, int nParam2)
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket msgPacket(MsgBuff,sizeof(MsgBuff));
	msgPacket.writeString(sMsg);
	msgPacket.writeString("");
	msgPacket << (byte)nParam1 << (WORD)nParam2;
	msgPacket.writeString(sParam1);
	msgPacket.writeString(sParam2);
	Assert(msgPacket.getPosition() <= MAX_MSG_COUNT);
	CMsgSystem::AddOfflineMsgByName(pActorName, nMsgId, MsgBuff, msgPacket.getPosition(), nSrcId);
	return true;
}

bool CActor::AddTxtMsgByActorName(const char *pActorName, const char *sMsg,unsigned int nSrcActorId)
{
	if (!pActorName || !sMsg) return false;
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	packet.writeString(sMsg);
	packet.writeString("");//无需标题文字
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName((char *)pActorName,CMsgSystem::mtTxtMsg,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddGmTxtMsgByActorName(const char *pActorName, const char *sMsg,unsigned int nSrcActorId)
{
	if (!pActorName || !sMsg) return false;
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	packet.writeString(sMsg);
	packet.writeString("");//无需标题文字
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName((char *)pActorName,CMsgSystem::mtGmTxtMsg,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddChangeMoneyMsg( unsigned int nActorId,unsigned int nSrcActorId,LPCSTR sTitle,LPCSTR sBtnTxt,BYTE nMoneyType,
	int nMoneyValue, int nWay ,LPCSTR sComment ,bool needLog )
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	sTitle?packet.writeString(sTitle):packet.writeString("");
	sBtnTxt?packet.writeString(sBtnTxt):packet.writeString("");
	packet << (BYTE)nMoneyType << (int)nMoneyValue << (int)nWay << (BYTE)needLog;
	if (sComment)
	{
		packet.writeString(sComment);
	}
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsg(nActorId,CMsgSystem::mtChangeMoney,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddChangeMoneyMsgByName( LPCSTR sActorName,unsigned int nSrcActorId,LPCSTR sTitle,LPCSTR sBtnTxt,
	BYTE nMoneyType,int nMoneyValue, int nWay /*=0*/,LPCSTR sComment /*=""*/,bool needLog/*=true */ )
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	sTitle?packet.writeString(sTitle):packet.writeString("");
	sBtnTxt?packet.writeString(sBtnTxt):packet.writeString("");
	packet << (BYTE)nMoneyType << (int)nMoneyValue << (int)nWay << (BYTE)needLog;
	sComment?packet.writeString(sComment):packet.writeString("");
	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName(sActorName,CMsgSystem::mtChangeMoney,MsgBuff,packet.getPosition(),nSrcActorId);
}


bool CActor::AddItemMsgByName( LPCSTR sActorName, unsigned int nSrcActorId, const CUserItemContainer::ItemOPParam& param, LPCSTR sTitle,
	LPCSTR sBtnTxt,INT_PTR nWay, LPCSTR sComment /*=""*/,bool needLog/*=true*/ )
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	sTitle?packet.writeString(sTitle):packet.writeString("");
	sBtnTxt?packet.writeString(sBtnTxt):packet.writeString("");
	packet << param << (int)nWay << (BYTE)needLog;
	sComment?packet.writeString(sComment):packet.writeString("");

	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsgByName(sActorName,CMsgSystem::mtAddIndexItem,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::AddItemMsg( unsigned int nActorId, unsigned int nSrcActorId, const CUserItemContainer::ItemOPParam& param, LPCSTR sTitle,
	LPCSTR sBtnTxt,INT_PTR nWay, LPCSTR sComment /*=""*/,bool needLog/*=true*/, INT_PTR nMsgType, INT_PTR nParam )
{
	char MsgBuff[MAX_MSG_COUNT];
	CDataPacket packet(MsgBuff,sizeof(MsgBuff));
	sTitle?packet.writeString(sTitle):packet.writeString("");
	sBtnTxt?packet.writeString(sBtnTxt):packet.writeString("");
	packet << param << (int)nWay << (BYTE)needLog;
	sComment?packet.writeString(sComment):packet.writeString("");
	packet << (int)nParam;

	Assert(packet.getPosition() <= MAX_MSG_COUNT);
	return CMsgSystem::AddOfflineMsg(nActorId,nMsgType,MsgBuff,packet.getPosition(),nSrcActorId);
}

bool CActor::ProcessOfflineMsg( INT_PTR nMsgType,char* pMsgBuf,SIZE_T size, INT64 nMsgId )
{
	if (!pMsgBuf || size <= 0) return true;
	CDataPacketReader reader(pMsgBuf,size);
	bool ret = false;
	switch(nMsgType)
	{
	case CMsgSystem::mtTxtMsg:
	case CMsgSystem::mtGmTxtMsg:
		{
			// deled by caphone 2011.4.21 目前这两个文本类型消息都不会有此ntype数据，这里能序列话出来也是因为buff过长（MsgData.Msg），
			// 而且此Buff段并未初始化，这里的ntype也是未知值
			//BYTE ntype = 0;
			//reader >> ntype;
			//SendTipmsg(reader.getOffsetPtr(),ntype);//存储的内容就已经是以0结尾的字符串
			ret = true;
			break;
		}
	case CMsgSystem::mtChangeMoney:
		{
			BYTE nMentyType=0,needLog=0;
			int nMoneyValue=0,nWay=0;
			reader >> nMentyType;
			reader >>nMoneyValue;
			reader >>nWay;
			reader >>needLog;
			//最后是备注的内容,getOffsetPtr()
			ret = ChangeMoney(nMentyType,nMoneyValue,nWay,0,reader.getOffsetPtr(),needLog > 0 ? true : false);
			break;
		}
	case CMsgSystem::mtFestivalWithDraw:
	case CMsgSystem::mtAddIndexItem:
	case CMsgSystem::mtAuctionItem:
	case CMsgSystem::mtSurpriseRet:
		{
			CUserItemContainer::ItemOPParam param;
			int nWay = 0, nParam = 0;
			LPCSTR sComment = NULL;
			BYTE bNeedLog = 0;
			reader >> param;
			reader >> nWay;
			reader >> bNeedLog;
			reader >> sComment;
			reader >> nParam;
			if (GetBagSystem().CanAddItem(param))
			{
				if (nParam && nParam > 0 && nMsgType == CMsgSystem::mtAuctionItem)	//竞拍
				{
					//param.nBestSmith = RandAttrSelector::InitSmithById(param.wItemId, RandAttrSelector::bawAuction);
					UpdateConsumeYuanbao(nParam,sComment);	//计入消费排行
					const CStdItem *pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(param.wItemId);
					if (pStdItem)	//记录
					{
						int nLevelAndCircle = MAKELONG(GetProperty<int>(PROP_CREATURE_LEVEL),GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));
						GetLogicServer()->GetLogClient()->SendConsumeLog((WORD)nWay, 
							(unsigned int)GetAccountID(), GetAccount(), GetEntityName(), (BYTE)mtYuanbao,
							(int)GetMoneyCount(mtYuanbao), (int)(-nParam), pStdItem->m_sName, nLevelAndCircle,getOldSrvId());
					}
					
				}
				GetBagSystem().AddItem(param,sComment,nWay);
				ret = true;
			}
			else
			{
				SendOldTipmsgWithId(tpBagFullNotGetSysItem, ttFlyTip);
			}
			break;
		}
	case CMsgSystem::mtSeverMaster:				//已告诉师傅，有徒弟与你解除关系了
	case CMsgSystem::mtExpelPupil:				//已告诉徒弟，师傅把你逐出师门了
		{
			ret = true;
			break;
		}
	case CMsgSystem::mtDeleteGuild:
	case CMsgSystem::mtReturnSuggestMsg:
		{
			ret = true;
			break;
		}
	case CMsgSystem::mtGuildDepotMsg:
		{
			break;
		}
	case CMsgSystem::mtGiveStoreItem:
		{
			ACTORNAME sGiveName;
			WORD nItemId, nItemCount;
			byte btQuality, btStrong, btBind;
			reader >> nItemId >> nItemCount >> btQuality >> btStrong >> btBind;
			reader.readString(sGiveName, ArrayCount(sGiveName));
			CUserItemContainer::ItemOPParam iParam;
			iParam.wItemId = nItemId;
			iParam.wCount = (WORD)nItemCount;
			iParam.btQuality = btQuality;
			iParam.btStrong =  btStrong;
			iParam.btBindFlag = btBind;
			if (GetBagSystem().CanAddItem(iParam))
			{
				//iParam.nBestSmith = RandAttrSelector::InitSmithById(nItemId, RandAttrSelector::bawDefault);
				GetBagSystem().AddItem(iParam, "!STORE OfflineMsg", GameLog::clBuyStoreItem);
				const char *pItemName = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetNameById(nItemId);
				SendOldTipmsgFormatWithId(tpGetGiveStoreItem, ttFlyTip, sGiveName, nItemCount, pItemName);
				ret = true;
			}
			else
			{
				SendOldTipmsgWithId(tpGiveItemBagFull, ttFlyTip);
			}
			break;
		}
	case CMsgSystem::mtPaTaMasterAward:
		{
			/*
			byte bType;
			int nFubenId = 0;
			int nDeadline = 0;
			reader >> bType >> nFubenId >> nDeadline;

			CScriptValueList paramList, retList;
			InitEventScriptVaueList(paramList,aeGetPaTaMasterAward);
			paramList << bType << nFubenId;
			OnEvent(paramList,retList);
			if(retList.count() > 0)
			{
				ret = (bool)retList[retList.count() - 1];  //返回最后一个参数表示结果
			}
			*/
			break;
		}

	case CMsgSystem::mtCorpsBattleAward:		//战队竞技给奖励
		{
			break;
		}

	case CMsgSystem::mtCombatRankAward:						//[废弃]
		{
			break;
		}

	case CMsgSystem::mtWholeActivityAward:				//全民奖励//2014-03-04改版弃用
		{
			//int nDays = 0;
			//int nValue = 0;
			//
			//CScriptValueList paramList, retList;
			//InitEventScriptVaueList(paramList,aeWholeActivityAwards);
			//reader >> nDays;
			//reader >> nValue;

			//paramList << nDays << nValue;
			//OnEvent(paramList,retList);
			//if(retList.count() > 0)
			//{
			//	ret = (bool)retList[retList.count() - 1];  //返回最后一个参数表示结果
			//	if (ret)
			//	{
			//		SetWholeAwards(nDays);
			//	}
			//}
			ret = true;
			break;
		}
	case CMsgSystem::mtGiveFirstSbkGuildAward:
		{
			break;
		}

	case CMsgSystem::mtDefendSbkThreeTimes:
		{
			break;
		}

	case CMsgSystem::mtDefendSbkSecAwards:
		{
			break;
		}

	case CMsgSystem::mtGiveFirstCombineSbkAward:
		{
			break;
		}

	case CMsgSystem::mtGiveFirstCombineSecSbkAward:
		{
			break;
		}

	case CMsgSystem::mtGiveCombineDefendSbkAward:
		{
			break;
		}

	case CMsgSystem::mtGiveCombineDefendSbkSecAward:
		{
			break;
		}

	case CMsgSystem::mtGiveCombineActivityAward:
		{
			int nId = 0;
			reader >> nId;
			int nIndex = 0;
			reader >> nIndex;
			int nBag = 0;
			reader >> nBag;

			ret = true;
			break;
		}
	case CMsgSystem::mtHundredYestodayConsumeRank:
		{
			//byte bType = 0, nIndex = 0;
			//reader >> bType >> nIndex;

			//CScriptValueList paramList, retList;
			//InitEventScriptVaueList(paramList,aeHundredServer);
			//paramList << bType << nIndex;
			//OnEvent(paramList,retList);
			//if(retList.count() > 0)
			//{
			//	ret = (bool)retList[retList.count() - 1];  //返回最后一个参数表示结果
			//}//不要了
			ret = true;
			break;
		}
	case CMsgSystem::mtLuckAward:
	case CMsgSystem::mtFireTop1:
		{
			/*
			int nParam = 0;
			reader >> nParam;
			CScriptValueList paramList, retList;
			InitEventScriptVaueList(paramList, aeFastivalFire);
			paramList << nParam;
			OnEvent(paramList,retList);
			if(retList.count() > 0)
			{
				ret = (bool)retList[retList.count() - 1];  //返回最后一个参数表示结果
			}
			*/
			break;
		}
	case CMsgSystem::mtOldPlayerBack:
		{
			break;
		}
	case CMsgSystem::mtSpecialConsumeYb:
		{
			BYTE tipMsgId = 0;
			int consumeYb = 0;
			reader >> tipMsgId;
			reader >> consumeYb;
			LPCTSTR sComment = GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(tipMsgId);
			if (sComment)
			{
				UpdateConsumeYuanbao(consumeYb,sComment);
			}
			ret = true;
			break;
		}
	case CMsgSystem::mtCombatLog:
		{
			int nResult = 0;
			int nId = 0;
			int nRank = 0;
			char strName[32];
			reader >> nResult >> nId >> nRank;
			reader.readString(strName, ArrayCount(strName));
			m_CombatSystem.OfflineCombatOp(nResult, nId, strName, nRank);

			ret = true;
			break;
		}
	}
	return ret;
}

void CActor::ReliveTimeOut()
{
	if (m_nDestoryTime != 0 && m_nDestoryTime < GetGlobalLogicEngine()->getMiniDateTime())
	{
		//已超时,脚本处理
		CScriptValueList paramList;
		//InitEventScriptVaueList(paramList,aeReliveTimeOut); //
		paramList << this;
		OnEvent(aeReliveTimeOut,paramList,paramList);
	}
		
}

void CActor::SetRelivePoint( int nPosX,int nPosY,unsigned int nSceneId,bool boFb/*=false*/ )
{
	bool bInRawServer = IsInRawServer();
	if (boFb)
	{
		if (bInRawServer)
		{
			m_nFubenRelivePosX = nPosX;
			m_nFubenRelivePosY = nPosY;
			m_nFubenReliveSceneId = nSceneId;
		}
		else
		{			
			m_commsrvFBRelivePointPos.SetData(nSceneId, nPosX, nPosY);
		}
		//OutputMsg(rmTip, _T("设置角色服务器副本复活点位置(%d, %d_%d)"), nSceneId, nPosX, nPosY);
	}
	else
	{
		if (bInRawServer)
		{
			m_nRelivePointPosX = nPosX;
			m_nRelivePointPosY = nPosY;
			m_nRelivePointSceneId = nSceneId;
		}
		else
		{			
			m_commsrvRelivePointPos.SetData(nSceneId, nPosX, nPosY);
		}
		//OutputMsg(rmTip, _T("设置角色服务器普通场景复活点位置(%d, %d_%d)"), nSceneId, nPosX, nPosY);
	}
}

void CActor::SetCityPoint( int nPosX,int nPosY,unsigned int nSceneId )
{
	if (IsInRawServer())
	{
		m_nCityPointPosX = nPosX;
		m_nCityPointPosY = nPosY;
		m_nCityPointSceneId = nSceneId;
	}
	else
	{		
		m_commsrvCityPointPos.SetData(nSceneId, nPosX, nPosY);
	}

	//OutputMsg(rmTip, _T("设置角色跨服服务器回城点位置(%d, %d_%d)"), nSceneId, nPosX, nPosY);
}

void CActor::ReturnCity()
{
	//返回普通场景的城市
	
	CFuBen* pfb = GetGlobalLogicEngine()->GetFuBenMgr()->GetFbStaticDataPtr(0);
	if (pfb)
	{
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(this,pfb,m_nCityPointSceneId,m_nCityPointPosX,m_nCityPointPosY,enDefaultTelePort,nEffId);
		//离开副本把狗要招回去
		m_petSystem.RecallPet();
	}
	//ReturnToStaticScene(m_nCityPointSceneId, m_nCityPointPosX, m_nCityPointPosY);
}

void CActor::Relive(bool isReturn = false)
{
	bool bInRawServer = IsInRawServer();
	CFuBen* pfb = GetFuBen();
	if (pfb)
	{
		INT_PTR nSceneId = 0, nPosX = 0, nPosY = 0;
		if (!isReturn && pfb->IsFb())
		{
			if (bInRawServer)
			{
				nSceneId = m_nFubenReliveSceneId;
				nPosX = m_nFubenRelivePosX;
				nPosY = m_nFubenRelivePosY;
			}
			else
			{
				nSceneId = m_commsrvFBRelivePointPos.nSceneId;
				nPosX = m_commsrvFBRelivePointPos.nPosX;
				nPosY = m_commsrvFBRelivePointPos.nPosY;
			}
		}
		else
		{
			if (bInRawServer)
			{
				nSceneId = m_nRelivePointSceneId;
				nPosX = m_nRelivePointPosX;
				nPosY = m_nRelivePointPosY;
				if (nSceneId == 0)
				{
					GLOBALCONFIG &  data=GetLogicServer()->GetDataProvider()->GetGlobalConfig();
					nSceneId = data.nInitSceneId;
					nPosX = data.nInitScenePosX;
					nPosY = data.nInitScenePosY;
				}
			}
			else
			{
				nSceneId = m_commsrvRelivePointPos.nSceneId;
				nPosX = m_commsrvRelivePointPos.nPosX;
				nPosY = m_commsrvRelivePointPos.nPosY;
			}
		}

		CScene *pScene = pfb->GetScene(nSceneId);
		SCENEAREA *pArea = NULL;
		if(pScene)
		{
			pArea = pScene->GetAreaAttri(nPosX, nPosY);
		}
		
		if (pArea && pScene && pScene->HasMapAttribute(nPosX, nPosY, aaSaftRelive, pArea))
		{
			int nRandomX, nRandomY = 0;
			AreaAttri * pAreaAttr = &pArea->attri[aaSaftRelive];
			if ( (pAreaAttr == NULL || pAreaAttr->nCount <= 0) && pArea && pArea->nPointCount >= 3 
				&& pScene->GetRandomPoint(this, pArea->lpPoints[0].x, pArea->lpPoints[0].y, pArea->lpPoints[2].x, pArea->lpPoints[2].y, nRandomX, nRandomY))
			{
				nPosX = nRandomX;
				nPosY = nRandomY;
			}
		}

		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(this,pfb,nSceneId,nPosX,nPosY,enDefaultTelePort,nEffId);
	}

	OnRelive();
}


void CActor::OnRelive()
{
	////群发通知队友
	//CTeam* pTeam = GetTeam();
	//if (pTeam)
	//{
	//	char data[16];
	//	unsigned int nActorId = GetProperty<unsigned int>(PROP_ENTITY_ID);
	//	CDataPacket pack(data,sizeof(data));			
	//	pack << (BYTE)enTeamSystemID << (BYTE)sMemberDieRelive << (unsigned int)nActorId << (BYTE)1;//0:死亡，1：复活
	//	pTeam->BroadCast(data,pack.getPosition(),nActorId);
	//}
}
INT_PTR CActor::GetAwardTypeByMoneyType(INT_PTR nMoneyType)
{
	INT_PTR awardType= -1;
	switch(nMoneyType)
	{
		case mtBindCoin:
			{
				awardType = qatBindMoney;
				break;
			}
		case mtCoin:
			{
				awardType = qatMoney;
				break;
			}
		case mtBindYuanbao:
			{
				awardType = qatBindYb;
				break;
			}
		case mtYuanbao:
			{
				awardType = qatYuanbao;
				break;
			}
		case mtCircleSoul:
		    {
			    awardType = qatCircleSoul;
			    break;		 
		    }
		case mtFlyShoes://飞鞋
			{
				awardType = qatFlyShoes;
			}
			break;
		default: 
			{
				break;
			}
	}
	return awardType;
}


INT_PTR CActor::GetMoneyTypeByAward(INT_PTR nAwardType)
{
	INT_PTR moneyType = -1;
	switch(nAwardType)
	{
	case qatBindMoney:
		{
			moneyType = mtBindCoin; 
			break;
		}
	case qatMoney:
		{
			moneyType = mtCoin; 
			break;
		}
	case qatBindYb:
		{
			moneyType = mtBindYuanbao; 
			break;
		}
	case qatYuanbao:
		{
			moneyType = mtYuanbao; 
			break;
		}
	case qatCircleSoul:
		{
			moneyType = mtCircleSoul;
			break;		 
		}
	case qatFlyShoes://飞鞋
		{
			moneyType = mtFlyShoes;
		}
		break;
	default: 
		{
			break;
		}
	}
	return moneyType;
}

const char* CActor::GetAwardTypeDesc(INT_PTR nAwardType,INT_PTR nAwardId ,bool useChatLink,CUserItem *pUserItem )
{
	int nTipmsgId =0;
	const CStdItem * pItem;

	switch(nAwardType)
	{
		case qatEquipment:
			if(useChatLink)
			{
				return CUserBag::GetItemLink((WORD)nAwardId,pUserItem);
			}
			else
			{
				pItem= GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nAwardId);
				if(pItem)
				{
					return (char *)pItem->m_sName;
				}
				else
				{
					return "";
				}
			}
			//经验
		case qatExp:
			{
				nTipmsgId =tpAwardTypeExp;
				break;
			}
			//帮派贡献度
		case qatGuildContribution:
			{
				nTipmsgId = tpAwardTypeGuildCont;
				break;
			}
			//绑定银两
		case qatBindMoney:
			{
				nTipmsgId =tpMoneyTypeBindCoin;
				break;
			}
			//银两
		case qatMoney:
			{
				nTipmsgId =tpMoneyTypeCoin;
				break;
			}
			//绑定元宝
		case qatBindYb:
			{
				nTipmsgId =tpMoneyTyeBindYuanbao;
				break;
			}
			// 元宝
		case qatYuanbao:
			{
				nTipmsgId =tpMoneyTypeYuanbao;
				break;
			}
		case qatAchieveEvent: //成就事件
			{
				nTipmsgId =tpAwardTypeAchievePoint;
				break;
			}
		case qatActivity:  //奖励活跃度
			{
				nTipmsgId =tpAwardTypeActivity;
				break;
			}

		case qatGuildFr:		//奖励繁荣度
			{
				nTipmsgId =tpAwardTypeGuildFr;
				break;
			}
		case qatAddExp:		//根据经验配置表加经验
			{
				nTipmsgId =tpAwardTypeExp;
				break;
			}
		case qatCircleSoul:
			{
				nTipmsgId =tpAwardTypeCircleSoul;
				break;
			}
		case qatPersonBossJifen:
			{
				nTipmsgId = tpPersonBossJifen;
				break;
			}
		case qatDragonSoulValue:
			{
				//nTipmsgId = tpAwardTypeDragonSoulValue;
				break;
			}
		case qatIntellectBallValue:
			{
				nTipmsgId = tpAwardTypeIntellectBallValue;
				break;
			}
			
		default: 
			{
				break;//自定义奖励，不需要这里处理
			}
	}
	if(nTipmsgId )
	{
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipmsgId);
	}
	return NULL;

}

bool CActor::CanGiveAward(INT_PTR nType, INT_PTR nId, INT_PTR nValue,
	INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, bool boNotice)
{
	bool boSuccess = true;
	switch(nType)
	{
	case qatBindYb:
		{
			int nNowBindYB = GetMoneyCount(mtBindYuanbao);
			int nMax = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBindYuanbaoLimit;
			if (nNowBindYB + abs(nValue) > nMax)
			{
				boSuccess = false;
				if (boNotice)
				{
					SendOldTipmsgFormatWithId(tpBindYuanbaoFull, ttFlyTip);
				}
			}
			break;
		}
	case qatUpgrade:
		{
			unsigned int nNewLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL) + (int)nValue;
			unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));
			if (nNewLevel > nMaxPlayerLevel)
			{
				boSuccess = false;
				if (boNotice)
				{
					SendOldTipmsgFormatWithId(tpTheMaxLevel, ttFlyTip);
				}
			} 
			break;
		}
	case qatPetUpgrade:
		{
			boSuccess = false;
			INT_PTR nPetCount = GetPetSystem().GetPetCount();
			for (INT_PTR i = 0; i < nPetCount; i++)
			{
				CPet* pPet = (CPet *)GetEntityFromHandle(GetPetSystem().GetPetData(i)->handle);
				if (pPet != NULL)
				{
					int nLevel = pPet->GetProperty<int>(PROP_CREATURE_LEVEL);
					if (nLevel < nValue)
					{
						boSuccess = true;
						break;
					}
				}
			}
			if (!boSuccess && boNotice)
			{				
				SendOldTipmsgFormatWithId(tpPetUpgradeFail, ttFlyTip, nValue);
			}
			break;
		}
		
	case qatGuildCoin:	//行会资金
		{
			if(GetGuildSystem()->GetGuildPtr() == NULL)
			{
				SendOldTipmsgFormatWithId(tpNoGuildCanNotUsed, ttFlyTip);
				boSuccess = false;
			}
			break;
		}

	case qatGuildContribution:	//行会贡献
		{
			if(GetGuildSystem()->GetGuildPtr() == NULL)
			{
				SendOldTipmsgFormatWithId(tpNoGuildCanNotUsed, ttFlyTip);
				boSuccess = false;
			}
			break;
		}
	}
	return boSuccess;
}

bool CActor::GiveAward(INT_PTR nType,INT_PTR nId, INT_PTR nValue,
	INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, const INT_PTR param,INT_PTR nLogId,LPCTSTR pStr, int nQualityDataIndex,const CUserItemContainer::ItemOPParam* itemOPParam)
{
	bool isSucceed =true; //一般都是成功的
	int nOldValue;
	CScene *pScene ; //当前的场景
	int x,y;
	switch(nType)
	{
		//装备
	case qatEquipment:
		{
			const CUserItemContainer::ItemOPParam *pItemOp;
			pItemOp = (CUserItemContainer::ItemOPParam *)itemOPParam;
			if (pItemOp == NULL)
			{
				if (nId <= 0) return false;
				CUserItemContainer::ItemOPParam ItemOp;
				ItemOp.wItemId = (WORD)nId;
				ItemOp.btQuality = (BYTE)nQuality;
				ItemOp.wStar = (WORD)nStrong;
				ItemOp.wCount = (WORD)nValue;
				ItemOp.btBindFlag = (BYTE)nBindFlag;
				ItemOp.nLeftTime = 0;
				pItemOp = &ItemOp;
			}
			if(pItemOp->wCount <= 0)
			{
				printf("GiveAward count:id = %d\n", pItemOp->wCount);
			}
			if (pItemOp == NULL) return false;
			if(GetBagSystem().CanAddItem(*pItemOp) ==false) return false;
			INT_PTR nAddValue = GetBagSystem().AddItem(*pItemOp,pStr,nLogId);
			if (abs(nAddValue) < abs(nValue))
				isSucceed = false;
			// m_questSystem.OnQuestEvent(CQuestData::qtGetItem,nValue, nId);
			break;
		}
		//经验
	case qatExp:
		{
			AddExp(nValue,nLogId,0,param);//增加经验
			break;
		}
	case qatStatic:
		{
			if(nId == 0) // 0计数器类型表示 是时间
				return false;
			GetStaticCountSystem().AddStaticCount(nId,nValue);//计数器
			break;
		}
	case qatWarPoint: //战令积分
		{
			GetStaticCountSystem().AddStaticCount(nOrderWard_StaticType, nValue);//计数器
			break;
		}
	case qatZLMoney: //战令币
		{
			GetStaticCountSystem().OnAddOrderWardMoney(nValue);//计数器
			
			SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY));
			break;
		}
	case qatMultiExpUnused://剩余未使用多倍经验
		{
			LONGLONG old_num = GetProperty<LONGLONG>(PROP_ACTOR_MULTI_EXP_UNUSED);
			LONGLONG new_num = old_num + nValue;
			SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED, new_num);
		break;	
		}
	//积分
	case qaIntegral:
		{
			isSucceed = ChangeActorRecyclePonit((int)nValue);
			break;
		}
		//帮派贡献度
	case qatGuildContribution:
		{
			if(!m_guildSystem.IsFree())
				m_guildSystem.ChangeGuildGx((int)nValue, nLogId);
			break;
		}
	case qatBindMoney:	//现在是给金币
		{

			isSucceed = ChangeBindCoin(nValue,nLogId,0,_T(pStr),TRUE);
			break;
		}
		//银两
	case qatMoney:
		{
			isSucceed = ChangeCoin(nValue,nLogId,0,_T(pStr),TRUE);
			break;
		}
		//绑定元宝
	case qatBindYb:
		{
			isSucceed = ChangeBindYuanbao(nValue,nLogId,0,_T(pStr),TRUE);
			break;
		}
		// 元宝
	case qatYuanbao:
		{
			isSucceed = ChangeYuanbao(nValue, nLogId, 0, _T(pStr), TRUE);
			break;
		}
		//称号
	case qatTitle:
		{
			isSucceed = GetNewTitleSystem().addNewTitle((WORD)nId);
			break;
		}
	//自定义称号
	case qatCustomTitle:
		{
			isSucceed = GetNewTitleSystem().addCustomTitle(nId);
			break;
		}
		//宠物
	case qatLootPet:
		{
			isSucceed = GetLootPetSystem().addLootPet((WORD)nId);
			break;
		}
		//声望值
	case qatPrestigeNum:
		{
			isSucceed = ChangePrestige((int)nValue);
			break;	
		}
	//飞鞋
	case qatFlyShoes:
		{
			// isSucceed = ChangeFlyShoesNum(nValue);
			// break;
			isSucceed = ChangeFlyShoes(nValue, nLogId, 0, _T(pStr), TRUE);
			break;
		}
	//交易额度
	case qatJyQuota :
	{
		isSucceed = ChangeJyQuota(nValue, nLogId, 0, _T(pStr), TRUE);
		break;
	}
		//喇叭
	case qatBroat:
		{
			isSucceed = ChangeBroatNum(nValue);
			break;
		}

	case qaGuildDonate://行会贡献
	{
		int nOldValue = GetProperty<unsigned int>(PROP_ACTOR_GUILDEXP);
		int nNewValue = nOldValue + nValue;
		if(nNewValue < 0)
			nNewValue = 0;
		SetProperty<unsigned int>(PROP_ACTOR_GUILDEXP, nNewValue);
		std::string sTemp = nValue > 0 ? "+":"";
		sTemp = intToString(nValue, sTemp);

		SendTipmsgFormatWithId(tmGuildDonate, tstEcomeny, sTemp.c_str());
		isSucceed = true;
		break;
	}
	///
		//技能
	case qatSkill:
		{
			INT_PTR nErr = ((CAnimal*)this)->GetSkillSystem().LearnSkill(nValue,param);
			isSucceed = nErr == 0 ? true : false;
			break;
		}
	case qatAchieveEvent: //成就点
		{
			GetAchieveSystem().SetAchieveFinished(nValue);	
			break;
		}

	case qatActivity:  //奖励活跃度
		{
			//GetAcitivityAward((int)nValue);
			ChangeActivity( (int)nValue );
			break;
		}

	case qatGuildFr:		//奖励繁荣度
		{
			//帮派是否存在
			if(!m_guildSystem.IsFree())
			{
				m_guildSystem.ChangeGuildFr((int)nValue);
			}
			break;
		}

	case qatXPVal:		// 命盘技能XP值
		{
			break;
		}
	case qatCircleSoul:
		{
			ChangeCircleSoul((int)nValue);
			break;
		}

	case qatAnger:
		{
			AddAnger((int)nValue);
			break;
		}
		//刷出一个boss
	case qatBoss:
		{
			pScene = GetScene(); //当前的场景
			if (pScene)
			{
				GetPosition(x,y); //获得当前的坐标
				//直接在自己旁边刷出一个怪物
				for(nOldValue =0; nOldValue < nValue; nOldValue ++)
				{
					isSucceed = pScene->CreateEntityAndEnterScene(nId, enMonster,x,y,-1,NULL,0) != NULL;
				}
				break;
			}
		}
	case qatUpgrade:
		{
			unsigned int nNewLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL) + (int)nValue;
			unsigned int nMaxPlayerLevel = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetPlayerMaxLevel(GetProperty<unsigned int>(PROP_ACTOR_CIRCLE));
			if (nNewLevel > nMaxPlayerLevel)
			{
				isSucceed = false;
				break;
			}
			SetLevel(nNewLevel);
			break;
		}
	case qatPetUpgrade:
		{
			bool boNotice = false;
			INT_PTR nPetCount = GetPetSystem().GetPetCount();
			for (INT_PTR i = 0; i < nPetCount; i++)
			{
				CPet * pPet = (CPet*)GetEntityFromHandle(GetPetSystem().GetPetData(i)->handle);
				if (pPet != NULL)
				{
					int nLevel = pPet->GetProperty<int>(PROP_CREATURE_LEVEL);
					if (nLevel < nValue)
					{
						((CEntity*)pPet)->SetLevel((unsigned int)nValue);
						boNotice = true;
					}
				}
			}
			if (boNotice)
			{
				SendOldTipmsgFormatWithId(tpPetGetLevel, ttFlyTip, nValue);
			}
			break;
		}

	case qatGuildCoin:		//行会资金
		{
			if(m_guildSystem.GetGuildPtr())
			{
				m_guildSystem.GetGuildPtr()->ChangeGuildCoin((int)nValue, nLogId,pStr);
			}
			break;
		}
	case qatPersonBossJifen: //个人BOSS积分
		{
			isSucceed = ChangeMoney(mtBossJiFen,nValue,nLogId,0,_T(pStr),true); 
			break;
		}
	case qatDragonSoulValue://龙魂碎片
		{
			//ChangePropertyValue(PROP_ACTOR_DRAGONSOUL_VALUE, nValue, tpAddDragonSoulValue, tpReduceDragonSoulValue);
			break;
		}
	case qatBossJifen:
		{
			isSucceed = ChangeMoney(mtBossJiFen,nValue,nLogId,0,_T(pStr),TRUE);
			break;
		}
	case qatCombatScore:
		{
			m_CombatSystem.ChangeCombatScore((int)nValue);
			break;
		}
	//神魔
	case qatGhost:
		{
			m_nGhostSystem.GetAwardNum(nValue);
			break;
		}
	//复活特权
	case qatReviveDuration:
		{
			OneReviveDurationConfig* pConfig = GetLogicServer()->GetDataProvider()->GetReviveDurationsConfig().GetReviveDurationConfig(nId);
			if(pConfig)
			{
				m_ReviveDurationSystem.AddReviveDuration(pConfig->nReviveDurationId);
				SendTipmsg(pConfig->sTip, tstUI);
			}
			break;
		}
	case qatSecretBoxScore:		//秘境宝箱数量(积分)
	case qatWordsBoxScore:		//字诀宝箱数量(积分)
	case qatMaterialsBoxScore:	//材料宝箱数量(积分)
		{
			static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if ( globalNpc )
			{
				unsigned int nActivityId = 43;	// 秘境打宝 活动Id
				CScriptValueList paramList;
				paramList << nActivityId;
				paramList << this;
				paramList << nType;
				paramList << nValue;

				if ( !globalNpc->GetScript().CallModule("ActivityType28", "UpdateActorTypesScore", paramList, paramList, 0) )
				{
					OutputMsg(rmError,"[GActivity 28] 秘境打宝 UpdateActorTypesScore 错误 玩家 %s 更新 积分类型 ：%d，积分 ：%d 失败", GetEntityName(), nType, nValue);
				}
			}
			break;
		}
	default: 
		{
			break;//自定义奖励，不需要这里处理
		}
	}
	//元宝必须要记录日志[zgame所有都记录消费日志]
	if(!getMoneyType(nType) && nValue > 0 && nType != 0)
	{
		Uint64 nNewValue = GetConsume(nType,nId);
		int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
		// 记录全局日志
		GetGlobalLogicEngine()->AddCosumeToLog((WORD)nLogId, 
					(int)GetId(), GetAccount(), GetEntityName(), (BYTE)nType,
					(int)nValue, (int)0, pStr, nLevel, (unsigned int)nNewValue, getOldSrvId());	
	}
	if (getMoneyType(nType))
	{
		Uint64 nNewValue = GetConsume(nType,nId);
		// 记录本地日志
		GetLogicServer()->SendCurrencyLocalLog((int)nLogId,this,nValue>0?1:2,(int)nType,nValue,(unsigned int)nNewValue,(nLogId == GameLog::Log_ShopBuy ? 1 : 0));
	}
	
	if(nType > 0 && !getMoneyType(nType) && nValue > 0)
	{
		m_achieveSystem.ActorAchievementComplete(nAchieveActorNum, nValue, nType);
	}
	return isSucceed;
}
bool CActor::getMoneyType(int nType)
{
	switch (nType)
	{
	case mtCoin:
	case mtBindCoin:
	case mtBindYuanbao:
	case mtYuanbao:
	case mtFlyShoes:
	case mtJyQuota:
		return true;
	}
	return false;
}

void CActor::BroadRelive()
{
	char buff[128];
	CDataPacket outPack(buff,sizeof(buff)); //下发的数据包
	outPack << (BYTE) enDefaultEntitySystemID << (BYTE)sActorRelive;
	outPack << (INT64)GetHandle();
	GetObserverSystem()->BroadCast(outPack.getMemoryPtr(), outPack.getPosition(),true); //附近广播
}

bool CActor::SetValueAward(int nType, int value)
{
	bool isSucceed =true; //一般都是成功的
	int nOldValue;
	CScene *pScene ; //当前的场景
	int x,y;
	switch(nType)
	{
	case qatExp:
		{
			SetProperty<Uint64>(PROP_ACTOR_EXP,(Uint64)value);
			break;
		}
		//帮派贡献度
	case qatBindMoney:	//现在是给金币
		{

			SetProperty<unsigned int>(PROP_ACTOR_BIND_COIN,value);
			break;
		}
		//银两
	case qatMoney:
		{
			SetProperty<unsigned int>(PROP_ACTOR_COIN,value);
			break;
		}
		//回收积分
	case qaIntegral:
		{
			SetProperty<unsigned int>(PROP_ACTOR_RECYCLE_POINTS,value);
			break;
		}
		//绑定元宝
	case qatBindYb:
		{
			SetProperty<unsigned int>(PROP_ACTOR_BIND_YUANBAO,value);
			break;
		}
		// 元宝
	case qatYuanbao:
		{
			SetProperty<unsigned int>(PROP_ACTOR_YUANBAO,value);
			break;
		}
	//飞鞋
	case qatFlyShoes:
		{
			SetProperty<unsigned int>(PROP_ACTOR_FLYSHOES,value);
			break;
		}
		//喇叭
	case qatBroat:
		{
			SetProperty<unsigned int>(PROP_ACTOR_BROATNUM,value);
			break;
		}
	case qatCircleSoul://转生修为
		{
			SetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL,(unsigned int)value);
			break;
		}
	case qaGuildDonate://行会贡献
		{

		SetProperty<unsigned int>(PROP_ACTOR_GUILDEXP, value);
		break;	
		}

	case qatMultiExpUnused://剩余未使用多倍经验
		{
		SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED, value);
		break;	
		}
	case qatBasicExpNum://当前基础经验数值
		{
		m_StaticCountSystem.SetStaticCount(nDailyStaticBasicExp,value);
		break;	
		}
	case qatPrestigeNum://声望值
		{
		SetProperty<unsigned int>(PROP_ACTOR_Prestige, value);
		break;	
		}
	case qatActivity://声望值
		{
		SetProperty<unsigned int>(PROP_ACTOR_ACTIVITY, value);
		break;	
		}

	default: 
		{
			break;//自定义奖励，不需要这里处理
		}
	}
	return isSucceed;
}


Uint64 CActor::GetConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue, INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, const INT_PTR nParam)
{
	Uint64 nResult = 0;
	unsigned int nCurValue = 0;
	switch(nType)
	{
	//装备 或 道具
	case qatEquipment:
		{
			const CUserItemContainer::ItemOPParam *pItemOp;
			pItemOp = (CUserItemContainer::ItemOPParam *) nParam;
			if (pItemOp == NULL)
			{
				if (nId <= 0) return false;
				CUserItemContainer::ItemOPParam ItemOp;
				ItemOp.wItemId = (WORD)nId;
				ItemOp.btQuality = (BYTE)nQuality;
				ItemOp.btStrong = (BYTE)nStrong;
				ItemOp.btBindFlag = (BYTE)nBindFlag;
				pItemOp = &ItemOp;
			}
			if (pItemOp == NULL) return false;
			nCurValue = (unsigned int)GetBagSystem().GetItemCount(pItemOp->wItemId,pItemOp->btQuality,pItemOp->btStrong,pItemOp->btBindFlag);
			break;
		}	
	//帮派贡献度
	case qaGuildDonate:
		{
			if(!m_guildSystem.IsFree())
				nCurValue = (unsigned int)m_guildSystem.GetGx();
			break;
		}
	//绑定银两
	case qatBindMoney:
		{
			//绑定银两不足 消耗银两
			nCurValue = (unsigned int)GetMoneyCount(mtBindCoin);
			break;
		}
	//战令积分
	case qatWarPoint:
		{
			if(nId == 0) // 0计数器类型表示 是时间
				return false;
			nCurValue = GetStaticCountSystem().GetStaticCount(nOrderWard_StaticType);//计数器
			break;
		}
	case qatZLMoney: //战令币
		{
			if(nId == 0) // 0计数器类型表示 是时间
				return false;
			nCurValue = GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY);//计数器
			
			break;
		}
	//银两
	case qatMoney:
		{
			nCurValue = (unsigned int)GetMoneyCount(mtCoin);
			break;
		}
	//绑定元宝
	case qatBindYb:
		{ 
			//绑元 + 元宝
			nCurValue = (unsigned int)GetMoneyCount(mtBindYuanbao,true);
			break;
		}
	// 元宝
	case qatYuanbao:
		{
			nCurValue = (unsigned int)GetMoneyCount(mtYuanbao);
			break;
		}
	//喇叭
	case qatBroat:
		{
			nCurValue = GetProperty<unsigned int>(PROP_ACTOR_BROATNUM);
			break;
		}
	//飞鞋
	case qatFlyShoes:
		{
			nCurValue = (unsigned int)GetMoneyCount(mtFlyShoes);
			break;
		}
	//回收积分
	case qaIntegral:
	{
		nCurValue = GetProperty< int>(PROP_ACTOR_RECYCLE_POINTS);
		break;
	}	
	//称号
	case qatTitle:
		{
			if(nId > 0 && GetNewTitleSystem().IsHaveNewTitle((WORD)nId))
				nCurValue = 1;
			break;
		}
	//技能
	case qatSkill:
		{
			if (nValue > 0)
				nCurValue = (unsigned int)((CAnimal*)this)->GetSkillSystem().GetSkillLevel(nValue);
			break;
		}
	//奖励活跃度
	case qatActivity:  
		{
			nCurValue = GetProperty<unsigned int>(PROP_ACTOR_ACTIVITY);
			break;
		}
	//奖励繁荣度
	case qatGuildFr:
		{
			//帮派是否存在
			if(!m_guildSystem.IsFree())
			{
				nCurValue = (unsigned int)m_guildSystem.GetGuildPtr()->GetFr();
			}
			break;
		}

	// 命盘技能XP值
	case qatXPVal:		
		{
			break;
		}
	//转生灵魄
	case qatCircleSoul:	
		{
			nCurValue = GetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL);
			break;
		}
	//声望
	case qatPrestigeNum:
	{

		nCurValue = GetProperty<unsigned int>(PROP_ACTOR_Prestige);
		break;	
	}

	case qatAnger:
		{
			nCurValue = GetProperty<unsigned int>(PROP_ACTOR_ANGER);
			break;
		}

	case qatDragonSoulValue:
		{
			//nCurValue = GetProperty<unsigned int>(PROP_ACTOR_DRAGONSOUL_VALUE);
			break;
		}
	case qatCombatScore:
		{
			nCurValue = m_CombatSystem.GetCombatScore();
			break;
		}
	//计数器类型
	case qatStatic:
	{
		if(nId == 0) // 0计数器类型表示 是时间
			break;
		nCurValue = GetStaticCountSystem().GetStaticCount(nId);//计数器
		break;
	}
	//交易额度
	case qatJyQuota:
	{
		nCurValue = (unsigned int)GetMoneyCount(mtJyQuota);
		break;
	}
	//
	default: 
		{
			break;//自定义奖励，不需要这里处理
		}
	}
	if (nType == qatExp)
	{
		nResult = (Uint64)GetProperty<Uint64>(PROP_ACTOR_EXP); 
	}
	else if(nType == qatMultiExpUnused)
	{
		return (Uint64)GetProperty<Uint64>(PROP_ACTOR_MULTI_EXP_UNUSED); 
	}
	else 
	{
		nResult = (Uint64)nCurValue;
	}
	return nResult;
}
////-----不用了 改用
bool CActor::CheckConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue, INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, const INT_PTR nParam)
{
	Uint64 nCurValue = GetConsume(nType, nId, nValue, nQuality, nStrong, nBindFlag, nParam);
	return nCurValue >= (Uint64)nValue ? true : false;
}

/*
此方法仅用于扣除消耗
*/
INT_PTR CActor::RemoveConsume(INT_PTR nType, INT_PTR nId, INT_PTR nValue, 
	INT_PTR nQuality, INT_PTR nStrong, INT_PTR nBindFlag, const INT_PTR nParam,INT_PTR nLogId,LPCTSTR sComment)
{
	INT_PTR nChgValue = 0;
	if (nValue <= 0) return 0;
	switch (nType)
	{
		//装备 或 道具
	case qatEquipment:
		{
			int ncount = nValue;
			do
			{
				CUserItem *pUserItem = GetBagSystem().FindItem(nId);
				if(pUserItem)
				{
					int cost = nValue < pUserItem->wCount? nValue:pUserItem->wCount;
					nValue -= cost;
					if(nValue < 0)
						nValue = 0;
					if(cost > 0)
						nChgValue += GetBagSystem().DeleteItem(pUserItem, cost,"self_compose_item",nLogId,true);
				}
				else
				{
					return nChgValue >= 0 ? nChgValue : -nChgValue;
				}
				
			} while (nValue > 0);

			GetAchieveSystem().ActorAchievementComplete(nAchieveCostItem, ncount,  nId);
			break;
		}
		//金币
	case qatMoney:
		{
			if (ChangeCoin(-nValue,nLogId,nParam,_T(sComment),TRUE)) {
				nChgValue = nValue;
			}
			break;
		}
		//绑定金币
	case qatBindMoney:
		{
			if( ChangeMoney(mtBindCoin, -nValue, nLogId,nParam,sComment)) {
				nChgValue = nValue;
			}
			break;
		}
		//绑定元宝
	case qatBindYb:
		{
			if( ChangeBindYuanbao( -nValue,nLogId,nParam,_T(sComment),TRUE)) {
				nChgValue = nValue;
			}
			break;
		}
		// 元宝
	case qatYuanbao:
		{
			if (ChangeYuanbao(-nValue,nLogId,nParam,_T(sComment),TRUE)) {
				nChgValue = nValue;
			}
			break;
		}
		//经验
	case qatExp:
		{
			Uint64 nCurValue = GetProperty<Uint64>(PROP_ACTOR_EXP);
			if (nCurValue < (Uint64)nValue)
				return 0;
			AddExp(-nValue,nLogId,nId);//增加经验
			nChgValue = nValue;
			break;
		}
	case qatCircleSoul:	//转生灵魄
		{
			unsigned int nCurValue = GetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL);
			if (nCurValue < (unsigned int)nValue)
				return 0;
			ChangeCircleSoul((int)nValue * -1);
			nChgValue = nValue;
			break;
		}
	case qatWarPoint: //战令积分
	{
		if(nId == 0) // 0计数器类型表示 是时间
			return false;
		GetStaticCountSystem().AddStaticCount(nOrderWard_StaticType,(int)nValue * -1);//计数器
		nChgValue = nValue;
		break;
	}
	case qatZLMoney: //战令币
	{
		if(nId == 0) // 0计数器类型表示 是时间
		{
			return false;
		}
		GetStaticCountSystem().OnAddOrderWardMoney((int)nValue * -1);//计数器
		SetProperty<unsigned int>(PROP_ACTOR_ZL_MONEY, GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY));
		nChgValue = nValue;
		break;
	}
	//飞鞋
	case qatFlyShoes:
		{
			if( ChangeFlyShoes( -nValue,nLogId,nParam,_T(sComment),TRUE)) {
				nChgValue = nValue;
			}
			// ChangeFlyShoesNum((int)nValue * -1);
			// nChgValue = nValue;
			break;
		}
	//交易额度
	case qatJyQuota:
		{
			if( ChangeJyQuota( -nValue,nLogId,nParam,_T(sComment),TRUE)) {
				nChgValue = nValue;
			}
			// ChangeFlyShoesNum((int)nValue * -1);
			// nChgValue = nValue;
			break;
		}
		//喇叭
	case qatBroat:
		{
			ChangeBroatNum((int)nValue * -1);
			nChgValue = nValue;
			break;
		}
	//积分
	case qaIntegral:
		{
			ChangeActorRecyclePonit((int)nValue * -1);
			nChgValue = nValue;
			break;
		}
		//行会贡献
	case qaGuildDonate:
		{
			// if(!m_guildSystem.IsFree())
			// {
			// 	unsigned int nCurValue = (unsigned int)m_guildSystem.GetGx();
			// 	if (nCurValue < (unsigned int)nValue)
			// 		return 0;
			// 	m_guildSystem.ChangeGuildGx(-(int)nValue);
			// 	nChgValue = nValue;
			// }
			int nOldValue = GetProperty<unsigned int>(PROP_ACTOR_GUILDEXP);
			int nNewValue = nOldValue - nValue;
			if(nNewValue < 0)
				nNewValue = 0;
			SetProperty<unsigned int>(PROP_ACTOR_GUILDEXP, nNewValue);
			nChgValue = nValue;
			break;
		}
	case qatPrestigeNum://声望
		{

			int nOldValue = GetProperty<unsigned int>(PROP_ACTOR_Prestige);
			int nNewValue = nOldValue - nValue;
			if(nNewValue < 0)
				nNewValue = 0;
			SetProperty<unsigned int>(PROP_ACTOR_Prestige, nNewValue);
			nChgValue = nValue;
			break;
		}

	
		//称号
	case qatTitle:
		{
			if(nId > 0 && GetNewTitleSystem().IsHaveNewTitle((WORD)nId))
			{
				GetNewTitleSystem().delNewTitle((WORD)nId);
				nChgValue = 1;
			}
			break;
		}
		//技能
	case qatSkill:
		{
			if (nValue > 0 && ((CAnimal*)this)->GetSkillSystem().GetSkillLevel(nValue) > 0)
			{
				if (((CAnimal*)this)->GetSkillSystem().ForgetSkill(nValue))
				{
					nChgValue = 1;
				}
			}
			break;
		}
	case qatActivity:  //奖励活跃度
		{
			unsigned int nCurValue = GetProperty<unsigned int>(PROP_ACTOR_ACTIVITY);
			if (nCurValue < (unsigned int)nValue)
				return 0;
			//GetAcitivityAward(-(int)nValue);
			ChangeActivity( -(int)nValue );
			nChgValue = nValue;
			break;
		}

	case qatGuildFr:		//奖励繁荣度
		{
			//帮派是否存在
			if(!m_guildSystem.IsFree())
			{
				unsigned int nCurValue = (unsigned int)m_guildSystem.GetGuildPtr()->GetFr();
				if (nCurValue < (unsigned int)nValue)
					return 0;
				m_guildSystem.ChangeGuildFr(-(int)nValue);
				nChgValue = nValue;
			}
			break;
		}

	case qatXPVal:		// 命盘技能XP值
		{
			break;
		}
	case qatAnger:	//怒气
		{
			unsigned int nCurValue = GetProperty<unsigned int>(PROP_ACTOR_ANGER);
			if (nCurValue < (unsigned int)nValue)
				return 0;
			int nOldValue=(int)( -nValue + nCurValue);
			if (nOldValue < 0)//不能低于0，无最大限制
			{
				nOldValue = 0;
			}
			int nMaxAnger = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationMaxAnger(GetProperty< int>(PROP_ACTOR_VOCATION));
			int curValue = MAKELONG(nOldValue,nMaxAnger);

			SetProperty<unsigned int>(PROP_ACTOR_ANGER,(unsigned int)curValue);
			nChgValue = nValue;
			break;
		}

	case qatGuildCoin:		//行会资金
		{
			if(m_guildSystem.GetGuildPtr())
			{
				if(m_guildSystem.GetGuildPtr()->GetCoin() >= nValue)
				{
					m_guildSystem.GetGuildPtr()->ChangeGuildCoin((int)nValue * -1,nLogId, sComment);
				}
			}
			break;
		}
	case qatPersonBossJifen: //个人BOSS积分
		{
			unsigned int nCurValue = GetProperty<unsigned int>(PROP_ACTOR_PERSONBOSS_JIFEN);
			if (nCurValue < (unsigned int)nValue)
				return 0;
			int nOldValue=(int)( -nValue + nCurValue);
			if (nOldValue < 0)
			{
				nOldValue = 0;
			}
			SetProperty<unsigned int>(PROP_ACTOR_PERSONBOSS_JIFEN,(unsigned int)nOldValue);
			nChgValue = nValue;
			break;
		}
	case qatCombatScore:
		{
			int nCurValue = m_CombatSystem.GetCombatScore();
			if (nCurValue < nValue)
			{
				return 0;
			}
			m_CombatSystem.ChangeCombatScore(-(int)nValue);
			nChgValue = nValue;
			break;
		}
	default: 
		{
			break;//自定义奖励，不需要这里处理
		}
	}

	//元宝必须要记录日志[zgame所有都记录消费日志]
	if(!getMoneyType(nType) && nValue > 0)
	{
		Uint64 nNewValue = GetConsume(nType,nId);
		int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
		// 记录全局日志
		GetGlobalLogicEngine()->AddCosumeToLog((WORD)nLogId, 
					(int)GetId(), GetAccount(), GetEntityName(), (BYTE)nType,
					(int)nValue*-1, (int)0, sComment, nLevel, (unsigned int)nNewValue, getOldSrvId());	
	}

	if (getMoneyType(nType))
	{
		Uint64 nNewValue = GetConsume(nType,nId);
		// 记录本地日志
		GetLogicServer()->SendCurrencyLocalLog((int)nLogId,this,nValue>0?1:2,(int)nType,nValue,(unsigned int)nNewValue,(nLogId == GameLog::Log_ShopBuy ? 1 : 0));
	}
	return nChgValue >= 0 ? nChgValue : -nChgValue;
}

void CActor::AddAnger(int nValue, bool boFull)
{
	int nOldValue=(int)(nValue + LOWORD(GetProperty< int>(PROP_ACTOR_ANGER)));
	if (nOldValue < 0)	//不能低于0，无最大限制
	{
		nOldValue = 0;
	}

	int nMaxAnger = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationMaxAnger(GetProperty< int>(PROP_ACTOR_VOCATION));
	if (boFull)
	{
		nOldValue = nMaxAnger;
	}
	if(nOldValue>nMaxAnger)
		nOldValue = nMaxAnger;

	//发消息失去了怒气
	if(nValue >0)
	{
		int addValue = nOldValue - LOWORD(GetProperty< int>(PROP_ACTOR_ANGER));
		if(addValue > 0)
			SendOldTipmsgFormatWithId(tpGetAngerTips,ttTipmsgWindow,(int)addValue);
	}
	else if(nValue < 0)
	{
		SendOldTipmsgFormatWithId(tpLostAngerTips,ttTipmsgWindow,(int)-nValue);
	}

	int curValue = MAKELONG(nOldValue,nMaxAnger);
	SetProperty<unsigned int>(PROP_ACTOR_ANGER,(unsigned int)curValue);	
}


//喇叭
bool CActor::ChangeBroatNum(int nValue)
{
	int num = GetProperty<int>(PROP_ACTOR_BROATNUM) + nValue;
	// if(num < 0)
	// {
	// 	SendTipmsgFormatWithId(tmBoratNotEnough,tstFigthing);
	// 	return false;
	// } 
	std::string sTemp = nValue > 0 ? "+":"";
	sTemp = intToString(nValue, sTemp);

	SendTipmsgFormatWithId(tmBroatNum, tstEcomeny, sTemp.c_str());
	SetProperty<unsigned int>(PROP_ACTOR_BROATNUM,(unsigned int)num);
	return true;
}


bool CActor::ChangeFlyShoesNum(int nValue)
{
	int num = GetProperty<int>(PROP_ACTOR_FLYSHOES) + nValue;
	// if(num < 0)
	// {
	// 	SendTipmsgFormatWithId(tmflyshoesNotEnough,tstFigthing);
	// 	return false;
	// } 

	std::string sTemp = nValue > 0 ? "+":"";
	sTemp = intToString(nValue, sTemp);

	SendTipmsgFormatWithId(tmFlyShoeNum, tstEcomeny, sTemp.c_str());
	SetProperty<unsigned int>(PROP_ACTOR_FLYSHOES,(unsigned int)num);
	return true;
}



/*奖励活跃度，不使用了

*/
void CActor::GetAcitivityAward(int value)
{
	return;
	/*
	int nOldValue;
	nOldValue=(int)( value + GetProperty<unsigned int>(PROP_ACTOR_ACTIVITY));

	if (nOldValue < 0)
	{
		nOldValue = 0;
	}
	SetProperty<unsigned int>(PROP_ACTOR_ACTIVITY,(unsigned int)nOldValue);	

	GetAchieveSystem().OnEvent(aAchiveActivityNum,nOldValue);

	if(value>0)
	{
		GetQuestSystem()->OnQuestEvent( CQuestData::qtActiviyPoint, 0, nOldValue);
		SendOldTipmsgFormatWithId(tpGetAcitivity,ttTipmsgWindow,value);
		CScriptValueList paramList, retList;
		InitEventScriptVaueList(paramList,aeAcitivityAward);
		paramList<<value;
		OnEvent(paramList,retList);
	}
	*/
}


void CActor::SetNotReloadMapPos( int nSceneId,int x,int y )
{
	m_nNotReloadMapPosX = x;
	m_nNotReloadMapPosY = y;
	m_nNotReloadMapSceneId = nSceneId;
}
void CActor::GetNotReloadMapPos( int& nSceneId,int& x,int& y )
{
	// 此地方不区分？
	//if (IsInRawServer())
	{
		x = m_nNotReloadMapPosX;
		y = m_nNotReloadMapPosY;
		nSceneId = m_nNotReloadMapSceneId;
	}

	//OutputMsg(rmTip, _T("设置玩家非重配地图位置信息(%d, %d_%d)"), nSceneId, x, y);
}

void CActor::InitFriendList(CVector<int> &actorIdList)
{
	for (INT_PTR i = 0; i < actorIdList.count(); i++)
	{
		m_FriendList.push(actorIdList[i]);		
	}
}

VOID CActor::OnFriendChanged(bool bAdd, unsigned int nActorId)
{
	if (bAdd)
	{
		m_FriendList.push(nActorId);	
		
	}
	else
	{
		for (INT_PTR i = 0; i < m_FriendList.count(); i++)
		{
			if (m_FriendList[i] == nActorId)
			{
				m_FriendList.remove(i);				
			}
		}
	}
}


void CActor::OnEntityScriptDelayMsgCallback(unsigned int nMsgIdx, int nParam1, int nParam2, int nParam3, int nParam4, int nParam5)
{
	/*
	CScriptValueList paramList, retList;
	InitEventScriptVaueList(paramList, aeScriptDelayCallback);
	paramList << nParam1 << nParam2 << nParam3 << nParam4 << nParam5;
	this->OnEvent(paramList, retList);
	*/
}

INT_PTR CActor::removeEntityScriptDelayMsg(unsigned int nMsgIdx)
{
	INT_PTR result = 0;
	CEntityMsg *pMsg;
	CLogicEngine *pLogicEngine = GetGlobalLogicEngine();
	CList<CEntityMsg*>::Iterator it(m_EntityMsgList);
	CList<CEntityMsg*>::NodeType *pNode;
	for (pNode=it.first(); pNode; pNode = it.next())
	{
		pMsg = *pNode;
		if (pMsg->nMsg == CEntityMsg::emEntityScriptDelay && ((unsigned int)(pMsg->nParam1) == nMsgIdx))
		{
			pMsg->bWaitDel =true;
			result++;
		}
	}
	return result;
}
//处理跨天的数据
void CActor::dealNewDayAchieve(int nDiffDay)
{
	if (nDiffDay == 0) return;
	int nlMaxDay = GetAchieveSystem().GetAchieveCompleteValueByType(nAchieveContinueLogin);
	if(nDiffDay > 1)
	{
		nlMaxDay = 0;
	}
	nlMaxDay++;
	GetAchieveSystem().ActorAchievementComplete(nAchieveContinueLogin, nlMaxDay);
	// GetStaticCountSystem().SetStaticCount(nContinueLoginMaxTimes, nlMaxDay);
}
void CActor::PreCloseActor()
{
	CMiniDateTime curTime;
	curTime.tv = GetGlobalLogicEngine()->getMiniDateTime();
	if (curTime.tv >= m_tomorrowDateTime.rel_today()/* && curTime.tv <= (m_tomorrowDateTime.tv +TIME_FLUCTUATE)*/)
	{
		printf("PreCloseActor:%d, rel_today:%d\n",curTime.tv, m_tomorrowDateTime.rel_today());
		OnNewDayArrive();
		GetAchieveSystem().ActorAchievementComplete(nAchieveLogin, 1);
		
	}
	//删除场景区域添加的buff
	CScene* pScene = GetScene();
	int x ,y;
	GetPosition(x,y);
	if(SCENEAREA* pArea = pScene?pScene->GetAreaAttri(x,y):NULL)
	{
		RemoveAreaBuff(pArea);
	}
	if (pScene)
	{
		if (pScene->HasMapAttribute(x, y, aaCannotSeeName))
		{
			SetSocialMask(smHideHeadMsg, false);
			CHero * pHero = GetHeroSystem().GetBattleHeroPtr();
			if(pHero)
			{
				pHero->SetSocialMask(smHideHeadMsg, false);
			}
		}
	}

	GetDealSystem().CancelDeal();
}

/*--注意仅在在线时跨天才调用
*/
void CActor::OnNewDayArrive(int nDayDiff) 
{
	// 清当天杀怪累积经验
	unsigned int nExpGetToday =m_nExpGetToday;
	m_nExpGetToday = 0;
	OnExpGetTodayStatusChanged(false);

	//清除当天的阵营贡献
	unsigned int nZyContriGetToday = m_nZyContriGetToday;
	m_nZyContriGetToday = 0;

	//清除活跃度
	// unsigned int nNewValue = 0;
	// SetProperty<unsigned int>(PROP_ACTOR_ACTIVITY,(unsigned int)nNewValue);	

	//清除活跃度是否领取奖励
	// SetSocialMask(smDarwActivityAward,false);


	//多倍经验更新
	do{
		int openday = GetLogicServer()->GetDaysSinceOpenServer() ;
		int i = m_property.nLastLoginOpendayNo_ ;
		//今天首次登陆
		if(i < openday ){
			//开服天数有关的加多倍经验总量
			EXPLMTOPENDAY & expLmt = GetLogicServer()->GetDataProvider()->GetExpLmtOpenday() ;
			LONGLONG exp_i = 0 ;

			// // 2021-08-03 多倍经验 暂时不用
			// for( ; i<openday ; i++){
			// 	if( expLmt.m_expLmtOpenday.find(i+1) != expLmt.m_expLmtOpenday.end() ){
			// 		exp_i += ( LONGLONG )expLmt.m_expLmtOpenday[i+1].multilmt ;	
			// 	}
			// }
			// SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED , GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) +exp_i);
			//m_property.lMultiExpUnused += exp_i ;



			//合服天数相关的加多倍经验
			//前三天每天多加100w
			int cmbDay = GetLogicServer()->GetDaysSinceCombineServer() ;
			//不在活动时间内, 跳出循环
			if(cmbDay>0 && cmbDay<=3)
			{
				SetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED , (GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED) + 1000000));
			}

			//合服操作
			if(cmbDay>0)
			{
				CMiniDateTime combineServerTime  = GetLogicServer()->GetServerCombineTime(); //获取合服的绝对时间
				if((combineServerTime.tv > 0) && (GetStaticCountSystem().GetStaticCount(nLastCombineSrvMinTime) != combineServerTime.tv ))
				{
					//合服天数差
					int diffDay = openday - m_property.nLastLoginOpendayNo_  ;
					//start  这里的逻辑只会在每次合服后的第一次登陆时执行
						//个人活动脚本处理
						m_activitySystem.OnCombineServer(diffDay);
						



					//end 
					GetStaticCountSystem().SetStaticCount(nLastCombineSrvMinTime,combineServerTime.tv) ;
				}
			}
			
			m_property.nLastLoginOpendayNo_ = openday ;//更新最后一次登录的开服天数
		}
 
	}while(0) ;

	//刷新一些特殊的成就，比如活动需要每天刷新
	m_achieveSystem.RefeshPerDay(false);

	m_LonginDays++;

	//刷新消费排行榜
	//m_GameStoreBuySystem.SendGetConsumeAwardsTips();

	m_miscSystem.SendOpenServerDays();
	dealNewDayAchieve(nDayDiff);

	// 返利系统
	m_RebateSystem.CheckNextDayAward();

	SetProperty<unsigned int>(PROP_ACTOR_CARD_AWARD_FLAG,(unsigned int)0);

	GetGuildSystem()->OnNewDayArriveOP();

	GetGuildSystem()->SendTodayGetGuildgx();
	//英雄每日清除祝福值
	GetHeroSystem().EveryDayClearBless();
	GetMiscSystem().ClearCheckSignIn();//
	// 脚本事件触发
	CScriptValueList paramList, retList;
	//InitEventScriptVaueList(paramList, aeNewDayArrive);
	//paramList << nExpGetToday<<nZyContriGetToday;
	paramList << nDayDiff;
	OnEvent(aeNewDayArrive,paramList, retList);	

	m_activitySystem.OnNewDayArrive();
	// m_questSystem.AutoAccept();

	m_CurLoginTime.tv =GetGlobalLogicEngine()->getMiniDateTime();
	m_tomorrowDateTime.tv = m_CurLoginTime.tomorrow();//跨天重置tomorrow时间，避免preCloseActor再次调用OnNewDayArrive
	
	
	SetStaticVarValue(szDayHadExchangeExploitCount, 0);
	m_StaticCountSystem.DailyRefresh();
	m_questSystem.CheckCanAcceptQuest();
	m_treasureSystem.RefreshBless(nDayDiff);
}

//改变行会贡献度
bool  CActor::ChangeGuildContri(int nValue,int nLogId)
{
	if(nValue ==0) return false;
	/*
	unsigned int nGuildConr = GetProperty<unsigned int>(PROP_ACTOR_GUILDEXP);
	if(nValue >0)
	{
		nGuildConr += nValue;
	}
	else
	{
		unsigned int nAbsValue = -nValue;
		if(nGuildConr  < nAbsValue)
		{
			return false;
		}
		nGuildConr += nValue;
	}
	SetProperty<unsigned int>(PROP_ACTOR_GUILDEXP,nGuildConr);
	//这里添加日志
	*/
	//GetGuildSystem()->AddGuildGx(nValue);
	GetGuildSystem()->ChangeGuildGx(nValue,nLogId);
	return true;
}

void CActor::OnExpGetTodayStatusChanged(bool bOverUpperLmt)
{
	/* 不清楚什么功能
	CScriptValueList paramList, retList;
	InitEventScriptVaueList(paramList, aeActorGetExpOverUpperLmt);
	paramList << (int)(m_nExpGetToday) << (int)bOverUpperLmt;
	OnEvent(paramList, retList);
	*/
}

void CActor::UpdateExpGetTodayWhenLevelUp()
{
	int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
	int nPrevLevel = nLevel - 1;
	if (nPrevLevel > 0)
	{
		CVector<unsigned int> &cfg = GetLogicServer()->GetDataProvider()->GetExpGetUpperLmtCfg();
		if (nLevel <= cfg.count())
		{
			if (m_nExpGetToday >= cfg[nPrevLevel-1] && m_nExpGetToday < cfg[nLevel-1])
			{				
				OnExpGetTodayStatusChanged(false);
			}
		}
	}
}

void CActor::ChangeVocation(int nVocation)
{
	if (nVocation <= enVocNone || nVocation >= enMaxVocCount)
		return;

	int nLastVocation = GetProperty<unsigned>(PROP_ACTOR_VOCATION);
	if (nVocation == nLastVocation)
		return;

	//遗忘职业技能
	//GetSkillSystem().ForgetSkillByClass(scSkillClassVocBasic);

	//遗忘必杀技能
	//GetSkillSystem().ForgetSkillByClass(scSkillCriticalBlow);

	//改变职业
	SetProperty<unsigned>(PROP_ACTOR_VOCATION, (unsigned)nVocation);

	//学习职业技能
	GetSkillSystem().AutoLearnVocSkill();

	//需要重新计算属性
	CollectOperate(CEntityOPCollector::coRefAbility);	

	//重新计算外观
	CollectOperate(CEntityOPCollector::coRefFeature);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
}

void CActor::UseChangeVocationCard(int nVoc,int nSex)
{
	if (nVoc <= enVocNone || nVoc >= enMaxVocCount)
		return;

	if (nSex != MALEACTOR && nSex != FEMALEACTOR)
		return;

	int nLastVocation = GetProperty<unsigned>(PROP_ACTOR_VOCATION);
	int nLastSex = GetProperty<unsigned>(PROP_ACTOR_SEX);
	if (nVoc == nLastVocation && nSex == nLastSex)
		return;

	if (m_NextChangeVocTime > GetGlobalLogicEngine()->getMiniDateTime())
	{
		SendTipmsgFormatWithId(tmChgVocInCD, tstUI); 
		return;
	}

	// 检查不消耗的道具（永久转职道具）
	bool is_need_to_consum = false;
	std::vector<std::tuple<int,int,int>> const& vecUnConsum = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetChangeVocCfg().vecChangeVocationUnConsum;
	for(int  i = 0; i < vecUnConsum.size(); i++)
	{
		GOODS_TAG item;
		std::tie(item.type, item.id, item.count) = vecUnConsum[i];
		if( !(CheckActorSource(item.type, item.id, item.count, tstUI)) )
		{
			is_need_to_consum = true;
			break;
		}
	}

	// 检查需要消耗的道具（一次性）
	if (is_need_to_consum)
	{
		std::vector<std::tuple<int,int,int>> const& vecConsum = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetChangeVocCfg().vecChangeVocationConsum;
		
		// 检查消耗
		for(int  i = 0; i < vecConsum.size(); i++)
		{
			GOODS_TAG item;
			std::tie(item.type, item.id, item.count) = vecConsum[i];
			if( !(CheckActorSource(item.type, item.id, item.count, tstUI)) )
			{
				return;
			}
		}

		// 消耗
		for(int  i = 0; i < vecConsum.size(); i++)
		{
			GOODS_TAG item;
			std::tie(item.type, item.id, item.count) = vecConsum[i];
			RemoveConsume(item.type, item.id, item.count,-1,-1,-1,0, GameLog::Log_ChgVocation, GameLog::LogItemReason[GameLog::Log_ChgVocation]);
		}
	}
	
	// 处理转职变性

	if (nVoc != nLastVocation)
	{
		//改变职业
		SetProperty<unsigned>(PROP_ACTOR_VOCATION, (unsigned)nVoc);
		//学习职业技能
		unsigned level = GetProperty<unsigned>(PROP_CREATURE_LEVEL);
		GetSkillSystem().OnLevelUp(level);
	}

	if (nSex != nLastSex)
	{
		SetProperty<unsigned>(PROP_ACTOR_SEX, (unsigned)nSex);
	}
	
	//需要重新计算属性
	CollectOperate(CEntityOPCollector::coRefAbility);	

	//重新计算外观
	CollectOperate(CEntityOPCollector::coRefFeature);

	// 重置转职时间戳
	m_NextChangeVocTime = GetGlobalLogicEngine()->getMiniDateTime() + GetLogicServer()->GetDataProvider()->GetMiscConfig().GetChangeVocCfg().nChangeVocCD;

	// 重置设置
	m_GameSets.OnChangeVocation();
	//清除宠物
	GetPetSystem().RemoveAllPets();
	//成功通知
	CActorPacket pack;
	CDataPacket &data=this->AllocPacket(pack);
	data << (BYTE)(enDefaultEntitySystemID) << (BYTE)sChangeVocResult;
	data << (BYTE)0;
	pack.flush();
}

void CActor::ChangeSex(int nSex)
{
	if (nSex != MALEACTOR && nSex != FEMALEACTOR)
		return;

	int nLastSex = GetProperty<unsigned>(PROP_ACTOR_SEX);
	if (nSex == nLastSex)
		return;

	SetProperty<unsigned>(PROP_ACTOR_SEX, (unsigned)nSex);

	//需要重新计算属性
	//CollectOperate(CEntityOPCollector::coRefAbility);	
	//重新计算外观
	CollectOperate(CEntityOPCollector::coRefFeature);
	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
}

void CActor::RemoveAreaBuff( SCENEAREA* pArea )
{
	if (!pArea) return;
	if (pArea->attri[aaAddBuff].bType == aaAddBuff)
	{
		AreaAttri& attri = pArea->attri[aaAddBuff];
		if(!(attri.pValues)) return;
		CBuffSystem* bs = GetBuffSystem();
		for (INT_PTR i = 0; i < attri.nCount; i++)//删除N个buff
		{
			int nBuffId = attri.pValues[i];
			CBuffProvider::BUFFCONFIG * pConfig = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
			if (pConfig != NULL)
			{
				bs->Remove((GAMEATTRTYPE)pConfig->nType, pConfig->nGroup); //不限定次数的buff必须删掉，避免离开这个区域后这个buff就永远保留了
			}
		}
	} //区域增加的buff删除

	// 离开安全区 删除技能buff
	if (pArea->attri[aaSaft].bType == aaSaft)
	{
		AreaAttri& attri = pArea->attri[aaSaft];
		for (INT_PTR i = 0; i < attri.nCount; i++)
		{
			int nSkillID = attri.pValues[i];
			const OneSkillData * pSkillConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillData(nSkillID);
			const CSkillSubSystem::SKILLDATA * pSkillData = this->GetSkillSystem().GetSkillInfoPtr(nSkillID);
			if (pSkillConf && pSkillData)
			{
				const SKILLONELEVEL * pSLevelConf = GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillLevelData(nSkillID, pSkillData->nLevel);
				if (pSLevelConf)
				{
					for(INT_PTR rangeID=0; rangeID < pSLevelConf->pranges.count; rangeID++)
					{
						PSKILLONERANGE pRangeConf = pSLevelConf->pranges[rangeID];
						INT_PTR nResultCount = pRangeConf->skillResults.count;
						for(INT_PTR resultID = 0;  resultID < nResultCount; resultID ++)
						{
							PSKILLRESULT pSkillResult = &(pRangeConf->skillResults[resultID]);
							if (pSkillResult->nResultType == srSkillResultAddBuff)
							{
								if (pSkillResult)
								{
									this->GetBuffSystem()->RemoveById(pSkillResult->nId);
								}
							}
						}
					}
				}//end pSLevelConf
			}//end (pSkillConf && pSkillData)
		}//end for
	}//end aaSaft

	// 离开改变外观的区域，恢复
	if (pArea->attri[aaChangeModel].bType == aaChangeModel ||
		pArea->attri[aaChangeWeapon].bType == aaChangeWeapon )
	{
		this->CollectOperate(CEntityOPCollector::coRefFeature);
		//GetEquipmentSystem().RefreshAppear(); //刷外观
	}

	// 禁用的技能，恢复
	if (pArea->attri[aaNotSkillId].bType == aaNotSkillId)
	{
		AreaAttri * pAreaAttr = &pArea->attri[aaNotSkillId];
		for (size_t i = 0; i < pAreaAttr->nCount; i++)
		{
			int nSkillId = pAreaAttr->pValues[i];
			this->GetSkillSystem().SetSkillClose(nSkillId, false);
			CSkillSubSystem::SKILLDATA * pSkillData = this->GetSkillSystem().GetSkillInfoPtr(nSkillId);
			this->GetSkillSystem().NoticeOneSkillData(pSkillData);
		}
	}

	// 触发离开活动区域事件
	if (pArea->attri[aaActivity].bType == aaActivity)
	{
    	static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc();
        CScriptValueList paramList;
		AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
		for (size_t i = 0; i < pAreaAttr->nCount; i++)
		{
			int nAtvId = pAreaAttr->pValues[i];
			// 个人活动
			if(this->GetActivitySystem().IsActivityRunning(nAtvId))
			{
				if (PersonActivtyData* pActivty = this->GetActivitySystem().GetActivity(nAtvId))
				{
					paramList.clear();
					paramList << (int)CActivitySystem::enOnExitArea;
					paramList << (int)pActivty->nActivityType;
					paramList << (int)pActivty->nId;
					paramList << this;
					if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
					{
						OutputMsg(rmError,"[Personal Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
					}
					CrossActivityArea(1);
				}
			}
			// 全局活动
			else if(GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
			{
				if (GlobalActivityData* pActivty = GetGlobalLogicEngine()->GetActivityMgr().GetActivity(nAtvId))
				{
					paramList.clear();
                    paramList << (int)CActivitySystem::enOnExitArea;
                    paramList << (int)pActivty->nActivityType;
                    paramList << (int)pActivty->nId;
					paramList << this;
                    if (!globalNpc->GetScript().CallModule("ActivityDispatcher", "OnEvent", paramList, paramList))
                    {
                        OutputMsg(rmError,"[Global Activity] OnEvent错误 enOnExitArea，活动Type=%d, 活动ID=%d !",pActivty->nActivityType,pActivty->nId);
                    }
					CrossActivityArea(1);
				}
			}
		}
	}
	
	/* //区域删除的buff加回来
	if (pArea->attri[aaLeaveDelBuf].bType == aaLeaveDelBuf)
	{
		AreaAttri& attri = pArea->attri[aaLeaveDelBuf];
		if(!(attri.pValues)) return;
		CBuffSystem* bs = GetBuffSystem();
	
		for (INT_PTR i = 0; i < attri.nCount; i++)//增加N个buff
		{
			int nBuffId = attri.pValues[i];
			bs->Append(nBuffId);
			//CBuffProvider::BUFFCONFIG * pConfig = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nBuffId);
			//if (pConfig != NULL)
			//{
			//	bs->Remove((GAMEATTRTYPE)pConfig->nType, pConfig->nGroup);//不限定次数的buff必须删掉，避免离开这个区域后这个buff就永远保留了
			//}			
		}
	} */
}

void CActor::GetEnterFbPos( INT_PTR &nEnterFbScenceID,INT_PTR &nEnterFbPosx,INT_PTR &nEnterFbPosy )
{
	if (IsInRawServer())
	{
		nEnterFbScenceID = m_nEnterFbScenceID;
		nEnterFbPosx = m_nEnterFbPosx;
		nEnterFbPosy = m_nEnterFbPosy;
	}
	else
	{
		nEnterFbScenceID = m_commsrvPosBeforeEnterFB.nSceneId;
		nEnterFbPosx = m_commsrvPosBeforeEnterFB.nPosX;
		nEnterFbPosy = m_commsrvPosBeforeEnterFB.nPosY;
	}
}

void CActor::RemoveMyMessageBox()
{
	UINT myId = GetProperty<UINT>(PROP_ENTITY_ID);
	for (INT_PTR i = MessageBoxList->count()-1; i >= 0; i--)
	{
		MessageBoxItem& item = (*MessageBoxList)[i];
		if (item.nActorid == myId)
		{
			MessageBoxList->remove(i);
		}
	}
}

void CActor::InitActorCampContr()
{
	SetCamp(GetCampId());	
}


VOID CActor::SendOperateResult(bool flag, unsigned int nStep, unsigned int nSendPkgTick,bool bNeedBreakAction)
{
	CActorPacket pack;
	CDataPacket & data = AllocPacket(pack);
	data <<(BYTE)enDefaultEntitySystemID << (BYTE)sOperatorResult << (bool) flag << (BYTE)nStep;
	data << (BYTE) (bNeedBreakAction?1:0);
	
	bool bIncMoveAck = nSendPkgTick != 0 ? true : false;
	data << bIncMoveAck;
	if (bIncMoveAck)
	{
		int nGateDelay	= (int)(m_llRcvPacketTick - m_lGateTickCount);
		int nLogicDelay	= (int)(_getTickCount()- m_llRcvPacketTick);
		data << nSendPkgTick << nGateDelay << nLogicDelay;		
	}

	pack.flush();	
}

void CActor::ResetSpeedTime()
{
	//重新设置一下
	m_nCommOpTimes =0;
	m_nStartCommOpTime=0;
	m_nOccuCommOpTime =0;
	m_useQuickSoftTime =0;
	m_tOpsTimer.SetNextHitTime(GetGateNetWorkTickCount());
}
bool CActor::CheckAttackOpTick(bool bUseGateTime)
{	
	TICKCOUNT nTime ;
	if (!bUseGateTime)
	{
		nTime = _getTickCount();
	}
	else
	{
		nTime = m_lGateTickCount;
	}
	return m_tOpsTimer.Check(nTime);
}
void CActor::SetCommonOpNextTimeImpl(INT_PTR nNextTime,bool bSendResult2Client ,bool bResult, unsigned int nStep, unsigned int nSendPkgTick, bool bUseGateTime,bool bUseCheckCd,bool bNearAttackSuc)
{	
	TICKCOUNT nTime ;
	if (!bUseGateTime)
	{
		nTime = _getTickCount();
	}
	else
	{
		nTime =m_lGateTickCount;
	}
	m_tOpsTimer.SetNextHitTime(nTime + nNextTime);

	if (bSendResult2Client)
	{
		/*OutputMsg(rmWaning,_T("gateRcvTime=%lld, logicRcvTime=%lld, 下次的OP=%lld, curTime=%lld"), ((CActor *)this)->GetGateNetWorkTickCount(),
		((CActor *)this)->GetLastRcvPacketTick(), ((CActor *)this)->GetGateNetWorkTickCount()+nNextTime, GetLogicCurrTickCount() );*/
		SendOperateResult(bResult, nStep, nSendPkgTick);
	}

	if (bUseGateTime && bUseCheckCd)
	{
		TICKCOUNT nCurrTime = m_lGateTickCount;

		if (s_CheckCommOpTimes == m_nCommOpTimes )
		{
			TICKCOUNT nEllapse =(TICKCOUNT) ((nCurrTime - m_nStartCommOpTime) * (1+s_fKickQuickRate)  ) ;		
			if (nEllapse >0 && nEllapse <= (TICKCOUNT)(m_nOccuCommOpTime))
			{
				if(NULL == m_buffSystem.GetBuff(aMoveSpeedAdd,-1) )
				{
					LPCSTR sActorName = GetEntityName();
					if (!sActorName)
					{
						return;
					}
					char logbuf[64];
					sprintf(logbuf,"%d,%d,%d,%d,%d,%d,%d,%d",nNextTime,bSendResult2Client,bResult,nStep,nSendPkgTick,bUseGateTime,bUseCheckCd,bNearAttackSuc);

					CScene* pScene = GetScene();
					if (m_nLastSceneId != -1 && pScene && m_nLastSceneId == pScene->GetSceneId())
					{
						m_useQuickSoftTime ++;
						if(s_nKickQuickUserCount >= 0 ) //不处理
						{
							if(m_useQuickSoftTime > 5 && m_nLastPosX !=0 && m_nLastPosY != 0)
							{
								if (s_nQuickBuffId > 0) m_buffSystem.Append(s_nQuickBuffId);
								m_nQuickBuffTimes ++;
							}

							if(s_nCheckQuickCountKick > 0 && m_nQuickBuffTimes > s_nCheckQuickCountKick)
							{
								OutputMsg(rmWaning,"Actorname=%s, use speed tool, kickout",GetEntityName());
								GetCheckSpeedSystem().UseSpeedToolHandle(lwiUseQuickTool);
								return;
							}
						}
					}
				}
			}
			else
			{
				if (m_useQuickSoftTime > 0)
				{
					m_useQuickSoftTime--;
				}
			}
			m_nCommOpTimes		= 0;
			m_nOccuCommOpTime	= 0;
		}

		if (0 == m_nCommOpTimes)
		{
			m_nStartCommOpTime = nCurrTime;
			if (CScene* pScene = GetScene()) m_nLastSceneId = GetScene()->GetSceneId();
			else m_nLastSceneId = 0;
			m_nLastPosX = GetProperty<int>(PROP_ENTITY_POSX);
			m_nLastPosY = GetProperty<int>(PROP_ENTITY_POSY);
		}

		
		/*OutputMsg(rmWaning, _T("currOccuTime=%d, curNexTime=%d"), (int)m_nOccuCommOpTime, nNextTime);*/
		m_nOccuCommOpTime += nNextTime;
		m_nCommOpTimes++;
		
		if(CCheckSpeedSystem::m_bOpenVerifyFlag && bNearAttackSuc)
		{
			m_CkSpeedSystem.CheckUseSpeed(nTime, nNextTime);  //检测加速
		}
	}
}

void CActor::PlayerViewNotice( CEntity * pActor )
{
	if(!pActor)
		return;	
	const CVector<EntityHandleTag>& entityList = ((CAnimal*)this)->GetObserverSystem()->GetVisibleList();
	for (INT_PTR i = 0; i < entityList.count(); i++)
	{
		CEntity* pEntity = GetEntityFromHandle(entityList[i].m_handle);
		if (pEntity && pEntity->GetType() == enActor && pEntity == pActor)
		{
			SendOldTipmsgFormatWithId(tpPlayerViewYou, ttChatWindow, ((CActor*)pActor)->GetEntityName());
		}
	}
}


//能否传送
bool CActor::CanTransMit(int nDesServerId)
{
	if (!GetLogicServer()->IsStartCommonServer() && !GetLogicServer()->IsCommonServer())
	{
		OutputMsg(rmTip,  _T("%s 本服没有开启跨服"), __FUNCTION__);
		return false;
	}
	CFuBen *pFB = GetFuBen();
	if (pFB->IsFb())
	{
		SendOldTipmsgWithId(tpSceneCanNotCrossTransfer,ttFlyTip);
		OutputMsg(rmTip,  _T("%s 副本中禁止跨服传送，请先退出副本"), __FUNCTION__);
		return false;
	}

	if (enTS_IDLE != m_nTransmitStatus)
	{
		OutputMsg(rmTip, _T("%s error，当前已经在传送状态"), __FUNCTION__);
		return false;
	}

	if (GetLogicServer()->GetServerIndex() == nDesServerId || 0 == nDesServerId)
	{
		OutputMsg(rmTip, _T("%s error, 不正确的目标服务器Id=%d"), __FUNCTION__, nDesServerId);
		return false;
	}
	return true;
}
void CActor::RequestTransmitTo(int nDestServerId, enTransmitType transType)
{
	if(nDestServerId == GetLogicServer()->GetServerIndex()) 
	{
		OutputMsg(rmTip,"已经在%d服，不需要传送%d",(int) GetLogicServer()->GetServerIndex(),nDestServerId);
		return;
	}
	
	m_nTransmitStatus = enTS_QueryDestServer;
	CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
	CDataPacket &packet = pSSClient->allocProtoPacket(SessionServerProto::cRequestTransmit);
	packet << GetAccountID() << GetId() << nDestServerId << (int)transType;
	pSSClient->flushProtoPacket(packet);
	OutputMsg(rmTip, _T("发消息给Session查询传送目标服务器[Id=%d]是否准备好"), nDestServerId);
}

void CActor::OnRecvTransmitToAck(CDataPacketReader &inPacket)
{
	// 如果可以传送，nResult为0；否则不能传送
	if (enTS_QueryDestServer != m_nTransmitStatus)
	{
		OutputMsg(rmTip, _T("%s 当前不是查询目标服务器是否准备好的传送状态，接收到查询结果消息"), __FUNCTION__);
		return;
	}

	int nAccountId = 0, nDestServerId = 0, nType = 0;
	bool bCanTransmit = false;
	inPacket >> nAccountId >> nDestServerId >> bCanTransmit >> nType;
	enTransmitType tt = (enTransmitType)nType;
	OutputMsg(rmNormal, 
		_T("接收到查询目标服务器状态的结果[destServerId=%d, canTransmit=%d, transmitType=%d]"), 
		nDestServerId, 
		(int)bCanTransmit,
		nType);

	m_nTransmitStatus = enTS_IDLE;
	if (nDestServerId && bCanTransmit)
	{		
		if (enTransmitInGame == tt)
		{
			OutputMsg(rmTip, _T("目标服务器[destServerId=%d]准备好，开始传送前的存角色数据保存操作"), nDestServerId);			
			CloseActor(lwiRecvTransmit, false, nDestServerId, false, true);			
			m_nTransmitDestServerId = nDestServerId;
			m_nTransmitStatus = enTS_WaitSaveActorData;
		}
		else
		{	
			SendClientStartTransmit(nDestServerId);
			m_nTransmitStatus = enTS_DoTransmit;
			SendOldTipmsgWithId(tpIsCrossServering,ttFlyTip);

			OutputMsg(rmTip, _T("目标服务器[destServerId=%d]准备好，登录直接传送到目标服务器"), nDestServerId);
		}
	}
	else
	{
		if (enTransmitAtLogin == nType)
		{
			OutputMsg(rmTip, _T("目标服务器[destServerId=%d]没有准备好， 直接登录本服务器"), nDestServerId);
			RequestActorSubSystemDataImpl();
		}
		else
		{
			OutputMsg(rmTip, _T("目标服务器[destServerId=%d]没有准备好,跨服失败"), nDestServerId);
			SendOldTipmsgWithId(tpCrossServerIsClose,ttDialog);
		}
	}
}

void CActor::OnActorLogoutAck()
{
	if (enTS_WaitSaveActorData == m_nTransmitStatus && m_nTransmitDestServerId != 0)
	{
		m_nTransmitStatus = enTS_DoTransmit;
		SendClientStartTransmit(m_nTransmitDestServerId);	
		OutputMsg(rmTip, _T("完成传送前的角色数据存盘操作，开始传送服务器%d..."), m_nTransmitDestServerId);
		
	}
}

void CActor::SendClientStartTransmit(int nDestServerId)
{
	CActorPacket ap;
	CDataPacket& DataPacket = AllocPacket(ap);
	DataPacket << (BYTE)enDefaultEntitySystemID << (BYTE)sStartTransmit << nDestServerId;
	if(IsInRawServer() ) //如果在本服的话，就是传到跨服，否则要传到本服
	{
		DataPacket.writeString(GetLogicServer()->GetCommonServerAddr()?GetLogicServer()->GetCommonServerAddr():"");
		DataPacket << (int)(GetLogicServer()->GetCommonServerRandPort());
	}
	else
	{
		DataPacket.writeString("");
		DataPacket << (int)(0);
	}
	ap.flush();
	OutputMsg(rmTip, _T("通知客户端开始传送到目标服务器[serverid=%d]"), nDestServerId);
}

bool CActor::IsInRawServer() const
{
	if( m_nRawServerIndex ==0 )
	{
		OutputMsg(rmWaning,"%s,actorid=%d, m_nRawServerIndex=0 not inited ",__FUNCTION__,GetId());
		return true;
	}
	return true;
	// return GetLogicServer()->GetServerIndex() == m_nRawServerIndex ? true : false;
}

void CActor::SetAttackOthersFlag(bool bFlag)
{
	bool bState = false;

	if(GetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS) == 0)
		bState = true;

	SetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS,bFlag);

	if(bFlag == true)
	{
		CScene* pScene = GetScene();
		int x ,y;
		GetPosition(x,y);
		if (pScene)
		{
			//战斗专门区域
			if (pScene->HasMapAttribute(x, y, aaWar))
			{
				SetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS,0);
				return;
			}
		}
		if(bState)
			SendOldTipmsgWithId(tpOnPkStateNow,ttTipmsgWindow);
		m_AttackOthersTime =  GetGlobalLogicEngine()->getMiniDateTime();
	}
	else
	{
		m_AttackOthersTime = 0;
	}

	UpdateNameClr_OnPkModeChanged(GetHandle());
}


int CActor::GetNameColorData()
{
	int nPkValue = GetProperty<int>(PROP_ACTOR_PK_VALUE);

	if(GetProperty<int>(PROP_ACTOR_PK_VALUE) >= GetLogicServer()->GetDataProvider()->GetPkConfig().redName)
	{
		return 3;
	}

	if(GetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS) == 1)
	{
		return 2;
	}

	if(GetProperty<int>(PROP_ACTOR_PK_VALUE) >= 100)
	{
		return 1;
	}

	return 0;
}



int CActor::OnDeathDropItem()
{
	if (CanDeathNotPunish())
	{
		return 0;
	}

	int x,y;
	GetPosition(x,y);
	CScene * pScene = GetScene();
	if (pScene == NULL)
	{
		return 0;
	}
	SCENEAREA* pArea = pScene->GetAreaAttri(x,y);	//直接返回区域属性，后面的函数判断直接用这个,避免多次获取
	if(pArea == NULL) return 0;

	const CPkProvider &config = GetLogicServer()->GetDataProvider()->GetPkConfig();
	const DEATHDROPCONFIG & deathDrop = GetLogicServer()->GetDataProvider()->GetDeathDropRateConfig();
	UINT nRedPkValue = (UINT)config.redName;
	UINT nMyPkVal = GetProperty<UINT>(PROP_ACTOR_PK_VALUE);
	bool bNoDropEquip = false;
	if(pScene->HasMapAttribute(x, y, asNoDropEquip,pArea))
	{
		AreaAttri& attri = pArea->attri[asNoDropEquip];
		if (attri.nCount == 0)
		{
			bNoDropEquip = true;		//不掉物品
		}
	}
	bool boMapNoDropBag = false;	//区域是否不掉背包(只掉装备)
	if (pScene->HasMapAttribute(x, y, asNoDropBagCross, pArea ))
	{
		boMapNoDropBag = true;		  //不掉背包
	}

	// 被玩家杀死，且掉落装备的地图，将被爆 “狂暴”
	int nFrenzy = GetProperty<int>(PROP_ACTOR_FRENZY);
	EntityHandle handle = GetKillHandle();
	if( !bNoDropEquip && (nFrenzy > 0) && handle.GetType() == enActor )
	{
		// 沙巴克活动区域内，不掉落狂暴
		bool IsInSaBaKe = false;
		// if (pScene->HasMapAttribute(x,y,aaActivity,pArea))
		// {
		// 	AreaAttri * pAreaAttr = &pArea->attri[aaActivity];
		// 	for (size_t i = 0; i < pAreaAttr->nCount; i++)
		// 	{
		// 		int nAtvId = pAreaAttr->pValues[i];
		// 		if (nAtvId == 13 && 
		// 			GetGlobalLogicEngine()->GetActivityMgr().IsRunning(nAtvId))
		// 		{
		// 			IsInSaBaKe = true;
		// 		}
		// 	}
		// }
		if ((pScene->GetSceneId() == 3) || (pScene->GetSceneId() == 47))
		{
			if (GetGlobalLogicEngine()->GetActivityMgr().IsRunning(13))
			{
				IsInSaBaKe = true;
			}
		}

		if (!IsInSaBaKe)
		{
			if (CEntity *pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(handle))
			{
				// 自己删除狂暴状态
				SetProperty<int>(PROP_ACTOR_FRENZY,0);
				GetPropertySystem().ResertProperty(); //重置属性

				// 击杀者加邮件
				FrenzyCfg const& cfg = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetFrenzyCfg();
				unsigned int nId = pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID);
				CMailSystem::MAILATTACH mailAttach[MAILATTACHCOUNT];
				for (size_t i = 0; (i < MAILATTACHCOUNT) && (i < cfg.vecKillAward.size()); i++)
				{
					std::tie
					(
						mailAttach[i].nType,
						mailAttach[i].item.wItemId,
						mailAttach[i].item.wCount
					)
					= cfg.vecKillAward[i];
				}
				CMailSystem::SendMail(nId, cfg.vecKillerTitleHead.c_str(), cfg.vecKillerTitleContent.c_str(), mailAttach);
			}
		}
	}
	

	const int nRatio = 10000;
	
	s_itemDropPtrList->clear();
	s_itemDropTypeList->clear();

	INT_PTR nAllItemCount = GetBagSystem().GetAllItemCount();
	int fBagDropCoefficient = 0;
	int fEquipDropCoefficient = 0;
	
	const CStdItemProvider &provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();

	byte nVipDropRate = 0;
	static int nVipRatio = 100;
	if (nMyPkVal < nRedPkValue)		//普通玩家,200要读取配置
	{
		fBagDropCoefficient = deathDrop.nWriteNameBagDropRate;
		fEquipDropCoefficient = deathDrop.nWriteNameEquipDropRate;
	}
	else
	{						//红名玩家
		fBagDropCoefficient = deathDrop.nRedNameBagDropRate;
		fEquipDropCoefficient = deathDrop.nRedNameEqiupRate;
	}

	//爆背包物品,且地图可以掉背包物品
	if(!boMapNoDropBag)
	{
		for (INT_PTR i = 0; i < GetBagSystem().count(); i++)
		{
			CUserItem * pUserIt = GetBagSystem().GetItemByIdx(i);			//背包物品
			if (pUserIt)					//未绑定掉落，不可交易的消失
			{
				const CStdItem* pItem = provider.GetStdItem(pUserIt->wItemId);
				if (!pItem || (pItem->m_Flags.denyDropdown || pItem->m_nPackageType != 0))
					continue;
				int nRand = (rand() % nRatio) + 1;
				if(nRand < fBagDropCoefficient)
				{
					s_itemDropPtrList->add(pUserIt);
					s_itemDropTypeList->add(0);  //是背包
				}
			}
		}
	}

	if(s_itemDropPtrList->count() >0)
	{
		//处理背包物品的掉落
		OnCreateDropBox( *s_itemDropPtrList,*s_itemDropTypeList);
	} 

	if(bNoDropEquip)	//目前没有必掉的装备，需要再添加
	{
		return 0;
	}

	//装备爆出，如果地图不掉装备，前面就直接return
	const CEquipVessel *pEquips = NULL;
	bool bFlag = false;
	CUserItem *pUserItem = NULL;
	pEquips = &(m_Equiptment);
	
	if (pEquips != NULL)
	{
		INT_PTR nEquipRateCount = 0;
		s_itemDropPtrList->clear();
		s_itemDropTypeList->clear();
		for (INT_PTR equipPos = itMaxEquipPos - 1; equipPos >=0 ; equipPos--)
		{
			if(equipPos == itDecorationPos || (equipPos >= itEquipDiamondPos && equipPos <= itBlackSandalwoodStonePos)) //勋章 神装不掉落 内功装备
				continue;
			if(nEquipRateCount >= deathDrop.nMaxEquipDropNum)
				break;//已达上限
			CUserItem * pEquipIt = pEquips->m_Items[equipPos];			//身上装备
			if (pEquipIt)
			{
				const CStdItem* pItem = provider.GetStdItem(pEquipIt->wItemId);
				if (!pItem || pItem->m_Flags.denyDropdown)
				{
					continue;
				}


				if (wrand(nRatio) < fEquipDropCoefficient)
				{
					if(pItem->m_nDeathsplit.size() > 0)
					{
						int nSize = pItem->m_nDeathsplit.size();
						for(int i = 0; i < nSize; i++)
						{
							if(pItem->m_nDeathsplit[i].nItemId != 0)
							{
								pUserItem = GetGlobalLogicEngine()->AllocUserItem(true); //申请一个道具
								if(pUserItem ==NULL) continue;
								{
									pUserItem->wItemId =pItem->m_nDeathsplit[i].nItemId ;
									pUserItem->wCount = pItem->m_nDeathsplit[i].nCount;
									pUserItem->btQuality = pItem->b_showQuality;
									pUserItem->wPackageType = pItem->m_nPackageType;
								}
							}
							
							if(!bFlag)
							{
								LPCSTR sSender = GetScene() != NULL ? GetScene()->GetSceneName():GetEntityName();	
								bool boResult= GetEquipmentSystem().DeleteEquip(equipPos, sSender, GameLog::Log_DeathDropItem, false);
								if(boResult)
								{
									if(pUserItem)
									{
										GetEquipmentSystem().TakeOn(pUserItem);
										bFlag = true;
									}
								}	
							}
							else
							{
								if(pUserItem)
								{
									s_itemDropPtrList->add(pUserItem);
									s_itemDropTypeList->add(2); //类型是物品
									nEquipRateCount++;
								}
								
							}
						}
					}
					else
					{
						s_itemDropPtrList->add(pEquipIt);
						s_itemDropTypeList->add(1); //类型是玩家的装备
						nEquipRateCount++;
					}
					
				}
			}
		}
		if(s_itemDropPtrList->count() >0)
		{
			LPTSTR sTitle = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeathDropMailTitle);
			std::string mailContext = "";
			char text[1024];
			LPTSTR sformat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeathDropMail);
			sprintf_s(text, sizeof(text),sformat, (char*)(pScene->GetSceneName()) );
			mailContext += text;

			unsigned int nId = GetProperty<unsigned int>(PROP_ENTITY_ID);

			mailContext += "\n";
			char sBagMsg[1024] = {0};
			OnCreateDropBox( *s_itemDropPtrList,*s_itemDropTypeList, sBagMsg);
			mailContext += sBagMsg;	
			CMailSystem::SendMail(nId, sTitle, mailContext.c_str());
		}
	}

	return 0;
}

/*
vDropIt：候选掉落物品的列表，随机抽取nLoopTimes个物品，产生实际掉落
*/
bool CActor::OnCreateDropBox(CVector<CUserItem*> & vDropIt, CVector<BYTE> &equipos, LPSTR sMsg)
{
	// LPSTR msG = dynamic_cast<LPSTR>(sMsg);
	std::string msG = "";
	INT_PTR nCount = vDropIt.count();
	if(nCount <=0) return false;
	
	const CStdItemProvider &provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	LPTSTR sFormat = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmDeathDropEqMail);
	for (int nDropI = 0; nDropI < nCount; nDropI++)
	{
		CUserItem * pRemItem = vDropIt[nDropI];
		
		if (pRemItem)
		{
			int nRemItemId = pRemItem->wItemId;
			INT_PTR nEquiptPos =  equipos[nDropI]; //装备的位置


			bool boResult = false;

			int nPosX,nPosY; //位置
			WORD nItemCount = pRemItem->wCount;
			GetPosition(nPosX,nPosY); //获取坐标
			if(nEquiptPos == 0)		//背包物品
			{
				LPCSTR sSender = GetScene() != NULL ? GetScene()->GetSceneName():GetEntityName();				
				boResult =( GetBagSystem().DeleteItem(pRemItem, nItemCount, sSender, GameLog::Log_DeathDropItem, false) > 0);

			}
			else if(nEquiptPos ==1 || nEquiptPos == 2) //玩家装备
			{
				if(nEquiptPos == 1) {//装备
					INT_PTR nIndex = GetEquipmentSystem().FindIndex(pRemItem->series);
					if(nIndex <0) continue; 

					LPCSTR sSender = GetScene() != NULL ? GetScene()->GetSceneName():GetEntityName();	
					boResult= GetEquipmentSystem().DeleteEquip((int)nIndex, sSender, GameLog::Log_DeathDropItem, false);
				}
				else {
					boResult = true;
				}
				
				const CStdItem* pItem = provider.GetStdItem(pRemItem->wItemId);
				if(pItem) {

					if(nDropI > 0) {
						LPTSTR point = GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmPoint);
						msG += point;
					}
					char text[1024] = {0};
					LPTSTR name = CUserEquipment::getEquipName(pItem);
					LPTSTR desc = CUserEquipment::getEquipNameDesc(pItem);
					if(sFormat && name && desc) {
						char descname[100] = {0};
						sprintf_s(descname, sizeof(descname), desc,pItem->m_sName);

						if(Item::itEquipDiamond == (Item::ItemType)pItem->m_btType)
							sprintf_s(text, sizeof(text), name);
						else sprintf_s(text, sizeof(text), sFormat, name, descname);

						msG += text;
					}
					
				}
			}
			else
			{
				boResult = true;
			}
			
			
			
			if (!boResult)			//如果扣物品失败了，避免流通
			{
				unsigned int nId = GetProperty<unsigned int>(PROP_ENTITY_ID);
				OutputMsg(rmError, _T("玩家[%s,%u] 死亡爆出物品[%d,%lld]失败,pos:%d"), GetEntityName(), nId, (int)pRemItem->wItemId,pRemItem->series,(int)nEquiptPos);
				continue;
			}
			
			pRemItem->wCount = nItemCount;
			// const CStdItem* pItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nRemItemId);
			// if (pItem && pItem->m_nDropBroadcast != 0 && (pItem->m_nDropBroadcast == -1 || pItem->m_nDropBroadcast >= GetLogicServer()->GetDaysSinceOpenServer()))
			// {
			// 	CScene * pScene = GetScene();
			// 	if (pScene != NULL)
			// 	{
			// 		GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tpDropPlayerBroad, ttScreenCenter+ttChatWindow, GetEntityName(),
			// 			pScene->GetSceneName(),CUserBag::GetItemLink(nRemItemId, pRemItem), nPosX, nPosY);
			// 	}
			// }

			if(pRemItem->btFlag != ufDenyDeal)//不可交易的不掉地上
			{
				//掉落的装备极品属性去掉
				if(nEquiptPos ==1)
				{
					memset(pRemItem->cBestAttr, 0, sizeof(pRemItem->cBestAttr));
					memset(pRemItem->Refining, 0, sizeof(pRemItem->Refining));
				}
				
				CDropItemEntity *pDropItem= CDropItemEntity::CreateDropItem(GetScene(),nPosX,nPosY,GameLog::clLootDropBox,GetEntityName()); //
				if(pDropItem != NULL)
				{
					pDropItem->SetItem(pRemItem);
					pDropItem->DealDropItemTimeInfo(this, 1);
				}
				else
				{
					if (pRemItem->wCount <= 0)
					{
						GetGlobalLogicEngine()->DestroyUserItem(pRemItem);
					}
				}
			}
			else
			{
				// if(pEntity != NULL)
				// {
				// 	DEATHDROPITEM stDropitem;
				// 	stDropitem.nActorId  = GetId();
				// 	stDropitem.nKillerId = ((CActor*)pEntity)->GetId();
				// 	stDropitem.nKillTime = GetGlobalLogicEngine()->getMiniDateTime();
				// 	stDropitem.nItemId	 = pRemItem->wItemId;
				// 	stDropitem.nItemCount= pRemItem->wCount;
				// 	GetFriendSystem().AddOnDeathItem(stDropitem);
				// }
			}
		}
	}
	if(msG != "" && sMsg)
		strcat(sMsg, msG.c_str());
	return true;
}

bool CActor::SetActorCircle(int nCircle)
{
	if (nCircle < 0) return false;

	SetProperty<unsigned int>(PROP_ACTOR_CIRCLE, (unsigned int)nCircle);
	if (GetTeam())
	{
		GetTeam()->BroadcastUpdateMember(this,PROP_ACTOR_CIRCLE);
	}
	//需要重新计算属性
	CollectOperate(CEntityOPCollector::coRefAbility);	
	if(!IsDeath())
		CollectOperate(CEntityOPCollector::coResetMaxHPMP);
	

	//记录日志
	Uint64 nActorExp = GetProperty<Uint64>(PROP_ACTOR_EXP);
	int nLevel = GetProperty<unsigned int>(PROP_CREATURE_LEVEL);
	char buff[128];
	sprintf_s(buff,sizeof(buff),_T("%d-%d-%lld"),nCircle,nLevel,nActorExp);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	m_NewTitleSystem.OnNewTitleEvent(mtEventCircle);
	m_achieveSystem.ActorAchievementComplete(nAchieveCircle, nCircle);
	m_questSystem.OnQuestEvent(CQuestData::qtcircle, nCircle);
	//通知任务系统
    // m_questSystem.OnQuestEvent(CQuestData::qtRichCircle,0,nCircle,FALSE,this);
	m_activitySystem.OnCircleUp();

	//触发脚本
	CScriptValueList  paramList;
	CScriptValueList  retParamList;
	//((CActor*)this)->InitEventScriptVaueList( paramList,(int)aeCircle );
	paramList << nCircle;
	((CActor*)this)->OnEvent( aeCircle,paramList,retParamList );


	/////////////////////////////////////////
	return true;
}

bool CActor::SetHeadTitle(INT_PTR nTitleId, bool bFlag)
{
	
	//先屏蔽
	if (true)
	{
		return true;
	}
	/*
	if (nTitleId < 0 || nTitleId > MAX_HEADTITLE_COUNT) return false;
	
	unsigned int nOldProp = GetProperty<unsigned int>(PROP_ACTOR_HEAD_TITLE);
	unsigned int nTempProp = nOldProp;
	if (bFlag)
	{
		//设置为当前的头衔 按位计算 属性不存盘
		nOldProp |= (0x1 << (nTitleId));
	}
	else
	{
		//删除为当前的头衔 按位计算 属性不存盘
		nOldProp &= ~(0x1 << (nTitleId));
	}
	SetProperty<unsigned int>(PROP_ACTOR_HEAD_TITLE, nOldProp);

	if (nOldProp != nTempProp)	//发生改变则刷属性
	{
		//设置角色刷新属性的标记
		CollectOperate(CEntityOPCollector::coRefAbility);
	}
	
	return true;
	*/
}

void CActor::SetSceneAreaFlag(int nFlag)
{
	int oldSceneFlag = m_nEnterSceneFlag;

	m_nEnterSceneFlag = nFlag;

	if (oldSceneFlag != nFlag)
	{
		if (nFlag == scfGuildSiege || oldSceneFlag == scfGuildSiege)
		{
			GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this,"OnGuildSiegeSceneFlagChange");
			UpdateNameClr_OnPkModeChanged(GetHandle());
		}
	
	}
}
//nLevelConfigId->tagOpenLevel 
bool CActor::CheckOpenLevel(int nLevelConfigId)
{
	CMiscProvider& miscProvider = GetLogicServer()->GetDataProvider()->GetMiscConfig();
	POPENLEVELCONFIG pOpenLevel = miscProvider.GetOpenLevel(nLevelConfigId);
	if (pOpenLevel)
	{
		return CheckLevel(pOpenLevel->level, pOpenLevel->circle);
	}
	return false;
}

/*
bCircleOnly为false时，需要同时判断转数和等级
*/
bool CActor::CheckLevel( int nMinLevel, int nMinCircle, bool bCircleOnly )
{
	int	nCurCircle = GetProperty<int>(PROP_ACTOR_CIRCLE);
	int	nCurLevel  = GetProperty<int>(PROP_CREATURE_LEVEL);
	if (nMinCircle > 0)
	{
		if (nCurCircle < nMinCircle)		//小于要求转数，直接false
		{
			return false;
		}
	}
	if( bCircleOnly && nCurCircle > nMinCircle )	//仅判断转数且转数大于要求，直接true
	{
		return true;
	}

	if (nCurLevel < nMinLevel)
	{
		return false;
	}
	return true;
}

bool CActor::CheckGuildLevel( int nMinLevel)
{
	int	nCurRecharge  = GetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL);
	if (nCurRecharge < nMinLevel)
	{
		return false;
	}
	return true;
}

/*
充值额度
*/
bool CActor::CheckRecharge( int nMinRecharge)
{
	int	nCurRecharge  = GetProperty<unsigned int>(PROP_ACTOR_DRAW_YB_COUNT);
	if (nCurRecharge < nMinRecharge)
	{
		return false;
	}
	return true;
}


/*
官职等级
*/
bool CActor::CheckOffice( int nofficeId)
{
	if (GetProperty<int>(PROP_ACTOR_OFFICE) < nofficeId)
	{
		return false;
	}
	return true;
}

/*
神魔等级
*/
bool CActor::CheckGhostLevel( int nGhostLevel )
{
	if (GetGhostSystem().GetSumLv() < nGhostLevel)
	{
		return false;
	}
	return true;
}

/*校验玩家曾经达到过的最大等级转数是否满足条件，转数优先
*/
bool CActor::CheckMaxCircleAndLevel(int nLevel, int nCircle)
{
	int nMaxCircle = 0;
	int nMaxLevel  = 0;
	GetMaxCircleAndLevel(nMaxCircle, nMaxLevel);
	if( nMaxCircle < nCircle )
	{
		return false;
	}
	else if( nMaxCircle > nCircle )
	{
		return true;
	}
	else
	{
		return nMaxLevel >= nLevel;
	}
}

int CActor::GetLevel()
{
	int	nActorLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
	return nActorLevel;
}

int CActor::GetCircleLevel()
{
	int	nActorLevel = GetProperty<int>(PROP_ACTOR_CIRCLE);
	return nActorLevel;
}

void CActor::AddSceneEffectToList(int nEffectId,int nSceneId,int nSec)
{
	ACTORSRCEFFECT tmpEffect;
	tmpEffect.nEffctId = nEffectId;
	tmpEffect.nSceneId = nSceneId;
	tmpEffect.nTime = GetGlobalLogicEngine()->getMiniDateTime() + nSec;
	m_SceneEffect.add(tmpEffect);
}

void CActor::DeleteSceneEffect(int nEffectId)
{
	CActorPacket ap;
	AllocPacket(ap);
	ap << (BYTE)enMiscSystemID << (BYTE)sDeleteSceneEffect<< (int)nEffectId;
	ap.flush();	
}

void CActor::NeedBroadSceneEffect(int nSceneId,int nLastSceneId)
{
	int curTime = GetGlobalLogicEngine()->getMiniDateTime();
	INT_PTR nCount = m_SceneEffect.count();

	for(INT_PTR i=0;i<nCount;i++)
	{
		ACTORSRCEFFECT& tmpEffect = m_SceneEffect[i];

		if(tmpEffect.nSceneId == nLastSceneId && tmpEffect.nTime < curTime)	//需要删除特效
		{
			DeleteSceneEffect(tmpEffect.nEffctId);
		}
		else if(tmpEffect.nSceneId == nSceneId && tmpEffect.nTime > curTime)  //需要添加特效
		{
			CActorPacket ap;
			CDataPacket& data = AllocPacket(ap);
			data << (BYTE)enMiscSystemID << (BYTE)sScrEffectCode << (WORD)tmpEffect.nEffctId;
			data << (int)(tmpEffect.nTime - curTime);
			data << (int)0;
			data << (int)0;
			ap.flush();	
		}
	}
}

bool CActor::CanTelport(int nSceneId, LPCSTR sSceneName, int nX, int nY)
{
	if (HasState(esStateStall))
	{
		SendOldTipmsgWithId(tpInStallState, ttFlyTip);
		return false;
	}
	//static int nPrisonSId = GetLogicServer()->GetDataProvider()->GetPkConfig().m_nPrisonSceneId;
	//if(GetSceneID() == nPrisonSId)
	//{
	//	SendOldTipmsgWithId(tpYouInPrison, ttFlyTip);
	//	return false;
	//}
	if(HasMapAttribute(aaNotTransfer))
	{
		SendOldTipmsgWithId(tpNoTranScene, ttFlyTip);
		return false;
	}
	return true;
}

void CActor::OnNpcTalk(const EntityHandle & hNpcHandle, short nFuncId, CDataPacket &pack)
{
	DECLARE_TIME_PROF("CActor::OnNpcTalk");

	//取得npc的指针
	CEntity* pEntity = NULL;
	bool boGlobal = false;
	if (hNpcHandle == 0)
	{
		pEntity = GetGlobalLogicEngine()->GetGlobalNpc();
		boGlobal = true;
	}
	else
		pEntity = GetGlobalLogicEngine()->GetEntityMgr()->GetEntity(hNpcHandle);
	if (pEntity != NULL)
	{
		SetTarget(pEntity->GetHandle());
		//只有npc才可以对话
		if (!pEntity->isNPC() )
		{
			OutputMsg(rmTip,_T("incorrect network data in NpcTalk 111"));
			return;
		}
		if (!boGlobal)
		{
			//检验数据和坐标，必须在规定范围以内
			if (pEntity->GetFuBen() != GetFuBen() || pEntity->GetScene() != GetScene())
			{
				return;
			}
			int actX,actY,npcX,npcY;
			GetPosition(actX,actY);
			pEntity->GetPosition(npcX,npcY);
			if (npcX > (actX+MOVE_GRID_COL_RADIO) || npcX < (actX-MOVE_GRID_COL_RADIO) 
				|| npcY > (actY+MOVE_GRID_ROW_RADIO) || npcY < (actY-MOVE_GRID_ROW_RADIO))
			{
				//OutputMsg(rmTip,_T("incorrect range in NpcTalk"));
				return;
			}
		}

		ClickNpc((CNpc *)pEntity, nFuncId, pack);
	}
}

void CActor::ClickNpc(CNpc * pNpc, short nFuncId, CDataPacket &pack)
{
	if (!pNpc || !this)
	{
		return;
	}
	if ( !m_isInited || IsDestory() )
	{
		OutputMsg(rmTip,_T("clicknpc error by destroy."));
	}
	else
	{
		if ( !IsInSameScreen(pNpc) && pNpc->GetId() != 0 )
		{
			OutputMsg(rmTip,_T("%s(%u,%d)clicknpc not in same screen."),GetEntityName(),GetId(),pNpc->GetId());
		}
	}
	
	if (unsigned int nNpcId = pNpc->GetId())
	{
		CScriptValueList paramList, retParamList;
		paramList << this;
		paramList << *(double*)(&pNpc->GetHandle());
		paramList << (int)nNpcId;
		paramList << (int)nFuncId;
		paramList << &pack;
		if (!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().FunctionExists("OnNpcEvent"))
		{
			return;
		}
		if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("OnNpcEvent", paramList, retParamList, 1))
		{
			const RefString &s = GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().getLastErrorDesc();
			if (paramList.count() >= 3)
			{
				CScriptValue& sValue = paramList[2];
				if (sValue.getType() == CScriptValue::vString)
				{
					SendOldTipmsgFormatWithId(tpOnNpcEventMsg,ttDialog,(LPCTSTR)s,(LPCTSTR)sValue);
				}else SendTipmsg((LPCTSTR)s,ttDialog);
			}
			else
			{
				SendTipmsg((LPCTSTR)s,ttDialog);
			}
			return;
		}
	}

	//unsigned int nNpcId = pNpc->GetId();
	// if ( nNpcId > 0 )		//非全局NPC，
	// {
	// 	NPCCONFIG* pNpcConfig = pNpc->GetConfig();
	// 	if ( !pNpcConfig )
	// 	{
	// 		return;
	// 	}
	//
	// 	if( pNpcConfig->nNoMain == 3 )	//渡魔任务NPC
	// 	{
	// 		CQuestProvider::FLUSHQUEST &flushQuest = GetLogicServer()->GetDataProvider()->GetQuestConfig()->GetFlushQuest();
	// 		if( flushQuest.nBookQuestLevel > 0 && CheckLevel(flushQuest.nBookQuestLevel, 0) )
	// 		{
	// 			GetQuestSystem()->SendBookQuest(flushQuest.nBookQuestId, CQuestData::qeotOpenUi);
	// 		}
	// 		else
	// 		{
	// 			SendOldTipmsgFormatWithId(tpBookQuestCommitLevelLimit, ttFlyTip, flushQuest.nBookQuestLevel);
	// 		}
	// 		return;
	// 	}
	// 	else if( pNpcConfig->nNoMain == 8 )		//赏金任务NPC
	// 	{
	// 		GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this,"OpenRewardQuestPanel");
	// 		return;
	// 	}
	// 	else if( pNpcConfig->nNoMain == dlMaterialFuBenPanel )		//材料副本面板
	// 	{
	// 		GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this,"OpenMaterialFubenPanel");
	// 		return;
	// 	}
	// 	else if (pNpcConfig->nNoMain > 0 )
	// 	{
	// 		//noMain 如果没有特殊指定填写窗口id
	// 		OpenDialog(pNpcConfig->nNoMain);
	// 		return;
	// 	}
	// }
	//
	//int nLen = 0;
	//CQuestSystem::m_sBuf[0] = 0;
	// if (sFuncName[0] == 0)
	// {
	// 	int nQuestCount = 0, nQid = 0, nState = 0;		
	// 	nLen = GetQuestSystem()->FormatQuestState(pNpc,CQuestSystem::m_sBuf,sizeof(CQuestSystem::m_sBuf),nQuestCount,nQid,nState);
	// 	if (nQuestCount == 1 && nState != 2) //只有一个，直接显示，就不读取npc其他选项了
	// 	{
	// 		CQuestSystem::m_sBuf[0] = nLen = 0;
	// 		paramList << (LPCTSTR)"ShowQuestTalk";
	// 		paramList << nQid << nState;
	// 	}
	// 	else
	// 	{
	// 		paramList << (LPCTSTR)"main";
	// 		paramList << nNpcId;
	// 	}
	// }
	// else
	// {
	// 	size_t nFunctionLen = strlen(sFuncName);
	// 	if ( nFunctionLen > MAXFUNCTIONLENGTH )
	// 	{
	// 		OutputMsg(rmWaning,"npc function length too long. npcid:%u",nNpcId);
	// 		return;
	// 	}
	//
	// 	CHAR sFnArgs[1024];
	// 	sFnArgs[0]=0;
	// 	//char sDestStr[1024] = {0};
	// 	//GetGlobalLogicEngine()->GetEncryptFuncMgr().DecryptFunctionName((char*)sFuncName,sDestStr, GetRundomKey());
	// 	//pNpc->FuncParamProcess(sDestStr,sFnArgs,sizeof(sFnArgs),paramList, true);	
	// 	pNpc->FuncParamProcess(sFuncName,sFnArgs,sizeof(sFnArgs),paramList, true);
	// }
	//
	// if(!GetGlobalLogicEngine()->GetScriptNpc()->GetScript().Call("OnNpcEvent", paramList, retParamList, 1))
	// {
	// 	const RefString &s = GetGlobalLogicEngine()->GetScriptNpc()->GetScript().getLastErrorDesc();
	// 	if (paramList.count() >= 3)
	// 	{
	// 		CScriptValue& sValue = paramList[2];
	// 		if (sValue.getType() == CScriptValue::vString)
	// 		{
	// 			SendOldTipmsgFormatWithId(tpOnNpcEventMsg,ttDialog,(LPCTSTR)s,(LPCTSTR)sValue);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		SendTipmsg((LPCTSTR)s,ttDialog);
	// 	}
	// 	return;
	// }
	// if (!m_boCallScript)
	// {
	// 	m_boCallScript = true;
	// 	return;
	// }
	// if (retParamList.count() > 0 )
	// {
	// 	CScriptValue & val = retParamList[0];
	// 	if (val.getType() == CScriptValue::vString)
	// 	{
	// 		sprintf_s(CQuestSystem::m_sBuf + nLen, sizeof(CQuestSystem::m_sBuf) - nLen, "%s", (LPCTSTR)val);
	// 	}			
	// }
	//
	// if (*CQuestSystem::m_sBuf)
	// {
	// 	CActorPacket ap;
	// 	CDataPacket &pack = AllocPacket(ap);
	// 	pack << (BYTE)enDefaultEntitySystemID;
	// 	pack << (BYTE)sNpcTalk;
	// 	pack << (BYTE)1;
	// 	pack << pNpc->GetHandle();
	// 	//pack << (LPCSTR)GetGlobalLogicEngine()->GetEncryptFuncMgr().EncryptExtractString(CQuestSystem::m_sBuf, GetRundomKey());
	// 	pack << (LPCSTR)CQuestSystem::m_sBuf;
	// 	ap.flush();
	// }
}

void CActor::SetFashionBattle( unsigned int nVal )
{
	m_nFashionBattle = nVal;
}

bool CActor::ChangeCircleSoul( int nValue )
{
	int nOldValue = GetProperty< int>(PROP_ACTOR_CIRCLE_SOUL);
	int nNewValue = nValue + nOldValue;
	if (nNewValue < 0)	//不能低于0，无最大限制
	{
		nNewValue = 0;
	}
	// //发消息失去了灵魄
	// if(nValue >0)
	// {
	// 	SendOldTipmsgFormatWithId(tpGetCircleSoul,ttTipmsgWindow,(int)nValue);
	// }
	// else 
	// {
	// 	SendOldTipmsgFormatWithId(tmCircleSoul，,ttTipmsgWindow,(int)-nValue);
	// }

	std::string sTemp = nValue > 0 ?"+":"";
	sTemp = intToString(nValue, sTemp);

	SendTipmsgFormatWithId(tmCircleSoul, tstEcomeny,sTemp.c_str());
	SetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL,(unsigned int)nNewValue);

	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	return true;
}

//改变活跃度
bool CActor::ChangeActivity( int nValue )
{
	int nOldValue = GetProperty< int>(PROP_ACTOR_ACTIVITY);
	int nNewValue = nValue + nOldValue;
	if (nNewValue < 0)	//不能低于0，无最大限制
	{
		nNewValue = 0;
	}

	SetProperty<unsigned int>(PROP_ACTOR_ACTIVITY,(unsigned int)nNewValue);
	if( nValue > 0 )
	{
		SendOldTipmsgFormatWithId(tpAddAcitivity,ttTipmsgWindow,(int)nValue);
	}
	else 
	{
		SendOldTipmsgFormatWithId(tpLoseAcitivity,ttTipmsgWindow,(int)-nValue);
	}
	//记录日志
	if(GetLogicServer()->GetLocalClient())
	{
	}
	return true;
}




//改变声望
bool CActor::ChangePrestige( int nValue )
{
	int nOldValue = GetProperty< unsigned int>(PROP_ACTOR_Prestige);
	int nNewValue = nValue + nOldValue;
	if (nNewValue < 0)	//不能低于0，无最大限制
	{
		nNewValue = 0;
	}

	SetProperty<unsigned int>(PROP_ACTOR_Prestige,(unsigned int)nNewValue);

	//std::string sTemp = nValue > 0 ? "+":"";
	//sTemp = intToString(nValue, sTemp);
	//SendTipmsgFormatWithId(tmAddInegral,tstEcomeny,sTemp.c_str());

	return true;
}


//改变积分
bool CActor::ChangeActorRecyclePonit( int nValue )
{
	int nOldValue = GetProperty< int>(PROP_ACTOR_RECYCLE_POINTS);
	int nNewValue = nValue + nOldValue;
	if (nNewValue < 0)	//不能低于0，无最大限制
	{
		nNewValue = 0;
	}

	SetProperty<unsigned int>(PROP_ACTOR_RECYCLE_POINTS,(unsigned int)nNewValue);
	std::string sTemp = nValue > 0 ? "+":"";
	sTemp = intToString(nValue, sTemp);

	SendTipmsgFormatWithId(tmAddInegral,tstEcomeny,sTemp.c_str());

	return true;
}

INT_PTR CActor::DelItemById(int wItemId,int btCount,int btQuality/* =-1 */,int btStrong/* =-1 */,
	int btBindFlag /* = -1 */, char * sComment/* =NULL */,int nLogID/* =2 */,const int nDuraOdds /* = -1 */,bool bIncEquipBar /* = false */)
{
	CUserItemContainer::ItemOPParam ItemPara;
	ItemPara.wItemId = wItemId;
	ItemPara.btQuality  =btQuality ;
	ItemPara.btStrong = btStrong;
	ItemPara.wCount =(WORD)btCount;
	ItemPara.btBindFlag = btBindFlag;
	LPCSTR sLogStr = sComment ? sComment : _T("Script");

	const CStdItem *pStdItem =GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(wItemId);
	if(pStdItem ==NULL) return 0;

	int nCount = 0;
	if (bIncEquipBar && pStdItem->m_btType < Item::itEquipMax)
	{
		nCount += (int)(GetEquipmentSystem().DeleteEquip(ItemPara,sLogStr,nLogID,nDuraOdds));
	}
	ItemPara.wCount -= (WORD)nCount;
	if (ItemPara.wCount > 0)
	{
		nCount += (int)(GetBagSystem().DeleteItem(ItemPara,sLogStr,nLogID,nDuraOdds));
	}

	return nCount;
}

LPCSTR CActor::GetFiveAttrDesc(BYTE btType)
{
	INT_PTR nTipMsgId =0;
	switch(btType)
	{
		case enFiveAttrGold:	nTipMsgId = tpFiveAttrNameGold; break;
		case enFiveAttrWood:	nTipMsgId = tpFiveAttrNameWood; break;
		case enFiveAttrWater:	nTipMsgId = tpFiveAttrNameWater; break;
		case enFiveAttrFire:	nTipMsgId = tpFiveAttrNameFire; break;
		case enFiveAttrEarth:	nTipMsgId = tpFiveAttrNameEarth; break;
	}
	if(nTipMsgId)
	{
		return GetLogicServer()->GetDataProvider()->GetOldTipmsgConfig().GetTipmsg(nTipMsgId);
	}
	else
	{
		return NULL;
	}
}


void CActor::SendShortCutMsg(const char* szMsg)
{
	CActorPacket pack;
	CDataPacket & data = AllocPacket(pack);
	data << (BYTE)enDefaultEntitySystemID <<(BYTE) sSendShortCutMsg ;
	data.writeString(szMsg);
	pack.flush();
}


void CActor::SendMoneyNotEnough(BYTE btType, int nCount)
{
	SendAwardNotEnough((BYTE)GetAwardTypeByMoneyType(btType),0, nCount);
}
void CActor::SendAwardNotEnough(BYTE btType, WORD wItemId, int nCount)
{
	CActorPacket pack;
	CDataPacket & data = AllocPacket(pack);
	data << (BYTE)enDefaultEntitySystemID <<(BYTE) sNotEnoughAward ;
	data << btType << wItemId << nCount;
	pack.flush();
}

void CActor::ChangeRecordData(RecordType btValue)
{
	unsigned int nValue = 0;
	int nPropId = 0;
	switch (btValue)
	{
		case rRecordOnLineTime:		nPropId = PROP_ACTOR_TOTAL_ONLINE_MIN; break;
		case rRecordBeKilled:		nPropId = PROP_ACTOR_BE_KILLED_COUNT; break;
		case rRecordKillMonster:	nPropId = PROP_ACTOR_KILL_MONSTER_COUNT; break;
	}
	if (nPropId <= 0)
	{
		return;
	}
	nValue = GetProperty<unsigned int>(nPropId);
	if (nValue == 0xFFFFFFFF)
	{
		return;
	}
	SetProperty<unsigned int>(nPropId,nValue+1);
}

/*
扣除绑定元宝，不足用元宝替代
返回值：扣除的元宝数量
>=0:扣除成功
-1：扣除失败

*/
INT_PTR CActor::DeleteBindYuanbaoAndYuanbao(INT_PTR nValue,WORD wLogId, int nCount,LPCSTR sComment,bool needLog, bool boForceLog,bool bNotice)
{
	unsigned int nBindYuanbao	= GetProperty<unsigned int>(PROP_ACTOR_BIND_YUANBAO);
	unsigned int nYuanbao		= GetProperty<unsigned int>(PROP_ACTOR_YUANBAO);
	INT_PTR nDelBindYuanbao = 0;
	INT_PTR nDelYuanbao = 0;
	if( nValue < 0 )
	{
		return -1;
	}
	//绑定元宝是否小于扣的数
	if( nBindYuanbao < nValue )
	{
		if( nBindYuanbao + nYuanbao < nValue )
		{
			return -1;
		}
		else
		{
			nDelBindYuanbao = nBindYuanbao;
			nDelYuanbao		= nValue - nDelBindYuanbao;
		}
	}
	else
	{
		nDelBindYuanbao = nValue;
	}

	if( nDelBindYuanbao > 0 )
	{
		if( !ChangeMoney(mtBindYuanbao,-nDelBindYuanbao,wLogId,nCount,sComment,needLog,boForceLog,bNotice) )
		{
			return -1;
		}
	}

	if( nDelYuanbao > 0 )
	{
		if( !ChangeMoney(mtYuanbao,-nDelYuanbao,wLogId,nCount,sComment,needLog,boForceLog,bNotice) )
		{
			return -1;
		}
	}

	return nDelYuanbao;
}



void CActor::OnDeathDropExp()
{
	if (CanDeathNotPunish())
	{
		return;
	}

	if (GetProperty<int>(PROP_CREATURE_HP) <= 0)
	{
		GLOBALCONFIG &globalConfig = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		if (globalConfig.nDeathDropExpTotalRate > 0) //是否开启死亡经验
		{
			UINT nRedPkValue = (UINT)GetLogicServer()->GetDataProvider()->GetPkConfig().redName;
			UINT nMyPkVal = GetProperty<UINT>(PROP_ACTOR_PK_VALUE);

			Uint64 nExp = GetProperty<Uint64>(PROP_ACTOR_EXP);
			INT_PTR nSubExp = 0;
			if (nMyPkVal < nRedPkValue) //普通玩家
			{
				nSubExp = (INT_PTR)(globalConfig.nWriteNameExpDropRate * nExp / globalConfig.nDeathDropExpTotalRate);
				if (nSubExp > globalConfig.nWriteNameExpDropMax)
				{
					nSubExp = globalConfig.nWriteNameExpDropMax;
				}
			}
			else //红名
			{
				nSubExp = (INT_PTR)(globalConfig.nRedNameExpDropRate * nExp / globalConfig.nDeathDropExpTotalRate);
				if (nSubExp > globalConfig.nWriteNameExpDropMax)
				{
					nSubExp = globalConfig.nRedNameExpDropMax;
				}
			}

			if (nSubExp <= 0) return;

			AddExp(-nSubExp, GameLog::clDeathDropExp);		
		}
	}
}

bool CActor::CanDeathNotPunish()
{
	bool bResult = false;
	int x,y;
	GetPosition(x,y);
	CScene * pScene = GetScene();
	if (pScene == NULL)
	{
		return bResult;
	}
	SCENEAREA* pArea = pScene->GetAreaAttri(x,y);	
	if(pArea == NULL) 
	{
		return bResult;
	}

	GLOBALCONFIG &config = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
	if(pScene->HasMapAttribute(x, y, aaNotDeathPunish,pArea))
	{
		bResult = true;
	}

	return bResult;
}

int CActor::GetStaticVarValue(LPCTSTR szVarName)
{
	CCLVariant& pVarMgr	= GetActorVar();
	int nValue = 0;
	CCLVariant* pVar = pVarMgr.get(szVarName);
	if( pVar != NULL )
	{
		nValue = (int)(double)*pVar;
	}
	return nValue;
}

void CActor::SetStaticVarValue(LPCTSTR szVarName, int nValue)
{
	CCLVariant& pVarMgr	= GetActorVar();
	pVarMgr.set(szVarName, nValue);
}

/*通用传送：
（nType, nValue）对应（0，13）协议的（传送类型，传送类型值）
*/
void CActor::CommonTelePort(unsigned int nType, unsigned int nValue, unsigned char nDir)
{
	switch(nType)
	{
	case enQuestTelePort:
		{
			// if( nDir == 1 )
			// {
			// 	GetQuestSystem()->QuestTelport(nValue);		//此时，这里是任务ID
			// }
			// else
			// {
			// 	GetQuestSystem()->TelportToQuestNpc(nValue);//此时，这里是任务ID
			// }
		}
		break;
	case enRecommendedUpgrade:
		{
			CNpc* pNpc = GetGlobalLogicEngine()->GetGlobalNpc();
			if (pNpc == NULL) return;
			CScriptValueList paramList, retParamList;
			paramList << this << nValue << nDir;
			pNpc->GetScript().Call("RecommendedUpgrade",paramList,retParamList);
		}
		break;
	}
}

// buff被删
void CActor::TriggerBuffRemovedEvent(int nBuffId) 
{
	// 触发我要复仇事件
	CScriptValueList paramList;
	//InitEventScriptVaueList(paramList, aeBuffRemoved); //
	paramList << nBuffId;
	OnEvent(aeBuffRemoved,paramList,paramList);
}

bool CActor::IsNightFighting()
{
	int nScenceId = GetSceneID();
	if(nScenceId == 79)
	{
		return true;
	}
	return false;
}


bool CActor::IsBadMan()
{
	if (GetProperty<UINT>(PROP_ACTOR_PK_VALUE) >= (UINT)GetLogicServer()->GetDataProvider()->GetPkConfig().redName)
	{
		return true;
	}
	if (GetProperty<UINT>(PROP_ACTOR_EVIL_PK_STATUS))
	{
		return true;
	}
	return false;
}


void CActor::AddDailyActivityDoneNum(int nActivityId, int nAddNum)
{
	GetGlobalLogicEngine()->GetMiscMgr().DoScriptFunction(this, "AddDailyActivityDoneNum", nActivityId, nAddNum );
}

void CActor::GetMaxCircleAndLevel(int &nCircle, int &nLevel)
{
	CCLVariant& pVarMgr	= GetActorVar();
	CCLVariant* pVar1 = pVarMgr.get(szMaxActorCircle);
	CCLVariant* pVar2 = pVarMgr.get(szMaxActorLevel);
	if( pVar1 != NULL )
	{
		nCircle = (int)(double)*pVar1;
	}
	else
	{
		nCircle = GetProperty<int>(PROP_ACTOR_CIRCLE);
	}
	if( pVar2 != NULL )
	{
		nLevel = (int)(double)*pVar2;
	}
	else
	{
		nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
	}
}

/*获取玩家拥有的奖励类型的数量
包括货币、装备、物品
*/
unsigned int CActor::GetAwardTypeCount( int nAwardType, INT_PTR nId, bool bBindAndUnBind )
{	
	switch( nAwardType )
	{
	case qatEquipment:		//装备、道具
		return (unsigned int)GetBagSystem().GetItemCount( nId );
		break;
	case qatBindMoney:		//ZGame的金币
		return m_property.nBindCoin;
		break;
	case qatMoney:			//不使用
		return m_property.nNonBindCoin;
		break;
	case qatBindYb:			//绑定元宝
		if( bBindAndUnBind )	//绑元+元宝一起计算
		{
			return m_property.nBindYuanbao+m_property.nNonBindYuanbao;
		}
		else
		{
			return m_property.nBindYuanbao;
		}
		break;
	case qatYuanbao:
		return m_property.nNonBindYuanbao;
		break;
	case qatCombatScore:
		return (unsigned int)m_CombatSystem.GetCombatScore();
		break;
	default: 
		return 0;
	}
}

/*
获取玩家的攻击力，区分职业
*/
unsigned int CActor::GetActorAttackValueMax()
{
	int nJob = GetProperty<int>(PROP_ACTOR_VOCATION);			//玩家职业
	enPropCreature ValueType = PROP_CREATURE_PHYSICAL_ATTACK_MAX;
	if( nJob == enVocMagician )	//法师
	{
		ValueType = PROP_CREATURE_MAGIC_ATTACK_MAX;
	}
	if( nJob == enVocWizard )	//道士
	{
		ValueType = PROP_CREATURE_WIZARD_ATTACK_MAX;
	}
	unsigned int nAttackValue = GetProperty<unsigned int>(ValueType);	//攻击力（分职业）
	return nAttackValue;
}

unsigned int CActor::GetActorDefenceValueMax()
{
	int nJob = GetProperty<int>(PROP_ACTOR_VOCATION);			//玩家职业
	enPropCreature ValueType = PROP_CREATURE_PYSICAL_DEFENCE_MAX;
	if( nJob == enVocMagician || nJob == enVocWizard)	//法师, 道士
	{
		ValueType = PROP_CREATURE_MAGIC_DEFENCE_MAX;
	}

	unsigned int nAttackValue = GetProperty<unsigned int>(ValueType);	//防御力（分职业）
	return nAttackValue;
}

/*
从属性库中随机生成装备属性，并将属性发送给客户端
nSmithId：属性库 SmithXXX.txt文件
nFlag: 1-生成鉴定属性（预留，暂时没有使用）
*/
void CActor::GenerateItemAttrsInSmith( CUserItem* pUserItem, int nSmithId, int nAttrNum, int nLockTypes[], int nFlag, unsigned char nJob )
{
	if( nAttrNum > CUserItem::MaxSmithAttrCount )
	{
		return;
	}

	CStdItemProvider& provider = GetLogicServer()->GetDataProvider()->GetStdItemProvider();
	if( nSmithId <= 0 )
	{
		return ;
	}

	if( nFlag == enGenerateItemAttr_SmithAttr )		//[ZGame]生成鉴定属性
	{
		RandAttrSelector::GenerateItemSmithAttrsInSmith( pUserItem, nSmithId, nAttrNum, nLockTypes, nJob);
	}
	CUserBag& bag = GetBagSystem();
	bag.NotifyItemChange( pUserItem );		//将物品属性发送到客户端
}

/*
装备属性转移
nFlag：1-鉴定属性转移
其他暂时没有
*/
bool CActor::TransferItemAttrs( CUserItem* pSrcUserItem, CUserItem* pDesUserItem, int nFlag )
{
	if( pSrcUserItem == pDesUserItem )			//同一件装备
	{
		return false;
	}

	CUserBag& bag = GetBagSystem();
	bag.TransferItemIdentifyAttrs( pSrcUserItem, pDesUserItem );
	bag.NotifyItemChange( pSrcUserItem );		//将物品属性发送到客户端
	bag.NotifyItemChange( pDesUserItem );		//将物品属性发送到客户端
	return true;
}


bool CActor::GiveDropAwardWithRet(int nDropGroupId, int nLogId,LPCTSTR logstring, std::vector<DROPGOODS> &RetDropInfo )
{
	std::vector<DROPGOODS> dropInfos;
	GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(nDropGroupId, dropInfos);
	RetDropInfo = dropInfos ;
	int count  = dropInfos.size();
	if(count <= 0)
	{
		return false;
	}
	else
	{
		for(int i = 0; i < count; i++)
		{
			CUserItemContainer::ItemOPParam iParam;
			iParam.wItemId = dropInfos[i].info.nId;
			iParam.wCount = (WORD)dropInfos[i].info.nCount;
			iParam.btQuality =1;
			iParam.btStrong = 0;
			iParam.btBindFlag = 0;
			if(dropInfos[i].info.nType  == 0)
			{
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropInfos[i].info.nId);
				if(!pStdItem)
					continue;
				iParam.btQuality = pStdItem->b_showQuality;
				int num = dropInfos[i].v_bestAttr.size();

				for(int j = 0; j < num; j++)
				{
					char buf[10];
					if(j != 0)
						strcat(iParam.cBestAttr, "|");

					DropAttribute data = dropInfos[i].v_bestAttr[j];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(iParam.cBestAttr, buf);
				}
				iParam.cBestAttr[sizeof(iParam.cBestAttr)-1]= '\0';	
			}
			GiveAward(dropInfos[i].info.nType, dropInfos[i].info.nId, dropInfos[i].info.nCount,0,0,0,0, nLogId,logstring,0, &iParam);
			
		}
	}
	return true;
}


bool CActor::GiveDropAward(int nDropGroupId, int nLogId, bool nSend2Client )
{
	std::vector<DROPGOODS> dropInfos;
	GetLogicServer()->GetDataProvider()->GetDropCfg().GetGiftDropInfoByDropGroupId(nDropGroupId, dropInfos);
	int count  = dropInfos.size();
	char sMsgFormat[1024] = {0};
	if(count <= 0)
	{
		return false;
	}
	else
	{
		for(int i = 0; i < count; i++)
		{
			CUserItemContainer::ItemOPParam iParam;
			iParam.wItemId = dropInfos[i].info.nId;
			iParam.wCount = (WORD)dropInfos[i].info.nCount;
			iParam.btQuality =1;
			iParam.btStrong = 0;
			iParam.btBindFlag = 0;
			if(dropInfos[i].info.nType  == 0)
			{
				const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(dropInfos[i].info.nId);
				if(!pStdItem)
					continue;
				iParam.btQuality = pStdItem->b_showQuality;
				int num = dropInfos[i].v_bestAttr.size();

				for(int j = 0; j < num; j++)
				{
					char buf[10];
					if(j != 0)
						strcat(iParam.cBestAttr, "|");

					DropAttribute data = dropInfos[i].v_bestAttr[j];
					sprintf(buf, "%d,%d", data.nType, data.nValue);
					strcat(iParam.cBestAttr, buf);
				}
				iParam.cBestAttr[sizeof(iParam.cBestAttr)-1]= '\0';	
			}
			if(nSend2Client)
			{
				char msg[24] = {0};
				sprintf(msg, "%d\\\\%d\\\\%d", dropInfos[i].info.nType, dropInfos[i].info.nId,dropInfos[i].info.nCount);
				if(strlen(sMsgFormat))
					strcat(sMsgFormat, "\\\\");
				strcat(sMsgFormat, msg);
			}
			bool success = GiveAward(dropInfos[i].info.nType, dropInfos[i].info.nId, dropInfos[i].info.nCount,0,0,0,0, nLogId,"",0, &iParam);
			if(!success)
				return success;
			// if(YuanBaoFlag){
			// 	GiveAward(dropInfos[i].info.nType, dropInfos[i].info.nId, dropInfos[i].info.nCount,0,0,0,0,nLogId,"DAOJU_ADD_YUANBAO",0, &iParam);
			// }else{
			// 	GiveAward(dropInfos[i].info.nType, dropInfos[i].info.nId, dropInfos[i].info.nCount,0,0,0,0,0,"",0, &iParam);
			// }
			
		}
		if(nSend2Client && strlen(sMsgFormat))
			SendTipmsg(sMsgFormat, tstDropWindows);
	}
	return true;
}

//检查玩家资源是否满足 --为了满足可以弹tips
bool CActor::CheckActorSource(int nType, int nId, int nCount, int nTipmsgType)
{
	int nSourceValue = 0;
	switch(nType)
	{
	case qatEquipment: //装备特殊处理一下
		{
			nSourceValue = GetBagSystem().GetItemCount(nId);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
				{
					const CStdItem* pStdItem = GetLogicServer()->GetDataProvider()->GetStdItemProvider().GetStdItem(nId);
					if(pStdItem)
						SendTipmsgFormatWithId(tmSelfItemNotEnough, nTipmsgType, pStdItem->m_sName);
				}
				return false;
			}
			
			return true;
		}
	case qatMoney: //金币
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_COIN);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNoMoreCoin, tstCoinNotEnough);
				return false;
			}
			
			return true;
		}
	case qatBindMoney:	//绑金
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_BIND_COIN);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNoMoreBindCoin, tstBindCoinNotEnough);
				return false;
			}
			
			return true;
		}
	case qatBindYb: //绑元宝
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_BIND_YUANBAO);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNomoreBindYubao, tstBindYbNotEnough);
				return false;
			}
			
			return true;
		}
	case qatWarPoint: //战令积分
	{
		if(nId == 0) // 0计数器类型表示 是时间
			return false;
		nSourceValue = GetStaticCountSystem().GetStaticCount(nOrderWard_StaticType);//计数器
		if(nSourceValue < nCount)
		{
			if(nTipmsgType)
				SendTipmsgFormatWithId(tmWarPointNoEnough, nTipmsgType);
			return false;
		}
		return true;;
	}
	case qatZLMoney: //战令币
	{
		if(nId == 0) // 0计数器类型表示 是时间
			return false;
		nSourceValue = GetStaticCountSystem().GetStaticCount(eSTATIC_TYPE_ORDERWARD_MONEY);//计数器
		if(nSourceValue < nCount)
		{
			if(nTipmsgType)
				SendTipmsgFormatWithId(tmZLMoneyNoEnough, nTipmsgType);
			return false;
		}
		return true;;
	}

	case qatYuanbao:// 元宝
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_YUANBAO);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNomoreYubao, tstYbNotEnough);
				return false;
			}
			
			return true;
		}
	case qatExp:  //经验
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_EXP);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmExpLimited, nTipmsgType);
				return false;
			}

			return true;
		}
	case qatCircleSoul: //转生修为
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_CIRCLE_SOUL);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmCircleSoulNotEnough, tstUI);
				return false;
			}
			
			return true;
		}
	case qatFlyShoes: // 飞鞋
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_FLYSHOES);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmflyshoesNotEnough, nTipmsgType);
				return false;
			}
			
			return true;
		}
	case qatBroat: //喇叭
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_BROATNUM);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmBoratNotEnough, nTipmsgType);
				return false;
			}
			
			return true;
		}
	case qaIntegral://回收积分
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_RECYCLE_POINTS);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmPointNotEnough, nTipmsgType);
				return false;
			}
				
			return true;
		}
	case qaGuildDonate://行会贡献
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_GUILDEXP);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmGuildDonateNotEnough, nTipmsgType);
				return false;
			}
				
			return true;
		}
	case qatPrestigeNum://声望
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_Prestige);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNoPrestigeNum, nTipmsgType);
				return false;
			}
				
			return true;
		}
	case qatActivity://活跃度	
		{
			nSourceValue = GetProperty<unsigned int>(PROP_ACTOR_ACTIVITY);
			if(nSourceValue < nCount)
			{
				if(nTipmsgType)
					SendTipmsgFormatWithId(tmNoActivityValue, nTipmsgType);
				return false;
			}
				
			return true;
		}
	// case qatMultiExpUnused://剩余未使用多倍经验
	// 	{
	// 		long long nSourceValue = GetProperty<long long>(PROP_ACTOR_MULTI_EXP_UNUSED);
	// 		if(nSourceValue < nCount)
	// 		{
	// 			if(nTipmsgType)
	// 				SendTipmsgFormatWithId(tmGuildDonateNotEnough, nTipmsgType);
	// 			return false;
	// 		}
				
	// 		return true;
	// 	}
	default: 
		{
			return true;//自定义奖励，不需要这里处理
		}
	}

	return false;
}



bool CActor::CheckTargetDistance(CActor *pTaget, int nDistance)
{
	if (!pTaget)
		return false;
	if(IsInited() == false || pTaget->IsInited() ==false) return false;

	int nSelf_X = 0, nSelf_Y = 0, nTarget_X = 0, nTarget_Y = 0;
	GetPosition(nSelf_X, nSelf_Y);
	pTaget->GetPosition(nTarget_X, nTarget_Y);
	if ((nDistance != 0 ) && (GetPosInfo().pScene != pTaget->GetPosInfo().pScene ||
	 abs(nSelf_X - nTarget_X) > nDistance || 
	 abs(nSelf_Y - nTarget_Y) > nDistance ) )
	{
		return false;
	}

	return true;
}



bool CActor::SetActorBless(int nBlessValue)
{
	if (nBlessValue < 0) return false;

	SetProperty<unsigned int>(PROP_ACTOR_Bless, (unsigned int)nBlessValue);

	//需要重新计算属性
	CollectOperate(CEntityOPCollector::coRefAbility);	
	// 更新升级后的血、蓝
	CBlessCfg& cfg = GetLogicServer()->GetDataProvider()->GetBlessCfg();
	BlessCfg* pBlessCfg = cfg.GetBlessCfg(nBlessValue);
	if(pBlessCfg)
	{
		m_achieveSystem.ActorAchievementComplete(nAchieveBless, pBlessCfg->nLevel);
		m_questSystem.OnQuestEvent(CQuestData::qtBless, pBlessCfg->nLevel);
	}
	return true;
}


void CActor::DealActorJDTime()
{
	int nMapGroup = 0;
	CScene* pScene = GetScene();
	if(pScene) {
		SCENECONFIG* pdata = pScene->GetSceneData();
		if(pdata)
			nMapGroup = pdata->nMapGgroup;
	}

	if((m_sJoinMap.m_nMapGroup == 0 || nMapGroup != m_sJoinMap.m_nMapGroup )&&
	(m_sJoinMap.m_nJDJoinMapId != 0 && GetSceneID() != m_sJoinMap.m_nJDJoinMapId ))
	{
		// m_sJoinMap.m_nJDJoinMapId = 0;
		// m_sJoinMap.m_nJDJoinTime = 0;
		memset(&m_sJoinMap, 0 ,sizeof(0));
	}

	if(m_sJoinMap.m_nJDJoinTime != 0 && time(NULL) >= m_sJoinMap.m_nJDJoinTime)
	{
		int x = 0, y = 0, nSceneId = 0;
		// GetNotReloadMapPos(nSceneId, x, y);
		// FlyShoseToStaticScene(nSceneId, x, y,10);
		ReturnCity();
		memset(&m_sJoinMap, 0 ,sizeof(0));
		// m_nJDJoinMapId = 0;
		// m_nJDJoinTime = 0;
	}
}


void CActor::DeathCancelBossBeLong(CActor* pActor)
{
	if(blongLists.count() > 0)
	{
		for(int i = 0; i < blongLists.count(); i++)
		{
			BeLongBoss& it = blongLists[i];
			GetGlobalLogicEngine()->GetBossMgr().BossCallLuaScript(it.nBossId, CBossSystem::enOnCancelBL, pActor);
		}
		//清空掉
		blongLists.empty();
	}
}

void CActor::SetBeLongBoss(int nBossId, int nSceneId)
{
	if(!nBossId || !nSceneId)
		return; 
	for(int i = 0; i < blongLists.count(); i++)
	{
		BeLongBoss& it = blongLists[i];
		if(it.nBossId == nBossId)
			return;
	}
	BeLongBoss it;
	it.nBossId = nBossId;
	it.nSceneId = nSceneId;
	blongLists.add(it);
}

void CActor::SelfCancelBossBeLong()
{
	blongLists.empty();
}


void CActor::CrossActivityArea(BYTE nState)
{
	CActorPacket ap;
	CDataPacket& data = AllocPacket(ap);
	data<<(BYTE)enMiscSystemID << (BYTE)sCrossAtvArea <<nState;
	ap.flush();
}
//泡点经验
void CActor::AddPaoDianExp(int nType, int nAtvId)
{
	CScene* pScene = GetScene();
	int x ,y;
	GetPosition(x,y);
	if (pScene) {
		SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
		if ((pArea && pScene->HasMapAttribute(x, y, aaPaodianExp)) || nAtvId) {
			
			if(!nAtvId) {
				AreaAttri& attri = pArea->attri[aaPaodianExp];
				if (attri.nCount > 0)
				{
					nType = attri.pValues[0] ;
				}
			}
			
			HookExpConfig& cfg = GetLogicServer()->GetDataProvider()->GethookExpCfg();
			if(cfg.m_HookMaps.find(nType) != cfg.m_HookMaps.end()) {
				//首充卡&&色卡会员 泡点plus

				int nAddExp =  cfg.m_HookMaps[nType].nBasePdValue + ( cfg.m_HookMaps[nType].nPdPercent/10000.0) * GetLevel();
				// int nPlusNum = GetMiscSystem().GetSuperRightAtvPaoDianPlus() ; 
				// if(nPlusNum > 0 ) {
				// 	nAddExp =nAddExp * (100+nPlusNum)/100.0 ;
				// } 
				AddExp(nAddExp, GameLog::Log_PaoDian,nAtvId);
			}
			
		}
	}
}

//check
bool CActor::checkGetMaxDouExp()
{
	Uint64 nNowexp =GetProperty<Uint64>(PROP_ACTOR_MULTI_EXP_UNUSED);
	int nMaxexp = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMaxDoubleExpValue;
	if(nNowexp >= nMaxexp)
	{
		return true;
	}
	return false;
}

//泡点经验
void CActor::AreaCostCoin()
{
	CScene* pScene = GetScene();
	int x ,y;
	GetPosition(x,y);
	if (pScene) {
		SCENEAREA* pArea = pScene->GetAreaAttri(x,y);
		if ((pArea && pScene->HasMapAttribute(x, y, aaCostCoin))) {
			int nCostCoin = 0;
			AreaAttri& attri = pArea->attri[aaCostCoin];
			if (attri.nCount > 0)
			{
				nCostCoin = attri.pValues[0] ;
			}
			int nNowValue =  GetMoneyCount(mtBindCoin);
			if(nCostCoin > nNowValue)
				nCostCoin = nNowValue;
			if(nCostCoin > 0)
				ChangeMoney(mtBindCoin,-nCostCoin,GameLog::Log_Area,0,"", true);
			if( GetMoneyCount(mtBindCoin) <= 0) {
				SendTipmsgFormatWithId(tmNoBindCoin2City, tstUI);
				ReturnCity();
			}
				
		}
	}
}


void CActor::SendRankTips()
{
	int nJob = GetJob();
	unsigned int nActorId = GetId();
	int nRankId = enRankBaseBegin;
	static int nMaxRankTipId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nMaxRankTipId;
	CRanking* pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankId+nJob);
	if (pRank)
	{
		int nIndex = pRank->GetIndexFromId(nActorId);
		if(nIndex >= 0 && nIndex <= nMaxRankTipId) {
			LPCTSTR arg1 = GetRankIndexTipByJob(nJob, 1);
			LPCTSTR arg2 = GetRankIndexTipByIndex(nIndex);
			if(arg1 == nullptr || arg2 == nullptr) return;
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmRankTips,tstExclusive,arg2,arg1,(char*)(GetEntityName()));
		}

	}
	nRankId = enMoBaiRankList;
	pRank = GetGlobalLogicEngine()->GetRankingMgr().GetRanking(nRankId+nJob);
	if (pRank)
	{
		int nIndex = pRank->GetIndexFromId(nActorId);
		if(nIndex >= 0 && nIndex <= nMaxRankTipId) {
			LPCTSTR arg1 = GetRankIndexTipByJob(nJob, 2);
			LPCTSTR arg2 = GetRankIndexTipByIndex(nIndex);
			if(arg1 == nullptr || arg2 == nullptr) return;
			GetGlobalLogicEngine()->GetEntityMgr()->BroadTipmsgWithParams(tmRankTips,tstExclusive,arg2,arg1,(char*)(GetEntityName()));
			// SendTipmsgFormatWithId(tmRankTips,tstExclusive,arg2,arg1,(char*)(GetEntityName()));
		}

	}
}

char* CActor::GetRankIndexTipByIndex(int nIndex)
{
	switch(nIndex)
	{
		case 0: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum1);
		case 1: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum2);
		case 2: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum3);
		case 3: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum4);
		case 4: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum5);
		case 5: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum6);
		case 6: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum7);
		case 7: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum8);
		case 8: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum9);
		case 9: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmRankNum10);
		default:
		break;
	}
	return nullptr;
}


char* CActor::GetRankIndexTipByJob(int nJob, int nType)
{
	//等级排行
	if(nType == 1)
	{
		switch(nJob)
		{
			case 1: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorJob1);
			case 2: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorJob2);
			case 3: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorJob3);
			default:
			break;
		}
	}else //战力排行
	{
		switch(nJob)
		{
			case 1: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorPower1);
			case 2: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorPower2);
			case 3: return GetLogicServer()->GetDataProvider()->GetTipmsgConfig().GetTipmsg(tmActorPower3);
			default:
			break;
		}
	}
	
	
	return nullptr;
}


void CActor::UpdateUseDeopt() 
{
	WarehouseCfg& cfg = GetLogicServer()->GetDataProvider()->GetStdItemProvider().m_WarehouseCfg;
	if(!CheckLevel(cfg.nOpenLevel, 0))
		return;
	int nCardLv = GetMiscSystem().GetMaxColorCardLevel();
	if(nCardLv < cfg.nOpenCardLv)
		return;
	int nMaxGrid = GetProperty<int>(PROP_ACTOR_DEPORT_GRID_COUNT);

	if(nCardLv >= cfg.v_warehouses.size())
		nCardLv = cfg.v_warehouses.size() -1;
	int nCount = nMaxGrid;
	if(nCardLv < 0) return;
	if(nCount < cfg.nInitial)
		nCount = cfg.nInitial;
	
	if(nCount < cfg.v_warehouses[nCardLv])
		nCount = cfg.v_warehouses[nCardLv];
	if(nCount <= nMaxGrid)	return;
	m_deportSystem.AddDepot(nCount); //更改格子的数量
}


void CActor::CheckCombineMail()
{
	if(IsInited() == false) return;
	int nCombineTime = GetStaticCountSystem().GetStaticCount(nACTOR_MAIL_COMBINETIME);
	if(!nCombineTime) {
		GetStaticCountSystem().SetStaticCount(nACTOR_MAIL_COMBINETIME,GetCreateTime());
		nCombineTime = GetCreateTime();
	}
	// OutputMsg(rmTip,_T("GetCreateTime() = %u"),GetCreateTime());
	CMiniDateTime combineTime = GetLogicServer()->GetServerCombineTime();
	if(combineTime.tv > 0) {

		CMiniDateTime openTime  = GetLogicServer()->GetServerOpenTime();
		if((nCombineTime < combineTime) && (nCombineTime > openTime)) {
			int nDay = (nCombineTime- openTime)/(24*3600);
			// OutputMsg(rmTip,_T("CheckCombineMail, actorID=%u,nCombineTime=%d,openTime=%d,day=%d"),GetId(),nCombineTime,openTime.tv,nDay);
			if(nDay > 0)
			{ 
				auto mails = GetLogicServer()->GetDataProvider()->GetMailConfig().getCombineMials();
				if(mails.size() <= 0) return;
				for(auto mail : mails) {
					for(auto id :mail.second) 
					{ 
						auto cfg = GetLogicServer()->GetDataProvider()->GetMailConfig().getOneMailCfg(id);
						if(cfg) 
						{
							CVector<ACTORAWARD> awardlist;
							for(int i =0; (i < cfg->vAwards.size()) && (i < MAILATTACHCOUNT); i++) {

								ACTORAWARD actorAward = cfg->vAwards[i];
								if (actorAward.wId ==0) {
									continue;
								}
								actorAward.wCount = actorAward.wCount * nDay;
								awardlist.add(actorAward);
							}
							// OutputMsg(rmTip,_T("GetCombineMail, actorID=%u"),GetId());
							CMailSystem::SendMailByAward(GetId(), cfg->cTitle, cfg->cContent, awardlist);
						}
					}
				}
				GetStaticCountSystem().SetStaticCount(nACTOR_MAIL_COMBINETIME,openTime.tv);
			}
		}
	} 
}

int CActor::getTotalOnlineTime()
{
	int nOnlineTime = GetGlobalLogicEngine()->getMiniDateTime() -  m_nAtvTotalOnlineTime.tv;//在线时间
	if(nOnlineTime < 0)
	{
		nOnlineTime =0;
	}
	m_nAtvTotalOnlineTime = GetGlobalLogicEngine()->getMiniDateTime();
	
	return nOnlineTime; 
}


void CActor::SendScriptTotalOnlineTime()
{
	int ntotalTime = GetGlobalLogicEngine()->getMiniDateTime() - m_nAtvTotalOnlineTime.tv;
	if(ntotalTime <0)
	{
		ntotalTime =0;
	}
	m_nAtvTotalOnlineTime = GetGlobalLogicEngine()->getMiniDateTime();
	CScriptValueList paramLists;
	paramLists << this;
	paramLists << ntotalTime;
	GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("UpdateOnlineTime", paramLists, paramLists, 0);
}

//救主灵刃CD时间保存(针对离线CD重置的处理)
void CActor::SaveJZLRCDTime(LPCTSTR szVarName, int nTime)
{
	CCLVariant& pVarMgr	= GetActorVar();
	pVarMgr.set(szVarName, nTime);
}

//救主灵刃CD时间获取(针对离线CD重置的处理)
int CActor::SetJZLRCDTime(LPCTSTR szVarName)
{
	CCLVariant& pVarMgr	= GetActorVar();
	int nValue = 0;
	CCLVariant* pVar = pVarMgr.get(szVarName);
	if( pVar != NULL )
	{
		nValue = (int)(double)*pVar;
	}
	this->m_nJZLRCDTime = nValue;

	// 发送救主Buff 剩余CD时间到客户端
	CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(nJZBuffId);
	if(pBUFFCONFIG)
	{
		CDynamicBuff* pDynamicBuff = this->GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
		if(pDynamicBuff)
		{	
			int nRemainTime = this->m_nJZLRCDTime - GetGlobalLogicEngine()->getMiniDateTime();
			if (nRemainTime < 0)
			{
				nRemainTime = 0;
			}
			pDynamicBuff->m_nBuffCd = nRemainTime;
			this->GetBuffSystem()->OnBuffChangeCdTime(pDynamicBuff);
		}
	}

	return nValue;
}




void CActor::CheckXYKD()
{
	if(IsDeath()) return;
	
	//兵魂-血饮狂刀
	HallowsLorderInfo *pHallowsLorderInfo = GetHallowsSystem().GetHallowsLoderv(0, eHallowsSSC_XYGD);

	int nHp = GetProperty<int>(PROP_CREATURE_HP);
	int nMaxHp = GetProperty<int>(PROP_CREATURE_MAXHP);
	int nTemp = nMaxHp * ( (float)GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nToBuffResidueHP / 10000 );

	if(nHp < nTemp && pHallowsLorderInfo && pHallowsLorderInfo->nLv > 0)
	{
		//是否正在触发中
		bool bReady = true;
		std::vector<int>::iterator iter = pHallowsLorderInfo->vLorderBuff.begin();
		for(; iter != pHallowsLorderInfo->vLorderBuff.end(); ++iter)
		{
			CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(*iter);
			if(pBUFFCONFIG)
			{
				CDynamicBuff* pDynamicBuff = GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
				if(pDynamicBuff)
				{
					bReady = false;
					break;
				}
			}
		}

		if(bReady)
		{
			iter = pHallowsLorderInfo->vLorderBuff.begin();
			for(; iter != pHallowsLorderInfo->vLorderBuff.end(); ++iter)
			{
				GetBuffSystem()->Append(*iter);
			}

			//永久生命回复
			int nHp = GetProperty<unsigned int>(PROP_CREATURE_HP) + m_nHpRenewAdd;
			if (nHp <= 0)
			{
				nHp = 0;
			}
			SetProperty<unsigned int>(PROP_CREATURE_HP, nHp);

			static int nTXResidueHP = GetLogicServer()->GetDataProvider()->GetHallowthen().m_HallowsSSCMainInfo.nTXResidueHP;
			CEntityMsg msg2(CEntityMsg::emShowEntityEffect, GetHandle());
			msg2.nParam1 = nTXResidueHP;
			PostEntityMsg(msg2);
		}
	}

	//兵魂-血饮狂刀(移除Buff)
	if(nHp >= nTemp && pHallowsLorderInfo && pHallowsLorderInfo->nLv > 0)
	{
		std::vector<int>::iterator iter = pHallowsLorderInfo->vLorderBuff.begin();
		for (; iter != pHallowsLorderInfo->vLorderBuff.end(); ++iter)
		{
			CBuffProvider::BUFFCONFIG * pBUFFCONFIG = GetLogicServer()->GetDataProvider()->GetBuffProvider().GetBuff(*iter);
			if(pBUFFCONFIG)
			{
				CDynamicBuff* pDynamicBuff = GetBuffSystem()->GetBuff((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
				if(pDynamicBuff)
				{
					GetBuffSystem()->Remove((GAMEATTRTYPE)pBUFFCONFIG->nType, pBUFFCONFIG->nGroup);
				}
			}
		}
	}
}

//跨服相关接口
void CActor::SendMsg2CrossServer(int nType) {

	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCrossData);
	int nServerId = GetLogicServer()->GetCrossServerId();
	unsigned int nActorID = GetId();
	packet << nServerId << nActorID;
	packet <<(BYTE)nType;
	packet << GetCrossActorId();
	packet << GetAccountID();
	OutputMsg(rmTip,_T("[CrossLogin 8] SendMsg2CrossServer type:%d nActorID:%d "),nType,nActorID);
	//数据
	ACTORDBDATA data; //实体的存盘数据
	PackActorBasicData(data, 0);
	// data.name = {0};
	memcpy(data.name, GetEntityName(), sizeof(data.name));
	data.nAccountId = GetAccountID();
	data.nID = GetCrossActorId();
	packet << data;
	pCrossClient->flushProtoPacket(packet);
}

void CActor::Crosslogin(UINT_PTR nSocket,UINT_PTR nActorID,UINT_PTR nGateID, 
	UINT_PTR nGateSessionIndex,UINT_PTR nServerSessionIndex,
	UINT_PTR nAccountID, INT_PTR nCrossSrvId) {
	
	m_nUserSocket = nSocket;
	//SetProperty<unsigned>( & m_property.nID, (unsigned)nActorID );
	SetProperty<unsigned>(PROP_ENTITY_ID,(unsigned int)nActorID);
	m_nGateID = (unsigned)nGateID; //所在的网关
	m_nGateSessionIndex  = (unsigned)nGateSessionIndex;  //在网关上的编号
	m_nServerSessionIndex = (unsigned)nServerSessionIndex; //在逻辑服务器上的编号
	m_nAccountID = (unsigned)nAccountID;
	m_nGmLevel  = (int) 0; //玩家的GM  
	SetRawServerIndex(nCrossSrvId);

	GetGlobalLogicEngine()->GetEntityMgr()->OnActorLogin((unsigned int)nAccountID, (unsigned int)nActorID, GetHandle());
	//加载数据
	GetGlobalLogicEngine()->GetCrossMgr().LoginSetState(nCrossSrvId, nActorID);
	OutputMsg(rmTip,_T("CActor::Crosslogin, actorID=%u,accountID=%u,nGateID=%d,nSocket=%d"),
		nActorID,m_nAccountID,nGateID,nSocket);
	

}

bool CActor::CrossInit(void * pData, size_t size) {
	DECLARE_TIME_PROF("CActor::CrossInit");
	//这里搞了一个
	EntityFlags & flag= GetAttriFlag();
	flag.CanAttackMonster =true;
	flag.CanSceneTelport =true;
	
	PACTORDBDATA  pActorData = (ACTORDBDATA *)pData;
	if(pActorData ==NULL || size != sizeof(ACTORDBDATA))
	{
		const int actordbdatasize = sizeof(ACTORDBDATA);
		if(pActorData ==NULL )
		{
			OutputMsg(rmWaning,_T("创建角色的数据长度错误，数据指针为空，可能是其他实体创建调用错了接口"));
		}
		else
		{
			OutputMsg(rmWaning,_T("创建角色的数据长度错误，初始化失败"));
		}
		return false;
	}
	CLogicGateUser *pGateUser= GetLogicServer()->GetGateManager()->GetUserPtr(m_nGateID,m_lKey);
	if (pGateUser && (pGateUser->nCharId != pActorData->nID || pGateUser->nGlobalSessionId != pActorData->nAccountId))
	{
		//2017-1-11
		//登录客户端发来的nAccountID ，nActorID作校验,避免可以登录其他账号的角色
		OutputMsg(rmWaning,_T("登录异常: gateChar:%u,gateAccountid:%u,actorid:%u,accountid:%u"),pGateUser->nCharId,pActorData->nID,pGateUser->nGlobalSessionId,pActorData->nAccountId);
		return false;
	}
	char pShowName[320];
	memset(pShowName, 0, sizeof(pShowName));
	CROSSSERVERCFG* cfg = GetLogicServer()->GetLogicServerConfig()->getCSConfigCrossById(m_nRawServerIndex);
	if(cfg) {
		strcpy(pShowName,cfg->sLocalNamePR);//拷贝玩家的名字进去
		strcat(pShowName,".");
	}
	
	strcat(pShowName,pActorData->name);//拷贝玩家的名字进去
	GetGlobalLogicEngine()->GetEntityMgr()->OnActorInit(pShowName, GetHandle());
	memcpy((char *)&m_property.nPkMod,(char *)&pActorData->nPkMod,sizeof(ACTORDBPARTDATA)); //全部拷贝到属性集
	
	//拷贝玩家的账户的信息
	memcpy(&m_sAccount,&pActorData->accountName,sizeof(m_sAccount));
	m_sAccount[sizeof(m_sAccount)-1]=0;

	int nMaxAnger = GetLogicServer()->GetDataProvider()->GetVocationConfig()->GetVocationMaxAnger(GetProperty< int>(PROP_ACTOR_VOCATION));
	m_property.nAnger = MAKELONG(m_property.nAnger,nMaxAnger);

	//朝向
	m_property.nDir = LOBYTE(LOWORD(pActorData->nActorDir)); //朝向
	
	m_nLastLogOutTime = pActorData->nLastLogoutTime;  //上次的登录时间

	m_nAccountID = pActorData->nAccountId; //玩家的账户id
	m_nLoginTime.tv =GetGlobalLogicEngine()->getMiniDateTime();
	m_nUpdateTime = m_nLoginTime;
	m_nAtvTotalOnlineTime = m_nLoginTime;
	m_CurLoginTime.tv =GetGlobalLogicEngine()->getMiniDateTime();

	//db过来[时装，幻武，翅膀，空]
	WORD n12 = 0; //LOWORD(pActorData->nAlmirahLv);
	WORD n34 = 0; //HIWORD(pActorData->nAlmirahLv);
	byte nFashionLv = LOBYTE(n12);
	byte nWeaponExLv = HIBYTE(n12);
	byte nSwingLv = LOBYTE(n34);
	byte nFootPrintLv = HIBYTE(n34);

	// 初始化玩家当日打怪获取经验值	
	InitActorExpGetToday(pActorData->nExpToday);

	//初始化角色当日阵营贡献
	InitActorZyContriToday(pActorData->nZyContrToday);

	// 
	InitTomorrowDateTime();
	
	InitActorRelivePointData(pActorData);
	m_nCeateTime = pActorData->nCreateTime;
	m_consumeyb = pActorData->nConsumeYb;
	m_nLastLoginServerId = pActorData->nLastLoginServerId;
	m_nSwingLevel = pActorData->nSwingLevel;
	m_nSwingid = pActorData->nSwingId;
	m_LonginDays = pActorData->nLoginDays;
	m_nSalary = pActorData->nSalary;
	m_nFashionBattle = pActorData->nFashionBattle;
	m_NextChangeVocTime = pActorData->nNextChangeVocTime;
	m_nOldSrvId = pActorData->nOldSrvId;
	m_nAttackLevel = 100; // 玩家的攻击等级定死！
	if( Inherited::Init(pData,size) ==false )
	{
		OutputMsg(rmWaning,_T("玩家的实体的基类初始化失败"));
		return false;
	}
	SetProperty<int>(PROP_ACTOR_FRENZY,0);
	InitActorCampContr(); // 保证阵营子系统已经创建完毕
	SetEntityName(pShowName);
	if(m_chatSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("聊天子系统初始化失败"));
		return false;
	}
	if(!m_Equiptment.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("Init fail for equip system "));
		return false; // 属性子系统的初始化
	}

	if(m_questSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("任务子系统初始化失败"));
		return false;
	}
	if (m_guildSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("帮派子系统初始化失败"));
		return false;
	}
	if(m_GameStoreBuySystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("GameStoreBuySubSystem creation failure..."));
		return false;
	}
	if(m_DealSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("DealSystem creation failure..."));
		return false;
	}
	if(m_lootSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("lootsystem creation failure..."));
		return false;
	}
	
	if(m_teamSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("team create failure..."));
		return false;
	}

	if(m_Bag.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("Init fail for bag system "));
		return false; // 背包子系统的初始化
	}

	if (m_fubenSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("fuben create failure..."));
		return false;
	}
	if (m_msgSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("msgSystem create failure..."));
		return false;
	}
	if (m_pkSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("PkSystem create failure..."));
		return false;
	}
	if (m_deportSystem.Create(this,pData,size) == false)
	{
		OutputMsg(rmWaning,_T("deport system create failure..."));
		return false;
	}
	if(m_miscSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("misc system create failure..."));
		return false;
	}
	if (!m_varSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning,_T("actor system create failure..."));
		return false;
	}
	if(!m_achieveSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("achieve system create failure..."));
		return false;
	}

	if (!m_petSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("pet system create failure..."));
		return false;
	}

	if (!m_heroSystem.Create(this,pData, size))
	{
		OutputMsg(rmWaning,_T("hero system create failure..."));
		return false;
	}

	if(!m_friendSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("friendSystem system create failure ..."));
		return false;
	}

	if (!m_BossSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("BossSystem create failure ..."));
		return false;
	}
	if (!m_MailSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("MailSystem create failure ..."));
		return false;
	}
	if(!m_CkSpeedSystem.Create(this, pData, size))		//加速外挂检测子系统
	{
		OutputMsg(rmWaning, _T("m_CkSpeedSystem system create failure ..."));
		return false;
	}
	if (!m_NewTitleSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_NewTitleSystem create failure ..."));
		return false;
	}

	if (!m_AlmirahSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_AlmirahSystem create failure ..."));
		return false;
	}
	if (!m_ConsignmentSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_ConsignmentSystem create failure ..."));
		return false;
	}
	if (!m_CombatSystem.Create(this, pData, size))
	{
		OutputMsg(rmWaning, _T("m_CombatSystem create failure ..."));
		return false;
	}

	if(m_GameSets.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("Gamesetsystem creation failure..."));
		return false;
	}
	if(m_StaticCountSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("m_StaticCountSystem creation failure..."));
		return false;
	}

	if(m_BasicFunctionSystem.Create(this,pData,size) ==false)
	{
		OutputMsg(rmWaning,_T("m_BasicFunctionSystem creation failure"));
		return false;
	}

	if(!m_treasureSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_treasureSystem creation failure"));
		return false;
	}

	if (!m_activitySystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_activitySystem creation failure"));
		return false;
	}

	
	if (!m_strengthenSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_strengthenSystem creation failure"));
		return false;
	}
	if (!m_nGhostSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nGhostSystem creation failure"));
		return false;
	}
	if (!m_nHallowsSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nHallowsSystem creation failure"));
		return false;
	}
	if (!m_nCCrossSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_nCCrossSystem creation failure"));
		return false;
	}
	if (!m_ReviveDurationSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_ReviveDurationSystem creation failure"));
		return false;
	}
	if (!m_LootPetSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_LootPetSystem creation failure"));
		return false;
	}
	if (!m_RebateSystem.Create(this,pData,size))
	{
		OutputMsg(rmWaning,_T("m_RebateSystem creation failure"));
		return false;
	}

	m_asopCollector.SetActor(this);	
	
	const ENTITYPOS& pos = GetPosInfo();
	int nSceneId = pos.nSceneId, posx,posy;
	GetPosition(posx,posy);
	if (nSceneId <= 0)
	{
		GLOBALCONFIG & gloCfg = GetLogicServer()->GetDataProvider()->GetGlobalConfig();
		nSceneId = gloCfg.nInitSceneId;
		posx = gloCfg.nInitScenePosX;
		posy = gloCfg.nInitScenePosY;
	}
	if( GetGlobalLogicEngine()->GetFuBenMgr()->EnterFuBen(this,pos.pFb,nSceneId,posx,posy) ==false )
	{
		OutputMsg(rmWaning,_T("actorID=%d,Init fail for EnterFuBen,posX=%d,posY=%d,nScenceID=%d"),m_property.nID,m_property.nPosX,m_property.nPosY,GetSceneID());
		return false;
	}
	
	//记录日志
	int nYb = GetProperty<int>(PROP_ACTOR_YUANBAO);
	int nLevel = GetProperty<int>(PROP_CREATURE_LEVEL);
	if(GetLogicServer()->GetLogClient())
	{
		CLogicGateUser *pGateUser= GetLogicServer()->GetGateManager()->GetUserPtr(m_nGateID,m_lKey);
		if(pGateUser)
		{
			_asncpytA(m_sIp, pGateUser->sIPAddr);
		}
		if (m_nLastLogOutTime == 0)
			GetLogicServer()->GetLogClient()->SendLoginLog(ltCreateActor,GetAccountID(),GetAccount(),m_sIp,GetEntityName(),nLevel, nYb, 0, GetId(), getOldSrvId());
		GetLogicServer()->GetLogClient()->SendLoginLog(ltEntryGame, GetAccountID(),GetAccount(),m_sIp,GetEntityName(),nLevel, nYb, 0, GetId(), getOldSrvId());
	}

	EnterCrossServerScene();//跨服场景
	
	SendLoadCrossServerGuild();
	OnCrossFinishOneInitStep(0);
	// StartRequestActorSubSystemData();
	GetGlobalLogicEngine()->GetCrossMgr().InitActorSystem(GetRawServerIndex(), GetId());
	return true;
}

void CActor::OnCrossFinishOneInitStep(int step) {
	if(m_wCrossInitLeftStep > 0)
	{
		OutputMsg(rmNormal, _T("CActor::OnCrossFinishOneInitStep step=%d"), step);
		m_wCrossInitLeftStep --;
		if(m_wCrossInitLeftStep == 0)
		{
			OutputMsg(rmNormal, _T("CActor::OnCrossFinishOneInitStep actor[%u] dbdata loaded, prepare send packet to client"), GetId());
			OnGmTitle();
			GetEquipmentSystem().RefreshAppear(); //刷外观
		
			SetInitFlag(true); //初始化好了
			SetCommonServerFlag();  //设置公共服务器的标记
			
			GetPropertySystem().ResertProperty();//因为最大内劲值是内存的上线要算一下

			SendCreateData(); //下发创建的包

			GetPropertyPtr()->ClearUpdateMask(); //这里要清除属性改变
			CMiniDateTime todaytime = CMiniDateTime::today();
			bool isNextDay = false;
			if(GetLastLogoutTime() < todaytime) //昨天下线的，已经跨天了
			{
				SYSTEMTIME sysTime;
				m_nLastLogOutTime.decode(sysTime);
				m_nLastLogOutTime.encode(sysTime.wYear,sysTime.wMonth,sysTime.wDay,0,0,0);
				typedef unsigned int uint;
				int dayDiff = (uint(todaytime) - uint(m_nLastLogOutTime) + 3600*24 - 1)/(3600*24);
				printf("logouttime:%d, today:%d\n",GetLastLogoutTime(), CMiniDateTime::today());
				if ((int)m_nLastLogOutTime == 0)
				{
					dayDiff = 0;
					GetAchieveSystem().ActorAchievementComplete(nAchieveContinueLogin, 1);
				}
				isNextDay = true;
				OnNewDayArrive(dayDiff);

				SetNewDayArrive(dayDiff);
			}
			OnCreated();
			OnEnterScene();
			ResetShowName(); //重设玩家的showname
			
			if(isNextDay)
			{
				GetAchieveSystem().ActorAchievementComplete(nAchieveLogin, 1);
			} 
			else
			{
				SetNewDayArrive(0);
			}
			//下发计分器数据
			GetLogicServer()->GetLogicEngine()->GetScoreRecorderManager().ActorLogin(this);

			m_RundomKey = ((unsigned int) wrandvalue() + (unsigned int)_getTickCount() ) % (unsigned int) 26;

			CScriptValueList paramList;
			paramList << (int)(m_isFirstLogin ?1:0); //登录事件里添加一个参数，标明是不是第1次登录
			OnEvent(aeUserLogin,paramList,paramList);	
			GetAchieveSystem().SendAchieveData();
			paramList.clear();
			paramList << this;
			GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("SendPcClientState", paramList, paramList, 0);
			m_activitySystem.OnUserLogin();
			GetGlobalLogicEngine()->GetActivityMgr().OnUserLogin(this);
			// printf("nid,:%d, nServerId:%d\n",nid,nServerId);
		}
	}
}


bool CActor::SendLoginData2CrossServer()
{
	if (!this->IsInited()) 
		return false;
	SendMsg2CrossServer(MSG_CACTOR);
	GetSkillSystem().SendMsg2CrossServer(MSG_CSKILL);
	GetGameSetsSystem().SendMsg2CrossServer(MSG_CGAMESET);
	GetEquipmentSystem().SendMsg2CrossServer(MSG_CEQUIP);
	GetNewTitleSystem().SendMsg2CrossServer(MSG_CTITLE);
	GetAlmirahSystem().SendMsg2CrossServer(MSG_CFASH);	// 衣橱系统
	GetStrengthenSystem().SendMsg2CrossServer(MSG_CSTRENGTH);
	GetGhostSystem().SendMsg2CrossServer(MSG_CGHOST);
	GetHallowsSystem().SendMsg2CrossServer(MSG_CHALIDOME);
	GetBagSystem().SendMsg2CrossServer(MSG_CUSEBAG);
	GetReviveDurationSystem().SendMsg2CrossServer(MSG_CRELIVE);
	GetLootPetSystem().SendMsg2CrossServer(MSG_LOOTPET);//宠物系统非 ai宠物
	GetMiscSystem().SendMsg2CrossServer(MSG_GLOBAL_DATA);	//系统其他设置 GM
	return true;
}

void CActor::EnterCrossServerScene()
{
	CROSSSERVERCFG* cfg = GetLogicServer()->GetLogicServerConfig()->getCSConfigCrossById(m_nRawServerIndex);
	if(cfg)
	{
		int point_x = cfg->nPointRange[0] ;
		int point_y = cfg->nPointRange[1];
		INT_PTR nWeight = cfg->nPointRange[2] ;
		INT_PTR nHeight =  cfg->nPointRange[3];
		//处理传送 传送到所需中心点
		int nEffId = GetLogicServer()->GetDataProvider()->GetGlobalConfig().nBlinkEnterEffect;
		if(FlyShoseToStaticScene(cfg->nDestination,point_x,point_y , nWeight, nHeight,enDefaultTelePort,nEffId)){
		}
	}
}

void CActor::SetScriptCrossServerData(int nRealyActorId)
{
	CScriptValueList paramList;
	paramList << (int)GetId();
	paramList << nRealyActorId;
	paramList << (int)m_nRawServerIndex;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","InitActor", paramList, paramList, 1))
	{
		OutputMsg(rmError,"[Actor] Crossserver 错误 actorId:%d, m_nRawServerIndex:%d,nRealyActorId:%d!",GetId(),m_nRawServerIndex,nRealyActorId);
	}
}

void CActor::Mailtest() {
	int nActorId = GetId();
	std::vector<ACTORAWARD> awardList;
	BYTE btType;
	WORD wId;
	int wCount;
	for(int i = 0; i < 3; i++) {
		ACTORAWARD award ;
		award.btType = 1;
		award.wId = 1;
		award.wCount = 1000;
		awardList.emplace_back(award);
	}
	CMailSystem::SendCrossServerMail(nActorId,"1121221", "23343435",awardList);
}

void CActor::SendLoadCrossServerGuild()
{
	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcLoadCrossGuildId);
	packet << GetRawServerIndex() << GetLogicServer()->GetServerIndex() << (unsigned int)GetId();
	pDBClient->flushProtoPacket(packet);
}

void CActor::CrossServerRetDb(int nCmd,int nErrorCode,CDataPacketReader& reader)
{
	//装载物品数据
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadCrossGuildId &&  nErrorCode == jxInterSrvComm::DbServerProto::reSucc) //返回物品的列表
	{
		int nGuildid = 0;
		int nSocialMask = 0;
		reader >> nGuildid >> nSocialMask;
		SetProperty<int>(PROP_ACTOR_SOCIALMASK,nSocialMask);
		SetProperty<int>(PROP_ACTOR_GUILD_ID,nGuildid);

		CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildid);
		if ( pGuild )
		{
			SetProperty<unsigned int>(PROP_ACTOR_GUILD_LEVEL,pGuild->GetGuildbuildingLev(gbtMainHall));
		}
	}
	OnCrossFinishOneInitStep(MSG_CLOADGUILD);
}

void CActor::GetCSRealyActorId(int nActorId, int& nRealyActor, int& nRawSrvId)
{
	CScriptValueList paramList;
	CScriptValueList retList;
	paramList << (int)nActorId;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","GetCrossServerActor", paramList, retList, 2))
	{
		OutputMsg(rmError,"[Actor] Crossserver 错误 actorId:%d!",nActorId);
	}

	if(retList.count() >= 2  && retList[0])
	{
		nRealyActor = (int)retList[0];
		nRawSrvId = (int)retList[1];
	}
}


void CActor::LoginCheckOfflineMail()
{
	CScriptValueList paramList;
	paramList << this;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().Call("LoginCheckMail", paramList, paramList, 1))
	{
		OutputMsg(rmError,"[Actor] LoginCheckOfflineMail 错误 actorId:%d!",GetId());
	}
}
 
//跨服相关接口

//踢下同角色的跨服角色
void CActor::LoginCloseActor2Center() 
{ 
	CLocalCrossClient *pCrossClient = GetLogicServer()->GetCrossClient();
	if(pCrossClient->connected()) {
		CDataPacket &packet = pCrossClient->allocProtoPacket(jxInterSrvComm::CrossServerProto::cSendCloseActor);
		unsigned int nActorID = GetCrossActorId();
		packet << (BYTE)1 << nActorID;
		pCrossClient->flushProtoPacket(packet);
	}
}
void CActor::SetScriptAccountId(int nAccountId, int nRawSrvId )
{
	CScriptValueList paramList;
	paramList << (int)nAccountId;
	paramList << (int)nRawSrvId;
	paramList << this;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","InitCSAccount", paramList, paramList, 1))
	{
		OutputMsg(rmError,"[Actor] InitCSAccount 错误 account:%d!",nAccountId);
	}
}

void CActor::DeleteScriptAccountId(int nAccountId, int nRawSrvId)
{
	CScriptValueList paramList;
	paramList << (int)nAccountId;
	paramList << (int)nRawSrvId;
	if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","DeleteCSAccount", paramList, paramList, 1))
	{
		OutputMsg(rmError,"[Actor] InitCSAccount 错误 account:%d!",nAccountId);
	}
}

void CActor::KickUserAccount()
{
	CActorPacket ap;
	CDataPacket &outPack = AllocPacket(ap);
	outPack << (BYTE) enLoginSystemId << (BYTE)sOtherLoginGame;
	ap.flush();
	CloseActor(lwiSameActorLogin, false);
}

void CActor::SetRechargeStatus(LPCTSTR szVarName, int nStatus)
{
	CCLVariant& pVarMgr	= GetActorVar();
	pVarMgr.set(szVarName, nStatus);
}

int CActor::GetRechargeStatus(LPCTSTR szVarName)
{
	CCLVariant& pVarMgr	= GetActorVar();
	int nValue = 0;
	CCLVariant* pVar = pVarMgr.get(szVarName);
	if( pVar != NULL )
	{
		nValue = (int)(double)*pVar;
	}
	return nValue;
}


