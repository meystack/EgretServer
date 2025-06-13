#include "StdAfx.h"
#include "ScoreRecorder.h"

CScoreRecorderManager::CScoreRecorderManager()
	:Inherited()
{
	m_dwNextCheckChangeTick = 0;
}

CScoreRecorderManager::~CScoreRecorderManager()
{
	Clear();
}

INT_PTR CScoreRecorderManager::Add(CCustomScoreRecorder *pRecorder)
{
	INT_PTR Result = Inherited::add(pRecorder);
	pRecorder->addRef();
	return Result;
}

VOID CScoreRecorderManager::Clear()
{
	INT_PTR i;
	CCustomScoreRecorder **pRecorderList = *this;
	for (i=count()-1; i>-1; --i)
	{
		if(pRecorderList[i]) {
			pRecorderList[i]->release();
		}

	}
	Inherited::clear();
}

VOID CScoreRecorderManager::Run(TICKCOUNT dwCurTick)
{	
	if (dwCurTick >= m_dwNextCheckChangeTick)
	{
		//DECLARE_FUN_TIME_PROF()
		m_dwNextCheckChangeTick = dwCurTick + 2000;

		INT_PTR i;
		CCustomScoreRecorder **pRecorderList = *this, *pRecorder;
		for (i=count()-1; i>-1; --i)
		{
			pRecorder = pRecorderList[i];
			if(!pRecorder) continue;
			if (pRecorder->isStoped())
			{
				remove(i);
				pRecorder->release();
			}
			else if (pRecorder->isChanged())
			{
				pRecorder->BoardCastScoreChanged();
			}
		}
	}
}

VOID CScoreRecorderManager::ActorLogin(CActor *pActor)
{
	INT_PTR i;
	CCustomScoreRecorder **pRecorderList = *this, *pRecorder;
	for (i=count()-1; i>-1; --i)
	{
		pRecorder = pRecorderList[i];
		if (pRecorder && !pRecorder->isStoped())
			pRecorder->ActorLogin(pActor);
	}
}

VOID CScoreRecorderManager::ActorJoinTarget(CActor *pActor, LPCVOID pTarget)
{
	INT_PTR i, nPackCount = 0;
	CCustomScoreRecorder **pRecorderList = *this, *pRecorder;

	for (i=count()-1; i>-1; --i)
	{
		pRecorder = pRecorderList[i];
		if (pRecorder && !pRecorder->isStoped())
			pRecorder->ActorJoinTarget(pActor, pTarget);
	}
}

VOID CScoreRecorderManager::ActorLeaveTarget(CActor *pActor, LPCVOID pTarget)
{
	INT_PTR i, nPackCount = 0;
	CCustomScoreRecorder **pRecorderList = *this, *pRecorder;

	for (i=count()-1; i>-1; --i)
	{
		pRecorder = pRecorderList[i];
		if (pRecorder && !pRecorder->isStoped())
			pRecorder->ActorLeaveTarget(pActor, pTarget);
	}
}


//-----------------------------------------------------------------//
LPCTSTR CCustomScoreRecorder::szClassName = _T("CCustomScoreRecorder");

CCustomScoreRecorder::CCustomScoreRecorder(LPCTSTR sName)
	:Inherited()
{
	_asncpytA(m_sName, sName);
	m_boChanged = false;
	m_boStoped = false;
	m_boIsMultiNotifyRecorder = false;
}

CCustomScoreRecorder::~CCustomScoreRecorder()
{

}

VOID CCustomScoreRecorder::destroy()
{
	stop();
	delete this;
}

bool CCustomScoreRecorder::itemExists(LPCTSTR sItemName) const
{
	return get(sItemName) != NULL;
}

INT CCustomScoreRecorder::getScore(LPCTSTR sItemName) const
{
	const SCORERECORD *pRec = get(sItemName);
	return pRec ? pRec->nScore : 0;
}

INT CCustomScoreRecorder::setScore(LPCTSTR sItemName, INT nScore, bool boCreateable)
{
	PSCORERECORD pRec = get(sItemName);
	if (!pRec)
	{
		if (boCreateable)
		{
			pRec = put(sItemName);
			_asncpyt(pRec->sName, sItemName);
		}
		else return 0;
	}
	else if (pRec->nScore == nScore)
		return nScore;

	pRec->nScore = nScore;
	pRec->boChanged = true;
	m_boChanged = true;
	return nScore;
}

