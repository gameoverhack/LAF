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
    newAppViewStates.addState(State(kAPPVIEW_MAKEWINDOWS, "kAPPVIEW_MAKEWINDOWS"));
    
    appModel->addStateGroup(newAppViewStates);
    
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    appViewStates.setState(kAPPVIEW_SHOWWINDOWS, false);
    appViewStates.setState(kAPPVIEW_SHOWPLAYERS, true);
    appViewStates.setState(kAPPVIEW_SHOWRECTS, true);
    appViewStates.setState(kAPPVIEW_SHOWCENTRES, true);
    appViewStates.setState(kAPPVIEW_SHOWINFO, true);
    appViewStates.setState(kAPPVIEW_SHOWWARP, false);
    appViewStates.setState(kAPPVIEW_MAKEWINDOWS, false);
    
    resetCamera();
    
}

//--------------------------------------------------------------
AppView::~AppView(){
    ofxLogNotice() << "Destroying AppView" << endl;
}

//--------------------------------------------------------------
void AppView::resetCamera(){
    
    videoFBOBig.allocate(appModel->getProperty<float>("VideoWidth"), appModel->getProperty<float>("VideoHeight"));
    videoFBOSmall.allocate(appModel->getProperty<float>("DrawSize"), appModel->getProperty<float>("DrawSize"));
    
    windowFades.resize(appModel->getWindows().size());
    
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
    
    
    if(appViewStates.getState(kAPPVIEW_MAKEWINDOWS)){
        
        
        return;
    }
    
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
         *******              Draw Heros                *******
         *****************************************************/
        
        ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
        
        float iY = 1.0f;
        
        if(hero != NULL){
            
            hero->update();
            
            float fY = hero->getFade() * hero->getHeight();
            float dY = (hero->getHeight() - fY) / 2.0;
            float cY = hero->getFade()*255;
            iY = (1 - hero->getFade());
            
            ofNoFill();
            ofSetColor(cY, cY, cY);
            
            
            hero->getTextureReference().drawSubsection(0, dY, hero->getWidth(), fY, 0, dY);
            ofRect(1, dY + 1, hero->getWidth() - 2, fY - 2);
            
            if(hero->getIsMovieDone() && hero->getQueueSize() == 0){
                appModel->stopHereo();
                appModel->resetHeroTimer();
            }
            
        }
        
        /******************************************************
         *******            Draw Windows                *******
         *****************************************************/

        
        ofNoFill();
        
        for(int i = 0; i < windowPositions.size(); i++){
            ofSetColor(255 * iY, 255 * iY, 255 * iY);
            
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
         *******            Draw Sequences              *******
         *****************************************************/
        
        
            
        vector<MovieSequence*>& sequences = appModel->getSequences();
        
        for(int i = 0; i < sequences.size(); i++){
            
            ofNoFill();
            ofSetColor(255, 255, 255);
            
            MovieSequence* sequence = sequences[i];
            MovieInfo& currentMovie = sequence->getCurrentMovie();
            ofxThreadedVideo* video = sequence->getVideo();
            
            /******************************************************
             *******            Calculate Fades             *******
             *****************************************************/
            
            // TODO: this is all a terrible mess -> should be a cue system a la ofxThreadedVideoFade...but on the sequence...grrrr...
            
            // centres
            ofRectangle& windowRect = windowPositions[sequence->getWindow()];
            ofPoint wC = windowRect.getCenter(); // need to cache?
            float distance = sequence->getScaledCentre().distance(wC);
            float maxDistance = sequence->getScaledCentreAt(1).distance(wC);
            
            float pct1 = 1.0f;
            float pct2 = 1.0f;
            float pct3 = 1.0f;
            float pct4 = 1.0f;
            float pct5 = 1.0f;
            float sFade = 0.0f;
            float bFade = 0.0f;
            float cFade = 0.0f;
            float iFade = 0.0f;
            float tFade = 0.0f;
            float iSmal = 0.0f;
            float framesFromStart = (25.0f * appModel->getProperty<int>("FadeTime") * sequence->getSpeed());
            float framesFromEnd = sequence->getTotalSequenceFrames() - (25.0f * appModel->getProperty<int>("FadeTime") * sequence->getSpeed());
            float framesFromSync = sequence->getSyncFrame() - (25.0f * appModel->getProperty<int>("SyncTime") * sequence->getSpeed());
            
            if(sequence->getCurrentSequenceFrame() < framesFromStart){
                
                pct1 = ( (float)sequence->getCurrentSequenceFrame() / framesFromStart );
                pct2 = pct1;
                pct3 = 1.0f;
                pct5 = pct1;
                //cout << i << " start " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
            }
            
            if(sequence->getCurrentSequenceFrame() >= framesFromEnd){
                
                pct1 = (float)(sequence->getTotalSequenceFrames() - sequence->getCurrentSequenceFrame()) / (sequence->getTotalSequenceFrames() - framesFromEnd);
                pct2 = pct1;
                pct3 = 1.0f;
                //cout << i << " end  " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
                
                if(!sequence->getHug()){
                    pct3 = 1.0 - pct1;
                    pct2 = 0.0f;
                    pct1 = 1.0f;
                }
            }

            if(sequence->getCurrentSequenceFrame() > framesFromStart && sequence->getCurrentSequenceFrame() < framesFromEnd){
                
                pct5 = pct3 = pct2 = ((distance - 200) / (maxDistance / 2.0) );
                //cout << i << " dist " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
            }
            
            if(sequence->getCurrentSequenceFrame() >= framesFromSync){
                
                pct4 = (float)(sequence->getSyncFrame() - sequence->getCurrentSequenceFrame()) / (sequence->getSyncFrame() - framesFromSync);
                pct5 = 0.0f;
                //cout << i << " sync " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
                
            }else{
                
                pct4 = pct1;
                //cout << i << " synb " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
            }
 
            
            
            sFade = 255 * CLAMP((      pct1), 0.0f, 1.0f) * iY;
            bFade = 255 * CLAMP((      pct4), 0.0f, 1.0f) * iY;
            cFade = 127 * CLAMP((1.0 - pct3), 0.0f, 1.0f) * iY;
            iFade = 127 * CLAMP((      pct2), 0.0f, 1.0f) * iY;
            tFade = 127 * CLAMP((      pct5), 0.0f, 1.0f) * iY;
            iSmal = 8   * CLAMP((      pct2), 0.0f, 1.0f) * iY;

            
            windowFades[sequence->getWindow()].cFade += cFade;
            windowFades[sequence->getWindow()].numPlayers ++;
            
            /******************************************************
             *******            Small Draw Players          *******
             *****************************************************/
            
            videoFBOSmall.begin();
            {
                ofClear(0, 0, 0);
                video->draw(0, 0, video->getWidth() * sequence->getNormalScale(), video->getHeight() * sequence->getNormalScale());
            }
            videoFBOSmall.end();
            
            ofSetColor(sFade, sFade, sFade);
            videoFBOSmall.draw(sequence->getScaledPosition().x, sequence->getScaledPosition().y);
            
            if(appViewStates.getState(kAPPVIEW_SHOWPLAYERS)){
                
                /******************************************************
                 *******            Big Draw Players            *******
                 *****************************************************/
                
                videoFBOBig.begin();
                {
                    ofClear(0, 0, 0, 0);
                    video->draw(0, 0, video->getWidth(), video->getHeight());
                }
                videoFBOBig.end();

                ofSetColor(bFade, bFade, bFade);
                videoFBOBig.draw(sequence->getPosition().x, sequence->getPosition().y);
            }
            
            /******************************************************
             *******              Draw Rects                *******
             *****************************************************/
            
            if(appViewStates.getState(kAPPVIEW_SHOWRECTS)){

                // window
//                ofFill();
//                ofSetColor(cFade, cFade, cFade);
//                ofRect(windowRect);
                ofNoFill();
                
                //big player bounding etc
                ofSetColor(0, tFade, tFade);
                ofRect(sequence->getTotalBounding());
//
//                ofSetColor(fFade / 2.0, 0, 0);
//                ofRect(sequence->getBounding());
//                ofCircle(sequence->getCentre(), 4);
                
                
                
                // small player bounding etc
                ofSetColor(iFade * 2, 0, 0);
                ofRect(sequence->getScaledBounding());
                ofCircle(sequence->getScaledCentre(), 4);
                
                ofSetColor(0, iFade, 0);
                ofLine(sequence->getScaledCentre(), wC);
                
                ofSetColor(iFade, 0, iFade / 1.5);
                ofRect(sequence->getScaledTotalBounding());
                
                int range = appModel->getProperty<int>("RectTrail");
                int startFrame = MAX(sequence->getCurrentSequenceFrame() - range, 0);
                int endFrame = MIN(sequence->getCurrentSequenceFrame() + range, sequence->getTotalSequenceFrames());
                
                for(int j = startFrame; j < endFrame; j++){
                    ofSetColor(0, iSmal, iSmal);
                    ofRect(sequence->getScaledBoundingAt(j));
                    //ofRect(sequence->getBoundingAt(j));
                }
                
            }

        }
        
        for(int i = 0; i < windowFades.size(); i++){
            ofRectangle& windowRect = windowPositions[i];
            
            if(windowFades[i].numPlayers > 0){
                
                float cFade = windowFades[i].cFade / (float)windowFades[i].numPlayers;
//                cout << i << " " << windowFades[i].cFade << " " << windowFades[i].numPlayers << " " << cFade << endl;
                
                ofFill();
                ofSetColor(cFade, cFade, cFade);
                ofRect(windowRect);
                ofNoFill();
            }
            windowFades[i].cFade = windowFades[i].numPlayers = 0;
        }
        
        ofDisableBlendMode();
        
        cam.end();
        

    }
    end();
    
}
