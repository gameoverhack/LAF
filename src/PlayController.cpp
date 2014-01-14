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
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            
        }
            break;
        case kPLAYCONTROLLER_MAKE:
        {
            
            for(int i = 0; i < players.size(); i++){
                int playerID = players[i]->getPlayerID();
                delete players[i];
                appModel->deletePlayerModel(playerID);
            }
            
            players.clear();
            masters.clear();
            
            for(int i = 0; i < appModel->getProperty<int>("NumberPlayers"); i++){
                createPlayer("MARTINW");
            }
            
            ofxThreadedVideo* hugVideo = appModel->getHugVideo();
            
            if(hugVideo == NULL) appModel->loadHugVideo("MARTINW");
            
            
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            
            vector<int> finished;
            vector<int> slavestart;
            
            appModel->clearIntersected();
            
            for(int i = 0; i < masters.size(); i++){
                PlayerModel * playerModelM = appModel->getPlayerModel(masters[i]);
                PlayerModel * playerModelS = appModel->getPlayerModel(playerModelM->getSlaveID());
                if(playerModelM->getPredictedFrameCurrent() >= playerModelM->getSlaveFrame()){
                    cout << "SYNCING" << endl;
                    cout << masters[i] << " " << playerModelM->getSlaveID() << endl;
                    cout << masters << endl;
                    players[playerModelM->getSlaveID()]->setPausedSquence(false);
                    slavestart.push_back(i);
                }
            }
            
            for(int i = 0; i < slavestart.size(); i++){
                cout << masters << endl;
                eraseAt(masters, slavestart[i]);
                cout << masters << endl;
            }
            
            for(int i = 0; i < players.size(); i++){
                
                players[i]->update();
                
                PlayerModel * playerModelA = appModel->getPlayerModel(players[i]->getPlayerID());
                
                if(players[i]->getIsFinished()) finished.push_back(i);
                
                ofRectangle& b1 = playerModelA->getBounding();
                for(int j = 0; j < players.size(); j++){
                    
                    PlayerModel * playerModelB = appModel->getPlayerModel(players[j]->getPlayerID());
                    
                    if(j != i && !appModel->isIntersected(players[i]->getPlayerID()) && !appModel->isIntersected(players[j]->getPlayerID())){
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
                            appModel->addIntersected(players[i]->getPlayerID(), players[j]->getPlayerID());
                        }
                    }
                }
                
//                if(players[i]->getPaused() && !appModel->isIntersected(i)) players[i]->setPaused(false);
                ostringstream os; os << endl;
                os  << playerModelA->getCurrentMovieInfo() << "   " << endl
                    << playerModelA->getDistanceToTarget() << " "
                    << playerModelA->getPredictedFrameCurrent() << " / "
                    << playerModelA->getPredictedFrameTotal() << "  "
                    << playerModelA->getPredictedFrameGoal() << "   "
                    << playerModelA->getPredictedFrameSync() << "   "
                    << players[i]->getPaused() << " - " << appModel->isIntersected(players[i]->getPlayerID()) << endl;
                
                appModel->setProperty("MovieInfo_" + ofToString(players[i]->getPlayerID()), os.str());
            }
            
            
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

    appModel->createPlayerModel(playerID, name);
    PlayerModel* playerModel = appModel->getPlayerModel(playerID);
    
    playerModel->setup(playerID);
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

//--------------------------------------------------------------
vector<int>& PlayController::getMasters(){
    return masters;
}
