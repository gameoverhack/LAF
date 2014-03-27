//
//  PlayController.cpp
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#include "PlayController.h"
#include "AStarSearch.h"

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
            if(appModel->getProperty<bool>("AutoGenerate")){
                vector<int>& targetWindows = appModel->getWindowTargets();
                int wTarget = appModel->getUniqueWindowTarget();// if it is not taken
                //if(wTarget != -1) makeSequence(appModel->getRandomPlayerName(), wTarget);
                if(wTarget != -1) makeAgent(appModel->getRandomPlayerName(), wTarget);
                //if(wTarget != -1) makeAgent("BLADIMIRSL", wTarget);
            }
            
            if(appModel->getProperty<bool>("ManualAgentControl")){
                makeManualAgent("BLADIMIRSL");
                appModel->setProperty("ManualAgentControl", true);
            }
            
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            
            if(appModel->getProperty<bool>("ShowHeroVideos")){
                if(appModel->checkHeroTimer()) appModel->activateHero();
            }

            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                Agent* agent = (Agent*)sequences[i];
                
            
                agent->update(appModel->getProperty<bool>("AvoidCollisions"), windowPositions, sequences);
                
                updatePosition(agent);

                if(agent->isSequequenceDone()) appModel->markPlayerForDeletion(agent->getViewID());
                ostringstream os;
                os << agent << endl;
                appModel->setProperty("MovieInfo_" + ofToString(agent->getViewID()), os.str());
            }
            
            appModel->deleteMarkedPlayers();
            
            if(sequences.size() < appModel->getProperty<int>("NumberPlayers") &&
               appModel->getProperty<bool>("AutoGenerate") &&
               appModel->hasUniqueWindowTargets()) playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            
            ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
            
            if(hero != NULL){
                if(hero->getFade() > 0.95 && sequences.size() > 0){
                    playControllerStates.setState(kPLAYCONTROLLER_STOP);
                }
            }
            
        }
            break;
        case kPLAYCONTROLLER_STOP:
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                appModel->markPlayerForDeletion(sequence->getViewID());
            }
            appModel->deleteMarkedPlayers();
            
            ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
            
            if(hero != NULL){
                if(hero->getFade() < 0.95 && sequences.size() == 0){
                    playControllerStates.setState(kPLAYCONTROLLER_MAKE);
                }
            }
            
        }
            break;
    }
    
}



//--------------------------------------------------------------
void PlayController::moveAgent(Agent* agent, char op) {

    string name = agent->getPlayerName();
    ofxLogNotice() << "Performing action " << op << " for " << name << endl;
    
    
    MovieInfo lastMovie = agent->getLastMovieInSequence();
    
    string lastMotion = ofSplitString(lastMovie.markername,"_")[0] + "_" + ofSplitString(lastMovie.markername,"_")[1];
    
    MotionGraph nestedForwardDirectionGraph = appModel->getGraph("DirectionGraph");
    nestedForwardDirectionGraph.nestGraph(appModel->getGraph("ForwardMotionGraph").getPossibilitie());
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    cout << name <<endl;
    
    agent->shiftPoint = ofPoint(0,0,0);
    
    switch (op) {
        case 'l':
        {
            // get the possible approach motions for going left
            string act = agent->getActionType("LR");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("LEFT");
            
            // get the first possible transition to the next action to left
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act)
                motion = transitions[t];
            }
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            
            break;
        case 'r':
        {
            // get the possible approach motions for going right
            string act = agent->getActionType("LR"); cout<<act <<endl;
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("RIGHT");
            
            // get the first possible transition to the next action to right
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act)
                motion = transitions[t];
            }
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            break;
        case 'u':
        {
            // get the possible approach motions for going up
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("UP");
            
            // get the first possible transition to the next action to up
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "UPPP")
                motion = transitions[t];
            }
            cout<< motion;
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
            
            agent->setActionType("LR", "TRAV");
        }
            break;
        case 'd':
        {
            // get the possible approach motions for going down
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("DOWN");
            
            // get the first possible transition to the next action to down
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "DOWN")
                motion = transitions[t];
            }
            cout<< motion;
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
            
            agent->setActionType("LR", "TRAV");
        }
            break;
        default:
            break;
    }
    
    for (int u=0;u<motionSequence.size();u++)
    cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;
    
    generateMoviesFromMotions(motionSequence, agent, name);
    getPositionsForMovieSequence(agent, agent->getPlayerName());
    agent->normalise();
    
  //  if (!agent->isPlaying())
        agent->play();
}


