
#include "AStar.h"
#include "stdafx.h"

namespace Util
{

    const int AStar::AS_MOVECOST[] = {AStar::DMOVECOST,AStar::RMOVECOST};
    const int AStar::NEIGHBORPOS_X_VALUES[] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int AStar::NEIGHBORPOS_Y_VALUES[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    //CObjectAllocator<ASMapCell> AStar::m_ASMapCellAllocator = CObjectAllocator<ASMapCell>(_T("ASMapCellAlloc"));
    
    void AStar::initFromMap(CAbstractMap* pMap)
    {
        this->m_nWidth = pMap->m_dwWidth;
        this->m_nHeight = pMap->m_dwHeight;

        // 调整网格地图格子列表
        int nOldCount = this->m_ASMapCells.size();
        int nNewCount = this->m_nWidth * this->m_nHeight;
        if (nOldCount < nNewCount)
        {
            this->m_ASMapCells.resize(nNewCount);
            for (int i = nOldCount; i < nNewCount; ++i)
            {
                this->m_ASMapCells[i] = m_ASMapCellAllocator.allocObjects();
                memset(this->m_ASMapCells[i], 0, sizeof(ASMapCell));
            }
        }
        
        //初始化每个坐标格子
        int x = 0, y = 0, idx = 0;
        ASMapCell* pCell;
        for (y = 0; y < this->m_nHeight; ++y)
        {
            for (x = 0; x < this->m_nWidth; ++x)
            {
                pCell = this->m_ASMapCells[idx];
                pCell->X = x;
                pCell->Y = y;
                pCell->CanNotMove = !pMap->canMove(x, y);
                idx++;
            }
        }
    }

    bool AStar::getPath(int fromX, int fromY, int targetX, int targetY, std::vector<SMALLCPOINT> &outPaths, CrossCheckFuncType& crossCheck,bool isLoopLimit)
    {
        int nLoopLimit = 2000;

        // 可行检测

        if (fromX == targetX && fromY == targetY)
            return false;
        
        if (fromX < 0 || fromX >= this->m_nWidth || fromY < 0 || fromY >= this->m_nHeight)
            return false;
        
        if (targetX < 0 || targetX >= this->m_nWidth || targetY < 0 || targetY >= this->m_nHeight)
            return false;

        // 获取目标格子数据
        try
        {
            ASMapCell* ac = this->m_ASMapCells.at(targetY * this->m_nWidth + targetX);
            if (ac->CanNotMove) return false;
            //if (!m_pScene->CanCross(nEntityType, targetX, targetY)) return false;
            
            this->reset(fromX, fromY);
        }
        catch(...)
        {
            return false;
        }
        
        // 初始化起始点
        ASMapCell* pCurCell = NULL;
        bool boPathFound = false;
        int nCurX = fromX, nCurY = fromY;
        try
        {
            pCurCell = this->m_ASMapCells.at(nCurY * this->m_nWidth + nCurX);
            pCurCell->GCost = 0;
            pCurCell->LastX = -1;
            pCurCell->LastY = -1;
            pCurCell->X = nCurX;
            pCurCell->Y = nCurY;
            pCurCell->MarkTag = this->m_nMarkTag;
            pCurCell->HCost = (std::abs(targetX - fromX) + std::abs(targetY - fromY)) * 10;
        }
        catch(...)
        {
            return false;
        }
        
        // 路径查找
        int i = 0, nX = 0, nY = 0;
        ASMapCell* pCell = NULL;
        while (true)
        {
            if (nCurX == targetX && nCurY == targetY)
            {
                boPathFound = true;
                break;
            }
                
            if (pCurCell->State != ASMapCell::CSCLOSE)
            {
                this->closeCell(pCurCell);
            }

            //遍历当前位置周围的8个格子
            for (i = 0; i < 8; i++)
            {
                if (isLoopLimit && (nLoopLimit-- < 0))
                {
                    return false;
                }
                
                nX = nCurX + AStar::NEIGHBORPOS_X_VALUES[i];
                nY = nCurY + AStar::NEIGHBORPOS_Y_VALUES[i];

                if (nX < 0 || nX >= this->m_nWidth || nY < 0 || nY >= this->m_nHeight)
                    continue;

                try
                {
                    pCell = this->m_ASMapCells.at(nY * this->m_nWidth + nX);
                }
                catch(...)
                {
                    return false;
                }

                if (pCell->CanNotMove) continue;
                if (nX == targetX && nY == targetY)
                {
                   // 优化一下，最终点可能为玩家站立点，让其为可走的位置
                }
                else if (!crossCheck(nX, nY)) continue;

                //pCell->MarkTag与当前的m_nMarkTag不同，也视为是未开启状态
                if (pCell->MarkTag != this->m_nMarkTag || pCell->State == ASMapCell::CSNONE)
                {
                    pCell->MarkTag = this->m_nMarkTag;
                    pCell->LastX = nCurX;
                    pCell->LastY = nCurY;
                    pCell->btDir = i;
                    pCell->GCost = pCurCell->GCost + AStar::AS_MOVECOST[i & 1];
                    pCell->HCost = (std::abs(targetX - nX) + std::abs(targetY - nY)) * 10;
                    pCell->pNext = NULL;
                    this->openCell(pCell);
                }
                else if (pCell->State == ASMapCell::CSOPEN)
                {
                    if (pCell->GCost > pCurCell->GCost + AStar::AS_MOVECOST[i & 1])
                    {
                        pCell->LastX = nCurX;
                        pCell->LastY = nCurY;
                        pCell->btDir = i;
                        pCell->GCost = pCurCell->GCost + AStar::AS_MOVECOST[i & 1];
                        this->reopenCell(pCell);
                    }
                }

            }// end for

            pCurCell = this->m_LastOpenCell;
            if (pCurCell == NULL)
                break;

            nCurX = pCurCell->X;
            nCurY = pCurCell->Y;

        }// end while
        
        // 找到路径，则存储
        SMALLCPOINT gridNode;
        if (boPathFound)
        {
            outPaths.clear();
            gridNode.MakePoint(pCurCell->X,pCurCell->Y,pCurCell->btDir);
            outPaths.push_back(gridNode);

            while (true)
            {
                try
                {
                    pCurCell = this->m_ASMapCells.at(pCurCell->LastY * this->m_nWidth + pCurCell->LastX);
                }
                catch(...)
                {
                    return false;
                }
                gridNode.MakePoint(pCurCell->X,pCurCell->Y,pCurCell->btDir);
                outPaths.push_back(gridNode);

                if ((pCurCell->LastX <= 0 && pCurCell->LastY <= 0) || pCurCell->MarkTag != this->m_nMarkTag)
                    break;
            }
            return true;
        }

        return false;
    }

    void AStar::reset(int cX, int cY)
    {
        ASMapCell* pCell = this->m_ASMapCells.at(cY * this->m_nWidth + cX);
        pCell->LastX = 0;
        pCell->LastY = 0;
        pCell->HCost = 0;
        pCell->GCost = 0;
        pCell->FValue = 0;
        pCell->State = 0;
        pCell->pPre = NULL;
        pCell->pNext = NULL;
        pCell->btDir = 0;

        this->m_LastOpenCell = NULL;
        this->m_nMarkTag = this->m_nMarkTag + 1;
    }

    void AStar::closeCell(ASMapCell* pCell)
    {
        //如果格子已经开启则进行路径链表的移除操作
        if (pCell->State == ASMapCell::CSOPEN)
        {
            if (pCell->pPre) pCell->pPre->pNext = pCell->pNext;
            if (pCell->pNext) pCell->pNext->pPre = pCell->pPre;
            if (pCell == this->m_LastOpenCell) this->m_LastOpenCell = pCell->pPre;
        }
        pCell->State = ASMapCell::CSCLOSE;
    }

    void AStar::openCell(ASMapCell* pCell)
    {
        pCell->State = ASMapCell::CSOPEN;
        int nFValue = pCell->HCost + pCell->GCost;
        pCell->FValue = nFValue;

        ASMapCell* pLastCell = this->m_LastOpenCell;
        if (!pLastCell)
        {
            this->m_LastOpenCell = pCell;
            pCell->pPre = NULL;
            pCell->pNext = NULL;
        }
        else
        {
            //开启格子的时候在已开启的格子链表中按移动估价值进行排序 TODO 这里需要解决死循环
            while (pLastCell->FValue < nFValue)
            {
                if (pLastCell->pPre == NULL)
                {
                    pLastCell->pPre = pCell;
                    pCell->pPre = NULL;
                    pCell->pNext = pLastCell;
                    return;
                }
                pLastCell = pLastCell->pPre;
            }

            //添加到当前开启格子链表的末尾
            pCell->pPre = pLastCell;
            if (pLastCell->pNext)
            {
                pCell->pNext = pLastCell->pNext;
                pLastCell->pNext->pPre = pCell;
                pLastCell->pNext = pCell;
            }
            else
            {
                pCell->pNext = NULL;
                pLastCell->pNext = pCell;
                this->m_LastOpenCell = pCell;
            }
        }
    }

    void AStar::reopenCell(ASMapCell* pCell)
    {
        int nFValue = pCell->HCost + pCell->GCost;
        pCell->FValue = nFValue;

        ASMapCell* pNextCell = pCell->pNext;
        if (pNextCell && pNextCell->FValue > nFValue)
        {
            do
            {
                pNextCell = pNextCell->pNext;
            }
            while (pNextCell && pNextCell->FValue > nFValue);

            if (pCell->pPre) pCell->pPre->pNext = pCell->pNext;
            if (pCell->pNext) pCell->pNext->pPre = pCell->pPre;

            if (pNextCell)
            {
                pCell->pNext = pNextCell;
                if (pNextCell->pPre)
                {
                    pCell->pPre = pNextCell->pPre;
                    pNextCell->pPre->pNext = pCell;
                }
                else pCell->pPre = NULL;
                pNextCell->pPre = pCell;
            }
            else
            {
                pCell->pPre = this->m_LastOpenCell;
                pCell->pNext = NULL;
                this->m_LastOpenCell->pNext = pCell;
                this->m_LastOpenCell = pCell;
            }
        }
    }

} // namespace Util
