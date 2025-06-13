#include "StdAfx.h"
#include "../interface/IntMap.h"

int IntMap::GetLen() const
{
	return (int)m_vecData.size();
}

void IntMap::Add( tagIntMapPair& Data )
{
	m_vecData.push_back(Data);
}

void IntMap::Add(int Key,int Value,int Value2)
{
	tagIntMapPair  data(Key, Value,Value2);
	Add(data);
}

tagIntMapPair IntMap::GetData( unsigned int nIndex ) const
{
	if(nIndex >= 0 && nIndex < m_vecData.size())
		return m_vecData[nIndex];
	return tagIntMapPair(-1,-1);
}


tagIntMapPair IntMap::operator[]( unsigned int nIndex ) const
{
	if(nIndex >= 0 && nIndex < m_vecData.size())
		return m_vecData[nIndex];
	return tagIntMapPair(-1,-1);
}

void IntMap::clear()
{
	m_vecData.clear();
}

tagIntMapPair IntMap::GetDataHead()
{
	return m_vecData.front();
}

void IntMap::SetData( unsigned int nIndex, tagIntMapPair& Data )
{
	if(nIndex >= 0 && nIndex < m_vecData.size())
		m_vecData[nIndex] = Data;
}

 IntMap &IntMap::operator = ( const IntMap &intMap )
{
	int len = intMap.GetLen();
	for(int i = 0; i < len; ++i)
	{
		m_vecData.push_back(intMap.GetData(i));
	}
	return *this;
}
