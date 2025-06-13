#pragma once
#include<vector>
#include<map>

//消耗

//激活
typedef struct tagRefiningCfg{
    int nId;
    std::vector<ACTORAWARD> cost;
    int nRefineId;
    tagRefiningCfg()
    {
        nId = 0;
        nRefineId = 0;
        cost.clear();
    }
    

}RefiningCfg;

typedef struct tagReplaceCfg{
    int nId;
    int nPrice;
    int nItemId;

    tagReplaceCfg()
    {
        memset(this, 0, sizeof(*this));
    }
    

}ReplaceCfg;

class CRefiningCfg
:public CLuaConfigBase
{
public:

    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

	CRefiningCfg();
	~CRefiningCfg();

    bool LoadConfig(LPCTSTR sFilePath);

    //通过祝福油获取最大等级
    bool ReadAllConfig();

    RefiningCfg* getRefiningCfg(int nId);

    ReplaceCfg* getReplaceCfg(int nItemId)
    {
        std::map<int, ReplaceCfg>::iterator it = m_nReplaces.find(nItemId);
        if(it != m_nReplaces.end())
        {
            return &(it->second);
        }
        return NULL;
    }
    void GetRefineAttribute(int type, DROPGOODS& item);
public:
    int nVoucher2; //元宝兑换单个祖玛凭证价格
    int nRefininglevel; //洗炼页签开放等级
    int nVoucher1; //元宝兑换单个沃玛凭证价格
    int nMaterialprice; //元宝兑换单个洗炼石价格
    std::map <int, RefiningCfg>  m_nReiningCfgs;
    std::map <int, ReplaceCfg>  m_nReplaces;
    std::map<int, DROPATTRIBUTECFG> m_RefingAttributeMaps; //洗炼属性
    CDataAllocator m_DataAllocator;

};