INT CCustomScoreRecorder::incScore(LPCTSTR sItemName, INT nScoreToAdd, bool boCreateable)
{
	PSCORERECORD pRec = get(sItemName);
	if (!pRec)
	{
		if (boCreateable)
		{
			pRec = put(sItemName);
			_asncpyt(pRec->sName, sItemName);
		}
		else return 0;
	}

	pRec->nScore += nScoreToAdd;
	pRec->boChanged = true;
	m_boChanged = true;
	return pRec->nScore;
}

size_t CCustomScoreRecorder::MakeScoreChangedBuffer(CDataPacket &pack, bool boAll)
{
	if (Inherited2::count() <= 0)
		return 0;

	size_t nOldPos = pack.getPosition();
	size_t nChgedPos, nChanged = 0;

	pack << (BYTE)enMiscSystemID << (BYTE)sSetScoreRecorderItems << (INT64)this;
	nChgedPos = pack.getPosition();
	pack << (WORD)0;

	CHashTableIterator<SCORERECORD> it(*this);
	PSCORERECORD pRec = it.first();
	while(pRec)
	{
		if (pRec->boChanged || boAll)
		{
			if (!boAll) pRec->boChanged = false;
			nChanged++;
			pack << pRec->sName << (INT)pRec->nScore;
		}
		pRec = it.next();
	}

	if (nChanged > 0)
	{
		PWORD pChgedPtr = (PWORD)pack.getPositionPtr(nChgedPos);
		*pChgedPtr = (WORD)nChanged;
		return pack.getPosition() - nOldPos;
	}
	else
	{
		pack.setPosition(nOldPos);
		return 0;
	}
}

size_t CCustomScoreRecorder::MakeScoreCreatedBuffer(CDataPacket &pack)
{
	size_t oldSize = pack.getPosition();
	pack << (BYTE)enMiscSystemID << (BYTE)sCreateScoreRecorder << (INT64)this << m_sName;
	MakeScoreChangedBuffer(pack, true);
	return pack.getPosition() - oldSize;
}

size_t CCustomScoreRecorder::MakeScoreStopedBuffer(CDataPacket &pack)
{
	size_t oldSize = pack.getPosition();
	pack << (BYTE)enMiscSystemID << (BYTE)sDeleteScoreRecorder << (INT64)this;
	return pack.getPosition() - oldSize;
}

VOID CCustomScoreRecorder::BoardCastScoreCreated()
{
	char buf[1024];
	CDataPacket pack(buf, sizeof(buf));
	MakeScoreCreatedBuffer(pack);
	SendBuffer(pack.getMemoryPtr(), pack.getLength());
}

VOID CCustomScoreRecorder::BoardCastScoreChanged()
{
	if (!m_boStoped && m_boChanged)
	{
		char buf[1024];
		CDataPacket pack(buf, sizeof(buf));

		size_t dwSize = MakeScoreChangedBuffer(pack, false);
		m_boChanged = false;
		
		if (dwSize > 0)
			SendBuffer(pack.getMemoryPtr(), dwSize);
	}
}

VOID CCustomScoreRecorder::stop()
{
	if (!m_boStoped)
	{
		m_boStoped = true;
		//广播删除计分器的消息
		char buf[256];
		CDataPacket pack(buf, sizeof(buf));
		MakeScoreStopedBuffer(pack);
		SendBuffer(pack.getMemoryPtr(), pack.getLength());
	}
}

VOID CCustomScoreRecorder::ActorLogin(CActor *pActor)
{
	CActorPacket pack;
	pActor->AllocPacket(pack);

	MakeScoreCreatedBuffer(*pack.packet);
	pack.flush();
}

VOID CCustomScoreRecorder::ActorJoinTarget(CActor *pActor, LPCVOID pTarget)
{
	if (!m_boStoped)
	{
		ActorLogin(pActor);
	}
}

VOID CCustomScoreRecorder::ActorLeaveTarget(CActor *pActor, LPCVOID pTarget)
{
	if (!m_boStoped)
	{
		CActorPacket pack;
		pActor->AllocPacket(pack);
		MakeScoreStopedBuffer(*pack.packet);
		pack.flush();
	}
}





//-----------------------------------------------------------------//
LPCTSTR CMultiNotifyScoreRecorder::szClassName = _T("CMultiNotifyScoreRecorder");

CMultiNotifyScoreRecorder::CMultiNotifyScoreRecorder(LPCTSTR sName)
	:Inherited(sName)
{
	m_boIsMultiNotifyRecorder = true;
	GetGlobalLogicEngine()->GetScoreRecorderManager().Add(this);
}

