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
#include "PlayerModel.h"
#include "VectorUtils.h"
#include "FileList.h"
#include "MotionGraph.h"
#include "PlayerController.h"

class AppModel : public BaseModel{
    
public:
    
    AppModel(){
        //BaseModel::BaseModel();
    }
    
    ~AppModel(){
        //BaseModel::~BaseModel();
    }
    
    void createPlayer(string name){
        ofxLogNotice() << "Creating player of type: " << name << endl;;
        createPlayerModel(name);
        PlayerController* p = new PlayerController;
        p->setup(getPlayerModel(name));
        p->getModel().setMotionGraph(motionGraph);
        p->getModel().setDirectionGraph(directionGraph);
        p->getModel().setWindowPositions(windows);
        players.push_back(p);
    }
    
    vector<PlayerController*>& getPlayers(){
        return players;
    }
    
    void createPlayerModel(string name){
        map<string, PlayerModel>::iterator it = playerModels.find(name);
        if(it == playerModels.end()){
            ofxLogNotice() << "Loading assets for players of type: " << name << endl;
            PlayerModel& m = playerModels[name];
            m.setup(name, getProperty<string>("mediaPath"), motionGraph);
        }else{
            ofxLogNotice() << "Reusing assets for players of type: " << name << endl;
        }

    }
    
    PlayerModel& getPlayerModel(string name){
        map<string, PlayerModel>::iterator it = playerModels.find(name);
        assert(it != playerModels.end());
        return it->second;
    }
    
    void loadWindowPositions(string path){
        
        ofxLogNotice() << "Setting up windows with: " << path << endl;
        
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
    
    vector<ofRectangle>& getWindows(){
        return windows;
    }
    
    void loadMotionGraph(string path){
        motionGraph.setup(path);
    }
    
    MotionGraph& getMotionGraph(){
        return motionGraph;
    }
    
    void loadDirectionGraph(string path){
        directionGraph.setup(path);
    }
    
    MotionGraph& getDirectionGraph(){
        return directionGraph;
    }
    
    void save(string filename, ArchiveType archiveType){
        Serializer.saveClass(filename, (*this), archiveType);
        BaseModel::save(filename + "_props", archiveType);
    }

    void load(string filename, ArchiveType archiveType){
        Serializer.loadClass(filename, (*this), archiveType);
        BaseModel::load(filename + "_props", archiveType);
    }
    
protected:
    
    vector<PlayerController*>   players;
    map<string, PlayerModel>    playerModels;
    
    vector<ofRectangle> windows;
    
    MotionGraph motionGraph;
    MotionGraph directionGraph;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(playerModels);
		ar & BOOST_SERIALIZATION_NVP(windows);
        ar & BOOST_SERIALIZATION_NVP(motionGraph);
        if(version == 1){
            ar & BOOST_SERIALIZATION_NVP(directionGraph);
        }
	}
    
};

BOOST_CLASS_VERSION(AppModel, 1)

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * appModel	= AppModelSingleton::Instance();

#endif
