//
//  AnalyzeController.h
//  LaughterForgetting
//
//  Created by gameover on 10/01/14.
//
//

#ifndef _H_ANALYZECONTROLLER
#define _H_ANALYZECONTROLLER

#include "BaseController.h"
#include "AppModel.h"

class AnalyzeController : public BaseController {
    
public:
	
    AnalyzeController();
    ~AnalyzeController();
    
    void setup();
    void update();
    
protected:
	
    ofDirectory mediaDirectory;
    
    int playersNeedingAnalysis;
    
private:
	
};

#endif
