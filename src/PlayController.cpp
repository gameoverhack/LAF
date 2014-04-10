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
            if(appModel->getProperty<bool>("AutoGenerate")){
                vector<int>& targetWindows = appModel->getWindowTargets();
                int wTarget = appModel->getUniqueWindowTarget();// if it is not taken
                //if(wTarget != -1) makeSequence(appModel->getRandomPlayerName(), wTarget);
                
                int start = appModel->getUniqueStartPosition();
                
                if (wTarget != -1)
                    makeAgent3(appModel->getRandomPlayerName(), start, wTarget);
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
                
               // updatePosition(agent);

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
void PlayController::cutSequenceFromCurrentMovie(Agent* agent, bool cutFromCurrentFrame) {
    // Remove the rest of the movies in the sequence as we are overwriting them
    if (agent->getCurrentMovieIndex() < agent->getSequenceSize()-1)
        agent->removeMoviesFromIndex(agent->getCurrentMovieIndex()+1)   ;
    getPositionsForMovieSequence(agent, agent->getPlayerName());
    agent->normalise();
    
    if (!cutFromCurrentFrame)
        return;
    // Cut the current movie and start the new movie at the current position
    MovieInfo* currentMovie = &agent->getCurrentMovie();
    string lastMotion = ofSplitString(currentMovie->markername,"_")[0] + "_" + ofSplitString(currentMovie->markername,"_")[1];
    int oldLength = currentMovie->endframe - currentMovie->startframe;
    
    // find the next or last marker and cut at that point
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(agent->getPlayerName());
    map<string, ofxXMP>& xmp = model.getXMP();
    
    ofxXMPMarker lastMarker = xmp[currentMovie->name].getLastMarker(currentMovie->frame);
    ofxXMPMarker nextMarker = xmp[currentMovie->name].getNextMarker(currentMovie->frame);
    
    
    cout << "lastmarker name = " << lastMarker.getName() << endl;
    cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
    cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
    cout << "cframe = " << currentMovie->frame << endl;
    
    
    
    int newEndFrame = currentMovie->startframe + currentMovie->frame;
    //int newEndFrame = currentMovie->startframe + nextMarker.getStartFrame();
    currentMovie->endframe = newEndFrame;
    agent->getMovieSequence()[agent->getCurrentMovieIndex()].endframe = newEndFrame;
    
    // we have pushed this movie before, so we need to fix the sequence frame
    agent->fixLastSequenceFrame(oldLength, currentMovie->endframe - currentMovie->startframe);
    // getPositionsForMovieSequence(agent, agent->getPlayerName());
}

//--------------------------------------------------------------
void PlayController::moveAgent(Agent* agent, char op) {

    string name = agent->getPlayerName();
    ofxLogNotice() << "Performing action " << op << " for " << name << endl;
    
    // Cut the current movie and start the new movie at the current position
    MovieInfo* currentMovie = &agent->getCurrentMovie();
    string lastMotion = ofSplitString(currentMovie->markername,"_")[2] + "_" + ofSplitString(currentMovie->markername,"_")[3];
    string lastDirection = ofSplitString(currentMovie->markername,"_")[3];
    char lastOp = tolower(lastDirection[0]);
    
    cout << lastOp << endl;
    cout << op << endl;
    
    if (lastOp == 'l') {
        if (op != 'r' && agent->getWillCollide())
            return;
        if (op == 'r' && agent->getSpeed() > 0) {
            cutSequenceFromCurrentMovie(agent,false);
            agent->setSpeed(-1);
            if (!agent->isPlaying()) {
                currentMovie->frame = currentMovie->frame - 10;
                agent->getVideo()->setFrame(agent->getVideo()->getCurrentFrame() - 10);
            }
            agent->setPaused(false);
            agent->stopAtAction(currentMovie->agentActionIndex-1);
            cout<<"reversing " <<endl;
            return;
            //currentMovie->speed =  -1 * abs(currentMovie->speed);
        }
        if (op == 'l')
            cutSequenceFromCurrentMovie(agent,false);
        else
            cutSequenceFromCurrentMovie(agent,true);
    }

    if (lastOp == 'r') {
        if (op != 'l' && agent->getWillCollide()) {
            return;
        }
        if (op == 'l' && agent->getSpeed() > 0) {
            cutSequenceFromCurrentMovie(agent,false);
            agent->setSpeed(-1);
            if (!agent->isPlaying()) {
                currentMovie->frame = currentMovie->frame - 10;
                agent->getVideo()->setFrame(agent->getVideo()->getCurrentFrame() - 10);
            }
            
            agent->setPaused(false);
            agent->stopAtAction(currentMovie->agentActionIndex-1);
            cout<<"reversing " <<endl;
            return;
            //currentMovie->speed =  -1 * abs(currentMovie->speed);
        }
        if (op == 'r')
            cutSequenceFromCurrentMovie(agent,false);
        else
            cutSequenceFromCurrentMovie(agent,true);
    }
    
    if (lastOp == 'u') {
        if (op != 'd' && agent->getWillCollide())
            return;
        if (op == 'd' && agent->getSpeed() > 0) {
            cutSequenceFromCurrentMovie(agent, false);
            agent->setSpeed(-1);
            if (!agent->isPlaying()) {
                currentMovie->frame = currentMovie->frame - 10;
                agent->getVideo()->setFrame(agent->getVideo()->getCurrentFrame() - 10);
            }
            agent->setPaused(false);
            agent->stopAtAction(currentMovie->agentActionIndex-1);
            cout<<"reversing " <<endl;
            return;
            //currentMovie->speed =  -1 * abs(currentMovie->speed);
        }
        if (op == 'u')
            cutSequenceFromCurrentMovie(agent,false);
        else
            cutSequenceFromCurrentMovie(agent,true);
    }
    
    if (lastOp == 'd') {
        if (op != 'u' && agent->getWillCollide())
            return;
        if (op == 'u' && agent->getSpeed() > 0) {
            cutSequenceFromCurrentMovie(agent, false);
            agent->setSpeed(-1);
            if (!agent->isPlaying()) {
                currentMovie->frame = currentMovie->frame - 10;
                agent->getVideo()->setFrame(agent->getVideo()->getCurrentFrame() - 10);
            }
            agent->setPaused(false);
            agent->stopAtAction(currentMovie->agentActionIndex-1);
            cout<<"reversing " <<endl;
            return;
            //currentMovie->speed =  -1 * abs(currentMovie->speed);
        }
        if (op == 'd')
            cutSequenceFromCurrentMovie(agent,false);
        else
            cutSequenceFromCurrentMovie(agent,true);
    }
    

    
    //MovieInfo lastMovie = agent->getLastMovieInSequence();
    
    int prevSequenceSize = agent->getSequenceSize();
    int prevTotalSeqFrames = agent->getTotalSequenceFrames();
    
    MotionGraph nestedForwardDirectionGraph = appModel->getGraph("DirectionGraph");
    nestedForwardDirectionGraph.nestGraph(appModel->getGraph("ForwardMotionGraph").getPossibilitie());
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    cout << name <<endl;
    
    
    switch (op) {
        case 'l':
        {
            // get the possible approach motions for going left
            string act = agent->getActionType("LR");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("LEFT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CLIM_LEFT");
            eraseAll(transitions, (string)"CRCH_LEFT");
            eraseAll(transitions, (string)"STND_LEFT");
            
            // get the first possible transition to the next action to left
            string motion ="";
//            for (int t=0;t<transitions.size();t++) {
//                if (ofSplitString(transitions[t],"_")[0]==act)
//                    motion = transitions[t];
//            }
            if (act == "") {
                motion = transitions[(int)ofRandom(transitions.size())];
                agent->setActionType("LR", ofSplitString(motion,"_")[0]);
            }
            else
                motion = act+"_LEFT";
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                agent->setActionType("UD", "");
            }
        }
            
            break;
        case 'r':
        {
            // get the possible approach motions for going right
            string act = agent->getActionType("LR"); cout<<act <<endl;
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("RIGHT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_RIGT");
            eraseAll(transitions, (string)"CRCH_RIGT");
            eraseAll(transitions, (string)"STND_RIGT");
            
            // get the first possible transition to the next action to right
            string motion ="";
//            for (int t=0;t<transitions.size();t++) {
//                if (ofSplitString(transitions[t],"_")[0]==act)
//                motion = transitions[t];
//            }
            
            if (act == "") {
                motion = transitions[(int)ofRandom(transitions.size())];
                agent->setActionType("LR", ofSplitString(motion,"_")[0]);
            }
            else
                motion = act+"_RIGT";
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                agent->setActionType("UD", "");
            }
        }
            break;
        case 'u':
        {
            // get the possible approach motions for going up
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("UP");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"STND_BACK");
            
            // get the first possible transition to the next action to up
            string motion ="";
//            for (int t=0;t<transitions.size();t++) {
//                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "UPPP")
//                motion = transitions[t];
//            }
            
            if (act == "") {
                motion = transitions[(int)ofRandom(transitions.size())];
                agent->setActionType("UD", ofSplitString(motion,"_")[0]);
            }
            else
                motion = act+"_UPPP";
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                agent->setActionType("LR", "");
            }
            
        }
            break;
        case 'd':
        {
            // get the possible approach motions for going down
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("DOWN");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CRCH_FRNT");
            
            // get the first possible transition to the next action to down
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "DOWN")
                motion = transitions[t];
            }
           
            if (act == "") {
                motion = transitions[(int)ofRandom(transitions.size())];
                agent->setActionType("UD", ofSplitString(motion,"_")[0]);
            }
            else
                motion = act+"_DOWN";
            
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                agent->setActionType("LR", "");
            }
        }
            break;
        default:
            break;
    }
    
    for (int u=0;u<motionSequence.size();u++)
    cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;
    
    
    // Create a vector to store the movies generated for this specific action befor pushing them to the agent
    vector<MovieInfo> newMovies;
    generateMoviesFromMotionsNoPush(motionSequence, agent, name, newMovies);
    
    
    // Calcuate the total bounding for this action
    ofRectangle totalBoundingForAction;
    
    for (int i=0;i<newMovies.size();i++) {
        MovieInfo& m = newMovies[i];
        for(int f = 0; f < m.endframe - m.startframe; f++){
            int frame = m.startframe + f;
            if(f == 0) totalBoundingForAction = agent->getPlayerModel()->getBoundingAt(m.name, frame);
            totalBoundingForAction.growToInclude(agent->getPlayerModel()->getBoundingAt(m.name, frame));
        }
    }
    
    // get the scaled bounding
    totalBoundingForAction.position = agent->getScaledPosition(); //TODO: at the next frame (next movie, currentmovie?)
    totalBoundingForAction.scale(agent->getNormalScale());
    
    // if the bounding intersects with windows, then do not push the sequence in; return from the function
    // TODO: this limits the movements
    for (int w = 0; w<appModel->getWindows().size(); w++)
        if (totalBoundingForAction.intersects(appModel->getWindows()[w]))
            return;
    
    // if it does not intersect, then push the new movie sequence for this action to the agent
    for (int i=0;i<newMovies.size();i++)
        agent->push(newMovies[i]);
    
    getPositionsForMovieSequence(agent, agent->getPlayerName());
    agent->normalise();
    
    
    agent->setSpeed(3);
    
    if (!agent->isPlaying()) {
        cout << "is not playing " << endl;
        agent->play();
    }

}


