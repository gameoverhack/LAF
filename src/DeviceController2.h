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
#include "PhilippeModel.h"

#include "ofxNetwork.h"
#include "ofxOSC.h"

#include "yarp/os/impl/NameConfig.h"
#include "yarp/os/all.h"

static vector<ofColor> colors;

//--------------------------------------------------------------
static void initColors(){
    colors.clear();
    colors.push_back(ofColor::white);
    colors.push_back(ofColor::gray);
    colors.push_back(ofColor::blue);
    colors.push_back(ofColor::cyan);
    colors.push_back(ofColor::olive);
    colors.push_back(ofColor::gold);
    colors.push_back(ofColor::magenta);
    colors.push_back(ofColor::violet);
}

//--------------------------------------------------------------
static ofColor generateRandomColor(){
    ofColor c;
    if(colors.size() == 0) initColors();
    int index = ofRandom(0, colors.size() - 1);
    c = colors[index];
    colors.erase(colors.begin() + index);
    return c;
}

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
    string clientMode;
    
    // OSC
    ofxOscReceiver OSCReceiver;
    
    // YARP
    yarp::os::Network YARPManager;
    yarp::os::BufferedPort<yarp::os::Bottle> YARPReceiver;
    yarp::os::BufferedPort<yarp::os::Bottle> YARPSender;
    yarp::os::Bottle* YARPBottle;
    string clientYarpMode;
    
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
