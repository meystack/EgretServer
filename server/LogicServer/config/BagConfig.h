#pragma once

#define MAX_BAG_ENLARGE_GRIDS 49   //背包最大扩展格子数

typedef enum enSPEnlargeFee
{
	spefFeeType = 0,//扩大背包需要的金钱的类型
	spefFeeVal,		//扩大背包的增量费用
	spefItemCount,	//扩大背包需要的物品增量
	spefMaxEnlargeFee,
}SPENLARGEFEE;

typedef struct Forge
{
	int nLevel;
	int nType;
	Forge()
	{
		memset(this, 0, sizeof(*this));
	}
}ForGe;

typedef struct tagBagConfig
{
	int nMaxBagCount;    //背包的最大的格子数量
	int nOneClickRecycling;//一键回收最低等级
	int nDefaultBagCount[3]; //默认的背包的格子数量
	int nScrollEnterEffect[3]; //道具、装备、材料背包数量(主动获取)
	// std::vector<ForGe> forge;
	//int nEnLargeFeeArray[spefMaxEnlargeFee];	//扩大背包需要的金钱的类型, 扩大背包的增量费用,扩大背包需要的物品增量
	//int enlargeFeeArray[MAX_BAG_ENLARGE_TIMES]; //扩大背包的费用
	//int enlargeFeeTypeArray[MAX_BAG_ENLARGE_TIMES];  //扩大背包需要的金钱的类型
	//int nEnlargeTimes[MAX_BAG_ENLARGE_GRIDS];
	//unsigned int nEnlargeGiveExp[MAX_BAG_ENLARGE_GRIDS];
	//int nDeportBagGrid; //下发每个格子的数量
	//int nSmith1;  //第几次的精锻激活1个属性
	//int nSmith2;  //第几次的精锻激活2个属性
	//int nSmith3;  //第几次的精锻激活3个属性
	//int nDeportGridPrice; //仓库的格子价格
	//int nDeportPageTotalGridNum; //仓库的页面格子数量
	//int nDeportPageNum;			//仓库页面数量
	//int nDeportInitGridNum;		//仓库初始化格子数量

}BAGCONFIG,*PBAGCNFIG;
