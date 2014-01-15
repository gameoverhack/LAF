//
//  AppModel.h
//  LAF
//
//  Created by gameover on 3/01/14.
//
//

#ifndef __H_APPMODEL
#define __H_APPMODEL

#define USE_OPENFRAMEWORKS_TYPES 1

#include "BaseModel.h"
#include "AppStates.h"
#include "VectorUtils.h"
#include "FileList.h"
#include "MotionGraph.h"
#include "PlayerView.h"
#include "PlayerModel.h"
#include "MovieSequence.h"
#include "MovieInfo.h"
#include "ofxCv.h"

class AppModel : public BaseModel{
    
public:
    
    AppModel(){
        //BaseModel::BaseModel();
    }
    
    ~AppModel(){
        //BaseModel::~BaseModel();
        deletePlayerViews();
    }
    
    //--------------------------------------------------------------
    void save(string filename, ArchiveType archiveType){
        Serializer.saveClass(filename, (*this), archiveType);
        BaseModel::save(filename + "_props", archiveType);
    }
    
    //--------------------------------------------------------------
    void backup(string filename, ArchiveType archiveType){
        Serializer.saveClass(filename + "_" + ofGetTimestampString() + ".bak", (*this), archiveType);
        BaseModel::save(filename + "_props_" + ofGetTimestampString() + ".bak", archiveType);
    }
    
    //--------------------------------------------------------------
    void load(string filename, ArchiveType archiveType){
        Serializer.loadClass(filename, (*this), archiveType);
        BaseModel::load(filename + "_props", archiveType);
    }
    
    //--------------------------------------------------------------
    void loadWindowPositions(string path){
        
        ofxLogNotice() << "Setting up windows with: " << path << endl;
        
        windows.clear();
        targetwindows.clear();
        
        ofBuffer b = ofBufferFromFile(ofToDataPath(path));
        
        int lineCount = 0;
        windows.clear();
        
        while (!b.isLastLine()) {
            string line = b.getNextLine();
            vector<string> windowPosition = ofSplitString(line, ",");
            if(windowPosition.size() == 4){
                addWindowTarget(windows.size());
                windows.push_back(ofRectangle(ofToFloat(windowPosition[0]), ofToFloat(windowPosition[1]), ofToFloat(windowPosition[2]), ofToFloat(windowPosition[3])));
            }
            lineCount++;
        }
        
    }
    
//    //--------------------------------------------------------------
//    vector<ofRectangle>& getWindowTargets(){
//        return targets;
//    }
    
    //--------------------------------------------------------------
    vector<int>& getWindowTargets(){
        return targetwindows;
    }
    
    //--------------------------------------------------------------
    void updateTargets(){
        //std::random_shuffle(targetwindows.begin(), targetwindows.end());
        targets.clear();
        for(int i = 0; i < targetwindows[i]; i++){
            targets.push_back(windows[targetwindows[i]]);
        }
    }
    
    //--------------------------------------------------------------
    bool isWindowTarget(int window){
        return contains(targetwindows, window);
    }
    
    //--------------------------------------------------------------
    void addWindowTarget(int window){
        if(!isWindowTarget(window)) targetwindows.push_back(window);
    }
    
    //--------------------------------------------------------------
    void removeWindowTarget(int window){
        eraseAll(targetwindows, window);
        updateTargets();
    }
    
    //--------------------------------------------------------------
    vector<ofRectangle>& getWindows(){
        return windows;
    }
    
    //--------------------------------------------------------------
    void setGraph(string path){
        
        string motionGraphName = ofSplitString(path, ".")[0];
        
        ofxLogNotice() << "Creating MotionGraph: " << motionGraphName << endl;
        
        MotionGraph motionGraph;
        motionGraph.setup(path);
        
        motionGraphs[motionGraphName] = motionGraph;
        
        if(motionGraphName == "TargetGraph"){
            
            ofxLogVerbose() << "Checking Window Targets from " << motionGraphName << endl;
            
            // check to see which windows have target motions
            for(int i = 0; i < windows.size(); i++){
                
                vector<string>& motions = motionGraph.getPossibleTransitions(ofToString(i));
                
                // remove the window if it doesn't have motions
                if(motions.size() == 0) removeWindowTarget(i);
                
            }
        }
    }

    //--------------------------------------------------------------
    MotionGraph& getGraph(string name){
        map<string, MotionGraph>::iterator it = motionGraphs.find(name);
        assert(it != motionGraphs.end());
        return motionGraphs[name];
    }
    
    //--------------------------------------------------------------
    ofVideoPlayer& getAnalysisVideo(){
        return analysisVideo;
    }
    
