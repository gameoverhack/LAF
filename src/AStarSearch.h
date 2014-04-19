//
//  AStarSearch.h
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#ifndef __LaughterForgetting__AStarSearch__
#define __LaughterForgetting__AStarSearch__

#include <vector>
#include <ofRectangle.h>
#include "myNode.h"


class PathPlanner {
public:
    float gridScaleX,gridScaleY;
    float obstAvoidBoundingW, obstAvoidBoundingH;
    
    float offsetX, offsetY;
    
    ofRectangle worldRect;
    vector<ofRectangle> obstacles;
    
    
    vector< vector< ofPoint > > findPaths(ofPoint startPoint, ofPoint finishPoint);
    char getDirection(ofPoint one, ofPoint two);
    vector< pair<char,float> > getDirectionsInPath(vector<ofPoint> path);
    
};

class myGraphDescription {
public:
    myGraphDescription();
    int getHashBin(myNode& n);
    double getHeuristics(myNode& n1, myNode& n2);
    void getSuccessors(myNode& n, std::vector<myNode>* s, std::vector<double>* c);
    bool isAccessible(myNode& n);
    bool stopSearch_fp(myNode& n);
    bool isInEnv(myNode& n);
    bool isInObstacle(myNode& n);
    float calcDistToObstacles(myNode& n);
    float distancePointToRectangle(ofPoint point, ofRectangle rect);
    
    myNode targetNode;
    
    PathPlanner* pathPlanner;
};

#endif /* defined(__LaughterForgetting__AStarSearch__) */
