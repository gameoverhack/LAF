//
//  PlayController2.h
//  LaughterForgetting
//
//  Created by omid on 09/02/14.
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
    
protected:
    
    void getPositionsForMovieSequence(MovieSequence* movieSequence, string name);
    void generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence, string name);
    void generateMotionsBetween(string m1, string m2, string name, vector<string>& motionSequence, bool bForward = true);
    
private:
	
};

#endif
