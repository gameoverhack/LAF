//
//  DeviceController.h
//  protoApp
//
//  Created by gameover on 26/08/13.
//
//

#ifndef __protoApp__DeviceController__
#define __protoApp__DeviceController__

#include "ofMain.h"
#include "Kalman.h"
#include "ofxOsc.h"
#include "AppModel.h"

#include "yarp/os/impl/NameConfig.h"
#include "yarp/os/all.h"


//enum DeviceType{
//    DEVICETYPE_IPHONE = 0,
//    DEVICETYPE_ANDROID
//};
//
//enum ServerType{
//    SERVERTYPE_MATTG = 0,
//    SERVERTYPE_MATTL,
//    SERVERTYPE_NORMJ
//};

//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ofxOscArgInt32);
//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ofxOscArgInt64);
//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ofxOscArgString);
//BOOST_SERIALIZATION_ASSUME_ABSTRACT(ofxOscArgFloat);

//namespace boost {
//    namespace serialization {
//        
//        template<class Archive>
//        void serialize(Archive & ar, ofPoint & p, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_NVP(p.x);
//            ar & BOOST_SERIALIZATION_NVP(p.y);
//            ar & BOOST_SERIALIZATION_NVP(p.z);
//        };
//
//        
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscArg & a, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ofxOscArgInt32);
//            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ofxOscArgInt64);
//            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ofxOscArgString);
//            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ofxOscArgFloat);
//        };
//        
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscArgInt32 & a, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_NVP(a.value);
//        };
//
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscArgInt64 & a, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_NVP(a.value);
//        };
//        
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscArgString & a, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_NVP(a.value);
//        };
//        
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscArgFloat & a, const unsigned int version) {
//            ar & BOOST_SERIALIZATION_NVP(a.value);
//        };
        
//        template<class Archive>
//        void serialize(Archive & ar, ofxOscMessage & o, const unsigned int version) {
//            ar.register_type(static_cast<ofxOscArgInt32 *>(NULL));
//            ar.register_type(static_cast<ofxOscArgInt64 *>(NULL));
//            ar.register_type(static_cast<ofxOscArgFloat *>(NULL));
//            ar.register_type(static_cast<ofxOscArgString*>(NULL));
//            ar & BOOST_SERIALIZATION_NVP(o.address);
//            ar & BOOST_SERIALIZATION_NVP(o.args);
//        };
//    };
//};

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

class DeviceMessage {
    
public:
    
    DeviceMessage(){}
    DeviceMessage(ofxOscMessage& m){
        clientID = m.getArgAsInt32(0);
        deviceType = (DeviceType)m.getArgAsInt32(1);
        serverType = (ServerType)m.getArgAsInt32(2);
        timestamp = m.getArgAsInt32(3);
        acceleration = ofPoint(m.getArgAsFloat(4), m.getArgAsFloat(5), m.getArgAsFloat(6));
        rotation = ofPoint(m.getArgAsFloat(7), m.getArgAsFloat(8), m.getArgAsFloat(9));
    }
    
    ~DeviceMessage(){}
    
    ofxOscMessage getOscMessage(){
        ofxOscMessage m;
        m.setAddress("/device");
        
        m.addIntArg(clientID);
        m.addIntArg(deviceType);
        m.addIntArg(serverType);
        
        m.addIntArg(timestamp);
        
        m.addFloatArg(acceleration.x);
        m.addFloatArg(acceleration.y);
        m.addFloatArg(acceleration.z);
        
        m.addFloatArg(rotation.x);
        m.addFloatArg(rotation.y);
        m.addFloatArg(rotation.z);
        return m;
    }
    
    int clientID;
    DeviceType deviceType;
    ServerType serverType;
    int timestamp;
    ofPoint acceleration;
    ofPoint rotation;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(clientID);
        ar & BOOST_SERIALIZATION_NVP(deviceType);
        ar & BOOST_SERIALIZATION_NVP(serverType);
        ar & BOOST_SERIALIZATION_NVP(timestamp);
        ar & BOOST_SERIALIZATION_NVP(acceleration);
        ar & BOOST_SERIALIZATION_NVP(rotation);
    }
    
};

class OSCRecorder {
    
public:
    
    OSCRecorder(){
        bIsRecording = false;
    }
    ~OSCRecorder(){}
    
    string getMessageAsString(ofxOscMessage& m){
        ostringstream os;
        for(int j = 0; j < m.getNumArgs(); j++){
            ofxOscArgType type = m.getArgType(j);
            switch(type){
                case OFXOSC_TYPE_INT32:
                    os << m.getArgAsInt32(j) << (j == m.getNumArgs() - 1 ? "" : ", ");
                    break;
                case OFXOSC_TYPE_INT64:
                    os << m.getArgAsInt64(j) << (j == m.getNumArgs() - 1 ? "" : ", ");
                    break;
                case OFXOSC_TYPE_FLOAT:
                    os << m.getArgAsFloat(j) << (j == m.getNumArgs() - 1 ? "" : ", ");
                    break;
                case OFXOSC_TYPE_STRING:
                    os << m.getArgAsString(j) << (j == m.getNumArgs() - 1 ? "" : ", ");
                    break;
            }
        }
        return os.str();
    }
    