void CMultiNotifyScoreRecorder::NotifyTarget::SendData(LPCVOID lpData, SIZE_T dwSize)
{
	switch(btType)
	{
	case srnActor: pAcotr->SendData(lpData, dwSize); break;
	case srnTeam: pTeam->SendData(lpData, dwSize); break;
	case srnGuild: pGuild->SendData(lpData, dwSize); break;
	case srnCamp: break;
	case srnFuBen: pFuBen->SendData(lpData, dwSize); break;
	}
}

bool CMultiNotifyScoreRecorder::NotifyTarget::InTarget(CActor *pActor)
{
	switch(btType)
	{
	case srnActor: return false/*没有关于多玩家通知的可继续通知的实现想法*/;
	case srnTeam: return pActor->GetTeam() == pTarget;
	case srnGuild: return pActor->GetGuildSystem()->GetGuildPtr() == pTarget;
	case srnCamp: return false;
	case srnFuBen: return pActor->GetFuBen() == pTarget;
	}
	return false;
}


VOID CMultiNotifyScoreRecorder::ActorLogin(CActor *pActor)
{
	INT_PTR i;
	NotifyTarget *pTargetList = m_TargetList, *pNotifyTarget;

	for (i=m_TargetList.count()-1; i>-1; --i)
	{
		pNotifyTarget = &pTargetList[i];
		if (pNotifyTarget->InTarget(pActor))
		{
			//向玩家发送创建计分器数据
			Inherited::ActorLogin(pActor);
		}
	}
}

VOID CMultiNotifyScoreRecorder::ActorJoinTarget(CActor *pActor, LPCVOID pTarget)
{
	INT_PTR i;
	NotifyTarget *pTargetList = m_TargetList, *pNotifyTarget;

	for (i=m_TargetList.count()-1; i>-1; --i)
	{
		pNotifyTarget = &pTargetList[i];
		if (pNotifyTarget->pTarget != pTarget)
			continue;
		if (pNotifyTarget->InTarget(pActor))
		{
			//向玩家发送创建计分器数据
			Inherited::ActorLogin(pActor);
		}
	}
}

VOID CMultiNotifyScoreRecorder::ActorLeaveTarget(CActor *pActor, LPCVOID pTarget)
{
	INT_PTR i;
	NotifyTarget *pTargetList = m_TargetList, *pNotifyTarget;

	for (i=m_TargetList.count()-1; i>-1; --i)
	{
		pNotifyTarget = &pTargetList[i];
		if (pNotifyTarget->pTarget != pTarget)
			continue;
		if (pNotifyTarget->InTarget(pActor))
		{
			//向玩家发送创建计分器数据
			Inherited::ActorLeaveTarget(pActor, pTarget);
		}
	}
}

bool CMultiNotifyScoreRecorder::RemoveNotifyTarget(void *pTarget)
{
	INT_PTR i;
	NotifyTarget *pTargetList = m_TargetList;

	//已经存在则不重复添加
	for (i=m_TargetList.count()-1; i>-1; --i)
	{
		if (pTargetList[i].pTarget == pTarget)
		{
			NotifyTarget target = m_TargetList[i];
			//从列表中移除
			m_TargetList.remove(i);
			//向通知目标发送删除计分器的数据包
			char buf[256];
			CDataPacket pack(buf, sizeof(buf));
			MakeScoreStopedBuffer(pack);
			target.SendData(pack.getMemoryPtr(), pack.getLength());
			return true;
		}
	}
	return false;
}

//添加一个通知对象，如果对象存在则不会重复添加
INT_PTR CMultiNotifyScoreRecorder::AddNotifyTarget(eScoreRecorderNotifyType eType, void *pTarget)
{
	INT_PTR i = GetNotifyTargetIndex(eType, pTarget);

	if (i > -1) return i;

	NotifyTarget target;
	target.btType = eType;
	target.pTarget = pTarget;
	i = m_TargetList.add(target);

	//向通知目标发送创建计分器的数据包
	char buf[1024];
	CDataPacket pack(buf, sizeof(buf));
	MakeScoreCreatedBuffer(pack);
	target.SendData(pack.getMemoryPtr(), pack.getLength());

	return i;
}

inline INT_PTR CMultiNotifyScoreRecorder::GetNotifyTargetIndex(eScoreRecorderNotifyType eType, void *pTarget)
{
	INT_PTR i;
	NotifyTarget *pTargetList = m_TargetList;

	//已经存在则不重复添加
	for (i=m_TargetList.count()-1; i>-1; --i)
	{
		if (pTargetList[i].pTarget == pTarget && pTargetList[i].btType == eType)
		{
			return i;
		}
	}
	return -1;
}