//--------------------------------------------------------------
void PlayController::insertMoviesFromAction(Agent* agent, pair<char,float> act) {
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
    
    
    switch (op) {
        case 'l':
        {
            // get the possible approach motions for going left
            string act = agent->getActionType("LR");
           
            if (act == "TRAV" && length > agent->getDrawSize() * 5) {
                agent->setActionType("LR", "WALK");
                act = agent->getActionType("LR");
            }
            
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("LEFT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CLIM_LEFT");
            eraseAll(transitions, (string)"CRCH_LEFT");
            eraseAll(transitions, (string)"STND_LEFT");
            
            // get the first possible transition to the next action to left
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "LEFT")
                motion = transitions[t];
            }
            motion = transitions[(int)ofRandom(transitions.size())];
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
            
            if (act == "TRAV" && length > agent->getDrawSize() * 5) {
                agent->setActionType("LR", "WALK");
                act = agent->getActionType("LR");
            }
            
            
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("RIGHT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_RIGT");
            eraseAll(transitions, (string)"CRCH_RIGT");
            eraseAll(transitions, (string)"STND_RIGT");

            
            // get the first possible transition to the next action to right
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "RIGT")
                motion = transitions[t];
            }
            motion = transitions[(int)ofRandom(transitions.size())];
            
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
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"STND_BACK");
            
            // get the first possible transition to the next action to up
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "UPPP")
                motion = transitions[t];
            }
            motion = transitions[(int)ofRandom(transitions.size())];
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                
                agent->setActionType("LR", "TRAV");
            }
        }
        break;
        case 'd':
        {
            // get the possible approach motions for going down
            string act = agent->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("DOWN");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CRCH_FRNT");
            
            // get the first possible transition to the next action to down
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "DOWN")
                motion = transitions[t];
            }
            motion = transitions[(int)ofRandom(transitions.size())];
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
                
                agent->setActionType("LR", "TRAV");
            }
            
        }
        break;
        default:
        break;
    }
    
    for (int u=0;u<motionSequence.size();u++)
        cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;

    generateMoviesFromMotionsAndActionsNoCut(motionSequence, agent, name,length);
    //generateMoviesFromMotions(motionSequence, agent, name);
    
    getPositionsForMovieSequence(agent, agent->getPlayerName());
