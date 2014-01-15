//
//  MovieSequence.h
//  LaughterForgetting
//
//  Created by gameover on 14/01/14.
//
//

#ifndef __H_MOVIESEQUENCE
#define __H_MOVIESEQUENCE

#include "MovieInfo.h"
#include "ofxLogger.h"
#include "ofxThreadedVideo.h"

class MovieSequence{
    
public:
    
    MovieSequence(){
        video = NULL;
        clear();
    }
    
    ~MovieSequence(){
        clear();
    }
    
    ofxThreadedVideo* getVideo(){
        return video;
    }
    
    void setVideo(ofxThreadedVideo * v, int index){
        ofxLogVerbose() << "Assigning video player" << endl;
        video = v;
        viewID = index;
    }
    
    int getViewID(){
        return viewID;
    }
    
    void update(){
        
        // don't try this on a NULL
        if(video == NULL) return;
        
        // update the video
        video->update();
        
        // make sure there are no commands cue'd on the video
        if(video->getQueueSize() > 0) return;
        
        // make sure the video paused state is the same as the sequence
        if(video->isPaused() != bPaused){
            video->setPaused(bPaused);
            return;
        }
        
        if(video->getSpeed() != speed){
            video->setSpeed(speed);
        }
        
        if(bPaused) return;
        
        if(video->isFrameNew()){
            
            // update frame info
            updateFrame();
            
            // update position info
            updatePosition();
            
            // render
            //updateRender();
            
        }

        // check if we're done TODO: reverse
        if(video->getIsMovieDone() || currentMovie.frame + currentMovie.startframe >= currentMovie.endframe) loadNextMovie();
        
    }
    
    void updateFrame(){
        // calculate frame info
        currentMovie.frame = video->getCurrentFrame() - currentMovie.startframe;
        currentSequenceFrame = sequenceFrames[currentSequenceIndex];
        currentSequenceFrame = currentSequenceFrame + currentMovie.frame;
    }
    
    void updatePosition(){
        if(currentSequenceFrame >= 0 && currentSequenceFrame < positions.size()){
            currentMovie.position   = spositions[currentSequenceFrame];
            currentMovie.bounding   = sboundings[currentSequenceFrame];
            currentMovie.centre     = scentres[currentSequenceFrame];
        }
        
    }
    
    void loadNextMovie(){
        
        ofxLogVerbose() << "Loading next movie: " << currentSequenceIndex << " -> " << currentSequenceIndex + 1 << " of " << sequence.size() << endl;
        
        // increment sequenceIndex TODO: reverse
        if(currentSequenceIndex + 1 < sequence.size()){
            currentSequenceIndex++;
        }
        else{
            // TODO: add loop?
            //currentSequenceIndex = 0;
            stop();
            return;
        }
        
        MovieInfo& nextMovie = sequence[currentSequenceIndex];
        
        if(nextMovie.path != currentMovie.path){
            ofxLogVerbose() << "Loading next movie: " << nextMovie.name << " " << nextMovie.startframe << endl;
            video->loadMovie(nextMovie.path);
            video->setLoopState(OF_LOOP_NONE);
            video->play();
        }else{
            ofxLogVerbose() << "Loading same movie: " << nextMovie.name << " " << nextMovie.startframe << endl;
        }
        video->setFrame(nextMovie.startframe);
        video->setSpeed(nextMovie.speed);
        
        currentMovie = nextMovie;
        
    }
    
    void play(){
        ofxLogVerbose() << "Play Sequence" << endl;
        setPaused(false);
        loadNextMovie();
    }
    
    void stop(){
        ofxLogVerbose() << "Stop Sequence" << endl;
        setPaused(true);
    }
    
    bool isSequequenceDone(){
        if(video == NULL) return false;
        return (getCurrentSequenceFrame() >= getTotalSequenceFrames());
    }
    
    void setSpeed(float s){
        speed = s;
    }
    
    float getSpeed(){
        return speed;
    }
    
    void setPaused(bool b){
        ofxLogVerbose() << "Pause Sequence: " << b << endl;
        bPaused = b;
    }
    
    bool isPlaying(){
        return bPaused;
    }
    
    void rewind(){
        ofxLogVerbose() << "Rewind Sequence" << endl;
        currentSequenceIndex = -1;
        currentSequenceFrame = 0;
    }
    
