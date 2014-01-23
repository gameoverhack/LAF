//
//  Pointer.h
//  LaughterForgetting
//
//  Created by gameoverlf on 1/22/2014.
//
//

#ifndef __H_POINTER
#define __H_POINTER

class Pointer {
    
public:
    
    Pointer(){
        reset();
    }
    ~Pointer(){}
    
    void reset(){
        position = ofPoint(ofGetWidth() / 2.0f, ofGetHeight() / 2.0f);
    }
    
    void draw(){
        
        //        ofPushStyle();
        ofPushMatrix();
        ofFill();
        ofCircle(position.x, position.y, 5);
        ofNoFill();
        ofPopMatrix();
        //        ofPopStyle();
        
    }
    
    ofPoint position;
    
};

#endif
