#pragma once


class CScene;
class CFuBen;
typedef Handle<UINT>  CSceneHandle;
typedef HandleMgr<CScene,CSceneHandle>	SceneHandleMgr;
//
//typedef struct
//{
//	int nCount;
//	CScene* pList;
//}SceneVector;

typedef Handle<UINT>  CFuBenHandle;

typedef HandleMgr<CFuBen,CFuBenHandle> FuBenHandleMgr;

//#define		SCREEN_WIDTH	2000		//游戏屏幕的宽度
//#define		SCREEN_HEIGHT	1200			//屏幕的高度 1000*600
//#define		GRID_SIZE		64			//每个网格的大小 64*64 正方形
//
//static const int SCREEN_GRID_COL	= SCREEN_WIDTH / GRID_SIZE + 2;		// 一个屏幕多少列网格
//static const int SCREEN_GRID_ROW	= SCREEN_HEIGHT / GRID_SIZE + 2;	//一个屏幕多少行网格
static const int MOVE_GRID_COL_RADIO = 12;				//实体移动时，广播消息的半径
static const int MOVE_GRID_ROW_RADIO = 14;
static const int MIN_MOVE_GRID_COL_RADIO = 9;
	

//消失的观察者距离
static const int BIG_MOVE_GRID_COL_RADIO = 16;				
static const int BIG_MOVE_GRID_ROW_RADIO = 16;


//定义场景操作中的常见错误码
typedef enum tagSceneErr{
	seSucc = 0,	//正确
	seOutRange,	//实体的坐标超出场景范围
	seNoEntity,	//没有这个实体
	seCanNotMove,	//这个位置不能移动
	seErr=0x0fff	//内部错误
}SCENE_ERR;



