#include "StdAfx.h"
#include "HeroSystem.h"
#include "../base/Container.hpp"
template<> const CHeroSystem::Inherid::OnHandleSockPacket CHeroSystem::Inherid::Handlers[]=
{
	&CHeroSystem::HandError, //占位
	&CHeroSystem::HandGetHeroList,
	&CHeroSystem::HandHeroStageUp, 
	&CHeroSystem::HandHeroSkillLevelUp,
	&CHeroSystem::HandChangeHeroName,
	&CHeroSystem::HandSetHeroState,
	&CHeroSystem::HandViewHero,
	&CHeroSystem::HandHeroLevelUp
};

CHeroSystem::CHeroSystem()
{
	Destroy();
	m_fExpRate =0.0; 
}

CHeroSystem::~CHeroSystem()
{
	Destroy();
}

//英雄DB数据
void CHeroSystem::OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader & reader )
{
	if(m_pEntity ==NULL)
	{
		OutputMsg(rmError,_T("load Hero data error,m_pEntity ==NULL"));
		return ;
	}
	//如果DB的数据已经初始化好了，再收到数据不处理
	if(nCmd == jxInterSrvComm::DbServerProto::dcLoadHeroData) //装载玩家的英雄数据
	{
		if(HasDbDataInit() )
		{
			OutputMsg(rmError,_T(" Hero data has loaded"));
			return ;
		}
		if(nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			int nCount=0;  
			reader >> nCount;
			if(nCount >0)
			{
				bool flag =true;
				HERODATA Hero;
				Hero.equips.clear();
				Hero.skills.clear();
				for ( int i=0; i< nCount; i++)
				{
					reader >> Hero.data;
					INT_PTR nPos = GetHeroPos (Hero.data.bID ); 
					if(nPos >=0)
					{
						OutputMsg(rmError,_T("add Hero error ,has same Hero id=%d"),(int)Hero.data.bID );
						continue;
					}
					m_heros.add(Hero);
				}
			}
		}
		else
		{
			OutputMsg(rmError,_T("load[%s]Hero data error,actorid=%u"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID));
		}
	}
	else if(nCmd == jxInterSrvComm::DbServerProto::dcLoadHeroSkillData) //装载玩家的英雄技能
	{
		if(HasDbDataInit() )
		{
			OutputMsg(rmError,_T(" Hero data has loaded"));
			return ;
		}
		if(nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{
			int nCount=0; 
			reader >> nCount;
			if(nCount >0)
			{
				CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();  //技能数据读取器
				CStdItemProvider & itemProvider = GetLogicServer()->GetDataProvider()->GetStdItemProvider(); //物品配置器

				HEROSKILLDATA data;
				for ( int i=0; i< nCount; i++)
				{ 
					memset(&data,0,sizeof(data));
					reader >> data;
					INT_PTR nPos = GetHeroPos (data.nHeroID);
					if(nPos <0)
					{
						OutputMsg(rmError,_T("Hero skill data errr,no Hero=%d"),data.nHeroID);
					}
					else
					{
						const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(data.nSkillID, data.nLevel); 
						const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
						if(pLevel ==NULL || pConfig ==NULL) continue;
						m_heros[nPos].skills.add(data);
					}
				}
			}
			((CActor*)m_pEntity)->OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_HERO_SYSTEM); //完成一个步骤
			OnDbInitData(); //这个时候才确定db的数据发生了修改
		}
		else
		{
			OutputMsg(rmError,_T("load[%s]Hero skill data error,actorid=%d"),((CActor*)m_pEntity)->GetEntityName(),m_pEntity->GetProperty<int>(PROP_ENTITY_ID));
		}
	}
}

void  CHeroSystem::ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet )
{
	if(m_pEntity ==NULL) return;
	if(nCmd >=0 && nCmd <ArrayCount(CHeroSystem::Inherid::Handlers))
	{
		(this->*Handlers[nCmd] )(packet);
	}
}

void CHeroSystem::OnEnterGame()
{
	CallbackBattleHero(false);
}

void CHeroSystem::OnEnterScene()
{


}

void CHeroSystem::Destroy()
{
	CEntity * pEntity;
	for(INT_PTR i=0; i< m_heros.count();i++)
	{
		m_heros[i].skills.clear();
		m_heros[i].equips.clear();
		pEntity =GetEntityFromHandle(m_heros[i].handle);
		if(pEntity)
		{
			GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(m_heros[i].handle); 
		}
	}
	m_heros.clear();
}

void CHeroSystem::HandViewHero(CDataPacketReader &packet)
{
	if(m_pEntity ==NULL) return;
	if (m_pEntity->HasMapAttribute(aaCannotViewOther))
	{
		(m_pEntity)->SendOldTipmsgWithId(tpChatDisableOperator);
		return;
	}

	char msg[54];
	packet.readString(msg,sizeof(msg));
	msg[sizeof(msg) -1] =0;
	CActor * pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetActorPtrByName(msg);
	if(pActor && pActor->IsInited())
	{
		pActor->GetHeroSystem().ViewHeros(m_pEntity); //观察其他玩家的英雄
	}
	else
	{
		((CActor*)m_pEntity)->SendOldTipmsgWithId(tpChatTargetOffLine); 
	}
}

