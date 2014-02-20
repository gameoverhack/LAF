//
//  myNode.h
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#ifndef LaughterForgetting_myNode_h
#define LaughterForgetting_myNode_h

//// A node of the graph
class myNode
{
public:
	int x, y; // Profiling observation: integer coordinates, hence operator==,
    //  makes the search significantly faster (almost 10 folds than double)
	bool operator==(const myNode& n) { return (x==n.x && y==n.y); }; // This must be defined for the node
};


#endif
