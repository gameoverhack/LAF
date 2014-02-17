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
                //int wTarget = (int)ofRandom(targetWindows.size());
                if(wTarget != -1) makeSequence(appModel->getRandomPlayerName(), wTarget);
               // if(wTarget != -1) makeSequence("BLADIMIRSL", wTarget); // Omid: use bladimirsl so that we have all the motions
            }
            
            if(appModel->getProperty<bool>("ManualAgentControl")){
                makeManualAgent("BLADIMIRSL");
                appModel->setProperty("ManualAgentControl", false);
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
                MovieSequence* sequence = sequences[i];
                
                
                if (appModel->getProperty<bool>("AvoidCollisions")) {
                    //------------- Collision Detection
                    int range = 5;
                    int startFrame = MAX(sequence->getCurrentSequenceFrame(), 0);
                    int endFrame = MIN(sequence->getCurrentSequenceFrame() + range, sequence->getTotalSequenceFrames());
                    
                    sequence->setWillCollide(false);
                    
                    for(int j = startFrame; j < endFrame; j+=1){
                        for (int w = 0; w < windowPositions.size(); w++) { // with windows except the target window
                            ofRectangle bounding =sequence->getScaledBoundingAt(j);
                            
                            if (w!= sequence->getWindow() && bounding.intersects(windowPositions[w])) {
                              
                                recoverFromCollisionWithWindow(sequence, w);
                            }
                        }
                        
                        for (int p = 0; p < sequences.size();p++) { // with other players
                            if (sequences[p] != sequence && sequences[p]->getScaledBounding().intersects(sequence->getScaledBounding())) {
                                
                                recoverFromCollisionWithPlayer(sequence, sequences[p]);
                            }
                        }
                    }
                    //-------------

                }
                
                sequence->update();
                if(sequence->isSequequenceDone()) appModel->markPlayerForDeletion(sequence->getViewID());
                ostringstream os;
                os << sequence << endl;
                appModel->setProperty("MovieInfo_" + ofToString(sequence->getViewID()), os.str());
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
void PlayController::recoverFromCollisionWithPlayer(MovieSequence* playerSequence, MovieSequence* collisionSequence) {
    //  sequence->stop();
    playerSequence->setWillCollide(true);
    playerSequence->setSpeed(-1);
}

//--------------------------------------------------------------
void PlayController::recoverFromCollisionWithWindow(MovieSequence* playerSequence, int window) {
    //sequence->stop();
    //sequence->StopAt(endFrame);
    playerSequence->setSpeed(-1);
    playerSequence->setWillCollide(true);
}

//--------------------------------------------------------------
void PlayController::doAction(string name, char op) {
    
  
    
    ofxLogNotice() << "Performing action " << op << " for " << name << endl;
    
    vector<MovieSequence*>& sequences = appModel->getSequences();
    MovieSequence* movieSequence = new MovieSequence;
    
    for(int i = 0; i < sequences.size(); i++){
       if (sequences[i]->getManual())
           movieSequence = sequences[i];
    }
    
    MovieInfo lastMovie = movieSequence->getLastMovieInSequence();
    
    string lastMotion = ofSplitString(lastMovie.markername,"_")[0] + "_" + ofSplitString(lastMovie.markername,"_")[1];

    MotionGraph nestedForwardDirectionGraph = appModel->getGraph("DirectionGraph");
    nestedForwardDirectionGraph.nestGraph(appModel->getGraph("ForwardMotionGraph").getPossibilitie());

    // create a sequence of motions
    vector<string> motionSequence;

    cout << name <<endl;
    
    switch (op) {
        case 'l':
        {
            // get the possible approach motions for going left
            string act = movieSequence->getActionType("LR");
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
            string act = movieSequence->getActionType("LR");
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
            string act = movieSequence->getActionType("UD");
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
            string act = movieSequence->getActionType("UD");
            vector<string> transitions = nestedForwardDirectionGraph.getPossibleTransitions("DOWN");
            
            // get the first possible transition to the next action to down
            string motion ="";
            for (int t=0;t<transitions.size();t++) {
                if (ofSplitString(transitions[t],"_")[0]==act && ofSplitString(transitions[t],"_")[1]== "DOWN")
                    motion = transitions[t];
            }
            if (motion!="") {
                generateMotionsBetween(lastMotion, motion, name, motionSequence);
                motionSequence.push_back(motion);
            }
        }
            break;
        default:
            break;
    }
    
    
    
    generateMoviesFromMotions(motionSequence, movieSequence, name);
    getPositionsForMovieSequence(movieSequence, name);
    movieSequence->normalise();
    
    movieSequence->setSpeed(ofRandom(1.0, 3.0));
    
   // appModel->addSequence(movieSequence);
    movieSequence->play();

}

//--------------------------------------------------------------
void PlayController::makeManualAgent(string name) {
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    
    
    float scale = appModel->getProperty<float>("DrawSize") / model.getWidth();
    
    // create a new MovieSequence
    MovieSequence* movieSequence = new MovieSequence;
    movieSequence->setManual(true);
    movieSequence->setWindow(0);
    movieSequence->push(model.getFirstMovie());
    movieSequence->setNormalPosition(ofPoint(0,0,0));
    movieSequence->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    // start standing front and go to -> motion
    motionSequence.push_back("STND_FRNT");
    
    generateMoviesFromMotions(motionSequence, movieSequence, name);
    getPositionsForMovieSequence(movieSequence, name);
    movieSequence->normalise();
    
    movieSequence->setSpeed(ofRandom(1.0, 3.0));
    
    appModel->addSequence(movieSequence);
    movieSequence->play();
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
    
    // create a new MovieSequence
    MovieSequence* movieSequence = new MovieSequence;
    movieSequence->setWindow(window);
    movieSequence->push(model.getFirstMovie());
    movieSequence->setNormalPosition(ofPoint(0,0,0));
    movieSequence->setNormalScale(scale); // TODO: store scale on the PlayerModel?
    
    // create a sequence of motions
    vector<string> motionSequence;
    
    // start standing front and go to -> motion
    motionSequence.push_back("STND_FRNT");
    generateMotionsBetween("STND_FRNT", motion, name, motionSequence);
    motionSequence.push_back(motion);
    
    generateMoviesFromMotions(motionSequence, movieSequence, name);
    getPositionsForMovieSequence(movieSequence, name);
    movieSequence->normalise();
    
    // calulate and insert loops of the action to travel far enough to get to the target window
    vector<ofRectangle>& windowPositions = appModel->getWindows();
    
    int inserts = 0;
    float target = appModel->getProperty<float>("DrawSize");
    MovieInfo loopMovie = movieSequence->getLastMovieInSequence();
    
    if(direction == "LEFT" || direction == "RIGT"){
        if(direction == "RIGT") target += windowPositions[window].x;
        if(direction == "LEFT") target += ofGetWidth() - windowPositions[window].x;
        while (movieSequence->getScaledTotalBounding().width < target) {
            inserts++;
            movieSequence->push(loopMovie);
            movieSequence->normalise();
            cout << direction << " " << movieSequence->getScaledTotalBounding().width << endl;
        }
    }else if(direction == "DOWN" || direction == "UPPP") {
        if(direction == "DOWN") target += windowPositions[window].y;
        if(direction == "UPPP") target = 2 * target + ofGetHeight() - windowPositions[window].y;
        while (movieSequence->getScaledTotalBounding().height < target) {
            inserts++;
            movieSequence->push(loopMovie);
            movieSequence->normalise();
            cout << direction << " " << movieSequence->getScaledTotalBounding().height << endl;
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
    
    generateMoviesFromMotions(motionSequence, movieSequence, name);
    getPositionsForMovieSequence(movieSequence, name);
    movieSequence->normalise();
    
    // calculate target and syncframes
    MovieInfo& lastMovieInSequence = movieSequence->getLastMovieInSequence();
    int goalFrame = movieSequence->getTotalSequenceFrames() - 1;
    int syncFrame = goalFrame - lastMovieInSequence.startframe + xmp[lastMovieInSequence.name].getMarker(motionSequence[motionSequence.size() - 1]).getStartFrame();
    
    movieSequence->setGoalFrame(goalFrame - lastMovieInSequence.endframe - lastMovieInSequence.startframe);
    movieSequence->setSyncFrame(syncFrame);
    
    ofPoint floorOffset = movieSequence->getScaledFloorOffset();
    ofPoint targetPosition = ofPoint(windowPositions[window].x + windowPositions[window].width / 2.0, windowPositions[window].y, 0.0f);
    ofPoint finalSequencePosition = targetPosition - movieSequence->getScaledPositionAt(goalFrame) - floorOffset;
    
    if(emotion != "FALL_BACK"){
        
        movieSequence->setHug(true);
        
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
        
        generateMoviesFromMotions(motionSequence, movieSequence, name);
        getPositionsForMovieSequence(movieSequence, name);
    }else{
        movieSequence->setHug(false);
    }

    movieSequence->setNormalPosition(finalSequencePosition);
    movieSequence->normalise();
    
    ofxLogVerbose() << "Adding MovieSequence" << movieSequence->getMovieSequenceAsString() << endl;
    ofxLogVerbose() << "E(nd) Motion: " << endMotionType << " of " << emotion << endl;
    
    movieSequence->setSpeed(ofRandom(1.0, 3.0));
    
    appModel->addSequence(movieSequence);
    movieSequence->play();

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
void PlayController::getPositionsForMovieSequence(MovieSequence* movieSequence, string name){
    
    PlayerModel& model = appModel->getPlayerTemplate(name);
    vector<MovieInfo>& sequence = movieSequence->getMovieSequence();
    
    for(int i = 0; i < sequence.size(); i++){
        
        MovieInfo& m = sequence[i];
        int totalframes = m.endframe - m.startframe;
        ostringstream os; os << m;
        
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