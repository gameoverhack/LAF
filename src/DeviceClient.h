//
//  DeviceClient.h
//  LaughterForgetting
//
//  Created by gameover on 14/04/14.
//
//

#ifndef __H_DEVICECLIENT
#define __H_DEVICECLIENT

#include "ofMain.h"

#include "Kalman.h"
#include "RingBuffer.h"
#include "ofxNetwork.h"
#include "ofxOSC.h"

#include "yarp/os/impl/NameConfig.h"
#include "yarp/os/all.h"

enum DeviceType{
    DEVICETYPE_IPHONE = 0,
    DEVICETYPE_ANDROID
};

enum ServerType{
    SERVERTYPE_MATTG = 0,
    SERVERTYPE_MATTL,
    SERVERTYPE_NORMJ
};

#pragma pack(push, 1)
typedef struct{
    int clientID;
    int deviceType;
    int serverType;
    float accelerationX;
    float accelerationY;
    float accelerationZ;
    float rotationX;
    float rotationY;
    float rotationZ;
    float attitudeX;
    float attitudeY;
    float attitudeZ;
    float gravityX;
    float gravityY;
    float gravityZ;
    float uaccelerationX;
    float uaccelerationY;
    float uaccelerationZ;
} DeviceMessage;
#pragma pack(pop)

typedef union {
    DeviceMessage deviceMessage;
    char data[sizeof(DeviceMessage)];
} DeviceMessageUnion;

class DeviceClient {
    
public:
    
    DeviceClient(){}
    ~DeviceClient(){}
    
    void push(ofxOscMessage& m){
        
    };
    
    void push(char * c){
        DeviceMessageUnion dm;
        for(int i = 0; i < sizeof(DeviceMessage); i++) dm.data[i] = c[i];
        push(dm.deviceMessage);
    };
    
    void push(yarp::os::Bottle *o){
        
    };
    
    void push(DeviceMessage dm){
        
    }
    
    RingBuffer accelerationBuffer;
    RingBuffer attitudeBuffer;
    RingBuffer positionBuffer;
    
    ofPoint lastAccelerationRaw;
    ofPoint lastRotationRaw;
    ofPoint lastAttitudeRaw;
    ofPoint lastGravityRaw;
    ofPoint lastUserAccelerationRaw;
    
    int lastTimeStamp;
    
    float refreshLastTime;
    float refreshAverage;
    
    DeviceType deviceType;
    ServerType serverType;
    int clientID;
    
    ofColor deviceColor;
    
    Kalman kalmanFilter;
    
};

#endif