//查看英雄
void  CHeroSystem::ViewHeros(CActor * pActor)
{
	if(m_pEntity ==NULL) return;
	if(pActor ==NULL || pActor->IsInited() ==false) return;
	INT_PTR nCount=  m_heros.count();
	if(nCount <=0 ) 
	{
		pActor->SendOldTipmsgWithId(tpOtherHasNoHero,ttChatWindow );
		return ;
	}
	//如果没有出战英雄，默认为第一个英雄
	INT_PTR nHeroIndex =0; 
	for(INT_PTR i=0; i< nCount; i++)
	{
		HERODATA & HERODATA = m_heros[i];
		if( HERODATA.data.bState == psStateNormal)
		{
			nHeroIndex = i;
			break;	
		}
	}
	HERODATA &hero = m_heros[nHeroIndex];
	HERODBDATA & heroData = m_heros[nHeroIndex].data;
	CActorPacket actorData;
	CDataPacket& data = pActor->AllocPacket(actorData);
	data <<(BYTE)GetSystemID() << (BYTE)sHeroView;
	data << (BYTE) heroData.bID;         //id
	data << (BYTE) heroData.bState;    //状态
	data << (BYTE) heroData.bStage;   //阶
	data << (BYTE) heroData.bLevel;   //等级
	data << (unsigned int)heroData.nExp; //当前经验
	data << (int)heroData.nBodyforce; //当前体力
	data << (int)heroData.nBless;  //当前祝福
	data.writeString(heroData.name);  //名字
	CVector<HEROSKILLDATA> & skills =  m_heros[nHeroIndex].skills;
	data <<(BYTE) skills.count();
	for(INT_PTR j=0;j < skills.count(); j++)
	{
		HEROSKILLDATA & skill =hero.skills[j];
		data << (WORD) skill.nSkillID;
		data << (BYTE) skill.nLevel;
	}
	actorData.flush();
	((CActor*)m_pEntity)->PlayerViewNotice(pActor);
}

//计算英雄的属性
void CHeroSystem::CalHeroProp(HERODATA & Hero)
{
	if(m_pEntity ==NULL) return;
	//英雄未出战，不计算属性
	if(Hero.data.bState  != psStateNormal)
		return;
	Hero.cal.reset();	   
	CAttrCalc calc;
	PHEROLEVEL  levelCfg =  GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroLevel(Hero.data.bID, Hero.data.bLevel);
	if(!levelCfg)
		return;
	for(int  i = 0; i < levelCfg->attri.nCount ; i++)
	{
		calc << levelCfg->attri.pAttrs[i];
	}
	for(INT_PTR i=0; i< Hero.skills.count(); i++)
	{
		((CHero*)m_pEntity)->GetSkillSystem().CalcOneSkillAttr(Hero.skills[i].nSkillID,Hero.skills[i].nLevel,calc); 	
	}
	Hero.cal  <<  calc;
}

//重载这个函数,初始化玩家的一些2级属性
bool CHeroSystem::Initialize(void *data,SIZE_T size)
{
	if(m_pEntity ==NULL ) return false;
	return true;
}

void CHeroSystem::Save(PACTORDBDATA pData)
{
	if(m_pEntity ==NULL) return;
	//保存英雄列表
	size_t count;
	int nRawServerIndex = m_pEntity->GetRawServerIndex();
	int nLoginServerIndex = GetLogicServer()->GetServerIndex();
	if(m_HeroBasicDataHasModifed)
	{
		count = m_heros.count();
		CDataPacket& HeroPacket =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveHeroData);
		HeroPacket << nRawServerIndex << nLoginServerIndex;
		HeroPacket <<(unsigned int ) m_pEntity->GetId(); //玩家ID
		HeroPacket  << (int)count; //玩家技能的数量
		for (int  i=0; i< count; i ++ )
		{
			HERODATA  &hero = m_heros[i];
			HeroPacket << hero.data; //只保持db需要的那部分
		}
		GetLogicServer()->GetDbClient()->flushProtoPacket(HeroPacket);
		SetHeroDataModifyFlag(edHeroBasicData,false);
	}

	//保存英雄的技能列表
	if(m_heroskillHasModified)
	{
		count =0;
		for(INT_PTR i=0; i< m_heros.count(); i++)
		{
			count += m_heros[i].skills.count();
		}
		CDataPacket& skillPack =GetLogicServer()->GetDbClient()->allocProtoPacket(jxInterSrvComm::DbServerProto::dcSaveHeroSkillData);
		skillPack << nRawServerIndex << nLoginServerIndex;
		skillPack << (unsigned int ) m_pEntity->GetProperty<unsigned int>(PROP_ENTITY_ID); //玩家ID
		skillPack  << (int)count; //玩家技能的数量
		for (INT_PTR i=0; i< m_heros.count(); i ++ )
		{
			for(INT_PTR j=0;j < m_heros[i].skills.count(); j++)
			{
				skillPack << m_heros[i].skills[j]; //只保持db需要的那部分
			}
		}
		GetLogicServer()->GetDbClient()->flushProtoPacket(skillPack);
		SetHeroDataModifyFlag(edHeroSkillData,false);
	}
}

void CHeroSystem::OnTimeCheck(TICKCOUNT nTickCount)
{
	if(m_pEntity ==NULL) return;
	INT_PTR nCount =m_heros.count();
	if(nCount <=0) return;
	for(INT_PTR i= nCount-1; i>-1; i--)
	{ 
		INT_PTR nState = m_heros[i].data.bState;
		if(nState ==psStateNormal)
		{
			CEntity *pHero = GetEntityFromHandle(m_heros[i].handle);
			if(pHero )
			{
				bool hasHpMpChange =false;  //HP,MP是否发生了改变
				unsigned int nHp = pHero->GetProperty<unsigned int>(PROP_CREATURE_HP);
				unsigned int nMp = pHero->GetProperty<unsigned int>(PROP_CREATURE_MP);
				if(nHp >0)
				{
					if(m_heros[i].data.nHp != nHp)
					{
						m_heros[i].data.nHp = nHp;
						hasHpMpChange = true;
					}	
				}
				if(m_heros[i].data.nMp != nMp)
				{
					m_heros[i].data.nMp = nMp;
					hasHpMpChange =true;
				}
				//数据发生了改变
				if(hasHpMpChange)
				{
					SetHeroDataModifyFlag(edHeroBasicData,true);
				}			
			}
		}
	}
}

//英雄属性附加到人物
CAttrCalc & CHeroSystem::CalcAttributes(CAttrCalc &cal)
{
	for(INT_PTR i=0; i< m_heros.count(); i++)
	{
		if ( m_heros[i].data.bState == psStateMerge) //附体属性
		{
			cal << m_heros[i].mergeCal;		//附体
		}
	}
	return cal;
}

