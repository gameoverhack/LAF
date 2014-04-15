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
        
        buffer.resize(dimensions);
        
        minimums.assign(dimensions, minimum);
        maximums.assign(dimensions, maximum);

        representation.resize(s);
        
        for(int i = 0; i < dimensions; i++){
            buffer[i].resize(s);
        }
        
    }
    
    void push(vector<float> data){
        
        assert(buffer.size() > 0);
        assert(data.size() == dimensions);
        for(int i = 0; i < dimensions; i++){
            
            // store data
            buffer[i][position] = data[i];
            
            // cache min and max values
            if(data[i] > maximums[i]) maximums[i] = data[i];
            if(data[i] < minimums[i]) minimums[i] = data[i];
            if(data[i] > maximum) maximum = data[i];
            if(data[i] < minimum) minimum = data[i];
            
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
        representation[position] = point;
        
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
        representation.clear();
    }
    
    int size(){
        return buffer.size();
    }
    
    vector< vector<float> >& getBuffer(){
        return buffer;
    }
    
    vector<float> front(){
        int f = position - 1;
        if(f < 0) f = buffer.size() - 1;
        return buffer[f];
    }
    
    vector<float> back(){
        int b = position + 1;
        if(b == buffer.size()) b = 0;
        return buffer[b];
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
    float minimum;
    float maximum;
    int minDimension;
    int maxDimension;
    
    
    vector< vector<float> > buffer;
    vector<ofPoint> representation;
    
    
};

#endif
