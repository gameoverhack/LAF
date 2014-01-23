//
//  DeviceController.cpp
//  protoApp
//
//  Created by gameover on 26/08/13.
//
//

#include "DeviceController.h"

using namespace ofxCv;
using namespace cv;

static int lastTime = ofGetElapsedTimeMillis();

void DeviceController::setup() {
    
    oscReceiver.setup(6666);
    oscSender.setup("192.168.42.101", 8000);
    
    bNormConnected = false;
    
    ofBackground(0, 0, 0);
    
}

void DeviceController::update() {
    
    while(oscReceiver.hasWaitingMessages()){
        
        ofxOscMessage m;
		oscReceiver.getNextMessage(&m);
        
        //cout << m.getAddress() << " " << m.getNumArgs() << endl;
        
        if(m.getAddress() == "/reset"){
            
            int clientID = m.getArgAsInt32(0);
            DeviceType deviceType = (DeviceType)m.getArgAsInt32(1);
            ServerType serverType = (ServerType)m.getArgAsInt32(2);
            
            ofLogVerbose() << "Reset: "
            << clientID << " "
            << getDeviceTypeAsString(deviceType) << " "
            << getServerTypeAsString(serverType);
            
            createClient(clientID, deviceType, serverType);
            
            DeviceClient& client = devices[clientID];
            
            client.accelerationHistoryRaw.clear();
            client.rotationHistoryRaw.clear();
            client.timeHistory.clear();
            client.accelerationHistoryKalman.clear();
            client.rotationHistoryKalman.clear();
            
        }
        
        if(m.getAddress() == "/record"){
            
            int clientID = m.getArgAsInt32(0);
            DeviceType deviceType = (DeviceType)m.getArgAsInt32(1);
            ServerType serverType = (ServerType)m.getArgAsInt32(2);
            int timestamp = m.getArgAsInt32(3);
            bool bRecord = (bool)m.getArgAsInt32(4);
            
            ofLogVerbose() << "Record: "
            << clientID << " "
            << getDeviceTypeAsString(deviceType) << " "
            << getServerTypeAsString(serverType) << " "
            << timestamp << " "
            << (bRecord ? "START RECORD" : "STOP RECORD");
            
            createClient(clientID, deviceType, serverType);
            
            DeviceClient& client = devices[clientID];
            OSCRecorder& recorder = recorders[clientID];
            
            if(!bRecord) recorder.save();
            recorder.setRecordingOSC(bRecord);

            client.accelerationHistoryRaw.clear();
            client.rotationHistoryRaw.clear();
            client.timeHistory.clear();
            client.accelerationHistoryKalman.clear();
            client.rotationHistoryKalman.clear();
            
        }
        
        if(m.getAddress() == "/device"){
            
            int clientID = m.getArgAsInt32(0);
            DeviceType deviceType = (DeviceType)m.getArgAsInt32(1);
            ServerType serverType = (ServerType)m.getArgAsInt32(2);
            
            /*
             ofLogVerbose()  << clientID << " "
             << getPhoneTypeAsString(phoneType) << " "
             << getServerTypeAsString(serverType) << " "
             << timeStamp << " "
             << acceleration << " "
             << rotation;
             */
            
            createClient(clientID, deviceType, serverType);
            
            DeviceClient& client = devices[clientID];
            OSCRecorder& recorder = recorders[clientID];
            
            client.lastAccelerationRaw = ofPoint(m.getArgAsFloat(4), m.getArgAsFloat(5), m.getArgAsFloat(6));
            client.lastRotationRaw = ofPoint(m.getArgAsFloat(7), m.getArgAsFloat(8), m.getArgAsFloat(9));
            client.lastAttitudeRaw = ofPoint(m.getArgAsFloat(10), m.getArgAsFloat(11), m.getArgAsFloat(12));
            client.lastGravityRaw = ofPoint(m.getArgAsFloat(13), m.getArgAsFloat(14), m.getArgAsFloat(15));
            client.lastUserAccelerationRaw = ofPoint(m.getArgAsFloat(16), m.getArgAsFloat(17), m.getArgAsFloat(18));
            client.lastTimeStamp = m.getArgAsInt32(3);
            
            for(int i = 0; i < 3; i++){
                client.maxAcceleration[i] = MAX(client.lastAccelerationRaw[i], client.maxAcceleration[i]);
                client.minAcceleration[i] = MIN(client.lastAccelerationRaw[i], client.minAcceleration[i]);
            }
            
            if(client.lastUserAccelerationKalman.length() > 1.0f) cout << "JERK: " << client.lastTimeStamp << endl;
            
            if(recorder.bIsRecording) recorder.push_back(m);
            
            if(client.accelerationHistoryRaw.size() == ofGetWidth()) client.accelerationHistoryRaw.clear();
            if(client.rotationHistoryRaw.size() == ofGetWidth()) client.rotationHistoryRaw.clear();
            if(client.attitudeHistoryRaw.size() == ofGetWidth()) client.attitudeHistoryRaw.clear();
            if(client.gravityHistoryRaw.size() == ofGetWidth()) client.gravityHistoryRaw.clear();
            if(client.userAccelerationHistoryRaw.size() == ofGetWidth()) client.userAccelerationHistoryRaw.clear();
            
            
            if(client.accelerationHistoryKalman.size() == ofGetWidth()) client.accelerationHistoryKalman.clear();
            if(client.rotationHistoryKalman.size() == ofGetWidth()) client.rotationHistoryKalman.clear();
            if(client.attitudeHistoryKalman.size() == ofGetWidth()) client.attitudeHistoryKalman.clear();
            if(client.gravityHistoryKalman.size() == ofGetWidth()) client.gravityHistoryKalman.clear();
            if(client.userAccelerationHistoryKalman.size() == ofGetWidth()) client.userAccelerationHistoryKalman.clear();
            
            if(client.timeHistory.size() == ofGetWidth()) client.timeHistory.clear();
            
            client.accelerationHistoryRaw.push_back(client.lastAccelerationRaw);
            client.rotationHistoryRaw.push_back(client.lastRotationRaw);
            client.timeHistory.push_back(client.lastTimeStamp);
            
            client.attitudeHistoryRaw.push_back(client.lastAttitudeRaw);
            client.gravityHistoryRaw.push_back(client.lastGravityRaw);
            client.userAccelerationHistoryRaw.push_back(client.lastUserAccelerationRaw);
            
            

            
            vector<ofPoint> measurement;
//            measurement.push_back(client.lastAccelerationRaw);
//            measurement.push_back(client.lastRotationRaw);
            measurement.push_back(client.lastAttitudeRaw);
//            measurement.push_back(client.lastGravityRaw);
            measurement.push_back(client.lastUserAccelerationRaw);
            
            client.kalmanFilter.setMeasured(measurement);
            
            cv::Mat k = client.kalmanFilter.getCorrected();
//            cv::Mat k = client.kalmanFilter.getPredicted();
            
            
            
            client.lastAttitudeKalman = toOf(cv::Point3f(k.at<float>(0), k.at<float>(1), k.at<float>(2)));
            client.lastUserAccelerationKalman = toOf(cv::Point3f(k.at<float>(3), k.at<float>(4), k.at<float>(5)));
            
            // circular buffer window
            client.accelerationWindowSmall.push_back(client.lastUserAccelerationRaw);
            
//            if(client.lastUserAccelerationKalman.length() < 0.3f){
//                client.lastAttitudeKalman = toOf(cv::Point3f(k.at<float>(6), k.at<float>(7), k.at<float>(8)));
//            }else{
//                client.lastAttitudeKalman = client.lastAttitudeKalman * 0.99 + toOf(cv::Point3f(k.at<float>(6), k.at<float>(7), k.at<float>(8))) * 0.01;
//            }
            
//            float highpass = 0.8f;
//            client.lastAttitudeKalman = (highpass) * client.lastAttitudeKalman + (1 - highpass) * toOf(cv::Point3f(k.at<float>(6), k.at<float>(7), k.at<float>(8)));
            
//            client.lastAttitudeKalman = toOf(cv::Point3f(k.at<float>(6), k.at<float>(7), k.at<float>(8)));
//            client.lastGravityKalman = toOf(cv::Point3f(k.at<float>(9), k.at<float>(10), k.at<float>(11)));
//            client.lastUserAccelerationKalman = toOf(cv::Point3f(k.at<float>(9), k.at<float>(10), k.at<float>(11)));
            
//            client.accelerationHistoryKalman.push_back(client.lastAccelerationKalman);
//            client.rotationHistoryKalman.push_back(client.lastRotationKalman);
            client.attitudeHistoryKalman.push_back(client.lastAttitudeKalman);
//            client.gravityHistoryKalman.push_back(client.lastGravityKalman);
            client.userAccelerationHistoryKalman.push_back(client.lastUserAccelerationKalman);
            
            client.refreshAverage = ofGetElapsedTimeMillis() - client.refreshLastTime;
            client.refreshLastTime = ofGetElapsedTimeMillis();
            
        }
        
    }
    
}

