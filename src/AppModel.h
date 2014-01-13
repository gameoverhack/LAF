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
#include "ofxCv.h"
#include "ofVideoPlayer.h"

class AppModel : public BaseModel{
    
public:
    
    AppModel(){
        //BaseModel::BaseModel();
    }
    
    ~AppModel(){
        //BaseModel::~BaseModel();
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
    void loadWindowPositions(string path){
        
        ofxLogNotice() << "Setting up windows with: " << path << endl;
        
        windows.clear();
        
        ofBuffer b = ofBufferFromFile(ofToDataPath(path));
        
        int lineCount = 0;
        windows.clear();
        
        while (!b.isLastLine()) {
            string line = b.getNextLine();
            vector<string> windowPosition = ofSplitString(line, ",");
            if(windowPosition.size() == 4){
                windows.push_back(ofRectangle(ofToFloat(windowPosition[0]), ofToFloat(windowPosition[1]), ofToFloat(windowPosition[2]), ofToFloat(windowPosition[3])));
            }
            lineCount++;
        }
    }
    
    //--------------------------------------------------------------
    vector<ofRectangle>& getWindows(){
        return windows;
    }
    
    //--------------------------------------------------------------
    void loadForwardMotionGraph(string path){
        forwardMotionGraph.setup(path);
    }
    
    //--------------------------------------------------------------
    void loadBackwardMotionGraph(string path){
        backwardMotionGraph.setup(path);
    }
    
    //--------------------------------------------------------------
    MotionGraph& getForwardMotionGraph(){
        return forwardMotionGraph;
    }
    
    //--------------------------------------------------------------
    MotionGraph& getBackwardMotionGraph(){
        return backwardMotionGraph;
    }
    
    //--------------------------------------------------------------
    void loadDirectionGraph(string path){
        directionGraph.setup(path);
    }
    
    //--------------------------------------------------------------
    MotionGraph& getDirectionGraph(){
        return directionGraph;
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
    PlayerView* getPlayerView(int playerID){
        map<int, PlayerView*>::iterator it = views.find(playerID);
        PlayerView * v;
        if(it == views.end()){
            ofxLogNotice() << "Creating view for PlayerID " << playerID << endl;
            v = new PlayerView;
            v->setup(getProperty<float>("VideoWidth"), getProperty<float>("VideoWidth"), 2);
            v->setTransitionLength(getProperty<int>("TransitionLength"));
            views[playerID] = v;
        }else{
            v = it->second;
        }
        return v;
    }
    
    //--------------------------------------------------------------
    PlayerModel* getPlayerModel(int playerID){
        map<int, PlayerModel*>::iterator it = models.find(playerID);
        PlayerModel * m;
        if(it == models.end()){
            ofxLogNotice() << "Creating model for PlayerID " << playerID << endl;
            m = new PlayerModel;
            models[playerID] = m;
        }else{
            m = it->second;
        }
        return m;
    }
    
    //--------------------------------------------------------------
    void deletePlayerModel(int playerID){
        map<int, PlayerModel*>::iterator it = models.find(playerID);
        if(it == models.end()){
            ofxLogNotice() << "Error no model for PlayerID " << playerID << endl;
        }else{
            models.erase(it);
        }
    }
    
    //--------------------------------------------------------------
    int getNumPlayerViews(){
        return views.size();
    }
    
    //--------------------------------------------------------------
    int getNumPlayerModels(){
        return models.size();
    }
    
protected:
    
    set<int>                intersected;
    
    ofVideoPlayer           analysisVideo;
    ofRectangle             analysisRectangle;
    ofxCv::ContourFinder    analysisContourFinder;
    
    map<int, PlayerView*>       views;
    map<int, PlayerModel*>      models;
    
    map<string, PlayerModel>    playerModels;
    
    vector<ofRectangle> windows;
    
    MotionGraph forwardMotionGraph;
    MotionGraph backwardMotionGraph;
    MotionGraph directionGraph;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(playerModels);
		ar & BOOST_SERIALIZATION_NVP(windows);
        ar & BOOST_SERIALIZATION_NVP(directionGraph);
        ar & BOOST_SERIALIZATION_NVP(forwardMotionGraph);
        ar & BOOST_SERIALIZATION_NVP(backwardMotionGraph);
	}
    
};

BOOST_CLASS_VERSION(AppModel, 2)

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * appModel	= AppModelSingleton::Instance();

#endif
