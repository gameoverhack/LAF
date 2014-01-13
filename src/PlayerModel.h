//
//  PModel.h
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#ifndef __H_PLAYERMODEL
#define __H_PLAYERMODEL

#define USE_OPENFRAMEWORKS_TYPES 1
#define USE_BOOST_SERIALIZE 1

#include "FileList.h"
#include "VectorUtils.h"
#include "ofxXMP.h"
#include "ofxLogger.h"
#include "MovieInfo.h"
#include "ofxThreadedVideo.h"

class PlayerModel{
    
public:
    
    PlayerModel(){
        
        ofxLogVerbose() << "Creating PlayerModel" << endl;
        
    }
    
    ~PlayerModel(){
        
        ofxLogVerbose() << "Destroying PlayerModel" << endl;
        
        playerName = "";
        playerFolder = "";
        fileList.clear();
        keyframes.clear();
        metadata.clear();
        fileDictionary.clear();
        markDictionary.clear();
        
    }
    
    //--------------------------------------------------------------
    void checkFileList(string name, string path, bool bForceCheck = false, bool bForceUpdateFileList = false){
        
        playerName = name;
        playerFolder = path + "/";
        
        if(bForceUpdateFileList){
            fileList.clear();
            fileList.allowExt("mov");
            fileList.listDir(playerFolder, false);
        }
        
        if(fileList.size() == 0){
            
            ofxLogNotice() << "Initializing media listing for " << name << endl;
            
            fileList.allowExt("mov");
            fileList.listDir(playerFolder, false);
            
            for(int i = 0; i < fileList.size(); i++){
                fileNames.push_back(fileList.getFile(i).name);
                filePaths.push_back(fileList.getFile(i).path);
            }
            
        }else{
            
            ofxLogNotice() << "Checking media listing for " << name << endl;
            
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
                    
                    ofxLogNotice() << "Deleting media listing for " << oldFile.name << endl;
                    
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
                
                if(checkFile || bForceCheck){
                    
                    ofxLogNotice() << "Marking media listing for " << oldFile.name << endl;
                    
                    fileNames.push_back(oldFile.name);
                    filePaths.push_back(oldFile.path);
                }
                
            }
        }
        