    string getLastRecordingAsString(){
        if(oscRecording.size() == 0) return "";
        ofxOscMessage& m = oscRecording[oscRecording.size() - 1];
        return getMessageAsString(m);
    }
    
    string getRecordingAsString(){
        if(oscRecording.size() == 0) return "";
        ostringstream os;
        for(int i = 0; i < oscRecording.size(); i++){
            ofxOscMessage& m = oscRecording[i];
            os << getMessageAsString(m) << endl;
        }
        return os.str();
    }
    
    void push_back(ofxOscMessage& m){
        if(oscRecording.size() == 0) lastTimeMillis = ofGetElapsedTimeMillis();
        oscRecording.push_back(m);
        deviceRecording.push_back(DeviceMessage(m));
        timestamps.push_back(ofGetElapsedTimeMillis() - lastTimeMillis);
        lastTimeMillis = ofGetElapsedTimeMillis();
    }
    
    void save(){
        
        FILE * file;
        string timestamp = "oscRecording_" + ofGetTimestampString();
        string filename = "recordings/" + timestamp + ".txt";
        
        Serializer.saveClass(filename, (*this), ARCHIVE_TEXT);
        filename = ofToDataPath("csv/" + timestamp + ".csv");
        file = fopen(filename.c_str(), "w");
        
        for(int i = 0; i < deviceRecording.size(); i++){
            ostringstream os;
            os  << deviceRecording[i].acceleration.x << ","
                << deviceRecording[i].acceleration.y << ","
                << deviceRecording[i].acceleration.z << ","
                << deviceRecording[i].rotation.x << ","
                << deviceRecording[i].rotation.y << ","
                << deviceRecording[i].rotation.z << ","
                << deviceRecording[i].timestamp << endl;
            
            fputs (os.str().c_str(), file);
        }
        
        fclose(file);
        
    }
    
    void load(string filename){
        
        Serializer.loadClass(filename, (*this), ARCHIVE_TEXT);
        cout << "Load: " << filename << " " << deviceRecording.size() << " " << timestamps.size() << endl;
        for(int i = 0; i < deviceRecording.size(); i++){
            oscRecording.push_back(deviceRecording[i].getOscMessage());
        }
    }
    
    void play(){
        if(oscRecording.size() == 0) return;
        sender.setup("10.1.1.1", 6666);
        cout << "play" << endl;
        currentMessageIndex = 0;
        nextMessage();
    }
    
    void nextMessage(){
        //cout << "next: " << currentMessageIndex << endl;
        ofxOscMessage& m = oscRecording[currentMessageIndex];
        sender.sendMessage(m);
        if(currentMessageIndex + 1 < oscRecording.size()){
            currentMessageIndex++;
            double delayInMilliseconds = timestamps[currentMessageIndex];
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInMilliseconds * NSEC_PER_SEC / 1000));
            dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                nextMessage();
            });
        }
    }
    
    bool getRecordingOSC(){
        return bIsRecording;
    }
    
    void setRecordingOSC(bool b){
        if(b){
            oscRecording.clear();
            deviceRecording.clear();
        }
        bIsRecording = b;
    }
    
    void toggleRecordingOSC(){
        if(bIsRecording){
            oscRecording.clear();
            deviceRecording.clear();
        }
        bIsRecording = !bIsRecording;
    }
    
    int currentMessageIndex;
    bool bIsRecording;
    
    vector<ofxOscMessage> oscRecording;
    vector<DeviceMessage> deviceRecording;
    
    vector<int> timestamps;
    
    int lastTimeMillis;
    
    ofxOscSender sender;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(deviceRecording);
        ar & BOOST_SERIALIZATION_NVP(timestamps);
    }
    
    
};

//class RingBuffer{
//    
//public:
//    
//    RingBuffer(){
//        clear();
//    }
//    
//    RingBuffer(int s, int d){
//        clear();
//        resize(s, d);
//    }
//    
//    ~RingBuffer(){
//        clear();
//    }
//    
//    void resize(int s, int d){
//        dimensions = d;
//        buffer.resize(dimensions);
//        representation.resize(s);
//        for(int i = 0; i < dimensions; i++){
//            buffer[i].resize(s);
//        }
//    }
//    
//    void push_back(ofPoint point){
//        assert(buffer.size() > 0);
//        buffer[0][position] = point.x;
//        buffer[1][position] = point.y;
//        buffer[2][position] = point.z;
//        representation[position] = point;
//        position++;
//        if(position == representation.size()) position = 0;
//    }
//    
//    int getMaxDimension(){
//        vector<float> maximums(3);
//        for(int i = 0; i < dimensions; i++){
//            maximums[i] = getVecMaxValue(buffer[i]);
//        }
//        return getVecMaxIndex(maximums);
//    }
//    
//    int getMinDimension(){
//        vector<float> minimums(3);
//        for(int i = 0; i < dimensions; i++){
//            minimums[i] = getVecMinValue(buffer[i]);
//        }
//        return getVecMinIndex(minimums);
//    }
//    
//    void clear(){
//        position = 0;
//        buffer.clear();
//        representation.clear();
//    }
//    
//    int size(){
//        return buffer.size();
//    }
//    
//    vector< vector<float> >& getBuffer(){
//        return buffer;
//    }
//    
//    vector<ofPoint>& getRepresentation(){
//        return representation;
//    }
//    
//protected:
//    
//    int position; int dimensions;
//    vector< vector<float> > buffer;
//    vector<ofPoint> representation;
//    
//    
//};

