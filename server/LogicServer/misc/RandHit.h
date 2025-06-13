#pragma once

// 概率命中, 针对游戏中一些概率发生事件的处理
// 例如：概率20%，意味着5次一定会命中一次。具体在哪次命中，这个具有一定的随机性
// 支持两种类型 1：百分比   2：指定n/m

#define CIRCLE_HIT_COUNT	100

class CRandHit : public Counter<CRandHit>
{
public:
	CRandHit()
	{
		m_nCurrHitCount		= 0;
		m_nRandCount 		= 0;
		m_nNeedRandCount	= 0;
		m_nHitCountInCircle = CIRCLE_HIT_COUNT;
	}

	// 设置命中概率（百分比，0-100）
	/*
	* Comments:设置随机概率
	* Param unsigned int nRate: 100次命中操作中随机的次数 【0，100】。0不会随机，100是百分百随机
	* @Return void: 100次为周期，随机nRate次
	*/
	void SetHitRate(unsigned int nRate)
	{	
		nRate = __min(nRate, CIRCLE_HIT_COUNT);
		m_nNeedRandCount	= nRate;
		m_nHitCountInCircle = CIRCLE_HIT_COUNT;		
		NextCircle();
	}

	/*
	* Comments: 设置随机参数。
	* Param unsigned int nHitCount:	命中次数m
	* Param unsigned int nRandCount:随机次数n
	* @Return void:  m次中随机n次
	*/
	void SetHitInfo(unsigned int nHitCount, unsigned int nRandCount)
	{
		nRandCount			= __min(nHitCount, nRandCount);
		m_nHitCountInCircle = nHitCount;
		m_nNeedRandCount	= nRandCount;
		NextCircle();
	}

	
	/*
	* Comments: 命中测试
	* @Return bool: 如果命中成功，返回true；否则返回false
	*/
	bool Hit()
	{
		bool bRet = false;
		if (m_nNeedRandCount == 0)						// 绝对不命中
			return false;

		m_nCurrHitCount++;
		if (!RandFinishedInCurrCircle())		
		{				
			if (GetLeftHitCount() < GetLeftRandCount())
				bRet = true; 								// 绝对命中
			else
				bRet = wrand(2) % 2 == 1 ? true : false;

			if (bRet)
				m_nRandCount++;
		}

		if (GetLeftHitCount() == 0)							// 开启新的周期		
			NextCircle();

		return bRet;
	}

private:
	void NextCircle()
	{
		m_nCurrHitCount		= 0;
		m_nRandCount 		= 0;
	}

	inline bool RandFinishedInCurrCircle()
	{
		return m_nRandCount == m_nNeedRandCount ? true : false;
	}

	inline unsigned int GetLeftHitCount()
	{
		return m_nHitCountInCircle - m_nCurrHitCount;
	}

	inline unsigned int GetLeftRandCount()
	{
		return m_nNeedRandCount - m_nRandCount;
	}

private:
	unsigned int	m_nCurrHitCount;			// 当前周期触发的次数	
	unsigned int	m_nNeedRandCount;			// 一个周期需要随机的次数 n
	unsigned int	m_nRandCount;				// 成功随机的次数
	unsigned int	m_nHitCountInCircle;		// 一个周期触发的总次数  m
};


// 组随机
// 功能：将多个概率随机归结到一个组。在组的每次随机中，一个组的值最多只随机出一个。
//class CRandHitGroup
//{
//public:
//	CVector<CRandHit> m_RandHitList;
//};
