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

class DeviceController2 : public BaseController, public ofThread {
    
public:
	
    DeviceController2();
    ~DeviceController2();

    void setup();
    void update();
    
    void threadedFunction();
    
    ofxTCPServer commandServer;
    ofxTCPClient commandClient;
    
};

#endif
