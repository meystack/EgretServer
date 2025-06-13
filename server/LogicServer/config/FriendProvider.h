#pragma once

#define DAILY_TRACK_FEE_NUM   5

class CFriendProvider :
	protected CCustomLogicLuaConfig 
{
public:
	typedef CObjectAllocator<char>	CDataAllocator;
	typedef CCustomLogicLuaConfig	Inherited;

	CFriendProvider();
	~CFriendProvider();
	//从文件加载配置
	bool LoadFriendConfig(LPCTSTR sFilePath);
private:
	//以下函数为覆盖父类的相关数据处理函数
	void showError(LPCTSTR sError);
	bool readConfigs();

public:
	int						m_MaxFriendNum;				//好友人数上限
	int						m_MaxStrangerNum;			//陌生人数上限
	int						m_MaxEnemyNum;				//仇人人数上限
	int						m_MaxBlacklist;				//黑名单上限

	int						m_TrackItemId;				//追踪令物品id
	int						m_TrackNeedYb;				//追踪需要的元宝
	int						m_AlarmItemId;				//报警灯的物品id
	int						m_SendNeedCoin;				//发送坐标需要金币

	int						m_MinAttendMaster;			//可以拜师的最低等级
	int						m_MaxAttendMaster;			//可以拜师的最高等级
	int						m_CanBeMaster;				//可以成为师父的等级

	int						m_MaxMaster;				//可以拜师的最大人数
	int						m_maxDisciple;				//可收徒弟的最大人数

	int						m_graduateLevel;			//出师的等级
	int						m_needIntimacy;				//出师需要的亲密度

	int						m_publishAttendMaster;		//发布出师或收徒需要的金币

	int						m_maxCanMasterNum;			//可以收徒或者拜师的最大数

	int						m_desertMasterNeedCoin;		//叛离师门 需要的金币
	int						m_driveOutMaster;			//逐出师门需要的金币

	int						m_awardExpRate;				//组队经验值奖励 倍率
	int						m_awardExpTeacherRate;		//徒弟杀怪获得的经验孝敬给师傅的比例(百分比)
	int						m_yuanbaoPupilRate;			//师傅消费元宝计算可分给徒弟的元宝比例

	int						m_changeNamCoin;			//修噶战队名称需要的金币
	int						m_CallFriendCoin;			//召唤好友扣钱
	int						m_DieListCount;				//死亡记录长度
	int						m_TrackActorYuanB;			//追踪消耗的元宝
	int						m_nMaxMoodString;			//最大心情数量
	int						m_nTrackActorFeeType;			//追踪玩家货币类型
	unsigned int			m_nTrackActorFeeNum[DAILY_TRACK_FEE_NUM];		// 追踪玩家货币数量
private:
	CDataAllocator			m_DataAllocator;			//对象申请器
};
