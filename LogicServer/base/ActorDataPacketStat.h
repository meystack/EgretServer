#ifndef ACTOR_DATA_PACKET_STAT_H_
#define ACTOR_DATA_PACKET_STAT_H_

// 数据包记录信息
class CDataPacketRecord
{
public:	
	UINT32				m_nHandle;			// 句柄
	BYTE				m_nSystemId;		// 系统号
	BYTE				m_nMsgId;			// 消息号
	WORD				m_nLenth;			// 消息长度（不算消息头，从系统号和消息号开始算起）	
	TICKCOUNT			m_nRecordTime;		// 数据包记录时间

	CDataPacketRecord()
	{
	}

	inline CDataPacketRecord(BYTE ucSysId, BYTE ucMsgId, WORD wLength, TICKCOUNT recordTime)
	{
		m_nHandle		= 0;
		m_nSystemId		= ucSysId;
		m_nMsgId		= ucMsgId;
		m_nLenth		= wLength;
		m_nRecordTime	= recordTime;
	}

	inline void SetData(const CDataPacketRecord &record)
	{
		CopyMemory(&this->m_nSystemId, &(record.m_nSystemId), sizeof(CDataPacketRecord) - sizeof(m_nHandle));
	}
};

//统计数据包
typedef struct tagDataPacketRecord
{
	BYTE				m_nSystemId;		// 系统号
	BYTE				m_nMsgId;			// 消息号
	WORD				m_nLenth;			// 消息长度（不算消息头，从系统号和消息号开始算起）	
	TICKCOUNT			m_nRecordTime;		// 数据包记录时间
	INT64				m_Count;			// 数据包数量
	char				m_MapName[32];		// 场景名称
	int					m_PosX;				// 场景坐标x
	int					m_PosY;				// 场景坐标Y
	INT64				m_TotalSzie;		//

	tagDataPacketRecord()
	{
		memset(this,0,sizeof(*this));
	}
}TDATAPACKETRECORD;

class CActorDataPacketStat
{
public:	
	/*
	* Comments: 析构函数
	* @Return :
	* @Remark: 主要是回收m_Records里头的对象
	*/
	~CActorDataPacketStat();

	/*
	* Comments: 添加一个数据包记录
	* Param BYTE ucSysId: 逻辑系统Id
	* Param BYTE ucMsgId: 逻辑消息Id
	* Param WORD wLength: 数据包长度
	* Param TICKCOUNT tickCount: 数据包申请时间
	* @Return void:
	* @Remark:
	*/
	void AddRecord(BYTE ucSysId, BYTE ucMsgId, WORD wLength, TICKCOUNT tickCount);

	/*
	* Comments: 添加一个数据包记录
	* Param const CDataPacketRecord & record:
	* @Return void:
	* @Remark:
	*/
	void AddRecord(const CDataPacketRecord &record);

	/*
	* Comments: 清除所有的数据包记录
	* @Return void:
	* @Remark:
	*/
	void Clear();

	/*
	* Comments: 将所有的数据包记录输出到文件中
	* Param CActor * pActor: 角色对象指针
	* @Return void:
	* @Remark:
	*/
	void WriteToFile(CActor* pActor);

	/*
	* Comments: 清除全服的所有数据包的统计
	* @Return void:
	* @Remark:
	*/
	void ClearRecords();

	/*
	* Comments: 添加一个数据包记录到全服数据
	* Param BYTE ucSysId: 逻辑系统Id
	* Param BYTE ucMsgId: 逻辑消息Id
	* Param WORD wLength: 数据包长度
	* Param TICKCOUNT tickCount: 数据包申请时间
	* @Return void:
	* @Remark:
	*/
	void AddAllRecord(BYTE ucSysId, BYTE ucMsgId, WORD wLength, TICKCOUNT tickCount,char* sMapName,int nPosX,int nPosY);

	/*
	* Comments: 将所有的数据包记录输出到文件中
	* Param CActor * pActor: 角色对象指针
	* @Return void:
	* @Remark:
	*/
	void WriteRecordToFile();

protected:
	/*
	* Comments: 将所有的数据包记录输出到流中
	* Param wylib::stream::CBaseStream & stream: 输出流
	* Param CActor* pActor: 角色对象指针
	* @Return void:
	* @Remark:
	*/
	void WriteToStream(wylib::stream::CBaseStream &stream, CActor* pActor);
	
	/*
	* Comments: 将所有的数据包记录输出到流中
	* Param wylib::stream::CBaseStream & stream: 输出流
	* Param CActor* pActor: 角色对象指针
	* @Return void:
	* @Remark:
	*/
	void WriteRecordToStream(wylib::stream::CBaseStream &stream);

private:
	CBaseList<CDataPacketRecord *>			m_Records;
	CVector<TDATAPACKETRECORD>				m_AllRecords;					//全服的所有数据包的统计
	int										m_Count;						//统计的次数
};

#endif