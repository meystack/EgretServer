#pragma once
#include<map>
#include<vector>
#include<string>

typedef struct DropAttribute
{
	uint8_t     nRate = 0;
	uint8_t     nType = 0;
	int32_t     nValue = 0;
	uint8_t     nFollowrate = 0;
	uint8_t     nFollowcount = 0;
}DROPATTRIBUTE, *PDROPATTRIBUTE;

typedef struct DropAttributeCfg
{
	uint8_t nRate = 0;
	std::vector<DropAttribute> m_vAttribute;
	DropAttributeCfg()
	{
		m_vAttribute.clear();
	}
		
}DROPATTRIBUTECFG, *PDROPATTRIBUTECFG;

typedef struct Dropinfo
{
	int nRate = 0;
	int nType = 0;
	int nId = 0;
	int nCount = 0;
	int nDropId = 0;
	int bNoDrop = 0;
}DROPINFO, *PDROPINFO;

typedef struct GiftDropGroup
{
	int nType = 0;
	std::vector<DROPINFO> vGiftDropGroups;
	int maxRate = 0;
	GiftDropGroup()
	{
		vGiftDropGroups.clear();
	}

}GIFTDROPGROUP, *PGIFTDROPGROUP;

typedef struct GiftDropTable
{
	int nType = 0;
	std::vector<DROPINFO> vGiftDropTables;
	int maxRate = 0;
	int isDropbest = 0;
	SYSTEMTIME startTime;
	SYSTEMTIME endTime;
	std::vector<int> vWeekLimt;
	int nLootType = 0; //捡取类型
	int nLimitServerOpenDay = 0;//开服天数
	int nCloseOpenDay = 0;//关闭
	GiftDropTable()
	{
		vGiftDropTables.clear();
		vWeekLimt.clear();
		memset(&startTime, 0, sizeof(SYSTEMTIME));
		memset(&endTime, 0, sizeof(SYSTEMTIME));
	}
}GIFTDROPTABLE, *PGIFTDROPTABLE;



typedef struct DropGoods
{
	Dropinfo  info;
	std::vector<DropAttribute> v_bestAttr;
	
	void AddDropItemAttr(DropAttribute& attr)
	{
		int count = v_bestAttr.size();
		bool flag = true;
		for(int i = 0; i < count; i++)
		{
			if(v_bestAttr[i].nType == attr.nType)
			{
				flag = false;
				v_bestAttr[i].nValue += attr.nValue;
			}

		}
		if(flag)
		{
			v_bestAttr.push_back(attr);
		}
	}

}DROPGOODS, *PDROPGOODS;

class CUserItem;
//掉落组
class DropGroupCfg:
	protected CCustomLogicLuaConfig
{
public:


	DropGroupCfg();
	~DropGroupCfg();

	//加载配置
	bool ReadDropGroupsConfig();

	bool ReadDropInfosConfig();
	bool ReadDropAttributeConfig();

	int ReadTableConfigByTable(int nDropid, LPCSTR sTableName, std::vector<DROPINFO>& vInfos);
	//加载
	bool LoadDropGroupsConfig(LPCTSTR sFilePath);

	//通过掉落组id获得掉落表id
	void getGiftDropGroup(int dropGroupid, std::vector<int>& result);

	//掉落判断是否在时间内
	//@param GIFTDROPGROUP& group:掉落表的掉落
	bool CheckTimeLimit(GiftDropTable& dropTable);

	//掉落判断是否在限定时间内
	//@param GIFTDROPGROUP& group:掉落表的掉落
	bool CheckWeekLimit(GiftDropTable& dropTable, int week);

	bool CheckLimit(int start, int end, int now);

	//通过掉落表id获得DROPINFO
	void GetGiftDropInfoByDropId(int dropid, std::vector<DROPGOODS>& result);

	//通过掉落组id获得DROPINFO
	void GetGiftDropInfoByDropGroupId(int dropGroupid, std::vector<DROPGOODS>& result);

	void GetDropAttribute(int type, DROPGOODS& item);
	GIFTDROPTABLE* GetDropTableConfig(int droptableid);

	void getDropItemOPParam(int nDropGroupId, std::vector<CUserItem>& dropItems);

private:
	std::map<int, GIFTDROPGROUP> m_GiftDropGroupMaps; //掉落组
	std::map<int, GIFTDROPTABLE> m_GiftDropTableMaps; //掉落表
	std::map<int, DROPATTRIBUTECFG> m_DropAttributeMaps; //掉落属性
};