//class DeviceClient {
//    
//public:
//    
//    DeviceClient(){}
//    ~DeviceClient(){}
//    
//    float refreshLastTime;
//    float refreshAverage;
//    
//    vector<ofPoint> accelerationHistoryRaw;
//    vector<ofPoint> rotationHistoryRaw;
//    
//    vector<ofPoint> attitudeHistoryRaw;
//    vector<ofPoint> gravityHistoryRaw;
//    vector<ofPoint> userAccelerationHistoryRaw;
//    
//    RingBuffer accelerationWindowSmall;
//    
//    vector<int> timeHistory;
//    
//    vector<ofPoint> accelerationHistoryKalman;
//    vector<ofPoint> rotationHistoryKalman;
//    vector<ofPoint> attitudeHistoryKalman;
//    vector<ofPoint> gravityHistoryKalman;
//    vector<ofPoint> userAccelerationHistoryKalman;
//    
//    ofPoint lastAccelerationRaw;
//    ofPoint lastRotationRaw;
//    ofPoint lastAttitudeRaw;
//    ofPoint lastGravityRaw;
//    ofPoint lastUserAccelerationRaw;
//    
//    ofPoint lastAccelerationKalman;
//    ofPoint lastRotationKalman;
//    ofPoint lastAttitudeKalman;
//    ofPoint lastGravityKalman;
//    ofPoint lastUserAccelerationKalman;
//    
//    int lastTimeStamp;
//    
//    ofPoint maxAcceleration = ofPoint(-INFINITY, -INFINITY, -INFINITY);
//    ofPoint minAcceleration = ofPoint(INFINITY, INFINITY, INFINITY);
//    
//    DeviceType deviceType;
//    ServerType serverType;
//    int clientID;
//    
//    ofColor deviceColorUp;
//    ofColor deviceColorDown;
//    
//    Kalman kalmanFilter;
//    
//};

class DeviceControllerThread : public ofThread {
    
    // johnty: this class is a hack, but fastest
    // way to get usable performance using more than 2 phones.
    // this thread handles the network receive.
    // contains a bunch of pointers to original data (initialized on setup)
    // in device controller and updates them during port read
public:
    void setup(map<int, DeviceClient>* d,
               map<int, OSCRecorder>* r,
               map<int, vector<float> > *xo,
               map<int, vector<float> > *yo,
               yarp::os::BufferedPort<yarp::os::Bottle>* p);
    void threadedFunction();
private:
    
    void createClient(int clientID, DeviceType deviceType, ServerType serverType);
    map<int, DeviceClient>* devs;
    map<int, OSCRecorder>* recs;
    map<int, vector<float> > *xopts;
    map<int, vector<float> > *yopts;
    map<int, vector<float> > *zopts;
    yarp::os::BufferedPort<yarp::os::Bottle>* port;
    
};

static DeviceClient NoClient;

class DeviceController {

public:
    
    void setup();
	void update();
	void draw();
    
    void exit();
	
    void keyPressed(int key);
    
    float getAvgVel(vector<float>& vals);
    
    void drawVector(float x, float y, float scale, vector<ofPoint> & vec, float fade);
    
    void createClient(int clientID, DeviceType deviceType, ServerType serverType);
    int getNumberDevices();
    DeviceClient& getDevice(int deviceIndex);
    OSCRecorder& getRecorder(int deviceIndex);
    
    bool getRecordOSC(int deviceIndex);
    void setRecordOSC(int deviceIndex, bool b);
    void toggleRecordOSC(int deviceIndex);
    
    ofxOscReceiver oscReceiver;
    ofxOscSender oscSender;
    
    bool bNormConnected;
    
    map<int, DeviceClient> devices;
    map<int, OSCRecorder> recorders;
    map<int, vector<float> > xoptical_vals;
    map<int, vector<float> > yoptical_vals;
    map<int, vector<float> > zoptical_vals;
    
    static string getDeviceTypeAsString(DeviceType deviceType);
    static string getServerTypeAsString(ServerType serverType);
    //vector<float> xOpticalVals, yOpticalVals; johnty: moved to map structure above
    
private:
    yarp::os::Network yarp;
    yarp::os::BufferedPort<yarp::os::Bottle> port;
    yarp::os::BufferedPort<yarp::os::Bottle> outPort;
    yarp::os::Bottle* outBot;
    DeviceControllerThread readThread;
    
    bool mDown;
    
};



#endif /* defined(__protoApp__DeviceController__) */
