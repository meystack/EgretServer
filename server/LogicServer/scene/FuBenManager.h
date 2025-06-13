#pragma once

#define REUSEFB

#define MAX_NEWCREATEFUBEN 5

class CLogicEngine;

using namespace jxcomm::gameMap;
class CFuBen;

typedef struct tagMapData
{
	char			szFileName[256];
	CAbstractMap*	pMapData;
	DECLARE_OBJECT_COUNTER(tagMapData)
}MAPDATA;

//保存地图障碍信息的列表
typedef CBaseList<MAPDATA*>	MAPDATAVECTOR;
typedef CVector<CFuBen*>	FUBENVECTOR;
typedef CHandleList<CFuBen*,UINT> FubenHandleList;

class CFuBenHandleMgr : public FuBenHandleMgr
{
typedef FuBenHandleMgr Inherited;
public:
	CFuBenHandleMgr(LPCTSTR lpszDesc);
	CFuBen*       GetDataPtr( CFuBenHandle handle );
	const CFuBen* GetDataPtr( CFuBenHandle handle ) const;
};

class FubenAllocator: public std::allocator<CFuBen>
{
    typedef CFuBen* pointer;
    pointer allocate(size_type _n, const void* = 0);
    void deallocate(pointer _p, size_type);
    void construct(pointer, const CFuBen&) { }
    void destroy(pointer) { }
};

