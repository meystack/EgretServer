#pragma once
#include <vector>
#include <map>
typedef struct  TagRateCfg
{
    /* data */
    int nRate; //概率
    int nValue; //值
    TagRateCfg()
    {
        memset(this, 0, sizeof(*this));
    }
}RateCfg;


typedef struct TagBlessRate
{
    int nMaxRate; //权重
    std::vector<RateCfg> m_RateVectors; //比例

    int GetValue()
    {
        int nRand = wrand(nMaxRate);
        // printf("bless rate:%d\n",nRand);
        int nSize = m_RateVectors.size();
        int nAddRate = 0;
        for(int i = 0; i < nSize; i++)
        {
            nAddRate += m_RateVectors[i].nRate;
            if(nRand <= nAddRate)
            {
                // printf("bless rate:%d, addRate:%d\n",nRand,nAddRate);
                return m_RateVectors[i].nValue;
            }
        }

        return 0;
    }
    TagBlessRate()
    {
        nMaxRate = 0;
        m_RateVectors.clear();
    }
}BlessRate;

//祝福升级
typedef struct TagBlessCfg
{
    int nLevel;     // 等级
    int nNeedBlessValue; //当前等级所需祝福油
    int nLuckyValue;      //当前幸运值
    BlessRate m_nRate; //概率
    AttriGroup	attri;	//属性
    int m_nDailyCost;//每日
    TagBlessCfg()
    {
        memset(this, 0 ,sizeof(*this));
    }
}BlessCfg;

class CBlessCfg :public CLuaConfigBase
{
public:
    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;
	CBlessCfg();
	~CBlessCfg();
    //装载幸运配置
    bool LoadBlessCfg(LPCTSTR sFilePath);

    //通过祝福油获取最大等级
    BlessCfg* GetBlessCfg(int nBlessValue);

private:
	
	/*
	* Comments:装载祝福升级配置
	* @Return bool: 失败返回false
	*/
	bool ReadAllConfig();
    /*
	* Comments:装载祝福const配置
	* @Return bool: 失败返回false
	*/
    bool ReadBlessConst();

    
public:
    int      m_nItemId = 0;//道具id
    int      m_nDailyCost = 0; //每日扣除点数

    std::vector<BlessCfg> m_nBlessVectors; //祝福
    CDataAllocator m_DataAllocator;
};
