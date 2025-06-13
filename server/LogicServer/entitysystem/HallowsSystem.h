
/************************************************************************/
/*                           圣物系统
/*
/************************************************************************/
#pragma once
#include<map>
#include<HallowsDef.h>

enum HallowsTotleType  
{
    eHallowsTotle_NULL = 0,
    eHallowsTotle_SoulWeapon = 1,    //兵魂  
    eHallowsTotle_MAX,
};

struct HallowsLorderInfo; 
struct WpRefiningInfo;
//圣物系统
class CHallowsSystem :public CEntitySubSystem<enHallowsSystemID, CHallowsSystem, CActor>
{
public:
    CHallowsSystem() {};
    virtual ~CHallowsSystem();  
public:
    //初始化
    virtual bool Initialize(void* data, size_t size);
    void OnEnterGame();

    /*db 数据*/ 
    //存盘
    virtual void OnSaveToDb();
    //返回
    virtual void OnDbRetData(INT_PTR nCmd, INT_PTR nErrorCode, CDataPacketReader& packet);

    //处理客户端消息
    void ProcessNetData(INT_PTR cmd, CDataPacketReader& packet); 
    
    //获取圣物信息
    HallowsSoulWeaponInfo* GetHallowsPtrInfo(int nType, int nPos);

    //查询接口
    //刷新属性
    VOID CalcAttributes(CAttrCalc &calc);
    //离线数据
    VOID OnReturnHallowsSoulWeaponInfo(CDataPacket &data);
    //战斗查询
    HallowsLorderInfo* GetHallowsLoderv(int nType, int nPos);
    
    //所有数据
    std::map<int, std::map<int, HallowsSoulWeaponInfo> >& GetHallowsSoulWeaponInfos()
    {
        return m_nHallowsSoulWeapon;
    }
private:  
    /*第一个参数默认是兵魂*/
    int GetHallowsLv(int nType, int nPos, int nSubType);
    //圣物升级
    int AddlvByType(int nType, int nPos, int nSubType);
    //圣物降级//只有星级可以降低
    int ReducelvByType(int nType, int nPos, int nSubType, int nLeve); 

    //消息分发
    void OnCustomGetSoulWeaponInfo(CDataPacketReader& packet);  //查询
    void OnCustomUpHallows(CDataPacketReader& packet);          //升级 
    void OnCustomRefining(CDataPacketReader& packet);           //洗练
    void OnCustomRefiningReplace(CDataPacketReader& packet);    //洗练替换

    //升级限制 没用到
    bool CheckHallowsthenLv(int nType, int nPos, int nLv);
    //生成洗练参数
    void GetRefineAttribute(WpRefiningInfo* pCfg, DROPGOODS& item);
//跨服相关
public:
    void OnCrossInitData( std::vector<SoulWeaponNetData>& vSoulWeapon);
    void SendMsg2CrossServer(int nType);
private:
    std::map<int, std::map<int, HallowsSoulWeaponInfo> > m_nHallowsSoulWeapon;
    bool m_isInitData = false;
    bool m_isDataModify = false; 
    
    std::map<int, char*> m_vcRefining;
    std::map<int, char*> m_vcBestAttr;  
};
