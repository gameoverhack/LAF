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
    
    // register key/mouse events (this way we get them from any window)
    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);
    
    // register AppController states
    StateGroup newAppControllerStates("AppControllerStates");
    newAppControllerStates.addState(State(kAPPCONTROLLER_INIT, "kAPPCONTROLLER_INIT"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_LOAD, "kAPPCONTROLLER_LOAD"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_ANALYZE, "kAPPCONTROLLER_ANALYZE"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_PLAY, "kAPPCONTROLLER_PLAY"));
    
    // add them to the model
    appModel->addStateGroup(newAppControllerStates);
    
    // get them back from the model so that changes go live
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    
    appControllerStates.setState(kAPPCONTROLLER_INIT);
    
    StateGroup newAppViewStates("AppViewStates", false);
    
    newAppViewStates.addState(State(kAPPVIEW_SHOWWARP, "kAPPVIEW_SHOWWARP"));
    
    appModel->addStateGroup(newAppViewStates);
    
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    appViewStates.setState(kAPPVIEW_SHOWWARP, false);
    
    // load the config file
    
    appModel->load("config", ARCHIVE_BINARY);
    
    //    appModel->setProperty("VerticalSync", false);
//    ofSetVerticalSync(appModel->getProperty<bool>("VerticalSync"));
    
    //    appModel->setProperty("LogToFile", false);
//    ofxLogSetLogToFile(appModel->getProperty<bool>("LogToFile"), ofToDataPath("log_" + ofGetTimestampString() + ".txt"));
    
    //    appModel->setProperty("warp0_x", 0.0f);
    //    appModel->setProperty("warp0_y", 0.0f);
    
    //    appModel->setProperty("warp1_x", 0.0f);
    //    appModel->setProperty("warp1_y", 0.0f);
    
//    w0x = appModel->getProperty<float>("warp0_x");
//    w0y = appModel->getProperty<float>("warp0_y");
//    w1x = appModel->getProperty<float>("warp1_x");
//    w1y = appModel->getProperty<float>("warp1_y");
    
//    appModel->setProperty("VideoPath", (string)rootPath + "ANIME60/");
    
    appModel->setProperty("OverrideVideoPath", true);
    appModel->setProperty("ImportClipRects", false);
    
    appModel->setProperty("ContourMinArea", 10);
    appModel->setProperty("ContourMaxArea", 1200);
    appModel->setProperty("ContourThreshold", 11);
    
    appModel->setProperty("VideoWidth", 550.0f);
    appModel->setProperty("VideoHeight", 550.0f);
    
    appModel->setProperty("OutputWidth", 1920.0f);
    appModel->setProperty("OutputHeight", 1080.0f);
    
    
    // create appView windows
//    appView = new AppView();
//    appView->setup(appModel->getProperty<float>("OutputWidth_" + ofToString(screen)),
//                   appModel->getProperty<float>("OutputHeight_" + ofToString(screen)),
//                   ViewOption(VIEW_USE_BEZIERWARP),
//                   (string)"screen_" + ofToString(screen));
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
    
    
    // make a debug window
//    debugView = new DebugView();
//    debugView->setup(ofGetWidth(),
//                     ofGetHeight(),
//                     ViewOption(),
//                     (string)"debug");
//    
//    analyzeView = new AnalyzeView();
//    
//    analyzeView->setup(appModel->getProperty<float>("VideoWidth"),
//                       appModel->getProperty<float>("VideoHeight"),
//                       ViewOption(),
//                       (string)"analyze");
    
    
    // Create other controllers
//    loadController = new LoadController();
//    loadController->setup();
//    
//    analyzeController = new AnalyzeController();
//    analyzeController->setup();
//    
//    playController = new PlayController();
//    playController->setup();
//    
//    networkController = new NetworkController();
//    networkController->setup();
    
    
//    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
//    StateGroup & analyzeViewStates = appModel->getStateGroup("AnalyzeViewStates");
//    debugViewStates.setState(kDEBUGVIEW_SHOWINFO, 0);
//    analyzeViewStates.setState(kANALYZEVIEW_SHOW, 0);
    
    
//    CGPoint p;
//    p.x = 1980 * 2; p.y = 1200;
//    CGPostMouseEvent( p, 1, 1, 1 );
//    CGPostMouseEvent( p, 1, 1, 0 );
    
    //ofHideCursor();
    //ofSetFullscreen(true);
    
    
}

//--------------------------------------------------------------
void AppController::update(){
    
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    
    switch (appControllerStates.getState()) {
            
        case kAPPCONTROLLER_INIT:
        {
            
//            appControllerStates.setState(kAPPCONTROLLER_LOAD);
        }
            break;
        case kAPPCONTROLLER_LOAD:
        {
//            loadController->update();
//            playController->resetClipGroups();
        }
            break;
        case kAPPCONTROLLER_ANALYZE:
        {
//            analyzeController->update();
        }
            break;
        case kAPPCONTROLLER_PLAY:
        {
//            playController->update();
        }
            break;
    }
    
}

//--------------------------------------------------------------
void AppController::draw(){
    
//    StateGroup & analyzeViewStates = appModel->getStateGroup("AnalyzeViewStates");
//    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
//    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
//    appViews[0]->update();
//    appViews[1]->update();
    
//    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->update();
//    if(analyzeViewStates.getState(kANALYZEVIEW_SHOW)) analyzeView->update();
    
    switch (appControllerStates.getState()) {
        case kAPPCONTROLLER_INIT:
        {
            // nothing to do
        }
            break;
        case kAPPCONTROLLER_LOAD:
        {
            // nothing to do
        }
            break;
        case kAPPCONTROLLER_ANALYZE:
        {
            // nothing to do
        }
            break;
        case kAPPCONTROLLER_PLAY:
        {
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);
            
            
        }
            break;
    }
    
//    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->draw();
//    if(analyzeViewStates.getState(kANALYZEVIEW_SHOW)) analyzeView->draw();
    
    ofDisableBlendMode();
    
//    glFlush();
    
}

//--------------------------------------------------------------
void AppController::exit(){
    
}

//--------------------------------------------------------------
void AppController::keyPressed(ofKeyEventArgs & e){
    
//    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
//    StateGroup & analyzeViewStates = appModel->getStateGroup("AnalyzeViewStates");
//    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
//    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
//    BezierWarp & warp0 = appViews[0]->getWarp<BezierWarp>();
//    BezierWarp & warp1 = appViews[1]->getWarp<BezierWarp>();
    
        
    //    ofxLogVerbose() << e.key << endl;
    
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