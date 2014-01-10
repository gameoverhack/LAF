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
    //appModel->backup("config", ARCHIVE_BINARY);
    
//    appModel->removeAllProperties();
    
//    appModel->setProperty("LogToFile", false);
//    
//    appModel->setProperty("VerticalSync", true);
//    appModel->setProperty("Ortho", true);
//    
//    appModel->setProperty("MediaPath", (string)"/Users/gameover/Desktop/LOTE/TESTRENDERS/media");
//    appModel->setProperty("NumberPlayers", 12);
//    
//    appModel->setProperty("ForceFileListUpdate", false);
//    appModel->setProperty("ForceFileListCheck", false);
//    
//    appModel->setProperty("ContourMinArea", 10);
//    appModel->setProperty("ContourMaxArea", 1200);
//    appModel->setProperty("ContourThreshold", 11);
//    
//    appModel->setProperty("VideoWidth", 550.0f);
//    appModel->setProperty("VideoHeight", 550.0f);
//    
//    appModel->setProperty("OutputWidth", 1920.0f);
//    appModel->setProperty("OutputHeight", 1080.0f);
    
    ofxLogSetLogToFile(appModel->getProperty<bool>("LogToFile"), ofToDataPath("log_" + ofGetTimestampString() + ".log"));
    
//    appModel->loadWindowPositions("WindowPositions.txt");
//    appModel->loadMotionGraph("MotionGraph.txt");
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
        case 'o':
            appView->toggleCameraOrtho();
            break;
        case ' ':
        {
            vector<PlayerController*> & players = appModel->getPlayers();
//            players[0]->getModel().generateAllPossibleTransitions();
        }
            
            break;
        case 'x':
        {
            
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