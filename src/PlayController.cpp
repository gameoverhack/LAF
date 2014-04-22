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

            // make the player views
            appModel->createPlayerViews(appModel->getProperty<int>("NumberPlayers"));
            
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
        }
            break;
        case kPLAYCONTROLLER_MAKE:
        {
            
            if(appModel->getAgents().size() < 5){
                createAgent(BEHAVIOUR_MANUAL);
            }else{
                createAgent(BEHAVIOUR_AUTO);
            }
            
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            
            if(appModel->getProperty<bool>("ShowHeroVideos")){
                if(appModel->checkHeroTimer()) appModel->activateHero();
            }

            vector<Agent2*>& agents = appModel->getAgents();

            vector<AgentInfo> allAgentInfo;
            for(int i = 0; i < agents.size(); i++) allAgentInfo.push_back(agents[i]->getAgentInfo());
            for(int i = 0; i < agents.size(); i++) agents[i]->setOtherAgents(allAgentInfo);
            
            for(int i = 0; i < agents.size(); i++){
                Agent2* agent = agents[i];
                
                agent->update();

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
bool PlayController::createAgent(BehaviourMode bMode){
    
    string name = appModel->getRandomPlayerName();
    ofRectangle target = appModel->getUniqueAgentTarget();
    ofPoint origin;
    switch (bMode) {
        case BEHAVIOUR_AUTO:
            origin = appModel->getUniqueAgentOrigin();
            break;
        case BEHAVIOUR_MANUAL:
            origin = ofPoint(target.x + target.width / 2.0, target.y, 0.0f);
            break;
    }
    if(origin != NoOrigin && target != NoTarget){
        createAgent(name, origin, target, COLLISION_AVOID, bMode);
        return true;
    }
    
    return false;

}

//--------------------------------------------------------------
void PlayController::createAgent(string name, ofPoint origin, ofRectangle target, CollisionMode cMode, BehaviourMode bMode){
    
    float tDrawSize = appModel->getProperty<float>("DefaultDrawSize");

    // make the agent and set model
    Agent2 * agent = new Agent2;
    appModel->addAgent(agent);
    
    agent->setDrawSize(tDrawSize);

    agent->setModel(appModel->getPlayerTemplate(name));
    
    // get and set motion  graphs
    agent->setMotionGraph(appModel->getGraph("ForwardMotionGraph"),
                          appModel->getGraph("DirectionGraph"),
                          appModel->getGraph("EndGraph"));
    
//    agent->setStartPosSegment(origin.z);
    
    // set position and target
    //origin.z = 0;
    agent->setOrigin(origin);
    //agent->setWindow(wTarget);
    
    // calculate plan boundary and draw size
    ofRectangle tPlanBoundary = ofRectangle(-tDrawSize * 2.0, -tDrawSize * 2.0, appModel->getProperty<float>("OutputWidth") + tDrawSize * 4.0, appModel->getProperty<float>("OutputHeight") + tDrawSize * 4.0);
    
    // set plan boundary and draw size
    vector<ofRectangle> obstacles;
    vector<int>& windowTargets = appModel->getWindowTargets();
    for(int i = 0; i < windowTargets.size(); i++) obstacles.push_back(appModel->getWindows()[windowTargets[i]]);
    
    agent->setWorldObstacles(obstacles);
    agent->setPlanBoundary(tPlanBoundary);
    //agent->setGridSize(tDrawSize / 2.0f, tDrawSize / 2.0f);
    agent->setGridSize(50, 50);
    
    // set modes
    agent->setCollisionMode(cMode);
    agent->setBehaviourMode(bMode);
    
    // start threading
    agent->startAgent();
    
    agent->setSpeed(3);
    agent->play();
    agent->plan(target);

}

//--------------------------------------------------------------
void PlayController::moveAgent(int agentIndex, char direction) {
    appModel->getAgents()[agentIndex]->move(direction);
}

//--------------------------------------------------------------
void PlayController::triggerReplan() {
    
    cout << "++++++++++++++++++++++++++++++++ REPLAN" << endl;
    
    int newWindowIndex = ofRandom(11);
    ofRectangle newWindow = appModel->getWindows()[newWindowIndex];
    
    vector<Agent2*>& agents = appModel->getAgents();
    
    for (int i=0; i < agents.size(); i++) {
        //agents[i]->removeAllMovies();
        //agents[i]->stop();
        //agents[i]->setWindow(newWindowIndex);
        agents[i]->plan(newWindow);
        
    }
    
}

