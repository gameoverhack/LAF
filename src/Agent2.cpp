//
//  Agent2.cpp
//  LaughterForgetting
//
//  Created by gameover on 18/04/14.
//
//

#include "Agent2.h"

//--------------------------------------------------------------
Agent2::Agent2(){
    cout << "Creating Agent " << sAgentID << endl;
    
    // init internal state
    MovieSequence::MovieSequence();
    
    agentInfo.bIsAgentLocked = false;
    agentInfo.state = AGENT_INIT;
    agentInfo.behaviourMode = BEHAVIOUR_AUTO;
    agentInfo.collisionMode = COLLISION_AVOID;
    agentInfo.target = ofRectangle(0, 0, 0, 0);
    agentInfo.manualActionTime = ofGetElapsedTimeMillis();
    agentInfo.bActionCollide = false;
    actionBounding = ofRectangle(0, 0, 0, 0);
    
    agentID = sAgentID;
    sAgentID++;
    
    bIsAgentLocked = false;
    deviceID = -1;
    
    //clear();
    
    willCollide = false;

}

//--------------------------------------------------------------
Agent2::~Agent2(){
    cout << "Destroying Agent" << endl;
    //MovieSequence::~MovieSequence();
}

//--------------------------------------------------------------
void Agent2::setModel(PlayerModel _model){
    model = _model;
    
    float scale = drawSize / model.getWidth();
    setNormalScale(scale);
    
    push(model.getFirstMovie());
    getPositionsForMovieSequence(sequence);
    normalise();
}

//--------------------------------------------------------------
void Agent2::setMotionGraph(MotionGraph _forwardGraph, MotionGraph _directionGraph, MotionGraph _endGraph, MotionGraph _targetGraph){
    forwardGraph = _forwardGraph;
    directionGraph = _directionGraph;
    endGraph = _endGraph;
    targetGraph = _targetGraph;
    directionGraph.nestGraph(forwardGraph.getPossibilitie());
}

//--------------------------------------------------------------
void Agent2::setOrigin(ofPoint _origin){
    ofPoint origin = _origin;
    setNormalPosition(origin - getScaledFloorOffsetAt(1));
    normalise();
}

//--------------------------------------------------------------
void Agent2::setWindows(vector<ofRectangle> _windows){
    windows = _windows;
}

//--------------------------------------------------------------
void Agent2::setPlanBoundary(ofRectangle _planBoundary){
    planBoundary = _planBoundary;
}

//--------------------------------------------------------------
void Agent2::setDrawSize(float _drawSize){
    drawSize = _drawSize;
}

//--------------------------------------------------------------
void Agent2::setGridSize(float _w, float _h){
    gridSizeX = _w;
    gridSizeY = _h;
}

//--------------------------------------------------------------
void Agent2::setWindow(int wTarget){
    windowTargetIndex = wTarget;
}

//--------------------------------------------------------------
int Agent2::getWindow(){
    return windowTargetIndex;
}

//--------------------------------------------------------------
void Agent2::setStartPosSegment(int posSegment){
    startPosSegment = posSegment;
}

//--------------------------------------------------------------
int Agent2::getStartPosSegment(){
    return startPosSegment;
}

//--------------------------------------------------------------
void Agent2::startAgent(){
    startThread(true, false);
}

//--------------------------------------------------------------
void Agent2::stopAgent(){
    waitForThread();
    //stopThread();
    video->stop();
    video->finish();
    clear();
}


//--------------------------------------------------------------
void Agent2::setGoalFrame(int f){
    gframe = f;
}

//--------------------------------------------------------------
int Agent2::getGoalFrame(){
    return gframe;
}

//--------------------------------------------------------------
void Agent2::setSyncFrame(int f){
    sframe = f;
}

//--------------------------------------------------------------
int Agent2::getSyncFrame(){
    return sframe;
}

//--------------------------------------------------------------
void Agent2::update(){
    
    MovieSequence::update();
    
//    for(map<Agent2*, AgentInfo>::iterator it = otherAgentInfo->begin(); it != otherAgentInfo->end(); ++it){
//        AgentInfo& info = it->second;
//        // etc
//    }
    
    if(!isAgentLocked()){

        lockAgent();
        
        agentInfo.agentID = agentID;
        if(sboundings.size() > currentSequenceFrame) agentInfo.currentBounding = sboundings[currentSequenceFrame];
        if(spositions.size() > currentSequenceFrame) agentInfo.currentPosition = spositions[currentSequenceFrame];
        if(scentres.size() > currentSequenceFrame) agentInfo.currentCentre = scentres[currentSequenceFrame];
        agentInfo.bIsAgentLocked = bIsAgentLocked;
        
        if(agentInfo.behaviourMode == BEHAVIOUR_MANUAL && agentInfo.currentBounding != ofRectangle(0,0,0,0)){
            ofRectangle r = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());
            if(!r.intersects(agentInfo.currentBounding)){
                cout << "Agent is gone walkies! " << agentInfo.currentBounding << endl;
                bSequenceIsDone = true;
            }
        }
        
        unlockAgent();
        
    }
    
}

//--------------------------------------------------------------
AgentInfo Agent2::getCurrentAgentInfo(){
    ofScopedLock lock(mMutex);
    return agentInfo;
}

//--------------------------------------------------------------
void Agent2::setCollisionMode(CollisionMode _collisionMode){
    lockAgent();
    {
        agentInfo.collisionMode = _collisionMode;
    }
    unlockAgent();
}

//--------------------------------------------------------------
void Agent2::setBehaviourMode(BehaviourMode _behaviourMode){
    lockAgent();
    {
        
        if(_behaviourMode == BEHAVIOUR_MANUAL){
            agentInfo.target = ofRectangle(0, 0, 0, 0);
            currentPath.clear();
            correctedPath.clear();
            sequence[CLAMP(currentSequenceIndex, 0, sequence.size() - 1)].isLoopedStatic = true;
            if(agentInfo.behaviourMode == BEHAVIOUR_AUTO && agentInfo.state != AGENT_INIT){
                unlockAgent(); // weird but true cos removeMovies is locked...
                removeMovies(true);
                lockAgent();
                sequence[0].isLoopedStatic = true;
                cout << "WARNING!!!!!!!!!!!!!!!!!!!! I think this is buggy" << endl;
            }
            MovieSequence::setAutoSequenceStop(false);
        }else{
            
            MovieSequence::setAutoSequenceStop(true);
            for(int i = 0; i < sequence.size(); i++){
                sequence[i].isLooped = false;
                sequence[i].isLoopedStatic = false;
            }
        }
        agentInfo.behaviourMode = _behaviourMode;
    }
    unlockAgent();
}


