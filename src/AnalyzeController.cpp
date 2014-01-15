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
    
    /******************************************************
     *******                States                  *******
     *****************************************************/

    StateGroup newAnalyzeControllerStates("AnalyzeControllerStates");
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_LOAD, "kANALYZECONTROLER_LOAD"));
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_ANALYZE, "kANALYZECONTROLER_ANALYZE"));
    newAnalyzeControllerStates.addState(State(kANALYZECONTROLER_DONE, "kANALYZECONTROLER_DONE"));
    
    appModel->addStateGroup(newAnalyzeControllerStates);
    
    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    
    analyzeControllerStates.setState(kANALYZECONTROLER_LOAD);
    
}

//--------------------------------------------------------------
void AnalyzeController::update(){

    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
//    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch (analyzeControllerStates.getState()) {
        case kANALYZECONTROLER_LOAD:
        {
            
            // check if we've aready scanned for media folders...
            if(mediaDirectory.size() == 0){
                
                // load all the folder names in the MediaPath
                mediaDirectory.listDir(appModel->getProperty<string>("MediaPath"));
                
                for(int i = 0; i < mediaDirectory.size(); i++){
                    
                    // if it's a directory we assume this is media for a player
                    if(mediaDirectory.getFile(i).isDirectory()){
                        
                        string name = mediaDirectory.getFile(i).getFileName();
                        string path = mediaDirectory.getFile(i).getAbsolutePath();
                        
                        ofxLogNotice() << "Initializing model for " << name << endl;
                        
                        // get/create a template model for this player
                        PlayerModel & m = appModel->getPlayerTemplate(name);
                        
                        // if it's been serialized then it will know what to do with the filelisting
                        // we can also either force it to check all files or force it to re-list the
                        // directory - this last is used when copying files to a new directory
                        m.checkFileList(name, path, appModel->getProperty<bool>("ForceFileListCheck"), appModel->getProperty<bool>("ForceFileListUpdate"));
                        
                        // check if we need analysis
                        if(m.getNeedsAnalysis()){
                            if(appModel->getProperty<bool>("CheckKeyFrames")) m.loadKeyFrames();
                            if(appModel->getProperty<bool>("CheckXMP")) m.loadKeyXMP();
                            appModel->setProperty("AnalyseName", (string)name); // just setting here to init for analysis
                            appModel->setProperty("AnalysePlayers", appModel->getProperty<int>("AnalysePlayers") + 1);
                        }
                    }
                }
                
            }
            
            // if anything needs analysis set the controller to begin doing it!
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
            map<string, PlayerModel>& playerModels = appModel->getPlayerTemplates();
            
            string name = appModel->getProperty<string>("AnalyseName");
            
            if(name != ""){
                
                PlayerModel & m = appModel->getPlayerTemplate(name);
                
                if(!m.getNeedsAnalysis()){
                    
                    appModel->setProperty("AnalyseName", (string)"");
                    appModel->setProperty("AnalysePlayers", appModel->getProperty<int>("AnalysePlayers") - 1);
               
                }else{
                    
                    vector<string>& fileNames = m.getFileNames();
                    vector<string>& filePaths = m.getFilePaths();
                    
                    string fileName = fileNames[0];
                    string filePath = filePaths[0];
                    
                    // check if we've got a video loaded
                    if(video.getMoviePath() != filePath){
                        
                        // load the movie
                        video.loadMovie(filePaths[0]);
                        video.setLoopState(OF_LOOP_NONE);
                        
                        // set some props for feedback and tracking
                        appModel->setProperty("AnalysisFrameF", (string)fileName);
                        appModel->setProperty("AnalysisFrameC", -1);
                        appModel->setProperty("AnalysisFrameT", video.getTotalNumFrames());
                        
                        // make sure we clear any data for this movies rect frames
                        map<string, vector<ofRectangle> >& rectFrames = m.getBoundingFrames();
                        vector<ofRectangle> & rects = rectFrames[fileName];
                        
                        bool checkRects = false;
                        if(appModel->getProperty<bool>("CheckRects")){
                            if(rects.size() != video.getTotalNumFrames()){
                                checkRects = true;
                                cout << "WRONG NUMBER OF FRAMES " << rects.size() << " " << video.getTotalNumFrames() << endl;
                            }else{
                                cout << "RIGHT NUMBER OF FRAMES " << rects.size() << " " << video.getTotalNumFrames() << endl;
                            }
                        }
                        
                        if(checkRects){
                            rects.clear();
                        }else{
                            video.close();
                            eraseAt(fileNames, 0);
                            eraseAt(filePaths, 0);
                        }
                        
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
                            map<string, vector<ofRectangle> >& rectFrames = m.getBoundingFrames();
                            rectFrames[fileName].push_back(r);
                            //cout << rectFrames[fileName].size() << " " << video.getCurrentFrame() << " " << analysisFrame << endl;
                            
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