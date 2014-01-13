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
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch (playControllerStates.getState()) {
        case kPLAYCONTROLLER_INIT:
        {
            
            ofxLogNotice() << "PLAYCONTROLLER INIT" << endl;
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            
        }
            break;
        case kPLAYCONTROLLER_MAKE:
        {
            
            for(int i = 0; i < players.size(); i++){
                delete players[i];
            }
            
            players.clear();
            
            for(int i = 0; i < appModel->getProperty<int>("NumberPlayers"); i++){
                appModel->createPlayer("MARTINW");
            }
            
            players = appModel->getPlayers();
            for(int i = 0; i < players.size(); i++){
                players[i]->setDrawScale(200.0/550.0);
                players[i]->setPosition(ofPoint(0,0,0));
//                players[i]->setNormalPosition(ofPoint(windowPositions[8].x + windowPositions[8].width / 2.0f,
//                                                      windowPositions[8].y, 0.0f));
            }
            
            
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            ostringstream os; os << endl;
            vector<int> finished;
            
            appModel->clearIntersected();
            
            for(int i = 0; i < players.size(); i++){
                
                players[i]->update();
                
                if(players[i]->getIsFinished()) finished.push_back(i);
                
                ofRectangle& b1 = players[i]->getBounding();
                for(int j = 0; j < players.size(); j++){
                    if(j != i && !appModel->isIntersected(j)){
                        ofRectangle& b2 = players[j]->getBounding();
                        if(b1.intersects(b2)){
//                            if(!players[i]->getPaused() && !players[j]->getPaused()){
//                                if(players[i]->getDistanceToTarget() > 100 && players[j]->getDistanceToTarget() > 100){
//                                    players[i]->setPaused(true);
//                                }else if(players[i]->getDistanceToTarget() > 100 && players[j]->getDistanceToTarget() < 100){
//                                    players[i]->setPaused(true);
//                                }else if(players[i]->getDistanceToTarget() < 100 && players[j]->getDistanceToTarget() > 100){
//                                    players[j]->setPaused(true);
//                                }
//                            }
                            appModel->addIntersected(i, j);
                        }
                    }
                }
                
//                if(players[i]->getPaused() && !appModel->isIntersected(i)) players[i]->setPaused(false);
                
                os  << players[i]->getCurrentMovieInfo() << "   " << endl
                    << players[i]->getDistanceToTarget() << " "
                    << players[i]->getPredictedFrameCurrent() << " / "
                    << players[i]->getPredictedFrameTotal() << "  "
                    << players[i]->getPredictedFrameGoal() << "   "
                    << players[i]->getPaused() << " - " << appModel->isIntersected(i) << endl;
            }
            appModel->setProperty("MovieInfo", os.str());
            
            for(int i = 0; i < finished.size(); i++){
                delete players[finished[i]];
                eraseAt(players, finished[i]);
            }
            
        }
            break;
        case kPLAYCONTROLLER_STOP:
        {
            
        }
            break;
    }
    
}