    void push(MovieInfo m){
        ostringstream os; os << m;
        ofxLogVerbose() << "Push Sequence: " << os.str() << endl;
        sequence.push_back(m);
        sequenceFrames.push_back(totalSequenceFrames + m.endframe - m.startframe);
        totalSequenceFrames += m.endframe - m.startframe;
        //normalise();
    }
    
    void clear(){
        ofxLogVerbose() << "Clear Sequence" << endl;
        bPaused = false;
        sequence.clear();
        sequenceFrames.clear();
        sequenceFrames.push_back(0);
        positions.clear();
        boundings.clear();
        pNormal = ofPoint(0,0,0);
        scale = 1.0f;
        currentMovie = NoMovie;
        currentSequenceIndex = lastnormalindex = -1;
        currentSequenceFrame = totalSequenceFrames = 0;
    }
    
    void setNormalPosition(ofPoint p){
        pNormal = p;
        lastnormalindex = -1;
        ofxLogWarning() << "Remember to call normalize() or rescale() after setting position" << endl;
    }
    
    void setNormalScale(float s){
        scale = s;
        lastnormalindex = -1;
        ofxLogWarning() << "Remember to call normalize() or rescale() after setting scale" << endl;
    }
    
    ofPoint getNormalPosition(){
        return pNormal;
    }
    
    float getNormalScale(){
        return scale;
    }
    
    void rescale(){
        
        ofxLogVerbose() << "Rescale Sequence: " << pNormal << " " << scale << endl;
        
        // copy positions/boundings
        spositions = positions;
        sboundings = boundings;
        scentres = centres;
        
        // scale and transform the total bounding box
        stotalBounding = totalBounding;
        stotalBounding.position = stotalBounding.position * scale + pNormal;
        stotalBounding.scale(scale);
        
        // rescale and move them by pNormal position
        for(int i = 0; i < positions.size(); i++){
            
            spositions[i] = spositions[i] * scale + pNormal;
            sboundings[i].position = sboundings[i].position * scale + pNormal;
            sboundings[i].scale(scale);
            scentres[i] = sboundings[i].getCenter();
            
        }
        
    }
    
    void normalise(){

        // set up temp store vars
        ofPoint     position;
        ofRectangle bounding;
        ofPoint     centre;
        
        ofPoint     sposition;
        ofRectangle sbounding;
        ofPoint scentre;
        
        ofPoint mNormal;
        ofPoint lNormal = ofPoint(0,0,0);
        
        int startNormalIndex = 0;
//        if(lastnormalindex == -1){
        
            ofxLogVerbose() << "Normalize Whole Sequence" << endl;
            
            // clear all the pos/bound storage
            positions.clear();
            boundings.clear();
            centres.clear();
            spositions.clear();
            sboundings.clear();
            scentres.clear();
            
//        }else{
//            ofxLogVerbose() << "Normalize Part Sequence: " << lastnormalindex << " < " << sequence.size() << endl;
//            startNormalIndex = lastnormalindex;
//            lNormal = positions[positions.size() - 1];
//        }
        
        for(int i = startNormalIndex; i < sequence.size(); i++){
            
            // store an offset based on first
            // frame of keyframes in this movie/loop
            MovieInfo& m = sequence[i];
            mNormal = m.positions[1]; //TODO check this first frame issue!!!
            
            for(int j = 0; j < m.positions.size(); j++){
                
                // calculate offset based on first frame and last frame of last movie
                position = lNormal + (mNormal - sequence[i].positions[j]);
                bounding = sequence[i].boundings[j];
                
                // offset the bounding box by the position
                bounding.position = bounding.position + position;
                centre = bounding.getCenter();
                
                // store it
                positions.push_back(position);
                boundings.push_back(bounding);
                centres.push_back(centre);
                
                // may as well calculate scaled
                // pos/bounds while we're here
                
                // make a copy of the point
                sposition = position;
                sbounding = bounding;
                
                // scale and transform it
                sposition = sposition * scale + pNormal;
                sbounding.position = sbounding.position * scale + pNormal;
                sbounding.scale(scale);
                scentre = sbounding.getCenter();
                
                // store 'em
                spositions.push_back(sposition);
                sboundings.push_back(sbounding);
                scentres.push_back(scentre);
                
                // calculate a rectangle that would fit all the bounding boxes inside
                if(i == 0 && j == 0) totalBounding = bounding;
                totalBounding.growToInclude(bounding);
                
            }
            
            lNormal = position;
            
        }
        
        // scale and transform the total bounding box
        stotalBounding = totalBounding;
        stotalBounding.position = stotalBounding.position * scale + pNormal;
        stotalBounding.scale(scale);
        
        lastnormalindex = sequence.size() - 1;
        
    }
    
