//
//  PlayerController.h
//  LAFTest
//
//  Created by gameover on 30/12/13.
//
//

#ifndef __H_PLAYERCONTROLLER
#define __H_PLAYERCONTROLLER

#include "ofMain.h"
#include "VectorUtils.h"
#include "ofxXMP.h"
#include "ofxThreadedVideo.h"
#include "AppModel.h"

class PlayerController{
    
public:
    
    PlayerController(){
        
    }
    
    ~PlayerController(){
        ofxLogVerbose() << "Destroying PlayerController with " << model->getPlayerName() << endl;
        video.close();
    }
    
    //--------------------------------------------------------------
    void setup(PlayerModel * pModel, PlayerView * pView, MotionGraph * fmGraph, MotionGraph * bmGraph, MotionGraph * dGraph){
        
        ofxLogNotice() << "Setting up PlayerController with " << pModel->getPlayerName() << endl;
     
        model = pModel;
        view = pView;
        forwardMotionGraph = fmGraph;
        backwardMotionGraph = bmGraph;
        directionGraph = dGraph;

        video.setPixelFormat(OF_PIXELS_BGRA);
        
        movieCue.push_back(model->getStartMovie());
        predictedFramesPlayed = predictedFrameCurrent = 0;
        
        bCueTransition = false;
        bFirstLoad = true;
        bFinsished = false;
    }
    
