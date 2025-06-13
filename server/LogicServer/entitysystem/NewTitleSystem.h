#pragma once
#include <map>

enum eNewTitleEvent
{
	mtEventGm				= 1,	///< GM
	mtEventCircle			= 3,	///< 转生
	mtEventVip				= 4,	///< vip
	mtEventSoul				= 5,	///< 武魂
	mtEventBuildBody		= 6,	///< 炼体
	mtEventDiamon			= 7,	///< 宝石
	mtEventAlmirahFasion	= 8,	///< 时装
	mtEventAlmirahSwing		= 9,	///< 翅膀
	mtEventAlmirahWeapon	= 10,	///< 幻武
	mtEventAlmirahFootprint	= 11,	///< 足迹

	mtEventTakeOnEquipLevel = 13,	///< 穿上等级装备
	mtEventTakeOnSuit		= 14,	///< 穿上套装
	mtMaxNewTitleEvent,
};

class CNewTitleSystem:
	public CEntitySubSystem<enNewTitleSystemID, CNewTitleSystem, CActor> 
{
	public:
		typedef CEntitySubSystem<enNewTitleSystemID, CNewTitleSystem, CActor> Inherited;

		typedef struct tagNEWTITLEDATA
		{
			WORD			wId;
			unsigned int	nRemainTime;
			tagNEWTITLEDATA()
			{
				ZeroMemory(this,sizeof(tagNEWTITLEDATA));
			};
		}NEWTITLEDATA;
	public:
		CNewTitleSystem();
		~CNewTitleSystem();

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

		void OnNewTitleEvent(BYTE btType, INT_PTR nParam1=-1, INT_PTR nParam2=-1);
		void OnCustomitleEvent(BYTE btType, INT_PTR nParam1=-1, INT_PTR nParam2=-1);

		int IsHaveNewTitle( WORD wId );
		bool addNewTitleTime( WORD wId );//头衔续期
		bool addNewTitle( WORD wId );
		bool delNewTitle( WORD wId );
		void SetCurNewTitle( WORD wId );

		int IsHaveCustomTitle( WORD wId );
		bool addCustomTitleTime( WORD wId );// 自定义称号续期
		bool addCustomTitle( unsigned int wId );
		bool delCustomTitle( unsigned int wId );
		void SetCurCustomTitle( WORD wId );
		void OnEnterScene();
		
		void SendClientTitleInfo(CDataPacket& data);
		void SendClientCustomTitleInfo(CDataPacket& data);

		CVector<NEWTITLEDATA>& getNewTitleInfos()
		{
			return m_NewTitleDataList;
		}
		
		CVector<NEWTITLEDATA>& getCustomTitleInfos()
		{
			return m_CustomTitleDataList;
		}

	private:
		void SendAllNewTitleData();
		void SendAddTimeTitleId(WORD wId, int nTime);
		void SendAddNewTitleId(WORD wId, int nTime);
		void SendDelNewTitleId(WORD wId);
		void ClientCutOverNewTitleId(CDataPacketReader &packet);

		void SendAllCustomTitleData();
		void SendAddTimeCustomTitleId(WORD wId, int nTime);
		void SendAddCustomTitleId(WORD wId, int nTime);
		void SendDelCustomTitleId(WORD wId);
		void ClientCutOverCustomTitleId(CDataPacketReader &packet);
	private:
		bool isInit;
		CVector<NEWTITLEDATA> m_NewTitleDataList;
		CVector<NEWTITLEDATA> m_CustomTitleDataList;
		std::vector<int> m_nMaxTips;
		std::vector<int> m_nMaxCustomTips;
public:
	//跨服数据
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	VOID OnCrossInitData(std::vector<CNewTitleSystem::NEWTITLEDATA>& titles, std::vector<CNewTitleSystem::NEWTITLEDATA>& CustomTitles);
};
