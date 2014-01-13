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
#include "Player.h"
#include "AppModel.h"

class PlayController : public BaseController {
    
public:
	
    PlayController();
    ~PlayController();
    
    void setup();
    void update();
    
    void createPlayer(string name);
    vector<Player*>& getPlayers();
    vector<int>& getMasters();
    
protected:
	
    vector<Player*>   players;
    vector<int>       masters;
    
private:
	
};

#endif