//--------------------------------------------------------------
void Agent2::setWorldObstacles(vector<ofRectangle> _obstacles){
    lockAgent();
    {
        obstacles = _obstacles;
    }
    unlockAgent();
}

//--------------------------------------------------------------
void Agent2::setOtherAgents(map<Agent2*, AgentInfo>* _otherAgentInfo){
    lockAgent();
    {
        otherAgentInfo = _otherAgentInfo;
    }
    unlockAgent();
}

//--------------------------------------------------------------
void Agent2::setIgnoreTarget(ofRectangle _ignoreTarget){
    agentInfo.ignoreTarget = _ignoreTarget;
}

//--------------------------------------------------------------
void Agent2::move(char _direction){
    lockAgent();
    {
        if(agentInfo.behaviourMode == BEHAVIOUR_AUTO){
            cout << "CALLED MOVE - Aborting as we're auto agent" << endl;
        }else{
            if(agentInfo.state != AGENT_MOVE){
                cout << "CALLED MOVE" << endl;
                agentInfo.direction = _direction;
                agentInfo.state = AGENT_MOVE;
            }
        }
    }
    unlockAgent();
}

//--------------------------------------------------------------
void Agent2::_move(){
    
    //video->finish();
    //update();
    
    // get last direction
    string lastMarkerOfSequence = getLastMovieInSequence().markername;
    string lastMarkerAtCurrent = currentMovie.markername;
    string lastEndMotionOfSequence = model.getEndMotionFromString(lastMarkerOfSequence);
    string lastEndMotionOfCurrent = model.getEndMotionFromString(lastMarkerAtCurrent);
    
    char lastDirectionOfSequence = model.getDirectionFromString(lastMarkerOfSequence);
    char lastDirectionOfCurrent = model.getDirectionFromString(lastMarkerAtCurrent);
    
    cout << "Moving to " << agentInfo.direction << " from either " << lastDirectionOfSequence << " or " << lastDirectionOfCurrent << " for last in sequence " << lastMarkerOfSequence << " and last current " << lastMarkerAtCurrent << endl;
    
    
    // if we're going in the same direction then don't bother cutting frames
    bool bSameDirection = false;
    if (agentInfo.direction == lastDirectionOfSequence) bSameDirection = true;
    
    // create a sequence of motions
    vector<string> motionSequence;
    vector<string> transitions;
    
    switch (agentInfo.direction) {
        case 'l':
        {
            // get the possible approach motions for going left
            transitions = directionGraph.getPossibleTransitions("LEFT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CLIM_LEFT");
            eraseAll(transitions, (string)"CRCH_LEFT");
            eraseAll(transitions, (string)"STND_LEFT");
            
            // remove transitions for specific players when we don't have the matching movies
            if(model.getPlayerName() == "CARAS") eraseAll(transitions, (string)"TRAV_LEFT");
        }
            break;
        case 'r':
        {
            // get the possible approach motions for going right
            transitions = directionGraph.getPossibleTransitions("RIGHT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_RIGT");
            eraseAll(transitions, (string)"CRCH_RIGT");
            eraseAll(transitions, (string)"STND_RIGT");
            
            // remove transitions for specific players when we don't have the matching movies
            if(model.getPlayerName() == "CARAS") eraseAll(transitions, (string)"TRAV_RIGT");
        }
            break;
        case 'u':
        {
            // get the possible approach motions for going up
            transitions = directionGraph.getPossibleTransitions("UP");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"STND_BACK");
        }
            break;
        case 'd':
        {
            // get the possible approach motions for going down
            transitions = directionGraph.getPossibleTransitions("DOWN");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CRCH_FRNT");
        }
            break;
        default:
            cout << "Invalid direction 1" << endl;
            assert(false);
            //return;
            break;
    }
    
    string lastMotion;
    // get last and generated random motions
    if (!bSameDirection) lastMotion = lastEndMotionOfCurrent;
    else lastMotion = lastEndMotionOfSequence;
    
    string motion = transitions[(int)ofRandom(transitions.size())];
    
    cout << "Generating motionSequence using " << lastMotion << " to " << motion << endl;
    
    if (motion != "") {
        generateMotionsBetween(lastMotion, motion, motionSequence);
        motionSequence.push_back(motion);
    }else{
        cout << "No motion" << endl;
        assert(false);
    }
    
    //for (int u = 0; u < motionSequence.size(); u++) cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;
    
    int boundingSequenceIndex = motionSequence.size() - 1;
    string lastMotionInNewSequence = motionSequence[boundingSequenceIndex];
    string reverseMotionInNewSequence = model.getReverseMotionFromString(lastMotionInNewSequence);
    
    generateMotionsBetween(lastMotionInNewSequence, reverseMotionInNewSequence, motionSequence);
    
    for (int u = 0; u < motionSequence.size(); u++) cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;
    
    cout << "Check motion sequence for collisions" << endl;
    
    // Create a vector to store the movies generated for this specific action befor pushing them to the agent
    MovieSequence ms;
    
    generateMoviesFromMotions(motionSequence, &ms);
    getPositionsForMovieSequence(ms.getMovieSequence());
    
    
    ms.setNormalScale(scale);
    if(!bSameDirection){
        if (model.isLoopMarker(lastMarkerAtCurrent))
            ms.setNormalPosition(getScaledPosition());
        else
            ms.setNormalPosition(getScaledPositionAt(sequenceFrames[currentSequenceIndex+1]));
    }else{
        ms.setNormalPosition(getScaledPositionAt(getTotalSequenceFrames()));
    }
    
    ms.normalise();
    
    actionBounding =  ms.getScaledTotalBounding();
    agentInfo.bActionCollide = false;
    // if the bounding intersects with windows, then do not push the sequence in; return from the function
    // TODO: this limits the movements
    for (int w = 0; w < obstacles.size(); w++){
        cout << "We colliding with you>? " << w << " " << obstacles[w] << " " << actionBounding << endl;
        if (obstacles[w] != agentInfo.ignoreTarget && actionBounding.intersects(obstacles[w])){
            cout << "We collide!!!" << endl;
            agentInfo.bActionCollide = true;
            break;
        }
    }
    
    agentInfo.manualActionTime = ofGetElapsedTimeMillis();
    
    if(agentInfo.bActionCollide){
        cout << "Bounding box collides with windows" << endl;
        agentInfo.state = AGENT_RUN;
        return;
    }else{
        cout << "No Collision" << endl;
        for(int i = 0; i < sequence.size(); i++){
            sequence[i].isLooped = false;
            sequence[i].isLoopedStatic = false;
        }
        setSpeed(3);
//        update();
//        video->finish();
    }
    
    if(!bSameDirection){
        cout << "Cut movies: " << string(model.isLoopMarker(lastMarkerAtCurrent) ? " cut frames" : " no cut frames") << endl;
        cutSequenceFromCurrentMovie(model.isLoopMarker(lastMarkerAtCurrent));
    }
    
    
    
    cout << "push movies" << endl;
    // if it does not intersect, then push the new movie sequence for this action to the agent
    for (int i = 0; i < boundingSequenceIndex; i++){
        cout << "pushing " << ms.getMovieSequence()[i].markername << endl;
        push(ms.getMovieSequence()[i]);
    }
    
    removeMovies(false);
    