//是否能增加英雄
bool CHeroSystem::CanAddHero(bool bWithTipmsg)
{  
	if(m_pEntity ==NULL) return false;
	int nMaxCount = GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroMaxCount(); 
	bool flag= m_heros.count()  <  nMaxCount;  
	if(!flag  && bWithTipmsg)
	{
		m_pEntity->SendOldTipmsgWithId(tpHeroCanNotGetMore,ttChatWindow);
	}
	return flag;
}

//添加英雄
int  CHeroSystem::AddHero(int  nHeroId, int  nStage,  int  nLevel)
{
	if(m_pEntity ==NULL) return -1;
	if(!CanAddHero()  ||  GetHeroPos(nHeroId) >= 0)
		return -1;
	PHEROCONFIG HeroCfg = GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroData(nHeroId);
	PHEROSTAGE StageCfg = GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroStage(nHeroId, nStage);
	if(!HeroCfg || !StageCfg)
		return - 1;
	HERODATA hero;
	HERODBDATA &heroData =hero.data; 
	int nOldCount = (int)m_heros.count();
	heroData.bStage = (BYTE)__max(nStage,1);
	heroData.bLevel = (BYTE)__max(nLevel,1);
	heroData.bID =  nHeroId;
	heroData.bVocation = enVocWarrior;  //默认为战士
	heroData.nModel = StageCfg->nModel;
	heroData.nWeaponAppear = StageCfg->nWeapon;
	heroData.nBodyforce = StageCfg->nGiveForce; //创建时系统给于的体力值
	strncpy(heroData.name, HeroCfg->name,sizeof(heroData.name));
	m_heros.add(hero);
	SendHeroData(hero);
	SetHeroDataModifyFlag(edHeroBasicData,true);
	LogHero(GameLog::clHeroAdd, (int)heroData.bID, (int)nOldCount, m_heros.count());
	ResetAllHeroProperty();
	return heroData.bID;
}

//下发英雄数据
void CHeroSystem::HandGetHeroList(CDataPacketReader &packet)
{
	if(m_pEntity ==NULL) return;
	CActorPacket pack;
	CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)GetSystemID() << (BYTE)sHeroList;
	INT_PTR nCount=  m_heros.count();
	data << (BYTE)nCount; 
	for(INT_PTR i=0; i< nCount; i++)
	{
		HERODBDATA & heroData = m_heros[i].data;
		HERODATA &hero = m_heros[i];
		data << (BYTE) heroData.bID;      //id
		data << (BYTE) heroData.bState;  //状态
		data << (BYTE) heroData.bStage; //阶
		data << (BYTE) heroData.bLevel; //等级
		data << (unsigned int)heroData.nExp; //经验
		data << (int)heroData.nBodyforce; //体力
		data << (int)heroData.nBless; //祝福
		data.writeString(heroData.name); //名字
	}
	pack.flush();
	SendAllSkills();
}

//下发英雄的技能
void CHeroSystem::SendAllSkills()
{
	if(m_pEntity ==NULL) return;
	INT_PTR nCount=0;
	for(INT_PTR i=0;i < m_heros.count(); i++)
	{
		nCount += m_heros[i].skills.count();
	}
	CActorPacket pack;
	CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)GetSystemID() << (BYTE)sHeroSkillList;
	data << (BYTE)nCount; 
	for(INT_PTR i=0;i < m_heros.count(); i++)
	{
		for(INT_PTR j=0;j <  m_heros[i].skills.count(); j++)
		{
			HEROSKILLDATA & skill = m_heros[i].skills[j];
			pack << (BYTE) skill.nHeroID;
			pack << (WORD) skill.nSkillID;
			pack << (BYTE) skill.nLevel;
		}
	}
	pack.flush();
}

//获取战斗中的英雄
CHero * CHeroSystem::GetBattleHeroPtr()
{
	for(INT_PTR i=0 ;i < m_heros.count(); i++)
	{
		if(m_heros[i].data.bState ==psStateNormal)
		{
			return (CHero*) GetEntityFromHandle(m_heros[i].handle);
		}
	}
	return NULL;
}

//获取战斗中的英雄ID
int   CHeroSystem::GetBattleHeroId()
{
	for(INT_PTR i=0;i <m_heros.count();i ++ )
	{
		HERODBDATA &data =m_heros[i].data;
		if(data.bState == psStateNormal) 
		{
			if(GetEntityFromHandle(m_heros[i].handle))
			{
				return data.bID;
			}
		}
	}
	return -1;
}

//获取英雄位置
INT_PTR  CHeroSystem::GetHeroPos(INT_PTR nHeroId) 
{
	for(INT_PTR i=0; i< m_heros.count();i ++)
	{
		if(m_heros[i].data.bID == (BYTE)nHeroId)
		{
			return i;
		}
	}
	return -1;
}

//日志
void CHeroSystem::LogHero(const INT_PTR nLogIdent, const INT_PTR nHeroId, const INT_PTR nParam1, const INT_PTR nParam2)
{
	if (m_pEntity == NULL) return;
	INT_PTR nPos = GetHeroPos(nHeroId);
	if(nPos <0) return; 
	HERODATA& Hero = m_heros[nPos];
	char sData[24]; 
	sprintf(sData,"%d_%d_%d",Hero.data.bID,Hero.data.bStage,Hero.data.bLevel); 
}

//英雄改名
void CHeroSystem::HandChangeHeroName(CDataPacketReader &packet)
{
	if(m_pEntity ==NULL) return;
	BYTE bHeroID=0; 
	packet >> bHeroID ; 
	INT_PTR nPos = GetHeroPos(bHeroID) ;
	if(nPos <0) return ;
	char name[32];
	packet.readString(name,sizeof(name));
	if( GetGlobalLogicEngine()->GetChatMgr().Filter(name) >0)
	{
		m_pEntity->SendOldTipmsgWithId(tpInputIsForbid,ttFlyTip);
		return;
	}
	ChangeHeroName(nPos,name);
}

