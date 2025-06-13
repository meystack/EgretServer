#pragma once

class CCustomLogicLuaConfig;
class CFuBen;
class CFuBenManager;
class CScene;
class CEnvirConfig;
class CLogicEngine;


class CEnvirConfig:
	public CCustomLogicLuaConfig
{
public:
	CEnvirConfig();
	~CEnvirConfig();

	bool LoadConfig(LPCTSTR sFileName);
	
	bool LoadSceneConfig();

	bool LoadNpcConfig( SCENECONFIG* sceneConf ) ;

	bool LoadRefreshConfig( SCENECONFIG* sceneConf ) ;

	bool LoadTelePortConfig( SCENECONFIG* sceneConf ) ;

	bool LoadAreasConfig( SCENECONFIG* sceneConf );

	bool LoadAreaCenterConfig( SCENECONFIG* sceneConf, SCENEAREA* area );

	bool LoadAreaAttriConfig( SCENECONFIG* sceneConf, SCENEAREA* area );

	bool LoadAreaRangeConfig( SCENECONFIG* sceneConf, SCENEAREA* area );

	bool LoadFuBenProgressTitleAndTips();

protected:
	void showError(LPCTSTR sError);
public:
	static bool checkParam(SCENEAREA* pArea,INT_PTR nType);

	static CBufferAllocator*			m_pAllocator;	// 用于作为NPC脚本编译后二进制数据流的DataPacket对象的内存分配器
private:
	CObjectAllocator<char>		m_DataAllocator;
	
	
};