//--------------------------------------------------------------
void PlayController::moveAgentByPixel(Agent* agent, pair<char,float> act) {
    char op = act.first;
    float length = act.second;
    string name = agent->getPlayerName();
    ofxLogNotice() << "Performing action " << op << " for " << name << endl;
    
    
    MovieInfo lastMovie = agent->getLastMovieInSequence();
    
    string lastMotion = ofSplitString(lastMovie.markername,"_")[0] + "_" + ofSplitString(lastMovie.markername,"_")[1];
    
    MotionGraph nestedForwardDirectionGraph = appModel->getGraph("DirectionGraph");
    nestedForwardDirectionGraph.nestGraph(appModel->getGraph("ForwardMotionGraph").getPossibilitie());
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    cout << name <<endl;
    
    agent->shiftPoint = ofPoint(0,0,0);
    
    if (length <= 200) {
      //  agent->shiftPosition(op, length);
        
        ofPoint t;
        if (op == 'l')
            t = ofPoint (-length,0,0);
        else if (op == 'r')
            t = ofPoint (length,0,0);
        else if (op == 'u')
            t = ofPoint (0,-length,0);
        else if (op == 'd')
            t = ofPoint (0,-length,0);
        
        agent->shiftPoint = t;
//        agent->normalise(agent->getCurrentMovieIndex(),t);
//       return;
    }
    
    switch (op) {
        case 'l':
        {
            // get the possible approach motions for going left
            string act = agent->getActionType("LR");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("LEFT");
            
            // get the first possible transition to the next action to left
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act)
                motion = transitions[t];
            }
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
        
        break;
        case 'r':
        {
            // get the possible approach motions for going right
            string act = agent->getActionType("LR"); cout<<act <<endl;
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("RIGHT");
            
            // get the first possible transition to the next action to right
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act)
                motion = transitions[t];
            }
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
        break;
        case 'u':
        {
            // get the possible approach motions for going up
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("UP");
            
            // get the first possible transition to the next action to up
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "UPPP")
                motion = transitions[t];
            }
            cout<< motion;
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
        break;
        case 'd':
        {
            // get the possible approach motions for going down
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("DOWN");
            
            // get the first possible transition to the next action to down
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "DOWN")
                motion = transitions[t];
            }
            cout<< motion;

            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
        break;
        default:
        break;
    }
    
    for (int u=0;u<motionSequence.size();u++)
        cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;

    generateMoviesFromMotionsAndActions(motionSequence, agent, name, agent->actionIndex,length);
    getPositionsForMovieSequence(agent, agent->getPlayerName());
//    agent->normalise(agent->getCurrentMovieIndex()+1,agent->shiftPoint);
    agent->normalise();
}

//--------------------------------------------------------------
void PlayController::updatePosition(Agent* agent) { //TODO: This is unnecessary now and has to be changed
if (agent->actionIndex<agent->actions.size())// && agent->getCurrentMovie().frame >=  agent->getCurrentMovie().endframe)
     moveAgentByPixel(agent, agent->actions[agent->actionIndex++]);
    
//    if (agent->actionIndex!= agent->getCurrentMovie().agentActionIndex) {
//        
//        if (agent->getCurrentMovie().agentActionIndex==-1)
//            agent->actionIndex=0;
//        else
//            agent->actionIndex = agent->getCurrentMovie().agentActionIndex;
//        
//        moveAgentByPixel(agent, agent->actions[agent->actionIndex]);
//    }
}


//--------------------------------------------------------------
void PlayController::makeManualAgent(string name) {
    ofxLogNotice() << "Making an agent for " << name << endl;
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    float scale = appModel->getProperty<float>("DrawSize") / model.getWidth();
    ofSeedRandom();
    
    // create a new Agent
    Agent* agent = new Agent;
    agent->setManual(true);
    agent->setWindow(0);
    agent->push(model.getFirstMovie());
    agent->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    agent->setPlayerName(name);
    
    // choose a random starting position
    vector<ofRectangle> windows = appModel->getWindows();
    float wStart = appModel->getProperty<float>("OutputWidth");
    float hStart = appModel->getProperty<float>("OutputHeight")+200;
    int startMargin = 400/appModel->getProperty<float>("gridScale");
    
    int sx = ofRandom(startMargin);
    int sy = ofRandom(startMargin);
    int q = floor(ofRandom(4));
    
    // ofPoint startPosition = ofPoint(sx%2==1?sx/2:wStart-sx/2, sy%2==1?sy/2:hStart-sy/2);
    ofPoint startPosition = ofPoint((int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"), (int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"));
    
    
    MovieInfo& loopMovie = agent->getLastMovieInSequence();
    loopMovie.isLooped = true;
    
    
    getPositionsForMovieSequence(agent, name);
    agent->normalise();
    
    ofPoint startPositionAgent = startPosition - agent->getScaledCentreAt(1);
    
    agent->setNormalPosition(startPositionAgent);
    agent->normalise();
    
    agent->setSpeed(3);//ofRandom(1.0, 3.0));
    appModel->addSequence(agent);
    agent->play();
  
}


