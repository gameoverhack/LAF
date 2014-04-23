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
    
    // stop the threading
    waitForThread();
    stopThread();
    
    // close all connections
    UDPbroadcast.Close();
    
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
    
    /******************************************************
     *******                Servers                 *******
     *****************************************************/
    
    // get ip address of the server
    serverIPfull = appModel->getIPAddress();
    
    if(serverIPfull != "0.0.0.0"){
        
        // determine broadcast IP by cropping and adding 255 to the server IP
        serverIProot = serverIPfull.substr(0, serverIPfull.rfind("."));
        serverIPpart = serverIPfull.substr(serverIPfull.rfind(".") + 1, string::npos);
        serverIPbroadcast = serverIProot + ".255";
        
        ofxLogNotice() << "Starting networking for server ip " << serverIPfull << " (" << serverIPpart << ") and broadcast ip " << serverIPbroadcast << endl;
        
        // open UDP broadcast port
        UDPbroadcast.Create();
        UDPbroadcast.Connect(serverIPbroadcast.c_str(), 10001);
        UDPbroadcast.SetEnableBroadcast(true);
        UDPbroadcast.SetNonBlocking(true);
        
        // OPEN COMMUNICATION PORTS FOR UDP, OSC and YARP
        
        // UDP
        UDPmanager.Create();
        if(UDPmanager.Bind(10002)){
            ofxLogNotice() << "Bind UDPmanager on " << serverIPfull << ":10002" << endl;
            UDPmanager.SetNonBlocking(true);
        }
        
        // OSC - need test for these?
        OSCReceiver.setup(10003);
        ofxLogNotice() << "Bind OSCReceiver on " << serverIPfull << ":10003" << endl;
        
        ofxOscSender& OSCSender = philModel->getOSCSender();
        
        OSCSender.setup("10.0.1.69", 10004);
        ofxLogNotice() << "Bind OSCSender on " << "10.0.1.69" << ":10004" << endl;
        
        // YARP
        yarp::os::impl::NameConfig yarpNameConfig;
        yarp::os::impl::Address yarpAddress(serverIPfull.c_str(), 10000);
        yarpNameConfig.setAddress(yarpAddress);
        yarpNameConfig.toFile();
        yarpNameConfig.fromFile();
        
        if(YARPReceiver.open("/motionReceiver")){
            ofxLogNotice() << "Bind YARPReceiver on " << serverIPfull << ":10000" << endl;
        }
        
        // set timer
        timerUDPBroadcastPing = ofGetElapsedTimeMillis();
        clientMode = "";
        clientYarpMode = "";
        
        appModel->setProperty("ClientMode", (string)"UDP");
        appModel->setProperty("ClientYarpMode", (string)"udp");
        
        // start threading - non-blocking, non-verbose
        startThread(true, false);                      // QUESTION: maybe blocking and flags is better for this?
        
    }else{
        
        ofxLogError() << "Cannot determine server IP address - not starting the DeviceController thread" << endl;
        deviceControllerControllerStates.setState(kDEVICECONTROLLER_ERROR);
        
    }

}

//--------------------------------------------------------------
void DeviceController2::update(){
    
    if(lock()){

        // using a global mutex on the device data as well as local locks
        // so that interconnect between MVC objects does not get out of sync
        appModel->getDeviceMutex().lock();
        
        map<int, DeviceClient>& devices = appModel->getAllDevices();
        
        // here we are checking that clients are alive and getting data from them
        // probably both things could be moved -> deletion to the appModel
        // info -> into a view...but for now it's convenient to have them here
        
        ostringstream os;
        
        int clients = 0; set<int> clientsToDelete;
        
        for(map<int, DeviceClient>::iterator it = devices.begin(); it != devices.end(); ++it){
            
            DeviceClient& client = it->second;
            
            // check for ping timeout from clients
            // and mark for deletion if we haven't
            // heard from them in "PingClient" millis
            if(ofGetElapsedTimeMillis() - client.timeLastPing > 2 * appModel->getProperty<int>("PingClient")){
                
                // mark for deletion
                clientsToDelete.insert(it->first);
                
            }else{
                
                // else cache info
                os << clients << " " << client << endl;
                clients++;
                
            }
            
        }
        
        // delete timed-out clients
        for(set<int>::iterator dt = clientsToDelete.begin(); dt != clientsToDelete.end(); ++dt){
            eraseAll(devices, *dt);
        }
        
        // store client data on the model
        appModel->setProperty("DeviceClientInfo", os.str());
        
        // unlock the mutexes
        appModel->getDeviceMutex().unlock();
        unlock();
    }
    
}

