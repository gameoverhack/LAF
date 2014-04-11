//
//  Agent.h
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#ifndef LaughterForgetting_Agent_h
#define LaughterForgetting_Agent_h

#include "AppModel.h"
#include "AStarSearch.h"

class Agent : public MovieSequence{
public:

    vector< pair<char,float> > actions;
    int actionIndex = 0;
    int currentAction = 0;

    
    Agent() {
        MovieSequence();
    }
    
    ~Agent(){
        MovieSequence::clear();
        clear();
    }
    
    void setWillCollide(bool w) {
        willCollide=w;
    }
    
    bool getWillCollide() {
        return willCollide;
    }
    
    
    void setActionType(string axes, string action) {
        if (axes == "LR")
            LRAction = action;
        else if (axes == "UD")
            UDAction = action;
    }
    
    string getActionType(string axes) {
        cout<<">"<<axes<< LRAction << endl;
        if (axes == "LR")
            return LRAction;
        else if (axes == "UD")
            return UDAction;
        else return "";
    }
    
    void setPlayerName(string name) {
        playerName = name;
    }
    
    string getPlayerName() {
        return playerName;
    }
    
    ofPolyline getCurrentPath() {
        return currentPath;
    }
    
    void setCurrentPath(ofPolyline p) {
        currentPath.clear();
        currentPath = p;
    }
    
    void setGoalFrame(int f){
        gframe = f;
    }
    
    int getGoalFrame(){
        return gframe;
    }
    
    void setSyncFrame(int f){
        sframe = f;
    }
    
    int getSyncFrame(){
        return sframe;
    }
    
    void setWindow(int w){
        window = w;
    }
    
    int getWindow(){
        return window;
    }
    
    void setHug(bool b){
        bHug = b;
    }
    
    bool getHug(){
        return bHug;
    }

    void clear(){
        MovieSequence::clear();
        
        willCollide = false;
        LRAction = "WALK";
        UDAction = "CLIM";
        playerName = "";
        faultFlag = false;
        
        //drawSize = 100;//appModel->getProperty<float>("DrawSize");
        actions.clear();
        currentAction = 0;
        actionIndex = 0;
        collisionSkipCounter = COLLISIONSKIP;
    }
    
    void update(){
        MovieSequence::update();
    }
    
