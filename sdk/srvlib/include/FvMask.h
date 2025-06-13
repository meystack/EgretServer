#pragma once

#ifndef _MASK_
#define _MASK_(flag) ((LONGLONG)0X01 << (flag))
#endif
namespace FvMask
{
	template<typename TFlag, typename TMask>
	bool HasAny(const TFlag flag, const TMask mask)
	{
		TFlag temp = (TFlag)mask;
		return ((flag&temp) != 0);
	}
	template<typename TFlag, typename TMask>
	bool HasAll(const TFlag flag, const TMask mask)
	{
		return ((flag&mask) == mask);
	}

	template<typename TFlag, typename TMask>
	void Add(TFlag& flag, const TMask mask)
	{
		flag |= mask;
	}

	template<typename TFlag, typename TMask>
	void Del(TFlag& flag, const TMask mask)
	{
		flag &= ~ mask;
	}

	template<typename TFlag, typename TMask>
	TFlag Remove(const TFlag flag, const TMask mask)
	{
		return (flag & (~ mask));
	}

	template<typename TFlag, typename TMask>
	TFlag IsAdd(const TFlag oldFlag, const TFlag newFlag, const TMask mask)
	{
		return (((oldFlag&mask) == 0) && ((newFlag&mask) != 0));
	}
	template<typename TFlag, typename TMask>
	TFlag IsDel(const TFlag oldFlag, const TFlag newFlag, const TMask mask)
	{
		return (((oldFlag&mask) != 0) && ((newFlag&mask) == 0));
	}

	template<typename TFlag>
	int ReturnMax(const TFlag flag)
	{
		int count = 8*sizeof(TFlag);
		for (int i = count - 1; i >= 0; --i)
		{
// 			if (((LONGLONG)flag << (i)) > 0)
// 			{
// 				return count - i - 1;
// 			}
			if (((flag&_MASK_(i)) != 0))
			{
				return i;
			}
		}
		return -1;
	}
	inline bool haveRule(LONGLONG gameRule, int oneRule)
	{
		if (oneRule<0||oneRule>=64)
		{
			return false;
		}
		return (gameRule&((LONGLONG)0X01 << (oneRule)))!=0;
	}
}