//    getPositionsForMovieSequence(sequence);
//    normalise();
    
    agentInfo.state = AGENT_RUN;
}

//--------------------------------------------------------------
void Agent2::plan(ofRectangle _target, int _numSequenceRetries){
    lockAgent();
    {
        numSequenceRetries = _numSequenceRetries;
        agentInfo.target = _target;
        if(agentInfo.behaviourMode == BEHAVIOUR_AUTO){
            cout << "CALLED PLAN" << endl;
            agentInfo.state = AGENT_PLAN;
        }else if(agentInfo.behaviourMode == BEHAVIOUR_VANILLA){
            cout << "CALLED PLAN VANILLA" << endl;
            agentInfo.state = AGENT_VANILLA;
        }else{
            cout << "CALLED PLAN - Aborting as we're manual agent!" << endl;
        }
        
    }
    unlockAgent();
}

//--------------------------------------------------------------
bool Agent2::checkPath(ofRectangle tTarget){
    
    // for now only windows are valid targets
    int window = -1;
    for(int i = 0; i < windows.size(); i++){
        if(windows[i] == agentInfo.target){
            window = i;
            break;
        }
    }
    
    if(window != -1) return false;
    windowTargetIndex = window;
    
    //removeMovies(true);
    
    cout << "CheckPlanA " << ofGetElapsedTimeMillis() << endl;
    
    ofPoint startPosition;
    if(currentSequenceIndex <= 0){ // ie., we haven't started the agent/moviesequence playing
        startPosition = getScaledFloorOffsetAt(1);
    }else{
        startPosition = getScaledFloorOffsetAt(sequenceFrames[currentSequenceIndex+1]);
    }
    
    
    // get a start point...do we need to lock?
    //ofPoint startPosition = getScaledFloorOffsetAt(1);
    ofPoint targetPosition = ofPoint(tTarget.x + tTarget.width / 2.0, tTarget.y, 0.0f);
    
    /////////////
    
    PathPlanner pp;
    
    pp.gridScaleX = gridSizeX;
    pp.gridScaleY = gridSizeY;
    
    pp.worldRect = planBoundary;
    
    vector<ofRectangle> tObstacles;
    for(int i = 0; i < obstacles.size(); i++){
        if(!agentInfo.target.intersects(obstacles[i])){ // is this test ok!!!
            tObstacles.push_back(obstacles[i]);
        }
    }
    
    pp.obstacles = tObstacles;
    
    // set the size of the area that the agent's bounding box can grow used in path finding to avoid possible collisions
    pp.obstAvoidBoundingW = drawSize / 2.0;
    pp.obstAvoidBoundingH = drawSize / 2.0;
    
    // find the paths using A*.
    cout << "Finding a path from (" << startPosition.x << "," << startPosition.y  << ") to  (" << targetPosition.x << "," << targetPosition.y << ")"  << endl;
    
    vector< vector< ofPoint > > paths = pp.findPaths(startPosition, targetPosition);
    
    if (paths.size() > 0){
        currentPath = paths[0];
        actions =  pp.getDirectionsInPath(paths[0]);
        cout << "CheckPlanB " << ofGetElapsedTimeMillis() << endl;
        return true;
    }else{
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!No path found for me !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
        //assert(false); // oh oh!
//        bFaultyMovieSequence = true;
//        agentInfo.state = AGENT_RUN;
        cout << "CheckPlanC " << ofGetElapsedTimeMillis() << endl;
        return false;
    }
}

