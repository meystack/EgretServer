#pragma once

template<typename T>
class CSwapQueue : public CQueueList<T>
{
public:
	CSwapQueue()
	{
		setLock(&m_Lock);
	}

private:
	CCSLock					m_Lock;
};