//--------------------------------------------------------------
void PlayController::makeSequence(string name, int window){
    
    ofxLogNotice() << "Making sequence for " << name << " targeting window " << window << endl;
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    
    // get the possible approach motions for this window
    vector<string> transitions = appModel->getGraph("TargetGraph").getPossibleTransitions(ofToString(window));
    
    // CARA's hack -> she doesn't have TRAV_LEFT or TRAV_RIGT
    if(name == "CARAS" || name == "MEGANHG"){
        eraseAll(transitions, (string)"TRAV_LEFT");
        eraseAll(transitions, (string)"TRAV_RIGT");
    }
    
    // randomly get a motion TODO: make this so that we don't have double approaches
    string motion = transitions[(int)ofRandom(transitions.size())];
    
    // split motion into action and direction
    string action = ofSplitString(motion, "_")[0];
    string direction = ofSplitString(motion, "_")[1];
    
    float scale = appModel->getProperty<float>("DrawSize") / model.getWidth();
    
    // create a new Agent
    Agent* agent = new Agent;
    agent->setWindow(window);
    agent->push(model.getFirstMovie());
    agent->setNormalPosition(ofPoint(0,0,0));
    agent->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    
    agent->setPlayerName(name);
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    // start standing front and go to -> motion
    motionSequence.push_back("STND_FRNT");
    generateMotionsBetween("STND_FRNT", motion, name, motionSequence);
    motionSequence.push_back(motion);
    
    generateMoviesFromMotions(motionSequence, agent, name);
    getPositionsForMovieSequence(agent, name);
    agent->normalise();
    
    // calulate and insert loops of the action to travel far enough to get to the target window
    vector<ofRectangle>& windowPositions = appModel->getWindows();
    
    int inserts = 0;
    float target = appModel->getProperty<float>("DrawSize");
    MovieInfo loopMovie = agent->getLastMovieInSequence();
    
    if(direction == "LEFT" || direction == "RIGT"){
        if(direction == "RIGT") target += windowPositions[window].x;
        if(direction == "LEFT") target += ofGetWidth() - windowPositions[window].x;
        while (agent->getScaledTotalBounding().width < target) {
            inserts++;
            agent->push(loopMovie);
            agent->normalise();
            cout << direction << " " << agent->getScaledTotalBounding().width << endl;
        }
    }else if(direction == "DOWN" || direction == "UPPP") {
        if(direction == "DOWN") target += windowPositions[window].y;
        if(direction == "UPPP") target = 2 * target + ofGetHeight() - windowPositions[window].y;
        while (agent->getScaledTotalBounding().height < target) {
            inserts++;
            agent->push(loopMovie);
            agent->normalise();
            cout << direction << " " << agent->getScaledTotalBounding().height << endl;
        }
    }
    
    motionSequence.clear();
    
    // randomise SYNCMOTIONS or WAITMOTIONS TODO: make this selectable
    vector<string> vEndMotionType(2);
    vEndMotionType[0] = "SYNCMOTIONS";
    vEndMotionType[1] = "WAITMOTIONS";
    string endMotionType = random(vEndMotionType);
    
    vector<string>& endMotions = appModel->getGraph("EndGraph").getPossibleTransitions(endMotionType);
    string emotion = random(endMotions);
    
    generateMotionsBetween(motion, emotion, name, motionSequence);
    
    generateMoviesFromMotions(motionSequence, agent, name);
    getPositionsForMovieSequence(agent, name);
    agent->normalise();
    
    
    // calculate target and syncframes
    MovieInfo& lastMovieInSequence = agent->getLastMovieInSequence();
    int goalFrame = agent->getTotalSequenceFrames() - 1;
    int syncFrame = goalFrame - lastMovieInSequence.startframe + xmp[lastMovieInSequence.name].getMarker(motionSequence[motionSequence.size() - 1]).getStartFrame();
    
    agent->setGoalFrame(goalFrame - lastMovieInSequence.endframe - lastMovieInSequence.startframe);
    agent->setSyncFrame(syncFrame);
    
    ofPoint floorOffset = agent->getScaledFloorOffset();
    ofPoint targetPosition = ofPoint(windowPositions[window].x + windowPositions[window].width / 2.0, windowPositions[window].y, 0.0f);
    ofPoint finalSequencePosition = targetPosition - agent->getScaledPositionAt(goalFrame) - floorOffset;
    
    
    if(emotion != "FALL_BACK"){
        
        agent->setHug(true);
        
        // reverse the motion to get back out
        motionSequence.clear();
        
        if(emotion == "LWNG_FRNT"){
            motionSequence.push_back("LWNG_FRNT");
            motionSequence.push_back("LWNG_FRNT");
        }
        
        if(emotion == "SITT_FRNT"){
            motionSequence.push_back("SITT_FRNT");
            motionSequence.push_back("SITT_FRNT");
            motionSequence.push_back("CRCH_FRNT");
        }
        
        motionSequence.push_back("STND_FRNT");
        
        string reversemotion;
        if(direction == "LEFT") reversemotion = action + "_RIGT";
        if(direction == "RIGT") reversemotion = action + "_LEFT";
        if(direction == "DOWN") reversemotion = action + "_UPPP";
        if(direction == "UPPP") reversemotion = action + "_DOWN";
        
        if(emotion == "SITT_FRNT"){
            generateMotionsBetween("CRCH_FRNT", reversemotion, name, motionSequence);
        }else{
            generateMotionsBetween("STND_FRNT", reversemotion, name, motionSequence);
        }
        
        for(int i = 0; i < inserts + 2; i++) motionSequence.push_back(reversemotion);
        
        generateMoviesFromMotions(motionSequence, agent, name);
        getPositionsForMovieSequence(agent, name);
    }else{
        agent->setHug(false);
    }
    
    agent->setNormalPosition(finalSequencePosition);
    agent->normalise();
    
    ofxLogVerbose() << "Adding MovieSequence" << agent->getMovieSequenceAsString() << endl;
    ofxLogVerbose() << "E(nd) Motion: " << endMotionType << " of " << emotion << endl;
    
    agent->setSpeed(ofRandom(1.0, 3.0));
    
    appModel->addSequence(agent);
    agent->play();
    
    
    // Find a sample path
    ofPoint startPos = finalSequencePosition;
    ofPoint endPos = targetPosition;
    cout << startPos.x << " " << startPos.y << endl;
    cout << endPos.x << " " << endPos.y << endl;
    
    
    // find the paths using A*.
    vector< vector< ofPoint > > paths = PathPlanning::findPaths(startPos,endPos,agent->getWindow());
    if (paths.size()>0)
        agent->setCurrentPath(paths[0]);
    else
        ofxLogVerbose() << "No path found for sequence " << appModel->getSequences().size()-1 << endl;
    
}

