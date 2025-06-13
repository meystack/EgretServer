#include "stdafx.h"

int CUtility::GetDir(int nSourceX, int nSourceY, int nTargetX, int nTargetY)
{
    int nDir = DIR_UP;
    int nSelfPosx = nTargetX - nSourceX;
    int nSelfPosy = nTargetY - nSourceY;
    
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
    return nDir;
}