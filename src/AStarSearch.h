//
//  AStarSearch.h
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#ifndef __LaughterForgetting__AStarSearch__
#define __LaughterForgetting__AStarSearch__

//#include <iostream>
#include "AppModel.h"
#include "myNode.h"


class myGraphDescription {
public:
    myGraphDescription();
    int getHashBin(myNode& n);
    double getHeuristics(myNode& n1, myNode& n2);
    void getSuccessors(myNode& n, std::vector<myNode>* s, std::vector<double>* c);
    bool isAccessible(myNode& n);
    bool stopSearch_fp(myNode& n);
    bool isInEnv(myNode& n);
    bool isInWindows(myNode& n);
    float calcDistToWindows(myNode& n);
    float distancePointToRectangle(ofPoint point, ofRectangle rect);
    
    myNode targetNode;
    int targetWindow;
};

class PathPlanning {
    public:
        static vector< vector< ofPoint > > findPaths(ofPoint startPoint, ofPoint finishPoint,int _targetWindow);
        static char getDirection(ofPoint one, ofPoint two);
        static vector< pair<char,float> > getDirectionsInPath(vector<ofPoint> path);
    
};


#endif /* defined(__LaughterForgetting__AStarSearch__) */
