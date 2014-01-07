#pragma once

#include "ofMain.h"

#define NUM_VIDS 12

#include "AppModel.h"
#include "PlayerController.h"

class testApp : public ofBaseApp{
	
public:
    
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
    void exit();
    
    bool checkRandom();
    
    ofEasyCam cam;
    
    vector<PlayerController*> pVideos;
    
    vector<int> iRandom;
    int numPlayers;
    
    bool bShowInfo;
    bool bShowWindows;
    bool bUseOrtho;
    
};
