//
//  PlayerView.h
//  LAFTest
//
//  Created by gameover on 30/12/13.
//
//

#ifndef __H_PLAYERVIEW
#define __H_PLAYERVIEW

#include "ofMain.h"
#include "MSAOpenCL.h"

enum RenderMode{
    RENDER_NORMAL,
    RENDER_TRANSITION
};

class PlayerView {
    
public:
    
    PlayerView(){
        
    }
    
    ~PlayerView(){
        
    }
    
    void setup(float w, float h, int s){
        
        renderMode = RENDER_NORMAL;
        
        width = w;
        height = h;
        stride = s;
        zdistance = 0.0f;

        numParticles = (width * height / stride);
        
        particlesPos = new float[numParticles * 3];
        particlesCol = new float[numParticles * 3];
        
        openCL.setupFromOpenGL();
        
        clImage[0].initWithTexture(width, height, GL_BGRA);
        clImage[1].initWithTexture(width, height, GL_BGRA);
        
        glGenBuffersARB(1, vbo);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float) * numParticles * 3, particlesPos, GL_DYNAMIC_COPY_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        
        clMemPosVBO.initFromGLObject(vbo[0]);
        
        glGenBuffersARB(1, cbo);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, cbo[0]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float) * numParticles * 3, particlesCol, GL_DYNAMIC_COPY_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        
        clMemColCBO.initFromGLObject(cbo[0]);
        
        for(int i = 0; i < 4; i++){
            particlesTrns[i] = new float[numParticles * 3];
            clMemTrns[i].initBuffer(sizeof(float) * numParticles * 3, CL_MEM_READ_WRITE, particlesTrns[i]);
        }
        
//        bDrawBoundingBox = false;
//        minMax = new int[4];
//        minMax[0] = INFINITY; minMax[1] = INFINITY; minMax[2] = -INFINITY; minMax[3] = -INFINITY;
//        clMemMinMax.initBuffer(sizeof(int) * 4, CL_MEM_READ_WRITE, minMax);
        
        // load and compile OpenCL program
        openCL.loadProgramFromFile("ParticleVideo.cl");
        
        openCL.loadKernel("particlize");
        openCL.loadKernel("morph");
        openCL.loadKernel("copy");
        
        msa::OpenCLKernel *kernel = openCL.kernel("particlize");
        kernel->setArg(0, clMemPosVBO.getCLMem());
        kernel->setArg(1, clMemColCBO.getCLMem());
