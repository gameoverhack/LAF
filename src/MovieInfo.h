//
//  MovieInfo.h
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#ifndef __H_MOVIEINFO
#define __H_MOVIEINFO

#include "ofxLogger.h"

class MovieInfo{
    
public:
    
    MovieInfo(){
        
    }
    
    ~MovieInfo(){
        predictedPosition.clear();
        possibleTransitions.clear();
        impossibleTransitions.clear();
        intersectionFrames.clear();
    }
    
    friend ostream& operator<< (ostream &os, MovieInfo &mi);
    
    string name = "";
    string path = "";
    int frame = 0;
    int startframe = 0;
    int genframe = 0;
    int totalframes = 0;
    float speed = 2.0;
    bool isFrameNew = false;
    bool isMovieDirty = false;
    string motion = "";
    ofRectangle bounding;
    ofPoint position = ofPoint(0,0,0);
    vector<ofPoint> predictedPosition;
    vector<ofRectangle> predictedBounding;
    vector<MovieInfo> possibleTransitions;
    vector<MovieInfo> impossibleTransitions;
    bool intersectedTransition = false;
    vector<int> intersectionFrames;
    
};


inline ostream& operator<<(ostream& os, MovieInfo &mi){
    os  << mi.name << " "
        << mi.motion << " "
        << mi.bounding << " ("
        << mi.possibleTransitions.size() << ", "
        << mi.impossibleTransitions.size() << ") ["
        << mi.startframe << ", "
        << mi.genframe << "] "
        << mi.frame;
    return os;
};

#endif
