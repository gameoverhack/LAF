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
        
        if (!isInEnv(n))
            return false;
        
        
        return true;
	}
    
	void myGraphDescription::getSuccessors(myNode& n, std::vector<myNode>* s, std::vector<double>* c)
	{
		// This function needn't account for obstacles or size of environment. That's done by "isAccessible"
		myNode tn;
		s->clear(); c->clear(); // Planner is supposed to clear these. Still, for safety we clear it again.

        // Define a 8-connected graph
        /*
		for (int a=-1; a<=1; a++)
			for (int b=-1; b<=1; b++) {
				if (a==0 && b==0) continue;
				tn.x = n.x + a;
				tn.y = n.y + b;
				s->push_back(tn);
				c->push_back(sqrt((double)(a*a+b*b)));
			}
         */
        
        
        // Define a 4-connected graph
        for (int a=-1; a<=1; a+=2) {
            tn.x = n.x+a;
            tn.y = n.y;
            if (!isInWindows(tn) && isInEnv(tn)) {
                s->push_back(tn);
                c->push_back(sqrt((double)(a*a)));
            }
            else {
                cout << "fdasfsdfdsfsadfefewfewewgergergergergeargergergergerg "<< isInWindows(tn) << endl;
            }
        }
        
        for (int b=-1; b<=1; b+=2) {
            tn.x = n.x;
            tn.y = n.y+b;
            if (!isInWindows(tn) && isInEnv(tn)) {
                s->push_back(tn);
                c->push_back(sqrt((double)(b*b)));
            }
            else {
                cout << "fdasfsdfdsfsadfefewfewewgergergergergeargergergergerg "<< isInWindows(tn) << endl;
            }
        }
        
	}
    
	double myGraphDescription::getHeuristics(myNode& n1, myNode& n2)
	{
		int dx = abs(n1.x - n2.x);
		int dy = abs(n1.y - n2.y);
		return (sqrt((double)(dx*dx + dy*dy))); // Euclidean distance as heuristics
	}

    bool myGraphDescription::stopSearch_fp(myNode& n) {
        
    }
    
	// -------------------------------
	// constructors
	myGraphDescription::myGraphDescription ()
    { }

bool myGraphDescription::isInEnv(myNode& n) {
    ofRectangle biggerEnv;
    biggerEnv.x = -appModel->getProperty<float>("OutputWidth")/2;
    biggerEnv.y = -appModel->getProperty<float>("OutputHeight");
    biggerEnv.width = appModel->getProperty<float>("OutputWidth")*2;
    biggerEnv.height = appModel->getProperty<float>("OutputHeight")*3;
    
//    biggerEnv.x = 0;
//    biggerEnv.y = 0;
//    biggerEnv.width = appModel->getProperty<float>("OutputWidth");
//    biggerEnv.height = appModel->getProperty<float>("OutputHeight");
    
    float gridScale = appModel->getProperty<float>("gridScale");
    ofPoint scaledNode;
    scaledNode.x = n.x * gridScale;
    scaledNode.y = n.y * gridScale;
    
    return biggerEnv.inside(scaledNode);

}

bool myGraphDescription::isInWindows(myNode& n) {
    float gridScale = appModel->getProperty<float>("gridScale");
    ofRectangle bounding;
    
    ofPoint scaledNode;
    scaledNode.x = n.x * gridScale;
    scaledNode.y = n.y * gridScale;
    
    bounding.setFromCenter(scaledNode,appModel->getProperty<float>("pathBoundingSize"),appModel->getProperty<float>("pathBoundingSize"));
    
    vector<ofRectangle> windows = appModel->getWindows();
    
    for (int w=0;w<windows.size();w++) {
        if (windows[w].intersects(bounding))
            return true;
    }
    
    return false;
}


// =============================================================================

vector< vector< ofPoint > > findPaths(ofPoint _startPoint, ofPoint _finishPoint)
{
	// Profiling observation: Using int instead of double cost provides marginal improvement (~10%)
	GenericSearchGraphDescriptor<myNode,double> myGraph;
	
	// We describe the graph, cost function, heuristics, and the start & goal in this block
	// ------------------------------------------------------------------------------------
	// Create an instance of "myGraphDescription"

	myGraphDescription myGraphClassInstance;
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
	
    // snap the input nodes to the grid
    float gridScale = appModel->getProperty<float>("gridScale");

    myNode startPoint;
    myNode finishPoint;
    startPoint.x = round(_startPoint.x/gridScale);
    startPoint.y = round(_startPoint.y/gridScale);
    
    finishPoint.x = round(_finishPoint.x/gridScale);
    finishPoint.y = round(_finishPoint.y/gridScale);
    
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
            path.push_back(ofPoint(paths[i][j].x*gridScale,paths[i][j].y*gridScale));
        }
        paths2.push_back(path);
    }
    

	return paths2;

}