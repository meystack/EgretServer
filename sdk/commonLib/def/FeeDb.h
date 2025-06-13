#pragma once
#pragma pack (push, 4)

struct FeeData
{
	unsigned int nId;		// 
	unsigned int nActorId;	// 玩家id
	char sPf[32];			// 平台
	char sProdId[48];		// 产品id
	unsigned int nNum;		// 数量
	ACCOUNT sAccount;

	//unsigned int nProdId;
};

#pragma pack(pop)
