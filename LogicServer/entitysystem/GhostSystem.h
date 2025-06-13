#pragma once
#include<map>
class CGhostSystem :public CEntitySubSystem<enGhostSystemID, CGhostSystem, CActor>
{
    public:

        //初始化
        virtual bool Initialize(void* data, size_t size);
        //处理db 数据返回
        virtual void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet);
        //存盘
        virtual void SaveToDb();

        //处理消息逻辑
        void ProcessNetData(INT_PTR cmd, CDataPacketReader& packet);
        
        //转生升级
        void upGhostLv(CDataPacketReader& packet);
        void sendGhostInfo();
        int GetLvByPos(int nPos);
        int GetBlessByPos(int nPos);
        void SetLvByPos(int nPos, int nLevel, int nBless);

        void CalcAttributes(CAttrCalc &calc);

        void returnPb(int nPos, CDataPacket& data);
        void Data2Packet(CDataPacket& dataPack);
        void AddBlessByPos(int nPos, int nAddValue);
        //数值奖励--接口
        void GetAwardNum(int nAddValue);
        //满等级发奖
        void SendAward();
        int GetSumLv();
        std::map<int, GhostData>& getGhostLists() {
            return m_nGhostLvs;
        } 
    private:
        std::map<int, GhostData> m_nGhostLvs;
        bool isInitData = false;
        bool isDataModify = false;
public:
    //跨服数据
    void SendMsg2CrossServer(int nType);
    //跨服数据初始化
	VOID OnCrossInitData(std::vector<GhostData>& mGhost);
};
