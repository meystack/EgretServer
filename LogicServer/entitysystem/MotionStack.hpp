#pragma once
/***************************************************************/
/*
/*  实体的运动状态的堆栈,实体的运动状态以堆栈的形式存在，
* 上一个状态结束了，前面一个状态生效生效
/*
/***************************************************************/

#pragma once

#define MAX_MOTION_TYPE_COUNT 6  //最大的堆栈的大小

struct MotionData
{
	BYTE bMotionType; //运动类型
	BYTE bAuxInfo;  //辅助信息
	WORD wAuxInfo;  //辅助信息
	WORD wAuxInfo2;
	WORD wAuxInfo3;
	WORD wAuxInfo4;
	union
	{
		struct
		{
			GAMEATTRVALUE values[2];  //值的数组
		};
		struct
		{
			Uint64 entityHandle;  //目标的handle
		};
		TICKCOUNT nExpiredTime; //过期时间
	};
};
class CMotionStack
{
public:
	CMotionStack()
	{
		memset(this,0,sizeof(CMotionStack));
	}
	inline void PushState(MotionData & data) //压入运动状态
	{
		if( m_bMotionIndex < MAX_MOTION_TYPE_COUNT -1)
		{
			m_bMotionIndex ++; 
			m_motionTypeStack[m_bMotionIndex] = data;
			
		}
		else
		{
			memmove(&m_motionTypeStack[1],&m_motionTypeStack[2],(MAX_MOTION_TYPE_COUNT -2) * sizeof(MotionData) ); //内存拷贝
		}
		m_motionTypeStack[m_bMotionIndex] = data;
	}

	inline void PopState() //堆栈顶弹出一个状态
	{
		if(m_bMotionIndex ==0)
		{
			return;
		}
		else
		{
			m_bMotionIndex --;
		}
	}

	//获得当前的运动状态 
	inline  MotionData& GetCurrentMotionState()
	{
		if(m_bMotionIndex >= 0 && m_bMotionIndex < MAX_MOTION_TYPE_COUNT)
		{
			return m_motionTypeStack[m_bMotionIndex];
		}
		else
		{
			return m_motionTypeStack[0];// 第1个总是空的
		}
	}

	void Clear()
	{
		m_bMotionIndex = 0;
	}
private:
	BYTE m_bMotionIndex; //堆栈顶的位置
	BYTE m_bReserver1;
	WORD m_wReserver2; 
	MotionData m_motionTypeStack[MAX_MOTION_TYPE_COUNT ];   //玩家的移动状态栈
};
