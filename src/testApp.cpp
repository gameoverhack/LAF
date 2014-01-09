#define STRINGIFY(A) #A

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
	ofSetLogLevel(OF_LOG_WARNING);
    ofxLogSetLogLevel(LOG_VERBOSE);
    ofxLogSetLogOptions(LOG_USE_TIME | LOG_USE_CALL | LOG_USE_PADD);
    
    ofBackground(0, 0, 0);
	ofSetVerticalSync(true);
    
    cam.enableMouseInput();
    cam.enableOrtho();
    cam.tilt(180);
    
    bShowInfo = true;
    bShowWindows = true;
    bUseOrtho = true;
    
    appModel->load("config", ARCHIVE_BINARY);

//    appModel->setProperty("mediaPath", (string)"/Users/gameover/Desktop/LOTE/TESTRENDERS");
//    appModel->loadWindowPositions("WindowPositions.txt");
//    appModel->loadMotionGraph("MotionGraph.txt");
//    appModel->loadDirectionGraph("DirectionGraph.txt");
    
    
    for(int i = 0; i < NUM_VIDS; i++){
        appModel->createPlayer("MARTINW");
    }
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    numPlayers = MIN(NUM_VIDS, windowPositions.size());
    uniqueRandomIndex(iRandom, 0, windowPositions.size(), numPlayers);
    iRandom[0] = 5;
    while(!checkRandom()){
        iRandom.clear();
        uniqueRandomIndex(iRandom, 0, windowPositions.size(), numPlayers);
    }

    vector<PlayerController*> & players = appModel->getPlayers();
    
    for(int i = 0; i < players.size(); i++){
        players[i]->getModel().setDrawScale(200.0/550.0);
        players[i]->getModel().setNormalPosition(ofPoint(windowPositions[iRandom[i]].x + windowPositions[iRandom[i]].width / 2.0f,
                                                         windowPositions[iRandom[i]].y, 0.0f));
    }
    
}

//--------------------------------------------------------------
bool testApp::checkRandom(){
    for(int i = 0; i < iRandom.size(); i++){
        if(iRandom[i] == 1 || iRandom[i] == 8 || iRandom[i] == 16 || iRandom[i] == 12) return false;
    }
    return true;
}

//--------------------------------------------------------------
void testApp::exit(){
    appModel->save("config", ARCHIVE_BINARY);
}

//--------------------------------------------------------------
void testApp::update(){

    vector<PlayerController*> & players = appModel->getPlayers();
    
    for(int i = 0; i < players.size(); i++){
        players[i]->update();
    }

}

