//
//  DeviceController.cpp
//  protoApp
//
//  Created by gameover on 26/08/13.
//
//

#include "DeviceController.h"
#include <math.h>


using namespace ofxCv;
using namespace cv;

static int lastTime = ofGetElapsedTimeMillis();

void DeviceController::setup() {
    
    //oscReceiver.setup(6666);
    //oscSender.setup("192.168.42.101", 8000);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    //setup read thread
    readThread.setup(&devices, &recorders, &xoptical_vals, &yoptical_vals, &port);
    readThread.startThread(true, false);
    
    yarp::os::impl::NameConfig nc;
    yarp::os::impl::Address addr("10.0.1.104", 10000);
    //yarp::os::impl::Address addr("206.12.30.240", 10000);
    nc.setAddress(addr);
    nc.toFile();
    nc.fromFile();
    
    port.open("/motionReceiver");
    
    outPort.open("/mouseEmulator");
    
    bNormConnected = false;
    
    ofBackground(0, 0, 0);
    
}

void DeviceController::exit() {
    //printf("stopping thread...\n");
    readThread.stopThread();
}

void DeviceController::update() {
    
    
    return; //moved everything to threaded reader!!!
    
    if (port.getPendingReads()) {
        yarp::os::Bottle *input = port.read();
        if (input->get(0).toString() == "/record") {
            int clientID = input->get(1).asInt();
            DeviceType deviceType = (DeviceType)input->get(2).asInt();
            ServerType serverType = (ServerType)input->get(3).asInt();
            int timestamp = input->get(4).asInt();
            bool bRecord = (bool)input->get(5).asInt();
            
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
        if (input->get(0).toString() == "/reset") {
            
            int clientID = input->get(1).asInt();
            DeviceType deviceType = (DeviceType)input->get(2).asInt();
            ServerType serverType = (ServerType)input->get(3).asInt();
            
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
        if (input->size() == 20) {
            //todo: handle /reset and /record
            if (input->get(0).toString() == "/device") {

                
                /*
                 ofLogVerbose()  << clientID << " "
                 << getPhoneTypeAsString(phoneType) << " "
                 << getServerTypeAsString(serverType) << " "
                 << timeStamp << " "
                 << acceleration << " "
                 << rotation;
                 */
                int clientID = input->get(1).asInt();
                DeviceType deviceType = (DeviceType)input->get(2).asInt();
                ServerType serverType = (ServerType)input->get(3).asInt();
                createClient(clientID, deviceType, serverType);
                
                DeviceClient& client = devices[clientID];
                OSCRecorder& recorder = recorders[clientID];
                
                
                
                client.lastAccelerationRaw = ofPoint(input->get(5).asDouble(), input->get(6).asDouble(), input->get(7).asDouble());
                client.lastRotationRaw = ofPoint(input->get(8).asDouble(), input->get(9).asDouble(), input->get(10).asDouble());
                client.lastAttitudeRaw = ofPoint(input->get(11).asDouble(), input->get(12).asDouble(), input->get(13).asDouble());
                client.lastGravityRaw = ofPoint(input->get(14).asDouble(), input->get(15).asDouble(), input->get(16).asDouble());
                client.lastUserAccelerationRaw = ofPoint(input->get(17).asDouble(), input->get(18).asDouble(), input->get(19).asDouble());
                client.lastTimeStamp = input->get(4).asInt();
                
                for(int i = 0; i < 3; i++){
                    client.maxAcceleration[i] = MAX(client.lastAccelerationRaw[i], client.maxAcceleration[i]);
                    client.minAcceleration[i] = MIN(client.lastAccelerationRaw[i], client.minAcceleration[i]);
                }
                
                if(client.lastUserAccelerationKalman.length() > 1.0f) cout << "JERK: " << client.lastTimeStamp << endl;
                
                //todo: update recorder with new type
                //if(recorder.bIsRecording) recorder.push_back(m);
                
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
    
    return; //bypass OSC stuff below...
    
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
        vector<float> xo;
        vector<float> yo;
        vector<float> zo;

        devices[clientID] = d;
        recorders[clientID] = r;
        xoptical_vals[clientID] = xo;
        yoptical_vals[clientID] = yo;
        zoptical_vals[clientID] = zo;
        
        DeviceClient& client = devices[clientID];
        client.accelerationWindowSmall.resize(50, 3);
        client.kalmanFilter.setup(2, 3);
        client.serverType = serverType;
        client.deviceType = deviceType;
        client.clientID = clientID;
    }
}

float DeviceController::getAvgVel(vector<float>& vals) {
    float output = -1.0;
    if (vals.size() >= 2) {
        for (std::vector<float>::size_type i = 1; i != vals.size(); ++i) {
            output+= vals[i] - vals[i-1];
        }
        output = output / vals.size();
    }
    return output;
}

void DeviceController::draw() {
    
    int count = 0;
    map<int, DeviceClient>::iterator deviceIt;
    
    for(deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt, ++count){
        
        DeviceClient& client = deviceIt->second;
        
        glPushMatrix();
        
        float xPos = ofMap(-client.lastAttitudeKalman.z, ofDegToRad(-35), ofDegToRad(35), 0.0f, ofGetWidth());
        float yPos = ofMap(-client.lastAttitudeKalman.x, ofDegToRad(-35), ofDegToRad(15), 0.0f, ofGetHeight());
        float zPos = ofMap(-client.lastAttitudeKalman.y, ofDegToRad(-15), ofDegToRad(35), 0.0f, ofGetHeight());

        //local x,y,z OpticalVals
        vector<float>& xOpticalVals = xoptical_vals[client.clientID];
        vector<float>& yOpticalVals = yoptical_vals[client.clientID];
        vector<float>& zOpticalVals = zoptical_vals[client.clientID];

        
        // gives a vector product on mouse pointer with respect to xaxis
        xOpticalVals.push_back(xPos);
        if(xOpticalVals.size() > 10) {
            // remove the first value
            xOpticalVals.erase(xOpticalVals.begin());
        }
        
        // gives a vector product on mouse pointer with respect to yaxis
        yOpticalVals.push_back(yPos);
        if(yOpticalVals.size() > 10) {
            // remove the first value
            yOpticalVals.erase(yOpticalVals.begin());
        }
        
        // gives a vector product on mouse pointer with respect to yaxis
        zOpticalVals.push_back(zPos);
        if(zOpticalVals.size() > 10) {
            // remove the first value
            zOpticalVals.erase(zOpticalVals.begin());
        }
        
        if(client.lastUserAccelerationRaw.length() < 0.5f){
            ofSetColor(0, 255, 255);
        }else{
            ofSetColor(255, 0, 127);
        }

        ofFill();
        
        //use the first device for "mouse emulation"
        if (deviceIt == devices.begin()) {
            float xV = getAvgVel(xOpticalVals);
            float yV = getAvgVel(yOpticalVals);
            //printf("%f: %f\n", xV, yV);
            
            //check movement threshold
            //printf("%f\n",(xV*xV+yV*yV));
            if ( (xV*xV+yV*yV) > 3 ) {
                if (!mDown) {
                    printf("mouse DOWN!\n");
                    mDown = true;
                    while (outPort.isWriting());
                    outBot = &outPort.prepare();
                    outBot->clear();
                    outBot->addString("/mouse");
                    outBot->addString("down");
                    outBot->addDouble(xPos/ofGetScreenWidth());
                    outBot->addDouble(yPos/ofGetScreenHeight());
                    outPort.write();
                    ofSleepMillis(25);
                    
                }
            }
            else if (mDown) {
                printf("mouse UP!\n");
                mDown = false;
                while (outPort.isWriting());
                outBot = &outPort.prepare();
                outBot->clear();
                outBot->addString("/mouse");
                outBot->addString("up");
                outBot->addDouble(xPos/ofGetScreenWidth());
                outBot->addDouble(yPos/ofGetScreenHeight());
                outPort.write();
            }
            
            //send "mouse" position
            if (mDown) {
                while (outPort.isWriting());
                outBot = &outPort.prepare();
                outBot->clear();
                outBot->addString("/mouse");
                outBot->addDouble(xPos/ofGetScreenWidth());
                outBot->addDouble(yPos/ofGetScreenHeight());
                outPort.write();
            }
        }
        //ofCircle(xPos, yPos, 4);
        
        
        //johnty: do we really want to only use the current position, and the position 10 pts ago for this "speed" calculation?
        //    using a weighted average of each value in the vector would be smoother.
        //ofEllipse(xPos, yPos, 4+(((xOpticalVals.back() - xOpticalVals.front())/10)*3), 4+(((yOpticalVals.back() - yOpticalVals.front())/10)*3));
        
        ofEllipse(xPos, yPos, 30+(((xOpticalVals.back() - xOpticalVals.front())/10)*5), 30+(((yOpticalVals.back() - yOpticalVals.front())/10)*5));
        
        // TODO need to define an interator to display vector representation for all the devices side by side
        ofLine(800,100, 800+(((xOpticalVals.back() - xOpticalVals.front())/10)*5), 100+(((yOpticalVals.back() - yOpticalVals.front())/10)*5));
        
        float y2 = 100+(((yOpticalVals.back() - yOpticalVals.front())/10)*5);
        float y1 = 100;
        float x2 = 800+(((xOpticalVals.back() - xOpticalVals.front())/10)*5);
        float x1 = 800;
        
        float slope = ((y2-y1)/(x2-x1));
        float theta = atan(slope);
        float degreeTheta = theta * (180 / 3.14159265 );
        float vectorProduct = (y2-y1)*(x2-x1)*sin(degreeTheta);

        //ofEllipse(xPos, yPos, 4, 4);
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
        os << "Optical Flow X Axis: " << xOpticalVals.back() - xOpticalVals.front() << endl;
        os << "Optical Flow Y Axis: " << yOpticalVals.back() - yOpticalVals.front() << endl;
        os << "Optical Flow Z Axis: " << zOpticalVals.back() - zOpticalVals.front() << endl;
        os << "Theta value of flow: " << degreeTheta << endl;
        //os << "Vector Product value of flow: " << vectorProduct << endl;

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
            //oscSender.sendMessage(m);
            break;
        }
        case 's':
        {
            cout << "count" << endl;
            ofxOscMessage m;
            m.setAddress("/server");
            m.addStringArg("count");
            m.addIntArg(6666);
            //oscSender.sendMessage(m);
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


void DeviceControllerThread::setup(map<int, DeviceClient> *d, map<int, OSCRecorder> *r,
                                   map<int, vector<float> > *xo,
                                   map<int, vector<float> > *yo,
                                   yarp::os::BufferedPort<yarp::os::Bottle>* p) {
    devs = d;
    recs = r;
    xopts = xo;
    yopts = yo;
    port = p;
}

void DeviceControllerThread::threadedFunction() {
    printf("starting read thread...\n");
    while (isThreadRunning()) {
        //parse input port
        if (port->getPendingReads()) {
            
            map<int, DeviceClient>& devices = *devs;
            map<int, OSCRecorder>& recorders = *recs;
            
            yarp::os::Bottle *input = port->read();
            
            if (input->get(0).toString() == "/record") {
                int clientID = input->get(1).asInt();
                DeviceType deviceType = (DeviceType)input->get(2).asInt();
                ServerType serverType = (ServerType)input->get(3).asInt();
                int timestamp = input->get(4).asInt();
                bool bRecord = (bool)input->get(5).asInt();
                
                ofLogVerbose() << "Record: "
                << clientID << " "
                << DeviceController::getDeviceTypeAsString(deviceType) << " "
                << DeviceController::getServerTypeAsString(serverType) << " "
                << timestamp << " "
                << (bRecord ? "START RECORD" : "STOP RECORD");
                lock();
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
                unlock();
                
            }
            if (input->get(0).toString() == "/reset") {
                
                int clientID = input->get(1).asInt();
                DeviceType deviceType = (DeviceType)input->get(2).asInt();
                ServerType serverType = (ServerType)input->get(3).asInt();
                
                ofLogVerbose() << "Reset: "
                << clientID << " "
                << DeviceController::getDeviceTypeAsString(deviceType) << " "
                << DeviceController::getServerTypeAsString(serverType);
                
                lock();
                createClient(clientID, deviceType, serverType);
                
                DeviceClient& client = devices[clientID];
                
                client.accelerationHistoryRaw.clear();
                client.rotationHistoryRaw.clear();
                client.timeHistory.clear();
                client.accelerationHistoryKalman.clear();
                client.rotationHistoryKalman.clear();
                unlock();
            }
            if (input->size() == 20) {
                //todo: handle /reset and /record
                if (input->get(0).toString() == "/device") {
                    
                    
                    /*
                     ofLogVerbose()  << clientID << " "
                     << getPhoneTypeAsString(phoneType) << " "
                     << getServerTypeAsString(serverType) << " "
                     << timeStamp << " "
                     << acceleration << " "
                     << rotation;
                     */
                    int clientID = input->get(1).asInt();
                    DeviceType deviceType = (DeviceType)input->get(2).asInt();
                    ServerType serverType = (ServerType)input->get(3).asInt();
                    
                    
                    lock();
                    createClient(clientID, deviceType, serverType);
                    
                    DeviceClient& client = devices[clientID];
                    OSCRecorder& recorder = recorders[clientID];
                    
                    client.lastAccelerationRaw = ofPoint(input->get(5).asDouble(), input->get(6).asDouble(), input->get(7).asDouble());
                    client.lastRotationRaw = ofPoint(input->get(8).asDouble(), input->get(9).asDouble(), input->get(10).asDouble());
                    client.lastAttitudeRaw = ofPoint(input->get(11).asDouble(), input->get(12).asDouble(), input->get(13).asDouble());
                    client.lastGravityRaw = ofPoint(input->get(14).asDouble(), input->get(15).asDouble(), input->get(16).asDouble());
                    client.lastUserAccelerationRaw = ofPoint(input->get(17).asDouble(), input->get(18).asDouble(), input->get(19).asDouble());
                    client.lastTimeStamp = input->get(4).asInt();
                    
                    for(int i = 0; i < 3; i++){
                        client.maxAcceleration[i] = MAX(client.lastAccelerationRaw[i], client.maxAcceleration[i]);
                        client.minAcceleration[i] = MIN(client.lastAccelerationRaw[i], client.minAcceleration[i]);
                    }
                    
                    if(client.lastUserAccelerationKalman.length() > 1.0f) cout << "JERK: " << client.lastTimeStamp << endl;
                    
                    //todo: update recorder with new type
                    //if(recorder.bIsRecording) recorder.push_back(m);
                    
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
                    
                    unlock();
                   
                    
                }
            }
            
        }
    }
    //printf("closing yarp port...\n");
    port->close();
    
}
void DeviceControllerThread::createClient(int clientID, DeviceType deviceType, ServerType serverType){
    map<int, DeviceClient>& devices = *devs;
    map<int, OSCRecorder>& recorders = *recs;
    map<int, vector<float> >& xoptical_vals = *xopts;
    map<int, vector<float> >& yoptical_vals = *yopts;
    map<int, DeviceClient>::iterator deviceIt = devices.find(clientID);
    if(deviceIt == devices.end()){
        ofLogNotice() << "Creating new device client ID " << clientID << " for: " << DeviceController::getServerTypeAsString(serverType);
        DeviceClient d;
        OSCRecorder r;
        vector<float> xo;
        vector<float> yo;
        vector<float> zo;

        devices[clientID] = d;
        recorders[clientID] = r;
        xoptical_vals[clientID] = xo;
        yoptical_vals[clientID] = yo;
        DeviceClient& client = devices[clientID];
        client.accelerationWindowSmall.resize(50, 3);
        client.kalmanFilter.setup(2, 3);
        client.serverType = serverType;
        client.deviceType = deviceType;
        client.clientID = clientID;
    }
}
