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

    void makeSequenceWithPath(string name, int window);
    void doAction(string name, char action);
    void makeManualAgent(string name);
    
protected:
    
    void getPositionsForMovieSequence(MovieSequence* movieSequence, string name);
    void generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence, string name);
    void generateMotionsBetween(string m1, string m2, string name, vector<string>& motionSequence, bool bForward = true);
    
    void recoverFromCollisionWithPlayer(Agent* playerSequence, Agent* collisionSequence);
    void recoverFromCollisionWithWindow(Agent* playerSequence, int window);
    
private:
	
};

#endif
