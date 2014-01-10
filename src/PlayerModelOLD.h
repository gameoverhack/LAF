//
//  PlayerModel.h
//  LAFTest
//
//  Created by gameover on 3/01/14.
//
//

#ifndef __H_PLAYERMODEL
#define __H_PLAYERMODEL

#define USE_OPENFRAMEWORKS_TYPES 1
#define USE_BOOST_SERIALIZE 1

#include "FileList.h"
#include "VectorUtils.h"
#include "ofxXMP.h"
#include "MotionGraph.h"
#include "ofxThreadedVideo.h"
#include "ofxCv.h"

struct MovieInfo{
    string name;
    string path;
    int frame;
    int startframe;
    int genframe;
    int totalframes;
    float speed;
    bool isFrameNew;
    bool isMovieDirty;
    string motion;
    ofRectangle bounding;
    ofPoint position = ofPoint(0,0,0);
    vector<ofPoint> predictedPosition;
    vector<ofRectangle> predictedBounding;
    vector<MovieInfo> possibleTransitions;
    vector<MovieInfo> impossibleTransitions;
    bool intersectedTransition = false;
    vector<int> intersectionFrames;
    
};

class PlayerModel{
    
public:
    
    PlayerModel(){
        playerName = "";
        playerFolder = "";
        resetAttempts = 0;
        directionProbabilities.resize(5);
        setDirectionEVENUP();
    }
    
    ~PlayerModel(){
        playerName = "";
        playerFolder = "";
        fileList.clear();
        keyframes.clear();
        metadata.clear();
        fileDictionary.clear();
        markDictionary.clear();
    }
    
