#pragma once

/**************************


	实现场景操作相关的类


****************************/

using namespace jxcomm::gameMap;
using namespace Util;

#define			DEATHTIME		1	//这个是怪物死亡后显示一段时间

class CEntityGrid;
class CScene;
class CLogicEngine;
class Util::AStar;

class jxcomm::gameMap::CAbstractMap;

class CScene : public Counter<CScene>,
	public VirturalMap
{
	friend class CFuBen;
	friend class CEntityGrid;	
public:
	CScene();
	virtual ~CScene();
	
public:
	/*
	* Comments: 场景克隆
	* Param CScene & DstScene: 克隆场景
	* Param bool boRun: 
	* @Return VOID:
	* @Remark: 克隆刷怪点列表
	*/
	void Clone(CScene& DstScene,bool boRun=true);

	/*
	* Comments://初始化操作，比如生成各个实体
	* Param SCENECONFIG * pData:场景的配置文件
	* Param CAbstractMap * pMapData:场景的地图文件
	* Param CFuBen * pFb:场景所在的副本指针
	* @Return BOOL:是否初始化成功
	*/
	BOOL Init(SCENECONFIG* pData, 
			  CAbstractMap* pMapData, 
			  CFuBen* pFb,
			  CObjectAllocator<char>* dataAlloc = NULL,
			  bool boRun = true);	

	/// 获取默认的进入点（一般是没法找到进入点时，才会使用该默认点）
	void GetDefaultEnterPos(int &x, int &y)
	{ 
		x = m_pSceneData->nDefaultX;
		y = m_pSceneData->nDefaultY;
	}

	/*
	* Comments: 设置怪物配置
	* Param CEntity * pEntity: 怪物实体对象指针
	* Param INT_PTR bornPoint: 怪物在场景中的刷新点索引（PROP_MONSTER_BORNPOINT）
	* Param INT_PTR nMonsterId: 怪物Id
	* @Return void:
	* @Remark:
	*/
	void SetMonsterConfig( CEntity* pEntity, INT_PTR bornPoint, INT_PTR nMonsterId, INT_PTR nBornLevel);

	// 寻路
	bool GetPath(CEntity* pEntity, CVector<SMALLCPOINT> &outPaths, int targetX, int targetY);

	/*
	* Comments:返回这个场景的特殊区域的中心点，通常用于传送
	* Param int & nPosX:返回的X坐标
	* Param int & nPosY:返回的Y坐标
	* Param int nIndex:每个场景会有多个这样的传送点，nIndex代表索引值
	* @Return BOOL:如果有正确的点返回，则返回TRUE，否则返回FALSE（如索引值超出）
	*/
	inline bool GetSceneAreaCenter(INT_PTR& nPosX,INT_PTR& nPosY,INT_PTR nIndex =0)
	{
		if (!m_pSceneData)
		{
			return false;
		}
		if (nIndex < 0 || m_pSceneData->vAreaList.pList == NULL 
			|| nIndex >= m_pSceneData->vAreaList.nCount)
			return false;

		SCENEAREA* pArea = m_pSceneData->vAreaList.pList + nIndex;
		nPosX = pArea->Center[0];
		nPosY = pArea->Center[1];
		return true;
	}

	//调试场景
	void Trace();


	/*
	* Comments:场景里一个实体能否通过
	* Param int x:坐标x
	* Param int y:坐标x
	* Param int nParam: 附加参数1
	* Param int nParam2: 附加参数2
	* @Return bool:实体能否
	*/
	virtual bool canEntityMove(int x,int y,int nParam,int nParam2, int nParam3=0, int nParam4=0)
	{
		return CanCross(nParam,x,y,nParam2?true:false, nParam3?true:false);
	}


	/*
	* Comments: 设置刷新列表
	* Param REFRESHLIST * list:
	* @Return void:
	* @Remark:
	*/
	void SetRefreshList(REFRESHLIST* list);
	
	//设置场景的npc是否可见
	void SetNpcCanSeeFlag(bool bSet); 

	/*
	* Comments:判断能否进入这个场景
	* Param CActor * pActor:
	* @Return bool:
	*/
	bool CanEnter(CActor* pActor,INT_PTR x,INT_PTR y,bool bState);

	//判断是否是押镖，押镖不能进入活动和副本
	bool CanPassScence(CActor* pActor);

	/*
	* Comments:进入场景的指定位置
	* Param CEntity * pEntity:
	* Param INT_PTR nPosX:
	* Param INT_PTR nPosY:
	* @Return BOOL:
	*/
	bool EnterScene(CEntity * pEntity,INT_PTR nPosX,INT_PTR nPosY, int nType = 0 ,int nValue = 0 , int nParam=-1);

	/*
	* Comments:进入场景的指定位置半径范围的某个随机点
	* Param CEntity * pEntity:
	* Param INT_PTR nPosX:	X坐标
	* Param INT_PTR nPosY:	Y坐标
	* Param INT_PTR nWeight: 宽半径
	* Param INT_PTR nHeight: 高半径
	* @Return BOOL:
	*/
	bool EnterSceneRandPt(CEntity *pEntity, INT_PTR nPosX, INT_PTR nPosY, INT_PTR nWeight, INT_PTR nHeight, int nType = 0,
		int nValue = 0, int nParam=-1);

	/*
	* Comments: 实体退出场景
	* Param CEntity * pEntity:
	* @Return void:
	* @Remark: 更新场景的实体列表并且从网格中删除此实体。对于怪物需要更新场景刷怪点动态数据
	*/
	void ExitScene(CEntity * pEntity);
		
	/*
	* Comments: 实体移动处理
	* Param CEntity * pEntity: 实体对象指针
	* Param INT_PTR nDir: 移动方向
	* Param INT_PTR nSpeed: 移动速度
	* @Return BOOL: 移动成功返回TRUE；否则返回FALSE
	* @Remark:
	*/
	inline BOOL Move(CEntity* pEntity, INT_PTR nDir, INT_PTR nSpeed = 1)
	{
		return m_Grid.Move(pEntity,nDir,nSpeed) == seSucc;
	}

	/*
	* Comments:判断是否有穿人穿怪的限制
	* Param INT_PTR x:
	* Param INT_PTR y:
	* @Return bool:如果有限制，并且这个位置有人或怪，返回false
	*/
	inline bool CanCross(INT_PTR nEntityType,INT_PTR x, INT_PTR y,bool bCanAlwaysEnter = false,bool bCanEnterFire = false)
	{
		return m_Grid.CanCross(nEntityType,x,y,bCanAlwaysEnter,bCanEnterFire);
	}
	// inline bool CanCross(INT_PTR nEntityType,INT_PTR x, INT_PTR y)
	// {
	// 	return m_Grid.CanCross(nEntityType,x,y,false,false);
	// }

	/*
	* Comments: 直接移动到某个坐标
	* Param CEntity * pEntity:
	* Param INT_PTR nX:
	* Param INT_PTR nY:
	* @Return BOOL:
	* @Remark:
	*/
	inline BOOL MoveTo(CEntity* pEntity, INT_PTR nX, INT_PTR nY, bool canAwaysEnter = false)
	{
		return m_Grid.MoveTo(pEntity, nX, nY, false, canAwaysEnter) == seSucc;
	}
	
	/*
	* Comments: 返回指定实体周围指定范围内可见实体列表
	* Param CEntity * pEntity:
	* Param EntityVector & vVisibleList:
	* Param int nLeft:
	* Param int nRight:
	* Param int nUp:
	* Param int nDown:
	* @Return bool:
	* @Remark:
	*/
	bool GetVisibleList(CEntity * pEntity,EntityVector& vVisibleList, 
						int nLeft=-MOVE_GRID_COL_RADIO,
						int nRight=MOVE_GRID_COL_RADIO,
						int nUp=-MOVE_GRID_ROW_RADIO,
						int nDown=MOVE_GRID_ROW_RADIO);

	/*
	* Comments: 获取指定位置指定范围的周围实体列表
	* Param int nPosX: 位置X坐标
	* Param int nPosY: 职位Y坐标
	* Param EntityVector & vVisibleList: 返回副本实体列表
	* Param int nLeft: 
	* Param int nRight:
	* Param int nUp:
	* Param int nDown:
	* @Return bool: 成功返回true；失败返回false
	* @Remark: 这里可见并非指得是实体可见属性，而是是否在这个指定范围。在范围内就是可见，不在范围内就是不可见
	*/
	bool GetVisibleList(int nPosX, int nPosY,
						EntityVector& vVisibleList,
						int nLeft = -MOVE_GRID_COL_RADIO,
						int nRight = MOVE_GRID_COL_RADIO,
						int nUp = -MOVE_GRID_ROW_RADIO,
						int nDown = MOVE_GRID_ROW_RADIO);

	/*
	* Comments: 返回指定以指定位置为中心的指定区域内的可见实体列表
	* Param int nPosX: 中心点X坐标
	* Param int nPosY: 中心点Y坐标
	* Param CObserverEntityList & vVisibleList:
	* Param int nLeft:
	* Param int nRight:
	* Param int nUp:
	* Param int nDown:
	* Param bool * pbActorInVisibleList:
	* @Return bool:
	* @Remark:
	*/
	bool GetVisibleList(int nPosX,int nPosY,
						CObserverEntityList& vVisibleList,
						int nLeft = -MOVE_GRID_COL_RADIO,
						int nRight = MOVE_GRID_COL_RADIO,
						int nUp = -MOVE_GRID_ROW_RADIO,
						int nDown = MOVE_GRID_ROW_RADIO, 
						bool *pbActorInVisibleList = NULL);

	/*
	* Comments:返回指定以指定位置为中心的指定区域内的有序可见实体列表
	* Param int nPosX:
	* Param int nPosY:
	* Param SequenceEntityList & visibleList:
	* Param int nLeft:
	* Param int nRight:
	* Param int nUp:
	* Param int nDown:
	* @Return bool:
	* @Remark:
	*/
	bool GetSeqVisibleList(int nPosX,int nPosY, 
						SequenceEntityList& visibleList, 
						int nLeft = -MOVE_GRID_COL_RADIO, 
						int nRight = MOVE_GRID_COL_RADIO, 
						int nUp = -MOVE_GRID_ROW_RADIO, 
						int nDown = MOVE_GRID_ROW_RADIO);

	/*
	* Comments:清理无用的实体handle
	* Param int nLeft:坐标参数含义跟GetVisibleList一样
	* Param int nRight:
	* Param int nUp:
	* Param int nDown:
	* @Return void:
	*/
	void ClearIncorrectHandle(CEntity * pEntity,int nLeft=-MOVE_GRID_COL_RADIO,int nRight=MOVE_GRID_COL_RADIO
		,int nUp=-MOVE_GRID_ROW_RADIO,int nDown=MOVE_GRID_ROW_RADIO);

	
	/* 
	* Comments: 创建一个实体并加入到到场景中，成功返回实体指针（实体包括怪物、传送门、采集物品等）
		参数nEntityId是实体id号，对应配置表的id,type代表实体的类型（如怪物、传送门等，Actor应该不在这里创建)
	* Param INT_PTR nEntityId:  实体的ID
	* Param INT_PTR nType:  实体的类型
	* Param INT_PTR nX: 位置x
	* Param INT_PTR nY: 位置y
	* Param INT_PTR nParam:  附加参数,如果是怪物表示刷怪点的id（没有刷怪点设置为-1），如果是宠物表示宠物的等级
	* Param  CActor* pOnwer:  玩家的指针
	* Param INT_PTR nLiveTime:  存活的时间
	* Param nForceVesterId : 怪物强制归属，只要怪物死亡时，玩家在线，则掉落物品归属他
	* @Return: 返回创建好的实体的指针
	*/
	CEntity* CreateEntityAndEnterScene(INT_PTR nEntityId, 
									  INT_PTR nType,
									  INT_PTR nX, 
									  INT_PTR nY, 
									  INT_PTR nParam = -1,
									  CActor* pOnwer=NULL,
									  INT_PTR nLiveTime=0,
									  INT_PTR nBornLevel = 0,
									  char* szMonsterName = NULL,
									  INT_PTR nForceVesterId = 0,
									  INT_PTR nInitMonsterPercent = 100
									  );

		
	

	/*
	* Comments:场景的实体死亡后，需要把实体加到这个列表中
	* Param CEntity * pEntity:
	* @Return void:
	* @Remark:不立即删除实体。在下次RunOne的时候删除。现在所有怪物实体销毁时间（非复用的）都是写死2s，
	*		  而RunOne删除实体间隔为5s。
	*/

	void _AddDeathEntity(CEntity* pEntity, LPCSTR file, INT_PTR line );
	
	#define AddDeathEntity(pEntity) _AddDeathEntity(pEntity, __FILE__, __LINE__)

	

	/*
	* Comments:判断指定的xy坐标是否能走动的点
	* Param int x:x坐标
	* Param int y:y坐标
	* @Return bool:
	*/
	inline bool CanMove(INT_PTR x,INT_PTR y) { return m_pMapData->canMove((DWORD)x,(DWORD)y);}
	
	/* 
	* Comments: 判断点能否移动,如果能够移动返回这个点是否是交叉点，以及可移动方向的掩码
	* Param INT_PTR x: 位置x
	* Param INT_PTR y: 位置y
	* Param bool & isCrossPoint:是否是路径的交叉点，只有在可以移动的时候才有效 
	* Param int &nDirMask: 可移动方向的掩码，只有在可以移动的时候才有效 
	* @Return bool: 能够移动返回true，否则返回false
	*/
	/*
	inline bool CanMove(INT_PTR x,INT_PTR y,bool &isCrossPoint,int &nDirMask) 
	{ 
		return  m_pMapData->canMove((unsigned int)x, (unsigned int)y, isCrossPoint, nDirMask);
	}
	*/

	//实体出现选择可移动坐标点
	inline  void GetEntityAppearPos(int curPosX, int curPosY, int  nDest,int& nDestX, int& nDestY)
	{
	   m_pMapData->GetEntityAppearPos(curPosX, curPosY, nDest, nDest,nDestX, nDestY);
	}

	/*
	* Comments:检查地图这个点是否具有某个属性，这个函数只检查属性类型，不带属性参数
	* Param INT_PTR nPosX:x坐标
	* Param INT_PTR nPosY:y坐标
	* Param BYTE bType:属性类型，见tagMapAreaAttribute定义
	* @Return bool:如果含有这个属性，返回true，否则false
	*/
	bool HasMapAttribute(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, SCENEAREA* pArea = NULL);

	/*
	* Comments:检查地图这个点是否具有某个属性，通常用于检查是否限制某个技能（物品等）
	* Param INT_PTR nPosX:
	* Param INT_PTR nPosY:
	* Param BYTE bType:
	* Param int value:通常是技能、物品id等
	* @Return bool:如果含有这个属性，并且属性值是指定的值，返回true；否则false
	*/
	bool HasMapAttribute(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, int value, SCENEAREA* pArea = NULL);
	
	/*
	* Comments:获取区域属性的属性值，只适用于只有一个参数的区域属性，比如经验加成、修为加成
	* Param INT_PTR nPosX: 地图X坐标
	* Param INT_PTR nPosY: 地图Y坐标
	* Param BYTE bType:区域属性类型值
	* Param int value:属性值返回
	* @Return bool:如果返回true，表示具有这个区域属性，这属性值保存在参数value里，否则表示没有这个区域属性，value的值是0
	*/
	bool GetMapAttriValue(INT_PTR nPosX, INT_PTR nPosY, BYTE bType, int& value, SCENEAREA* pArea = NULL);
		
	/*
	* Comments:获取指定位置的区域属性
	* Param INT_PTR x:
	* Param INT_PTR y:
	* @Return SCENEAREA*:
	*/
	SCENEAREA* GetAreaAttri(INT_PTR x, INT_PTR y);

	/*
	* Comments:获取这个区域的中心点
	* Param int & x:返回的x坐标
	* Param int & y:返回的y坐标
	* @Return void:
	*/
	void GetAreaCenter(int& nPosX,int& nPosY);

	/*
	* Comments: 获取场景第一个区域的默认中心点
	* Param int & nPosX:
	* Param int & nPosY:
	* @Return void:
	* @Remark:
	*/
	void GetFirstAreaCenterPoint(int &nPosX, int &nPosY);

	void GetRangeAreaCrossPoint(CEntity* pEntity, int &nPosX, int& nPosY);
	
	/*
	* Comments:获取本场景有划分了多少个区域
	* @Return INT_PTR:
	*/
	INT_PTR GetAreaCount();

	/*
	* Comments:设置区域属性
	* Param INT_PTR nIndex:区域的索引值，对应场景的配置表中区域索引，不能超过总区域的数量
	* Param int_PTR nType:区域属性类型
	* Param int * pValues:区域属性的参数，都是整数
	* Param INT_PTR nCount:参数的数量
	* Param LPCSTR sFunc:如果是进入区域执行脚本的类型，这个要写执行的脚本函数
	* Param Uint64 hNpc:执行的NPC的handle
	* Param int noTips:区域是否提示 
	* @Return bool: 设置是否成功，失败的原因都是nIndex或者nType越界,或者参数有误
	*/
	bool SetAreaAttri(INT_PTR nIndex, INT_PTR nType, int* pValues, INT_PTR nCount, LPCSTR sFunc=NULL, Uint64 hNpc=0, int noTips = 0);
	
	/*
	* Comments:判断前后两个点是否在同一个区域
	* Param INT_PTR nOldX:
	* Param INT_PTR nOldY:
	* Param INT_PTR nNewX:
	* Param INT_PTR nNewY:
	* @Return bool:
	*/
	bool IsSameArea(INT_PTR nOldX, INT_PTR nOldY, INT_PTR nNewX, INT_PTR nNewY);

	/*
	* Comments:判断这个坐标是否传送门
	* Param INT_PTR nX:
	* Param INT_PTR nY:
	* @Return bool:
	*/
	bool IsTelePort(INT_PTR nX, INT_PTR nY);

	/*
	* Comments: 计算刷新点应该刷新的怪物数量
	* Param REFRESHCONFIG * pRefreshConf: 刷新点配置
	* Param PMONSTERCONFIG pMC: 此刷新点的怪物配置
	* Param INT_PTR nOnlinePlayerCnt: 在线玩家数量
	* @Return INT_PTR: 返回此刷新点应该刷新的怪物数量
	* @Remark: 只针对野外普通怪物的刷新，对于副本以及野外Boss、头目都不遵循此规则。
	*		   以1000人为基数，每增加100人，刷怪数量动态增加10%（最少1个），最多增加100%，
	*		   比如刷怪设置10个，在线150人，则这个刷怪点出来15个，在线3000人，出来20个
	*/	
	INT_PTR GetRefreshMonsterCount(REFRESHCONFIG* pRefreshConf, PMONSTERCONFIG pMC, INT_PTR nOnlinePlayerCnt);

	
	
	/*
	* Comments: 将所有场景的怪物都刷新出来
	* @Return void:
	* @Remark:测试用
	*/
	void RefreshMonster();
		
	/*
	* Comments: 同场景数据包广播
	* Param char * pData: 数据包
	* Param SIZE_T size: 数据包长度
	* @Return void:
	*/
	void Broadcast(char *pData, SIZE_T size);
		
	/*
	* Comments: 场景中定期需要执行的事情，比如死亡怪物清除以及场景刷怪
	* @Return BOOL: 如果执行超时返回true；否则返回false
	* @Remark:每次执行只能有固定的时间内，超出时间必须停止
	*/
	bool RunOne();

	/*
	* Comments:返回当前的地图的长和宽
	* Param int & nWidth:
	* Param int & nHeight:
	* @Return void:
	* @Remark:
	*/
	inline void GetMapSize(int &nWidth,int &nHeight)
	{ 
		nWidth= m_pMapData->getWidth(); 
		nHeight = m_pMapData->getHeight(); 
	}
	
	/*
	* Comments:获取地图的指针
	* @Return CAbstractMap *:
	* @Remark:
	*/
	CAbstractMap * GetMapPtr(){ return m_pMapData; } 
		
	/*
	* Comments:获取场景中活的怪物数量
	* @Return INT_PTR:
	* @Remark:
	*/
	 INT_PTR GetLiveMonsterCount();
	
	/*
	* Comments:获取场景中的怪物列表+
	* @Return CEntityList&:
	* @Remark:
	*/
#ifndef MONSTER_MANAGER_BY_SCENE
	inline CEntityList& GetMonsterList()	{ return m_MonsterList;}
#else
	inline std::map<EntityHandle,CMonster*>& GetMonsterList()	{ return m_MonsterList;}
#endif
	
	/* 
	* Comments: 获取掉落物品列表
	* @Return CEntityList&:  
	*/
	inline CEntityList& GetDropItemList() {return m_dropItemList;}
	
	/*
	* Comments:获取怪物的数量
	* Param int nMonsterId:怪物的id，如是0，表示所有的怪物；否则只统计指定ID的怪物
	* Param EntityHandle hHandle：拥有者句柄。如果是0，表示忽略拥有者；否则只统计归属于此拥有者的怪物数量
	* @Return int: 
	*/
	int GetMonsterCount(int nMonsterId, EntityHandle hHandle);

	/*
	* Comments: 是否杀死场景中所有指定的怪物
	* Param int nMosterId: 怪物id
	* Param EntityHandle hHandle: 怪物的拥有者
	* @Return bool:如果还有，返回false
	*/
	bool IsKillAllMonster(int nMonsterId, EntityHandle hHandle = 0);

	/*
	* Comments: 查找场景中指定名称的NPC
	* Param LPCSTR sName: NPC名称
	* @Return CNpc*:
	* @Remark:
	*/
	CNpc* GetNpc(LPCSTR sName);


	/*
	* Comments:通过id获取npc指针
	* Param int nNpcId:npcID
	* @Return CNpc *:
	*/
	CNpc * GetNpc( int nNpcId);

	/*
	* Comments: 获取指定位置处的NPC
	* Param const INT_PTR nX: 位置X坐标
	* Param const INT_PTR nY: 位置Y坐标
	* @Return CNpc*: 返回指定位置的NPC对象指针
	* @Remark: 返回找到的第一个NPC
	*/
	CNpc* GetNpcAtXY(const INT_PTR nX, const INT_PTR nY);

	int GetEntityCountAtXY(const int nX, const int nY, const int nEntityType);
	
	/*
	* Comments: 获取场景中的NPC列表
	* @Return CEntityList*:
	* @Remark:
	*/
	CEntityList* GetNpcList() { return &m_NpcList;}

	/*
	* Comments: 获取场景中的玩家数量
	* @Return INT_PTR:
	* @Remark:
	*/
	inline INT_PTR GetPlayCount() { return m_PlayList.count(); }

	/*
	* Comments: 获取场景中玩家列表
	* @Return CEntityList&:
	* @Remark:
	*/
	inline CEntityList& GetPlayList()		{ return m_PlayList;}
	
	//返回宠物的列表
	inline CEntityList&GetPetList() {return m_petList;}


	//返回英雄的列表
	inline CEntityList&GetHeroList() {return m_heroList;}

	/*
	* Comments: 获取场景名称
	* @Return LPCTSTR:
	* @Remark:
	*/
	inline LPCTSTR GetSceneName() {return m_pSceneData ? m_pSceneData->szScenceName : "";}
	/*
	* Comments: 设置场景所属副本
	* Param CFuBen * pFb:
	* @Return VOID:
	* @Remark:
	*/
	inline VOID SetFuBen(CFuBen* pFb) { m_pFb = pFb; }

	/*
	* Comments:获取场景所属副本
	* @Return CFuBen*:
	* @Remark:
	*/
	inline CFuBen* GetFuBen() { return m_pFb; }

	/*
	* Comments: 设置场景句柄
	* Param CSceneHandle hHandle:
	* @Return VOID:
	* @Remark:
	*/
	inline 	void SetHandle(const CSceneHandle& hHandle) {m_hHandle = hHandle;}

	/*
	* Comments: 获取场景句柄
	* @Return CSceneHandle:
	* @Remark:
	*/
	inline const CSceneHandle& GetHandle()	{return m_hHandle;}

	/*
	* Comments: 获取场景Id
	* @Return int:
	* @Remark:
	*/
	inline int GetSceneId() {return m_pSceneData ? m_pSceneData->nScenceId : -1;}

	/*
	* Comments: 获取场景数据
	* @Return SCENECONFIG*:
	* @Remark:
	*/
	inline SCENECONFIG* GetSceneData()	{return m_pSceneData;}
	
	/*
	* Comments: 获取场景剩余时间
	* @Return int: 返回场景剩余时间，以秒为单位
	* @Remark:
	*/
	int GetSceneTime();

	/*
	* Comments: 设置场景剩余时间（相对于当前时刻来说）
	* Param UINT nTime: 剩余时间，秒为单位
	* @Return void:
	* @Remark:
	*/
	void SetSceneTime(UINT nTime);

	/*
	* Comments: 获取一个点的实体的数目
	* Param int nPosX:点的坐标x
	* Param int nPosY:点的坐标y
	* Param int nEntityType:实体的类型，比如玩家
	* @Return :
	*/
	inline int GetTypeEntityCount(int nPosX,int nPosY,int nEntityType)
	{
		MapEntityGrid* pGrid= m_Grid.GetList(nPosX);
		if(pGrid ==NULL) return 0;
		return pGrid->GetTypeEntityPosCount(nEntityType, nPosY);
	}

	//获取某个坐标的实体列表
	inline const MapEntityGrid * GetMapEntityGrid(int nPosX)
	{
		return (const MapEntityGrid *)m_Grid.GetList(nPosX);
	}

	INT_PTR GetSceneWidth() {return m_nWidth;}

	INT_PTR GetSceneHeight() {return m_nHeight;}

	//创建npc和传送门
	void CreateNpcTransport(bool boRun =true);
	//推送新的区域属性
	void BroadcastNewAttr();

	//获取一个场景的格子
	CEntityGrid & GetGrid (){return m_Grid;}

	//获取P1, P2两个点组合成的矩形内的一个随机点
	bool GetRandomPoint(CEntity *pEntity, int nP1_x, int nP1_y, int nP2_x, int nP2_y, int &nResult_x, int &nResult_y);

	//获取P1, P2两个点组合成的矩形内的一个随机点
	bool GetRandomPointByEntityType(int nEntityType, int nP1_x, int nP1_y, int nP2_x, int nP2_y, 
									int &nResult_x, int &nResult_y, bool canAlwaysEnter = false, bool bForcePos = false);

	/*
	* Comments: gm创建场景中所有NPC
	* Param char* npcName:场景npc名称
	* @Return bool: 成功返回true; 失败返回false
	* @Remark:
	*/
	bool GmCreateNpcByName(char* npcName);

	/*
	* Comments:gm创建npc
	* Param int nNpcID:npcID
	* @Return bool: 成功返回true; 失败返回false
	*/
	bool GmCreateNpcById(int nNpcID, int nX, int nY);

	/*
	* Comments:通过名字杀死怪
	* Param const char * sName:
	* @Return int:
	*/
	int KillMonsterByName(const char * sName, CEntity * pGmEntity, byte nDropItem);
	void KillAllMonster(CEntity * pGmEntity);
	/* 
	* Comments: 直接清除死亡的怪物
	* Param EntityHandle & handle:  
	* @Return void:  
	*/
	void ClearDeathEntity(EntityHandle handle);


	/* 
	* Comments: 清除某类型的实体
	* Param int nType: 实体类型
	* @Return void:  
	*/
	void ClearEntity(int nType);

	/* 
	* Comments: 清除掉落物品
	* @Return void:  
	*/
	void ClearDropItem(){m_dropItemList.clear();}
	//发消息给场景中所有的人
	void SendMsg(LPCSTR sMsg, int nType);

	void SendSecneShortCutMsg(LPCSTR sMsg);

	//获取与下次刷新还剩余多少秒 bsec = true 发送秒，bDie = true怪物是死的
	int GetFlushTime(int nMonsId, TICKCOUNT nCurTick,bool bsec = false,bool bDie = false);

	//根据bossID获得是否死亡 true 死了(只适合怪物唯一的情况)
	bool GetBossIsDieById(int nMonsId);

		/*
	* Comments: 将实体从对应的场景实体列表中删除
	* Param CEntity * pEntity: 实体对象指针
	* @Return void:
	* @Remark: 只是更新场景的实体列表，并没有更新场景格子中的实体列表。真正的实体对象删除，
	*		   是在场景的RunOne中执行。
	*/
	void RemoveEntity(CEntity* pEntity );

	bool GetMobMonsterXY(REFRESHCONFIG* pRefreshConf, INT_PTR &nX, INT_PTR &nY);
	CEntity* getMonsterPtrById(int nMonsterId );

	/*设置不可移动点*/
	inline void SetUnablePoint(INT_PTR x,INT_PTR y, BYTE btValue) 
	{
		m_pMapData->setUnablePoint((DWORD)x,(DWORD)y,btValue);
	}

	int CalcAllActor(int nLevel = 0);
	int CloseSceneAllActor(int nLevel = 0);

	int GetNowSceneGuildList();
	//当前区域在 场景的index
	int GetAreaListIndex(INT_PTR x, INT_PTR y);

	//重置区域配置
	void ResetSceneConfig();
protected:	
	/*
	* Comments: 创建场景中的所有传送门
	* @Return void:
	* @Remark:
	*/
	void CreateTeleport();

	/*
	* Comments: 重置场景的刷怪动态数据
	* Param REFRESHLIST * list: 场景刷怪点静态配置
	* @Return void:
	* @Remark: 将场景刷怪动态数据全部重置（刷怪点的刷怪数量置为0）
	*/
	void ResetRefreshList(REFRESHLIST* list);

	/*
	* Comments: 场景重置，以后重用
	* @Return BOOL:
	* @Remark:
	*/
	BOOL Reset();

	/*
	* Comments: 场景复用处理。从空闲列表中取出后必须做的一些工作
	* @Return void:
	* @Remark: 主要用于重设NPC的初始化标记（用于例行处理的执行）以及场景的刷新列表初始化
	*/
	void OnReuse();

	/*
	* Comments: 释放场景配置
	* @Return void:
	* @Remark:
	*/
	void ReleaseConfig();

	/*
	* Comments: 复制一下场景配置
	* Param SCENECONFIG * pData:
	* @Return void:
	* @Remark: 先释放老的区域配置，然后重新设置场景配置，区域属性需要重新创建一份
	*/
	void CloneSceneConfig(SCENECONFIG* pData);
	
	/*
	* Comments: 创建场景中所有NPC
	* Param SCENECONFIG * pData:场景配置数据（主要用到场景的NPC配置数据）
	* Param bool bInited:是否已经初始化。未初始化的NPC不会执行逻辑循环处理
	* @Return bool: 成功返回true; 失败返回false
	* @Remark:
	*/
	bool CreateNpc( SCENECONFIG* pData,bool boRun=true );

	/*
	* Comments: 创建场景中等待NPC
	* @Return bool: 成功返回true; 失败返回false
	* @Remark:
	*/
	bool CreateWaitNpc();

	/*
	* Comments:重置所有npc，不释放内存
	* @Return void:
	* @Remark:
	*/
	//void ResetAllNpc();

	//删除
	void DestroySceneEntity();



	/*
	* Comments:计算场景的区域设定，给每个网格设置属性索引值
	* Param CDataAllocator & dataAlloc:
	* @Return void:
	*/
	void SetAreaConfig(CObjectAllocator<char>& dataAlloc);

	/*
	* Comments: 构造传送门索引。
	* Param CObjectAllocator<char> & dataAlloc:
	* @Return void:
	* @Remark: 构造传送门表。为地图每个点记录一个该场景传送门的索引，用于快速查询某个位置是否是传送门。
			   如果某个点不是传送门，那么此点的传送门索引为0；否则此点的值是传送门索引+1。
	*/
	void SetTelePortTable(CObjectAllocator<char>& dataAlloc);
		
	/*
	* Comments: 重新加载npc的脚本
	* Param LPCTSTR szNpcName: 指定的场景名，如果为NULL，则寻找所有的场景
	* @Return INT_PTR: 
	* @Remark:
	*/
	INT_PTR ReloadNpcScript(LPCTSTR szNpcName);  

	/*
	* Comments:踢出所有的玩家，玩家将返回到普通场景，如果本副本是常规副本，这函数不起作用
	* @Return void:
	*/
	void ExitAllActor();
	void KickOutAllActor();
	
public:
	CAbstractMap*		m_pMapData;	//地图的障碍信息

	PBYTE				m_pMapAreaTable;	// 记录每个网格对应的属性索引值。请看场景的配置表area部分.这个索引值默认是0，通过[MapWidth * Y + X]访问
	PBYTE				m_pTelePortTable;	// 传送门的索引值，如果改点不是传送门的位置，则为0，第一个传送门索引是1，类推

protected:

	AStar				m_AStar;	//A星寻路工具
	SCENECONFIG*		m_pSceneData;//场景的配置信息，注意，保留场景不能使用这个指针（看CLogicEngine::InitReserveScene函数定义)
	SCENECONFIG*		m_pSrcSceneData;//原始的场景配置，用来clone的
	INT_PTR				m_nWidth;
	INT_PTR				m_nHeight;

private:
	CEntityGrid			m_Grid;	//网格
	CFuBen*				m_pFb;

	CSceneHandle		m_hHandle;

	//场景内玩家列表
	CEntityList			m_PlayList;
	CEntityList			m_NpcList;
	CEntityList			m_TeleportList;//传送门
#ifndef MONSTER_MANAGER_BY_SCENE
	CEntityList			m_MonsterList;
#else
	std::map<EntityHandle,CMonster*> m_MonsterList;
	std::map<EntityHandle,CMonster*>::iterator m_MonInterator;
#endif
	CEntityList         m_petList;  //宠物的列表
	CEntityList         m_heroList;  //英雄的列表
	
	CEntityList         m_dropItemList;  //掉落物品的列表
	CEntityList         m_fireList;  //法师的火

	//CEntityList			m_DeathList;//已死亡的实体的列表，这个要定期检查，并且清除掉

	CVector<EntityHandle>             m_DeathList;  //已死亡的实体的列表，这个要定期检查，并且清除掉

	REFRESHVALUE*		m_vRefreshMonster;
						
	CTimer<1000>		m_1s;
	CTimer<5000>		m_5s;	//5秒清理死亡的怪物一次
	CTimer<500>			m_500msTimer; // 刷怪

	UINT				m_restTime;	//剩余时间，默认是0，有设置时间的通常是活动
	UINT				m_nRefreshtime;	//剩余时间，默认是0，有设置时间的通常是活动
	std::vector<NPCPOS>       m_waitNpcLists;//等待刷新的npcid
};