//--------------------------------------------------------------
void PlayController::makeAgent(string name, int window){
    
    ofxLogNotice() << "Making an agent for " << name << " targeting window " << window << endl;
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    float scale = appModel->getProperty<float>("DrawSize") / model.getWidth();
    ofSeedRandom();
    
    // create a new Agent
    Agent* agent = new Agent;
    agent->setManual(false);
    agent->setWindow(window);
    agent->push(model.getFirstMovie());
    agent->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    agent->setPlayerName(name);
    
    // choose a random starting position
    vector<ofRectangle> windows = appModel->getWindows();
    float wStart = appModel->getProperty<float>("OutputWidth");
    float hStart = appModel->getProperty<float>("OutputHeight")+200;
    int startMargin = 400/appModel->getProperty<float>("gridScale");
    
    int sx = ofRandom(startMargin);
    int sy = ofRandom(startMargin);
    int q = floor(ofRandom(4));
   
   // ofPoint startPosition = ofPoint(sx%2==1?sx/2:wStart-sx/2, sy%2==1?sy/2:hStart-sy/2);
    ofPoint startPosition = ofPoint((int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"), (int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"));

  //  ofPoint startPosition = ofPoint(3*appModel->getProperty<float>("gridScale"), 3*appModel->getProperty<float>("gridScale"));
    
    
    ofPoint targetPosition = ofPoint(windows[window].x + windows[window].width / 2.0, windows[window].y - appModel->getProperty<float>("pathBoundingSizeH")/2, 0.0f);
    
    
    // get the possible approach motions for this window
    vector<string> targetTransitions = appModel->getGraph("TargetGraph").getPossibleTransitions(ofToString(window));
    
    // CARA's hack -> she doesn't have TRAV_LEFT or TRAV_RIGT
    if(name == "CARAS" || name == "MEGANHG"){
        eraseAll(targetTransitions, (string)"TRAV_LEFT");
        eraseAll(targetTransitions, (string)"TRAV_RIGT");
    }
    
    
   
    getPositionsForMovieSequence(agent, name);
    agent->normalise();
    
    ofPoint startPositionAgent = startPosition - agent->getScaledCentreAt(1);
    
    agent->setNormalPosition(startPositionAgent);
    agent->normalise();
    
    agent->setSpeed(3);//ofRandom(1.0, 3.0));
    appModel->addSequence(agent);
    agent->play();
    
    
    agent->plan(targetPosition); //TODO: fix this issue
    // find the paths using A*.
    ofxLogVerbose() << "Finding a path from (" << agent->getScaledCentreAt(1).x << "," << agent->getScaledCentreAt(1).y  << ") to  (" << targetPosition.x << "," << targetPosition.y << ")"  << endl;
    vector< vector< ofPoint > > paths = PathPlanning::findPaths(agent->getScaledCentreAt(1),targetPosition,agent->getWindow());
    if (paths.size()>0){
        agent->setCurrentPath(paths[0]);
        agent->actions =  PathPlanning::getDirectionsInPath(paths[0]);
    }
    else
        ofxLogVerbose() << "No path found for me "  << endl;
   

//    vector< ofPoint > pp;
//    ofPoint nextPoint = ofPoint(agent->getScaledCentreAt(1).x,agent->getScaledCentreAt(1).y);
//    pp.push_back(nextPoint);
//    ofPoint nextPoint2 = ofPoint(agent->getScaledCentreAt(1).x,agent->getScaledCentreAt(1).y+270);
//    pp.push_back(nextPoint2);
//    ofPoint nextPoint3 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+270);
//    pp.push_back(nextPoint3);
//    ofPoint nextPoint4 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+640);
//    pp.push_back(nextPoint4);
//    ofPoint nextPoint5 = ofPoint(agent->getScaledCentreAt(1).x+320,agent->getScaledCentreAt(1).y+680);
//    pp.push_back(nextPoint5);
//    
//    agent->setCurrentPath(pp);
//    agent->actions = PathPlanning::getDirectionsInPath(pp);
    
    for (int a=0;a<agent->actions.size();a++) {
        cout << "########## action " << agent->actions[a].first << endl;
     }
}



