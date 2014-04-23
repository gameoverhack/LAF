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
void Agent2::setMotionGraph(MotionGraph _forwardGraph, MotionGraph _directionGraph, MotionGraph _endGraph){
    forwardGraph = _forwardGraph;
    directionGraph = _directionGraph;
    endGraph = _endGraph;
    directionGraph.nestGraph(forwardGraph.getPossibilitie());
}

//--------------------------------------------------------------
void Agent2::setOrigin(ofPoint _origin){
    ofPoint origin = _origin;
    setNormalPosition(origin - getScaledFloorOffsetAt(1));
    normalise();
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
    stopThread();
    stop();
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
    
    if(!isAgentLocked()){

        lockAgent();
        
        agentInfo.agentID = agentID;
        if(sboundings.size() > currentSequenceFrame) agentInfo.currentBounding = sboundings[currentSequenceFrame];
        if(spositions.size() > currentSequenceFrame) agentInfo.currentPosition = spositions[currentSequenceFrame];
        if(scentres.size() > currentSequenceFrame) agentInfo.currentCentre = scentres[currentSequenceFrame];
        agentInfo.bIsAgentLocked = bIsAgentLocked;
        
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
        agentInfo.behaviourMode = _behaviourMode;
        if(agentInfo.behaviourMode == BEHAVIOUR_MANUAL){
            if(currentSequenceIndex <= 0){ // ie., we haven't started the agent/moviesequence playing
                sequence[0].isLooped = true;
            }
            MovieSequence::setAutoSequenceStop(false);
        }else{
            MovieSequence::setAutoSequenceStop(true);
        }
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
void Agent2::setOtherAgents(vector<AgentInfo> _otherAgentInfo){
    lockAgent();
    {
        otherAgentInfo = _otherAgentInfo;
    }
    unlockAgent();
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
    bActionCollide = false;
    // if the bounding intersects with windows, then do not push the sequence in; return from the function
    // TODO: this limits the movements
    for (int w = 0; w < obstacles.size(); w++){
        cout << "We colliding with you>? " << w << " " << obstacles[w] << " " << actionBounding << endl;
        if (obstacles[w] != agentInfo.target && actionBounding.intersects(obstacles[w])){
            cout << "We collide!!!" << endl;
            bActionCollide = true;
            break;
        }
    }
    
    if(bActionCollide){
        cout << "Bounding box collides with windows" << endl;
        agentInfo.state = AGENT_RUN;
        return;
    }else{
        cout << "No Collision" << endl;
        currentMovie.isLoopedStatic = false;
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
    
    removePreviousMovies();
    
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
        }else{
            cout << "CALLED PLAN - Aborting as we're manual agent!" << endl;
        }
        
    }
    unlockAgent();
}

//--------------------------------------------------------------
void Agent2::_plan(){
    
    cout << ">>>>>>>>PLANNING START" << endl;
    
    ofPoint startPosition;
    
    if(currentSequenceIndex <= 0){ // ie., we haven't started the agent/moviesequence playing
        startPosition = getScaledFloorOffsetAt(1);
    }else{
        startPosition = getScaledFloorOffsetAt(currentSequenceFrame);
    }

    ofPoint targetPosition = ofPoint(agentInfo.target.x + agentInfo.target.width / 2.0, agentInfo.target.y, 0.0f); // where I'm going
    
    currentMovie.agentActionIndex = -1;
    for(int i = 0; i < sequence.size(); i++){
        sequence[i].agentActionIndex = -1;
    }
    
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
    pp.obstAvoidBoundingW = 2.5 * drawSize / 3.0;
    pp.obstAvoidBoundingH = drawSize;
    
    // find the paths using A*.
    cout << "Finding a path from (" << startPosition.x << "," << startPosition.y  << ") to  (" << targetPosition.x << "," << targetPosition.y << ")"  << endl;
    
    vector< vector< ofPoint > > paths = pp.findPaths(startPosition, targetPosition);
    
    if (paths.size()>0){
        currentPath = paths[0];
        actions =  pp.getDirectionsInPath(paths[0]);
    }else{
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!No path found for me !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"  << endl;
        //assert(false); // oh oh!
    }
    
    /////////
    
    // Make the start positon of the path to the start position of the agent
    // The target position is aligned
    
    float xOffset;
    float yOffset;
    
    
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
    
    
    bool solved = false;
    
    for (int t = 0; t < numSequenceRetries && !solved; t++ ) {
        // Remove the rest of the movies in the sequence as we are overwriting them
        //removeAllMovies();
        
        if(currentSequenceIndex <= 0){
            removeMoviesFromIndex(1);
        }else{
            removeMoviesFromIndex(currentSequenceIndex + 1);
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
    
    cout << "<<<<<<<<<<<PLANNING END" << endl;
}

//--------------------------------------------------------------
AgentInfo Agent2::getAgentInfo(){
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
void Agent2::removePreviousMovies(){
    
    lockAgent();
    
    cout << "Remove previous movies" << endl;
    
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
    
    //vector<MovieInfo>& movieSequence = getMovieSequence();
    //float scale = getNormalScale();
    
    // for each action, i.e., each segment in path, cut the last movie to match the length
    
    int lastMovIndex = 0;
    int firstMovIndex = 0;
    
    cout<< "------- start ------" << endl;
    
    for (int a=0; a < actions.size(); a++) {
        cout << " ------ action " << a << endl;
        float totalDistance = 0;
        
        int firstNextMovIndex = 0;
        
        
        char currentActionDirection = actions[a].first;
        float currentActionLength = actions[a].second;
        
        
        MovieInfo* lastActionMovie;
        
        firstMovIndex = lastMovIndex + 1;

        for (int i = firstMovIndex; i < sequence.size(); i++) {
            if (sequence[i].agentActionIndex == a) lastMovIndex = i;
        }
        
        lastActionMovie = &sequence[lastMovIndex];
        cout << "last action movie is " << lastMovIndex << endl;
        
        
        for (int i = lastMovIndex+1; i < sequence.size() && sequence[i].agentActionIndex == a+1; i++) {
            if (model.isLoopMarker(sequence[i].markername)){
                firstNextMovIndex = i;
                break;
            }
            
        }
        cout << "first none-transition movie of the next action is " << firstNextMovIndex << endl;
        
        
        if (firstNextMovIndex < lastMovIndex) // there is no other movie after this action
            firstNextMovIndex = sequence.size()-1;
        
        
        cout << "firstMovIndex = " << firstMovIndex << endl;
        cout << "firstNextMovIndex = " << firstNextMovIndex << endl;
        cout << "lastMovIndex = " << lastMovIndex << endl;
        
        
        // --- correct the length based on reality

        
        ofPoint actStartPos = getScaledFloorOffsetAt(getSequenceFrames()[firstMovIndex]);
        ofPoint actEndPos = getCorrectedPath()[a+1];
        
        float realLength = 0;
        
        if (currentActionDirection == 'r' || currentActionDirection == 'l')
            realLength = abs(actStartPos.x - actEndPos.x);
        if (currentActionDirection == 'u' || currentActionDirection == 'd')
            realLength = abs(actStartPos.y - actEndPos.y);
        
        cout << "real length = " << realLength << " vs action length = " << currentActionLength << endl;
        currentActionLength = realLength;
        //---
        
        totalDistance = calculateMovieDistanceNormalised(firstMovIndex, firstNextMovIndex+1, currentActionDirection, 0);
        cout << "total distance from first movie to next movie before isertion = " << totalDistance << " / required length = " << currentActionLength << endl;
        
        
        
        /// insert enough movies to cover the distance
        
        
        
        MovieInfo nextMovie;
        nextMovie.name = lastActionMovie->name;
        nextMovie.path = lastActionMovie->path;
        nextMovie.startframe = lastActionMovie->startframe;
        nextMovie.endframe = lastActionMovie->endframe;
        nextMovie.speed = lastActionMovie->speed;
        nextMovie.markername = lastActionMovie->markername;
        nextMovie.agentActionIndex = lastActionMovie->agentActionIndex;
        
        
        while (totalDistance < currentActionLength) {
            
            cout << "WTF" << endl;
            pushAt(nextMovie, lastMovIndex);
            
            getPositionsForMovieSequence(sequence);
            normalise();
            
            totalDistance += calculateMovieDistanceNormalised(lastMovIndex, lastMovIndex + 1, currentActionDirection, 0);
            
            cout << "Repeating the last movie: total distance = " << totalDistance << endl;
        }
        
        
        // find the last movie for this action after insertion
        for (int i = firstMovIndex; i < sequence.size(); i++) {
            if (sequence[i].agentActionIndex == a)  lastMovIndex = i;
        }
        for (int i = lastMovIndex+1; i < sequence.size() && sequence[i].agentActionIndex == a+1; i++) {
            if (model.isLoopMarker(sequence[i].markername)){
                firstNextMovIndex = i;
                break;
            }
        }
        if (firstNextMovIndex < lastMovIndex) // there is no other movie after this action
            firstNextMovIndex = sequence.size()-1;
        
        lastActionMovie = &sequence[lastMovIndex];
        cout << "last action movie is " << lastMovIndex << endl;

        totalDistance = calculateMovieDistanceNormalised(firstMovIndex, firstNextMovIndex+1, currentActionDirection, 0);
        cout << "updated total distance after all insertions = " << totalDistance << endl;
        
        // Now that we have inserted movies to go over the required distance, we need to cut the last movie to get the exact distance
        
        // subtract the distance travelled in the last movie so that we check it frame-by-frame next
        int d = calculateMovieDistanceNormalised(lastMovIndex, lastMovIndex + 1, currentActionDirection, 0);
        totalDistance-= d;
        cout << "last action movie distance = " << d << endl;
        cout << "total distance after subtracting the last action movie = " << totalDistance << endl;
        
        for(int f = lastActionMovie->startframe + 1; f <=lastActionMovie->endframe; f++){
            float dist = calculateMovieDistanceNormalised(lastMovIndex, lastMovIndex, currentActionDirection, f - lastActionMovie->startframe);
            //float errorOffset = action
            
            float dd = 0;
            
            //dd = totalDistance + dist;
            ofPoint pos = getScaledFloorOffsetAt(getSequenceFrames()[lastMovIndex] + f - lastActionMovie->startframe);
            
            if (currentActionDirection == 'r' || currentActionDirection == 'l')
                dd = abs(pos.x - actEndPos.x);
            if (currentActionDirection == 'u' || currentActionDirection == 'd')
                dd = abs(pos.y - actEndPos.y);
            
            
            //if ((totalDistance+dist) >= currentActionLength) { // if one movie is enough to cover the distance, this is the right cut frame
            
            if (dd < 3) {
                cout << "dist at frame " << f << " = " << dist << endl;
                cout << "Ending the last movie at " << f << " instead of " << lastActionMovie->endframe << endl;
                int oldLength = lastActionMovie->endframe - lastActionMovie->startframe;
                lastActionMovie->endframe=f;
                lastActionMovie->isCut = true;
                // we have pushed this movie before, so we need to fix the sequence frames
                break;
            }
        }
        
        
        totalDistance = calculateMovieDistanceNormalised(firstMovIndex, firstNextMovIndex+1, currentActionDirection, 0);
        cout << "total distance from first movie to next movie after cut = " << totalDistance << " / required length = " << currentActionLength << endl;
        
        
        rebuildSequenceFrames();
        getPositionsForMovieSequence(sequence);
        normalise();
        
        d = calculateMovieDistanceNormalised(firstMovIndex, firstNextMovIndex+1, currentActionDirection, 0);
        totalDistance =d;
        cout << "final total distance from first movie to next movie = " << totalDistance << " / required length = " << currentActionLength << endl;
        
        //assert(abs(totalDistance - currentActionLength) < 20 );
        cout << abs(totalDistance - currentActionLength)  << endl;
        if (abs(totalDistance - currentActionLength) > 10)
            validSolution = false;
    }
    
    return validSolution;
}

//--------------------------------------------------------------
float Agent2::calculateMovieDistanceNormalised(int indexA, int indexB, char dir, int frameOffset) {
    
    ofPoint movieSP = getScaledFloorOffsetAt(getSequenceFrames()[indexA]);
    ofPoint movieEP = getScaledFloorOffsetAt(getSequenceFrames()[indexB] + frameOffset);
    
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
    
    // randomise SYNCMOTIONS or WAITMOTIONS TODO: make this selectable
    vector<string> vEndMotionType(2);
    vEndMotionType[0] = "SYNCMOTIONS";
    vEndMotionType[1] = "WAITMOTIONS";
    string endMotionType = random(vEndMotionType);
    
    vector<string>& endMotions = endGraph.getPossibleTransitions(endMotionType);
    string emotion = "STND_FRNT";//random(endMotions);
    
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