#pragma once

/************************************************************************/
/*
/*                             实体消息类
/*
/************************************************************************/
class CEntity;

class CEntityMsg
{
public:
	enum eEntityMsgID
	{
		emSkillResult = 1,		//技能的结果
		emShowEntityEffect,   	//添加目标特效
		emShowSceneEffect,     //添加场景特效
		emAppendEntityEffect,   //添加目标特效 TO DELETE
		emAppendSceneEffect,    //添加场景特效 TO DELETE
		emChangeHp,				//掉血，比如跳跃延迟掉血
		emGetQuestState,		//获取NPC对特定人物的任务状态
		emReUseMonster,			// 怪物重生
		emEntityTransfer,       // 实体的发生传送
		emEntityScriptDelay,	// 脚本延迟处理消息
		emAddDurEffect,         //在实体的身上添加持续的特效数据，考虑到特效的存储，持续的特效放到这
		emEntityDead,			// 实体死亡消息
		emRealTransfer,			//延迟一定时间后再传送
		emRealLaunchSkill,		//延时执行技能
		emSkillApplyOneRange,	//延时单范围技能效果
	};
public:
	int			 nMsg;		//消息号
	bool		 nForceCallWhenDestroy; // 销毁时强制执行。默认为0表示不执行；1表示实体销毁时强制执行
	bool		 bWaitDel;		// 消息等待删除，避免消息执行中嵌套删除消息
	bool         bIsUsed;       //是否正在使用中
	bool         bReserver;     //保留
	WORD		 wLine;			//行数
	CEntity*	 pOwner;		//消息的所有者，在释放和删除的时候使用，避免删除了别人的消息
	TICKCOUNT    dwDelay;	//生效时间 
	EntityHandle nSender;	//技能中使用，比如技能的发送者
	LPCSTR  file;    //申请的文件的位置
	union
	{
		struct 
		{
			INT_PTR nParam1;
			INT_PTR nParam2;
			INT_PTR nParam3;
			INT_PTR nParam4;
			INT_PTR nParam5;
			INT_PTR nParam6;
			INT_PTR nParam7;
			INT_PTR nParam8;
			INT_PTR nParam9;
			INT_PTR nParam10;
		};
		struct 
		{
			LPVOID pParam1;
			LPVOID pParam2;
			LPVOID pParam3;
			LPVOID pParam4;
			LPVOID pParam5;
			LPVOID pParam6;
			LPVOID pParam7;
			LPVOID pParam8;
			LPVOID pParam9;
			LPVOID pParam10;

		};
	};

public:
	CEntityMsg(INT_PTR msg)
	{
		memset(this, 0, sizeof(*this));

		nMsg = (int)msg;
	}
	CEntityMsg(INT_PTR msg, EntityHandle aSender)
	{
		memset(this, 0, sizeof(*this));
		nSender = aSender;
		nMsg = (int)msg;

	}
	CEntityMsg(INT_PTR msg, TICKCOUNT nDelay)
	{
		memset(this, 0, sizeof(*this));
		nMsg = (int)msg;
		dwDelay = nDelay;
	}
	inline CEntityMsg& operator = (const CEntityMsg &another)
	{
		memcpy(this, &another, sizeof(*this));
		return *this;
	}

	//设置申请的脚本的位置和行数
	inline void SetAllocFileLine(LPCSTR allocFile,INT_PTR nLine )
	{
		file =allocFile;
		wLine = (WORD)nLine;
	}

	//设置实体的所属的handle
	inline void SetOwnerPtr(CEntity * pEntity)
	{
		pOwner = pEntity;
	}
};
