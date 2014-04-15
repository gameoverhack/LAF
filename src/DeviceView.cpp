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
            ofCircle(client.positionBuffer.frontAsPoint().x, client.positionBuffer.frontAsPoint().y, 50);
        }
        
        appModel->getDeviceMutex().unlock();
        
    }
    end();
    
}