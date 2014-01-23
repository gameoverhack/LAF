//
//  Kalman.cpp
//  ofApp
//
//  Created by gameover on 3/08/13.
//
//

#include "Kalman.h"

using namespace ofxCv;
using namespace cv;

// based on code from:
// http://www.morethantechnical.com/2011/06/17/simple-kalman-filter-for-tracking-using-opencv-2-2-w-code/


Kalman::Kalman(){
    
}

Kalman::~Kalman(){
    
}

void Kalman::setup(int _numVariables, int _numDimensions){
    
    numVariables = _numVariables;
    numDimensions = _numDimensions;
    
    int dynamParams = 2 * numVariables * numDimensions;
    int measureParams = numVariables * numDimensions;
    
    KF.init(dynamParams, measureParams, 0);
    KF.transitionMatrix = Mat_<float>::zeros(dynamParams, dynamParams);
    
    for(int i = 0; i < dynamParams; i++){
        
        KF.statePre.at<float>(i) = 0.0f;
        
        for(int j = 0; j < dynamParams; j++){
            if(i == j){
                KF.transitionMatrix.at<float>(i, j) = 1.0f;
                if(i + 6 < dynamParams - 1) KF.transitionMatrix.at<float>(i + 6, j) = 1.0f;
            }
            
        }
    }
    
	measurement = Mat_<float>::zeros(measureParams, 1);
	
	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(KF.errorCovPost, Scalar::all(.1));
    
}

cv::Mat& Kalman::getCorrected(){
    return corrected;
}

cv::Mat& Kalman::getPredicted(){
    return prediction;
}

void Kalman::setMeasured(vector<ofPoint>& _measurement){
    
    //    assert(_measurement.size() == numVariables * numDimensions);
    
    for (int var = 0; var < numVariables; var++) {
        for (int dim = 0; dim < numDimensions; dim++) {
            measurement(dim + var * numDimensions) = _measurement[var][dim];
        }
    }
    
    prediction = KF.predict();
    corrected = KF.correct(measurement);
    
}