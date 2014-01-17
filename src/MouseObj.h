//
//  MouseObj.h
//  emptyExample
//
//  Created by game over on 5/04/13.
//  Copyright (c) 2013 trace media. All rights reserved.
//

#ifndef _H_MOUSEOBJ
#define _H_MOUSEOBJ

#include "ofMain.h"

class MouseObj {
    
public:
	
    MouseObj(){
        setPosition(0.0f, 0.0f);
    };
    
    MouseObj(float _x, float _y, float _width = 100.0f, float _height = 100.0f){
        set(_x, _y, _width, _height);
    };
    
    ~MouseObj(){
        // clean up
    };
    
    void set(float _x, float _y, float _width, float _height){
        setPosition(_x, _y);
        setSize(_width, _height);
    }
    
    void setPosition(float _x, float _y){
        x = _x;
        y = _y;
    };
    
    void setSize(float _width, float _height){
        width = _width;
        height = _height;
    };
    
    void draw(){
        ostringstream os;
        os << x << ", " << y << ", " << width << ", " << height;
        ofPushMatrix();
        ofTranslate(x, y);
        ofNoFill();
        ofDrawBitmapString(os.str(), 0, -14);
        ofRect(0, 0, width, height);
        ofPopMatrix();
    }
    
    bool inside(float _x, float _y){
        return ofRectangle(x, y, width, height).inside(_x, _y);
    }
    
    ofPoint getPosition(){
        ofPoint p = ofPoint(x, y);
        return p;
    };
    
    float getWidth(){
        return width;
    }
    
    float getHeight(){
        return height;
    }
    
    void setWidth(float _width){
        width = _width;
    }
    
    void setHeight(float _height){
        height = _height;
    }
    
protected:
	
    float x, y, width, height;
    
private:
	
};

#endif