//--------------------------------------------------------------
void testApp::draw(){

    vector<PlayerController*> & players = appModel->getPlayers();
    
    cam.begin();
    
    if(bUseOrtho){
        ofTranslate(0, -ofGetHeight());
    }else{
        ofTranslate(-ofGetWidth() / 2.0, -ofGetHeight() / 2.0);
    }
    
    
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    
    if(bShowWindows){
        ofNoFill();
        ofSetColor(255, 255, 255);
        for(int i = 0; i < windowPositions.size(); i++){
            ostringstream os;
            os << i << ": " << windowPositions[i].x << ", " << windowPositions[i].y << ", " << windowPositions[i].width << ", " << windowPositions[i].height;
            ofDrawBitmapString(os.str(), windowPositions[i].x, windowPositions[i].y - 14);
            ofRect(windowPositions[i]);
        }
    }
    
    ofPushMatrix();
    
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    for(int i = 0; i < players.size(); i++){
        glPushMatrix();
        
        glTranslatef(players[i]->getPosition().x, players[i]->getPosition().y, players[i]->getPosition().z);
        glScalef(players[i]->getDrawScale(), players[i]->getDrawScale(), 1.0f);
        
        players[i]->getView().drawParticles();
        
        glPopMatrix();
    }
    
    glPopAttrib();
    
    ofPopMatrix();
    ofPushMatrix();
    
    ofEnableBlendMode(OF_BLENDMODE_SCREEN);
    
    for(int i = 0; i < players.size(); i++){
        glPushMatrix();

        glTranslatef(players[i]->getPosition().x, players[i]->getPosition().y, players[i]->getPosition().z);
        glScalef(players[i]->getDrawScale(), players[i]->getDrawScale(), 1.0f);

        players[i]->getView().drawImage();
        
        glPopMatrix();
    }
    
    if(bShowInfo){
        
        for(int i = 0; i < players.size(); i++){
            glPushMatrix();
            ofNoFill();
            ofSetColor(255, 0, 0);
            ofRect(players[i]->getBounding());
            glPopMatrix();
        }
        
        for(int i = 0; i < players.size(); i++){
            glPushMatrix();
            
            MovieInfo& mI = players[i]->getModel().getCurrentMovieInfo();
            
            
            for(int j = mI.frame - mI.startframe; j < mI.predictedBounding.size(); j++){
                ofNoFill();
                ofSetColor(0, 127, 0);
                for(int k = 0; k < windowPositions.size(); k++){
                    if(j + mI.startframe == mI.intersectionFrames[k]){
                        ofFill();
                        ofSetColor(127, 0, 127);
                        break;
                    }
                    //                        if(mI.predictedBounding[j].intersects(windowPositions[k])){
                    //                            ofSetColor(0, 10, 10);
                    //                            break;
                    //                        }
                }
                ofRect(mI.predictedBounding[j]);
            }
            
            for(int l = 0; l < mI.possibleTransitions.size(); l++){
                MovieInfo& pI = mI.possibleTransitions[l];
                
                for(int j = pI.frame - pI.startframe; j < pI.predictedBounding.size(); j++){
                    ofNoFill();
                    ofSetColor(10, 10, 0);
                    for(int k = 0; k < windowPositions.size(); k++){
                        if(j + pI.startframe == pI.intersectionFrames[k]){
                            ofFill();
                            ofSetColor(127, 0, 127);
                            break;
                        }
//                        if(pI.predictedBounding[j].intersects(windowPositions[k])){
//                            ofSetColor(0, 50, 50);
//                            break;
//                        }
                    }
                    
                    ofRect(pI.predictedBounding[j]);
                }
                
                for(int y = 0; y < pI.possibleTransitions.size(); y++){
                    MovieInfo& yI = pI.possibleTransitions[y];
                    
                    for(int j = yI.frame - yI.startframe; j < yI.predictedBounding.size(); j++){
                        ofNoFill();
                        ofSetColor(0, 0, 127);
                        for(int k = 0; k < windowPositions.size(); k++){
                            if(j + yI.startframe == yI.intersectionFrames[k]){
                                ofFill();
                                ofSetColor(127, 0, 127);
                                break;
                            }
//                            if(yI.predictedBounding[j].intersects(windowPositions[k])){
//                                ofSetColor(20, 0, 20);
//                                break;
//                            }
                        }
                        
                        ofRect(yI.predictedBounding[j]);
                    }
                    
                }
                
                for(int l = 0; l < mI.impossibleTransitions.size(); l++){
                    MovieInfo& pI = mI.impossibleTransitions[l];
                    
                    for(int j = pI.frame - pI.startframe; j < pI.predictedBounding.size(); j++){
                        ofNoFill();
                        ofSetColor(0, 10, 10);
                        for(int k = 0; k < windowPositions.size(); k++){
                            if(j + pI.startframe == pI.intersectionFrames[k]){
                                ofFill();
                                ofSetColor(127, 0, 127);
                                break;
                            }
                            //                        if(pI.predictedBounding[j].intersects(windowPositions[k])){
                            //                            ofSetColor(0, 50, 50);
                            //                            break;
                            //                        }
                        }
                        
                        ofRect(pI.predictedBounding[j]);
                    }
                }
                
                
            }
            
            glPopMatrix();
        }
    }
    
    ofSetColor(255, 255, 255);

    
    ofDisableBlendMode();
    
    ofPopMatrix();
    cam.end();
    
    
    ostringstream os;
    os << "FPS: " << ofGetFrameRate() << endl;
    
    if(bShowInfo){

    }

    ofDrawBitmapString(os.str(), 20, 20);
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    vector<ofRectangle> & windowPositions = appModel->getWindows();
    vector<PlayerController*> & players = appModel->getPlayers();
    
    switch(key) {
        case '0':
        case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
            cout << key << endl;
            
            break;
        case ' ':
            
        {
            vector<PlayerController*> & players = appModel->getPlayers();
            players[0]->getModel().generateAllPossibleTransitions();
        }

            break;
        case 'i':
            bShowInfo = !bShowInfo;
            break;
        case 'w':
            bShowWindows = !bShowWindows;
            break;
        case 'o':
            bUseOrtho = !bUseOrtho;
            if(bUseOrtho){
                cam.enableOrtho();
            }else{
                cam.disableOrtho();
            }
            break;
        case 'r':
        {
            
            iRandom.clear();
            uniqueRandomIndex(iRandom, 0, windowPositions.size(), numPlayers);
            while(!checkRandom()){
                iRandom.clear();
                uniqueRandomIndex(iRandom, 0, windowPositions.size(), numPlayers);
            }
            
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDrawScale(200.0/550.0);
                players[i]->getModel().setNormalPosition(ofPoint(windowPositions[iRandom[i]].x + windowPositions[iRandom[i]].width / 2.0f,
                                                                 windowPositions[iRandom[i]].y, 0.0f));
            }
        }
            break;
        case OF_KEY_LEFT:
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDirectionLEFT();
            }
        }
            break;
            
        case OF_KEY_RIGHT:
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDirectionRIGHT();
            }
        }
            break;
        case OF_KEY_UP:
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDirectionUP();
            }
        }
            break;
            
        case OF_KEY_DOWN:
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDirectionDOWN();
            }
        }
            break;
        case '/':
        {
            for(int i = 0; i < players.size(); i++){
                players[i]->getModel().setDirectionEVENUP();
            }
        }
            break;
            
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

