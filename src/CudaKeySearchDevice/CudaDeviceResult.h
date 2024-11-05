#ifndef _CUDA_DEVICE_RESULT
#define _CUDA_DEVICE_RESULT

// Structures that exist on both host and device side
struct CudaDeviceResult {
    int thread;
    int block;
    int idx;
    bool compressed;
    unsigned int x[8];
    unsigned int y[8];
    unsigned int digest[5];
};

#endif