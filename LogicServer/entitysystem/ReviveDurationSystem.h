#pragma once
#include <map>

class CReviveDurationSystem:
	public CEntitySubSystem<enReviveDurationSystemID, CReviveDurationSystem, CActor> 
{
	public:
		typedef CEntitySubSystem<enReviveDurationSystemID, CReviveDurationSystem, CActor> Inherited;

		typedef struct tagReviveDurationDATA
		{
			WORD			wId;
			unsigned int	nRemainTime;
			tagReviveDurationDATA()
			{
				ZeroMemory(this,sizeof(tagReviveDurationDATA));
			};
		}ReviveDurationDATA;
	public:
		CReviveDurationSystem();
		~CReviveDurationSystem();

	public:
		virtual bool Initialize(void *data,SIZE_T size);
		virtual void Destroy();
		virtual VOID OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader);
		virtual VOID OnTimeCheck(TICKCOUNT nTickCount);
 
	public:
		
		void LoadFromDB();
		void SaveToDB();
		int IsHaveReviveDuration( WORD wId );
		void AddReviveDuration( WORD wId );
		void DelReviveDuration( WORD wId );
	
	private:
		CVector<ReviveDurationDATA> m_ReviveDurationDataList;
public:
	//跨服相关
	void SendMsg2CrossServer(int nType);
	VOID OnCrossInitData(std::vector<ReviveDurationDATA>& titles);
};
