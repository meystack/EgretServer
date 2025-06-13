//初始化寻路的一些查找矩阵

#include "AdvanceAstar.h"

//寻路节点的状态
enum tagNodeStatus
{
	enNodeStatusNone, //没有被访问过
	enNodeStatusPath, //已经定好的寻路点
	enNodeStatusBlock,  //过不去
};

	//实体移动的方向
 typedef enum tagDirCode_{
	DIR_UP = 0,		//上
	DIR_UP_RIGHT=1,	//右上
	DIR_RIGHT=2,		//右
	DIR_DOWN_RIGHT=3,	//右下
	DIR_DOWN=4,		//下
	DIR_DOWN_LEFT=5,	//左下
	DIR_LEFT=6,		//左
	DIR_UP_LEFT=7,	//左上	
	DIR_STOP		//不动
}DIRCODE ;
	


void CAdvanceAstar::InitAstar()
{
	m_openList.reserve(MATRIX_ARRAY_LENGTH);

	m_dirDistance_x[0] =0;
	m_dirDistance_y[0] =-1;
	m_dirDistance_x[1] =1;
	m_dirDistance_y[1] =-1;
	m_dirDistance_x[2] =1;
	m_dirDistance_y[2] =0;
	m_dirDistance_x[3] =1;
	m_dirDistance_y[3] =1;
	m_dirDistance_x[4] =0;
	m_dirDistance_y[4] =1;
	m_dirDistance_x[5] =-1;
	m_dirDistance_y[5] =1;
	m_dirDistance_x[6] =-1;
	m_dirDistance_y[6] =0;
	m_dirDistance_x[7] =-1;
	m_dirDistance_y[7] =-1;

	for(INT_PTR i=0; i< ArrayCount(m_dir); i++)
	{
		m_dir[i] = i % 8;
	}

	//查找矩阵，如果与目标有间隔，则分2个方向查找
	m_matrix_x[0] =0;
	m_matrix_y[0] =0;
	m_matrix_x[1] =1;
	m_matrix_y[1] =1;
	m_matrix_x[2] =7;
	m_matrix_y[2] =2;
	m_matrix_x[3] =2;
	m_matrix_y[3] =1;
	m_matrix_x[4] =6;
	m_matrix_y[4] =2;
	m_matrix_x[5] =3;
	m_matrix_y[5] =1;
	m_matrix_x[6] =5;
	m_matrix_y[6] =2;
	m_matrix_x[7] =4;
	m_matrix_y[7] =3;
}




