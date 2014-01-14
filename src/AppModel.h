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

class AppModel : public BaseModel{
    
public:
    
    AppModel(){
        //BaseModel::BaseModel();
        hugvideo = NULL;
    }
    
    ~AppModel(){
        //BaseModel::~BaseModel();
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
            ofxLogError() << "No model for PlayerID " << playerID << endl;
            assert(false);
        }else{
            m = it->second;
        }
        return m;
    }
    
    //--------------------------------------------------------------
    void createPlayerModel(int playerID, string name){
        models[playerID] = new PlayerModel;
        PlayerModel m = playerModels[name];
        std::swap(*models[playerID], m);
        playerIDS.push_back(playerID);
    }
    
    //--------------------------------------------------------------
    void deletePlayerModel(int playerID){
        map<int, PlayerModel*>::iterator it = models.find(playerID);
        if(it == models.end()){
            ofxLogError() << "No model for PlayerID " << playerID << endl;
        }else{
            ofxLogNotice() << "Erasing playerID " << playerID << " " << models.size() << " " << playerIDS << endl;
            models.erase(it);
            eraseAll(playerIDS, playerID);
            ofxLogNotice() << "Erasing playerID " << playerID << " " << models.size() << " " << playerIDS << endl;
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
    
    //--------------------------------------------------------------
    vector<int>& getPlayerIDS(){
        return playerIDS;
    }
    
    void loadHugVideo(string name){
        cout << "LOADINGHUGG" << endl;
        hugvideo = new ofxThreadedVideo;
        hugvideo->setPixelFormat(OF_PIXELS_BGRA);
        hugvideo->loadMovie(getProperty<string>("MediaPath") + "/" + name + "HUGG.mov");
//        hugvideo->setLoopState(OF_LOOP_NONE);
        hugvideo->setFade(0.0f);
        hugvideo->play();
//        hugvideo.setPaused(true);
//        hugvideo.setFrame(0);
    }
    
    ofxThreadedVideo* getHugVideo(){
        return hugvideo;
    }
    
protected:
    
    ofxThreadedVideo*       hugvideo;
    
    set<int>                intersected;
    
    ofVideoPlayer           analysisVideo;
    ofRectangle             analysisRectangle;
    ofxCv::ContourFinder    analysisContourFinder;

    map<int, PlayerView*>       views;
    map<int, PlayerModel*>      models;
    vector<int>                 playerIDS;
    
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
