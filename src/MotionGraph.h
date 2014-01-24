//
//  MotionGraph.h
//  LAFTest
//
//  Created by gameover on 30/12/13.
//
//

#ifndef __H_MOTIONGRAPH
#define __H_MOTIONGRAPH

#include "ofMain.h"
#include "VectorUtils.h"
#include "ofxLogger.h"

class MotionGraph{
    
public:
    
    MotionGraph(){
        
    }
    
    ~MotionGraph(){
        
    }
    
    void setup(string path){
        
        ofxLogVerbose() << "Setting up motion graph with: " << path << endl;
        
        ofBuffer b = ofBufferFromFile(ofToDataPath(path));
        
        int lineCount = 0;
        
        while (!b.isLastLine()) {
            string line = b.getNextLine();
            
            if(lineCount == 0){
                
                vector<string> chunks = ofSplitString(line, "	");
                
                for(int i = 0; i < chunks.size(); i++){

                    alphabet.push_back(chunks[i]);
                }

                cout << alphabet << endl;
            }
            
            if(lineCount > 0){
                
                vector<string> chunks = ofSplitString(line, "	");
                
                string motion = "";
                vector<float > probabilities;
                vector<string> possibilities;
                
                for(int i = 0; i < chunks.size(); i++){

                    if(i == 0) motion = chunks[i];
                    if(i >  0){
                        float p = ofToFloat(chunks[i]);
                        probabilities.push_back(p);
                        if(p > 0) possibilities.push_back(alphabet[i - 1]);
                    }
                    
                }
                
                keys.push_back(motion);
                mgProbabilities[motion] = probabilities;
                mgPossibilities[motion] = possibilities;
                
                ofxLogVerbose() << motion << " == " << probabilities << " [" << probabilities.size() << " = " << alphabet.size() << "]" << endl;
            }
            
            lineCount++;
            
        }
        
    }
    
    
    
    void nestGraph(map<string, vector<string> > & otherGraph){
        
        mgPossibilitiesOther.clear();
        
        for(map<string, vector<string> >::iterator Oit = otherGraph.begin(); Oit != otherGraph.end(); ++Oit){
            
            map<string, vector<string> >& thisMap = mgPossibilitiesOther[Oit->first];
            vector<string>& restrictions = Oit->second;
            
            //cout << "Checking: " << Oit->first << " with " << restrictions << endl;
            
            for(map<string, vector<string> >::iterator Mit = mgPossibilities.begin(); Mit != mgPossibilities.end(); ++Mit){
                
                vector<string>& Opossibilities = thisMap[Mit->first];
                vector<string>& Mpossibilities = Mit->second;
                
                //cout << "    " << Mit->first << endl;
                
                for(int i = 0; i < Mpossibilities.size(); i++){
                    //cout << "        " << Mpossibilities[i];
                    if(contains(restrictions, Mpossibilities[i])){
                        //cout << " YES" << endl;
                        Opossibilities.push_back(Mpossibilities[i]);
                    }else{
                        //cout << " NO" << endl;
                    }
                }
            }
            
        }
    }
    
    string getRandomTransition(string motion){
        vector<string> possibilities = getPossibleTransitions(motion);
        return possibilities[(int)ofRandom(possibilities.size())];
    }
    
    vector<string>& getPossibleTransitions(string restriction, string motion){
        map<string, map<string, vector<string> > >::iterator Oit = mgPossibilitiesOther.find(restriction);
        assert(Oit != mgPossibilitiesOther.end()); // harsh
        map<string, vector<string> >& Omap = Oit->second;
        map<string, vector<string> >::iterator it = Omap.find(motion);
        assert(it != Omap.end()); // harsh
        return it->second;
    }
    
    vector<string>& getPossibleTransitions(string motion){
        map<string, vector<string> >::iterator it = mgPossibilities.find(motion);
        assert(it != mgPossibilities.end()); // harsh
        return it->second;
    }
    
    vector<string> getPossibleTransitionsCopy(string motion){
        map<string, vector<string> >::iterator it = mgPossibilities.find(motion);
        assert(it != mgPossibilities.end()); // harsh
        return it->second;
    }
    
    vector<string>& getPossibleTransitions(int motionIndex){
        assert(motionIndex < mgPossibilities.size());
        return getMapValueFromIndex(mgPossibilities, motionIndex);
    }
    
    map<string, vector<string> >& getPossibilitie(){
        return mgPossibilities;
    }
    
    vector<string>& getAlphabet(){
        return alphabet;
    }
    
    vector<string>& getKeys(){
        return keys;
    }
    
protected:
    
    map<string, vector<float>  > mgProbabilities;
    
    map<string, vector<string> > mgPossibilities;
    map<string, map<string, vector<string> > > mgPossibilitiesOther;
    
    vector<string>              alphabet;
    vector<string>              keys;
    
    friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
        ar & BOOST_SERIALIZATION_NVP(mgProbabilities);
        ar & BOOST_SERIALIZATION_NVP(mgPossibilities);
        ar & BOOST_SERIALIZATION_NVP(alphabet);
        if(version > 0){
            ar & BOOST_SERIALIZATION_NVP(keys);
        }
	}
    
};

//typedef Singleton<MotionGraph> BaseMotionGraphSingleton;					// Global declaration
//
//static MotionGraph * motionGraph = BaseMotionGraphSingleton::Instance();

BOOST_CLASS_VERSION(MotionGraph, 1)

#endif