void CHeroSystem::ChangeHeroName(INT_PTR nPos,char * name)
{
	if(m_pEntity ==NULL) return;
	HERODATA &Hero =   m_heros[nPos];
	HERODBDATA &data = Hero.data;
	if(strcmp(name,data.name) ==0 ) return ;
	if(strlen(name)  == 0)
	{
		m_pEntity->SendOldTipmsgWithId(tpHeroNameIsEmpty,ttFlyTip);
		return;
	} 
	for (int i=0; i < strlen(name); i++)
	{
		if (name[i] == '"' || name[i] == '\\' || name[i] == '\''|| name[i] == '\''|| name[i] == '\''|| name[i] == '\'')
		{
			m_pEntity->SendOldTipmsgWithId(tpInputIsForbid,ttFlyTip);
			return;
		}
	}
	strncpy(data.name,name,sizeof(data.name));
	SendHeroData(Hero);
	//如果英雄已出战
	if(data.bState ==psStateNormal)
	{
		CHero *pHero =(CHero*) GetEntityFromHandle(m_heros[nPos].handle);
		if(pHero )
		{
			char sName[50];
			sprintf_s(sName,sizeof(sName),"%s\\%s",name, m_pEntity->GetEntityName());
			pHero->SetEntityName(data.name);
			pHero->SetShowName(sName);
			pHero->ChangeName(sName);
		}
	}
	LogHero(GameLog::clHeroChangeName, data.bID, 0, 0);
	SetHeroDataModifyFlag(edHeroBasicData,true);
	m_pEntity->SendOldTipmsgWithId(tpHeroChangeNameSuccess,ttFlyTip);
}

//英雄事件
void CHeroSystem::TriggerEvent(INT_PTR nSubEvent,INT_PTR nParam1,INT_PTR nParam2,INT_PTR nParam3,INT_PTR nParam4)
{
	if(m_pEntity ==NULL) return;
	CScriptValueList paramList;
	//((CActor*)m_pEntity)->InitEventScriptVaueList(paramList,aeHero);
	paramList << (int)nSubEvent;
	if(nParam1 >=0)
	{
		paramList << (int)nParam1;
		if(nParam2 >=0)
		{
			paramList << (int)nParam2;
			if(nParam3 >=0)
			{
				paramList << (int)nParam3;
				if(nParam4 >=0)
				{
					paramList << (int)nParam4;
				}
			}
		}	
	}
	((CActor*)m_pEntity)->OnEvent(aeHero,paramList,paramList);
}

//英雄学习技能
bool CHeroSystem::LearnSkill(int nHeroID, int nSkillID)
{
	if(m_pEntity ==NULL) return false;
	INT_PTR nPos = GetHeroPos(nHeroID) ;
	if(nPos <0) return false ;
	HERODATA &Hero = m_heros[nPos];
	if(GetSkillLevel(nHeroID,nSkillID) > 0 ) 
	{
		return false;
	}
	HEROSKILLDATA data;
	data.nHeroID = nHeroID;
	data.nSkillID = nSkillID;
	data.nLevel =1;
	CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();  
	const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(data.nSkillID, data.nLevel); 
	const OneSkillData *pConfig =  pSkillProvider->GetSkillData(data.nSkillID);
	if(pLevel ==NULL || pConfig ==NULL) return false;
	Hero.skills.add(data);

	CActorPacket pack;
	CDataPacket & dataPack =  ((CActor*)m_pEntity)->AllocPacket(pack);
	dataPack << (BYTE)GetSystemID() << (BYTE)sHeroSkillUpdate;
	dataPack << (BYTE)nHeroID;      //英雄的ID
	dataPack << (WORD)nSkillID;    //技能的ID
	dataPack << (BYTE)data.nLevel;  //技能的等级
	pack.flush();

	//如果处于战斗状态的话，英雄立即学习技能
	if(Hero.data.bState == psStateNormal) 
	{
		CHero *pHero = (CHero*)GetEntityFromHandle(Hero.handle);
		if(pHero)
		{
		    //if(!pConfig->bIsDelete)
			//{
				pHero->GetSkillSystem().LearnSkill(nSkillID,1,false,true,true);
				pHero->GetAI()->RefreshSkillDis();
			//}
			TriggerEvent(OpHeroBattle,nHeroID,nSkillID, 1);  //script run
			ResetProperty(nPos); 
		}
	}
	//记录日志
	LogHero(GameLog::clHeroLearnSkill, nHeroID,nSkillID,1);
	SetHeroDataModifyFlag(edHeroSkillData,true);
	return true;
}

//英雄技能升级
void CHeroSystem::HandHeroSkillLevelUp(CDataPacketReader &packet)
{
	if(m_pEntity ==NULL) return;
	BYTE bHeroID=0; 
	WORD wSkillId =0;
	packet >> bHeroID >> wSkillId ; 
	INT_PTR nPos = GetHeroPos(bHeroID) ;
	if(nPos <0) return ;
	int nLevel = GetSkillLevel(bHeroID,wSkillId);
	if(nLevel <=0 ) 
	{
		m_pEntity->SendOldTipmsgFormatWithId(tpHeroSkillNotActivate, ttFlyTip);
		return;
	} 
	CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(wSkillId, nLevel+1); 
	if(pLevel == NULL )
	{    
		m_pEntity->SendOldTipmsgFormatWithId(tpHeroSkillLevelFull, ttFlyTip);
		return;
	}
	TriggerEvent(OpHeroUpSkill,bHeroID,wSkillId,nLevel);  //script run
}

