//
//  AStarSearch.cpp
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#include "AStarSearch.h"
#include "yagsbpl/yagsbpl_base.h"
#include "yagsbpl/planners/A_star.h"



// ============================================================
// Functions that describe the graph, are placed inside a class


    
	int myGraphDescription::getHashBin(myNode& n) // Use the absolute value of x coordinate as hash bin counter. Not a good choice though!
	{
		return ((int)fabs(n.x)) % 1000;
	}
    
	bool myGraphDescription::isAccessible(myNode& n)
	{
        
        
//        ofPoint scaledNode;
//        scaledNode.x = n.x * pathPlanner->gridScaleX + pathPlanner->offsetX;
//        scaledNode.y = n.y * pathPlanner->gridScaleY + pathPlanner->offsetY;
//        
//        if (!pathPlanner->screenBoundary.inside(scaledNode)) {
//            if ((scaledNode.y > pathPlanner->screenBoundary.height) || (scaledNode.y < pathPlanner->screenBoundary.y)) {
//                if (n.prevNode)
//                    if (n.y == n.prevNode->y)
//                        return false;
//            }
//            
//            if ((scaledNode.x > pathPlanner->screenBoundary.width) || (scaledNode.x < pathPlanner->screenBoundary.x)) {
//                if (n.prevNode)
//                    if (n.x == n.prevNode->x)
//                        return false;
//            }
//        }
        
        if (!isInEnv(n))
            return false;
        
        return true;
	}
    
	void myGraphDescription::getSuccessors(myNode& n, std::vector<myNode>* s, std::vector<double>* c) //TODO: Change the cost function or heuristics to prefer straight lines
	{
		myNode tn;
        
        tn.prevX = n.x;
        tn.prevY = n.y;
        tn.prevNode = &n;
        
		s->clear(); c->clear(); // Planner is supposed to clear these. Still, for safety we clear it again.

        
        int xCost = 0;
        int yCost = 0;
        
        int penalty = 1000000; //TODO: This is now just experimental. Calculate it!
        
        if (n.prevX == n.x)
            xCost = penalty;
        else if (n.prevY == n.y)
            yCost = penalty;
        
        
        if (n.prevNode && n.prevNode->prevNode) {
            
            if (n.x == n.prevNode->x) {
                
                for (int b=-1; b<=1; b+=2) {
                    tn.x = n.x;
                    tn.y = n.y+b;
                    if (!isInObstacle(tn) && isInEnv(tn)) {
                        s->push_back(tn);
                        //c->push_back(sqrt((double)(b*b)));
                        c->push_back(calcDistToObstacles(tn)+yCost);
                    }
                }
                
                if (n.x == n.prevNode->prevNode->x) {
                    for (int a=-1; a<=1; a+=2) {
                        tn.x = n.x+a;
                        tn.y = n.y;
                        if (!isInObstacle(tn) && isInEnv(tn)) {
                            s->push_back(tn);
                            //c->push_back(sqrt((double)(a*a)));
                            c->push_back(calcDistToObstacles(tn)+xCost);
                            
                        }
                    }
                }
                
            } else if (n.y == n.prevNode->y) {
                
                for (int a=-1; a<=1; a+=2) {
                    tn.x = n.x+a;
                    tn.y = n.y;
                    if (!isInObstacle(tn) && isInEnv(tn)) {
                        s->push_back(tn);
                        //c->push_back(sqrt((double)(a*a)));
                        c->push_back(calcDistToObstacles(tn)+xCost);
                    }
                }
                
                if (n.y == n.prevNode->prevNode->y) {
                    for (int b=-1; b<=1; b+=2) {
                        tn.x = n.x;
                        tn.y = n.y+b;
                        if (!isInObstacle(tn) && isInEnv(tn)) {
                            s->push_back(tn);
                            //c->push_back(sqrt((double)(b*b)));
                            c->push_back(calcDistToObstacles(tn)+yCost);
                        }
                    }
                }
                
            }
            
            
        } else {
            for (int a=-1; a<=1; a+=2) {
                tn.x = n.x+a;
                tn.y = n.y;
                if (!isInObstacle(tn) && isInEnv(tn)) {
                    s->push_back(tn);
                    //c->push_back(sqrt((double)(a*a)));
                    c->push_back(calcDistToObstacles(tn)+xCost);
                    
                }
            }

            for (int b=-1; b<=1; b+=2) {
                tn.x = n.x;
                tn.y = n.y+b;
                if (!isInObstacle(tn) && isInEnv(tn)) {
                    s->push_back(tn);
                    //c->push_back(sqrt((double)(b*b)));
                    c->push_back(calcDistToObstacles(tn)+yCost);
                }
            }
        }
        

        
        
	}

	double myGraphDescription::getHeuristics(myNode& n1, myNode& n2)
	{
        int directH = 0;
        
        if (n1.prevNode && n1.prevNode->prevNode) {
            if ((n1.x == n1.prevNode->x  && n1.x == n1.prevNode->prevNode->x)
                || (n1.y == n1.prevNode->y  && n1.y == n1.prevNode->prevNode->y)) {
              
                    if ((n1.prevNode->prevNode->prevNode) &&
                           ( (n1.x == n1.prevNode->prevNode->prevNode->x)
                        || (n1.y == n1.prevNode->prevNode->prevNode->y)))
                        directH = 20000;
                    else
                        directH = 10000;
            }
            
        }
        
        int dx = abs(n1.x - n2.x);
        int dy = abs(n1.y - n2.y);
        int distToTarget = 0;//sqrt((double)(dx*dx + dy*dy));
        int rnd = ofRandom(2000); // make some variety in paths
        return (directH + distToTarget + rnd); // Euclidean distance as heuristics
	}

    bool myGraphDescription::stopSearch_fp(myNode& n) {

    }
    
	// -------------------------------
	// constructors
	myGraphDescription::myGraphDescription (){
    
    }

