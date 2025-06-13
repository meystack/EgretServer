/*
	创建排行榜的相关接口
*/


//tolua_begin
namespace Ranking
{

	/*
	* Comments:增加引用值
	* Param void * pRank:
	* @Return void:
	*/
	void addRef(void* pRank);

	/*
	* Comments:减少引用值
	* Param void * pRank:
	* @Return void:
	*/
	void release(void* pRank);

	/*
	* Comments:获取引用值
	* Param void * pRank:
	* @Return int:
	*/
	int getRef(void* pRank);

	/*
	* Comments:获取排行榜的指针
	*/
	void* getRanking(int rankId);

	/*
	* Comments:增加一个排行榜
	* Param char * sName:排行榜的名称
	* Param int nMax:发给客户端最多的行数
	* Param boDisplay:是否在客户端显示，默认是0，不显示，1显示
	* @Return void*:添加成功会返回排行榜的指针，返回NULL
	*/
	void* add(int rankId,char* sName,int nMax,int boDisplay=0, int nBroadCount = 0);

	/*
	* Comments:排行榜从文件中读入数据
	* Param CRanking * pRank:排行榜的指针
	* Param char * sFile:指定的文件，文件保存在当前目录的 ph/XXX,传入XXX就行
	* @Return bool:如果文件存在并读取成功返回true
	*/
	bool load(void* pRank,char* sFile);

	/*
	* Comments:排行榜保存数据到文件
	* Param void * pRank:
	* Param char * sFile:文件名，不需要包含路径
	* Param forceSave: 强制保存
	* @Return bool:是否保存成功
	*/
	bool save(void* pRank,char* sFile, bool bForceSave = false);

	/*
	* Comments:增加一列
	* Param CRanking * pRank:
	* Param char * sTitle:列的标题名称
	* Param int nIndex:-1表示在最后增加，否则在指定的列前增加
	* Param bool bUnique:true表示唯一，不能插入相同列名的列
	* @Return void:
	*/
	void addColumn(void* pRank,char* sTitle, int nIndex = -1, bool bUnique = false);

	/* 
	* Comments: 设置列的标题
	* Param void * pRank: 排行榜的指针
	* Param char * sTitle: 列的标题名称
	* Param int nIndex: 列的索引 由0开始
	* @Return void:  
	*/
	void setColumnTitle(void* pRank, char* sTitle, int nIndex);

	/*
	* Comments:增加一个排行榜项
	* Param void * pRank:排行榜的指针
	* Param int nId:id值
	* Param int nPoint:初始的分值
	* Param bool bRankFlag:true 表示需要检测排序
	* @Return void*:成功返回排行榜项，否则返回NULL,如果已经存在对应的id，会加入失败
	*/
	void* addItem(void* pRank, unsigned int nId, int nPoint, bool bRankFlag = true);

	/*
	* Comments:获取某个id在排行榜中的位置，第一位是0，不在这个返回-1
	* Param int nId:
	* @Return int:
	*/
	int getItemIndexFromId(void* pRank,unsigned int nId);

	/*
	* Comments:根据id获取排行榜项的指针
	* Param void * pRank:
	* Param int nId:
	* @Return void*:不存在返回NULL
	*/
	void* getItemPtrFromId(void* pRank,unsigned int nId);

	/*
	* Comments:更新排行榜项的分值
	* Param void * pRank:
	* Param int nId:id
	* Param int nPoint:增加分值
	* @Return void*:成功会返回对应的排行榜项，失败（不存在）返回NULL
	*/
	void* updateItem( void* pRank,unsigned int nId,int nPoint );

	/*
	* Comments:设置排行榜项的分值
	* Param void * pRank:
	* Param int nId:id
	* Param int nPoint:分值
	* @Return void*:成功会返回对应的排行榜项，失败（不存在）返回NULL
	*/
	void* setItem( void* pRank,unsigned int nId,int nPoint );

