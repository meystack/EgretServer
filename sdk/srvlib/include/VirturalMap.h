#pragma once
class VirturalMap
{
public:
	VirturalMap(){};
	virtual bool canEntityMove(int x,int y,int nParam,int nParam2,int nParam3,int nParam4){return false;} ;
};