//--------------------------------------------------------------
void Agent2::_plan(){
    
    cout << "START PLANNING" << endl;
    
    removeMovies(true);
    resetActionIndexes(); //???
    
    ofPoint startPosition = getScaledFloorOffsetAt(1);
    ofPoint targetPosition = ofPoint(agentInfo.target.x + agentInfo.target.width / 2.0, agentInfo.target.y, 0.0f);
    
    /////////
    
    // Make the start positon of the path to the start position of the agent
    // The target position is aligned
    
    float xOffset = 0;
    float yOffset = 0;
    
    if (actions.size() > 0) {
        
        xOffset =  currentPath[0].x - startPosition.x;
        yOffset =  currentPath[0].y - startPosition.y;
        
        if (actions[0].first == 'l') actions[0].second -=xOffset;
        if (actions[0].first == 'r') actions[0].second +=xOffset;
        if (actions[0].first == 'u') actions[0].second -=yOffset;
        if (actions[0].first == 'd') actions[0].second +=yOffset;
    }
    
    if (actions.size() > 1) {
        
        if (actions[1].first == 'l') actions[1].second -=xOffset;
        if (actions[1].first == 'r') actions[1].second +=xOffset;
        if (actions[1].first == 'u') actions[1].second -=yOffset;
        if (actions[1].first == 'd') actions[1].second +=yOffset;
    }
    
    
    correctedPath.clear();
    
    int index;
    for (int i=0; i < sequence.size(); i++){
        if (sequence[i].agentActionIndex == 0) {
            index = i;
            break;
        }
    }
    
    ofPoint point = startPosition;
    correctedPath.addVertex(point);
    
    for (int a = 0; a < actions.size(); a++) {
        if (actions[a].first == 'l')
            point.x -= actions[a].second;
        else if (actions[a].first == 'r')
            point.x += actions[a].second;
        else if (actions[a].first == 'u')
            point.y -= actions[a].second;
        else if (actions[a].first == 'd')
            point.y += actions[a].second;
        
        correctedPath.addVertex(point);
    }

    cout << "STARTS   aF " << currentPath[0] << " == " << startPosition << endl;
    cout << "CURRENT  aF " << currentPath[currentPath.size() - 1] << " == " << targetPosition << endl;
    cout << "CORRECT  aF " << correctedPath[correctedPath.size() - 1] << " == " << targetPosition << endl;
    cout << "ACTION   aF " << actions[actions.size() - 1].first << " valid? " << actions[actions.size() - 1].second << " SIZE: " << actions.size() << endl;


    bool solved = false;

    for (int t = 0; t < numSequenceRetries && !solved; t++ ) {
        // Remove the rest of the movies in the sequence as we are overwriting them
        //removeAllMovies();
        
        if(currentSequenceIndex <= 0){
            removeMoviesFromIndex(1);
        }else{
            removeMoviesFromIndex(currentSequenceIndex + 1);
        }
        
        for(int i = 0; i < sequence.size(); i++){
            sequence[i].agentActionIndex = -1;
        }
        
        getPositionsForMovieSequence(sequence);
        normalise();
        bFaultyMovieSequence = false;
        
        // Now that the agent has a set of actions, let's insert movies for them
        for (int a=0; a < actions.size(); a++) {
            cout << "########## action " << actions[a].first << endl;
            insertMoviesFromAction(actions[a]);
        }

        insertEndMotion();

        //agent->storeSBoundings();
        
        // Make sure the moveis for each action travel the exact lenth
        solved = cutMoviesForActionsNormalised();
        
        cout << "<><><><><><><<><><><><><><><><><> at try "  << t << " the path is " << solved << endl;
    }
    
    bFaultyMovieSequence = !solved;
    
    //------
//    int goalFrame = getTotalSequenceFrames() - 1;
//    ofPoint floorOffset = getScaledFloorOffsetAt(1);
//    ofPoint tp = ofPoint(target.x + target.width / 2.0, target.y, 0.0f);
//    ofPoint finalSequencePosition = tp - getScaledPositionAt(goalFrame) - floorOffset;
//    setNormalPosition(finalSequencePosition);
    
       // normalise();
    
    //------
    
    //play();
    
    agentInfo.state = AGENT_RUN;
    
    cout << "PLANNING END" << endl;
}

//--------------------------------------------------------------
AgentInfo Agent2::getAgentInfo(){
    ofScopedLock lock(mMutex);
    return agentInfo;
}

//--------------------------------------------------------------
void Agent2::threadedFunction(){
    
    while(isThreadRunning()){
        
        // check if agent is locked
        
        if(!isAgentLocked()){
            
            lockAgentFlag();
            
            switch (agentInfo.state) {
                case AGENT_INIT:
                    agentInfo.state = AGENT_RUN;
                    break;
                    
                case AGENT_RUN:
                    // nothing for now
                    break;
                    
                case AGENT_PLAN:
                    _plan();
                    break;
                    
                case AGENT_VANILLA:
                    _planVanilla();
                    break;
                    
                case AGENT_MOVE:
                    _move();
                    break;
            }
            
            unlockAgentFlag();
        }
        
        ofSleepMillis(1);
        
    }
    
}

//--------------------------------------------------------------
bool Agent2::isAgentLocked(){
    ofScopedLock lock(mMutex);
    return bIsAgentLocked;
}

//--------------------------------------------------------------
void Agent2::lockAgentFlag(){
    mMutex.lock();
    bIsAgentLocked = true;
    mMutex.unlock();
}

void Agent2::unlockAgentFlag(){
    mMutex.lock();
    bIsAgentLocked = false;
    mMutex.unlock();
}

//--------------------------------------------------------------
void Agent2::lockAgent(){
    if(isThreadRunning()){
        mMutex.lock();
        bIsAgentLocked = true;
    }
}

void Agent2::unlockAgent(){
    if(isThreadRunning()){
        bIsAgentLocked = false;
        mMutex.unlock();
    }
}

//--------------------------------------------------------------
void Agent2::cutSequenceFromCurrentMovie(bool cutFromCurrentFrame) {
    // Remove the rest of the movies in the sequence as we are overwriting them
    if (currentSequenceIndex < sequence.size() - 1){
        removeMoviesFromIndex(currentSequenceIndex + 1);
    }
    
    if (!cutFromCurrentFrame){
        rebuildSequenceFrames();
        return;
    }
    
    
    // Cut the current movie and start the new movie at the current position
    string lastMotion = ofSplitString(currentMovie.markername,"_")[0] + "_" + ofSplitString(currentMovie.markername,"_")[1];
    int oldLength = currentMovie.endframe - currentMovie.startframe;
    
    // find the next or last marker and cut at that point
    
    // get the players model
    map<string, ofxXMP>& xmp = model.getXMP();
    
    ofxXMPMarker lastMarker = xmp[currentMovie.name].getLastMarker(currentMovie.frame);
    ofxXMPMarker nextMarker = xmp[currentMovie.name].getNextMarker(currentMovie.frame);
    
    
    cout << "lastmarker name = " << lastMarker.getName() << endl;
    cout << "lastmarker st frame =  " << lastMarker.getStartFrame() << endl;
    cout << "nextmarker st frame =  " << nextMarker.getStartFrame() << endl;
    cout << "cframe = " << currentMovie.frame << endl;
    
    
    int newEndFrame = currentMovie.startframe + currentMovie.frame;
    //int newEndFrame = currentMovie->startframe + nextMarker.getStartFrame();
    
    if(currentMovie.startframe == newEndFrame){
        cout << "Adding a frame when cutting!!!! " << currentMovie << endl;
        newEndFrame++;
    }
    currentMovie.endframe = newEndFrame;
    sequence[currentSequenceIndex].endframe = newEndFrame;
    
    
    
    // we have pushed this movie before, so we need to fix the sequence frame
    rebuildSequenceFrames();
}

