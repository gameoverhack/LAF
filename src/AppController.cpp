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
    //appModel->backup("config", ARCHIVE_BINARY);
    
//    appModel->removeAllProperties();
    
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
    
    //appModel->setProperty("MediaPath", (string)"/Users/gameoverlf/Desktop/LAF/media");
    appModel->setProperty("MediaPath", (string)"/Users/gameover/Desktop/LOTE/media");
    //appModel->setProperty("MediaPath", (string)"/Volumes/LongingAndForgetting02/LOTE/TESTRENDERS/mediaANIME");
    //appModel->setProperty("MediaPath", (string)"/Users/omid/Desktop/LAF/media");
    appModel->setProperty("NumberPlayers", 10);
    appModel->setProperty("RectTrail", 200);
    
    appModel->setProperty("ForceFileListUpdate", true);
    appModel->setProperty("ForceFileListCheck", false);
    appModel->setProperty("CheckKeyFrames", false);
    appModel->setProperty("CheckXMP", false);
    appModel->setProperty("CheckRects", false);
    appModel->setProperty("ForceCheckRects", false);
    
    appModel->setProperty("ContourMinArea", 10);
    appModel->setProperty("ContourMaxArea", 1200);
    appModel->setProperty("ContourThreshold", 11);
    
    appModel->setProperty("VideoWidth", 550.0f);
    appModel->setProperty("VideoHeight", 550.0f);
    appModel->setProperty("DefaultDrawSize", 100.0f);
    
    appModel->setProperty("TransitionLength", 12);
    
    appModel->setProperty("OutputWidth",    1920.0f);
    appModel->setProperty("OutputHeight", 666.0f);
    
    appModel->setProperty("AnalysePlayers", 0);
    appModel->setProperty("AnalyseName", (string)"");
    
    appModel->setProperty("ManualAgentControl", false);
    appModel->setProperty("AutoGenerate", true);
    
    appModel->setProperty("AvoidCollisions", true);
    appModel->setProperty("DefaultGridScale", 50.0f);  // 40
//    appModel->setProperty("pathBoundingSizeW", 70.0f); // 15
//    appModel->setProperty("pathBoundingSizeH", 110.0f); // 15
    
    appModel->setProperty("DistanceThreshold", 200.0f);
    appModel->setProperty("FadeTime", 0);
    appModel->setProperty("SyncTime", 2);
    appModel->setProperty("HeroTime", 80000);
    appModel->setProperty("HeroFade", 10000);
    
    appModel->setProperty("ShowPathGrid", false);
//    appModel->setProperty("ShowWindowTargets", true);
//    appModel->setProperty("ShowWindowOutline", true);
//    appModel->setProperty("ShowWindowInfo", true);
    appModel->setProperty("ShowAvatarsLarge", false);
//    appModel->setProperty("ShowAvatarsSmall", true);
//    appModel->setProperty("ShowTotalBoundsLarge", true);
//    appModel->setProperty("ShowTotalBoundsSmall", true);
//    //appModel->setProperty("ShowTrailBoundsLarge", true);
//    appModel->setProperty("ShowTrailBoundsSmall", true);
//    //appModel->setProperty("ShowCurrentBoundsLarge", true);
//    appModel->setProperty("ShowCurrentBoundsSmall", true);
//    //appModel->setProperty("ShowDistanceLarge", true);
//    appModel->setProperty("ShowDistanceSmall", true);
//    //appModel->setProperty("ShowInfoLarge", true);
    appModel->setProperty("ShowInfoSmall", false);
    appModel->setProperty("ShowHeroVideos", false);
    
    ofxLogSetLogToFile(appModel->getProperty<bool>("LogToFile"), ofToDataPath("log_" + ofGetTimestampString() + ".log"));
    
    appModel->loadWindowPositions("WindowPositions.txt");
    //appModel->loadWindowPositions("WindowPositions_Reduced.txt"); //Omid
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
    
    deviceController = new DeviceController2();
    deviceController->setup();
    
    /******************************************************
     *******              Mouse/Screen              *******
     *****************************************************/
    