//    agent->normalise(agent->getCurrentMovieIndex()+1,agent->shiftPoint);
    agent->normalise();
}



//--------------------------------------------------------------
void PlayController::makeManualAgent(string name) {
    ofxLogNotice() << "Making an agent for " << name << endl;
    name = "PRIYAR";
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    // float scale = appModel->getProperty<float>("DefaultDrawSize") / model.getWidth();
    ofSeedRandom();
    
    float drawSizes[] = {100,150,200};
    
    // create a new Agent
    Agent* agent = new Agent;
    agent->setPlayerName(name);
    agent->setPlayerModel(model);
    agent->setBehaviourMode(bMANUAL);
    agent->setDrawSize(appModel->getProperty<float>("DefaultDrawSize"));
    //agent->setGridSizeX(appModel->getProperty<float>("DefaultGridScale"));
    //agent->setGridSizeY(appModel->getProperty<float>("DefaultGridScale"));
    //agent->setDrawSize(drawSizes[(int)ofRandom(3)]);
    agent->setGridSizeX(agent->getDrawSize()/2);
    agent->setGridSizeY(agent->getDrawSize()/2);
    agent->setActionType("LR", "");
    agent->setActionType("UD", "");
    agent->setWindow(0); //TODO: set it to -1
    
    float scale = agent->getDrawSize() / model.getWidth();
    
    agent->push(model.getFirstMovie());
    agent->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    agent->setPlayerName(name);

    
    // choose a random starting position
    int startMargin = 400/appModel->getProperty<float>("gridScale");

    // ofPoint startPosition = ofPoint((int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"), (int)ofRandom(startMargin)*appModel->getProperty<float>("gridScale"));
    ofPoint startPosition = ofPoint(2*appModel->getProperty<float>("gridScale"), 4*appModel->getProperty<float>("gridScale"));

    
    MovieInfo& loopMovie = agent->getLastMovieInSequence();
    loopMovie.isLooped = true;
    loopMovie.agentActionIndex = 0;
    
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
    
    float scale = appModel->getProperty<float>("DefaultDrawSize") / model.getWidth();
    
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
    float target = appModel->getProperty<float>("DefaultDrawSize");
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
}