void DeviceController::createClient(int clientID, DeviceType deviceType, ServerType serverType){
    map<int, DeviceClient>::iterator deviceIt = devices.find(clientID);
    if(deviceIt == devices.end()){
        ofLogNotice() << "Creating new device client ID " << clientID << " for: " << getServerTypeAsString(serverType);
        DeviceClient d;
        OSCRecorder r;
        devices[clientID] = d;
        recorders[clientID] = r;
        DeviceClient& client = devices[clientID];
        client.accelerationWindowSmall.resize(50, 3);
        client.kalmanFilter.setup(2, 3);
        client.serverType = serverType;
        client.deviceType = deviceType;
        client.clientID = clientID;
    }
}

void DeviceController::draw() {
    
    int count = 0;
    map<int, DeviceClient>::iterator deviceIt;
    
    for(deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt, ++count){
        
        DeviceClient& client = deviceIt->second;
        
        glPushMatrix();
        
        float xPos = ofMap(-client.lastAttitudeKalman.z, ofDegToRad(-35), ofDegToRad(35), 0.0f, ofGetWidth());
        float yPos = ofMap(-client.lastAttitudeKalman.x, ofDegToRad(-35), ofDegToRad(15), 0.0f, ofGetHeight());
        
        if(client.lastUserAccelerationRaw.length() < 0.5f){
            ofSetColor(0, 255, 255);
        }else{
            ofSetColor(255, 0, 127);
        }

        ofFill();
        ofCircle(xPos, yPos, 4);
        ofNoFill();
        
        glPopMatrix();
        
        glPushMatrix();
        
        ofSetColor(255, 255, 255);
        ofTranslate(40, 300 * count + 300 * count);
        
        ostringstream os;
        os << endl;
        os << getServerTypeAsString(client.serverType) << " ";
        os << getDeviceTypeAsString(client.deviceType) << " ";
        os << "ID: " << deviceIt->first << " ";
        os << "HZ: " << client.refreshAverage << endl;
        os << "LN: " << client.lastUserAccelerationKalman.length() << endl;
        os << "MAX: " << client.maxAcceleration << " MIN: " << client.minAcceleration << endl;
        os << "WIN: " << client.accelerationWindowSmall.getMaxDimension() << " " << client.accelerationWindowSmall.getMinDimension() << endl;
        
        ofSetColor(255, 255, 255);
        ofDrawBitmapString(os.str(), 20, 20);
        
//        drawVector(0, 50, 70, client.accelerationHistoryRaw, 0.5);
        drawVector(0, 200, 20, client.userAccelerationHistoryRaw, 0.5);
        
        ofTranslate(0, 300);
        
//        drawVector(0, 50, 70, client.accelerationHistoryKalman, 0.5);
        drawVector(0, 200, 20, client.accelerationWindowSmall.getRepresentation(), 0.5);
        
        glPopMatrix();
        
    }
    
    ostringstream os;

    for(map<int, OSCRecorder>::iterator it = recorders.begin(); it != recorders.end(); ++it){
        os << it->second.getLastRecordingAsString() << endl;
    }
    
    ofSetColor(255, 255, 255);
    ofDrawBitmapString(os.str(), 20, 60);
    
}

