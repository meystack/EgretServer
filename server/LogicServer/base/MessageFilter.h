#ifndef MESSAGE_FILTER_H_
#define MESSAGE_FILTER_H_

class CMessageFilter
{
public:
	// 最大消息号
	static const int MAX_MESSAGE_ID			= 0xff;
	// 最大消息数量
	static const int MAX_MESSAGE_COUNT		= MAX_MESSAGE_ID + 1;
	// 统配消息数量
	static const int WILDCARD_MESSAGE_ID	= -1;
public:
	CMessageFilter()
	{
		ZeroMemory(this, sizeof(*this));
	}

	// 添加消息过滤
	inline void SetFilter(const int nMsgId, bool bFilter)
	{
		if (nMsgId > MAX_MESSAGE_ID)
			return;

		if (WILDCARD_MESSAGE_ID == nMsgId)
		{
			for (int i = 0; i <= MAX_MESSAGE_ID; i++)
			{
				m_MsgFilterFlag[i] = bFilter;
			}
			m_nFilterMsgCount = MAX_MESSAGE_COUNT;
		}
		else
		{
			if (m_MsgFilterFlag[nMsgId] != bFilter)
			{
				m_MsgFilterFlag[nMsgId] = bFilter;
				m_nFilterMsgCount += bFilter ? 1 : -1;
			}
		}
	}

	// 对具体的消息应用过滤器，如果过滤，返回true；否则返回false
	inline bool Apply(const int nMsgId)
	{
		if (nMsgId < 0 || nMsgId > MAX_MESSAGE_ID)
			return false;

		return m_MsgFilterFlag[nMsgId];
	}

	// 返回消息过滤器中是否全部消息都被过滤。
	inline bool Apply()
	{
		return MAX_MESSAGE_COUNT == m_nFilterMsgCount ? true : false;
	}

	// 过滤消息总数量
	int		m_nFilterMsgCount;
	// 消息过滤标记，如果对应消息标记位为true，表明此消息需要过滤
	bool		m_MsgFilterFlag[MAX_MESSAGE_ID+1];
};

// 角色消息过滤器
class CActorMsgFilter
{
public:
	// 最大子系统ID
	static const int MAX_SUBSYSTEM_ID = 0xff;

	// 设置过滤器
	void SetFilter(const int nSystemId, const int nMsgId, bool bFilter)
	{		
		if (nSystemId < 0 || nSystemId > MAX_SUBSYSTEM_ID)
			return;

		m_SubSytemMsgFilters[nSystemId].SetFilter(nMsgId, bFilter);
	}

	// 检测子系统是否被屏蔽
	bool Apply(const int nSystemId)
	{
		if (nSystemId < 0 || nSystemId > MAX_SUBSYSTEM_ID)
			return true;

		return m_SubSytemMsgFilters[nSystemId].Apply();
	}

	bool Apply(const int nSystemId, const int nMsgId)
	{
		if (nSystemId < 0 || nSystemId > MAX_SUBSYSTEM_ID)
			return true;

		return m_SubSytemMsgFilters[nSystemId].Apply(nMsgId);
	}


	CMessageFilter	m_SubSytemMsgFilters[MAX_SUBSYSTEM_ID+1];
};


#endif
