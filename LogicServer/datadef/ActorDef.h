/*
* actor类里用的一些辅助的定义，放在这里
*/

#pragma once


struct CActorPacket
{
	CActor*		pActor;		// 申请数据包的角色对象
	CDataPacket *packet;
	INT64		nHdrPos;//追加本次数据的前，datapacket已有的数据长度
public:
	//提交数据包
	void flush();
	//撤销数据包
	void rollBack();

	template<typename T>
	inline CActorPacket& operator << (const T value)
	{
		*packet << value;
		return *this;
	}
};

/*
	本功能用于 角色A向角色B发送信息，角色B界面弹出一个对话框（有多个按钮），当角色B点击某个按钮后，执行相应的脚本函数
*/
//最大的按钮数量
#define		MAX_BUTTON_COUNT	3

typedef struct tagMessageBoxItem
{
	Uint64	hNpc;	//将要处理这消息的npc
	UINT	nActorid;	//接受这消息的角色id
	char	sFnName[MAX_BUTTON_COUNT][128];	//对应的按钮的
	BYTE	bButtonCount;	//实际是按钮数量
	//bool	boFlag;
	int		msgid;
	tagMessageBoxItem()
	{
		//boFlag = false;
		
		hNpc = 0;
		nActorid = 0;
		bButtonCount = 0;
	}

	DECLARE_OBJECT_COUNTER(tagMessageBoxItem)
}MessageBoxItem;

//对玩家播放的场景特效
typedef struct tagActorSrcEffect
{
	int			nEffctId;				//特效id
	int         nTime;					//特效到期时间
	int         nSceneId;				//播放的场景id
}ACTORSRCEFFECT;