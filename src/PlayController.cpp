//
//  PlayController.cpp
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#include "PlayController.h"

//--------------------------------------------------------------
PlayController::PlayController(){
    ofxLogVerbose() << "Creating PlayController" << endl;
}

//--------------------------------------------------------------
PlayController::~PlayController(){
    ofxLogVerbose() << "Destroying PlayController" << endl;
}

//--------------------------------------------------------------
void PlayController::setup(){
    
    StateGroup newPlayControllerStates("PlayControllerStates", true);
    
    newPlayControllerStates.addState(State(kPLAYCONTROLLER_INIT, "kPLAYCONTROLLER_INIT"));
    newPlayControllerStates.addState(State(kPLAYCONTROLLER_MAKE, "kPLAYCONTROLLER_MAKE"));
    newPlayControllerStates.addState(State(kPLAYCONTROLLER_PLAY, "kPLAYCONTROLLER_PLAY"));
    newPlayControllerStates.addState(State(kPLAYCONTROLLER_STOP, "kPLAYCONTROLLER_STOP"));
    
    appModel->addStateGroup(newPlayControllerStates);
    
    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
    playControllerStates.setState(kPLAYCONTROLLER_INIT);
    
}

//--------------------------------------------------------------
void PlayController::update(){
    
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch (playControllerStates.getState()) {
        case kPLAYCONTROLLER_INIT:
        {
            
            ofxLogNotice() << "PLAYCONTROLLER INIT" << endl;
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
            
        }
            break;
        case kPLAYCONTROLLER_MAKE:
        {
            
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->update();
            }
        }
            break;
        case kPLAYCONTROLLER_STOP:
        {
            
        }
            break;
    }
    
}
