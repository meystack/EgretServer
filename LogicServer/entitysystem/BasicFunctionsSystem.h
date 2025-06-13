#pragma once

class BasicFunctionsSystem:
	public CEntitySubSystem<enBasicFunctionsSystemID, BasicFunctionsSystem, CActor> 
{
	public:
		BasicFunctionsSystem();
		~BasicFunctionsSystem();

	public:
		virtual bool Initialize(void *data,SIZE_T size);
		VOID ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet);
		//合成
        void HandleComposeItem(CDataPacketReader& inPacket);
		//锻造
        void HandleForgeItem(CDataPacketReader& inPacket);
        void HandleForgeInfo(CDataPacketReader& inPacket);//本服锻造id
		//洗炼
		void HandleRefining(CDataPacketReader& inPacket);
		//洗炼替换
		void HandleRefiningReplace(CDataPacketReader& inPacket);
		//元宝回收
		void SendRecoverItemInfo();
		//升星
		void UpdateItemStar(CDataPacketReader& packet);

};
