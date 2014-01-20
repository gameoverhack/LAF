//
//  AppController.h
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#ifndef __H_APPCONTROLLER
#define __H_APPCONTROLLER

#include "BaseController.h"
#include "AppModel.h"
#include "AppView.h"
#include "DebugView.h"
#include "PlayController.h"
#include "AnalyzeController.h"

class AppController : public BaseController{
    
public:
    
    AppController();
    ~AppController();
    
    void setup();
    void update();
    void draw();
    
    void exit();
    void restart();
    
    void keyPressed(ofKeyEventArgs & e);
    void keyReleased(ofKeyEventArgs & e);
    
    void mouseMoved(ofMouseEventArgs & e);
    void mouseDragged(ofMouseEventArgs & e);
    void mousePressed(ofMouseEventArgs & e);
    void mouseReleased(ofMouseEventArgs & e);
    
protected:
    
    bool resize;
    float offsetX, offsetY;
    
    AppView * appView;
    DebugView * debugView;
    
    AnalyzeController * analyzeController;
    PlayController * playController;
    
};

#endif /* defined(__LAFTest__AppController__) */
