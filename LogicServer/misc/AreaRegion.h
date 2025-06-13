#pragma once

class CAreaRegion
{
public:
	CAreaRegion();
	~CAreaRegion();

	/*
	* Comments: 初始化区域地图，设置地图大小（非区域大小）
	* Param INT_PTR uMapWidth: 地图宽度
	* Param INT_PTR uMapHeight: 地图高度
	* @Return void:
	*/
	void setSize(INT_PTR uMapWidth, INT_PTR uMapHeight);
	/*
	* Comments: 添加并填充一个区域
	* Param LPPOINT lpPoints: 用于构成区域的坐标集，坐标集中必须至少包含3个坐标（三点构成面）
	* Param int nNumPoints: 坐标集中的坐标数量
	* Param int nAreaIndex: 区域ID，必须是1到255之间的一个数字
	* Param LPBYTE lpMapData: 用于存储地图区域索引数据的缓冲区，如果调用者提供，则类内部会将数据生成到此缓冲区中。
	*                         缓冲区的大小不得小于地图宽度*地图高度。如果不提供缓冲区，则类会在第一次添加区域的时
	*                         候自动创建一个缓冲区。
	* @Return bool: 返回添加区域成功与否，导致失败的唯一原因是坐标集中的坐标数量少于3个或区域ID不在有效范围内
	*/
	bool addArea(LPPOINT lpPoints, int nNumPoints, int nAreaIndex, LPBYTE lpMapData = NULL);
	/*
	* Comments: 获取区域地图数据
	* @Return PBYTE: 返回一个以BYTE为单位的地图数据，数据中包含了每个坐标的区域索引值，使用[MapWidth * Y + X]定位坐标
	*/
	inline PBYTE getAreaMapData(){ return m_pMapData; }

protected:
	/** 定义填充区域时的区域数据结构，仅类内部使用 **/
	struct AreaDesc
	{
		INT_PTR nLeft;
		INT_PTR nTop;
		INT_PTR nRight;
		INT_PTR nBottom;
		INT_PTR nWidth;
		INT_PTR nHeight;
		LPPOINT lpPoints;
		LPBYTE  lpMapData;
		INT_PTR nPointCount;
		BYTE    btIndex;
	};

	//计算区域所处最小矩形信息
	void calcAreaBounds(IN OUT AreaDesc &area);
	//绘制区域边界
	void drawAreaOutline(const AreaDesc area);
	//填充区域内部
	void fillAreaRegion(const AreaDesc area);
	//以文字输出区域形状，用于调试
	void traceAreaShape(const AreaDesc area);
protected:
	INT_PTR m_nMapWdith;
	INT_PTR m_nMapHeight;
	PBYTE m_pMapData;

	DECLARE_OBJECT_COUNTER(CAreaRegion)
};
