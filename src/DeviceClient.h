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

using namespace ofxCv;
using namespace cv;

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
    int timestamp;
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
    
    void push(ofxOscMessage& oscData){
        
        cout << "New OSC" << endl;
        
        DeviceMessage dm;
        
        dm.clientID =       oscData.getArgAsInt32(0);
        dm.deviceType =     (DeviceType)oscData.getArgAsInt32(1);
        dm.serverType =     (ServerType)oscData.getArgAsInt32(2);
        dm.timestamp =      oscData.getArgAsInt32(3);
        dm.accelerationX =  oscData.getArgAsFloat(4);
        dm.accelerationY =  oscData.getArgAsFloat(4);
        dm.accelerationZ =  oscData.getArgAsFloat(4);
        dm.rotationX =      oscData.getArgAsFloat(4);
        dm.rotationY =      oscData.getArgAsFloat(4);
        dm.rotationZ =      oscData.getArgAsFloat(4);
        dm.attitudeX =      oscData.getArgAsFloat(4);
        dm.attitudeY =      oscData.getArgAsFloat(4);
        dm.attitudeZ =      oscData.getArgAsFloat(4);
        dm.gravityX =       oscData.getArgAsFloat(4);
        dm.gravityY =       oscData.getArgAsFloat(4);
        dm.gravityZ =       oscData.getArgAsFloat(4);
        dm.uaccelerationX = oscData.getArgAsFloat(4);
        dm.uaccelerationY = oscData.getArgAsFloat(4);
        dm.uaccelerationZ = oscData.getArgAsFloat(4);
        
        push(dm);
        
    };
    
    void push(char * c){
        
        DeviceMessageUnion dm;
        for(int i = 0; i < sizeof(DeviceMessage); i++) dm.data[i] = c[i];
        push(dm.deviceMessage);
        
    };
    
    void push(yarp::os::Bottle *yarpData){
        
        DeviceMessage dm;
        
        dm.clientID =       yarpData->get(1).asInt();
        dm.deviceType =     (DeviceType)yarpData->get(2).asInt();
        dm.serverType =     (ServerType)yarpData->get(3).asInt();
        dm.timestamp =      yarpData->get(4).asInt();
        dm.accelerationX =  yarpData->get(5).asDouble();
        dm.accelerationY =  yarpData->get(6).asDouble();
        dm.accelerationZ =  yarpData->get(7).asDouble();
        dm.rotationX =      yarpData->get(8).asDouble();
        dm.rotationY =      yarpData->get(9).asDouble();
        dm.rotationZ =      yarpData->get(10).asDouble();
        dm.attitudeX =      yarpData->get(11).asDouble();
        dm.attitudeY =      yarpData->get(12).asDouble();
        dm.attitudeZ =      yarpData->get(13).asDouble();
        dm.gravityX =       yarpData->get(14).asDouble();
        dm.gravityY =       yarpData->get(15).asDouble();
        dm.gravityZ =       yarpData->get(16).asDouble();
        dm.uaccelerationX = yarpData->get(17).asDouble();
        dm.uaccelerationY = yarpData->get(18).asDouble();
        dm.uaccelerationZ = yarpData->get(19).asDouble();
        
        push(dm);
        
    };
    
    void push(DeviceMessage& dm){
        
        lastDeviceMessage = dm;
        
        ofPoint lastUserAccelerationRaw = ofPoint(dm.uaccelerationX, dm.uaccelerationY, dm.uaccelerationZ);
        ofPoint lastAttitudeRaw = ofPoint(dm.attitudeX, dm.attitudeX, dm.attitudeX);
        
        vector<ofPoint> measurement;
        measurement.push_back(lastAttitudeRaw);
        measurement.push_back(lastUserAccelerationRaw);
        kalmanFilter.setMeasured(measurement);
        cv::Mat k = kalmanFilter.getCorrected();
        
        ofPoint lastAttitudeKalman = toOf(cv::Point3f(k.at<float>(0), k.at<float>(1), k.at<float>(2)));
        ofPoint lastUserAccelerationKalman = toOf(cv::Point3f(k.at<float>(3), k.at<float>(4), k.at<float>(5)));
        
        ofPoint lastPositionKalman;
        
        lastPositionKalman.x = ofMap(-lastAttitudeKalman.z, ofDegToRad(-35), ofDegToRad(35), 0.0f, ofGetWidth());
        lastPositionKalman.y = ofMap(-lastAttitudeKalman.x, ofDegToRad(-35), ofDegToRad(15), 0.0f, ofGetHeight());
        lastPositionKalman.z = ofMap(-lastAttitudeKalman.y, ofDegToRad(-15), ofDegToRad(35), 0.0f, ofGetHeight());
        
        attitudeBuffer.push(lastAttitudeRaw);
        accelerationBuffer.push(lastUserAccelerationKalman);
        positionBuffer.push(lastPositionKalman);
        
        // calculate frameRate -> taken from ofAppRunner
        prevMillis = ofGetElapsedTimeMillis();
        timeNow = ofGetElapsedTimef();
        double diff = timeNow-timeThen;
        if( diff  > 0.00001 ){
            fps			= 1.0 / diff;
            frameRate	*= 0.9f;
            frameRate	+= 0.1f*fps;
        }
        lastFrameTime	= diff;
        timeThen		= timeNow;
        
        cout << "New DeviceMessage: " << frameRate << endl;
        
    }
    
    RingBuffer attitudeBuffer;
    RingBuffer accelerationBuffer;
    RingBuffer positionBuffer;
    
    DeviceMessage lastDeviceMessage;
    
    DeviceType deviceType;
    ServerType serverType;
    int clientID;
    
    ofColor deviceColor;
    
    Kalman kalmanFilter;
    
    double prevMillis, lastFrameTime, timeNow, timeThen, fps, frameRate;
    
};

#endif
