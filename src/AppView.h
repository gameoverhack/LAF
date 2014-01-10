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
    
};

#endif
