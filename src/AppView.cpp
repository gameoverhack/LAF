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
    
    /******************************************************
     *******                States                  *******
     *****************************************************/

    StateGroup newAppViewStates("AppViewStates", false);
    newAppViewStates.addState(State(kAPPVIEW_SHOWWINDOWS, "kAPPVIEW_SHOWWINDOWS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWPLAYERS, "kAPPVIEW_SHOWPLAYERS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWRECTS, "kAPPVIEW_SHOWRECTS"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWCENTRES, "kAPPVIEW_SHOWCENTRES"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWINFO, "kAPPVIEW_SHOWINFO"));
    newAppViewStates.addState(State(kAPPVIEW_SHOWWARP, "kAPPVIEW_SHOWWARP"));
    
    appModel->addStateGroup(newAppViewStates);
    
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    appViewStates.setState(kAPPVIEW_SHOWWINDOWS, false);
    appViewStates.setState(kAPPVIEW_SHOWPLAYERS, true);
    appViewStates.setState(kAPPVIEW_SHOWRECTS, true);
    appViewStates.setState(kAPPVIEW_SHOWCENTRES, true);
    appViewStates.setState(kAPPVIEW_SHOWINFO, true);
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
//    cam.setPosition(0, 0, 0);
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

    /******************************************************
     *******                Analysis                *******
     *****************************************************/
    
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
        
        for(int i = 0; i < windowPositions.size(); i++){
            ofSetColor(255, 255, 255);
            
            if(appViewStates.getState(kAPPVIEW_SHOWWINDOWS)){
                
                bool bIsTarget = appModel->isWindowTarget(i);
                
                ostringstream os;
                
                os  << i
                    << ": "
                    << windowPositions[i].x << ", "
                    << windowPositions[i].y << ", "
                    << windowPositions[i].width << ", "
                    << windowPositions[i].height;
                
                //if(bIsTarget) os << endl << appModel->getTargetGraph().getPossibleTransitions(ofToString(i));
                
                ofDrawBitmapString(os.str(), windowPositions[i].x, windowPositions[i].y - 14);
                
                if(bIsTarget){
                    ofSetColor(255, 0, 255);
                }else{
                    ofSetColor(255, 255, 255);
                }
            }
            
            ofRect(windowPositions[i]);
        }
        
        /******************************************************
         *******            Draw Players                *******
         *****************************************************/
        
        if(appViewStates.getState(kAPPVIEW_SHOWPLAYERS)){
            
            vector<MovieSequence*>& sequences = appModel->getSequences();
            
            for(int i = 0; i < sequences.size(); i++){
                
                ofNoFill();
                ofSetColor(255, 255, 255);
                
                MovieSequence* sequence = sequences[i];
                MovieInfo& currentMovie = sequence->getCurrentMovie();
                ofxThreadedVideo* video = sequence->getVideo();
                
                video->draw(sequence->getScaledPosition().x, sequence->getScaledPosition().y, 200, 200);
                
                /******************************************************
                 *******              Draw Rects                *******
                 *****************************************************/
                
                if(appViewStates.getState(kAPPVIEW_SHOWRECTS)){

                    video->draw(sequence->getPosition().x, sequence->getPosition().y, video->getWidth(), video->getHeight());
                    
                    ofSetColor(0, 127, 127);
                    ofRect(sequence->getTotalBounding());
                    
                    ofSetColor(127, 0, 127);
                    ofRect(sequence->getScaledTotalBounding());

                    ofSetColor(127, 0, 0);
                    ofRect(sequence->getBounding());
                    ofCircle(sequence->getCentre(), 4);
                    
                    ofSetColor(0, 127, 0);
                    ofRect(sequence->getScaledBounding());
                    ofCircle(sequence->getScaledCentre(), 4);
                    
                }
                
            }
            
        }
        
        ofDisableBlendMode();
        
        cam.end();
        

    }
    end();
    
}
