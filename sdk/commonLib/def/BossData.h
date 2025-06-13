#pragma once

struct CMiniDateTime;

#pragma pack (push, 4)

struct GLOBALBOSSINFO
{
    int  nBossId;      // bossId
    int  nReferTime;   // 刷新时间
    int  nDeath;       //是否死亡
    int  nBossLv;      //boss等级
    GLOBALBOSSINFO() {
        memset(this, 0, sizeof(*this));
    }
};

#pragma pack(pop)