int CAdvanceAstar::Search(VirturalMap * pMap, int nMapWidth, int nMapHeight, int nStartPosX, int nStartPosY, int nEndPosX,int nEndPosY,int nLoopTime,int nParam, int nParam2,int nParam3,int nParam4)
{

	//先判断一下参数是否正确
	if( nMapWidth <=0 || nMapHeight <=0 || 
		nStartPosX <0 || nStartPosX > nMapWidth ||
		nEndPosX <0   || nEndPosX > nMapWidth ||
		nStartPosY <0 || nStartPosY > nMapHeight ||
		nEndPosY <0   || nEndPosY > nMapHeight 		
		)
	{
		return enSearchParamError;
	}

	if(!m_bHasInit )
	{
		//初始化寻路参数
		InitAstar(); 
		m_bHasInit =true;
	}
	int nAbsX = abs(nEndPosX - nStartPosX);
	int nAbsY = abs(nEndPosY - nStartPosY);

	//距离过远
	if( nAbsX >= MATRIX_ARRAY_LENGTH || nAbsY >= MATRIX_ARRAY_LENGTH)
	{
		return enSearchTooLong;
	}

	//矩阵的中心点的坐标
	int xCenter = __min(nStartPosX ,nEndPosX) + nAbsX / 2;
	int yCenter = __min(nStartPosY, nEndPosY) + nAbsY / 2;

	//与中心点的位移
	m_nOffsetX = SIZE - xCenter; 
	m_nOffsetY = SIZE - yCenter;

	m_nDesPosX = nEndPosX;
	m_nDesPosY = nEndPosY;

	//清除掉内存，初始化全部编辑为没有访问
	memset(m_visitedTable,0,sizeof(m_visitedTable));

	m_openList.clear();  //清理掉列表

	//添加起点
	SMALLCPOINT point;
	point.MakePoint(nStartPosX,nStartPosY);
	m_openList.add(point);

	INT_PTR nLoopCount =0; //循环的次数

	//先判断目标点的旁边是否还有点可以移动，如果没有点可以移动，这次寻路失败，根本就不可走
	bool canMove =false;
	for(INT_PTR i=0;i <  ArrayCount(m_dirDistance_x); i++)
	{
		int x = nEndPosX + m_dirDistance_x[i];
		int y = nEndPosY + m_dirDistance_y[i];
		if( pMap->canEntityMove(x,y,nParam,nParam2,nParam3,nParam4)  )
		{
			canMove =true;
			break;
		}
	}
	if(canMove ==false)
	{
		return enSearchUnReach;
	}
	int nRelX = nStartPosX  + m_nOffsetX;
	int	nRelY = nStartPosY  + m_nOffsetY;
	
	if(nRelX<0 || nRelX>= MATRIX_ARRAY_LENGTH || nRelY <0 || nRelY>= MATRIX_ARRAY_LENGTH) return enSearchTooLong;
	
	
	m_visitedTable[nRelX][nRelY] =(char) enNodeStatusPath;

	//INT_PTR nHeadPos =0; //这个列表的头

	//对于考察列表，一个个遍历，如果循环次数过多需要跳出循环
	//这里为了不每次删除内存，内存不删除
	while( m_openList.count() > nLoopCount  && nLoopCount <= nLoopTime)
	{
		point=m_openList[nLoopCount];  //开始总是选里边的那个

		//m_openList.remove(0); //踢出第1个
		nLoopCount ++;        //这里防止死循环
		nRelX = point.x  + m_nOffsetX;
		nRelY = point.y  + m_nOffsetY;

		int nDir ; //和目标的方向

		//下面是计算当前点到目标点的方向，本来是在别的函数里调用的，放到这里，避免一次函数调用
		int nSelfPosx = nEndPosX - point.x;
		int nSelfPosy = nEndPosY - point.y;
		if(nSelfPosx >0)
		{
			if( nSelfPosy >0 )
			{
				nDir =DIR_DOWN_RIGHT  ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_RIGHT;
			}
			else
			{
				nDir =DIR_UP_RIGHT ;
			}
		}
		else if(nSelfPosx ==0)
		{
			if( nSelfPosy >0 )
			{
				nDir =DIR_DOWN  ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_STOP;
			}
			else
			{
				nDir =DIR_UP ;
			}
		}
		else
		{
			if( nSelfPosy >0 )
			{
				nDir = DIR_DOWN_LEFT ;
			}
			else if(nSelfPosy ==0)
			{
				nDir = DIR_LEFT;
			}
			else
			{
				nDir = DIR_UP_LEFT;
			}
		}

		int nNewPosX,nNewPosY,nNewDir;

		bool clockwiseFlag =false,anticlockwiseFlag =false; //顺时针和逆时针是否找到了一个

		int nCount =0;
		for(int i=0; i< ArrayCount(m_matrix_x) && nCount < 2; i++  )
		{

			nNewDir =nDir+m_matrix_x[i];
			nNewPosX =point.x + m_dirDistance_x[m_dir[nNewDir]];
			nNewPosY =point.y+ m_dirDistance_y[m_dir[nNewDir]];

			int nPointType  = m_matrix_y[i]; //点的类型，0表示正前方，1表示顺时针方向，2表示逆时针方向
			if(nNewPosX <0 || nNewPosX >= nMapWidth || nNewPosY <0 || nNewPosY >= nMapHeight)
			{
				nCount ++;
				continue;
			}
			if(nPointType ==1 && clockwiseFlag)
			{
				continue;
			}
			if(nPointType ==2 && anticlockwiseFlag)
			{
				continue;
			}
			nRelX = nNewPosX + m_nOffsetX;
			nRelY = nNewPosY + m_nOffsetY;
			if(nRelX<0 || nRelX>= MATRIX_ARRAY_LENGTH || nRelY <0 || nRelY>= MATRIX_ARRAY_LENGTH) continue; //已经溢出了
			int nNodeValue =m_visitedTable[nRelX][nRelY];

			//如果这个点无法通过
			if(nNodeValue ==enNodeStatusBlock )
			{
				continue;		
			}
			else if(nNodeValue == enNodeStatusNone) //没有开垦的节点	
			{

				if( pMap->canEntityMove(nNewPosX,nNewPosY,nParam,nParam2,nParam3,nParam4)  )
				{
					switch(nPointType)
					{
					case 0://正前方可以走，直接跳出循环
						nCount +=2;
						break;
					case 1: //顺时针
						clockwiseFlag =true;
						nCount ++; 
						break;
					case 2:
						anticlockwiseFlag =true;
						nCount ++; 
						break;
					}

					m_visitedTable[nRelX][nRelY] =(char) enNodeStatusPath;
					SMALLCPOINT * pPoint=&m_parentTable[nRelX][nRelY];

					//如果离目标点的距离小于1了，就认为到了
					pPoint->x = point.x;
					pPoint->y = point.y;

					//取到目标点的估计距离
					int xDistance =  point.x  - nEndPosX;
					if(xDistance <0)
					{
						xDistance =-xDistance;
					}
					int yDistance = point.y - nEndPosY;
					if(yDistance <0)
					{
						yDistance =- yDistance;
					}
					int nTargetDis = (xDistance > yDistance)? xDistance:yDistance; //取2个值的绝对值的最大的那个

					if( nTargetDis <=1) //到周围了
					{
						//如果目标点和当前的点就是一个点的话，不需要处理,如果只是差一个点，也算到了
						if(nTargetDis==1 )
						{
							if(nNewPosX == nEndPosX && nNewPosY== nEndPosY) //如果是这个点就不处理了
							{

							}
							else
							{
								int nTargetRelX =nEndPosX + m_nOffsetX;
								int nTargetRelY = nEndPosY+ m_nOffsetY;

								SMALLCPOINT * pParentPoint=&m_parentTable[nTargetRelX][nTargetRelY];
								pParentPoint->x =nNewPosX;
								pParentPoint->y = nNewPosY;
							}
						}
						return 0;
					}
					else
					{
						pPoint->x = point.x;
						pPoint->y = point.y;
						SMALLCPOINT newPoint;
						newPoint.MakePoint(nNewPosX,nNewPosY);
						m_openList.add(newPoint); //放到列表里
					}

				}
				else
				{
					m_visitedTable[nRelX][nRelY] =(char) enNodeStatusBlock;
				}

			}
			else if(nNodeValue )
			{
				if( 1== nPointType)
				{
					if(!clockwiseFlag)
					{
						clockwiseFlag =true;
						nCount ++;
					}
				}
				else if(2==nPointType)
				{
					if(!anticlockwiseFlag)
					{
						anticlockwiseFlag =true;
						nCount ++;
					}

				}
			}

		}

	}

	//如果循环次数太多，也要退出
	if(nLoopCount >=nLoopTime)
	{
		return enSearchError;
	}
	else
	{
		return enSearchCalError;
	}

}