//    CGPoint p;
//    p.x = 1980 * 2; p.y = 1200;
//    CGPostMouseEvent( p, 1, 1, 1 );
//    CGPostMouseEvent( p, 1, 1, 0 );
    
    //ofHideCursor();
    //ofSetFullscreen(true);
    
    ofSetWindowPosition(0, 1080.0/4.0);
    
    int& currentObject = appModel->getCurrentMouseObject();
    resize = false;
    currentObject = -1;
    offsetX = offsetY = 0.0f;
    
    ofBackground(0, 0, 0);
    ofSetFrameRate(60.0);
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
            deviceController->update();
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
            //deviceController->draw(); TODO: move to a view!
            
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
            
        case '1':
            appModel->toggleProperty("ShowAvatarsSmall");
            break;
        case '!':
            appModel->toggleProperty("ShowAvatarsLarge");
            break;
        case '2':
            appModel->toggleProperty("ShowTotalBoundsSmall");
            break;
        case '@':
            appModel->toggleProperty("ShowTotalBoundsLarge");
            break;
        case '3':
            appModel->toggleProperty("ShowTrailBoundsSmall");
            break;
        case '$':
            //appModel->toggleProperty("ShowTrailBoundsLarge");
            break;
        case '5':
            appModel->toggleProperty("ShowCurrentBoundsSmall");
            break;
        case '%':
            //appModel->toggleProperty("ShowCurrentBoundsLarge");
            break;
        case '6':
            appModel->toggleProperty("ShowDistanceSmall");
            break;
        case '^':
            //appModel->toggleProperty("ShowDistanceLarge");
            break;
        case '7':
            appModel->toggleProperty("ShowInfoSmall");
            break;
        case '&':
            //appModel->toggleProperty("ShowInfoLarge");
            break;
        case '8':
            appModel->toggleProperty("ShowHeroVideos");
            break;
        case '*':
            
            break;
        case '9':
            appModel->toggleProperty("ShowWindowTargets");
            break;
        case '(':
            
            break;
        case '0':
            appModel->toggleProperty("ShowWindowOutline");
            break;
        case ')':
            appModel->toggleProperty("ShowWindowInfo");
            break;
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
        case 'g':
        {
            bool bAuto = appModel->getProperty<bool>("ShowPathGrid");
            appModel->setProperty("ShowPathGrid", !bAuto);
        }
            break;
            
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
            vector<MouseObj>& mouseObjects = appModel->getMouseObjects();
            if(appControllerStates.getState(kAPPCONTROLLER_PLAY)){
                if(mouseObjects.size() == 0){
                    vector<ofRectangle>& windows = appModel->getWindows();
                    
                    for(int i = 0; i < windows.size(); i++){
                        MouseObj m;
                        ofRectangle& r = windows[i];
                        m.set(r.x, r.y, r.width, r.height);
                        mouseObjects.push_back(m);
                    }
                }
                appViewStates.setState(kAPPVIEW_MAKEWINDOWS);
                playControllerStates.setState(kPLAYCONTROLLER_STOP);
                appControllerStates.setState(kAPPCONTROLLER_MAKEWINDOWS);
            }else{
                appViewStates.setState(kAPPVIEW_NORMAL);
                //playControllerStates.setState(kPLAYCONTROLLER_MAKE);
                appControllerStates.setState(kAPPCONTROLLER_PLAY);
                
                // show me the windows
                ofxLogNotice() << "NEW WINDOWS" << endl;
                
                for(int i = 0; i < mouseObjects.size(); i++){
                    cout << mouseObjects[i].getPosition().x << "," << mouseObjects[i].getPosition().y << "," << mouseObjects[i].getWidth() << "," << mouseObjects[i].getHeight() << endl;
                }
            }
        }
            break;
        case OF_KEY_LEFT:
        {
            if (appModel->getProperty<bool>("ManualAgentControl"))
                playController->moveAgent((Agent*)appModel->getSequences()[0], 'l');
        }
            break;
            
        case OF_KEY_RIGHT:
        {
            if (appModel->getProperty<bool>("ManualAgentControl"))
                playController->moveAgent((Agent*)appModel->getSequences()[0], 'r');
        }
            break;
        case OF_KEY_UP:
        {
            if (appModel->getProperty<bool>("ManualAgentControl"))
                playController->moveAgent((Agent*)appModel->getSequences()[0], 'u');
        }
            break;
            
        case OF_KEY_DOWN:
        {
            if (appModel->getProperty<bool>("ManualAgentControl"))
                playController->moveAgent((Agent*)appModel->getSequences()[0], 'd');
        }
            break;
        
        case 'z':
        {
            playController->triggerReplan();
        }
            break;
        
        case ']':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(sequence->getSpeed()+0.2);
            }
        }
            break;
            
        case '[':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(sequence->getSpeed()-0.2);
            }
            
        }
            break;

        case 'h':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(-1);
            }
        }
            break;
        case 'j':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(-0.5);
            }
        }
            break;
        case 'k':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(0);
            }
        }
            break;
        case 'l':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(1);
            }
        }
            break;
        case ';':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(3);
            }
        }
            break;
        case '\'':
        {
            vector<MovieSequence*>& sequences = appModel->getSequences();
            for(int i = 0; i < sequences.size(); i++){
                MovieSequence* sequence = sequences[i];
                sequence->setSpeed(6);
            }
        }
            break;
            
//        case '/':
//        {
//            
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