//
//  AppView.h
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#ifndef __H_APPVIEW
#define __H_APPVIEW

#include "AppModel.h"
#include "BaseView.h"

typedef struct{
    
    float cFade;
    int numPlayers;
    
} WindowFades;

class AppView : public BaseView{
    
public:
	
    AppView();
    ~AppView();
    
    void resetCamera();
    void setCameraOrtho(bool b);
    void toggleCameraOrtho();
    
    void update();
    
protected:
	
    ofEasyCam cam;
    ofFbo videoFBOBig;
    ofFbo videoFBOSmall;
    ofFbo videoFBOHero;
    
    ofShader shader;
    
    vector<WindowFades> windowFades;
    
};

#endif
