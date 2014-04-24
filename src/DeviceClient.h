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
#include "PhilippeModel.h"
#include "Agent2.h"
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
    char control;
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
    
    DeviceClient(){
        clientID = -1;
        bButton = false;
        timeThisMessage = timeLastMessage = fps = frameRate = 0;
    }
    ~DeviceClient(){
        deassociate();
    }
    
    void push(ofxOscMessage& oscData){
        
        // process an osc message
        
        DeviceMessage dm;
        
        dm.clientID =       oscData.getArgAsInt32(0);
        dm.deviceType =     (DeviceType)oscData.getArgAsInt32(1);
        dm.serverType =     (ServerType)oscData.getArgAsInt32(2);
        dm.timestamp =      oscData.getArgAsInt32(3);
        dm.accelerationX =  oscData.getArgAsFloat(4);
        dm.accelerationY =  oscData.getArgAsFloat(5);
        dm.accelerationZ =  oscData.getArgAsFloat(6);
        dm.rotationX =      oscData.getArgAsFloat(7);
        dm.rotationY =      oscData.getArgAsFloat(8);
        dm.rotationZ =      oscData.getArgAsFloat(9);
        dm.attitudeX =      oscData.getArgAsFloat(10);
        dm.attitudeY =      oscData.getArgAsFloat(11);
        dm.attitudeZ =      oscData.getArgAsFloat(12);
        dm.gravityX =       oscData.getArgAsFloat(13);
        dm.gravityY =       oscData.getArgAsFloat(14);
        dm.gravityZ =       oscData.getArgAsFloat(15);
        dm.uaccelerationX = oscData.getArgAsFloat(16);
        dm.uaccelerationY = oscData.getArgAsFloat(17);
        dm.uaccelerationZ = oscData.getArgAsFloat(18);
        
        push(dm);
        
    };
    
    void push(yarp::os::Bottle *yarpData){
        
        // process a yarp message
        
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
    
    void push(DeviceMessage dm){
        
        // cache this message
        lastDeviceMessage = dm;
        
        // convert to ofPoint
        ofPoint lastAttitudeRaw = ofPoint(dm.attitudeX, dm.attitudeY, dm.attitudeZ);
        ofPoint lastUserAccelerationRaw = ofPoint(dm.uaccelerationX, dm.uaccelerationY, dm.uaccelerationZ);

        // do kalman filtering on attitude v acceleration
        vector<ofPoint> measurement;
        measurement.push_back(lastAttitudeRaw);
        measurement.push_back(lastUserAccelerationRaw);
        kalmanFilter.setMeasured(measurement);
        cv::Mat k = kalmanFilter.getCorrected();
        
        // get results in ofPoint
        ofPoint lastAttitudeKalman = toOf(cv::Point3f(k.at<float>(0), k.at<float>(1), k.at<float>(2)));
        ofPoint lastUserAccelerationKalman = toOf(cv::Point3f(k.at<float>(3), k.at<float>(4), k.at<float>(5)));

        // calculate and cache position TODO: add resistance to position
        lastPositionKalman.x = ofMap(-lastAttitudeKalman.z, ofDegToRad(-35), ofDegToRad(35), 0.0f, ofGetWidth());
        lastPositionKalman.y = ofMap(-lastAttitudeKalman.x, ofDegToRad(-15), ofDegToRad(15), 0.0f, ofGetHeight());
        //lastPositionKalman.z = ofMap(-lastAttitudeKalman.y, ofDegToRad(-35), ofDegToRad(35), 0.0f, ofGetHeight());
        
        // cache attitude, acceleration and position in ringbuffers
        attitudeBuffer.push(lastAttitudeRaw);
        accelerationBuffer.push(lastUserAccelerationKalman);
        positionBuffer.push(lastPositionKalman);
        
        // and send to OSCSender -> philippe
        ofxOscSender& OSCSender = philModel->getOSCSender();

        ofxOscMessage m;
        
        ofPoint& pF = positionBuffer.getFrontAsPoint();
        ofPoint& tF = attitudeBuffer.getFrontAsPoint();
        ofPoint& aF = accelerationBuffer.getFrontAsPoint();
        
        ofPoint& pD = positionBuffer.getDifferenceAsPoint();
        float aN = positionBuffer.getFlowAngle();
        FlowDirection fD = positionBuffer.getFlowDirection();
        
        m.setAddress("/device");
        
        m.addIntArg(clientID);
        
        m.addFloatArg(pD.length());
        
        m.addFloatArg(pD.x);
        m.addFloatArg(pD.y);
        m.addFloatArg(pD.z);
        
        m.addFloatArg(aN);
        m.addIntArg((int)fD);
        
        m.addFloatArg(pF.x);
        m.addFloatArg(pF.y);
        m.addFloatArg(pF.z);
        
        m.addFloatArg(tF.x);
        m.addFloatArg(tF.y);
        m.addFloatArg(tF.z);
        
        m.addFloatArg(aF.x);
        m.addFloatArg(aF.y);
        m.addFloatArg(aF.z);
        
        OSCSender.sendMessage(m);
        
        // calculate frameRate
        timeThisMessage = ofGetElapsedTimef();
        double diff = timeThisMessage - timeLastMessage;
        if( diff  > 0.00001 ){
            fps			= 1.0 / diff;
            frameRate	*= 0.9f;
            frameRate	+= 0.1f*fps;
        }
        timeLastMessage	= timeThisMessage;
        
    }
    
    void deassociate(Agent2* agent){
        agent->setDeviceID(-1);
        eraseAll(agents, agent);
    }
    
    void deassociate(){
        for(int i = 0; i < agents.size(); i++){
            Agent2* agent = agents[i];
            cout << "Device deassociating: " << clientID << " to " << agent->getAgentID() << endl;
            agent->setDeviceID(-1);
        }
        agents.clear();
    }
    
    bool associate(Agent2* agent){
        int agentDeviceID = agent->getDeviceID();
        if(agentDeviceID == -1){
            agent->setDeviceID(clientID);
            cout << "Device associating: " << clientID << " to " << agent->getAgentID() << endl;
            agents.push_back(agent);
        }else{
            cout << "Device already associated: " << agentDeviceID << endl;
        }
    }
    
    friend inline ostream& operator<<(ostream& os, DeviceClient& dc);
    
    RingBuffer attitudeBuffer;
    RingBuffer accelerationBuffer;
    RingBuffer positionBuffer;
    
    ofPoint lastPositionKalman;
    
    DeviceMessage lastDeviceMessage;
    
    DeviceType deviceType;
    ServerType serverType;
    int clientID;
    
    ofColor deviceColor;
    
    Kalman kalmanFilter;
    
    int timeLastPing;
    
    double timeThisMessage, timeLastMessage, fps, frameRate;
    
    ofPoint cursor;
    bool bButton, lButton;
    int flowTimeout;
    
    vector<Agent2*> agents;
};

inline ostream& operator<<(ostream& os, DeviceMessage& dm){
    os << "at: (" << dm.attitudeX << ", " << dm.attitudeY << ", " << dm.attitudeZ << ")";
    return os;
}

inline ostream& operator<<(ostream& os, DeviceClient& dc){
    os << dc.clientID << "  " << dc.frameRate << "  " << dc.lastDeviceMessage << " opt: " << dc.positionBuffer.getDifferenceAsPoint() << " t: " << dc.positionBuffer.getFlowAngle();
    return os;
}

#endif