bool  CHeroSystem::SkillLevelUp(int nHeroId,int nSkillId)
{
	if(m_pEntity ==NULL) return false;
	INT_PTR nPos = GetHeroPos(nHeroId) ;
	if(nPos <0) return false ;
	int nSkillLevel = GetSkillLevel(nHeroId,nSkillId);
	if(nSkillLevel <= 0) return false;
	nSkillLevel =  nSkillLevel + 1;
	//已达到最高技能等级
	CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
	const OneSkillData *pConfig =  pSkillProvider->GetSkillData(nSkillId);
	const SKILLONELEVEL * pLevel = pSkillProvider->GetSkillLevelData(nSkillId, nSkillLevel); 
	if(pConfig ==NULL || pLevel ==NULL ) return false;

	HERODATA &Hero = m_heros[nPos];	 
	CVector<HEROSKILLDATA> &skills =  Hero.skills;
	for(INT_PTR i=0;i < skills.count(); i++)
	{
		if(skills[i].nSkillID == nSkillId)
		{
			skills[i].nLevel += 1;
			//记录日志
			LogHero(GameLog::clHeroSkillLevelUp, nHeroId, nSkillId, skills[i].nLevel);
			break;
		}
	}

	CActorPacket pack;
	CDataPacket & dataPack =  ((CActor*)m_pEntity)->AllocPacket(pack);
	dataPack << (BYTE)GetSystemID() << (BYTE)sHeroSkillUpdate ;
	dataPack << (BYTE)nHeroId; 
	dataPack << (WORD)nSkillId; 
	dataPack << (BYTE)nSkillLevel;
	pack.flush();

	//如果处于战斗状态的话，英雄立即学习技能
	if(Hero.data.bState == psStateNormal) 
	{
		CHero *pHero = (CHero*)GetEntityFromHandle(Hero.handle);
		if(pHero)
		{
		   //if(!pConfig->bIsDelete)
		   //{
			    pHero->GetSkillSystem().LearnSkill(nSkillId,nSkillLevel,false,true,true);
		    	pHero->GetAI()->RefreshSkillDis();
			//}
			TriggerEvent(OpHeroBattle,nHeroId,nSkillId, nSkillLevel);  //script run
			ResetProperty(nPos); //重置属性
		}
	}
	SetHeroDataModifyFlag(edHeroSkillData,true);
	m_pEntity->SendOldTipmsgFormatWithId(tpHeroSkillLevelUp,ttTipmsgWindow,Hero.data.name,
		GetLogicServer()->GetDataProvider()->GetSkillProvider()->GetSkillNameById(nSkillId),
		nSkillLevel);
	return true;
}

//获取英雄等级
int CHeroSystem::GetSkillLevel(int nHeroId, int nSkillId)
{
	INT_PTR nPos = GetHeroPos(nHeroId) ;
	if(nPos <0) return -1 ;
	HERODATA &Hero = m_heros[nPos];
	CVector<HEROSKILLDATA> &skills =  Hero.skills;
	for(INT_PTR i=0;i < skills.count(); i++)
	{
		if(skills[i].nSkillID == nSkillId)
		{
			return skills[i].nLevel;
		}
	}
	return -1 ;
}

//英雄升级
void CHeroSystem::HandHeroLevelUp(CDataPacketReader &packet)
{
	 TriggerEvent(OpHeroLevelUp);  
}

//英雄升阶
void CHeroSystem::HandHeroStageUp(CDataPacketReader &packet)
{
	BYTE bHeroID=0; 
	BYTE bCount =0;
	packet >> bHeroID >> bCount;
	INT_PTR nPos = GetHeroPos(bHeroID) ;
	if(nPos <0) return ;
	HERODATA &Hero = m_heros[nPos];
	TriggerEvent(OpHeroUpStage,bHeroID,Hero.data.bStage,Hero.data.nBless,bCount);  //script run
}

bool CHeroSystem::StageUp(int nHeroId, int nStage, int nBless)
{
	if(m_pEntity ==NULL) return false;
	INT_PTR nPos = GetHeroPos(nHeroId) ;
	if(nPos <0) return false;
	HERODATA &Hero = m_heros[nPos];
	int  nOldStage = Hero.data.bStage;
	CHeroProvider &provider = GetLogicServer()->GetDataProvider()->GetHeroConfig();
	PHEROCONFIG    heroCfg =  provider.GetHeroData(nHeroId);
	PHEROSTAGE  stageCfg = provider.GetHeroStage(nHeroId,nStage);
	if(!heroCfg || !stageCfg) return false;
	Hero.data.bStage = nStage;
	if(nBless > stageCfg->nMaxBless)
	{
		Hero.data.nBless =  stageCfg->nMaxBless;
	}else
	{
		Hero.data.nBless =  nBless;
	}
	Hero.data.nModel = stageCfg->nModel; //升阶改变英雄模型
	Hero.data.nWeaponAppear = stageCfg->nWeapon;
	SendHeroData(Hero);
	ResetProperty(nPos,false,true);
	SetHeroDataModifyFlag(edHeroBasicData,true);
	LogHero(GameLog::clHeroStageUp, (int)Hero.data.bID, nOldStage, nStage);
	ResetAllHeroProperty();
	return true;
}

//下发英雄数据(创建&&更新)
void  CHeroSystem::SendHeroData(HERODATA &Hero)
{
	if(m_pEntity ==NULL) return;
	CActorPacket pack;
	CDataPacket & data =  ((CActor*)m_pEntity)->AllocPacket(pack);
	data << (BYTE)GetSystemID() << (BYTE)sHeroUpdate;
	data << (BYTE) Hero.data.bID;         //id
	data << (BYTE) Hero.data.bState;    //状态
	data << (BYTE) Hero.data.bStage;   //阶
	data << (BYTE) Hero.data.bLevel;   //等级
	data << (unsigned int)Hero.data.nExp; //当前经验
	data << (int)Hero.data.nBodyforce; //当前体力
	data << (int)Hero.data.nBless;  //当前祝福
	data.writeString(Hero.data.name);  //名字
	pack.flush();
}

