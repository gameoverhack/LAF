//
//  Kalman.h
//  ofApp
//
//  Created by gameover on 3/08/13.
//
//

#include "ofMain.h"
#include "ofxCv.h"

#ifndef __H_KALMANFILTER
#define __H_KALMANFILTER

class Kalman {
    
public:
    
    Kalman();
    ~Kalman();
    
    void setup(int numVariables, int numDimensions);
    
    void setMeasured(vector<ofPoint>& measurement);
    cv::Mat& getCorrected();
    cv::Mat& getPredicted();
    
protected:
    
    int numVariables;
    int numDimensions;
    
    cv::KalmanFilter KF;
	cv::Mat_<float> measurement;
	
	ofPolyline predicted, line, correct;
	ofPoint point;
    
    cv::Mat corrected, prediction;
    
private:
    
};

#endif