bool CAdvanceAstar::GetSon(int x, int y, int &nSonX,int & nSonY)
{
	int nLoopCount=100;
	//当前点的坐标
	int nCurrentX = m_nDesPosX;
	int nCurrentY = m_nDesPosY;

	int nParentx,nParenty;
	while(nLoopCount >0)
	{
		if(GetParent(nCurrentX,nCurrentY,nParentx,nParenty))
		{
			if(nParentx == x && nParenty == y)
			{
				nSonX = nCurrentX;
				nSonY = nCurrentY;
				return true;
			}	
			else
			{
				nCurrentX = nParentx;
				nCurrentY = nParenty;
			}
		}
		else
		{
			return false;
		}
		nLoopCount --;
	};
	return false;
}

bool CAdvanceAstar::GetParent(int x, int y, int &nParentX,int & nParentY)
{
	int nRelX = x + m_nOffsetX;
	int nRelY = y + m_nOffsetY;
	if(nRelX<0 || nRelX>= MATRIX_ARRAY_LENGTH || nRelY <0 || nRelY>= MATRIX_ARRAY_LENGTH) return false;
	SMALLCPOINT * pPoint = &m_parentTable[nRelX][nRelY];
	nParentX = pPoint->x;
	nParentY = pPoint->y;
	return true;
}