//--------------------------------------------------------------
void Agent2::removeMovies(bool bAllMovies){
    
    lockAgent();
    
    if(bAllMovies){
        cout << "Remove all movies" << endl;
    }else{
        cout << "Remove previous movies" << endl;
    }

    //currentPath.clear();
    
    vector<MovieInfo> sequencecopy = sequence;
    
    ofPoint tCurrentPosition = getScaledPositionAt(sequenceFrames[currentSequenceIndex]);
    int tCurrentVideoFrame = video->getCurrentFrame();
    int tCurrentSequenceIndex = currentSequenceIndex;
    int tCurrentSpeed = speed;
    float tCurrentScale = scale;
    bool tCurrentAutoStop = bAutoSequenceStop;
    
    MovieSequence::clear();
    
    for(int i = tCurrentSequenceIndex; i < sequencecopy.size(); i++){
        push(sequencecopy[i]);
        if(bAllMovies && model.isLoopMarker(sequencecopy[i].markername)) break;
    }
    
    setNormalPosition(tCurrentPosition);
    setNormalScale(tCurrentScale);
    
    cout << "get positions" << endl;
    getPositionsForMovieSequence(sequence);
    normalise();
    
    bAutoSequenceStop = tCurrentAutoStop;
    bPaused = false;
    currentSequenceIndex = 0;
    currentMovie = sequence[0];
    setSpeed(tCurrentSpeed);
    updateFrame();
    updatePosition();
    
    unlockAgent();
    
}

