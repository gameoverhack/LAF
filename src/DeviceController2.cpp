//
//  DeviceController2.cpp
//  LaughterForgetting
//
//  Created by gameover on 12/04/14.
//
//

#include "DeviceController2.h"

//--------------------------------------------------------------
DeviceController2::DeviceController2(){
    ofxLogVerbose() << "Creating DeviceController2" << endl;
}

//--------------------------------------------------------------
DeviceController2::~DeviceController2(){
    
    ofxLogVerbose() << "Destroying DeviceController2" << endl;
    
    // stop the threading
    waitForThread();
    stopThread();
    
    // close all connections
    UDPbroadcast.Close();
    
}

//--------------------------------------------------------------
void DeviceController2::setup(){
    
    ofxLogNotice() << "DeviceController2 start setup" << endl;
    
    /******************************************************
     *******                States                  *******
     *****************************************************/

    StateGroup newDeviceControllerControllerStates("DeviceControllerStates");
    newDeviceControllerControllerStates.addState(State(kDEVICECONTROLLER_INIT, "kDEVICECONTROLLER_INIT"));
    newDeviceControllerControllerStates.addState(State(kDEVICECONTROLLER_ERROR, "kDEVICECONTROLLER_ERROR"));
    newDeviceControllerControllerStates.addState(State(kDEVICECONTROLLER_READY, "kDEVICECONTROLLER_READY"));
    
    appModel->addStateGroup(newDeviceControllerControllerStates);
    
    StateGroup & deviceControllerControllerStates = appModel->getStateGroup("DeviceControllerStates");

    deviceControllerControllerStates.setState(kDEVICECONTROLLER_INIT);
    
    // get ip address of the server
    serverIPfull = appModel->getIPAddress();
    
    if(serverIPfull != "0.0.0.0"){
        
        // determine broadcast IP by cropping and adding 255 to the server IP
        serverIProot = serverIPfull.substr(0, serverIPfull.rfind("."));
        serverIPpart = serverIPfull.substr(serverIPfull.rfind(".") + 1, string::npos);
        serverIPbroadcast = serverIProot + ".255";
        
        ofxLogNotice() << "Starting networking for server ip " << serverIPfull << " (" << serverIPpart << ") and broadcast ip " << serverIPbroadcast << endl;
        
        // open UDP broadcast port
        UDPbroadcast.Create();
        UDPbroadcast.Connect(serverIPbroadcast.c_str(), 10001);
        UDPbroadcast.SetEnableBroadcast(true);
        UDPbroadcast.SetNonBlocking(true);
        
        // OPEN COMMUNICATION PORTS FOR UDP, OSC and YARP
        
        // UDP
        UDPmanager.Create();
        if(UDPmanager.Bind(10002)){
            ofxLogNotice() << "Bind UDPmanager on " << serverIPfull << ":10002" << endl;
            UDPmanager.SetNonBlocking(true);
        }
        
        // OSC - need test for these?
        OSCReceiver.setup(10003);
        ofxLogNotice() << "Bind OSCReceiver on " << serverIPfull << ":10003" << endl;
        
        // YARP
        yarp::os::impl::NameConfig yarpNameConfig;
        yarp::os::impl::Address yarpAddress(serverIPfull.c_str(), 10000);
        yarpNameConfig.setAddress(yarpAddress);
        yarpNameConfig.toFile();
        yarpNameConfig.fromFile();
        
        if(YARPReceiver.open("/motionReceiver")){
            ofxLogNotice() << "Bind YARPReceiver on " << serverIPfull << ":10000" << endl;
        }
        
        // set timer
        timerUDPBroadcastPing = ofGetElapsedTimeMillis();
        
        // start threading - non-blocking, non-verbose
        startThread(false, false);                      // QUESTION: maybe blocking and flags is better for this?
        
    }else{
        
        ofxLogError() << "Cannot determine server IP address - not starting the DeviceController thread" << endl;
        deviceControllerControllerStates.setState(kDEVICECONTROLLER_ERROR);
        
    }

}

//--------------------------------------------------------------
void DeviceController2::update(){
    
    if(lock()){
        
        // do stuff
        
        unlock();
    }
    
}

//--------------------------------------------------------------
void DeviceController2::threadedFunction(){
    
    while (isThreadRunning()){
        
        if(lock()){
            
            // get devices from the appmodel
            map<int, DeviceClient>& devices = appModel->getAllDevices();
            
            // broadcast (ping) the ip address of the server allowing auto connection
            // on DHCP host for any IP addresses of both client and server
            if(ofGetElapsedTimeMillis() - timerUDPBroadcastPing > appModel->getProperty<int>("PingBroadcast")){
                ofxLogVerbose() << "UDP broadcast ping on ip: " << serverIPbroadcast << endl;
                string msg = "S_" + serverIPpart;
                UDPbroadcast.Send(msg.c_str(), msg.size());
                timerUDPBroadcastPing = ofGetElapsedTimeMillis();
            }
            
            char udpBroadcastMessageChar[1024];
            UDPbroadcast.Receive(udpBroadcastMessageChar, 1024);
            string udpBroadcastMessageStr = udpBroadcastMessageChar;
            
            if(udpBroadcastMessageStr != ""){
                
                ofxLogVerbose() << "UDP Broadcast Message: " << udpBroadcastMessageStr << endl;
                
                vector<string> command = ofSplitString(udpBroadcastMessageStr, "_");
                
                if(command[0] == "C"){
                    
                    ofxLogNotice() << "Client connected at: " << serverIProot << "." << command[1] << endl;
                    
                    int clientID = ofToInt(command[1]);
                    
                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    if(it == devices.end()){
                        ofxLogNotice() << "Creating client device with ID: " << clientID << endl;
                    }else{
                        ofxLogWarning() << "Replacing client device with ID: " << clientID << endl;
                    }

                    // create the device
                    DeviceClient d;
                    devices[clientID] = d;
                    DeviceClient& client = devices[clientID];
                    client.accelerationBuffer.resize(50, 3);
                    client.attitudeBuffer.resize(50, 3);
                    client.positionBuffer.resize(50, 3);
                    client.kalmanFilter.setup(2, 3);
                    client.clientID = clientID;
                    client.deviceColor = generateRandomColor();
                }
                
            }
            
            while(OSCReceiver.hasWaitingMessages()){
                
                ofxOscMessage m;
                OSCReceiver.getNextMessage(&m);
                
                if(m.getAddress() == "/device"){
                    
                    int clientID = m.getArgAsInt32(0);

                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    if(it != devices.end()) it->second.push(m);
                    
                }
                
            }
            
            if(YARPReceiver.getPendingReads()){
                
                yarp::os::Bottle *input = YARPReceiver.read();
                
                if(input->get(0).toString() == "/device"){
                    
                    int clientID = input->get(1).asInt();
                    
                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    if(it != devices.end()) it->second.push(input);
                    
                }
                
            }
            
            unlock();
        }
        
    }
    
}