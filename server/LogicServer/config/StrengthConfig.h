#pragma once
//强化配置
struct StrengthCost
{
    int nId;//道具id
    int nCount;//num
    int nType;//道具类型
    StrengthCost()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct StrengthUpLimit
{
    int nPos;//位置
    int nLimitlv;//等级
    int zsLevel; 
    StrengthUpLimit()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct StrengthenInfo
{
    int nPos; //位置
    int nLv;//等级
    int nJob;//职业
    std::vector<StrengthCost> cost;
    AttriGroup		attri;	//属性
    std::vector<StrengthUpLimit> limit;
    int nId;
    char nTips[1024];
    StrengthenInfo()
    {
        nPos = 0;
        nLv = 0;
        cost.clear();
        limit.clear();
        nId = 0;
        nJob = 0;
        memset(this->nTips, 0, sizeof(this->nTips));
    }
};

class CStrengthenCfg:protected CLuaConfigBase

{
public:
    enum strengthenType
    {
        StrengthenEquip =1,//装备
        StrengthenFourStar = 2, //四象
        StrengthenRing = 3, //特戒
        StrengthenDizzyRing = 4, //麻痹戒指
        StrengthenWords = 5, //字诀
    };
    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

    CStrengthenCfg() {};
    ~CStrengthenCfg() {};
    bool ReadAllConfig() {};
    //读取配置文件
    bool LoadStrengthenConfig(LPCSTR FileName);
    
    void read(LPCSTR Table, std::map<int, std::vector<StrengthenInfo> >& infos);
    void readCfg(LPCSTR Table, std::map<int, std::vector<StrengthenInfo> >& infos);

    void readRing(LPCSTR Table, std::map<int, std::map<int, std::vector<StrengthenInfo> > >& infos);
    //其他
    StrengthenInfo* getPstrengthenCfg(int nType, int nPos, int Level, int nJob = 0);
    

private:
    void readEquipStrengConfig();
    void readFourStarsConfig();
    void readRingConfig();
    void readDizzyRingConfig();
    void readWordsConfig();         // 字诀
    //
    void readData(StrengthenInfo& info);
    
    StrengthenInfo* getPstrengthen(std::map<int, std::vector<StrengthenInfo> >& infos, int nPos, int nLevel, int nJob = 0);
    //特戒
    StrengthenInfo* getRingtrengthen(std::map<int, std::map<int , std::vector<StrengthenInfo> > >& infos, 
                                    int nPos, int nLevel, int nJob);
public:
    std::map<int, std::vector<StrengthenInfo> > m_EquipUpCfg; //位置强化
    std::map<int, std::vector<StrengthenInfo> > m_FourStarsCfg; //四象强化
    std::map<int, std::vector<StrengthenInfo> > m_RingsCfg; // 特戒强化
    std::map<int, std::map<int, std::vector<StrengthenInfo> > > m_DizzyRingsCfg; // 麻痹
    std::map<int, std::vector<StrengthenInfo> > m_WordsCfg; //字诀
    CDataAllocator m_DataAllocator; //内存分配器
    typedef std::map<int, std::vector<StrengthenInfo> >::iterator vStrengthenIter;
    typedef std::map<int, std::map<int, std::vector<StrengthenInfo> > >::iterator mStrengthenIter;
};
