//
//  Agent.h
//  LaughterForgetting
//
//  Created by Omid on 2/20/2014.
//
//

#ifndef LaughterForgetting_Agent_h
#define LaughterForgetting_Agent_h

class Agent : public MovieSequence{
public:
    
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
protected:
    
    bool bHug;
    int window;
    int sframe;
    int gframe;
    
    bool willCollide;
    
    bool isManual;
    
    string playerName;
    
    string LRAction;
    string UDAction;
    
    ofPolyline currentPath;

};



#endif
