#include "StdAfx.h"

using namespace jxInterSrvComm::DbServerProto;

const char sGlobalVarFile[] = "./data/runtime/globalVar_";

CGlobalVarMgr::CGlobalVarMgr()
{
	m_boInited = false;
	m_hasModify =false;
	m_saveDataPack.setAllocator(GetLogicServer()->GetBuffAllocator());
	//Load();
}

CGlobalVarMgr::~CGlobalVarMgr()
{
	Destroy();
}

VOID CGlobalVarMgr::Load()
{
	//CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	//if (!m_boInited && pDbClient->connected())
	//{
	//	CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcLoadVar);
	//	DataPacket << (int)0;
	//	GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	//}

	///改成读本地文件
	char sFile[256];
 	sprintf_s(sFile,sizeof(sFile),"%s%d.bin",sGlobalVarFile,GetLogicServer()->GetServerIndex());
	wylib::stream::CMemoryStream ms;
	if ( ms.loadFromFile(sFile) > 0 )
	{
		m_Var.loadFromMemory((char*)ms.getMemory(),(size_t)ms.getSize());
	}
	m_boInited = true;

	CDataClient* pDbClient = GetLogicServer()->GetDbClient();
	if (pDbClient->connected())
	{
		CDataPacket& DataPacket =pDbClient->allocProtoPacket(dcLoadCircleCount);
		GetLogicServer()->GetDbClient()->flushProtoPacket(DataPacket);
	}
}

void CGlobalVarMgr::OnDbRetData( INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader )
{
	if(nCmd == dcLoadVar )
	{
		if (nErrorCode == reSucc)
		{
			m_Var.loadFromMemory(reader.getOffsetPtr(), reader.getAvaliableLength());
			m_boInited = true;
		}
		else
		{
			OutputMsg(rmError,_T("load globalVar error!errcode=%d"),nErrorCode);
			m_boInited = false;
		}
	}
}

void CGlobalVarMgr::DelDbRetData(INT_PTR nCmd, void* data, size_t len)
{
	CDataPacketReader pack(data, len);
	if(nCmd == dcLoadCircleCount)
	{
		BYTE count = 0;
		BYTE errorcode = 0;
		pack >> errorcode;
		if(errorcode == reSucc)
		{
			m_CircleMap.clear();
			pack >> count;
			for(int i = 0; i < count; i++)
			{
				int nCircle = 0;
				int nCount;
				pack >> nCount;
				pack >> nCircle;
				m_CircleMap[nCircle] = nCount;
			}


		}
	}
}
int CGlobalVarMgr::GetCountByCircleLevel(int nCircleLevel )
{
	// std::map<int, int>::iterator it = m_CircleMap.find(nCircleLevel);
	// if(it != m_CircleMap.end())
	// {
	// 	return it->second;
	// }
	int nCount = 0;
	std::map<int, int>::iterator it = m_CircleMap.begin();
	for(; it != m_CircleMap.end(); it++)
	{
		if(it->first >= nCircleLevel)
			nCount += it->second;
	}
	return nCount;
}

void CGlobalVarMgr::AddCountByCircleLevel(int nCircleLevel, int nCount)
{
	std::map<int, int>::iterator it = m_CircleMap.find(nCircleLevel);
	if(it != m_CircleMap.end())
	{
		m_CircleMap[nCircleLevel] += nCount;
		SubCountByCircleLevel(nCircleLevel-1);
	}
	else
	{
		m_CircleMap.insert(std::make_pair(nCircleLevel, nCount));
	}
	
}

void CGlobalVarMgr::SubCountByCircleLevel(int nCircleLevel)
{
	std::map<int, int>::iterator it = m_CircleMap.find(nCircleLevel);
	if(it != m_CircleMap.end())
	{
		if(it->second > 0)
			m_CircleMap[nCircleLevel]--;
	}	
}

VOID CGlobalVarMgr::RunOne()
{	
	TICKCOUNT nCurr = _getTickCount();
	if (m_timer.CheckAndSet(nCurr))
	{
		Save();
	}
}

void CGlobalVarMgr::Save()
{
	if(!m_hasModify ) return;
	DECLARE_TIME_PROF("CGlobalVarMgr::Save");
	
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif 
{	

	///改成保存到文件
	char sFile[256];
	sprintf_s(sFile,sizeof(sFile),"%s%d.bin",sGlobalVarFile,GetLogicServer()->GetServerIndex());
	CCLVariant *pContinueVar = &m_Var;
	wylib::stream::CMemoryStream ms;
	
	

	m_saveDataPack.setLength(4096);//预留4K空间
	//m_saveDataPack.adjustOffset(0);
	m_saveDataPack.setPosition(0);	//每次存盘要重置偏移
	
	size_t savesize = 0;
	while (true)
	{
		savesize = pContinueVar->saveToMemory(m_saveDataPack.getOffsetPtr(), m_saveDataPack.getAvaliableCapacity(), &pContinueVar);
		m_saveDataPack.adjustOffset(savesize);
		if (!pContinueVar)
			break;
		//这里修改了一处死循环，问题是大小大于4096将无法申请空间
		m_saveDataPack.setLength(m_saveDataPack.getLength() + 4096);//每次增长4K
		OutputMsg(rmWaning,_T("GLOBAL VAL  the var data too large"));

	}
	ms.write(m_saveDataPack.getMemoryPtr(),m_saveDataPack.getPosition());

	ms.saveToFile(sFile);
}

#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
	}	
#endif 
#endif 
	m_hasModify =false;
}