    void setup(string name, string path, MotionGraph & m){
        
        ofxLogNotice() << "Load assets for " << name << " from " << path;

        motionGraph = m;
        
        playerName = name;
        playerFolder = path + "/" + name + "/";
        
        vector<string> fileNames;
        vector<string> filePaths;
        
        if(fileList.size() == 0){
            
            fileList.allowExt("mov");
            fileList.listDir(playerFolder, false);
            
            for(int i = 0; i < fileList.size(); i++){
                fileNames.push_back(fileList.getFile(i).name);
                filePaths.push_back(fileList.getFile(i).path);
            }
            
        }else{
            
            FileList tFileList;
            tFileList.allowExt("mov");
            tFileList.listDir(playerFolder, false);
            
            for(int i = 0; i < fileList.size(); i++){
                
                File oldFile = fileList.getFile(i);
                bool checkFile = false;
                bool deleteFile = true;
                
                for(int j = 0; j < tFileList.size(); j++){
                    
                    File newFile = tFileList.getFile(i);
                    if(oldFile.name == newFile.name){
                        deleteFile = false;
                        if(oldFile.date != newFile.date){
                            checkFile = true;
                        }
                    }
                }
                
                if(deleteFile){
                    
                    eraseAll(rectframes, oldFile.name);
                    eraseAll(keyframes, oldFile.name);
                    eraseAll(metadata, oldFile.name);
                    eraseAll(fileDictionary, oldFile.name);
                    
                    for(map<string, vector<string> >::iterator it = markDictionary.begin(); it != markDictionary.end(); ++it){
                        vector<string>& v = it->second;
                        eraseAll(v, oldFile.name);
                        if(v.size() == 0) markDictionary.erase(it); --it;
                    }
                    
                }
                
                if(checkFile){
                    
                    fileNames.push_back(oldFile.name);
                    filePaths.push_back(oldFile.path);
                }
                
            }
        }
        
        fileList.clear();
        fileList.allowExt("mov");
        fileList.listDir(playerFolder, false);
        
        if(fileNames.size() > 0){
            // read keyframe data
            ofBuffer b = ofBufferFromFile(ofToDataPath(playerFolder + playerName + ".txt"));
            
            vector<ofPoint> kFrames;
            string clipName = "";
            while (!b.isLastLine()) {
                
                string line = b.getNextLine();
                
                vector<string> lineParts = ofSplitString(line, " ");
                
                // 3 part line is the name and frame/duration for the clip
                if(lineParts.size() == 3){
                    if(clipName == "") clipName = lineParts[0] + "_" + playerName;
                    if(kFrames.size() > 0){
                        if(contains(fileNames, clipName)){
                            ofxLogVerbose() << "Storing " << kFrames.size() << " keyframes for " << clipName << endl;
                            keyframes[clipName] = kFrames;
                        }
                        kFrames.clear();
                        clipName = lineParts[0] + "_" + playerName;
                    }
                }
                
                // two part is the frame num and the keyframe point in x, y, z
                if(lineParts.size() == 2){
                    vector<string> kParts = ofSplitString(lineParts[1], ",");
                    ofPoint p = ofPoint(ofToFloat(kParts[0]), ofToFloat(kParts[1]), ofToFloat(kParts[2]));
                    kFrames.push_back(p);
                }
            }
            
            if(contains(fileNames, clipName)){
                // store the last lot of keyframes
                ofxLogVerbose() << "Storing " << kFrames.size() << " keyframes for " << clipName << endl;
                keyframes[clipName] = kFrames;
            }
        }

        for(int i = 0; i < fileNames.size(); i++){
            
            string fileName = fileNames[i];
            string filePath = filePaths[i];
            
            ofxLogVerbose() << "Analysing frames for: " << filePath << endl;

            ofxCv::ContourFinder contourFinder;
            contourFinder.setMinAreaRadius(10);
            contourFinder.setMaxAreaRadius(1200);
            contourFinder.setThreshold(11);
            
            ofVideoPlayer p;
            p.loadMovie(filePath);
            
            width = p.getWidth();
            height = p.getHeight();
            
            int step = 1;
            
            ofRectangle r;
            
            for(int i = 0; i < p.getTotalNumFrames(); i++){

                if(i % step == 0 || i == 0){
                    
                    r.x = p.getWidth();
                    r.y = p.getHeight();
                    r.width = 0;
                    r.height = 0;
                    
                    p.setFrame(i);
                    p.update();
                    if(p.isFrameNew()){
                        
                        contourFinder.findContours(p);

                        for(int i = 0; i < contourFinder.getContours().size(); i++){
                            vector<cv::Point> pts = contourFinder.getContours()[i];
                            for(int j = 0; j < pts.size(); j++){
                                r.x = MIN(pts[j].x, r.x);
                                r.y = MIN(pts[j].y, r.y);
                                r.width = MAX(pts[j].x, r.width + r.x) - r.x;
                                r.height = MAX(pts[j].y, r.height + r.y) - r.y;
                            }
                        }
                    }
                }
//                cout << "    Rect frame: " << i << " == " << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
                rectframes[fileName].push_back(r);
            }
            
            ofxLogVerbose() << "Analysing XMP for: " << fileName << endl;
            
            ofxXMP & xmp = metadata[fileName];
            
            xmp.setup();
            xmp.setNormaliseMarkers(true);
            xmp.loadXMP(filePath);
            xmp.listMarkers();
            //xmp.dumpDynamicMetaData();
            
            vector<string> nameParts = ofSplitString(fileName, "_");
            string firstMarker = nameParts[0] + "_" + nameParts[1] + "_" + nameParts[0] + "_" + nameParts[1];
            
            for(int j = 0; j < xmp.size(); j++){
                
                ofxXMPMarker & m = xmp.getMarker(j);
                vector<string> markerParts = ofSplitString(m.getName(), "_");
                
                if(markerParts.size() == 4){
                    
                    string transition = m.getName();
                    
                    cout << fileName << "  " << m << " == " << transition << endl;
                    
                    vector<string> & transitions = fileDictionary[fileName];
                    vector<string> & files = markDictionary[transition];
                    
                    transitions.push_back(transition);
                    files.push_back(fileName);
                }
            }
        }
        
    }
    
