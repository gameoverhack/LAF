//
//  DeviceController2.h
//  LaughterForgetting
//
//  Created by gameover on 12/04/14.
//
//

#ifndef __H_DEVICECONTROLLER2
#define __H_DEVICECONTROLLER2

#include "BaseController.h"
#include "AppModel.h"

#include "ofxNetwork.h"
#include "ofxOSC.h"

#include "yarp/os/impl/NameConfig.h"
#include "yarp/os/all.h"

class DeviceController2 : public BaseController, public ofThread {
    
public:
	
    DeviceController2();
    ~DeviceController2();

    void setup();
    void update();
    
    void threadedFunction();
    
    // COMMUNICATION PROTOCOLS
    
    // UDP
    ofxUDPManager UDPmanager;
    
    // OSC
    ofxOscReceiver OSCReceiver;
    ofxOscSender OSCSender;
    
    // YARP
    yarp::os::Network YARPManager;
    yarp::os::BufferedPort<yarp::os::Bottle> YARPReceiver;
    yarp::os::BufferedPort<yarp::os::Bottle> YARPSender;
    yarp::os::Bottle* YARPBottle;
    
    // UDP broadcast server
    ofxUDPManager UDPbroadcast;
    int timerUDPBroadcastPing;
    
    // ip address storage
    string serverIPfull;
    string serverIProot;
    string serverIPpart;
    string serverIPbroadcast;
    
};

#endif