//        kernel->setArg(6, clMemMinMax.getCLMem());

        fbo.allocate(width, height, GL_BGRA);
        
    }
    
    void setTransitionStartPixels(ofPixels & pixels){
        
        msa::OpenCLKernel *kernel = openCL.kernel("particlize");
        cl_float cl_width = (cl_float)width;
        cl_int cl_stride = (cl_int)stride;
        cl_float cl_zdistance = 50;
        
        if(pixels.getPixels() != NULL) clImage[0].write(pixels.getPixels());
        
        kernel->setArg(0, clMemTrns[0].getCLMem());
        kernel->setArg(1, clMemTrns[1].getCLMem());
        kernel->setArg(2, cl_width);
        kernel->setArg(3, cl_stride);
        kernel->setArg(4, cl_zdistance);
        kernel->setArg(5, clImage[0].getCLMem());
        
//        minMax[0] = INFINITY; minMax[1] = INFINITY; minMax[2] = -INFINITY; minMax[3] = -INFINITY;
//        clMemMinMax.write(minMax, 0, sizeof(int) * 4);
        kernel->run2D(width, height);
//        clMemMinMax.read(minMax, 0, sizeof(int) * 4);
//        boundingBox = ofRectangle(minMax[0], minMax[1], minMax[2] - minMax[0], minMax[3] - minMax[1]);
        
        openCL.finish();
        
    }
    
    void setTransitionEndPixels(ofPixels & pixels){
        
        msa::OpenCLKernel *kernel = openCL.kernel("particlize");
        cl_float cl_width = (cl_float)width;
        cl_int cl_stride = (cl_int)stride;
        cl_float cl_zdistance = zdistance;
        
        if(pixels.getPixels() != NULL) clImage[1].write(pixels.getPixels());
        
        kernel->setArg(0, clMemTrns[2].getCLMem());
        kernel->setArg(1, clMemTrns[3].getCLMem());
        kernel->setArg(2, cl_width);
        kernel->setArg(3, cl_stride);
        kernel->setArg(4, cl_zdistance);
        kernel->setArg(5, clImage[1].getCLMem());
        
//        minMax[0] = INFINITY; minMax[1] = INFINITY; minMax[2] = -INFINITY; minMax[3] = -INFINITY;
//        clMemMinMax.write(minMax, 0, sizeof(int) * 4);
        kernel->run2D(width, height);
//        clMemMinMax.read(minMax, 0, sizeof(int) * 4);
//        boundingBox = ofRectangle(minMax[0], minMax[1], minMax[2] - minMax[0], minMax[3] - minMax[1]);
        
    }
    
    void setPixels(ofPixels & pixels){
        
        if(renderMode == RENDER_TRANSITION) return;

        if(pixels.getPixels() != NULL) clImage[0].write(pixels.getPixels());
        
        msa::OpenCLKernel *kernel = openCL.kernel("particlize");
        cl_float cl_width = (cl_float)width;
        cl_int cl_stride = (cl_int)stride;
        cl_float cl_zdistance = (cl_float)zdistance;
        
        kernel->setArg(0, clMemPosVBO.getCLMem());
        kernel->setArg(1, clMemColCBO.getCLMem());
        kernel->setArg(2, cl_width);
        kernel->setArg(3, cl_stride);
        kernel->setArg(4, cl_zdistance);
        kernel->setArg(5, clImage[0].getCLMem());
        kernel->run2D(width, height);
        
        
//        minMax[0] = INFINITY; minMax[1] = INFINITY; minMax[2] = -INFINITY; minMax[3] = -INFINITY;
//        clMemMinMax.write(minMax, 0, sizeof(int) * 4);
        
//        clMemMinMax.read(minMax, 0, sizeof(int) * 4);
//        boundingBox = ofRectangle(minMax[0], minMax[1], minMax[2] - minMax[0], minMax[3] - minMax[1]);
        
    }
    
    void setTransitionLength(int frames){
        tTotalTime = frames * (1.0f / 25.0f * 1000.0f);
    }
    
    void setRenderMode(RenderMode mode){
        renderMode = mode;
        if(mode == RENDER_TRANSITION) tStartTime = ofGetElapsedTimeMillis();
    }
    
    RenderMode getRenderMode(){
        return renderMode;
    }
    
    void update(){
        
        if(renderMode == RENDER_TRANSITION){

            tween = (cl_float)(ofGetElapsedTimeMillis() - tStartTime) / (cl_float)tTotalTime;
            
            if(tween >= 1.0f){
                renderMode = RENDER_NORMAL;
            }else{
                msa::OpenCLKernel *kernel = openCL.kernel("morph");
                kernel->setArg(0, clMemTrns[0].getCLMem());
                kernel->setArg(1, clMemTrns[1].getCLMem());
                kernel->setArg(2, clMemTrns[2].getCLMem());
                kernel->setArg(3, clMemTrns[3].getCLMem());
                kernel->setArg(4, clMemPosVBO.getCLMem());
                kernel->setArg(5, clMemColCBO.getCLMem());
                kernel->setArg(6, tween);
                
                kernel->run1D(numParticles);
                openCL.finish();
            }

        }
        
    }
    
    void drawParticles(){
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        openCL.finish();
        
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, cbo[0]);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        
        glDrawArrays(GL_POINTS, 0, numParticles);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        
    }
    
    void drawImage(){
        
        glColor3f(1.0f, 1.0f, 1.0f);
        clImage[0].getTexture().draw(0, 0, width, height);
        
    }
    
    void setZDistance(float z){
        zdistance = z;
    }
    
    float getZDistance(){
        return zdistance;
    }
    
    float getTween(){
        return (float)tween;
    }
    
//    ofRectangle getBoundingBox(){
//        return boundingBox;
//    }
//    
//    void setDrawBoundingBox(bool b){
//        bDrawBoundingBox = b;
//    }
//    
//    void toggleDrawBoundingBox(){
//        bDrawBoundingBox = !bDrawBoundingBox;
//    }
//    
//    bool getDrawBoundBox(){
//        return bDrawBoundingBox;
//    }
    
protected:
    
    ofFbo               fbo;
    
    msa::OpenCL			openCL;
    msa::OpenCLImage	clImage[2];
    
    GLuint				vbo[1];
    GLuint				cbo[1];
    
//    ofRectangle         boundingBox;
//    int*				minMax;
//    msa::OpenCLBuffer	clMemMinMax;
    
    float*				particlesPos;
    msa::OpenCLBuffer	clMemPosVBO;
    
    float*               particlesCol;
    msa::OpenCLBuffer	 clMemColCBO;
    
    float*               particlesTrns[4];
    msa::OpenCLBuffer	 clMemTrns[4];
    
    bool bDrawBoundingBox;
    
    float width, height;
    int stride;
    float zdistance;
    int numParticles;
    RenderMode renderMode;
    
    int tStartTime, tTotalTime;
    cl_float tween;
    
};

#endif