//--------------------------------------------------------------
void PlayController::generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence, string name){
    cout << "----------------" << motionSequence.size() << endl;
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, vector<string> >& markDictionary = model.getMarkerDictionary();
    map<string, ofxXMP>& xmp = model.getXMP();

    // convert the motionSequnce to marker names...
    
    vector<string> markerSequence;
    for(int i = 1; i < motionSequence.size(); i++){
        
        // ...by chaining the motion sequences together
        string markerName = motionSequence[i - 1] + "_" + motionSequence[i];
        
        // if we get to HUGG then let's insert a stand TODO: deal with other situations such as LWNG etc
        if(markerName == "HUGG_FRNT_STND_FRNT") markerName = "STND_FRNT_STND_FRNT";
        
        markerSequence.push_back(markerName);
    }
    
    ofxLogVerbose() << "Generating movies for marker sequence: " << markerSequence << endl;

    MovieInfo lastMovie = movieSequence->getLastMovieInSequence();
    
    for(int i = 0; i < markerSequence.size(); i++){
        
        ofxLogVerbose() << "Find movies with: " << markerSequence[i] << endl;
        
        if(i == 0 && markerSequence[i] == lastMovie.markername){
            ofxLogVerbose() << "...movie already in sequence, continuing" << endl;
            continue;
        }

        // get all the movies with this marker name
        map<string, vector<string> >::iterator it = markDictionary.find(markerSequence[i]);
        assert(it != markDictionary.end());
        vector<string> uniqueMovies = it->second;
        
        // we prefer jumping to a marker in the same movie
        // so see if it's in amongst the unique movies
        int randomIndex = first(uniqueMovies, lastMovie.name);                  // returns -1 if not there
        if(randomIndex == -1) randomIndex = (int)ofRandom(uniqueMovies.size()); // ...hence choose any movie
        
        // select the movie name
        string rMovieName = uniqueMovies[randomIndex];
        
        // let's just check to see if the next marker is the one we want...
        ofxXMPMarker pStartMarker = xmp[rMovieName].getNextMarker(lastMovie.startframe + 1);
        ofxXMPMarker pEndMarker = xmp[rMovieName].getNextMarker(pStartMarker.getStartFrame() + 1);
        
        // if it's not the same movie or the next marker isn't the one we want, then randomly choose one
        if(rMovieName != lastMovie.name || pStartMarker.getName() != markerSequence[i]){

            // get all the markers in the movie which match the sequence name - there can be many!!
            vector<ofxXMPMarker> markers = xmp[rMovieName].getMarkers(markerSequence[i]);
            pStartMarker = markers[(int)ofRandom(markers.size())];
            pEndMarker = xmp[rMovieName].getNextMarker(pStartMarker.getStartFrame() + 1);
            
            ofxLogVerbose() << "Selecting RAND marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << " of " << markers.size() << endl;
            
        }else{
            ofxLogVerbose() << "Selecting NEXT marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << endl;
        }
        
        int startFrame = pStartMarker.getStartFrame();
        int endFrame = pEndMarker.getStartFrame();

        MovieInfo nextMovie;
        nextMovie.name = rMovieName;
        nextMovie.path = model.getPlayerFolder() + nextMovie.name + ".mov";
        nextMovie.startframe = startFrame;
        nextMovie.endframe = endFrame;
        nextMovie.speed = lastMovie.speed;
        nextMovie.markername = markerSequence[i];

        ostringstream os; os << nextMovie;
        ofxLogVerbose() << "Adding movie: " << os.str() << endl;
        
        movieSequence->push(nextMovie);
        lastMovie = nextMovie;
        
    }
    
}

