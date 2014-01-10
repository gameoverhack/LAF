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
}

//--------------------------------------------------------------
AppView::~AppView(){
    ofxLogNotice() << "Destroying AppView" << endl;
}

//--------------------------------------------------------------
void AppView::update(){
    
//    StateGroup & playControllerStates = appModel->getStateGroup("PlayControllerStates");
//    StateGroup & debugViewStates = appModel->getStateGroup("DebugViewStates");
    
    begin();
    {
        ofSetBackgroundColor(0, 0, 0, 0);
        ofEnableBlendMode(OF_BLENDMODE_SCREEN);

        ofDisableBlendMode();
    }
    end();
    
}