        fileList.clear();
        fileList.allowExt("mov");
        fileList.listDir(playerFolder, false);
    }
    
    //--------------------------------------------------------------
    void loadKeyFrames(){
        
        keyframes.clear();
        
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
    
    //--------------------------------------------------------------
    void loadKeyXMP(){
        
        metadata.clear();
        fileDictionary.clear();
        markDictionary.clear();
        
        for(int i = 0; i < fileNames.size(); i++){
            
            string fileName = fileNames[i];
            string filePath = filePaths[i];
            
            ofxLogVerbose() << "Analysing XMP for: " << fileName << endl;
            
            ofxXMP & xmp = metadata[fileName];
            
            xmp.setup();
            xmp.setNormaliseMarkers(true);
            xmp.setAllowDoubles(true);
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
                    if(!contains(files, fileName)) files.push_back(fileName);
                }
            }
        }
        
    }
    
    //--------------------------------------------------------------
    void setup(int pID){
        cout << "Setting up model: " << pID << endl;
        playerID = pID;
        bFirstLoad = true;
        movieCue.push_back(getStartMovie());
        predictedFramesPlayed = predictedFrameCurrent = 0;
    }
    
    //--------------------------------------------------------------
    void update(ofxThreadedVideo& video){
        
        currentMovie.frame = video.getCurrentFrame();
        currentMovie.totalframes = video.getTotalNumFrames();
        currentMovie.speed = video.getSpeed();
        currentMovie.name = ofSplitString(video.getMovieName(), ".mov")[0];
        currentMovie.path = video.getMoviePath();
        currentMovie.isFrameNew = video.isFrameNew();
        predictedFrameCurrent = predictedFramesPlayed + currentMovie.frame - currentMovie.startframe;
        
        if(currentMovie.isFrameNew){
            
            bFirstLoad = false;
            
            ofPoint kFrame = getKeyFrame(currentMovie.name, currentMovie.frame);
            currentMovie.position = pNormal - (kFrame - kNormal) * scale;
            pNormal = currentMovie.position;
            kNormal = kFrame;
            
            currentMovie.bounding = getScaledRectFrame(currentMovie.name, currentMovie.frame, currentMovie.position);
            currentMovie.bounding.x -= floorOffset.x;
            currentMovie.bounding.y -= floorOffset.y;
            
            playerCentre = currentMovie.bounding.getCenter();
            distance = playerCentre.distance(windowCentre);
            
        }
    }
    
    //--------------------------------------------------------------
    MovieInfo getStartMovie(){
        
        MovieInfo mI;
        mI.name = "STND_TODO_CRCH_TODO_STND_TODO_00_" + playerName;
        mI.path = playerFolder + mI.name + ".mov";
        mI.speed = 3.0;
        mI.frame = 0;
        mI.startframe = 0;
        mI.motion = metadata[mI.name].getLastMarker(mI.startframe).getName();
        mI.genframe = metadata[mI.name].getNextMarker(mI.startframe + 1).getStartFrame();
        mI.predictedBounding = getProjectedRects(mI, mI);
        
        return mI;
        
    }
    
    void clearChains(){
        movieCue.clear();
        predictedChainRects.clear();
        predictedChainPositions.clear();
        normalisedChainRects.clear();
        normalisedChainPositions.clear();
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
    void setCurrentMovie(MovieInfo& m){
        m.position = currentMovie.position;
        m.bounding = currentMovie.bounding;
        currentMovie = m;
        if(!bFirstLoad) pNormal = currentMovie.position;
        kNormal = getKeyFrame(m.name, m.startframe);
        if(predictedFramesPlayed == -1) predictedFrameCurrent = 0;
        predictedFramesPlayed = predictedFrameCurrent;
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
        ofRectangle r = getRectFrame(getStartMovie().name, 1);
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
    float setDimensions(float w, float h){
        width = w; height = h;
    }
    
    //--------------------------------------------------------------
    float getWidth(){
        return width;
    }
    
    //--------------------------------------------------------------
    float getHeight(){
        return height;
    }
    
    //--------------------------------------------------------------
    bool getNeedsAnalysis(){
        return (filePaths.size() > 0);
    }
    
    //--------------------------------------------------------------
    string getPlayerFolder(){
        return playerFolder;
    }
    
    //--------------------------------------------------------------
    string getPlayerName(){
        return playerName;
    }
    
    //--------------------------------------------------------------
    vector<string>& getFilePaths(){
        return filePaths;
    }
    
    //--------------------------------------------------------------
    vector<string>& getFileNames(){
        return fileNames;
    }
    
    //--------------------------------------------------------------
    map<string, vector<ofRectangle> >& getRectFrames(){
        return rectframes;
    }
    
    //--------------------------------------------------------------
    vector<ofRectangle> getProjectedRects(MovieInfo& pMovie, MovieInfo& cMovie){
        if(pMovie.genframe < pMovie.frame) return vector<ofRectangle>(0);
        pMovie.predictedPosition.resize(pMovie.genframe - pMovie.startframe);
        vector<ofRectangle> pRects(pMovie.genframe - pMovie.frame);
        ofPoint pN = cMovie.position;
        ofPoint kN = getKeyFrame(pMovie.name, pMovie.frame);
        for(int i = pMovie.frame; i < pMovie.genframe; i++){
            ofRectangle r = rectframes[pMovie.name][i];
            ofPoint k = getKeyFrame(pMovie.name, i);
            pMovie.predictedPosition[i - pMovie.startframe] = pN - (k - kN) * scale;
            pN = pMovie.predictedPosition[i - pMovie.startframe]; kN = k;
            r = getScaledRectTransform(r, pMovie.predictedPosition[i - pMovie.startframe]);
            pRects[i - pMovie.frame] = r;
        }
        return pRects;
    }
    
    //--------------------------------------------------------------
    ofPoint getPredictedPosition(string movieName, int startFrame, int endFrame, ofPoint startingPoint, float scale){
        ofPoint p;
        ofPoint pN = startingPoint;
        ofPoint kN = getKeyFrame(movieName, startFrame);
        for(int i = startFrame; i < endFrame; i++){
            ofPoint k = getKeyFrame(movieName, i);
            p = pN - (k - kN) * scale;
            pN = p; kN = k;
        }
        return p;
    }
    
    //--------------------------------------------------------------
    ofRectangle getScaledRectTransform(ofRectangle& r, ofPoint& p){
        r.x = r.x * scale + p.x;
        r.y = r.y * scale + p.y;
        r.width = r.width * scale;
        r.height = r.height * scale;
        return r;
    }
    
    //--------------------------------------------------------------
    ofRectangle getScaledRectFrame(string name, int frame, ofPoint& p){
        ofRectangle r = getRectFrame(name, frame);
        return getScaledRectTransform(r, p);
    }
    
    //--------------------------------------------------------------
    ofRectangle getRectFrame(string name, int frame){
        vector<ofRectangle> & r = rectframes[name];
        frame = CLAMP(frame, 0, r.size() - 1);
        return r[frame];
    }
    
    //--------------------------------------------------------------
    ofPoint getKeyFrame(string name, int frame){
        vector<ofPoint> & k = keyframes[name];
        frame = CLAMP(frame, 0, k.size() - 1);
        return k[frame];
    }
    
    //--------------------------------------------------------------
    int getTotalKeyFrames(string name){
        vector<ofPoint> & k = keyframes[name];
        return k.size();
    }
    
    //--------------------------------------------------------------
    map<string, vector<string> >& getMarkerDictionary(){
        return markDictionary;
    }
    
    //--------------------------------------------------------------
    map<string, ofxXMP>& getMetaData(){
        return metadata;
    }
    
    //--------------------------------------------------------------
    float printKeyDifferences(){
        for(map<string, vector<ofPoint> >::iterator it = keyframes.begin(); it != keyframes.end(); ++it){
            string movie = it->first;
            vector<ofPoint>& k = it->second;
            cout << movie << "   " << k[k.size() - 1] - k[0] << endl;
        }
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
        cout << "SETTTING WINDOW: " << r << endl;
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
    int getPredictedFrameSync(){
        return predictedFrameSync;
    }
    
    //--------------------------------------------------------------
    void setPredictedFrameGoal(int f){
        
        predictedFrameGoal = f;
        MovieInfo& m = movieCue[movieCue.size() - 1];
        
        cout << "SET FRAME GOAL: " << "    " << m << endl;
        
        ofxXMPMarker marker = metadata[m.name].getMarker("STND_FRNT_FALL_BACK");
        
        if(marker != NoMarker){
            cout << "FALLING" << endl;
        }else{
            marker = metadata[m.name].getMarker("STND_FRNT_HUGG_FRNT");
            cout << "HUGGING" << endl;
        }
        
        assert(marker != NoMarker);
        
        cout << marker << endl;
        
        // in the case of the fall predictedFrameSync already equals predictedFrameGoal
        // but for HUGGING and other continuous actions we need to calculate an offset:
        predictedFrameSync = predictedFrameGoal - m.startframe + marker.getStartFrame();
        
        cout << predictedFrameGoal << " =?= " << predictedFrameSync << endl;
    }

    //--------------------------------------------------------------
    void setPredictedFramesPlayed(int f){
        predictedFramesPlayed = f;
    }
    
    //--------------------------------------------------------------
    int getPredictedFramesPlayed(){
        return predictedFramesPlayed;
    }
    
    //--------------------------------------------------------------
    deque<MovieInfo>& getMovieCue(){
        return movieCue;
    }
    
    //--------------------------------------------------------------
    int getPlayerID(){
        return playerID;
    }
    
protected:
    
    bool bFirstLoad;
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
    int predictedFrameSync;

    ofPoint pNormal, kNormal, oNormal;
    float scale;
    
    int playerID;
    
    //============================================
    
    vector<string> fileNames;
    vector<string> filePaths;
    
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

#endif
