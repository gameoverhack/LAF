//
//  DeviceView.cpp
//  LaughterForgetting
//
//  Created by gameover on 14/04/14.
//
//

#include "DeviceView.h"

//--------------------------------------------------------------
DeviceView::DeviceView(){
    
    ofxLogNotice() << "Constructing DeviceView" << endl;
    
    /******************************************************
     *******                States                  *******
     *****************************************************/
    
//    StateGroup newAppViewStates("AppViewStates", true);
//    newAppViewStates.addState(State(kAPPVIEW_NORMAL, "kAPPVIEW_NORMAL"));
//    newAppViewStates.addState(State(kAPPVIEW_MAKEWINDOWS, "kAPPVIEW_MAKEWINDOWS"));
//    
//    appModel->addStateGroup(newAppViewStates);
//    
//    StateGroup & appViewStates = appModel->getStateGroup("AppViewStates");
    
}

//--------------------------------------------------------------
DeviceView::~DeviceView(){
    ofxLogNotice() << "Destroying DeviceView" << endl;
}

//--------------------------------------------------------------
void DeviceView::update(){
    
    begin();
    {
        
        appModel->getDeviceMutex().lock();
        
        map<int, DeviceClient>& devices = appModel->getAllDevices();

        for(map<int, DeviceClient>::iterator it = devices.begin(); it != devices.end(); ++it){
            
            DeviceClient& client = it->second;
            
            ofNoFill();
            ofSetColor(client.deviceColor);
            
            ofPoint& pF = client.positionBuffer.getFrontAsPoint();
            ofPoint& pD = client.positionBuffer.getDifferenceAsPoint();
            int pS = client.positionBuffer.size();
            
            ofCircle(pF.x, pF.y, 50);
            
            //ofLine(pF.x, pF.y, pF.x + (((pB.x - pF.x) / pS) * pS), pF.y + (((pB.y - pF.y) / pS) * pS));
            ofLine(pF.x, pF.y, pF.x + pD.x, pF.y + pD.y);
            
        }
        
        appModel->getDeviceMutex().unlock();
        
    }
    end();
    
}