//--------------------------------------------------------------
void PlayController::generateMoviesFromMotionsAndActions(vector<string>& motionSequence, Agent* agent, string name, int actionIndex, int length){
   
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, vector<string> >& markDictionary = model.getMarkerDictionary();
    map<string, ofxXMP>& xmp = model.getXMP();
    
    //
   float scale = appModel->getProperty<float>("DrawSize") / model.getWidth();
  //  float scale = 1;
    // convert the motionSequnce to marker names...
    
    vector<string> markerSequence;
    for(int i = 1; i < motionSequence.size(); i++){
        
        // ...by chaining the motion sequences together
        string markerName = motionSequence[i - 1] + "_" + motionSequence[i];
        
        // if we get to HUGG then let's insert a stand TODO: deal with other situations such as LWNG etc
        if(markerName == "HUGG_FRNT_STND_FRNT") markerName = "STND_FRNT_STND_FRNT";
        
        markerSequence.push_back(markerName);
    }
    
    ofxLogVerbose() << "Generating movies for marker sequence: " << markerSequence << endl;
    
    MovieInfo lastMovie = agent->getLastMovieInSequence();
    
    int lastMovieIndexBeforeAdd = agent->getSequenceSize()-1;

    
    for(int i = 0; i < markerSequence.size(); i++){
        
        ofxLogVerbose() << "Find movies with: " << markerSequence[i] << endl;
        
        if(i == 0 && markerSequence[i] == lastMovie.markername){
            ofxLogVerbose() << "...movie already in sequence, continuing" << endl;
            continue;
        }
        
        // get all the movies with this marker name
        map<string, vector<string> >::iterator it = markDictionary.find(markerSequence[i]);
        assert(it != markDictionary.end());
        vector<string> uniqueMovies = it->second;
        
        // we prefer jumping to a marker in the same movie
        // so see if it's in amongst the unique movies
        int randomIndex = first(uniqueMovies, lastMovie.name);                  // returns -1 if not there
        if(randomIndex == -1) randomIndex = (int)ofRandom(uniqueMovies.size()); // ...hence choose any movie
        
        // select the movie name
        string rMovieName = uniqueMovies[randomIndex];
        
        // let's just check to see if the next marker is the one we want...
        ofxXMPMarker pStartMarker = xmp[rMovieName].getNextMarker(lastMovie.startframe + 1);
        ofxXMPMarker pEndMarker = xmp[rMovieName].getNextMarker(pStartMarker.getStartFrame() + 1);
        
        // if it's not the same movie or the next marker isn't the one we want, then randomly choose one
        if(rMovieName != lastMovie.name || pStartMarker.getName() != markerSequence[i]){
            
            // get all the markers in the movie which match the sequence name - there can be many!!
            vector<ofxXMPMarker> markers = xmp[rMovieName].getMarkers(markerSequence[i]);
            pStartMarker = markers[(int)ofRandom(markers.size())];
            pEndMarker = xmp[rMovieName].getNextMarker(pStartMarker.getStartFrame() + 1);
            
            ofxLogVerbose() << "Selecting RAND marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << " of " << markers.size() << endl;
            
        }else{
            ofxLogVerbose() << "Selecting NEXT marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << endl;
        }
        
        int startFrame = pStartMarker.getStartFrame();
        int endFrame = pEndMarker.getStartFrame();
        
        MovieInfo nextMovie;
        nextMovie.name = rMovieName;
        nextMovie.path = model.getPlayerFolder() + nextMovie.name + ".mov";
        nextMovie.startframe = startFrame;
        nextMovie.endframe = endFrame;
        nextMovie.speed = lastMovie.speed;
        nextMovie.markername = markerSequence[i];
        nextMovie.agentActionIndex = actionIndex;
        
        ostringstream os; os << nextMovie;
        ofxLogVerbose() << "Adding movie: " << os.str() << endl;
        
        agent->push(nextMovie);
        lastMovie = nextMovie;
    }
  
    string direction = ofSplitString(motionSequence[motionSequence.size()-1], "_")[1];
    
    vector<MovieInfo>& movs = agent->getMovieSequence();
    
    ofPoint startPos;
    ofPoint movieSP;
    ofPoint movieEP;
    
    float totalDistance = 0;
    int distx =0;
    int disty =0;
    
    
    //calculate the total distance the agent travels for the motion sequence generated here
    //the distance between the first and last movies that are pushed this time
    for (int i=lastMovieIndexBeforeAdd+1;i<movs.size();i++) {
        
        //find the scaled bounding box from the beginning of the movie
        ofRectangle startBounding = model.getBoundingAt(movs[i].name, movs[i].startframe);
        startBounding.position = (startBounding.getPosition() + model.getKeyFrameAt(movs[i].name, movs[i].startframe))*scale;
        startBounding.scale(scale);
        
        //find the scaled bounding box from the end of the movie
        ofRectangle endBounding = model.getBoundingAt(movs[i].name, movs[i].endframe);
        endBounding.position = (endBounding.getPosition() + model.getKeyFrameAt(movs[i].name, movs[i].endframe))*scale;
        endBounding.scale(scale);
        
        // get the centers of those bounding boxes to calculate their distances
        movieSP = startBounding.getCenter();
        movieEP = endBounding.getCenter();

        distx =abs(movieEP.x - movieSP.x);
        disty =abs(movieEP.y - movieSP.y);
        
        if (direction=="LEFT" || direction=="RIGT")
            totalDistance+=distx;
        if (direction=="DOWN" || direction=="UPPP")
            totalDistance+=disty;
    }

        // if the last movie is not enough to cover the distance repeat it
        while (totalDistance < length) {
            MovieInfo nextMovie;
            nextMovie.name = lastMovie.name;
            nextMovie.path = lastMovie.path;
            nextMovie.startframe = lastMovie.startframe;
            nextMovie.endframe = lastMovie.endframe;
            nextMovie.speed = lastMovie.speed;
            nextMovie.markername = lastMovie.markername;
            nextMovie.agentActionIndex = actionIndex;

            agent->push(nextMovie);
    
            if (direction=="LEFT" || direction=="RIGT")
                totalDistance+=distx;
            
            if (direction=="DOWN" || direction=="UPPP")
                totalDistance+=disty;
            
            ofxLogVerbose() << "Repeating the last movie: distance = " << distx << " or " << disty << " total distance = " << totalDistance << endl;
        }
    
    
        // Now that we have inserted movies to go over the required distance, we need to cut the last movie to get the exact distance
    
        // subtract the distance travelled in the last movie so that we check it frame-by-frame next
        if (direction=="LEFT" || direction=="RIGT")
            totalDistance-=distx;
    
        if (direction=="DOWN" || direction=="UPPP")
            totalDistance-=disty;

    
        // check the last movie and find the cut frame (if any)
        MovieInfo* lastMovieInSeq = &agent->getLastMovieInSequence();
    
        // get the scaled center of the bounding box for the start frame of the movie
        ofRectangle startBounding2 =model.getBoundingAt(lastMovieInSeq->name, lastMovieInSeq->startframe);
        startBounding2.position = (startBounding2.getPosition() + model.getKeyFrameAt(lastMovieInSeq->name, lastMovieInSeq->startframe))*scale;
        startBounding2.scale(scale);
        startPos = startBounding2.getCenter();
    
        for(int f = lastMovieInSeq->startframe+1; f <=lastMovieInSeq->endframe; f++){
            // get the scaled center of the bounding box for the current frame in the loop
            ofRectangle currentPositionBounding = model.getBoundingAt(lastMovieInSeq->name, f);
            currentPositionBounding.position = (currentPositionBounding.getPosition() + model.getKeyFrameAt(lastMovieInSeq->name, f))*scale;
            currentPositionBounding.scale(scale);
            ofPoint pos = currentPositionBounding.getCenter();
            
            int dist;
            distx =abs(startPos.x - pos.x);
            disty =abs(startPos.y - pos.y);
            
            if (direction=="LEFT" || direction=="RIGT")
                dist=distx;
            
            if (direction=="DOWN" || direction=="UPPP")
                dist=disty;
            
            if ((totalDistance+dist) >= length) { // if one movie is enough to cover the distance, this is the right cut frame
                ofxLogVerbose() << "Ending the last movie at " << f << " instead of " << lastMovieInSeq->endframe << endl;
                int oldLength = lastMovieInSeq->endframe - lastMovieInSeq->startframe;
                lastMovieInSeq->endframe=f;//lastMovieInSeq->startframe+f;
                lastMovieInSeq->isCut = true;
                // we have pushed this movie before, so we need to fix the sequence frames
                agent->fixLastSequenceFrame(oldLength, lastMovieInSeq->endframe - lastMovieInSeq->startframe);
                break;
            }
        }
    
//     for (int i=lastMovieIndexBeforeAdd+1;i<movs.size();i++) {
//         
//         cout << " <><><><><> " << movs[i].markername << endl;
//     }
  
}


