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

enum FlowDirection{
    FLOW_LEFT = 0,
    FLOW_RIGHT,
    FLOW_UP,
    FLOW_DOWN
};

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
            
            // calculate + cache averages and differences between front and back
            averages[i] = getVecAvg(buffer[position]);
            differences[i] = buffer[position][i] - buffer[(position + 1 < buffer.size() ? position + 1 : 0)][i];

            // cache min and max values
            if(data[i] > maximums[i]) maximums[i] = data[i];
            if(data[i] < minimums[i]) minimums[i] = data[i];
            if(data[i] > maximum) maximum = data[i];
            if(data[i] < minimum) minimum = data[i];
            
        }
        
        // should i do this for > 3 dimensions???
        if(dimensions <= 3){
            
            // cache an ofPoint version of the data
            representation[position] = ofPoint(buffer[position][0], buffer[position][1], buffer[position][2]);
            
            // calc + cache front + back values in the buffer
            frontPoint = representation[position];
            backPoint = representation[(position + 1 < buffer.size() ? position + 1 : 0)];
            
            // calc + cache difference, average and angle (of flow)
            differencePoint = frontPoint - backPoint;
            averagePoint = ofPoint(averages[0], averages[1], averages[2]);
            angle = ofRadToDeg(atan2(differencePoint.x, differencePoint.y));
            
            // calc + cache 'direction' of flow
            if(angle > 45.0f && angle <= 135.0f) direction = FLOW_RIGHT;
            if((angle > 135.0f && angle <= 180.0f) || (angle >= -180.0f && angle <= -135.0f)) direction = FLOW_UP;
            if(angle > -135.0f && angle <= -45.0f) direction = FLOW_LEFT;
            if((angle > -45.0f && angle <= 0.0f) || (angle > 0.0f && angle < 45.0f)) direction = FLOW_DOWN;
            
        }
        
        // update position in buffer, fyi -> we are pushing to the FRONT of the buffer!
        position++;
        if(position == buffer.size()) position = 0;
        
    }
    
    void push(ofPoint point){
        
        // convenience wrapper for ofPoint
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
    
    float getFlowAngle(){
        return angle;
    }
    
    FlowDirection getFlowDirection(){
        return direction;
    }
    
    string getFlowDirectionAsString(){
        return getFlowDirectionAsString(direction);
    }
    
    string getFlowDirectionAsString(FlowDirection f){
        switch(f){
            case FLOW_LEFT:
                return "FLOW_LEFT";
                break;
            case FLOW_RIGHT:
                return "FLOW_RIGHT";
                break;
            case FLOW_UP:
                return "FLOW_UP";
                break;
            case FLOW_DOWN:
                return "FLOW_DOWN";
                break;
        }
    };
    
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
    
    ofPoint& getAveragesAsPoint(){
        return averagePoint;
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
        return getVecMaxIndex(maximums); // TODO: cache
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
    
    ofPoint averagePoint, differencePoint, frontPoint, backPoint;
    
    float angle;
    FlowDirection direction;
    
    vector< vector<float> > buffer;
    vector<ofPoint> representation;
    
};

#endif