    //--------------------------------------------------------------
    ofRectangle& getAnalysisRectangle(){
        return analysisRectangle;
    }
    
    //--------------------------------------------------------------
    ofxCv::ContourFinder & getAnalysisContourFinder(){
        return analysisContourFinder;
    }
    
    //--------------------------------------------------------------
    bool isIntersected(int index){
        return (intersected.find(index) != intersected.end());
    }
    
    //--------------------------------------------------------------
    void clearIntersected(){
        intersected.clear();
    }
    
    //--------------------------------------------------------------
    void addIntersected(int firstIndex, int secondIndex){
        intersected.insert(firstIndex);
        intersected.insert(secondIndex);
    }
    
    //--------------------------------------------------------------
    PlayerModel& getPlayerTemplate(string name){
        map<string, PlayerModel>::iterator it = playerModels.find(name);
        if(it == playerModels.end()){
            ofxLogNotice() << "Creating new template model for " << name << endl;
        }
        return playerModels[name];
    }
    
    //--------------------------------------------------------------
    map<string, PlayerModel>& getPlayerTemplates(){
        return playerModels;
    }
    
    //--------------------------------------------------------------
    void createPlayerViews(int number){
        ofxLogNotice() << "Creating " << number << " PlayerViews" << endl;
        for(int i = 0; i < number; i++){
            ofxLogVerbose() << "Creating PlayerView " << i << endl;
            ofxThreadedVideo* video = new ofxThreadedVideo;
            video->setPixelFormat(OF_PIXELS_BGRA);
            videos.push_back(video);
        }
    }
    
    //--------------------------------------------------------------
    void deletePlayerViews(){
        ofxLogNotice() << "Deleting " << videos.size() << " PlayerViews" << endl;
        for(int i = 0; i < videos.size(); i++){
            ofxLogVerbose() << "Deleting PlayerView " << i << endl;
            videos[i]->flush();
            videos[i]->close();
            delete videos[i];
        }
        videos.clear();
    }
    
    //--------------------------------------------------------------
    void markPlayerForDeletion(int index){
        ofxLogVerbose() << "Marking Player for Delete: " << index << endl;
        todelete.insert(index);
    }
    
    //--------------------------------------------------------------
    void deleteMarkedPlayers(){
        if(todelete.size() == 0) return;
        ofxLogNotice() << "Deleting Marked Players" << endl;
        for(set<int>::iterator it = todelete.begin(); it != todelete.end(); ++it){
            int index = *it;
            ofxLogVerbose() << "Deleting Marked Player: " << index << endl;
            MovieSequence* movieSequence = sequences[index];
            assigned.erase(assigned.find(movieSequence->getViewID()));
            movieSequence->getVideo()->close();
            movieSequence->clear();
            delete movieSequence;
            eraseAt(sequences, index);
        }
        todelete.clear();
    }
    
    //--------------------------------------------------------------
    void addSequence(MovieSequence * sequence){
        if(assigned.size() < videos.size()){
            ofxLogNotice() << "Assigning video to sequence with " << assigned.size() << " assigned out of " << videos.size() << " views" << endl;
            for(int i = 0; i < videos.size(); i++){
                if(assigned.find(i) == assigned.end()){
                    sequence->setVideo(videos[i], i);
                    assigned.insert(i);
                    ofxLogVerbose() << "Assigned video to sequence with view " << i << " " << videos.size() - assigned.size() << " free" << endl;
                    break;
                }
            }
        }else{
            ofxLogError() << "Not enough views to assign video" << endl;
            assert(false);
        }
        sequences.push_back(sequence);
    }
    
    //--------------------------------------------------------------
    vector<MovieSequence*>& getSequences(){
        return sequences;
    }
    
protected:
    
    vector<ofxThreadedVideo*>   videos;
    vector<MovieSequence*>      sequences;
    
    set<int>                todelete;
    set<int>                assigned;
    set<int>                intersected;
    
    ofVideoPlayer           analysisVideo;
    ofRectangle             analysisRectangle;
    ofxCv::ContourFinder    analysisContourFinder;
    
    map<string, PlayerModel>    playerModels;
    
    vector<ofRectangle> windows;
    vector<ofRectangle> targets;
    vector<int>         targetwindows;
    
    map<string, MotionGraph> motionGraphs;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(playerModels);
		ar & BOOST_SERIALIZATION_NVP(windows);
        ar & BOOST_SERIALIZATION_NVP(targets);
        ar & BOOST_SERIALIZATION_NVP(targetwindows);
        ar & BOOST_SERIALIZATION_NVP(motionGraphs);
	}
    
};

BOOST_CLASS_VERSION(AppModel, 4)

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * appModel	= AppModelSingleton::Instance();

#endif