//--------------------------------------------------------------
void PlayController::getPositionsForMovieSequence(MovieSequence* movieSequence, string name){
    
    PlayerModel& model = appModel->getPlayerTemplate(name);
    vector<MovieInfo>& sequence = movieSequence->getMovieSequence();
    
    for(int i = 0; i < sequence.size(); i++){
        
        MovieInfo& m = sequence[i];
        int totalframes = m.endframe - m.startframe;
        ostringstream os; os << m;
        cout<< "####### totoal frames = " << totalframes << endl;
        if(m.positions.size() == totalframes && m.boundings.size() == totalframes){
            ofxLogWarning() << "Assuming positions are the same for " << os.str() << endl;
            continue;
        }
        
        ofxLogVerbose() << "Getting positions and boundings for " << os.str() << endl;
        
        m.positions.resize(totalframes);
        m.boundings.resize(totalframes);
        m.centres.resize(totalframes);
        
        for(int f = 0; f < totalframes; f++){
            int frame = m.startframe + f;
            m.positions[f] = model.getKeyFrameAt(m.name, frame);
            m.boundings[f] = model.getBoundingAt(m.name, frame);
            m.centres[f] = m.boundings[f].getCenter();
            
            if(f == 0) m.totalbounding = m.boundings[f];
            m.totalbounding.growToInclude(m.boundings[f]);
            
        }
    }
}

