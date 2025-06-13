#ifndef ACTOR_DATA_PACKET_STAT_ALLOC_H_
#define ACTOR_DATA_PACKET_STAT_ALLOC_H_
#include "ActorDataPacketStat.h"

 
// 负责CDataPacketRecord的分配
class CActorDataPacketStatAlloc
{
public:	
	typedef Handle<UINT32>								ADPSHandle;			// 角色数据包统计对象句柄
	typedef HandleMgr<CDataPacketRecord, ADPSHandle>	ADPSHdlMgr;			// 角色数据包统计对象句柄管理器

	CActorDataPacketStatAlloc();
	CDataPacketRecord* AllocRecord()
	{
		ADPSHandle handle;
		CDataPacketRecord *pRecord = m_hdlMgr.Acquire(handle);
		if (pRecord)
		{
			pRecord->m_nHandle = handle;
		}

		return pRecord;
	}

	void FreeRecord(const CDataPacketRecord *pRecord)
	{		
		m_hdlMgr.Release(pRecord->m_nHandle);
	}

	static CActorDataPacketStatAlloc* m_spADPSAllocator;

private:
	ADPSHdlMgr		m_hdlMgr;
};

#endif