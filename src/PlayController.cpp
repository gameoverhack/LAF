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
                makeSequence(appModel->getRandomPlayerName(), random(targetWindows));
            }
            playControllerStates.setState(kPLAYCONTROLLER_PLAY);
        }
            break;
        case kPLAYCONTROLLER_PLAY:
        {
            if(appModel->checkHeroTimer()) appModel->activateHero();
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->update();
                if(sequence->isSequequenceDone()) appModel->markPlayerForDeletion(sequence->getViewID());
                ostringstream os;
                os << sequence << endl;
                appModel->setProperty("MovieInfo_" + ofToString(sequence->getViewID()), os.str());
            }
            
            appModel->deleteMarkedPlayers();
            
            if(sequences.size() < appModel->getProperty<int>("NumberPlayers") &&
               appModel->getProperty<bool>("AutoGenerate")) playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            
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
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
        }
            break;
    }
    
}

//--------------------------------------------------------------
void PlayController::makeSequence(string name, int window){
    
    ofxLogNotice() << "Making sequence for " << name << " targeting window " << window << endl;
    
    // get the players model
    PlayerModel& model = appModel->getPlayerTemplate(name);
    map<string, ofxXMP>& xmp = model.getXMP();
    
    // get the possible approach motions for this window
    vector<string>& transitions = appModel->getGraph("TargetGraph").getPossibleTransitions(ofToString(window));
    
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
    
    vector<string>& endMotions = appModel->getGraph("EndGraph").getPossibleTransitions("SYNCMOTIONS");
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
    
    if(emotion == "HUGG_FRNT"){
        
        movieSequence->setHug(true);
        
        // reverse the motion to get back out
        motionSequence.clear();
        motionSequence.push_back("STND_FRNT");
        
        string reversemotion;
        if(direction == "LEFT") reversemotion = action + "_RIGT";
        if(direction == "RIGT") reversemotion = action + "_LEFT";
        if(direction == "DOWN") reversemotion = action + "_UPPP";
        if(direction == "UPPP") reversemotion = action + "_DOWN";
        
        generateMotionsBetween("STND_FRNT", reversemotion, name, motionSequence);
        for(int i = 0; i < inserts + 2; i++) motionSequence.push_back(reversemotion);
        
        generateMoviesFromMotions(motionSequence, movieSequence, name);
        getPositionsForMovieSequence(movieSequence, name);
    }else{
        movieSequence->setHug(false);
    }

    movieSequence->setNormalPosition(finalSequencePosition);
    movieSequence->normalise();
    
    ofxLogVerbose() << "Adding MovieSequence" << movieSequence->getMovieSequenceAsString() << endl;
    ofxLogVerbose() << "E(nd) Motion: " << emotion << endl;
    
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
                                                        if(allPossibleTransitions6[o] == endMotion){
                                                            //cout << "FOUND SOLUTION: " << allPossibleTransitions[i] << " -> " << allPossibleTransitions2[j] << " -> " << allPossibleTransitions3[k] << " -> " << allPossibleTransitions4[m] << " -> " << allPossibleTransitions5[n] << " -> " << allPossibleTransitions6[o] << " -> " << allPossibleTransitions6[p] << endl;
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