//
//  PlayController.cpp
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#include "PlayController.h"
#include "AgentBehaviours.h"

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
    
    /******************************************************
     *******                States                  *******
     *****************************************************/
    
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
            
            appModel->resetUniqueTargets();
            appModel->resetHeroTimer();
            
            appModel->initStartingPositions();

            // make the player views
            appModel->createPlayerViews(appModel->getProperty<int>("NumberPlayers"));
            
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
        }
            break;
        case kPLAYCONTROLLER_MAKE:
        {
            
            createAgent();
            
//            if(appModel->getProperty<bool>("AutoGenerate")){
//                vector<int>& targetWindows = appModel->getWindowTargets();
//                int wTarget = appModel->getUniqueWindowTarget();// if it is not taken
//                //if(wTarget != -1) makeSequence(appModel->getRandomPlayerName(), wTarget);
//                
//                int start = appModel->getUniqueStartPosition();
//                
//                if (wTarget != -1)
//                    makeAgent3(appModel->getRandomPlayerName(), start, wTarget);
//                //if(wTarget != -1) makeAgent("BLADIMIRSL", wTarget);
//            }
//            
//            if(appModel->getProperty<bool>("ManualAgentControl")){
//                makeManualAgent("BLADIMIRSL");
//                appModel->setProperty("ManualAgentControl", true);
//            }
            
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            
            if(appModel->getProperty<bool>("ShowHeroVideos")){
                if(appModel->checkHeroTimer()) appModel->activateHero();
            }

            vector<Agent2*>& agents = appModel->getAgents();
            for(int i = 0; i < agents.size(); i++){
                Agent2* agent = agents[i];
                agent->update();
            
//                agent->update(appModel->getProperty<bool>("AvoidCollisions"), windowPositions, sequences);
                
               // updatePosition(agent);

                if(agent->isSequequenceDone()) appModel->markPlayerForDeletion(agent->getViewID());
                ostringstream os;
                os << agent << endl;
                appModel->setProperty("MovieInfo_" + ofToString(agent->getViewID()), os.str());
            }
            
            appModel->deleteMarkedPlayers();
            
            if(agents.size() < appModel->getProperty<int>("NumberPlayers") &&
               appModel->getProperty<bool>("AutoGenerate") &&
               appModel->hasUniqueWindowTargets()) playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            
            ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
            
            if(hero != NULL){
                if(hero->getFade() > 0.95 && agents.size() > 0){
                    playControllerStates.setState(kPLAYCONTROLLER_STOP);
                }
            }
            
        }
            break;
        case kPLAYCONTROLLER_STOP:
        {
            vector<Agent2*>& agents = appModel->getAgents();
            for(int i = 0; i < agents.size(); i++){
                Agent2* agent = agents[i];
                appModel->markPlayerForDeletion(agent->getViewID());
            }
            appModel->deleteMarkedPlayers();
            
            ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
            
            if(hero != NULL){
                if(hero->getFade() < 0.95 && agents.size() == 0){
                    playControllerStates.setState(kPLAYCONTROLLER_MAKE);
                }
            }
            
        }
            break;
    }
}

//--------------------------------------------------------------
bool PlayController::createAgent(){

    int wTarget = ofRandom(appModel->getWindows().size());//appModel->getUniqueWindowTarget();
    
    if(wTarget != -1){
        
        string name = appModel->getRandomPlayerName();
        ofPoint origin = appModel->getRandomPlayerPosition();
        if(origin.z >= 0){
            ofRectangle target = appModel->getWindows()[wTarget];
            createAgent(name, origin, target, COLLISION_AVOID, BEHAVIOUR_AUTO, wTarget);
            return true;
        }else{
            return false;
        }
        
        
    }
    
    return false;
    
    
}

//--------------------------------------------------------------
void PlayController::createAgent(string name, ofPoint origin, ofRectangle target, CollisionMode cMode, BehaviousnMode bMode, int wTarget){
    
    float tDrawSize = appModel->getProperty<float>("DefaultDrawSize");

    name = "MARTINW";
    
    // make the agent and set model
    Agent2 * agent = new Agent2;
    
    agent->setDrawSize(tDrawSize);

    agent->setModel(appModel->getPlayerTemplate(name));
    
    // get and set motion  graphs
    agent->setMotionGraph(appModel->getGraph("ForwardMotionGraph"),
                          appModel->getGraph("DirectionGraph"),
                          appModel->getGraph("EndGraph"));
    
    agent->setStartPosSegment(origin.z);
    
    // set position and target
    origin.z = 0;
    agent->setOrigin(origin);
    agent->setWindow(wTarget);
    
    // calculate plan boundary and draw size
    ofRectangle tPlanBoundary = ofRectangle(0, 0, appModel->getProperty<float>("OutputWidth"), appModel->getProperty<float>("OutputHeight"));
    tPlanBoundary.growToInclude(-tDrawSize, -tDrawSize);
    tPlanBoundary.growToInclude(appModel->getProperty<float>("OutputWidth") + tDrawSize, appModel->getProperty<float>("OutputHeight") + tDrawSize);
    
    // set plan boundary and draw size
    agent->setWorldObstacles(appModel->getWindows());
    agent->setPlanBoundary(tPlanBoundary);
    //agent->setGridSize(tDrawSize / 2.0f, tDrawSize / 2.0f);
    agent->setGridSize(50, 50);
    
    // set modes
    agent->setCollisionMode(cMode);
    agent->setBehaviousnMode(bMode);
    
    // start threading
    agent->start();
    
    agent->plan(target);
    appModel->addAgent(agent);
    agent->setSpeed(3);
    agent->play();
}

//--------------------------------------------------------------
void PlayController::triggerReplan() {
    int newWindowIndex = ofRandom(11);
    ofRectangle newWindow = appModel->getWindows()[newWindowIndex];
    
    vector<Agent2*>& agents = appModel->getAgents();
    
    for (int i=0; i < agents.size(); i++) {
        //agents[i]->removeAllMovies();
        agents[i]->setWindow(newWindowIndex);
        agents[i]->plan(newWindow);
        
    }
    
}