//--------------------------------------------------------------
void PlayController::makeAgent3(string name, int startX, int window){
    //window = 3;
    //name = "MARTINW";
    name = "PRIYAR";
    
    ofxLogNotice() << "Making an agent for " << name << " targeting window " << window << endl;
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    vector<ofRectangle> windows = appModel->getWindows();
    
    ofSeedRandom();
    
    float drawSizes[] = {100,150,200};
    
    // create a new Agent
    // Set the agent parameters
    Agent* agent = new Agent;
    agent->setPlayerName(name);
    agent->setPlayerModel(model);
    agent->setBehaviourMode(bAUTO_REALISTIC);
    agent->setDrawSize(appModel->getProperty<float>("DefaultDrawSize"));
    //agent->setGridSizeX(appModel->getProperty<float>("DefaultGridScale"));
    //agent->setGridSizeY(appModel->getProperty<float>("DefaultGridScale"));
    //agent->setDrawSize(drawSizes[(int)ofRandom(3)]);
    agent->setGridSizeX(agent->getDrawSize()/2);
    agent->setGridSizeY(agent->getDrawSize()/2);
    agent->setStartPosSegment(startX);
    agent->setWindow(window);
    float scale = agent->getDrawSize() / model.getWidth();
    agent->setNormalScale(scale);
    agent->setPlayerName(name);
    
    
    // Push the first default movie to always start from stand front
    agent->push(model.getFirstMovie());
    
    
    // Find the start and end position of the the agent
    float startYRegion = (int)ofRandom(2)*700;
    //int startX = (int)ofRandom(1920/agent->getGridSizeX())+1;
    int startY = (int)ofRandom(2);
    
    float xSpace = (appModel->getProperty<float>("OutputWidth")-100)/appModel->getProperty<int>("NumberPlayers");
    xSpace = (xSpace/agent->getGridSizeX())*agent->getGridSizeX();
    startX--;
    ofPoint pathStartCenterPosition = ofPoint(startX*xSpace+50 , startY * agent->getGridSizeY() + startYRegion);
    //ofPoint pathStartCenterPosition = ofPoint(400 , startY * agent->getGridSizeY() + startYRegion);
    
    
    // Normalise for the first movie to get the scaled center
    getPositionsForMovieSequence(agent, name);
    agent->normalise();
    
    // Calculate the position of the first video from the scaled center of the first frame, used for normalisation
    ofPoint videoStartPosition = pathStartCenterPosition - agent->getScaledCentreAt(1); // agent's video position
    ofPoint floorOffset = agent->getScaledFloorOffset();
    
    // Target position would be on top of the target window
    ofPoint targetPosition = ofPoint(windows[window].x + windows[window].width / 2.0, windows[window].y, 0.0f);
    ofPoint pathTargetPosition = targetPosition ;// - agent->getScaledCentreAt(1);// - floorOffset;
    pathTargetPosition.y = pathTargetPosition.y - agent->getScaledBoundingAt(1).height/2;
   
    
    // Define the world size that the agent is allowed to move within (plan a path) as a rectangle
    ofRectangle worldRect;
    worldRect.x = 0;//- 1* agent->getDrawSize();
    worldRect.y = 0;//- 1* agent->getDrawSize();
    worldRect.width = appModel->getProperty<float>("OutputWidth") + 1* agent->getDrawSize();
    worldRect.height = appModel->getProperty<float>("OutputHeight") + 1* agent->getDrawSize();
    
    
    // Make the agent plan a path based on the start and target positions, within the world rectangle, and avoiding the windows
    // A path is a sequence of actions
    // Each action represents a direction and a length that the agent has to travel to perform that action
    agent->plan(pathStartCenterPosition, pathTargetPosition, worldRect, appModel->getWindows());
    
    
    agent->setNormalPosition(videoStartPosition);
    agent->normalise();
    
    
    // Make the start positon of the path to the start position of the agent
    // The target position is aligned
    
    // 1. check the A* code, make maybe I have to disable aligning the grid to the target position
    
    // 2. add the uncounted distances to the path, either begining or the end
    //    this needs to be done for both x and y dimensions
    
    float xOffset =  agent->getCurrentPath()[0].x - pathStartCenterPosition.x;
    float yOffset =  agent->getCurrentPath()[0].y - pathStartCenterPosition.y;
 
    cout << "xOffset = " << xOffset << endl;
    cout << "yOffset = " << yOffset << endl;
    
    cout << " Actions before fixing the start position" << endl;
    for (int a=0;a<agent->actions.size();a++) {
        cout << "########## action " << agent->actions[a].first << " , " <<  agent->actions[a].second << endl;
    }
    
    if (agent->actions.size() > 0) {
        if (agent->actions[0].first == 'l')
            agent->actions[0].second -=xOffset;

        if (agent->actions[0].first == 'r')
            agent->actions[0].second +=xOffset;
        
        if (agent->actions[0].first == 'u')
            agent->actions[0].second -=yOffset;
        
        if (agent->actions[0].first == 'd')
            agent->actions[0].second +=yOffset;
    }
    
    if (agent->actions.size() > 1) {
    
        if (agent->actions[1].first == 'l')
            agent->actions[1].second -=xOffset;
        
        if (agent->actions[1].first == 'r')
            agent->actions[1].second +=xOffset;
        
        if (agent->actions[1].first == 'u')
            agent->actions[1].second -=yOffset;
        
        if (agent->actions[1].first == 'd')
            agent->actions[1].second +=yOffset;
    }
    
    cout << " Actions after fixing the start position" << endl;
    for (int a=0;a<agent->actions.size();a++) {
        cout << "########## action " << agent->actions[a].first << " , " <<  agent->actions[a].second << endl;
    }
 
    
    cout << endl;
    // Now that the agent has a set of actions, let's insert movies for them
    for (int a=0;a<agent->actions.size();a++) {
        cout << "########## action " << agent->actions[a].first << endl;
        agent->actionIndex++;
        insertMoviesFromAction(agent, agent->actions[a]);
    }
    
    // Make sure the moveis for each action travel the exact lenth
    cutMoviesForActionsNormalised(agent);
    
    
    // Insert the end motions
    vector<string> motionSequence;
    motionSequence.clear();
    string motion = ofSplitString(agent->getLastMovieInSequence().markername,"_")[0]+"_"+ofSplitString(agent->getLastMovieInSequence().markername,"_")[1];
    
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

    
    
    // TODO: may not need the following anymore:
    // calculate target and syncframes
    MovieInfo& lastMovieInSequence = agent->getLastMovieInSequence();
    int goalFrame = agent->getTotalSequenceFrames() - 1;
    int syncFrame = goalFrame;//
    
    agent->setGoalFrame(goalFrame - lastMovieInSequence.endframe - lastMovieInSequence.startframe);
    agent->setSyncFrame(syncFrame);
    ofPoint finalSequencePosition = targetPosition - agent->getScaledPositionAt(goalFrame) - floorOffset;
    
    
    //agent->setNormalPosition(finalSequencePosition);
    //agent->normalise();
    // ******
    
    agent->setSpeed(3);//ofRandom(1,3));
    appModel->addSequence(agent);
    agent->play();
    
  
    
}