	/*
	* Comments:设置某列是否显示在客户端
	* Param void * pRank:排行榜指针
	* Param int nIndex:列索引，第一列是0
	* Param int boDisplay:是否显示，1显示，0不显示，注意：默认是显示的
	* @Return void:
	*/
	void setColumnDisplay( void* pRank, int nIndex ,int boDisplay);

	/*
	* Comments:设置排行榜项的列的显示内容
	* Param void * pRankItem:排行榜项的指针
	* Param int nIndex:第几列,从0开始数
	* Param char * sData:字符串内容
	* @Return void:
	*/
	void setSub(void* pRankItem,int nIndex, char* sData);

	/*
	* Comments:删除指定的排版榜
	*/
	void removeRanking(int rankId);

	/*
	* Comments:获取本排行榜名单的数量
	* Param void * pRank:
	* @Return int:
	*/
	int getRankItemCount(void* pRank);

	/*
	* Comments:获取本排行榜名单的数量
	* Param void * pRank:
	* @Return int:
	*/
	int getRankItemCountByLimit(void* pRank, int nLimit);

	/*
	* Comments:获取玩家的分数
	* Param void * pRankItem:
	* @Return int:
	*/
	int getPoint(void* pRankItem);

	/*
	* Comments:获取排行榜项的id
	* Param void * pRankItem:
	* @Return int:
	*/
	unsigned int getId(void* pRankItem);

	/*
	* Comments:获取名次
	* Param void * pRankItem:
	* @Return int:第一名是0
	*/
	int getIndexFromPtr(void* pRankItem);

	/*
	* Comments:获取某列的字符串数据
	* Param void * pRankItem:排行榜项的指针
	* Param int nIndex:列索引
	* @Return char*:
	*/
	const char* getSub(void* pRankItem,int nIndex);

	
	/*
	* Comments:清除排行榜所有的名单数据
	* Param void * pRank:排行榜的指针
	* @Return void:
	*/
	void clearRanking(void* pRank);

	/*
	* Comments:获取排行榜某个位置的排行榜项
	* Param void * pRank:
	* Param int nIndex:名词，0表示第一名
	* @Return void*:
	*/
	void* getItemFromIndex(void* pRank,int nIndex);

	/*
	* Comments:设置排行榜的显示在客户端的名称
	* Param void * pRank:排行榜的指针
	* Param const char * sName:名称
	* @Return void:
	*/
	void setDisplayName(void* pRank, const char* sName);

	/*
	* Comments:设置id的显示标题
	* Param void * pRank:
	* Param const char * sName:
	* @Return void:
	*/
	void setIdTitle(void* pRank,const char* sName);

	/*
	* Comments:设置分值的显示标题
	* Param void * pRank:
	* Param const char * sName:
	* @Return void:
	*/
	void setPointTitle(void* pRank, const char* sName);

	/*
	* Comments:删除id
	* Param void * pRank:
	* Param int id:
	* @Return void:
	*/
	void removeId(void* pRank, unsigned int id);

	/*
	* Comments:更新跨服排行榜
	*/
	void UpdateCsRank(int rankId);

	///*
	//* Comments:查找是否参与第几次报名
	//* Param void * pRank:
	//* Param int nId:角色id
	//* Param int nType:报名的类型,type按数字顺序0、1、2、3 类推
	//* @Return bool:
	//*/
	//bool queryIdType( void* pRank, int nId, int nType);

	///*
	//* Comments:增加一个报名
	//* Param void * pRank:
	//* Param int nId:角色id
	//* Param int nType:报名类型
	//* @Return void:
	//*/
	//void addIdType( void* pRank, int nId, int nType );

	//设置属性排行榜配置数据 nRankPropIndex:属性配置表对应的配置索引 从0开始 ,nRankPropMax:最大配置排名
	void setRankPropConfig(void* pRank, int nRankPropIndex);

	/// 获取排行榜列数
	/// @param pRank 排行榜指针
	int GetRankColumnCount(void* pRank);

	void forceSave(void* pRank,char* sFile);
	bool CheckActorIdInRank(void* pRank,unsigned int id);
};

//tolua_end