bool myGraphDescription::isInEnv(myNode& n) {
    ofRectangle biggerEnv;
    
    ofPoint scaledNode;
    scaledNode.x = n.x * pathPlanner->gridScaleX + pathPlanner->offsetX;
    scaledNode.y = n.y * pathPlanner->gridScaleY + pathPlanner->offsetY;
    
    return pathPlanner->worldRect.inside(scaledNode);

}

bool myGraphDescription::isInObstacle(myNode& n) {
    ofRectangle bounding;
    
    ofPoint scaledNode;
    scaledNode.x = n.x * pathPlanner->gridScaleX + pathPlanner->offsetX;
    scaledNode.y = (n.y * pathPlanner->gridScaleY + pathPlanner->offsetY) - pathPlanner->obstAvoidBoundingH/2;
    

    bounding.setFromCenter(scaledNode,pathPlanner->obstAvoidBoundingW, pathPlanner->obstAvoidBoundingH);


    for (int w=0;w<pathPlanner->obstacles.size();w++) {
        if (pathPlanner->obstacles[w].intersects(bounding))
            return true;
    }
    
    return false;
}

float myGraphDescription::calcDistToObstacles(myNode& n) {
    float cost = 0;
    
//    ofPoint scaledNode;
//    scaledNode.x = n.x * pathPlanner->gridScaleX + pathPlanner->offsetX;
//    scaledNode.y = n.y * pathPlanner->gridScaleY + pathPlanner->offsetY;
//    
//    
//    for (int w=0;w<pathPlanner->obstacles.size();w++) {
//        if (distancePointToRectangle(scaledNode,pathPlanner->obstacles[w]) < 1000)  //TODO: use a dynamic parameter
//            cost += distancePointToRectangle(scaledNode,pathPlanner->obstacles[w]);
//    }

    return cost;
}

float myGraphDescription::distancePointToRectangle(ofPoint point, ofRectangle rect) {
   
    //return point.distance(rect.getCenter());

    //  Calculate a distance between a point and a rectangle.
    
    if (point.x < rect.getMinX()) {
        if (point.y < rect.getMinY()) {
            return point.distance(ofPoint(rect.getMinX(), rect.getMinY()));
        }
        else if (point.y > rect.getMaxY()) {
            return point.distance(ofPoint(rect.getMinX(), rect.getMaxY()));
        }
        else {
            return rect.getMinX() - point.x;
        }
    }
    else if (point.x > rect.getMaxX()) {
        if (point.y < rect.getMinY()) {
            return point.distance(ofPoint(rect.getMaxX(), rect.getMinY()));
        }
        else if (point.y > rect.getMaxY()) {
            return point.distanceSquared(ofPoint(rect.getMaxX(), rect.getMaxY()));
        }
        else {
            return point.x - rect.getMaxX();
        }
    }
    else {
        if (point.y < rect.getMinY()) {
            return rect.getMinY() - point.y;
        }
        else if (point.y > rect.getMaxX()) {
            return point.y - rect.getMaxY();
        }
        else {
            return 0;
        }
    }
}

// =============================================================================

