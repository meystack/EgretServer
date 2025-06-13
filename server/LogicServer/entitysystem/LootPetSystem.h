#pragma once
#include <map>
#include<vector>
class CLootPetSystem:
	public CEntitySubSystem<enLootPetSystemID, CLootPetSystem, CActor> 
{
public:
	typedef CEntitySubSystem<enLootPetSystemID, CLootPetSystem, CActor> Inherited;

	typedef struct tagLootPetData
	{
		WORD			wId;
		WORD			nType;
		int	nContinueTime;
		tagLootPetData()
		{
			ZeroMemory(this,sizeof(tagLootPetData));
		};
	}LOOTPETDATA;
public:
	CLootPetSystem();
	~CLootPetSystem();

public:
	virtual bool Initialize(void *data,SIZE_T size);
	virtual void Destroy();
	virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);
	virtual VOID OnTimeCheck(TICKCOUNT nTickCount);

	//角色各子系统初始化完毕，向客户端发送创建主角的数据包后，调用该接口
	virtual void OnEnterGame();  
public:
	VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet);
	void LoadFromDB();
	void SaveToDB();
	void CalcAttributes( CAttrCalc &calc );

	int IsHaveLootPet( WORD wId);
	int IsHaveLootPetType( WORD wType);
	bool addLootPetTime( WORD wId );//续期
	bool addLootPet( WORD wId );
	bool delLootPet( WORD wId );
	void SetCurLootPet( WORD wId );
	
	void SendClientOfflineLootPetInfo(CDataPacket& data);

	std::vector<LOOTPETDATA>& getLootPetInfos()
	{
		return m_vLootPets;
	}

	VOID OnReturnLootPetInfo(CDataPacket &data);
private:
	void SendAllLootPetData();
	void SendAddTimeLootPetId(WORD wId, WORD wType, int nTime);
	void SendAddLootPetId(WORD wId, WORD wType, int nTime);
	void SendDelLootPetId(WORD wId, WORD wType);
	void ClientCutOverLootPetId(CDataPacketReader &packet);
	
public:
	//跨服数据
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(std::vector<CLootPetSystem::LOOTPETDATA>& petLists);

private:
	bool isInit;
	std::vector<LOOTPETDATA> m_vLootPets;
};
