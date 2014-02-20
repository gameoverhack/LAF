//
//  AppView.cpp
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#include "AppView.h"
#include "AStarSearch.h"

//--------------------------------------------------------------
AppView::AppView(){
    ofxLogNotice() << "Constructing AppView" << endl;
    
    /******************************************************
     *******                States                  *******
     *****************************************************/

    StateGroup newAppViewStates("AppViewStates", true);
    newAppViewStates.addState(State(kAPPVIEW_NORMAL, "kAPPVIEW_NORMAL"));
    newAppViewStates.addState(State(kAPPVIEW_MAKEWINDOWS, "kAPPVIEW_MAKEWINDOWS"));
    
    appModel->addStateGroup(newAppViewStates);
    
    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
    appViewStates.setState(kAPPVIEW_NORMAL);
    
    resetCamera();
    
#ifdef USE_PRORES
    shader.load(ofToDataPath("yuyvtorgba"));
#endif
    
}

//--------------------------------------------------------------
AppView::~AppView(){
    ofxLogNotice() << "Destroying AppView" << endl;
}

//--------------------------------------------------------------
void AppView::resetCamera(){
    
    videoFBOBig.allocate(appModel->getProperty<float>("VideoWidth"), appModel->getProperty<float>("VideoHeight"));
    videoFBOSmall.allocate(appModel->getProperty<float>("DrawSize"), appModel->getProperty<float>("DrawSize"));
    
#ifdef USE_PRORES
    videoFBOHero.allocate(ofGetWidth(), ofGetHeight());
#endif
    
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
        vector<MouseObj>& mouseObjects = appModel->getMouseObjects();
        int& currentObject = appModel->getCurrentMouseObject();
        for(int i = 0; i < mouseObjects.size(); i++){
            if(currentObject != -1 && currentObject == i){
                ofSetColor(255, 255, 255);
            }else{
                ofSetColor(255, 255, 255);
            }
            mouseObjects[i].draw();
        }
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
        
        float iY = 1.0f;
        
        if(appModel->getProperty<bool>("ShowHeroVideos")){
            
            ofxThreadedVideo* hero = appModel->getCurrentHeroVideo();
            
            if(hero != NULL){
                
                hero->update();
                
                float fY = hero->getFade() * ofGetHeight();
                float dY = (ofGetHeight() - fY) / 2.0;
                float cY = hero->getFade()*255;
                iY = CLAMP((1 - hero->getFade()), 0.0f, 1.0f);

                ofNoFill();
                ofSetColor(cY, cY, cY);
                
#ifdef USE_PRORES
                videoFBOHero.begin();
                ofClear(0, 0, 0);
                videoFBOHero.end();
                shader.begin();
                shader.setUniformTexture("yuvTex", hero->getTextureReference(), 1);
                shader.setUniform1i("conversionType", (true ? 709 : 601));
                shader.setUniform1f("fade", hero->getFade());
                videoFBOHero.getTextureReference().drawSubsection(0, dY, hero->getWidth(), fY, 0, dY);
                shader.end();
#else
                hero->getTextureReference().drawSubsection(0, dY, hero->getWidth(), fY, 0, dY);
#endif

                ofRect(1, dY + 1, hero->getWidth() - 2, fY - 2);
                
                if(hero->getIsMovieDone() && hero->getQueueSize() == 0){
                    appModel->stopHereo();
                    appModel->resetHeroTimer();
                }
                
            }
        }
        
        /******************************************************
         *******            Draw Windows                *******
         *****************************************************/
        
        ofNoFill();
        
        if(appModel->getProperty<bool>("ShowWindowOutline")){
            for(int i = 0; i < windowPositions.size(); i++){
                ofSetColor(255 * iY, 255 * iY, 255 * iY);
                
                if(appModel->getProperty<bool>("ShowWindowInfo")){
                    
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
            
            if(appModel->getProperty<bool>("ShowAvatarsSmall")){
                
                videoFBOSmall.begin();
                {
                    ofClear(0, 0, 0);
                    video->draw(0, 0, video->getWidth() * sequence->getNormalScale(), video->getHeight() * sequence->getNormalScale());
                }
                videoFBOSmall.end();
                
#ifdef USE_PRORES
                shader.begin();
                shader.setUniformTexture("yuvTex", videoFBOSmall.getTextureReference(), 1);
                shader.setUniform1i("conversionType", (true ? 709 : 601));
                shader.setUniform1f("fade", CLAMP(pct1, 0.0f, 1.0f) * iY);
                videoFBOSmall.draw(sequence->getScaledPosition().x, sequence->getScaledPosition().y);
                shader.end();
#else
                ofSetColor(sFade, sFade, sFade);
                videoFBOSmall.draw(sequence->getScaledPosition().x, sequence->getScaledPosition().y);
#endif
                
            }
            
            /******************************************************
             *******            Big Draw Players            *******
             *****************************************************/
            
            if(appModel->getProperty<bool>("ShowAvatarsLarge")){
                
                videoFBOBig.begin();
                {
                    ofClear(0, 0, 0, 0);
                    video->draw(0, 0, video->getWidth(), video->getHeight());
                }
                videoFBOBig.end();
                
#ifdef USE_PRORES
                shader.begin();
                shader.setUniformTexture("yuvTex", videoFBOBig.getTextureReference(), 1);
                shader.setUniform1i("conversionType", (true ? 709 : 601));
                shader.setUniform1f("fade", CLAMP(pct4, 0.0f, 1.0f) * iY);
                videoFBOBig.draw(sequence->getPosition().x, sequence->getPosition().y);
                shader.end();
#else
                ofSetColor(bFade, bFade, bFade);
                videoFBOBig.draw(sequence->getPosition().x, sequence->getPosition().y);
#endif

            }
            
            /******************************************************
             *******            Draw Large Rects            *******
             *****************************************************/
            

            ofNoFill();
            
            if(appModel->getProperty<bool>("ShowTotalBoundsLarge")){
                ofSetColor(0, tFade, tFade);
                ofRect(sequence->getTotalBounding());
            }

//            if(appModel->getProperty<bool>("ShowCurrentBoundsLarge")){
//                ofSetColor(iFade * 2, 0, 0);
//                ofRect(sequence->getBounding());
//                ofCircle(sequence->getCentre(), 4);
//            }
            
            /******************************************************
             *******            Draw Small Rects            *******
             *****************************************************/
            
            if(appModel->getProperty<bool>("ShowTotalBoundsSmall")){
                ofSetColor(iFade, 0, iFade / 1.5);
                ofRect(sequence->getScaledTotalBounding());
            }
            
            if(appModel->getProperty<bool>("ShowCurrentBoundsSmall")){
                if (sequence->getWillCollide())
                    ofSetColor(100,40,40);
                else
                    ofSetColor(0,40,iFade * 2);
                ofRect(sequence->getScaledBounding());
            }
            
            if(appModel->getProperty<bool>("ShowDistanceSmall")){ // Omid
                ofSetColor(0, 100, 0);/*
                ofCircle(sequence->getScaledCentre(), 4);
                ofPoint playCenter = sequence->getScaledCentre();
                ofPoint distanceTrailCenter;
                if (currentMovie.getCurrentDirection() == "LEFT" || currentMovie.getCurrentDirection() == "RIGT" )
                    distanceTrailCenter.set(wC.x, playCenter.y);
                else
                    distanceTrailCenter.set(playCenter.x, wC.y);
                ofLine(sequence->getScaledCentre(), distanceTrailCenter);
                */
                ofSetLineWidth(4.0f);
                if (sequence->getCurrentPath().size() >0)
                    sequence->getCurrentPath().draw();
                ofSetLineWidth(1.0f);
            }
            
            if(appModel->getProperty<bool>("ShowInfoSmall") && appModel->hasProperty<string>("MovieInfo_" + ofToString(sequence->getViewID()))){
                ofSetColor(iFade, iFade, iFade);
                ofDrawBitmapString(appModel->getProperty<string>("MovieInfo_" + ofToString(sequence->getViewID())), sequence->getScaledCentre());
            }
            
            if(appModel->getProperty<bool>("ShowTrailBoundsSmall")){ //Omid
                
                int range = appModel->getProperty<int>("RectTrail");
                int startFrame = MAX(sequence->getCurrentSequenceFrame() - range, 0);
                int endFrame = MIN(sequence->getCurrentSequenceFrame() + range, sequence->getTotalSequenceFrames());
                
                for(int j = startFrame; j < endFrame; j++){
                    if (sequence->getWillCollide())
                        ofSetColor(100,10,10);
                    else
                        ofSetColor(0, iSmal, iSmal);
                    ofRect(sequence->getScaledBoundingAt(j));
                    //ofRect(sequence->getBoundingAt(j));
                }
                
            }

        }
        
        if(appModel->getProperty<bool>("ShowWindowTargets")){
            for(int i = 0; i < windowFades.size(); i++){
                ofRectangle& windowRect = windowPositions[i];
                
                if(windowFades[i].numPlayers > 0){
                    
                    float cFade = windowFades[i].cFade / (float)windowFades[i].numPlayers;
                    
                    ofFill();
                    ofSetColor(cFade, cFade, cFade);
                    ofRect(windowRect);
                    ofNoFill();
                }
                windowFades[i].cFade = windowFades[i].numPlayers = 0;
            }
        }
        
        if(appModel->getProperty<bool>("ShowPathGrid")){
            
            myGraphDescription myGraphClassInstance;
            // Now set that instance and its functions as our function container
            
            ofRectangle biggerEnv;

            
//            biggerEnv.x = 0;
//            biggerEnv.y = 0;
//            biggerEnv.width = appModel->getProperty<float>("OutputWidth");
//            biggerEnv.height = appModel->getProperty<float>("OutputHeight");
            
            biggerEnv.x = -appModel->getProperty<float>("OutputWidth")/2;
            biggerEnv.y = -appModel->getProperty<float>("OutputHeight");
            biggerEnv.width = appModel->getProperty<float>("OutputWidth")*2;
            biggerEnv.height = appModel->getProperty<float>("OutputHeight")*3;
            
            ofSetColor(200, 0, 100);
            ofRect(biggerEnv);
            
             ofSetLineWidth(0.5f);
            ofSetColor(100, 0, 200);
            float gridScale = appModel->getProperty<float>("gridScale");
            for (int i=0;i<biggerEnv.width/gridScale;i++) {
                for (int j=0;j<biggerEnv.height/gridScale;j++) {
                    ofLine(biggerEnv.x, biggerEnv.y+j*gridScale, biggerEnv.width + biggerEnv.x, biggerEnv.y+j*gridScale);
                    ofLine(biggerEnv.x+i*gridScale, biggerEnv.y, biggerEnv.x+i*gridScale, biggerEnv.y+biggerEnv.height);
                }
            }
            
        }

        ofDisableBlendMode();
        
        cam.end();
        

    }
    end();
    
}
