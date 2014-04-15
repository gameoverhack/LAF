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
            
            // get last position and difference from buffer
            ofPoint& pF = client.positionBuffer.getFrontAsPoint();
            ofPoint& pD = client.positionBuffer.getDifferenceAsPoint();
            float angle = client.positionBuffer.getFlowAngle();
            
            // draw pointer
            ofCircle(pF.x, pF.y, 50);
            
            // draw 'optical flow'
            ofLine(pF.x, pF.y, pF.x + pD.x, pF.y + pD.y);
            
            // testing jerk and direction
            if(pD.length() > 200.0f){
                cout << "JERK->" << client.positionBuffer.getFlowDirectionAsString() << endl;
            }
            
        }
        
        appModel->getDeviceMutex().unlock();
        
    }
    end();
    
}