    ofPoint getScaledFloorOffset(){
        ofRectangle r = getScaledBoundingAt(1); //TODO: there's a bug with frame 0!!!
        ofPoint floorOffset;
        floorOffset.x = (r.x + r.width / 2.0);
        floorOffset.y = (r.y + r.height) + 4;
        return floorOffset;
    }
    
    MovieInfo& getLastMovieInSequence(){
        return sequence[sequence.size() - 1];
    }
    
    ofPoint getScaledPosition(){
        return getScaledPositionAt(currentSequenceFrame);
    }
    
    ofRectangle& getScaledBounding(){
        return getScaledBoundingAt(currentSequenceFrame);
    }
    
    ofPoint getScaledCentre(){
        return getScaledCentreAt(currentSequenceFrame);
    }
    
    ofPoint& getScaledPositionAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return spositions[sequenceFrame];
    }
    
    ofRectangle& getScaledBoundingAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return sboundings[sequenceFrame];
    }
    
    ofPoint& getScaledCentreAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return scentres[sequenceFrame];
    }
    
    ofPoint& getPosition(){
        return getPositionAt(currentSequenceFrame);
    }
    
    ofRectangle& getBounding(){
        return getBoundingAt(currentSequenceFrame);
    }
    
    ofPoint& getCentre(){
        return getCentreAt(currentSequenceFrame);
    }
    
    ofPoint& getPositionAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return positions[sequenceFrame];
    }
    
    ofRectangle& getBoundingAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return boundings[sequenceFrame];
    }
    
    ofPoint& getCentreAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return centres[sequenceFrame];
    }
    
    ofRectangle& getTotalBounding(){
        return totalBounding;
    }
    
    ofRectangle& getScaledTotalBounding(){
        return stotalBounding;
    }
    
    vector<MovieInfo>& getMovieSequence(){
        return sequence;
    }
    
    string getMovieSequenceAsString(){
        ostringstream os; os << endl;
        for(int i = 0; i < sequence.size(); i++){
            os << sequence[i] << endl;
        }
        return os.str();
    }
    
    MovieInfo& getCurrentMovie(){
        return currentMovie;
    }
    
    int getCurrentSequenceFrame(){
        return currentSequenceFrame;
    }
    
    int getTotalSequenceFrames(){
        return totalSequenceFrames;
    }
    
    int getCurrentMovieIndex(){
        return currentSequenceIndex;
    }
    
    int getSequenceSize(){
        return sequence.size();
    }
    
    friend ostream& operator<< (ostream &os, MovieSequence &mS);
    
protected:
    
    float speed;
    bool bPaused;
    int viewID;
    
    ofxThreadedVideo * video;
    
    vector<MovieInfo> sequence;
    vector<int> sequenceFrames;
    
    MovieInfo currentMovie;
    
    int currentSequenceIndex;
    int currentSequenceFrame;
    int totalSequenceFrames;
    
    int lastnormalindex;
    ofPoint pNormal;
    float scale;
    
    vector<ofPoint>     positions;
    vector<ofRectangle> boundings;
    vector<ofPoint>     centres;
    
    vector<ofPoint>     spositions;
    vector<ofRectangle> sboundings;
    vector<ofPoint>     scentres;
    
    ofRectangle totalBounding;
    ofRectangle stotalBounding;
    
};

inline ostream& operator<<(ostream& os, MovieSequence *mS){
    os << mS->getCurrentMovie() << " || " << mS->getCurrentMovieIndex() << " / " << mS->getSequenceSize() << " " << mS->getCurrentSequenceFrame() << " / " << mS->getTotalSequenceFrames();
    return os;
    };
    
    inline ostream& operator<<(ostream& os, MovieSequence &mS){
        os << mS.getCurrentMovie() << " || " << mS.getCurrentMovieIndex() << " / " << mS.getSequenceSize() << " " << mS.getCurrentSequenceFrame() << " / " << mS.getTotalSequenceFrames();
        return os;
    };

#endif
