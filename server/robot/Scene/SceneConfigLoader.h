#pragma once
#include "JXAbsGameMap.h"
#include <functional>
typedef struct tagNpcName
{
	int		nId;
	char	sName[32];
}NPCNAME,*PNPCNAME;

struct tagSceneMapConf: public tagSceneConfig
{
	jxcomm::gameMap::CAbstractMap		m_MapData;		//地图点数据
	tagSceneMapConf():m_MapData(){}
};
typedef struct tagSceneMapConf SCENEMAPCONF, *PSCENEMAPCONF;

struct SceneMapConfigList
{
	int			 nCount;
	SCENEMAPCONF* pList;
};

class CSceneConfigLoader : public CCustomLogicLuaConfig
{
public:
	const static LPCTSTR szEnvirConfigFileName;
	const static int snNormalFubenID = 0;		// 常规副本（普通场景）的副本Id
	CSceneConfigLoader();
	/*
	* Comments: 加载场景相关的配置文件并且读取配置
	* Param LPCTSTR szFileName: 配置文件名
	* @Return bool:
	* @Remark:
	*/
	bool Load();
	
	/*
	* Comments: 获取场景配置
	* Param int nSceneId: 场景Id
	* @Return SCENECONFIG*:
	* @Remark:
	*/
	SCENEMAPCONF* GetSceneConfig(int nSceneId);

	/*
	* Comments: 获取随机的刷新点坐标
	* Param const int nSceneId:场景Id
	* Param int & nX:
	* Param int & nY:
	* @Return void:
	* @Remark: 用于给挂机找合适的传送点
	*/
	void GetRandomRefreshPos(const int nSceneId, int& nX, int& nY);

	/*
	* Comments: 判断场景指定点是否可以移动
	* Param const int nSceneId: 场景Id
	* Param const int nX: 场景中的位置坐标X
	* Param const int nY: 场景中的位置坐标Y
	* @Return bool: 如果可以移动返回true；否则返回false
	* @Remark:
	*/
	bool CanMove(const int nSceneId, const int nX, const int nY);
	
	/*
	* Comments: 判断指定场景中的某个点是否是传送门
	* Param const int nSceneId: 场景Id
	* Param const int nX: 场景中的位置坐标X
	* Param const int nY: 场景中的位置坐标Y
	* @Return bool: 如果是传送门返回true；否则返回false
	* @Remark:
	*/
	bool IsTeleport(const int nSceneId, const int nX, const int nY);

	/*
	* Comments: 获取场景默认点
	* Param int& nX:
	* Param int& nY:
	* @Return void:
	* @Remark: 默认点是第一个区域的中心店
	*/
	static void GetSceneDefaultPoint(SCENECONFIG* sc, int& nX, int& nY);

	/*
	* Comments:通过场景id和实体id取x,y坐标
	* Param int nSceneId:
	* Param int nEntityId:
	* Param int & nX:
	* Param int & nY:
	* @Return bool:
	*/
	bool GetMonsterPos(int nSceneId, int nEntityId, int & nX, int & nY);

private:
	CObjectAllocator<char>		m_DataAllocator;
	SceneMapConfigList			m_sceneConfigList;	// 场景（公共地图，不处理副本了）
	
protected:
	/*
	* Comments: 输出错误信息
	* Param LPCTSTR sError:
	* @Return void:
	* @Remark:
	*/
	void showError(LPCTSTR sError);

	/*
	* Comments: 读取配置
	* @Return bool:
	* @Remark:
	*/
	bool LoadSceneConfig();

	/*
	* Comments: 加载区域配置
	* Param SCENECONFIG * sceneConf:
	* @Return bool:
	* @Remark:
	*/
	bool LoadAreasConfig(SCENECONFIG* sceneConf);

	/*
	* Comments:加载npc配置
	* Param SCENECONFIG * sceneConf:
	* @Return bool:
	*/
	bool LoadNpcConfig(SCENECONFIG * sceneConf);

	/*
	* Comments: 读取单个FB配置数据
	* Param int nId: 副本索引
	* Param int nFBId: 副本Id
	* @Return bool:
	* @Remark: 配置需要保证副本ID和索引是相同的
	*/
	bool ReadSingleFBConfig(int nId, int nFBId);

	/*
	* Comments: 加载区域范围配置
	* Param SCENECONFIG * sceneConf:
	* Param SCENEAREA * area:
	* @Return bool:
	* @Remark:
	*/
	bool LoadAreaRangeConfig(SCENECONFIG* sceneConf, SCENEAREA* area);

	/*
	* Comments: 加载区域中心配置
	* Param SCENECONFIG * sceneConf:
	* Param SCENEAREA * area:
	* @Return bool:
	* @Remark:
	*/
	bool LoadAreaCenterConfig(SCENECONFIG* sceneConf, SCENEAREA* area);

	/*
	* Comments: 加载传送点配置
	* Param SCENECONFIG * sceneConf:
	* @Return bool:
	* @Remark:
	*/
	bool LoadTelePortConfig(SCENECONFIG* sceneConf);

	/*
	* Comments: 加载挂机点配置
	* Param SCENECONFIG * sceneConf:
	*/
	bool LoadHookConfig(SCENECONFIG* sceneConf);

	/*
	* Comments: 加载刷新点配置
	* Param SCENECONFIG * sceneConf:
	* @Return bool:
	* @Remark:
	*/
	bool LoadRefreshConfig(SCENECONFIG* sceneConf);
	/*
	* Comments: 加载场景地图
	* @Return bool:
	* @Remark:
	*/
	bool LoadSceneMap(SCENEMAPCONF* sceneConf);
};