class CFuBenManager:
	public CComponent
{
	//执行刷怪需要的参数，保证刷怪占用的时间片
	typedef struct tagRefreshPara
	{
		CFuBen* pFuben;//上次刷怪的副本
		int		nScenePos;//上次刷怪的场景，这个不是场景id，而是在副本中场景的索引
		int		nRefreshPos;//上次刷怪点的位置
		VOID setFB(CFuBen* pFb)
		{
			pFuben = pFb;
			nScenePos = 0;
			nRefreshPos = 0;
		}
		VOID setScene(int i)
		{
			nScenePos = i;
			nRefreshPos = 0;
		}
	}RefreshPara;


public:
	CFuBenManager(void);
	virtual ~CFuBenManager(void);

public:
	
	 //组件的销毁函数，这里要消耗一些S分配的内存
	VOID  Destroy();

	//创建一批空闲的副本备用
	void CreateFreeFb();

	/*
	* Comments: 实体请求进入一个副本
	* Param CEntity * pEntity: 实体的指针
	* Param unsigned int hFbHandle: 副本的handle
	* Param unsigned int nScenceID: 场景的ID
	* Param int nPosX: 位置x
	* Param int nPosY: 位置y
	* @Return bool:    成功返回true，否则返回false
	*/
	inline bool EnterFuBen(CEntity * pEntity,unsigned int hFbHandle,int nScenceID,int nPosX, int nPosY)
	{
		CFuBenHandle hHandle(hFbHandle);
		CFuBen* pFuBen = m_FuBenMgr->GetDataPtr(hHandle) ;
		return EnterFuBen(pEntity,pFuBen,nScenceID,nPosX,nPosY);
	}

	/*
	* Comments: 进入一个副本
	* Param CEntity * pEntity: 实体的指针
	* Param CFuBen * pFuBen:副本的指针,如果NULL表示进入静态副本
	* Param unsigned int nScenceID:场景的ID
	* Param int nPosX: 位置x
	* Param int nPosY: 位置y
	* Param int nType:  传送类型（0-默认，1-任务传送）
	* Param int nValue: 传送值：（类型为1时，是任务ID）
	* Param int nParam: 传送参数：（类型为1时，是任务的目标ID）
	* @Return bool: 成功返回true，否则返回false
	*/
	bool EnterFuBen(CEntity * pEntity,CFuBen* pFuBen ,INT_PTR nScenceID,INT_PTR nPosX, INT_PTR nPosY,int nType = 0, int nValue = 0, int nParam=-1);

	/*
	* Comments: 获取一个副本的静态拷贝的指针，每个副本份都至少有1份拷贝，普通场景是1份，其他场景可能是多份，但是静态的只有1份
	* Param unsigned int nFbId:
	* @Return CFuBen *:
	*/
	inline CFuBen * GetFbStaticDataPtr(unsigned int nFbId)
	{
		if ((nFbId < 0) || (nFbId >= (UINT)m_vStaticFbList.count()))
		{
			OutputMsg(rmWaning,_T("Enter fb fail for fBid=%d is invalid"),nFbId);
			return NULL;
		}
		return m_vStaticFbList[nFbId];
	}

	/*
	* Comments: 通过场景的名字返回副本的指针和场景的指针，主要用于GM指令的传送
	* Param LPCTSTR sSceneName:  场景名字
	* Param CFuBen * & pRetFb: 副本的指针
	* Param CScene * & pRetScene: 场景的指针
	* @Return bool: 如果找到了返回true，否则返回false
	* Remark: 是在静态副本中查。对于动态副本来说，可能有多个实例，没法根据名字查找的到副本对象
	*/
	bool GetFbPtrBySceneName(LPCTSTR sSceneName, CFuBen *& pRetFb, CScene *& pRetScene);

	/*
	* Comments: 通过场景的ID返回副本的指针和场景的指针，主要用于GM指令的传送
	* Param INT_PTR nSceneID:  场景ID
	* Param CFuBen * & pRetFb: 副本的指针
	* Param CScene * & pRetScene: 场景的指针
	* @Return bool: 如果找到了返回true，否则返回false
	* Remark: 是在静态副本中查。对于动态副本来说，可能有多个实例，没法根据名字查找的到副本对象
	*/
	bool GetFbPtrBySceneId( unsigned int nSceneID, CFuBen *& pRetFb, CScene *& pRetScene);

	bool GetFbPtrBySceneId( unsigned int nFbId, int nSceneID,  CFuBen *& pRetFb, CScene *& pRetScene);

	//每次逻辑循环都需执行一次的操作
	VOID RunOne();

	inline FUBENVECTOR* GetFbList() {return &m_vStaticFbList;}

	//生成一个新的副本
	CFuBen* CreateFuBen(int nFbId);

	CFuBen* AllocNewFb( int nFbId,bool boRun =true );

	//删除一个副本,释放资源
	VOID ReleaseFuBen(CFuBen* pFuBen);

	//加一个静态副本到列表中，本列表中的副本不释放，作为其他副本的克隆原体
	VOID AddStaticFuBen(CFuBen* pFb);

	//根据文件名得到地图的障碍信息
	//如果不存在列表中，则创建一个并加入到列表
	CAbstractMap* GetMapData(const char* szFileName);

	
	bool GetVisibleList(CEntity * pEntity,EntityVector& vVisibleList,int nLeft=-MOVE_GRID_COL_RADIO,int nRight=MOVE_GRID_COL_RADIO
		,int nUp=-MOVE_GRID_ROW_RADIO,int nDown=MOVE_GRID_ROW_RADIO);

	bool GetVisibleList(CEntity * pEntity,CObserverEntityList& vVisibleList,int nLeft=-MOVE_GRID_COL_RADIO,int nRight=MOVE_GRID_COL_RADIO
		,int nUp=-MOVE_GRID_ROW_RADIO,int nDown=MOVE_GRID_ROW_RADIO, bool *pbActorInVisibleList = NULL);

	// 获取有序的可见列表（按照句柄排序）
	bool GetSeqVisibleList(CEntity * pEntity, SequenceEntityList& visibleList, int nLeft=-MOVE_GRID_COL_RADIO,int nRight=MOVE_GRID_COL_RADIO
		,int nUp=-MOVE_GRID_ROW_RADIO,int nDown=MOVE_GRID_ROW_RADIO);
	/*
	* Comments:重新读入npc的脚本
	* Param LPCTSTR szSceneName:指定的场景名，如果为NULL，则寻找所有的场景
	* Param LPCTSTR szNpcName:指定的NPC名，如果为NULL，则重新读入所有npc的脚本
	* @Return void:
	*/
	INT_PTR ReloadNpcScript(LPCTSTR szSceneName,LPCTSTR szNpcName);

	/*
	* Comments:获取指定副本的配置信息
	* Param int nFbid:
	* @Return COMMONFUBENCONFIG*:
	*/
	COMMONFUBENCONFIG* GetConfig(int nFbid);

	/*
	* Comments:追踪副本的情况
	* @Return void:
	*/
	void Trace();

	//输出刷怪的位置
	void TraceRefreshPos();

	//重置副本的刷怪
	void ResetFbRefresh();

	/*
	* Comments:根据副本的id获取全服中这个副本的所有copy,返回副本的handle
	* Param UINT nFbid:副本的id
	* Param UINT * pList:返回的副本列表在这里
	* Param size_t nBufSize:列表缓冲区的大小
	* @Return int:返回副本的实际数量
	*/
	int GetFbCountFromId(UINT nFbid,UINT* pList,size_t nBufSize);

	/*
	* Comments: 将所有静态场景的怪物都创建出来
	* @Return void:
	* @Remark: 注意，只创建静态场景中的怪物
	*/
	void RefreshMonster();

	/*
	* Comments: 统计所有静态场景的已经创建出来怪物数量
	* @Return void:
	* @Remark: 
	*/
	unsigned int TotalAllSceneMonsterCount();

	/*
	* Comments: dump副本相关的统计信息
	* @Return void:
	* @Remark:
	*/
	void Dump();

	/*
	* Comments: 场景相关内存统计
	* @Return void:
	* @Remark:
	*/
	void DumpFbDataAlloc(wylib::stream::CBaseStream& stream);

	//判断是否需要初始化副本面板
	bool IsNeedShowFubenForm(int nFbId);

	//获取随机出生坐标
	void GetRandomCreatePos(int nToken,int& nScenceID,int& nPosX,int& nPosY);
private:
	
	FubenHandleList			m_vAllFbList;		// CHandleList<CFuBen*,UINT> 游戏中出现的所有副本copy,不包括m_vStaticFbList里面的
	FUBENVECTOR				m_vStaticFbList;	// CVector<CFuBen*> 副本的静态实例，其他副本都从这些副本中clone出来，里面的实例都是new出来，没有使用HandleManager内存管理
	std::map<int, CFuBen*>	m_vMuiltyFbMap;		// 多人副本<fbid，pFb>
	CVector<FUBENVECTOR*>	m_FreeFbList;		// 保存空闲的副本列表,便于重用

	// 空闲副本，按照副本Id来保存的（要求副本ID连续），每个ID对应的可能有多个此副本对象

	CTimer<200>				m_RefTimer; 			// 副本刷新定时器
	CTimer<300000>			m_FreeFbTimer;			// 定时释放多余的空闲副本
	int						m_anFubenCount[256];	// 统计每个副本数量
	MAPDATAVECTOR			m_vMapDataList;			// 所有地图的障碍信息列表

public:
	
	static CFuBenHandleMgr*     m_FuBenMgr;			//全局的副本管理内存块
	static SceneHandleMgr*		m_SceneMgr;			//全局的场景管理内存块
	static RefreshPara			m_RefreshPara;		//分片刷新
	static EntityVector*		m_pVisibleList;		//临时用来保存可见的实体列表
};