    void reset(){
        
        
        
        nextMovieInfo.name = "STND_TODO_CRCH_TODO_STND_TODO_00_" + playerName;
        nextMovieInfo.path = playerFolder + nextMovieInfo.name + ".mov";
        nextMovieInfo.speed = 2.0;
        nextMovieInfo.frame = 1;

        currentMarker = metadata[nextMovieInfo.name].getLastMarker(1);
        currentMovieInfo.genframe = metadata[nextMovieInfo.name].getNextMarker(1).getStartFrame();
        currentMovieInfo.startframe = 1;

        pNormal = currentMovieInfo.position;
        kNormal = getKeyframe(nextMovieInfo.name, nextMovieInfo.frame);
        
        //currentMovieInfo.predictedBounding = getProjectedRects(nextMovieInfo, currentMovieInfo);

    }
    
    void updateMovie(ofxThreadedVideo & video){
        
        currentMovieInfo.frame = video.getCurrentFrame();
        currentMovieInfo.totalframes = video.getTotalNumFrames();
        currentMovieInfo.speed = video.getSpeed();
        currentMovieInfo.name = ofSplitString(video.getMovieName(), ".mov")[0];
        currentMovieInfo.path = video.getMoviePath();
        currentMovieInfo.isFrameNew = video.isFrameNew();
        currentMovieInfo.isMovieDirty = (video.getQueueSize() > 0);
        
        if(!currentMovieInfo.isMovieDirty){

            if(!nextMovieNeeded()){
                currentMovieInfo.position = pNormal - (getKeyframe(currentMovieInfo.name, currentMovieInfo.frame) - kNormal) * scale;
                pNormal = currentMovieInfo.position;
                kNormal = getKeyframe(currentMovieInfo.name, currentMovieInfo.frame);
            }
            
            vector<ofRectangle> & r = rectframes[currentMovieInfo.name];
            currentMovieInfo.bounding = r[(currentMovieInfo.frame < r.size() ? currentMovieInfo.frame : currentMovieInfo.frame - 1)];
            currentMovieInfo.bounding = getScaledTransform(currentMovieInfo.bounding, currentMovieInfo.position);
        }
        
        checkMarkers();
            
    }
    
    void checkMarkers(){
        
        if(!currentMovieInfo.isMovieDirty && nextMarker == NoMarker && currentMovieInfo.frame >= currentMovieInfo.genframe - 4){
            
            if(currentMovieInfo.intersectedTransition){
                cout << "USE PRECALCULATE" << endl;
                generateAllPossibleTransitions(currentMovieInfo, 1, 1);
            }else{
                cout << "USE  RECALCULATE" << endl;
                generateAllPossibleTransitions(currentMovieInfo, 0, 1);
            }
            
            cout << "Options: " << currentMovieInfo.possibleTransitions.size() << endl;
            
            if(currentMovieInfo.possibleTransitions.size() > 0){
                
                int rSelection = (int)ofRandom(currentMovieInfo.possibleTransitions.size());
                
                MovieInfo& mI = currentMovieInfo.possibleTransitions[rSelection];
                
//                bool reUse = false;
//                for(int i = 0; i < currentMovieInfo.possibleTransitions.size(); i++){
//                    if(currentMovieInfo.possibleTransitions[i].name == currentMovieInfo.name && currentMovieInfo.possibleTransitions[i].motion == mI.motion){
//                        mI = currentMovieInfo.possibleTransitions[i];
//                        reUse = true;
//                    }
//                }
                
                nextMovieInfo = mI;
                nextMovieInfo.possibleTransitions = currentMovieInfo.possibleTransitions;
                nextMovieInfo.impossibleTransitions = currentMovieInfo.impossibleTransitions;
                nextMarker = metadata[nextMovieInfo.name].getLastMarker(nextMovieInfo.frame);
                
//                if(reUse){
//                    cout << "REEUSING: " << nextMovieInfo.name << " " << nextMovieInfo.frame << " " << nextMovieInfo.genframe << " " << nextMarker.getName() << " " << endl;
//                }else{
                    cout << "GENUSING: " << nextMovieInfo.name << " " << nextMovieInfo.frame << " " << nextMovieInfo.genframe << " " << nextMarker.getName() << " " << endl;
//                }
                
                
                
            }else{
                cout << "GAMEOVER" << endl;
            }
            
        }

    }
    
