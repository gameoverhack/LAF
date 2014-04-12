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
    
    // start threading - non-blocking, non-verbose
    // QUESTION: maybe blocking and flags is better for this?
    
    startThread(false, false);
    
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
            
            // do stuff
            
            unlock();
        }
        
    }
}