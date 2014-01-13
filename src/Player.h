//
//  Player.h
//  LAFTest
//
//  Created by gameover on 30/12/13.
//
//

#ifndef __H_PLAYER
#define __H_PLAYER

#define USE_OPENFRAMEWORKS_TYPES 1
#define USE_BOOST_SERIALIZE 1

#include "ofMain.h"
#include "AppModel.h"

class Player{
    
public:
    
    Player(){
        
    }
    
    ~Player(){
        PlayerModel * model = appModel->getPlayerModel(playerID);
        ofxLogVerbose() << "Destroying Player with " << model->getPlayerName() << endl;
        video.close();
    }
    
    //--------------------------------------------------------------
    void setup(int pID){
        
        playerID = pID;
        
        PlayerModel * model = appModel->getPlayerModel(playerID);
        ofxLogNotice() << "Setting up Player " << model->getPlayerName() << endl;

        video.setPixelFormat(OF_PIXELS_BGRA);
        
        bPausedSequence = false;
        bCueTransition = false;
        bFinsished = false;
    }
    
    //--------------------------------------------------------------
    void update(){

        PlayerModel * model = appModel->getPlayerModel(playerID);
        PlayerView * view = appModel->getPlayerView(playerID);
        deque<MovieInfo>& movieCue = model->getMovieCue();
        
        video.update();
        
        MovieInfo& currentMovie = model->getCurrentMovieInfo();
        currentMovie.isMovieDirty = (video.getQueueSize() > 0);
        
        if(bPausedSequence) return;
        
        if(!currentMovie.isMovieDirty){
            
            model->update(video);
            
            if(currentMovie.isFrameNew) view->setPixels(video.getPixelsRef());

            if(bCueTransition){
                view->setTransitionEndPixels(video.getPixelsRef());
                view->setRenderMode(RENDER_TRANSITION);
                bCueTransition = false;
            }

            if(currentMovie.frame >= currentMovie.genframe || currentMovie.name == ""){
                
                if(movieCue.size() > 0 && !bCueTransition){
                    
                    bCueTransition = true;
                    view->setTransitionStartPixels(video.getPixelsRef());
                    
                    MovieInfo& m = movieCue[0];
                    
                    bool bLoad = false;
                    int tFrame = -1;
                    
                    if(m.name == currentMovie.name){
                        if(m.startframe - currentMovie.genframe != 0){
                            cout << "Load Marker (frame   ): " << m << endl;
                            tFrame = m.startframe;
                        }else{
                            cout << "Load Marker (continue): " << m << endl;
                            bCueTransition = false;
                        }
                    }else{
                        cout << "Load Marker (load    ): " << m << endl;
                        bLoad = true;
                    }
                    
                    model->setCurrentMovie(m);
                    
                    if(bLoad){
                        video.loadMovie(m.path);
                        video.setLoopState(OF_LOOP_NONE);
                        video.play();
                        video.setFrame(m.frame);
                        video.setSpeed(m.speed);
                    }
                    if(tFrame > -1){
                        video.setFrame(tFrame);
                    }
                    
                    cout << movieCue.size() << endl;
                    movieCue.pop_front();
                    
                    if(model->getPredictedFramesPlayed() == -1){
                        model->setPredictedFramesPlayed(0);
                    }else{
                        model->setPredictedFramesPlayed(model->getPredictedFrameCurrent());
                    }
                    
                }else if(currentMovie.name != ""){ //if(movieCue.size() > 0){
                    
                    if(currentMovie.motion == "STND_FRNT_FALL_BACK"){
                        cout << "FALLING" << endl;
                        if(model->getDistanceToTarget() < 300.0){
                            model->setPosition(model->getPosition() + model->getFloorOffset() + ofPoint(0, 8, 0));
                            currentMovie.bounding.y += 8.0f;
                        }else{
                            bFinsished = true;
                        }
                        
                    }else if(currentMovie.motion == "STND_FRNT_HUGG_FRNT"){
                        cout << "HUGGING" << endl;
                    }else{
                        cout << "LOOPING: " << currentMovie << endl;
                        video.setFrame(currentMovie.startframe);
                    }
                    
                }
            }
            
        }
        
        view->update();
        
    }
    
