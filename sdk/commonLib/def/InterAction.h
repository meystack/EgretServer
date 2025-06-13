#pragma once
//交互系统与数据服务器通信的数据包定义

//禁止编译器对此结构进行字节对齐
#pragma pack (push, 1)
	//保存这个玩家的副本的记录数据
	typedef struct tagFriendsData
	{
		BYTE				bRsType;	//关系类型，对应tagRelationShip的定义
		UINT				nActorId;	//对方的角色id
		jxSrvDef::ACTORNAME			szName;		//对方的名字
		UINT				nParam;		//用于操作的参数值，比如准备时结婚这个记录所在的副本的handle，准备离婚时这个记录了离婚的时间（7日）
		tagFriendsData()
		{
			nActorId = 0;
			szName[0] = 0;
			nParam = 0;
		}
	}FriendsData;
#pragma pack(pop)