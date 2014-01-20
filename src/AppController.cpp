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
    
    /******************************************************
     *******                States                  *******
     *****************************************************/
    
    StateGroup newAppControllerStates("AppControllerStates");
    newAppControllerStates.addState(State(kAPPCONTROLLER_INIT, "kAPPCONTROLLER_INIT"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_PLAY, "kAPPCONTROLLER_PLAY"));
    newAppControllerStates.addState(State(kAPPCONTROLLER_MAKEWINDOWS, "kAPPCONTROLLER_MAKEWINDOWS"));
    
    appModel->addStateGroup(newAppControllerStates);
    
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    
    /******************************************************
     *******                Config                  *******
     *****************************************************/
    
    appModel->load("config", ARCHIVE_BINARY);
    appModel->backup("config", ARCHIVE_BINARY);
    
    appModel->removeAllProperties();
    
    for(int i = 0; i < 50; i++){
        string prop = "MovieInfo_" + ofToString(i);
        if(appModel->hasProperty<string>(prop)){
            appModel->removeProperty<string>(prop);
        }else{
            break;
        }
    }

    appModel->setProperty("LogToFile", false);
    
    appModel->setProperty("VerticalSync", true);
    appModel->setProperty("Ortho", true);
    
    appModel->setProperty("MediaPath", (string)"/Users/gameover/Desktop/LOTE/TESTRENDERS/media");
    appModel->setProperty("NumberPlayers", 8);
    appModel->setProperty("RectTrail", 200);

    appModel->setProperty("ForceFileListUpdate", false);
    appModel->setProperty("ForceFileListCheck", false);
    appModel->setProperty("CheckKeyFrames", false);
    appModel->setProperty("CheckXMP", false);
    appModel->setProperty("CheckRects", false);

    appModel->setProperty("ContourMinArea", 10);
    appModel->setProperty("ContourMaxArea", 1200);
    appModel->setProperty("ContourThreshold", 11);
    
    appModel->setProperty("VideoWidth", 550.0f);
    appModel->setProperty("VideoHeight", 550.0f);
    appModel->setProperty("DrawSize", 200.0f);
    
    appModel->setProperty("TransitionLength", 12);
    
    appModel->setProperty("OutputWidth", 1920.0f);
    appModel->setProperty("OutputHeight", 666.0f);
    
    appModel->setProperty("AnalysePlayers", 0);
    appModel->setProperty("AnalyseName", (string)"");
    
    appModel->setProperty("AutoGenerate", true);
    appModel->setProperty("DistanceThreshold", 200.0f);
    appModel->setProperty("FadeTime", 5);
    appModel->setProperty("SyncTime", 2);
    appModel->setProperty("HeroTime", 80000);
    appModel->setProperty("HeroFade", 10000);
    
    ofxLogSetLogToFile(appModel->getProperty<bool>("LogToFile"), ofToDataPath("log_" + ofGetTimestampString() + ".log"));
    
    appModel->loadWindowPositions("WindowPositions.txt");
    appModel->setGraph("ForwardMotionGraph.txt");
    appModel->setGraph("BackwardMotionGraph.txt");
    appModel->setGraph("DirectionGraph.txt");
    appModel->setGraph("TargetGraph.txt");
    appModel->setGraph("EndGraph.txt");
    
    /******************************************************
     *******                Views                   *******
     *****************************************************/
    
    // create appView windows
    appView = new AppView();
    appView->setup(appModel->getProperty<float>("OutputWidth"),
                   appModel->getProperty<float>("OutputHeight"),
                   ViewOption(), //VIEW_USE_FBO
                   (string)"output");
    
    // make a debug window
    debugView = new DebugView();
    debugView->setup(ofGetWidth(),
                     ofGetHeight(),
                     ViewOption(),
                     (string)"debug");
    
    /******************************************************
     *******               Controllers              *******
     *****************************************************/
    
    analyzeController = new AnalyzeController();
    analyzeController->setup();
    
    playController = new PlayController();
    playController->setup();
    
    /******************************************************
     *******              Mouse/Screen              *******
     *****************************************************/
    
//    CGPoint p;
//    p.x = 1980 * 2; p.y = 1200;
//    CGPostMouseEvent( p, 1, 1, 1 );
//    CGPostMouseEvent( p, 1, 1, 0 );
    
    //ofHideCursor();
    //ofSetFullscreen(true);
    
    int& currentObject = appModel->getCurrentMouseObject();
    resize = false;
    currentObject = -1;
    offsetX = offsetY = 0.0f;
    
    ofBackground(0, 0, 0);
    ofSetVerticalSync(appModel->getProperty<bool>("VerticalSync"));
    
    // register key/mouse events (this way we get them from any window)
    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);
    
    appControllerStates.setState(kAPPCONTROLLER_INIT);
    
}