//--------------------------------------------------------------
void PlayController::generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence, string name){
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
    
    int nextActionIndex = lastMovie.agentActionIndex+1;
    
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
        nextMovie.agentActionIndex = nextActionIndex;
        
        ostringstream os; os << nextMovie;
        ofxLogVerbose() << "Adding movie: " << os.str() << endl;
        
        movieSequence->push(nextMovie);
        lastMovie = nextMovie;
        
    }
    
}

//--------------------------------------------------------------
void PlayController::generateMoviesFromMotionsNoPush(vector<string>& motionSequence, MovieSequence* movieSequence, string name, vector<MovieInfo>& resultMovies) {
    
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
    
    int nextActionIndex = lastMovie.agentActionIndex+1;
    
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
        nextMovie.agentActionIndex = nextActionIndex;
        
        ostringstream os; os << nextMovie;
        ofxLogVerbose() << "Adding movie: " << os.str() << endl;
        
        resultMovies.push_back(nextMovie);
        lastMovie = nextMovie;
        
    }
}


//--------------------------------------------------------------
void PlayController::generateMoviesFromMotionsAndActions(vector<string>& motionSequence, Agent* agent, string name, int actionIndex, int length){
   
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, vector<string> >& markDictionary = model.getMarkerDictionary();
    map<string, ofxXMP>& xmp = model.getXMP();
    
    //
   float scale = agent->getDrawSize() / model.getWidth();
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
    
    // let's consider that some of the length will be covered by the next movie during the transition
    // if (direction=="LEFT" || direction=="RIGT")
    //     length = length - agent->getGridSizeX() - 10;
    
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
void PlayController::generateMoviesFromMotionsAndActionsNoCut(vector<string>& motionSequence, Agent* agent, string name, int length){
    
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, vector<string> >& markDictionary = model.getMarkerDictionary();
    map<string, ofxXMP>& xmp = model.getXMP();
    
    //
    float scale = agent->getDrawSize() / model.getWidth();
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
    
    int newActionIndex = lastMovie.agentActionIndex+1;
    
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
        nextMovie.agentActionIndex = newActionIndex;
        
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
    
    // let's consider that some of the length will be covered by the next movie during the transition
    // if (direction=="LEFT" || direction=="RIGT")
    //     length = length - agent->getGridSizeX() - 10;
    
    //calculate the total distance the agent travels for the motion sequence generated here
    //the distance between the first and last movies that are pushed this time
    for (int i=lastMovieIndexBeforeAdd+1;i<movs.size();i++) {
        
        totalDistance += calcMovieDistance(agent, &movs[i], tolower(direction[0]));

//        //find the scaled bounding box from the beginning of the movie
//        ofRectangle startBounding = model.getBoundingAt(movs[i].name, movs[i].startframe);
//        startBounding.position = (startBounding.getPosition() + model.getKeyFrameAt(movs[i].name, movs[i].startframe))*scale;
//        startBounding.scale(scale);
//        
//        //find the scaled bounding box from the end of the movie
//        ofRectangle endBounding = model.getBoundingAt(movs[i].name, movs[i].endframe);
//        endBounding.position = (endBounding.getPosition() + model.getKeyFrameAt(movs[i].name, movs[i].endframe))*scale;
//        endBounding.scale(scale);
//        
//        // get the centers of those bounding boxes to calculate their distances
//        movieSP = startBounding.getCenter();
//        movieEP = endBounding.getCenter();
//        
//        distx =abs(movieEP.x - movieSP.x);
//        disty =abs(movieEP.y - movieSP.y);
//        
//        if (direction=="LEFT" || direction=="RIGT")
//            totalDistance+=distx;
//        if (direction=="DOWN" || direction=="UPPP")
//            totalDistance+=disty;
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
        nextMovie.agentActionIndex = newActionIndex;
        
        agent->push(nextMovie);

        totalDistance += calcMovieDistance(agent, &nextMovie, tolower(direction[0]));
        
//        if (direction=="LEFT" || direction=="RIGT")
//            totalDistance+=distx;
//        
//        if (direction=="DOWN" || direction=="UPPP")
//            totalDistance+=disty;
        
        ofxLogVerbose() << "Repeating the last movie: distance = " << distx << " or " << disty << " total distance = " << totalDistance << endl;
    }
}

//--------------------------------------------------------------
void PlayController::cutMoviesForActionsNormalised(Agent* agent) {
    vector<MovieInfo>& movieSequence = agent->getMovieSequence();
    PlayerModel* model = agent->getPlayerModel();
    float scale = agent->getNormalScale();
    
    // for each action, i.e., each segment in path, cut the last movie to match the length
    
    int lastMovIndex = 0;
    int firstMovIndex = 0;
    
    cout<< "------- start ------" << endl;
    
    for (int a=0; a<agent->actions.size();a++) {
        cout << " ------ action " << a << endl;
        float totalDistance = 0;
        
        char currentActionDirection = agent->actions[a].first;
        float currentActionLength = agent->actions[a].second;
        
        MovieInfo* lastActionMovie;
        
        firstMovIndex = lastMovIndex+1;
        
        for (int i=lastMovIndex+1;i<movieSequence.size() && movieSequence[i].agentActionIndex == a; i++) {
            int d = calcMovieDistanceNormalised(agent,i, &movieSequence[i],currentActionDirection);
            totalDistance +=d;
            cout<< "adding distance of movie " << i << " = " << d << endl;
            cout << "total distance so far = " << totalDistance << endl;
            lastMovIndex = i;
        }
        
        
        lastActionMovie = &movieSequence[lastMovIndex];
        cout << "last action movie is " << lastMovIndex << endl;
            
        
        for (int i=lastMovIndex+1;i<movieSequence.size() && movieSequence[i].agentActionIndex == a+1; i++) {
            string markername = movieSequence[i].markername;
            
            //if (!(ofSplitString(markername, "_")[0] == ofSplitString(markername, "_")[2] &&
            //      ofSplitString(markername, "_")[1] == ofSplitString(markername, "_")[3]))
            {
                int d = calcMovieDistanceNormalised(agent, i, &movieSequence[i],currentActionDirection);
                totalDistance += d;
                cout<< "adding distance of next transition movie " << i <<  " = " << d << endl;
                cout << "total distance so far = " << totalDistance << endl;
            }
        }
            
 
        // Now that we have inserted movies to go over the required distance, we need to cut the last movie to get the exact distance
        
        // subtract the distance travelled in the last movie so that we check it frame-by-frame next
        int d = calcMovieDistanceNormalised(agent, lastMovIndex, lastActionMovie,currentActionDirection);
        totalDistance-= d;
        cout << "last action movie distance = " << d << endl;
        cout << "total distance after subtracting the last action movie = " << totalDistance << endl;
        
        for(int f = lastActionMovie->startframe+1; f <=lastActionMovie->endframe; f++){
            float dist = calcMovieDistanceToFrameNormalised(agent, lastMovIndex, lastActionMovie, currentActionDirection, f);
            if ((totalDistance+dist) >= currentActionLength) { // if one movie is enough to cover the distance, this is the right cut frame
                cout << "dist at frame " << f << " = " << dist << endl;
                ofxLogVerbose() << "Ending the last movie at " << f << " instead of " << lastActionMovie->endframe << endl;
                int oldLength = lastActionMovie->endframe - lastActionMovie->startframe;
                lastActionMovie->endframe=f;
                lastActionMovie->isCut = true;
                // we have pushed this movie before, so we need to fix the sequence frames
                break;
            }
        }
        
        // calc the final distance to check the result
        float finalDistance = 0;
        float finalDistance2 = 0;
        float finalDistance3 = 0;
        for (int i=1;i<movieSequence.size();i++) {
            if (movieSequence[i].agentActionIndex == a) {
                finalDistance += calcMovieDistance(agent, &movieSequence[i], currentActionDirection);
                finalDistance2 += calcMovieDistanceNormalised(agent, i, &movieSequence[i], currentActionDirection);
            }
        }

        agent->rebuildSequenceFrames();
        getPositionsForMovieSequence(agent, agent->getPlayerName());
        agent->normalise();
        
        ofPoint movieSP = agent->getScaledCentreAt(agent->getSequenceFrames()[firstMovIndex]);
        ofPoint movieEP = agent->getScaledCentreAt(agent->getSequenceFrames()[lastMovIndex+1]);
        
        if (currentActionDirection == 'l' || 'r')
            finalDistance3 =  abs(movieSP.x - movieEP.x);
        else if (currentActionDirection == 'u' || 'd')
            finalDistance3 =  abs(movieSP.y - movieEP.y);
        
        cout << "final total distance for action " << a << " = " << finalDistance << " while the required length was = " << currentActionLength << endl;
        cout << "final total distance2 for action " << a << " = " << finalDistance2 << " while the required length was = " << currentActionLength << endl;
        cout << "final total distance3 for action " << a << " = " << finalDistance3 << " while the required length was = " << currentActionLength << endl;
    }
    
}



//--------------------------------------------------------------
float PlayController::calcMovieDistance(Agent* agent, MovieInfo* movie, char dir) {
    PlayerModel* model = agent->getPlayerModel();
    float scale = agent->getNormalScale();

    ofPoint movieSP;
    ofPoint movieEP;
    
    //find the scaled bounding box from the beginning of the movie
    ofRectangle startBounding = model->getBoundingAt(movie->name, movie->startframe);
    startBounding.position = (startBounding.getPosition() + model->getKeyFrameAt(movie->name, movie->startframe))*scale;
    startBounding.scale(scale);
    
    //find the scaled bounding box from the end of the movie
    ofRectangle endBounding = model->getBoundingAt(movie->name, movie->endframe);
    endBounding.position = (endBounding.getPosition() + model->getKeyFrameAt(movie->name, movie->endframe))*scale;
    endBounding.scale(scale);
    
    // get the centers of those bounding boxes to calculate their distances
    movieSP = startBounding.getCenter();
    movieEP = endBounding.getCenter();
    
    if (dir == 'l')
        return abs(movieEP.x - movieSP.x);
    else if (dir == 'r')
        return abs(movieSP.x - movieEP.x);
    else if (dir == 'u')
        return abs(movieEP.y - movieSP.y);
    else if (dir == 'd')
        return abs(movieEP.y - movieSP.y);
}

//--------------------------------------------------------------
float PlayController::calcMovieDistanceNormalised(Agent* agent, int index, MovieInfo* movie, char dir) {
    ofPoint movieSP;
    ofPoint movieEP;
    
    // get the centers of those bounding boxes to calculate their distances
    movieSP = agent->getScaledCentreAt(agent->getSequenceFrames()[index]);
    movieEP = agent->getScaledCentreAt(agent->getSequenceFrames()[index+1]);
    
    if (dir == 'l')
        return (movieSP.x - movieEP.x);
    else if (dir == 'r')
        return (movieEP.x - movieSP.x);
    else if (dir == 'u')
        return (movieSP.y - movieEP.y);
    else if (dir == 'd')
        return (movieEP.y - movieSP.y);
}

//--------------------------------------------------------------
float PlayController::calcMovieDistanceToFrame(Agent* agent, MovieInfo* movie, char dir, int frame) {
    PlayerModel* model = agent->getPlayerModel();
    float scale = agent->getNormalScale();
    
    ofPoint movieSP;
    ofPoint movieEP;
    
    //find the scaled bounding box from the beginning of the movie
    ofRectangle startBounding = model->getBoundingAt(movie->name, movie->startframe);
    startBounding.position = (startBounding.getPosition() + model->getKeyFrameAt(movie->name, movie->startframe))*scale;
    startBounding.scale(scale);
    
    //find the scaled bounding box from the end of the movie
    ofRectangle endBounding = model->getBoundingAt(movie->name, frame);
    endBounding.position = (endBounding.getPosition() + model->getKeyFrameAt(movie->name, frame))*scale;
    endBounding.scale(scale);
    
    // get the centers of those bounding boxes to calculate their distances
    movieSP = startBounding.getCenter();
    movieEP = endBounding.getCenter();
    
    
//    if (dir == 'l')
//        return abs(movieEP.x - movieSP.x);
//    else if (dir == 'r')
//        return abs(movieSP.x - movieEP.x);
//    else if (dir == 'u')
//        return abs(movieEP.y - movieSP.y);
//    else if (dir == 'd')
//        return abs(movieEP.y - movieSP.y);
    
    if (dir == 'l')
        return (movieSP.x - movieEP.x);
    else if (dir == 'r')
        return (movieEP.x - movieSP.x);
    else if (dir == 'u')
        return (movieSP.y - movieEP.y);
    else if (dir == 'd')
        return (movieEP.y - movieSP.y);
}

//--------------------------------------------------------------
float PlayController::calcMovieDistanceToFrameNormalised(Agent* agent, int index, MovieInfo* movie, char dir, int frame) {
    PlayerModel* model = agent->getPlayerModel();
    float scale = agent->getNormalScale();
    
    ofPoint movieSP;
    ofPoint movieEP;
    
    // get the centers of those bounding boxes to calculate their distances
    movieSP = agent->getScaledCentreAt(agent->getSequenceFrames()[index]);
    movieEP = agent->getScaledCentreAt(agent->getSequenceFrames()[index]+ frame - movie->startframe);
    
    if (dir == 'l')
        return (movieSP.x - movieEP.x);
    else if (dir == 'r')
        return (movieEP.x - movieSP.x);
    else if (dir == 'u')
        return (movieSP.y - movieEP.y);
    else if (dir == 'd')
        return (movieEP.y - movieSP.y);
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
void PlayController::triggerReplan() {
    int newWindow = 11;
    
    vector<ofRectangle> windows = appModel->getWindows();
    
    ofPoint targetPosition = ofPoint(windows[newWindow].x + windows[newWindow].width / 2.0, windows[newWindow].y, 0.0f);

    
    vector<MovieSequence*>& sequences = appModel->getSequences();
    for(int i = 0; i < sequences.size(); i++){
        Agent* agent = (Agent*)sequences[i];
        
        // store the last action index in the old plan
        int lastActionIndex = agent->actions.size()-1;
        
        agent->actionIndex=0;
        
        ofRectangle worldRect;
        worldRect.x = 0;
        worldRect.y = 0;
        worldRect.width = appModel->getProperty<float>("OutputWidth") + 1* agent->getDrawSize();
        worldRect.height = appModel->getProperty<float>("OutputHeight") + 1* agent->getDrawSize();
        
        agent->setWindow(newWindow); //TODO: change the target window later to allow collision with the old window 
        agent->rePlan(targetPosition, worldRect, windows);
        
        for (int a=0;a<agent->actions.size();a++) {
            cout << "########## action " << agent->actions[a].first << endl;
            insertMoviesFromAction(agent, agent->actions[agent->actionIndex++]);
        }
        
        // ******
        vector<string> motionSequence;
        motionSequence.clear();
        string motion = ofSplitString(agent->getLastMovieInSequence().markername,"_")[0]+"_"+ofSplitString(agent->getLastMovieInSequence().markername,"_")[1];
        
        // randomise SYNCMOTIONS or WAITMOTIONS TODO: make this selectable
        vector<string> vEndMotionType(2);
        vEndMotionType[0] = "SYNCMOTIONS";
        vEndMotionType[1] = "WAITMOTIONS";
        string endMotionType = random(vEndMotionType);
        
        vector<string>& endMotions = appModel->getGraph("EndGraph").getPossibleTransitions(endMotionType);
        string emotion = random(endMotions);
        
        generateMotionsBetween(motion, emotion, agent->getPlayerName(), motionSequence);
        
        generateMoviesFromMotions(motionSequence, agent, agent->getPlayerName());
        getPositionsForMovieSequence(agent, agent->getPlayerName());
        agent->normalise();
        
        
        // calculate target and syncframes
        MovieInfo& lastMovieInSequence = agent->getLastMovieInSequence();
        int goalFrame = agent->getTotalSequenceFrames() - 1;
        int syncFrame = goalFrame;//
        
        agent->setGoalFrame(goalFrame - lastMovieInSequence.endframe - lastMovieInSequence.startframe);
        agent->setSyncFrame(syncFrame);
        
        
        agent->normalise();
        // ******
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