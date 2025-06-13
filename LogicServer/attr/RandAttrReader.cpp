#include "StdAfx.h"
#include "RandAttrReader.h"

CRandAttrReader::~CRandAttrReader()
{
	m_pLua = NULL;
}

void CRandAttrReader::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[RandAttrReader]");
	s += sError;
	throw s;
}

__forceinline GAMEATTRVALUE CRandAttrReader::readAttrValue(const tagGameAttributeType attrType, LPCSTR sName)
{
	GAMEATTRVALUE result;
	result.nValue =0;
	if (attrType < 1 || attrType >= GameAttributeCount) return result;
	

	switch(AttrDataTypes[attrType])
	{
	case adSmall:
	case adShort:
	case adInt:
		result.nValue = getFieldInt(sName);
		break;
	case adUSmall:
	case adUShort:
	case adUInt:
		result.uValue = (UINT)getFieldInt64(sName);
		break;
	case adFloat:
		result.fValue = (float)getFieldNumber(sName);
		break;
	}
	return result;
}

INT_PTR CRandAttrReader::readConfig(lua_State* L, PRANDOMATTRTERM pRandAttrTerms, CDataAllocator &dataAllocator, INT_PTR numTermToRead)
{
	static INT DefRequestValue = 0;
	INT_PTR numReaded = 0;

	m_pLua = L;
	if (enumTableFirst())
	{
		do 
		{
			if ( numReaded >= numTermToRead )
			{
				endTableEnum();
				break;
			}

			int nDefaultJob = 0;	
			int nDefaultMaxNum = 0;
			int nDefaultLockMaxValue = 0;
			pRandAttrTerms->type = (BYTE)getFieldInt("type");
			pRandAttrTerms->job  = (BYTE)getFieldInt("job", &nDefaultJob);
			pRandAttrTerms->prob =(WORD) getFieldInt("prop");
			pRandAttrTerms->maxNum  = (BYTE)getFieldInt("maxNum", &nDefaultMaxNum);
			pRandAttrTerms->lockMaxValue  = getFieldInt("lockMaxValue", &nDefaultLockMaxValue);

			if (feildTableExists("typeValue") && openFieldTable("typeValue"))
			{
				pRandAttrTerms->valueList.count = lua_objlen(m_pLua, -1); 
				pRandAttrTerms->valueList.pData = (RANDOMATTVALUE*)dataAllocator.allocObjects( pRandAttrTerms->valueList.count * sizeof(RANDOMATTVALUE)) ;
				
				if (enumTableFirst())
				{
					int valueIndex = 0;
					int def = 0;
					do 
					{
						RANDOMATTVALUE* pSmithTypeValue = pRandAttrTerms->valueList.pData + valueIndex;
						pSmithTypeValue->minValue = readAttrValue((tagGameAttributeType)pRandAttrTerms->type, "min");
						pSmithTypeValue->maxValue = readAttrValue((tagGameAttributeType)pRandAttrTerms->type, "max");
						if ((AttrDataTypes[pRandAttrTerms->type] ==adInt &&  pSmithTypeValue->minValue.nValue > pSmithTypeValue->maxValue.nValue)
							||(AttrDataTypes[pRandAttrTerms->type] ==adFloat &&  pSmithTypeValue->minValue.fValue > pSmithTypeValue->maxValue.fValue))
						{
							OutputMsg(rmError, "Type=%d:RandAttrMin > RandAttrMax",pRandAttrTerms->type);
							return numReaded;
						}
						pSmithTypeValue->prob = getFieldInt("prop", &def);
						valueIndex++;
					}while (enumTableNext());
				}
				closeTable();
			}
			numReaded++;
			pRandAttrTerms++;
		} 
		while (enumTableNext());
	}
	return numReaded;
}
