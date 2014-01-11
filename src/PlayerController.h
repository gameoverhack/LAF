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
    
    void setup(PlayerModel * pModel, PlayerView * pView, MotionGraph * fmGraph, MotionGraph * bmGraph, MotionGraph * dGraph){
        
        ofxLogNotice() << "Setting up PlayerController with " << pModel->getPlayerName() << endl;
     
        model = pModel;
        view = pView;
        forwardMotionGraph = fmGraph;
        backwardMotionGraph = bmGraph;
        directionGraph = dGraph;

        video.setPixelFormat(OF_PIXELS_BGRA);
        
        movieCue.push_back(model->getStartMovie());
        
        bCueTransition = false;
        bFirstLoad = true;
    }
    
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

            if(currentMovie.isFrameNew){
                view->setPixels(video.getPixelsRef());
                
                ofPoint kFrame = model->getKeyFrame(currentMovie.name, currentMovie.frame);
                currentMovie.position = pNormal - (kFrame - kNormal) * scale;
                pNormal = currentMovie.position;
                kNormal = kFrame;
                
                currentMovie.bounding = model->getScaledRectFrame(currentMovie.name, currentMovie.frame, currentMovie.position, scale);
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
                    loadMovie(movieCue[0]);
                    movieCue.pop_front();
                    
                }else if(currentMovie.name != ""){ //if(movieCue.size() > 0){
                    
                    video.setFrame(currentMovie.startframe);
//                    cueRandomTransition();
                    
                }
            }
            
        }
        

        view->update();
        
    }
    
    void generateMoviesFromMotions(vector<string> motions){
        
        vector<string> markerNames;
        map<string, ofxXMP> & metadata = model->getMetaData();
        map<string, vector<string> > & markDictionary = model->getMarkerDictionary();
        
        for(int i = 1; i < motions.size(); i++){
            string markerName = motions[i - 1] + "_" + motions[i];
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
                continue;
            }

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
        
        predictedChainRects.clear();
        predictedChainPositions.clear();
        
        for(int i = 0; i < mIChain.size(); i++){
            for(int j = 0; j < mIChain[i].predictedBounding.size(); j++){
                predictedChainRects.push_back(mIChain[i].predictedBounding[j]);
                predictedChainPositions.push_back(mIChain[i].predictedPosition[j]);
            }
            movieCue.push_back(mIChain[i]);
        }
        
        
    }
    
    //--------------------------------------------------------------
    void setTargetPosition(ofPoint startPosition){
        ofPoint endPosition = predictedChainPositions[predictedChainPositions.size() - 1];
        pNormal -= (endPosition - startPosition);
        ofRectangle r = model->getRectFrame(model->getStartMovie().name, 1);
        pNormal.x -= (r.x + r.width / 2.0f) * scale;
        pNormal.y -= (r.y + r.height) * scale + 4;
        
        for(int i = 0; i < predictedChainPositions.size(); i++){
            predictedChainPositions[i] = predictedChainPositions[i] - startPosition;
            predictedChainRects[i].x = (predictedChainRects[i].x + pNormal.x) + (r.x + r.width / 2.0f) * scale;
            predictedChainRects[i].y = (predictedChainRects[i].y + pNormal.y) + (r.y + r.height) * scale + 4;
        }
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
    void cueRandomTransition(){
        generatePossibleMotions(currentMovie);
        movieCue.push_back(currentMovie.possibleTransitions[(int)ofRandom(currentMovie.possibleTransitions.size())]);
    }
    
    //--------------------------------------------------------------
    void generatePossibleMotions(MovieInfo& cMovie){
        
        ofSeedRandom();
        
        string tab = "";
        
        cout << tab << cMovie << endl;
        
        string motion = getEndMotionFromString(cMovie.motion);
        vector<string> allPossibleTransitions = forwardMotionGraph->getPossibleTransitions(motion);
        
        map<string, vector<string> > & markDictionary = model->getMarkerDictionary();
        map<string, ofxXMP> & metadata = model->getMetaData();
        
        for(int i = 0; i < allPossibleTransitions.size(); i++){
            
            string sMotion = motion + "_" + allPossibleTransitions[i];

            map<string, vector<string> >::iterator it = markDictionary.find(sMotion);
            
            if(it == markDictionary.end()){
                
                cout << tab << "No such marker exists currently, ignoring: " << sMotion << endl;
                
            }else{
                
                cout << tab << "Compiling all movies with " << sMotion << endl;
                
                vector<string> uniqueMovies = it->second;
                
                for(int j = 0; j < uniqueMovies.size(); j++){
                    
                    vector<ofxXMPMarker> mTs = metadata[uniqueMovies[j]].getMarkers(sMotion);
                    
                    for(int k = 0; k < mTs.size(); k++){
                        ofxXMPMarker& mT = mTs[k];
                        ofxXMPMarker& nT = metadata[uniqueMovies[j]].getNextMarker(mT.getStartFrame() + 1);
//                        cout << tab << "       Goto: " << mT << endl;
                        
                        int interestingFrame = -1;
                        while(!isLoopMarker(mT) && isLoopMarker(nT)){
//                            cout << tab << "           Finding next loop marker...after " << nT << endl;
                            if(interestingFrame == nT.getStartFrame()){
//                                cout << tab << "          That's the end of the movie" << endl;
                                break;
                            }
                            interestingFrame = nT.getStartFrame();
                            nT = metadata[uniqueMovies[j]].getNextMarker(nT.getStartFrame() + 1);
                        }
                        
                        MovieInfo mI;
                        mI.name = uniqueMovies[j];
                        mI.path = model->getPlayerFolder() + uniqueMovies[j] + ".mov";
                        mI.startframe = mI.frame = mT.getStartFrame();
                        mI.genframe = nT.getStartFrame();
                        mI.speed = cMovie.speed;
                        mI.isMovieDirty = true;
                        mI.isFrameNew = false;
                        mI.motion = sMotion;
                        mI.position = cMovie.position;
                        mI.predictedBounding = model->getProjectedRects(mI, cMovie, scale);
//                        mI.intersectionFrames.assign(windowPositions.size(), mI.startframe + mI.predictedBounding.size());
//                        mI.intersectedTransition = false;
                        
//                        cout << tab << "Check : " << mI.name << " " << mI.frame << " " << mI.genframe << " " << mI.predictedBounding.size() << " " << sMotion << endl;
//                        
//                        int minIntersection = mI.startframe + mI.predictedBounding.size();
//                        
//                        for(int w = 0; w < windowPositions.size(); w++){
//                            for(int f = 0; f < mI.predictedBounding.size(); f++){
//                                if(mI.predictedBounding[f].width != 0 && mI.predictedBounding[f].height != 0 && mI.predictedBounding[f].intersects(windowPositions[w])){
//                                    mI.intersectionFrames[w] = mI.startframe + f;
//                                    minIntersection = MIN(minIntersection, mI.startframe + f);
//                                    break;
//                                }
//                            }
//                        }
//                        
//                        bool addMovieInfo = true;
//                        
//                        if(minIntersection != mI.startframe + mI.predictedBounding.size()){
//                            
//                            addMovieInfo = false;
//                            
//                            if(iteration < iterations){
//                                
//                                iteration++;
//                                
//                                cout << tab << "ITCHECKING: " << mI.name << " " << minIntersection << endl;
//                                
//                                minIntersection -= 500;
//                                
//                                for(int f = minIntersection; f > mI.startframe; f = f - 50){
//                                    
//                                    cout << tab << "Frame generating: " << minIntersection << " " << f << " " << minIntersection - mI.startframe << endl;
//                                    
//                                    MovieInfo pI = mI;
//                                    
//                                    pI.frame = f;
//                                    pI.position = mI.predictedPosition[pI.frame - mI.startframe];
//                                    
//                                    generateAllPossibleTransitions(pI, iteration, iterations);
//                                    
//                                    cout << tab << "Possible transitions at target frame " << f << " == " << pI.possibleTransitions.size() << endl;
//                                    
//                                    if(pI.possibleTransitions.size() > 0){
//                                        addMovieInfo = true;
//                                        mI.possibleTransitions = pI.possibleTransitions;
//                                        mI.impossibleTransitions = pI.impossibleTransitions;
//                                        mI.genframe = f;
//                                        cMovie.intersectedTransition = true;
//                                        break;
//                                    }
//                                    
//                                }
//                            }
//                        }
                        
                        bool addMovieInfo = true;
                        if(addMovieInfo){
                            cout << tab << "Adding: " << mI.name << " " << mI.frame << " " << mI.genframe << " " << mI.predictedBounding.size() << " " << sMotion << endl;
                            cMovie.possibleTransitions.push_back(mI);
                        }else{
                            cout << tab << "Reject: " << mI.name << " " << mI.frame << " " << mI.genframe << " " << mI.predictedBounding.size() << " " << sMotion << endl;
                            cMovie.impossibleTransitions.push_back(mI);
                        }
                        
                    }
                    
                }
                
            }
            
        }
        
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
    
    void setNormalPosition(ofPoint p){
        ofRectangle r = model->getRectFrame(model->getStartMovie().name, 1);
        pNormal = p;
        pNormal.x -= (r.x + r.width / 2.0f) * scale;
        pNormal.y -= (r.y + r.height) * scale + 4;
    }
    
    vector<ofRectangle>& getPredictedChainRects(){
        return predictedChainRects;
    }
    
protected:
    
    void loadMovie(MovieInfo& m){
        
        cout << "Load Movie: " << m << endl;
        
        m.position = currentMovie.position;
        m.bounding = currentMovie.bounding;
        currentMovie = m;
        if(!bFirstLoad) pNormal = currentMovie.position;
        kNormal = model->getKeyFrame(m.name, m.startframe);
        
        video.loadMovie(m.path);
        video.setLoopState(OF_LOOP_NONE);
        video.play();
        video.setFrame(m.frame);
        video.setSpeed(m.speed);

        bFirstLoad = false;
    }
    
    MovieInfo currentMovie;
    deque<MovieInfo> movieCue;
    
    vector<ofRectangle> predictedChainRects;
    vector<ofPoint> predictedChainPositions;
    
    bool bCueTransition;
    bool bFirstLoad;
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