//收回英雄
bool CHeroSystem::SetHeroSleep(INT_PTR nHeroId,bool bClient)
{
	if(m_pEntity ==NULL) return false;
	INT_PTR nPos = GetHeroPos(nHeroId);
	if(nPos <0) return false; 
	HERODATA & Hero = m_heros[nPos];
	INT_PTR nCurrentState = Hero.data.bState;
	if( nCurrentState== psStateNormal) //战斗状态
	{
		GetGlobalLogicEngine()->GetEntityMgr()->DestroyEntity(Hero.handle);
		Hero.handle =0;
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	//重新刷能力,玩家的属性发生了改变
	}
	else if(nCurrentState == psStateMerge) //如果当前是合体状态的话
	{
		m_pEntity->CollectOperate(CEntityOPCollector::coRefAbility);	//重新刷能力,玩家的属性发生了改变
		m_pEntity->RemoveState(esStateHeroMerge);	//去除附体状态
	}
	Hero.data.bState = psStateSleep; 
	if(bClient)
	{
		SendHeroStateChange(nPos);
	}
	SetHeroDataModifyFlag(edHeroBasicData,true);
	return true;
}

//玩家Death时英雄的处理
void CHeroSystem::OnDeath()
{
	for(INT_PTR i=0;i < m_heros.count();i ++)
	{
		if(m_heros[i].data.bState == psStateNormal)
		{
			SetHeroSleep(m_heros[i].data.bID,true); 
		}
	}
}

//下发英雄经验
void CHeroSystem::SetClientHeroExp(HERODATA &Hero)
{
	if(m_pEntity ==NULL) return;
	CActorPacket pack;
	CDataPacket & dataPack =  ((CActor*)m_pEntity)->AllocPacket(pack);
	dataPack << (BYTE)GetSystemID() << (BYTE)sHeroExpChange ;
	dataPack << (BYTE)Hero.data.bID << (unsigned int) Hero.data.nExp; 
	pack.flush();
}

//下发英雄状态
void CHeroSystem::SendHeroStateChange(INT_PTR nPos)
{
	if(m_pEntity ==NULL) return;
	if(nPos <0 || nPos >= m_heros.count()) return; 
	HERODBDATA &data = m_heros[nPos].data;
	CActorPacket pack;
	CDataPacket & dataPack =  ((CActor*)m_pEntity)->AllocPacket(pack);
	dataPack << (BYTE)GetSystemID() << (BYTE)sHeroStateChange ;
	dataPack << (BYTE)data.bID << (BYTE) data.bState; 
	pack.flush();
	SetHeroDataModifyFlag(edHeroBasicData,true);
}

//英雄被攻击
void CHeroSystem::OnAttacked(CAnimal *pEntity)
{
	CHero * pHero;
	for(INT_PTR i=0; i< m_heros.count(); i++)
	{
		if(m_heros[i].data.bState ==psStateNormal)
		{
			pHero= (CHero*)GetEntityFromHandle(m_heros[i].handle);
			if(pHero &&pHero->GetAI())
			{
				pHero->GetAI()->AttackedBy(pEntity);
			}
		}
	}
}

//英雄攻击
void CHeroSystem::OnAttackOther(CAnimal * pEntity)
{
	CHero * pHero;
	for(INT_PTR i=0; i< m_heros.count(); i++)
	{
		if(m_heros[i].data.bState ==psStateNormal)
		{
			pHero= (CHero*)GetEntityFromHandle(m_heros[i].handle);
			if(pHero &&pHero->GetAI())
			{
				pHero->GetAI()->AttackedBy(pEntity);
			}
		}
	}
}

//英雄跟随
void CHeroSystem::FollowEntity(CAnimal *pActor)
{
	CHero * pHero;
	for(INT_PTR i=0; i< m_heros.count(); i++)
	{
		if(m_heros[i].data.bState ==psStateNormal)
		{
			pHero= (CHero*)GetEntityFromHandle(m_heros[i].handle);
			if(pHero &&pHero->GetAI())
			{
				if (pHero->GetMoveSystem()->GetMotionType() == CMovementSystem::mtMotionTypeChase)
				{
					pHero->GetMoveSystem()->PopCurrentMotionState();
				}
				pHero->GetAI()->EnterEvadeMode();//退出战斗
                pHero->GetMoveSystem()->MoveFollow(pActor);

			}
		}
	}
}

//英雄获得打怪经验
void CHeroSystem::OnAddExp(unsigned int nExp,bool boRate, int nHeroId)
{
	for(INT_PTR i=0;i <m_heros.count();i ++ )
	{
		HERODATA &Hero = m_heros[i];
		HERODBDATA &data = Hero.data;
		if(data.bState == psStateNormal ||  ((i + 1) == nHeroId ))
		{  
			if(boRate)
			{  
				unsigned int  nAddExp =   (unsigned int) ( (nExp *  Hero.fExpRate) * (1 + m_fExpRate));
				RealChangeExp(i,nAddExp);
			}else
			{
				RealChangeExp(i,nExp);
			}
		}
	}
}