//--------------------------------------------------------------
bool Agent2::cutMoviesForActionsNormalised(){
    
    bool validSolution = true;
    
    
    cout << " ----- starting trimming the movies to match the action length -----" << endl;
    
    for (int a = 0; a < actions.size(); a++) {
        char actionDirection = actions[a].first;
        float actionLength = actions[a].second;
        
        cout << ">> Trimming action " << a << " | going in direction " << actionDirection << " for " << actionLength << " pixels." << endl;
        
        int actionFirstMovieIndex = -1;
        int actionLastMovieIndex = 0;
        
        int nextActionLoopMarkerMovieIndex = -1;
        int nextActionLastMovieIndex = 0;
        
        // find the indices of the movies for this action and the next action (if any)
        for (int i = 0; i < sequence.size(); i++) {
            if (sequence[i].agentActionIndex == a && actionFirstMovieIndex == -1) actionFirstMovieIndex = i;
            if (sequence[i].agentActionIndex == a) actionLastMovieIndex = i;
            
            if (sequence[i].agentActionIndex == a+1 && model.isLoopMarker(sequence[i].markername)) nextActionLoopMarkerMovieIndex = i;
            if (sequence[i].agentActionIndex == a+1) nextActionLastMovieIndex = i;
        }
        
        if (nextActionLoopMarkerMovieIndex < actionLastMovieIndex) { // there is no other movie after this action
            cout << "could not find any loopmarker movie for next action. using the last movie instead." << endl;
            nextActionLoopMarkerMovieIndex = sequence.size()-1;
        }
        
        cout << sequence << endl;
        cout << "actionFirstMovieIndex = " << actionFirstMovieIndex << endl;
        cout << "actionLastMovieIndex = " << actionLastMovieIndex << endl;
        cout << "nextActionLoopMarkerMovieIndex = " << nextActionLoopMarkerMovieIndex << endl;
        cout << "nextActionLastMovieIndex = " << nextActionLastMovieIndex << endl;
        
        
        // --- correct the length based on reality
        
        
        ofPoint actionStartPosition = getScaledFloorOffsetAt(sequenceFrames[actionFirstMovieIndex]+2); //TODO: BUG? sometimes it returns strange numbers for one dimension of the point
        ofPoint pathSegmentEndPosition = getCorrectedPath()[a+1];
        
        cout << "going from " << actionStartPosition << " to " << pathSegmentEndPosition << endl;
        
        float realNeededLength = 0;
        
        if (actionDirection == 'r' || actionDirection == 'l')
            realNeededLength = abs(actionStartPosition.x - pathSegmentEndPosition.x);
        if (actionDirection == 'u' || actionDirection == 'd')
            realNeededLength = abs(actionStartPosition.y - pathSegmentEndPosition.y);
        
        cout << "real length = " << realNeededLength << " vs path's action length = " << actionLength << endl;
        //---
        
        
        ///// Calculating the initial total distance from movies in the sequence
        
        float totalDistance = calculateMovieDistanceNormalised(actionFirstMovieIndex, nextActionLoopMarkerMovieIndex, actionDirection, 0); // if the nextActionLoopMarkerMovie drifts in this direction, once we insert more of that later, it may corrupt these calculations. SOLOTUIN: maybe start from the end!
        cout << "total distance from first movie to next actions loopmarker movie before insertions = " << totalDistance  << endl;
        
        
        
        ///// Calculate the distance during the transition movies
        float actionTransitionDistance = calculateMovieDistanceNormalised(actionFirstMovieIndex, actionLastMovieIndex, actionDirection, 0);
        cout << "The distance moved during the transition movies for this action is " << actionTransitionDistance << endl;
       
        //assert (actionTransitionDistance < realNeededLength);
        if(actionTransitionDistance < realNeededLength) validSolution = false;
        
        
        ///// Calculate the distance during the transition movies for the next action
        float nextActionTransitionDistance = calculateMovieDistanceNormalised(actionLastMovieIndex+1, nextActionLastMovieIndex, actionDirection, 0);
        cout << "The distance moved during the transition movies for next action is " << nextActionTransitionDistance << endl;
        
        
        ///// insert enough movies to cover the distance
        
        
        
        MovieInfo nextMovie;
        nextMovie.name = sequence[actionLastMovieIndex].name;
        nextMovie.path = sequence[actionLastMovieIndex].path;
        nextMovie.startframe = sequence[actionLastMovieIndex].startframe;
        nextMovie.endframe = sequence[actionLastMovieIndex].endframe;
        nextMovie.speed = sequence[actionLastMovieIndex].speed;
        nextMovie.markername = sequence[actionLastMovieIndex].markername;
        nextMovie.agentActionIndex = a;
        
        int inserts = 0;
        
        while (totalDistance < realNeededLength) {
            inserts++;
            cout << "WTF" << endl;
            pushAt(nextMovie, actionLastMovieIndex + inserts);
            
            getPositionsForMovieSequence(sequence);
            normalise();
            
            totalDistance = calculateMovieDistanceNormalised(actionFirstMovieIndex, inserts + nextActionLoopMarkerMovieIndex, actionDirection, 0);
            
            cout << "Repeating the last movie for the " << inserts << " time | total distance = " << totalDistance << endl;
        }

        
        /////
        // update the indices of the movies for this action and the next action (if any) after insertions
        for (int i = 0; i < sequence.size(); i++) {
            if (sequence[i].agentActionIndex == a && actionFirstMovieIndex == -1) actionFirstMovieIndex = i;
            if (sequence[i].agentActionIndex == a) actionLastMovieIndex = i;
            
            if (sequence[i].agentActionIndex == a+1 && model.isLoopMarker(sequence[i].markername)) nextActionLoopMarkerMovieIndex = i;
            if (sequence[i].agentActionIndex == a+1) nextActionLastMovieIndex = i;
        }
        
        if (nextActionLoopMarkerMovieIndex < actionLastMovieIndex) { // there is no other movie after this action
            cout << "could not find any loopmarker movie for next action. using the last movie instead." << endl;
            nextActionLoopMarkerMovieIndex = sequence.size()-1;
        }
        
        cout << sequence << endl;
        cout << "actionFirstMovieIndex = " << actionFirstMovieIndex << endl;
        cout << "actionLastMovieIndex = " << actionLastMovieIndex << endl;
        cout << "nextActionLoopMarkerMovieIndex = " << nextActionLoopMarkerMovieIndex << endl;
        cout << "nextActionLastMovieIndex = " << nextActionLastMovieIndex << endl;
        /////
        
        
        
        ///// Now it is time to cut the last movie of this action (the loopmarker movie) to fit the real needed length
        
        MovieInfo* actionLastMovie = &sequence[actionLastMovieIndex];
        ofPoint cutPosition;
        float diff = 0;
        for (int frame = 1; frame < actionLastMovie->endframe - actionLastMovie->startframe; frame++) {
            cutPosition = getScaledFloorOffsetAt(sequenceFrames[actionLastMovieIndex]+frame);
            
            if (actionDirection == 'r' || actionDirection == 'l')
                diff = abs(cutPosition.x - pathSegmentEndPosition.x);
            if (actionDirection == 'u' || actionDirection == 'd')
                diff = abs(cutPosition.y - pathSegmentEndPosition.y);
//            cout << " diff = " << diff << endl;
//            cout << " diff - tr = " << diff - nextActionTransitionDistance << endl;
            
            float dist = nextActionTransitionDistance + calculateMovieDistanceNormalised(actionFirstMovieIndex, actionLastMovieIndex, actionDirection, frame);
//            cout << " dist = " << dist << endl;
            //if (diff - nextActionTransitionDistance < 3) {
            if (dist >= realNeededLength) {
                cout << "Ending the last movie at frame " << frame << " instead of " << actionLastMovie->endframe  << " the distance is " << dist << endl;
                actionLastMovie->endframe=actionLastMovie->startframe + frame;
                actionLastMovie->isCut = true;
                break;
            }
        }
        
        ///// After trimming, need to re-calculate the sequence frames, positions, and do normalise
        
        rebuildSequenceFrames();
        getPositionsForMovieSequence(sequence);
        normalise();
        
        ///// See how we did
        
        cout << " the end frame is " << sequence[actionLastMovieIndex].endframe << endl;
        
        float finalTotalDistance = nextActionTransitionDistance + calculateMovieDistanceNormalised(actionFirstMovieIndex, nextActionLoopMarkerMovieIndex, actionDirection, -1);
        cout << "The total distance after trimming is " << finalTotalDistance << " while the needed distance was " << realNeededLength  << endl;
        
       // assert(abs(finalTotalDistance - realNeededLength) < 5);
        if (abs(finalTotalDistance - realNeededLength) < 5) validSolution = false;
        
        cout << abs(finalTotalDistance - realNeededLength);
    }
    
    
    
    return validSolution;
}

//--------------------------------------------------------------
float Agent2::calculateMovieDistanceNormalised(int indexA, int indexB, char dir, int frameOffset) {
    
    ofPoint movieSP = getScaledFloorOffsetAt(getSequenceFrames()[indexA] + 1);
    ofPoint movieEP = getScaledFloorOffsetAt(getSequenceFrames()[indexB] + frameOffset + 1);
    
    if (dir == 'l')
        return abs(movieSP.x - movieEP.x);
    else if (dir == 'r')
        return abs(movieEP.x - movieSP.x);
    else if (dir == 'u')
        return abs(movieSP.y - movieEP.y);
    else if (dir == 'd')
        return abs(movieEP.y - movieSP.y);
}

