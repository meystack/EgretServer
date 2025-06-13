#pragma once
/*
玩家宝物功能系统
*/
class CTreasureSystem :public CEntitySubSystem<enTreasureSystem, CTreasureSystem, CActor>
{
    public:

        //初始化
        virtual bool Initialize(void* data, size_t size);
        void OnEnterGame();

        //处理消息逻辑
        void ProcessNetData(INT_PTR cmd, CDataPacketReader& packet);

        void ActorBless(CDataPacketReader& pack);
        //每天刷新
        void RefreshBless(int nDay = 1);
};