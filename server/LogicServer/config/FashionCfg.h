#pragma once
#include<vector>
#include<map>


//消耗

typedef struct tagFashionCost {
    int nId;
    int nType;
    int nCount;
    tagFashionCost()
    {
        memset(this, 0, sizeof(*this));
    }
}FashionCost;

//激活
typedef struct tagFashionActive{
    int nId;
    int nType;
    int nDisplay;
    int nBack;
    int nWear;//
    std::vector<tagFashionCost> cost;
    
    tagFashionActive()
    {
        nId = 0;
         nType = 0;
         nDisplay = 0;
        nBack = 0;
        nWear = 0;
        cost.clear();
    }
    

}FashionActive;

typedef struct tagFashionAttribute{
    int nId;
    int nLv;
    std::vector<tagFashionCost> cost;
    AttriGroup	attri;	//属性
    tagFashionAttribute()
    {
        nId = 0;
        nLv = 0;
        memset(&(this->attri), 0, sizeof(this->attri));
        cost.clear();
    }
    

}FashionAttribute;

class CFashionCfg
:public CLuaConfigBase
{
public:

    typedef CLuaConfigBase Inherited;
    typedef CObjectAllocator<char> CDataAllocator;

	CFashionCfg();
	~CFashionCfg();

    bool LoadFashionCfg(LPCTSTR sFilePath);

    //通过祝福油获取最大等级
    bool ReadAllConfig();
    bool ReadTableConfig();

    FashionActive* getFashionActive(int nId);

    tagFashionAttribute* getFashionUpdate(int nId, int nLv);
public:
    int nOpenLv; //开放等级
    int nOpenDay; //开放天数
 
    std::map <int, tagFashionActive>  m_mActive;
    std::map< int, std::vector<tagFashionAttribute > > m_mAttribute;
     CDataAllocator m_DataAllocator;
    std::map<int, std::vector<int> > takeofflist;   
};



