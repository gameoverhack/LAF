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
    
//    videoFBOBig.allocate(appModel->getProperty<float>("VideoWidth"), appModel->getProperty<float>("VideoHeight"));
//    videoFBOSmall.allocate(appModel->getProperty<float>("DefaultDrawSize"), appModel->getProperty<float>("DefaultDrawSize"));
    
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
         *******            Draw Agents              *******
         *****************************************************/
            
        vector<Agent2*>& agents = appModel->getAgents();
        
        for(int i = 0; i < agents.size(); i++){
            
            ofNoFill();
            ofSetColor(255, 255, 255);
            
            Agent2* agent = agents[i];
            AgentInfo agentInfo = agents[i]->getAgentInfo();
//            MovieInfo currentMovie = agentInfo.currentMovieInfo;
            ofxThreadedVideo* video = agent->getVideo();
            
            /******************************************************
             *******            Calculate Fades             *******
             *****************************************************/
            
            // TODO: this is all a terrible mess -> should be a cue system a la ofxThreadedVideoFade...but on the agent...grrrr...
            
            // centres
            ofRectangle& windowRect = agentInfo.target;//windowPositions[agent->getWindow()];
            ofPoint wC = windowRect.getCenter(); // need to cache?
            float distance = agentInfo.currentCentre.distance(wC);
            float maxDistance = agent->getScaledCentreAt(1).distance(wC);
            
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
            float framesFromStart = (25.0f * appModel->getProperty<int>("FadeTime") * agent->getSpeed());
            float framesFromEnd = agent->getTotalSequenceFrames() - (25.0f * appModel->getProperty<int>("FadeTime") * agent->getSpeed());
            float framesFromSync = agent->getSyncFrame() - (25.0f * appModel->getProperty<int>("SyncTime") * agent->getSpeed());
            
            if(agent->getCurrentSequenceFrame() < framesFromStart){
                
                pct1 = ( (float)agent->getCurrentSequenceFrame() / framesFromStart );
                pct2 = pct1;
                pct3 = 1.0f;
                pct5 = pct1;
                //cout << i << " start " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
            }
            
            if(agent->getCurrentSequenceFrame() >= framesFromEnd){
                
                pct1 = (float)(agent->getTotalSequenceFrames() - agent->getCurrentSequenceFrame()) / (agent->getTotalSequenceFrames() - framesFromEnd);
                pct2 = pct1;
                pct3 = 1.0f;
                //cout << i << " end  " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
                
                if(!agent->getHug()){
                    pct3 = 1.0 - pct1;
                    pct2 = 0.0f;
                    pct1 = 1.0f;
                }
            }

            if(agent->getCurrentSequenceFrame() > framesFromStart && agent->getCurrentSequenceFrame() < framesFromEnd){
                
                pct5 = pct3 = pct2 = ((distance - 200) / (maxDistance / 2.0) );
                //cout << i << " dist " << pct1 << "  " << "   " << pct2 << "   " << pct3 << "   " << pct4 << "   " << pct5 << endl;
            }
            
            if(agent->getCurrentSequenceFrame() >= framesFromSync){
                
                pct4 = (float)(agent->getSyncFrame() - agent->getCurrentSequenceFrame()) / (agent->getSyncFrame() - framesFromSync);
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
            
            
            //windowFades[agent->getWindow()].cFade += cFade;
            //windowFades[agent->getWindow()].numPlayers ++;
            
            /******************************************************
             *******            Small Draw Players          *******
             *****************************************************/
            
            if(appModel->getProperty<bool>("ShowAvatarsSmall") && agentInfo.state != AGENT_INIT && agentInfo.currentPosition.x != 0 && agentInfo.currentPosition.y != 0){
                
                agent->getFboSmall()->begin();
                {
                    ofClear(0, 0, 0);
                    video->draw(0, 0, video->getWidth() * agent->getNormalScale(), video->getHeight() * agent->getNormalScale());
                }
                agent->getFboSmall()->end();
                
#ifdef USE_PRORES
                shader.begin();
                shader.setUniformTexture("yuvTex", agent->getFboSmall()->getTextureReference(), 1);
                shader.setUniform1i("conversionType", (true ? 709 : 601));
                shader.setUniform1f("fade", CLAMP(pct1, 0.0f, 1.0f) * iY);
                agent->getFboSmall()->draw(agentInfo.currentPosition.x, agentInfo.currentPosition.y);
                shader.end();
#else
                ofSetColor(sFade, sFade, sFade);
                agent->getFboSmall()->draw(agentInfo.currentPosition.x, agentInfo.currentPosition.y);
#endif
                
            }
            
            /******************************************************
             *******            Big Draw Players            *******
             *****************************************************/
            
            if(appModel->getProperty<bool>("ShowAvatarsLarge") && agentInfo.state != AGENT_INIT && agentInfo.currentPosition.x != 0 && agentInfo.currentPosition.y != 0){
                
                agent->getFboBig()->begin();
                {
                    ofClear(0, 0, 0, 0);
                    video->draw(0, 0, video->getWidth(), video->getHeight());
                }
                agent->getFboBig()->end();
                
#ifdef USE_PRORES
                shader.begin();
                shader.setUniformTexture("yuvTex", agent->getFboBig()->getTextureReference(), 1);
                shader.setUniform1i("conversionType", (true ? 709 : 601));
                shader.setUniform1f("fade", CLAMP(pct4, 0.0f, 1.0f) * iY);
                agent->getFboBig()->draw(agent->getPosition().x, agent->getPosition().y);
                shader.end();
#else
                ofSetColor(bFade, bFade, bFade);
                agent->getFboBig()->draw(agent->getPosition().x, agent->getPosition().y);
#endif

            }
            
            /******************************************************
             *******            Draw Large Rects            *******
             *****************************************************/
            

            ofNoFill();
            
            if(appModel->getProperty<bool>("ShowTotalBoundsLarge")){
                ofSetColor(0, tFade, tFade);
                ofRect(agent->getTotalBounding());
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
                ofRect(agent->getScaledTotalBounding());
            }
            
            if(appModel->getProperty<bool>("ShowCurrentBoundsSmall")){
                if (agent->getWillCollide())
                    ofSetColor(100,40,40);
                else
                    ofSetColor(0,40,iFade * 2);
                
                if(agent->getScaledBoundings().size() > agent->getCurrentSequenceFrame()) ofRect(agent->getScaledBounding());
                //if(!agent->isAgentLocked())  ofRect(agent->getScaledBounding());
                
                if (agent->getAgentInfo().behaviourMode == BEHAVIOUR_MANUAL)
                    ofRect(agent->actionBounding);
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
               
                ofPolyline pathFromHere; //= ofPolyline(agent->getCurrentPath().getVertices());
                ofPoint point;
                
                int index;
                for (int i=0; i < agent->getMovieSequence().size(); i++)
                    if (agent->getMovieSequence()[i].agentActionIndex == 0) {
                        index = i;
                        break;
                    }
                
                if (agentInfo.state != AGENT_PLAN && agentInfo.behaviourMode == BEHAVIOUR_AUTO) {
                    
                    point = agent->getScaledFloorOffsetAt(agent->getSequenceFrames()[index]);
                    pathFromHere.addVertex(point);
                    
                    
                    for (int a=0;a<agent->actions.size();a++) {
                        if (agent->actions[a].first == 'l')
                            point.x-=agent->actions[a].second;
                        else if (agent->actions[a].first == 'r')
                            point.x+=agent->actions[a].second;
                        else if (agent->actions[a].first == 'u')
                            point.y-=agent->actions[a].second;
                        else if (agent->actions[a].first == 'd')
                            point.y+=agent->actions[a].second;
                        
                        pathFromHere.addVertex(point);
                    }
                    
                    
                }

                //TODO: delelet the traveled path points
//                if (agent->getCurrentMovie().agentActionIndex > 0) {
//                        pathFromHere.getVertices().erase(pathFromHere.getVertices().begin(), pathFromHere.getVertices().begin()+agent->getCurrentMovie().agentActionIndex);
//                }
                
                ofSetLineWidth(3.0f);
                if (pathFromHere.size() >0)
                    pathFromHere.draw();
                
                ofSetColor(100, 10, 0);
                ofSetLineWidth(3.0f);
                if (agent->getCurrentPath().size() >0)
                    agent->getCurrentPath().draw();
                
                ofSetLineWidth(1.0f);
            }
            
            if(appModel->getProperty<bool>("ShowInfoSmall") && appModel->hasProperty<string>("MovieInfo_" + ofToString(agent->getViewID()))){
                ofSetColor(iFade, iFade, iFade);
                ofDrawBitmapString(appModel->getProperty<string>("MovieInfo_" + ofToString(agent->getViewID())), agent->getScaledCentre());
            }
            
            if(appModel->getProperty<bool>("ShowTrailBoundsSmall")){ //Omid
                
                int range = appModel->getProperty<int>("RectTrail");
                int startFrame = MAX(agent->getCurrentSequenceFrame() - range, 0);
                int endFrame = MIN(agent->getCurrentSequenceFrame() + range, agent->getTotalSequenceFrames());
                
                int step = 1;
                
                for(int j = startFrame; j < endFrame; j = j + step){
                    
                    if (agent->getFaultyFlag())
                        ofSetColor(250,250,10);
                    else if (agent->getWillCollide())
                        ofSetColor(100,10,10);
                    else
                        ofSetColor(0, 60, 60);
                        //ofSetColor(0, iSmal, iSmal);
                    
                    if(agent->getScaledBoundings().size() > j) ofRect(agent->getScaledBoundingAt(j));
                    //if(!agent->isAgentLocked()) ofRect(agent->getScaledBoundingAt(j));
                    
                    ofSetColor(60, 10, 60);
                    
                    if (agent->getOrgScaledBoundings().size() > 0) ofRect(agent->getOrgScaledBoundingAt(j));
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
            
            float gridScale = appModel->getProperty<float>("DefaultGridScale");

            ofRectangle biggerEnv;

            biggerEnv.x = 0;//-1*gridScale;
            biggerEnv.y = 0;//-1*gridScale;
            biggerEnv.width = appModel->getProperty<float>("OutputWidth") + 1*gridScale;
            biggerEnv.height = appModel->getProperty<float>("OutputHeight") + 1*gridScale;
            

//            biggerEnv.x = -appModel->getProperty<float>("OutputWidth")/2;
//            biggerEnv.y = -appModel->getProperty<float>("OutputHeight");
//            biggerEnv.width = appModel->getProperty<float>("OutputWidth")*2;
//            biggerEnv.height = appModel->getProperty<float>("OutputHeight")*3;
            
            ofSetColor(200, 0, 100);
            ofRect(biggerEnv);
            
             ofSetLineWidth(0.5f);
            ofSetColor(100, 0, 200);
            
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
