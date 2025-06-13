#include "StdAfx.h"
#include "Actor.h"
#include "../script/interface/LogDef.h"
#include "../base/Container.hpp" 
 

int CActor::GetGmLevel()
{ 
	return m_nGmLevel;
}
void CActor::SetGmLevel(int nGmLevel)
{ 
	m_nGmLevel = nGmLevel; 
}
