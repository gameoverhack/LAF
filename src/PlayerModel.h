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
            xmp.setRemoveCarriageReturns(true);
            xmp.loadXMP(filePath);
            xmp.listMarkers();
            //xmp.dumpDynamicMetaData();
            
            vector<string> nameParts = ofSplitString(fileName, "_");
            string firstMarker = nameParts[0] + "_" + nameParts[1] + "_" + nameParts[0] + "_" + nameParts[1];
            
            for(int j = 0; j < xmp.size(); j++){
                
                ofxXMPMarker m = xmp.getMarker(j);
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
    MovieInfo getFirstMovie(){
        MovieInfo mI;
        mI.name = "STND_TODO_CRCH_TODO_STND_TODO_00_" + playerName;
        mI.path = playerFolder + mI.name + ".mov";
        mI.frame = 0;
        mI.startframe = 0;
        mI.markername = metadata[mI.name].getLastMarker(mI.startframe).getName();
        mI.endframe = metadata[mI.name].getNextMarker(mI.startframe + 1).getStartFrame();
        ostringstream os; os << mI;
        ofxLogVerbose() << "Getting FirstMovie: " << os.str() << endl;
        return mI;
    }
    
    //--------------------------------------------------------------
    ofxXMPMarker getMarkerAt(string name, int frame){
        return metadata[name].getLastMarker(frame);
    }
    
    //--------------------------------------------------------------
    ofRectangle getBoundingAt(string name, int frame){
        vector<ofRectangle> & r = rectframes[name];
        frame = CLAMP(frame, 0, r.size() - 1);
        return r[frame];
    }
    
    //--------------------------------------------------------------
    ofPoint getKeyFrameAt(string name, int frame){
        vector<ofPoint> & k = keyframes[name];
        frame = CLAMP(frame, 0, k.size() - 1);
        return k[frame];
    }
    
    //--------------------------------------------------------------
    int getTotalFrames(string name){
        vector<ofPoint> & k = keyframes[name];
        return k.size();
    }
    
    //--------------------------------------------------------------
    vector<string>& getMotionsWith(string name){
        return fileDictionary[name];
    }
    
    //--------------------------------------------------------------
    vector<string>& getFilesWith(string motion){
        return markDictionary[motion];
    }
    
    //--------------------------------------------------------------
    bool isLoopMarker(ofxXMPMarker& m){
        vector<string> markerParts = ofSplitString(m.getName(), "_");
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
    void setDimensions(float w, float h){
        width = w;
        height = h;
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
    map<string, vector<ofPoint> >& getKeyFrames(){
        return keyframes;
    }
    
    //--------------------------------------------------------------
    map<string, vector<ofRectangle> >& getBoundingFrames(){
        return rectframes;
    }
    
    //--------------------------------------------------------------
    map<string, vector<string> >& getFileDictionary(){
        return fileDictionary;
    }
    
    //--------------------------------------------------------------
    map<string, vector<string> >& getMarkerDictionary(){
        return markDictionary;
    }
    
    //--------------------------------------------------------------
    map<string, ofxXMP>& getXMP(){
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

protected:
    
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
