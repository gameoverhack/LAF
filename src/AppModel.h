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

static ofPoint NoOrigin = ofPoint(-INFINITY, -INFINITY);
static ofRectangle NoTarget = ofRectangle(-INFINITY, -INFINITY, 0, 0);

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
    ofRectangle getUniqueAgentTarget(){
        
        bool bUnique = false;

        ofRectangle rWindow;
        int iWindow;
        
        while(!bUnique){
            
            iWindow = random(targetwindows);
            rWindow = windows[iWindow];
            
            bUnique = true;
            for(int i = 0; i < agents.size(); i++){
                
                Agent2* agent = agents[i];
                AgentInfo agentInfo = agents[i]->getAgentInfo();
                
                if(agentInfo.target == rWindow){
                    bUnique = false;
                    break;
                }
                
            }
            
        }

        cout << "Unque target window: " << iWindow << " = " << rWindow << " " << bUnique << endl;
        
        if(!bUnique){
            return NoTarget;
        }else{
            return rWindow;
        }
        
    }
    
    //--------------------------------------------------------------
    ofPoint getUniqueAgentOrigin(){
        
        float dDrawSize = getProperty<float>("DefaultDrawSize") / 2.0f;
        float dWidth = getProperty<float>("OutputWidth");
        float dHeight = getProperty<float>("OutputHeight");
        
        bool bUnique = false;
        float xPos, yPos;
        
        while(!bUnique){
            
            int tSegIndex = ofRandom(0, 4);
            
            switch(tSegIndex){
                case 0:
                    // top line
                    xPos = ofRandom(-dDrawSize, dWidth + dDrawSize);
                    yPos = -dDrawSize;
                    break;
                case 1:
                    // left line
                    xPos = -dDrawSize;
                    yPos = ofRandom(-dDrawSize, dHeight + dDrawSize);
                    break;
                case 2:
                    // bottom line
                    xPos = ofRandom(-dDrawSize, dWidth + dDrawSize);
                    yPos = dHeight + dDrawSize;
                    break;
                case 3:
                    // right line
                    xPos = dWidth + dDrawSize;
                    yPos = ofRandom(-dDrawSize, dHeight + dDrawSize);
                    break;
            }
            
            ofRectangle r = ofRectangle(xPos - dDrawSize / 2.0f, yPos - dDrawSize / 2.0f, dDrawSize, dDrawSize);
            
            bUnique = true;
            for(int i = 0; i < agents.size(); i++){
                
                Agent2* agent = agents[i];
                AgentInfo agentInfo = agents[i]->getAgentInfo();
                
                ofRectangle o = ofRectangle(agentInfo.origin.x - dDrawSize / 2.0f, agentInfo.origin.x - dDrawSize / 2.0f, dDrawSize, dDrawSize);
                
                if(o.intersects(r)){
                    bUnique = false;
                    break;
                }
                
            }
            
            for(int i = 0; i < targetwindows.size(); i++){
                
                ofRectangle o = windows[targetwindows[i]];
                
                if(o.intersects(r)){
                    bUnique = false;
                    break;
                }
                
            }
            
        }
        
        cout << "(" << xPos << ", " << yPos << ") " << bUnique << endl;
        
        if(bUnique){
            return ofPoint(xPos, yPos);
        }else{
            return NoOrigin;
        }
        
        
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
        views.resize(number);
        for(int i = 0; i < views.size(); i++){
            ofxLogVerbose() << "Creating PlayerView " << i << endl;
            views[i].video = new ofxThreadedVideo;
#ifdef USE_PRORES
            views[i].video->setPixelFormat(OF_PIXELS_2YUV);
#else
            views[i].video->setPixelFormat(OF_PIXELS_BGRA);
#endif
            views[i].fboSmall = new ofFbo;
            views[i].fboSmall->allocate(getProperty<float>("DefaultDrawSize"), getProperty<float>("DefaultDrawSize"));
            
            views[i].fboBig = new ofFbo;
            views[i].fboBig->allocate(getProperty<float>("VideoWidth"), getProperty<float>("VideoHeight"));
        }
    }
    
    //--------------------------------------------------------------
    void deletePlayerViews(){
        ofxLogNotice() << "Deleting " << views.size() << " PlayerViews" << endl;
        for(int i = 0; i < views.size(); i++){
            ofxLogVerbose() << "Deleting PlayerView " << i << endl;
            views[i].video->flush();
            views[i].video->close();
        }
        views.clear();
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
                    
                    int deviceID = agent->getDeviceID();
                    
                    if(deviceID != -1){
                        
                        deviceMutex.lock();
                        
                        map<int, DeviceClient>::iterator it = devices.find(deviceID);
                        
                        if(it != devices.end()){
                            
                            ofxLogVerbose() << "Deleting Device Agent ref for: " << deviceID << endl;
                            
                            DeviceClient& device = it->second;
                            device.deassociate(agent);
                            
                        }
                        
                        deviceMutex.unlock();
                    }

                    agent->stopAgent();
                    
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
        if(assigned.size() < views.size()){
            ofxLogNotice() << "Assigning video to sequence with " << assigned.size() << " assigned out of " << views.size() << " views" << endl;
            for(int i = 0; i < views.size(); i++){
                if(assigned.find(i) == assigned.end()){
                    agent->setView(views[i], i);
                    assigned.insert(i);
                    ofxLogVerbose() << "Assigned video to sequence with view " << i << " " << views.size() - assigned.size() << " free" << endl;
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
    
    map<int, DeviceClient>& getAllDevices(){
        return devices;
    }
    
    ofMutex& getDeviceMutex(){
        return deviceMutex;
    }
    
    vector<AgentInfo>& getAgentInfos(){
        return infos;
    }
    
protected:
    
    KeyModifiers                keyModifiers;
    vector<MouseObj>            mouseObjects;
    int                         currentObject;
    
    int                         heroStartTime;
    int                         heroStopTime;
    
    vector<ofxThreadedVideo*>   herovideos;
    int                         herovideoindex;
    
    vector<MovieView>           views;
    vector<Agent2*>             agents;
    vector<AgentInfo>           infos;
    
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
