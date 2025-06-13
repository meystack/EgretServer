#pragma once
#include "AdvanceAstar.h"
#include <vector>
#include <deque>
#include "ObjectAllocator.hpp"
#include <functional>

namespace jxcomm
{
	namespace gameMap
	{
		class CAbstractMap;
    }
}

template<class DATA> class CVector;

using jxcomm::gameMap::CAbstractMap;

//template <typename T>
//class CObjectAllocator;

struct ASMapCell
{
    /**
     * 格子状态值定义
     */
    enum{
        CSNONE = 0,     //未处理的格子
        CSOPEN = 1,     //格子已经开启
        CSCLOSE = 2,    //格子已经关闭
    };
    
    short X = 0;            //格子的X坐标
    short Y = 0;            //格子的Y坐标
    short btDir = 0;
    bool CanNotMove = false;    //是否不可移动
    unsigned short MarkTag = 0;      //用于优化寻路算法效率，免去循环初始化所有节点的开销

    /**
     * 寻路计算过程中的相关参数
     */

    short LastX = 0;        //上一个格子的X坐标
    short LastY = 0;        //下一个格子的Y坐标
    short HCost = 0;
    short GCost = 0;
    short FValue = 0;       //距离目标格子的估价值
    short State = 0;        //状态，表示空闲、开启或关闭
    ASMapCell* pPre = NULL; //上一个格子
    ASMapCell* pNext = NULL; //下一个格子
};

class AStar
{
public:/*寻路算法有关的常量定义*/
        
    static const int RMOVECOST = 14;    // 倾斜方向的移动耗费
    static const int DMOVECOST = 10;    // 直线方向的移动耗费
    static const int AS_MOVECOST[];

    //用于快速计算临近坐标值的优化数据
    // 7 0 1
    // 6   2
    // 5 4 3
    static const int NEIGHBORPOS_X_VALUES[];
    static const int NEIGHBORPOS_Y_VALUES[];

    typedef std::function<bool(INT_PTR,INT_PTR)> CrossCheckFuncType;

private:/*成员数据定义*/

    CObjectAllocator<ASMapCell> m_ASMapCellAllocator;//分配器
    ASMapCell* m_LastOpenCell;               //最后一个开启的格子
    std::vector<ASMapCell*> m_ASMapCells;   //网格地图格子列表，格子的访问索引为 : y * width + x
    int m_nWidth;                           //地图宽度
    int m_nHeight;                          //地图高度
    int m_nMarkTag;                         //寻路优化成员，免去循环初始化所有节点的开销

public:

    AStar()
    : m_ASMapCellAllocator(_T("ASMapCellAlloc")),m_LastOpenCell(NULL),m_nWidth(0),m_nHeight(0),m_nMarkTag(0)
    {  }

    /**
     * 从地图中初始化
     * @param pMap 地图资源指针
     */
    void initFromMap(CAbstractMap* pMap = NULL);

    /**
     * 寻路
     * @param fromX 起始坐标X
     * @param fromY 起始坐标Y
     * @param targetX 目的地坐标X
     * @param targetY 目的地坐标Y
     */
    bool getPath(int fromX, int fromY, int targetX, int targetY, std::vector<SMALLCPOINT> &outPaths, CrossCheckFuncType& crossCheck,bool isLoopLimit);

    /**
     * 重置起点数据
     * @param cX
     * @param cY
     */
    void reset(int cX, int cY);

    /**
     * 关闭指定的格子
     * @param pCell
     */
    void closeCell(ASMapCell* pCell);

    /**
     * 开启指定的格子
     * @param pCell
     */
    void openCell(ASMapCell* pCell);

    /**
     * 重新开启指定的格子更新移动估价值并重新再已开启格子链表中排序
     * @param pCell
     */
    void reopenCell(ASMapCell* pCell);

};







