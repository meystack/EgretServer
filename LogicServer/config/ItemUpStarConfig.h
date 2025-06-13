#pragma once
#include<vector>
#include<map>
typedef struct tagRateConfig
{
    int nRate;
    int nValue;//
    tagRateConfig()
    {
        memset(this, 0, sizeof(*this));
    }
}RateConf;
//激活
typedef struct tagItemUpStarCfg 
{
    int nId;
    std::vector<ACTORAWARD> cost;
    std::vector<ACTORAWARD> recoverys;
    int nStarLv;//
    AttriGroup  attr;//属性
    int nMaxRate;//
    std::vector<RateConf> m_rates; //降级
    int nUpRate;//升星失败率
    tagItemUpStarCfg() {
        nId = 0;
        cost.clear();
        nStarLv = 0;
        recoverys.clear();
        m_rates.clear();
        nMaxRate = 0;
        nUpRate = 0;
    }
    
}ItemUpStarCfg;

typedef struct tagStarReplaceCfg{
    int nId;
    int nPrice;
    int nItemId;

    tagStarReplaceCfg()
    {
        memset(this, 0, sizeof(*this));
    }
}StarReplaceCfg;
class ItemUpStarConfig
:public CLuaConfigBase
{
public:

    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

	ItemUpStarConfig();
	~ItemUpStarConfig();

    bool LoadConfig(LPCTSTR sFilePath);

    //通过祝福油获取最大等级
    bool ReadAllConfig();
    StarReplaceCfg* getUpStareplaceCfg(int nItemId)
    {
        std::map<int, StarReplaceCfg>::iterator it = m_nReplaces.find(nItemId);
        if(it != m_nReplaces.end())
        {
            return &(it->second);
        }
        return NULL;
    }

    ItemUpStarCfg* getItemUpStarCfg(int nId, int nStar);
public:
    std::map <int, std::vector<ItemUpStarCfg> >  m_ItemUpStarMaps;
    CDataAllocator m_DataAllocator;
    std::map<int, StarReplaceCfg> m_nReplaces;

};



