
#include "StdAfx.h"
#include "SkillSubSystem.h"
INT_PTR CSkillSubSystem::GetDirection(INT_PTR nX, INT_PTR nY, INT_PTR nTargetX, INT_PTR nTargetY)
{
	INT_PTR Result = 4;
	if (nTargetX < nX)
	{
		if (nTargetY < nY)
			Result = 7;
		else if (nTargetY == nY)
			Result = 6;
		else Result = 5;
	}
	else if (nTargetX == nX)
	{
		if (nTargetY < nY)
			Result = 0;
		else Result = 4;
	}
	else
	{    
		if (nTargetY < nY)
			Result = 1;
		else if (nTargetY == nY)
			Result = 2;
		else Result = 3;
	}
	return Result;
}

void CSkillSubSystem::GetPosition(INT_PTR nX, INT_PTR nY, INT_PTR nDir, INT_PTR nStep, INT_PTR &nNewX, INT_PTR &nNewY)
{
	switch(nDir & 7)
	{
	case 0:
		nNewX = nX;
		nNewY = nY - nStep;
		break;
	case 1:
		nNewX = nX + nStep;
		nNewY = nY - nStep;
		break;   
	case 2:
		nNewX = nX + nStep;
		nNewY = nY;
		break;
	case 3:
		nNewX = nX + nStep;
		nNewY = nY + nStep;
		break;
	case 4:
		nNewX = nX;
		nNewY = nY + nStep;
		break;
	case 5:
		nNewX = nX - nStep;
		nNewY = nY + nStep;
		break; 
	case 6:
		nNewX = nX - nStep;
		nNewY = nY;
		break; 
	case 7:
		nNewX = nX - nStep;
		nNewY = nY - nStep;
		break;                
	}
}

/*  计算坐标旋转
 *  cX 旋转中心X
 *  cY 旋转中心Y
 *  nX 基于中心位置的X相对坐标
 *  nY 基于中心位置的Y相对坐标
 *  nDir 旋转方向(0到7)
 *  &nNewX 输出旋转后的绝对坐标X
 *  &nNewY 输出旋转后的绝对坐标Y
 */
