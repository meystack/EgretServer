/*
* 优化过的B*寻路的实现
* 主要特色是不申请内存
*/

#pragma once

#include "../include/VirturalMap.h"
#include <_memchk.h>
//#include "CommonDef.h"

//#include <Windows.h>
//#include <tchar.h>

#include "_ast.h"
#include "List.h"

//点的坐标
struct SMALLCPOINT
{
	unsigned short  x;     //
	unsigned short  y;
	unsigned short  dir;
        
	//设置是否合法
	inline bool isInvalid() { return x== 0xFF && y== 0xFF;}

	//设置点非法
	inline void SetInvalid() {x= 0xFF;y=0xFF;}

	inline void MakePoint(int _x,int _y, int _dir = NULL) {x=(unsigned short)_x; y=(unsigned short)_y; dir=(unsigned short)_dir;}
	
	inline SMALLCPOINT &operator =(const SMALLCPOINT &cval)   {x=cval.x; y=cval.y;  dir=cval.dir; return *this;}
	inline bool		operator ==(const SMALLCPOINT &cval)  {return (x==cval.x && y==cval.y && dir==cval.dir);}
	inline bool		operator !=(const SMALLCPOINT &cval)  {return !(this->operator==(cval));}
};

 

enum tagAstarResultCode
{
	enSearchTooManyPoint =-1, //太多的寻路点
	enSearchUnReach =-2,      //目标掉不可达到
	enSearchParamError=-3,    //参数错误
	enSearchCalError=-4,    //内部计算错误
	enSearchTooLong =-5,    //距离过远
	enSearchError =-6,      //计算错误，进入了很大的循环 
};



//优化过的A*寻路算法
class CAdvanceAstar
{
public: 
	CAdvanceAstar()
	{
		m_bHasInit =false;
	};
	const static int SIZE =64;  //矩阵的一半大小
	const static int MATRIX_ARRAY_LENGTH = 2*SIZE +1;
	/*
	* Comments:从起点到终点的寻路,按目前的设置，起点到终点长、宽只能在64*64的范围里，超过了则不寻路了
	* Param VirturalMap * pMap:地图
	* Param int nMapWidth:地图的宽度
	* Param int nMapHeight:地图的高度
	* Param int nStartPosX:起始点x
	* Param int nStartPosY:起始点y
	* Param int nEndPosX:结束点x
	* Param int nEndPosY:结束点y
	* Param int nLoopTime:最大的循环次数
	* Param int nParam : 附加的参数，比如实体类型，用于查找是否能够寻路的点
	* Param int nParam2 : 附加的参数，比如实体类型，用于查找是否能够寻路的点
	* @Return int:0表示成功，<0 返回的是错误码，在tagAstarResultCode中定义
	*/
	int Search(VirturalMap * pMap, int nMapWidth, int nMapHeight,int nStartPosX, int nStartPosY, int nEndPosX,int nEndPosY,int nLoopTime=1000,int nParam=0,int nParam2=0,int nParam3=0,int nParam4=0);

	/*
	* Comments:获得寻路结果中一个点的父节点，
	* Param int x:该点的坐标x
	* Param int y:该点的坐标y
	* Param int& nParentX:父亲节点x
	* Param int& nParentY:父亲节点y
	* @Return bool:如果有父亲节点返回true，否则返回false
	*/
	bool GetParent(int x, int y, int &nParentX,int & nParentY);


	/*
	* Comments:成功寻路以后查找一个点的下一个节点
	* Param INT_PTR x:节点x
	* Param INT_PTR y:节点y
	* Param INT_PTR & nSonX:下一个节点
	* Param INT_PTR & nSonY:下一个节点
	* @Return bool:成功返回true，否则返回false
	*/
	bool GetSon(int x, int y, int &nSonX,int & nSonY);
	
private:

	//初始化寻路参数
	void InitAstar();


private:
	wylib::container::CBaseList<SMALLCPOINT>  m_openList;  //open列表
	char				m_visitedTable[MATRIX_ARRAY_LENGTH][MATRIX_ARRAY_LENGTH];		//状态的表
	SMALLCPOINT         m_parentTable[MATRIX_ARRAY_LENGTH][MATRIX_ARRAY_LENGTH];        //父亲节点的坐标

	int                 m_nOffsetX;    //坐标变化的误差
	int                 m_nOffsetY;    //坐标误差

	char				m_dirDistance_x[8];  //8个方向x
	char                m_dirDistance_y[8];  //8个方向y

	char                m_dir[24];         //8个方向，为了加快方向的速度，保存起来查找
	char                m_matrix_x[8];   //2方向查找矩阵
	char                m_matrix_y[8];
	
	bool                m_bHasInit;   //是否初始化完毕了

	int                 m_nDesPosX;    //目标点的位置x
	int					m_nDesPosY;    //目标点的位置y

};