bool CHeroSystem::RealChangeExp(INT_PTR nPos,unsigned int nAddExp)
{
	if(m_pEntity ==NULL) return false;
	if(nPos < 0 || nPos >= m_heros.count())
		return false;
	HERODATA &Hero= m_heros[nPos];
	HERODBDATA &data = Hero.data;
	int  nHeroId = data.bID;
	int  nStage = data.bStage;
	int  nLevel = data.bLevel;
	CHeroProvider &provider = GetLogicServer()->GetDataProvider()->GetHeroConfig();
	PHEROSTAGE  stageCfg = provider.GetHeroStage(nHeroId,nStage);
	PHEROLEVEL   levelCfg = provider.GetHeroLevel(nHeroId, nLevel);
	BOOL  boLevelUp = false;
	unsigned int nRealAddExp = 0;
	if(nAddExp > 0 && stageCfg  && levelCfg)
	{
		bool boLoop = true;
		while (boLoop)
		{          		        
			levelCfg = provider.GetHeroLevel(nHeroId, data.bLevel);
			if(!levelCfg)
			{
				boLoop = false;
				break; 
			}
			//已达本阶最高级
			if(data.bLevel >= stageCfg->nMaxLevel)
			{
				boLoop = false;
				if( (data.nExp + nAddExp) > levelCfg->nLevelUpExp)
				{
					nRealAddExp += levelCfg->nLevelUpExp - (unsigned int)data.nExp;	
					data.nExp = levelCfg->nLevelUpExp;  
				}else
				{
					nRealAddExp +=  nAddExp;
					data.nExp += nAddExp;					  
				}
				break;
			}else
			{    
				//可以升级了
				if((data.nExp + nAddExp) >= (unsigned int)levelCfg->nLevelUpExp)
				{
					BYTE  nNewLevel = data.bLevel + 1;
					PHEROLEVEL   nextLevelCfg = provider.GetHeroLevel(nHeroId, nNewLevel);
					if(nextLevelCfg)
					{
						boLevelUp  = true;
						data.bLevel = nNewLevel;
						int  nDecExp = (unsigned int)levelCfg->nLevelUpExp - (unsigned int)data.nExp;
						data.nExp = 0;   //升级后经验需要归0
						nRealAddExp +=  nDecExp;
						nAddExp -=  nDecExp;						   
					}else
					{
						boLoop = false;
					}				 
				}else
				{
					data.nExp += nAddExp;
					nRealAddExp += nAddExp;
					boLoop = false;
				}
			} 
		}

		if(nRealAddExp  > 0)
		{
		    if(m_fExpRate > 0.0)
			{
                int  nRateValue =  (int)( (nRealAddExp / (1 + m_fExpRate)) * m_fExpRate );
				m_pEntity->SendOldTipmsgFormatWithId(tpHeroGetExpRate, ttTipmsgWindow,nRealAddExp,nRateValue);
			}else
			{
			    m_pEntity->SendOldTipmsgFormatWithId(tpHeroGetExp, ttTipmsgWindow,nRealAddExp);
			}		
		} 
		if(boLevelUp)
		{
			CHero *pHero = (CHero*)GetEntityFromHandle(Hero.handle);
			if (pHero)
			{
				pHero->SetLevel(data.bLevel);
			}
			ResetProperty(nPos);
			SendHeroData(Hero);
			LogHero(GameLog::clHeroUpgrade, data.bID, nLevel, data.bLevel);
		}
		SetHeroDataModifyFlag(edHeroBasicData,true);
		SetClientHeroExp(Hero);
		return true;
	}
	return  false;
}

//重置所有英雄属性
void CHeroSystem::ResetAllHeroProperty(bool bNewHero,bool bRefeshAppear)
{
	for(INT_PTR i=0; i < m_heros.count();i ++)
	{
		ResetProperty(i, bNewHero, bRefeshAppear);
	}
}

//重置英雄属性
void CHeroSystem::ResetProperty(INT_PTR nHeroPos,bool boNewHero ,bool boRefAppear)
{
	if (nHeroPos < 0 || nHeroPos >= m_heros.count())
		return;
	HERODATA  &Hero =  m_heros[nHeroPos]; 
	CalHeroProp(Hero); 
	if(Hero.data.bState == psStateNormal) //如果处于出战状态，需要刷新属性
	{
		CHero * pHero = (CHero*)(GetEntityFromHandle(Hero.handle)); 
		if(pHero)
		{
			//最大hp, mp
			unsigned int  nMaxHp  =   (unsigned int)Hero.cal.m_AttrValues[aMaxHpAdd].nValue; //最大hp
			unsigned int  nMaxMp =  (unsigned int)Hero.cal.m_AttrValues[aMaxMpAdd].nValue; //最大mp
			pHero->SetProperty<unsigned int>(PROP_CREATURE_MAXHP,  nMaxHp);
			pHero->SetProperty<unsigned int>(PROP_CREATURE_MAXMP, nMaxMp);
			pHero->CollectOperate(CEntityOPCollector::coRefAbility);	
			pHero->SetLevel(Hero.data.bLevel); 
			pHero->GetAI()->RefreshSkillDis();
			if(boRefAppear)
			{
				pHero->SetProperty<unsigned int>(PROP_ENTITY_MODELID,Hero.data.nModel);
				pHero->SetProperty<unsigned int>(PROP_ACTOR_WEAPONAPPEARANCE,Hero.data.nWeaponAppear);
			}
		}
	}
}

