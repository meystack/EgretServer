#pragma once

class CWorldLevelMgr :
	public CComponent
{
public:
	enum emLoadTopLevelActorType
	{
		enTopLevelWorldLevelSet=1,//设置世界等级
	};
	typedef struct tagWorldLevel
	{
	  	int   nWorldLevel;	//世界等级
		tagWorldLevel()
		{
			memset(this, 0, sizeof(*this));
		}
	}WORLDLEVELINFO, *PWORLDLEVELINFO;
public:
	CWorldLevelMgr(void);
	~CWorldLevelMgr(void);
	bool  Initialize();

	/*
	* Comments:数据服务器返回数据
	* Param INT_PTR nCmd:
	* Param INT_PTR nErrorCode:
	* Param CDataPacketReader & inPacket:
	* @Return VOID:
	*/

	VOID OnDbReturnData(INT_PTR nCmd,char * data,SIZE_T size);	
	//世界等级是否开放
	bool IsWorldLevelOpen();

	//人物是否能有世界等级加成
	bool IsCanHaveWorldLevelExp(int nActorLevel);

	//人物是否能显示世界等级图标
	bool IsWorldLevelIconOpen(int nActorLevel);

	/* 
	* Comments: 检查世界等级
	* Param CActor * pActor: 
	* Param bool isSend: 是否发送到客户端更新面板
	* @Return void:  
	*/
	void CheckWorldLevel(CActor* pActor, bool isSend = false);
	/* 
	* Comments: 重置世界等级
	* @Return void:  
	*/
	void ResetWorldLevel();

	/* 
	* Comments: 重新读取世界等级
	* @Return void:  
	*/
	void ReloadWorldLevel();

	//获取世界等级加成
	float getWorldLevelRate(int nLevel);

	//发送世界等级信息
	void SendAllActorWorldLevel();

	//发送世界等级信息
	void SendWorldLevel(CEntity* pEntity);


	//获得世界等级
	int GetWorldLevel() {return m_worldLevelInfo.nWorldLevel;}

private:
	/* 
	* Comments: 读取等级最高的玩家
	* @Return void:  
	*/
	void LoadTopLevelOneFromDb(int nType);
	void LoadTopLevelFromDbResult(int nType, CDataPacketReader& inPacket);


private:
	WORLDLEVELINFO			m_worldLevelInfo;	//世界等级
};

