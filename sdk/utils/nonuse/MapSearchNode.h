////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// STL A* Search implementation
// (C)2001 Justin Heyes-Jones
//
// Finding a path on a simple grid maze
// This shows how to do shortest path finding using A*

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32


#pragma once
#include "VirturalMap.h"
#include "stlastar.h" // See header for copyright and usage information


#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

using namespace std;




// Definitions
class CMapSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;	
	static VirturalMap * m_pMap; //µØÍ¼µÄÖ¸Õë

	
	CMapSearchNode() { x = y = -1; }
	CMapSearchNode(  int px,  int py ) { x=px; y=py;}

	static void SetMapPtr(VirturalMap * pMap)
	{
		m_pMap = pMap;
	}
	int GoalDistanceEstimate( CMapSearchNode &nodeGoal );
	bool IsGoal( CMapSearchNode &nodeGoal );
	bool GetSuccessors( AStarSearch<CMapSearchNode> *astarsearch, CMapSearchNode *parent_node );
	int GetCost( CMapSearchNode &successor );
	bool IsSameState( CMapSearchNode &rhs );
	void PrintNodeInfo(); 


	inline int GetMap(int x,int y,int nParam,int nParam2,int nParam3,int nParam4)
	{
		if(m_pMap==NULL)
		{
			return  -1;
		}
		else
		{
			return m_pMap->canEntityMove(x,y,nParam,nParam2,nParam3,nParam4) ?1:0;
		}
	}
	

};

#endif

