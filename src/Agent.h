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
//#include "AStarSearch.h"

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
    
    bool getManual() {
        return isManual;
    }
    
    void setManual(bool m) {
        isManual = m;
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
        isManual = false;
        LRAction = "WALK";
        UDAction = "CLIM";
        playerName = "";

    }
    
    void update(){
        MovieSequence::update();
    }
    
    void update(bool avoidCollisions, vector<ofRectangle> obstacles, vector<MovieSequence*> agents){
        //------------- Collision Detection
        if (avoidCollisions) {
            int range = 5;
            int startFrame = MAX(this->getCurrentSequenceFrame(), 0);
            int endFrame = MIN(this->getCurrentSequenceFrame() + range, this->getTotalSequenceFrames());
            
            this->setWillCollide(false);
            
            for(int j = startFrame; j < endFrame; j+=1){
                for (int w = 0; w < obstacles.size(); w++) { // with windows except the target window
                    ofRectangle bounding =this->getScaledBoundingAt(j);
                    
                    if (w!= this->getWindow() && bounding.intersects(obstacles[w])) {
                        
                        recoverFromCollisionWithObstacle(w);
                    }
                }
                
                for (int p = 0; p < agents.size();p++) { // with other players
                    if ((Agent*)agents[p] != this && (Agent*)agents[p]->getScaledBounding().intersects(this->getScaledBounding())) {
                        
                        recoverFromCollisionWithAgent((Agent*)agents[p]);
                    }
                }
            }
        }
        //-------------
        
        MovieSequence::update();
    }
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithAgent(Agent* otherPlayer) {
        //  playerSequence->stop();
        this->setWillCollide(true);
        //  thisPlayer->setSpeed(-1);
        
    }
    
    //--------------------------------------------------------------
    void recoverFromCollisionWithObstacle(int window) {
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
        this->setWillCollide(true);
        
        /*
         change stopAt to changeSequenceAt (frame, new motion/movie sequence)
         so, first we find an alternate path/sequence,
         second, the player moves back to the previous marker,
         third, the player replaces its old sequecnce with the new sequence and plays it!
         
         */
    }
    //-----------------------------------------------------------
    
    void plan(ofPoint targetPosition) {  //TODO: Fix the dependency issues
//        // find the paths using A*.
//        ofxLogVerbose() << "Finding a path from (" << this->getScaledCentreAt(1).x << "," << this->getScaledCentreAt(1).y  << ") to  (" << targetPosition.x << "," << targetPosition.y << ")"  << endl;
//        vector< vector< ofPoint > > paths = PathPlanning::findPaths(this->getScaledCentreAt(1),targetPosition,this->getWindow());
//        if (paths.size()>0){
//            this->setCurrentPath(paths[0]);
//            this->actions =  PathPlanning::getDirectionsInPath(paths[0]);
//        }
//        else
//            ofxLogVerbose() << "No path found for me "  << endl;
    }
    
protected:
    
    bool bHug;
    int window;
    int sframe;
    int gframe;
    
    bool willCollide;
    
    bool isManual;
    
    string playerName;
    
    string LRAction = "WALK";
    string UDAction = "CLIM";
    
    ofPolyline currentPath;

};



#endif
