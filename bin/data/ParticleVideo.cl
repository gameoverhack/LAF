//#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
//#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable

//--------------------------------------------------------------
__kernel void particlize(__global float* posBuffer, __global float* colBuffer, const float width, const int stride, const float zdistance, read_only image2d_t srcImage){//, __global int* minMax){

    int i = get_global_id(0);
	int j = get_global_id(1);
    
    if(i * j % stride == 0){
        int2 coords1 = (int2)(i, j);
        int id = (j * width + i);
        
        float4 color = read_imagef(srcImage, CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST, coords1);
        
        float thresh = 0.1;

        int index = (id / stride) * 3;
        
        if(color[0] > thresh &&
           color[1] > thresh &&
           color[2] > thresh){
            
//            atomic_min(&minMax[0], i);
//            atomic_min(&minMax[1], j);
//            atomic_max(&minMax[2], i);
//            atomic_max(&minMax[3], j);
            
            posBuffer[index + 0] = i;
            posBuffer[index + 1] = j;
            posBuffer[index + 2] = ( (color[0] + color[1] + color[2]) / 3.0f ) * zdistance;
            colBuffer[index + 0] = color[0];
            colBuffer[index + 1] = color[1];
            colBuffer[index + 2] = color[2];
                 
        }else{

            posBuffer[index + 0] = i;//width / 2.0;
            posBuffer[index + 1] = j;//width / 2.0;
            posBuffer[index + 2] = 20;
            colBuffer[index + 0] = 0;
            colBuffer[index + 1] = 0;
            colBuffer[index + 2] = 0;
            
        }
    }
    
}

//--------------------------------------------------------------
__kernel void morph(__global float* orgBuffer1, __global float* orgBuffer2, __global float* dstBuffer1, __global float* dstBuffer2, __global float* posBuffer, __global float* colBuffer, const float tween){
    
    int i = get_global_id(0);
    
    posBuffer[i * 3 + 0] = orgBuffer1[i * 3 + 0] * (1 - tween) + dstBuffer1[i * 3 + 0] * (tween);
    posBuffer[i * 3 + 1] = orgBuffer1[i * 3 + 1] * (1 - tween) + dstBuffer1[i * 3 + 1] * (tween);
    posBuffer[i * 3 + 2] = orgBuffer1[i * 3 + 2] * (1 - tween) + dstBuffer1[i * 3 + 2] * (tween);
    
    colBuffer[i * 3 + 0] = (orgBuffer2[i * 3 + 0] * (1 - tween) + dstBuffer2[i * 3 + 0] * (tween));// * tween;
    colBuffer[i * 3 + 1] = (orgBuffer2[i * 3 + 1] * (1 - tween) + dstBuffer2[i * 3 + 1] * (tween));// * tween;
    colBuffer[i * 3 + 2] = (orgBuffer2[i * 3 + 2] * (1 - tween) + dstBuffer2[i * 3 + 2] * (tween));// * tween;
    
}

//--------------------------------------------------------------
__kernel void copy(__global float* orgBuffer1, __global float* orgBuffer2, __global float* dstBuffer1, __global float* dstBuffer2){
    
    int i = get_global_id(0);
    
    dstBuffer1[i * 3 + 0] = orgBuffer1[i * 3 + 0];
    dstBuffer1[i * 3 + 1] = orgBuffer1[i * 3 + 1];
    dstBuffer1[i * 3 + 2] = orgBuffer1[i * 3 + 2];
    
    dstBuffer2[i * 3 + 0] = orgBuffer2[i * 3 + 0];
    dstBuffer2[i * 3 + 1] = orgBuffer2[i * 3 + 1];
    dstBuffer2[i * 3 + 2] = orgBuffer2[i * 3 + 2];
    
}