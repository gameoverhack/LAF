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
    AGENT_PLAN = 0,
    AGENT_RUN
};

enum BehaviousnMode{
    BEHAVIOUR_AUTO = 0,
    BEHAVIOUR_MANUAL
};

enum CollisionMode{
    COLLISION_AVOID = 0,
    COLLISION_IGNORE
};

typedef struct{
    
    ofRectangle currentBounding;
    
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
    void start();
    void stop();
    
    // update is both threaded and unthreaded
    void update();
    
    // SEMI-PERSISTANT
    
    void setCollisionMode(CollisionMode _collisionMode);
    void setBehaviousnMode(BehaviousnMode _behaviourMode);
    
    // PERCEPTIONS
    
    // long term
    void setWorldObstacles(vector<ofRectangle> _obstacles);
    
    // short term
    void setOtherAgents(vector<AgentInfo> _otherAgentInfo);
    
    // COMMANDS
    
    // immediate occasionaly called commands
    void plan(ofRectangle _target, int _numSequenceRetries = 3);
    
    // CHECKS
    
    // external check on thread state
    bool isAgentLocked();
    
    // TALKING?
    
    AgentInfo getAgentInfo();
    
protected:
    
    // METHODS
    
    void insertMoviesFromAction(pair<char,float> act);
    void insertEndMotion();
    bool cutMoviesForActionsNormalised();
    void generateMoviesFromMotions(vector<string>& motionSequence, bool isEnd = false);
    void getPositionsForMovieSequence();
    void generateMotionsBetween(string startMotion, string endMotion, vector<string>& motionSequence);
    
    float calculateMovieDistanceNormalised(int indexA, int indexB, char dir, int frameOffset);
    
    // worker thread
    void threadedFunction();
    
    // convenience locks of bool to indicate threading is occuring
    void lockAgentFlag();
    void unlockAgentFlag();
    void lockAgent();
    void unlockAgent();
    
    // actual worker methods that require threading
    void _plan();
    
    // DATA
    
    // persistant agent vars
    PlayerModel model;
    ofPoint origin;
    ofRectangle planBoundary;
    float drawSize;
    float gridSizeX, gridSizeY;
    MotionGraph forwardGraph, directionGraph, endGraph;
    
    // plan vars
    ofPolyline currentPath;
    vector< pair<char, float> > actions;
    bool bFaultyMovieSequence;
    int numSequenceRetries;
    
    // semi-persistant
    CollisionMode collisionMode;
    BehaviousnMode behaviourMode;
    
    // agent state info
    bool bIsAgentLocked;
    AgentState state;
    AgentInfo agentInfo;
    
    // agents 'memory'
    vector<AgentInfo> otherAgentInfo;
    vector<ofRectangle> obstacles;
    ofRectangle target;
    
};

#endif
