#pragma once
#pragma  pack(4)

//玩家身上的兵魂表soulWeapon
struct HallowsSoulWeaponInfo
{
    int nId;            // 四个武器1-4
    int nStarId;        // 升星 
    int nLorderId;      // 升阶 
    int nLvId;          // 升级 
    
    //char cBestAttr1[50];
    HallowsSoulWeaponInfo()
    {
        memset(this, 0, sizeof(*this));
    }
};
struct SoulWeaponNetData
{
    int nId;            // 四个武器1-4
    int nStarId;        // 升星 
    int nLorderId;      // 升阶 
    int nLvId;          // 升级  
    char cBestAttr[50]; //极品属性
    SoulWeaponNetData()
    {
        memset(this, 0, sizeof(*this));
    }
};

#pragma  pack()