//--------------------------------------------------------------
void Agent2::insertMoviesFromAction(pair<char,float> act) {
    
    char op = act.first;
    float length = act.second;
    
    cout << "Performing action " << op << " for " << model.getPlayerName() << endl;
    
    
    MovieInfo lastMovie = getLastMovieInSequence();
    
    string lastMotion = ofSplitString(lastMovie.markername,"_")[0] + "_" + ofSplitString(lastMovie.markername,"_")[1];
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    switch (op) {
        case 'l':
        {
            
            vector<string> transitions = directionGraph.getPossibleTransitions("LEFT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CLIM_LEFT");
            eraseAll(transitions, (string)"CRCH_LEFT");
            eraseAll(transitions, (string)"STND_LEFT");
            
            // remove transitions for specific players when we don't have the matching movies
            if(model.getPlayerName() == "CARAS") eraseAll(transitions, (string)"TRAV_LEFT");
            
            // get the first possible transition to the next action to left
            string motion = transitions[(int)ofRandom(transitions.size())];
            if (motion != "") {
                generateMotionsBetween(lastMotion, motion, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            
            break;
        case 'r':
        {
            
            vector<string> transitions = directionGraph.getPossibleTransitions("RIGHT");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_RIGT");
            eraseAll(transitions, (string)"CRCH_RIGT");
            eraseAll(transitions, (string)"STND_RIGT");
            
            // remove transitions for specific players when we don't have the matching movies
            if(model.getPlayerName() == "CARAS") eraseAll(transitions, (string)"TRAV_RIGT");
            
            // get the first possible transition to the next action to right
            string motion = transitions[(int)ofRandom(transitions.size())];
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            break;
        case 'u':
        {
            
            vector<string> transitions = directionGraph.getPossibleTransitions("UP");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_UPRT");
            eraseAll(transitions, (string)"CLIM_UPLF");
            eraseAll(transitions, (string)"STND_BACK");
            
            // get the first possible transition to the next action to up
            string motion = transitions[(int)ofRandom(transitions.size())];
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            break;
        case 'd':
        {
            
            vector<string> transitions = directionGraph.getPossibleTransitions("DOWN");
            
            // removes the movies that do not exist
            eraseAll(transitions, (string)"CLIM_DNRT");
            eraseAll(transitions, (string)"CLIM_DNLF");
            eraseAll(transitions, (string)"CRCH_FRNT");
            
            // get the first possible transition to the next action to down
            string motion = transitions[(int)ofRandom(transitions.size())];
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, motionSequence);
                motionSequence.push_back(motion);
            }
            
        }
            break;
        default:
            break;
    }
    
    for (int u=0; u<motionSequence.size();u++) cout << " >>>>>>>>>>>> " << motionSequence[u] << endl;
    
    generateMoviesFromMotions(motionSequence, this);
    getPositionsForMovieSequence(sequence);
    normalise();
}

//--------------------------------------------------------------
void Agent2::insertEndMotion(){
    
    // Insert the end motions
    vector<string> motionSequence;
    string motion = ofSplitString(getLastMovieInSequence().markername,"_")[0]+"_"+ofSplitString(getLastMovieInSequence().markername,"_")[1];
    
//    // randomise SYNCMOTIONS or WAITMOTIONS TODO: make this selectable
//    vector<string> vEndMotionType(2);
//    vEndMotionType[0] = "SYNCMOTIONS";
//    vEndMotionType[1] = "WAITMOTIONS";
//    string endMotionType = random(vEndMotionType);
//    
//    vector<string>& endMotions = endGraph.getPossibleTransitions(endMotionType);
    string emotion = "FALL_BACK";//random(endMotions);
    
    generateMotionsBetween(motion, emotion, motionSequence);
    
    generateMoviesFromMotions(motionSequence, this);
    getPositionsForMovieSequence(sequence);
    normalise();
}

//--------------------------------------------------------------
void Agent2::generateMoviesFromMotions(vector<string>& motionSequence, MovieSequence* movieSequence){
    
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
    
    cout << "Generating movies for marker sequence: " << markerSequence << endl;
    
    MovieInfo lastMovie = getLastMovieInSequence();
    
    int nextActionIndex = lastMovie.agentActionIndex + 1;
    
    for(int i = 0; i < markerSequence.size(); i++){
        
        cout << "Find movies with: " << markerSequence[i] << endl;
        
        if(i == 0 && markerSequence[i] == lastMovie.markername){
            cout << "...movie already in sequence, continuing" << endl;
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
            
            cout << "Selecting RAND marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << " of " << markers.size() << endl;
            
        }else{
            cout << "Selecting NEXT marker match: " << motionSequence[i] << " == " << pStartMarker.getName() << endl;
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
        
        assert(nextMovie.startframe != nextMovie.endframe);
        
        ostringstream os; os << nextMovie;
        cout << "Adding movie: " << os.str() << endl;
        
        movieSequence->push(nextMovie);
        lastMovie = nextMovie;
        
    }
    
}

//--------------------------------------------------------------
void Agent2::getPositionsForMovieSequence(vector<MovieInfo>& movieSequence){
    
    for(int i = 0; i < movieSequence.size(); i++){
        
        MovieInfo& m = movieSequence[i];
        int totalframes = m.endframe - m.startframe;
        if(m.positions.size() == totalframes && m.boundings.size() == totalframes){
            continue;
        }
        
        cout << "Getting positions and boundings " << endl;
        
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
void Agent2::generateMotionsBetween(string startMotion, string endMotion, vector<string>& motionSequence){
    
    ofSeedRandom();
    
    vector<string> allPossibleTransitions;
    
    cout << "Finding shortest path between:   " << startMotion << " -> " << endMotion << " for player " << model.getPlayerName() << endl;
    
    allPossibleTransitions = forwardGraph.getPossibleTransitions(startMotion);
    
    vector< vector<string> > solutions;
    for(int i = 0; i < allPossibleTransitions.size(); i++){
        if(allPossibleTransitions[i] == endMotion){
            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << endl;
            vector<string> transitions;
            transitions.push_back(startMotion);
            transitions.push_back(allPossibleTransitions[i]);
            solutions.push_back(transitions);
        }else{
            vector<string> allPossibleTransitions2 = forwardGraph.getPossibleTransitions(allPossibleTransitions[i]);
            for(int j = 0; j < allPossibleTransitions2.size(); j++){
                if(allPossibleTransitions2[j] == endMotion){
                    //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << endl;
                    vector<string> transitions;
                    transitions.push_back(startMotion);
                    transitions.push_back(allPossibleTransitions[i]);
                    transitions.push_back(allPossibleTransitions2[j]);
                    solutions.push_back(transitions);
                }else{
                    vector<string> allPossibleTransitions3 = forwardGraph.getPossibleTransitions(allPossibleTransitions2[j]);
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
                            vector<string> allPossibleTransitions4 = forwardGraph.getPossibleTransitions(allPossibleTransitions3[k]);
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
                                    vector<string> allPossibleTransitions5 = forwardGraph.getPossibleTransitions(allPossibleTransitions4[m]);
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
                                            vector<string> allPossibleTransitions6 = forwardGraph.getPossibleTransitions(allPossibleTransitions5[n]);
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
                                                    vector<string> allPossibleTransitions7 = forwardGraph.getPossibleTransitions(allPossibleTransitions6[o]);
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
        
        ofxLogError() << "No solution found for path between: " << startMotion << " -> " << endMotion << " for player " << model.getPlayerName() << endl;
        assert(false);
        
    }else{
        
        cout << "Solution found for path between: " << startMotion << " -> " << endMotion << " for player " << model.getPlayerName() << endl;
        cout << "                                 " << solutions[index] << endl;
        
        for(int i = 0; i < solutions[index].size(); i++){
            motionSequence.push_back(solutions[index][i]);
        }
        
    }
    
}

//--------------------------------------------------------------
void Agent2::_planVanilla(){
    
    int window = -1;
    for(int i = 0; i < windows.size(); i++){
        if(windows[i] == agentInfo.target){
            window = i;
            break;
        }
    }
    
    assert(window != -1);
    windowTargetIndex = window;
    
    cout << "Making sequence for " << model.getPlayerName() << " targeting window " << window << endl;
    
    // get the players model
    map<string, ofxXMP>& xmp = model.getXMP();
    
    // get the possible approach motions for this window
    vector<string> transitions = targetGraph.getPossibleTransitions(ofToString(window));
    
    // CARA's hack -> she doesn't have TRAV_LEFT or TRAV_RIGT
    if(model.getPlayerName() == "CARAS"){
        eraseAll(transitions, (string)"TRAV_LEFT");
        eraseAll(transitions, (string)"TRAV_RIGT");
    }
    
    // randomly get a motion TODO: make this so that we don't have double approaches
    string motion = transitions[(int)ofRandom(transitions.size())];
    
    // split motion into action and direction
    string action = ofSplitString(motion, "_")[0];
    string direction = ofSplitString(motion, "_")[1];
    
    // create a new MovieSequence
    float tScale = scale;
    clear();
    push(model.getFirstMovie());
    setNormalPosition(ofPoint(0,0,0));
    setNormalScale(tScale);
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    // start standing front and go to -> motion
    motionSequence.push_back("STND_FRNT");
    generateMotionsBetween("STND_FRNT", motion, motionSequence);
    motionSequence.push_back(motion);
    
    generateMoviesFromMotions(motionSequence, this);
    getPositionsForMovieSequence(sequence);
    normalise();
    
    // calulate and insert loops of the action to travel far enough to get to the target window
    
    int inserts = 0;
    float target = drawSize;
    MovieInfo loopMovie = getLastMovieInSequence();
    
    if(direction == "LEFT" || direction == "RIGT"){
        if(direction == "RIGT") target += windows[window].x;
        if(direction == "LEFT") target += ofGetWidth() - windows[window].x;
        while (getScaledTotalBounding().width < target) {
            inserts++;
            push(loopMovie);
            normalise();
            cout << direction << " " << getScaledTotalBounding().width << endl;
        }
    }else if(direction == "DOWN" || direction == "UPPP") {
        if(direction == "DOWN") target += windows[window].y;
        if(direction == "UPPP") target = 2 * target + ofGetHeight() - windows[window].y;
        while (getScaledTotalBounding().height < target) {
            inserts++;
            push(loopMovie);
            normalise();
            cout << direction << " " << getScaledTotalBounding().height << endl;
        }
    }
    
    motionSequence.clear();
    
    // randomise SYNCMOTIONS or WAITMOTIONS TODO: make this selectable
    vector<string> vEndMotionType(2);
    vEndMotionType[0] = "SYNCMOTIONS";
    vEndMotionType[1] = "WAITMOTIONS";
    string endMotionType = random(vEndMotionType);
    
    vector<string>& endMotions = endGraph.getPossibleTransitions(endMotionType);
    string emotion = "HUGG_FRNT";//random(endMotions);
    
    generateMotionsBetween(motion, emotion, motionSequence);
    
    generateMoviesFromMotions(motionSequence, this);
    getPositionsForMovieSequence(sequence);
    normalise();
    
    // calculate target and syncframes
    MovieInfo& lastMovieInSequence = getLastMovieInSequence();
    int goalFrame = getTotalSequenceFrames() - 1;
    int syncFrame = goalFrame - lastMovieInSequence.startframe + xmp[lastMovieInSequence.name].getMarker(motionSequence[motionSequence.size() - 1]).getStartFrame();
    
    setGoalFrame(goalFrame - lastMovieInSequence.endframe - lastMovieInSequence.startframe);
    setSyncFrame(syncFrame);
    
    ofPoint floorOffset = getScaledFloorOffset();
    ofPoint targetPosition = ofPoint(windows[window].x + windows[window].width / 2.0, windows[window].y, 0.0f);
    ofPoint finalSequencePosition = targetPosition - getScaledPositionAt(goalFrame) - floorOffset;
    
    if(emotion != "FALL_BACK"){
        
        setHug(true);
        
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
            generateMotionsBetween("CRCH_FRNT", reversemotion, motionSequence);
        }else{
            generateMotionsBetween("STND_FRNT", reversemotion, motionSequence);
        }
        
        for(int i = 0; i < inserts + 2; i++) motionSequence.push_back(reversemotion);
        
        generateMoviesFromMotions(motionSequence, this);
        getPositionsForMovieSequence(sequence);
    }else{
        setHug(false);
    }
    
    setNormalPosition(finalSequencePosition);
    normalise();
    
    cout << "Adding MovieSequence" << getMovieSequenceAsString() << endl;
    cout << "E(nd) Motion: " << endMotionType << " of " << emotion << endl;
    
//    setSpeed(ofRandom(1.0, 3.0));
//    play();
    
    agentInfo.state = AGENT_RUN;
    
}