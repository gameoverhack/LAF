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
        positions.clear();
        boundings.clear();
    }
    
    friend ostream& operator<< (ostream &os, MovieInfo &mI);
    
    string name = "";
    string path = "";
    int frame = 0; // this is the frame within the loop (ie, 0 == startframe)
    int startframe = 0; // this is the start frame in the actual video
    int endframe = 0; // this is the end frame in the atual video
    float speed = 3.0;
    string markername = "";
    ofRectangle bounding = ofRectangle(0,0,0,0);
    ofPoint position = ofPoint(0,0,0);
    ofPoint centre = ofPoint(0,0,0);
    vector<ofPoint> positions;
    vector<ofRectangle> boundings;
    vector<ofPoint> centres;
    ofRectangle totalbounding;
    
    bool isLooped = false;
    bool isLoopedStatic = false;
    int staticLoopFrames = 20;
    int agentActionIndex = -1;
    bool isCut = false;
    bool isEnd = false; // if the movie is part of the end motion (including the transitions leading to it)
    
    string currentDirection;
    string getCurrentDirection() {
        // markername
        
        return ofSplitString(markername, "_")[1];
    }

};

//inline bool operator==(const MovieInfo& lhs, const MovieInfo& rhs){ /* do actual comparison */ }
//inline bool operator< (const MovieInfo& lhs, const MovieInfo& rhs){ /* do actual comparison */ }
//inline bool operator!=(const MovieInfo& lhs, const MovieInfo& rhs){return !operator==(lhs,rhs);}
//inline bool operator> (const MovieInfo& lhs, const MovieInfo& rhs){return  operator< (rhs,lhs);}
//inline bool operator<=(const MovieInfo& lhs, const MovieInfo& rhs){return !operator> (lhs,rhs);}
//inline bool operator>=(const MovieInfo& lhs, const MovieInfo& rhs){return !operator< (lhs,rhs);}

inline ostream& operator<<(ostream& os, MovieInfo &mI){
    os  << mI.markername << " "
        << mI.name << " "
        << mI.bounding << " ["
        << mI.startframe << " -> "
        << mI.endframe << "] "
        << mI.frame;
    return os;
};


   
    
static MovieInfo NoMovie;
    
#endif
