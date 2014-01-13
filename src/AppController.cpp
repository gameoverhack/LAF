//
//  AppController.cpp
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#include "AppController.h"

//--------------------------------------------------------------
AppController::AppController(){
    ofxLogVerbose() << "Creating AppController" << endl;
}

//--------------------------------------------------------------
AppController::~AppController(){
    ofxLogVerbose() << "Destroying AppController" << endl;
}

//--------------------------------------------------------------
void AppController::setup(){
    
    // set log levels
    ofxLogSetLogLevel(LOG_VERBOSE);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    ofxLogNotice() << "AppController start setup" << endl;
    
    // register AppController states
    StateGroup newAppControllerStates("AppControllerStates");
    newAppControllerStates.addState(State(kAPPCONTROLLER_INIT, "kAPPCONTROLLER_INIT"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_PLAY, "kAPPCONTROLLER_PLAY"));
    
    // add them to the model
    appModel->addStateGroup(newAppControllerStates);
    
    // get them back from the model so that changes go live
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    
    // load the config file
    
    appModel->load("config", ARCHIVE_BINARY);
//    appModel->backup("config", ARCHIVE_BINARY);
    
//    appModel->removeAllProperties();
    
//    appModel->setProperty("LogToFile", false);
//    
//    appModel->setProperty("VerticalSync", true);
//    appModel->setProperty("Ortho", true);
//    
//    appModel->setProperty("MediaPath", (string)"/Users/gameover/Desktop/LOTE/TESTRENDERS/media");
    appModel->setProperty("NumberPlayers", 11);
    appModel->setProperty("RectTrail", 200);
//
//    appModel->setProperty("ForceFileListUpdate", false);
//    appModel->setProperty("ForceFileListCheck", false);
//    appModel->setProperty("CheckKeyFrames", false);
//    appModel->setProperty("CheckXMP", false);
//    appModel->setProperty("CheckRects", false);
//
//    appModel->setProperty("ContourMinArea", 10);
//    appModel->setProperty("ContourMaxArea", 1200);
//    appModel->setProperty("ContourThreshold", 11);
//    
//    appModel->setProperty("VideoWidth", 550.0f);
//    appModel->setProperty("VideoHeight", 550.0f);
    appModel->setProperty("TransitionLength", 12);
//    
//    appModel->setProperty("OutputWidth", 1920.0f);
//    appModel->setProperty("OutputHeight", 1080.0f);
    
    ofxLogSetLogToFile(appModel->getProperty<bool>("LogToFile"), ofToDataPath("log_" + ofGetTimestampString() + ".log"));
    
    appModel->loadWindowPositions("WindowPositions.txt");
//    appModel->loadForwardMotionGraph("ForwardMotionGraph.txt");
//    appModel->loadBackwardMotionGraph("BackwardMotionGraph.txt");
//    appModel->loadDirectionGraph("DirectionGraph.txt");
    
    // create appView windows
    appView = new AppView();
    appView->setup(appModel->getProperty<float>("OutputWidth"),
                   appModel->getProperty<float>("OutputHeight"),
                   ViewOption(),
                   (string)"output");
    
    // make a debug window
    debugView = new DebugView();
    debugView->setup(ofGetWidth(),
                     ofGetHeight(),
                     ViewOption(),
                     (string)"debug");
    
    analyzeController = new AnalyzeController();
    analyzeController->setup();
    
    playController = new PlayController();
    playController->setup();
    
//    networkController = new NetworkController();
//    networkController->setup();
    
    
//    CGPoint p;
//    p.x = 1980 * 2; p.y = 1200;
//    CGPostMouseEvent( p, 1, 1, 1 );
//    CGPostMouseEvent( p, 1, 1, 0 );
    
    //ofHideCursor();
    //ofSetFullscreen(true);
    
    ofBackground(0, 0, 0);
    ofSetVerticalSync(appModel->getProperty<bool>("VerticalSync"));
    
    // register key/mouse events (this way we get them from any window)
    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);
    
    appControllerStates.setState(kAPPCONTROLLER_INIT);
    
    //    appModel->setProperty("warp0_x", 0.0f);
    //    appModel->setProperty("warp0_y", 0.0f);
    
    //    appModel->setProperty("warp1_x", 0.0f);
    //    appModel->setProperty("warp1_y", 0.0f);
    
    //    w0x = appModel->getProperty<float>("warp0_x");
    //    w0y = appModel->getProperty<float>("warp0_y");
    //    w1x = appModel->getProperty<float>("warp1_x");
    //    w1y = appModel->getProperty<float>("warp1_y");
    //
    //    // setup the warp grid for this appView window
    //    BezierWarp & warp = appView->getWarp<BezierWarp>();
    //    warp.setWarpGrid(5, 4);
    //    warp.setWarpGridResolution(ceil(1920.0/80.0f), ceil(1080.0/80.0f));
    //
    //    // load the control points for this appView Window
    //    string pntPropName = "warpPoints_"+ofToString(screen);
    //    if(appModel->hasProperty< vector<float> >(pntPropName)){
    //        warp.setControlPoints(appModel->getProperty< vector<float> >(pntPropName));
    //    }
    
    //    appModel->removeAllProperties();
    //    appModel->setProperty("warpPoints_0", appViews[0]->getWarp<BezierWarp>().getControlPoints());
    //    appModel->setProperty("warpPoints_1", appViews[1]->getWarp<BezierWarp>().getControlPoints());
    //    appModel->setProperty("warp0_x", w0x);
    //    appModel->setProperty("warp0_y", w0y);
    //    appModel->setProperty("warp1_x", w1x);
    //    appModel->setProperty("warp1_y", w1y);
    
}