    bool nextMovieNeeded(){
        return (nextMarker != NoMarker);
    }
    
    void resetMovieNeeded(){
        currentMarker = nextMarker;
        currentMovieInfo = nextMovieInfo;
        pNormal = currentMovieInfo.position;
        kNormal = getKeyframe(nextMovieInfo.name, nextMovieInfo.frame);
        nextMarker = NoMarker;
    }
    
    void setDrawScale(float s){
        scale = s;
    }
    
    float getDrawScale(){
        return scale;
    }
    
    void setNormalPosition(ofPoint p){
        ofRectangle r = rectframes[nextMovieInfo.name][1];
        pNormal = p;
        pNormal.x -= (r.x + r.width / 2.0f) * scale;
        pNormal.y -= (r.y + r.height) * scale + 4;
    }
    
    vector<ofRectangle>& getProjectedBounding(){
        return projectedRects;
    }
    
    ofRectangle& getBounding(){
        return currentMovieInfo.bounding;
    }
    
    void generateAllPossibleTransitions(){
        generateAllPossibleTransitions(currentMovieInfo, 0, 0);
    }
    
    void setDirectionEVENUP(){
        cout << "Direction EVENUP" << endl;
        directionProbabilities[0] = 0.100; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.200; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.200; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.300; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.200; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void setDirectionSTATIC(){
        cout << "Direction STATIC" << endl;
        directionProbabilities[0] = 0.920; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.020; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.020; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.020; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.020; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void setDirectionLEFT(){
        cout << "Direction LEFT" << endl;
        directionProbabilities[0] = 0.020; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.920; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.020; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.020; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.020; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void setDirectionRIGHT(){
        cout << "Direction RIGHT" << endl;
        directionProbabilities[0] = 0.020; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.020; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.920; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.020; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.020; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void setDirectionUP(){
        cout << "Direction UP" << endl;
        directionProbabilities[0] = 0.020; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.020; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.020; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.920; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.020; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void setDirectionDOWN(){
        cout << "Direction DOWN" << endl;
        directionProbabilities[0] = 0.020; //STAT     cout << tab << directions[0] << " " << probabilities[0] << endl;
        directionProbabilities[1] = 0.020; //LEFT     cout << tab << directions[1] << " " << probabilities[1] << endl;
        directionProbabilities[2] = 0.020; //RIGHT    cout << tab << directions[2] << " " << probabilities[2] << endl;
        directionProbabilities[3] = 0.020; //UP       cout << tab << directions[3] << " " << probabilities[3] << endl;
        directionProbabilities[4] = 0.920; //DOWN     cout << tab << directions[4] << " " << probabilities[4] << endl;
    }
    
    void generateAllPossibleTransitions(MovieInfo & cMovie, int iteration, int iterations){
        
        ofSeedRandom();
        
        string tab = string(iteration == 0 ? "" : "      ");
        
        cout << tab << "Generating with: " << cMovie.name << " " << cMovie.frame << endl;
        cout << tab << metadata[cMovie.name].getLastMarker(cMovie.startframe) << endl;
        cout << tab << metadata[cMovie.name].getLastMarker(cMovie.frame) << endl;
        cout << tab << metadata[cMovie.name].getLastMarker(cMovie.genframe) << endl;
        
        ofxXMPMarker& thisMarker = metadata[cMovie.name].getLastMarker(cMovie.frame);
        
        if(thisMarker.getName() == ""){
            cout << tab << "Last marker is a blank - using previous marker for search" << endl;
            thisMarker = metadata[cMovie.name].getLastMarker(cMovie.startframe);
        }
        
        string motion = getEndMotionFromMarker(thisMarker);
        cout << tab << "Searching for all possible transitions with: " << motion << " " << thisMarker << endl;
        
        
        cMovie.possibleTransitions.clear();
        cMovie.impossibleTransitions.clear();
        vector<string> allPossibleTransitions;// = motionGraph.getPossibleTransitions(motion);
        
        if(iteration == 0){
            while(allPossibleTransitions.size() == 0){
                
                vector<string>& directions = directionGraph.getKeys();
                
                string preferedDirection = getRandomElementFromDistribution(directionProbabilities, directions);

                allPossibleTransitions = motionGraph.getPossibleTransitions(preferedDirection, motion);
                
                if(allPossibleTransitions.size() > 0){
                    cout << tab << "could    use ";
                }else{
                    cout << tab << "couldn't use ";
                }
                cout << tab << preferedDirection << endl;
            }
        }else{
            allPossibleTransitions = motionGraph.getPossibleTransitions(motion);
        }

        cout << tab << "All Possible Transitions" << endl;
        cout << tab << "========================" << endl;
        cout << tab << allPossibleTransitions << endl;
        cout << tab << "========================" << endl;
        
        for(int i = 0; i < allPossibleTransitions.size(); i++){
            
            string sMotion = motion + "_" + allPossibleTransitions[i];
            map<string, vector<string> >::iterator it = markDictionary.find(sMotion);
            
            if(it == markDictionary.end()){
                
                cout << tab << "No such marker exists currently, ignoring: " << sMotion << endl;
                
            }else{
                
                cout << tab << "Compiling all movies with " << sMotion << endl;
                
                vector<string>& allMovies = it->second;
                vector<string> uniqueMovies = unique(allMovies);
                
                for(int j = 0; j < uniqueMovies.size(); j++){
                    
                    //cout << tab << "   Name: " << uniqueMovies[j] << endl;
                    
                    if(uniqueMovies[j].find("HUGG") != string::npos || uniqueMovies[j].find("FREE") != string::npos){
                        cout << tab << "Skipping the Hugging and Falling" << endl;
                        continue;
                    }
                    
                    vector<ofxXMPMarker> mTs = metadata[uniqueMovies[j]].getMarkers(sMotion);
                    
                    for(int k = 0; k < mTs.size(); k++){
                        ofxXMPMarker& mT = mTs[k];
                        ofxXMPMarker& nT = metadata[uniqueMovies[j]].getNextMarker(mT.getStartFrame() + 1);
                        //cout << tab << "       Goto: " << mT << endl;
                        
                        int interestingFrame = -1;
                        while(!isLoopMarker(mT) && isLoopMarker(nT) && iteration == 0){
                            cout << tab << "           Finding next loop marker...after " << nT << endl;
                            if(interestingFrame == nT.getStartFrame()){
                                cout << tab << "          That's the end of the movie" << endl;
                                break;
                            }
                            interestingFrame = nT.getStartFrame();
                            nT = metadata[uniqueMovies[j]].getNextMarker(nT.getStartFrame() + 1);
                        }
//                        cout << tab << "       Recheck: " << nT << endl;
                        
                        MovieInfo mI;
                        mI.name = uniqueMovies[j];
                        mI.path = playerFolder + uniqueMovies[j] + ".mov";
                        mI.startframe = mI.frame = mT.getStartFrame();
                        mI.genframe = nT.getStartFrame();
                        mI.speed = cMovie.speed;
                        mI.isMovieDirty = true;
                        mI.isFrameNew = false;
                        mI.motion = sMotion;
                        mI.position = cMovie.position;
                        mI.predictedBounding = getProjectedRects(mI, cMovie);
                        mI.intersectionFrames.assign(windowPositions.size(), mI.startframe + mI.predictedBounding.size());
                        mI.intersectedTransition = false;
                        
                        cout << tab << "Check : " << mI.name << " " << mI.frame << " " << mI.genframe << " " << mI.predictedBounding.size() << " " << sMotion << endl;
                        
                        int minIntersection = mI.startframe + mI.predictedBounding.size();
                        
                        for(int w = 0; w < windowPositions.size(); w++){
                            for(int f = 0; f < mI.predictedBounding.size(); f++){
                                if(mI.predictedBounding[f].width != 0 && mI.predictedBounding[f].height != 0 && mI.predictedBounding[f].intersects(windowPositions[w])){
                                    mI.intersectionFrames[w] = mI.startframe + f;
                                    minIntersection = MIN(minIntersection, mI.startframe + f);
                                    break;
                                }
                            }
                        }
    
                        bool addMovieInfo = true;
                        
                        if(minIntersection != mI.startframe + mI.predictedBounding.size()){
                            
                            addMovieInfo = false;
                            
                            if(iteration < iterations){
                                
                                iteration++;
                                
                                cout << tab << "ITCHECKING: " << mI.name << " " << minIntersection << endl;
                                
                                minIntersection -= 500;
                                
                                for(int f = minIntersection; f > mI.startframe; f = f - 50){
                                    
                                    cout << tab << "Frame generating: " << minIntersection << " " << f << " " << minIntersection - mI.startframe << endl;
                                    
                                    MovieInfo pI = mI;
                                    
                                    pI.frame = f;
                                    pI.position = mI.predictedPosition[pI.frame - mI.startframe];
                                    
                                    generateAllPossibleTransitions(pI, iteration, iterations);
                                    
                                    cout << tab << "Possible transitions at target frame " << f << " == " << pI.possibleTransitions.size() << endl;
                                    
                                    if(pI.possibleTransitions.size() > 0){
                                        addMovieInfo = true;
                                        mI.possibleTransitions = pI.possibleTransitions;
                                        mI.impossibleTransitions = pI.impossibleTransitions;
                                        mI.genframe = f;
                                        cMovie.intersectedTransition = true;
                                        break;
                                    }
                                    
                                }
                            }
                        }
                        
                        
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
    
    string getEndMotionFromMarker(ofxXMPMarker& m){
        vector<string> mP = ofSplitString(m.getName(), "_");
        if(mP.size() < 4) return "";
        return string(mP[2] + "_" + mP[3]);
    }
    
    string getStartMotionFromMarker(ofxXMPMarker& m){
        vector<string> mP = ofSplitString(m.getName(), "_");
        if(mP.size() < 4) return "";
        return string(mP[0] + "_" + mP[1]);
    }
    
    vector<ofRectangle> getProjectedRectsNextMarker(){
        if(currentMovieInfo.isMovieDirty) return vector<ofRectangle>(0);
        return getProjectedRects(currentMovieInfo, currentMovieInfo);
    }
    
    vector<ofRectangle> getProjectedRects(MovieInfo& pMovie, MovieInfo& cMovie){
        if(pMovie.genframe < pMovie.frame) return vector<ofRectangle>(0);
        pMovie.predictedPosition.resize(pMovie.genframe - pMovie.startframe);
        vector<ofRectangle> pRects(pMovie.genframe - pMovie.frame);
        ofPoint pN = cMovie.position;
        ofPoint kN = getKeyframe(pMovie.name, pMovie.frame);
        for(int i = pMovie.frame; i < pMovie.genframe; i++){
            ofRectangle r = rectframes[pMovie.name][i];
            ofPoint k = getKeyframe(pMovie.name, i);
            pMovie.predictedPosition[i - pMovie.startframe] = pN - (k - kN) * scale;
            pN = pMovie.predictedPosition[i - pMovie.startframe]; kN = k;
            r = getScaledTransform(r, pMovie.predictedPosition[i - pMovie.startframe]);
            pRects[i - pMovie.frame] = r;
        }
        return pRects;
    }
    
    ofPoint getPredictedPosition(string movieName, int startFrame, int endFrame, ofPoint startingPoint){
        ofPoint p;
        ofPoint pN = startingPoint;
        ofPoint kN = getKeyframe(movieName, startFrame);
        for(int i = startFrame; i < endFrame; i++){
            ofPoint k = getKeyframe(movieName, i);
            p = pN - (k - kN) * scale;
            pN = p; kN = k;
        }
        return p;
    }
    
    ofRectangle getScaledTransform(ofRectangle& r, ofPoint& p){
        r.x = r.x * scale + p.x;
        r.y = r.y * scale + p.y;
        r.width = r.width * scale;
        r.height = r.height * scale;
        return r;
    }
    
    ofPoint& getPosition(){
        return currentMovieInfo.position;
    }
    
    float getWidth(){
        return width;
    }
    
    float getHeight(){
        return height;
    }
    
    MovieInfo& getNextMovieInfo(){
        return nextMovieInfo;
    }
    
    MovieInfo& getCurrentMovieInfo(){
        return currentMovieInfo;
    }
    
    void setWindowPositions(vector<ofRectangle>& w){
        windowPositions = w;
    }
    
    void setMotionGraph(MotionGraph& m){
        motionGraph = m;
    }
    
    MotionGraph& getMotionGraph(){
        return motionGraph;
    }
    
    void setDirectionGraph(MotionGraph& m){
        directionGraph = m;
        motionGraph.nestGraph(directionGraph.getPossibilitie());
    }
    
    MotionGraph& getDirectionGraph(){
        return directionGraph;
    }
    
    ofPoint getKeyframe(string name, int frame){
        vector<ofPoint> & k = keyframes[name];
        if(frame >= k.size()){
            return k[frame - 1];
        }else{
            return k[frame];
        }
    }
    
    bool isLoopMarker(ofxXMPMarker marker){
        vector<string> markerParts = ofSplitString(marker.getName(), "_");
        if(markerParts.size() != 4) return true;
        return (markerParts[0] + "_" + markerParts[1] == markerParts[2] + "_" + markerParts[3]);
    }
    
    bool getDoesCurrentMovieHaveTransition(string transition){
        vector<string> & transitions = fileDictionary[currentMovieInfo.name];
        for(int i = 0; i < transitions.size(); i++){
            if(transitions[i] == transition) return true;
        }
        return false;
    }
    
    void checkMetaData(){
        for(map<string, ofxXMP>::iterator it = metadata.begin(); it != metadata.end(); ++it){
            cout << it->first << endl;
            ofxXMP & xmp = it->second;
            for(int i = 0; i < xmp.size(); i++){
                cout << xmp.getMarker(i) << endl;
            }
        }
    }
    
protected:
    
    int resetAttempts;
    
    vector<float> directionProbabilities;

    vector<ofRectangle> projectedRects;
    
    bool bMovieDirty;
    float scale;
    
    ofxXMPMarker currentMarker;
    ofxXMPMarker nextMarker;
    
    MovieInfo currentMovieInfo;
    MovieInfo nextMovieInfo;

    ofPoint kNormal, pNormal;
    
    MotionGraph motionGraph, directionGraph;
    vector<ofRectangle> windowPositions;
    
    string playerName;
    string playerFolder;
    FileList fileList;
    
    float width, height;
    
    map<string, vector<ofRectangle> >   rectframes;
    map<string, vector<ofPoint> >       keyframes;
    map<string, ofxXMP>                 metadata;
    map<string, vector<string> >        fileDictionary;
    map<string, vector<string> >        markDictionary;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        cout << "PlayerModel archive version: " << version << endl;
        ar & BOOST_SERIALIZATION_NVP(playerName);
        ar & BOOST_SERIALIZATION_NVP(playerFolder);
        ar & BOOST_SERIALIZATION_NVP(fileList);
        ar & BOOST_SERIALIZATION_NVP(rectframes);
        ar & BOOST_SERIALIZATION_NVP(keyframes);
        ar & BOOST_SERIALIZATION_NVP(metadata);
        ar & BOOST_SERIALIZATION_NVP(fileDictionary);
        ar & BOOST_SERIALIZATION_NVP(markDictionary);
        ar & BOOST_SERIALIZATION_NVP(width);
        ar & BOOST_SERIALIZATION_NVP(height);
	}
    
};

//BOOST_CLASS_VERSION(PlayerModel, 2)

#endif