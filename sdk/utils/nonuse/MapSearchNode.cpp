//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//// STL A* Search implementation
//// (C)2001 Justin Heyes-Jones
////
//// Finding a path on a simple grid maze
//// This shows how to do shortest path finding using A*
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//#include "MapSearchNode.h" // See header for copyright and usage information
//#include <iostream>
//#include <math.h>
//VirturalMap * CMapSearchNode::m_pMap =NULL;
//
//bool CMapSearchNode::IsSameState( CMapSearchNode &rhs )
//{
//
//	// same state in a maze search is simply when (x,y) are the same
//	if( (x == rhs.x) &&
//		(y == rhs.y) )
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//
//}
//
//void CMapSearchNode::PrintNodeInfo()
//{
//	cout << "Node position : (" << x << ", " << y << ")" << endl;
//}
//
//// Here's the heuristic function that estimates the distance from a Node
//// to the Goal. 
//
//int CMapSearchNode::GoalDistanceEstimate( CMapSearchNode &nodeGoal )
//{
//	return  abs(x - nodeGoal.x) + abs(y - nodeGoal.y);
//}
//
//bool CMapSearchNode::IsGoal( CMapSearchNode &nodeGoal )
//{
//
//	if( (x == nodeGoal.x) &&
//		(y == nodeGoal.y) )
//	{
//		return true;
//	}
//
//	return false;
//}
//
//// This generates the successors to the given Node. It uses a helper function called
//// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
//// is done for each node internally, so here you just set the state information that
//// is specific to the application
//bool CMapSearchNode::GetSuccessors( AStarSearch<CMapSearchNode> *astarsearch, CMapSearchNode *parent_node )
//{
//
//	int parent_x = -1; 
//	int parent_y = -1; 
//
//	if( parent_node )
//	{
//		parent_x = parent_node->x;
//		parent_y = parent_node->y;
//	}
//	
//
//	CMapSearchNode NewNode;
//
//	// push each possible move except allowing the search to go backwards
//	int nCurrentX,nCurrentY;
//	if(parent_x  <0 || parent_y <0)
//	{
//		for(int i=-1;i <=1; i++)
//		{
//			for(int j=-1; j<=1; j++ )
//			{
//				if(i==0 && j==0) continue;
//
//				nCurrentX = x +i;
//				nCurrentY =  y+j;
//				if( GetMap( nCurrentX, nCurrentY,0,0 ) >= 0 )
//				{
//					NewNode = CMapSearchNode( nCurrentX, nCurrentY );
//					astarsearch->AddSuccessor( NewNode );
//				}
//			}
//		}
//	}
//	else
//	{
//		for(int i=-1;i <=1; i++)
//		{
//			for(int j=-1; j<=1; j++ )
//			{
//				if(i==0 && j==0) continue;
//				nCurrentX = x +i;
//				nCurrentY =  y+j;
//
//				if((parent_x == nCurrentX) && (parent_y == nCurrentY) )continue;
//				
//				if( GetMap( nCurrentX, nCurrentY,0 ,0) >= 0 )
//				{
//					NewNode = CMapSearchNode( nCurrentX, nCurrentY );
//					astarsearch->AddSuccessor( NewNode );
//				}
//			}
//		}
//	}
//	
//	return true;
//}
//
//// given this node, what does it cost to move to successor. In the case
//// of our map the answer is the map terrain value at this node since that is 
//// conceptually where we're moving
//
//int CMapSearchNode::GetCost( CMapSearchNode &successor )
//{
//	if(x == -1 || y ==-1 || successor.x ==-1 || successor.y ==-1 )
//	{
//		return 1000000;
//	}
//	else
//	{
//		return  abs(x - successor.x) + abs(y - successor.y);
//	}
//}
//
//