//--------------------------------------------------------------
void AppController::update(){
    
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    
    switch (appControllerStates.getState()) {
            
        case kAPPCONTROLLER_INIT:
        {
            analyzeController->update();
            if(analyzeControllerStates.getState(kANALYZECONTROLER_DONE)) appControllerStates.setState(kAPPCONTROLLER_PLAY);
        }
            break;
        case kAPPCONTROLLER_PLAY:
        {
            playController->update();
        }
            break;
    }
    
}

//--------------------------------------------------------------
void AppController::draw(){
    
    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
//    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
    appView->update();
    
    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->update();
    
    switch (appControllerStates.getState()) {
        case kAPPCONTROLLER_INIT:
        {
            // nothing to do
        }
            break;
        case kAPPCONTROLLER_PLAY:
        {
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);
            appView->draw();
            
        }
            break;
    }
    
    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->draw();
    
    ofDisableBlendMode();
    
//    glFlush();
    
}

//--------------------------------------------------------------
void AppController::exit(){
    appModel->save("config", ARCHIVE_BINARY);
}

//--------------------------------------------------------------
void AppController::keyPressed(ofKeyEventArgs & e){
    
    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
//    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
//    BezierWarp & warp0 = appViews[0]->getWarp<BezierWarp>();
//    BezierWarp & warp1 = appViews[1]->getWarp<BezierWarp>();
    
        
//    ofxLogVerbose() << e.key << endl;
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch(e.key) {
        case 'd':
            debugViewStates.toggleState(kDEBUGVIEW_SHOWINFO);
            break;
        case 'p':
            debugViewStates.toggleState(kDEBUGVIEW_SHOWPROPS);
            break;
        case 's':
            debugViewStates.toggleState(kDEBUGVIEW_SHOWSTATES);
            break;
        case 'c':
            appView->resetCamera();
            break;
        case 'r':
            appViewStates.toggleState(kAPPVIEW_SHOWRECTS);
            break;
        case 'w':
            appViewStates.toggleState(kAPPVIEW_SHOWWINDOWS);
            break;
        case 'a':
            if(appModel->getProperty<int>("RectTrail") == 200){
                appModel->setProperty("RectTrail", 1000000);
            }else{
                appModel->setProperty("RectTrail", 200);
            }
            
            break;
        case 'o':
            appView->toggleCameraOrtho();
            break;
        case ' ':
        {
            vector<PlayerController*> & players = appModel->getPlayers();
            //players[0]->generatePossibleMotions(players[0]->getCurrentMovieInfo());
            
//            players[0]->generateMoviesBetween("STND_FRNT", "CLIM_UPPP");
//            vector<string> alphabet = appModel->getForwardMotionGraph().getAlphabet();
//            string m1 = alphabet[(int)ofRandom(alphabet.size())];
//            string m2 = alphabet[(int)ofRandom(alphabet.size())];
//            players[0]->generateMoviesBetween(m1, m2, true);
//            players[0]->generateMoviesBetween(m1, m2, false);
            
            
            
            
//            motions.push_back("STND_FRNT");
//            players[0]->generateMotionsBetween("STND_FRNT", "WALK_LEFT", true, motions);
//            players[0]->generateMotionsBetween("WALK_LEFT", "CLIM_UPPP", true, motions);
//            players[0]->generateMotionsBetween("CLIM_UPPP", "FALL_BACK", true, motions);
//            
//            players[0]->generateMoviesFromMotions(motions);
//            
            
//            motions.clear();
//
            
            vector<int> windows;
            windows.push_back(0);
            windows.push_back(2);
            windows.push_back(3);
            windows.push_back(4);
            windows.push_back(5);
            windows.push_back(6);
            windows.push_back(7);
            windows.push_back(9);
            windows.push_back(10);
            windows.push_back(11);
            windows.push_back(13);
            
            for(int i = 0; i < players.size(); i++){
                
                vector<string> motions;
                
                int windowIndex = ofRandom(windows.size());
                int window = windows[windowIndex];
                eraseAt(windows, windowIndex);
                
                vector<string> movements;
                
                switch (window) {
                    case 0:
                    case 2:
                        movements.push_back("WALK_RIGT");
                        movements.push_back("TRAV_RIGT");
                        movements.push_back("CLIM_UPPP");
                        movements.push_back("CLIM_DOWN");
                        movements.push_back("CRWL_RIGT");
                        movements.push_back("CREP_RIGT");
                        break;
                    case 3:
                    case 5:
                        movements.push_back("WALK_RIGT");
                        movements.push_back("TRAV_RIGT");
                        movements.push_back("CLIM_UPPP");
                        movements.push_back("CLIM_DOWN");
                        movements.push_back("CRWL_RIGT");
                        movements.push_back("CREP_RIGT");
                        break;
                    case 4:
                        movements.push_back("CLIM_UPPP");
                        movements.push_back("CLIM_DOWN");
                        break;
                    case 6:
                        movements.push_back("TRAV_LEFT");
                        movements.push_back("TRAV_RIGT");
                        movements.push_back("CLIM_UPPP");
                        movements.push_back("CLIM_DOWN");
                        break;
                    case 7:
                    case 9:
                    case 10:
                    case 11:
                    case 13:
                        movements.push_back("CLIM_UPPP");
                        movements.push_back("CLIM_DOWN");
                        break;
                    default:
                        break;
                }
                
                
                vector<string> ends;
                ends.push_back("HUGG_FRNT");
                ends.push_back("HUGG_FRNT");
                
                string m1 = movements[(int)ofRandom(movements.size())];
                string e1 = ends[(int)ofRandom(ends.size())];
                
                vector<string> mParts = ofSplitString(m1, "_");
                
                ofPoint startPosition = ofPoint(windowPositions[window].x + windowPositions[window].width / 2.0f,
                                                windowPositions[window].y, 0.0f);
                
                players[i]->setTargetWindow(windowPositions[window], window);
                

                float target;
                if(mParts[1] == "RIGT") target = windowPositions[window].x;
                if(mParts[1] == "LEFT") target = 200 + (ofGetWidth() - windowPositions[window].x);
                if(mParts[1] == "DOWN") target = windowPositions[window].y;
                if(mParts[1] == "UPPP") target = 200 + (ofGetHeight() - windowPositions[window].y);
                
                float distance = target - 1;
                int inserts = 0;
                
                cout << "INSERTRECT0: " << distance << " " << target << endl;

                motions.push_back("STND_FRNT");
                players[i]->generateMotionsBetween("STND_FRNT", m1, true, motions);
                motions.push_back(m1);
                
                while(distance < target){
                    inserts++;
                    motions.push_back(m1);

                    players[i]->clearChains();
                    players[i]->generateMoviesFromMotions(motions, true);

                    vector<ofRectangle>& pRectangles = players[i]->getPredictedChainRects();
                    ofRectangle startRectangle = pRectangles[0];
                    ofRectangle endRectangle = pRectangles[pRectangles.size() - 1];
                    
                    if(mParts[1] == "RIGT") distance = ABS(endRectangle.position.x - startRectangle.position.x);
                    if(mParts[1] == "LEFT") distance = ABS(endRectangle.position.x - startRectangle.position.x);
                    if(mParts[1] == "DOWN") distance = ABS(endRectangle.position.y - startRectangle.position.y);
                    if(mParts[1] == "UPPP") distance = ABS(endRectangle.position.y - startRectangle.position.y);
                    
                    cout << "INSERTRECT" << inserts << ": " << distance << " " << target << endl;
                }
                
                players[i]->generateMotionsBetween(m1, e1, true, motions);
                players[i]->generateMoviesFromMotions(motions, true);
                
                if(e1 == "HUGG_FRNT") motions.push_back("STND_FRNT");
                
                players[i]->clearChains();
                players[i]->generateMoviesFromMotions(motions, true);

                vector<ofPoint>& pPositions = players[i]->getPredictedChainPositions();
                ofPoint endPoint = pPositions[pPositions.size() - 1];
                players[i]->setPredictedFrameGoal(pPositions.size() - 1);
                
                if(e1 == "HUGG_FRNT"){
                    motions.clear();
                    motions.push_back("STND_FRNT");
                    if(m1 == "WALK_RIGT"){
                        m1 = "WALK_LEFT";
                    }else if(m1 == "WALK_LEFT"){
                        m1 = "WALK_RIGT";
                    }
                    if(m1 == "CREP_RIGT"){
                        m1 = "CREP_LEFT";
                    }else if(m1 == "CREP_LEFT"){
                        m1 = "CREP_RIGT";
                    }
                    if(m1 == "CRWL_RIGT"){
                        m1 = "CRWL_LEFT";
                    }else if(m1 == "CRWL_LEFT"){
                        m1 = "CRWL_RIGT";
                    }
                    if(m1 == "TRAV_LEFT"){
                        m1 = "TRAV_RIGT";
                    }else if(m1 == "TRAV_RIGT"){
                        m1 = "TRAV_LEFT";
                    }
                    if(m1 == "CLIM_UPPP"){
                        m1 = "CLIM_DOWN";
                    }else if(m1 == "CLIM_DOWN"){
                        m1 = "CLIM_UPPP";
                    }
                    players[i]->generateMotionsBetween("STND_FRNT", m1, true, motions);
                    motions.push_back(m1);
                    for(int j = 0; j < inserts; j++){
                        motions.push_back(m1);
                    }
                    players[i]->generateMotionsBetween(m1, "FALL_BACK", true, motions);
                    
                    players[i]->generateMoviesFromMotions(motions, true);

                }
                
                players[i]->setPosition(startPosition - endPoint);
                players[i]->normalisePredictedChains(startPosition);
                

            }
            
            
            
        }
            
            break;
        case 'm':
            for(int i = 0; i < players.size(); i++) players[i]->setPaused(false);
            
            break;
        case 'x':
        {
            playControllerStates.setState(kPLAYCONTROLLER_MAKE);
        }
            break;
//        case OF_KEY_LEFT:
//        {
//            for(int i = 0; i < players.size(); i++){
//                players[i]->getModel().setDirectionLEFT();
//            }
//        }
//            break;
//            
//        case OF_KEY_RIGHT:
//        {
//            for(int i = 0; i < players.size(); i++){
//                players[i]->getModel().setDirectionRIGHT();
//            }
//        }
//            break;
//        case OF_KEY_UP:
//        {
//            for(int i = 0; i < players.size(); i++){
//                players[i]->getModel().setDirectionUP();
//            }
//        }
//            break;
//            
//        case OF_KEY_DOWN:
//        {
//            for(int i = 0; i < players.size(); i++){
//                players[i]->getModel().setDirectionDOWN();
//            }
//        }
//            break;
//        case '/':
//        {
//            for(int i = 0; i < players.size(); i++){
//                players[i]->getModel().setDirectionEVENUP();
//            }
//        }
//            break;
            
	}
    
}

