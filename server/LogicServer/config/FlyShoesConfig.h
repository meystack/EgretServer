#pragma once
#include<vector>
#include<map>
/*******************************************************/
/*
/*					小飞鞋传送类
/*          IndexFlyShoesConfig存放button集合（见配置文件）
/*          FlyShoesConfig存放传送到具体地方的约束信息
/*   
/*******************************************************/



class CCustomLogicLuaConfig;
class CActor;
class CFuBen;

    //小飞鞋配置表1
    typedef struct IndexFlyShoesConfig{
        int nIndexItem ;  // 目录id
        int nCost;     // 消耗飞鞋数量
        //std::vector<int> Index_vec; //包含按钮id集合

    } FLYSHOESINDEX;

    //小飞鞋配置表2
    typedef struct FlyShoesConfig{
        int nButtonId ;              //按钮id
        int nDestination;		//目的地
    	int nPointRange[8]   ;   //传送范围，四个坐标点
        int nOpenLevel;        //开放等级
        int nOpenCircle;        //开放转身等级
        int nOpenDay;           //开放开服天数

        int nShowLevel;         //显示开放的等级要求
        int nShowCircle;        //显示转身等级要求
        int nShowDay;           //显示开服天数

        int nMoney;            //充值元宝限制
        int nNpcId;            //npcid
        
    
    }FLYSHOESCONFIG;

class CFlyShoes
:public CCustomLogicLuaConfig
{
public:





	CFlyShoes();
	~CFlyShoes();

	//接口，消耗小飞鞋
   // void ConsumeFlyshoes(CActor* pActor,int dest1,int dest2) ;


    bool ReadIndexConfig();
    bool ReadTableConfig();
    bool LoadIndex(LPCTSTR sFilePath);
    bool LoadTable(LPCTSTR sFilePath);

	//检查使用飞鞋传送条件是否满足
    //bool CheckTransmit(CActor* PActor,int dest1,int dest2,bool bOpenTips =1);



public:

    std::map <int,  struct IndexFlyShoesConfig >  m_Index;
    std::map< int, struct FlyShoesConfig > m_ButtonTable;

};









