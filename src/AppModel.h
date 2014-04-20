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
#define USE_PRORES

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
#include "MouseObj.h"
#include "KeyModifiers.h"
#include "Pointer.h"
#include "Agent.h"
#include "Agent2.h"
#include "DeviceClient.h"

typedef struct{
    
    int masterPlayer;
    int slavePlayer;
    int syncFrame;
    int targetWindow;
    
} PlayerTargets;

class AppModel : public BaseModel{
    
public:
    
    AppModel(){
        //BaseModel::BaseModel();
        herovideoindex = -1;
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
    
    //--------------------------------------------------------------
    vector<int>& getWindowTargets(){
        return targetwindows;
    }
    
    //--------------------------------------------------------------
    vector<int>& getUniqueWindowTargets(){
        return targetunique;
    }
    
    //--------------------------------------------------------------
    int getUniqueWindowTarget(){
        int tWindow = -1;
        cout << "Unique request: " << targetunique.size() << endl;
        if(targetunique.size() > 0){
            tWindow = random(targetunique);
            eraseAll(targetunique, tWindow);
            cout << "Unique request: " << targetunique.size() << " giving " << tWindow << endl;
        }
        return tWindow;
    }
    
    //--------------------------------------------------------------
    void resetUniqueTargets(){
        targetunique = targetwindows;
    }
    
    //--------------------------------------------------------------
    bool hasUniqueWindowTargets(){
        return (targetunique.size() > 0);
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
    void listPlayerTemplates(){
        for(map<string, PlayerModel>::iterator it = playerModels.begin(); it != playerModels.end(); ++it){
            ofxLogNotice() << "Template: " << it->first << endl;
        }
    }
    
    //--------------------------------------------------------------
    ofPoint getRandomPlayerPosition(){

        cout << "uniqueStartingPositions: " <<  uniqueStartingPositions.size() << endl;

        int startPos = getUniqueStartPosition();
        int start2 = (int)ofRandom(2);
        
        float startXRegion = (int)ofRandom(2)*1850;
        float startYRegion = (int)ofRandom(2)*700;
        
        
        float xSpace = (getProperty<float>("OutputWidth") - 100) / getProperty<int>("NumberPlayers");
        float ySpace = (getProperty<float>("OutputHeight") - 100) / getProperty<int>("NumberPlayers");
        
        ofPoint p;
        if (startPos % 2 == 0){
            startPos--;
            p = ofPoint(startPos * xSpace + 50 , start2 * getProperty<float>("DefaultDrawSize") / 2.0f + startYRegion, startPos + 1); // what the fuck?
        } else {
            startPos--;
            p = ofPoint(start2 * getProperty<float>("DefaultDrawSize") / 2.0f + startXRegion, startPos *  ySpace + 50, startPos + 1); // what the fuck?
        }
        
        
        //ofPoint p = ofPoint(100,100);
        
        /*
        
        bool bFitted = false;
        
        while(!bFitted){
            
            float xOffset = ofRandom(0, getProperty<float>("DefaultDrawSize") * 2.0f) - getProperty<float>("DefaultDrawSize");
            float yOffset = ofRandom(0, getProperty<float>("DefaultDrawSize") * 2.0f) - getProperty<float>("DefaultDrawSize");
            
            if(xOffset > 0) xOffset += getProperty<float>("OutputWidth");
            if(yOffset > 0) yOffset += getProperty<float>("OutputHeight");
            
            p = ofPoint(xOffset, yOffset);
            
            for(int i = 0; i < agents.size(); i++){
                ofRectangle& r = agents[i]->getScaledBounding();
                if(r.inside(p)){
                    bFitted = false;
                    break;
                }else{
                    bFitted = true;
                }
            }
        }
        
        */
         
        return p;
        
    }
    
    //--------------------------------------------------------------
    string getRandomPlayerName(){
        int rSelect = (int)ofRandom(playerModels.size());
        int count = 0;
        for(map<string, PlayerModel>::iterator it = playerModels.begin(); it != playerModels.end(); ++it){
            if(rSelect == count) return it->first;
            count++;
        }
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
#ifdef USE_PRORES
            video->setPixelFormat(OF_PIXELS_2YUV);
#else
            video->setPixelFormat(OF_PIXELS_BGRA);
#endif
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
            int viewID = *it; int index;
            ofxLogVerbose() << "Deleting Marked Player: " << viewID << endl;
            Agent2* agent;
            for(int i = 0; i < agents.size(); i++){
                agent = agents[i];
                if(agent->getViewID() == viewID){
                    agent->stop();
                    targetunique.push_back(agent->getWindow());
                    uniqueStartingPositions.push_back(agent->getStartPosSegment());
                    delete agent;
                    index = i;
                    break;
                }
            }
            assigned.erase(assigned.find(viewID));
            string prop = "MovieInfo_" + ofToString(viewID);
            if(hasProperty<string>(prop)) removeProperty<string>(prop);
            eraseAt(agents, index);
            
        }
        todelete.clear();
    }
    
    //--------------------------------------------------------------
    void addAgent(Agent2 * agent){
        if(assigned.size() < videos.size()){
            ofxLogNotice() << "Assigning video to sequence with " << assigned.size() << " assigned out of " << videos.size() << " views" << endl;
            for(int i = 0; i < videos.size(); i++){
                if(assigned.find(i) == assigned.end()){
                    agent->setVideo(videos[i], i);
                    assigned.insert(i);
                    ofxLogVerbose() << "Assigned video to sequence with view " << i << " " << videos.size() - assigned.size() << " free" << endl;
                    break;
                }
            }
        }else{
            ofxLogError() << "Not enough views to assign video" << endl;
            assert(false);
        }
        agents.push_back(agent);
    }
    
    //--------------------------------------------------------------
    vector<Agent2*>& getAgents(){
        return agents;
    }
    
//    //--------------------------------------------------------------
//    void addSequence(MovieSequence * sequence){
//        if(assigned.size() < videos.size()){
//            ofxLogNotice() << "Assigning video to sequence with " << assigned.size() << " assigned out of " << videos.size() << " views" << endl;
//            for(int i = 0; i < videos.size(); i++){
//                if(assigned.find(i) == assigned.end()){
//                    sequence->setVideo(videos[i], i);
//                    assigned.insert(i);
//                    ofxLogVerbose() << "Assigned video to sequence with view " << i << " " << videos.size() - assigned.size() << " free" << endl;
//                    break;
//                }
//            }
//        }else{
//            ofxLogError() << "Not enough views to assign video" << endl;
//            assert(false);
//        }
//        sequences.push_back(sequence);
//    }
//    
//    //--------------------------------------------------------------
//    vector<MovieSequence*>& getSequences(){
//        return sequences;
//    }
    
    //--------------------------------------------------------------
    void addHeroVideo(string path){
        ofxLogNotice() << "Loading hero video: " << path << endl;
        ofxThreadedVideo* v = new ofxThreadedVideo;
#ifdef USE_PRORES
        v->setPixelFormat(OF_PIXELS_2YUV);
#else
        v->setPixelFormat(OF_PIXELS_BGRA);
#endif
        v->loadMovie(path);
        v->play();
        v->setFade(0.0f);
        v->setPaused(true);
        v->setLoopState(OF_LOOP_NONE);
        v->finish();
        v->setFade(0, getProperty<int>("HeroFade"), 1.0f);
        v->setFade(-1, getProperty<int>("HeroFade"), 0.0f);
        herovideos.push_back(v);
    }
    
    //--------------------------------------------------------------
    vector<ofxThreadedVideo*>& getHeroVideos(){
        return herovideos;
    }
    
    void stopHereo(){
        if(herovideoindex > -1){
            ofxLogNotice() << "Stopping (stop) hero: " << herovideoindex << endl;
            herovideos[herovideoindex]->setFade(0.0f);
            herovideos[herovideoindex]->setPaused(true);
        }
        herovideoindex = -1;
    }
    
    void activateHero(){
        if(herovideos.size() > 0){
            
            if(herovideoindex > -1){
                ofxLogNotice() << "Stopping (activate) hero: " << herovideoindex << endl;
                herovideos[herovideoindex]->setFade(0.0f);
                herovideos[herovideoindex]->setPaused(true);
            }
            herovideoindex = (int)ofRandom(herovideos.size());
            ofxLogNotice() << "Activating hero: " << herovideoindex << endl;
            herovideos[herovideoindex]->setPaused(false);
            herovideos[herovideoindex]->setFrame(1);
        }
    }
    
    int getActiveHero(){
        return herovideoindex;
    }
    
    ofxThreadedVideo* getCurrentHeroVideo(){
        if(herovideoindex != -1 && herovideos.size() > 0){
            return herovideos[herovideoindex];
        }else{
            return NULL;
        }
    }
    
    void resetHeroTimer(){
        heroStartTime = ofGetElapsedTimeMillis();
        heroStopTime = ofRandom(getProperty<int>("HeroTime"), 2 * getProperty<int>("HeroTime"));
        ofxLogNotice() << "Setting Hero Time for: " << heroStopTime << " milliseconds from now" << endl;
    }
    
    void stopHeroTimer(){
        heroStartTime = INFINITY;
    }
    
    bool checkHeroTimer(){
        if(heroStartTime + heroStopTime < ofGetElapsedTimeMillis()){
            ofxLogNotice() << "Activate Hero Video" << endl;
            stopHeroTimer();
            return true;
        }else{
            setProperty("HeroTrigger", string(ofToString(heroStartTime + heroStopTime) + " -> " + ofToString(ofGetElapsedTimeMillis())));
            return false;
        }
    }
    
    vector<MouseObj>& getMouseObjects(){
        return mouseObjects;
    }
    
    int& getCurrentMouseObject(){
        return currentObject;
    }
    
    KeyModifiers& getKeyModifiers(){
        return keyModifiers;
    }
    
    void initStartingPositions() {
        // set the possible starting positions for agents
        for (int i=0;i<getProperty<int>("NumberPlayers");i++) {
            uniqueStartingPositions.push_back(i+1);
        }
    }
    
    int getUniqueStartPosition() {
        int s = -1;
        if(uniqueStartingPositions.size() > 0){
            s = random(uniqueStartingPositions);
            eraseAll(uniqueStartingPositions, s);
        }
        return s;
    }
    
    map<int, DeviceClient>& getAllDevices(){
        return devices;
    }
    
    ofMutex& getDeviceMutex(){
        return deviceMutex;
    }
    
protected:
    
    KeyModifiers                keyModifiers;
    vector<MouseObj>            mouseObjects;
    int                         currentObject;
    
    int                         heroStartTime;
    int                         heroStopTime;
    
    vector<ofxThreadedVideo*>   herovideos;
    int                         herovideoindex;
    
    vector<ofxThreadedVideo*>   videos;
    vector<MovieSequence*>      sequences;
    vector<Agent2*>             agents;
    
    vector<PlayerTargets>   playertargets;
    
    set<int>                todelete;
    set<int>                assigned;
    set<int>                intersected;
    
    ofVideoPlayer           analysisVideo;
    ofRectangle             analysisRectangle;
    ofxCv::ContourFinder    analysisContourFinder;
    
    map<string, PlayerModel>    playerModels;
    
    vector<ofRectangle> windows;
    vector<ofRectangle> targets;
    vector<int>         targetunique;
    vector<int>         targetwindows;
    
    vector<int> uniqueStartingPositions;
    
    map<string, MotionGraph> motionGraphs;
    
    // device client storage
    map<int, DeviceClient> devices;
    ofMutex deviceMutex;
    
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
