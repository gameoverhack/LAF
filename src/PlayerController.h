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
        
        ofxLogNotice() << "Setting up PlayerController with " << playerModel.getPlayerName() << endl;
     
        model = playerModel;
       
        view.setup(model.getWidth(), model.getHeight(), 2);
        view.setTransitionLength(12);

        video.setPixelFormat(OF_PIXELS_BGRA);
        
        movieCue.push_back(model.getStartMovie());
        
        bCueTransition = false;
        
    }
    
    void update(){
        
        video.update();

        currentMovie.isMovieDirty = (video.getQueueSize() > 0);
        
        if(!currentMovie.isMovieDirty){
            
            currentMovie.frame = video.getCurrentFrame();
            currentMovie.totalframes = video.getTotalNumFrames();
            currentMovie.speed = video.getSpeed();
            currentMovie.name = ofSplitString(video.getMovieName(), ".mov")[0];
            currentMovie.path = video.getMoviePath();
            currentMovie.isFrameNew = video.isFrameNew();

            if(currentMovie.isFrameNew){
                view.setPixels(video.getPixelsRef());
                
                ofPoint kFrame = model.getKeyFrame(currentMovie.name, currentMovie.frame);
                currentMovie.position = pNormal - (kFrame - kNormal) * scale;
                pNormal = currentMovie.position;
                kNormal = kFrame;
                
                currentMovie.bounding = model.getScaledRectFrame(currentMovie.name, currentMovie.frame, currentMovie.position, scale);
            }

        
            if(bCueTransition){
                view.setTransitionEndPixels(video.getPixelsRef());
                view.setRenderMode(RENDER_TRANSITION);
                bCueTransition = false;
            }

            if(currentMovie.frame >= currentMovie.genframe || currentMovie.name == ""){
                
                if(movieCue.size() > 0 && !bCueTransition){
                    
                    bCueTransition = true;
                    view.setTransitionStartPixels(video.getPixelsRef());
                    loadMovie(movieCue[0]);
//                    movieCue.pop_front();
                    
                }else if(currentMovie.name != ""){ //if(movieCue.size() > 0){
                    
                    video.setFrame(currentMovie.startframe);
                    
                }
            }
            
        }
        

        view.update();
        
    }
    
    //--------------------------------------------------------------
    void setDrawScale(float s){
        scale = s;
    }
    
    //--------------------------------------------------------------
    float getDrawScale(){
        return scale;
    }

    //--------------------------------------------------------------
    ofRectangle& getBounding(){
        return currentMovie.bounding;
    }

    //--------------------------------------------------------------
    ofPoint& getPosition(){
        return currentMovie.position;
    }
    
    //--------------------------------------------------------------
    PlayerModel& getModel(){
        return model;
    }
    
    //--------------------------------------------------------------
    PlayerView& getView(){
        return view;
    }
    
    //--------------------------------------------------------------
    MovieInfo& getCurrentMovieInfo(){
        return currentMovie;
    }
    
    void setNormalPosition(ofPoint p){
        ofRectangle r = model.getRectFrame(model.getStartMovie().name, 1);
        pNormal = p;
        pNormal.y -= (r.height + r.y) * scale + 14;
    }
    
protected:
    
    void loadMovie(MovieInfo& m){
        
        cout << "Load Movie: " << m << endl;
        
        m.position = currentMovie.position;
        m.bounding = currentMovie.bounding;
        currentMovie = m;

        video.loadMovie(m.path);
        video.setLoopState(OF_LOOP_NONE);
        video.play();
        video.setFrame(m.frame);
        video.setSpeed(m.speed);

    }
    
    MovieInfo currentMovie;
    deque<MovieInfo> movieCue;
    
    bool bCueTransition;
    
    ofPoint pNormal, kNormal, oNormal;
    float scale;
    
    ofxThreadedVideo    video;
    
    PlayerModel         model;
    PlayerView          view;
    
};

#endif
