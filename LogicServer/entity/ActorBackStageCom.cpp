#include "StdAfx.h"
#include "Actor.h"
#include "../script/interface/LogDef.h" 
#include<vector> 
#include "InterServerComm.h"   
#include "../base/Container.hpp"  

using namespace jxInterSrvComm;
void CActor::SaveCustomInfoData()
{ 
	// unsigned int * pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_ID]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_ENTITY_ID); 
	// pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_POSX]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_ENTITY_POSX);
	// pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_POSY]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_ENTITY_POSY); 
	// int * pDataint  = (int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_MODELID]; 
	// *pDataint = GetProperty<int>(PROP_ENTITY_MODELID);

	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_ID, PROP_ENTITY_ID);
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_POSX, PROP_ENTITY_POSX);
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_POSY, PROP_ENTITY_POSY); 
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_ICON, PROP_ENTITY_ICON);
	COPY_CUSTOM_PROPERTY_INT(ACTORRULE_PROP_ENTITY_MODELID, PROP_ENTITY_MODELID); 
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_DIR, PROP_ENTITY_DIR);
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_CREATURE_LEVEL, PROP_CREATURE_LEVEL);
	COPY_CUSTOM_PROPERTY(ACTORRULE_PROP_ENTITY_POSX, PROP_ENTITY_POSX);

	// pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_ICON]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_ENTITY_ICON);
	// pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_DIR]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_ENTITY_DIR);   
	// pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_CREATURE_LEVEL]; 
	// *pDataUint = GetProperty<unsigned int>(PROP_CREATURE_LEVEL);  
	 
	// ACTORRULE_PROP_CREATURE_HP = CAL_ARS(7), 					//
	// ACTORRULE_PROP_CREATURE_MP = CAL_ARS(8), 					//
	// ACTORRULE_PROP_CREATURE_STATE = CAL_ARS(9), 				//
	// ACTORRULE_PROP_CREATURE_COLOR = CAL_ARS(10), 				//
	// ACTORRULE_PROP_CREATURE_MAXHP = CAL_ARS(11), 				//
	// ACTORRULE_PROP_CREATURE_MAXMP = CAL_ARS(12), 				//
	// ACTORRULE_PROP_CREATURE_PHYSICAL_ATTACK_MIN = CAL_ARS(13), 	//
	// ACTORRULE_PROP_CREATURE_PHYSICAL_ATTACK_MAX = CAL_ARS(14), 	//
	// ACTORRULE_PROP_CREATURE_MAGIC_ATTACK_MIN = CAL_ARS(15), 	//最小魔法攻击
	// ACTORRULE_PROP_CREATURE_MAGIC_ATTACK_MAX = CAL_ARS(16), 	//最大魔法攻击   
	// ACTORRULE_PROP_CREATURE_WIZARD_ATTACK_MIN = CAL_ARS(17), 	//最小道术攻击  
	// ACTORRULE_PROP_CREATURE_WIZARD_ATTACK_MAX = CAL_ARS(18), 	//最大道术攻击  
	// ACTORRULE_PROP_CREATURE_PYSICAL_DEFENCE_MIN = CAL_ARS(19), 	//最小物理防御  
	// ACTORRULE_PROP_CREATURE_PYSICAL_DEFENCE_MAX = CAL_ARS(20), 	//最大物理防御  
	// ACTORRULE_PROP_CREATURE_MAGIC_DEFENCE_MIN = CAL_ARS(21), 	//最小魔法防御  
	// ACTORRULE_PROP_CREATURE_MAGIC_DEFENCE_MAX = CAL_ARS(22), 	//最大魔法防御    
	// ACTORRULE_PROP_CREATURE_HITVALUE = CAL_ARS(23), 	//物理命中    
	// ACTORRULE_PROP_CREATURE_DODVALUE = CAL_ARS(24), 	//物理闪避    
	// ACTORRULE_PROP_CREATURE_MAGIC_HITRATE = CAL_ARS(25), 	//魔法命中    
	// ACTORRULE_PROP_CREATURE_MAGIC_DOGERATE = CAL_ARS(26), 	//魔法闪避  
	// ACTORRULE_PROP_CREATURE_HP_RATE_RENEW = CAL_ARS(27), 	//HP万分比恢复  
	// ACTORRULE_PROP_CREATURE_MP_RATE_RENEW = CAL_ARS(28), 	//MP万分比恢复   
	// ACTORRULE_PROP_CREATURE_MOVEONESLOTTIME = CAL_ARS(29), 	//移动1格需要的时间，单位ms   
	// ACTORRULE_PROP_CREATURE_ATTACK_SPEED = CAL_ARS(30), 	//攻击速度   
	// ACTORRULE_PROP_CREATURE_LUCK = CAL_ARS(31), 			//幸运   
	// ACTORRULE_PROP_CREATURE_HP_RENEW = CAL_ARS(32), 		//HP值恢复   
	// ACTORRULE_PROP_CREATURE_MP_RENEW = CAL_ARS(33), 		//MP值恢复   
	// ACTORRULE_PROP_CREATURE_DIZZY_STATUS = CAL_ARS(34),		//麻痹     
	// pDataint = (int *)&m_ProCustom.cbCustomProperty[2044]; 
	// *pDataint = 12233;
 

	CDataClient *pDBClient = GetLogicServer()->GetDbClient();
	CDataPacket &packet = pDBClient->allocProtoPacket(DbServerProto::dcSaveCustomInfo);
	int nServerId = GetLogicServer()->GetServerIndex();
	unsigned int nActorID = GetId();
	packet << GetRawServerIndex() << nServerId << nActorID;
	INT_PTR nSize = sizeof(m_ProCustom); 
	packet.writeBuf(&m_ProCustom, nSize);
	pDBClient->flushProtoPacket(packet);
}
int CActor::OnGetCustomInfoInt(int index)
{
	if(index < 0 || index > 2048)
	{
		return 0;
	}
	int * pData = (int *)&m_ProCustom.cbCustomProperty[index]; 
	int pDataTmp = *pData; 
	return pDataTmp;
}
unsigned int CActor::OnGetCustomInfoUInt(int index)
{
	if(index < 0 || index > 2048)
	{
		return 0;
	}
	unsigned int * pData = (unsigned int *)&m_ProCustom.cbCustomProperty[index]; 
	unsigned int pDataTmp = *pData; 
	return pDataTmp;
}
BYTE CActor::OnGetCustomInfoByte(int index)
{
	if(index < 0 || index > 2048)
	{
		return 0;
	}
	BYTE * pData = (BYTE *)&m_ProCustom.cbCustomProperty[index]; 
	BYTE pDataTmp = *pData; 
	return pDataTmp;
}
VOID CActor::OnDbCustomInfoData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader &reader)
{    
    CActor *pActor = (CActor*)this;

	if (nCmd == jxInterSrvComm::DbServerProto::dcSaveCustomInfo)
	{
		//保存成功
        return; 
	}
	
	CActorCustomProperty data;
    if( nCmd == jxInterSrvComm::DbServerProto::dcLoadCustomInfo
	 //||	nCmd == jxInterSrvComm::DbServerProto::dcLoadCustomInfoUnForbid
		) //返回数据
    {
		if (nErrorCode == jxInterSrvComm::DbServerProto::reSucc)
		{  
			reader.readBuf(&data, sizeof(CActorCustomProperty));
			//reader >> data; 
		}
		else
		{

		} 
    } 
	memcpy( &m_ProCustom, &data,  sizeof(m_ProCustom) );
 
	unsigned int * pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[ACTORRULE_PROP_ENTITY_ID]; 
	unsigned int pDataUintTmp = *pDataUint; 
	// //unsigned int pDataUintTmp = *pDataUint;
	// int * pDataint = (int *)&m_ProCustom.cbCustomProperty[2044];  
 
	// int pDataintTmp = *pDataint;
    OnFinishOneInitStep(eMSG_USR_LOGIN_DATA_ACTOR_CUSTOM);
}
//后台封禁角色
void CActor::OnBackForbidUser(INT_PTR nDuration)
{ 
	//m_ProCustom.cbCustomProperty[ACTORRULE_KICK_USER] = 1;//标记踢角色  
	ADD_CUSTOM_PROPERTY_BYTE(ACTORRULE_KICK_USER, 1);
	unsigned int nFreePostTime = (unsigned int)(GetGlobalLogicEngine()->getMiniDateTime() + nDuration); 

	ADD_CUSTOM_PROPERTY(ACTORRULE_KICK_USER_FREETIME, nFreePostTime);
	//m_ProCustom.cbCustomProperty[ACTORRULE_KICK_USER_FREETIME] = nFreePostTime;//标记时间
}

