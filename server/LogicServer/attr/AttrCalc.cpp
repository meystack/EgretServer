#include "../StdAfx.h"
#include <string.h>
#ifdef WIN32
#include <Windows.h>
#include <AtomOPDef.h>
#endif
#include "AttrDef.h"
#include "AttrCalc.h"

CAttrCalc::CAttrCalc()
{
	reset();
}

CAttrCalc& CAttrCalc::operator *= (float rate)
{
	for (INT_PTR i=1; i<GameAttributeCount; ++i )
	{
		if(AttrNeedCalc[i] == false) 
		{
			continue;
		}
		switch(AttrDataTypes[i])
		{
		case adSmall://有符号1字节类型
		case adShort://有符号2字节类型
		case adInt://有符号4字节类型
			m_AttrValues[i].nValue =(int)( m_AttrValues[i].nValue * rate);
			break;

		case adUShort://无符号2字节类型
		case adUInt://无符号4字节类型
		case adUSmall://无符号1字节类型
			m_AttrValues[i].uValue =(unsigned int)( m_AttrValues[i].uValue * rate);
			break;
		
		case adFloat://单精度浮点类型值
			m_AttrValues[i].fValue *= rate;
			break;
		}
	}

	return *this; 
}

CAttrCalc& CAttrCalc::operator << (const tagGameAttribute &attr)
{	
	if(attr.type <0 || attr.type >= GameAttributeCount)
	{
		//Trace(rmError,_T("属性计算器的ID=%d过大"),attr.type);
		return *this;
	}
	if(AttrNeedCalc[attr.type] == false) 
	{
		return *this;
	}
	switch(AttrDataTypes[attr.type])
	{
	case adSmall://有符号1字节类型
		m_AttrValues[attr.type].nValue += attr.value.nValue;
		break;
	case adUSmall://无符号1字节类型
		m_AttrValues[attr.type].uValue += attr.value.uValue;
		break;
	case adShort://有符号2字节类型
		m_AttrValues[attr.type].nValue += attr.value.nValue;
		break;
	case adUShort://无符号2字节类型
		m_AttrValues[attr.type].uValue += attr.value.uValue;
		break;
	case adInt://有符号4字节类型
		m_AttrValues[attr.type].nValue += attr.value.nValue;
		break;
	case adUInt://无符号4字节类型
		m_AttrValues[attr.type].uValue += attr.value.uValue;
		break;
	case adFloat://单精度浮点类型值
		m_AttrValues[attr.type].fValue += attr.value.fValue;
		break;
	}
	return *this;
}

CAttrCalc& CAttrCalc::operator << (const tagPackedGameAttribute &attr)
{
	if(attr.type <0 || attr.type >= GameAttributeCount)
	{
		//Trace(rmError,_T("属性计算器的ID=%d过大"),attr.type);
		return  *this ;
	}
	if(AttrNeedCalc[attr.type] == false ) 
	{
		return *this;
	}

	tagGameAttribute gameAttr;
	gameAttr.type = attr.type;
	
	switch(AttrDataTypes[attr.type])
	{
	case adSmall://有符号1字节类型
	case adShort://有符号2字节类型
	case adInt://有符号4字节类型
		if(attr.sign) //负数
		{
			m_AttrValues[attr.type].nValue += -1* (int)attr.value;
		}
		else //正数
		{
			m_AttrValues[attr.type].nValue +=   (int)attr.value;
		}
		break;
	case adUSmall://无符号1字节类型
	case adUShort://无符号2字节类型
	case adUInt://无符号4字节类型
		m_AttrValues[attr.type].uValue += attr.value; //无符号的不会出现负数的
		break;
	case adFloat://单精度浮点类型值
		if(attr.sign) //负数
		{
			m_AttrValues[attr.type].fValue += (float)(-1 *tagPackedGameAttribute::FloatValueBase * attr.value);
		}
		else //正数
		{
			m_AttrValues[attr.type].fValue += (float)(tagPackedGameAttribute::FloatValueBase * attr.value);
		}
		break;
	}

	return *this;
}

CAttrCalc& CAttrCalc::operator << (const CAttrCalc &attrCalc)
{
	INT_PTR i;
	GAMEATTR attr;
	for ( i=0; i<GameAttributeCount; ++i )
	{
		attr.type = (BYTE)i;
		attr.value = attrCalc.m_AttrValues[i];
		this->operator << (attr);
	}
	return *this;
}

CAttrCalc& CAttrCalc::applyPowerAttributes(const CAttrCalc& baseAttrs)
{
	//TODO：clear power attr values
	for(INT_PTR i= Level2PropertyStart; i< Level2PropertyEnd ; i++)
	{
		if( i%2 ==1)
		{
			continue;
		}
		if(m_AttrValues[i].nValue) //有beilv
		{
			
			switch(AttrDataTypes[i-1])
			{
			case adSmall://有符号1字节类型

				m_AttrValues[i-1].nValue += (int)( baseAttrs.m_AttrValues[i-1].nValue * ((m_AttrValues[i].nValue)/10000.0) ); 
				break;
			case adUSmall://无符号1字节类型
				m_AttrValues[i-1].nValue += (int)( baseAttrs.m_AttrValues[i-1].nValue * ((m_AttrValues[i].nValue)/10000.0)); 
				break;
			case adShort://有符号2字节类型
				m_AttrValues[i-1].nValue += (int)( baseAttrs.m_AttrValues[i-1].nValue * ((m_AttrValues[i].nValue)/10000.0)); 
				break;
			case adUShort://无符号2字节类型
				m_AttrValues[i-1].uValue += (unsigned int)( baseAttrs.m_AttrValues[i-1].nValue *((m_AttrValues[i].nValue)/10000.0) ); 
				break;
			case adInt://有符号4字节类型
				m_AttrValues[i-1].nValue += (int)( baseAttrs.m_AttrValues[i-1].nValue * ((m_AttrValues[i].nValue)/10000.0) );
				break;
			case adUInt://无符号4字节类型
				m_AttrValues[i-1].uValue += (unsigned int)( baseAttrs.m_AttrValues[i-1].nValue * ((m_AttrValues[i].nValue)/10000.0) );
				break;
			case adFloat://单精度浮点类型值
				//baseAttrs.m_AttrValues[i-1].fValue  *= (1+ m_AttrValues[i].fValue); 
				m_AttrValues[i-1].fValue += (float)( baseAttrs.m_AttrValues[i-1].nValue *((m_AttrValues[i].nValue)/10000.0) );
				break; 
			}
			m_AttrValues[i].fValue =0;
		}
		
	}
	return *this;
}

