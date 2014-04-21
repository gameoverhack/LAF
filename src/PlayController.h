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
    
    bool createAgent();
    void createAgent(string name, ofPoint origin, ofRectangle target, CollisionMode cMode, BehaviourMode bMode);
    
    void triggerReplan();
protected:

    
private:
	
};

#endif
