#pragma once

/************************************************************************/
/*
/*      属性的评分器，用于评价一个属性集的分数，保留装备等
/*     比如内功攻击 1点 评分为2分，外功攻击一点评分为2.5分，根据这个算出一个属性集或者属性的分数
/************************************************************************/

class CAttrEval
{
public:
	CAttrEval()
	{
		reset();
	}

	//辅助的计算器，用于一些辅助的评估，比如宠物的技能，id不是bufftype
	typedef struct AuxEvalValue
	{
		int nId;		//ID
		float fScore;	//该ID的分数
	} AUXEVALVALUE,*PAUXEVALVALUE;
	
	/* 重置属性计算器 */
	inline void reset(){ memset(m_AttrValues, 0, sizeof(m_AttrValues)); m_auxValue.clear(); }
	
	
	//将一个属性初始化到评分器里
	CAttrEval& operator << (const tagGameAttribute &attr)
	{
		if(attr.type <0 || attr.type >= GameAttributeCount)
		{
			return *this;
		}
		if(AttrNeedCalc[attr.type] == false) 
		{
			return *this;
		}
		m_AttrValues[attr.type].fValue = attr.value.fValue;
		return *this;
	}

	//初始化评分器,每个属性一点占多少分,比如一点外功攻击2分
	CAttrEval& operator <<(const CAttrCalc &attrCalc)
	{
		INT_PTR i;
		GAMEATTR attr;
		for ( i=1; i<GameAttributeCount; ++i )
		{
			attr.type = (BYTE)i;
			attr.value = attrCalc.m_AttrValues[i];
			if(attr.value.nValue !=0)
			{
				this->operator << (attr);
			}
		}
		return *this;
	}

	//将另外一个评分器的数据放到本评分器里
	CAttrEval& operator <<(const CAttrEval &eval)
	{
		INT_PTR i;
	
		for ( i=1; i<GameAttributeCount; ++i )
		{
			if(eval.m_AttrValues[i].nValue !=0)
			{
				m_AttrValues[i].fValue = eval.m_AttrValues[i].fValue;
			}

		}
		return *this;
	}
	
	/*
	* Comments: 添加辅助的参数，添加这个主要用于一些技能ID等产生的评价的数值
	* Param int nId:id，比如技能的id
	* Param float score:该参数的分数
	* @Return void:
	*/
	 void AddAuxEvalParam(int nId, float score)
	{
		//如果已经存在的话
		if( score ==0.0) return ;
		for(INT_PTR i=0; i< m_auxValue.count(); i++ )
		{
			if(m_auxValue[i].nId == nId)
			{
				m_auxValue[i].fScore = score;
				return ;
			}
		}
		AUXEVALVALUE data;
		data.nId = nId;
		data.fScore = score;
		m_auxValue.add(data);
	}
	 
	/*
	* Comments:获取辅助参数上的得分
	* Param int nId:辅助参数的id，比如技能的ID
	* Param float fValue:辅助参数上的得分
	* @Return float:返回该项得分
	*/
	float GetAuxScore(int nId, float fValue=1.0) 
	{
		for(INT_PTR i=0; i< m_auxValue.count(); i++ )
		{
			if(m_auxValue[i].nId == nId)
			{
				return m_auxValue[i].fScore * fValue ;
			}
		}
		return 0.0;
	}

	//获取一个属性的评分分数
	float GetOneAttrScore(GAMEATTR attr)
	{
		INT_PTR nType= attr.type;
		if(nType <1 || nType >=GameAttributeCount) return 0.0; //溢出
		
		switch(AttrDataTypes[nType])
		{
		case adSmall://有符号1字节类型
		case adShort:
		case adInt:  //有符号4字节类型
			return  (float)(attr.value.nValue * m_AttrValues[nType].fValue);
			break;

		case adUSmall://无符号1字节类型,无符号2字节类型
		case  adUShort:
		case adUInt://无符号4字节类型
			return (float)(attr.value.uValue * m_AttrValues[nType].fValue);
			break;

		case adFloat://单精度浮点类型值
			return (float)(attr.value.fValue * m_AttrValues[nType].fValue);
			break;
		}
		return 0.0;

	}

	//获取一个属性集的评分分数
	float GetAttrSetScore( CAttrCalc &attrCalc )
	{
		INT_PTR i;
		//GAMEATTR attr;
		float fScore =0.0; //分数
		for ( i=1; i<GameAttributeCount; ++i )
		{

			if( attrCalc.m_AttrValues[i].nValue !=0 )
			{
				switch(AttrDataTypes[i])
				{
				case adSmall://有符号1字节类型
				case adShort:
				case adInt:  //有符号4字节类型
					fScore += fabs((float)(attrCalc.m_AttrValues[i].nValue * m_AttrValues[i].fValue));
					break;

				case adUSmall://无符号1字节类型,无符号2字节类型
				case  adUShort:
				case adUInt://无符号4字节类型
					fScore += fabs((float)(attrCalc.m_AttrValues[i].uValue * m_AttrValues[i].fValue));
					break;
					
				case adFloat://单精度浮点类型值
					fScore += fabs((float)(attrCalc.m_AttrValues[i].fValue * m_AttrValues[i].fValue));
					break;
				}

			}
		}
		return fScore;
	}

	

public:
	GAMEATTRVALUE	m_AttrValues[GameAttributeCount];	//属性集定义
	CVector<AUXEVALVALUE> m_auxValue; //辅助的属性
};
