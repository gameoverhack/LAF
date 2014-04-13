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
        if(UDPmanager.Connect(serverIPfull.c_str(), 10002)){
            ofxLogNotice() << "Connected UDPmanager on " << serverIPfull << ":10002" << endl;
            UDPmanager.SetNonBlocking(true);
        }
        
        
        // OSC - need test for these?
        OSCReceiver.setup(10003);
        ofxLogNotice() << "Connected OSCReceiver on " << serverIPfull << ":10003" << endl;
        OSCSender.setup(serverIPfull.c_str(), 10004);
        ofxLogNotice() << "Connected OSCSender on " << serverIPfull << ":10004" << endl;
        
        // YARP
        yarp::os::impl::NameConfig yarpNameConfig;
        yarp::os::impl::Address yarpAddress(serverIPfull.c_str(), 10000);
        yarpNameConfig.setAddress(yarpAddress);
        yarpNameConfig.toFile();
        yarpNameConfig.fromFile();
        
        if(YARPReceiver.open("/motionReceiver")){
            ofxLogNotice() << "Connected YARPReceiver on " << serverIPfull << ":10000" << endl;
            if(YARPSender.open("/mouseEmulator")){
                ofxLogNotice() << "Connected YARPSender on " << serverIPfull << ":10000" << endl;
            }
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
            
            // broadcast (ping) the ip address of the server allowing auto connection
            // on DHCP host for any IP addresses of both client and server
            if(ofGetElapsedTimeMillis() - timerUDPBroadcastPing > appModel->getProperty<int>("PingBroadcast")){
                ofxLogVerbose() << "UDP broadcast ping on ip: " << serverIPbroadcast << endl;
                UDPbroadcast.Send(serverIPpart.c_str(), serverIPpart.size());
                timerUDPBroadcastPing = ofGetElapsedTimeMillis();
            }
            
            unlock();
        }
        
    }
    
}