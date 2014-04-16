//
//  PhilippeModel.h
//  LaughterForgetting
//
//  Created by gameover on 16/04/14.
//
//

#ifndef __COSHESPECIAL
#define __COSHESPECIAL

#include "BaseModel.h"
#include "ofxNetwork.h"
#include "ofxOSC.h"

class PhilippeModel : public BaseModel{
    
public:
    
    PhilippeModel(){
        //BaseModel::BaseModel();
    }
    
    ~PhilippeModel(){
        //BaseModel::~BaseModel();
    }

    ofxOscSender& getOSCSender(){
        return OSCSender;
    }
    
protected:
    
    ofxOscSender OSCSender;
    
};

typedef Singleton<PhilippeModel> PhilippeModelSingleton;					// Global declaration

static PhilippeModel * philModel	= PhilippeModelSingleton::Instance();

#endif