vector< vector< ofPoint > > PathPlanner::findPaths(ofPoint _startPoint, ofPoint _finishPoint){
    
    
	// Profiling observation: Using int instead of double cost provides marginal improvement (~10%)
	GenericSearchGraphDescriptor<myNode,double> myGraph;
	
	// We describe the graph, cost function, heuristics, and the start & goal in this block
	// ------------------------------------------------------------------------------------
	// Create an instance of "myGraphDescription"

	myGraphDescription myGraphClassInstance;
    myGraphClassInstance.pathPlanner = this;
    
	// Now set that instance and its functions as our function container
	SearchGraphDescriptorFunctionPointerContainer<myNode,double,myGraphDescription>* fun_pointer_container
    = new SearchGraphDescriptorFunctionPointerContainer<myNode,double,myGraphDescription>;
    
	fun_pointer_container->p = &myGraphClassInstance;
	fun_pointer_container->getHashBin_fp = &myGraphDescription::getHashBin;
	fun_pointer_container->isAccessible_fp = &myGraphDescription::isAccessible;
	fun_pointer_container->getSuccessors_fp = &myGraphDescription::getSuccessors;
	fun_pointer_container->getHeuristics_fp = &myGraphDescription::getHeuristics;
    
    //fun_pointer_container->stopSearch_fp = 0;//&myGraphDescription::stopSearch_fp;
    //fun_pointer_container->storePath_fp = 0;
	
    myGraph.func_container = (SearchGraphDescriptorFunctionContainer<myNode,double>*)fun_pointer_container;
	// Set other variables
	myGraph.hashTableSize = 1001; // Since in this problem, "getHashBin" can return a max of value 201.
	myGraph.hashBinSizeIncreaseStep = 512; // By default it's 128. For this problem, we choose a higher value.
	
    ///
    
    screenBoundary = ofRectangle(100, 100, 1820, 566);
    
    
    
    // snap the input nodes to the grid

    myNode startPoint;
    myNode finishPoint;
    startPoint.x = round(_startPoint.x/this->gridScaleX);
    startPoint.y = round(_startPoint.y/this->gridScaleY);
    
    finishPoint.x = round(_finishPoint.x/this->gridScaleX);
    finishPoint.y = round(_finishPoint.y/this->gridScaleY);
    
    offsetX = _finishPoint.x - finishPoint.x*gridScaleX;
    offsetY = _finishPoint.y - finishPoint.y*gridScaleY;
    
	myGraph.SeedNode = startPoint;
	myGraph.TargetNode = finishPoint;

	
	// Planning
	A_star_planner<myNode,double>  planner;
//	planner.setParams(1.0, 10); // optional.
	planner.init(myGraph);
	planner.plan();
	
    // scale back and convert to ofPoint
    vector< vector< myNode > > paths = planner.getPlannedPaths();
    vector< vector< ofPoint > > paths2;

    for (int i=0; i < paths.size(); i++) {
        vector<ofPoint> path;
        for (int j=paths[i].size()-1;j>=0;j--) {  // the points are reverse
            path.push_back(ofPoint(paths[i][j].x*this->gridScaleX+offsetX,paths[i][j].y*this->gridScaleY+offsetY));
        }
        paths2.push_back(path);
    }
    
	return paths2;
}

 vector< pair<char,float> > PathPlanner::getDirectionsInPath(vector<ofPoint> path) { //TODO: calculate the distance between start and end of one direction
    vector< pair<char,float> > result;
    char last = getDirection(path[0], path[1]);
    int lastIndex = 0;
    for (int i=1;i<path.size()-1;i++) {
        if (getDirection(path[i], path[i+1]) !=last) {
            pair<char,float> segment(last,path[lastIndex].distance(path[i]));
            result.push_back(segment);
            last = getDirection(path[i], path[i+1]);
            lastIndex = i;
        }
    }
     
    if (result.size() > 0 && last != result[result.size()-1].first) {
        pair<char,float> segment(last,path[lastIndex].distance(path[path.size()-1]));
        result.push_back(segment);
    }
     if (result.size() == 0) {
         pair<char,float> segment(last,path[lastIndex].distance(path[path.size()-1]));
         result.push_back(segment);

     }
    return result;
}

char PathPlanner::getDirection(ofPoint one, ofPoint two) { //TODO: add diagonal directions if needed
    if (one == two)
        return 's'; //still
    else if (one.x == two.x && one.y > two.y)
        return 'u'; //up
    else if (one.x == two.x && one.y < two.y)
        return 'd'; //down
    else if (one.x > two.x && one.y == two.y)
        return 'l'; //left
    else if (one.x < two.x && one.y == two.y)
        return 'r'; //right
    else
        return 'x'; //unknown
    
}