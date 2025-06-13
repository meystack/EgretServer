#pragma once
#include<map>
struct StrengthInfo
{
    int nId; // 下表
    int nLv;// 等级
    StrengthInfo()
    {
        memset(this, 0, sizeof(*this));
    }
};
/*
强化系统
*/
class CStrengthenSystem :public CEntitySubSystem<enStrengthenSystemID, CStrengthenSystem, CActor>
{
    public:

        //初始化
        virtual bool Initialize(void* data, size_t size);
        void OnEnterGame();
        //处理db 数据返回
        virtual void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet);
        //存盘
        virtual void SaveToDb();

        //处理消息逻辑
        void ProcessNetData(INT_PTR cmd, CDataPacketReader& packet);

        void GetStrongInfo(CDataPacketReader& packet);
        void UpStrong(CDataPacketReader& packet);
        //获取当前强化信息
        StrengthInfo* GetStrongPtrInfo(int nType, int nPos);

        bool CheckStrengthenLv(int nType, int nPos, int nLv);

        void SetStrengthenLv(int nType, int nPos, int nLv);

        int GetStrongLv(int nType, int nPos);
        //属性
        VOID CalcAttributes(CAttrCalc &calc);

        //返回数据
        VOID returnStrongInfo(CDataPacket &data);

        //处理强化是否满足
        bool checkCanUPStrengthenLv(int nType, int nLv);
        //
        int AddlvByType(int nType);
        
        std::map<int, std::vector<StrengthInfo> >& GetStrongInfos() {
            return m_nStrengths;
        }
    private:
        std::map<int, std::vector<StrengthInfo> > m_nStrengths; //2四象  3暴击戒指 4麻痹戒指
        bool isInitData = false;
        bool isDataModify = false;
        typedef std::map<int, std::vector<StrengthInfo> >::iterator strongMapIter;
        typedef std::vector<StrengthInfo>::iterator strongVectorIter;
public:
    //跨服数据
    void SendMsg2CrossServer(int nType);
    //跨服数据初始化
	VOID OnCrossInitData(std::map<int, std::vector<StrengthInfo> >& nStrengths);
};
