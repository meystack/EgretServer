#pragma once
 
#include "ItemUpStarConfig.h"  

enum  
{
    eHallowsReplaceType_NULL = 0,     
    eHallowsReplaceType_Star = 1,       //升星  
    eHallowsReplaceType_Refining = 2,   //洗练  
    eHallowsReplaceType_MAX, 
};
 
enum HallowsSSCType //兵魂参数
{
    eHallowsSSC_NULL = 0,     
    eHallowsSSC_JZLR = 1,    //救主灵刃 
    eHallowsSSC_JDCJ,        //剧毒裁决 
    eHallowsSSC_SZAS,        //霜之哀伤 
    eHallowsSSC_XYGD,        //救主灵刃 
    eHallowsSSC_MAX, 
};
 
enum HallowsUpType //升级类型参数
{
    HallowsUpgrade_NULL = 0,     
    HallowsUpgrade_Lorder = 1,  //升阶 
    HallowsUpgrade_Star,        //升星 
    HallowsUpgrade_Lv,          //升级 
    HallowsUpgrade_Refining,    //洗炼 
    HallowsUpgrade_MAX, 
};
struct HallowsSSCMainInfo
{
    int nSaviorPercen;
    int nFrozenAddValue;
    int nReduceAttack;
    int nFrozenTime;
    int nToBuffResidueHP; 
    //int nReduceAttackMon; 
    int nMaxFrozen;
    int nTXJZLR;
	int nTXJDCJ;
	int nTXSZAS;
    int nFrozenBuffId;
    int nTXResidueHP;
    int nFrozenTimes;
    std::vector<int> vReduceAttackMon;
    std::vector<int> vPoisonBuff;
    std::vector<int> vJZLRBuff;
    HallowsSSCMainInfo()
    { 
        nSaviorPercen = 0;
        nFrozenAddValue = 0;
        nReduceAttack = 0;
        nFrozenTime = 0;
        nToBuffResidueHP = 0;
        
        nMaxFrozen = 0;
        nTXJZLR = 0;
        nTXJDCJ = 0;
        nTXSZAS = 0;
        nFrozenBuffId = 0;
        nTXResidueHP = 0;
        nFrozenTimes = 0;
        vReduceAttackMon.clear();
        vPoisonBuff.clear();
        vJZLRBuff.clear();
    }
}; 
//强化配置
struct HallowsthCost
{
    int nId;//道具id
    int nCount;//num
    int nType;//道具类型
    HallowsthCost()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct HallowsUpLimit
{
    int nPos;//位置
    int nLimitlv;//等级
    HallowsUpLimit()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct HallowsLorderInfo
{
    //int nId;        //分类
    int nPos;       //位置
    int nLv;        //等级
 
    std::vector<HallowsUpLimit> limit;
    std::vector<int> vLorderBuff;
    std::vector<HallowsthCost> cost;
    AttriGroup		attri;	//属性 
    HallowsLorderInfo()
    {
        //nId = 0;
        nPos = 0;
        nLv = 0; 
        limit.clear(); 
        cost.clear();  
        vLorderBuff.clear();   
    }
};
struct HallowsLvInfo
{
    //int nId;        //分类
    int nPos;       //位置
    int nLv;        //等级
 
    std::vector<HallowsUpLimit> limit;
    std::vector<HallowsthCost> cost;
    AttriGroup		attri;	//属性  
    HallowsLvInfo()
    {
    //    nId = 0;
        nPos = 0;
        nLv = 0; 
        limit.clear(); 
        cost.clear();   
    }
}; 
struct HallowsStarInfo
{
    //int nId;        //分类
    int nPos;       //位置
    int nLv;        //等级
    int nReducestar;
    int nUpRate;    //升星失败率
    int nMaxRate;   //
    std::vector<RateConf> m_rates; //降级
  
    std::vector<HallowsUpLimit> limit;
    std::vector<HallowsthCost> consume;
    AttriGroup		attri;	//属性   
    HallowsStarInfo()
    {
        //nId = 0;
        nPos = 0;
        nLv = 0;
        nReducestar = 0;  
        limit.clear(); 
        consume.clear(); 
    }
};
struct HallowDropAttribute
{
	uint8_t     nRate = 0;
	uint8_t     nType = 0;
	int32_t     nValue = 0;
	uint8_t     nFollowrate = 0;
	uint8_t     nFollowcount = 0;
};

struct WpRefiningInfo
{
    //int nId;        //分类
    //int nPos;       //位置
    int nType;       
	uint8_t nRate;
  
    std::vector<HallowsUpLimit> limit;
    std::vector<HallowsthCost> cost;
	std::vector<DropAttribute> vAttribute;
    AttriGroup		attri;	//属性  
    WpRefiningInfo()
    { 
        nType = 0;
        nRate = 0; 
        cost.clear(); 
        limit.clear(); 
		vAttribute.clear();
    }
}; 
//洗练元宝替换表
struct WpReplaceInfo
{
    int type;
    int nPos;       //位置
    //int nId;
    //int nPrice;
    int nItemId;
    int idx; 

    std::vector<HallowsthCost> vPrice;
    WpReplaceInfo()
    { 
        type = 0;
        nPos = 0; 
        nItemId = 0; 
        idx = 0; 
        vPrice.clear();    
    }
};

class CHallowsthenCfg:protected CLuaConfigBase
{
public:
    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

    typedef std::map<int, WpReplaceInfo> ReplacesItems;
    typedef std::map <int, ReplacesItems > ReplacesMap;

public:
    CHallowsthenCfg() {};
    virtual ~CHallowsthenCfg() {};
    bool ReadAllConfig() {};
    //读取配置文件
    bool LoadAll(LPCSTR FileName); 
    
    //获得兵魂总数
    int OnGetSoulWeapon();

    bool OnCheckLorderInfo(int nPos, int Level);
    bool OnCheckLvInfo(int nPos, int Level);
    bool OnCheckStarInfo(int nPos, int Level); 
    //查询
    HallowsLorderInfo* OnGetLorderInfo(int nPos, int Level);
    HallowsLvInfo* OnGetLvInfo(int nPos, int Level);
    HallowsStarInfo* OnGetStarInfo(int nPos, int Level);
    WpRefiningInfo* OnGetWpRefiningInfo(int nPos);
    
    //WpReplaceInfo* getReplaceCfg(int nItemId)
    //{
    //    std::map<int, WpReplaceInfo>::iterator it = m_nReplaces.find(nItemId);
    //    if(it != m_nReplaces.end())
    //    {
    //        return &(it->second);
    //    }
    //    return NULL;
    //} 
    WpReplaceInfo* getReplaceCfgByPos(int type, int pos, int nItemId)
    {
        if( eHallowsReplaceType_Star != type
            && eHallowsReplaceType_Refining != type)
        {
            return NULL;
        }
        std::map <int, ReplacesMap>::iterator it = m_nReplaces.find(type);
        if(it != m_nReplaces.end())
        {
            std::map <int, ReplacesItems >::iterator itsub = it->second.find(pos);
            if(itsub != it->second.end())
            { 
                std::map<int, WpReplaceInfo>::iterator itItem = itsub->second.find(nItemId);
                if(itItem != itsub->second.end())
                {
                    return &(itItem->second);
                }
            } 
        }
        return NULL;
    }
    
private:
    bool readSoldierSoulMain();

    bool readWeaponLorderConfig();
    void readWeaponLorderData(HallowsLorderInfo& info);

    bool readWeaponLvConfig();
    void readWeaponLvData(HallowsLvInfo& info);

    bool readStarConfig();
    void readWeaponStarData(HallowsStarInfo& info);

    bool readWpRefiningConfig();
    bool readWpReplaceConfig();
    void readWpReplaceData(WpReplaceInfo& info);
   
    /*
    HallowsthenInfo* getPstrengthen(std::map<int, std::vector<HallowsthenInfo> >& infos, int nPos, int nLevel, int nJob = 0);
    //特戒
    HallowsthenInfo* getRingtrengthen(std::map<int, std::map<int , std::vector<HallowsthenInfo> > >& infos, 
                                    int nPos, int nLevel, int nJob);
                                    */
public: 
    //兵魂的属性
    HallowsSSCMainInfo m_HallowsSSCMainInfo;
    std::map<int, std::map<int, HallowsLorderInfo> > m_LorderInfo; //阶
    std::map<int, std::map<int, HallowsLvInfo> > m_LvInfo;         //级
    std::map<int, std::map<int, HallowsStarInfo> > m_StarInfo;     //星 
    std::map<int, WpRefiningInfo> m_WpRefiningInfo;                //洗练
    std::map <int, ReplacesMap>  m_nReplaces;

    CDataAllocator m_DataAllocator; //内存分配器
    //typedef std::map<int, std::vector<HallowsthenInfo> >::iterator vHallowsthenIter;
    //typedef std::map<int, std::map<int, std::vector<HallowsthenInfo> > >::iterator mHallowsthenIter;
};
