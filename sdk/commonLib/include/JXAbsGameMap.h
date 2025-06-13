#pragma once

/*************************************************************

             《剑啸江湖》 游戏抽象地图加载类

		地图数据读取后将不保存用于显示的图片编号等数据，而仅仅
	保存用于进行逻辑处理的点阵数据——使用一个字节表示一个坐标。
	当坐标数据的值为1的时候，表示该坐标可以移动，否则表示该
	坐标不可移动。

		地图坐标数据采用连续的字节数组存储，访问一个坐标值的方
	法为：y * 地图宽度 + x

************************************************************/
#pragma  once 
//#include "MapSearchNode.h"
#include "CommonDef.h"
#include "VirturalMap.h"
class CBufferAllocator;
namespace jxcomm
{
	namespace gameMap
	{
		class CAbstractMap:
			public VirturalMap //VirturalMap 只有一个父函数
		{
		public:
			static const int s_nInvalidMapPos	= -2;
			static const int s_nBlockPos		= -1;
			static const int MAX_WAY_POINT_COUNT =1024*16; //一个地图最大的关键点的数目

			
	
			//关键节点的信息，用于
			/*
			typedef struct tagKeyPoint
			{
				MAPPOINT nearPoints[8];  //旁边的8个方向上关键点的id和距离
				MAPPOINT pos;         //本身的坐标
			}KEYPOINT,*PKEYPOINT;
			*/

			//typedef struct tag
			DWORD		m_dwWidth;		//地图宽度
			DWORD		m_dwHeight;		//地图高度
		protected:
			void		*m_pGridData;
			BYTE		*m_pMoveableIndex;//每坐标对应的可移动索引，用于外层优化
			INT			m_nMoveableCount;//移动坐标数量		

			CBufferAllocator*	m_pAllocator;		//内存块管理器

			//PKEYPOINT        m_pWayPointMap;  //存储相邻关键点(waypoint)之间的
			//INT				 m_nWayPointCount;    //关键点的数目

			INT			m_nBBoxT;		// 全空飞包围盒top
			INT			m_nBBoxL;		// 全空飞包围盒left
			INT			m_nBBoxR;		// 全空飞包围盒right
			INT			m_nBBoxB;		// 全空飞包围盒bottom

			
		public:
			CAbstractMap();
			virtual ~CAbstractMap();

			//地图的一个点的值
			//virtual int GetMapPosValue(int x,int y) ;
			

			/* 从流中加载地图数据 */
			bool LoadFromStream(wylib::stream::CBaseStream& stream);
			/* 从文件中加载地图数据 */
			bool LoadFromFile(LPCTSTR sFileName);
			
			/* 生成一张全部可以移动的地图 */
			void initDefault(DWORD dwWidth, DWORD dwHeight);
			
			/* 获取地图宽度 */
			inline int getWidth(){ return m_dwWidth; }
			/* 获取地图高度 */
			inline int getHeight(){ return m_dwHeight; }
			/* 获取可移动坐标数量 */
			inline int getMoveableCount(){ return m_nMoveableCount; }
			/* 获取地图中所有单元数目 */
			inline int getUnitCount() const { return m_dwWidth * m_dwHeight; }
			/* 获取地图中x, y坐标的可移动索引，返回-1表示此位置不可移动，返回s_nInvalidMapPos表示 */

			/*
			inline int getMoveableIndex(DWORD x, DWORD y)
			{
				if ( x >= m_dwWidth || y >= m_dwHeight )
					return s_nInvalidMapPos;
				return m_pMoveableIndex[y * m_dwWidth + x].nValue; 
			}
			*/
			inline bool isValidPos(DWORD x, DWORD y)
			{
				if ( x >= m_dwWidth || y >= m_dwHeight )
					return false;
				return true;
			}
			inline INT_PTR getPosIndex(DWORD x, DWORD y)
			{
				if (!isValidPos(x, y ))
					return s_nInvalidMapPos;

				return y * m_dwWidth + x;
			}

			/* 判断地图中x, y坐标位置是否可以移动，返回true表示坐标可以移动 */
			inline bool canMove(DWORD x, DWORD y)
			{
				if ( x >= m_dwWidth || y >= m_dwHeight )
					return false;
				return m_pMoveableIndex[y * m_dwWidth + x]  != 0; 
			}

			/*设置不可移动点*/
			inline void setUnablePoint(DWORD x, DWORD y, BYTE btValue)
			{
				if ( x >= m_dwWidth || y >= m_dwHeight )
					return;
				m_pMoveableIndex[y * m_dwWidth + x] = btValue;
			}
			/*
			inline INT getWayPointDistance(INT x,INT y)
			{
				if(x==y || m_pWayPointMap ==NULL)return -1;
				if( x <=0 || x> m_nWayPointCount || y <=0 || y > m_nWayPointCount) return -1;
				if( x > y )
				{
					return  m_pMoveableIndex[y * m_dwWidth + x].nValue;
				}
				return -1;
			}
			*/

			/* 
			* Comments: 判断一个点是否能够移动
			* Param unsigned int x: 点的坐标x
			* Param unsigned int y: 点的坐标y
			* Param bool & isCrossPoint: 是否是交叉点
			* Param int & nMoveabledDirMask:  能够移动的方向的掩码，按位操作的，比如方向2能够移动，那么第2位将为1
			* @Return bool: 返回是否能够移动
			*/
			/*
			inline bool canMove(unsigned int x,unsigned int y,bool & isCrossPoint,int& nMoveabledDirMask  )
			{
				if ( x >= m_dwWidth || y >= m_dwHeight )
				{
					return false;
				}
				MAPPOINT point= m_pMoveableIndex[y * m_dwWidth + x];
				if(point.nValue == -1 )
				{
					return false ;
				}
				if(point.isKeyPoint) //高16位不为0表示是转湾点
				{
					isCrossPoint = true; 
				}
				else
				{
					isCrossPoint = false; 
				}
				nMoveabledDirMask = point.auxPointID;
				//nMoveabledDirMask =( nValue & 0xFFFF); //低位表示能够移动方向的掩码
				return true;
			}
			*/

			// 
			// 寻点规则：以当前位置为中心的24*24区域（可视区域）内找落脚点（可移动区域）。从中心点连线到6点位置，然后
			// 分左右两边，左边顺时针，右边逆时针转动，汇聚到12点重合。
			
			inline void GetEntityAppearPos(int curPosX, int curPosY, int nRangeMin,  int nRangeMax, int& nDestX, int& nDestY)
			{	
				nDestX = nDestY = -1;
				for (int r = nRangeMin; r <= nRangeMax; r++)
				{
					// 下
					int nTempX = 0;
					int nTempY = curPosY + r;
					if (nTempY < (int)m_dwHeight)
					{
						for (int col = 0; col <= r; col++)
						{
							nTempX = curPosX + col;
							if (canMove(nTempX, nTempY))
							{
								nDestX = nTempX;
								nDestY = nTempY;
								return;
							}	

							if (col != 0)
							{
								nTempX = curPosX - col;
								if (nTempX >= 0 && canMove(nTempX, nTempY))
								{
									nDestX = nTempX;
									nDestY = nTempY;
									return;
								}	
							}
						}
					}					

					// 左
					nTempX = curPosX - r;
					if (nTempX >= 0)
					{
						for (int row = r-1; row > -r; row--)
						{
							nTempY = curPosY + row;
							if (nTempY >= 0 && canMove(nTempX, nTempY))
							{
								nDestX = nTempX;
								nDestY = nTempY;
								return;
							}
						}
					}


					// 右
					nTempX = curPosX + r;
					if (nTempX < (int)m_dwWidth)
					{
						for (int row = r-1; row > -r; row--)
						{
							nTempY = curPosY + row;
							if (nTempY >= 0 && canMove(nTempX, nTempY))
							{
								nDestX = nTempX;
								nDestY = nTempY;
								return;
							}
						}
					}

					// 上
					nTempY = curPosY - r;
					if (nTempY >= 0)
					{
						for (int col = 0; col <= r; col++)
						{
							nTempX = curPosX + col;
							if (canMove(nTempX, nTempY))
							{
								nDestX = nTempX;
								nDestY = nTempY;
								return;
							}	

							if (col != 0)
							{
								nTempX = curPosX - col;
								if (nTempX >= 0 && canMove(nTempX, nTempY))
								{
									nDestX = nTempX;
									nDestY = nTempY;
									return;
								}	
							}
						}
					}
				}

				// 如果在可视区域找不到，就在垂直方向找一个点
				int nTempX = curPosX;
				for (int i = nRangeMin+1; i < (int)m_dwWidth; i++)
				{
					if (canMove(nTempX, curPosY+i))
					{
						nDestX = nTempX;
						nDestY = curPosY+i;
						return;
					}
				}

				for (int i = -nRangeMin-1; i >= 0; i--)
				{
					if (canMove(nTempX, curPosY+i))
					{
						nDestX = nTempX;
						nDestY = curPosY+i;
						return;
					}
				}
			}

		private:
			/* 
			* Comments: 初始化1个点旁边的2个方向的点信息
			* Param int nPosX1: 点1的x
			* Param int nPosY1: 点1的y
			* Param int nPosX2: 点2的x
			* Param int nPosY2: 点2的y
			* Param int nDir1: 点1相对基础点的方向
			* Param int nDir2: 点2相对基础点的方向
			* Param int nWidth: 长度
			* Param int nHeight: 宽度
			* Param int & nDirMask: 方向的mask
			* @Return bool: 如果2个方向有一个能走就返回true,否则返回false
			*/
			/*
			inline bool Init2DirInfo(int nPosX1,int nPosY1,int nPosX2,int nPosY2, int nDir1,int nDir2, int &nDirMask,int nWidth,int nHeight)
			{
				bool dir1Moveable =false,dir2Moveable=false; //2个对着的方向是否能够通过
				
				if(nPosX1 >=0 && nPosX1 < nWidth && nPosY1 >=0 &&  nPosY1 <  nHeight)
				{
					dir1Moveable = canMove(nPosX1,nPosY1) ;  //点1能否移动
				}
				
				if(nPosX2 >=0 && nPosX2 < nWidth && nPosY2 >=0 &&  nPosY2 <  nHeight)
				{
					dir2Moveable = canMove(nPosX2,nPosY2) ;  //点2能否移动
				}
				if(dir1Moveable)
				{
					nDirMask |= (1 << nDir1);
				}
				if(dir2Moveable)
				{
					nDirMask |= (1 << nDir2);
				}
				return (dir1Moveable ||  dir2Moveable);
			}
			*/

		};
	};
};