void CActor::OnBackUnForbidUser()
{    
	ADD_CUSTOM_PROPERTY_BYTE(ACTORRULE_KICK_USER, 0); 
	unsigned int nCurrTm = GetGlobalLogicEngine()->getMiniDateTime();
	ADD_CUSTOM_PROPERTY(ACTORRULE_KICK_USER_FREETIME, nCurrTm); 
} 
// void CActor::OnSetNewCdkGift(std::string giftCode)
// {  
// 	std::string realCode = "";
// 	int SrvId = 0;
// 	std::vector<std::string> results = SplitStr(giftCode, "|");
// 	int nCount = results.size(); 
// 	for (int i = 0; i < nCount; i++)
// 	{			
// 		int state = atoi(results[i].c_str());
// 		if (5 == i)
// 		{
// 			realCode = results[i].c_str();
// 		}
// 		if (3 == i)
// 		{
// 			SrvId = atoi(results[i].c_str());
// 		}
// 	}
// 	if(SrvId == 0 || realCode == "")
// 	{ 
// 		OutputMsg(rmTip, "[YXMCdkGift]error  CdkGift :SrvId%d ,Code %s", SrvId, realCode.c_str());
// 		return;
// 	}
// 	//处理CDK礼包码
//  	static CNpc* globalNpc = GetGlobalLogicEngine()->GetGlobalNpc(); 
// 	CScriptValueList paramList, retList; 
// 	// paramList << enMiscSystemID;
// 	// paramList << cUseNewCdkey; 
// 	paramList << this;  
	
