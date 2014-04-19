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
//            return;
        }
        
        if(video->getSpeed() != speed){
            video->setSpeed(speed);
        }
        
        if(bPaused){
            return;
        }
        
        if(video->isFrameNew()){
            
            // update frame info
            updateFrame();
            
            // update position info
            updatePosition();
            
            // render
            //updateRender();
            
        }
        
        
        if (pauseFrame > -1 && pauseFrame >= currentMovie.frame) {
            stop();
            pauseFrame = -1;
        }

        
        // check if we're done TODO: reverse
        if((video->getIsMovieDone()) || currentMovie.frame + currentMovie.startframe >= currentMovie.endframe
        || (speed < 0 && currentMovie.frame  < 0)
           ) {
            
            // check if the next movie belongs to another action and if we want to stop now
            if ((speed > 0 && sequence[CLAMP(currentSequenceIndex+1,0,sequence.size()-1)].agentActionIndex == pauseActionIndex) ||
                (speed < 0 && sequence[CLAMP(currentSequenceIndex-1,0,sequence.size()-1)].agentActionIndex == pauseActionIndex)) {
                
                stop();
                setSpeed(abs(getSpeed()));
                pauseActionIndex = -1;
            }
            else
                loadNextMovie();
        }
        
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
    
    void loadNextMovie(int frameSeek = 0){
        
        ofxLogVerbose() << "Loading next movie: " << currentSequenceIndex << " -> " << currentSequenceIndex + 1 << " of " << sequence.size() << endl;
        
        // increment sequenceIndex TODO: reverse
        if(speed>0 && currentSequenceIndex + 1 < sequence.size()){
            bSequenceIsDone = false;
            currentSequenceIndex++;
        }
        else if (speed< 0 && currentSequenceIndex > 0) {
            bSequenceIsDone = false;
            currentSequenceIndex--;
        }
        else if (currentMovie.isLooped) {
            bSequenceIsDone = false;
        }
        else{
            // TODO: add loop?
            //currentSequenceIndex = 0;
            if (bAutoSequenceStop) bSequenceIsDone = true;
            stop();
            return;
        }
        
        MovieInfo& nextMovie = sequence[currentSequenceIndex];
        
        if(nextMovie.agentActionIndex != currentMovie.agentActionIndex){
            ofxLogVerbose() << "Starting to perform the next action old = " << currentMovie.agentActionIndex << "  new = " << nextMovie.agentActionIndex << endl;
            cout << endl;
           // if (currentMovie.agentActionIndex!=-1) stop();
        }
        
        nextMovie.speed = speed; //ASK: do we update the speed in each movie or for the sequence
        
        if(nextMovie.path != currentMovie.path){
            ofxLogVerbose() << "Loading next movie: " << nextMovie.name << " " << nextMovie.startframe << endl;
            video->loadMovie(nextMovie.path);
            video->setLoopState(OF_LOOP_NONE);
            video->play();
        }else{
            ofxLogVerbose() << "Loading same movie: " << nextMovie.name << " " << nextMovie.startframe << endl;
        }
        
        if(speed> 0){
            video->setFrame(nextMovie.startframe + frameSeek);
            video->setSpeed(nextMovie.speed);
        }else{
            video->setFrame(nextMovie.endframe);
            video->setSpeed(nextMovie.speed);
        }
        
        
        currentMovie = nextMovie;
        
    }
    
    void play(){
        ofxLogVerbose() << "Play Sequence" << endl;
        setPaused(false);
        bSequenceIsDone = false;
        pauseFrame = -1;
        loadNextMovie();
    }
    
    void stop(){
        ofxLogVerbose() << "Stop Sequence" << endl;
        setPaused(true);
    }
    
    
    void StopAt(int frame) {
         ofxLogVerbose() << "Stoping current movie at frame " << frame << "" << endl;
        pauseFrame = frame;
    }
    
    void stopAtAction(int actionIndex) {
        pauseActionIndex = actionIndex;
    }
    
    bool isSequequenceDone(){
        return bSequenceIsDone;
    }
    
    void setFrame(int f){
        int frameSeek = f;
        for(int i = 0; i < sequenceFrames.size() - 1; i++){
            frameSeek += sequenceFrames[i];
            if(f > sequenceFrames[i] && f < sequenceFrames[i + 1]){
                currentSequenceIndex = i - 1;
                loadNextMovie(frameSeek);
            }
        }
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
        if(video->isPlaying()) video->setPaused(bPaused);
    }
    
    bool isPlaying(){
        return !bPaused; // Changed to ! by omid; TODO: Ask matt
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
    
    void pushAt(MovieInfo m, int index){
        ostringstream os; os << m;
        ofxLogVerbose() << "Push Sequence: " << os.str() << endl;
        sequence.insert(sequence.begin()+index, m);
        rebuildSequenceFrames();
    }
    
    void fixLastSequenceFrame(int oldLength, int newLength) {
        totalSequenceFrames -= oldLength;
        sequenceFrames[sequenceFrames.size()-1] = totalSequenceFrames+newLength;
        totalSequenceFrames += newLength;
    }
    
    void rebuildSequenceFrames() {
        totalSequenceFrames = 0;
        sequenceFrames.clear();
        
        sequenceFrames.push_back(0);
        
        for (int i=0;i<sequence.size();i++) {
            sequenceFrames.push_back(totalSequenceFrames + sequence[i].endframe - sequence[i].startframe);
            totalSequenceFrames += sequence[i].endframe - sequence[i].startframe;
        }
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
        bAutoSequenceStop = true;
        
        pauseActionIndex = -1;
        pauseFrame = -1;
        
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
    
    void setAutoSequenceStop(bool b){
        bAutoSequenceStop = b;
    }
    
    bool getAutoSequenceStop(){
        return bAutoSequenceStop;
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
    
    void shiftPosition(char direction, float length) {
        ofPoint t;
        if (direction == 'l')
            t = ofPoint (-length,0,0);
        else if (direction == 'r')
            t = ofPoint (length,0,0);
        else if (direction == 'u')
            t = ofPoint (0,-length,0);
        else if (direction == 'd')
            t = ofPoint (0,-length,0);
        
        normalise(0,t);
    }
    
    void normalise(int startNormalIndex = 0, ofPoint lNormal = ofPoint(0,0,0)){

        // set up temp store vars
        ofPoint     position;
        ofRectangle bounding;
        ofPoint     centre;
        
        ofPoint     sposition;
        ofRectangle sbounding;
        ofPoint scentre;
        
        ofPoint mNormal;
//        ofPoint lNormal = ofPoint(0,0,0);
        
//        int startNormalIndex = 0;
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
            
            lNormal = position + ofPoint(0,0);
            
        }
        
        // scale and transform the total bounding box
        stotalBounding = totalBounding;
        stotalBounding.position = stotalBounding.position * scale + pNormal;
        stotalBounding.scale(scale);
        
        lastnormalindex = sequence.size() - 1;
        
    }
    
    ofPoint getScaledFloorOffset(){
        return getScaledFloorOffsetAt(currentSequenceFrame);
    }
    
    ofPoint getScaledFloorOffsetAt(int f){
        ofRectangle r = getScaledBoundingAt(f); //TODO: there's a bug with frame 0!!!
        ofPoint floorOffset;
        floorOffset.x = (r.x + r.width / 2.0);
        floorOffset.y = (r.y + r.height) + 0; // TODDO: needs a plus if i'm doing collisions!
        return floorOffset;
    }
    
    MovieInfo& getLastMovieInSequence(){
        return sequence[sequence.size() - 1];
    }
    
    ofPoint& getScaledPosition(){
        return getScaledPositionAt(currentSequenceFrame);
    }
    
    ofRectangle& getScaledBounding(){
        return getScaledBoundingAt(currentSequenceFrame);
    }
    
    ofPoint& getScaledCentre(){
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
    
    ofRectangle& getOrgScaledBoundingAt(int sequenceFrame){
        sequenceFrame = CLAMP(sequenceFrame, 0, positions.size() - 1);
        return orgsboundings[sequenceFrame];
    }
    
    vector<ofRectangle>& getOrgScaledBoundings(){
        return orgsboundings;
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
    
    ofPoint shiftPoint;
    
    void removeMoviesFromIndex (int ind) {
        
        ind++;
        sequence.erase(sequence.begin()+ind,sequence.end()+1);
//        sequence.erase(sequence.end());
        
         for (int i=sequenceFrames.size()-1; i>=ind;i--) {
             totalSequenceFrames -= sequenceFrames[i] - sequenceFrames[i-1];
         }
        
        currentSequenceFrame = totalSequenceFrames + currentMovie.frame;
        
        sequenceFrames.erase(sequenceFrames.begin()+ind,sequenceFrames.end());
        //sequenceFrames.erase(sequenceFrames.end());
        //Remember to normalise after
    }
    
    vector<int> getSequenceFrames() {
        return sequenceFrames;
    }
    
    void storeSBoundings() {
        orgsboundings = vector<ofRectangle>(sboundings);
    }
    
protected:
    
    float speed;
    bool bPaused;
    bool bSequenceIsDone;
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
    
    vector<ofRectangle> orgsboundings;

    
    ofRectangle totalBounding;
    ofRectangle stotalBounding;
    
    int pauseFrame;
    int pauseActionIndex;
    bool bAutoSequenceStop;
};



inline ostream& operator<<(ostream& os, MovieSequence *mS){
    os << mS->getCurrentMovie() << " || " << mS->getCurrentMovieIndex() << " / " << mS->getSequenceSize() << " " << mS->getCurrentSequenceFrame() << " / " << mS->getTotalSequenceFrames();
    ofxThreadedVideo * video = mS->getVideo();
    if(video != NULL){
        os << "  " << video->getCurrentFrame() << " - " << video->getTotalNumFrames() << "  " << video->getIsMovieDone() << "  " << video->isPaused();
    }
    return os;
    };
    
    inline ostream& operator<<(ostream& os, MovieSequence &mS){
        os << mS.getCurrentMovie() << " || " << mS.getCurrentMovieIndex() << " / " << mS.getSequenceSize() << " " << mS.getCurrentSequenceFrame() << " / " << mS.getTotalSequenceFrames();
        ofxThreadedVideo * video = mS.getVideo();
        if(video != NULL){
            os << "  " << video->getCurrentFrame() << " - " << video->getTotalNumFrames() << "  " << video->getIsMovieDone() << "  " << video->isPaused();
        }
        return os;
    };

#endif