    //--------------------------------------------------------------
    void update(){
        
        video.update();

        currentMovie.isMovieDirty = (video.getQueueSize() > 0);
        
        if(!currentMovie.isMovieDirty){
            
            currentMovie.frame = video.getCurrentFrame();
            currentMovie.totalframes = video.getTotalNumFrames();
            currentMovie.speed = video.getSpeed();
            currentMovie.name = ofSplitString(video.getMovieName(), ".mov")[0];
            currentMovie.path = video.getMoviePath();
            currentMovie.isFrameNew = video.isFrameNew();
            predictedFrameCurrent = predictedFramesPlayed + currentMovie.frame - currentMovie.startframe;
            
            if(currentMovie.isFrameNew){
                
                bFirstLoad = false;
                
                view->setPixels(video.getPixelsRef());
                
                ofPoint kFrame = model->getKeyFrame(currentMovie.name, currentMovie.frame);
                currentMovie.position = pNormal - (kFrame - kNormal) * scale;
                pNormal = currentMovie.position;
                kNormal = kFrame;
                
                currentMovie.bounding = model->getScaledRectFrame(currentMovie.name, currentMovie.frame, currentMovie.position, scale);
                currentMovie.bounding.x -= floorOffset.x;
                currentMovie.bounding.y -= floorOffset.y;
                
                playerCentre = currentMovie.bounding.getCenter();
                distance = playerCentre.distance(windowCentre);
                
                
                
            }

        
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
                    
                    m.position = currentMovie.position;
                    m.bounding = currentMovie.bounding;
                    currentMovie = m;
                    if(!bFirstLoad) pNormal = currentMovie.position;
                    kNormal = model->getKeyFrame(m.name, m.startframe);
                    
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
                    
                    movieCue.pop_front();
                    
                    if(predictedFramesPlayed == -1) predictedFrameCurrent = 0;
                    predictedFramesPlayed = predictedFrameCurrent;
                    
                }else if(currentMovie.name != ""){ //if(movieCue.size() > 0){
                    cout << currentMovie.motion << endl;
                    if(currentMovie.motion == "STND_FRNT_FALL_BACK"){
                        cout << "FALLING" << endl;
                        predictedFrameCurrent = 0;
                        if(getDistanceToTarget() < 300.0){
                            pNormal.y += 8.0f;
                            currentMovie.bounding.y += 8.0f;
                        }else{
                            bFinsished = true;
                        }
                        
                    }else if(currentMovie.motion == "STND_FRNT_HUGG_FRNT"){
                        cout << "HUGGING" << endl;
                    }else{
                        video.setFrame(currentMovie.startframe);
                    }
                    
                }
            }
            
        }
        
        view->update();
        
    }
    
    //--------------------------------------------------------------
    void generateMoviesFromMotions(vector<string> motions, bool bAddToCue){
        
        vector<string> markerNames;
        map<string, ofxXMP> & metadata = model->getMetaData();
        map<string, vector<string> > & markDictionary = model->getMarkerDictionary();
        
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
                lastMovieInfo.position = -floorOffset;
                continue;
            }
            
            if(i == 0) lastMovieInfo.position = -floorOffset;
            
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
            mI.position = lastMovieInfo.position - (kF - kN) * scale;
            
            mI.predictedBounding = model->getProjectedRects(mI, lastMovieInfo, scale);
            
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
        
        predictedFrameCurrent = 0;
        predictedFramesPlayed = -1;
    }
    
    void clearChains(){
        movieCue.clear();
        predictedChainRects.clear();
        predictedChainPositions.clear();
        normalisedChainRects.clear();
        normalisedChainPositions.clear();
    }
    
    //--------------------------------------------------------------
    void generateMotionsBetween(string m1, string m2, bool bForward, vector<string>& motions){
        
        ofSeedRandom();

        MotionGraph motionGraph;
        vector<string> allPossibleTransitions;
        string startMotion, endMotion;
        
        if(bForward){
            motionGraph = *forwardMotionGraph;
            startMotion = m1;
            endMotion = m2;
        }else{
            motionGraph = *backwardMotionGraph;
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
    void normalisePredictedChains(ofPoint pN){
        
        normalisedChainRects = predictedChainRects;
        normalisedChainPositions = predictedChainPositions;

        float pX;
        float pY;
        
        for(int i = 0; i < predictedChainPositions.size(); i++){
            
            if(i < predictedFrameGoal){
                pX = pNormal.x;
                pY = pNormal.y;
            }else{
                pX = pNormal.x + predictedChainPositions[predictedFrameGoal].x + floorOffset.x;
                pY = pNormal.y + predictedChainPositions[predictedFrameGoal].y + floorOffset.y;
            }
            
            normalisedChainPositions[i].x += pX;
            normalisedChainPositions[i].y += pY;
            normalisedChainRects[i].x += pX;
            normalisedChainRects[i].y += pY;
        }

    }
    
    //--------------------------------------------------------------
    void setPosition(ofPoint p){
        pNormal = p - floorOffset;
    }
    
    //--------------------------------------------------------------
    bool isLoopMarker(ofxXMPMarker marker){
        vector<string> markerParts = ofSplitString(marker.getName(), "_");
        if(markerParts.size() != 4) return true;
        return (markerParts[0] + "_" + markerParts[1] == markerParts[2] + "_" + markerParts[3]);
    }
    
    //--------------------------------------------------------------
    string getStartMotionFromMarker(ofxXMPMarker& m){
        vector<string> mP = ofSplitString(m.getName(), "_");
        if(mP.size() < 4) return "";
        return string(mP[0] + "_" + mP[1]);
    }
    
    //--------------------------------------------------------------
    string getEndMotionFromMarker(ofxXMPMarker& m){
        vector<string> mP = ofSplitString(m.getName(), "_");
        if(mP.size() < 4) return "";
        return string(mP[2] + "_" + mP[3]);
    }
    
    //--------------------------------------------------------------
    string getStartMotionFromString(string m){
        vector<string> mP = ofSplitString(m, "_");
        if(mP.size() < 4) return "";
        return string(mP[0] + "_" + mP[1]);
    }
    
    //--------------------------------------------------------------
    string getEndMotionFromString(string m){
        vector<string> mP = ofSplitString(m, "_");
        if(mP.size() < 4) return "";
        return string(mP[2] + "_" + mP[3]);
    }
    
    //--------------------------------------------------------------
    void setDrawScale(float s){
        scale = s;
        ofRectangle r = model->getRectFrame(model->getStartMovie().name, 1);
        floorOffset.x = (r.x + r.width / 2.0f) * scale;
        floorOffset.y = (r.y + r.height) * scale + 4;
    }
    
    //--------------------------------------------------------------
    ofPoint& getFloorOffset(){
        return floorOffset;
    }
    
    //--------------------------------------------------------------
    float getDrawScale(){
        return scale;
    }

    //--------------------------------------------------------------
    ofRectangle& getBounding(){
        return currentMovie.bounding;
    }

    //--------------------------------------------------------------
    ofPoint& getPosition(){
        return currentMovie.position;
    }
    
    //--------------------------------------------------------------
    PlayerModel* getModel(){
        return model;
    }
    
    //--------------------------------------------------------------
    PlayerView* getView(){
        return view;
    }
    
    //--------------------------------------------------------------
    MovieInfo& getCurrentMovieInfo(){
        return currentMovie;
    }

    //--------------------------------------------------------------
    vector<ofPoint>& getPredictedChainPositions(){
        return predictedChainPositions;
    }
    
    //--------------------------------------------------------------
    vector<ofPoint>& getNormalisedChainPositions(){
        return normalisedChainPositions;
    }
    
    //--------------------------------------------------------------
    vector<ofRectangle>& getPredictedChainRects(){
        return predictedChainRects;
    }
    
    //--------------------------------------------------------------
    vector<ofRectangle>& getNormalisedChainRects(){
        return normalisedChainRects;
    }
    
    //--------------------------------------------------------------
    bool getIsLoaded(){
        return !bFirstLoad;
    }
    
    //--------------------------------------------------------------
    bool getIsFinished(){
        return bFinsished;
    }

    //--------------------------------------------------------------
    float& getDistanceToTarget(){
        return distance;
    }
    
    //--------------------------------------------------------------
    ofPoint& getPlayerCentre(){
        return playerCentre;
    }
    
    //--------------------------------------------------------------
    ofPoint& getTargetCentre(){
        return windowCentre;
    }
    
    //--------------------------------------------------------------
    ofRectangle& getTargetWindowRectangle(){
        return windowRectangle;
    }
    
    int getTargetWindowIndex(){
        return windowIndex;
    }
    
    //--------------------------------------------------------------
    void setTargetWindow(ofRectangle& r, int wIndex){
        windowRectangle = r;
        windowCentre = r.getCenter();
        windowIndex = wIndex;
    }
    
    //--------------------------------------------------------------
    int getPredictedFrameCurrent(){
        return predictedFrameCurrent;
    }
    
    //--------------------------------------------------------------
    int getPredictedFrameTotal(){
        return predictedChainRects.size();
    }
    
    //--------------------------------------------------------------
    int getPredictedFrameGoal(){
        return predictedFrameGoal;
    }
    
    //--------------------------------------------------------------
    void setPredictedFrameGoal(int f){
        predictedFrameGoal = f;
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
    
protected:
    
    ofPoint floorOffset;
    ofPoint targetPosition;
    
    ofPoint playerCentre;
    ofPoint windowCentre;
    ofRectangle windowRectangle;
    int windowIndex;
    float distance;
    
    MovieInfo currentMovie;
    deque<MovieInfo> movieCue;
    
    vector<ofRectangle> normalisedChainRects;
    vector<ofPoint> normalisedChainPositions;
    
    vector<ofRectangle> predictedChainRects;
    vector<ofPoint> predictedChainPositions;
    
    int predictedFrameCurrent;
    int predictedFramesPlayed;
    int predictedFrameGoal;
    
    bool bCueTransition;
    bool bFirstLoad, bFinsished;
    ofPoint pNormal, kNormal, oNormal;
    float scale;
    
    ofxThreadedVideo    video;
    
    MotionGraph*         forwardMotionGraph;
    MotionGraph*         backwardMotionGraph;
    MotionGraph*         directionGraph;
    PlayerModel*         model;
    PlayerView*          view;
    
};

#endif
