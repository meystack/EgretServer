#pragma once
#include  <iostream>
#include <vector>
#include"GameSets.h"
#define MAX_BROCASTKEY_SIZE 11 //最大快捷键
#define MAX_BROCASTKEYAPP_SIZE 5 //最大快捷键
//游戏设置
template<int SUBSYSTEMID,class SonClass, class EntityClass>
class CEntitySubSystem;
class CActor;

class CGameSetsSystem:
	public CEntitySubSystem<enGameSetsSystemID, CGameSetsSystem, CActor> 
{
public:
	typedef CEntitySubSystem<enGameSetsSystemID, CGameSetsSystem, CActor> Inherited;

	typedef struct tagKeyBoardDATA
	{
		unsigned int			nPos;
		unsigned int			nType;
		unsigned int	        uId;
		tagKeyBoardDATA()
		{
			ZeroMemory(this,sizeof(tagKeyBoardDATA));
		};
	}KEYBOARDDATA;

	enum{
		EN_GAMESET_BASE = 1, //基础
		EN_GAMESET_ITEM = 2, //物品
		EN_GAMESET_MEDI = 3, //药品
		EN_GAMESET_PROT = 4, //保护
		EN_GAMESET_HOOK = 5, //挂机
		EN_GAMESET_SYST = 6, //系统
		EN_GAMESET_RECYLE = 7, //回收
	};

	enum
	{
		eSPECIAL_HOTKEY_RECYLE = 55, //回收
		eSPECIAL_HOTKEY_RANDOM = 58, //随机
		eSPECIAL_HOTKEY_MEDICINE = 239, //药 
	};
	enum
	{
		eSPECIAL_HOTKEY_RECYLE_POS = 15, //回收默认位置
		eSPECIAL_HOTKEY_RANDOM_POS = 11, //随机默认位置
		eSPECIAL_HOTKEY_MEDICINE_POS = 12, //药默认位置
	};
public:
	CGameSetsSystem();
	~CGameSetsSystem();

public:
	virtual bool Initialize(void *data,SIZE_T size);
	virtual void Destroy();
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);
	
public:
	void OnEnterScene();
	VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet);
	void LoadFromDB();
	void SaveToDB();

	void UpdateGameSetsData(CDataPacketReader &packet);
	void DoUpdateGameSetsData(int nCType,int nCPos, int nCId,int lastPos);
	void DiscardGameSetsData(CDataPacketReader &packet);
	bool CheckSameTypeAndId(int pos, int type, int id, int& samepos); //查询是否存在冲突的id
	void DealKeyBoard();
		
	//手机端
	void UpdateGameSetsDataApp(CDataPacketReader &packet); 
	void SendAppKeyBoardsInfo(); 
	void DiscardGameSetsDataApp(CDataPacketReader &packet);  
	void AutoGameSetSkillApp(int nCType, int nCId);//自动加技能格
	void DoUpdateGameSetsDataApp(int nCType,int nCPos, int nCId,int lastPos = 0, bool isAuto = true);
	bool CheckSameTypeAndIdApp(int pos, int type, int id, int& samepos); //查询是否存在冲突的id

	void SetGameCheck(CDataPacketReader &packet);	
	void SetGameValue(CDataPacketReader &packet);
	void Reset(CDataPacketReader &packet);
	void OneKeyItem(CDataPacketReader &packet);
	void SendOneData(BYTE nSetType);
	//自动补位
	void AutoGameSetSkill(int nCType, int nCId);
	void SendAllData();

	void OnChangeVocation();

	//获取设置数据
	GAMESETSDATA2& GetGameSet() { return m_GsData2; }

private:
	void SendGameSetsData();

public:
	//跨服数据发送
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(GAMESETSDATA& GsData, GAMESETSDATA2& GsData2);
private:
	GAMESETSDATA        m_GsData;
	GAMESETSDATA2       m_GsData2;
	std::vector<KEYBOARDDATA> m_kbDataList;
	std::vector<KEYBOARDDATA> m_kbAppDataList;//手机端
};
