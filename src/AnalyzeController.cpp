//
//  AnalyzeController.cpp
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#include "AnalyzeController.h"

//--------------------------------------------------------------
AnalyzeController::AnalyzeController(){
    ofxLogVerbose() << "Creating AnalyzeController" << endl;
}

//--------------------------------------------------------------
AnalyzeController::~AnalyzeController(){
    ofxLogVerbose() << "Destroying AnalyzeController" << endl;
}

//--------------------------------------------------------------
void AnalyzeController::setup(){
    
    ofxLogVerbose() << "Setting Up AnalyzeController" << endl;
    
    // register AppController states
    StateGroup newAnalyzeControllerStates("AnalyzeControllerStates");
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_LOAD, "kANALYZECONTROLER_LOAD"));
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_ANALYZE, "kANALYZECONTROLER_ANALYZE"));
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_DONE, "kANALYZECONTROLER_DONE"));
    
    // add them to the model
    appModel->addStateGroup(newAnalyzeControllerStates);
    
    // get them back from the model so that changes go live
    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    
    appModel->setProperty("AnalysePlayers", 0);
    appModel->setProperty("AnalyseName", (string)"");
    analyzeControllerStates.setState(kANALYZECONTROLER_LOAD);
    
}

//--------------------------------------------------------------
void AnalyzeController::update(){

    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch (analyzeControllerStates.getState()) {
        case kANALYZECONTROLER_LOAD:
        {
            
            if(mediaDirectory.size() == 0){
                
                mediaDirectory.listDir(appModel->getProperty<string>("MediaPath"));
                
                for(int i = 0; i < mediaDirectory.size(); i++){
                    if(mediaDirectory.getFile(i).isDirectory()){
                        string name = mediaDirectory.getFile(i).getFileName();
                        string path = mediaDirectory.getFile(i).getAbsolutePath();
                        ofxLogNotice() << "Initializing model for " << name << endl;
                        PlayerModel & m = appModel->getPlayerModel(name);
                        m.checkFileList(name, path, appModel->getProperty<bool>("ForceFileListCheck"), appModel->getProperty<bool>("ForceFileListUpdate"));
                        if(m.getNeedsAnalysis()){
                            m.loadKeyFrames();
                            m.loadKeyXMP();
                            appModel->setProperty("AnalyseName", (string)name);
                            appModel->setProperty("AnalysePlayers", appModel->getProperty<int>("AnalysePlayers") + 1);
                        }
                    }
                }
                
            }
            
            if(appModel->getProperty<int>("AnalysePlayers") > 0){
                analyzeControllerStates.setState(kANALYZECONTROLER_ANALYZE);
            }else{
                analyzeControllerStates.setState(kANALYZECONTROLER_DONE);
            }
            
        }
            break;
        case kANALYZECONTROLER_ANALYZE:
        {
            
            ofVideoPlayer & video = appModel->getAnalysisVideo();
            ofxCv::ContourFinder& contourFinder = appModel->getAnalysisContourFinder();
            map<string, PlayerModel>& playerModels = appModel->getPlayerModels();
            
            string name = appModel->getProperty<string>("AnalyseName");
            
            if(name != ""){
                
                PlayerModel & m = appModel->getPlayerModel(name);
                
                if(!m.getNeedsAnalysis()){
                    
                    appModel->setProperty("AnalyseName", (string)"");
                    appModel->setProperty("AnalysePlayers", appModel->getProperty<int>("AnalysePlayers") - 1);
               
                }else{
                    
                    vector<string>& fileNames = m.getFileNames();
                    vector<string>& filePaths = m.getFilePaths();
                    
                    // check if we've got a video loaded
                    if(video.getMoviePath() != filePaths[0]){
                        
                        // load the movie
                        video.loadMovie(filePaths[0]);
                        video.setLoopState(OF_LOOP_NONE);
                        
                        // set some props for feedback and tracking
                        appModel->setProperty("AnalysisFrameF", (string)fileNames[0]);
                        appModel->setProperty("AnalysisFrameC", -1);
                        appModel->setProperty("AnalysisFrameT", video.getTotalNumFrames());
                        
                        // make sure we clear any data for this movies rect frames
                        map<string, vector<ofRectangle> >& rectFrames = m.getRectFrames();
                        rectFrames.clear();
                        
                        // set dimensions if not already done
                        if(m.getWidth() != video.getWidth() || m.getHeight() != video.getHeight()){
                            ofxLogNotice() << "Setting player width x height: " << video.getWidth() << " " << video.getHeight() << endl;
                            m.setDimensions(video.getWidth(), video.getHeight());
                        }
                        
                    }else{ // we're already loaded
                        
                        // set to next frame
                        appModel->setProperty("AnalysisFrameC", appModel->getProperty<int>("AnalysisFrameC") + 1);
                        int analysisFrame = appModel->getProperty<int>("AnalysisFrameC");
                        
                        // make sure we're not at end of the movie
                        if(analysisFrame < video.getTotalNumFrames()){
                            
                            // send the playhead to the next frame
                            video.setFrame(analysisFrame);
                            video.update();
                            
                            // analyse the frame if it's new
                            if(video.isFrameNew()){
                                
                                // set contour finder props - bit overzealous doing it every frame?
                                contourFinder.setMinAreaRadius(appModel->getProperty<int>("ContourMinArea"));
                                contourFinder.setMaxAreaRadius(appModel->getProperty<int>("ContourMaxArea"));
                                contourFinder.setThreshold(appModel->getProperty<int>("ContourThreshold"));
                                
                                // find the contour
                                contourFinder.findContours(video);
                                
                                // get rectangle reference
                                ofRectangle& r = appModel->getAnalysisRectangle();
                                
                                // reset rectangle to inverse bounds
                                r.x = video.getWidth();
                                r.y = video.getHeight();
                                r.width = 0;
                                r.height = 0;
                                
                                // find max and minima
                                for(int i = 0; i < contourFinder.getContours().size(); i++){
                                    vector<cv::Point> pts = contourFinder.getContours()[i];
                                    for(int j = 0; j < pts.size(); j++){
                                        r.x = MIN(pts[j].x, r.x);
                                        r.y = MIN(pts[j].y, r.y);
                                        r.width = MAX(pts[j].x, r.width + r.x) - r.x;
                                        r.height = MAX(pts[j].y, r.height + r.y) - r.y;
                                    }
                                }
                                
                                // store the frame rect in the player model
                                map<string, vector<ofRectangle> >& rectFrames = m.getRectFrames();
                                rectFrames[fileNames[0]].push_back(r);
                                
                            }
                            
                        }else{
                            
                            ofxLogNotice() << "Finsished frame analysis for " << fileNames[0] << endl;
                            
                            // close the video and 'pop' the video out fo the fileNames/Paths vectors
                            video.close();
                            eraseAt(fileNames, 0);
                            eraseAt(filePaths, 0);
                        }
                        
                    }
                }
                    
                
                
            }else{
                
                // find the next player needing analysis TODO: test this I haven't tried it yet ;)
                for(map<string, PlayerModel>::iterator it = playerModels.begin(); it != playerModels.end(); ++it){
                    PlayerModel & m = it->second;
                    if(m.getNeedsAnalysis()){
                        appModel->setProperty("AnalyseName", (string)it->first);
                        break;
                    }
                }
                
                // if we don't have one then let's play!
                if(appModel->getProperty<string>("AnalyseName") == ""){
                    analyzeControllerStates.setState(kANALYZECONTROLER_DONE);
                }
            }
            
            
            
        }
            break;
        case kANALYZECONTROLER_DONE:
        {
            ofxLogNotice() << "Analysis complete!!" << endl;
            // nothing to do
        }
            break;
            
            
    }
}