//--------------------------------------------------------------
int DeviceController::getNumberDevices(){
    return devices.size();
}


//--------------------------------------------------------------
DeviceClient& DeviceController::getDevice(int deviceIndex){
    
    if(deviceIndex < 0 || deviceIndex > devices.size()){
        cout << "Error deviceIndex out of bounds: " << deviceIndex << endl;
        return NoClient;
    }
    
    int count = 0;
    map<int, DeviceClient>::iterator deviceIt;
    
    for(deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt, ++count){
        if(deviceIndex == count) return deviceIt->second;
    }
    
}

OSCRecorder& DeviceController::getRecorder(int deviceIndex){
    DeviceClient& d = getDevice(deviceIndex);
    return recorders[d.clientID];
}

bool DeviceController::getRecordOSC(int deviceIndex){
    
    DeviceClient& d = getDevice(deviceIndex);
    OSCRecorder& r = recorders[d.clientID];
    return r.getRecordingOSC();
    
}

void DeviceController::setRecordOSC(int deviceIndex, bool b){
    
    DeviceClient& d = getDevice(deviceIndex);
    OSCRecorder& r = recorders[d.clientID];
    r.setRecordingOSC(b);
    
}

void DeviceController::toggleRecordOSC(int deviceIndex){
    DeviceClient& d = getDevice(deviceIndex);
    OSCRecorder& r = recorders[d.clientID];
    r.toggleRecordingOSC();
}

