//
//  Agent2.h
//  LaughterForgetting
//
//  Created by gameover on 18/04/14.
//
//

#ifndef __H_AGENT2
#define __H_AGENT2

#include "ofMain.h"

#include "MovieSequence.h"
#include "VectorUtils.h"
#include "PlayerModel.h"
#include "MotionGraph.h"

#include "AStarSearch.h"

enum AgentState{
    AGENT_INIT = 0, 
    AGENT_PLAN,
    AGENT_MOVE,
    AGENT_RUN
};

enum BehaviourMode{
    BEHAVIOUR_AUTO = 0,
    BEHAVIOUR_MANUAL
};

enum CollisionMode{
    COLLISION_AVOID = 0,
    COLLISION_IGNORE
};

static int sAgentID = 0;

typedef struct{
    
    // persistant
    int agentID;
    
    // changelable
//    MovieInfo currentMovieInfo;
    ofRectangle currentBounding;
    ofPoint currentPosition;
    ofPoint currentCentre;
    
    // semi-persistant
    CollisionMode collisionMode;
    BehaviourMode behaviourMode;
    
    ofRectangle target;
    ofPoint origin;
    
    char direction;
    
    // agent states
    bool bIsAgentLocked;
    AgentState state;
    
    
    
} AgentInfo;

class Agent2 : public MovieSequence, ofThread{
    
public:
    
    Agent2();
    ~Agent2();
    
    // PERSISTANT SETTERS
    
    void setModel(PlayerModel _model);
    void setMotionGraph(MotionGraph _forwardGraph, MotionGraph _directionGraph, MotionGraph _endGraph);
    
    void setOrigin(ofPoint _origin);
    void setPlanBoundary(ofRectangle _planBoundary);
    void setDrawSize(float _drawSize);
    void setGridSize(float _w, float _h);
    
    // start and stop threading/thinking
    void startAgent();
    void stopAgent();
    
    // update is both threaded and unthreaded
    void update();
    
    AgentInfo getCurrentAgentInfo();
    
    // SEMI-PERSISTANT
    
    void setCollisionMode(CollisionMode _collisionMode);
    void setBehaviourMode(BehaviourMode _behaviourMode);
    
    // PERCEPTIONS
    
    // long term
    void setWorldObstacles(vector<ofRectangle> _obstacles);
    
    // short term
    void setOtherAgents(vector<AgentInfo> _otherAgentInfo);
    
    // COMMANDS
    
    // immediate occasionaly called commands
    void plan(ofRectangle _target, int _numSequenceRetries = 3);
    void move(char _direction);
    
    // CHECKS
    
    // external check on thread state
    inline bool isAgentLocked();
    
    // TALKING?
    
    AgentInfo getAgentInfo();
    
    // annoying
    
    ofRectangle actionBounding;
    bool bActionCollide;
    
    void setWindow(int wTarget);
    int getWindow();
    
    void setGoalFrame(int f);
    int getGoalFrame();
    void setSyncFrame(int f);
    int getSyncFrame();
    
    void setStartPosSegment(int posSegment);
    int getStartPosSegment();
    
    void setHug(bool b){
        bHug = b;
    }
    
    bool getHug(){
        return bHug;
    }
    
    ofPolyline getCurrentPath() {
        return currentPath;
    }
    
    bool getWillCollide() {
        return willCollide;
    }
    
    bool getFaultyFlag() {
        return bFaultyMovieSequence;
    }

    ofPolyline getCorrectedPath() {
        ofPolyline pathFromHere; //= ofPolyline(agent->getCurrentPath().getVertices());
        ofPoint point;
        
        //if (!isAgentLocked())
        {
            
        int index;
        for (int i=0; i < getMovieSequence().size(); i++)
            if (getMovieSequence()[i].agentActionIndex == 0) {
                index = i;
                break;
            }
        
            point = getScaledFloorOffsetAt(getSequenceFrames()[index]);
            pathFromHere.addVertex(point);
            
            for (int a=0;a<actions.size();a++) {
                if (actions[a].first == 'l')
                    point.x-=actions[a].second;
                else if (actions[a].first == 'r')
                    point.x+=actions[a].second;
                else if (actions[a].first == 'u')
                    point.y-=actions[a].second;
                else if (actions[a].first == 'd')
                    point.y+=actions[a].second;
                
                pathFromHere.addVertex(point);
            }
        }
        return pathFromHere;
    }
    
    vector< pair<char, float> > actions;
    
    int getAgentID(){
        return agentID;
    }
    
    void setDeviceID(int _deviceID){
        deviceID = _deviceID;
    }
    
    int getDeviceID(){
        return deviceID;
    }
    
protected:
    
    // METHODS
    
    void insertMoviesFromAction(pair<char,float> act);
    void insertEndMotion();
    bool cutMoviesForActionsNormalised();
    void generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence);
    void getPositionsForMovieSequence(vector<MovieInfo>& movieSequence);
    void generateMotionsBetween(string startMotion, string endMotion, vector<string>& motionSequence);
    
    float calculateMovieDistanceNormalised(int indexA, int indexB, char dir, int frameOffset);
    void cutSequenceFromCurrentMovie(bool cutFromCurrentFrame);
    void generateMoviesFromMotionsNoPush(vector<string>& motionSequence, vector<MovieInfo>& resultMovies);
    void removeMovies(bool bAllMovies);
    
    // worker thread
    void threadedFunction();
    
    // convenience locks of bool to indicate threading is occuring
    void lockAgentFlag();
    void unlockAgentFlag();
    void lockAgent();
    void unlockAgent();
    
    // actual worker methods that require threading
    void _move();
    void _plan();
    
    // DATA
    
    // persistant agent vars
    PlayerModel model;
    ofRectangle planBoundary;
    float drawSize;
    float gridSizeX, gridSizeY;
    MotionGraph forwardGraph, directionGraph, endGraph;
    
    // plan vars
    ofPolyline currentPath;
    bool bFaultyMovieSequence;
    int numSequenceRetries;
    int startPosSegment;
    int windowTargetIndex;
    
    int sframe, gframe;
    bool bHug;
    bool willCollide;
    
    // agent state info
    AgentInfo agentInfo;
    int agentID;
    int deviceID;
    bool bIsAgentLocked;
    
    BehaviourMode behaviourMode;
    
    // agents 'memory'
    vector<AgentInfo> otherAgentInfo;
    vector<ofRectangle> obstacles;
    
};

#endif
