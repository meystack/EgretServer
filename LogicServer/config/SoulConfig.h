#pragma once

#include "LuaConfigBase.h"
#include "attr/AttrDef.h"

/// 武魂一个级的属性
typedef LuaArray<GAMEATTR> LvProp;

typedef LuaArray<LvProp> Prop;

struct SoulCfg{
	Prop prop;
	IntArray lianHun;
};

/// 武魂系统的配置
class CSoulConfig :
	public CLuaConfigBase
{
	bool ReadPropOneLevel(LvProp& lvProp);
public:
	CSoulConfig(void);
	~CSoulConfig(void);
	
	virtual bool ReadAllConfig();

	SoulCfg soulCfg;
};
