/*
	导出一些功能接口给测试脚本使用
*/


//tolua_begin
namespace TestSys
{
	/*
	* Comments: 设置玩家等级
	* Param void * sysarg:玩家对象指针
	* Param int nLevel:要设置的目标等级
	* @Return void:
	* @Remark:
	*/
	void setLevel(void* sysarg, int nLevel);

	/*
	* Comments: 通过Id添加物品
	* Param void * sysarg: 玩家对象指针
	* Param int nItemId:物品Id
	* Param int nCount:物品数量
	* Param int nQuality:物品品质
	* Param int nStrong:物品强化等级
	* Param bool bBind: 是否绑定
	* @Return int: 返回添加到背包中的物品数量
	* @Remark:
	*/
	int addItemById(void* sysarg, int nItemId, int nCount = 1, int nQuality = 0, int nStrong = 0, bool bBind = true);

};


//tolua_end