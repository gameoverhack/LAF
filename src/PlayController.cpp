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
    
    switch (playControllerStates.getState()) {
        case kPLAYCONTROLLER_INIT:
        {

            ofxLogNotice() << "PLAYCONTROLLER INIT" << endl;
//            for(int i = 0; i < playerViews.size(); i++){
//                ofxLogNotice() << "Creating Player View: " << i << endl;
//                PlayerView* v = new PlayerView;
//                v->setup(appModel->getProperty<float>("VideoWidth"), appModel->getProperty<float>("VideoWidth"), 2);
//                v->setTransitionLength(appModel->getProperty<int>("TransitionLength"));
//                playerViews[i] = v;
//            }
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
                createPlayer("MARTINW");
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
                
                PlayerModel * playerModelA = appModel->getPlayerModel(i);
                
                players[i]->update();
                
                if(players[i]->getIsFinished()) finished.push_back(i);
                
                ofRectangle& b1 = playerModelA->getBounding();
                for(int j = 0; j < players.size(); j++){
                    
                    PlayerModel * playerModelB = appModel->getPlayerModel(j);
                    
                    if(j != i && !appModel->isIntersected(j) && !appModel->isIntersected(i)){
                        ofRectangle& b2 = playerModelB->getBounding();
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
                
                os  << playerModelA->getCurrentMovieInfo() << "   " << endl
                    << playerModelA->getDistanceToTarget() << " "
                    << playerModelA->getPredictedFrameCurrent() << " / "
                    << playerModelA->getPredictedFrameTotal() << "  "
                    << playerModelA->getPredictedFrameGoal() << "   "
                    << players[i]->getPaused() << " - " << appModel->isIntersected(i) << endl;
            }
            appModel->setProperty("MovieInfo", os.str());
            
            for(int i = 0; i < finished.size(); i++){
                int playerID = players[finished[i]]->getPlayerID();
                delete players[finished[i]];
                eraseAt(players, finished[i]);
                appModel->deletePlayerModel(playerID);
            }
            
        }
            break;
        case kPLAYCONTROLLER_STOP:
        {
            
        }
            break;
    }
    
}

//--------------------------------------------------------------
void PlayController::createPlayer(string name){

    int playerID = players.size();
    
    PlayerModel playerTemplate = appModel->getPlayerTemplate(name);
    PlayerModel * playerModel = appModel->getPlayerModel(playerID);
    std::swap(*playerModel, playerTemplate);
    
    playerModel->setup();
    playerModel->clearChains();
    playerModel->setDrawScale(200.0/550.0);
    playerModel->setPosition(ofPoint(0,0,0));
    
    Player* p = new Player;
    p->setup(playerID);
    players.push_back(p);

    
}

//--------------------------------------------------------------
vector<Player*>& PlayController::getPlayers(){
    return players;
}
