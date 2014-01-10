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

class AppController : public BaseController{
    
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
    
    AppView * appView;
    
};

#endif /* defined(__LAFTest__AppController__) */