    //--------------------------------------------------------------
    void generateMoviesFromMotions(vector<string> motions, bool bAddToCue){
        
        PlayerModel * model = appModel->getPlayerModel(playerID);
        MovieInfo& currentMovie = model->getCurrentMovieInfo();
        deque<MovieInfo>& movieCue = model->getMovieCue();
        map<string, vector<string> >& markDictionary = model->getMarkerDictionary();
        map<string, ofxXMP>& metadata = model->getMetaData();
        vector<ofRectangle>& predictedChainRects = model->getPredictedChainRects();
        vector<ofPoint>& predictedChainPositions = model->getPredictedChainPositions();
        
        vector<string> markerNames;
        
        for(int i = 1; i < motions.size(); i++){
            string markerName = motions[i - 1] + "_" + motions[i];
            if(markerName == "HUGG_FRNT_STND_FRNT") markerName = "STND_FRNT_STND_FRNT";
            markerNames.push_back(markerName);
        }
        
        cout << markerNames << endl;
        
        MovieInfo lastMovieInfo;
        vector<MovieInfo> mIChain;
        
        for(int i = 0; i < markerNames.size(); i++){
            
            cout << markerNames[i] << endl;
            
            if(i == 0 && markerNames[i] == currentMovie.motion){
                cout << "   MATCHED CURRENT " << currentMovie << endl;
                lastMovieInfo = currentMovie;
                lastMovieInfo.position = -model->getFloorOffset();
                continue;
            }
            
            if(i == 0) lastMovieInfo.position = -model->getFloorOffset();
            
            map<string, vector<string> >::iterator it = markDictionary.find(markerNames[i]);
            assert(it != markDictionary.end());
            
            vector<string> uniqueMovies = it->second;
            
            string rMovieName = "";
            std::random_shuffle(uniqueMovies.begin(), uniqueMovies.end());
            
            for(int j = 0; j < uniqueMovies.size(); j++){
                
                cout << "   " << j << " " << uniqueMovies[j] << "   " << markerNames[i] << endl;
                
                rMovieName = uniqueMovies[j];
                if(rMovieName == lastMovieInfo.name) break;
                
            }
            
            vector<ofxXMPMarker> mTs = metadata[rMovieName].getMarkers(markerNames[i]);
            
            std::random_shuffle(mTs.begin(), mTs.end());
            int startFrame = 0; int endFrame = 0;
            for(int k = 0; k < mTs.size(); k++){
                
                ofxXMPMarker& mT = mTs[k];
                ofxXMPMarker& nT = metadata[rMovieName].getNextMarker(mT.getStartFrame() + 1);
                
                cout << "     last " << lastMovieInfo.genframe << endl;
                cout << "     start " << mT << endl;
                cout << "     end   " << nT << endl;
                
                startFrame = mT.getStartFrame();
                endFrame = nT.getStartFrame();
                
                if(mT.getStartFrame() == lastMovieInfo.genframe) break;
                
            }
            
            MovieInfo mI;
            mI.name = rMovieName;
            mI.path = model->getPlayerFolder() + mI.name + ".mov";
            mI.startframe = mI.frame = startFrame;
            mI.genframe = endFrame;
            mI.speed = lastMovieInfo.speed;
            mI.isMovieDirty = false;
            mI.isFrameNew = false;
            mI.motion = markerNames[i];
            
            ofPoint kN = model->getKeyFrame(mI.name, mI.startframe);
            ofPoint kF = model->getKeyFrame(mI.name, mI.genframe);
            mI.position = lastMovieInfo.position - (kF - kN) * model->getDrawScale();
            
            mI.predictedBounding = model->getProjectedRects(mI, lastMovieInfo);
            
            mIChain.push_back(mI);
            
            lastMovieInfo = mI;
            
        }
        
        cout << mIChain << endl;

        for(int i = 0; i < mIChain.size(); i++){
            for(int j = 0; j < mIChain[i].predictedBounding.size(); j++){
                predictedChainRects.push_back(mIChain[i].predictedBounding[j]);
                predictedChainPositions.push_back(mIChain[i].predictedPosition[j]);
            }
            if(bAddToCue) movieCue.push_back(mIChain[i]);
        }

        model->setPredictedFramesPlayed(-1);

    }
    
    //--------------------------------------------------------------
    void generateMotionsBetween(string m1, string m2, bool bForward, vector<string>& motions){
        
        PlayerModel * model = appModel->getPlayerModel(playerID);
        
        ofSeedRandom();
        
        MotionGraph motionGraph;
        vector<string> allPossibleTransitions;
        string startMotion, endMotion;
        
        if(bForward){
            motionGraph = appModel->getForwardMotionGraph();
            startMotion = m1;
            endMotion = m2;
        }else{
            motionGraph = appModel->getBackwardMotionGraph();
            startMotion = m2;
            endMotion = m1;
        }
        
        cout << startMotion << " -> " << endMotion << endl;
        
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
        
        map<string, vector<string> > & markDictionary = model->getMarkerDictionary();
        std::random_shuffle(solutions.begin(), solutions.end());
        int shortest = INFINITY; int index = -1;
        for(int i = 0; i < solutions.size(); i++){
            
            bool legal = true;
            for(int j = 1; j < solutions[i].size(); j++){
                string sMotion = solutions[i][j - 1] + "_" + solutions[i][j];
                map<string, vector<string> >::iterator it = markDictionary.find(sMotion);
                if(it == markDictionary.end()){
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
            
            cout << "NO SOLUTION" << endl;
            assert(false);
            
        }else{
            
            cout << "SOLUTION: " << solutions[index] << endl;
            for(int i = 0; i < solutions[index].size(); i++){
                motions.push_back(solutions[index][i]);
            }
            
        }
        
    }
    
    //--------------------------------------------------------------
    bool getIsFinished(){
        return bFinsished;
    }
    
    //--------------------------------------------------------------
    void setFrame(int f){
        cout << "Frame: " << f << endl;
        video.setFrame(f);
        video.finish();
    }
    
    //--------------------------------------------------------------
    void setPaused(bool b){
        cout << "Pause: " << b << endl;
        video.setPaused(b);
        video.finish();
    }
    
    //--------------------------------------------------------------
    bool getPaused(){
        return video.isPaused();
    }
    
    //--------------------------------------------------------------
    int getPlayerID(){
        return playerID;
    }
    
    //--------------------------------------------------------------
    float getSpeed(){
        PlayerModel * model = appModel->getPlayerModel(playerID);
        MovieInfo& currentMovie = model->getCurrentMovieInfo();
        if(video.getSpeed() != currentMovie.speed){
            ofxLogWarning() << "SPEED: Video and CurrentMovieInfo are out of sync!" << endl;
        }
        return currentMovie.speed;
    }
    
    //--------------------------------------------------------------
    void setSpeed(float s){
        PlayerModel * model = appModel->getPlayerModel(playerID);
        MovieInfo& currentMovie = model->getCurrentMovieInfo();
        video.setSpeed(s);
        currentMovie.speed = s;
    }
    
    void setPausedSquence(bool b){
        bPausedSequence = b;
        setPaused(b);
    }
    
protected:
    
    bool bPausedSequence;
    int playerID;    
    bool bCueTransition, bFinsished;
    
    ofxThreadedVideo video;
    
    
};

#endif