// 	std::string strSendBuff = "";
// 	strSendBuff.resize(1000);   
// 	CDataPacket pack((char *)strSendBuff.data(), 1000); 
// 	pack << (byte)1;
// 	pack << (int)SrvId;     
// 	pack.writeString(realCode.c_str());  
// 	pack.setPosition(0);
// 	CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
// 	paramList << &dp; 
// 	bool ret = globalNpc->GetScript().CallModule("NewCdkey", "OnCMDBackStageCdKey", paramList, retList, 1); 
// 	if (!ret)
// 	{
// 		const RefString& s = globalNpc->GetScript().getLastErrorDesc();  
// 		OutputMsg(rmTip, "error SetNewCdkGift:%s", (const char*)s);
// 	} 
// 	paramList.clear();
// 	retList.clear();
 
// 	// CScriptValueList paramList;
// 	// paramList << (int)nAccountId;
// 	// paramList << (int)nRawSrvId;
// 	// paramList << this;
// 	// if(!GetGlobalLogicEngine()->GetGlobalNpc()->GetScript().CallModule("CrossServerDispatcher","InitCSAccount", paramList, paramList, 1))
// 	// {
// 	// 	OutputMsg(rmError,"[Actor] InitCSAccount 错误 account:%d!",nAccountId);
// 	// }

// 	// INT_PTR nOffer2 = pack.getPosition();  
// 	// pack.setPosition(nOffer);
// 	// pack << (byte)pagesize; 
// 	// pack.setPosition(nOffer2);
// 	// pack.setPosition(0);
// 	// CDataPacketReader dp(pack.getOffsetPtr(), pack.getAvaliableLength()); 
// 	// CScriptValueList paramList, retList; 
// 	// paramList << enMiscSystemID;
// 	// paramList << sFcmAllUseHeatbeat; 
// 	// paramList << pActor; 
// 	// paramList << &dp;
// 	// static CNpc* gp = GetGlobalLogicEngine()->GetGlobalNpc();
	
// 	// bool ret = gp->GetScript().Call("OnNetMsg", paramList, retList);
// 	// if (!ret)
// 	// {
// 	// 	//脚本错误，将以模态对话框的形式呈现给客户端
// 	// 	const RefString& s = gp->GetScript().getLastErrorDesc();  
// 	// 	OutputMsg(rmTip, "load KidsFcm:%s", (const char*)s);
// 	// } 
// }