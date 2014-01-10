//
//  PlayerController.h
//  LAFTest
//
//  Created by gameover on 30/12/13.
//
//

#ifndef __H_PLAYERCONTROLLER
#define __H_PLAYERCONTROLLER

#include "ofMain.h"
#include "VectorUtils.h"
#include "ofxXMP.h"
#include "ofxThreadedVideo.h"
#include "AppModel.h"
#include "PlayerModel.h"
#include "PlayerView.h"

class PlayerController{
    
public:
    
    PlayerController(){
        
    }
    
    ~PlayerController(){
        
    }
    
    void setup(PlayerModel & playerModel){
        
//        bTransition = false;
//        
//        model = playerModel;
//        model.reset();
//        
//        view.setup(model.getWidth(), model.getHeight(), 2);
//        view.setTransitionLength(12);
//        
//        video.setPixelFormat(OF_PIXELS_BGRA);
//        
//        loadMovie(model.getNextMovieInfo());
        
    }
    
    void update(){
        
//        video.update();
//        model.updateMovie(video);
//        
//        MovieInfo& mInfo = model.getCurrentMovieInfo();
//                
//        if(mInfo.isFrameNew){
//            view.setPixels(video.getPixelsRef());
//        }
//        
//        view.update();
//        
//        if(!mInfo.isMovieDirty && model.nextMovieNeeded() && bCueTransition){
//            cout << mInfo.frame << " " << mInfo.name << endl;
//            view.setTransitionEndPixels(video.getPixelsRef());
//            view.setRenderMode(RENDER_TRANSITION);
//            bCueTransition = false;
//            bTransition = true;
//        }
//        
//        if(!mInfo.isMovieDirty && model.nextMovieNeeded() && bTransition){
//            cout << "Transition over" << endl;
//            bCueTransition = bTransition = false;
//            model.resetMovieNeeded();
//        }
//        
//        if(!mInfo.isMovieDirty && model.nextMovieNeeded() && !bCueTransition && !bTransition){
//            bCueTransition = true;
//            view.setTransitionStartPixels(video.getPixelsRef());
//            MovieInfo& nInfo = model.getNextMovieInfo();
//            cout << "Set video to go: " << nInfo.frame << " " << nInfo.name << endl;
//            if(nInfo.name == mInfo.name){
//                video.setFrame(nInfo.frame);
//            }else{
//                loadMovie(nInfo);
//            }
//        }
        

        
    }
    
    float getWidth(){
        return model.getWidth();
    }
    
    float getHeight(){
        return model.getHeight();
    }
    
//    float getDrawScale(){
//        return model.getDrawScale();
//    }
//    
//    ofRectangle& getBounding(){
//        return model.getBounding();
//    }
//    
//    ofPoint& getPosition(){
//        return model.getPosition();
//    }
    
    PlayerModel& getModel(){
        return model;
    }
    
    PlayerView& getView(){
        return view;
    }
    
protected:
    
//    void loadMovie(MovieInfo& m){
//        
//        cout << "Load Movie: " << m.path << "   " << m.frame << endl;
//        
//        video.loadMovie(m.path);
//        video.setLoopState(OF_LOOP_NONE);
//        video.play();
//        video.setFrame(m.frame);
//        video.setSpeed(m.speed);
//        
//        model.updateMovie(video);
//        
//    }
    
    bool bTransition, bCueTransition;
    
    ofxThreadedVideo    video;
    
    PlayerModel         model;
    PlayerView          view;
    
};

#endif
