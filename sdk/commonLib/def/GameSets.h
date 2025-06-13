#pragma once
/****************************************************************************************/
/* 游戏设置DB相关的一些结构体的定义                                               
/****************************************************************************************/
#pragma  pack(4)
/// 游戏设置存盘的数据


typedef struct GAMESETSDATA
{
    char m_vKbs[500]; //快捷键

}*PGAMESETSDATA;

#define GAMESET_ITEMS_COUNT 1000

#define GAMESET_VERSION 3

typedef struct tagGameSets2
{
    int m_nVersion;                         // 设置的版本号（用以更新后设置默认）

    int m_nBase;                            // 基础设置

    short m_nSystem;                        // 系统设置

    short m_nMedicine;                      // 药品设置
    BYTE m_NmHp;                            // 强效金疮药百分比
    BYTE m_NmMp;                            // 太阳水百分比
    BYTE m_ImHp;                            // 强效太阳水百分比
    BYTE m_ImMp;                            // 万年雪霜百分比
    BYTE m_PerAdd1;                         // 疗伤药百分比
    BYTE m_PerAdd2;                         // 
    BYTE m_PerAdd3;                         // 
    BYTE m_PerAdd4;                         // 
    int m_NmHpVal;                          // 强效金疮药 剩余HP
    int m_NmHpTime;                         // 强效金疮药 剩余HP 间隔
    int m_NmMpVal;                          // 太阳水 剩余MP
    int m_NmMpTime;                         // 太阳水 剩余MP 间隔
    int m_ImHpVal;                          // 强效太阳水 剩余HP
    int m_ImHpTime;                         // 强效太阳水 剩余HP 间隔
    int m_ImMpVal;                          // 万年雪霜 剩余MP
    int m_ImMpTime;                         // 万年雪霜 剩余MP 间隔
    int m_ValAdd1;                          // 疗伤药 剩余MP
    int m_TimeAdd1;                         // 疗伤药 剩余MP 间隔

    int m_nHp1Val;                          // 血量1低于
    int m_nHp1Item;                         // 使用物品1
    int m_nHp2Val;                          // 血量2低于
    int m_nHp2Item;                         // 使用物品2
    short m_nProt;                          // 保护设置

    short m_nHook;                          // 挂机设置
    BYTE m_nAutoSkillID;                    // 自动施放的技能id
    BYTE m_nPet;                            // 自动召唤的宠物
    BYTE m_nHpLess;                         // 血量低于百分比
    BYTE m_nHpLessSkill;                    // 血量低于，使用技能

    int m_nRecycle[4];                         // 回收

    // 以下保持4字节对齐需要
    static const int nByte = (GAMESET_ITEMS_COUNT + 1 + 1)/4;
    static const int nIntCount = (nByte + 3) / 4;
    int m_nItems[nIntCount]; // 物品设置

}GAMESETSDATA2, *PGAMESETSDATA2;


//其他数据
struct GameUserDataOther
{
	int nGmLevel;     //玩家的GM等级
};


#pragma  pack()
