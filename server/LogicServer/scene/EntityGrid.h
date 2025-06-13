#pragma once

/************************
	实现场景中网格的管理和操作
***********************/


//class CLinkedNode<Entity*>	EntityNode;
class CBaseEntityList;
class CEntity;
class CScene;

/*
	因为内存非常巨大，为了节省内存，进行优化，将x坐标一样的放一个列表，这样1行的玩家在一个列表里
*/


#ifndef GRID_USE_STL_MAP
struct EntityHandlePos
{
	EntityHandle hd;   //实体的handle
	WORD pos;          //位置
};
typedef CBaseList<EntityHandlePos>	EntityPosVector;
#else
struct HandleCmpter
{
	bool operator()(const EntityHandle& _x, const EntityHandle& _y) const
	{ return Uint64(_x) < Uint64(_y); }
};
typedef std::map<EntityHandle, WORD, HandleCmpter> EntityPosVector;
#endif

// 定义游戏中单个实体格子
class MapEntityGrid
{
public:
	MapEntityGrid(){
		//m_entitys.reserve(16);
	}
	~MapEntityGrid();
	
	// 添加实体
	void addEntity(EntityHandle handle, int posY){
#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = 0; i < m_entitys.count(); i++)
		{
			if (m_entitys[i].hd == handle)
			{
				return ;
			}
		}
		EntityHandlePos pos;
		pos.hd = handle;
		pos.pos = (WORD)posY;
		m_entitys.add(pos);
#else
		assert(m_entitys.find(handle) == m_entitys.end());
		m_entitys[handle] = posY;
#endif
	}

	void setEntityPos(EntityHandle handle,int posY)
	{
#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = m_entitys.count() -1; i >-1 ; i--)
		{
			if (m_entitys[i].hd == handle)
			{
				m_entitys[i].pos=(WORD)posY;
				return ;
			}
		}
#else
		assert(m_entitys.find(handle) != m_entitys.end());
		m_entitys[handle] = posY;
#endif
	}

	// 移除实体
	void removeEntity(EntityHandle handle)
	{
#ifndef GRID_USE_STL_MAP
		for (INT_PTR i = m_entitys.count() -1; i >-1 ; i--)
		{
			if (m_entitys[i].hd == handle)
			{
				m_entitys.remove(i);
				return ;
			}
		}
#else
	//玩家初始化不成功，可能不在格子里
	EntityPosVector::iterator it = m_entitys.find(handle);
	//assert(m_entitys.find(handle) != m_entitys.end());
	if(it != m_entitys.end())
	{ 
        m_entitys.erase(it);
	}

#endif
	}
	int GetTypeEntityPosCount(int nType,int nPosY)
	{
		int nCount=0;
#ifndef GRID_USE_STL_MAP
		for(INT_PTR i=0;i < m_entitys.count(); i++)
		{
			if((int)m_entitys[i].hd.GetType() == nType && m_entitys[i].pos == nPosY)
			{
				nCount ++;
			}
		}
#else
		typedef typename EntityPosVector::iterator Iter;
		Iter it;
		Iter end = m_entitys.end();
		for (it = m_entitys.begin(); it != end; it++)
		{
			if((int)(*it).first.GetType() == nType && (*it).second == nPosY)
				nCount ++;
		}
#endif
		return nCount;
	}

	EntityPosVector m_entitys;		// 格子中的实体列表
	
};

//typedef HandleMgr<CEntityList,CEntityListHandle > CEntityListMgr;

class CEntityGrid
{
	friend class CScene;
	friend class AStart;
    friend class ASMapCell;
	
public:
	CEntityGrid();
	virtual ~CEntityGrid();
	void SetScene(CScene* pScene) {m_pScene = pScene;}

	//inline CEntityList* GetList(INT_PTR x,INT_PTR y);
	MapEntityGrid* GetList(INT_PTR x);
	

	/*inline VOID SetList(INT_PTR x,INT_PTR y,CEntityList* pList);*/

	//释放资源
	VOID Release();

	//只清理怪物和玩家，以备重用
	void Reset();

	//释放一个list
	VOID ReleaseList(CEntityList* el);

	CEntityList* GetEntityList();
	//重新设置网格的大小
	VOID Init(int nRowCount,int nColCount);
	/*
	* Comments: 把一个实体添加到一个位置
	* Param CEntity * pEntity: 实体的指针
	* Param int nPosx: 位置x
	* Param int nPosY: 位置y
	* @Return int: 成功返回true，否则返回false
	*/
	//bool AddEntity(CEntity * pEntity,INT_PTR nPosx,INT_PTR nPosY);

	/*
	* Comments:判断当前位置是否传送点，如果是则传送
	* Param CEntity * pEntity:
	* @Return void:
	*/
	void TelePort( CEntity * pEntity );

