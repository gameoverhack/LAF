//
//  PlayController.h
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#ifndef __H_PLAYCONTROLLER
#define __H_PLAYCONTROLLER

#include "BaseController.h"
#include "AppModel.h"


class PlayController : public BaseController {
    
public:
	
    PlayController();
    ~PlayController();
    
    void setup();
    void update();
    
    void makeSequence(string name, int window);

    void makeAgent(string name, int window);
    void makeAgent2(string name, int start, int window);
    void makeAgent3(string name, int start, int window);
    void doAction(string name, char action);
    void makeManualAgent(string name);
    void moveAgent(Agent* agent, char op);
    void updatePosition(Agent* agent);
    void insertMoviesFromAction(Agent* agent, pair<char,float> act);
    void triggerReplan();
    void cutSequenceFromCurrentMovie(Agent* agent, bool cutFromCurrentFrame);
    
protected:
    
    void getPositionsForMovieSequence(MovieSequence* movieSequence, string name);
    void generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence, string name, bool isEnd=false);
    void generateMoviesFromMotionsAndActions(vector<string>& motionSequence, Agent* agent, string name, int actionIndex, int length);
    void generateMoviesFromMotionsAndActionsNoCut(vector<string>& motionSequence, Agent* agent, string name, int length);
    void generateMoviesFromMotionsNoPush(vector<string>& motionSequence, MovieSequence* movieSequence, string name, vector<MovieInfo>& resultMovies);
    void generateMotionsBetween(string m1, string m2, string name, vector<string>& motionSequence, bool bForward = true);
    void cutMoviesForActions(Agent* agent);
    void cutMoviesForActionsNormalised(Agent* agent);
    bool cutMoviesForActionsNormalised2(Agent* agent);

    float calcMovieDistance(Agent* agent, MovieInfo* movie, char dir);
    float calcMovieDistanceToFrame(Agent* agent, MovieInfo* movie, char dir, int frame);
    float calcMovieDistanceNormalised(Agent* agent, int index,  MovieInfo* movie, char dir);
    float calcMovieDistanceToFrameNormalised(Agent* agent, int index, MovieInfo* movie, char dir, int frame);
    float calcMoviesDistanceNormalised(Agent* agent, int indexS, int indexE, char dir);
    
    //void recoverFromCollisionWithPlayer(Agent* playerSequence, Agent* collisionSequence);
    //void recoverFromCollisionWithWindow(Agent* playerSequence, int window);
    
private:
	
};

#endif