//--------------------------------------------------------------
void PlayController::generateMotionsBetween(string m1, string m2, string name, vector<string>& motionSequence, bool bForward){
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    
    ofSeedRandom();
    
    MotionGraph motionGraph;
    vector<string> allPossibleTransitions;
    string startMotion, endMotion;
    
    if(bForward){
        motionGraph = appModel->getGraph("ForwardMotionGraph");
        startMotion = m1;
        endMotion = m2;
    }else{
        motionGraph = appModel->getGraph("BackwardMotionGraph");
        startMotion = m2;
        endMotion = m1;
    }
    
    ofxLogVerbose() << "Finding shortest path between:   " << startMotion << " -> " << endMotion << " for player " << name << endl;
    
    allPossibleTransitions = motionGraph.getPossibleTransitions(startMotion);
    
    vector< vector<string> > solutions;
    for(int i = 0; i < allPossibleTransitions.size(); i++){
        if(allPossibleTransitions[i] == endMotion){
            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << endl;
            vector<string> transitions;
            transitions.push_back(startMotion);
            transitions.push_back(allPossibleTransitions[i]);
            solutions.push_back(transitions);
        }else{
            vector<string> allPossibleTransitions2 = motionGraph.getPossibleTransitions(allPossibleTransitions[i]);
            for(int j = 0; j < allPossibleTransitions2.size(); j++){
                if(allPossibleTransitions2[j] == endMotion){
                    //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << endl;
                    vector<string> transitions;
                    transitions.push_back(startMotion);
                    transitions.push_back(allPossibleTransitions[i]);
                    transitions.push_back(allPossibleTransitions2[j]);
                    solutions.push_back(transitions);
                }else{
                    vector<string> allPossibleTransitions3 = motionGraph.getPossibleTransitions(allPossibleTransitions2[j]);
                    for(int k = 0; k < allPossibleTransitions3.size(); k++){
                        if(allPossibleTransitions3[k] == endMotion){
                            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << endl;
                            vector<string> transitions;
                            transitions.push_back(startMotion);
                            transitions.push_back(allPossibleTransitions[i]);
                            transitions.push_back(allPossibleTransitions2[j]);
                            transitions.push_back(allPossibleTransitions3[k]);
                            solutions.push_back(transitions);
                        }else{
                            vector<string> allPossibleTransitions4 = motionGraph.getPossibleTransitions(allPossibleTransitions3[k]);
                            for(int m = 0; m < allPossibleTransitions4.size(); m++){
                                if(allPossibleTransitions4[m] == endMotion){
                                    //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << endl;
                                    vector<string> transitions;
                                    transitions.push_back(startMotion);
                                    transitions.push_back(allPossibleTransitions[i]);
                                    transitions.push_back(allPossibleTransitions2[j]);
                                    transitions.push_back(allPossibleTransitions3[k]);
                                    transitions.push_back(allPossibleTransitions4[m]);
                                    solutions.push_back(transitions);
                                }else{
                                    vector<string> allPossibleTransitions5 = motionGraph.getPossibleTransitions(allPossibleTransitions4[m]);
                                    for(int n = 0; n < allPossibleTransitions5.size(); n++){
                                        if(allPossibleTransitions5[n] == endMotion){
                                            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << endl;
                                            vector<string> transitions;
                                            transitions.push_back(startMotion);
                                            transitions.push_back(allPossibleTransitions[i]);
                                            transitions.push_back(allPossibleTransitions2[j]);
                                            transitions.push_back(allPossibleTransitions3[k]);
                                            transitions.push_back(allPossibleTransitions4[m]);
                                            transitions.push_back(allPossibleTransitions5[n]);
                                            solutions.push_back(transitions);
                                        }else{
                                            vector<string> allPossibleTransitions6 = motionGraph.getPossibleTransitions(allPossibleTransitions5[n]);
                                            for(int o = 0; o < allPossibleTransitions6.size(); o++){
                                                if(allPossibleTransitions6[o] == endMotion){
                                                    //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << " -> " << allPossibleTransitions6[o] << endl;
                                                    vector<string> transitions;
                                                    transitions.push_back(startMotion);
                                                    transitions.push_back(allPossibleTransitions[i]);
                                                    transitions.push_back(allPossibleTransitions2[j]);
                                                    transitions.push_back(allPossibleTransitions3[k]);
                                                    transitions.push_back(allPossibleTransitions4[m]);
                                                    transitions.push_back(allPossibleTransitions5[n]);
                                                    transitions.push_back(allPossibleTransitions6[o]);
                                                    solutions.push_back(transitions);
                                                }else{
                                                    vector<string> allPossibleTransitions7 = motionGraph.getPossibleTransitions(allPossibleTransitions6[o]);
                                                    for(int p = 0; p < allPossibleTransitions7.size(); p++){
                                                        if(allPossibleTransitions7[p] == endMotion){
                                                            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << " -> " << allPossibleTransitions6[o] << " -> " << allPossibleTransitions7[p] << endl;
                                                            vector<string> transitions;
                                                            transitions.push_back(startMotion);
                                                            transitions.push_back(allPossibleTransitions[i]);
                                                            transitions.push_back(allPossibleTransitions2[j]);
                                                            transitions.push_back(allPossibleTransitions3[k]);
                                                            transitions.push_back(allPossibleTransitions4[m]);
                                                            transitions.push_back(allPossibleTransitions5[n]);
                                                            transitions.push_back(allPossibleTransitions6[o]);
                                                            transitions.push_back(allPossibleTransitions7[p]);
                                                            solutions.push_back(transitions);
                                                        }
//                                                        else{
//                                                            vector<string> allPossibleTransitions8 = motionGraph.getPossibleTransitions(allPossibleTransitions7[p]);
//                                                            for(int q = 0; q < allPossibleTransitions8.size(); q++){
//                                                                if(allPossibleTransitions8[q] == endMotion){
//                                                                    cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << " -> " << allPossibleTransitions6[o] << " -> " << allPossibleTransitions7[p] << " -> " << allPossibleTransitions8[q] << endl;
//                                                                    vector<string> transitions;
//                                                                    transitions.push_back(startMotion);
//                                                                    transitions.push_back(allPossibleTransitions[i]);
//                                                                    transitions.push_back(allPossibleTransitions2[j]);
//                                                                    transitions.push_back(allPossibleTransitions3[k]);
//                                                                    transitions.push_back(allPossibleTransitions4[m]);
//                                                                    transitions.push_back(allPossibleTransitions5[n]);
//                                                                    transitions.push_back(allPossibleTransitions6[o]);
//                                                                    transitions.push_back(allPossibleTransitions7[p]);
//                                                                    transitions.push_back(allPossibleTransitions8[q]);
//                                                                    solutions.push_back(transitions);
//                                                                }else{
//                                                                    vector<string> allPossibleTransitions9 = motionGraph.getPossibleTransitions(allPossibleTransitions8[q]);
//                                                                    for(int r = 0; r < allPossibleTransitions9.size(); r++){
//                                                                        if(allPossibleTransitions9[r] == endMotion){
//                                                                            cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << " -> " << allPossibleTransitions6[o] << " -> " << allPossibleTransitions7[p] << " -> " << allPossibleTransitions8[q] << " -> " << allPossibleTransitions9[r] << endl;
//                                                                            vector<string> transitions;
//                                                                            transitions.push_back(startMotion);
//                                                                            transitions.push_back(allPossibleTransitions[i]);
//                                                                            transitions.push_back(allPossibleTransitions2[j]);
//                                                                            transitions.push_back(allPossibleTransitions3[k]);
//                                                                            transitions.push_back(allPossibleTransitions4[m]);
//                                                                            transitions.push_back(allPossibleTransitions5[n]);
//                                                                            transitions.push_back(allPossibleTransitions6[o]);
//                                                                            transitions.push_back(allPossibleTransitions7[p]);
//                                                                            transitions.push_back(allPossibleTransitions8[q]);
//                                                                            transitions.push_back(allPossibleTransitions9[r]);
//                                                                            solutions.push_back(transitions);
//                                                                        }
//                                                                    }
//                                                                }
//                                                            }
//                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    map<string, vector<string> >& markDictionary = model.getMarkerDictionary();
    
    std::random_shuffle(solutions.begin(), solutions.end());
    int shortest = INFINITY; int index = -1;
    for(int i = 0; i < solutions.size(); i++){
        
        bool legal = true;
        for(int j = 1; j < solutions[i].size(); j++){
            string sMotion = solutions[i][j - 1] + "_" + solutions[i][j];
            map<string, vector<string> >::iterator it = markDictionary.find(sMotion);
            if(it == markDictionary.end()){
                //ofxLogError() << "Solution is not legal: " << sMotion << "   " << solutions[i] << endl;
                legal = false;
                break;
            }
        }
        
        if(solutions[i].size() < shortest && legal){
            index = i;
            shortest = solutions[i].size();
        }
    }
    
    if(index == -1){
        
        ofxLogError() << "No solution found for path between: " << startMotion << " -> " << endMotion << " for player " << name << endl;
        assert(false);
        
    }else{
        
        ofxLogVerbose() << "Solution found for path between: " << startMotion << " -> " << endMotion << " for player " << name << endl;
        ofxLogVerbose() << "                                 " << solutions[index] << endl;
        
        for(int i = 0; i < solutions[index].size(); i++){
            motionSequence.push_back(solutions[index][i]);
        }
        
    }
    
}