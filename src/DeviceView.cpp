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
        
//        vector<Agent2*>& agents = appModel->getAgents();
//        
//        appModel->getDeviceMutex().lock();
//        
//        map<int, DeviceClient>& devices = appModel->getAllDevices();
//
//        for(map<int, DeviceClient>::iterator it = devices.begin(); it != devices.end(); ++it){
//            
//            DeviceClient& client = it->second;
//            
//            ofNoFill();
//            ofSetColor(client.deviceColor);
//            
//            // get last position and difference from buffer
//            ofPoint& pF = client.positionBuffer.getFrontAsPoint();
//            ofPoint& pD = client.positionBuffer.getDifferenceAsPoint();
//            float angle = client.positionBuffer.getFlowAngle();
//            
//            // draw pointer
//            ofCircle(pF.x, pF.y, 50);
//
//            // draw 'optical flow'
//            ofLine(pF.x, pF.y, pF.x + pD.x, pF.y + pD.y);
//
//            for(int i = 0; i < agents.size(); i++){
//                
//                AgentInfo info = agents[i]->getAgentInfo();
//                ofRectangle r = ofRectangle(pF.x - 50, pF.y - 50, 100, 100);
//
//                if(info.currentBounding.intersects(r)){
//                    ofFill();
//                    ofSetColor(client.deviceColor.r / 2.0, client.deviceColor.g / 2.0, client.deviceColor.b / 2.0);
//                    ofCircle(pF.x, pF.y, 50);
//                    ofNoFill();
//                    client.associate(agents[i]);
//                    break;
//                }
//            }
//            
//            // testing jerk and direction
//            if(pD.length() > 400.0f){
//                
//                cout << "JERK->" << client.positionBuffer.getFlowDirectionAsString() << endl;
//                
//                // and send to OSCSender -> philippe
//                ofxOscSender& OSCSender = philModel->getOSCSender();
//                
//                ofxOscMessage m;
//                m.setAddress("/" + client.positionBuffer.getFlowDirectionAsString());
//                
//                m.addIntArg(client.clientID);
//                m.addFloatArg(pD.length());
//                m.addFloatArg(angle);
//                
//                OSCSender.sendMessage(m);
//                
//                for(int a = 0; a < client.agents.size(); a++){
//                    Agent2* agent = client.agents[a];
//                    if(client.positionBuffer.getFlowDirection() == FLOW_LEFT) agent->move('l');
//                    if(client.positionBuffer.getFlowDirection() == FLOW_RIGHT) agent->move('r');
//                    if(client.positionBuffer.getFlowDirection() == FLOW_UP) agent->move('u');
//                    if(client.positionBuffer.getFlowDirection() == FLOW_DOWN) agent->move('d');
//                }
//                
//            }
//            
//        }
//        
//        appModel->getDeviceMutex().unlock();
        
    }
    end();
    
}