//--------------------------------------------------------------
void AppController::update(){
    
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");
    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
    switch (appControllerStates.getState()) {
            
        case kAPPCONTROLLER_INIT:
        {
            analyzeController->update();
            if(analyzeControllerStates.getState(kANALYZECONTROLER_DONE)) appControllerStates.setState(kAPPCONTROLLER_PLAY);
        }
            break;
        case kAPPCONTROLLER_MAKEWINDOWS:
        {
            playController->update();
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
    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
    
    appView->update();
    
    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->update();
    
    switch (appControllerStates.getState()) {
        case kAPPCONTROLLER_INIT:
        {
            // nothing to do
        }
            break;
        case kAPPCONTROLLER_MAKEWINDOWS:
        case kAPPCONTROLLER_PLAY:
        {
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);


            appView->draw();

        }
            break;
    }
    
    if(debugViewStates.getState(kDEBUGVIEW_SHOWINFO)) debugView->draw();
    
    ofDisableBlendMode();
    
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
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    
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
        case 'i':
            appViewStates.toggleState(kAPPVIEW_SHOWINFO);
            appViewStates.toggleState(kAPPVIEW_SHOWCENTRES);
            break;
        case 'b':
            appViewStates.toggleState(kAPPVIEW_SHOWPLAYERS);
            break;
        case 'e':
            if(appModel->getProperty<int>("RectTrail") == 200){
                appModel->setProperty("RectTrail", 1000000);
            }else{
                appModel->setProperty("RectTrail", 200);
            }
            
            break;
        case 'o':
            appView->toggleCameraOrtho();
            break;
        case 'a':
        {
            bool bAuto = appModel->getProperty<bool>("AutoGenerate");
            appModel->setProperty("AutoGenerate", !bAuto);
        }
            break;
        case ' ':
        {
            if(playControllerStates.getState(kPLAYCONTROLLER_PLAY)){
                playControllerStates.setState(kPLAYCONTROLLER_STOP);
            }else{
                playControllerStates.setState(kPLAYCONTROLLER_MAKE);
            }
        }
            
            break;
        case 'n':
        {
            appModel->activateHero();
        }
            break;
        case 'm':
        {
            appModel->stopHereo();
        }
            break;
        case 'x':
        {
            if(appControllerStates.getState(kAPPCONTROLLER_PLAY)){
                appViewStates.setState(kAPPVIEW_MAKEWINDOWS, true);
                playControllerStates.setState(kPLAYCONTROLLER_STOP);
                appControllerStates.setState(kAPPCONTROLLER_MAKEWINDOWS);
            }else{
                appViewStates.setState(kAPPVIEW_MAKEWINDOWS, false);
                playControllerStates.setState(kPLAYCONTROLLER_MAKE);
                appControllerStates.setState(kAPPCONTROLLER_PLAY);
                
                // show me the windows
                ofxLogNotice() << "NEW WINDOWS" << endl;
                vector<MouseObj>& mouseObjects = appModel->getMouseObjects();
                for(int i = 0; i < mouseObjects.size(); i++){
                    cout << i << "  " << mouseObjects[i].getPosition().x << ", " << mouseObjects[i].getPosition().y << ", " << mouseObjects[i].getWidth() << ", " << mouseObjects[i].getHeight() << endl;
                }
            }
        }
            break;
//        case OF_KEY_LEFT:
//        {
//        }
//            break;
//            
//        case OF_KEY_RIGHT:
//        {
//        }
//            break;
//        case OF_KEY_UP:
//        {
//        }
//            break;
//            
//        case OF_KEY_DOWN:
//        {
//        }
//            break;
//        case '/':
//        {
//        }
//            break;
            
	}
    
}

//--------------------------------------------------------------
void AppController::keyReleased(ofKeyEventArgs & e){

}

//--------------------------------------------------------------
void AppController::mouseMoved(ofMouseEventArgs & e){

}

//--------------------------------------------------------------
void AppController::mouseDragged(ofMouseEventArgs & e){
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    if(!appControllerStates.getState(kAPPCONTROLLER_MAKEWINDOWS)) return;
    int& currentObject = appModel->getCurrentMouseObject();
    vector<MouseObj>& mouseObjects = appModel->getMouseObjects();
    if(currentObject != -1 && currentObject < mouseObjects.size()){ // sanity check
        if(!resize) mouseObjects[currentObject].setPosition(e.x + offsetX, e.y + offsetY);
        if(resize) mouseObjects[currentObject].setSize(e.x + offsetX, e.y + offsetY);
    }
}

//--------------------------------------------------------------
void AppController::mousePressed(ofMouseEventArgs & e){
    StateGroup & appControllerStates = appModel->getStateGroup("AppControllerStates");
    if(!appControllerStates.getState(kAPPCONTROLLER_MAKEWINDOWS)) return;
    int& currentObject = appModel->getCurrentMouseObject();
    vector<MouseObj>& mouseObjects = appModel->getMouseObjects();
    KeyModifiers& keyModifiers = appModel->getKeyModifiers();
    currentObject = -1;
    
    for(int i = 0; i < mouseObjects.size(); i++){
        if(mouseObjects[i].inside(e.x, e.y)){
            currentObject = i;
            break;
        }
    }
    if(currentObject == -1){
        // make a new one
        MouseObj m = MouseObj(e.x, e.y, 50, 50);
        mouseObjects.push_back(m);
        currentObject = mouseObjects.size() - 1;
    }else{
        // we're inside one so calculate the offset
        ofPoint p = mouseObjects[currentObject].getPosition();
        float w = mouseObjects[currentObject].getWidth();
        float h = mouseObjects[currentObject].getHeight();
        ofRectangle r = ofRectangle(p.x + w - 5, p.y + h - 5, 5, 5);
        
        if(keyModifiers.getAppleControlModifier()){
            mouseObjects.erase(mouseObjects.begin() + currentObject);
            currentObject = -1;
            offsetX = offsetY = 0;
        }else{
            if(r.inside(e.x, e.y)){
                resize = true;
                offsetX =  - p.x;
                offsetY =  - p.y;
            }else{
                resize = false;
                offsetX = p.x - e.x;
                offsetY = p.y - e.y;
            }
        }
        
    }
}

//--------------------------------------------------------------
void AppController::mouseReleased(ofMouseEventArgs & e){
    int& currentObject = appModel->getCurrentMouseObject();
    resize = false;
    currentObject = -1;
    offsetX = offsetY = 0.0f;
}