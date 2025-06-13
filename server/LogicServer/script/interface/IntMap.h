#pragma once
#include <vector>

class tagIntMapPair
{
public:
	int Key;
	int Value;
	int Value2;

	tagIntMapPair(int nKey,int nValue) :
	Key(nKey),Value(nValue)
	{
		Value2 = 0;
	}

	tagIntMapPair(int nKey,int nValue,int nValue2) :
		Key(nKey),Value(nValue),Value2(nValue2)
	{}
	 tagIntMapPair() :
		Key(0),Value(0)	,Value2(0)	
	{}
};


class IntMap
{
public:

	IntMap(){};
	~IntMap(){};

	IntMap& operator = (const IntMap &intMap);
	int GetLen() const;
	void Add(tagIntMapPair& Data);
	void Add(int Key,int Value,int Value2 = -1);
	tagIntMapPair GetDataHead();
	tagIntMapPair GetData(unsigned int nIndex) const;
	void SetData(unsigned int nIndex, tagIntMapPair& Data);
	tagIntMapPair operator [] (unsigned int nIndex) const;
	void clear();

private:
	std::vector<tagIntMapPair> m_vecData;
};