	//判断是否可以通过传送门
	bool CanPassPort( CEntity * pEntity,int nPassId);

	/*
	* Comments:判断能否把角色安排到网格中
	* Param CEntity * pEntity:
	* Param int nPosx:nposx会作为一个返回值，因为有可能安排的位置和这个不同
	* Param int nPosY:
	* Param bool bShowErr: 是否输出错误
	* @Return bool:
	*/
	bool CanAddEntity(CEntity * pEntity,INT_PTR& nX,INT_PTR& nY,bool bShowErr = true, bool bCanAlwaysEnter=false);

	/*
	* Comments:场景中删除一个实体（实体死亡或退出游戏或者去了其他场景）
	* Param CEntity * pEntity:
	* Param int nX:实体所在的位置
	* Param int nY:
	*/
	void DeleteEntity(CEntity * pEntity);

	//移动一个实体
	int Move(CEntity * pEntity,INT_PTR nDir,INT_PTR nSpeed=1);

	/*
	* Comments:直接移动到某个坐标
	* Param CEntity * pEntity:实体指针
	* Param INT_PTR nNewX:要移动的位置
	* Param INT_PTR nNewY:
	* Param bool boNew:是否从其他场景跳转过来的
	* @Return int:
	*/
	int MoveTo(CEntity * pEntity,INT_PTR nNewX,INT_PTR nNewY,bool boNew = false,bool bCanAlwaysEnter=false);



	/*
	* Comments:判断是否有穿人穿怪的限制
	* Param INT_PTR nEntityType: 实体的类型
	* Param INT_PTR x:坐标x
	* Param INT_PTR y:坐标y
	* @Return bool:如果有限制，并且这个位置有人或怪，返回false
	*/
	bool CanCross(INT_PTR nEntityType, INT_PTR x, INT_PTR y,bool bCanAlwaysEnter=false,bool bCanEnterFire = false);

	/*
	* Comments:根据旧的坐标和方向，速度计算
	* Param INT_PTR & nX:
	* Param INT_PTR & nY:
	* Param BYTE nDir:
	* Param int nSpeed:
	* @Return VOID:
	*/
	static VOID NewPosition(INT_PTR& nX,INT_PTR& nY,INT_PTR nDir,INT_PTR nSpeed);

	/*
	* Comments:计算能不能移动到新位置
	* Param CEntity* pEntity: 实体对象
	* Param INT_PTR & nX:
	* Param INT_PTR & nY:
	* Param INT_PTR nDir:
	* Param INT_PTR nSpeed:
	* @Return bool:
	*/
	bool CanMoveTo(CEntity* pEntity, INT_PTR& nX,INT_PTR& nY,INT_PTR nDir,BYTE nSpeed);
	
	/*
	* Comments:判断这个网格能否站立
	* Param CEntity* pEntity:实体对象
	* Param INT_PTR x:x坐标
	* Param INT_PTR y:y坐标
	* @Return bool:
	*/
	bool CanMoveThisGrid(CEntity* pEntity,INT_PTR x ,INT_PTR y);


	/*
	* Comments:在一个点的周围寻找一个可以移动的点
	* Param CEntity * pEntity:实体的指针
	* Param CScene * pScene:场景的指针
	* Param INT_PTR x: 位置x
	* Param INT_PTR y:位置y
	* Param INT_PTR & nResultX: 结果的位置x
	* Param INT_PTR & nResultY:结果的位置y
	* Param INT_PTR & nType:传进去的实体类型
	* @Return bool:如果成功就返回true，否则返回false
	*/
	static bool GetMoveablePoint(CEntity *pEntity,CScene * pScene, INT_PTR x,INT_PTR y,INT_PTR &nResultX,INT_PTR &nResultY,bool bCanAlwaysEnter = false,INT_PTR nType = 1);

	//将区域下发给玩家
	inline void SendAreaConfig(SCENEAREA* pArea,CActor *pActor);

	
protected:
	/*
	* Comments:角色移动时要做的判断，如果是跨过不同的区域，则要执行不同的操作
	* Param CActor * pActor:
	* @Return void:
	*/
	void CrossArea(CActor* pActor);

private:
	/*
	* Comments:判断指定的xy坐标是否能走动的点
	* Param int x:x坐标
	* Param int y:y坐标
	* @Return bool:
	*/
	//bool CanMove(INT_PTR x,INT_PTR y);

private:
	//网格的行数和列数
	int					m_nRow;
	int					m_nCol;

	//一个二维数组保存链表的指针
	//CEntityList**		m_pGrid;
	MapEntityGrid*		m_pGrids;	// 格子集合，用于一个维护一个场景中的所有小格子列表

	CScene*				m_pScene;



public:
	//static CEntityListMgr*	g_EntityListMgr;	//实体链表的内存管理类，全局变量
};