//--------------------------------------------------------------
void AppController::keyReleased(ofKeyEventArgs & e){
    //    ofxLogVerbose() << e.key << endl;
}

//--------------------------------------------------------------
void AppController::mouseMoved(ofMouseEventArgs & e){
    //    ofxLogVerbose() << e.x << " " << e.y << " " << e.button << endl;
//    appModel->setProperty("mouseX", e.x);
//    appModel->setProperty("mouseY", e.y);
}

//--------------------------------------------------------------
void AppController::mouseDragged(ofMouseEventArgs & e){
    //    ofxLogVerbose() << e.x << " " << e.y << " " << e.button << endl;
    
//    if(e.button != 2) return;
//    
//    float diffX = e.x - mouseDownX;
//    float diffY = e.y - mouseDownY;
//    
//    BezierWarp & warp0 = appViews[0]->getWarp<BezierWarp>();
//    BezierWarp & warp1 = appViews[1]->getWarp<BezierWarp>();
//    
//    if(warp0.getShowWarpGrid()){
//        w0x = appModel->getProperty<float>("warp0_x") + diffX;
//        w0y = appModel->getProperty<float>("warp0_y") + diffY;
//        warp0.setWarpGridPosition(w0x, w0y, warp0.getWidth(), warp0.getHeight());
//    }
//    
//    if(warp1.getShowWarpGrid()){
//        w1x = appModel->getProperty<float>("warp1_x") + diffX;
//        w1y = appModel->getProperty<float>("warp1_y") + diffY;
//        warp1.setWarpGridPosition(w1x + 1920.0f * (ofGetWidth()/2.0f) / 1920.0f, w1y, warp1.getWidth(), warp1.getHeight());
//    }
    
}

//--------------------------------------------------------------
void AppController::mousePressed(ofMouseEventArgs & e){
    //    ofxLogVerbose() << e.x << " " << e.y << " " << e.button << endl;
    
//    mouseDownX = e.x;
//    mouseDownY = e.y;
}

//--------------------------------------------------------------
void AppController::mouseReleased(ofMouseEventArgs & e){
    //    ofxLogVerbose() << e.x << " " << e.y << " " << e.button << endl;
//    if(e.button != 2) return;
//    
//    BezierWarp & warp0 = appViews[0]->getWarp<BezierWarp>();
//    BezierWarp & warp1 = appViews[1]->getWarp<BezierWarp>();
//    
//    if(warp0.getShowWarpGrid()){
//        appModel->setProperty("warp0_x", w0x);
//        appModel->setProperty("warp0_y", w0y);
//    }
//    
//    if(warp1.getShowWarpGrid()){
//        appModel->setProperty("warp1_x", w1x);
//        appModel->setProperty("warp1_y", w1y);
//    }
//    
//    mouseDownX = mouseDownY = 0;
}