    void update(bool avoidCollisions, vector<ofRectangle> windows, vector<MovieSequence*> agents){
        //------------- Collision Detection
        if ((collisionSkipCounter == 0) && avoidCollisions) {
            collisionSkipCounter = COLLISIONSKIP;
            
            int range = 50;
            int startFrame = MAX(this->getCurrentSequenceFrame(), 0);
            int endFrame = MIN(this->getCurrentSequenceFrame() + range, this->getTotalSequenceFrames());
            
            this->setWillCollide(false);
            
            for(int j = startFrame; j < endFrame; j+=10){
                for (int w = 0; w < windows.size(); w++) { // with windows except the target window
                    ofRectangle bounding =this->getScaledBoundingAt(j);
                    
                    if (w!= this->getWindow() && bounding.intersects(windows[w])) {
                        
                        recoverFromCollisionWithObstacle(windows, w);
                    }
                }
                
                for (int p = 0; p < agents.size();p++) { // with other players
                    if ((Agent*)agents[p] != this && (Agent*)agents[p]->getScaledBounding().intersects(this->getScaledBounding())) {
                        
                        recoverFromCollisionWithAgent(windows, (Agent*)agents[p]);
                    }
                }
            }
        }
        //-------------
        collisionSkipCounter--;
        MovieSequence::update();
    }
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithAgent(vector<ofRectangle> windows, Agent* otherPlayer) {
       
        this->setWillCollide(true);
       
        return; //!
        
        // get the players model
        map<string, ofxXMP>& xmp = playerModel->getXMP();

        MovieInfo currentMovie = getCurrentMovie();

        ofxXMPMarker lastMarker = xmp[currentMovie.name].getLastMarker(currentMovie.frame);
        ofxXMPMarker nextMarker = xmp[currentMovie.name].getNextMarker(currentMovie.frame);


        cout << "lastmarker name = " << lastMarker.getName() << endl;
        cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
        cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
        cout << "cframe = " << currentMovie.frame << endl;
        
        
        //playerSequence->stop();
        setSpeed(-1* abs(speed));

        StopAt(lastMarker.getStartFrame());

        
    }
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithObstacle(vector<ofRectangle> obstacles, int obstacle) {
        this->setWillCollide(true);
        
        switch (behaviourMode) {
            case bAUTO_REALISTIC:
                recoverFromCollisionWithObstacleAUTO(obstacles,obstacle);
                break;
                
            case bMANUAL:
                recoverFromCollisionWithObstacleMANUAL(obstacles,obstacle);
                break;
            default:
                break;
        }
        
        
        
        
        // get the players model
//        PlayerModel& model = appModel->getPlayerTemplate(playerAgent->getPlayerName());
//        map<string, ofxXMP>& xmp = model.getXMP();
//        
//        MovieInfo currentMovie = playerAgent->getCurrentMovie();
//        
//        ofxXMPMarker lastMarker = xmp[currentMovie.name].getLastMarker(currentMovie.frame);
//        ofxXMPMarker nextMarker = xmp[currentMovie.name].getNextMarker(currentMovie.frame);
//        
//        
//        cout << "lastmarker name = " << lastMarker.getName() << endl;
//        cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
//        cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
//        cout << "cframe = " << currentMovie.frame << endl;
        
        
        //playerSequence->stop();
        // playerSequence->StopAt(lastMarker.getStartFrame());
        //    playerAgent->setSpeed(-1* abs(playerAgent->getSpeed()));

        
        /*
         change stopAt to changeSequenceAt (frame, new motion/movie sequence)
         so, first we find an alternate path/sequence,
         second, the player moves back to the previous marker,
         third, the player replaces its old sequecnce with the new sequence and plays it!
         
         */
    }
    
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithObstacleAUTO(vector<ofRectangle> obstacles, int obstacle) {

    }
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithObstacleMANUAL(vector<ofRectangle> obstacles, int obstacle) {
        stop();
        
        return; //!
        
        // get the players model
        map<string, ofxXMP>& xmp = playerModel->getXMP();
        
        MovieInfo currentMovie = getCurrentMovie();
        
        ofxXMPMarker lastMarker = xmp[currentMovie.name].getLastMarker(currentMovie.frame);
        ofxXMPMarker nextMarker = xmp[currentMovie.name].getNextMarker(currentMovie.frame);
        
        
        cout << "lastmarker name = " << lastMarker.getName() << endl;
        cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
        cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
        cout << "cframe = " << currentMovie.frame << endl;
        
        
        setSpeed(-1* abs(speed));
        StopAt(lastMarker.getStartFrame());
    }
    
    //-----------------------------------------------------------
    void plan(ofPoint startPosition, ofPoint targetPosition, ofRectangle worldRect, vector<ofRectangle> obstacles) {
        PathPlanner pp;
        
        pp.gridScaleX = this->gridSizeX;
        pp.gridScaleY = this->gridSizeY;
        
        pp.worldRect = worldRect;
        pp.obstacles = obstacles;
        
        // the agent can collide with its target window
        pp.targetWindow = this->window;
        
        // set the size of the area that the agent's bounding box can grow used in path finding to avoid possible collisions
        pp.obstAvoidBoundingW = 2.5 * this->drawSize /3.0;
        pp.obstAvoidBoundingH = this->drawSize;
        
        // find the paths using A*.
        ofxLogVerbose() << "Finding a path from (" << this->getScaledCentreAt(1).x << "," << this->getScaledCentreAt(1).y  << ") to  (" << targetPosition.x << "," << targetPosition.y << ")"  << endl;
        
        //vector< vector< ofPoint > > paths = pp.findPaths(this->getScaledCentreAt(1),targetPosition,this->getWindow());
        vector< vector< ofPoint > > paths = pp.findPaths(startPosition,targetPosition,this->getWindow());
        
        if (paths.size()>0){
            this->setCurrentPath(paths[0]);
            this->actions =  pp.getDirectionsInPath(paths[0]);
        }
        else
            ofxLogVerbose() << "No path found for me "  << endl;
        
        //    vector< ofPoint > pp;
        //    ofPoint nextPoint = ofPoint(agent->getScaledCentreAt(1).x,agent->getScaledCentreAt(1).y);
        //    pp.push_back(nextPoint);
        //    ofPoint nextPoint2 = ofPoint(agent->getScaledCentreAt(1).x,agent->getScaledCentreAt(1).y+270);
        //    pp.push_back(nextPoint2);
        //    ofPoint nextPoint3 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+270);
        //    pp.push_back(nextPoint3);
        //    ofPoint nextPoint4 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+640);
        //    pp.push_back(nextPoint4);
        //    ofPoint nextPoint5 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+680);
        //    pp.push_back(nextPoint5);
        //
        //    agent->setCurrentPath(pp);
        //    agent->actions = PathPlanning::getDirectionsInPath(pp);
    }
    