//--------------------------------------------------------------
void DeviceController::keyPressed(int key){
    switch (key) {
        case 'c':
        {
            cout << "connect" << endl;
            ofxOscMessage m;
            m.setAddress("/server");
            m.addStringArg("start");
            m.addIntArg(6666);
            m.addIntArg(-1);
            m.addIntArg(15);
            oscSender.sendMessage(m);
            break;
        }
        case 's':
        {
            cout << "count" << endl;
            ofxOscMessage m;
            m.setAddress("/server");
            m.addStringArg("count");
            m.addIntArg(6666);
            oscSender.sendMessage(m);
            break;
        }
        default:
            break;
    }
}

//--------------------------------------------------------------
void DeviceController::drawVector(float x, float y, float scale, vector<ofPoint> & vec, float fade){
    if(vec.size() < 2) return;
    ofPushMatrix();
    ofTranslate(x, y);
    ofNoFill();
    for(int i = 0; i < vec.size() - 1; i++){
        ofPushMatrix();
        ofTranslate(0, scale * 0);
        ofSetColor(255*fade, 0, 0);
        ofLine(i, vec[i].x * scale, i + 1, vec[i + 1].x * scale);
        ofPopMatrix();
        ofPushMatrix();
        ofTranslate(0, scale * 1);
        ofSetColor(0, 255*fade, 0);
        ofLine(i, vec[i].y * scale, i + 1, vec[i + 1].y * scale);
        ofPopMatrix();
        ofPushMatrix();
        ofTranslate(0, scale * 2);
        ofSetColor(0, 0, 255*fade);
        ofLine(i, vec[i].z * scale, i + 1, vec[i + 1].z * scale);
        ofPopMatrix();
    }
    ofPopMatrix();
}

string DeviceController::getDeviceTypeAsString(DeviceType deviceType){
    switch (deviceType) {
        case DEVICETYPE_IPHONE:
            return "DEVICETYPE_IPHONE";
            break;
        case DEVICETYPE_ANDROID:
            return "DEVICETYPE_ANDROID";
            break;
        default:
            return "DEVICETYPE_UNKOWN";
            break;
    }
}

string DeviceController::getServerTypeAsString(ServerType serverType){
    switch (serverType) {
        case SERVERTYPE_MATTG:
            return "SERVERTYPE_MATTG";
            break;
        case SERVERTYPE_MATTL:
            return "SERVERTYPE_MATTL";
            break;
        case SERVERTYPE_NORMJ:
            return "SERVERTYPE_NORMJ";
            break;
        default:
            return "SERVERTYPE_UNKOWN";
            break;
    }
}
