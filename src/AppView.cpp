//
//  AppView.cpp
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#include "AppView.h"

//--------------------------------------------------------------
AppView::AppView(){
    ofxLogNotice() << "Constructing AppView" << endl;
    
    // register AppController states
    StateGroup newAppViewStates("AppViewStates", false);
    newAppViewStates.addState(State(kAPPVIEW_SHOWWINDOWS, "kAPPVIEW_SHOWWINDOWS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWPLAYERS, "kAPPVIEW_SHOWPLAYERS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWRECTS, "kAPPVIEW_SHOWRECTS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWCENTRES, "kAPPVIEW_SHOWCENTRES"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWWARP, "kAPPVIEW_SHOWWARP"));
    
    // add them to the model
    appModel->addStateGroup(newAppViewStates);
    
    // get them back from the model so that changes go live
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    appViewStates.setState(kAPPVIEW_SHOWWINDOWS, false);
    appViewStates.setState(kAPPVIEW_SHOWPLAYERS, true);
    appViewStates.setState(kAPPVIEW_SHOWRECTS, false);
    appViewStates.setState(kAPPVIEW_SHOWCENTRES, false);
    appViewStates.setState(kAPPVIEW_SHOWWARP, false);
    
    resetCamera();
    
}

//--------------------------------------------------------------
AppView::~AppView(){
    ofxLogNotice() << "Destroying AppView" << endl;
}

//--------------------------------------------------------------
void AppView::resetCamera(){
    
    cout << cam.getX() << " " << cam.getY() << " " << cam.getZ() << " " << cam.getRoll() << " " << cam.getPitch() << " " << cam.getHeading() << endl;
    cam.enableMouseInput();
    cam.resetTransform();
    cam.setPosition(0, 5.02717e-05, -575.041);
    setCameraOrtho(appModel->getProperty<bool>("Ortho"));
    cam.tilt(180);
    cam.setTranslationKey('z');
    cout << cam.getX() << " " << cam.getY() << " " << cam.getZ() << " " << cam.getRoll() << " " << cam.getPitch() << " " << cam.getHeading() << endl;
}

//--------------------------------------------------------------
void AppView::setCameraOrtho(bool b){
    if(b){
        cam.enableOrtho();
    }else{
        cam.disableOrtho();
    }
    appModel->setProperty("Ortho", b);
}

//--------------------------------------------------------------
void AppView::toggleCameraOrtho(){
    if(!cam.getOrtho()){
        cam.enableOrtho();
    }else{
        cam.disableOrtho();
    }
    appModel->setProperty("Ortho", cam.getOrtho());
}

//--------------------------------------------------------------
void AppView::update(){
    
    StateGroup & analyzeControllerStates = appModel->getStateGroup("AnalyzeControllerStates");

    if(analyzeControllerStates.getState(kANALYZECONTROLER_ANALYZE)){
        ofVideoPlayer & analysisVideo = appModel->getAnalysisVideo();
        ofRectangle & analysisRectangle = appModel->getAnalysisRectangle();
        ofxCv::ContourFinder& analysisContourFinder = appModel->getAnalysisContourFinder();
        begin();
        {
            ofSetBackgroundColor(0, 0, 0, 0);
            ofSetColor(255, 255, 255);
            analysisVideo.draw(0, 0, analysisVideo.getWidth(), analysisVideo.getHeight());
            analysisContourFinder.draw();
            ofNoFill();
            ofSetColor(255, 0, 0);
            ofRect(analysisRectangle);
        }
        end();
        return;
    }
    
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    vector<PlayerController*> & players = appModel->getPlayers();
    
    begin();
    {
        ofSetBackgroundColor(0, 0, 0, 0);
        
        cam.begin();
        
        if(cam.getOrtho()){
            ofTranslate(0, -ofGetHeight());
        }else{
            ofTranslate(-ofGetWidth() / 2.0, -ofGetHeight() / 2.0);
        }
        
        ofEnableBlendMode(OF_BLENDMODE_SCREEN);
        
        /******************************************************
         *******            Draw Windows                *******
         *****************************************************/

        
        ofNoFill();
        ofSetColor(255, 255, 255);
        for(int i = 0; i < windowPositions.size(); i++){
            
            /******************************************************
             *******            Draw WindowInfo             *******
             *****************************************************/
            
            if(appViewStates.getState(kAPPVIEW_SHOWWINDOWS)){
                ostringstream os;
                os << i << ": " << windowPositions[i].x << ", " << windowPositions[i].y << ", " << windowPositions[i].width << ", " << windowPositions[i].height;
                ofDrawBitmapString(os.str(), windowPositions[i].x, windowPositions[i].y - 14);
            }
            ofRect(windowPositions[i]);
        }
        
        
        for(int i = 0; i < players.size(); i++){
            ofNoFill();
            ofSetColor(127, 0, 0);
            
            /******************************************************
             *******            Draw Centres                *******
             *****************************************************/
            
            if(appViewStates.getState(kAPPVIEW_SHOWCENTRES)) ofLine(players[i]->getPlayerCentre(), players[i]->getTargetCentre());
            
            /******************************************************
             *******            Draw Rects                  *******
             *****************************************************/
            
            if(appViewStates.getState(kAPPVIEW_SHOWRECTS)){
                
                ofNoFill();
                ofSetColor(127, 0, 0);
                ofRect(players[i]->getBounding());
                vector<ofRectangle>& chainRects = players[i]->getPredictedChainRects();
                ofSetColor(0, 10, 10);
                for(int j = 0; j < chainRects.size(); j++){
                    ofRect(chainRects[j]);
                }
                
            }
            
            /******************************************************
             *******            Draw TargetWindow           *******
             *****************************************************/
            
            float d = players[i]->getDistanceToTarget() - 100.0f;
            float dT = 200.0f;
            if(d < dT){
                ofFill();
                float c = 127 * CLAMP((1.0 - d / dT), 0.0f, 1.0f);
                ofSetColor(c, c, c);
                ofRect(players[i]->getTargetWindow());
            }
            
        }
        
        
        /******************************************************
         *******            Draw Players                *******
         *****************************************************/
        
        if(appViewStates.getState(kAPPVIEW_SHOWPLAYERS)){
            
            ofSetColor(255, 255, 255);
            ofNoFill();
            
            glPushAttrib(GL_ENABLE_BIT);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            
            for(int i = 0; i < players.size(); i++){
                glPushMatrix();
                
                glTranslatef(players[i]->getPosition().x, players[i]->getPosition().y, players[i]->getPosition().z);
                glScalef(players[i]->getDrawScale(), players[i]->getDrawScale(), 1.0f);
                
                players[i]->getView()->drawParticles();
                
                glPopMatrix();
            }
            
            glPopAttrib();
            
            for(int i = 0; i < players.size(); i++){
                glPushMatrix();
                
                glTranslatef(players[i]->getPosition().x, players[i]->getPosition().y, players[i]->getPosition().z);
                glScalef(players[i]->getDrawScale(), players[i]->getDrawScale(), 1.0f);
                
                players[i]->getView()->drawImage();
                
                glPopMatrix();
            }
            
        }
        
        ofDisableBlendMode();
        
        cam.end();
        
    }
    end();
    
}