//英雄出战
bool CHeroSystem::SetHeroBattle(INT_PTR nHeroId,bool boClient)
{
	if(m_pEntity ==NULL) return false;
	INT_PTR nPos = GetHeroPos(nHeroId);
	if(nPos <0) return false; 
	HERODATA & Hero = m_heros[nPos];
	PHEROCONFIG  HeroCfg =  GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroData((int)nHeroId);
	PHEROLEVEL  LevelCfg =  GetLogicServer()->GetDataProvider()->GetHeroConfig().GetHeroLevel(Hero.data.bID, Hero.data.bLevel);
	if(!HeroCfg && !LevelCfg)
		return false;
	//已出战的其他英雄收回
	CallbackBattleHero(boClient);
	HERODBDATA & data= Hero.data;
	CScene *pScene = m_pEntity->GetScene();
	if(pScene ==NULL) return false;
	int nPosX,nPosY;
	m_pEntity->GetPosition(nPosX,nPosY); 
	//已经放出了一个英雄
	CEntity* pEntity =GetEntityFromHandle(Hero.handle);
	if(pEntity != NULL)
	{
		return false;
	}
	//创建一个英雄
	int nNewPosX = 0, nNewPosY = 0;
	pScene->GetEntityAppearPos(nPosX, nPosY, 4,nNewPosX, nNewPosY);
	if(nNewPosX > 0 && nNewPosY > 0)
	{
        pEntity = pScene->CreateEntityAndEnterScene(nHeroId,enHero,nNewPosX,nNewPosY,data.bLevel,m_pEntity); 
	}else
	{
	    pEntity = pScene->CreateEntityAndEnterScene(nHeroId,enHero,nPosX,nPosY,data.bLevel,m_pEntity); 
	}
	
	if(pEntity ==NULL) return false;
	CHero *pHero = (CHero*) pEntity;
	Hero.data.bState = psStateNormal; 
	Hero.handle = pEntity->GetHandle();
	//计算英雄属性
	CalHeroProp(Hero);
	char sAllName[512];
	sprintf_s(sAllName,sizeof(sAllName),"%s\\%s",data.name, m_pEntity->GetEntityName());
	pHero->SetEntityName(data.name);
	pHero->SetShowName(sAllName);
	//设置模型，hp,mp(用属性值的填充)
	pEntity->SetProperty<int>(PROP_ENTITY_MODELID,data.nModel);
	pEntity->SetProperty<unsigned int>(PROP_ACTOR_WEAPONAPPEARANCE,data.nWeaponAppear);
	CAttrCalc &cal = Hero.cal;
	unsigned int  nMaxHp  =   (unsigned int)cal.m_AttrValues[aMaxHpAdd].nValue; //最大hp
	unsigned int  nMaxMp =  (unsigned int)cal.m_AttrValues[aMaxMpAdd].nValue; //最大mp
	pEntity->SetProperty<unsigned int>(PROP_CREATURE_MAXHP, nMaxHp);
	pEntity->SetProperty<unsigned int>(PROP_CREATURE_HP, nMaxHp);
	pEntity->SetProperty<unsigned int>(PROP_CREATURE_MP,nMaxMp);
	pEntity->SetProperty<unsigned int>(PROP_CREATURE_MAXMP,nMaxMp);
	Hero.nBaseMaxHp =  nMaxHp;
	Hero.nBaseMaxMp = nMaxMp;
	Hero.fExpRate = HeroCfg->fExpRate;
	pHero->InitAi();
	if( pHero->GetAI() ==NULL)
	{
		OutputMsg(rmError,"hero InitAi fail, hero id = %d",data.bID);
		return false;
	}
	int nAttackSpd = pHero->GetProperty<int>(PROP_CREATURE_ATTACK_SPEED);
	if(nAttackSpd > 0)
	{
		pHero->SetAttackInterval(nAttackSpd);
	}
	pEntity->SetInitFlag(true);
	//立即学习技能
	for(INT_PTR i=0;i < Hero.skills.count(); i++)
	{
		int nSkillId = Hero.skills[i].nSkillID;
		int nSkillLevel = Hero.skills[i].nLevel;
		CSkillProvider *pSkillProvider = GetLogicServer()->GetDataProvider()->GetSkillProvider();
		const OneSkillData *pConfig =  pSkillProvider->GetSkillData(nSkillId);
		//if(!pConfig->bIsDelete)
		//{
		     pHero->GetSkillSystem().LearnSkill(nSkillId,nSkillLevel,false,true,true);
		//}		
		TriggerEvent(OpHeroBattle,nHeroId,nSkillId, nSkillLevel);  //script run
	}
	pHero->GetAI()->RefreshSkillDis(); //重新刷新技能
	pHero->GetAttriFlag().DenyAutoAddHp =  false;  //允许回血
	pHero->GetAttriFlag().DenyAutoAddMp = false;  //允许回蓝
	pHero->GetAttriFlag().DenyUseNearAttack =false; //支持肉搏
	if(boClient)
	{
		SendHeroStateChange(nPos);
	}
	return true;
}


void CHeroSystem::HandSetHeroState(CDataPacketReader &packet)
{
	if(m_pEntity ==NULL) return;
	BYTE bHeroId=0,bState =0; 
	packet >> bHeroId >> bState ; 
	INT_PTR nPos = GetHeroPos(bHeroId) ;
	if(nPos <0) return ;
	HERODATA &Hero =  m_heros[nPos];
	HERODBDATA &data = Hero.data;
	if(data.bState == bState)return; 

	if(bState == psStateNormal)
	{
		CScene *pScene = m_pEntity->GetScene();
		int x, y;
		m_pEntity->GetPosition(x,y);
		if (pScene && pScene->HasMapAttribute(x, y, aaNotCallHero))
		{
			m_pEntity->SendOldTipmsgFormatWithId(tpCannotCallHeroInTheArea, ttFlyTip);
			return;
		}
		CHeroProvider &provider = GetLogicServer()->GetDataProvider()->GetHeroConfig();
		PHEROCONFIG HeroCfg = provider.GetHeroData(bHeroId);
		if(HeroCfg)
		{
			if(data.nBodyforce <  HeroCfg->nBattleForce) 
			{
				m_pEntity->SendOldTipmsgFormatWithId(tpHeroBattleForceLacking, ttFlyTip);
				return;		       
			}
			data.nBodyforce = data.nBodyforce - HeroCfg->nBattleForce;
			SetHeroDataModifyFlag(edHeroBasicData,true);
			SendHeroData(Hero);
			SetHeroBattle(bHeroId);
		}

	}
	else if(bState == psStateSleep)
	{
		SetHeroSleep(bHeroId);
	}
	ResetAllHeroProperty();
}


//收回所有出战英雄
void  CHeroSystem::CallbackBattleHero(bool boClient)
{
	for(int i = 0; i < m_heros.count(); i++)
	{
		if(m_heros[i].data.bState ==psStateNormal)
		{
			SetHeroSleep(m_heros[i].data.bID,boClient);
		}
	}
}

//跨天清除祝福值
void  CHeroSystem::EveryDayClearBless()
{
	for(int i= 0; i < m_heros.count(); i++)
	{
		HERODATA &Hero = m_heros[i];
		CHeroProvider &provider = GetLogicServer()->GetDataProvider()->GetHeroConfig();
		PHEROSTAGE  stage = provider.GetHeroStage(Hero.data.bID, Hero.data.bStage);
		if(stage && stage->boEveryDayClearBless)
		{
			Hero.data.nBless = 0;
			SetHeroDataModifyFlag(edHeroBasicData,true);
			SendHeroData(Hero);
		}
	}
}

//英雄 合体
bool CHeroSystem::SetHeroMerge(INT_PTR nHeroId,bool bClient)
{
	return true;
}
