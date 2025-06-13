// #pragma once

// /*************************************************************
// /*
// /*             《剑啸江湖》 游戏抽象地图加载类
// /*
// /*		地图数据读取后将不保存用于显示的图片编号等数据，而仅仅
// /*	保存用于进行逻辑处理的点阵数据——使用一个字节表示一个坐标。
// /*	当坐标数据的值为1的时候，表示该坐标可以移动，否则表示该
// /*	坐标不可移动。
// /*
// /*		地图坐标数据采用连续的字节数组存储，访问一个坐标值的方
// /*	法为：y * 地图宽度 + x
// /*
// /************************************************************/


// class CAbstractMap
// {
// protected:
// 	DWORD		m_dwWidth;		//地图宽度
// 	DWORD		m_dwHeight;		//地图高度
// 	PINT		m_pMoveableIndex;//每坐标对应的可移动索引，用于外层优化
// 	INT			m_nMoveableCount;//移动坐标数量			
// public:
// 	CAbstractMap();
// 	virtual ~CAbstractMap();

// 	/* 从流中加载地图数据 */
// 	bool LoadFromStream(wylib::stream::CBaseStream& stream);
// 	/* 从文件中加载地图数据 */
// 	bool LoadFromFile(LPCTSTR sFileName);
// 	/* 生成一张全部可以移动的地图 */
// 	void initDefault(DWORD dwWidth, DWORD dwHeight);
// 	/* 获取地图宽度 */
// 	inline int getWidth(){ return m_dwWidth; }
// 	/* 获取地图高度 */
// 	inline int getHeight(){ return m_dwHeight; }
// 	/* 获取可移动坐标数量 */
// 	inline int getMoveableCount(){ return m_nMoveableCount; }
// 	/* 获取地图中x, y坐标的可移动索引，返回-1表示此位置不可移动 */
// 	inline int getMoveableIndex(DWORD x, DWORD y)
// 	{
// 		if ( x >= m_dwWidth || y >= m_dwHeight )
// 			return false;
// 		return m_pMoveableIndex[y * m_dwWidth + x]; 
// 	}
// 	/* 判断地图中x, y坐标位置是否可以移动，返回true表示坐标可以移动 */
// 	inline bool canMove(DWORD x, DWORD y)
// 	{
// 		if ( x >= m_dwWidth || y >= m_dwHeight )
// 			return false;
// 		return m_pMoveableIndex[y * m_dwWidth + x] != -1; 
// 	}
// };

