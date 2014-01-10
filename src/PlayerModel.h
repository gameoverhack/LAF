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
        
        for(int i = 0; i < fileNames.size(); i++){
            
            string fileName = fileNames[i];
            string filePath = filePaths[i];
            
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
    
    MovieInfo getStartMovie(){

        MovieInfo mi;
        mi.name = "STND_TODO_CRCH_TODO_STND_TODO_00_" + playerName;
        mi.path = playerFolder + mi.name + ".mov";
        mi.speed = 2.0;
        mi.frame = 0;
        mi.startframe = 0;
        mi.genframe = metadata[mi.name].getNextMarker(mi.startframe + 1).getStartFrame();
        
        return mi;
        
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
    
    ofRectangle getScaledRectFrame(string name, int frame, ofPoint& p, float scale){
        ofRectangle r = getRectFrame(name, frame);
        r.x = r.x * scale + p.x;
        r.y = r.y * scale + p.y;
        r.width = r.width * scale;
        r.height = r.height * scale;
        return r;
    }
    
    ofRectangle getRectFrame(string name, int frame){
        vector<ofRectangle> & r = rectframes[name];
        frame = CLAMP(frame, 0, r.size());
        return r[frame];
    }
    
    ofPoint getKeyFrame(string name, int frame){
        vector<ofPoint> & k = keyframes[name];
        frame = CLAMP(frame, 0, k.size());
        return k[frame];
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