//--------------------------------------------------------------
void DeviceController2::threadedFunction(){
    
    while (isThreadRunning()){
        
        if(lock()){
            
            // lock global data mutex
            appModel->getDeviceMutex().lock();
            
            // get devices from the appmodel
            map<int, DeviceClient>& devices = appModel->getAllDevices();
            
            /******************************************************
             *******                Broadcast               *******
             *****************************************************/
            
            // broadcast (ping) the ip address of the server allowing auto connection
            // on DHCP host for any IP addresses of both client and server
            // ie., we send ip to all clients on x.x.x.255 at "PingBroadcast" millis intervals
            // essentially this is a uber basic 'name' server with no names ;)
            
            if(ofGetElapsedTimeMillis() - timerUDPBroadcastPing > appModel->getProperty<int>("PingBroadcast")){
                ofxLogVerbose() << "UDP broadcast ping on ip: " << serverIPbroadcast << endl;
                string msg = "S_" + serverIPpart;
                UDPbroadcast.Send(msg.c_str(), msg.size());
                timerUDPBroadcastPing = ofGetElapsedTimeMillis();
            }
            
            /******************************************************
             *******             Client Receive             *******
             *****************************************************/
            
            // UDP connection and control
            
            char udpReceiveMessageChar[1024];
            UDPmanager.Receive(udpReceiveMessageChar, 1024);
            string udpReceiveMessageStr = udpReceiveMessageChar;
            
            if(udpReceiveMessageStr != ""){
                
                //ofxLogVerbose() << "UDP Comms Message: " << udpReceiveMessageStr << endl;
                
                vector<string> command = ofSplitString(udpReceiveMessageStr, "_");
                
                if(command[0] == "C"){
                    
                    int clientID = ofToInt(command[1]);
                    
                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    
                    // clients are sending there IP address everytime
                    // they hear the BroadcastPing so we can either
                    // create new client or timeout if we don't hear
                    // the ping. TODO: calculate round trip latency??
                    
                    if(it == devices.end()){
                        
                        ofxLogNotice() << "Creating client device with ID: " << clientID << endl;
                        
                        // create the device
                        DeviceClient d;
                        devices[clientID] = d;
                        
                        // setup the device
                        DeviceClient& client = devices[clientID];
                        client.accelerationBuffer.resize(50, 3);
                        client.attitudeBuffer.resize(50, 3);
                        client.positionBuffer.resize(20, 3);
                        client.kalmanFilter.setup(2, 3);
                        client.clientID = clientID;
                        client.timeLastPing = ofGetElapsedTimeMillis();
                        client.deviceColor = generateRandomColor();
                        
                        // this ensures that clientMode and yarpMode are resent on reconnections
                        // NB: not that efficient but it works ;)
                        clientMode = clientYarpMode = "";
                        
                    }else{
                        
                        // store ping from clients
                        DeviceClient& client = devices[clientID];
                        client.timeLastPing = ofGetElapsedTimeMillis();
                        
                    }

                }else{
                    
                    // if the udp data is not a ping then we assume it's sending us data packets
                    
                    DeviceMessageUnion dmu;
                    
                    // readout data from char to union
                    for(int i = 0; i < sizeof(DeviceMessage); i++) dmu.data[i] = udpReceiveMessageChar[i];
                    
                    // process data
                    
                    int clientID = dmu.deviceMessage.clientID;
                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    
                    if(it != devices.end()){
                        
                        DeviceClient& device = it->second;
                        
                        if(dmu.deviceMessage.control == 'd'){
                            device.push(dmu.deviceMessage);
                        }
                        
                        if(dmu.deviceMessage.control == 'r'){
                            cout << "Record DOWN" << endl;
                            device.bButton = true;
                        }
                        
                        if(dmu.deviceMessage.control == 'R'){
                            cout << "Record UP" << endl;
                            device.bButton = false;
                        }
                    }

                }
                
            }
            
            // BROADCAST MODES TO CLIENTS
            
            string pClientMode = appModel->getProperty<string>("ClientMode");
            if(clientMode != pClientMode){
                ofxLogNotice() << "Changing client mode from " << clientMode << " to " << pClientMode << endl;
                string msg = "M_" + pClientMode;
                UDPbroadcast.Send(msg.c_str(), msg.size());
                clientMode = pClientMode;
            }
            
            string pClientYarpMode = appModel->getProperty<string>("ClientYarpMode");
            if(clientYarpMode != pClientYarpMode){
                ofxLogNotice() << "Changing client YARP mode from " << clientYarpMode << " to " << pClientYarpMode << endl;
                string msg = "T_" + pClientYarpMode;
                UDPbroadcast.Send(msg.c_str(), msg.size());
                clientYarpMode = pClientYarpMode;
            }
            
            /******************************************************
             *******                   OSC                  *******
             *****************************************************/
            
            while(OSCReceiver.hasWaitingMessages()){
                
                ofxOscMessage m;
                OSCReceiver.getNextMessage(&m);
                
                if(m.getAddress() == "/device"){
                    
                    int clientID = m.getArgAsInt32(0);

                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    if(it != devices.end()) it->second.push(m);
                    
                }
                
            }
            
            /******************************************************
             *******                  YARP                  *******
             *****************************************************/
            
            if(YARPReceiver.getPendingReads()){
                
                yarp::os::Bottle *input = YARPReceiver.read();
                
                if(input->get(0).toString() == "/device"){
                    
                    int clientID = input->get(1).asInt();
                    
                    map<int, DeviceClient>::iterator it = devices.find(clientID);
                    if(it != devices.end()) it->second.push(input);
                    
                }
                
            }
            
            // don't thrash the thread!
            ofSleepMillis(1);
            
            // unlock global data mutex
            appModel->getDeviceMutex().unlock();
            unlock();
        }
        
    }
    
}