    //-----------------------------------------------------------
    void rePlan(ofPoint newTargetPosition, ofRectangle worldRect, vector<ofRectangle> obstacles) {
        // TODO: make a smoother transition
        ofPoint currentPos = getScaledPositionAt(sequenceFrames[currentSequenceIndex] + currentMovie.frame - currentMovie.startframe);
        cutMoviesFromCurrentFrame();
        plan(currentPos, newTargetPosition, worldRect, obstacles);
    }

    //--------------------------------------------------------------
    void cutMoviesFromCurrentFrame() {
        // Remove the rest of the movies in the sequence as we are overwriting them
        if (this->getCurrentMovieIndex() < this->getSequenceSize()-1)
            this->removeMoviesFromIndex(this->getCurrentMovieIndex()+1)   ;
//        this->normalise();
       
        //return;
        // Cut the current movie and start the new movie at the current position
       
        int oldLength = currentMovie.endframe - currentMovie.startframe;
        
        // find the next or last marker and cut at that point
        
        // get the players model
        PlayerModel* model = this->getPlayerModel();
        map<string, ofxXMP>& xmp = model->getXMP();
        
        ofxXMPMarker lastMarker = xmp[currentMovie.name].getLastMarker(currentMovie.frame);
        ofxXMPMarker nextMarker = xmp[currentMovie.name].getNextMarker(currentMovie.frame);
        
        
        cout << "lastmarker name = " << lastMarker.getName() << endl;
        cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
        cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
        cout << "cframe = " << currentMovie.frame << endl;
        
        
        
        int newEndFrame = currentMovie.startframe + currentMovie.frame;
        //int newEndFrame = currentMovie->startframe + nextMarker.getStartFrame();
        currentMovie.endframe = newEndFrame;
        this->getMovieSequence()[this->getCurrentMovieIndex()].endframe = newEndFrame;
        
        // we have pushed this movie before, so we need to fix the sequence frame
        this->fixLastSequenceFrame(oldLength, currentMovie.endframe - currentMovie.startframe);
        // getPositionsForMovieSequence(agent, agent->getPlayerName());
    }

    
    float getDrawSize(){
        return drawSize;
    }
    
    void setDrawSize(float d){
        drawSize = d;
    }
    
    float getGridSizeX(){
        return gridSizeX;
    }
    
    void setGridSizeX(float d){
        gridSizeX = d;
    }
    
    float getGridSizeY(){
        return gridSizeY;
    }
    
    void setGridSizeY(float d){
        gridSizeY = d;
    }
    
    void setPlayerModel(PlayerModel& m) {
        playerModel = &m;
    }
    
    PlayerModel* getPlayerModel() {
        return playerModel;
    }
    
    int getStartPosSegment() {
        return startPosSegment;
    }
    
    void setStartPosSegment(int p) {
        startPosSegment = p;
    }
    
    void setFaultyFlag(bool b) {
        faultFlag = b;
    }
    
    bool getFaultyFlag() {
        return faultFlag;
    }
    
protected:
    const int COLLISIONSKIP = 6;
    
    bool bHug;
    int window;
    int sframe;
    int gframe;
        
    string playerName;
    PlayerModel* playerModel;
    
    string LRAction = "WALK";
    string UDAction = "CLIM";
    
    ofPolyline currentPath;
    
    float drawSize;
    float gridSizeX; //girdSizeX should be proportionate to the agent's min left and right movement length (in the movies); changes by changing the drawsize.
    float gridSizeY; //girdSizeY should be proportionate to the agent's min up and down movement length (in the movies); changes by changing the drawsize.
    
    int startPosSegment;
    int collisionSkipCounter = 1;
    
    bool faultFlag = false;
};


#endif