void CSkillSubSystem::PositionRotation(INT_PTR cX, INT_PTR cY, INT_PTR nX, INT_PTR nY, INT_PTR nDir, INT_PTR &nNewX, INT_PTR &nNewY)
{
	INT_PTR nDistX, nDistY, nFixedY, nFixedX, nNewDir;

	nDir = nDir & 7;
	if ((nDir == 0) || ((nX == cX) && (nY == cY)))
	{
		nNewX = nX;
		nNewY = nY;
	}
	else
	{  
		nDistX = nX - cX;
		nDistY = nY - cY;

		if (nDistX == 0)
		{
			nDir = (GetDirection( cX, cY, nX, nY ) + nDir) & 7;
			if (nDistY < 0)
				nDistY = -nDistY;
			GetPosition( cX, cY, nDir, nDistY, nNewX, nNewY );
		}
		else if (nDistY == 0) 
		{
			nDir = (GetDirection( cX, cY, nX, nY ) + nDir) & 7;
			if (nDistX < 0) 
				nDistX = -nDistX;
			GetPosition( cX, cY, nDir, nDistX, nNewX, nNewY );
		}
		else if (abs(nDistX) == abs(nDistY)) 
		{
			nDir = (GetDirection( cX, cY, nX, nY ) + nDir) & 7;
			if (nDistX < 0)
				nDistX = -nDistX;
			GetPosition( cX, cY, nDir, nDistX, nNewX, nNewY );
		}         
		else 
		{
			if (nDistX < 0)
			{
				if (nDistY < 0) //fourth quadrant
				{
					if (nDistX < nDistY) 
					{
						nFixedY = nDistX - nDistY;
						nNewY = nY + nFixedY;
						nNewDir = (GetDirection( cX, cY, nX, nNewY ) + nDir) & 7;   
						GetPosition( cX, cY, nNewDir, -nDistX, nNewX, nNewY );
						switch(nDir)
						{
						case 1:
						case 2:
							nNewX = nNewX + nFixedY;
							break;
						case 3:
						case 4:
							nNewY = nNewY + nFixedY;
							break;
						case 5:
						case 6:
							nNewX = nNewX - nFixedY;
							break;
						case 7:
							nNewY = nNewY - nFixedY;
							break;
						}
					}
					else 
					{
						nFixedX = nDistY - nDistX;
						nNewX = nX + nFixedX;
						nNewDir = (GetDirection( cX, cY, nNewX, nY ) + nDir) & 7;   
						GetPosition( cX, cY, nNewDir, -nDistY, nNewX, nNewY );
						switch(nDir)
						{
						case 1:
							nNewX = nNewX - nFixedX;
							break;
						case 2:
						case 3:
							nNewY = nNewY - nFixedX;
							break;
						case 4:
						case 5:
							nNewX = nNewX + nFixedX;
							break;
						case 6:
						case 7:
							nNewY = nNewY + nFixedX;
							break;
						}
					}
				}
				else 
				{
					if (-nDistX < nDistY) //thired quadrant
					{
						nFixedX = nDistY + nDistX;
						nNewX = nX - nFixedX;
						nNewDir = (GetDirection( cX, cY, nNewX, nY ) + nDir) & 7;   
						GetPosition( cX, cY, nNewDir, nDistY, nNewX, nNewY );
						switch(nDir)
						{
						case 1:
						case 2:
							nNewY = nNewY + nFixedX;
							break;
						case 3:
						case 4:
							nNewX = nNewX - nFixedX;
							break;
						case 5:
						case 6:
							nNewY = nNewY - nFixedX;
							break;
						case 7:
							nNewX = nNewX + nFixedX;
							break;
						}
					}
					else 
					{    
						nFixedY = nDistX + nDistY;
						nNewY = nY - nFixedY;
						nNewDir = (GetDirection( cX, cY, nX, nNewY ) + nDir) & 7;
						GetPosition( cX, cY, nNewDir, -nDistX, nNewX, nNewY );
						switch(nDir)
						{
						case 1: 
							nNewY = nNewY + nFixedY;
							break;
						case 2:
						case 3:
							nNewX = nNewX - nFixedY;
							break;
						case 4:
						case 5:
							nNewY = nNewY - nFixedY;
							break;
						case 6:
						case 7:
							nNewX = nNewX + nFixedY;
							break;
						}
					}
				}
			}
			else 
			{
				if (nDistY < 0) //first quadrant
				{     
					if (-nDistX < nDistY)
					{          
						nFixedY = nDistX + nDistY;
						nNewY = nY - nFixedY;
						nNewDir = (GetDirection( cX, cY, nX, nNewY ) + nDir) & 7;
						GetPosition( cX, cY, nNewDir, nDistX, nNewX, nNewY );
						switch(nDir)
						{
						case 1: 
							nNewY = nNewY + nFixedY;
							break;
						case 2:
						case 3:
							nNewX = nNewX - nFixedY;
							break;
						case 4:
						case 5:
							nNewY = nNewY - nFixedY;
							break;
						case 6:
						case 7:
							nNewX = nNewX + nFixedY;
						break;
						}
					}
					else 
					{    
						nFixedX = nDistY + nDistX;
						nNewX = nX - nFixedX;
						nNewDir = (GetDirection( cX, cY, nNewX, nY ) + nDir) & 7;
						GetPosition( cX, cY, nNewDir, -nDistY, nNewX, nNewY );  
						switch(nDir)
						{
						case 1:
						case 2:
							nNewY = nNewY + nFixedX;
							break;
						case 3:
						case 4:
							nNewX = nNewX - nFixedX;
							break;
						case 5:
						case 6:
							nNewY = nNewY - nFixedX;
							break;
						case 7:
							nNewX = nNewX + nFixedX;
							break;
						}
					}
				}
				else //second quadrant 
				{ 
					if (nDistX < nDistY)
					{      
						nFixedX = nDistY - nDistX;
						nNewX = nX + nFixedX;
						nNewDir = (GetDirection( cX, cY, nNewX, nY ) + nDir) & 7;   
						GetPosition( cX, cY, nNewDir, nDistY, nNewX, nNewY );
						switch(nDir)
						{
						case 1:
							nNewX = nNewX - nFixedX;
							break;
						case 2:
						case 3:
							nNewY = nNewY - nFixedX;
							break;
						case 4:
						case 5:
							nNewX = nNewX + nFixedX;
							break;
						case 6:
						case 7:
							nNewY = nNewY + nFixedX;
							break;
						}
					}
				else 
				{   
					nFixedY = nDistX - nDistY;
					nNewY = nY + nFixedY;
					nNewDir = (GetDirection( cX, cY, nX, nNewY ) + nDir) & 7;   
					GetPosition( cX, cY, nNewDir, nDistX, nNewX, nNewY );
					switch(nDir)
					{
					case 1:
					case 2:
						nNewX = nNewX + nFixedY;
						break;
					case 3:
					case 4:
						nNewY = nNewY + nFixedY;
						break;
					case 5:
					case 6:
						nNewX = nNewX - nFixedY;
						break;
					case 7:
						nNewY = nNewY - nFixedY;
						break;
					}
				}
			}
		}
		}
	}     
}

/*
* Comments:精确旋转,(nX,nY)旋转到(nRotateTargetX,nRotateTargetY)方向以后得到新的位置（nNewX，nNewY）
* Param int nRotateTargetX:旋转的目标点的x
* Param int nRotateTargetY:旋转的目标点的y
* Param int nX:要旋转的点x
* Param int nY:要旋转的点y
* Param int & nNewX:新坐标x
* Param int & nNewY:新坐标y
* @Return void:
*/
void  CSkillSubSystem::AccurateRotation( INT_PTR nRotateTargetX,INT_PTR nRotateTargetY,INT_PTR nX,INT_PTR nY,INT_PTR &nNewX,INT_PTR &nNewY)
{
	
	if(nRotateTargetX ==0) //如果是在y轴上，
	{
		
		//不旋转
		if(nRotateTargetY ==0)
		{
			nNewX = nX;
			nNewY = nY;
		}
		else if(nRotateTargetY >0)
		{
			
			nNewX = 0;
			double R = (double)(nX * nX + nY * nY); //旋转半径的平方和
			nNewY = (int) sqrt(R);
		}
		else 
		{
			nNewX = 0;
			double R = (double)(nX * nX + nY * nY); //旋转半径的平方和
			nNewY = (int)(-sqrt(R));
		}
	}
	else
	{
		double k2= (double)(nRotateTargetY * nRotateTargetY) /(nRotateTargetX * nRotateTargetX); //k的平方
		
		double R =(double) (nX * nX + nY * nY); //旋转半径的平方和
		if(nRotateTargetX >=0)
		{
			nNewX =(INT_PTR) sqrt ( R/(1 + k2 ));
		}
		else
		{
			nNewX =-1* (INT_PTR)sqrt ( R/(1 + k2 ));
		}
		nNewY = (INT_PTR)((double)nRotateTargetY/nRotateTargetX  * nNewX); //y= kx
	}

}