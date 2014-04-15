//
//  RingBuffer.h
//  LaughterForgetting
//
//  Created by gameover on 14/04/14.
//
//

#ifndef __H_RINGBUFFER
#define __H_RINGBUFFER

#include "ofMain.h"
#include "VectorUtils.h"

class RingBuffer{
    
public:
    
    RingBuffer(){
        clear();
    }
    
    RingBuffer(int s, int d){
        resize(s, d);
    }
    
    ~RingBuffer(){
        clear();
    }
    
    void resize(int s, int d){
        
        clear();
        
        dimensions = d;
        
        buffer.resize(s);
        
        minimums.assign(dimensions, minimum);
        maximums.assign(dimensions, maximum);
        
        averages.assign(dimensions, 0.0f);
        differences.assign(dimensions, 0.0f);
        
        representation.resize(s);
        
        for(int i = 0; i < s; i++){
            buffer[i].resize(dimensions);
        }
        
    }
    
    void push(vector<float> data){
        
        assert(buffer.size() > 0);
        assert(data.size() == dimensions);
        
        for(int i = 0; i < dimensions; i++){
            
            // store data
            buffer[position][i] = data[i];
            
            averages[i] = getVecAvg(buffer[position]);
            differences[i] = buffer[position][i] - buffer[(position + 1 < buffer.size() ? position + 1 : 0)][i];

            // cache min and max values
            if(data[i] > maximums[i]) maximums[i] = data[i];
            if(data[i] < minimums[i]) minimums[i] = data[i];
            if(data[i] > maximum) maximum = data[i];
            if(data[i] < minimum) minimum = data[i];
            
        }
        
        for(int i = 0; i < dimensions; i++)
        if(dimensions <= 3){
            representation[position] = frontPoint = ofPoint(buffer[position][0], buffer[position][1], buffer[position][2]);
            backPoint = representation[(position + 1 < buffer.size() ? position + 1 : 0)];
            differencePoint = frontPoint - backPoint;
        }
        
        position++;
        if(position == buffer.size()) position = 0;
        
    }
    
    void push(ofPoint point){
        
        vector<float> data(3);
        data[0] = point.x;
        data[1] = point.y;
        data[2] = point.z;
        push(data);
        
    }
    
    void clear(){
        dimensions = 0;
        position = 0;
        minimum = +INFINITY;
        maximum = -INFINITY;
        minDimension = maxDimension = -1;
        buffer.clear();
        minimums.clear();
        maximums.clear();
        averages.clear();
        differences.clear();
        representation.clear();
    }
    
    int size(){
        return buffer.size();
    }
    
    vector< vector<float> >& getBuffer(){
        return buffer;
    }
    
    ofPoint& getDifferenceAsPoint(){
        return differencePoint;
    }
    
    ofPoint& getFrontAsPoint(){
        return frontPoint;
    }
    
    ofPoint& getBackAsPoint(){
        return backPoint;
    }
    
    vector<float>& front(){
        int f = position - 1;
        if(f < 0) f = buffer.size() - 1;
        return buffer[f];
    }
    
    vector<float>& back(){
        int b = position + 1;
        if(b == buffer.size()) b = 0;
        return buffer[b];
    }
    
    vector<float>& getAverages(){
        return averages;
    }
    
    vector<float>& getMinimums(){
        return minimums;
    }
    vector<float>& getMaximums(){
        return maximums;
    }
    
    int getMaximum(){
        return maximum;
    }
    
    int getMinimum(){
        return minimum;
    }
    
    int getMaxDimension(){
        return getVecMaxIndex(minimums); // TODO: cache
    }
    
    int getMinDimension(){
        return getVecMinIndex(minimums); // TODO: cache
    }
    
    vector<ofPoint>& getRepresentation(){
        return representation;
    }
    
protected:
    
    int position;
    int dimensions;
    
    vector<float> minimums;
    vector<float> maximums;
    vector<float> averages;
    vector<float> differences;
    float minimum;
    float maximum;
    int minDimension;
    int maxDimension;
    
    ofPoint differencePoint, frontPoint, backPoint;
    
    vector< vector<float> > buffer;
    vector<ofPoint> representation;
    
};

#endif
